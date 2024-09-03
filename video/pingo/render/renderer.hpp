#pragma once

#include <cstring>
#include <cstdio>

#include <displaycontroller.h>

#include "../math/helpers.hpp"
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
    Renderer(Scene* scene, Camera* camera, PingoDepth* z_buffer, uint16_t width, uint16_t height, fabgl::RGBA2222 clearColor, int clear);
};

// Function to render the entire scene using a specific Renderer
int rendererRender(Renderer* r);

// Function to render a Scene using a specific Renderer and transformation matrix
int renderScene(Mat4 transform, Renderer* r, Renderable ren);

// Function to render a Renderable using a specific Renderer and transformation matrix
void renderRenderable(Mat4 transform, Renderer* r, Renderable ren);

// Function to render a textured object
int renderTexObject(Mat4 object_transform, Renderer* r, Renderable ren);

// Function to compute the edge function for three points (for triangle rasterization)
int edgeFunction(const Vec2f* a, const Vec2f* b, const Vec2f* c);

// Function to check if three points form a clockwise turn
float isClockWise(float x1, float y1, float x2, float y2, float x3, float y3);

// Function to compute the orientation of three 2D points (returns >0 for counterclockwise, 0 for collinear, <0 for clockwise)
int orient2d(Vec2i a, Vec2i b, Vec2i c);

// Function to draw a pixel in the framebuffer using a Renderer, a Bitmap, and a color with illumination
void backendDrawPixel(Renderer* r, fabgl::Bitmap* f, Vec2i pos, fabgl::RGBA2222 color, float illumination);

} // namespace p3d
