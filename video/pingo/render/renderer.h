#pragma once

#include "../math/vec4.h"
#include "depth.h"
#include "texture.h"
#include "renderable.h"
#include "pixel.h"
#include "camera.h"

typedef struct tag_Scene Scene;
typedef struct tag_BackEnd BackEnd;

typedef enum {
    REND_NO_CLEAR = 0,  // 0: No clearing
    REND_CLEAR = 1,     // 1: Clear the frameBuffer
    REND_BACKGROUND = 2 // 2: Clear with a background texture
} RenderClearType;

typedef struct Renderer{
    Camera camera;
    Scene * scene;

    PingoDepth * z_buffer;

    Texture frameBuffer;
    
    int clear;
    Pixel clearColor;
    Texture background;

    BackEnd * backEnd;

} Renderer;

extern int rendererRender(Renderer *);

extern int rendererInit(Renderer *, Vec2i size, BackEnd * backEnd);

extern int rendererSetScene(Renderer *r, Scene *s);

extern int rendererSetCamera(Renderer *r, Vec2i camera);

int renderFrame(Renderer *r, Renderable ren);

int renderSprite(Mat4 transform, Renderer *r, Renderable ren);

void renderRenderable(Mat4 transform, Renderer *r, Renderable ren);

int renderScene(Mat4 transform, Renderer *r, Renderable ren);

int renderObject(Mat4 object_transform, Renderer *r, Renderable ren);

float isClockWise(float x1, float y1, float x2, float y2, float x3, float y3);

void backendDrawPixel (Renderer * r, Texture * f, Vec2i pos, Pixel color, float illumination);


// SCRATCHPIXEL FUNCTIONS
// https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/perspective-correct-interpolation-vertex-attributes.html
static inline void persp_divide(struct Vec3f* p);
static inline void to_raster(const Vec2i size, struct Vec3f* const p);
static inline void tri_bbox(const Vec3f* const p0, const Vec3f* const p1, const Vec3f* const p2, float* const bbox);
static inline float edge(const Vec3f* const a, const Vec3f* const b, const Vec3f* const test);
static Pixel shade(const Texture* texture, Vec2f uv);
// NOT SCRATCHPIXEL BUT NOT PINGO EITHER
static inline void rasterize_scanline(Vec3f* p0, Vec3f* p1, Vec3f* p2, Vec2f* uv0, Vec2f* uv1, Vec2f* uv2, const Texture* texture, const Vec2i scrSize, Renderer* r, float near, float diffuseLight);
static inline void swap_float(float* a, float* b);
static inline void sort_vertices_with_uvs(Vec3f** p0, Vec3f** p1, Vec3f** p2, Vec2f* uv0, Vec2f* uv1, Vec2f* uv2);
static float interpolate_x(const Vec3f* v0, const Vec3f* v1, float y);
