#pragma once

// #include <fabgl.h>
#include "../math/mat4.hpp"

namespace p3d {

class Renderer; 

enum RenderableType {
    RENDERABLE_SCENE = 0,
    RENDERABLE_SPRITE,
    RENDERABLE_TEXOBJECT,
    RENDERABLE_COUNT,
};

struct Renderable {
    RenderableType renderableType;
    void* impl;
};

extern int (*renderingFunctions[RENDERABLE_COUNT])(Mat4 transform, Renderer*, Renderable);

// Main renderer function that initializes rendering
int rendererRender(Renderer* r);

// Renders a scene given a transformation matrix and renderer
int renderScene(Mat4 transform, Renderer* r, Renderable ren);

// Renders a specific renderable object
void renderRenderable(Mat4 transform, Renderer* r, Renderable ren);

// Edge function for triangle rasterization
int edgeFunction(const Vec2f* a, const Vec2f* b, const Vec2f* c);

// Determines if a triangle is oriented clockwise
float isClockWise(float x1, float y1, float x2, float y2, float x3, float y3);

// 2D orientation function for determining the order of points
int orient2d(Vec2i a, Vec2i b, Vec2i c);

// // TODO: IMPLEMENT THIS FOR ILLUMINATION
// // Draws a pixel to a framebuffer with illumination
// void backendDrawPixel(Renderer* r, fabgl::Bitmap* f, Vec2i pos, fabgl::RGBA2222 color, float illumination);

// Renders a textured object given a transformation matrix and renderer
int renderTexObject(Mat4 object_transform, Renderer* r, Renderable ren);

} // namespace p3d