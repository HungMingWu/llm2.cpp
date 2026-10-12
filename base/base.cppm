module;
#include <cstdint>
#include <stdfloat>

export module base;

export {
    struct Tensor {
        void* data;
        std::float16_t* scales;
        int shape[4];
    };
}
