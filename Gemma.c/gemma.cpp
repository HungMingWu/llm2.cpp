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
#include <array>
#include <compare>
#include <cstddef>
#include <functional>
#include <mdspan>
#include <memory>
#include <meta>
#include <new>
#include <optional>
#include <print>
#include <generator>
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


/*----------------------------------------------------------------------------*/
/* 02 config: runtime knobs, dtypes, ISA switches, tiny utils                 */
/*----------------------------------------------------------------------------*/

typedef struct {
  int qk_blk;
  int gemm_mc;
  int gemm_nc;
  int gemm_kc;
  const char *img_pre;
  const char *crop_pre;
  int min_crop;
  int max_crop;
} RuntimeConfig;

static RuntimeConfig g_rt = {
    .qk_blk = 64,   /* block size of blockwise causal masking */
    .gemm_mc = 128, /* rows of A resident per (jc, ic) tile block, mult of 8 */
    .gemm_nc = 256, /* cols of B packed per outer jc block, mult of 8 */
    .gemm_kc = 256, /* k-length streamed per inner pc chunk */
    /* prompt used in pan & scan
     * google/gemma_pytorch/blob/main/gemma/gemma3_preprocessor.py */
    .img_pre = "here is the original image",
    .crop_pre = "and here are some crops to help you see better",
    /* default parameters used in pan & scan
     * google/gemma_pytorch/blob/main/gemma/siglip_vision/pan_and_scan.py */
    .min_crop = 256,
    .max_crop = 4,
};

static int env_int(const char *name, int def) {
  const char *s = getenv(name);
  char *end;
  long v;

  if (s == NULL || *s == '\0')
    return def;
  v = strtol(s, &end, 10);
  if (end == s)
    return def;
  return (int)v;
}

static const char *env_str(const char *name, const char *def) {
  const char *s = getenv(name);
  return (s != NULL && *s != '\0') ? s : def;
}

/* block sizes have to stay multiples of 8 for the 8x8 GEMM tiles. */
static int env_block(const char *name, int def) {
  int v = env_int(name, def);
  if (v < 8)
    v = 8;
  return v / 8 * 8;
}

static void setup_runtime_config(void) {
  g_rt.qk_blk = env_block("QK_BLOCK_SIZE", g_rt.qk_blk);
  g_rt.gemm_mc = env_block("GEMM_MC", g_rt.gemm_mc);
  g_rt.gemm_nc = env_block("GEMM_NC", g_rt.gemm_nc);
  g_rt.gemm_kc = env_block("GEMM_KC", g_rt.gemm_kc);
  g_rt.img_pre = env_str("CROPPED_IMAGE_PREFIX", g_rt.img_pre);
  g_rt.crop_pre = env_str("CROPPED_IMAGE_FILTER", g_rt.crop_pre);
  g_rt.min_crop = env_int("MIN_CROP_SIZE", g_rt.min_crop);
  g_rt.max_crop = env_int("MAX_NUM_CROPS", g_rt.max_crop);
}

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
static std::optional<size_t>
get_strarr_bytes(FILE *fp, int count)
{
  // Read a sequence of pascal-style strings
  size_t offset = 0;
  long   pos    = ftell(fp);
  if (pos == -1L)
  {
    log_error("ftell failed");
    return std::nullopt;
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
  uint8_t nl;       /* ViT layers */
  uint8_t n_heads;  /* attention heads */
  uint16_t img_px;  /* pixels per image side */
  uint16_t psize;   /* patches per image side */
  uint16_t dim;     /* ViT dimension */
  uint16_t mlp_dim; /* intermediate size in FFN */
  float eps;        /* LayerNorm epsilon */
};

/* */
static VisionConfig *
read_vision_config(FILE *fp)
{
  auto vcfg = std::make_unique<VisionConfig>();

  if (!read_uint8_ckd(fp, &vcfg->nl) ||
      !read_uint8_ckd(fp, &vcfg->n_heads) ||
      !read_uint16_ckd(fp, &vcfg->mlp_dim) ||
      !read_uint16_ckd(fp, &vcfg->dim) ||
      !read_uint16_ckd(fp, &vcfg->img_px) ||
      !read_uint16_ckd(fp, &vcfg->psize) || !read_fp32_ckd(fp, &vcfg->eps))
    return nullptr;

  return vcfg.release();

}

/* Configuration for the main Gemma text encoder */
struct TextConfig
{
  uint8_t nl;                      /* transformer layers */
  uint8_t n_heads;                 /* attention heads */
  uint8_t nkv;                     /* key/value heads (GQA) */
  uint16_t hdim;                   /* dim per head */
  uint16_t dim;                    /* model dimension */
  uint16_t mlp_dim;                /* intermediate size in FFN */
  uint16_t q_scale;                /* scale applied to queries before attention */
  uint16_t slide;                  /* sliding-window size */
  uint16_t img_tok;                /* number of soft tokens per image */
  uint32_t max_seq;                /* max number of position embeddings */
  uint32_t nvocab;                 /* number of possible tokens */
  float theta_l;                   /* RoPE base for local (sliding) attention */
  float theta_g;                   /* RoPE base for full attention */
  float eps;                       /* RMSNorm epsilon */
  float sc_att;                    /* tanh softcap on attention scores (0 = off) */
  float sc_log;                    /* tanh softcap on final logits (0 = off) */
  std::vector<uint8_t> swa;        /* true = use sliding window for that layer */
  bool qk_norm;                    /* query/key RMSNorm */
  bool mlp_pre;
  bool mlp_pst;
};

/* */
static TextConfig *
read_text_config(FILE *fp, bool *support_mm)
{
  auto tcfg = std::make_unique<TextConfig>();
  std::vector<char> swa_buf;

  if (!read_uint8_ckd(fp, &tcfg->nl) ||
      !read_uint8_ckd(fp, &tcfg->n_heads) ||
      !read_uint8_ckd(fp, &tcfg->nkv) ||
      !read_uint16_ckd(fp, &tcfg->hdim) ||
      !read_uint16_ckd(fp, &tcfg->dim) ||
      !read_uint16_ckd(fp, &tcfg->mlp_dim) ||
      !read_uint16_ckd(fp, &tcfg->q_scale) ||
      !read_uint16_ckd(fp, &tcfg->slide) ||
      !read_uint16_ckd(fp, &tcfg->img_tok) ||
      !read_uint32_ckd(fp, &tcfg->max_seq) ||
      !read_uint32_ckd(fp, &tcfg->nvocab) ||
      !read_fp32_ckd(fp, &tcfg->theta_l) ||
      !read_fp32_ckd(fp, &tcfg->theta_g) ||
      !read_fp32_ckd(fp, &tcfg->eps) ||
      !read_fp32_ckd(fp, &tcfg->sc_att) ||
      !read_fp32_ckd(fp, &tcfg->sc_log))
    return nullptr;

  // Packed bit-field of which layers use sliding-window attention
  // A terrible terrible idea, wish I didn't do this
  uint8_t n_bytes;
  if (!read_uint8_ckd(fp, &n_bytes)) return nullptr;
  if (n_bytes * 8 < tcfg->nl)
  {
    log_error("insufficient swa bytes");
    return nullptr;
  }
  swa_buf.resize(n_bytes);
  tcfg->swa.resize(tcfg->nl);
  if (!fread_ckd(swa_buf.data(), n_bytes, sizeof(char), fp))
    return nullptr;

  for (int i = 0; i < tcfg->nl; i++)
  {
    int pos             = i;
    int byte_idx        = pos / 8;
    int bit_idx         = 7 - (pos % 8);
    tcfg->swa[i] = (swa_buf[byte_idx] >> bit_idx) & 1;
  }

  // Extra feature flags packed into one byte
  uint8_t extra_flags;
  if (!read_uint8_ckd(fp, &extra_flags)) return nullptr;
  *support_mm        = (extra_flags & 8) == 8;
  tcfg->qk_norm      = (extra_flags & 4) == 4;
  tcfg->mlp_pre = (extra_flags & 2) == 2;
  tcfg->mlp_pst = (extra_flags & 1) == 1;

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
  int rank;

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
  uint32_t nmerge;

  int nvocab;
  int maxlen;
  int bos; /* beginning of sequence */
  int eos; /* end of sequence */
  int sot; /* start of turn */
  int eot; /* end of turn */
  int soi; /* start of image */
  int eoi; /* end of image */
  int ist; /* image soft token */

  [[=no_print]]
  std::vector<char> vdata;
  [[=no_print]]
  std::vector<char> mdata;
  [[=no_print]]
  std::vector<char *> vocab;
  [[=no_print]]
  std::vector<Token> vsort; /* sorted for binary search */
  [[=no_print]]
  std::vector<Merge> ranks; /* sorted merges for BPE */
};

/* Look up a string in the sorted vocab -> token id (or -1) */
static int
get_token_idx(GemmaTokenizer *tok, std::string_view key)
{
  auto val = std::lower_bound(
    tok->vsort.begin(), tok->vsort.end(), key,
    [](const Token &token, std::string_view value) { return token.val < value; }
  );
  if (val == tok->vsort.end() || val->val != key)
  {
    return -1;
  }
  return val->idx;
}

/* Look up a (str1, str2) pair in the merges table */
static std::optional<Merge>
get_merge_rec(GemmaTokenizer *tok, std::string_view str1, std::string_view str2)
{
  Merge key = {.str1 = str1, .str2 = str2};
  auto val = std::lower_bound(tok->ranks.begin(), tok->ranks.end(), key);
  if (val == tok->ranks.end() || (*val <=> key) != 0)
    return std::nullopt;
  return *val;
}

/* */
static std::unique_ptr<GemmaTokenizer>
read_tokenizer(FILE *fp, TextConfig *tcfg, bool support_mm)
{
  auto tok = std::make_unique<GemmaTokenizer>();
  size_t offset = 0;

  tok->nvocab = tcfg->nvocab;
  if (support_mm)
  {
    tok->nvocab++;
  }  // ++ for the <image_soft_token>

  std::optional<size_t> vocab_data_bytes = get_strarr_bytes(fp, tok->nvocab);
  if (!vocab_data_bytes.has_value()) return nullptr;

  tok->vdata.resize(*vocab_data_bytes);
  tok->vocab.resize(tok->nvocab);

  tok->vsort.resize(tok->nvocab);

  offset = 0;
  tok->maxlen = 0;
  for (int i = 0; i < tok->nvocab; i++)
  {
    char *str;
    if (!read_str_ckd(&str, fp, tok->vdata.data(), &offset)) return nullptr;

    tok->vocab[i]            = str;
    tok->vsort[i].idx = i;
    tok->vsort[i].val = str;

    int len = strlen(str);
    if (len > tok->maxlen)
    {
      tok->maxlen = len;
    }
  }

  std::sort(
    tok->vsort.begin(), tok->vsort.end(),
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
  if (!read_uint32_ckd(fp, &tok->nmerge)) return nullptr;

  tok->ranks.resize(tok->nmerge);

  std::optional<size_t> merge_bytes = get_strarr_bytes(fp, tok->nmerge * 2);
  if (!merge_bytes.has_value()) return nullptr;

  tok->mdata.resize(*merge_bytes);

  offset = 0;
  for (uint32_t i = 0; i < tok->nmerge; i++)
  {
    char *str1, *str2;
    if (!read_str_ckd(&str1, fp, tok->mdata.data(), &offset)) return nullptr;
    if (!read_str_ckd(&str2, fp, tok->mdata.data(), &offset)) return nullptr;

    tok->ranks[i].rank = i;
    tok->ranks[i].str1 = str1;
    tok->ranks[i].str2 = str2;
  }

  std::sort(tok->ranks.begin(), tok->ranks.end());
  return tok;
}

/* Minimal byte-pair encoding algorithm implementation */
int
encode(
  GemmaTokenizer *tok,
  std::string_view text,
  int            *tokens
)
{
	//printf("encode invoke\n");
  const auto *ustr = reinterpret_cast<const unsigned char *>(text.data());

  // Convert UTF-8 string to tokens of individual codepoints
  size_t i  = 0;
  int tok_i = 0;

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
    std::optional<Merge> best_pair;

    // Find the merge with best rank
    for (int i = 0; i < tok_i - 1; i++)
    {
      char *str1 = tok->vocab[tokens[i]];
      char *str2 = tok->vocab[tokens[i + 1]];

      auto merge = get_merge_rec(tok, str1, str2);
      if (merge.has_value() && (!best_pair.has_value() || merge->rank < best_pair->rank))
      {
        best_pair = merge;
      }
    }

    if (!best_pair.has_value()) break;  // No more merges

    int i = 0;
    while (i < tok_i - 1)
    {
      Merge pair = {
        .str1 = tok->vocab[tokens[i]], .str2 = tok->vocab[tokens[i + 1]]
      };
      if ((*best_pair <=> pair) == 0)
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

  return tok_i;
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

/* a linear layer, either dense fpx or int8 + scales */
struct QTensor
{
  bool quant = false;
  floatx *fpx = nullptr;
  struct
  {
    int8_t *q = nullptr;
    floatx *scales = nullptr;
  } i8;
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

struct IoReader
{
  std::span<std::byte> datas;

  // Borrow mapped elements and advance to the next field.
  template <class T>
  T *read(size_t count)
  {
    if (count > datas.size() / sizeof(T))
      throw std::out_of_range("mapped model weights are truncated");

    auto *elements = reinterpret_cast<T *>(datas.data());
    datas = datas.subspan(count * sizeof(T));
    return elements;
  }
};

static inline void
mmap_qtensor(QTensor *w, IoReader &reader, int m, int n, bool quant)
{
  const size_t count = static_cast<size_t>(m) * n;
  w->quant = quant;
  if (quant)
  {
    w->i8.q = reader.read<int8_t>(count);
    w->i8.scales = reader.read<floatx>(n);
  }
  else
  {
    w->fpx = reader.read<floatx>(count);
  }
}

/* one gemma transformer block */
struct TextDecoderLayer
{
  /* attention weights
   * 2D weights are stored transposed for better GEMV cache use */

  QTensor wq; /* (n_heads * hdim, dim) */
  QTensor wk; /* (nkv * hdim, dim) */
  QTensor wv; /* (nkv * hdim, dim) */
  QTensor wo; /* (dim, n_heads * hdim) */

  /* feedforward weights */
  QTensor w1; /* (mlp_dim, dim) */
  QTensor w2; /* (mlp_dim, dim) */
  QTensor w3; /* (dim, mlp_dim) */

  /* RMSNorm weights (gemma adds 1.0 to the weight) */
  floatx *nq; /* (hdim,) */
  floatx *nk; /* (hdim,) */
  floatx *n1; /* (dim,) */
  floatx *n2; /* (dim,) */
  floatx *n3; /* (dim,) */
  floatx *n4; /* (dim,) */
};

/* one SigLIP (vision) encoder block */
struct VisionEncoderLayer
{
  /* ViT attention weights */
  QTensor wq; /* (dim, dim) */
  QTensor wk; /* (dim, dim) */
  QTensor wv; /* (dim, dim) */
  QTensor wo; /* (dim, dim) */
  floatx *bq;  /* (dim,) */
  floatx *bk;  /* (dim,) */
  floatx *bv;  /* (dim,) */
  floatx *bo;  /* (dim,) */

  /* ViT feedforward weights */
  QTensor w1; /* (mlp_dim, dim) */
  QTensor w2; /* (mlp_dim, dim) */
  floatx *b1;  /* (mlp_dim,) */
  floatx *b2;  /* (mlp_dim,) */

  /* ViT layernorm weights */
  floatx *n1;   /* (dim,) */
  floatx *n2;   /* (dim,) */
  floatx *n1_b; /* (dim,) */
  floatx *n2_b; /* (dim,) */
};

/* Text decoder container */
struct TextDecoder
{
  TextConfig config;
  /* (nvocab, dim), shared with lm_head (tied weights) */
  QTensor emb;
  std::vector<TextDecoderLayer> layers;
  floatx *fnorm; /* (dim,) */
};

/* */
size_t
get_text_decoder_size(const TextConfig *tcfg, bool quant)
{
  size_t size       = 0;
  int    dim  = tcfg->dim;
  int    mlp_dim    = tcfg->mlp_dim;
  int    q_size     = tcfg->n_heads * tcfg->hdim;
  int    kv_size    = tcfg->nkv * tcfg->hdim;
  int    nvocab = tcfg->nvocab;

  // embedding: linear (dim -> nvocab)
  size += qtensor_size(dim, nvocab, quant);

  for (int l = 0; l < tcfg->nl; l++)
  {
    // wq, wk, wv, wo
    size += qtensor_size(dim, q_size, quant);
    size += qtensor_size(dim, kv_size, quant);
    size += qtensor_size(dim, kv_size, quant);
    size += qtensor_size(q_size, dim, quant);

    // optional nq, nk
    if (tcfg->qk_norm)
    {
      size += (size_t)tcfg->hdim * sizeof(floatx);
      size += (size_t)tcfg->hdim * sizeof(floatx);
    }

    // w1, w2, w3
    size += qtensor_size(dim, mlp_dim, quant);
    size += qtensor_size(dim, mlp_dim, quant);
    size += qtensor_size(mlp_dim, dim, quant);

    // n1, n2
    size += (size_t)dim * sizeof(floatx);
    size += (size_t)dim * sizeof(floatx);

    // optional n3, n4
    if (tcfg->mlp_pre)
    {
      size += (size_t)dim * sizeof(floatx);
    }
    if (tcfg->mlp_pst)
    {
      size += (size_t)dim * sizeof(floatx);
    }
  }

  // final_norm
  size += (size_t)dim * sizeof(floatx);

  return size;
}

/* */
static TextDecoder *
mmap_text_decoder(IoReader &reader, TextConfig *tc, bool quant)
{
  auto dec = std::make_unique<TextDecoder>();
  int dim = tc->dim;
  int mlp_dim = tc->mlp_dim;
  int q_sz = tc->n_heads * tc->hdim;
  int kv_sz = tc->nkv * tc->hdim;
  int nvocab = tc->nvocab;
  dec->config = *tc;

  /* embedding: linear (dim -> nvocab) */
  mmap_qtensor(&dec->emb, reader, dim, nvocab, quant);

  /* layers array */
  dec->layers.resize(tc->nl);

  for (auto &layer : dec->layers)
  {
    /* attention weights */
    mmap_qtensor(&layer.wq, reader, dim, q_sz, quant);
    mmap_qtensor(&layer.wk, reader, dim, kv_sz, quant);
    mmap_qtensor(&layer.wv, reader, dim, kv_sz, quant);
    mmap_qtensor(&layer.wo, reader, q_sz, dim, quant);

    /* QK normalization (optional) */
    if (tc->qk_norm)
    {
      layer.nq = reader.read<floatx>(tc->hdim);
      layer.nk = reader.read<floatx>(tc->hdim);
    }
    else
    {
      layer.nq = NULL;
      layer.nk = NULL;
    }

    /* feedforward weights */
    mmap_qtensor(&layer.w1, reader, dim, mlp_dim, quant);
    mmap_qtensor(&layer.w2, reader, dim, mlp_dim, quant);
    mmap_qtensor(&layer.w3, reader, mlp_dim, dim, quant);

    /* RMSNorm weights */
    layer.n1 = reader.read<floatx>(dim);
    layer.n2 = reader.read<floatx>(dim);

    /* optional pre/post MLP norms */
    if (tc->mlp_pre)
    {
      layer.n3 = reader.read<floatx>(dim);
    }
    else
    {
      layer.n3 = NULL;
    }

    if (tc->mlp_pst)
    {
      layer.n4 = reader.read<floatx>(dim);
    }
    else
    {
      layer.n4 = NULL;
    }

  }

  /* final norm */
  dec->fnorm = reader.read<floatx>(dim);

  return dec.release();
}

/* vision encoder container */
struct VisionEncoder
{
  VisionConfig config;

  floatx *pe;       /* (dim, 3, psize, psize) */
  floatx *pe_b;     /* (dim,) */
  QTensor pos_emb; /* ((img_px / psize)^2, dim) */

  std::vector<VisionEncoderLayer> layers;

  floatx *post_n;  /* (dim,) */
  floatx *post_nb; /* (dim,) */
  floatx *norm;    /* (dim,) */
  QTensor proj;   /* (dim, dim) */
};

size_t get_vision_encoder_size(const VisionConfig *vc, const TextConfig *tc,
                               bool quant) {
  size_t size = 0;
  int P = vc->psize;
  int dim = vc->dim;
  int mlp_dim = vc->mlp_dim;
  int npatch = vc->img_px / P;
  npatch *= npatch;

  /* pe */
  size += (size_t)dim * 3 * P * P * sizeof(floatx);
  /* pe_b */
  size += (size_t)dim * sizeof(floatx);

  /* pos_emb: linear (dim -> npatch) */
  size += qtensor_size(dim, npatch, quant);

  /* each layer */
  for (int l = 0; l < vc->nl; l++) {
    /* n1, n1_b */
    size += (size_t)dim * sizeof(floatx);
    size += (size_t)dim * sizeof(floatx);

    /* wq, wk, wv, wo: linear (dim -> dim) */
    size += qtensor_size(dim, dim, quant);
    size += qtensor_size(dim, dim, quant);
    size += qtensor_size(dim, dim, quant);
    size += qtensor_size(dim, dim, quant);

    /* bq, bk, bv, bo */
    size += (size_t)dim * sizeof(floatx);
    size += (size_t)dim * sizeof(floatx);
    size += (size_t)dim * sizeof(floatx);
    size += (size_t)dim * sizeof(floatx);

    /* n2, n2_b */
    size += (size_t)dim * sizeof(floatx);
    size += (size_t)dim * sizeof(floatx);

    /* w1: linear (dim -> mlp_dim), w2: linear (mlp_dim -> dim) */
    size += qtensor_size(dim, mlp_dim, quant);
    size += qtensor_size(mlp_dim, dim, quant);

    /* b1 (mlp_dim), b2 (dim) */
    size += (size_t)mlp_dim * sizeof(floatx);
    size += (size_t)dim * sizeof(floatx);
  }

  /* post_n, post_nb */
  size += (size_t)dim * sizeof(floatx);
  size += (size_t)dim * sizeof(floatx);

  /* norm */
  size += (size_t)dim * sizeof(floatx);

  /* proj: linear (dim -> tc->dim) */
  size += qtensor_size(dim, tc->dim, quant);

  return size;
}

/* */
static VisionEncoder *
mmap_vision_encoder(
  IoReader &reader, TextConfig *tc, VisionConfig *vc, bool quant
)
{
  auto enc = std::make_unique<VisionEncoder>();
  int P = vc->psize;
  int dim = vc->dim;
  int mlp_dim = vc->mlp_dim;
  int npatch = vc->img_px / P;
  enc->config = *vc;

  npatch *= npatch;

  /* pe: dim * 3 * P * P floats */
  enc->pe = reader.read<floatx>(static_cast<size_t>(dim) * 3 * P * P);

  /* pe_b: dim floats */
  enc->pe_b = reader.read<floatx>(dim);

  /* pos_emb: linear (dim -> npatch) */
  mmap_qtensor(&enc->pos_emb, reader, dim, npatch, quant);

  /* layers array */
  enc->layers.resize(vc->nl);

  for (auto &layer : enc->layers)
  {
    /* n1, n1_b */
    layer.n1 = reader.read<floatx>(dim);
    layer.n1_b = reader.read<floatx>(dim);

    /* attention weights: wq, wk, wv, wo (linear dim -> dim) */
    mmap_qtensor(&layer.wq, reader, dim, dim, quant);
    mmap_qtensor(&layer.wk, reader, dim, dim, quant);
    mmap_qtensor(&layer.wv, reader, dim, dim, quant);
    mmap_qtensor(&layer.wo, reader, dim, dim, quant);

    /* attention biases: bq, bk, bv, bo */
    layer.bq = reader.read<floatx>(dim);
    layer.bk = reader.read<floatx>(dim);
    layer.bv = reader.read<floatx>(dim);
    layer.bo = reader.read<floatx>(dim);

    /* n2, n2_b */
    layer.n2 = reader.read<floatx>(dim);
    layer.n2_b = reader.read<floatx>(dim);

    /* feedforward weights: w1 (dim -> mlp_dim), w2 (mlp_dim -> dim) */
    mmap_qtensor(&layer.w1, reader, dim, mlp_dim, quant);
    mmap_qtensor(&layer.w2, reader, mlp_dim, dim, quant);

    /* feedforward biases: b1 (mlp_dim), b2 (dim) */
    layer.b1 = reader.read<floatx>(mlp_dim);
    layer.b2 = reader.read<floatx>(dim);
  }

  /* post_n, post_nb */
  enc->post_n = reader.read<floatx>(dim);
  enc->post_nb = reader.read<floatx>(dim);

  /* norm */
  enc->norm = reader.read<floatx>(dim);

  /* proj: linear (dim -> dim) */
  mmap_qtensor(&enc->proj, reader, dim, tc->dim, quant);

  return enc.release();
}

/* Top-level model container */
struct GemmaModel
{
  // Keep the mapped weights alive until all borrowers have been destroyed.
  std::optional<io::mapped_region> mapped_weights;
  std::vector<std::byte> weights;
  std::unique_ptr<TextDecoder> decoder;
  std::unique_ptr<VisionEncoder> encoder;
  std::unique_ptr<GemmaTokenizer> tokenizer;
  bool            quant;  // W8A8
  bool            support_mm;
};

/* check the file header and return a flag that indicates whether the model
 * is quantized. return -1 if failed. */
static int check_head(FILE *fp, const char *fname) {
  /* magic header */
  char hdr[8];
  if (!fread_ckd(hdr, 8, 1, fp))
    return -1;
  if (memcmp(hdr, "GEMA", 4) != 0) {
    log_error("not a valid gemma model file: %s", fname);
    return -1;
  }
  /* dtype */
  if (memcmp(hdr + 4, DTYPE_MAGIC, 3) != 0) {
    log_error("unsupported dtype '%c%c%c' in model file", hdr[4], hdr[5],
              hdr[6]);
    return -1;
  }
  /* W8A8 flag ('Q' / 'U') */
  if (hdr[7] == 'Q') {
    return 1;
  } else if (hdr[7] != 'U') {
    log_error("not a valid gemma model file: %s", fname);
    return -1;
  }

  return 0;
}

using FilePtr = std::unique_ptr<FILE, decltype(&fclose)>;

/* Map the entire text & vision model into virtual memory */
std::unique_ptr<GemmaModel>
mmap_gemma_model(const char *filename, bool enable_mm)
{
  FilePtr file(fopen(filename, "rb"), fclose);
  FILE *fp = file.get();
  auto model = std::make_unique<GemmaModel>();
  std::unique_ptr<TextConfig> tcfg;
  std::unique_ptr<VisionConfig> vcfg;
  int quant = -1;
  bool use_mm = false;
  size_t mapping_size = 0;
  if (fp == NULL)
  {
    log_error("failed to open file: %s", filename);
    return nullptr;
  }

  /* header */
  quant = check_head(fp, filename);
  if (quant == -1) return nullptr;
  model->quant = (bool)quant;

  /* text config */
  tcfg.reset(read_text_config(fp, &model->support_mm));
  if (tcfg == NULL) return nullptr;

  use_mm = model->support_mm && enable_mm;

  /* vision config */
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

  /* tokenizer */
  model->tokenizer = read_tokenizer(fp, tcfg.get(), model->support_mm);
  if (!model->tokenizer) return nullptr;

  const long offset_l = ftell(fp);
  if (offset_l < 0)
  {
    log_error("ftell failed: %s", filename);
    return nullptr;
  }
  const size_t offset = static_cast<size_t>(offset_l);

  auto mapped_file = io::mapped_file::open(filename, io::file_access::read);
  if (!mapped_file)
  {
    log_error("failed to open file: %s: %s", filename,
              mapped_file.error().message().c_str());
    return nullptr;
  }

  /* get the total amount of bytes for the weights */
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
  IoReader reader{model->mapped_weights->bytes().subspan(offset)};

  try
  {
    model->decoder.reset(mmap_text_decoder(reader, tcfg.get(), model->quant));
    if (use_mm)
    {
      model->encoder.reset(
        mmap_vision_encoder(reader, &model->decoder->config, vcfg.get(), model->quant)
      );
    }
  }
  catch (const std::out_of_range &error)
  {
    log_error("failed to read mapped model weights: %s: %s", filename, error.what());
    return nullptr;
  }

  return model;
}

/* Load the entire text & vision model into RAM */
std::unique_ptr<GemmaModel>
read_gemma_model(const char *filename, bool enable_mm)
{
  FilePtr file(fopen(filename, "rb"), fclose);
  FILE *fp = file.get();
  auto model = std::make_unique<GemmaModel>();
  std::unique_ptr<TextConfig> tcfg;
  std::unique_ptr<VisionConfig> vcfg;
  int quant = -1;
  bool use_mm = false;
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
  model->tokenizer = read_tokenizer(fp, tcfg.get(), model->support_mm);
  if (!model->tokenizer) return nullptr;

  size_t weights_size = get_text_decoder_size(tcfg.get(), model->quant);
  if (use_mm)
    weights_size += get_vision_encoder_size(vcfg.get(), tcfg.get(), model->quant);

  model->weights.resize(weights_size);
  if (!fread_ckd(model->weights.data(), model->weights.size(), 1, fp))
    return nullptr;

  IoReader reader{model->weights};

  try
  {
    model->decoder.reset(mmap_text_decoder(reader, tcfg.get(), model->quant));
    if (use_mm)
    {
      model->encoder.reset(
        mmap_vision_encoder(reader, &model->decoder->config, vcfg.get(), model->quant)
      );
    }
  }
  catch (const std::out_of_range &error)
  {
    log_error("failed to read model weights: %s: %s", filename, error.what());
    return nullptr;
  }
  return model;
}

// Runtime buffers (allocated once, reused every step)

struct VisionBuffer
{
  std::vector<int8_t> x_i8_;
  std::vector<floatx> xsc;     /* (npatch,) */
  std::vector<int8_t> mlp_i8_;
  std::vector<floatx> mlpsc;   /* (npatch) */
  std::vector<floatx> x_;
  std::vector<floatx> resid_;
  std::vector<floatx> xq_;
  std::vector<floatx> xk_;
  std::vector<floatx> xv_;
  std::vector<floatx> att_out_;
  std::vector<floatx> mlp_h_;
  std::vector<floatx> scores_;
  std::mdspan<int8_t, std::dims<2>> x_i8; /* (npatch, dim) */
  std::mdspan<floatx, std::dims<2>> x; /* (npatch, dim) */
  std::mdspan<floatx, std::dims<2>> resid; /* (npatch, dim) */
  std::mdspan<floatx, std::dims<2>> xq; /* (npatch, dim) */
  std::mdspan<floatx, std::dims<2>> xk; /* (npatch, dim) */
  std::mdspan<floatx, std::dims<2>> xv; /* (npatch, dim) */
  std::mdspan<floatx, std::dims<2>> mlp_h; /* (npatch, mlp_dim) */
  std::mdspan<int8_t, std::dims<2>> mlp_i8;  /* (npatch, mlp_dim) */
  std::mdspan<floatx, std::dims<3>> scores; /* (n_heads, npatch, npatch) */
  std::mdspan<floatx, std::dims<2>> att_out; /* (npatch, dim) */
  VisionBuffer(const VisionConfig &vcfg, bool quant);
};

size_t get_vision_buffer_size(VisionConfig *vc, bool quant) {
  size_t size = 0;

  int ppi = vc->img_px / vc->psize;
  int npatch = ppi * ppi;

  /* quantized buffers */
  if (quant) {
    size +=
        npatch * (sizeof(int8_t) * (vc->dim + vc->mlp_dim) + /* x_i8 & mlp_i8 */
                  sizeof(floatx) * 2                         /* xsc & mlpsc */
                 );
  }

  /* main buffers */
  size += npatch * sizeof(floatx) *
          (vc->dim * 6 +        /* x & resid & xq & xk & xv & att_out */
           vc->mlp_dim +        /* mlp_h */
           npatch * vc->n_heads /* scores */
          );

  return size;
}

/* */
VisionBuffer::VisionBuffer(const VisionConfig &vc, bool quant)
{
  size_t dim = (size_t)vc.dim;
  size_t ppi = (size_t)vc.img_px / vc.psize;
  size_t n_patches = ppi * ppi;
  size_t mlp_dim = (size_t)vc.mlp_dim;
  size_t n_heads = (size_t)vc.n_heads;

  if (quant)
  {
    // Quantization buffers
    x_i8_.resize(n_patches * dim);
    x_i8 = std::mdspan(x_i8_.data(), n_patches, dim);
    mlp_i8_.resize(n_patches * mlp_dim);
    mlp_i8 = std::mdspan(mlp_i8_.data(), n_patches, mlp_dim);
    xsc.resize(n_patches);
    mlpsc.resize(n_patches);
  }

  x_.resize(n_patches * dim);
  x = std::mdspan(x_.data(), n_patches, dim);
  resid_.resize(n_patches * dim);
  resid = std::mdspan(resid_.data(), n_patches, dim);
  xq_.resize(n_patches * dim);
  xq = std::mdspan(xq_.data(), n_patches, dim);
  xk_.resize(n_patches * dim);
  xv_.resize(n_patches * dim);
  xk = std::mdspan(xk_.data(), n_patches, dim);
  xv = std::mdspan(xv_.data(), n_patches, dim);
  att_out_.resize(n_patches * dim);
  att_out = std::mdspan(att_out_.data(), n_patches, dim);
  mlp_h_.resize(n_patches * mlp_dim);
  mlp_h = std::mdspan(mlp_h_.data(), n_patches, mlp_dim);
  scores_.resize(n_heads * n_patches * n_patches);
  scores = std::mdspan(scores_.data(), n_heads, n_patches, n_patches);
}

/* gemma language model runtime buffer */
struct TextBuffer
{
  int clen; /* cache len */

  /* temporary quantized activations (when quant=true) */
  std::vector<int8_t> x_i8;  /* ([ntok], dim,) */
  std::vector<floatx> xsc;   /* ([ntok],) */
  std::vector<int8_t> xo_i8; /* (n_heads, [ntok], hdim) */
  std::vector<floatx> xosc;  /* ([ntok],) */
  std::vector<int8_t> xg_i8; /* ([ntok], mlp_dim,) */
  std::vector<floatx> xgsc;  /* ([ntok],) */

  /* pre-computed cos/sin for RoPE */
  std::vector<floatx> cs_slid; /* ([ntok], hdim / 2, 2) */
  std::vector<floatx> cs_full; /* ([ntok], hdim / 2, 2) */

  /* residual stream */
  std::vector<floatx> x;     /* ([ntok], dim,) */
  std::vector<floatx> resid; /* ([ntok], dim,) */

  /* attention buffers */
  std::vector<floatx> xq;  /* (n_heads, [ntok], hdim) */
  std::vector<floatx> xk;  /* (nkv, [ntok], hdim) */
  std::vector<floatx> xv;  /* (nkv, hdim, [ntok]) */
  std::vector<floatx> xo;  /* ([ntok], n_heads, hdim) */
  std::vector<floatx> att; /* (n_heads, [ntok], clen) */
  std::vector<floatx> kvc; /* (nl, 2, nkv, clen, hdim) */

  /* MLP buffers */
  std::vector<floatx> xg_;
  std::vector<floatx> xu;     /* ([ntok], mlp_dim,) */
  std::vector<floatx> logits; /* (nvocab,) */
  std::mdspan<floatx, std::dims<2>> xg; /* ([ntok], mlp_dim,) */
  TextBuffer(const TextConfig &tcfg, bool multimodal, int cache_len,
	     int chunk_size, bool use_mm, bool quant);
};

size_t get_text_buffer_size(TextConfig *tc, VisionConfig *vc, int clen,
                            int chunk, bool use_mm, bool quant) {
  size_t size = 0;
  int q_sz = tc->n_heads * tc->hdim;
  int kv_sz = tc->nkv * tc->hdim;
  int ppi = vc->img_px / vc->psize;
  int mult = use_mm ? std::max(ppi * ppi, chunk) : chunk;

  /* quantized buffers */
  if (quant) {
    /* x_i8 & xo_i8 & xg_i8 */
    size += sizeof(int8_t) * (tc->dim + q_sz + tc->mlp_dim);
  }

  /* main buffers */
  size += sizeof(floatx) * ((size_t)tc->nl * 2 * clen * kv_sz + /* kvc */
                            tc->nvocab +                        /* logits */
                            mult * tc->dim +                    /* x */
                            mult * tc->dim +                    /* resid */
                            mult * q_sz +                       /* xq */
                            mult * kv_sz +                      /* xk */
                            mult * tc->hdim +                   /* cs_slid */
                            mult * tc->hdim +                   /* cs_full */
                            mult * kv_sz +                      /* xv */
                            mult * q_sz +                       /* xo */
                            (size_t)mult * tc->n_heads * clen + /* att */
                            mult * tc->mlp_dim +                /* xg */
                            mult * tc->mlp_dim                  /* xu */
                           );

  return size;
}

/* */
TextBuffer::TextBuffer(
  const TextConfig &tcfg,
  bool multimodal,
  int           clen,
  int           chunk_size,
  bool          use_mm,
  bool          quant
) : clen(clen)
{
  int dim = tcfg.dim;
  int nl = tcfg.nl;
  int hdim = tcfg.hdim;
  int n_heads = tcfg.n_heads;
  int mlp_dim = tcfg.mlp_dim;
  int q_size = n_heads * hdim;
  int kv_size = tcfg.nkv * hdim;
  int mult = chunk_size;
  kvc.resize((size_t)nl * 2 * (size_t)clen * (size_t)kv_size);
  logits.resize(tcfg.nvocab);
  // Multimodal models need space for a whole image worth of tokens
  if (use_mm && multimodal)
  {
    mult = std::max<int>(mult, tcfg.img_tok);
  }

  if (quant)
  {
    // Quantization buffers
    x_i8.resize(mult * dim);
    xo_i8.resize(mult * q_size);
    xg_i8.resize(mult * mlp_dim);
    xsc.resize(mult);
    xosc.resize(mult);
    xgsc.resize(mult);
  }

  x.resize(mult * dim);
  resid.resize(mult * dim);
  xq.resize(mult * q_size);
  xk.resize(mult * kv_size);
  cs_slid.resize(mult * hdim);
  cs_full.resize(mult * hdim);
  xv.resize(mult * kv_size);
  xo.resize(mult * q_size);
  att.resize(mult * n_heads * clen);
  xg_.resize(mult * mlp_dim);
  xg = std::mdspan(xg_.data(), mult, mlp_dim);
  xu.resize(mult * mlp_dim);
}

// Math primitives

/* gemma-style RMSNorm: (x * rsqrt(mean(x^2) + eps)) * (weight + 1) */
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

/* classic LayerNorm used inside the SigLIP vision tower */
static void layernorm(std::mdspan<floatx, std::dims<2>> dst, std::mdspan<const floatx, std::dims<2>> src, const floatx *weight,
                      const floatx *bias, float eps) {

  for (size_t i = 0; i < dst.extent(0); i++) {
    float mean = 0.0f;
    for (size_t j = 0; j < src.extent(1); j++) {
      mean += (float)src[i, j];
    }
    mean /= (float)src.extent(1);

    float var = 0.0f;

    for (size_t j = 0; j < src.extent(1); j++) {
      float diff = (float)src[i, j] - mean;
      var += diff * diff;
    }
    var /= (float)src.extent(1);

    float inv_std = 1.0f / sqrtf(var + eps);

    for (size_t j = 0; j < src.extent(1); j++) {
      dst[i, j] = (floatx)(((float)src[i, j] - mean) * inv_std * (float)weight[j] +
                        (float)bias[j]);
    }
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
 * Symmetric int8 quantization for source and destination matrices of the same shape
 * Q(fpx src (m, n)) ~= int8 dst (m, n) * fpx dst_scales (m,)
 */
static void
quantize_acts(
  std::mdspan<int8_t, std::dims<2>> dst,
  floatx *RESTRICT dst_scales,
  std::mdspan<const floatx, std::dims<2>> src
)
{
  const size_t cols = src.extent(1);
  for (size_t row = 0; row < src.extent(0); row++)
  {
    const size_t offset = row * cols;
    dst_scales[row] = quantize_act(dst.data_handle() + offset,
                                  src.data_handle() + offset, static_cast<int>(cols));
  }
}

/* */
static inline floatx
gemv_fpx_row(
  const floatx *RESTRICT vec, std::span<const floatx> mat)
{
  float sum = 0;
  for (size_t j = 0; j < mat.size(); j++)
  {
    sum += (float)mat[j] * (float)vec[j];
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
  std::mdspan<const floatx, std::dims<2>> mat,
  const floatx *RESTRICT vec
)
{
  for (size_t i = 0; i < mat.extent(0); i++)
  {
    dst[i] = gemv_fpx_row(vec, std::span(&mat[i, 0], mat.extent(1)));
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
  int                    n
)
{
  for (int i = 0; i < m; i++)
  {
    dst[i] = gemv_int8_row(vec, vec_scale, mat, mat_scales, n, i);
  }
}

/**
 * one projection tensor of a fused multi-gemv.
 * `w` is an int8 matrix (gemv_int8_multi) or a floatx matrix
 * (gemv_fpx_multi), `sc` the per-row scales of the int8 case.
 */
typedef struct {
  const void *w;
  const floatx *sc;
  floatx *dst;
  int m;
} GemvJob;

/* walks a flat row index across the jobs in job order */
static inline int gemv_job_of(std::span<const GemvJob> jobs, int r, int *row) {
  int j = 0;
  while (r >= jobs[j].m) {
    r -= jobs[j].m;
    j++;
  }
  *row = r;
  return j;
}

/**
 * fused int8 MVs (NT): the q/k/v (or gate/up) projections of a layer read the
 * same activation vector and do not depend on each other, so they can share
 * one parallel region.
 */
static void gemv_int8_multi(std::span<const GemvJob> jobs,
                            const int8_t *RESTRICT vec, floatx vsc, int n) {
  int rows = 0;
  for (size_t j = 0; j < jobs.size(); j++) {
    rows += jobs[j].m;
  }

  int r;
  for (r = 0; r < rows; r++) {
    int i;
    int j = gemv_job_of(jobs, r, &i);
    jobs[j].dst[i] =
        gemv_int8_row(vec, vsc, (const int8_t *)jobs[j].w, jobs[j].sc, n, i);
  }
}

/** fpx flavour of gemv_int8_multi. */
static void gemv_fpx_multi(std::span<const GemvJob> jobs,
                           const floatx *RESTRICT vec, int n) {
  int rows = 0;
  for (size_t j = 0; j < jobs.size(); j++) {
    rows += jobs[j].m;
  }

  int r;
  for (r = 0; r < rows; r++) {
    int i;
    int j = gemv_job_of(jobs, r, &i);
    const floatx *mat = (const floatx *)jobs[j].w;
    jobs[j].dst[i] = gemv_fpx_row(vec, std::span(mat + (size_t)i * n, n));
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

/* first query head of kv head `hk` */
static inline int gqa_head_of(int hk, int n_heads, int nkv) {
  return (hk * n_heads + nkv - 1) / nkv;
}

/**
 * att[h][t] = k[t] . xq[h], `att`/`kc` offset to the first row of interest.
 */
static void att_scores_gqa(floatx *RESTRICT att, int astr,
                           const floatx *RESTRICT xq, const floatx *RESTRICT kc,
                           int kstr, int nkv, int n_heads, int attlen, int hdim) {

  int ntask = nkv * attlen;
  int idx;
  for (idx = 0; idx < ntask; idx++) {
    int hk = idx / attlen;
    int t = idx - hk * attlen;
    int h0 = gqa_head_of(hk, n_heads, nkv);
    int hn = gqa_head_of(hk + 1, n_heads, nkv) - h0;

    /* k row stays in L1 across the query heads of the group */
    const floatx *krow = kc + (size_t)hk * kstr + (size_t)t * hdim;
    floatx *ah = att + (size_t)h0 * astr + t;
    const floatx *qg = xq + (size_t)h0 * hdim;

    for (int g = 0; g < hn; g++) {
      ah[(size_t)g * astr] = gemv_fpx_row(krow, std::span(qg + (size_t)g * hdim, hdim));
    }
  }
}

/* output dims per task: 16 floats is exactly one 64 byte line in fp32, so the
 * tiles of one v row never share a line and every line is read exactly once */
#define GQA_TD 16

/**
 * xo[h][:] = sum_t att[h][t] * v[t][:], for the heads of one kv group
 */
static void att_values_gqa(floatx *RESTRICT xo, const floatx *RESTRICT att,
                           int astr, const floatx *RESTRICT vc, int vstr,
                           int nkv, int n_heads, int attlen, int hdim) {
  int ntile = (hdim + GQA_TD - 1) / GQA_TD;
  int ntask = nkv * ntile;
  int idx;

  for (idx = 0; idx < ntask; idx++) {
    int hk = idx / ntile;
    int d0 = (idx - hk * ntile) * GQA_TD;
    int h0 = gqa_head_of(hk, n_heads, nkv);
    int hn = gqa_head_of(hk + 1, n_heads, nkv) - h0;

    const floatx *vc_h = vc + (size_t)hk * vstr;
    const floatx *ag = att + (size_t)h0 * astr;
    floatx *og = xo + (size_t)h0 * hdim;
    int g;

    if (d0 + GQA_TD <= hdim) {
#if defined(GEMMA_GEMM_AVX2)
/* out[g][d0:d0+16] = sum_t att[g][t] * v[t][d0:d0+16], for NG heads at once.
 * the v chunk is loaded once per t and reused for all NG heads. */
#define GQA_VALUES_BODY(NG)                                                    \
  do {                                                                         \
    __m256 a0[NG], a1[NG];                                                     \
    for (int g = 0; g < NG; g++) {                                             \
      a0[g] = _mm256_setzero_ps();                                             \
      a1[g] = _mm256_setzero_ps();                                             \
    }                                                                          \
    for (int t = 0; t < attlen; t++) {                                         \
      const floatx *vr = vc_h + (size_t)t * hdim + d0;                         \
      __m256 v0 = gemm_load_fpx8(vr);                                          \
      __m256 v1 = gemm_load_fpx8(vr + 8);                                      \
      for (int g = 0; g < NG; g++) {                                           \
        __m256 w = _mm256_set1_ps((float)agg[(size_t)g * astr + t]);           \
        a0[g] = _mm256_fmadd_ps(w, v0, a0[g]);                                 \
        a1[g] = _mm256_fmadd_ps(w, v1, a1[g]);                                 \
      }                                                                        \
    }                                                                          \
    for (int g = 0; g < NG; g++) {                                             \
      gemm_store_fpx8(ogg + (size_t)g * hdim + d0, a0[g]);                     \
      gemm_store_fpx8(ogg + (size_t)g * hdim + d0 + 8, a1[g]);                 \
    }                                                                          \
  } while (0)

      /* four heads at a time: 4 x 2 ymm accumulators still fit the register
       * file, more would spill */
      for (int g0 = 0; g0 < hn; g0 += 4) {
        int gn = min(4, hn - g0);
        const floatx *agg = ag + (size_t)g0 * astr;
        floatx *ogg = og + (size_t)g0 * hdim;
        switch (gn) {
        case 4:
          GQA_VALUES_BODY(4);
          break;
        case 3:
          GQA_VALUES_BODY(3);
          break;
        case 2:
          GQA_VALUES_BODY(2);
          break;
        default:
          GQA_VALUES_BODY(1);
          break;
        }
      }
#else
      for (g = 0; g < hn; g++) {
        float acc[GQA_TD];
        for (int d = 0; d < GQA_TD; d++) {
          acc[d] = 0.0f;
        }
        for (int t = 0; t < attlen; t++) {
          float w = (float)ag[(size_t)g * astr + t];
          const floatx *vr = vc_h + (size_t)t * hdim + d0;
          for (int d = 0; d < GQA_TD; d++) {
            acc[d] += w * (float)vr[d];
          }
        }
        for (int d = 0; d < GQA_TD; d++) {
          og[(size_t)g * hdim + d0 + d] = (floatx)acc[d];
        }
      }
#endif
    } else {
      /* ragged tail: at most GQA_TD-1 dims, scalar is fine */
      for (g = 0; g < hn; g++) {
        for (int d = d0; d < hdim; d++) {
          float acc = 0.0f;
          for (int t = 0; t < attlen; t++) {
            acc += (float)ag[(size_t)g * astr + t] *
                   (float)vc_h[(size_t)t * hdim + d];
          }
          og[(size_t)g * hdim + d] = (floatx)acc;
        }
      }
    }
  }
}

// A decent GEMM kernel implementation, I pretty much vibe-coded this part :D

/**
 * Pack every R-row panel of a (total_rows x k) matrix, back to back,
 * into one contiguous buffer.
 */
template <class LayoutPolicy>
static void
pack_all_fpx(
  float *RESTRICT        packed_full,
  std::mdspan<const floatx, std::dims<2>, LayoutPolicy> mat,
  int                    total_rows,
  int                    R,
  int                    k
)
{
  for (int b = 0; b < total_rows; b += R)
  {
    for (int r = 0; r < R; r++)
      for (int l = 0; l < k; l++)
        packed_full[(size_t)b * k + l * R + r] = (float)mat[b + r, l];
  }
}

/* Scalar fallback for the remainder tiles. */
template <class DstLayoutPolicy, class MatLayoutPolicy, class SrcLayoutPolicy>
static inline void
gemm_fpx_scalar(
  std::mdspan<floatx, std::dims<2>, DstLayoutPolicy> dst,
  std::mdspan<const floatx, std::dims<2>, MatLayoutPolicy> mat,
  std::mdspan<const floatx, std::dims<2>, SrcLayoutPolicy> src
)
{
  for (size_t i = 0; i < dst.extent(0); i++)
    for (size_t j = 0; j < dst.extent(1); j++)
    {
      float sum = 0.0f;
      for (size_t l = 0; l < src.extent(1); l++)
        sum += (float)src[i, l] * (float)mat[j, l];
      dst[i, j] = (floatx)sum;
    }
}

static inline std::mdspan<const floatx, std::dims<2>, std::layout_stride>
gemm_fpx_input_view(const floatx *src, int rows, int cols, int stride)
{
  std::dims<2> extents(rows, cols);
  if (stride == 0) stride = cols;
  return {src, std::layout_stride::mapping(extents, std::array<size_t, 2>{(size_t)stride, 1})};
}

static inline std::mdspan<floatx, std::dims<2>, std::layout_stride>
gemm_fpx_dst_view(floatx *dst, int rows, int cols, int stride)
{
  std::dims<2> extents(rows, cols);
  if (stride == 0) stride = cols;
  return {dst, std::layout_stride::mapping(extents, std::array<size_t, 2>{(size_t)stride, 1})};
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

/* */
template <class LayoutPolicy>
static inline void
gemm_fpx_compute_row(
  std::mdspan<floatx, std::dims<2>, LayoutPolicy> dst,
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

  for (int r = 0; r < mc; r++)
    for (int c = 0; c < nc; c++)
      dst[ic + r, c] = (floatx)acc_buf[(size_t)r * GEMM_NC + c];
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
template <class LayoutPolicy>
static void
gemm_fpx_compute_block(
  std::mdspan<floatx, std::dims<2>, LayoutPolicy> dst,
  const float *RESTRICT Ap_full,   // packed A, (m_rows, k)
  const float *RESTRICT Bp_block,  // packed B, (nc, k)
  int                   m_rows,
  int                   nc,
  int                   k
)
{
  int ic;

  for (ic = 0; ic < m_rows; ic += GEMM_MC)
  {
    gemm_fpx_compute_row(
      dst, Ap_full, Bp_block, m_rows, ic, nc, k
    );
  }
}

/**
 * fpx matrix-matrix multiply (NT)
 * fpx src (m, k) @ fpx mat.T (k, n) = fpx dst (m, n)
 */
template <class DstLayoutPolicy>
static int
gemm_fpx(
  std::mdspan<floatx, std::dims<2>, DstLayoutPolicy> dst,
  std::mdspan<const floatx, std::dims<2>, std::layout_stride> mat,
  std::mdspan<const floatx, std::dims<2>, std::layout_stride> src
)
{
  const int m = static_cast<int>(dst.extent(0));
  const int n = static_cast<int>(dst.extent(1));
  const int k = static_cast<int>(src.extent(1));
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

    pack_all_fpx(Ap_full, src, m_full, 8, k);

    for (int jc = 0; jc < n_full; jc += GEMM_NC)
    {
      int nc = n_full - jc;
      if (nc > GEMM_NC) nc = GEMM_NC;

      auto mat_block = std::submdspan(mat, std::pair{jc, jc + nc}, std::full_extent);
      pack_all_fpx(Bp_block, mat_block, nc, 8, k);

      auto dst_block = std::submdspan(dst, std::full_extent, std::pair{jc, jc + nc});
      gemm_fpx_compute_block(dst_block, Ap_full, Bp_block, m_full, nc, k);
    }
  }

  // Remainder: leftover rows (full width) + leftover columns (remaining
  // height only, so the (m_full:m, n_full:n) corner isn't done twice)
  if (m_full < m)
  {
    gemm_fpx_scalar(
      std::submdspan(dst, std::pair{m_full, m}, std::full_extent), mat,
      std::submdspan(src, std::pair{m_full, m}, std::full_extent)
    );
  }
  if (n_full < n && m_full > 0)
  {
    gemm_fpx_scalar(
      std::submdspan(dst, std::pair{0, m_full}, std::pair{n_full, n}),
      std::submdspan(mat, std::pair{n_full, n}, std::full_extent),
      std::submdspan(src, std::pair{0, m_full}, std::full_extent)
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
  std::mdspan<floatx, std::dims<2>, std::layout_stride> dst,
  std::mdspan<const floatx, std::dims<2>, std::layout_stride> mat,
  std::mdspan<const floatx, std::dims<2>, std::layout_stride> src
)
{
  const int mr = dst.extent(0);
  const int n = dst.extent(1);
  const int k = src.extent(1);

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
    for (int j = 0; j < n; j++)
    {
      row[j] = (float)mat[l, j];
    }

    for (int ii = 0; ii < mr; ii++)
    {
      float a = (float)src[ii, l];
      for (int j = 0; j < n; j++)
      {
        ACC(ii, j) += a * row[j];
      }
    }
  }

  for (int ii = 0; ii < mr; ii++)
  {
    for (int j = 0; j < n; j++)
    {
      dst[ii, j] = (floatx)ACC(ii, j);
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
  std::mdspan<floatx, std::dims<2>, std::layout_stride> dst,
  std::mdspan<const floatx, std::dims<2>, std::layout_stride> mat,
  std::mdspan<const floatx, std::dims<2>, std::layout_stride> src
)
{
  const size_t m = dst.extent(0);
  for (size_t ib = 0; ib < m; ib += 8)
  {
    const auto rows = std::pair{ib, std::min(ib + 8, m)};
    gemm_fpx_nn_kernel(std::submdspan(dst, rows, std::full_extent), mat,
                       std::submdspan(src, rows, std::full_extent));
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
  std::mdspan<int8_t, std::dims<2>> packed,
  std::mdspan<const int8_t, std::dims<2>> src
)
{
  for (size_t r = 0; r < src.extent(0); r++)
    for (size_t l = 0; l < src.extent(1); l++)
      packed[l, r] = src[r, l];
}

/* */
static void
pack_all_i8(
  std::mdspan<int8_t, std::dims<3>> packed,
  std::mdspan<const int8_t, std::dims<2>> mat
)
{
  for (size_t panel = 0; panel < packed.extent(0); panel++)
  {
    const size_t row = panel * 8;
    pack_panel_i8(
      std::submdspan(packed, panel, std::full_extent, std::full_extent),
      std::submdspan(mat, std::pair{row, row + 8}, std::full_extent)
    );
  }
}

/* */
static inline void
gemm_int8_scalar_row(
  std::mdspan<floatx, std::dims<2>, std::layout_stride> dst,
  std::mdspan<const int8_t, std::dims<2>> mat,
  const floatx *RESTRICT mat_scales,
  std::mdspan<const int8_t, std::dims<2>> src,
  const floatx *RESTRICT src_scales,
  size_t i,
  size_t j
)
{
  int32_t sum = 0;
  for (size_t l = 0; l < src.extent(1); l++)
    sum += (int32_t)src[i, l] * (int32_t)mat[j, l];

  dst[i, j] = (floatx)((float)sum * (float)src_scales[i] * (float)mat_scales[j]);
}

/* Scalar fallback for the remainder tiles. */
static inline void
gemm_int8_scalar(
  std::mdspan<floatx, std::dims<2>, std::layout_stride> dst,
  std::mdspan<const int8_t, std::dims<2>> mat,
  const floatx *RESTRICT mat_scales,
  std::mdspan<const int8_t, std::dims<2>> src,
  const floatx *RESTRICT src_scales
)
{
  for (size_t i = 0; i < dst.extent(0); i++)
    for (size_t j = 0; j < dst.extent(1); j++)
      gemm_int8_scalar_row(dst, mat, mat_scales, src, src_scales, i, j);
}

/* Same int8 x int8 -> int32 outer-product micro-kernel. */
static inline void
gemm_int8_kernel_accum(
  std::mdspan<int32_t, std::dims<2>, std::layout_stride> acc_ext,
  std::mdspan<const int8_t, std::dims<2>> Bp,
  std::mdspan<const int8_t, std::dims<2>> Ap
)
{
  int32_t acc[64] = {0};

  for (size_t l = 0; l < Ap.extent(0); l++)
  {
    // clang-format off
    int32_t a0 = Ap[l, 0], a1 = Ap[l, 1], a2 = Ap[l, 2], a3 = Ap[l, 3];
    int32_t a4 = Ap[l, 4], a5 = Ap[l, 5], a6 = Ap[l, 6], a7 = Ap[l, 7];
    int32_t b0 = Bp[l, 0], b1 = Bp[l, 1], b2 = Bp[l, 2], b3 = Bp[l, 3];
    int32_t b4 = Bp[l, 4], b5 = Bp[l, 5], b6 = Bp[l, 6], b7 = Bp[l, 7];

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
      acc_ext[i, j] += acc[i * 8 + j];
}

/* Applies the per-row / per-column dequant scales to an (mc x nc) int32
 * accumulator block and writes the result into dst once. `src_scales` and
 * `mat_scales` are expected to already be offset to this block's first
 * row / column (i.e. src_scales[r] / mat_scales[c] are absolute, not relative
 * to some further outer offset). */
static inline void
gemm_int8_flush_block(
  std::mdspan<floatx, std::dims<2>, std::layout_stride> dst,
  std::mdspan<const int32_t, std::dims<2>, std::layout_stride> acc,
  const floatx *RESTRICT src_scales,
  const floatx *RESTRICT mat_scales
)
{
  for (size_t r = 0; r < dst.extent(0); r++)
  {
    float fscale = (float)src_scales[r];
    for (size_t c = 0; c < dst.extent(1); c++)
      dst[r, c] = (floatx)((float)acc[r, c] * fscale * (float)mat_scales[c]);
  }
}

/* */
static inline void
gemm_int8_compute_row(
  std::mdspan<floatx, std::dims<2>, std::layout_stride> dst,
  std::mdspan<const int8_t, std::dims<3>> Ap_full,
  std::mdspan<const int8_t, std::dims<3>> Bp_block,
  const floatx *RESTRICT src_scales,
  const floatx *RESTRICT mat_scales
)
{
  const size_t mc = dst.extent(0);
  const size_t nc = dst.extent(1);
  const size_t k = Ap_full.extent(1);

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

  std::mdspan acc(acc_buf, std::layout_stride::mapping(
    std::dims<2>(mc, nc), std::array<size_t, 2>{(size_t)GEMM_NC, 1}));
  for (size_t r = 0; r < mc; r++)
    memset(&acc[r, 0], 0, nc * sizeof(int32_t));

  for (size_t pc = 0; pc < k; pc += GEMM_KC)
  {
    const auto depth = std::pair{pc, std::min(pc + GEMM_KC, k)};
    for (size_t jb = 0; jb < nc; jb += 8)
    {
      const auto Bp = std::submdspan(Bp_block, jb / 8, depth, std::full_extent);
      for (size_t ib = 0; ib < mc; ib += 8)
      {
        gemm_int8_kernel_accum(
          std::submdspan(acc, std::pair{ib, ib + 8}, std::pair{jb, jb + 8}),
          Bp, std::submdspan(Ap_full, ib / 8, depth, std::full_extent)
        );
      }
    }
  }

  gemm_int8_flush_block(dst, acc, src_scales, mat_scales);
}

/* Cache-blocked compute core for gemm_int8, blocks the m dimension by GEMM_MC
 * and the k dimension by GEMM_KC so the A/B slices touched by the innermost
 * loop stay small, then applies the dequant scales once per (ic, jc) tile after
 * the full k range has been summed in int32. */
static void
gemm_int8_compute_block(
  std::mdspan<floatx, std::dims<2>, std::layout_stride> dst,
  std::mdspan<const int8_t, std::dims<3>> Ap_full,
  std::mdspan<const int8_t, std::dims<3>> Bp_block,
  const floatx *RESTRICT src_scales,
  const floatx *RESTRICT mat_scales
)
{
  const size_t m = dst.extent(0);
  for (size_t ic = 0; ic < m; ic += GEMM_MC)
  {
    const size_t end = std::min(ic + GEMM_MC, m);
    gemm_int8_compute_row(
      std::submdspan(dst, std::pair{ic, end}, std::full_extent),
      std::submdspan(Ap_full, std::pair{ic / 8, end / 8}, std::full_extent, std::full_extent),
      Bp_block, src_scales + ic, mat_scales
    );
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
  std::mdspan<floatx, std::dims<2>> dst,
  std::mdspan<const int8_t, std::dims<2>> mat,
  const floatx *RESTRICT mat_scales,
  std::mdspan<const int8_t, std::dims<2>> src,
  const floatx *RESTRICT src_scales
)
{
  const int m = static_cast<int>(dst.extent(0));
  const int n = static_cast<int>(dst.extent(1));
  const int k = static_cast<int>(src.extent(1));

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
    std::mdspan Ap_full(scratch, m_full / 8, k, 8);
    pack_all_i8(Ap_full, std::submdspan(src, std::pair{0, m_full}, std::full_extent));

    for (int jc = 0; jc < n_full; jc += GEMM_NC)
    {
      const int nc = std::min(n_full - jc, GEMM_NC);
      const auto cols = std::pair{jc, jc + nc};
      std::mdspan Bp_block(scratch + (size_t)m_full * k, nc / 8, k, 8);

      pack_all_i8(Bp_block, std::submdspan(mat, cols, std::full_extent));
      gemm_int8_compute_block(
        std::submdspan(dst, std::pair{0, m_full}, cols), Ap_full, Bp_block,
        src_scales, mat_scales + jc
      );
    }
  }

  // Handle leftover rows, then leftover columns of the full rows.
  if (m_full < m)
  {
    const auto rows = std::pair{m_full, m};
    gemm_int8_scalar(
      std::submdspan(dst, rows, std::full_extent), mat, mat_scales,
      std::submdspan(src, rows, std::full_extent), src_scales + m_full
    );
  }
  if (n_full < n && m_full > 0)
  {
    const auto rows = std::pair{0, m_full};
    const auto cols = std::pair{n_full, n};
    gemm_int8_scalar(
      std::submdspan(dst, rows, cols),
      std::submdspan(mat, cols, std::full_extent), mat_scales + n_full,
      std::submdspan(src, rows, std::full_extent), src_scales
    );
  }

  return 0;
}

/**
 * tanh approximation of GELU, the activation every gemma uses
 *   GELU(x) ~= 0.5 x (1 + tanh(sqrt(2/pi) (x + 0.044715 x^3)))
 */
static inline float gelu_tanh(float x) {
  const float c = 0.79788456080287f; /* = sqrt(2 / pi) */

  return 0.5f * x * (1.0f + tanhf(c * (x + 0.044715f * x * x * x)));
}

/**
 * GELU gate over `rows` rows of `dim` values
 *   dst[i] = gelu(src[i] + bias[i % dim]) * mul[i]
 * `bias` and `mul` are optional, NULL means "no bias" / "no multiply".
 */
static void gelu(std::mdspan<floatx, std::dims<2>> dst, std::mdspan<const floatx, std::dims<2>> src,
                 const floatx *bias, const floatx *mul) {
  for (size_t i = 0; i < dst.extent(0); i++) {
    for (size_t j = 0; j < dst.extent(1); j++) {
      float x = (float)src[i, j] + (bias != NULL ? (float)bias[j] : 0.0f);
      float g = gelu_tanh(x);
      dst[i, j] = (floatx)(mul != NULL ? g * (float)mul[i * dst.extent(1) + j] : g);
    }
  }
}

/**
 * RoPE frequency table for a single position, `hdim` floats per table
 * with cos & sin interleaved (that's what the rotation below reads).
 */
static void rope_freqs_pos(floatx *slid, floatx *full, int hdim, float theta_l,
                           float theta_g, int pos) {
  for (int d = 0; d < hdim / 2; d++) {
    float e = (float)(-2 * d) / (float)hdim; /* exponent */

    /* rotation angles for sliding window attentions */
    float freq = powf(theta_l, e);
    slid[d * 2] = (floatx)cosf(freq * (float)pos);
    slid[d * 2 + 1] = (floatx)sinf(freq * (float)pos);

    /* rotation angles for full attentions */
    freq = powf(theta_g, e);
    full[d * 2] = (floatx)cosf(freq * (float)pos);
    full[d * 2 + 1] = (floatx)sinf(freq * (float)pos);
  }
}

/**
 * RoPE frequency table for the `n` positions starting at `pos`, one
 * `hdim` block per position, `stride` floats apart in both tables.
 */
static void rope_freqs(floatx *slid, floatx *full, int hdim, int stride,
                       float theta_l, float theta_g, int pos, int n) {

  if (n > 1) {
    int t;
    for (t = 0; t < n; t++) {
      rope_freqs_pos(slid + (size_t)t * stride, full + (size_t)t * stride, hdim,
                     theta_l, theta_g, pos + t);
    }
  } else {
    rope_freqs_pos(slid, full, hdim, theta_l, theta_g, pos);
  }
}

/**
 * apply RoPE in place to `n` vectors of `hdim` values laid out back to
 * back, rotating with the frequency block `fstr` floats apart (0 when
 * all the vectors share one block, as in a decode step). `scale` is folded
 * into the result.
 */
static void rope_apply(floatx *x, int n, int hdim, const floatx *freqs, int fstr, float scale) {
  for (int v = 0; v < n; v++) {
    floatx *data = x + (size_t)v * hdim;
    const floatx *fr = freqs + (size_t)v * fstr;

    for (int d = 0; d < hdim / 2; d++) {
      float cfr = (float)fr[2 * d];
      float sfr = (float)fr[2 * d + 1];
      float a = (float)data[d];            /* first half vector */
      float b = (float)data[d + hdim / 2]; /* second half vector */

      data[d] = (floatx)((a * cfr - b * sfr) * scale);
      data[d + hdim / 2] = (floatx)((a * sfr + b * cfr) * scale);
    }
  }
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

/**
 * turns one row of raw attention scores into probabilities: the q_scale, the
 * optional softcapping and the softmax.
 */
static void att_finalize(floatx *att, int attlen, floatx asc, float sc_att) {
  for (int t = 0; t < attlen; t++) {
    att[t] *= asc;
  }

  if (sc_att != 0.0f) {
    for (int t = 0; t < attlen; t++) {
      float val = (float)att[t] / sc_att;
      att[t] = (floatx)(tanhf(val) * sc_att);
    }
  }

  softmax(att, att, attlen);
}

/* */
std::vector<floatx>
prepare_image(const std::string &path, int img_px)
{
  int rows, cols, channels;
  std::unique_ptr<unsigned char, decltype(&stbi_image_free)> raw(
    load_image_utf8(path.c_str(), &cols, &rows, &channels, 3), stbi_image_free
  );
  if (!raw)
  {
    log_error("%s", stbi_failure_reason());
    return {};
  }

  size_t count = (size_t)img_px * img_px * 3;
  std::vector<unsigned char> resized(count);
  if (!stbir_resize(
        raw.get(), cols, rows, 0, resized.data(), img_px, img_px, 0,
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
  const std::string &path,
  int         img_px,
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

  size_t epi = (size_t)img_px * img_px * 3;  // Elements per image

  *n_crops = 0;

  // Load the full image
  std::unique_ptr<unsigned char, decltype(&stbi_image_free)> raw(
    load_image_utf8(path.c_str(), &cols, &rows, &channels, 3), stbi_image_free
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
    raw.get(), cols, rows, cols * 3, rsz.data(), img_px, img_px, 0, STBIR_RGB,
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
        raw_crop, pw, ph, cols * 3, rsz_crop, img_px, img_px, 0,
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
 * img: (img_px, img_px, 3)
 */
int
forward_vision(VisionEncoder *enc, TextConfig *tc, TextBuffer *tb,
               VisionBuffer *vb, const floatx *img, bool quant) {
  VisionConfig *vc = &enc->config;

  int dim = vc->dim;
  int psize = vc->psize;
  int img_px = vc->img_px;
  int ppi = img_px / psize;
  int npatch = ppi * ppi;
  int img_tok = tc->img_tok;
  int side = (int)roundf(sqrtf((float)img_tok));
  int ksize = (img_px / vc->psize) / side;
  int hdim = dim / vc->n_heads;
  int mlp_dim = vc->mlp_dim;
  int idim = 3 * psize * psize;

  /* patch embedding */

  /* iterate over all the patch.
   * can be further optimized by reusing GEMM, but this part is executed only
   * once per call, the cost is acceptable.
   * (oy, ox) is walked as one flat index, a hand-written `collapse(2)`, so
   * compilers without the clause (MSVC) also split the grid per patch
   * instead of per row. */
  int pi;
  for (pi = 0; pi < npatch; pi++) {
    int oy = pi / ppi;
    int ox = pi - oy * ppi;

    /* compute the embed vector for the current patch */
    for (int oc = 0; oc < dim; oc++) {
      float sum = 0.0f;

      /* equivalent to Conv2d(
       *   in_channels=3, out_channels=dim, ksize=psize,
       *   stride=psize, bias=True) */

      for (int py = 0; py < psize; py++) {
        for (int px = 0; px < psize; px++) {
          for (int c = 0; c < 3; c++) {
            /* img[c, oy*psize + py, ox*psize + px] */
            int in_idx =
                ((oy * psize + py) * img_px + (ox * psize + px)) * 3 + c;
            /* pe[oc, c, py, px] */
            int w_idx = oc * idim + c * psize * psize + py * psize + px;
            sum += (float)enc->pe[w_idx] * (float)img[in_idx];
          }
        }
      }

      vb->x[pi, oc] = std::clamp<floatx>((floatx)(sum + (float)enc->pe_b[oc]), -FLOATX_MAX, FLOATX_MAX);
    }
  }

  if (is_interrupted())
    return 1;

  /* position embedding */
  if (!quant) {
    for (int i = 0; i < npatch; i++) {
      for (int j = 0; j < dim; j++) {
        vb->x[i, j] = std::clamp<floatx>(vb->x[i, j] + enc->pos_emb.fpx[i * dim + j], -FLOATX_MAX, FLOATX_MAX);
      }
    }
  } else {
    /* dequantize per row */
    for (int i = 0; i < npatch; i++) {
      for (int j = 0; j < dim; j++) {
        float scale = (float)enc->pos_emb.i8.scales[i];
        float val = (float)enc->pos_emb.i8.q[i * dim + j] * scale;
        vb->x[i, j] = std::clamp<floatx>(vb->x[i, j] + (floatx)val, -FLOATX_MAX, FLOATX_MAX);
      }
    }
  }

  if (is_interrupted())
    return 1;

  /* encoder layers */
  for (int l = 0; l < vc->nl; l++) {
    VisionEncoderLayer &layer = enc->layers[l];

    for (int i = 0; i < npatch; i++) {
      for (int j = 0; j < dim; j++) {
        vb->resid[i, j] = vb->x[i, j];
      }
    }

    layernorm(vb->x, vb->x, layer.n1, layer.n1_b, vc->eps);
    if (is_interrupted())
      return 1;

    /* QKV projections */
    if (!quant) {
      if (gemm_fpx(vb->xq, gemm_fpx_input_view(layer.wq.fpx, dim, dim, 0), vb->x) == 1 ||
          gemm_fpx(vb->xk, gemm_fpx_input_view(layer.wk.fpx, dim, dim, 0), vb->x) == 1 ||
          gemm_fpx(vb->xv, gemm_fpx_input_view(layer.wv.fpx, dim, dim, 0), vb->x) == 1)
        return 1;
    } else {
      quantize_acts(vb->x_i8, vb->xsc.data(), vb->x);
      if (gemm_int8(vb->xq, std::mdspan(layer.wq.i8.q, dim, dim), layer.wq.i8.scales,
                    vb->x_i8, vb->xsc.data()) == 1 ||
          gemm_int8(vb->xk, std::mdspan(layer.wk.i8.q, dim, dim), layer.wk.i8.scales,
                    vb->x_i8, vb->xsc.data()) == 1 ||
          gemm_int8(vb->xv, std::mdspan(layer.wv.i8.q, dim, dim), layer.wv.i8.scales,
                    vb->x_i8, vb->xsc.data()) == 1)
        return 1;
    }
    if (is_interrupted())
      return 1;

    /* add biases */
    for (int i = 0; i < npatch; i++) {
      for (int j = 0; j < dim; j++) {
        vb->xq[i, j] += layer.bq[j];
        vb->xk[i, j] += layer.bk[j];
        vb->xv[i, j] += layer.bv[j];
      }
    }
    if (is_interrupted())
      return 1;

    /* attention */
    for (size_t i = 0; i < npatch; i++) {
      for (size_t j = 0; j < dim; j++) {
        vb->att_out[i, j] = (floatx)0.0;
      }
    }
    float scale = 1.0f / sqrtf((float)hdim);

    int h;
    bool quit = false;

    for (h = 0; h < vc->n_heads; h++) {
      /* scores = Q @ K^T * scale */
      if (gemm_fpx(
            std::submdspan(vb->scores, h, std::full_extent, std::full_extent),
            std::submdspan(vb->xk, std::full_extent,
                           std::pair{h * hdim, (h + 1) * hdim}),
            std::submdspan(vb->xq, std::full_extent,
                           std::pair{h * hdim, (h + 1) * hdim})) == 1)
        quit = true;

      /* apply scale and softmax per row */
      for (int i = 0; i < npatch; i++) {
        floatx *row = &vb->scores[h, i, 0];
        for (int j = 0; j < npatch; j++) {
          row[j] *= scale;
        }
        softmax(row, row, npatch);
      }

      /* weighted sum of values: out = scores @ V */
      gemm_fpx_nn(
        std::submdspan(vb->att_out, std::full_extent, std::pair{h * hdim, (h + 1) * hdim}),
        std::submdspan(vb->xv, std::full_extent, std::pair{h * hdim, (h + 1) * hdim}),
        std::submdspan(vb->scores, h, std::full_extent, std::full_extent)
      );
    }
    if (quit || is_interrupted())
      return 1;

    /* output projection */
    if (!quant) {
      if (gemm_fpx(vb->x, gemm_fpx_input_view(layer.wo.fpx, dim, dim, 0), vb->att_out) == 1)
        return 1;
    } else {
      quantize_acts(vb->x_i8, vb->xsc.data(), vb->att_out);
      if (gemm_int8(vb->x, std::mdspan(layer.wo.i8.q, dim, dim), layer.wo.i8.scales,
                    vb->x_i8, vb->xsc.data()) == 1)
        return 1;
    }
    if (is_interrupted())
      return 1;

    /* add output bias */
    for (int i = 0; i < npatch; i++) {
      for (int j = 0; j < dim; j++) {
        vb->x[i, j] += layer.bo[j];
      }
    }
    if (is_interrupted())
      return 1;

    /* residual connection */
    for (int i = 0; i < npatch; i++) {
      for (int j = 0; j < dim; j++) {
        vb->x[i, j] = std::clamp<floatx>(vb->x[i, j] + vb->resid[i, j], -FLOATX_MAX, FLOATX_MAX);
      }
    }
    if (is_interrupted())
      return 1;

    for (int i = 0; i < npatch; i++) {
      for (int j = 0; j < dim; j++) {
        vb->resid[i, j] = vb->x[i, j];
      }
    }

    layernorm(vb->x, vb->x, layer.n2, layer.n2_b, vc->eps);
    if (is_interrupted())
      return 1;

    /* x @ fc1 = mlp_h */
    if (!quant) {
      if (gemm_fpx(vb->mlp_h, gemm_fpx_input_view(layer.w1.fpx, mlp_dim, dim, 0), vb->x) == 1)
        return 1;
    } else {
      quantize_acts(vb->x_i8, vb->xsc.data(), vb->x);
      if (gemm_int8(vb->mlp_h, std::mdspan(layer.w1.i8.q, mlp_dim, dim), layer.w1.i8.scales,
                    vb->x_i8, vb->xsc.data()) == 1)
        return 1;
    }
    if (is_interrupted())
      return 1;

    /* apply the fc1 biases, then the GELU activation */
    gelu(vb->mlp_h, vb->mlp_h, layer.b1, NULL);
    if (is_interrupted())
      return 1;

    /* mlp_h @ fc2 = x */
    if (!quant) {
      if (gemm_fpx(vb->x, gemm_fpx_input_view(layer.w2.fpx, dim, mlp_dim, 0), vb->mlp_h) == 1)
        return 1;
    } else {
      quantize_acts(vb->mlp_i8, vb->mlpsc.data(), vb->mlp_h);
      if (gemm_int8(vb->x, std::mdspan(layer.w2.i8.q, dim, mlp_dim), layer.w2.i8.scales,
                    vb->mlp_i8, vb->mlpsc.data()) == 1)
        return 1;
    }
    if (is_interrupted())
      return 1;

    /* x += b2 */
    for (int i = 0; i < npatch; i++) {
      for (int j = 0; j < dim; j++) {
        vb->x[i, j] += layer.b2[j];
      }
    }

    if (is_interrupted())
      return 1;

    /* residual connection */
    for (int i = 0; i < npatch; i++) {
      for (int j = 0; j < dim; j++) {
        vb->x[i, j] = std::clamp<floatx>(vb->x[i, j] + vb->resid[i, j], -FLOATX_MAX, FLOATX_MAX);
      }
    }
    if (is_interrupted())
      return 1;
  }

  /* post-norm + average pooling down to img_tok tokens */
  layernorm(vb->x, vb->x, enc->post_n, enc->post_nb, vc->eps);
  if (is_interrupted())
    return 1;

  /* average pooling (AvgPool2d(kernel_size=K, stride=K)) */

  /* be careful that this loop must remain serial, in-place pooling writes
   * results to the front of vb->x.data(), which overlaps with input data needed by
   * other output positions. parallelizing this loop introduces a read/write
   * race condition. */
  for (int oy = 0; oy < side; oy++) {
    for (int ox = 0; ox < side; ox++) {
      int out_idx = (oy * side + ox) * dim;
      for (int d = 0; d < dim; d++) {
        float sum = 0.0f;
        for (int ky = 0; ky < ksize; ky++) {
          for (int kx = 0; kx < ksize; kx++) {
            int py = oy * ksize + ky;
            int px = ox * ksize + kx;
            int tidx = (py * ppi + px) * dim + d;
            sum += (float)vb->x_.data()[tidx];
          }
        }
        vb->x_.data()[out_idx + d] = (floatx)(sum / (ksize * ksize));
      }
    }
  }

  if (is_interrupted())
    return 1;
  /* tb->x now becomes (img_tok, dim) */

  /* final RMSNorm */
  std::mdspan v_x = std::submdspan(vb->x, std::pair {0, img_tok}, std::full_extent);
  rmsnorm(v_x, v_x, enc->norm, vc->eps);
  if (is_interrupted())
    return 1;

  /* final projection into language model embedding space
   * x (img_tok, dim) = x (img_tok, dim) @ proj (dim, dim) */
  if (!quant) {
    if (gemm_fpx(gemm_fpx_dst_view(tb->x.data(), img_tok, tc->dim, 0), gemm_fpx_input_view(enc->proj.fpx, tc->dim, dim, 0), v_x) == 1)
      return 1;
  } else {
    std::mdspan x_i8 = std::submdspan(vb->x_i8, std::pair {0, img_tok}, std::full_extent);
    quantize_acts(x_i8, vb->xsc.data(), v_x);
    if (gemm_int8(std::mdspan(tb->x.data(), img_tok, tc->dim), std::mdspan(enc->proj.i8.q, tc->dim, dim), enc->proj.i8.scales,
                  x_i8, vb->xsc.data()) == 1)
      return 1;
  }
  if (is_interrupted())
    return 1;

  return 0;
}

int
forward_text_decode(
  TextDecoder *dec, TextBuffer *tb, int pos, bool quant, bool do_logit
)
{
  TextConfig *tc = &dec->config;

  int dim = tc->dim;
  int n_heads = tc->n_heads;
  int nkv = tc->nkv;
  int hdim = tc->hdim;
  int q_sz = n_heads * hdim;
  int kv_sz = nkv * hdim;

  if (pos >= tb->clen)
  {
    log_error("kv cache is full");
    return 1;
  }

  /* precompute cos & sin for all frequencies (used in RoPE) */
  rope_freqs(tb->cs_slid.data(), tb->cs_full.data(), hdim, /*fstr=*/0, tc->theta_l,
             tc->theta_g, pos, 1);
  if (is_interrupted()) return 1;

  /* forward all the layers */
  for (auto [l, layer, sliding] : std::views::zip(
         std::views::iota(0), dec->layers, tc->swa
       ))
  {
    memcpy(tb->resid.data(), tb->x.data(), dim * sizeof(*tb->x.data()));

    std::mdspan x2_mdspan(tb->x.data(), 1, dim);
    rmsnorm(x2_mdspan, x2_mdspan, layer.n1, tc->eps);
    if (is_interrupted()) return 1;

    /* the attention block. q, k and v are independent and share the same
     * activation vector, so they run as one fused gemv: a single parallel
     * region, one pass over x, and enough rows to keep every thread busy on
     * the tiny k/v projections. */
    if (!quant)
    {
      GemvJob jobs[3] = {{layer.wq.fpx, NULL, tb->xq.data(), q_sz},
                         {layer.wk.fpx, NULL, tb->xk.data(), kv_sz},
                         {layer.wv.fpx, NULL, tb->xv.data(), kv_sz}};
      gemv_fpx_multi(jobs, tb->x.data(), dim);
    }
    else
    {
      floatx x_scale = quantize_act(tb->x_i8.data(), tb->x.data(), dim);
      GemvJob jobs[3] = {
          {layer.wq.i8.q, layer.wq.i8.scales, tb->xq.data(), q_sz},
          {layer.wk.i8.q, layer.wk.i8.scales, tb->xk.data(), kv_sz},
          {layer.wv.i8.q, layer.wv.i8.scales, tb->xv.data(), kv_sz}};
      gemv_int8_multi(jobs, tb->x_i8.data(), x_scale, dim);
    }
    if (is_interrupted()) return 1;

    if (tc->qk_norm)
    {
      // Use the non-threading version here since we are running this over
      // every head
      // Query RMSNorm
      std::mdspan xq(tb->xq.data(), n_heads, hdim);
      rmsnorm(xq, xq, layer.nq, tc->eps);
      // Key RMSNorm
      std::mdspan xk(tb->xk.data(), nkv, hdim);
      rmsnorm(xk, xk, layer.nk, tc->eps);
    }
    if (is_interrupted()) return 1;

    floatx *cfreqs = sliding ? tb->cs_slid.data() : tb->cs_full.data();

    /* apply RoPE to queries & keys */
    rope_apply(tb->xq.data(), n_heads, hdim, cfreqs, /*fstr=*/0, 1.0f);
    rope_apply(tb->xk.data(), nkv, hdim, cfreqs, /*fstr=*/0, 1.0f);
    if (is_interrupted()) return 1;

    /* (nkv, clen, hdim) */
    /* the layer offset must be computed in size_t: at long context the int
     * product overflows (7b at --seqlen 16384: 16 * 2 * 16384 * 4096 lands
     * exactly on 2^31, so layer 16 of 28 jumps to -2^31 and segfaults) */
    floatx *k_cache = tb->kvc.data() + (size_t)l * 2 * tb->clen * kv_sz;
    floatx *v_cache = k_cache + (size_t)tb->clen * kv_sz;

    /* write to kvc */
    for (int h = 0; h < nkv; h++) {
      floatx *xk_head =
          k_cache + (size_t)h * tb->clen * hdim + (size_t)pos * hdim;
      floatx *xv_head =
          v_cache + (size_t)h * tb->clen * hdim + (size_t)pos * hdim;
      memcpy(xk_head, tb->xk.data() + h * hdim, hdim * sizeof(*tb->xk.data()));
      memcpy(xv_head, tb->xv.data() + h * hdim, hdim * sizeof(*tb->xv.data()));
    }
    if (is_interrupted()) return 1;

    /* sliding-window (true) or full attention (false)? */
    bool loc_att = sliding && pos >= tc->slide;
    /* starting position of kvc */
    int spos = loc_att ? (pos + 1 - tc->slide) : 0;
    int attlen = pos + 1 - spos; /* include the current pos */

    floatx asc = (floatx)(1.0f / sqrtf((float)tc->q_scale));

    /* attention, fused over the query heads of a kv group: the k/v cache is
     * streamed once per kv head instead of once per query head, which is
     * what the decode traffic at long context is made of */
    int h;

    /* scores: att[h, spos:] = k[spos:, :] . xq[h, :] */
    att_scores_gqa(tb->att.data() + spos, attlen, tb->xq.data(), k_cache + spos * hdim,
                   tb->clen * hdim, nkv, n_heads, attlen, hdim);

    /* q_scale, optional softcapping, softmax: one task per head */
    for (h = 0; h < n_heads; h++) {
      att_finalize(tb->att.data() + h * attlen + spos, attlen, asc, tc->sc_att);
    }

    /* values: xo[h, :] = att[h, spos:] @ v[spos:, :] */
    att_values_gqa(tb->xo.data(), tb->att.data() + spos, attlen, v_cache + spos * hdim,
                   tb->clen * hdim, nkv, n_heads, attlen, hdim);

    if (is_interrupted())
      return 1;
 
    /* output projection maps xo back to x
     * x (dim,) = xo (hdim,) @ wo^T (hdim, dim) */
    if (!quant)
    {
      gemv_fpx(tb->x.data(), std::mdspan(layer.wo.fpx, dim, q_sz), tb->xo.data());
    }
    else
    {
      floatx xo_scale = quantize_act(tb->xo_i8.data(), tb->xo.data(), q_sz);
      gemv_int8(
        tb->x.data(), layer.wo.i8.q, layer.wo.i8.scales, tb->xo_i8.data(), xo_scale,
        dim, q_sz
      );
    }
    if (is_interrupted()) return 1;

    if (tc->mlp_pre) {
      /* gemma 2/3: post-norm on attention output */
      std::mdspan x3_mdspan(tb->x.data(), 1, dim);
      rmsnorm(x3_mdspan, x3_mdspan, layer.n2, tc->eps);
    }
    if (is_interrupted()) return 1;

    /* combine the residual stream */
    floatx *RESTRICT x     = tb->x.data();
    floatx *RESTRICT resid = tb->resid.data();
    for (int d = 0; d < dim; d++)
    {
      /* Sometimes the residual stream accumulates huge values on certain
       * channels, especially in pretrained/bigger models (Sun et al., 2024,
       * https://arxiv.org/abs/2402.17762). It works fine in fp32 or bf16, but
       * it can easily overflow fp16 and become inf, causing all the activations
       * turning into nan after the next RMSNorm, so we need to clamp it. */

      /* NOTE: Actually this should never trigger now since I added activation
       * scalers afterwards (see export.py), the clamp here is more of a
       * last-resort safety net. */

      tb->x[d] = std::clamp<floatx>(x[d] + resid[d], -FLOATX_MAX, FLOATX_MAX);
    }
    if (is_interrupted()) return 1;

    memcpy(tb->resid.data(), tb->x.data(), dim * sizeof(*tb->x.data()));

    /* pre feedforward RMSNorm */
    std::mdspan x5(tb->x.data(), 1, dim);
    if (tc->mlp_pre) {
      rmsnorm(x5, x5, layer.n3, tc->eps);
    } else {
      /* gemma 1: n2 is exactly pre MLP norm */
      rmsnorm(x5, x5, layer.n2, tc->eps);
    }
    if (is_interrupted()) return 1;

    /* MLP feedforward layer (SwiGLU-style). gate and up are independent and
     * share x, so they run as one fused gemv like q/k/v above. */
    if (!quant)
    {
      GemvJob jobs[2] = {{layer.w2.fpx, NULL, tb->xg_.data(), tc->mlp_dim},
                         {layer.w1.fpx, NULL, tb->xu.data(), tc->mlp_dim}};
      gemv_fpx_multi(jobs, tb->x.data(), dim);
    }
    else
    {
      floatx x_scale = quantize_act(tb->x_i8.data(), tb->x.data(), dim);
      GemvJob jobs[2] = {
          {layer.w2.i8.q, layer.w2.i8.scales, tb->xg_.data(), tc->mlp_dim},
          {layer.w1.i8.q, layer.w1.i8.scales, tb->xu.data(), tc->mlp_dim}};
      gemv_int8_multi(jobs, tb->x_i8.data(), x_scale, dim);
    }
    if (is_interrupted()) return 1;

    /* GELU gate, fused with the up projection (xg * xu) */
    std::mdspan xg = std::submdspan(tb->xg, std::pair { 0, 1 }, std::full_extent);
    gelu(xg, xg, NULL, tb->xu.data());
    if (is_interrupted()) return 1;

    if (!quant)
    {
      gemv_fpx(
        tb->x.data(), std::mdspan(layer.w3.fpx, dim, tc->mlp_dim), tb->xg_.data()
      );
    }
    else
    {
      floatx xscale = quantize_act(tb->xg_i8.data(), tb->xg_.data(), tc->mlp_dim);
      gemv_int8(
        tb->x.data(), layer.w3.i8.q, layer.w3.i8.scales, tb->xg_i8.data(), xscale,
        dim, tc->mlp_dim
      );
    }
    if (is_interrupted()) return 1;

    /* post feedforward RMSNorm */
    if (tc->mlp_pst)
    {
      std::mdspan x(tb->x.data(), 1, dim);
      rmsnorm(x, x, layer.n4, tc->eps);
    }
    if (is_interrupted()) return 1;

    /* second residual */
    x     = tb->x.data();
    resid = tb->resid.data();
    for (int d = 0; d < dim; d++)
    {
      tb->x[d] = std::clamp<floatx>(x[d] + resid[d], -FLOATX_MAX, FLOATX_MAX);
    }
    if (is_interrupted()) return 1;
  }

  /* final RMSNorm */
  std::mdspan x(tb->x.data(), 1, dim);
  rmsnorm(x, x, dec->fnorm, tc->eps);
  if (is_interrupted()) return 1;

  /* compute logits (tied embedding) */
  if (do_logit)
  {
    if (!quant)
    {
      gemv_fpx(
        tb->logits.data(), std::mdspan(dec->emb.fpx, tc->nvocab, dim), tb->x.data()
      );
    }
    else
    {
      floatx xscale = quantize_act(tb->x_i8.data(), tb->x.data(), dim);
      gemv_int8(
        tb->logits.data(), dec->emb.i8.q, dec->emb.i8.scales,
        tb->x_i8.data(), xscale, tc->nvocab, dim
      );
    }
    if (is_interrupted()) return 1;

    /* optional logit softcapping */
    if (tc->sc_log != 0.0f)
    {
      for (uint32_t d = 0; d < tc->nvocab; d++)
      {
        float val       = (float)tb->logits[d] / tc->sc_log;
        tb->logits[d] = (floatx)(tanhf(val) * tc->sc_log);
      }
    }
    if (is_interrupted()) return 1;
  }
  return 0;
}

/* language model forward (a chunk of tokens) */
static int
forward_text_chunk(
  TextDecoder *dec,
  TextBuffer  *tb,
  int          spos,
  int          ntok,
  bool         mask,
  bool         quant,
  bool         do_logit
)
{
  TextConfig *tc = &dec->config;

  if (quant && (tb->xsc.data() == NULL || tb->xosc.data() == NULL ||
                 tb->xgsc.data() == NULL))
  {
    log_error("scale buffers are not allocated");
    return 1;
  }

  int dim = tc->dim;
  int n_heads = tc->n_heads;
  int nkv = tc->nkv;
  int hdim = tc->hdim;
  int q_sz = n_heads * hdim;
  int kv_sz = nkv * hdim;

  int epos = spos + ntok - 1;

  if (epos >= tb->clen)
  {
    log_error("kv cache is full");
    return 1;
  }

  /* precompute RoPE angles */
  rope_freqs(tb->cs_slid.data(), tb->cs_full.data(), hdim, /*fstr=*/hdim, tc->theta_l,
             tc->theta_g, spos, ntok);
  if (is_interrupted()) return 1;

  floatx asc = (floatx)(1.0f / sqrtf((float)tc->q_scale));

  for (const auto &[l, layer] : std::views::enumerate(dec->layers))
  {
    memcpy(tb->resid.data(), tb->x.data(), ntok * dim * sizeof(*tb->x.data()));

    int t;
    std::mdspan x_mdspan1(tb->x.data(), ntok, dim);
    rmsnorm(x_mdspan1, x_mdspan1, layer.n1, tc->eps);
    if (is_interrupted()) return 1;

    /* the attention block */

    if (quant)
    {
      quantize_acts(std::mdspan(tb->x_i8.data(), ntok, dim), tb->xsc.data(), x_mdspan1);
    }

    /* compute xq & xk & xv. an output row only depends on the matching row
     * of x, so the work also splits over token blocks: one task per (tensor
     * head, token block) pair. splitting by head alone leaves most threads
     * idle on the small models (4 heads against 10 threads). */
    int  h;
    bool quit = false;

    const int rblock = 128;
    int n_rb = (ntok + rblock - 1) / rblock;
    int n_task = (n_heads + 2 * nkv) * n_rb;
    int idx;

    for (idx = 0; idx < n_task; idx++) {
      h = idx / n_rb; /* 0..n_heads-1: q, then k, then v */

      const QTensor *w;
      floatx *dst;
      int hd;
      if (h < n_heads) {
        w = &layer.wq;
        hd = h;
        dst = tb->xq.data();
      } else if (h < n_heads + nkv) {
        w = &layer.wk;
        hd = h - n_heads;
        dst = tb->xk.data();
      } else {
        w = &layer.wv;
        hd = h - n_heads - nkv;
        dst = tb->xv.data();
      }

      int t0 = (idx % n_rb) * rblock;
      int tn = std::min(rblock, ntok - t0);

      /* (tn, hdim) = x[t0:t0+tn] (tn, dim) @ w[hd]^T */
      floatx *dst_b = dst + ((size_t)hd * ntok + t0) * hdim;
      size_t woff = (size_t)hd * hdim * dim;

      if (!quant) {
        if (gemm_fpx(gemm_fpx_dst_view(dst_b, tn, hdim, hdim), gemm_fpx_input_view(w->fpx + woff, hdim, dim, 0), gemm_fpx_input_view(tb->x.data() + (size_t)t0 * dim, tn, dim, 0)) == 1)
          quit = true;
      } else {
        if (gemm_int8(std::mdspan(dst_b, tn, hdim), std::mdspan(w->i8.q + woff, hdim, dim), w->i8.scales + hd * hdim,
                      std::mdspan(tb->x_i8.data() + (size_t)t0 * dim, tn, dim), tb->xsc.data() + t0) == 1)
          quit = true;
      }
    }
    if (quit || is_interrupted())
      return 1;

    /* optional q & k norm */
    if (tc->qk_norm)
    {
      int h;
      std::mdspan xq(tb->xq.data(), n_heads, ntok, hdim);
      std::mdspan xk(tb->xk.data(), nkv, ntok, hdim);
      // Normalize each token in this head exactly once
      for (h = 0; h < n_heads; h++)
      {
        auto xq_ = std::submdspan(xq, h, std::full_extent, std::full_extent);
	rmsnorm(xq_, xq_, layer.nq, tc->eps);
      }
      for (h = 0; h < nkv; h++)
      {
        auto xk_ = std::submdspan(xk, h, std::full_extent, std::full_extent);
	rmsnorm(xk_, xk_, layer.nk, tc->eps);
      }
    }
    if (is_interrupted()) return 1;

    bool sliding = tc->swa[l];
    floatx *cfreqs = sliding ? tb->cs_slid.data() : tb->cs_full.data();

     /* RoPE. the attention scale is applied beforehand to the queries here,
     * mathematically equivilant, but avoids scaling the entire
     * ntok * max_k attention matrix */

    for (h = 0; h < n_heads + nkv; h++) {
      bool is_q = h < n_heads;
      floatx *data = is_q ? tb->xq.data() + h * ntok * hdim
                          : tb->xk.data() + (h - n_heads) * ntok * hdim;

      rope_apply(data, ntok, hdim, cfreqs, /*fstr=*/hdim,
                 is_q ? (float)asc : 1.0f);
    }
    
    if (is_interrupted()) return 1;

    /* (nkv, clen, hdim) */
    floatx *k_cache = tb->kvc.data() + (size_t)l * 2 * tb->clen * kv_sz;
    floatx *v_cache = k_cache + (size_t)tb->clen * kv_sz;

    /* write to kvc */
    for (int h = 0; h < nkv; h++) {
      floatx *xk_head =
          k_cache + (size_t)h * tb->clen * hdim + (size_t)spos * hdim;
      floatx *xv_head =
          v_cache + (size_t)h * tb->clen * hdim + (size_t)spos * hdim;
      memcpy(xk_head, tb->xk.data() + h * ntok * hdim, ntok * hdim * sizeof(*tb->xk.data()));
      memcpy(xv_head, tb->xv.data() + h * ntok * hdim, ntok * hdim * sizeof(*tb->xv.data()));
    }
    if (is_interrupted()) return 1;

    int max_k = epos + 1;
    quit = false;
    int qbi;

    /* one task per (head, query block) pair. splitting by head alone leaves
     * most threads idle on the small models (4 heads against 10 threads),
     * and a (head, query block) pair only ever writes its own rows of att[]
     * and xo[]. the dense path simply has a single block per head. */
    int qk = g_rt.qk_blk;
    int n_qb = mask ? (ntok + qk - 1) / qk : 1;
    int qidx;

    for (qidx = 0; qidx < n_heads * n_qb; qidx++) {
      h = qidx / n_qb;
      qbi = qidx % n_qb;

      int h_kv = h * nkv / n_heads; /* GQA mapping */

      /* (ntok, max_k) */
      floatx *att_h = tb->att.data() + h * ntok * max_k; /* att[h, :, :] */
      /* (clen, hdim) */
      floatx *xv_head = v_cache + (size_t)h_kv * tb->clen * hdim;
      floatx *xo_head =
          tb->xo.data() + h * hdim; /* (ntok, hdim), column stride q_sz */

      /* blockwise causal masking, the tiling algorithm used in the
       * FlashAttention paper (dao et al., 2022,
       * https://arxiv.org/abs/2205.14135).
       *
       * naively computing causal attention means either materializing the
       * full [ntok, attlen] score matrix and masking out the upper triangle
       * afterwards (wastes ~half the compute), or looping token by token with
       * a triangular schedule (ragged inner loop length basically kills
       * vectorization across threads). so I tile both the query and key range
       * into blocks and classify each (qb, kb) pair into five cases:
       *
       * kb > qb -> skip, every key in this block lies in the future of queries
       * kb < qb -> full GEMM, every key lies in the past of queries
       * kb = qb -> full GEMM & mask the upper triangle
       * kb < spos -> skip, every key in lies before the sliding window
       * kb = spos -> full GEMM & mask the lower triangle. */

      if (mask) /* only apply tiling if mask=true */
      {
        int q_start = qbi * qk;

        /* xq[h, q_start:q_end, :] (q_end - q_start, hdim) */
        floatx *qb = tb->xq.data() + h * ntok * hdim + q_start * hdim;
        int q_end = std::min(q_start + qk, ntok);
        int qabs_e = spos + q_end;
        int qabs_s = spos + q_start;
        if (sliding && qabs_s >= tc->slide) {
          qabs_s = qabs_s + 1 - tc->slide;
        } else {
          qabs_s = 0;
        }
        int spos_b = qabs_s / qk * qk;

        for (int k_start = spos_b; k_start < qabs_e; k_start += qk) {
          /* k_cache[h_kv, k_start:k_end, :] (k_end - k_start, hdim) */
          floatx *kb =
              k_cache + (size_t)h_kv * tb->clen * hdim + (size_t)k_start * hdim;
          int k_end = std::min(k_start + qk, max_k);
          floatx *att_b = att_h + q_start * max_k + k_start;

          /* typically people don't quantize this */
          if (gemm_fpx(gemm_fpx_dst_view(att_b, q_end - q_start, k_end - k_start, max_k), gemm_fpx_input_view(kb, k_end - k_start, hdim, 0), gemm_fpx_input_view(qb, q_end - q_start, hdim, 0)) == 1)
            quit = true;

          /* apply causal mask & sliding window mask */
          for (int qi = q_start; qi < q_end; qi++) {
            int pos_i = spos + qi;
            int spos_i =
                (sliding && pos_i >= tc->slide) ? (pos_i + 1 - tc->slide) : 0;
            for (int ki = k_start; ki < k_end; ki++) {
              if (ki > pos_i || ki < spos_i) {
                att_h[qi * max_k + ki] = 0.0f;
              }
            }
          }
        }
        /* softmax & softcap for the rows */
        for (int qi = q_start; qi < q_end; qi++) {
          floatx *att_row = att_h + qi * max_k;
          int pos_i = spos + qi;
          int spos_i =
              (sliding && pos_i >= tc->slide) ? (pos_i + 1 - tc->slide) : 0;

          /* optional tanh softcapping */
          if (tc->sc_att != 0.0f) {
            for (int t = spos_i; t <= pos_i; t++) {
              float val = (float)att_row[t] / tc->sc_att;
              att_row[t] = (floatx)(tanhf(val) * tc->sc_att);
            }
          }
          /* only softmax in the range [spos_i, pos_i] */
          softmax(att_row + spos_i, att_row + spos_i, pos_i - spos_i + 1);
        }

        /* xo_blk (qb_len, hdim) = att_blk (qb_len, qb_len)
         *                       @ v_block (qb_len, hdim) */
        floatx *xo_blk = xo_head + q_start * q_sz;
        floatx *att_blk = att_h + q_start * max_k + spos_b;
        floatx *v_block = xv_head + spos_b * hdim;

        gemm_fpx_nn(
          gemm_fpx_dst_view(xo_blk, q_end - q_start, hdim, q_sz),
          std::mdspan(v_block, qabs_e - spos_b, hdim),
          gemm_fpx_input_view(att_blk, q_end - q_start, qabs_e - spos_b, max_k)
        );
      } else { /* mask=false, take the dense path */
        /* (ntok, hdim) */
        floatx *xq_head = tb->xq.data() + h * ntok * hdim; /* xq[h, :, :] */
        /* k_cache[h_kv, :, :] (clen, hdim) */
        floatx *xk_head = k_cache + (size_t)h_kv * tb->clen * hdim;
        /* att_h = xq_head @ xk_head^T */
        if (gemm_fpx(gemm_fpx_dst_view(att_h, ntok, max_k, max_k), gemm_fpx_input_view(xk_head, max_k, hdim, 0), gemm_fpx_input_view(xq_head, ntok, hdim, 0)))
          quit = true;

        for (int qi = 0; qi < ntok; qi++) {
          floatx *att_row = att_h + qi * max_k;

          /* optional tanh softcapping */
          for (int t = 0; t < max_k; t++) {
            if (tc->sc_att != 0.0f) {
              float val = (float)att_row[t] / tc->sc_att;
              att_row[t] = (floatx)(tanhf(val) * tc->sc_att);
            }
          }
          /* softmax */
          softmax(att_row, att_row, max_k);
        }
        /* xo_head (ntok, hdim) = att_h (ntok, max_k)
         *                      @ xv_head[:max_k] (max_k, hdim) */
        gemm_fpx_nn(
          gemm_fpx_dst_view(xo_head, ntok, hdim, q_sz),
          std::mdspan(xv_head, max_k, hdim), std::mdspan(att_h, ntok, max_k)
        );
      }
    }
    if (is_interrupted()) return 1;
    /* x (ntok, dim) = xo_head (ntok, hdim) @ wo^T (hdim, dim) */
    if (!quant) {
      if (gemm_fpx(gemm_fpx_dst_view(tb->x.data(), ntok, dim, 0), gemm_fpx_input_view(layer.wo.fpx, dim, q_sz, 0), gemm_fpx_input_view(tb->xo.data(), ntok, q_sz, q_sz)) == 1)
        return 1;
    } else {
      quantize_acts(std::mdspan(tb->xo_i8.data(), ntok, q_sz), tb->xosc.data(),
                    std::mdspan(tb->xo.data(), ntok, q_sz));
      if (gemm_int8(std::mdspan(tb->x.data(), ntok, dim), std::mdspan(layer.wo.i8.q, dim, q_sz), layer.wo.i8.scales,
                    std::mdspan(tb->xo_i8.data(), ntok, q_sz), tb->xosc.data()) == 1)
        return 1;
    }
    if (is_interrupted()) return 1;

    if (tc->mlp_pre) {
      /* gemma 2/3: post-norm on attention output */
      std::mdspan x_mdspan(tb->x.data(), ntok, dim);
      rmsnorm(x_mdspan, x_mdspan, layer.n2, tc->eps);
      if (is_interrupted()) return 1;
    }

    floatx *RESTRICT x     = tb->x.data();
    floatx *RESTRICT resid = tb->resid.data();

    int d;
    for (d = 0; d < ntok * dim; d++)
    {
      /* combine the residual stream */
      tb->x[d] = std::clamp<floatx>(x[d] + resid[d], -FLOATX_MAX, FLOATX_MAX);
    }
    if (is_interrupted()) return 1;

    memcpy(tb->resid.data(), tb->x.data(), ntok * dim * sizeof(*tb->x.data()));

    std::mdspan x6(tb->x.data(), ntok, dim);
    if (tc->mlp_pre) {
      /* gemma 2/3: dedicated pre_feedforward_rmsnorm */
      rmsnorm(x6, x6, layer.n3, tc->eps);
    } else {
      /* gemma 1: post_attention_rmsnorm is pre-MLP norm */
      rmsnorm(x6, x6, layer.n2, tc->eps);
    }
    if (is_interrupted()) return 1;

    /* MLP */
    std::mdspan xg = std::submdspan(tb->xg, std::pair { 0, ntok }, std::full_extent);
    if (!quant) {
      if (gemm_fpx(gemm_fpx_dst_view(tb->xu.data(), ntok, tc->mlp_dim, 0), gemm_fpx_input_view(layer.w1.fpx, tc->mlp_dim, dim, 0), gemm_fpx_input_view(tb->x.data(), ntok, dim, 0)) == 1 ||
          gemm_fpx(xg, gemm_fpx_input_view(layer.w2.fpx, tc->mlp_dim, dim, 0), gemm_fpx_input_view(tb->x.data(), ntok, dim, 0)) == 1)
        return 1;
    } else {
      quantize_acts(std::mdspan(tb->x_i8.data(), ntok, dim), tb->xsc.data(), x_mdspan1);
      if (gemm_int8(std::mdspan(tb->xu.data(), ntok, tc->mlp_dim), std::mdspan(layer.w1.i8.q, tc->mlp_dim, dim), layer.w1.i8.scales,
                    std::mdspan(tb->x_i8.data(), ntok, dim), tb->xsc.data()) == 1 ||
          gemm_int8(xg, std::mdspan(layer.w2.i8.q, tc->mlp_dim, dim), layer.w2.i8.scales,
                    std::mdspan(tb->x_i8.data(), ntok, dim), tb->xsc.data()))
        return 1;
    }
    if (is_interrupted()) return 1;

    /* GELU gate, fused with the up projection (xg * xu) */
    gelu(xg, xg, NULL, tb->xu.data());
    if (is_interrupted())
      return 1;

    /* down projection */
    if (!quant) {
      if (gemm_fpx(gemm_fpx_dst_view(tb->x.data(), ntok, dim, 0), gemm_fpx_input_view(layer.w3.fpx, dim, tc->mlp_dim, 0), xg) == 1)
        return 1;
    } else {
      quantize_acts(std::mdspan(tb->xg_i8.data(), ntok, tc->mlp_dim), tb->xgsc.data(),
                    xg);
      if (gemm_int8(std::mdspan(tb->x.data(), ntok, dim), std::mdspan(layer.w3.i8.q, dim, tc->mlp_dim), layer.w3.i8.scales,
                    std::mdspan(tb->xg_i8.data(), ntok, tc->mlp_dim), tb->xgsc.data()) == 1)
        return 1;
    }
    if (is_interrupted()) return 1;

    if (tc->mlp_pst)
    {
      std::mdspan x(tb->x.data(), ntok, dim);
      rmsnorm(x, x, layer.n4, tc->eps);
    }
    if (is_interrupted()) return 1;

    x     = tb->x.data();
    resid = tb->resid.data();
    for (d = 0; d < ntok * dim; d++)
    {
      /* second residual */
      tb->x[d] = std::clamp<floatx>(x[d] + resid[d], -FLOATX_MAX, FLOATX_MAX);
    }
    if (is_interrupted()) return 1;
  }

  /* final RMSNorm */
  std::mdspan x(tb->x.data(), ntok, dim);
  rmsnorm(x, x, dec->fnorm, tc->eps);

  /* compute logits (tied embedding) */
  if (do_logit) {
    floatx *last_x = tb->x.data() + (ntok - 1) * dim;
    if (!quant) {
      gemv_fpx(
        tb->logits.data(), std::mdspan(dec->emb.fpx, tc->nvocab, dim), last_x);
    } else {
      floatx xscale = quantize_act(tb->x_i8.data(), last_x, dim);
      gemv_int8(tb->logits.data(), dec->emb.i8.q, dec->emb.i8.scales, tb->x_i8.data(),
                xscale, tc->nvocab, dim);
    }
    if (is_interrupted())
      return 1;

    /* optional logit softcapping */
    if (tc->sc_log != 0.0f) {
      for (uint32_t d = 0; d < tc->nvocab; d++) {
        float val = (float)tb->logits[d] / tc->sc_log;
        tb->logits[d] = (floatx)(tanhf(val) * tc->sc_log);
      }
    }
    if (is_interrupted())
      return 1;
  }
  return 0;
}

NO_FAST_MATH_END;

/* language + vision model forward */
int forward_gemma_decode(GemmaModel *model, TextBuffer *tb, int token, int pos) {
  TextDecoder *dec = model->decoder.get();
  int rc;

  floatx esc;
  if (dec->config.mlp_pre) {
    /* gemma 2/3 */
    esc = 1.0f;
  } else {
    /* gemma 1 */
    esc = (floatx)sqrtf((float)dec->config.dim);
  }
  if (model->quant) {
    /* dequantize */
    esc *= dec->emb.i8.scales[token];
  }

  int dim = dec->config.dim;

  /* x = emb[tok] * esc */
  int d;
  for (d = 0; d < dim; d++) {
    if (!model->quant) {
      tb->x[d] = dec->emb.fpx[token * dim + d] * esc;
    } else {
      tb->x[d] = (floatx)dec->emb.i8.q[token * dim + d] * esc;
    }
  }
  if (is_interrupted()) {
    rc = 1;
  } else {
    rc = forward_text_decode(model->decoder.get(), tb, pos, model->quant, true);
  }
  return rc;
}

/* language model prefill */
int
forward_gemma_prefill(
  GemmaModel *model,
  TextBuffer *tb,
  std::span<const int> tokens,
  int        *pos,
  size_t     chunk_size,
  std::span<uint8_t> seen,
  bool       do_logit
)
{
  TextDecoder *dec       = model->decoder.get();
  int          dim = dec->config.dim;

  /* prefill by chunks */
  for (size_t off = 0; off < tokens.size(); off += chunk_size)
  {
    size_t  cur_len       = std::min(chunk_size, tokens.size() - off);
    bool is_last_chunk = (off + cur_len == tokens.size());

    /* embedding lookup */
    for (int t = 0; t < cur_len; t++)
    {
      int token   = tokens[off + t];
      int emb_off = t * dim;

      seen[token] = 1;

      floatx esc;
      if (dec->config.mlp_pre) {
        /* gemma 2/3 */
        esc = 1.0f;
      } else {
        /* gemma 1 */
        esc = (floatx)sqrtf((float)dec->config.dim);
      }
      if (model->quant) {
        esc *= dec->emb.i8.scales[token];
      }
 
      for (int d = 0; d < dim; d++)
      {
        if (!model->quant)
        {
          tb->x[emb_off + d] = dec->emb.fpx[token * dim + d] * esc;
        }
        else
        {
          tb->x[emb_off + d] = (floatx)dec->emb.i8.q[token * dim + d] * esc;
        }
      }
    }

    int rc = forward_text_chunk(
      dec, tb, *pos + off, cur_len, true, model->quant,
      is_last_chunk && do_logit
    );

    if (rc != 0) return rc;
  }

  *pos += tokens.size();
  return 0;
}

/* vision model forward + embedding prefill */
int
forward_gemma_image(
  GemmaModel   *model,
  TextBuffer   *tb,
  VisionBuffer *vb,
  const floatx *image,
  int          *pos,
  bool          do_logit
)
{
  VisionEncoder *enc = model->encoder.get();
  if (enc == NULL)
  {
    return 1;
  }

  TextDecoder *dec = model->decoder.get();
  if (forward_vision(enc, &dec->config, tb, vb, image, model->quant) == 1)
  {
    return 1;
  }

  int img_tok = dec->config.img_tok;

  int suc = forward_text_chunk(
    /* gemma 3 models uses bi-directional attention for vision tokens */
    dec, tb, *pos, img_tok, false, model->quant, do_logit
  );

  if (suc == 0)
  {
    *pos += img_tok;
  }
  return suc;
}

// Sampling

/* */
static int
argmax(floatx *logits, int nvocab)
{
  // Pick the index with the max value
  int    max_idx = -1;
  floatx max_val = -FLOATX_MAX;
  {
    int    local_idx = -1;
    floatx local_val = -FLOATX_MAX;

    int i;
    for (i = 0; i < nvocab; i++)
    {
      if (logits[i] > local_val)
      {
        local_val = logits[i];
        local_idx = i;
      }
    }
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
apply_topk(floatx *logits, std::span<FloatIdx> logit_indices, int nvocab, int k)
{
  if (k <= 0)
  {
    k = 1;
  }
  if (k > nvocab)
  {
    k = nvocab;
  }

  // Record index info
  int i;
  for (i = 0; i < nvocab; i++)
  {
    logit_indices[i].idx = i;
    logit_indices[i].val = logits[i];
  }
  quickselect_topk(logit_indices.data(), 0, nvocab - 1, k - 1);

  // Keep the top k channels
  for (i = 0; i < nvocab; i++)
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
  int       nvocab,
  int       k,
  float     p
)
{
  if (k > nvocab)
  {
    k = nvocab;
  }
  // Softmax to get the probs, store in fpbuf
  softmax(fpbuf.data(), logits, nvocab);
  int heap_size = (k == 0) ? nvocab : k;

  if (k == 0)
  {
    int i;
    for (i = 0; i < nvocab; i++)
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
  memcpy(fpbuf.data(), logits, nvocab * sizeof(floatx));

  // Set logits to -inf
  int i;
  for (i = 0; i < nvocab; i++)
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
    sift_down(logit_indices.data(), heap_size, 0);         // O(log(nvocab))

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
  std::span<int> tokens;
  floatx *image;
} InjectData;

/* Sample the next token from the logits */
int
sample_from_logits(
  floatx   *logits,
  std::span<floatx> probs,
  std::span<FloatIdx> logit_indices,
  std::span<const uint8_t> visited,
  int       nvocab,
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
    return argmax(logits, nvocab);
  }

  bool use_topk = dosample && topk != 0;
  bool use_topp = dosample && topp < 1.0f;
  bool use_rpen = dosample && rpen > 1.0f;

  // Apply the temperature
  int d;
  for (d = 0; d < nvocab; d++)
  {
    logits[d] /= (floatx)temperature;
  }

  if (use_topk)
  {
    apply_topk(logits, logit_indices, nvocab, topk);
  }
  if (use_topp)
  {
    apply_topp(logits, probs, logit_indices, nvocab, topk, topp);
  }
  if (use_rpen)
  {
    apply_rpen(logits, visited, rpen);
  }

  // Softmax to get the probs
  softmax(probs.data(), logits, nvocab);

  // Multinomial sample from probs
  float r   = (float)((float)rand() / (RAND_MAX + 1.0));
  float sum = 0.0f;

  int token = nvocab - 1;
  for (int d = 0; d < nvocab; d++)
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
  TextBuffer   *tb,
  VisionBuffer *vb,
  const Options &options,
  std::function_ref<InjectData(int)> inject_callback
)
{
  TextConfig *tcfg       = &model->decoder->config;
  int         nvocab = tcfg->nvocab;

  double    prf_t0        = 0.0;
  double    prf_t1        = 0.0;
  double    gen_t0        = 0.0;
  double    gen_t1        = 0.0;
  int       pos           = 0;
  int       token         = 0;
  std::vector<uint8_t>    visited(nvocab, 0);
  std::vector<floatx>     probs(nvocab, 0.0);;
  std::vector<FloatIdx> logit_indices(nvocab);
  int       bos_tok       = 0;
  InjectData injected    = {};

  std::tuple<double, int, double, int> stats{};
  auto &[prefill_elapsed, prefill_toks, gen_elapsed, gen_toks] = stats;

  // `<bos>` is always the very first token of the sequence
  prf_t0      = now_sec();
  bos_tok = model->tokenizer->bos;
  if (forward_gemma_prefill(
        model, tb, std::span(&bos_tok, 1), &pos, options.chunk_size, visited, false
      ) == 1)
  {
    return stats;
  }
  prf_t1 = now_sec();
  prefill_elapsed += prf_t1 - prf_t0;
  prefill_toks = 1;

  injected = inject_callback(EOF);

  while (pos < options.seqlen && injected.type != INJECT_QUIT)
  {
    if (injected.type == INJECT_NONE)
    {
      // pure auto-regressive step
      gen_t0 = now_sec();
      if (forward_gemma_decode(model, tb, token, pos) == 1) return stats;
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
      InjectData next    = inject_callback(EOF);
      bool       is_last = (next.type == INJECT_DONE);

      int prev_pos = pos;
      prf_t0       = now_sec();

      if (injected.type == INJECT_TEXT)
      {
        if (forward_gemma_prefill(
              model, tb, injected.tokens, &pos, options.chunk_size,
              visited, is_last
            ) == 1)
        {
          return stats;
        }
      }
      else if (injected.type == INJECT_IMAG)
      {
        if (forward_gemma_image(
              model, tb, vb, injected.image, &pos, is_last
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
      tb->logits.data(), probs, logit_indices, visited, nvocab, options.temperature,
      options.topk, options.topp, options.rpen
    );
    visited[token] = 1;
    gen_toks++;

    gen_t1 = now_sec();
    gen_elapsed += gen_t1 - gen_t0;

    injected = inject_callback(token);
  }
  return stats;
}

/* */
struct CommandContext
{
  std::vector<floatx> img;
  std::string arg;

  // Used in pan & scan
  std::string    path;
  std::vector<floatx> crops;
  int     n_crops = 0;
};

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
  std::generator<InjectData> (*inject)(
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
  std::string_view text;
  CommandContext cmd_ctx;
  int           *tokens_buf;
  int            tokens_len;
};

/* */
struct ChatContext
{
  std::vector<char> line_buf;
  InjectContext *jctx;
  ChatContext(size_t line_cap) : line_buf(line_cap) {}
};

struct InjectStream
{
  std::generator<InjectData> values;
  std::ranges::iterator_t<std::generator<InjectData>> current;

  InjectStream() = default;
  InjectStream(const InjectStream &) = delete;
  InjectStream &operator=(const InjectStream &) = delete;

  InjectStream(InjectStream &&other) noexcept = default;

  InjectStream &operator=(InjectStream &&other) noexcept = default;

  InjectData next()
  {
    if (current == values.end())
      return InjectData{.type = INJECT_DONE};

    auto ret = *current;
    ++current;
    return ret;
  }
  InjectStream(std::generator<InjectData> values) : values(std::move(values)), current(this->values.begin()) {}
};

/* */
bool
image_should_ignore(GemmaModel *model, bool enable_mm)
{
  (void)model;
  return !(model->support_mm && enable_mm);
}

/* */
std::generator<InjectData>
image_inject(GemmaModel *model, InjectContext *ctx, bool enable_mm)
{
  /* Template:
   * \n\n<start_of_image>[image soft tokens]<end_of_image>\n\n */

  VisionEncoder  *enc     = model->encoder.get();
  GemmaTokenizer *tok     = model->tokenizer.get();
  CommandContext *cmd_ctx = &ctx->cmd_ctx;

  int  spos, epos;
  bool use_mm = model->support_mm && enable_mm;
  int img_px = (enc && use_mm) ? enc->config.img_px : 0;

  spos = ctx->tokens_len;
  ctx->tokens_len += encode(tok, "\n\n", ctx->tokens_buf + spos);
  ctx->tokens_buf[ctx->tokens_len++] = tok->soi;
  epos = ctx->tokens_len;
  co_yield InjectData{.type = INJECT_TEXT,
    .tokens = std::span(ctx->tokens_buf + spos, epos - spos)};

  cmd_ctx->img = prepare_image(cmd_ctx->arg, img_px);
  if (cmd_ctx->img.empty())
  {
    log_error("failed to prepare image: '%s'", cmd_ctx->arg.c_str());
    co_yield InjectData{.type = INJECT_QUIT};
    co_return;
  }
  co_yield InjectData{.type = INJECT_IMAG, .image = cmd_ctx->img.data()};

  spos = ctx->tokens_len;
  ctx->tokens_buf[ctx->tokens_len++] = tok->eoi;
  ctx->tokens_len += encode(tok, "\n\n", ctx->tokens_buf + ctx->tokens_len);
  epos = ctx->tokens_len;
  co_yield InjectData{.type = INJECT_TEXT,
    .tokens = std::span(ctx->tokens_buf + spos, epos - spos)};
}

/* */
bool
longimage_should_ignore(GemmaModel *model, bool enable_mm)
{
  (void)model;
  return !(model->support_mm && enable_mm);
}

/* */
std::generator<InjectData>
longimage_inject(GemmaModel *model, InjectContext *ctx, bool enable_mm)
{
  VisionEncoder *enc = model->encoder.get();
  GemmaTokenizer *tok = model->tokenizer.get();
  CommandContext *cmd = &ctx->cmd_ctx;
  bool use_mm = model->support_mm && enable_mm;
  int img_px = (enc && use_mm) ? enc->config.img_px : 0;
  int spos, epos;

  spos = ctx->tokens_len;
  ctx->tokens_len += encode(tok, CROPPED_IMAGE_PREFIX, ctx->tokens_buf + spos);
  ctx->tokens_len += encode(tok, "\n\n", ctx->tokens_buf + ctx->tokens_len);
  ctx->tokens_buf[ctx->tokens_len++] = tok->soi;
  epos = ctx->tokens_len;
  co_yield InjectData{.type = INJECT_TEXT,
    .tokens = std::span(ctx->tokens_buf + spos, epos - spos)};

  cmd->path = cmd->arg;
  cmd->crops = prepare_longimage(cmd->path, img_px, MIN_CROP_SIZE,
    MAX_NUM_CROPS, &cmd->n_crops);
  if (cmd->crops.empty())
  {
    log_error("failed to prepare image: '%s'", cmd->path.c_str());
    co_yield InjectData{.type = INJECT_QUIT};
    co_return;
  }
  co_yield InjectData{.type = INJECT_IMAG, .image = cmd->crops.data()};

  spos = ctx->tokens_len;
  ctx->tokens_buf[ctx->tokens_len++] = tok->eoi;
  ctx->tokens_len += encode(tok, "\n\n", ctx->tokens_buf + ctx->tokens_len);
  if (cmd->n_crops <= 1)
  {
    epos = ctx->tokens_len;
    co_yield InjectData{.type = INJECT_TEXT,
      .tokens = std::span(ctx->tokens_buf + spos, epos - spos)};
    co_return;
  }

  ctx->tokens_len += encode(tok, CROPPED_IMAGE_FILTER, ctx->tokens_buf + ctx->tokens_len);
  ctx->tokens_len += encode(tok, "\n\n", ctx->tokens_buf + ctx->tokens_len);
  ctx->tokens_buf[ctx->tokens_len++] = tok->soi;
  epos = ctx->tokens_len;
  co_yield InjectData{.type = INJECT_TEXT,
    .tokens = std::span(ctx->tokens_buf + spos, epos - spos)};

  for (int crop_i = 1; crop_i < cmd->n_crops; ++crop_i)
  {
    if (crop_i != 1)
    {
      spos = ctx->tokens_len;
      ctx->tokens_len += encode(tok, "\n\n", ctx->tokens_buf + spos);
      ctx->tokens_buf[ctx->tokens_len++] = tok->soi;
      epos = ctx->tokens_len;
      co_yield InjectData{.type = INJECT_TEXT,
        .tokens = std::span(ctx->tokens_buf + spos, epos - spos)};
    }
    co_yield InjectData{.type = INJECT_IMAG,
      .image = cmd->crops.data() + (size_t)crop_i * img_px * img_px * 3};
    spos = ctx->tokens_len;
    ctx->tokens_buf[ctx->tokens_len++] = tok->eoi;
    ctx->tokens_len += encode(tok, "\n\n", ctx->tokens_buf + ctx->tokens_len);
    epos = ctx->tokens_len;
    co_yield InjectData{.type = INJECT_TEXT,
      .tokens = std::span(ctx->tokens_buf + spos, epos - spos)};
  }
}

/* */
CommandType command_types[] = {
  {
    .name          = "image",
    .id            = COMMAND_IMAGE,
    .should_ignore = image_should_ignore,
    .inject   = image_inject,
  },
  {
    .name          = "longimage",
    .id            = COMMAND_LONGIMAGE,
    .should_ignore = longimage_should_ignore,
    .inject   = longimage_inject,
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
std::generator<InjectData>
inject_next(GemmaModel *model, InjectContext *ctx, bool enable_mm)
{
  GemmaTokenizer *tok = model->tokenizer.get();
  while (!ctx->text.empty())
  {
    ScanResult event = scan_next_event(ctx->text);
    if (std::holds_alternative<std::monostate>(event.value)) co_return;
    if (auto text = std::get_if<std::string_view>(&event.value))
    {
      int spos = ctx->tokens_len;
      ctx->tokens_len += encode(tok, *text, ctx->tokens_buf + spos);
      int epos = ctx->tokens_len;
      co_yield InjectData{.type = INJECT_TEXT,
        .tokens = std::span(ctx->tokens_buf + spos, epos - spos)};
      ctx->text = event.remaining;
      continue;
    }

    CommandRecord command = std::get<CommandRecord>(event.value);
    if (command.type.should_ignore(model, enable_mm))
    {
      int spos = ctx->tokens_len;
      ctx->tokens_len += encode(tok, command.raw, ctx->tokens_buf + spos);
      int epos = ctx->tokens_len;
      co_yield InjectData{.type = INJECT_TEXT,
        .tokens = std::span(ctx->tokens_buf + spos, epos - spos)};
      ctx->text = event.remaining;
      continue;
    }

    ctx->cmd_ctx = CommandContext{.arg = std::string(command.arg)};
    auto command_values = command.type.inject(model, ctx, enable_mm);
    for (InjectData value : command_values)
      co_yield value;
    ctx->text = event.remaining;
  }
}

/* */
std::tuple<double, int, double, int>
generate(
  // Input parameters
  GemmaModel   *model,
  TextBuffer   *tb,
  VisionBuffer *vb,
  const Options& options
)
{
  fputs(options.prompt.c_str(), stdout);

  int  tokens_cap = options.seqlen * model->tokenizer->maxlen;
  std::vector<int> tokens_buf(tokens_cap);

  InjectContext ctx = {
    .text = options.prompt,
    .tokens_buf = tokens_buf.data(),
    .tokens_len = 0
  };

  auto generate_inject = [&, stream = InjectStream(inject_next(model, &ctx, options.enable_mm))](int token) mutable -> InjectData {
    GemmaTokenizer *tok = model->tokenizer.get();
    if (token == EOF)
    {
      // Request for prefilling
      return stream.next();
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
    model, tb, vb, options, 
    generate_inject
  );
}

/* Build the Gemma chat template */
static std::generator<InjectData>
new_turn(GemmaModel *model, bool enable_mm, ChatContext *ctx)
{
  /* Template (from https://ai.google.dev/gemma/docs/core/prompt-structure):
   * <start_of_turn>user\n
   * What is Cramer's Rule?<end_of_turn>\n
   * <start_of_turn>model */

  GemmaTokenizer *tok  = model->tokenizer.get();
  InjectContext  *jctx = ctx->jctx;

  int spos, epos;
  std::print("\n> ");
  if (fgets(ctx->line_buf.data(), ctx->line_buf.size(), stdin) == NULL)
  {
    if (!is_interrupted())
    {
#if defined(_WIN32)
      Sleep(10);
#endif
    }
    if (!is_interrupted()) log_error("failed to read user input");
    co_yield InjectData{.type = INJECT_QUIT};
    co_return;
  }
  if (strchr(ctx->line_buf.data(), '\n') == NULL)
  {
    log_error("input too long");
    co_yield InjectData{.type = INJECT_QUIT};
    co_return;
  }
  if (is_interrupted())
  {
    co_yield InjectData{.type = INJECT_QUIT};
    co_return;
  }
  ctx->line_buf[strcspn(ctx->line_buf.data(), "\n")] = '\0';

  spos = jctx->tokens_len;
  jctx->tokens_buf[jctx->tokens_len++] = tok->sot;
  jctx->tokens_len += encode(tok, "user\n", jctx->tokens_buf + jctx->tokens_len);
  epos = jctx->tokens_len;
  co_yield InjectData{.type = INJECT_TEXT,
    .tokens = std::span(jctx->tokens_buf + spos, epos - spos)};

  jctx->text = ctx->line_buf.data();
  for (InjectData value : inject_next(model, jctx, enable_mm))
    co_yield value;

  spos = jctx->tokens_len;
  if (!jctx->text.empty())
    jctx->tokens_len += encode(tok, jctx->text, jctx->tokens_buf + spos);
  jctx->tokens_buf[jctx->tokens_len++] = tok->eot;
  jctx->tokens_buf[jctx->tokens_len++] = get_token_idx(tok, "\n");
  jctx->tokens_buf[jctx->tokens_len++] = tok->sot;
  jctx->tokens_len += encode(tok, "model\n", jctx->tokens_buf + jctx->tokens_len);
  epos = jctx->tokens_len;
  co_yield InjectData{.type = INJECT_TEXT,
    .tokens = std::span(jctx->tokens_buf + spos, epos - spos)};
}

/* */
std::tuple<double, int, double, int>
chat(
  // Input parameters
  GemmaModel   *model,
  TextBuffer   *tb,
  VisionBuffer *vb,
  const Options& options
)
{

  int max_toklen = model->tokenizer->maxlen;
  int tokens_cap = options.seqlen * max_toklen;
  int line_cap   = options.seqlen * max_toklen;
  InjectContext jctx = {};
  ChatContext ctx(line_cap);

  std::vector<int> tokens_buf(tokens_cap);

  jctx = (InjectContext){
    .tokens_buf = tokens_buf.data(),
    .tokens_len = 0
  };
  ctx.jctx = &jctx;

  auto chat_inject = [&, stream = InjectStream(new_turn(model, options.enable_mm, &ctx))](int token) mutable -> InjectData
  {
    GemmaTokenizer *tok = model->tokenizer.get();

    if (token == tok->eos || token == tok->eot)
    {
      // Model completed the turn
      stream = InjectStream(new_turn(model, options.enable_mm, &ctx));
      return stream.next();
    }
    if (token == EOF)
    {
      // Requesting more injection data
      return stream.next();
    }

    char byte_buf[2];
    fputs(decode(tok, token, byte_buf), stdout);
    fflush(stdout);
    return (InjectData){.type = INJECT_NONE};
  };

  return sample(
    model, tb, vb, options,
    chat_inject
  );
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
  double kvc_t_KB = tcfg->nl * 2 * tcfg->nkv * tcfg->hdim *
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

  const char* modelfile = options.modelfile.c_str();
  std::unique_ptr<GemmaModel> model = 
    options.use_mmap ? mmap_gemma_model(modelfile, options.enable_mm)
             : read_gemma_model(modelfile, options.enable_mm);
  if (!model) return 1;

  const TextConfig &tcfg = model->decoder->config;
  bool use_mm = options.enable_mm && model->support_mm;
  VisionConfig *vcfg = model->encoder ? &model->encoder->config : nullptr;
  TextBuffer tb(tcfg, vcfg != nullptr, options.seqlen, options.chunk_size, use_mm, model->quant);

  std::unique_ptr<VisionBuffer> vb;
  if (use_mm)
  {
    vb = std::make_unique<VisionBuffer>(*vcfg, model->quant);
  }

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
        model.get(), &tb, vb.get(),
        options
      )
    : generate(
        model.get(), &tb, vb.get(),
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

