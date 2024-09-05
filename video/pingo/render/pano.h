#pragma once

#include "../math/vec2.h"
#include "pixel.h"

typedef struct {
    Vec2i size;             // Dimensions of the equirectangular texture (width, height)
    Pixel *pixels;          // Pointer to the texture pixel data

    float fov_y;            // Vertical field of view (FOV) in radians
    int viewport_width;     // Width of the rendering viewport in pixels
    int viewport_height;    // Height of the rendering viewport in pixels

    float vertical_step;    // Precomputed vertical step size for mapping viewport to texture
    float aspect_ratio;     // Aspect ratio of the viewport (width / height)

    // State Variables for Rendering
float yaw;              // Current yaw angle for rendering (in rad@QAwSAW   ians)
    int view_height;        // Height in pixels of the portion of the texture visible given the current FOV
    int half_viewport_width; // Half of the viewport width, used for centering calculations

    // Optionally, add more precomputed values if required for further optimization
} Pano;
