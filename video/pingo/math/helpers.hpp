#pragma once
#include <cstdint>

namespace p3d {

float convert_scale_value(int32_t value) {
    static const float factor = 1.0f / 256.0f;
    return ((float) value) * factor;
}

float convert_rotation_value(int32_t value) {
    if (value & 0x8000) {
        value = (int32_t)(int16_t)(uint16_t) value;
    }
    static const float factor = (2.0f * 3.1415926f) / 32767.0f;
    return ((float) value) * factor;
}

float convert_translation_value(int32_t value) {
    if (value & 0x8000) {
        value = (int32_t)(int16_t)(uint16_t) value;
    }
    static const float factor = 256.0f / 32767.0f;
    return ((float) value) * factor;
}

float convert_position_value(int32_t value) {
    if (value & 0x8000) {
        value = (int32_t)(int16_t)(uint16_t) value;
    }
    static const float factor = 1.0f / 32767.0f;
    return ((float) value) * factor;
}

float convert_texture_coordinate_value(int32_t value) {
    static const float factor = 1.0f / 65535.0f;
    return ((float) value) * factor;
}

} // namespace p3d