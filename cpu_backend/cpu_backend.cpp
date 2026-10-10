module;
#include <cassert>
#include <cmath>
#include <cstddef>
#include <immintrin.h>
#include <mdspan>
#include <ranges>
#include <simd>

module cpu_backend;
import base;

using f32x8 = std::simd::vec<float, 8>;
using f32x4 = std::simd::vec<float, 4>;

// workaround for std::simd::fma
auto fma(auto a, auto b, auto c) {
    return a * b + c;
}

// Multiplies dynamically quantized activations by packed int8 weights in blocks of 16
// output rows. VNNI handles eight input rows at once while AVX2 handles four.
#if defined(__AVX512VNNI__)
static inline __attribute__((always_inline)) void matmul_block(float* output, const int8_t* input_q,
                                                               const float* input_scales,
                                                               const Tensor* weight, size_t rows,
                                                               size_t output_block) {
    const size_t block_rows = 16;
    size_t groups_per_row = (size_t)weight->shape[1] / 64;
    const int8_t* packed_weights =
        (const int8_t*)weight->data + output_block * block_rows * weight->shape[1];
    for (size_t row_start = 0; row_start < rows; row_start += 8) {
        size_t active_rows = rows - row_start < 8 ? rows - row_start : 8;
        __m512 result[8] = {0};
        for (size_t group = 0; group < groups_per_row; group++) {
            __m512i dot[8] = {0};
            __m512i correction = _mm512_setzero_si512();
            __m512i offset_bytes = _mm512_set1_epi8(
                -128); // Flip signed activations into the unsigned range required by VNNI.
            for (int chunk = 0; chunk < 16; chunk++) {
                __m512i weight_values =
                    _mm512_load_si512((const __m512i*)(packed_weights + group * 1024 + chunk * 64));
                correction = _mm512_dpbusd_epi32(correction, offset_bytes, weight_values);
                for (size_t row = 0; row < active_rows; row++) {
                    __m512i input_values = _mm512_broadcastd_epi32(_mm_loadu_si32(
                        input_q + (row_start + row) * weight->shape[1] + group * 64 + chunk * 4));
                    input_values = _mm512_xor_si512(input_values, offset_bytes);
                    dot[row] = _mm512_dpbusd_epi32(dot[row], input_values, weight_values);
                }
            }
            __m512 weight_scales = _mm512_cvtph_ps(_mm256_loadu_si256(
                (const __m256i*)(weight->scales +
                                 (output_block * groups_per_row + group) * block_rows)));
            for (size_t row = 0; row < active_rows; row++)
                result[row] = _mm512_fmadd_ps(
                    _mm512_cvtepi32_ps(_mm512_sub_epi32(dot[row], correction)),
                    _mm512_mul_ps(
                        weight_scales,
                        _mm512_set1_ps(input_scales[(row_start + row) * groups_per_row + group])),
                    result[row]);
        }
        for (size_t row = 0; row < active_rows; row++)
            _mm512_storeu_ps(output + (row_start + row) * weight->shape[0] +
                                 output_block * block_rows,
                             result[row]);
    }
}
#else
static inline __attribute__((always_inline)) void matmul_block(float* output, const int8_t* input_q,
                                                               const float* input_scales,
                                                               const Tensor* weight, size_t rows,
                                                               size_t output_block) {
    const size_t block_rows = 16;
    size_t groups_per_row = (size_t)weight->shape[1] / 64;
    const int8_t* packed_weights =
        (const int8_t*)weight->data + output_block * block_rows * weight->shape[1];
    for (size_t row_start = 0; row_start < rows; row_start += 4) {
        size_t active_rows = rows - row_start < 4 ? rows - row_start : 4;
        for (int half = 0; half < 2; half++) {
            __m256 result[4] = {0};
            for (size_t group = 0; group < groups_per_row; group++) {
                __m256i dot[4] = {0};
                for (int chunk = 0; chunk < 16; chunk++) {
                    __m256i weight_values = _mm256_loadu_si256(
                        (const __m256i*)(packed_weights + group * 1024 + chunk * 64 + half * 32));
                    __m256i weight_magnitudes = _mm256_abs_epi8(weight_values);
                    for (size_t row = 0; row < active_rows; row++) {
                        __m256i input_values = _mm256_broadcastd_epi32(
                            _mm_loadu_si32(input_q + (row_start + row) * weight->shape[1] +
                                           group * 64 + chunk * 4));
                        dot[row] = _mm256_add_epi32(
                            dot[row],
                            _mm256_madd_epi16(
                                _mm256_maddubs_epi16(weight_magnitudes,
                                                     _mm256_sign_epi8(input_values, weight_values)),
                                _mm256_set1_epi16(1)));
                    }
                }
                __m256 weight_scales = _mm256_cvtph_ps(_mm_loadu_si128(
                    (const __m128i*)(weight->scales +
                                     (output_block * groups_per_row + group) * block_rows +
                                     half * 8)));
                for (size_t row = 0; row < active_rows; row++)
                    result[row] = _mm256_fmadd_ps(
                        _mm256_cvtepi32_ps(dot[row]),
                        _mm256_mul_ps(
                            weight_scales,
                            _mm256_set1_ps(
                                input_scales[(row_start + row) * groups_per_row + group])),
                        result[row]);
            }
            for (size_t row = 0; row < active_rows; row++)
                _mm256_storeu_ps(output + (row_start + row) * weight->shape[0] +
                                     output_block * block_rows + half * 8,
                                 result[row]);
        }
    }
}
#endif

void matmul_int8(float* output, const int8_t* input_q, const float* input_scales,
                 const Tensor* weight, size_t rows) {
    const size_t block_rows = 16;
#pragma omp for schedule(static)
    for (size_t output_block = 0; output_block < (size_t)weight->shape[0] / block_rows;
         output_block++)
        matmul_block(output, input_q, input_scales, weight, rows, output_block);
}

void rmsnorm(std::mdspan<float, std::dims<2>> output, std::mdspan<const float, std::dims<2>> input,
             const float* weights, float epsilon) {
#pragma omp for schedule(static)
    for (size_t row = 0; row < input.extent(0); row++) {
        float sum_squares = 0.0f;
        for (size_t i = 0; i < input.extent(1); i++)
            sum_squares += input[row, i] * input[row, i];
        float inverse_rms = 1.0f / sqrtf(sum_squares / (float)input.extent(1) + epsilon);
        for (size_t i = 0; i < input.extent(1); i++)
            output[row, i] = (weights ? weights[i] : 1.0f) * (inverse_rms * input[row, i]);
    }
}

void add_and_scale(float* output, const float* addend, size_t count, float scale) {
#pragma omp for schedule(static)
    for (size_t i = 0; i < count; i++)
        output[i] = (output[i] + addend[i]) * scale;
}

void softmax(float* values, int count) {
    float max = values[0], sum = 1.0f;
    for (int i = 1; i < count; i++) {
        if (values[i] > max) {
            sum = sum * expf(max - values[i]) + 1.0f;
            max = values[i];
        } // Rescale the sum when a new maximum appears so expf() stays in range.
        else
            sum += expf(values[i] - max);
    }

    for (int i = 0; i < count; i++)
        values[i] = expf(values[i] - max) / sum;
}

void attention_scores(float* scores, std::mdspan<const float, std::dims<1>> query,
                      std::mdspan<const float, std::dims<2>> key_cache, int first_key,
                      int num_keys) {
    for (int key_index = 0; key_index < num_keys; key_index++) {
        const int cache_position = (first_key + key_index) % key_cache.extent(0);

        // Accumulate products in two sets of eight SIMD lanes.
        f32x8 partial_dot_0{}, partial_dot_1{};
        for (int dimension = 0; dimension < query.extent(0); dimension += 16) {
            partial_dot_0 =
                fma(std::simd::unchecked_load<f32x8>(&query[dimension], 8),
                    std::simd::unchecked_load<f32x8>(&key_cache[cache_position, dimension], 8),
                    partial_dot_0);
            partial_dot_1 =
                fma(std::simd::unchecked_load<f32x8>(&query[dimension + 8], 8),
                    std::simd::unchecked_load<f32x8>(&key_cache[cache_position, dimension + 8], 8),
                    partial_dot_1);
        }

        // Fold the eight partial sums into four lanes, then horizontally reduce to one dot product.
        const f32x4 dot_product_lanes = [&]() {
            auto [lower_half, upper_half] = std::simd::chunk<f32x4>(partial_dot_0 + partial_dot_1);
            f32x4 pair_sums = lower_half + upper_half;
            f32x4 opposite_pair_sums = std::simd::permute(pair_sums, [](auto lane) {
                constexpr std::array source_lane{2, 3, 2, 3};
                return source_lane[lane];
            });
            pair_sums = pair_sums + opposite_pair_sums;
            f32x4 other_pair_sums = std::simd::permute(pair_sums, [](auto lane) {
                constexpr std::array source_lane{1, 1, 3, 3};
                return source_lane[lane];
            });
            return pair_sums + other_pair_sums;
        }();
        scores[key_index] = dot_product_lanes[0];
    }
}

void weighted_value_sum(float* output, const float* probabilities,
                        std::mdspan<const float, std::dims<2>> value_cache, int first_key,
                        int num_keys) {
    for (size_t j = 0; j < value_cache.extent(1); j += 64) {
        std::array<f32x8, 8> sum{};
        for (int key_index = 0; key_index < num_keys; key_index++) {
            const int cache_position = (first_key + key_index) % value_cache.extent(0);
            f32x8 probability{probabilities[key_index]};
            for (size_t u = 0; u < 8; u++)
                sum[u] = fma(
                    probability,
                    std::simd::unchecked_load<f32x8>(&value_cache[cache_position, j + u * 8], 8),
                    sum[u]);
        }
        for (size_t u = 0; u < 8; u++)
            std::simd::unchecked_store(sum[u], output + j + u * 8, 8);
    }
}

void embedding(std::mdspan<float, std::dims<3>> output,
               std::mdspan<const int8_t, std::dims<5>> packed_embeddings,
               std::mdspan<const uint16_t, std::dims<3>> row_scales, std::span<const int> tokens,
               float multiplier) {
    // Each 64-value group has one half-precision scale per row in the block.
#pragma omp for schedule(static)
    for (auto [output_row, token_id] : std::ranges::views::enumerate(tokens)) {
        const size_t block_index = (size_t)(token_id / packed_embeddings.extent(3));
        const int row_in_block = token_id % packed_embeddings.extent(3);
        for (size_t group_index = 0; group_index < packed_embeddings.extent(1); group_index++) {
            // Convert this row's half-precision quantization scale, then apply the caller's
            // multiplier.
            const float scale =
                _cvtsh_ss(row_scales[block_index, group_index, row_in_block]) * multiplier;
            for (size_t value_index = 0; value_index < output.extent(2); value_index++) {
                const int chunk_index = value_index / 4;
                const int value_in_chunk = value_index % 4;
                output[output_row, group_index, value_index] =
                    (float)packed_embeddings[block_index, group_index, chunk_index, row_in_block,
                                             value_in_chunk] *
                    scale;
            }
        }
    }
}
