#include "depth.hpp"

namespace p3d {

PingoDepth* depth_buffer_create(int size) {
    PingoDepth* buffer = (PingoDepth*) heap_caps_malloc(size * sizeof(PingoDepth), MALLOC_CAP_SPIRAM);
    if (buffer != nullptr) {
        std::memset(buffer, 0, sizeof(PingoDepth) * size);
    }
    return buffer;
}

void depth_buffer_destroy(PingoDepth* buffer) {
    heap_caps_free(buffer);
}

void depth_write(PingoDepth* d, int idx, float value) {
    d[idx].d = (uint32_t)(value * (float)UINT32_MAX);
}

bool depth_check(PingoDepth* d, int idx, float value) {
    return (uint32_t)(value * (float)UINT32_MAX) < d[idx].d;
}

void depth_clear(PingoDepth* d, int size) {
    std::memset(d, 0, sizeof(PingoDepth) * size);
}


} // namespace p3d