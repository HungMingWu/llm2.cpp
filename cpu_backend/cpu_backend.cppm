module;
#include <cstddef>
#include <cstdint>

export module cpu_backend;
import base;

export
{
  void matmul_int8(float *output, const int8_t *input_q, const float *input_scales, const Tensor *weight, size_t rows);
  void rmsnorm(float *output, const float *input, const Tensor *weight, int width, float epsilon, size_t row_count);
  void add_and_scale(float *output, const float *addend, size_t count, float scale);
}
