#pragma once

#include "texture.h"
#include "renderable.h"
#include "pixel.h"
#include "../math/vec4.h"

typedef struct tag_Scene Scene;
typedef struct tag_BackEnd BackEnd;

typedef enum {
    REND_NO_CLEAR = 0,  // 0: No clearing
    REND_CLEAR = 1,     // 1: Clear the frameBuffer
    REND_BACKGROUND = 2 // 2: Clear with a background texture
} RenderClearType;

typedef struct tag_Renderer{
    Vec4i camera;
    Scene * scene;

    Texture frameBuffer;
    
    int clear;
    Pixel clearColor;
    Texture background;

    Mat4 camera_projection;
    Mat4 camera_view;

    BackEnd * backEnd;

} Renderer;

extern int rendererRender(Renderer *);

extern int rendererInit(Renderer *, Vec2i size, struct tag_BackEnd * backEnd);

extern int rendererSetScene(Renderer *r, Scene *s);

extern int rendererSetCamera(Renderer *r, Vec4i camera);

// SCRATCHPIXEL FUNCTIONS
// https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/perspective-correct-interpolation-vertex-attributes.html
static inline void persp_divide(struct Vec3f* p);
static inline void to_raster(const Vec2i size, struct Vec3f* const p);
static inline void tri_bbox(const Vec3f* const p0, const Vec3f* const p1, const Vec3f* const p2, float* const bbox);
static inline float edge(const Vec3f* const a, const Vec3f* const b, const Vec3f* const test);
static Pixel shade(const Texture* texture, Vec2f uv);
static inline void rasterize(int x0, int y0, int x1, int y1, const Vec3f* const p0, const Vec3f* const p1, const Vec3f* const p2, const Vec2f* const uv0, const Vec2f* const uv1, const Vec2f* const uv2, const Texture* const texture, const Vec2i scrSize, Renderer* r, float near, float diffuseLight);
void mat4ExtractPerspective(const Mat4* m, float* near, float* far, float* aspect, float* fov);
Pixel rgba2222_to_pixel(uint8_t data);