#pragma once

#include <cstdint>
#include <displaycontroller.h>  // Include for fabgl::RGBA2222

namespace p3d {

// Function prototypes for conversion functions

// Converts an int32_t scale value to a float
float convert_scale_value(int32_t value);

// Converts an int32_t rotation value to a float (in radians)
float convert_rotation_value(int32_t value);

// Converts an int32_t translation value to a float
float convert_translation_value(int32_t value);

// Converts an int32_t position value to a float
float convert_position_value(int32_t value);

// Converts an int32_t texture coordinate value to a float
float convert_texture_coordinate_value(int32_t value);

// Converts a uint8_t to an RGBA2222 color
fabgl::RGBA2222 uint8ToRGBA2222(uint8_t packed);

// Converts an RGBA2222 color to a uint8_t
uint8_t RGBA2222ToUint8(const fabgl::RGBA2222& color);

} // namespace p3d
