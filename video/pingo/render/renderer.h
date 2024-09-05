#pragma once

#include "../math/vec4.h"

#include "renderable.h"
#include "depth.h"
#include "pixel.h"
#include "texture.h"
#include "pano.h"

typedef struct tag_Scene Scene;
typedef struct tag_BackEnd BackEnd;

typedef enum {
    REND_NO_CLEAR = 0,  // 0: No clearing
    REND_CLEAR = 1,     // 1: Clear the frameBuffer
    REND_BACKGROUND = 2, // 2: Clear with a background texture
    REND_PANO = 3       // 3: Clear with a pano texture
} RenderClearType;

typedef struct tag_Renderer{
    Vec4i camera;
    Scene * scene;

    PingoDepth * z_buffer;

    Texture frameBuffer;
    
    int clear;
    Pixel clearColor;
    Texture background;

    Mat4 camera_projection;
    Mat4 camera_view;

    BackEnd * backEnd;

    Pano pano;

} Renderer;

extern int rendererRender(Renderer *);

extern int rendererInit(Renderer * r, Vec2i size, BackEnd * backEnd);

extern int rendererSetScene(Renderer *r, Scene *s);

extern int rendererSetCamera(Renderer *r, Vec4i camera);

extern void panoInit(Renderer * renderer, Pixel *pixels, Vec2i size, float fov_y_rad, int viewport_width, int viewport_height);

void renderPanoBackground(Renderer *r);

// Helper to set yaw angle and update state
void setYaw(Pano* pano, float new_yaw);

// Helper to compute the horizontal pixel mapping for the current yaw
void computeHorizontalMapping(Pano* pano, int* horizontalMapping);

// Helper to compute the vertical pixel mapping based on FOV
void computeVerticalMapping(Pano* pano, int* verticalMapping);

// Combined helper function to prepare all parameters for rendering
void preparePanoRendering(Pano* pano, int* horizontalMapping, int* verticalMapping);

// SCRATCHPIXEL FUNCTIONS
// https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/perspective-correct-interpolation-vertex-attributes.html
static inline void persp_divide(struct Vec3f* p);
static inline void to_raster(const Vec2i size, struct Vec3f* const p);
static inline void tri_bbox(const Vec3f* const p0, const Vec3f* const p1, const Vec3f* const p2, float* const bbox);
static inline float edge(const Vec3f* const a, const Vec3f* const b, const Vec3f* const test);
static Pixel shade(const Texture* texture, Vec2f uv);
static inline void rasterize(int x0, int y0, int x1, int y1, const Vec3f* const p0, const Vec3f* const p1, const Vec3f* const p2, const Vec2f* const uv0, const Vec2f* const uv1, const Vec2f* const uv2, const Texture* const texture, const Vec2i scrSize, Renderer* r, float near, float diffuseLight);
void mat4ExtractPerspective(const Mat4* m, float* near, float* far, float* aspect, float* fov);
