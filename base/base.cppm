module;
#include <cstdint>

export module base;

export
{
  struct Tensor {
      void *data;
      uint16_t *scales;
      int shape[4];
  };
}
