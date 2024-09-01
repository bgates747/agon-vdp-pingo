#pragma once

#include <cstring>
#include <cstdint>
#include <esp_heap_caps.h>

namespace p3d {

// Integer-based struct
struct PingoDepth {
    uint32_t d;
};

PingoDepth* depth_buffer_create(int size);
void depth_buffer_destroy(PingoDepth* buffer);
void depth_write(PingoDepth * d, int idx, float value);
bool depth_check(PingoDepth * d, int idx, float value);
void depth_clear(PingoDepth * d, int size);

} // namespace p3d