#pragma once

#include <cstring>
#include <cstdio>

#include <displaycontroller.h>

#include "../math/vec4.hpp"
#include "renderable.hpp"
#include "depth.hpp"
#include "scene.hpp"
#include "object.hpp"
#include "camera.hpp"
#include "rasterizer.hpp"

namespace p3d {

struct Scene;

struct Renderer {
    Scene* scene;                    // Pointer to the Scene to be rendered
    Camera* camera;                  // Pointer to the Camera used for rendering
    fabgl::Bitmap* frameBuffer;      // Framebuffer for rendered pixels
    PingoDepth* z_buffer;            // Depth buffer for depth information
    int clear;                       // Clear flag for the renderer
    fabgl::RGBA2222 clearColor;      // Color used to clear framebuffer before rendering
    fabgl::Bitmap* background;       // Background image used to clear framebuffer before rendering

    // Constructor
    Renderer(Scene* scene, Camera* camera, uint16_t width, uint16_t height,
             fabgl::RGBA2222 clearColor = {0x00}, int clear = 1);
};

} // namespace p3d
