/* Gemma 1, 2 and 3 implemented primarily in C++26.
 *
 * This is a from-scratch inference engine for Google's Gemma family of
 * language models: Gemma 1, 2 and 3, text-only and multimodal (SigLIP vision
 * tower included). It is not a wrapper around llama.cpp or
 * ggml, the transformer, the attention masks, the RoPE tables, the int8
 * kernels, the BPE tokenizer, and the image decoder / resizer are all
 * implemented here. I did this mostly to actually understand how Gemma
 * works end to end, and the by-product is a codebase that's small enough to
 * read in an afternoon. It's inspired by karpathy/llama2.c, which is the
 * same idea applied to Llama.
 *
 * Features:
 *
 *   - Gemma 1 / 2 / 3, text-only and multimodal, no third-party runtime deps
 *   - Hand-written GEMM/GEMV kernels (fpx and int8), packed/tiled, OpenMP
 *     across heads/rows
 *   - W8A8 quantization (per-channel weights, per-token activations) with a
 *     one-shot `-q` export flag
 *   - Hybrid sliding-window / full attention (Gemma 2/3) with a
 *     FlashAttention-style causal tiling scheme
 *   - Full SigLIP vision encoder with bidirectional image-token attention,
 *     plus a pan & scan crop utility for high-aspect-ratio images
 *   - mmap'd weight loading (falls back to a plain read on `--no-mmap`)
 *   - KV-cache streaming generation, chunked prefill
 *   - Temperature / top-k / top-p / repetition-penalty sampling
 *   - float32 / float16 / bfloat16 weights, selectable at compile time
 *   - A from-scratch BPE tokenizer, weights embedded straight into the
 *     `.bin` file by the export script
 *   - Runs on Linux, macOS, and Windows (MinGW), with GCC, Clang or MSVC
 *
 * Model files are a custom binary format produced by the accompanying
 * `export.py`, which converts a HuggingFace Gemma checkpoint (weights,
 * config, tokenizer vocab + BPE merges) into this inference-ready layout,
 * applying the activation rescaling (>:O) and optional W8A8 quantization.
 *
 * Build it:
 *
 *   ```bash
 *   make
 *   ```
 *
 * `make` builds the C++26 inference engine and compiles the bundled STB image
 * codec as C for compatibility with that library's implementation details.
 *
 * Weight dtype defaults to float32; override with -DDTYPE at compile time
 * (FP32 / FP16 / BF16), or `make DTYPE=BF16`:
 *
 *   ```bash
 *   make DTYPE=BF16
 *   ```
 *
 * (This has to match the dtype the .bin file was exported with.)
 *
 * Basic usage:
 *
 *   ```bash
 *   ./gemma model.bin --prompt "Hello I'm a language model, "
 *   ```
 *
 * Interactive chat, maintaining history across turns with Gemma's own
 * <start_of_turn>/<end_of_turn> template:
 *
 *   ```bash
 *   ./gemma model.bin --chat
 *   ```
 *
 * Multimodal models take images inline via @image{...}:
 *
 *   ```bash
 *   ./gemma model.bin --prompt "Looking at @image{photo.jpg}, we can see"
 *   ```
 *
 *   ```
 *   > Describe what you see in @image{photo.jpg}.
 *   ```
 *
 * The image gets decoded, resized to the model's expected input size, run
 * through the vision encoder, and the resulting soft tokens get spliced into
 * the text sequence between <start_of_image>/<end_of_image>. Pass
 * `--text-only` to skip loading the vision tower entirely and save memory on
 * a text-only workload.
 *
 * For very wide/tall images, @longimage{...} runs Gemma 3's pan & scan crop
 * utility first, so the model gets both the full downsized image and a few
 * higher-resolution crops of it:
 *
 *   ```
 *   > What's happening at the bottom of @longimage{long_screenshot.png}?
 *   ```
 *
 * `--seqlen`, `--temperature`, `--topk`, `--topp`, `--rpen`, `--seed` and
 * friends cover the usual sampling knobs; run `--help` / `-?` for the full
 * list.
 *
 * Code layout, roughly top to bottom: cross-platform shims (UTF-8 console
 * handling) -> embedded stb_image(_resize2) blob -> config
 * structs and the .bin (de)serialization -> BPE tokenizer -> GEMM/GEMV
 * kernels (fpx and int8) -> SigLIP vision forward pass -> Gemma transformer
 * forward pass (single-token decode, chunked prefill, and the combined
 * text+vision path) -> sampling -> chat template + CLI + main loop. The
 * matmul kernels in particular are written to be swappable in isolation if
 * you want to drop in something faster (BLAS, a hand-tuned SIMD kernel,
 * whatever) without touching the model code around them.
 *
 * v1.0  (09/11/2026)
 *
 * @TerryGuo (https://github.com/terryguo3180-eng | terry.guo2021@outlook.com)
 */

// Kill MSVC CRT "use *_s instead" deprecation spam on Windows (clang + cl)
#if defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#else
#  define _POSIX_C_SOURCE 200809L
#endif

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stb_image.h"
#include <algorithm>
#include <compare>
#include <functional>
#include <mdspan>
#include <memory>
#include <meta>
#include <new>
#include <optional>
#include <print>
#include <ranges>
#include <stdexcept>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>
#include <variant>

import cli;
import io;

#if defined(_MSC_VER) && !defined(__clang__)
// clang defines _MSC_VER on Windows, so we have to exclude __clang__
#  define __MSVC__
#endif
#if defined(__GNUC__) && !defined(__clang__)
// clang sometimes defines __GNUC__ as well, kind of funny
#  define __GCC__
#endif

#if defined(__clang__) || defined(__GCC__)
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined(__MSVC__)
#  include <malloc.h>
#  define RESTRICT      __restrict
#elif defined(__cplusplus)
#  define RESTRICT __restrict
#else
#  define RESTRICT restrict
#endif

// A bunch of hairy cross-platform code, unrelated to the inference engine
#if defined(_WIN32)
#  include <time.h>
#  include <windows.h>

// Global interruption flag
static volatile LONG g_interrupted = 0;

/* */
static inline void
set_interrupted(void)
{
  InterlockedExchange(&g_interrupted, 1);
}

/* */
static inline int
is_interrupted(void)
{
  return InterlockedCompareExchange(&g_interrupted, 0, 0) != 0;
}

// UTF-8 console encoding support on Windows

/* */
static void
set_utf8_console(void)
{
  // Set encoding to UTF-8
  SetConsoleOutputCP(65001);
  SetConsoleCP(65001);
}

/* Convert Windows command line to UTF-8 argc/argv */
static char **
get_utf8_argv(int *argc_out)
{
  wchar_t **wargv = CommandLineToArgvW(GetCommandLineW(), argc_out);
  if (!wargv) return NULL;

  char **argv = malloc((*argc_out + 1) * sizeof(char *));
  if (!argv)
  {
    LocalFree(wargv);
    return NULL;
  }

  for (int i = 0; i < *argc_out; i++)
  {
    int size =
      WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, NULL, 0, NULL, NULL);
    argv[i] = malloc(size);
    if (!argv[i])
    {
      for (int j = 0; j < i; j++)
      {
        free(argv[j]);
      }

      free(argv);
      LocalFree(wargv);
      return NULL;
    }
    WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, argv[i], size, NULL, NULL);
  }
  argv[*argc_out] = NULL;

  LocalFree(wargv);
  return argv;
}

/* */
static void
free_utf8_argv(char **argv, int argc)
{
  if (argv == NULL) return;
  for (int i = 0; i < argc; i++)
  {
    free(argv[i]);
  }
  free(argv);
}

// Interruption handling on Windows

#  define SLEEP_SEC(sec) Sleep((sec) * 1000)
#  define GETPID()       GetCurrentProcessId()
/* */
BOOL WINAPI
console_handler_(DWORD dwCtrlType)
{
  switch (dwCtrlType)
  {
    case CTRL_C_EVENT:         // Ctrl+C
    case CTRL_BREAK_EVENT:     // Ctrl+Break
    case CTRL_CLOSE_EVENT:     // Closed console window
    case CTRL_LOGOFF_EVENT:    // User logoff
    case CTRL_SHUTDOWN_EVENT:  // System shutdown
      set_interrupted();
      return TRUE;
    default:
      return FALSE;
  }
}

#else
#  include <time.h>
#  include <unistd.h>

#  define SLEEP_SEC(sec) sleep(sec)
#  define GETPID()       getpid()

static volatile sig_atomic_t g_interrupted = 0;

/* */
static inline void
set_interrupted(void)
{
  g_interrupted = 1;
}

/* */
static inline int
is_interrupted(void)
{
  return g_interrupted != 0;
}

/* */
static void
set_utf8_console(void)
{
}

/* */
static char **
get_utf8_argv(int *argc_out)
{
  (void)argc_out;
  return NULL;
}

/* */
static void
free_utf8_argv(char **argv, int argc)
{
  (void)argv;
  (void)argc;
}
#endif  // _WIN32

/* */
static void
signal_handler(int signum)
{
  (void)signum;
  set_interrupted();
}

/* */
void
setup_signal_handler(void)
{
#if defined(_WIN32)
  if (!SetConsoleCtrlHandler(console_handler_, TRUE))
  {
    // Should almost never happen
    signal(SIGINT, signal_handler);
  }
#else
  struct sigaction sa;
  sa.sa_handler = signal_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  if (sigaction(SIGINT, &sa, NULL) == -1)
  {
    perror("sigaction(SIGINT)");
  }
  if (sigaction(SIGTERM, &sa, NULL) == -1)
  {
    perror("sigaction(SIGTERM)");
  }
#endif  // _WIN32
}

// Cross-platform wall time

/* */
static double
now_sec(void)
{
#if defined(_WIN32)
  static LARGE_INTEGER freq;
  static int           freq_init = 0;
  LARGE_INTEGER        counter;

  if (!freq_init)
  {
    QueryPerformanceFrequency(&freq);
    freq_init = 1;
  }
  QueryPerformanceCounter(&counter);
  return (double)counter.QuadPart / (double)freq.QuadPart;
#else
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
#endif
}

/* This giant blob is a self-contained, pre-processed implementation of
 * "stb_image" and "stb_image_resize2". :D
 *
 * I want to keep this project in a single .c file, to avoid requiring users to
 * obtain the original stb headers, and to keep the source code free of tens of
 * thousands of lines of third-party code, I just took the source code of the
 * two libraries, compressed / minified them into a "code blob" and embedded
 * directly here. The functions declared `extern` remain visible. (This blob
 * polluted the global namespace quite seriously, but I'm good with that ;-)
 *
 * The blob is equivilant to the following code if you have the two headers:
 *
 * ```c
 * #define STBI_NO_SIMD
 * #define STBI_ONLY_JPEG
 * #define STBI_ONLY_PNG
 * #define STB_IMAGE_IMPLEMENTATION
 * #include "stb_image.h"
 *
 * #define STBIR_NO_SIMD
 * #define STBIR_USE_FMA
 * #define STB_IMAGE_RESIZE_IMPLEMENTATION
 * #include "stb_image_resize2.h"
 * ```
 *
 * The blob is treated as a black box, please do not edit it. Skip this part if
 * you are reading the code.
 */

/* Image codec implementation is compiled as C in stb_image.c. */
/* Wrapper for stbi_load to handle utf-8 filenames properly */
unsigned char *
load_image_utf8(const char *utf8_path, int *w, int *h, int *ch, int req_comp)
{
#ifdef _WIN32
  // Windows: convert utf-8 -> utf-16 and open with _wfopen
  int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8_path, -1, NULL, 0);
  if (wlen == 0) return NULL;

  wchar_t *wpath = (wchar_t *)malloc(wlen * sizeof(wchar_t));
  MultiByteToWideChar(CP_UTF8, 0, utf8_path, -1, wpath, wlen);

  FILE *f = _wfopen(wpath, L"rb");
  free(wpath);
  if (!f) return NULL;

  unsigned char *data = stbi_load_from_file(f, w, h, ch, req_comp);
  fclose(f);
  return data;
#else
  return stbi_load(utf8_path, w, h, ch, req_comp);
#endif
}

// The actual inference code starts here :D

// Block size of blockwise causal masking
#define QK_BLOCK_SIZE 64

// Block sizes for the m/n/k cache-blocked GEMM
#define GEMM_MC 128  // rows of A resident per (jc, ic) tile block, mult of 8
#define GEMM_NC 256  // cols of B packed per outer jc block, mult of 8
#define GEMM_KC 256  // k-length streamed per inner pc chunk

// OpenMP parameters
#define OMP_NUM_THREADS 8
#define OMP_PARA_ARGS   num_threads(OMP_NUM_THREADS)
#if defined(__MSVC__) && !defined(MSVC_OMP_COLLAPSE)
// Most versions of MSVC does not support the `collapse` clause
#  define OMP_COLLAPSE2
#else
#  define OMP_COLLAPSE2 collapse(2)
#endif

// Prompt used in pan & scan
// google/gemma_pytorch/blob/main/gemma/gemma3_preprocessor.py
#define CROPPED_IMAGE_PREFIX "here is the original image"
#define CROPPED_IMAGE_FILTER "and here are some crops to help you see better"

// Default parameters used in pan & scan
// google/gemma_pytorch/blob/main/gemma/siglip_vision/pan_and_scan.py
#define MIN_CROP_SIZE 256
#define MAX_NUM_CROPS 4

// Macros for different dtypes
// TODO: dispatch dtype at runtime using the `#include __FILE__` trick
#define FP16 1
#define BF16 2
#define FP32 3

// Default dtype
#if !defined(DTYPE)
#  define DTYPE FP32
#endif

// clang-format off
#if DTYPE == FP16
#  define floatx     _Float16
#  define DTYPE_MAGIC  "f16"
#  define FLOATX_MAX (((union {floatx f; uint16_t b; }){.b = 0x00007BFF}).f)
#elif DTYPE == BF16
#  define floatx     __bf16
#  define DTYPE_MAGIC  "b16"
#  define FLOATX_MAX (((union {floatx f; uint16_t b; }){.b = 0x00007F7F}).f)
#elif DTYPE == FP32
#  define floatx     float
#  define DTYPE_MAGIC  "f32"
#  define FLOATX_MAX (((union {floatx f; uint32_t b; }){.b = 0x7F7FFFFF}).f)
#else
#  error "unsupported DTYPE"
#endif
// clang-format on

// no-fast-math
#if defined(__GCC__)
#  define NO_FAST_MATH_BEGIN                                                   \
    _Pragma("GCC push_options") _Pragma("GCC optimize (\"no-fast-math\")")
#  define NO_FAST_MATH_END _Pragma("GCC pop_options")
#elif defined(__clang__) || defined(__MSVC__)
#  define NO_FAST_MATH_BEGIN                                                   \
    _Pragma("float_control(push)") _Pragma("float_control(precise, on)")
#  define NO_FAST_MATH_END _Pragma("float_control(pop)")
#endif

#define GENERATOR_EXIT 10000

#define TOSTRING(x) STRINGIFY_(x)
// I usually use a trailing underscore to indicate a variable/function/macro is
// temporary
#define STRINGIFY_(x) #x

#define ARRAYLEN(arr) (sizeof(arr) / sizeof((arr)[0]))

// Error logger

thread_local static char g_errmsg[128] = {0};

/* */
static int
log_error(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  int ret = vsnprintf(g_errmsg, sizeof(g_errmsg), fmt, ap);
  va_end(ap);
  return ret;
}

/* */
static char *
get_error(void)
{
  return g_errmsg;
}

// Memory wrappers

/* */
static bool
read_uint8_ckd(FILE *fp, uint8_t *out)
{
  int c = fgetc(fp);
  if (c == EOF)
  {
    log_error("file read failed");
    return false;
  }
  *out = (uint8_t)c;
  return true;
}

/* */
static bool
fseek_ckd(FILE *fp, long offset, int base)
{
  if (fseek(fp, offset, base) != 0)
  {
    log_error("file seek failed");
    return false;
  }
  return true;
}

/* */
static bool
fread_ckd(void *ptr, size_t count, size_t elt_size, FILE *fp)
{
  size_t got = fread(ptr, elt_size, count, fp);
  if (got != count)
  {
    log_error("file read failed (expected %zu, got %zu)", count, got);
    return false;
  }
  return true;
}

/* */
static bool
read_uint16_ckd(FILE *fp, uint16_t *out)
{
  unsigned char buf[2];
  if (!fread_ckd(buf, 2, 1, fp)) return false;
  *out = (uint16_t)(((uint16_t)buf[0] << 8) | (uint16_t)buf[1]);
  return true;
}

/* */
static bool
read_uint32_ckd(FILE *fp, uint32_t *out)
{
  unsigned char buf[4];
  if (!fread_ckd(buf, 4, 1, fp)) return false;
  *out = ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) |
         ((uint32_t)buf[2] << 8) | (uint32_t)buf[3];
  return true;
}

/* */
static bool
read_fp32_ckd(FILE *fp, float *out)
{
  uint32_t bits;
  if (!read_uint32_ckd(fp, &bits)) return false;
  memcpy(out, &bits, sizeof(float));
  return true;
}

template <typename T>
static bool
read_tensor_ckd(std::vector<T> &storage, T *&view, size_t count, FILE *fp)
{
  storage.resize(count);
  view = storage.data();
  return fread_ckd(view, count, sizeof(T), fp);
}

/* Pascal-style string: first byte is length, then that many chars */
static bool
read_str_ckd(char **var, FILE *fp, char *data, size_t *offset)
{
  uint8_t len;
  if (!read_uint8_ckd(fp, &len)) return false;

  *var = data + *offset;
  if (!fread_ckd(*var, (size_t)len, 1, fp)) return false;

  data[*offset + (size_t)len] = '\0';
  *offset += (size_t)len + 1;
  return true;
}

/* Peek at how many bytes a sequence of pascal strings will occupy */
static size_t
get_strarr_bytes(FILE *fp, int count)
{
  // Read a sequence of pascal-style strings
  size_t offset = 0;
  long   pos    = ftell(fp);
  if (pos == -1L)
  {
    log_error("ftell failed");
    return (size_t)-1;
  }
  // Get the total number of bytes
  for (int i = 0; i < count; i++)
  {
    uint8_t len;
    if (!read_uint8_ckd(fp, &len)) return (size_t)-1;
    if (!fseek_ckd(fp, len, SEEK_CUR)) return (size_t)-1;
    offset += len + 1;
  }
  // Resume position
  if (!fseek_ckd(fp, pos, SEEK_SET)) return (size_t)-1;
  return offset;
}

/* Configuration for the SigLIP vision encoder */
struct VisionConfig
{
  uint8_t  n_layers;    // ViT layers
  uint8_t  n_heads;     // Attention heads
  uint16_t image_size;  // Pixels per image side
  uint16_t patch_size;  // Patches per image side
  uint16_t hidden_dim;  // ViT dimension
  uint16_t mlp_dim;     // Intermediate size in FFN
  float    eps;         // LayerNorm epsilon
};

/* */
static VisionConfig *
read_vision_config(FILE *fp)
{
  auto vcfg = std::make_unique<VisionConfig>();

  if (!read_uint8_ckd(fp, &vcfg->n_layers) ||
      !read_uint8_ckd(fp, &vcfg->n_heads) ||
      !read_uint16_ckd(fp, &vcfg->mlp_dim) ||
      !read_uint16_ckd(fp, &vcfg->hidden_dim) ||
      !read_uint16_ckd(fp, &vcfg->image_size) ||
      !read_uint16_ckd(fp, &vcfg->patch_size) || !read_fp32_ckd(fp, &vcfg->eps))
    return nullptr;

  return vcfg.release();

}

/* Configuration for the main Gemma text encoder */
struct TextConfig
{
  uint8_t  n_layers;       // Transformer layers
  uint8_t  n_heads;        // Attention heads
  uint8_t  n_kv_heads;     // Key/value heads (GQA)
  uint16_t head_dim;       // Dim per head
  uint16_t embed_dim;      // Model dimension
  uint16_t mlp_dim;        // Intermediate size in FFN
  uint16_t q_scale;        // Scale applied to queries before attention
  uint16_t slide_len;      // Sliding-window size
  uint16_t image_toks;     // Number of soft tokens per image
  uint32_t max_seqlen;     // Max number of position embeddings
  uint32_t vocab_size;     // Number of possible tokens
  float    local_theta;    // RoPE base for local (sliding) attention
  float    global_theta;   // RoPE base for full attention
  float    eps;            // RMSNorm epsilon
  float    att_softcap;    // tanh softcap on attention scores (0 = off)
  float    logit_softcap;  // tanh softcap on final logits (0 = off)
  std::vector<uint8_t>    att_layers;     // true = use sliding window for that layer
  bool     qk_norm;        // query/key RMSNorm
  bool     pre_mlp_norm;
  bool     pst_mlp_norm;
};

/* */
static TextConfig *
read_text_config(FILE *fp, bool *support_mm)
{
  auto tcfg = std::make_unique<TextConfig>();
  std::vector<char> att_layers_buf;

  if (!read_uint8_ckd(fp, &tcfg->n_layers) ||
      !read_uint8_ckd(fp, &tcfg->n_heads) ||
      !read_uint8_ckd(fp, &tcfg->n_kv_heads) ||
      !read_uint16_ckd(fp, &tcfg->head_dim) ||
      !read_uint16_ckd(fp, &tcfg->embed_dim) ||
      !read_uint16_ckd(fp, &tcfg->mlp_dim) ||
      !read_uint16_ckd(fp, &tcfg->q_scale) ||
      !read_uint16_ckd(fp, &tcfg->slide_len) ||
      !read_uint16_ckd(fp, &tcfg->image_toks) ||
      !read_uint32_ckd(fp, &tcfg->max_seqlen) ||
      !read_uint32_ckd(fp, &tcfg->vocab_size) ||
      !read_fp32_ckd(fp, &tcfg->local_theta) ||
      !read_fp32_ckd(fp, &tcfg->global_theta) ||
      !read_fp32_ckd(fp, &tcfg->eps) ||
      !read_fp32_ckd(fp, &tcfg->att_softcap) ||
      !read_fp32_ckd(fp, &tcfg->logit_softcap))
    return nullptr;

  // Packed bit-field of which layers use sliding-window attention
  // A terrible terrible idea, wish I didn't do this
  uint8_t n_bytes;
  if (!read_uint8_ckd(fp, &n_bytes)) return nullptr;
  if (n_bytes * 8 < tcfg->n_layers)
  {
    log_error("insufficient att_layers bytes");
    return nullptr;
  }
  att_layers_buf.resize(n_bytes);
  tcfg->att_layers.resize(tcfg->n_layers);
  if (!fread_ckd(att_layers_buf.data(), n_bytes, sizeof(char), fp))
    return nullptr;

  for (int i = 0; i < tcfg->n_layers; i++)
  {
    int pos             = i;
    int byte_idx        = pos / 8;
    int bit_idx         = 7 - (pos % 8);
    tcfg->att_layers[i] = (att_layers_buf[byte_idx] >> bit_idx) & 1;
  }

  // Extra feature flags packed into one byte
  uint8_t extra_flags;
  if (!read_uint8_ckd(fp, &extra_flags)) return nullptr;
  *support_mm        = (extra_flags & 8) == 8;
  tcfg->qk_norm      = (extra_flags & 4) == 4;
  tcfg->pre_mlp_norm = (extra_flags & 2) == 2;
  tcfg->pst_mlp_norm = (extra_flags & 1) == 1;

  return tcfg.release();

}

// Tokenizer implementation

/* */
struct Token
{
  std::string_view val;
  int   idx;
};

struct Merge
{
  std::string_view str1;
  std::string_view str2;
  int   rank;

  friend std::strong_ordering
  operator<=>(const Merge &a, const Merge &b)
  {
    if (auto cmp = a.str1 <=> b.str1; cmp != 0)
    {
      return cmp;
    }
    return a.str2 <=> b.str2;
  }
};

struct no_print_ {};
static constexpr no_print_ no_print;

/* */
struct GemmaTokenizer
{
  uint32_t n_merges;

  int vocab_size;
  int max_toklen;
  int bos;  // beginning of sequence
  int eos;  // end of sequence
  int sot;  // start of turn
  int eot;  // end of turn
  int soi;  // start of image
  int eoi;  // end of image
  int ist;  // image soft token

  [[=no_print]]
  std::vector<char> vocab_data;
  [[=no_print]]
  std::vector<char> merge_data;
  [[=no_print]]
  std::vector<char *> vocab;
  [[=no_print]]
  std::vector<Token> vocab_sorted;  // sorted for binary search
  [[=no_print]]
  std::vector<Merge> ranks;  // sorted merges for BPE
};

/* Look up a string in the sorted vocab -> token id (or -1) */
static int
get_token_idx(GemmaTokenizer *tok, std::string_view key)
{
  auto val = std::lower_bound(
    tok->vocab_sorted.begin(), tok->vocab_sorted.end(), key,
    [](const Token &token, std::string_view value) { return token.val < value; }
  );
  if (val == tok->vocab_sorted.end() || val->val != key)
  {
    return -1;
  }
  return val->idx;
}

/* Look up a (str1, str2) pair in the merges table */
static Merge *
get_merge_rec(GemmaTokenizer *tok, std::string_view str1, std::string_view str2)
{
  Merge  key = {.str1 = str1, .str2 = str2};
  auto val = std::lower_bound(tok->ranks.begin(), tok->ranks.end(), key);
  if (val == tok->ranks.end() || (*val <=> key) != 0)
  {
    return NULL;
  }
  return &*val;
}

/* */
static GemmaTokenizer *
read_tokenizer(FILE *fp, TextConfig *tcfg, bool support_mm)
{
  auto tok = std::make_unique<GemmaTokenizer>();
  size_t vocab_data_bytes = 0;
  size_t offset = 0;
  size_t merge_bytes = 0;

  tok->vocab_size = tcfg->vocab_size;
  if (support_mm)
  {
    tok->vocab_size++;
  }  // ++ for the <image_soft_token>

  vocab_data_bytes = get_strarr_bytes(fp, tok->vocab_size);
  if (vocab_data_bytes == (size_t)-1) return nullptr;

  tok->vocab_data.resize(vocab_data_bytes);
  tok->vocab.resize(tok->vocab_size);

  tok->vocab_sorted.resize(tok->vocab_size);

  offset = 0;
  tok->max_toklen = 0;
  for (int i = 0; i < tok->vocab_size; i++)
  {
    char *str;
    if (!read_str_ckd(&str, fp, tok->vocab_data.data(), &offset)) return nullptr;

    tok->vocab[i]            = str;
    tok->vocab_sorted[i].idx = i;
    tok->vocab_sorted[i].val = str;

    int len = strlen(str);
    if (len > tok->max_toklen)
    {
      tok->max_toklen = len;
    }
  }

  std::sort(
    tok->vocab_sorted.begin(), tok->vocab_sorted.end(),
    [](const Token &a, const Token &b) { return a.val < b.val; }
  );

  // Special tokens
  tok->bos = get_token_idx(tok.get(), "<bos>");
  tok->eos = get_token_idx(tok.get(), "<eos>");
  tok->sot = get_token_idx(tok.get(), "<start_of_turn>");
  tok->eot = get_token_idx(tok.get(), "<end_of_turn>");
  tok->soi = get_token_idx(tok.get(), "<start_of_image>");
  tok->eoi = get_token_idx(tok.get(), "<end_of_image>");
  tok->ist = get_token_idx(tok.get(), "<image_soft_token>");

  // Build merges
  if (!read_uint32_ckd(fp, &tok->n_merges)) return nullptr;

  tok->ranks.resize(tok->n_merges);

  merge_bytes = get_strarr_bytes(fp, tok->n_merges * 2);
  if (merge_bytes == (size_t)-1) return nullptr;

  tok->merge_data.resize(merge_bytes);

  offset = 0;
  for (uint32_t i = 0; i < tok->n_merges; i++)
  {
    char *str1, *str2;
    if (!read_str_ckd(&str1, fp, tok->merge_data.data(), &offset)) return nullptr;
    if (!read_str_ckd(&str2, fp, tok->merge_data.data(), &offset)) return nullptr;

    tok->ranks[i].rank = i;
    tok->ranks[i].str1 = str1;
    tok->ranks[i].str2 = str2;
  }

  std::sort(tok->ranks.begin(), tok->ranks.end());
  return tok.release();

}

/* Minimal byte-pair encoding algorithm implementation */
void
encode(
  GemmaTokenizer *tok,
  std::string_view text,
  int            *tokens,
  int             spos,
  int            *n_tokens
)
{
  const auto *ustr = reinterpret_cast<const unsigned char *>(text.data());

  // Convert UTF-8 string to tokens of individual codepoints
  size_t i  = 0;
  int tok_i = 0;
  tokens += spos;

  while (i < text.size())
  {
    /* table from https://zh.wikipedia.org/wiki/UTF-8
     *
     * U+00000-U+00007F  1  0xxxxxxx
     * U+00080-U+0007FF  2  110xxxxx  10xxxxxx
     * U+00800-U+00FFFF  3  1110xxxx  10xxxxxx  10xxxxxx
     * U+10000-U+1FFFFF  4  11110xxx  10xxxxxx  10xxxxxx  10xxxxxx
     */
    int n_bytes = 0;
    size_t start = i;

    if (ustr[i] >> 7 == 0)
    {
      n_bytes = 1;
    }
    else if (i + 1 < text.size() && ustr[i] >> 5 == 6 && ustr[i + 1] >> 6 == 2)
    {
      n_bytes = 2;
    }
    else if (i + 2 < text.size() && ustr[i] >> 4 == 14 && ustr[i + 1] >> 6 == 2 &&
             ustr[i + 2] >> 6 == 2)
    {
      n_bytes = 3;
    }
    else if (i + 3 < text.size() && ustr[i] >> 3 == 30 && ustr[i + 1] >> 6 == 2 &&
             ustr[i + 2] >> 6 == 2 && ustr[i + 3] >> 6 == 2)
    {
      n_bytes = 4;
    }
    else
    {
      n_bytes = 1;
    }

    char cstr[5];
    for (int b = 0; b < n_bytes; b++)
    {
      cstr[b] = (char)ustr[i++];
    }
    cstr[n_bytes] = '\0';

    int token = get_token_idx(tok, cstr);
    if (token == -1)
    {
      // Fallback to byte level tokens
      char bstr[10];
      for (size_t b = start; b < i; b++)
      {
        snprintf(bstr, 10, "<0x%02X>", (unsigned char)ustr[b]);
        tokens[tok_i++] = get_token_idx(tok, bstr);
      }
    }
    else
    {
      tokens[tok_i++] = token;
    }
  }

  // Keep merging the highest-rank pair until nothing left
  for (;;)
  {
    int    best_rank = 2147483647;
    const Merge *best_pair = NULL;

    // Find the merge with best rank
    for (int i = 0; i < tok_i - 1; i++)
    {
      char *str1 = tok->vocab[tokens[i]];
      char *str2 = tok->vocab[tokens[i + 1]];

      const Merge *merge = get_merge_rec(tok, str1, str2);
      if (merge != NULL && merge->rank < best_rank)
      {
        best_rank = merge->rank;
        best_pair = merge;
      }
    }

    if (best_pair == NULL) break;  // No more merges

    int i = 0;
    while (i < tok_i - 1)
    {
      Merge pair = {
        .str1 = tok->vocab[tokens[i]], .str2 = tok->vocab[tokens[i + 1]]
      };
      if (pair.str1 == best_pair->str1 && pair.str2 == best_pair->str2)
      {
        // Merge the pair, left shift all the tokens on its right side
        std::string merged(best_pair->str1);
        merged.append(best_pair->str2);
        tokens[i] = get_token_idx(tok, merged);
        for (int j = i + 1; j < tok_i - 1; j++)
        {
          tokens[j] = tokens[j + 1];
        }
        tok_i--;
      }
      i++;
    }
  }

  *n_tokens += tok_i;
}

/* */
const char *
decode(GemmaTokenizer *tok, int id, char *byte_buf)
{
  char *s = tok->vocab[id];
  // Byte-fallback tokens look like <0xAB>, turn them back into a raw byte
  size_t len = strlen(s);
  if (len == 6 && s[0] == '<' && s[1] == '0' && s[2] == 'x' && s[5] == '>')
  {
    unsigned int byte_val = 0;
    if (sscanf(s + 3, "%2x", &byte_val) == 1)
    {
      byte_buf[0] = (char)byte_val;
      byte_buf[1] = '\0';
      return byte_buf;
    }
  }
  return s;
}

// Weight storage

/* A tensor's pointers are views into owned vectors or the model mapping. */
struct QTensor
{
  bool quant = false;
  floatx *fpx = nullptr;
  struct
  {
    int8_t *q = nullptr;
    floatx *scales = nullptr;
  } i8;
  std::vector<floatx> fpx_data;
  std::vector<int8_t> q_data;
  std::vector<floatx> scales_data;
};

/* */
static inline size_t
qtensor_size(int m, int n, bool quant)
{
  if (!quant)
  {
    return (size_t)m * n * sizeof(floatx);
  }
  else
  {
    return (size_t)m * n * sizeof(int8_t) + (size_t)n * sizeof(floatx);
  }
}

// Linear layer weights: either plain floatx or int8 + per-channel scales
static void
read_qtensor(QTensor *w, FILE *fp, int m, int n, bool quant)
{
  if (!quant)
  {
    w->quant = false;
    read_tensor_ckd(w->fpx_data, w->fpx, (size_t)m * (size_t)n, fp);
  }
  else
  {
    w->quant = true;
    read_tensor_ckd(w->q_data, w->i8.q, (size_t)m * (size_t)n, fp);
    read_tensor_ckd(w->scales_data, w->i8.scales, (size_t)n, fp);
  }
}

/* */
static inline void
mmap_qtensor(QTensor *w, uint8_t *base, size_t *off, int m, int n, bool quant)
{
  if (!quant)
  {
    w->quant = false;
    w->fpx   = (floatx *)(base + *off);
    *off += (size_t)m * n * sizeof(floatx);
  }
  else
  {
    w->quant = true;
    w->i8.q  = (int8_t *)(base + *off);
    *off += (size_t)m * n * sizeof(int8_t);
    w->i8.scales = (floatx *)(base + *off);
    *off += (size_t)n * sizeof(floatx);
  }
}

/* One Gemma transformer block */
struct TextDecoderLayer
{
  // Attention weights
  // 2D weights are stored transposed for better GEMV cache use

  QTensor wq;  // (embed_dim, n_heads * head_dim).T
  QTensor wk;  // (embed_dim, n_kv_heads * head_dim).T
  QTensor wv;  // (embed_dim, n_kv_heads * head_dim).T
  QTensor wo;  // (n_heads * head_dim, embed_dim).T

  // Feedforward weights
  QTensor w1;  // (embed_dim, mlp_dim).T
  QTensor w2;  // (embed_dim, mlp_dim).T
  QTensor w3;  // (mlp_dim, embed_dim).T

  // RMSNorm weights (Gemma adds 1.0 to the weight)
  floatx *nq;  // (head_dim,)
  floatx *nk;  // (head_dim,)
  floatx *n1;  // (embed_dim,)
  floatx *n2;  // (embed_dim,)
  floatx *n3;  // (embed_dim,)
  floatx *n4;  // (embed_dim,)

  // Storage is empty when the corresponding pointer borrows mmap memory.
  std::vector<floatx> nq_data;
  std::vector<floatx> nk_data;
  std::vector<floatx> n1_data;
  std::vector<floatx> n2_data;
  std::vector<floatx> n3_data;
  std::vector<floatx> n4_data;
};

/* One SigLIP (vision) encoder block */
struct VisionEncoderLayer
{
  // ViT attention weights
  QTensor wq;  // (hidden_dim, hidden_dim).T
  QTensor wk;  // (hidden_dim, hidden_dim).T
  QTensor wv;  // (hidden_dim, hidden_dim).T
  QTensor wo;  // (hidden_dim, hidden_dim).T
  floatx  *bq;  // (hidden_dim,)
  floatx  *bk;  // (hidden_dim,)
  floatx  *bv;  // (hidden_dim,)
  floatx  *bo;  // (hidden_dim,)

  // ViT feedforward weights
  QTensor w1;  // (hidden_dim, mlp_dim).T
  QTensor w2;  // (hidden_dim, mlp_dim).T
  floatx  *b1;  // (mlp_dim,)
  floatx  *b2;  // (mlp_dim,)

  // ViT layernorm weights
  floatx *n1;    // (hidden_dim,)
  floatx *n2;    // (hidden_dim,)
  floatx *n1_b;  // (hidden_dim,)
  floatx *n2_b;  // (hidden_dim,)

  // Storage is empty when the corresponding pointer borrows mmap memory.
  std::vector<floatx> bq_data;
  std::vector<floatx> bk_data;
  std::vector<floatx> bv_data;
  std::vector<floatx> bo_data;
  std::vector<floatx> b1_data;
  std::vector<floatx> b2_data;
  std::vector<floatx> n1_data;
  std::vector<floatx> n2_data;
  std::vector<floatx> n1_b_data;
  std::vector<floatx> n2_b_data;
};

/* Text decoder container */
struct TextDecoder
{
  TextConfig config;
  // (vocab_size, embed_dim), shared with lm_head (tied weights)
  QTensor embedding;
  std::vector<TextDecoderLayer> layers;
  floatx            *final_norm;  // (embed_dim,)

  // Storage is empty when the corresponding pointer borrows mmap memory.
  std::vector<floatx> final_norm_data;
};

/* */
size_t
get_text_decoder_size(const TextConfig *tcfg, bool quant)
{
  size_t size       = 0;
  int    embed_dim  = tcfg->embed_dim;
  int    mlp_dim    = tcfg->mlp_dim;
  int    q_size     = tcfg->n_heads * tcfg->head_dim;
  int    kv_size    = tcfg->n_kv_heads * tcfg->head_dim;
  int    vocab_size = tcfg->vocab_size;

  // embedding: linear (embed_dim -> vocab_size)
  size += qtensor_size(embed_dim, vocab_size, quant);

  for (int l = 0; l < tcfg->n_layers; l++)
  {
    // wq, wk, wv, wo
    size += qtensor_size(embed_dim, q_size, quant);
    size += qtensor_size(embed_dim, kv_size, quant);
    size += qtensor_size(embed_dim, kv_size, quant);
    size += qtensor_size(q_size, embed_dim, quant);

    // optional nq, nk
    if (tcfg->qk_norm)
    {
      size += (size_t)tcfg->head_dim * sizeof(floatx);
      size += (size_t)tcfg->head_dim * sizeof(floatx);
    }

    // w1, w2, w3
    size += qtensor_size(embed_dim, mlp_dim, quant);
    size += qtensor_size(embed_dim, mlp_dim, quant);
    size += qtensor_size(mlp_dim, embed_dim, quant);

    // n1, n2
    size += (size_t)embed_dim * sizeof(floatx);
    size += (size_t)embed_dim * sizeof(floatx);

    // optional n3, n4
    if (tcfg->pre_mlp_norm)
    {
      size += (size_t)embed_dim * sizeof(floatx);
    }
    if (tcfg->pst_mlp_norm)
    {
      size += (size_t)embed_dim * sizeof(floatx);
    }
  }

  // final_norm
  size += (size_t)embed_dim * sizeof(floatx);

  return size;
}

/* */
static TextDecoder *
read_text_decoder(FILE *fp, TextConfig *tcfg, bool quant)
{
  auto dec = std::make_unique<TextDecoder>();
  int embed_dim = tcfg->embed_dim;
  int mlp_dim = tcfg->mlp_dim;
  int q_size = tcfg->n_heads * tcfg->head_dim;
  int kv_size = tcfg->n_kv_heads * tcfg->head_dim;
  int vocab_size = tcfg->vocab_size;
  dec->config = *tcfg;

  /* The embedding shape is (vocab_size, embed_dim), but it uses per-tensor
   * quantization rather than per-channel like other weights. Gemma uses tied
   * weights, which means the final lm_head shares the same weights with the
   * embedding table, but transposed. So it becomes per-channel quantization in
   * the final lm_head. */

  read_qtensor(&dec->embedding, fp, embed_dim, vocab_size, quant);
  dec->layers.resize(tcfg->n_layers);

  // Read all the layers
  for (auto &layer : dec->layers)
  {
    // Attention weights
    read_qtensor(&layer.wq, fp, embed_dim, q_size, quant);
    read_qtensor(&layer.wk, fp, embed_dim, kv_size, quant);
    read_qtensor(&layer.wv, fp, embed_dim, kv_size, quant);
    read_qtensor(&layer.wo, fp, q_size, embed_dim, quant);

    if (tcfg->qk_norm)
    {
      // QK RMSNorms
      if (!read_tensor_ckd(layer.nq_data, layer.nq, tcfg->head_dim, fp) ||
          !read_tensor_ckd(layer.nk_data, layer.nk, tcfg->head_dim, fp))
        return nullptr;
    }
    else
    {
      layer.nq = NULL;
      layer.nk = NULL;
    }

    // Feedforward weights
    read_qtensor(&layer.w1, fp, embed_dim, mlp_dim, quant);
    read_qtensor(&layer.w2, fp, embed_dim, mlp_dim, quant);
    read_qtensor(&layer.w3, fp, mlp_dim, embed_dim, quant);
    if (
      // RMSNorm weights
      !read_tensor_ckd(layer.n1_data, layer.n1, embed_dim, fp) ||
      !read_tensor_ckd(layer.n2_data, layer.n2, embed_dim, fp))
      return nullptr;

    if (tcfg->pre_mlp_norm)
    {
      if (!read_tensor_ckd(layer.n3_data, layer.n3, embed_dim, fp))
        return nullptr;
    }
    else
    {
      layer.n3 = NULL;
    }
    if (tcfg->pst_mlp_norm)
    {
      if (!read_tensor_ckd(layer.n4_data, layer.n4, embed_dim, fp))
        return nullptr;
    }
    else
    {
      layer.n4 = NULL;
    }
  }

  if (!read_tensor_ckd(dec->final_norm_data, dec->final_norm, embed_dim, fp))
    return nullptr;

  return dec.release();

}

/* */
static TextDecoder *
mmap_text_decoder(void *data, TextConfig *tcfg, size_t *offset, bool quant)
{
  auto dec = std::make_unique<TextDecoder>();
  uint8_t *base = (uint8_t *)data;
  int embed_dim = tcfg->embed_dim;
  int mlp_dim = tcfg->mlp_dim;
  int q_size = tcfg->n_heads * tcfg->head_dim;
  int kv_size = tcfg->n_kv_heads * tcfg->head_dim;
  int vocab_size = tcfg->vocab_size;
  dec->config = *tcfg;

  // Embedding: linear (embed_dim -> vocab_size)
  mmap_qtensor(&dec->embedding, base, offset, embed_dim, vocab_size, quant);

  // Layers array
  dec->layers.resize(tcfg->n_layers);

  for (auto &layer : dec->layers)
  {

    // Attention weights
    mmap_qtensor(&layer.wq, base, offset, embed_dim, q_size, quant);
    mmap_qtensor(&layer.wk, base, offset, embed_dim, kv_size, quant);
    mmap_qtensor(&layer.wv, base, offset, embed_dim, kv_size, quant);
    mmap_qtensor(&layer.wo, base, offset, q_size, embed_dim, quant);

    // QK normalization (optional)
    if (tcfg->qk_norm)
    {
      layer.nq = (floatx *)(base + *offset);
      *offset += (size_t)tcfg->head_dim * sizeof(floatx);
      layer.nk = (floatx *)(base + *offset);
      *offset += (size_t)tcfg->head_dim * sizeof(floatx);
    }
    else
    {
      layer.nq = NULL;
      layer.nk = NULL;
    }

    // Feedforward weights
    mmap_qtensor(&layer.w1, base, offset, embed_dim, mlp_dim, quant);
    mmap_qtensor(&layer.w2, base, offset, embed_dim, mlp_dim, quant);
    mmap_qtensor(&layer.w3, base, offset, mlp_dim, embed_dim, quant);

    // RMSNorm weights
    layer.n1 = (floatx *)(base + *offset);
    *offset += (size_t)embed_dim * sizeof(floatx);
    layer.n2 = (floatx *)(base + *offset);
    *offset += (size_t)embed_dim * sizeof(floatx);

    // Optional pre/post MLP norms
    if (tcfg->pre_mlp_norm)
    {
      layer.n3 = (floatx *)(base + *offset);
      *offset += (size_t)embed_dim * sizeof(floatx);
    }
    else
    {
      layer.n3 = NULL;
    }

    if (tcfg->pst_mlp_norm)
    {
      layer.n4 = (floatx *)(base + *offset);
      *offset += (size_t)embed_dim * sizeof(floatx);
    }
    else
    {
      layer.n4 = NULL;
    }

  }

  // Final norm
  dec->final_norm = (floatx *)(base + *offset);
  *offset += (size_t)embed_dim * sizeof(floatx);

  return dec.release();

}

/* Vision encoder container */
struct VisionEncoder
{
  VisionConfig config;

  floatx  *patch_emb;      // (hidden_dim, 3, patch_size, patch_size)
  floatx  *patch_emb_b;    // (hidden_dim,)
  QTensor pos_embedding;  // ((image_size / patch_size)^2, hidden_dim)

  std::vector<VisionEncoderLayer> layers;

  floatx  *post_norm;    // (hidden_dim,)
  floatx  *post_norm_b;  // (hidden_dim,)
  floatx  *norm;         // (hidden_dim,)
  QTensor proj;         // (hidden_dim, embed_dim).T

  // Storage is empty when the corresponding pointer borrows mmap memory.
  std::vector<floatx> patch_emb_data;
  std::vector<floatx> patch_emb_b_data;
  std::vector<floatx> post_norm_data;
  std::vector<floatx> post_norm_b_data;
  std::vector<floatx> norm_data;
};

/* */
size_t
get_vision_encoder_size(
  const VisionConfig *vcfg, const TextConfig *tcfg, bool quant
)
{
  size_t size      = 0;
  int    P         = vcfg->patch_size;
  int    embed_dim = vcfg->hidden_dim;
  int    mlp_dim   = vcfg->mlp_dim;
  int    n_patches = vcfg->image_size / P;
  n_patches *= n_patches;

  // patch_emb
  size += (size_t)embed_dim * 3 * P * P * sizeof(floatx);
  // patch_emb_b
  size += (size_t)embed_dim * sizeof(floatx);

  // pos_embedding: linear (embed_dim -> n_patches)
  size += qtensor_size(embed_dim, n_patches, quant);

  // each layer
  for (int l = 0; l < vcfg->n_layers; l++)
  {
    // n1, n1_b
    size += (size_t)embed_dim * sizeof(floatx);
    size += (size_t)embed_dim * sizeof(floatx);

    // wq, wk, wv, wo: linear (embed_dim -> embed_dim)
    size += qtensor_size(embed_dim, embed_dim, quant);
    size += qtensor_size(embed_dim, embed_dim, quant);
    size += qtensor_size(embed_dim, embed_dim, quant);
    size += qtensor_size(embed_dim, embed_dim, quant);

    // bq, bk, bv, bo
    size += (size_t)embed_dim * sizeof(floatx);
    size += (size_t)embed_dim * sizeof(floatx);
    size += (size_t)embed_dim * sizeof(floatx);
    size += (size_t)embed_dim * sizeof(floatx);

    // n2, n2_b
    size += (size_t)embed_dim * sizeof(floatx);
    size += (size_t)embed_dim * sizeof(floatx);

    // w1: linear (embed_dim -> mlp_dim), w2: linear (mlp_dim -> embed_dim)
    size += qtensor_size(embed_dim, mlp_dim, quant);
    size += qtensor_size(mlp_dim, embed_dim, quant);

    // b1 (mlp_dim), b2 (embed_dim)
    size += (size_t)mlp_dim * sizeof(floatx);
    size += (size_t)embed_dim * sizeof(floatx);
  }

  // post_norm, post_norm_b
  size += (size_t)embed_dim * sizeof(floatx);
  size += (size_t)embed_dim * sizeof(floatx);

  // norm
  size += (size_t)embed_dim * sizeof(floatx);

  // proj: linear (embed_dim -> tcfg->embed_dim)
  size += qtensor_size(embed_dim, tcfg->embed_dim, quant);

  return size;
}

/* */
static VisionEncoder *
read_vision_encoder(FILE *fp, TextConfig *tcfg, VisionConfig *vcfg, bool quant)
{
  auto enc = std::make_unique<VisionEncoder>();
  int P = vcfg->patch_size;
  int embed_dim = vcfg->hidden_dim;
  int mlp_dim = vcfg->mlp_dim;
  int n_patches = vcfg->image_size / P;

  enc->config = *vcfg;

  n_patches *= n_patches;

  if (!read_tensor_ckd(enc->patch_emb_data, enc->patch_emb, (size_t)embed_dim * 3 * P * P, fp) ||
      !read_tensor_ckd(enc->patch_emb_b_data, enc->patch_emb_b, (size_t)embed_dim, fp))
    return nullptr;
  read_qtensor(&enc->pos_embedding, fp, embed_dim, n_patches, quant);
  enc->layers.resize(vcfg->n_layers);

  // Read all the layers of ViT
  for (auto &layer : enc->layers)
  {
    // Attention weights
    read_qtensor(&layer.wq, fp, embed_dim, embed_dim, quant);
    read_qtensor(&layer.wk, fp, embed_dim, embed_dim, quant);
    read_qtensor(&layer.wv, fp, embed_dim, embed_dim, quant);
    read_qtensor(&layer.wo, fp, embed_dim, embed_dim, quant);
    // Feedforward weights
    read_qtensor(&layer.w1, fp, embed_dim, mlp_dim, quant);
    read_qtensor(&layer.w2, fp, mlp_dim, embed_dim, quant);
    if (
      // First LayerNorm
      !read_tensor_ckd(layer.n1_data, layer.n1, (size_t)embed_dim, fp) ||
      !read_tensor_ckd(layer.n1_b_data, layer.n1_b, (size_t)embed_dim, fp) ||
      // Attention biases
      !read_tensor_ckd(layer.bq_data, layer.bq, (size_t)embed_dim, fp) ||
      !read_tensor_ckd(layer.bk_data, layer.bk, (size_t)embed_dim, fp) ||
      !read_tensor_ckd(layer.bv_data, layer.bv, (size_t)embed_dim, fp) ||
      !read_tensor_ckd(layer.bo_data, layer.bo, (size_t)embed_dim, fp) ||
      // Second LayerNorm
      !read_tensor_ckd(layer.n2_data, layer.n2, (size_t)embed_dim, fp) ||
      !read_tensor_ckd(layer.n2_b_data, layer.n2_b, (size_t)embed_dim, fp) ||
      // Feedforward biases
      !read_tensor_ckd(layer.b1_data, layer.b1, (size_t)mlp_dim, fp) ||
      !read_tensor_ckd(layer.b2_data, layer.b2, (size_t)embed_dim, fp))
      return nullptr;
  }

  // Final projection
  read_qtensor(&enc->proj, fp, embed_dim, tcfg->embed_dim, quant);
  if (
    // Post layernorm
    !read_tensor_ckd(enc->post_norm_data, enc->post_norm, (size_t)embed_dim, fp) ||
    !read_tensor_ckd(enc->post_norm_b_data, enc->post_norm_b, (size_t)embed_dim, fp) ||
    // Soft embedding RMSNorm
    !read_tensor_ckd(enc->norm_data, enc->norm, (size_t)embed_dim, fp))
    return nullptr;

  return enc.release();

}

/* */
static VisionEncoder *
mmap_vision_encoder(
  void *data, TextConfig *tcfg, VisionConfig *vcfg, size_t *offset, bool quant
)
{
  auto enc = std::make_unique<VisionEncoder>();
  uint8_t *base = (uint8_t *)data;
  int P = vcfg->patch_size;
  int embed_dim = vcfg->hidden_dim;
  int mlp_dim = vcfg->mlp_dim;
  int n_patches = vcfg->image_size / P;
  enc->config = *vcfg;

  n_patches *= n_patches;

  // patch_emb: embed_dim * 3 * P * P floats
  enc->patch_emb = (floatx *)(base + *offset);
  *offset += (size_t)embed_dim * 3 * P * P * sizeof(floatx);

  // patch_emb_b: embed_dim floats
  enc->patch_emb_b = (floatx *)(base + *offset);
  *offset += (size_t)embed_dim * sizeof(floatx);

  // pos_embedding: linear (embed_dim -> n_patches)
  mmap_qtensor(&enc->pos_embedding, base, offset, embed_dim, n_patches, quant);

  // layers array
  enc->layers.resize(vcfg->n_layers);

  for (auto &layer : enc->layers)
  {

    // n1, n1_b
    layer.n1 = (floatx *)(base + *offset);
    *offset += (size_t)embed_dim * sizeof(floatx);
    layer.n1_b = (floatx *)(base + *offset);
    *offset += (size_t)embed_dim * sizeof(floatx);

    // attention weights: wq, wk, wv, wo (linear embed_dim -> embed_dim)
    mmap_qtensor(&layer.wq, base, offset, embed_dim, embed_dim, quant);
    mmap_qtensor(&layer.wk, base, offset, embed_dim, embed_dim, quant);
    mmap_qtensor(&layer.wv, base, offset, embed_dim, embed_dim, quant);
    mmap_qtensor(&layer.wo, base, offset, embed_dim, embed_dim, quant);

    // attention biases: bq, bk, bv, bo
    layer.bq = (floatx *)(base + *offset);
    *offset += (size_t)embed_dim * sizeof(floatx);
    layer.bk = (floatx *)(base + *offset);
    *offset += (size_t)embed_dim * sizeof(floatx);
    layer.bv = (floatx *)(base + *offset);
    *offset += (size_t)embed_dim * sizeof(floatx);
    layer.bo = (floatx *)(base + *offset);
    *offset += (size_t)embed_dim * sizeof(floatx);

    // n2, n2_b
    layer.n2 = (floatx *)(base + *offset);
    *offset += (size_t)embed_dim * sizeof(floatx);
    layer.n2_b = (floatx *)(base + *offset);
    *offset += (size_t)embed_dim * sizeof(floatx);

    // feedforward weights: w1 (embed_dim -> mlp_dim), w2 (mlp_dim -> embed_dim)
    mmap_qtensor(&layer.w1, base, offset, embed_dim, mlp_dim, quant);
    mmap_qtensor(&layer.w2, base, offset, mlp_dim, embed_dim, quant);

    // feedforward biases: b1 (mlp_dim), b2 (embed_dim)
    layer.b1 = (floatx *)(base + *offset);
    *offset += (size_t)mlp_dim * sizeof(floatx);
    layer.b2 = (floatx *)(base + *offset);
    *offset += (size_t)embed_dim * sizeof(floatx);

  }

  // post_norm, post_norm_b
  enc->post_norm = (floatx *)(base + *offset);
  *offset += (size_t)embed_dim * sizeof(floatx);
  enc->post_norm_b = (floatx *)(base + *offset);
  *offset += (size_t)embed_dim * sizeof(floatx);

  // norm
  enc->norm = (floatx *)(base + *offset);
  *offset += (size_t)embed_dim * sizeof(floatx);

  // proj: linear (embed_dim -> embed_dim)
  mmap_qtensor(&enc->proj, base, offset, embed_dim, tcfg->embed_dim, quant);

  return enc.release();

}

/* Top-level model container */
struct GemmaModel
{
  // Keep the mapped weights alive until all borrowers have been destroyed.
  std::optional<io::mapped_region> mapped_weights;
  std::unique_ptr<TextDecoder> decoder;
  std::unique_ptr<VisionEncoder> encoder;
  std::unique_ptr<GemmaTokenizer> tokenizer;
  bool            quant;  // W8A8
  bool            support_mm;
};

/* Check the file header and return a flag that indicates whether the model
 * is quantized. Return -1 if failed. */
static int
check_head(FILE *fp, const char *filename)
{
  // Magic header
  char hdr[8];
  if (!fread_ckd(hdr, 8, 1, fp)) return -1;
  if (memcmp(hdr, "GEMA", 4) != 0)
  {
    log_error("not a valid gemma model file: %s", filename);
    return -1;
  }
  // dtype
  if (memcmp(hdr + 4, DTYPE_MAGIC, 3) != 0)
  {
    std::print(
      stderr, "unsupported dtype '{}{}{}' in model file", hdr[4], hdr[5], hdr[6]
    );
    return -1;
  }
  // W8A8 flag ('Q' / 'U')
  if (hdr[7] == 'Q')
  {
    return 1;
  }
  else if (hdr[7] != 'U')
  {
    log_error("not a valid gemma model file: %s", filename);
    return -1;
  }

  return 0;
}

using FilePtr = std::unique_ptr<FILE, decltype(&fclose)>;

/* Map the entire text & vision model into virtual memory */
GemmaModel *
mmap_gemma_model(const char *filename, bool enable_mm)
{
  FilePtr file(fopen(filename, "rb"), fclose);
  FILE *fp = file.get();
  auto model = std::make_unique<GemmaModel>();
  std::unique_ptr<TextConfig> tcfg;
  std::unique_ptr<VisionConfig> vcfg;
  int quant = -1;
  bool use_mm = false;
  TextDecoder *dec = nullptr;
  long offset_l = 0;
  size_t offset = 0;
  size_t mapping_size = 0;
  void *data = nullptr;
  if (fp == NULL)
  {
    log_error("failed to open file: %s", filename);
    return nullptr;
  }

  // Header
  quant = check_head(fp, filename);
  if (quant == -1) return nullptr;
  model->quant = (bool)quant;

  // Text config
  tcfg.reset(read_text_config(fp, &model->support_mm));
  if (tcfg == NULL) return nullptr;

  use_mm = model->support_mm && enable_mm;

  // Vision config
  vcfg = NULL;
  if (use_mm)
  {
    vcfg.reset(read_vision_config(fp));
    if (vcfg == NULL) return nullptr;
  }
  else if (model->support_mm)
  {
    // Skip vision config if user didn't ask for multimodal
    std::unique_ptr<VisionConfig> tmp(read_vision_config(fp));
    if (tmp == NULL) return nullptr;
  }

  // Tokenizer
  model->tokenizer.reset(read_tokenizer(fp, tcfg.get(), model->support_mm));
  if (!model->tokenizer) return nullptr;

  offset_l = ftell(fp);
  if (offset_l < 0)
  {
    log_error("ftell failed");
    return nullptr;
  }
  offset = (size_t)offset_l;

  auto mapped_file = io::mapped_file::open(filename, io::file_access::read);
  if (!mapped_file)
  {
    log_error("failed to open file: %s: %s", filename,
              mapped_file.error().message().c_str());
    return nullptr;
  }

  // Get the total amount of bytes for the weights
  mapping_size = offset + get_text_decoder_size(tcfg.get(), model->quant);
  if (use_mm)
  {
    mapping_size += get_vision_encoder_size(vcfg.get(), tcfg.get(), model->quant);
  }

  auto region = mapped_file->map({.size = mapping_size, .access = io::map_access::read});
  if (!region)
  {
    log_error("memory mapping failed: %s: %s", filename,
              region.error().message().c_str());
    return nullptr;
  }
  model->mapped_weights.emplace(std::move(*region));
  data = model->mapped_weights->data();

  // Text decoder
  dec = mmap_text_decoder(data, tcfg.get(), &offset, model->quant);
  if (dec == NULL) return nullptr;
  model->decoder.reset(dec);
  tcfg.release();

  // Vision encoder
  if (use_mm)
  {
    VisionEncoder *enc =
      mmap_vision_encoder(data, &dec->config, vcfg.get(), &offset, model->quant);
    if (enc == NULL) return nullptr;
    model->encoder.reset(enc);
    vcfg.release();
  }

  return model.release();

}

/* Load the entire text & vision model into RAM */
GemmaModel *
read_gemma_model(const char *filename, bool enable_mm)
{
  FilePtr file(fopen(filename, "rb"), fclose);
  FILE *fp = file.get();
  auto model = std::make_unique<GemmaModel>();
  std::unique_ptr<TextConfig> tcfg;
  std::unique_ptr<VisionConfig> vcfg;
  int quant = -1;
  bool use_mm = false;
  TextDecoder *dec = nullptr;
  if (fp == NULL)
  {
    log_error("failed to open file: %s", filename);
    return nullptr;
  }

  // Header
  quant = check_head(fp, filename);
  if (quant == -1) return nullptr;
  model->quant = (bool)quant;

  // Text config
  tcfg.reset(read_text_config(fp, &model->support_mm));
  if (tcfg == NULL) return nullptr;

  use_mm = model->support_mm && enable_mm;

  // Vision config
  vcfg = NULL;
  if (use_mm)
  {
    vcfg.reset(read_vision_config(fp));
    if (vcfg == NULL) return nullptr;
  }
  else if (model->support_mm)
  {
    // Skip vision config if user didn't ask for multimodal
    std::unique_ptr<VisionConfig> tmp(read_vision_config(fp));
    if (tmp == NULL) return nullptr;
  }

  // Tokenizer
  model->tokenizer.reset(read_tokenizer(fp, tcfg.get(), model->support_mm));
  if (!model->tokenizer) return nullptr;

  // Text decoder
  dec = read_text_decoder(fp, tcfg.get(), model->quant);
  if (dec == NULL) return nullptr;
  model->decoder.reset(dec);
  tcfg.release();

  // Vision encoder
  if (use_mm)
  {
    VisionEncoder *enc =
      read_vision_encoder(fp, &dec->config, vcfg.get(), model->quant);
    if (enc == NULL) return nullptr;
    model->encoder.reset(enc);
    vcfg.release();
  }

  return model.release();

}

// Runtime buffers (allocated once, reused every step)

struct VisionBuffer
{
  std::vector<int8_t> x_i8;        // (n_patches, embed_dim)
  std::vector<floatx> x_scales;    // (n_patches,)
  std::vector<int8_t> mlp_i8;      // (n_patches, mlp_dim)
  std::vector<floatx> mlp_scales;  // (n_patches)

  std::vector<floatx> x;           // (n_patches, embed_dim)
  std::vector<floatx> resid;       // (n_patches, embed_dim)
  std::vector<floatx> xq;          // (n_patches, embed_dim)
  std::vector<floatx> xk;          // (n_patches, embed_dim)
  std::vector<floatx> xv;          // (n_patches, embed_dim)
  std::vector<floatx> att_out;     // (n_patches, embed_dim)
  std::vector<floatx> mlp_hidden;  // (n_patches, mlp_dim)
  std::vector<floatx> scores;      // (n_heads, n_patches, n_patches)

};

size_t
get_vision_buffer_size(VisionConfig *vcfg, bool quant)
{
  size_t size = 0;

  int ppi       = vcfg->image_size / vcfg->patch_size;
  int n_patches = ppi * ppi;

  // Quantized buffers
  if (quant)
  {
    size += n_patches * (
      sizeof(int8_t) * (vcfg->hidden_dim + vcfg->mlp_dim) +  // x_i8 & mlp_i8
      sizeof(floatx) * 2  // x_scales & mlp_scales
    );
  }

  // Main buffers
  size += n_patches * sizeof(floatx) * (
    vcfg->hidden_dim * 6 +     // x & resid & xq & xk & xv & att_out
    vcfg->mlp_dim +            // mlp_hidden
    n_patches * vcfg->n_heads  // scores
  );

  return size;
}

/* */
VisionBuffer *
malloc_vision_buffer(VisionConfig *vcfg, bool quant)
{
  auto vbuf = std::make_unique<VisionBuffer>();
  size_t embed_dim = (size_t)vcfg->hidden_dim;
  size_t ppi = (size_t)vcfg->image_size / vcfg->patch_size;
  size_t n_patches = ppi * ppi;
  size_t mlp_dim = (size_t)vcfg->mlp_dim;
  size_t n_heads = (size_t)vcfg->n_heads;

  if (quant)
  {
    // Quantization buffers
    vbuf->x_i8.resize(n_patches * embed_dim);
    vbuf->mlp_i8.resize(n_patches * mlp_dim);
    vbuf->x_scales.resize(n_patches);
    vbuf->mlp_scales.resize(n_patches);
  }

  vbuf->x.resize(n_patches * embed_dim);
  vbuf->resid.resize(n_patches * embed_dim);
  vbuf->xq.resize(n_patches * embed_dim);
  vbuf->xk.resize(n_patches * embed_dim);
  vbuf->xv.resize(n_patches * embed_dim);
  vbuf->att_out.resize(n_patches * embed_dim);
  vbuf->mlp_hidden.resize(n_patches * mlp_dim);
  vbuf->scores.resize(n_heads * n_patches * n_patches);
  return vbuf.release();
}

/* Gemma language model runtime buffer */
struct TextBuffer
{
  int cache_len;

  // Temporary quantized activations (when quant=true)
  std::vector<int8_t> x_i8;       // ([n_tokens], embed_dim,)
  std::vector<floatx> x_scales;   // ([n_tokens],)
  std::vector<int8_t> xo_i8;      // (n_heads, [n_tokens], head_dim)
  std::vector<floatx> xo_scales;  // ([n_tokens],)
  std::vector<int8_t> xg_i8;      // ([n_tokens], mlp_dim,)
  std::vector<floatx> xg_scales;  // ([n_tokens],)

  // Pre-computed cos/sin for RoPE
  std::vector<floatx> csfreqs_slid;  // ([n_tokens], head_dim / 2, 2)
  std::vector<floatx> csfreqs_full;  // ([n_tokens], head_dim / 2, 2)

  // Residual stream
  std::vector<floatx> x;      // ([n_tokens], embed_dim,)
  std::vector<floatx> resid;  // ([n_tokens], embed_dim,)

  // Attention buffers
  std::vector<floatx> xq;        // (n_heads, [n_tokens], head_dim)
  std::vector<floatx> xk;        // (n_kv_heads, [n_tokens], head_dim)
  std::vector<floatx> xv;        // (n_kv_heads, head_dim, [n_tokens])
  std::vector<floatx> xo;        // ([n_tokens], n_heads, head_dim)
  std::vector<floatx> att;       // (n_heads, [n_tokens], cache_len)
  std::vector<floatx> kv_cache;  // (n_layers, 2, n_kv_heads, cache_len, head_dim)

  // MLP buffers
  std::vector<floatx> xg;      // ([n_tokens], mlp_dim,)
  std::vector<floatx> xu;      // ([n_tokens], mlp_dim,)
  std::vector<floatx> logits;  // (vocab_size,)

};

/* */
size_t
get_text_buffer_size(
  TextConfig   *tcfg,
  VisionConfig *vcfg,
  int           cache_len,
  int           chunk_size,
  bool          use_mm,
  bool          quant
)
{
  size_t size    = 0;
  int    q_size  = tcfg->n_heads * tcfg->head_dim;
  int    kv_size = tcfg->n_kv_heads * tcfg->head_dim;
  int    ppi     = vcfg->image_size / vcfg->patch_size;
  int    mult    = use_mm ? std::max(ppi * ppi, chunk_size) : chunk_size;

  // Quantized buffers
  if (quant)
  {
    // x_i8 & xo_i8 & xg_i8
    size += sizeof(int8_t) * (tcfg->embed_dim + q_size + tcfg->mlp_dim);
  }

  // Main buffers
  size += sizeof(floatx) * (
    tcfg->n_layers * 2 * cache_len * kv_size +  // kv_cache
    tcfg->vocab_size +                          // logits
    mult * tcfg->embed_dim +                    // x
    mult * tcfg->embed_dim +                    // resid
    mult * q_size +                             // xq
    mult * kv_size +                            // xk
    mult * tcfg->head_dim +                     // csfreqs_slid
    mult * tcfg->head_dim +                     // csfreqs_full
    mult * kv_size +                            // xv
    mult * q_size +                             // xo
    mult * tcfg->n_heads * cache_len +          // att
    mult * tcfg->mlp_dim +                      // xg
    mult * tcfg->mlp_dim                        // xu
  );

  return size;
}

/* */
TextBuffer *
malloc_text_buffer(
  TextConfig   *tcfg,
  VisionConfig *vcfg,
  int           cache_len,
  int           chunk_size,
  bool          use_mm,
  bool          quant
)
{
  auto tbuf = std::make_unique<TextBuffer>();
  int embed_dim = tcfg->embed_dim;
  int n_layers = tcfg->n_layers;
  int head_dim = tcfg->head_dim;
  int n_heads = tcfg->n_heads;
  int mlp_dim = tcfg->mlp_dim;
  int q_size = n_heads * head_dim;
  int kv_size = tcfg->n_kv_heads * head_dim;
  int mult = chunk_size;
  tbuf->cache_len = cache_len;
  tbuf->kv_cache.resize((size_t)n_layers * 2 * (size_t)cache_len * (size_t)kv_size);
  tbuf->logits.resize(tcfg->vocab_size);
  // Multimodal models need space for a whole image worth of tokens
  if (use_mm && vcfg != NULL)
  {
    mult = std::max<int>(mult, tcfg->image_toks);
  }

  if (quant)
  {
    // Quantization buffers
    tbuf->x_i8.resize(mult * embed_dim);
    tbuf->xo_i8.resize(mult * q_size);
    tbuf->xg_i8.resize(mult * mlp_dim);
    tbuf->x_scales.resize(mult);
    tbuf->xo_scales.resize(mult);
    tbuf->xg_scales.resize(mult);
  }

  tbuf->x.resize(mult * embed_dim);
  tbuf->resid.resize(mult * embed_dim);
  tbuf->xq.resize(mult * q_size);
  tbuf->xk.resize(mult * kv_size);
  tbuf->csfreqs_slid.resize(mult * head_dim);
  tbuf->csfreqs_full.resize(mult * head_dim);
  tbuf->xv.resize(mult * kv_size);
  tbuf->xo.resize(mult * q_size);
  tbuf->att.resize(mult * n_heads * cache_len);
  tbuf->xg.resize(mult * mlp_dim);
  tbuf->xu.resize(mult * mlp_dim);
  return tbuf.release();
}

// Math primitives

NO_FAST_MATH_BEGIN;

/* */
static inline floatx
clamp_fpx(floatx v)
{
  return (floatx)fminf(FLOATX_MAX, fmaxf((float)-FLOATX_MAX, (float)v));
}

NO_FAST_MATH_END;

/* Gemma-style RMSNorm applied independently to each row. */
static void rmsnorm(std::mdspan<floatx, std::dims<2>> output, std::mdspan<const floatx, std::dims<2>> input,
             const floatx* weights, float epsilon) {
    for (size_t row = 0; row < input.extent(0); row++) {
        float sum_squares = 0.0f;
        for (size_t i = 0; i < input.extent(1); i++)
            sum_squares += (float)input[row, i] * (float)input[row, i];
        float inverse_rms = 1.0f / sqrtf(sum_squares / (float)input.extent(1) + epsilon);
        for (size_t i = 0; i < input.extent(1); i++)
            output[row, i] = (weights ? (weights[i] + 1) : 1.0f) * (inverse_rms * input[row, i]);
    }
}

/* Classic LayerNorm used inside the SigLIP vision tower */
static void
layernorm(
  floatx       *dst,
  const floatx *src,
  const floatx *weight,
  const floatx *bias,
  int           dim,
  float         eps
)
{
  float mean = 0.0f;
  for (int i = 0; i < dim; i++)
  {
    mean += (float)src[i];
  }
  mean /= (float)dim;

  float var = 0.0f;

  for (int i = 0; i < dim; i++)
  {
    float diff = (float)src[i] - mean;
    var += diff * diff;
  }
  var /= (float)dim;

  float inv_std = 1.0f / sqrtf(var + eps);

  for (int i = 0; i < dim; i++)
  {
    dst[i] = (floatx)(((float)src[i] - mean) * inv_std * (float)weight[i] +
                      (float)bias[i]);
  }
}

/**
 * Symmetric per-vector quantization into int8 [-127, 127]
 * Q(fpx src (dim,)) ~= int8 dst (dim,) * fpx vec_scale (1,)
 */
static floatx
quantize_act(int8_t *dst, const floatx *vec, int dim)
{
  floatx amax = 0.0f;

  for (int d = 0; d < dim; d++)
  {
    floatx av = vec[d] >= 0 ? vec[d] : -vec[d];
    if (av > amax)
    {
      amax = av;
    }
  }

  float vec_scale = (float)amax > 0.0f ? (float)amax / 127.0f : 1.0f;

  for (int d = 0; d < dim; d++)
  {
    int q = (int)roundf((float)vec[d] / vec_scale);
    if (q > 127)
    {
      q = 127;
    }
    else if (q < -127)
    {
      q = -127;
    }
    dst[d] = (int8_t)q;
  }

  return (floatx)vec_scale;
}

/**
 * Symmetric int8 quantization for a matrix of rows
 * Q(fpx src (m, n)) ~= int8 dst (m, n) * fpx dst_scales (m,)
 */
static void
quantize_acts(
  int8_t *RESTRICT       dst,
  floatx *RESTRICT       dst_scales,
  const floatx *RESTRICT src,
  int                    src_stride,
  int                    m,
  int                    n,
  bool                   omp
)
{
  if (src_stride == 0)
  {
    src_stride = n;
  }

  if (omp)
  {
    int i;
    #pragma omp parallel for OMP_PARA_ARGS
    for (i = 0; i < m; i++)
    {
      dst_scales[i] = quantize_act(dst + i * n, src + i * src_stride, n);
    }
    return;
  }
  for (int i = 0; i < m; i++)
  {
    dst_scales[i] = quantize_act(dst + i * n, src + i * src_stride, n);
  }
}

/* */
static inline floatx
gemv_fpx_row(
  const floatx *RESTRICT vec, const floatx *RESTRICT mat, int n, int i
)
{
  float sum = 0;
  for (int j = 0; j < n; j++)
  {
    sum += (float)mat[i * n + j] * (float)vec[j];
  }
  return (floatx)sum;
}

/**
 * fpx matrix-vector multiply (NT)
 * fpx vec (n,) @ fpx mat (m, n).T = fpx dst (m,)
 */
static void
gemv_fpx(
  floatx *RESTRICT       dst,
  const floatx *RESTRICT mat,
  const floatx *RESTRICT vec,
  int                    m,
  int                    n,
  bool                   omp
)
{
  if (omp)
  {
    int i;  // Loop vars are not automatically privatized in MSVC, have to keep
            // it explicit
    #pragma omp parallel for private(i) OMP_PARA_ARGS
    for (i = 0; i < m; i++)
    {
      dst[i] = gemv_fpx_row(vec, mat, n, i);
    }
    return;
  }
  for (int i = 0; i < m; i++)
  {
    dst[i] = gemv_fpx_row(vec, mat, n, i);
  }
}

/* */
static inline floatx
gemv_int8_row(
  const int8_t *RESTRICT vec,
  floatx                 vec_scale,
  const int8_t *RESTRICT mat,
  const floatx *RESTRICT mat_scales,
  int                    n,
  int                    i
)
{
  int32_t sum = 0;

  for (int j = 0; j < n; j++)
  {
    sum += (int32_t)mat[i * n + j] * (int32_t)vec[j];
  }
  return (floatx)((float)sum * (float)vec_scale * (float)mat_scales[i]);
}

/**
 * int8 matrix-vector multiply + dequant (NT)
 *   (int8 vec (n,)   * fpx vec_scale  (1,))
 * @ (int8 mat (m, n) * fpx mat_scales (m,)).T = fpx dst (m,)
 */
static void
gemv_int8(
  floatx *RESTRICT       dst,
  const int8_t *RESTRICT mat,
  const floatx *RESTRICT mat_scales,
  const int8_t *RESTRICT vec,
  floatx                 vec_scale,
  int                    m,
  int                    n,
  bool                   omp
)
{
  if (omp)
  {
    int i;
    #pragma omp parallel for private(i) OMP_PARA_ARGS
    for (i = 0; i < m; i++)
    {
      dst[i] = gemv_int8_row(vec, vec_scale, mat, mat_scales, n, i);
    }
    return;
  }

  for (int i = 0; i < m; i++)
  {
    dst[i] = gemv_int8_row(vec, vec_scale, mat, mat_scales, n, i);
  }
}

/* fpx matrix-vector multiply (NN)
 * fpx vec (n,) @ fpx mat (n, m) = fpx dst (m,) */
static void
gemv_fpx_nn(
  floatx *RESTRICT       dst,
  const floatx *RESTRICT vec,
  const floatx *RESTRICT mat,
  int                    mat_stride,
  int                    n,
  int                    m
)
{
  if (mat_stride == 0) mat_stride = m;

#if defined(__MSVC__)
  float *acc = (float *)_alloca((size_t)m * sizeof(float));
#else
  float acc[m];
#endif

  for (int j = 0; j < m; j++)
  {
    acc[j] = 0.0f;
  }

  for (int i = 0; i < n; i++)
  {
    float         v       = (float)vec[i];
    const floatx *mat_row = mat + i * mat_stride;

    for (int j = 0; j < m; j++)
    {
      acc[j] += v * (float)mat_row[j];
    }
  }

  for (int j = 0; j < m; j++)
  {
    dst[j] = (floatx)acc[j];
  }
}

// A decent GEMM kernel implementation, I pretty much vibe-coded this part :D

/**
 * Copy `rows` rows (stride `row_stride`, length k each) of `src` into a
 * contiguous, already-upcast-to-float buffer laid out as `packed[l*rows + r]`.
 */
static inline void
pack_panel_fpx(
  float *RESTRICT        packed,
  const floatx *RESTRICT src,
  int                    row_stride,
  int                    rows,
  int                    k
)
{
  for (int r = 0; r < rows; r++)
  {
    const floatx *row = src + r * row_stride;
    for (int l = 0; l < k; l++)
    {
      packed[l * rows + r] = (float)row[l];
    }
  }
}

/**
 * Pack every R-row panel of a (total_rows x k) matrix, back to back,
 * into one contiguous buffer.
 */
static void
pack_all_fpx(
  float *RESTRICT        packed_full,
  const floatx *RESTRICT mat,
  int                    stride,
  int                    total_rows,
  int                    R,
  int                    k,
  bool                   omp
)
{
  if (omp)
  {
    int b;
    #pragma omp parallel for private(b) OMP_PARA_ARGS
    for (b = 0; b < total_rows; b += R)
    {
      pack_panel_fpx(
        packed_full + (size_t)b * k, mat + (size_t)b * stride, stride, R, k
      );
    }
  }
  else
  {
    for (int b = 0; b < total_rows; b += R)
    {
      pack_panel_fpx(
        packed_full + (size_t)b * k, mat + (size_t)b * stride, stride, R, k
      );
    }
  }
}

/* */
static inline void
gemm_fpx_scalar_row(
  floatx *RESTRICT       dst,
  int                    dst_stride,
  const floatx *RESTRICT mat,
  int                    mat_stride,
  const floatx *RESTRICT src,
  int                    src_stride,
  int                    i,
  int                    j,
  int                    k
)
{
  float         sum     = 0.0f;
  const floatx *src_row = src + i * src_stride;
  const floatx *w_row   = mat + j * mat_stride;

  for (int l = 0; l < k; l++)
  {
    sum += (float)src_row[l] * (float)w_row[l];
  }
  dst[i * dst_stride + j] = (floatx)sum;
}

/* Scalar fallback for the remainder tiles. */
static inline void
gemm_fpx_scalar(
  floatx *RESTRICT       dst,
  int                    dst_stride,
  const floatx *RESTRICT mat,
  int                    mat_stride,
  const floatx *RESTRICT src,
  int                    src_stride,
  int                    m,
  int                    n,
  int                    k,
  bool                   omp
)
{
  if (omp)
  {
    int i, j;
    #pragma omp parallel for private(i, j) OMP_COLLAPSE2 OMP_PARA_ARGS
    for (i = 0; i < m; i++)
      for (j = 0; j < n; j++)
      {
        gemm_fpx_scalar_row(
          dst, dst_stride, mat, mat_stride, src, src_stride, i, j, k
        );
      }
  }
  else
  {
    for (int i = 0; i < m; i++)
      for (int j = 0; j < n; j++)
      {
        gemm_fpx_scalar_row(
          dst, dst_stride, mat, mat_stride, src, src_stride, i, j, k
        );
      }
  }
}

// thread_local is critical here since these will be used in openmp threads
thread_local static std::vector<float> gemm_fpx_pack_scratch;

/* */
static float *
gemm_fpx_pack_scratch_get(size_t needed_floats)
{
  gemm_fpx_pack_scratch.resize(needed_floats);
  return gemm_fpx_pack_scratch.data();
}

/* Performs acc_ext += Ap * Bp over a k-slice of length kc. */
static inline void
gemm_fpx_kernel_accum(
  float *RESTRICT       acc_ext,
  int                   acc_stride,
  const float *RESTRICT Bp,
  const float *RESTRICT Ap,
  int                   kc
)
{
  float acc[64] = {0};

  for (int l = 0; l < kc; l++)
  {
    /*
     * Compute the outer product of Ap[l] and Bp[l], then accumulate into acc.
     *
     *    acc += a @ b.T (8x8):
     *
     *         b0   b1   b2   b3   b4   b5   b6   b7
     *      +-----------------------------------------+
     *  a0  | a0b0 a0b1 a0b2 a0b3 a0b4 a0b5 a0b6 a0b7 |
     *  a1  | a1b0 a1b1 a1b2 a1b3 a1b4 a1b5 a1b6 a1b7 |
     *  a2  | a2b0 a2b1 a2b2 a2b3 a2b4 a2b5 a2b6 a2b7 |
     *  a3  | a3b0 a3b1 a3b2 a3b3 a3b4 a3b5 a3b6 a3b7 |
     *  a4  | a4b0 a4b1 a4b2 a4b3 a4b4 a4b5 a4b6 a4b7 |
     *  a5  | a5b0 a5b1 a5b2 a5b3 a5b4 a5b5 a5b6 a5b7 |
     *  a6  | a6b0 a6b1 a6b2 a6b3 a6b4 a6b5 a6b6 a6b7 |
     *  a7  | a7b0 a7b1 a7b2 a7b3 a7b4 a7b5 a7b6 a7b7 |
     *      +-----------------------------------------+
     */

    // Such a shame that msvc generated absolutely no vectorization for this
    // kernel, even though I unrolled the entire loop like this

    // clang-format off
    float a0 = Ap[l*8+0], a1 = Ap[l*8+1], a2 = Ap[l*8+2], a3 = Ap[l*8+3];
    float a4 = Ap[l*8+4], a5 = Ap[l*8+5], a6 = Ap[l*8+6], a7 = Ap[l*8+7];
    float b0 = Bp[l*8+0], b1 = Bp[l*8+1], b2 = Bp[l*8+2], b3 = Bp[l*8+3];
    float b4 = Bp[l*8+4], b5 = Bp[l*8+5], b6 = Bp[l*8+6], b7 = Bp[l*8+7];

    acc[ 0] += a0*b0; acc[ 1] += a0*b1; acc[ 2] += a0*b2; acc[ 3] += a0*b3;
    acc[ 4] += a0*b4; acc[ 5] += a0*b5; acc[ 6] += a0*b6; acc[ 7] += a0*b7;
    acc[ 8] += a1*b0; acc[ 9] += a1*b1; acc[10] += a1*b2; acc[11] += a1*b3;
    acc[12] += a1*b4; acc[13] += a1*b5; acc[14] += a1*b6; acc[15] += a1*b7;
    acc[16] += a2*b0; acc[17] += a2*b1; acc[18] += a2*b2; acc[19] += a2*b3;
    acc[20] += a2*b4; acc[21] += a2*b5; acc[22] += a2*b6; acc[23] += a2*b7;
    acc[24] += a3*b0; acc[25] += a3*b1; acc[26] += a3*b2; acc[27] += a3*b3;
    acc[28] += a3*b4; acc[29] += a3*b5; acc[30] += a3*b6; acc[31] += a3*b7;
    acc[32] += a4*b0; acc[33] += a4*b1; acc[34] += a4*b2; acc[35] += a4*b3;
    acc[36] += a4*b4; acc[37] += a4*b5; acc[38] += a4*b6; acc[39] += a4*b7;
    acc[40] += a5*b0; acc[41] += a5*b1; acc[42] += a5*b2; acc[43] += a5*b3;
    acc[44] += a5*b4; acc[45] += a5*b5; acc[46] += a5*b6; acc[47] += a5*b7;
    acc[48] += a6*b0; acc[49] += a6*b1; acc[50] += a6*b2; acc[51] += a6*b3;
    acc[52] += a6*b4; acc[53] += a6*b5; acc[54] += a6*b6; acc[55] += a6*b7;
    acc[56] += a7*b0; acc[57] += a7*b1; acc[58] += a7*b2; acc[59] += a7*b3;
    acc[60] += a7*b4; acc[61] += a7*b5; acc[62] += a7*b6; acc[63] += a7*b7;
    // clang-format on
  }

  for (int i = 0; i < 8; i++)
    for (int j = 0; j < 8; j++)
      acc_ext[i * acc_stride + j] += acc[i * 8 + j];
}

/* Converts an (mc x nc) fp32 accumulator block to floatx and writes it
 * into dst once, after the full k range has been accumulated. */
static inline void
gemm_fpx_flush_block(
  floatx *RESTRICT      dst,
  int                   dst_stride,
  const float *RESTRICT acc,
  int                   acc_stride,
  int                   mc,
  int                   nc
)
{
  for (int r = 0; r < mc; r++)
    for (int c = 0; c < nc; c++)
      dst[r * dst_stride + c] = (floatx)acc[r * acc_stride + c];
}

/* */
static inline void
gemm_fpx_compute_row(
  floatx *RESTRICT      dst,  // dst + jc, i.e. the start of this column block
  int                   dst_stride,
  const float *RESTRICT Ap_full,   // packed A, (m_rows, k)
  const float *RESTRICT Bp_block,  // packed B, (nc, k)
  int                   m_rows,
  int                   ic,
  int                   nc,
  int                   k
)
{
  int mc = m_rows - ic;
  if (mc > GEMM_MC) mc = GEMM_MC;

  // Thread-local fp32 accumulator, reused across (ic,jc) iterations within
  // this thread so we're not malloc'ing per tile.
  static thread_local float *acc_buf     = NULL;
  static thread_local size_t acc_buf_cap = 0;
  size_t                      needed       = (size_t)GEMM_MC * GEMM_NC;

  if (needed > acc_buf_cap)
  {
    free(acc_buf);
    acc_buf     = (float *)malloc(needed * sizeof(float));
    acc_buf_cap = (acc_buf != NULL) ? needed : 0;
  }
  if (acc_buf == NULL)
  {
    log_error("gemm_fpx accumulator scratch allocation failed");
    return;  // best-effort: skip this tile rather than crash
  }

  // acc_buf's row stride is the fixed GEMM_NC (allocation size), NOT nc,
  // so it must be zeroed row-by-row rather than as one flat mc*nc memset
  for (int r = 0; r < mc; r++)
    memset(acc_buf + (size_t)r * GEMM_NC, 0, (size_t)nc * sizeof(float));

  for (int pc = 0; pc < k; pc += GEMM_KC)
  {
    int kc = k - pc;
    if (kc > GEMM_KC) kc = GEMM_KC;

    for (int jb = 0; jb < nc; jb += 8)
      for (int ib = 0; ib < mc; ib += 8)
      {
        const float *Ap_panel = Ap_full + (size_t)(ic + ib) * k + pc * 8;
        const float *Bp_panel = Bp_block + (size_t)jb * k + pc * 8;

        gemm_fpx_kernel_accum(
          acc_buf + ib * GEMM_NC + jb, GEMM_NC, Bp_panel, Ap_panel, kc
        );
      }
  }

  gemm_fpx_flush_block(
    dst + (size_t)ic * dst_stride, dst_stride, acc_buf, GEMM_NC, mc, nc
  );
}

/*
 * Cache-blocked compute core shared by the "pack everything up front" path
 * below: given an already-packed A panel covering `m_rows` rows (a
 * multiple of 8) x k, and an already-packed B panel covering this jc
 * column-block's `nc` columns (a multiple of 8) x k, compute the
 * corresponding (m_rows x nc) region of `dst`.
 *
 * Loop nest (classic BLIS-style blocking, minus the outermost jc level
 * which the caller already handles):
 *
 *   for ic in 0..m_rows step MC:  // keeps a small A slab hot
 *     zero a (mc x nc) fp32 accumulator
 *     for pc in 0..k step KC:     // keeps the working A/B slices
 *                                 // small enough for L1/L2
 *       for jb, ib: 8x8 micro-kernel accumulate
 *     flush accumulator -> dst (floatx), once
 *
 * Ap_full / Bp_block use the same panel layout pack_all_fpx already
 * produces (each 8-row group packed as packed[l*8+r], groups laid out
 * back-to-back), so slicing a `kc`-long sub-range of `l` for a given
 * 8-row group is just a pointer offset (+pc*8) into that group -- no
 * re-packing needed to stream through k in chunks.
 */
static void
gemm_fpx_compute_block(
  floatx *RESTRICT      dst,  // dst + jc, i.e. the start of this column block
  int                   dst_stride,
  const float *RESTRICT Ap_full,   // packed A, (m_rows, k)
  const float *RESTRICT Bp_block,  // packed B, (nc, k)
  int                   m_rows,
  int                   nc,
  int                   k,
  bool                  omp
)
{
  int ic;

  if (omp)
  {
    #pragma omp parallel for private(ic) OMP_PARA_ARGS if (omp)
    for (ic = 0; ic < m_rows; ic += GEMM_MC)
    {
      gemm_fpx_compute_row(
        dst, dst_stride, Ap_full, Bp_block, m_rows, ic, nc, k
      );
    }
  }
  else
  {
    for (ic = 0; ic < m_rows; ic += GEMM_MC)
    {
      gemm_fpx_compute_row(
        dst, dst_stride, Ap_full, Bp_block, m_rows, ic, nc, k
      );
    }
  }
}

/**
 * fpx matrix-matrix multiply (NT)
 * fpx src (m, k) @ fpx mat.T (k, n) = fpx dst (m, n)
 */
static int
gemm_fpx(
  floatx *RESTRICT       dst,
  int                    dst_stride,
  const floatx *RESTRICT mat,
  int                    mat_stride,
  const floatx *RESTRICT src,
  int                    src_stride,
  int                    m,
  int                    n,
  int                    k,
  bool                   omp
)
{
  if (dst_stride == 0) dst_stride = n;
  if (mat_stride == 0) mat_stride = k;
  if (src_stride == 0) src_stride = k;

  int m_full = (m / 8) * 8;
  int n_full = (n / 8) * 8;

  if (m_full > 0 && n_full > 0)
  {
    // Scratch holds the full A panel (m_full x k) plus one NC-wide B
    // column block (at most GEMM_NC x k) at a time, instead of the
    // whole n_full x k B matrix up front.
    int    nc_cap = n_full < GEMM_NC ? n_full : GEMM_NC;
    size_t needed  = (size_t)m_full * k + (size_t)nc_cap * k;
    float *scratch = gemm_fpx_pack_scratch_get(needed);
    if (scratch == NULL)
    {
      log_error("gemm packing scratch allocation failed");
      return 1;
    }
    float *Ap_full  = scratch;
    float *Bp_block = scratch + (size_t)m_full * k;

    /*
     * For both src and mat, group every 8 rows into a panel:
     *
     *  src (m x k)           mat (n x k)           dst (m x n)
     * +-------------+       +-------------+       +-------------+
     * | 8 rows (A0) |--+    | 8 rows (B0) |--+    |             |
     * +-------------+  |    +-------------+  |    |   ib x jb   |
     * | 8 rows (A1) |  |    | 8 rows (B1) |  |    |    tiles    |
     * +-------------+  |    +-------------+  |    |             |
     * | 8 rows (A2) |  |    | 8 rows (B2) |  |    |             |
     * +-------------+  |    +-------------+  |    |             |
     * |    ...      |  V    |    ...      |  V    |             |
     * +-------------+       +-------------+       +-------------+
     * (m_full rows)         (n_full rows)
     *
     * Then transpose all the panels to column-major:
     *
     * original:
     * Ra: [aa ab ac ad ae af ag ah ai aj ak al am an ao]
     * Rb: [ba bb bc bd be bf bg bh bi bj bk bl bm bn bo]
     * Rc: [ca cb cc cd ce cf cg ch ci cj ck cl cm cn co]
     * Rd: [da db dc dd de df dg dh di dj dk dl dm dn do]
     * Re: [ea eb ec ed ee ef eg eh ei ej ek el em en eo]
     * Rf: [fa fb fc fd fe ff fg fh fi fj fk fl fm fn fo]
     * Rg: [ga gb gc gd ge gf gg gh gi gj gk gl gm gn go]
     * Rh: [ha hb hc hd he hf hg hh hi hj hk hl hm hn ho]
     *
     * packed:
     * Ra: [aa ba ca da ea fa ga ha]
     * Rb: [ab bb cb db eb fb gb hb]
     * Rc: [ac bc cc dc ec fc gc hc]
     * Rd: [ad bd cd dd ed fd gd hd]
     * Re: [ae be ce de ee fe ge he]
     * Rf: [af bf cf df ef ff gf hf]
     * Rg: [ag bg cg dg eg fg gg hg]
     * Rh: [ah bh ch dh eh fh gh hh]
     * Ri: [ai bi ci di ei fi gi hi]
     * Rj: [aj bj cj dj ej fj gj hj]
     * Rk: [ak bk ck dk ek fk gk hk]
     * Rl: [al bl cl dl el fl gl hl]
     * Rm: [am bm cm dm em fm gm hm]
     * Rn: [an bn cn dn en fn gn hn]
     * Ro: [ao bo co do eo fo go ho]
     */

    pack_all_fpx(Ap_full, src, src_stride, m_full, 8, k, omp);

    for (int jc = 0; jc < n_full; jc += GEMM_NC)
    {
      int nc = n_full - jc;
      if (nc > GEMM_NC) nc = GEMM_NC;

      pack_all_fpx(
        Bp_block, mat + (size_t)jc * mat_stride, mat_stride, nc, 8, k, omp
      );

      gemm_fpx_compute_block(
        dst + jc, dst_stride, Ap_full, Bp_block, m_full, nc, k, omp
      );
    }
  }

  // Remainder: leftover rows (full width) + leftover columns (remaining
  // height only, so the (m_full:m, n_full:n) corner isn't done twice)
  if (m_full < m)
  {
    gemm_fpx_scalar(
      dst + m_full * dst_stride, dst_stride, mat, mat_stride,
      src + m_full * src_stride, src_stride, m - m_full, n, k, omp
    );
  }
  if (n_full < n && m_full > 0)
  {
    gemm_fpx_scalar(
      dst + n_full, dst_stride, mat + n_full * mat_stride, mat_stride, src,
      src_stride, m_full, n - n_full, k, omp
    );
  }

  return 0;
}

/**
 * Compute an `mr`x`n` (mr <= 8) block of dst = src @ mat.
 * `mat` is converted from floatx -> float once per `l` (into `row`) and
 * then reused across all `mr` accumulator rows.
 */
static inline void
gemm_fpx_nn_kernel(
  floatx *RESTRICT       dst,
  int                    dst_stride,
  const floatx *RESTRICT mat,
  int                    mat_stride,
  const floatx *RESTRICT src,
  int                    src_stride,
  int                    mr,
  int                    n,
  int                    k
)
{
#if defined(__MSVC__)
#  define ACC(ii, j) acc[(ii) * n + (j)]

  float *acc = (float *)_alloca(8 * (size_t)n * sizeof(float));
  float *row = (float *)_alloca((size_t)n * sizeof(float));
  for (int ii = 0; ii < mr; ii++)
  {
    memset(acc + ii * n, 0, (size_t)n * sizeof(float));
  }
#else
#  define ACC(ii, j) acc[(ii)][(j)]

  float acc[8][n];
  float row[n];
  for (int ii = 0; ii < mr; ii++)
  {
    memset(acc[ii], 0, n * sizeof(float));
  }

#endif

  for (int l = 0; l < k; l++)
  {
    const floatx *mat_row = mat + l * mat_stride;
    for (int j = 0; j < n; j++)
    {
      row[j] = (float)mat_row[j];
    }

    for (int ii = 0; ii < mr; ii++)
    {
      float a = (float)src[ii * src_stride + l];
      for (int j = 0; j < n; j++)
      {
        ACC(ii, j) += a * row[j];
      }
    }
  }

  for (int ii = 0; ii < mr; ii++)
  {
    floatx *dst_row = dst + ii * dst_stride;
    for (int j = 0; j < n; j++)
    {
      dst_row[j] = (floatx)ACC(ii, j);
    }
  }

#undef ACC
}

/**
 * fpx matrix-matrix multiply (NN)
 * fpx src (m, k) @ fpx mat (k, n) = fpx dst (m, n)
 */
static void
gemm_fpx_nn(
  floatx *RESTRICT       dst,
  int                    dst_stride,
  const floatx *RESTRICT mat,
  int                    mat_stride,
  const floatx *RESTRICT src,
  int                    src_stride,
  int                    m,
  int                    n,
  int                    k,
  bool                   omp
)
{
  if (dst_stride == 0) dst_stride = n;
  if (mat_stride == 0) mat_stride = n;
  if (src_stride == 0) src_stride = k;

  int m_full = (m / 8) * 8;

  if (omp)
  {
    int ib;
    #pragma omp parallel for private(ib) OMP_PARA_ARGS
    for (ib = 0; ib < m_full; ib += 8)
    {
      gemm_fpx_nn_kernel(
        dst + ib * dst_stride, dst_stride, mat, mat_stride,
        src + ib * src_stride, src_stride, 8, n, k
      );
    }
  }
  else
  {
    for (int ib = 0; ib < m_full; ib += 8)
    {
      gemm_fpx_nn_kernel(
        dst + ib * dst_stride, dst_stride, mat, mat_stride,
        src + ib * src_stride, src_stride, 8, n, k
      );
    }
  }

  // Remainder: leftover rows (m % 8), still full width
  if (m_full < m)
  {
    gemm_fpx_nn_kernel(
      dst + m_full * dst_stride, dst_stride, mat, mat_stride,
      src + m_full * src_stride, src_stride, m - m_full, n, k
    );
  }
}

static thread_local std::vector<int8_t> gemm_i8_pack_scratch;

/* */
static int8_t *
gemm_i8_pack_scratch_get(size_t needed_bytes)
{
  gemm_i8_pack_scratch.resize(needed_bytes);
  return gemm_i8_pack_scratch.data();
}

/* */
static inline void
pack_panel_i8(
  int8_t *RESTRICT       packed,
  const int8_t *RESTRICT src,
  int                    row_stride,
  int                    rows,
  int                    k
)
{
  for (int r = 0; r < rows; r++)
  {
    const int8_t *row = src + r * row_stride;
    for (int l = 0; l < k; l++)
    {
      packed[l * rows + r] = row[l];
    }
  }
}

/* */
static void
pack_all_i8(
  int8_t *RESTRICT       packed_full,
  const int8_t *RESTRICT mat,
  int                    stride,
  int                    total_rows,
  int                    R,
  int                    k,
  bool                   omp
)
{
  if (omp)
  {
    int b;
    #pragma omp parallel for private(b) OMP_PARA_ARGS
    for (b = 0; b < total_rows; b += R)
    {
      pack_panel_i8(
        packed_full + (size_t)b * k, mat + (size_t)b * stride, stride, R, k
      );
    }
  }
  else
  {
    for (int b = 0; b < total_rows; b += R)
    {
      pack_panel_i8(
        packed_full + (size_t)b * k, mat + (size_t)b * stride, stride, R, k
      );
    }
  }
}

/* */
static inline void
gemm_int8_scalar_row(
  floatx *RESTRICT       dst,
  int                    dst_stride,
  const int8_t *RESTRICT mat,
  int                    mat_stride,
  const floatx *RESTRICT mat_scales,
  const int8_t *RESTRICT src,
  int                    src_stride,
  const floatx *RESTRICT src_scales,
  int                    i,
  int                    j,
  int                    k
)
{
  float fscale = (float)src_scales[i];

  int32_t       sum     = 0;
  const int8_t *src_row = src + i * src_stride;
  const int8_t *mat_row = mat + j * mat_stride;

  for (int l = 0; l < k; l++)
  {
    sum += (int32_t)src_row[l] * (int32_t)mat_row[l];
  }
  float val               = (float)sum * fscale * (float)mat_scales[j];
  dst[i * dst_stride + j] = (floatx)val;
}

/* Scalar fallback for the remainder tiles. */
static inline void
gemm_int8_scalar(
  floatx *RESTRICT       dst,
  int                    dst_stride,
  const int8_t *RESTRICT mat,
  int                    mat_stride,
  const floatx *RESTRICT mat_scales,
  const int8_t *RESTRICT src,
  int                    src_stride,
  const floatx *RESTRICT src_scales,
  int                    m,
  int                    n,
  int                    k,
  bool                   omp
)
{
  if (omp)
  {
    int i, j;
    #pragma omp parallel for private(i, j) OMP_COLLAPSE2 OMP_PARA_ARGS
    for (i = 0; i < m; i++)
      for (j = 0; j < n; j++)
      {
        gemm_int8_scalar_row(
          dst, dst_stride, mat, mat_stride, mat_scales, src, src_stride,
          src_scales, i, j, k
        );
      }
  }
  else
  {
    for (int i = 0; i < m; i++)
      for (int j = 0; j < n; j++)
      {
        gemm_int8_scalar_row(
          dst, dst_stride, mat, mat_stride, mat_scales, src, src_stride,
          src_scales, i, j, k
        );
      }
  }
}

/* Same int8 x int8 -> int32 outer-product micro-kernel. */
static inline void
gemm_int8_kernel_accum(
  int32_t *RESTRICT      acc_ext,
  int                    acc_stride,
  const int8_t *RESTRICT Bp,
  const int8_t *RESTRICT Ap,
  int                    kc
)
{
  int32_t acc[64] = {0};

  for (int l = 0; l < kc; l++)
  {
    // clang-format off
    int32_t a0 = Ap[l*8+0], a1 = Ap[l*8+1], a2 = Ap[l*8+2], a3 = Ap[l*8+3];
    int32_t a4 = Ap[l*8+4], a5 = Ap[l*8+5], a6 = Ap[l*8+6], a7 = Ap[l*8+7];
    int32_t b0 = Bp[l*8+0], b1 = Bp[l*8+1], b2 = Bp[l*8+2], b3 = Bp[l*8+3];
    int32_t b4 = Bp[l*8+4], b5 = Bp[l*8+5], b6 = Bp[l*8+6], b7 = Bp[l*8+7];

    acc[ 0] += a0*b0; acc[ 1] += a0*b1; acc[ 2] += a0*b2; acc[ 3] += a0*b3;
    acc[ 4] += a0*b4; acc[ 5] += a0*b5; acc[ 6] += a0*b6; acc[ 7] += a0*b7;
    acc[ 8] += a1*b0; acc[ 9] += a1*b1; acc[10] += a1*b2; acc[11] += a1*b3;
    acc[12] += a1*b4; acc[13] += a1*b5; acc[14] += a1*b6; acc[15] += a1*b7;
    acc[16] += a2*b0; acc[17] += a2*b1; acc[18] += a2*b2; acc[19] += a2*b3;
    acc[20] += a2*b4; acc[21] += a2*b5; acc[22] += a2*b6; acc[23] += a2*b7;
    acc[24] += a3*b0; acc[25] += a3*b1; acc[26] += a3*b2; acc[27] += a3*b3;
    acc[28] += a3*b4; acc[29] += a3*b5; acc[30] += a3*b6; acc[31] += a3*b7;
    acc[32] += a4*b0; acc[33] += a4*b1; acc[34] += a4*b2; acc[35] += a4*b3;
    acc[36] += a4*b4; acc[37] += a4*b5; acc[38] += a4*b6; acc[39] += a4*b7;
    acc[40] += a5*b0; acc[41] += a5*b1; acc[42] += a5*b2; acc[43] += a5*b3;
    acc[44] += a5*b4; acc[45] += a5*b5; acc[46] += a5*b6; acc[47] += a5*b7;
    acc[48] += a6*b0; acc[49] += a6*b1; acc[50] += a6*b2; acc[51] += a6*b3;
    acc[52] += a6*b4; acc[53] += a6*b5; acc[54] += a6*b6; acc[55] += a6*b7;
    acc[56] += a7*b0; acc[57] += a7*b1; acc[58] += a7*b2; acc[59] += a7*b3;
    acc[60] += a7*b4; acc[61] += a7*b5; acc[62] += a7*b6; acc[63] += a7*b7;
    // clang-format on
  }

  for (int i = 0; i < 8; i++)
    for (int j = 0; j < 8; j++)
      acc_ext[i * acc_stride + j] += acc[i * 8 + j];
}

/* Applies the per-row / per-column dequant scales to an (mc x nc) int32
 * accumulator block and writes the result into dst once. `src_scales` and
 * `mat_scales` are expected to already be offset to this block's first
 * row / column (i.e. src_scales[r] / mat_scales[c] are absolute, not relative
 * to some further outer offset). */
static inline void
gemm_int8_flush_block(
  floatx *RESTRICT        dst,
  int                     dst_stride,
  const int32_t *RESTRICT acc,
  int                     acc_stride,
  const floatx *RESTRICT  src_scales,
  const floatx *RESTRICT  mat_scales,
  int                     mc,
  int                     nc
)
{
  for (int r = 0; r < mc; r++)
  {
    float fscale = (float)src_scales[r];
    for (int c = 0; c < nc; c++)
      dst[r * dst_stride + c] =
        (floatx)((float)acc[r * acc_stride + c] * fscale * (float)mat_scales[c]);
  }
}

/* */
static inline void
gemm_int8_compute_row(
  floatx *RESTRICT       dst,  // dst + jc
  int                    dst_stride,
  const int8_t *RESTRICT Ap_full,           // packed A, (m_rows, k)
  const int8_t *RESTRICT Bp_block,          // packed B, (nc, k)
  const floatx *RESTRICT src_scales,        // absolute, one per row of Ap_full
  const floatx *RESTRICT mat_scales_block,  // offset to this jc block already
  int                    m_rows,
  int                    nc,
  int                    ic,
  int                    k
)
{
  int mc = m_rows - ic;
  if (mc > GEMM_MC) mc = GEMM_MC;

  static thread_local int32_t *acc_buf     = NULL;
  static thread_local size_t   acc_buf_cap = 0;
  size_t                        needed       = (size_t)GEMM_MC * GEMM_NC;

  if (needed > acc_buf_cap)
  {
    free(acc_buf);
    acc_buf     = (int32_t *)malloc(needed * sizeof(int32_t));
    acc_buf_cap = (acc_buf != NULL) ? needed : 0;
  }
  if (acc_buf == NULL)
  {
    log_error("gemm_int8 accumulator scratch allocation failed");
    return;
  }

  for (int r = 0; r < mc; r++)
    memset(acc_buf + (size_t)r * GEMM_NC, 0, (size_t)nc * sizeof(int32_t));

  for (int pc = 0; pc < k; pc += GEMM_KC)
  {
    int kc = k - pc;
    if (kc > GEMM_KC) kc = GEMM_KC;

    for (int jb = 0; jb < nc; jb += 8)
      for (int ib = 0; ib < mc; ib += 8)
      {
        const int8_t *Ap_panel = Ap_full + (size_t)(ic + ib) * k + pc * 8;
        const int8_t *Bp_panel = Bp_block + (size_t)jb * k + pc * 8;

        gemm_int8_kernel_accum(
          acc_buf + ib * GEMM_NC + jb, GEMM_NC, Bp_panel, Ap_panel, kc
        );
      }
  }

  gemm_int8_flush_block(
    dst + (size_t)ic * dst_stride, dst_stride, acc_buf, GEMM_NC,
    src_scales + ic, mat_scales_block, mc, nc
  );
}

/* Cache-blocked compute core for gemm_int8, blocks the m dimension by GEMM_MC
 * and the k dimension by GEMM_KC so the A/B slices touched by the innermost
 * loop stay small, then applies the dequant scales once per (ic, jc) tile after
 * the full k range has been summed in int32. */
static void
gemm_int8_compute_block(
  floatx *RESTRICT       dst,  // dst + jc
  int                    dst_stride,
  const int8_t *RESTRICT Ap_full,           // packed A, (m_rows, k)
  const int8_t *RESTRICT Bp_block,          // packed B, (nc, k)
  const floatx *RESTRICT src_scales,        // absolute, one per row of Ap_full
  const floatx *RESTRICT mat_scales_block,  // offset to this jc block already
  int                    m_rows,
  int                    nc,
  int                    k,
  bool                   omp
)
{
  int ic;

  if (omp)
  {
    #pragma omp parallel for private(ic) OMP_PARA_ARGS if (omp)
    for (ic = 0; ic < m_rows; ic += GEMM_MC)
    {
      gemm_int8_compute_row(
        dst, dst_stride, Ap_full, Bp_block, src_scales, mat_scales_block, m_rows,
        nc, ic, k
      );
    }
  }
  else
  {
    for (ic = 0; ic < m_rows; ic += GEMM_MC)
    {
      gemm_int8_compute_row(
        dst, dst_stride, Ap_full, Bp_block, src_scales, mat_scales_block, m_rows,
        nc, ic, k
      );
    }
  }
}

/**
 * int8 matrix-matrix multiply (NT) + dequant
 *
 *   (int8 src (m, k) * fpx src_scales (m,))
 * @ (int8 mat (n, k) * fpx mat_scales (n,)).T = (fpx dst (m, n))
 */
static int
gemm_int8(
  floatx *RESTRICT       dst,
  int                    dst_stride,
  const int8_t *RESTRICT mat,
  int                    mat_stride,
  const floatx *RESTRICT mat_scales,
  const int8_t *RESTRICT src,
  int                    src_stride,
  const floatx *RESTRICT src_scales,
  int                    m,
  int                    n,
  int                    k,
  bool                   omp
)
{
  if (dst_stride == 0) dst_stride = n;
  if (mat_stride == 0) mat_stride = k;
  if (src_stride == 0) src_stride = k;

  int m_full = (m / 8) * 8;
  int n_full = (n / 8) * 8;

  if (m_full > 0 && n_full > 0)
  {
    // Scratch holds the full A panel (m_full x k) plus one NC-wide B
    // column block (at most GEMM_NC x k) at a time, instead of the
    // whole n_full x k B matrix up front.
    int     nc_cap  = n_full < GEMM_NC ? n_full : GEMM_NC;
    size_t  needed  = (size_t)m_full * k + (size_t)nc_cap * k;
    int8_t *scratch = gemm_i8_pack_scratch_get(needed);
    if (scratch == NULL)
    {
      log_error("gemm_int8 packing scratch allocation failed");
      return 1;
    }
    int8_t *Ap_full  = scratch;
    int8_t *Bp_block = scratch + (size_t)m_full * k;

    pack_all_i8(Ap_full, src, src_stride, m_full, 8, k, omp);

    for (int jc = 0; jc < n_full; jc += GEMM_NC)
    {
      int nc = n_full - jc;
      if (nc > GEMM_NC) nc = GEMM_NC;

      pack_all_i8(
        Bp_block, mat + (size_t)jc * mat_stride, mat_stride, nc, 8, k, omp
      );

      gemm_int8_compute_block(
        dst + jc, dst_stride, Ap_full, Bp_block, src_scales,
        mat_scales + jc, m_full, nc, k, omp
      );
    }
  }

  // Remainder: leftover rows (full width) + leftover columns (remaining
  // height only, so the (m_full:m, n_full:n) corner isn't done twice)
  if (m_full < m)
  {
    gemm_int8_scalar(
      dst + m_full * dst_stride, dst_stride, mat, mat_stride, mat_scales,
      src + m_full * src_stride, src_stride, src_scales + m_full, m - m_full, n,
      k, omp
    );
  }
  if (n_full < n && m_full > 0)
  {
    gemm_int8_scalar(
      dst + n_full, dst_stride, mat + n_full * mat_stride, mat_stride,
      mat_scales + n_full, src, src_stride, src_scales, m_full, n - n_full, k,
      omp
    );
  }

  return 0;
}

/* */
static void
softmax(floatx *dst, const floatx *src, int dim)
{
  floatx max = -FLOATX_MAX;
  for (int i = 0; i < dim; i++)
  {
    if (src[i] > max)
    {
      max = src[i];
    }
  }

  float expsum = 0.0f;
  for (int i = 0; i < dim; i++)
  {
    float val = expf((float)(src[i] - max));
    dst[i]    = (floatx)val;
    expsum += val;
  }

  for (int i = 0; i < dim; i++)
  {
    dst[i] = (floatx)((float)dst[i] / expsum);
  }
}

/* */
std::vector<floatx>
prepare_image(const char *path, int image_size)
{
  int rows, cols, channels;
  std::unique_ptr<unsigned char, decltype(&stbi_image_free)> raw(
    load_image_utf8(path, &cols, &rows, &channels, 3), stbi_image_free
  );
  if (!raw)
  {
    log_error("%s", stbi_failure_reason());
    return {};
  }

  size_t count = (size_t)image_size * image_size * 3;
  std::vector<unsigned char> resized(count);
  if (!stbir_resize(
        raw.get(), cols, rows, 0, resized.data(), image_size, image_size, 0,
        STBIR_RGB, STBIR_TYPE_UINT8, STBIR_EDGE_CLAMP, STBIR_FILTER_CATMULLROM
      ))
    return {};

  std::vector<floatx> out(count);
  for (size_t i = 0; i < count; i++)
  {
    out[i] = (resized[i] / 255.0f - 0.5f) / 0.5f;
  }
  return out;
}

/* */
std::vector<floatx>
prepare_longimage(
  const char *path,
  int         image_size,
  int         min_crop_size,
  int         max_crops,
  int        *n_crops
)
{
  int            rows, cols, channels;
  std::vector<unsigned char> rsz;
  unsigned char *rsz_r = NULL;
  std::vector<floatx> out;
  int wn_crops = 0;
  int hn_crops = 0;
  int cw = 0, ch = 0;

  size_t epi = (size_t)image_size * image_size * 3;  // Elements per image

  *n_crops = 0;

  // Load the full image
  std::unique_ptr<unsigned char, decltype(&stbi_image_free)> raw(
    load_image_utf8(path, &cols, &rows, &channels, 3), stbi_image_free
  );
  if (!raw) return {};

  // Compute the number of crops (W / H)
  if (cols >= rows)
  {
    if ((float)cols / rows >= 1.5)
    {
      wn_crops = (int)floor((double)cols / rows + 0.5);
      int cap  = (int)floor((double)cols / min_crop_size);
      wn_crops = std::clamp(std::min(cap, wn_crops), 2, max_crops);
      hn_crops = 1;
    }
  }
  else
  {
    if ((float)rows / cols >= 1.5)
    {
      hn_crops = (int)floor((double)rows / cols + 0.5);
      int cap  = (int)floor((double)rows / min_crop_size);
      hn_crops = std::clamp(std::min(cap, hn_crops), 2, max_crops);
      wn_crops = 1;
    }
  }

  // Size of each crop
  if (wn_crops > 0 && hn_crops > 0)
  {
    cw = (cols + wn_crops - 1) / wn_crops;
    ch = (rows + hn_crops - 1) / hn_crops;
    if (cw < min_crop_size || ch < min_crop_size)
    {
      wn_crops = hn_crops = 0;
    }
  }

  int crop_count = wn_crops * hn_crops;

  rsz.resize((size_t)(crop_count + 1) * epi);
  out.resize((size_t)(crop_count + 1) * epi);

  // The first entry represents the full image (resized)
  rsz_r = (unsigned char *)stbir_resize(
    raw.get(), cols, rows, cols * 3, rsz.data(), image_size, image_size, 0, STBIR_RGB,
    STBIR_TYPE_UINT8, STBIR_EDGE_CLAMP, STBIR_FILTER_CATMULLROM
  );
  if (rsz_r == NULL) return {};
  for (size_t i = 0; i < epi; i++)
  {
    out[i] = (rsz[i] / 255.0f - 0.5f) / 0.5f;
  }

  for (int i = 0; i < hn_crops; i++)
    for (int j = 0; j < wn_crops; j++)
    {
      int idx = i * wn_crops + j + 1;  // +1 to skip over the first entry

      int px = j * cw;
      int py = i * ch;
      int pw = cw;
      int ph = ch;

      pw = std::min(pw, cols - px);
      ph = std::min(ph, rows - py);

      // Resize crop
      unsigned char *rsz_crop = rsz.data() + (size_t)idx * epi;
      unsigned char *raw_crop = raw.get() + ((size_t)py * cols + px) * 3;

      rsz_r = static_cast<unsigned char*>(stbir_resize(
        raw_crop, pw, ph, cols * 3, rsz_crop, image_size, image_size, 0,
        STBIR_RGB, STBIR_TYPE_UINT8, STBIR_EDGE_CLAMP, STBIR_FILTER_CATMULLROM
      ));
      if (rsz_r == NULL) return {};

      // Normalize into out
      floatx *dst = out.data() + (size_t)idx * epi;
      for (size_t k = 0; k < epi; k++)
      {
        dst[k] = (rsz_crop[k] / 255.0f - 0.5f) / 0.5f;
      }
    }

  *n_crops = crop_count + 1;
  return out;
}

// Forward passes

NO_FAST_MATH_BEGIN;

/**
 * Vision forward pass (SigLIP)
 * img: (image_size, image_size, 3)
 */
int
forward_vision(
  VisionEncoder *enc,
  TextConfig    *tcfg,
  TextBuffer    *tbuf,
  VisionBuffer  *vbuf,
  const floatx  *img,
  bool           quant
)
{
  VisionConfig *vcfg = &enc->config;

  int embed_dim   = vcfg->hidden_dim;
  int patch_size  = vcfg->patch_size;
  int image_size  = vcfg->image_size;
  int ppi         = image_size / patch_size;
  int n_patches   = ppi * ppi;
  int image_toks  = tcfg->image_toks;
  int side_len    = (int)roundf(sqrtf((float)image_toks));
  int kernel_size = (image_size / vcfg->patch_size) / side_len;
  int head_dim    = embed_dim / vcfg->n_heads;
  int mlp_dim     = vcfg->mlp_dim;
  int input_dim   = 3 * patch_size * patch_size;

  // Patch Embedding

  // Iterate over all the patch
  // Can be further optimized by reusing GEMM, but this part is executed only
  // once per call, the cost is acceptable
  int oy, ox;
  #pragma omp parallel for private(oy, ox) OMP_COLLAPSE2 OMP_PARA_ARGS
  for (oy = 0; oy < ppi; oy++)
    for (ox = 0; ox < ppi; ox++)
    {
      int patch_idx = oy * ppi + ox;

      // Compute the embed vector for the current patch
      for (int oc = 0; oc < embed_dim; oc++)
      {
        float sum = 0.0f;

        // equivalent to Conv2d(
        //   in_channels=3, out_channels=embed_dim, kernel_size=patch_size,
        //   stride=patch_size, bias=True)

        for (int py = 0; py < patch_size; py++)
          for (int px = 0; px < patch_size; px++)
            for (int c = 0; c < 3; c++)
            {
              // img[c, oy*patch_size + py, ox*patch_size + px]
              int in_idx =
                ((oy * patch_size + py) * image_size + (ox * patch_size + px)) *
                  3 +
                c;
              // patch_emb[oc, c, py, px]
              int w_idx = oc * input_dim + c * patch_size * patch_size +
                          py * patch_size + px;
              sum += (float)enc->patch_emb[w_idx] * (float)img[in_idx];
            }

        vbuf->x[patch_idx * embed_dim + oc] =
          clamp_fpx((floatx)(sum + (float)enc->patch_emb_b[oc]));
      }
    }

  if (is_interrupted()) return 1;

  // Position Embedding
  if (!quant)
  {
    for (int d = 0; d < n_patches * embed_dim; d++)
    {
      vbuf->x[d] = clamp_fpx(vbuf->x[d] + enc->pos_embedding.fpx[d]);
    }
  }
  else
  {
    // Dequantize per row
    for (int i = 0; i < n_patches; i++)
      for (int j = 0; j < embed_dim; j++)
      {
        float scale = (float)enc->pos_embedding.i8.scales[i];
        float val = (float)enc->pos_embedding.i8.q[i * embed_dim + j] * scale;
        vbuf->x[i * embed_dim + j] =
          clamp_fpx(vbuf->x[i * embed_dim + j] + (floatx)val);
      }
  }

  if (is_interrupted()) return 1;

  // Encoder Layers
  for (auto &layer : enc->layers)
  {

    memcpy(vbuf->resid.data(), vbuf->x.data(), n_patches * embed_dim * sizeof(floatx));

    int i;
    #pragma omp parallel for private(i) OMP_PARA_ARGS
    for (i = 0; i < n_patches; i++)
    {
      floatx *row = vbuf->x.data() + i * embed_dim;
      layernorm(row, row, layer.n1, layer.n1_b, embed_dim, vcfg->eps);
    }
    if (is_interrupted()) return 1;

    // QKV projections
    if (!quant)
    {
      if (gemm_fpx(
            vbuf->xq.data(), 0, layer.wq.fpx, 0, vbuf->x.data(), 0, n_patches, embed_dim,
            embed_dim, true
          ) == 1 ||
          gemm_fpx(
            vbuf->xk.data(), 0, layer.wk.fpx, 0, vbuf->x.data(), 0, n_patches, embed_dim,
            embed_dim, true
          ) == 1 ||
          gemm_fpx(
            vbuf->xv.data(), 0, layer.wv.fpx, 0, vbuf->x.data(), 0, n_patches, embed_dim,
            embed_dim, true
          ) == 1)
        return 1;
    }
    else
    {
      quantize_acts(
        vbuf->x_i8.data(), vbuf->x_scales.data(), vbuf->x.data(), 0, n_patches, embed_dim, true
      );
      if (gemm_int8(
            vbuf->xq.data(), 0, layer.wq.i8.q, 0, layer.wq.i8.scales, vbuf->x_i8.data(),
            0, vbuf->x_scales.data(), n_patches, embed_dim, embed_dim, true
          ) == 1 ||
          gemm_int8(
            vbuf->xk.data(), 0, layer.wk.i8.q, 0, layer.wk.i8.scales, vbuf->x_i8.data(),
            0, vbuf->x_scales.data(), n_patches, embed_dim, embed_dim, true
          ) == 1 ||
          gemm_int8(
            vbuf->xv.data(), 0, layer.wv.i8.q, 0, layer.wv.i8.scales, vbuf->x_i8.data(),
            0, vbuf->x_scales.data(), n_patches, embed_dim, embed_dim, true
          ) == 1)
        return 1;
    }
    if (is_interrupted()) return 1;

    // Add biases
    for (int i = 0; i < n_patches; i++)
      for (int j = 0; j < embed_dim; j++)
      {
        int idx = i * embed_dim + j;
        vbuf->xq[idx] += layer.bq[j];
        vbuf->xk[idx] += layer.bk[j];
        vbuf->xv[idx] += layer.bv[j];
      }
    if (is_interrupted()) return 1;

    // Attention
    memset(vbuf->att_out.data(), 0, n_patches * embed_dim * sizeof(floatx));
    float scale = 1.0f / sqrtf((float)head_dim);

    int  h;
    bool quit = false;

    #pragma omp parallel for private(h) reduction(|| : quit) OMP_PARA_ARGS
    for (h = 0; h < vcfg->n_heads; h++)
    {
      floatx *scores =
        vbuf->scores.data() +
        h * n_patches * n_patches;  // (n_patches, n_patches) for this head

      // scores = Q @ K^T * scale
      if (gemm_fpx(
            scores, /*dst_stride=*/n_patches, vbuf->xk.data() + h * head_dim,
            /*mat_stride=*/embed_dim, vbuf->xq.data() + h * head_dim,
            /*src_stride=*/embed_dim, n_patches, n_patches, head_dim, false
          ) == 1)
        quit = true;

      // Apply scale and softmax per row
      for (int i = 0; i < n_patches; i++)
      {
        floatx *row = scores + i * n_patches;
        for (int j = 0; j < n_patches; j++)
        {
          row[j] *= scale;
        }
        softmax(row, row, n_patches);
      }

      // Weighted sum of values: out = scores @ V
      gemm_fpx_nn(
        vbuf->att_out.data() + h * head_dim, /*dst_stride=*/embed_dim,
        vbuf->xv.data() + h * head_dim,
        /*mat_stride=*/embed_dim, scores, /*src_stride=*/n_patches, n_patches,
        head_dim, n_patches, false
      );
    }
    if (quit || is_interrupted()) return 1;

    // Output projection
    if (!quant)
    {
      if (gemm_fpx(
            vbuf->x.data(), 0, layer.wo.fpx, 0, vbuf->att_out.data(), 0, n_patches,
            embed_dim, embed_dim, true
          ) == 1)
        return 1;
    }
    else
    {
      quantize_acts(
        vbuf->x_i8.data(), vbuf->x_scales.data(), vbuf->att_out.data(), 0, n_patches, embed_dim, true
      );
      if (gemm_int8(
            vbuf->x.data(), 0, layer.wo.i8.q, 0, layer.wo.i8.scales, vbuf->x_i8.data(), 0,
            vbuf->x_scales.data(), n_patches, embed_dim, embed_dim, true
          ) == 1)
        return 1;
    }
    if (is_interrupted()) return 1;

    // Add output bias
    for (int i = 0; i < n_patches; i++)
      for (int j = 0; j < embed_dim; j++)
      {
        vbuf->x[i * embed_dim + j] += layer.bo[j];
      }
    if (is_interrupted()) return 1;

    // Residual connection
    for (int i = 0; i < n_patches * embed_dim; i++)
    {
      vbuf->x[i] = clamp_fpx(vbuf->x[i] + vbuf->resid[i]);
    }
    if (is_interrupted()) return 1;

    memcpy(vbuf->resid.data(), vbuf->x.data(), n_patches * embed_dim * sizeof(floatx));

    #pragma omp parallel for private(i) OMP_PARA_ARGS
    for (i = 0; i < n_patches; i++)
    {
      floatx *row = vbuf->x.data() + i * embed_dim;
      layernorm(row, row, layer.n2, layer.n2_b, embed_dim, vcfg->eps);
    }
    if (is_interrupted()) return 1;

    // x @ fc1 = mlp_hidden
    if (!quant)
    {
      if (gemm_fpx(
            vbuf->mlp_hidden.data(), 0, layer.w1.fpx, 0, vbuf->x.data(), 0, n_patches,
            mlp_dim, embed_dim, true
          ) == 1)
        return 1;
    }
    else
    {
      quantize_acts(
        vbuf->x_i8.data(), vbuf->x_scales.data(), vbuf->x.data(), 0, n_patches, embed_dim, true
      );
      if (gemm_int8(
            vbuf->mlp_hidden.data(), 0, layer.w1.i8.q, 0, layer.w1.i8.scales,
            vbuf->x_i8.data(), 0, vbuf->x_scales.data(), n_patches, mlp_dim, embed_dim, true
          ) == 1)
        return 1;
    }
    if (is_interrupted()) return 1;

    for (int i = 0; i < n_patches; i++)
      for (int j = 0; j < mlp_dim; j++)
      {
        // Apply fc1 biases
        float val =
          (float)vbuf->mlp_hidden[i * mlp_dim + j] + (float)layer.b1[j];
        // GELU tanh approximation
        float c = 0.79788456080287f;
        val =
          0.5f * val * (1.0f + tanhf(c * (val + 0.044715f * val * val * val)));
        vbuf->mlp_hidden[i * mlp_dim + j] = (floatx)val;
      }
    if (is_interrupted()) return 1;

    // mlp_hidden @ fc2 = x
    if (!quant)
    {
      if (gemm_fpx(
            vbuf->x.data(), 0, layer.w2.fpx, 0, vbuf->mlp_hidden.data(), 0, n_patches,
            embed_dim, mlp_dim, true
          ) == 1)
        return 1;
    }
    else
    {
      quantize_acts(
        vbuf->mlp_i8.data(), vbuf->mlp_scales.data(), vbuf->mlp_hidden.data(), 0, n_patches, mlp_dim,
        true
      );
      if (gemm_int8(
            vbuf->x.data(), 0, layer.w2.i8.q, 0, layer.w2.i8.scales, vbuf->mlp_i8.data(),
            0, vbuf->mlp_scales.data(), n_patches, embed_dim, mlp_dim, true
          ) == 1)
        return 1;
    }
    if (is_interrupted()) return 1;

    // x += b2
    for (int i = 0; i < n_patches; i++)
      for (int j = 0; j < embed_dim; j++)
      {
        vbuf->x[i * embed_dim + j] += layer.b2[j];
      }

    if (is_interrupted()) return 1;

    // Residual connection
    #pragma omp parallel for private(i) OMP_PARA_ARGS
    for (i = 0; i < n_patches * embed_dim; i++)
    {
      vbuf->x[i] = clamp_fpx(vbuf->x[i] + vbuf->resid[i]);
    }
    if (is_interrupted()) return 1;
  }

  // Post-norm + average pooling down to image_toks tokens
  int i;
  #pragma omp parallel for private(i) OMP_PARA_ARGS
  for (i = 0; i < n_patches; i++)
  {
    floatx *row = vbuf->x.data() + i * embed_dim;
    layernorm(row, row, enc->post_norm, enc->post_norm_b, embed_dim, vcfg->eps);
  }
  if (is_interrupted()) return 1;

  // Average pooling (AvgPool2d(kernel_size=K, stride=K))

  /* Be careful that this loop must remain serial, in-place pooling writes
   * results to the front of vbuf->x.data(), which overlaps with input data needed by
   * other output positions. Parallelizing this loop introduces a read/write
   * race condition. */
  for (int oy = 0; oy < side_len; oy++)
    for (int ox = 0; ox < side_len; ox++)
    {
      int out_idx = (oy * side_len + ox) * embed_dim;
      for (int d = 0; d < embed_dim; d++)
      {
        float sum = 0.0f;
        for (int ky = 0; ky < kernel_size; ky++)
        {
          for (int kx = 0; kx < kernel_size; kx++)
          {
            int py        = oy * kernel_size + ky;
            int px        = ox * kernel_size + kx;
            int token_idx = (py * ppi + px) * embed_dim + d;
            sum += (float)vbuf->x[token_idx];
          }
        }
        vbuf->x[out_idx + d] = (floatx)(sum / (kernel_size * kernel_size));
      }
    }

  if (is_interrupted()) return 1;
  // tbuf->x.data() now becomes (image_toks, embed_dim)

  // Final RMSNorm
  std::mdspan v_x(vbuf->x.data(), image_toks, embed_dim);
  rmsnorm(v_x, v_x, enc->norm, vcfg->eps);
  if (is_interrupted()) return 1;

  // Final projection into language model embedding space
  // x (image_toks, embed_dim) = x (image_toks, embed_dim)
  //                           @ proj (embed_dim, embed_dim)
  if (!quant)
  {
    if (gemm_fpx(
          tbuf->x.data(), 0, enc->proj.fpx, 0, vbuf->x.data(), 0, image_toks,
          tcfg->embed_dim, embed_dim, true
        ) == 1)
      return 1;
  }
  else
  {
    quantize_acts(
      vbuf->x_i8.data(), vbuf->x_scales.data(), vbuf->x.data(), 0, image_toks, embed_dim, true
    );
    if (gemm_int8(
          tbuf->x.data(), 0, enc->proj.i8.q, 0, enc->proj.i8.scales, vbuf->x_i8.data(), 0,
          vbuf->x_scales.data(), image_toks, tcfg->embed_dim, embed_dim, true
        ) == 1)
      return 1;
  }
  if (is_interrupted()) return 1;

  return 0;
}

/* Language model forward (one token) */
int
forward_text_decode(
  TextDecoder *dec, TextBuffer *tbuf, int pos, bool quant, bool compute_logits
)
{
  TextConfig *tcfg = &dec->config;

  int embed_dim  = tcfg->embed_dim;
  int n_heads    = tcfg->n_heads;
  int n_kv_heads = tcfg->n_kv_heads;
  int head_dim   = tcfg->head_dim;
  int q_size     = n_heads * head_dim;
  int kv_size    = n_kv_heads * head_dim;

  if (pos >= tbuf->cache_len)
  {
    log_error("kv cache is full");
    return 1;
  }

  // Precompute cos & sin for all frequencies (used in RoPE)
  for (int d = 0; d < head_dim / 2; d++)
  {
    float freq;
    float e = (float)(-2 * d) / (float)head_dim;  // exponent

    // Rotation angles for sliding window attentions
    freq                          = powf(tcfg->local_theta, e);
    tbuf->csfreqs_slid[d * 2]     = (floatx)cosf(freq * (float)pos);
    tbuf->csfreqs_slid[d * 2 + 1] = (floatx)sinf(freq * (float)pos);

    // Rotation angles for full attentions
    freq                          = powf(tcfg->global_theta, e);
    tbuf->csfreqs_full[d * 2]     = (floatx)cosf(freq * (float)pos);
    tbuf->csfreqs_full[d * 2 + 1] = (floatx)sinf(freq * (float)pos);
  }
  if (is_interrupted()) return 1;

  // Forward all the layers
  floatx *layer_cache = tbuf->kv_cache.data();
  for (auto [layer, is_local] : std::views::zip(dec->layers, tcfg->att_layers))
  {
    memcpy(tbuf->resid.data(), tbuf->x.data(), embed_dim * sizeof(*tbuf->x.data()));

    std::mdspan x2_mdspan(tbuf->x.data(), 1, embed_dim);
    rmsnorm(x2_mdspan, x2_mdspan, layer.n1, tcfg->eps);
    if (is_interrupted()) return 1;

    // The attention block
    if (!quant)
    {
      gemv_fpx(
        tbuf->xq.data(), layer.wq.fpx, tbuf->x.data(), q_size, embed_dim, true
      );  // (n_heads, head_dim)
      // (n_kv_heads, head_dim)
      gemv_fpx(tbuf->xk.data(), layer.wk.fpx, tbuf->x.data(), kv_size, embed_dim, true);
      gemv_fpx(tbuf->xv.data(), layer.wv.fpx, tbuf->x.data(), kv_size, embed_dim, true);
    }
    else
    {
      floatx x_scale = quantize_act(tbuf->x_i8.data(), tbuf->x.data(), embed_dim);
      gemv_int8(
        tbuf->xq.data(), layer.wq.i8.q, layer.wq.i8.scales, tbuf->x_i8.data(), x_scale,
        q_size, embed_dim, true
      );
      gemv_int8(
        tbuf->xk.data(), layer.wk.i8.q, layer.wk.i8.scales, tbuf->x_i8.data(), x_scale,
        kv_size, embed_dim, true
      );
      gemv_int8(
        tbuf->xv.data(), layer.wv.i8.q, layer.wv.i8.scales, tbuf->x_i8.data(), x_scale,
        kv_size, embed_dim, true
      );
    }
    if (is_interrupted()) return 1;

    if (tcfg->qk_norm)
    {
      // Use the non-threading version here since we are running this over
      // every head
      // Query RMSNorm
      std::mdspan xq(tbuf->xq.data(), n_heads, head_dim);
      rmsnorm(xq, xq, layer.nq, tcfg->eps);
      // Key RMSNorm
      std::mdspan xk(tbuf->xk.data(), n_kv_heads, head_dim);
      rmsnorm(xk, xk, layer.nk, tcfg->eps);
    }
    if (is_interrupted()) return 1;

    floatx *freqs_cs = is_local ? tbuf->csfreqs_slid.data() : tbuf->csfreqs_full.data();

    // Apply RoPE to queries & keys
    for (int idx = 0; idx < n_heads + n_kv_heads; idx++)
    {
      floatx *data;
      if (idx < n_heads)
      {
        data = tbuf->xq.data() + idx * head_dim;  // Apply to queries
      }
      else
      {
        data = tbuf->xk.data() + (idx - n_heads) * head_dim;  // Apply to keys
      }

      for (int d = 0; d < head_dim / 2; d++)
      {
        float cfr = (float)freqs_cs[2 * d];
        float sfr = (float)freqs_cs[2 * d + 1];
        float a   = (float)data[d];  // Index in the first half vector
        float b   = (float)data[d + head_dim / 2];  // ... second half vector

        data[d]                = (floatx)(a * cfr - b * sfr);
        data[d + head_dim / 2] = (floatx)(a * sfr + b * cfr);
      }
    }
    if (is_interrupted()) return 1;

    // (n_kv_heads, cache_len, head_dim)
    floatx *k_cache = layer_cache;
    floatx *v_cache = k_cache + tbuf->cache_len * kv_size;
    layer_cache += (size_t)2 * tbuf->cache_len * kv_size;

    // Write to kv_cache
    for (int h = 0; h < n_kv_heads; h++)
    {
      floatx *xk_head =
        k_cache + h * tbuf->cache_len * head_dim + pos * head_dim;
      floatx *xv_head =
        v_cache + h * tbuf->cache_len * head_dim + pos * head_dim;
      memcpy(xk_head, tbuf->xk.data() + h * head_dim, head_dim * sizeof(*tbuf->xk.data()));
      memcpy(xv_head, tbuf->xv.data() + h * head_dim, head_dim * sizeof(*tbuf->xv.data()));
    }
    if (is_interrupted()) return 1;

    // Sliding-window (true) or full attention (false)?
    bool local_att = is_local && pos >= tcfg->slide_len;
    // Starting position of kv_cache
    int spos   = local_att ? (pos + 1 - tcfg->slide_len) : 0;
    int attlen = pos + 1 - spos;  // Include the current pos

    floatx att_scale = (floatx)(1.0f / sqrtf((float)tcfg->q_scale));

    // Iterate over all the attention heads
    int h;
    #pragma omp parallel for private(h) OMP_PARA_ARGS
    for (h = 0; h < n_heads; h++)
    {
      int h_kv = h * n_kv_heads / n_heads;  // GQA mapping

      floatx *xq_head = tbuf->xq.data() + h * head_dim;  // xq[h, :]
      // k_cache[h_kv, spos:, :]
      floatx *xk_head =
        k_cache + h_kv * tbuf->cache_len * head_dim + spos * head_dim;
      // att[h, spos:]
      floatx *att_head = tbuf->att.data() + h * attlen + spos;

      // Compute dot product of the current query across all the keys
      gemv_fpx(att_head, xk_head, xq_head, attlen, head_dim, false);
      for (int t = 0; t < attlen; t++)
      {
        att_head[t] *= att_scale;
      }

      // Attention score softcapping
      if (tcfg->att_softcap != 0.0f)
      {
        for (int t = 0; t < attlen; t++)
        {
          float val   = (float)att_head[t] / tcfg->att_softcap;
          att_head[t] = (floatx)(tanhf(val) * tcfg->att_softcap);
        }
      }

      // Softmax
      softmax(att_head, att_head, attlen);

      // Compute output as weighted sum of values
      // v_cache[h_kv, spos:, :]
      floatx *xv_head =
        v_cache + h_kv * tbuf->cache_len * head_dim + spos * head_dim;
      floatx *xo_head = tbuf->xo.data() + h * head_dim;

      // xo_head (head_dim,) = att_head (attlen,) @ xv_head (attlen, head_dim)
      gemv_fpx_nn(xo_head, att_head, xv_head, 0, attlen, head_dim);
    }
    if (is_interrupted()) return 1;

    // Output projection maps xo back to x
    // x (embed_dim,) = xo (head_dim,) @ wo.T (head_dim, embed_dim)
    if (!quant)
    {
      gemv_fpx(tbuf->x.data(), layer.wo.fpx, tbuf->xo.data(), embed_dim, q_size, true);
    }
    else
    {
      floatx xo_scale = quantize_act(tbuf->xo_i8.data(), tbuf->xo.data(), q_size);
      gemv_int8(
        tbuf->x.data(), layer.wo.i8.q, layer.wo.i8.scales, tbuf->xo_i8.data(), xo_scale,
        embed_dim, q_size, true
      );
    }
    if (is_interrupted()) return 1;

    std::mdspan x3_mdspan(tbuf->x.data(), 1, embed_dim);
    rmsnorm(x3_mdspan, x3_mdspan, layer.n2, tcfg->eps);
    if (is_interrupted()) return 1;

    // Combine the residual stream
    floatx *RESTRICT x     = tbuf->x.data();
    floatx *RESTRICT resid = tbuf->resid.data();
    for (int d = 0; d < embed_dim; d++)
    {
      /* Sometimes the residual stream accumulates huge values on certain
       * channels, especially in pretrained/bigger models (Sun et al., 2024,
       * https://arxiv.org/abs/2402.17762). It works fine in fp32 or bf16, but
       * it can easily overflow fp16 and become inf, causing all the activations
       * turning into nan after the next RMSNorm, so we need to clamp it. */

      /* NOTE: Actually this should never trigger now since I added activation
       * scalers afterwards (see export.py), the clamp here is more of a
       * last-resort safety net. */

      tbuf->x[d] = clamp_fpx(x[d] + resid[d]);
    }
    if (is_interrupted()) return 1;

    memcpy(tbuf->resid.data(), tbuf->x.data(), embed_dim * sizeof(*tbuf->x.data()));

    // Pre feedforward RMSNorm
    if (tcfg->pre_mlp_norm)
    {
      std::mdspan x(tbuf->x.data(), 1, embed_dim);
      rmsnorm(x, x, layer.n3, tcfg->eps);
    }
    if (is_interrupted()) return 1;

    // MLP feedforward layer (SwiGLU-style)
    if (!quant)
    {
      gemv_fpx(
        tbuf->xu.data(), layer.w1.fpx, tbuf->x.data(), tcfg->mlp_dim, embed_dim, true
      );
      gemv_fpx(
        tbuf->xg.data(), layer.w2.fpx, tbuf->x.data(), tcfg->mlp_dim, embed_dim, true
      );
    }
    else
    {
      floatx x_scale = quantize_act(tbuf->x_i8.data(), tbuf->x.data(), embed_dim);
      gemv_int8(
        tbuf->xg.data(), layer.w2.i8.q, layer.w2.i8.scales, tbuf->x_i8.data(), x_scale,
        tcfg->mlp_dim, embed_dim, true
      );
      gemv_int8(
        tbuf->xu.data(), layer.w1.i8.q, layer.w1.i8.scales, tbuf->x_i8.data(), x_scale,
        tcfg->mlp_dim, embed_dim, true
      );
    }
    if (is_interrupted()) return 1;

    // GELU gate
    int d;
    #pragma omp parallel for private(d) OMP_PARA_ARGS
    for (d = 0; d < tcfg->mlp_dim; d++)
    {
      // Tanh approximation of GELU
      float x     = (float)tbuf->xg[d];
      float c     = 0.79788456080287f;  // = sqrt(2 / pi)
      x           = 0.5 * x * (1 + tanhf(c * (x + 0.044715 * x * x * x)));
      tbuf->xg[d] = (floatx)x;
      tbuf->xg[d] *= tbuf->xu[d];  // Fuse xg * xu into xg
    }
    if (is_interrupted()) return 1;

    if (!quant)
    {
      gemv_fpx(
        tbuf->x.data(), layer.w3.fpx, tbuf->xg.data(), embed_dim, tcfg->mlp_dim, true
      );
    }
    else
    {
      floatx xscale = quantize_act(tbuf->xg_i8.data(), tbuf->xg.data(), tcfg->mlp_dim);
      gemv_int8(
        tbuf->x.data(), layer.w3.i8.q, layer.w3.i8.scales, tbuf->xg_i8.data(), xscale,
        embed_dim, tcfg->mlp_dim, true
      );
    }
    if (is_interrupted()) return 1;

    // Post feedforward RMSNorm
    if (tcfg->pst_mlp_norm)
    {
      std::mdspan x(tbuf->x.data(), 1, embed_dim);
      rmsnorm(x, x, layer.n4, tcfg->eps);
    }
    if (is_interrupted()) return 1;

    // Second residual
    x     = tbuf->x.data();
    resid = tbuf->resid.data();
    for (int d = 0; d < embed_dim; d++)
    {
      tbuf->x[d] = clamp_fpx(x[d] + resid[d]);
    }
    if (is_interrupted()) return 1;
  }

  // Final RMSNorm
  std::mdspan x(tbuf->x.data(), 1, embed_dim);
  rmsnorm(x, x, dec->final_norm, tcfg->eps);
  if (is_interrupted()) return 1;

  // Compute logits (tied embedding)
  if (compute_logits)
  {
    if (!quant)
    {
      gemv_fpx(
        tbuf->logits.data(), dec->embedding.fpx, tbuf->x.data(), tcfg->vocab_size, embed_dim,
        true
      );
    }
    else
    {
      floatx xscale = quantize_act(tbuf->x_i8.data(), tbuf->x.data(), embed_dim);
      gemv_int8(
        tbuf->logits.data(), dec->embedding.i8.q, dec->embedding.i8.scales,
        tbuf->x_i8.data(), xscale, tcfg->vocab_size, embed_dim, true
      );
    }
    if (is_interrupted()) return 1;

    // Optional logit softcapping
    if (tcfg->logit_softcap != 0.0f)
    {
      for (uint32_t d = 0; d < tcfg->vocab_size; d++)
      {
        float val       = (float)tbuf->logits[d] / tcfg->logit_softcap;
        tbuf->logits[d] = (floatx)(tanhf(val) * tcfg->logit_softcap);
      }
    }
    if (is_interrupted()) return 1;
  }
  return 0;
}

/* Language model forward (a chunk of tokens) */
static int
forward_text_chunk(
  TextDecoder *dec,
  TextBuffer  *tbuf,
  int          spos,
  int          n_tokens,
  bool         mask,
  bool         quant,
  bool         compute_logits
)
{
  TextConfig *tcfg = &dec->config;

  if (quant && (tbuf->x_scales.data() == NULL || tbuf->xo_scales.data() == NULL ||
                 tbuf->xg_scales.data() == NULL))
  {
    log_error("scale buffers are not allocated");
    return 1;
  }

  int embed_dim  = tcfg->embed_dim;
  int n_heads    = tcfg->n_heads;
  int n_kv_heads = tcfg->n_kv_heads;
  int head_dim   = tcfg->head_dim;
  int q_size     = n_heads * head_dim;
  int kv_size    = n_kv_heads * head_dim;

  int epos = spos + n_tokens - 1;

  if (epos >= tbuf->cache_len)
  {
    log_error("kv cache is full");
    return 1;
  }

  // Precompute RoPE angles
  int t;
  #pragma omp parallel for private(t) OMP_PARA_ARGS
  for (t = 0; t < n_tokens; t++)
  {
    int pos = spos + t;
    int off = t * head_dim;

    for (int d = 0; d < head_dim / 2; d++)
    {
      float freq;
      float e = (float)(-2 * d) / (float)head_dim;

      // Sliding window angles
      freq                                = powf(tcfg->local_theta, e);
      tbuf->csfreqs_slid[off + d * 2]     = (floatx)cosf(freq * (float)pos);
      tbuf->csfreqs_slid[off + d * 2 + 1] = (floatx)sinf(freq * (float)pos);

      // Full attention angles
      freq                                = powf(tcfg->global_theta, e);
      tbuf->csfreqs_full[off + d * 2]     = (floatx)cosf(freq * (float)pos);
      tbuf->csfreqs_full[off + d * 2 + 1] = (floatx)sinf(freq * (float)pos);
    }
  }
  if (is_interrupted()) return 1;

  floatx att_scale = (floatx)(1.0f / sqrtf((float)tcfg->q_scale));

  for (auto &layer : dec->layers)
  {
    const size_t l = &layer - dec->layers.data();

    memcpy(tbuf->resid.data(), tbuf->x.data(), n_tokens * embed_dim * sizeof(*tbuf->x.data()));

    int t;
    std::mdspan x_mdspan1(tbuf->x.data(), n_tokens, embed_dim);
    rmsnorm(x_mdspan1, x_mdspan1, layer.n1, tcfg->eps);
    if (is_interrupted()) return 1;

    // The attention block

    if (quant)
    {
      quantize_acts(
        tbuf->x_i8.data(), tbuf->x_scales.data(), tbuf->x.data(), 0, n_tokens, embed_dim, true
      );
    }

    // Compute xq & xk & xv
    int  h;
    bool quit = false;

    #pragma omp parallel for private(h) reduction(|| : quit) OMP_PARA_ARGS
    for (h = 0; h < n_heads; h++)
    {
      floatx *xq_head =
        tbuf->xq.data() + h * n_tokens * head_dim;  // (n_tokens, head_dim)
      if (!quant)
      {
        if (gemm_fpx(
              xq_head, 0, layer.wq.fpx + h * head_dim * embed_dim, 0, tbuf->x.data(),
              0, n_tokens, head_dim, embed_dim, false
            ) == 1)
          quit = true;
      }
      else
      {
        if (gemm_int8(
              xq_head, 0, layer.wq.i8.q + h * head_dim * embed_dim, 0,
              layer.wq.i8.scales + h * head_dim, tbuf->x_i8.data(), 0,
              tbuf->x_scales.data(), n_tokens, head_dim, embed_dim, false
            ) == 1)
          quit = true;
      }
      if (h >= n_kv_heads) continue;

      floatx *xk_head = tbuf->xk.data() + h * n_tokens * head_dim;
      floatx *xv_head = tbuf->xv.data() + h * n_tokens * head_dim;

      if (!quant)
      {
        // (n_tokens, n_kv_heads, head_dim)
        if (gemm_fpx(
              xk_head, 0, layer.wk.fpx + h * head_dim * embed_dim, 0, tbuf->x.data(),
              0, n_tokens, head_dim, embed_dim, false
            ) == 1 ||
            gemm_fpx(
              xv_head, 0, layer.wv.fpx + h * head_dim * embed_dim, 0, tbuf->x.data(),
              0, n_tokens, head_dim, embed_dim, false
            ) == 1)
          quit = true;
      }
      else
      {
        if (gemm_int8(
              xk_head, 0, layer.wk.i8.q + h * head_dim * embed_dim, 0,
              layer.wk.i8.scales + h * head_dim, tbuf->x_i8.data(), 0,
              tbuf->x_scales.data(), n_tokens, head_dim, embed_dim, false
            ) == 1 ||
            gemm_int8(
              xv_head, 0, layer.wv.i8.q + h * head_dim * embed_dim, 0,
              layer.wv.i8.scales + h * head_dim, tbuf->x_i8.data(), 0,
              tbuf->x_scales.data(), n_tokens, head_dim, embed_dim, false
            ) == 1)
          quit = true;
      }
    }
    if (quit || is_interrupted()) return 1;

    // Optional q & k norm
    if (tcfg->qk_norm)
    {
      int h;
      std::mdspan xq(tbuf->xq.data(), n_heads, n_tokens, head_dim);
      std::mdspan xk(tbuf->xk.data(), n_kv_heads, n_tokens, head_dim);
      // Normalize each token in this head exactly once
      for (h = 0; h < n_heads; h++)
      {
	      auto xq_ = std::submdspan(xq, h, std::full_extent, std::full_extent);
	      rmsnorm(xq_, xq_, layer.nq, tcfg->eps);
      }
      for (h = 0; h < n_kv_heads; h++)
      {
              auto xk_ = std::submdspan(xk, h, std::full_extent, std::full_extent);
	      rmsnorm(xk_, xk_, layer.nk, tcfg->eps);
      }
    }
    if (is_interrupted()) return 1;

    bool    is_local = tcfg->att_layers[l];
    floatx *freqs_cs = is_local ? tbuf->csfreqs_slid.data() : tbuf->csfreqs_full.data();

    // RoPE
    #pragma omp parallel for private(h, t) OMP_COLLAPSE2 OMP_PARA_ARGS
    for (h = 0; h < n_heads + n_kv_heads; h++)
      for (t = 0; t < n_tokens; t++)
      {
        floatx *data;
        if (h < n_heads)
        {
          data = tbuf->xq.data() + h * n_tokens * head_dim +
                 t * head_dim;  // Apply to queries
        }
        else
        {
          data = tbuf->xk.data() + (h - n_heads) * n_tokens * head_dim +
                 t * head_dim;  // Apply to keys
        }

        for (int d = 0; d < head_dim / 2; d++)
        {
          float cfr = (float)freqs_cs[2 * d + t * head_dim];
          float sfr = (float)freqs_cs[2 * d + 1 + t * head_dim];
          float a   = (float)data[d];  // Index in the first half vector
          float b   = (float)data[d + head_dim / 2];  // ... second half vector
          float r0  = a * cfr - b * sfr;
          float r1  = a * sfr + b * cfr;

          // Apply att_scale beforehand in this step, mathematically equivilant,
          // but avoided scaling the entire n_tokens * max_k attention matrix
          if (h < n_heads)
          {
            r0 *= (float)att_scale;
            r1 *= (float)att_scale;
          }

          data[d]                = (floatx)r0;
          data[d + head_dim / 2] = (floatx)r1;
        }
      }

    if (is_interrupted()) return 1;

    // (n_kv_heads, cache_len, head_dim)
    floatx *k_cache = tbuf->kv_cache.data() + l * 2 * tbuf->cache_len * kv_size;
    floatx *v_cache = k_cache + tbuf->cache_len * kv_size;

    // Write to kv_cache
    for (int h = 0; h < n_kv_heads; h++)
    {
      floatx *xk_head =
        k_cache + h * tbuf->cache_len * head_dim + spos * head_dim;
      floatx *xv_head =
        v_cache + h * tbuf->cache_len * head_dim + spos * head_dim;
      memcpy(
        xk_head, tbuf->xk.data() + h * n_tokens * head_dim,
        n_tokens * head_dim * sizeof(*tbuf->xk.data())
      );
      memcpy(
        xv_head, tbuf->xv.data() + h * n_tokens * head_dim,
        n_tokens * head_dim * sizeof(*tbuf->xv.data())
      );
    }
    if (is_interrupted()) return 1;

    int max_k = epos + 1;
    quit      = false;

    #pragma omp parallel for private(h) reduction(|| : quit) OMP_PARA_ARGS
    for (h = 0; h < n_heads; h++)
    {
      int h_kv = h * n_kv_heads / n_heads;  // GQA mapping

      // (n_tokens, max_k)
      floatx *att_head = tbuf->att.data() + h * n_tokens * max_k;  // att[h, :, :]
      // (cache_len, head_dim)
      floatx *xv_head = v_cache + h_kv * tbuf->cache_len * head_dim;
      floatx *xo_head =
        tbuf->xo.data() + h * head_dim;  // (n_tokens, head_dim), column stride q_size

      /* Blockwise causal masking, the tiling algorithm used in the
       * FlashAttention paper (Dao et al., 2022,
       * https://arxiv.org/abs/2205.14135).
       *
       * The original algorithm was designed for GPUs to reduce HBM access,
       * which is not a problem for CPUs, but the idea can also be used on CPUs
       * to provide better performance for causal masking.
       *
       * Naively computing causal attention means either (a) materializing the
       * full [n_tokens, attlen] score matrix and masking out the upper triangle
       * afterwards (wastes ~half the compute), or (b) looping token by token
       * with a triangular schedule (correct FLOP count, but ragged inner loop
       * length basically kills vectorization across threads). So I tile both
       * the query and key range into blocks and classify each (qb, kb) pair
       * into five cases:
       *
       * kb > qb -> skip, every key in this block lies in the future of queries.
       * kb < qb -> full GEMM, every key lies in the past of queries.
       * kb = qb -> full GEMM & mask the upper triangle.
       * kb < spos -> skip, every key in lies before the sliding window.
       * kb = spos -> full GEMM & mask the lower triangle.
       */

      if (mask)  // Only apply tiling if mask=true
      {
        for (int q_start = 0; q_start < n_tokens; q_start += QK_BLOCK_SIZE)
        {
          // xq[h, q_start:q_end, :] (q_end - q_start, head_dim)
          floatx *qb = tbuf->xq.data() + h * n_tokens * head_dim + q_start * head_dim;
          int     q_end       = std::min(q_start + QK_BLOCK_SIZE, n_tokens);
          int     abs_q_end   = spos + q_end;
          int     abs_q_start = spos + q_start;
          if (is_local && abs_q_start >= tcfg->slide_len)
          {
            abs_q_start = abs_q_start + 1 - tcfg->slide_len;
          }
          else
          {
            abs_q_start = 0;
          }
          int spos_b = abs_q_start / QK_BLOCK_SIZE * QK_BLOCK_SIZE;

          for (int k_start = spos_b; k_start < abs_q_end;
            k_start += QK_BLOCK_SIZE)
          {
            // k_cache[h_kv, k_start:k_end, :] (k_end - k_start, head_dim)
            floatx *kb =
              k_cache + h_kv * tbuf->cache_len * head_dim + k_start * head_dim;
            int     k_end = std::min(k_start + QK_BLOCK_SIZE, max_k);
            floatx *att_b = att_head + q_start * max_k + k_start;

            // Typically people don't quantize this
            if (gemm_fpx(
                  att_b, max_k, kb, 0, qb, 0, q_end - q_start, k_end - k_start,
                  head_dim, false
                ) == 1)
              quit = true;

            // Apply causal mask & sliding window mask
            for (int qi = q_start; qi < q_end; qi++)
            {
              int pos_i  = spos + qi;
              int spos_i = (is_local && pos_i >= tcfg->slide_len)
                             ? (pos_i + 1 - tcfg->slide_len)
                             : 0;
              for (int ki = k_start; ki < k_end; ki++)
              {
                if (ki > pos_i || ki < spos_i)
                {
                  att_head[qi * max_k + ki] = 0.0f;
                }
              }
            }
          }
          // Softmax & softcap for the rows
          for (int qi = q_start; qi < q_end; qi++)
          {
            floatx *att_row = att_head + qi * max_k;
            int     pos_i   = spos + qi;
            int     spos_i  = (is_local && pos_i >= tcfg->slide_len)
                                ? (pos_i + 1 - tcfg->slide_len)
                                : 0;

            // Optional tanh softcapping
            if (tcfg->att_softcap != 0.0f)
            {
              for (int t = spos_i; t <= pos_i; t++)
              {
                float val  = (float)att_row[t] / tcfg->att_softcap;
                att_row[t] = (floatx)(tanhf(val) * tcfg->att_softcap);
              }
            }
            // only softmax in the range [spos_i, pos_i]
            softmax(att_row + spos_i, att_row + spos_i, pos_i - spos_i + 1);
          }

          // xo_block (qb_len, head_dim) = att_block (qb_len, qb_len)
          //                             @ v_block (qb_len, head_dim)
          floatx *xo_block  = xo_head + q_start * q_size;
          floatx *att_block = att_head + q_start * max_k + spos_b;
          floatx *v_block   = xv_head + spos_b * head_dim;

          gemm_fpx_nn(
            xo_block, /*dst_stride=*/q_size, v_block, /*mat_stride=*/0,
            att_block,
            /*src_stride=*/max_k, q_end - q_start, head_dim, abs_q_end - spos_b,
            false
          );
        }
      }
      else  // mask=false, take the dense path
      {
        // (n_tokens, head_dim)
        floatx *xq_head = tbuf->xq.data() + h * n_tokens * head_dim;  // xq[h, :, :]
        // k_cache[h_kv, :, :] (cache_len, head_dim)
        floatx *xk_head = k_cache + h_kv * tbuf->cache_len * head_dim;
        // att_head = xq_head @ xk_head.T
        if (gemm_fpx(
              att_head, max_k, xk_head, 0, xq_head, 0, n_tokens, max_k,
              head_dim, false
            ))
          quit = true;

        for (int qi = 0; qi < n_tokens; qi++)
        {
          floatx *att_row = att_head + qi * max_k;

          // Optional tanh softcapping
          for (int t = 0; t < max_k; t++)
          {
            if (tcfg->att_softcap != 0.0f)
            {
              float val  = (float)att_row[t] / tcfg->att_softcap;
              att_row[t] = (floatx)(tanhf(val) * tcfg->att_softcap);
            }
          }
          // Softmax
          softmax(att_row, att_row, max_k);
        }
        // xo_head (n_tokens, head_dim) = att_head (n_tokens, max_k)
        //                              @ xv_head[:max_k] (max_k, head_dim)
        gemm_fpx_nn(
          xo_head, /*dst_stride=*/q_size, xv_head, /*mat_stride=*/0, att_head,
          /*src_stride=*/max_k, n_tokens, head_dim, max_k, false
        );
      }
    }
    if (is_interrupted()) return 1;
    // x (n_tokens, embed_dim) = xo_head (n_tokens, head_dim)
    //                         @ wo.T (head_dim, embed_dim)
    if (!quant)
    {
      if (gemm_fpx(
            tbuf->x.data(), 0, layer.wo.fpx, 0, tbuf->xo.data(), q_size, n_tokens,
            embed_dim, q_size, true
          ) == 1)
        return 1;
    }
    else
    {
      quantize_acts(
        tbuf->xo_i8.data(), tbuf->xo_scales.data(), tbuf->xo.data(), q_size, n_tokens, q_size, true
      );
      if (gemm_int8(
            tbuf->x.data(), 0, layer.wo.i8.q, 0, layer.wo.i8.scales, tbuf->xo_i8.data(),
            0, tbuf->xo_scales.data(), n_tokens, embed_dim, q_size, true
          ) == 1)
        return 1;
    }
    if (is_interrupted()) return 1;

    std::mdspan x_mdspan(tbuf->x.data(), n_tokens, embed_dim);
    rmsnorm(x_mdspan, x_mdspan, layer.n2, tcfg->eps);
    if (is_interrupted()) return 1;

    floatx *RESTRICT x     = tbuf->x.data();
    floatx *RESTRICT resid = tbuf->resid.data();

    int d;
    #pragma omp parallel for private(d) OMP_PARA_ARGS
    for (d = 0; d < n_tokens * embed_dim; d++)
    {
      // Combine the residual stream
      tbuf->x[d] = clamp_fpx(x[d] + resid[d]);
    }
    if (is_interrupted()) return 1;

    memcpy(tbuf->resid.data(), tbuf->x.data(), n_tokens * embed_dim * sizeof(*tbuf->x.data()));

    if (tcfg->pre_mlp_norm)
    {
      int t;
      #pragma omp parallel for private(t) OMP_PARA_ARGS
      for (t = 0; t < n_tokens; t++)
      {
        floatx *x_row = tbuf->x.data() + t * embed_dim;
        std::mdspan x(x_row, 1, embed_dim);
        rmsnorm(x, x, layer.n3, tcfg->eps);
      }
    }
    if (is_interrupted()) return 1;

    // MLP
    if (!quant)
    {
      if (gemm_fpx(
            tbuf->xu.data(), 0, layer.w1.fpx, 0, tbuf->x.data(), 0, n_tokens, tcfg->mlp_dim,
            embed_dim, true
          ) == 1 ||
          gemm_fpx(
            tbuf->xg.data(), 0, layer.w2.fpx, 0, tbuf->x.data(), 0, n_tokens, tcfg->mlp_dim,
            embed_dim, true
          ) == 1)
        return 1;
    }
    else
    {
      quantize_acts(
        tbuf->x_i8.data(), tbuf->x_scales.data(), tbuf->x.data(), 0, n_tokens, embed_dim, true
      );
      if (gemm_int8(
            tbuf->xu.data(), 0, layer.w1.i8.q, 0, layer.w1.i8.scales, tbuf->x_i8.data(),
            0, tbuf->x_scales.data(), n_tokens, tcfg->mlp_dim, embed_dim, true
          ) == 1 ||
          gemm_int8(
            tbuf->xg.data(), 0, layer.w2.i8.q, 0, layer.w2.i8.scales, tbuf->x_i8.data(),
            0, tbuf->x_scales.data(), n_tokens, tcfg->mlp_dim, embed_dim, true
          ))
        return 1;
    }
    if (is_interrupted()) return 1;

    // GELU gate
    #pragma omp parallel for private(d) OMP_PARA_ARGS
    for (d = 0; d < n_tokens * tcfg->mlp_dim; d++)
    {
      // Tanh approximation of GELU
      float x     = (float)tbuf->xg[d];
      float c     = 0.79788456080287f;  // = sqrt(2 / pi)
      x           = 0.5 * x * (1 + tanhf(c * (x + 0.044715 * x * x * x)));
      tbuf->xg[d] = (floatx)x;
      tbuf->xg[d] *= tbuf->xu[d];  // Fuse xg * xu into xg
    }
    if (is_interrupted()) return 1;

    // Down projection
    if (!quant)
    {
      if (gemm_fpx(
            tbuf->x.data(), 0, layer.w3.fpx, 0, tbuf->xg.data(), 0, n_tokens, embed_dim,
            tcfg->mlp_dim, true
          ) == 1)
        return 1;
    }
    else
    {
      quantize_acts(
        tbuf->xg_i8.data(), tbuf->xg_scales.data(), tbuf->xg.data(), 0, n_tokens, tcfg->mlp_dim, true
      );
      if (gemm_int8(
            tbuf->x.data(), 0, layer.w3.i8.q, 0, layer.w3.i8.scales, tbuf->xg_i8.data(),
            0, tbuf->xg_scales.data(), n_tokens, embed_dim, tcfg->mlp_dim, true
          ) == 1)
        return 1;
    }
    if (is_interrupted()) return 1;

    if (tcfg->pst_mlp_norm)
    {
      #pragma omp parallel for private(t) OMP_PARA_ARGS
      for (t = 0; t < n_tokens; t++)
      {
        floatx *x_row = tbuf->x.data() + t * embed_dim;
        std::mdspan x(x_row, 1, embed_dim);
        rmsnorm(x, x, layer.n4, tcfg->eps);
      }
    }
    if (is_interrupted()) return 1;

    x     = tbuf->x.data();
    resid = tbuf->resid.data();
    #pragma omp parallel for private(d) OMP_PARA_ARGS
    for (d = 0; d < n_tokens * embed_dim; d++)
    {
      // Second residual
      tbuf->x[d] = clamp_fpx(x[d] + resid[d]);
    }
    if (is_interrupted()) return 1;
  }

  // Final RMSNorm
  #pragma omp parallel for private(t) OMP_PARA_ARGS
  for (t = 0; t < n_tokens; t++)
  {
    floatx *x_row = tbuf->x.data() + t * embed_dim;
    std::mdspan x(x_row, 1, embed_dim);
    rmsnorm(x, x, dec->final_norm, tcfg->eps);
  }

  // Compute logits (tied embedding)
  if (compute_logits)
  {
    floatx *last_x = tbuf->x.data() + (n_tokens - 1) * embed_dim;
    if (!quant)
    {
      gemv_fpx(
        tbuf->logits.data(), dec->embedding.fpx, last_x, tcfg->vocab_size, embed_dim,
        true
      );
    }
    else
    {
      floatx xscale = quantize_act(tbuf->x_i8.data(), last_x, embed_dim);
      gemv_int8(
        tbuf->logits.data(), dec->embedding.i8.q, dec->embedding.i8.scales,
        tbuf->x_i8.data(), xscale, tcfg->vocab_size, embed_dim, true
      );
    }
    if (is_interrupted()) return 1;

    // Optional logit softcapping
    if (tcfg->logit_softcap != 0.0f)
    {
      for (uint32_t d = 0; d < tcfg->vocab_size; d++)
      {
        float val       = (float)tbuf->logits[d] / tcfg->logit_softcap;
        tbuf->logits[d] = (floatx)(tanhf(val) * tcfg->logit_softcap);
      }
    }
    if (is_interrupted()) return 1;
  }
  return 0;
}

NO_FAST_MATH_END;

/* Language + vision model forward */
int
forward_gemma_decode(GemmaModel *model, TextBuffer *tbuf, int token, int pos)
{
  TextDecoder *dec = model->decoder.get();

  /* Embedding lookup.
   * The export script already applied 1/sqrt(embed_dim), so the usual
   * Gemma sqrt(embed_dim) scale cancels out to 1.0 here. */
  floatx embed_scale =
    1.0f;  // equivalent to sqrt(embed_dim) * (1/sqrt(embed_dim))
  if (model->quant)
  {
    // Dequantize
    embed_scale *= dec->embedding.i8.scales[token];
  }

  int embed_dim = dec->config.embed_dim;

  // x = embedding[tok] * embed_scale
  int d;
  #pragma omp parallel for private(d) OMP_PARA_ARGS
  for (d = 0; d < embed_dim; d++)
  {
    if (!model->quant)
    {
      tbuf->x[d] = dec->embedding.fpx[token * embed_dim + d] * embed_scale;
    }
    else
    {
      tbuf->x[d] =
        (floatx)dec->embedding.i8.q[token * embed_dim + d] * embed_scale;
    }
  }
  if (is_interrupted()) return 1;
  return forward_text_decode(model->decoder.get(), tbuf, pos, model->quant, true);
}

/* Language model prefill */
int
forward_gemma_prefill(
  GemmaModel *model,
  TextBuffer *tbuf,
  int        *tokens,
  int         n_tokens,
  int        *pos,
  int         chunk_size,
  std::span<uint8_t> rpen_visited,
  bool        compute_logits
)
{
  TextDecoder *dec       = model->decoder.get();
  int          embed_dim = dec->config.embed_dim;

  // Prefill by chunks
  for (int off = 0; off < n_tokens; off += chunk_size)
  {
    int  cur_len       = std::min(chunk_size, n_tokens - off);
    bool is_last_chunk = (off + cur_len == n_tokens);

    // Embedding lookup
    for (int t = 0; t < cur_len; t++)
    {
      int token   = tokens[off + t];
      int emb_off = t * embed_dim;

      rpen_visited[token] = 1;

      for (int d = 0; d < embed_dim; d++)
      {
        if (!model->quant)
        {
          tbuf->x[emb_off + d] =
            dec->embedding.fpx[token * embed_dim + d];  // * 1.0f
        }
        else
        {
          tbuf->x[emb_off + d] =
            (floatx)dec->embedding.i8.q[token * embed_dim + d] *
            dec->embedding.i8.scales[token];
        }
      }
    }

    int rc = forward_text_chunk(
      dec, tbuf, *pos + off, cur_len, true, model->quant,
      is_last_chunk && compute_logits
    );

    if (rc != 0) return rc;
  }

  *pos += n_tokens;
  return 0;
}

/* Vision model forward + embedding prefill */
int
forward_gemma_image(
  GemmaModel   *model,
  TextBuffer   *tbuf,
  VisionBuffer *vbuf,
  const floatx *image,
  int          *pos,
  bool          compute_logits
)
{
  VisionEncoder *enc = model->encoder.get();
  if (enc == NULL)
  {
    return 1;
  }

  TextDecoder *dec = model->decoder.get();
  if (forward_vision(enc, &dec->config, tbuf, vbuf, image, model->quant) == 1)
  {
    return 1;
  }

  int image_toks = dec->config.image_toks;

  int suc = forward_text_chunk(
    // Gemma 3 models uses bi-directional attention for vision tokens
    dec, tbuf, *pos, image_toks, false, model->quant, compute_logits
  );

  *pos += image_toks;
  return suc;
}

// Sampling

/* */
static int
argmax(floatx *logits, int vocab_size)
{
  // Pick the index with the max value
  int    max_idx = -1;
  floatx max_val = -FLOATX_MAX;
  #pragma omp parallel OMP_PARA_ARGS
  {
    int    local_idx = -1;
    floatx local_val = -FLOATX_MAX;

    int i;
    #pragma omp for nowait
    for (i = 0; i < vocab_size; i++)
    {
      if (logits[i] > local_val)
      {
        local_val = logits[i];
        local_idx = i;
      }
    }
    #pragma omp critical
    {
      // Only one thread is able to run this at a time
      if (local_val > max_val)
      {
        max_val = local_val;
        max_idx = local_idx;
      }
    }
  }
  return max_idx;
}

/* */
typedef struct
{
  floatx val;
  int    idx;
} FloatIdx;

/* Tiny xorshift for pivot selection */
static uint32_t qs_rand_state = 3418323524;
static inline uint32_t
qs_rand(void)
{
  qs_rand_state ^= qs_rand_state << 13;
  qs_rand_state ^= qs_rand_state >> 7;
  qs_rand_state ^= qs_rand_state << 17;
  return (uint32_t)qs_rand_state;
}

/* */
static int
partition_desc(FloatIdx *arr, int lo, int hi)
{
  // Pick the pivot randomly (use a seperate rand sequence)
  uint32_t range = (uint32_t)(hi - lo + 1);
  int      r     = lo + (int)(qs_rand() % range);
  std::swap(arr[r], arr[hi]);

  floatx pivot = arr[hi].val;

  int i = lo;
  for (int j = lo; j < hi; j++)
  {
    if (arr[j].val > pivot)
    {
      // Put the greater one on the left
      std::swap(arr[i++], arr[j]);
    }
  }
  std::swap(arr[i], arr[hi]);
  return i;
}

/* Quickselect to find the top-k elements (descending) */
static void
quickselect_topk(FloatIdx *arr, int lo, int hi, int k_idx)
{
  while (lo < hi)
  {
    int p = partition_desc(arr, lo, hi);
    if (p == k_idx) return;
    else if (p < k_idx)
    {
      lo = p + 1;
    }
    else
    {
      hi = p - 1;
    }
  }
}

/* */
static void
apply_topk(floatx *logits, std::span<FloatIdx> logit_indices, int vocab_size, int k)
{
  if (k <= 0)
  {
    k = 1;
  }
  if (k > vocab_size)
  {
    k = vocab_size;
  }

  // Record index info
  int i;
  #pragma omp parallel for private(i) OMP_PARA_ARGS
  for (i = 0; i < vocab_size; i++)
  {
    logit_indices[i].idx = i;
    logit_indices[i].val = logits[i];
  }
  quickselect_topk(logit_indices.data(), 0, vocab_size - 1, k - 1);

  // Keep the top k channels
  #pragma omp parallel for private(i) OMP_PARA_ARGS
  for (i = 0; i < vocab_size; i++)
  {
    logits[i] = -FLOATX_MAX;
  }
  for (i = 0; i < k; i++)
  {
    logits[logit_indices[i].idx] = logit_indices[i].val;
  }
}

/* Max-heap helpers for top-p */
static void
sift_down(FloatIdx *arr, int n, int i)
{
  // Make sure the parent node arr[i] is greater than its children in
  // the heap
  for (;;)
  {
    // l & r are the two children node
    int l       = 2 * i + 1;
    int r       = 2 * i + 2;
    int largest = i;
    if (l < n && arr[l].val > arr[largest].val)
    {
      largest = l;
    }
    if (r < n && arr[r].val > arr[largest].val)
    {
      largest = r;
    }
    if (largest == i) break;
    std::swap(arr[i], arr[largest]);
    i = largest;
  }
}

/* */
static void
build_heap(FloatIdx *arr, int n)
{
  for (int i = n / 2 - 1; i >= 0; i--)
  {
    sift_down(arr, n, i);
  }
}

/* */
static void
apply_topp(
  floatx   *logits,
  std::span<floatx> fpbuf,
  std::span<FloatIdx> logit_indices,
  int       vocab_size,
  int       k,
  float     p
)
{
  if (k > vocab_size)
  {
    k = vocab_size;
  }
  // Softmax to get the probs, store in fpbuf
  softmax(fpbuf.data(), logits, vocab_size);
  int heap_size = (k == 0) ? vocab_size : k;

  if (k == 0)
  {
    int i;
    #pragma omp parallel for private(i) OMP_PARA_ARGS
    for (i = 0; i < vocab_size; i++)
    {
      logit_indices[i].idx = i;
      logit_indices[i].val = fpbuf[i];
    }
  }
  else
  {
    for (int i = 0; i < k; i++)
    {
      int idx = logit_indices[i].idx;  // Reuse the candidates from topk
      logit_indices[i].val = fpbuf[idx];
    }
  }
  build_heap(logit_indices.data(), heap_size);  // O(k)

  // fpbuf is now a copy of the original logits
  memcpy(fpbuf.data(), logits, vocab_size * sizeof(floatx));

  // Set logits to -inf
  int i;
  #pragma omp parallel for private(i) OMP_PARA_ARGS
  for (i = 0; i < vocab_size; i++)
  {
    logits[i] = -FLOATX_MAX;
  }

  float cum = 0.0f;  // Cumulative prob

  while (heap_size > 0)
  {
    // Pop the current max prob
    FloatIdx top     = logit_indices[0];
    logit_indices[0] = logit_indices[--heap_size];  // Put the last element to
                                                    // the top
    sift_down(logit_indices.data(), heap_size, 0);         // O(log(vocab_size))

    logits[top.idx] = fpbuf[top.idx];
    cum += (float)top.val;
    if (cum >= p) break;
  }
}

/* Simple repetition penalty */
void
apply_rpen(floatx *logits, std::span<const uint8_t> visited,  float rpen)
{
  // rpen short for Repetition Penalty
  size_t i;
  #pragma omp parallel for private(i) OMP_PARA_ARGS
  for (i = 0; i < visited.size(); i++)
  {
    if (!visited[i]) continue;
    float val = (float)logits[i];
    if (val > 0.0f)
    {
      logits[i] = (floatx)(val / rpen);
    }
    else
    {
      logits[i] = (floatx)(val * rpen);
    }
  }
}

/* */
typedef enum
{
  INJECT_NONE,
  INJECT_TEXT,
  INJECT_IMAG,
  INJECT_QUIT,
  INJECT_DONE,
} InjectDataType;

/* */
typedef struct
{
  InjectDataType type;
  union
  {
    struct
    {
      int *tokens;
      int  n_tokens;
    };
    floatx *image;
  };
} InjectData;

/* Sample the next token from the logits */
int
sample_from_logits(
  floatx   *logits,
  std::span<floatx> probs,
  std::span<FloatIdx> logit_indices,
  std::span<const uint8_t> visited,
  int       vocab_size,
  float     temperature,
  int       topk,
  float     topp,
  float     rpen
)
{
  bool dosample = temperature != 0 && topk != 1;

  // Manipulate the logits & sample the next token
  if (!dosample)
  {
    // Argmax sampling
    return argmax(logits, vocab_size);
  }

  bool use_topk = dosample && topk != 0;
  bool use_topp = dosample && topp < 1.0f;
  bool use_rpen = dosample && rpen > 1.0f;

  // Apply the temperature
  int d;
  #pragma omp parallel for private(d) OMP_PARA_ARGS
  for (d = 0; d < vocab_size; d++)
  {
    logits[d] /= (floatx)temperature;
  }

  if (use_topk)
  {
    apply_topk(logits, logit_indices, vocab_size, topk);
  }
  if (use_topp)
  {
    apply_topp(logits, probs, logit_indices, vocab_size, topk, topp);
  }
  if (use_rpen)
  {
    apply_rpen(logits, visited, rpen);
  }

  // Softmax to get the probs
  softmax(probs.data(), logits, vocab_size);

  // Multinomial sample from probs
  float r   = (float)((float)rand() / (RAND_MAX + 1.0));
  float sum = 0.0f;

  int token = vocab_size - 1;
  for (int d = 0; d < vocab_size; d++)
  {
    sum += (float)probs[d];
    if (r < sum)
    {
      token = d;
      break;
    }
  }
  return token;
}

#define DEFAULT_SEQLEN      16384
#define DEFAULT_TOPK        0
#define DEFAULT_CHUNK_SIZE  1024
#define DEFAULT_TEMPERATURE 1.0
#define DEFAULT_TOPP        1.0
#define DEFAULT_RPEN        1.0
#define DEFAULT_PROMPT      "Once upon a time"

// clang-format off
struct Options {
  [[= cli::positional{}]]
  std::string modelfile;

  [[= cli::long_name("seqlen")]]
  [[= cli::short_name('l')]]
  int seqlen = DEFAULT_SEQLEN;

  [[= cli::long_name("topk")]]
  [[= cli::short_name('k')]]
  int topk = DEFAULT_TOPK;

  [[= cli::long_name("chunk")]]
  [[= cli::short_name('c')]]
  int chunk_size  = DEFAULT_CHUNK_SIZE;

  [[= cli::long_name("seed")]]
  unsigned int seed = (unsigned int)time(NULL);

  [[= cli::long_name("topp")]]
  float topp = DEFAULT_TOPP;

  [[= cli::long_name("rpen")]]
  float rpen = DEFAULT_RPEN;

  [[= cli::long_name("temperature")]]
  [[= cli::short_name('t')]]
  float temperature = DEFAULT_TEMPERATURE;

  [[= cli::long_name("chat")]]
  bool chatmode = false;

  [[= cli::long_name("text-only")]]
  [[= cli::negate{}]]
  bool enable_mm = true;

  [[= cli::long_name("no-mmap")]]
  [[= cli::negate{}]]
  bool use_mmap = true;

  [[= cli::long_name("verbose")]]
  bool verbose = false;

  [[= cli::long_name("prompt")]]
  [[= cli::short_name('i')]]
  std::string prompt = DEFAULT_PROMPT;

  [[= cli::long_name("help")]]
  [[= cli::short_name('?')]]
  bool help = false;
};
// clang-format on

/* The main sampling loop */
std::tuple<double, int, double, int>
sample(
  // Input parameters
  GemmaModel   *model,
  TextBuffer   *tbuf,
  VisionBuffer *vbuf,
  const Options &options,
  std::function_ref<InjectData(int, GemmaModel*, bool)> inject_callback
)
{
  TextConfig *tcfg       = &model->decoder->config;
  int         vocab_size = tcfg->vocab_size;

  double    prf_t0        = 0.0;
  double    prf_t1        = 0.0;
  double    gen_t0        = 0.0;
  double    gen_t1        = 0.0;
  int       pos           = 0;
  int       token         = 0;
  std::vector<uint8_t>    visited(vocab_size, 0);
  std::vector<floatx>     probs(vocab_size, 0.0);;
  std::vector<FloatIdx> logit_indices(vocab_size);
  int       bos_tok       = 0;
  InjectData injected    = {};

  std::tuple<double, int, double, int> stats{};
  auto &[prefill_elapsed, prefill_toks, gen_elapsed, gen_toks] = stats;

  // `<bos>` is always the very first token of the sequence
  prf_t0      = now_sec();
  bos_tok = model->tokenizer->bos;
  if (forward_gemma_prefill(
        model, tbuf, &bos_tok, 1, &pos, options.chunk_size, visited, false
      ) == 1)
  {
    return stats;
  }
  prf_t1 = now_sec();
  prefill_elapsed += prf_t1 - prf_t0;
  prefill_toks = 1;

  injected = inject_callback(EOF, model, options.enable_mm);

  while (pos < options.seqlen && injected.type != INJECT_QUIT)
  {
    if (injected.type == INJECT_NONE)
    {
      // pure auto-regressive step
      gen_t0 = now_sec();
      if (forward_gemma_decode(model, tbuf, token, pos) == 1) return stats;
      pos++;

      if (is_interrupted()) break;
      // Fall through to common sample
    }
    else if (injected.type == INJECT_DONE)
    {
      break;
    }
    else
    {
      // Data injection (text / image)
      InjectData next    = inject_callback(EOF, model, options.enable_mm);
      bool       is_last = (next.type == INJECT_DONE);

      int prev_pos = pos;
      prf_t0       = now_sec();

      if (injected.type == INJECT_TEXT)
      {
        if (forward_gemma_prefill(
              model, tbuf, injected.tokens, injected.n_tokens, &pos, options.chunk_size,
              visited, is_last
            ) == 1)
        {
          return stats;
        }
      }
      else if (injected.type == INJECT_IMAG)
      {
        if (forward_gemma_image(
              model, tbuf, vbuf, injected.image, &pos, is_last
            ) == 1)
        {
          return stats;
        }
      }

      prf_t1 = now_sec();
      prefill_elapsed += prf_t1 - prf_t0;
      prefill_toks += pos - prev_pos;

      if (is_interrupted()) break;

      if (!is_last)
      {
        injected = next;
        continue;
      }
      // is_last: fall through to common sample
      gen_t0 = now_sec();
    }

    // Common sample path (both Case A and last-injection)
    token = sample_from_logits(
      tbuf->logits.data(), probs, logit_indices, visited, vocab_size, options.temperature,
      options.topk, options.topp, options.rpen
    );
    visited[token] = 1;
    gen_toks++;

    gen_t1 = now_sec();
    gen_elapsed += gen_t1 - gen_t0;

    injected = inject_callback(token, model, options.enable_mm);
  }
  return stats;
}

/* */
typedef struct
{
  int         state;
  std::vector<floatx> img;
  const char *arg;
  int         arg_len;

  // Used in pan & scan
  char    path[4096];
  std::vector<floatx> crops;
  int     n_crops;
  int     crop_i;
} CommandContext;

/* */
typedef struct InjectContext InjectContext;

/* */
typedef enum
{
  COMMAND_IMAGE = 0,
  COMMAND_LONGIMAGE,
} CommandId;

typedef struct
{
  const char *name;
  CommandId id;
  bool (*should_ignore)(GemmaModel *model, bool enable_mm);
  InjectData (*inject_next)(
    GemmaModel *model, InjectContext *ctx, bool enable_mm
  );
} CommandType;

/* */
typedef struct
{
  CommandType type;
  std::string_view raw;
  std::string_view arg;
} CommandRecord;

/* */
struct ScanResult
{
  std::variant<std::monostate, std::string_view, CommandRecord> value;
  std::string_view remaining;
};

/* */
struct InjectContext
{
  int            state;
  std::string_view text;
  ScanResult     event;
  CommandContext cmd_ctx;
  int           *tokens_buf;
  int            tokens_len;
  int            tokens_cap;
};

/* */
typedef struct
{
  int            state;
  char          *line_buf;
  int            line_cap;
  InjectContext *jctx;
} ChatContext;

/* */
bool
image_should_ignore(GemmaModel *model, bool enable_mm)
{
  (void)model;
  return !(model->support_mm && enable_mm);
}

/* */
InjectData
image_inject_next(GemmaModel *model, InjectContext *ctx, bool enable_mm)
{
  /* Template:
   * \n\n<start_of_image>[image soft tokens]<end_of_image>\n\n */

  VisionEncoder  *enc     = model->encoder.get();
  GemmaTokenizer *tok     = model->tokenizer.get();
  CommandContext *cmd_ctx = &ctx->cmd_ctx;

  int  spos, epos;
  bool use_mm = model->support_mm && enable_mm;
  int image_size = (enc && use_mm) ? enc->config.image_size : 0;
  char path[4096] = {0};

  /* A classic Duff-style hack that fakes a generator/coroutine using an
   * explicit state machine, each call resumes at the `case` matching the
   * current state, does one step, and returns an `InjectData` value as a sort
   * of pseudo-`yield`. The caller loops until it receives INJECT_DONE (or
   * INJECT_QUIT on error). The `state` field is effectively a saved program
   * counter that lets the sequence survive across multiple invocations. */

  switch (cmd_ctx->state)
  {
    case 0:
      // Inject header ("\n\n<start_of_image>")
      spos = ctx->tokens_len;
      encode(tok, "\n\n", ctx->tokens_buf, spos, &ctx->tokens_len);
      ctx->tokens_buf[ctx->tokens_len++] = tok->soi;
      epos                               = ctx->tokens_len;

      // The following structure can be treated as a `yield`:

      // ----------------------------------------------------
      cmd_ctx->state = 1;
      return (InjectData){
        .type     = INJECT_TEXT,
        .tokens   = ctx->tokens_buf + spos,
        .n_tokens = epos - spos,
      };

    case 1:
      // ----------------------------------------------------

      // Inject image
      ;  // "label followed by a declaration is a C23 extension" :P
      if ((size_t)cmd_ctx->arg_len >= sizeof(path))
      {
        log_error("image path too long");
        cmd_ctx->state = GENERATOR_EXIT;
        return (InjectData){.type = INJECT_QUIT};
      }
      memcpy(path, cmd_ctx->arg, cmd_ctx->arg_len);
      cmd_ctx->img = prepare_image(path, image_size);
      if (cmd_ctx->img.empty())
      {
        log_error("failed to prepare image: '%s'", path);
        cmd_ctx->state = GENERATOR_EXIT;
        return (InjectData){.type = INJECT_QUIT};
      }

      cmd_ctx->state = 2;
      return (InjectData){.type = INJECT_IMAG, .image = cmd_ctx->img.data()};

    case 2:
      // Inject trailer ("<end_of_image>\n\n")
      spos                               = ctx->tokens_len;
      ctx->tokens_buf[ctx->tokens_len++] = tok->eoi;
      encode(
        tok, "\n\n", ctx->tokens_buf, ctx->tokens_len, &ctx->tokens_len
      );
      epos = ctx->tokens_len;

      cmd_ctx->state = GENERATOR_EXIT;
      return (InjectData){
        .type     = INJECT_TEXT,
        .tokens   = ctx->tokens_buf + spos,
        .n_tokens = epos - spos,
      };

    default:
      return (InjectData){.type = INJECT_DONE};
  }
}

/* */
bool
longimage_should_ignore(GemmaModel *model, bool enable_mm)
{
  (void)model;
  return !(model->support_mm && enable_mm);
}

/* */
InjectData
longimage_inject_next(GemmaModel *model, InjectContext *ctx, bool enable_mm)
{
  // A funny workaround for long images, crop the full image and send the crops
  // to the model

  /* Template:
   * here is the original image
   * \n\n<start_of_image>
   * [full image soft tokens]
   * <end_of_image>\n\n
   * and here are some crops to help you see better
   * \n\n<start_of_image>
   * [crop 1 soft tokens]
   * <end_of_image>\n\n
   * \n\n<start_of_image>
   * [crop 2 soft tokens]
   * <end_of_image>\n\n
   * ...
   */

  VisionEncoder  *enc     = model->encoder.get();
  GemmaTokenizer *tok     = model->tokenizer.get();
  CommandContext *cmd_ctx = &ctx->cmd_ctx;

  int  spos, epos;
  bool use_mm     = model->support_mm && enable_mm;
  int  image_size = (enc && use_mm) ? enc->config.image_size : 0;

  switch (cmd_ctx->state)
  {
    case 0:
      // Inject prefix prompt
      spos = ctx->tokens_len;
      encode(
        tok, CROPPED_IMAGE_PREFIX,
        ctx->tokens_buf, spos, &ctx->tokens_len
      );
      // Inject image header ("\n\n<start_of_image>")
      encode(
        tok, "\n\n", ctx->tokens_buf, ctx->tokens_len, &ctx->tokens_len
      );
      ctx->tokens_buf[ctx->tokens_len++] = tok->soi;
      epos                               = ctx->tokens_len;

      cmd_ctx->state = 1;
      return (InjectData){
        .type     = INJECT_TEXT,
        .tokens   = ctx->tokens_buf + spos,
        .n_tokens = epos - spos,
      };

    case 1:
      if ((size_t)cmd_ctx->arg_len >= sizeof(cmd_ctx->path))
      {
        log_error("image path too long");
        cmd_ctx->state = GENERATOR_EXIT;
        return (InjectData){.type = INJECT_QUIT};
      }
      memcpy(cmd_ctx->path, cmd_ctx->arg, cmd_ctx->arg_len);
      cmd_ctx->path[cmd_ctx->arg_len] = '\0';

      // Save the image to cmd_ctx->crops.data()
      cmd_ctx->crops = prepare_longimage(
        cmd_ctx->path, image_size, MIN_CROP_SIZE, MAX_NUM_CROPS,
        &cmd_ctx->n_crops
      );
      if (cmd_ctx->crops.empty())
      {
        log_error("failed to prepare image: '%s'", cmd_ctx->path);
        cmd_ctx->state = GENERATOR_EXIT;
        return (InjectData){.type = INJECT_QUIT};
      }

      cmd_ctx->state = 2;

      /* Inject the full image.
       * Passing in cmd_ctx->crops.data() is safe here since `sample()` will only take
       * the first `image_size * image_size * 3` elements. */
      return (InjectData){.type = INJECT_IMAG, .image = cmd_ctx->crops.data()};

    case 2:
      // Trailer of the full image
      spos                               = ctx->tokens_len;
      ctx->tokens_buf[ctx->tokens_len++] = tok->eoi;
      encode(
        tok, "\n\n", ctx->tokens_buf, ctx->tokens_len, &ctx->tokens_len
      );

      if (cmd_ctx->n_crops <= 1)
      {
        epos           = ctx->tokens_len;
        cmd_ctx->state = GENERATOR_EXIT;  // Finish off
        return (InjectData){
          .type     = INJECT_TEXT,
          .tokens   = ctx->tokens_buf + spos,
          .n_tokens = epos - spos,
        };
      }
      // Inject crop prompt ("and here are some crops to help you see better")
      encode(
        tok, CROPPED_IMAGE_FILTER,
        ctx->tokens_buf, ctx->tokens_len, &ctx->tokens_len
      );
      // Image header of the first crop
      encode(
        tok, "\n\n", ctx->tokens_buf, ctx->tokens_len, &ctx->tokens_len
      );
      ctx->tokens_buf[ctx->tokens_len++] = tok->soi;
      epos                               = ctx->tokens_len;

      cmd_ctx->state = 3;
      return (InjectData){
        .type     = INJECT_TEXT,
        .tokens   = ctx->tokens_buf + spos,
        .n_tokens = epos - spos,
      };

    case 3:
      // Inject all the crops
      for (cmd_ctx->crop_i = 1; cmd_ctx->crop_i < cmd_ctx->n_crops;
        cmd_ctx->crop_i++)
      {
        if (cmd_ctx->crop_i != 1)
        {
          // Inject crop header
          spos = ctx->tokens_len;
          encode(tok, "\n\n", ctx->tokens_buf, spos, &ctx->tokens_len);
          ctx->tokens_buf[ctx->tokens_len++] = tok->soi;
          epos                               = ctx->tokens_len;

          cmd_ctx->state = 4;
          return (InjectData){
            .type     = INJECT_TEXT,
            .tokens   = ctx->tokens_buf + spos,
            .n_tokens = epos - spos,
          };
        }

        case 4:
          // Inject image crop
          cmd_ctx->state = 5;
          return (InjectData){
            .type  = INJECT_IMAG,
            .image = cmd_ctx->crops.data() +
                     (size_t)cmd_ctx->crop_i * image_size * image_size * 3,
          };

        case 5:
          // Inject crop trailer
          spos                               = ctx->tokens_len;
          ctx->tokens_buf[ctx->tokens_len++] = tok->eoi;
          encode(
            tok, "\n\n", ctx->tokens_buf, ctx->tokens_len, &ctx->tokens_len
          );
          epos = ctx->tokens_len;

          cmd_ctx->state = 6;
          return (InjectData){
            .type     = INJECT_TEXT,
            .tokens   = ctx->tokens_buf + spos,
            .n_tokens = epos - spos,
          };

        case 6:;
      }

      [[fallthrough]]
    default:
      return (InjectData){.type = INJECT_DONE};
  }
}

/* */
CommandType command_types[] = {
  {
    .name          = "image",
    .id            = COMMAND_IMAGE,
    .should_ignore = image_should_ignore,
    .inject_next   = image_inject_next,
  },
  {
    .name          = "longimage",
    .id            = COMMAND_LONGIMAGE,
    .should_ignore = longimage_should_ignore,
    .inject_next   = longimage_inject_next,
  },
};

/* */
ScanResult
scan_next_event(std::string_view text)
{
  if (text.empty())
  {
    return ScanResult{};
  }

  CommandRecord first_cmd = {};
  size_t first_pos = std::string_view::npos;

  for (size_t i = 0; i < ARRAYLEN(command_types); i++)
  {
    CommandType cmd_type = command_types[i];

    std::string pattern = "@" + std::string(cmd_type.name) + "{";
    size_t pos = text.find(pattern);

    if (pos != std::string_view::npos && pos < first_pos)
    {
      // Look for closing '}'
      size_t closing = text.find('}', pos + pattern.size());
      if (closing != std::string_view::npos)
      {
        size_t raw_len = closing + 1 - pos;
        size_t arg_off = pattern.size();
        size_t arg_len = raw_len - arg_off - 1;

        first_pos = pos;
        first_cmd = CommandRecord{
          .type    = cmd_type,
          .raw     = text.substr(pos, raw_len),
          .arg     = text.substr(pos + arg_off, arg_len),
        };
      }
    }
  }

  if (first_pos == std::string_view::npos)
  {
    // No command found, return the whole text
    return ScanResult{
      .value     = text,
      .remaining = text.substr(text.size()),
    };
  }
  if (first_pos > 0)
  {
    // Text before command
    return ScanResult{
      .value     = text.substr(0, first_pos),
      .remaining = text.substr(first_pos),
    };
  }
  // Starts with command
  return ScanResult{
    .value     = first_cmd,
    .remaining = text.substr(first_pos + first_cmd.raw.size()),
  };
}

/* */
InjectData
inject_next(GemmaModel *model, InjectContext *ctx, bool enable_mm)
{
  GemmaTokenizer *tok = model->tokenizer.get();

  int spos, epos;
  std::string_view raw;
  InjectData r = {};

  switch (ctx->state)
  {
    // Main loop: consume text from ctx->text until exhausted
    while (!ctx->text.empty())
    {
      case 0:
        // Scan for the next event (text / command / done)
        ctx->event = scan_next_event(ctx->text);

        if (std::holds_alternative<std::monostate>(ctx->event.value)) break;

        if (std::holds_alternative<std::string_view>(ctx->event.value))
        {
          // Plain text: tokenize it and emit as INJECT_TEXT
          spos = ctx->tokens_len;
          encode(
            tok, std::get<std::string_view>(ctx->event.value), ctx->tokens_buf,
            spos, &ctx->tokens_len
          );
          epos = ctx->tokens_len;

          ctx->state = 1;
          return (InjectData){
            .type     = INJECT_TEXT,
            .tokens   = ctx->tokens_buf + spos,
            .n_tokens = epos - spos,
          };

          case 1:
            // Advance past the consumed text
            ctx->text = ctx->event.remaining;
            ctx->state = 0;
            continue;
        }

        // Otherwise it's a command event
        raw = std::get<CommandRecord>(ctx->event.value).raw;

        // If this command should be ignored for the current mode, just
        // tokenize its raw text and emit it verbatim
        if (std::get<CommandRecord>(ctx->event.value).type.should_ignore(model, enable_mm))
        {
          spos = ctx->tokens_len;
          encode(
            tok, raw, ctx->tokens_buf, spos,
            &ctx->tokens_len
          );
          epos = ctx->tokens_len;

          ctx->state = 2;
          return (InjectData){
            .type     = INJECT_TEXT,
            .tokens   = ctx->tokens_buf + spos,
            .n_tokens = epos - spos,
          };

          case 2:
            ctx->text = ctx->event.remaining;
            continue;
        }

        // Otherwise, delegate to the command's own inject handler
        ctx->cmd_ctx = (CommandContext){
          .state   = 0,
          .arg     = std::get<CommandRecord>(ctx->event.value).arg.data(),
          .arg_len = static_cast<int>(std::get<CommandRecord>(ctx->event.value).arg.size()),
        };

        for (;;)
        {
          // Drive the sub-injector until it's done, yielding any intermediate
          // results up to the caller
          r = std::get<CommandRecord>(ctx->event.value).type.inject_next(model, ctx, enable_mm);
          if (r.type != INJECT_DONE)
          {
            ctx->state = 3;
            return r;
            case 3:;
          }
          else
          {
            ctx->state = GENERATOR_EXIT;  // done
            break;
          }
        }

        // Advance past the command in the input text
        ctx->text = ctx->event.remaining;
    }

#if !defined(__MSVC__)
    __attribute__((fallthrough));
#endif

    default:
      return (InjectData){.type = INJECT_DONE};
  }
}

/* */
std::tuple<double, int, double, int>
generate(
  // Input parameters
  GemmaModel   *model,
  TextBuffer   *tbuf,
  VisionBuffer *vbuf,
  const Options& options
)
{
  fputs(options.prompt.c_str(), stdout);

  int  tokens_cap = options.seqlen * model->tokenizer->max_toklen;
  std::vector<int> tokens_buf(tokens_cap);

  InjectContext ctx = {
    .state      = 0,
    .text       = options.prompt,
    .tokens_buf = tokens_buf.data(),
    .tokens_len = 0,
    .tokens_cap = tokens_cap,
  };

  auto generate_inject = [&](int token, GemmaModel *model, bool enable_mm) -> InjectData {
    GemmaTokenizer *tok = model->tokenizer.get();
    if (token == EOF)
    {
      // Request for prefilling
      return inject_next(model, &ctx, enable_mm);
    }
    if (token == tok->eos || token == tok->eot)
    {
      return (InjectData){.type = INJECT_QUIT};
    }

    // Prints the token to stdout
    char byte_buf[2];
    fputs(decode(tok, token, byte_buf), stdout);
    fflush(stdout);

    return (InjectData){.type = INJECT_NONE};
  };

  return sample(
    model, tbuf, vbuf, options, 
    generate_inject
  );
}

/* Build the Gemma chat template */
static InjectData
new_turn(GemmaModel *model, bool enable_mm, ChatContext *ctx)
{
  /* Template (from https://ai.google.dev/gemma/docs/core/prompt-structure):
   * <start_of_turn>user\n
   * What is Cramer's Rule?<end_of_turn>\n
   * <start_of_turn>model */

  GemmaTokenizer *tok  = model->tokenizer.get();
  InjectContext  *jctx = ctx->jctx;

  int        spos, epos;
  InjectData r;
  auto quit = [ctx]() {
    ctx->state = GENERATOR_EXIT;
    return InjectData{.type = INJECT_QUIT};
  };

  switch (ctx->state)
  {
    case 0:
      // Get a fresh input from stdin
      std::print("\n> ");

      if (fgets(ctx->line_buf, ctx->line_cap, stdin) == NULL)
      {
        if (!is_interrupted())
        {
#if defined(_WIN32)
          // Wait 10ms for the console handler to set the flag
          Sleep(10);
#endif
        }
        if (!is_interrupted())
        {
          log_error("failed to read user input");
        }
        return quit();
      }
      if (strchr(ctx->line_buf, '\n') == NULL)
      {
        // Input is truncated (too long)
        log_error("input too long");
        return quit();
      }
      if (is_interrupted()) return quit();

      // Set string end
      ctx->line_buf[strcspn(ctx->line_buf, "\n")] = '\0';

      spos                                 = jctx->tokens_len;
      jctx->tokens_buf[jctx->tokens_len++] = tok->sot;  // <start_of_turn>
      encode(
        tok, "user\n", jctx->tokens_buf, spos,
        &jctx->tokens_len
      );
      epos = jctx->tokens_len;

      ctx->state = 1;
      return (InjectData){
        .type     = INJECT_TEXT,
        .tokens   = jctx->tokens_buf + spos,
        .n_tokens = epos - spos,
      };

    case 1:
      jctx->text     = ctx->line_buf;
      jctx->state    = 0;

      for (;;)
      {
        r = inject_next(model, jctx, enable_mm);
        if (r.type != INJECT_DONE)
        {
          ctx->state = 2;
          return r;

          case 2:;
        }
        else
        {
          break;
        }
      }

      spos = jctx->tokens_len;

      // No more command ahead, encode whatever plain text remains and wrap up
      // the turn with the chat template's closing tokens
      if (!jctx->text.empty())
      {
        encode(
          tok, jctx->text, jctx->tokens_buf, spos,
          &jctx->tokens_len
        );
      }

      jctx->tokens_buf[jctx->tokens_len++] = tok->eot;
      jctx->tokens_buf[jctx->tokens_len++] = get_token_idx(tok, "\n");
      jctx->tokens_buf[jctx->tokens_len++] = tok->sot;

      encode(
        tok, "model\n", jctx->tokens_buf, jctx->tokens_len,
        &jctx->tokens_len
      );

      epos = jctx->tokens_len;

      ctx->state = GENERATOR_EXIT;
      return (InjectData){
        .type     = INJECT_TEXT,
        .tokens   = jctx->tokens_buf + spos,
        .n_tokens = epos - spos,
      };

    default:
      return (InjectData){.type = INJECT_DONE};

  }
}

/* */
std::tuple<double, int, double, int>
chat(
  // Input parameters
  GemmaModel   *model,
  TextBuffer   *tbuf,
  VisionBuffer *vbuf,
  const Options& options
)
{

  int max_toklen = model->tokenizer->max_toklen;
  int tokens_cap = options.seqlen * max_toklen;
  int line_cap   = options.seqlen * max_toklen;
  InjectContext jctx = {};
  ChatContext ctx = {};

  std::vector<int> tokens_buf(tokens_cap);
  std::vector<char> line_buf(line_cap);

  jctx = (InjectContext){
    .tokens_buf = tokens_buf.data(),
    .tokens_len = 0,
    .tokens_cap = tokens_cap,
  };
  ctx = (ChatContext){
    .state    = 0,
    .line_buf = line_buf.data(),
    .line_cap = line_cap,
    .jctx     = &jctx,
  };

  auto chat_inject = [&](int token, GemmaModel *model, bool enable_mm) -> InjectData
  {
    GemmaTokenizer *tok = model->tokenizer.get();

    if (token == tok->eos || token == tok->eot)
    {
      // Model completed the turn
      ctx.state = 0;
      return new_turn(model, enable_mm, &ctx);
    }
    if (token == EOF)
    {
      // Requesting more injection data
      return new_turn(model, enable_mm, &ctx);
    }

    char byte_buf[2];
    fputs(decode(tok, token, byte_buf), stdout);
    fflush(stdout);
    return (InjectData){.type = INJECT_NONE};
  };

  return sample(
    model, tbuf, vbuf, options,
    chat_inject
  );
}

/* */
int
setup_gemma_model(
  const char *modelfile, const Options &options,
  std::unique_ptr<GemmaModel> *model, std::unique_ptr<TextBuffer> *tbuf, std::unique_ptr<VisionBuffer> *vbuf
)
{
  *model = nullptr;
  std::unique_ptr<GemmaModel> loaded(
    options.use_mmap ? mmap_gemma_model(modelfile, options.enable_mm)
             : read_gemma_model(modelfile, options.enable_mm)
  );
  if (!loaded) return 1;

  TextConfig *tcfg = &loaded->decoder->config;
  VisionConfig *vcfg = loaded->encoder ? &loaded->encoder->config : nullptr;
  bool use_mm = options.enable_mm && loaded->support_mm;
  std::unique_ptr<TextBuffer> text(
    malloc_text_buffer(tcfg, vcfg, options.seqlen, options.chunk_size, use_mm, loaded->quant)
  );
  if (!text) return 1;
  std::unique_ptr<VisionBuffer> vision;
  if (use_mm)
  {
    vision.reset(malloc_vision_buffer(vcfg, loaded->quant));
    if (!vision) return 1;
  }

  *model = std::move(loaded);
  *tbuf = std::move(text);
  *vbuf = std::move(vision);
  return 0;
}

template <typename F>
class ScopeExit
{
  F cleanup;
public:
  explicit ScopeExit(F fn) : cleanup(std::move(fn)) {}
  ScopeExit(const ScopeExit &) = delete;
  ScopeExit &operator=(const ScopeExit &) = delete;
  ~ScopeExit() { cleanup(); }
};

// CLI

template <typename T>
void print(const T& object, const int width)
{
  template for (constexpr auto member :
    std::define_static_array(std::meta::nonstatic_data_members_of(
    ^^T, std::meta::access_context::unchecked()))) {
      if constexpr (!cli::has_annotation(member, no_print)) {
        using M = std::remove_cvref_t<
          decltype(object.[:member:])
        >;

        if constexpr (std::formattable<M, char>) {
          std::println(
	    "  {:<{}}: {}",
	    std::meta::identifier_of(member),
	    width,
	    object.[:member:]
	  );
	}
      }
  }
}

/* Pretty-print of the loaded model */
void
print_model_config(
  GemmaModel *model, int width, int seqlen, int chunk_size, bool enable_mm
)
{
  bool            use_mm = model->support_mm && enable_mm;
  VisionEncoder  *enc    = model->encoder.get();
  TextDecoder    *dec    = model->decoder.get();
  GemmaTokenizer *tok    = model->tokenizer.get();
  TextConfig     *tcfg   = &dec->config;
  VisionConfig   *vcfg   = NULL;
  if (use_mm)
  {
    vcfg = &enc->config;
  }

  std::println("\n========== model configuration ==========");
  std::println("  {:<{}}: {}", "quant", width, model->quant ? "true" : "false");
  putchar('\n');
  // clang-format off
  
  // Vision encoder architecture (if available and enabled)
  if (use_mm)
  {
    std::println("encoder:");
    print(*vcfg, width);
  }

  std::println("decoder:");
  print(*tcfg, width);

  // Tokenizer
  std::println("\ntokenizer:");
  print(*tok, width);
  // clang-format on

  // Parameters
  std::println("\nparameters:");

  size_t enc_params, dec_params = 0;
  if (use_mm)
  {
    enc_params = get_vision_encoder_size(vcfg, tcfg, false) / sizeof(floatx);
    std::println("  {:<{}}: {}", "encoder params", width, enc_params);
  }
  dec_params = get_text_decoder_size(tcfg, false) / sizeof(floatx);
  std::println("  {:<{}}: {}", "decoder params", width, dec_params);

  // Memory footprint
  std::println("\nmemory footprint (estimated):");
  double GB = 1024.0 * 1024.0;

  // Vision buffer & weights
  if (use_mm)
  {
    double enc_GB   = get_vision_buffer_size(vcfg, model->quant) / GB;
    double enc_w_GB = get_vision_encoder_size(vcfg, tcfg, model->quant) / GB;
    std::println("  {:<{}}: {:.2f} GB", "encoder buffer", width, enc_GB);
    std::println("  {:<{}}: {:.2f} GB", "encoder weights", width, enc_w_GB);
  }

  // Text buffer & weights
  double dec_GB =
    get_text_buffer_size(tcfg, vcfg, seqlen, chunk_size, use_mm, model->quant) /
    GB;
  double dec_w_GB = get_text_decoder_size(tcfg, model->quant) / GB;
  std::println("  {:<{}}: {:.2f} GB", "decoder buffer", width, dec_GB);
  std::println("  {:<{}}: {:.2f} GB", "decoder weights", width, dec_w_GB);

  // KV cache per tok
  double kvc_t_KB = tcfg->n_layers * 2 * tcfg->n_kv_heads * tcfg->head_dim *
                    sizeof(floatx) / 1024.0;
  std::println("  {:<{}}: {:.2f} KB", "kv cache (tok)", width, kvc_t_KB);
  std::println("=========================================\n");
}

/* */
void
print_usage(void)
{
  // clang-format off
  std::print(
    "usage:\n"
    "  ./gemma <modelfile> [options]\n"
    "\n"
    "arguments:\n"
    "  modelfile                  path to the model file\n"
    "\n"
    "options:\n"
    "  --seqlen <n_patches>       set sequence length"
    " (default: " TOSTRING(DEFAULT_SEQLEN) ")\n"
    "  --topk <n_patches>         set top-k sampling value"
    " (default: " TOSTRING(DEFAULT_TOPK) ")\n"
    "  --seed <n_patches>         set random seed"
    " (default: current time)\n"
    "  --chunk <n_patches>        set prefilling chunk size, must be >= 1"
    " (default: " TOSTRING(DEFAULT_CHUNK_SIZE) ")\n"
    "  --temperature <F>          set temperature value, must be >= 0.0"
    " (default: " TOSTRING(DEFAULT_TEMPERATURE) ")\n"
    "  --topp <F>                 set top-p sampling value, must be 0.0 < p <= 1.0"
    " (default: " TOSTRING(DEFAULT_TOPP) ")\n"
    "  --rpen <F>                 set repetition penalty, must be >= 1.0"
    " (default: " TOSTRING(DEFAULT_RPEN) ")\n"
    "  --prompt <S>               set input prompt, ignored if chat mode is enabled"
    " (default: \"" DEFAULT_PROMPT "\")\n"
    "  --chat                     enable chat mode\n"
    "  --text-only                disable multimodal capability\n"
    "  --no-mmap                  disable mmap (memory mapped file)\n"
    "  --verbose                  print model info\n"
    "  --help, -?                 display this help message\n"
    "\n"
    "controls:\n"
    "  Ctrl+C                    gracefully interrupt generation and exit\n"
    "\n"
    "examples:\n"
    "  ./gemma model.bin -l 2048 -t 0.8 --chat\n"
    "  ./gemma model.bin -i \"Hello I'm a language model,\""
    " --seqlen 4096 --topk 50 --seed 12345\n"
  );
  // clang-format on
}

/* */
int
main(int argc, char **argv)
{
  set_utf8_console();
  setup_signal_handler();

  std::unique_ptr<GemmaModel> model;
  std::unique_ptr<TextBuffer> tbuf;
  std::unique_ptr<VisionBuffer> vbuf;
  Options options;

  // On Windows, get UTF-8 encoded command line arguments
  char **utf8_argv = get_utf8_argv(&argc);
  if (utf8_argv != NULL)
  {
    argv = utf8_argv;
  }

  ScopeExit arguments([&] { free_utf8_argv(utf8_argv, argc); });

  int result = [&]() -> int {
  try {
    options = cli::parse<Options>(
      std::span(argv, static_cast<std::size_t>(argc)).subspan(1)
    );
  } catch (const std::exception &error) {
    print_usage();
    log_error("%s", error.what());
    return 1;
  }

  if (options.help)
  {
    print_usage();
    return 0;
  }

  if (options.modelfile.empty())
  {
    print_usage();
    log_error("model filename is not provided");
    return 1;
  }
  if (options.seqlen <= 0)
  {
    log_error("invalid number for --seqlen: must be >= 1");
    return 1;
  }
  if (options.topk < 0)
  {
    log_error("invalid number for --topk: must be >= 0");
    return 1;
  }
  if (options.chunk_size <= 0)
  {
    log_error("invalid number for --chunk: must be >= 1");
    return 1;
  }
  if (options.temperature < 0.0f)
  {
    log_error("invalid temperature: must be >= 0");
    return 1;
  }
  if (options.topp <= 0.0f || options.topp > 1.0f)
  {
    log_error("invalid top-p: must be 0 < p <= 1");
    return 1;
  }
  if (options.rpen < 1.0f)
  {
    log_error("invalid repetition penalty: must be >= 1");
    return 1;
  }

  srand(options.seed);

  if (setup_gemma_model(
        options.modelfile.c_str(), options, &model, &tbuf, &vbuf
      ) != 0)
    return 1;

  if (options.verbose)
  {
    // Print config

    // clang-format off
    const int width = 20;
    print_model_config(model.get(), width, options.seqlen, options.chunk_size, options.enable_mm);

    // Print command line args
    std::println("\n========= inference arguments ===========");
    print(options, 20);
    std::println("=========================================\n");
    // clang-format on
  }

  auto [prefill_elapsed, prefill_toks, gen_elapsed, gen_toks] = options.chatmode
    ? chat(
        model.get(), tbuf.get(), vbuf.get(),
        options
      )
    : generate(
        model.get(), tbuf.get(), vbuf.get(),
        options
      );

  if (options.verbose)
  {
    // Print timing stats
    std::println("\n");
    if (prefill_elapsed > 0.0)
    {
      std::println(
        "prefilled {} tokens in {:.2f} seconds ({:.2f} tok/s)", prefill_toks,
        prefill_elapsed, prefill_toks / prefill_elapsed
      );
    }
    else
    {
      std::println("prefilled {} tokens instantly", prefill_toks);
    }
    if (gen_elapsed > 0.0)
    {
      std::println(
        "generated {} tokens in {:.2f} seconds ({:.2f} tok/s)", gen_toks,
        gen_elapsed, gen_toks / gen_elapsed
      );
    }
    else
    {
      std::println("generated {} tokens instantly", gen_toks);
    }
  }

  return 0;
  }();

  if (g_errmsg[0] != '\0')
  {
    std::println(stderr, "error: {}", get_error());
  }

  return result;
}

