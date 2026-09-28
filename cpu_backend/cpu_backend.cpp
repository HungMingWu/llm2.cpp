module;
#include <immintrin.h>
#include <cstddef>
#include <cmath>

module cpu_backend;
import base;

// Multiplies dynamically quantized activations by packed int8 weights in blocks of 16
// output rows. VNNI handles eight input rows at once while AVX2 handles four.
#if defined(__AVX512VNNI__)
static inline __attribute__((always_inline)) void matmul_block(
    float *output, const int8_t *input_q, const float *input_scales,
    const Tensor *weight, size_t rows, size_t output_block) {
    const size_t block_rows = 16;
    size_t groups_per_row = (size_t)weight->shape[1] / 64;
    const int8_t *packed_weights = (const int8_t *)weight->data + output_block * block_rows * weight->shape[1];
    for (size_t row_start = 0; row_start < rows; row_start += 8) {
        size_t active_rows = rows - row_start < 8 ? rows - row_start : 8;
        __m512 result[8] = {0};
        for (size_t group = 0; group < groups_per_row; group++) {
            __m512i dot[8] = {0};
            __m512i correction = _mm512_setzero_si512();
            __m512i offset_bytes = _mm512_set1_epi8(-128); // Flip signed activations into the unsigned range required by VNNI.
            for (int chunk = 0; chunk < 16; chunk++) {
                __m512i weight_values = _mm512_load_si512((const __m512i *)(packed_weights + group * 1024 + chunk * 64));
                correction = _mm512_dpbusd_epi32(correction, offset_bytes, weight_values);
                for (size_t row = 0; row < active_rows; row++) {
                    __m512i input_values = _mm512_broadcastd_epi32(
                        _mm_loadu_si32(input_q + (row_start + row) * weight->shape[1] + group * 64 + chunk * 4));
                    input_values = _mm512_xor_si512(input_values, offset_bytes);
                    dot[row] = _mm512_dpbusd_epi32(dot[row], input_values, weight_values);
                }
            }
            __m512 weight_scales = _mm512_cvtph_ps(_mm256_loadu_si256((const __m256i *)(weight->scales + (output_block * groups_per_row + group) * block_rows)));
            for (size_t row = 0; row < active_rows; row++)
                result[row] = _mm512_fmadd_ps(_mm512_cvtepi32_ps(_mm512_sub_epi32(dot[row], correction)), _mm512_mul_ps(weight_scales, _mm512_set1_ps(input_scales[(row_start + row) * groups_per_row + group])), result[row]);
        }
        for (size_t row = 0; row < active_rows; row++)
            _mm512_storeu_ps(output + (row_start + row) * weight->shape[0] + output_block * block_rows, result[row]);
    }
}
#else
static inline __attribute__((always_inline)) void matmul_block(
    float *output, const int8_t *input_q, const float *input_scales,
    const Tensor *weight, size_t rows, size_t output_block) {
    const size_t block_rows = 16;
    size_t groups_per_row = (size_t)weight->shape[1] / 64;
    const int8_t *packed_weights = (const int8_t *)weight->data + output_block * block_rows * weight->shape[1];
    for (size_t row_start = 0; row_start < rows; row_start += 4) {
        size_t active_rows = rows - row_start < 4 ? rows - row_start : 4;
        for (int half = 0; half < 2; half++) {
            __m256 result[4] = {0};
            for (size_t group = 0; group < groups_per_row; group++) {
                __m256i dot[4] = {0};
                for (int chunk = 0; chunk < 16; chunk++) {
                    __m256i weight_values = _mm256_loadu_si256((const __m256i *)(packed_weights + group * 1024 + chunk * 64 + half * 32));
                    __m256i weight_magnitudes = _mm256_abs_epi8(weight_values);
                    for (size_t row = 0; row < active_rows; row++) {
                        __m256i input_values = _mm256_broadcastd_epi32(
                            _mm_loadu_si32(input_q + (row_start + row) * weight->shape[1] + group * 64 + chunk * 4));
                        dot[row] = _mm256_add_epi32(dot[row], _mm256_madd_epi16(_mm256_maddubs_epi16(weight_magnitudes, _mm256_sign_epi8(input_values, weight_values)), _mm256_set1_epi16(1)));
                    }
                }
                __m256 weight_scales = _mm256_cvtph_ps(_mm_loadu_si128((const __m128i *)(weight->scales + (output_block * groups_per_row + group) * block_rows + half * 8)));
                for (size_t row = 0; row < active_rows; row++)
                    result[row] = _mm256_fmadd_ps(_mm256_cvtepi32_ps(dot[row]), _mm256_mul_ps(weight_scales, _mm256_set1_ps(input_scales[(row_start + row) * groups_per_row + group])), result[row]);
            }
            for (size_t row = 0; row < active_rows; row++)
                _mm256_storeu_ps(output + (row_start + row) * weight->shape[0] + output_block * block_rows + half * 8, result[row]);
        }
    }
}
#endif

void matmul_int8(float *output, const int8_t *input_q, const float *input_scales, const Tensor *weight, size_t rows) {
    const size_t block_rows = 16;
    #pragma omp for schedule(static)
    for (size_t output_block = 0; output_block < (size_t)weight->shape[0] / block_rows; output_block++)
        matmul_block(output, input_q, input_scales, weight, rows, output_block);
}

void rmsnorm(float *output, const float *input, const Tensor *weight, int width, float epsilon, size_t row_count) {
    const float *weights = weight ? (const float *)weight->data : NULL;
    #pragma omp for schedule(static)
    for (size_t row = 0; row < row_count; row++) {
        const float *input_row = input + row * width;
        float *output_row = output + row * width;
        float sum_squares = 0.0f;
        for (int i = 0; i < width; i++)
            sum_squares += input_row[i] * input_row[i];
        float inverse_rms = 1.0f / sqrtf(sum_squares / (float)width + epsilon);
        for (int i = 0; i < width; i++)
            output_row[i] = (weights ? weights[i] : 1.0f) * (inverse_rms * input_row[i]);
    }
}

void add_and_scale(float *output, const float *addend, size_t count, float scale) {
    #pragma omp for schedule(static)
    for (size_t i = 0; i < count; i++) output[i] = (output[i] + addend[i]) * scale;
}

