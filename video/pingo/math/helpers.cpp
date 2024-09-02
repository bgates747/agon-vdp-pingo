#include "helpers.hpp"

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

fabgl::RGBA2222 uint8ToRGBA2222(uint8_t packed) {
    return fabgl::RGBA2222(
        packed & 0b00000011,            // Extract bits 0-1 for Red
        (packed >> 2) & 0b00000011,     // Extract bits 2-3 for Green
        (packed >> 4) & 0b00000011,     // Extract bits 4-5 for Blue
        (packed >> 6) & 0b00000011      // Extract bits 6-7 for Alpha
    );
}

uint8_t RGBA2222ToUint8(const fabgl::RGBA2222& color) {
    return (color.A << 6) | (color.B << 4) | (color.G << 2) | color.R;
}

} // namespace p3d