module;
#include <cassert>
#include <cmath>
#include <cstddef>
#include <immintrin.h>
#include <mdspan>
#include <ranges>
#include <simd>
#include <stdfloat>

module cpu_backend;

using f32x16 = std::simd::vec<float, 16>;
using f32x8 = std::simd::vec<float, 8>;
using f32x4 = std::simd::vec<float, 4>;
using f16x8 = std::simd::vec<std::float16_t, 8>;
using f16x16 = std::simd::vec<std::float16_t, 16>;
using i8x4 = std::simd::vec<std::int8_t, 4>;
using i8x32 = std::simd::vec<std::int8_t, 32>;
using i8x64 = std::simd::vec<std::int8_t, 64>;
using i16x16 = std::simd::vec<std::int16_t, 16>;
using i32x4 = std::simd::vec<std::int32_t, 4>;
using i32x8 = std::simd::vec<std::int32_t, 8>;
using i32x16 = std::simd::vec<std::int32_t, 16>;
using u8x32 = std::simd::vec<std::uint8_t, 32>;
using u8x64 = std::simd::vec<std::uint8_t, 64>;

// workaround for std::simd::fma
auto fma(auto a, auto b, auto c) {
    return a * b + c;
}

inline i32x16 dpbusd(const i32x16& acc, const u8x64& a, const i8x64& b) {
    return i32x16([&](auto i) {
        const std::size_t j = std::size_t(i) * 4;

        std::int32_t dot = std::int32_t(a[j]) * std::int32_t(b[j]) +
                           std::int32_t(a[j + 1]) * std::int32_t(b[j + 1]) +
                           std::int32_t(a[j + 2]) * std::int32_t(b[j + 2]) +
                           std::int32_t(a[j + 3]) * std::int32_t(b[j + 3]);

        std::uint32_t sum =
            std::bit_cast<std::uint32_t>(std::int32_t(acc[i])) + static_cast<std::uint32_t>(dot);

        return std::bit_cast<std::int32_t>(sum);
    });
}

inline i8x32 sign_epi8(const i8x32& a, const i8x32& b) {
    const i8x32 zero{};

    auto result = std::simd::select(b < zero, -a, a);

    return std::simd::select(b == zero, zero, result);
}

inline i16x16 maddubs(const u8x32& a, const i8x32& b) {
    return i16x16([&](auto i) {
        const std::size_t j = std::size_t(i) * 2;

        std::int32_t sum = std::int32_t(a[j]) * std::int32_t(b[j]) +
                           std::int32_t(a[j + 1]) * std::int32_t(b[j + 1]);

        // Signed saturation
        if (sum > 32767)
            sum = 32767;
        if (sum < -32768)
            sum = -32768;

        return static_cast<std::int16_t>(sum);
    });
}

inline i32x8 madd_epi16(const i16x16& a, const i16x16& b) {
    return i32x8([&](auto i) {
        const std::size_t j = std::size_t(i) * 2;

        const std::int32_t p0 = std::int32_t(a[j]) * std::int32_t(b[j]);

        const std::int32_t p1 = std::int32_t(a[j + 1]) * std::int32_t(b[j + 1]);

        // Match VPMADDWD 32-bit wrapping semantics
        const std::uint32_t sum = std::uint32_t(p0) + std::uint32_t(p1);

        return std::bit_cast<std::int32_t>(sum);
    });
}

// abs_epi8 is very slow
inline i8x32 abs_epi8(const i8x32& a) {
    const i8x32 zero{};

    return std::simd::select(a < zero, -a, a);
}

// Multiplies dynamically quantized activations by packed int8 weights in blocks of 16
// output rows. VNNI handles eight input rows at once while AVX2 handles four.
#if 0 // defined(__AVX512VNNI__)
static inline __attribute__((always_inline)) void
matmul_block(std::mdspan<float, std::dims<2>, std::layout_stride> output,
             std::mdspan<const int8_t, std::dims<2>> input_q,
             std::mdspan<const float, std::dims<2>> input_scales,
             std::mdspan<const int8_t, std::dims<4>> packed_weights,
             std::mdspan<const std::float16_t, std::dims<2>> weight_scales) {
    size_t groups_per_row = packed_weights.extent(0);
    while (!output.empty()) {
        size_t active_rows = std::min(size_t(8), output.extent(0));
        std::array<f32x16, 8> result{};
        for (size_t group = 0; group < groups_per_row; group++) {
            std::array<i32x16, 8> dot{};
            i32x16 correction{};
            u8x64 offset_bytes{std::uint8_t{128}};
            for (int chunk = 0; chunk < 16; chunk++) {
                auto weight_values =
                    std::simd::unchecked_load<i8x64>(&packed_weights[group, chunk, 0, 0], 64);
                // correction = dpbusd(correction, offset_bytes, weight_values);
                correction = _mm512_dpbusd_epi32(correction, offset_bytes, weight_values);
                for (size_t row = 0; row < active_rows; row++) {
                    i8x64 input_values([&](auto lane) {
                        return input_q[row, group * 64 + chunk * 4 + std::size_t(lane) % 4];
                    });
                    input_values = input_values ^ i8x64 {
                        std::int8_t {
                            -128
                        }
                    };
                    // dot[row] = dpbusd(dot[row], std::bit_cast<u8x64>(input_values),
                    // weight_values);
                    dot[row] = _mm512_dpbusd_epi32(dot[row], input_values, weight_values);
                }
            }
            auto group_scales =
                f32x16{std::simd::unchecked_load<f16x16>(&weight_scales[group, 0], 16)};
            for (size_t row = 0; row < active_rows; row++)
                result[row] = fma(f32x16{dot[row] - correction},
                                  group_scales * f32x16{input_scales[row, group]}, result[row]);
        }
        for (size_t row = 0; row < active_rows; row++)
            std::simd::unchecked_store(result[row], &output[row, 0], 16);
        output = std::submdspan(output, std::pair{active_rows, output.extent(0)}, std::full_extent);
        input_q =
            std::submdspan(input_q, std::pair{active_rows, input_q.extent(0)}, std::full_extent);
        input_scales = std::submdspan(input_scales, std::pair{active_rows, input_scales.extent(0)},
                                      std::full_extent);
    }
}
#else
static inline __attribute__((always_inline)) void
matmul_block(std::mdspan<float, std::dims<2>, std::layout_stride> output,
             std::mdspan<const int8_t, std::dims<2>> input_q,
             std::mdspan<const float, std::dims<2>> input_scales,
             std::mdspan<const int8_t, std::dims<4>> packed_weights,
             std::mdspan<const std::float16_t, std::dims<2>> weight_scales) {
    size_t groups_per_row = packed_weights.extent(0);
    while (!output.empty()) {
        size_t active_rows = std::min(size_t(4), output.extent(0));
        for (int half = 0; half < 2; half++) {
            std::array<f32x8, 4> result{};
            for (size_t group = 0; group < groups_per_row; group++) {
                std::array<i32x8, 4> dot{};
                for (int chunk = 0; chunk < 16; chunk++) {
                    auto weight_values = std::simd::unchecked_load<i8x32>(
                        &packed_weights[group, chunk, half * 8, 0], 32);
                    u8x32 weight_magnitudes =
                        /*_mm256_abs_epi8*/ std::bit_cast<u8x32>(abs_epi8(weight_values));
                    for (size_t row = 0; row < active_rows; row++) {
                        i8x32 input_values([&](auto lane) {
                            return input_q[row, group * 64 + chunk * 4 + std::size_t(lane) % 4];
                        });
                        dot[row] =
                            dot[row] +
                            /*_mm256_madd_epi16*/ madd_epi16(
                                /*_mm256_maddubs_epi16*/ maddubs(
                                    weight_magnitudes,
                                    /*_mm256_sign_epi8*/ sign_epi8(input_values, weight_values)),
                                i16x16{std::int16_t(1)});
                    }
                }
                auto group_scales =
                    f32x8{std::simd::unchecked_load<f16x8>(&weight_scales[group, half * 8], 8)};
                for (size_t row = 0; row < active_rows; row++)
                    result[row] = fma(f32x8{dot[row]},
                                      group_scales * f32x8{input_scales[row, group]}, result[row]);
            }
            for (size_t row = 0; row < active_rows; row++)
                std::simd::unchecked_store(result[row], &output[row, half * 8], 8);
        }
        output = std::submdspan(output, std::pair{active_rows, output.extent(0)}, std::full_extent);
        input_q =
            std::submdspan(input_q, std::pair{active_rows, input_q.extent(0)}, std::full_extent);
        input_scales = std::submdspan(input_scales, std::pair{active_rows, input_scales.extent(0)},
                                      std::full_extent);
    }
}
#endif

void matmul_int8(std::mdspan<float, std::dims<2>> output,
                 std::mdspan<const int8_t, std::dims<2>> input_q,
                 std::mdspan<const float, std::dims<2>> input_scales,
                 std::mdspan<const int8_t, std::dims<2>> packed_weights1,
                 std::mdspan<const std::float16_t, std::dims<2>> weight_scales1) {
    const size_t block_rows = 16;
    const size_t groups_per_row = weight_scales1.extent(1) / 64;
    std::mdspan packed_weights(&packed_weights1[0, 0], packed_weights1.extent(0) / block_rows,
                               groups_per_row, 16, block_rows, 4);
    std::mdspan weight_scales(&weight_scales1[0, 0], weight_scales1.extent(0) / block_rows,
                              groups_per_row, block_rows);
#pragma omp for schedule(static)
    for (size_t output_block = 0; output_block < packed_weights.extent(0); output_block++)
        matmul_block(
            std::submdspan(output, std::full_extent,
                           std::pair{output_block * block_rows, output_block * block_rows + 16}),
            input_q, input_scales,
            std::submdspan(packed_weights, output_block, std::full_extent, std::full_extent,
                           std::full_extent, std::full_extent),
            std::submdspan(weight_scales, output_block, std::full_extent, std::full_extent));
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
               std::mdspan<const std::float16_t, std::dims<3>> row_scales,
               std::span<const int> tokens, float multiplier) {
    // Each 64-value group has one half-precision scale per row in the block.
#pragma omp for schedule(static)
    for (auto [output_row, token_id] : std::ranges::views::enumerate(tokens)) {
        const size_t block_index = (size_t)(token_id / packed_embeddings.extent(3));
        const int row_in_block = token_id % packed_embeddings.extent(3);
        for (size_t group_index = 0; group_index < packed_embeddings.extent(1); group_index++) {
            // Convert this row's half-precision quantization scale, then apply the caller's
            // multiplier.
            const float scale = _cvtsh_ss(std::bit_cast<uint16_t>(
                                    row_scales[block_index, group_index, row_in_block])) *
                                multiplier;
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

void apply_rope(std::mdspan<const float, std::dims<2>> cosine,
                std::mdspan<const float, std::dims<2>> sine,
                std::mdspan<float, std::dims<3>> vector, int start_pos) {
    const size_t head_dim = vector.extent(2);
#pragma omp for schedule(static)
    for (size_t token = 0; token < vector.extent(0); token++) {
        for (size_t head = 0; head < vector.extent(1); head++) {
            for (size_t j = 0; j < cosine.extent(1); j++) {
                float first = vector[token, head, j];
                float second = vector[token, head, j + head_dim / 2];
                vector[token, head, j] =
                    first * cosine[start_pos + token, j] - second * sine[start_pos + token, j];
                vector[token, head, j + head_dim / 2] =
                    second * cosine[start_pos + token, j] + first * sine[start_pos + token, j];
            }
        }
    }
}

void geglu(std::mdspan<float, std::dims<2>> gate, std::mdspan<const float, std::dims<2>> up,
           std::span<const float> table, const float lower, const float upper, const float scale) {
#pragma omp for collapse(2) schedule(static)
    for (size_t row = 0; row < gate.extent(0); row++) {
        for (size_t i = 0; i < gate.extent(1); i++) {
            float x = gate[row, i];
            if (x <= lower) {
                x = table[0];
            } else if (!(x >= upper)) {
                float position = (x - lower) * scale;
                int index = (int)position;
                float fraction = position - (float)index;
                x = table[index] + fraction * (table[index + 1] - table[index]);
            }
            gate[row, i] = x * up[row, i];
        }
    }
}
