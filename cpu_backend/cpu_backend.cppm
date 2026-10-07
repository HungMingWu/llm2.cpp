module;
#include <cstddef>
#include <cstdint>

export module cpu_backend;
import base;

export {
    void matmul_int8(float* output, const int8_t* input_q, const float* input_scales,
                     const Tensor* weight, size_t rows);
    void rmsnorm(float* output, const float* input, const Tensor* weight, int width, float epsilon,
                 size_t row_count);
    void add_and_scale(float* output, const float* addend, size_t count, float scale);
    void softmax(float* values, int count);
    void attention_scores(float* scores, const float* query, const float* key_cache, int first_key,
                          int num_keys, int cache_mask, int head_dim);

    void weighted_value_sum(float* output, const float* probabilities, const float* value_cache,
                            int first_key, int num_keys, int cache_mask, int head_dim);
}
