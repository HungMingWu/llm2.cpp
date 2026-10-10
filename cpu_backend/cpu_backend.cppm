module;
#include <cstddef>
#include <cstdint>
#include <mdspan>

export module cpu_backend;
import base;

export {
    void matmul_int8(float* output, const int8_t* input_q, const float* input_scales,
                     const Tensor* weight, size_t rows);
    void add_and_scale(float* output, const float* addend, size_t count, float scale);
    void softmax(float* values, int count);
    void attention_scores(float* scores, std::mdspan<const float, std::dims<1>> query,
                          std::mdspan<const float, std::dims<2>> key_cache, int first_key,
                          int num_keys);

    void weighted_value_sum(float* output, const float* probabilities,
                            std::mdspan<const float, std::dims<2>> value_cache, int first_key,
                            int num_keys);
    void rmsnorm(std::mdspan<float, std::dims<2>> output,
                 std::mdspan<const float, std::dims<2>> input, const float* weights, float epsilon);
    void embedding(std::mdspan<float, std::dims<3>> output,
                   std::mdspan<const int8_t, std::dims<5>> packed_embeddings,
                   std::mdspan<const uint16_t, std::dims<3>> row_scales,
                   std::span<const int> tokens, float multiplier);
    // Rotates pairs of query or key channels using each position's sine and cosine values so
    // attention can distinguish token order.
    void apply_rope(std::mdspan<const float, std::dims<2>> cosine,
                    std::mdspan<const float, std::dims<2>> sine,
                    std::mdspan<float, std::dims<3>> vector, int start_pos);

    // Approximates GELU from the exported lookup table and multiplies it by the up projection to
    // produce the MLP's gated activation.
    void geglu(std::mdspan<float, std::dims<2>> gate, std::mdspan<const float, std::dims<2>> up,
               std::span<const float> table, const float lower, const float upper,
               const float scale);
}
