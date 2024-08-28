#include <string.h>
#include <stdio.h>
#include <math.h>

#include "renderer.h"
#include "sprite.h"
#include "pixel.h"
#include "depth.h"
#include "backend.h"
#include "scene.h"
#include "rasterizer.h"
#include "object.h"

#define MIN(a, b)(((a) < (b)) ? (a) : (b))
#define MAX(a, b)(((a) > (b)) ? (a) : (b))
#define Z_THRESHOLD -0.01f

// FORWARD DECLARATIONS
static inline void persp_divide(struct Vec3f* p);
static inline void to_raster(const Vec2i size, struct Vec3f* const p);
static inline void tri_bbox(const Vec3f* const p0, const Vec3f* const p1, const Vec3f* const p2, float* const bbox);
static inline float edge(const Vec3f* const a, const Vec3f* const b, const Vec3f* const test);
static Pixel shade(const Texture* texture, Vec2f uv);
static inline void rasterize(int x0, int y0, int x1, int y1, const Vec3f* const p0, const Vec3f* const p1, const Vec3f* const p2, const Vec2f* const uv0, const Vec2f* const uv1, const Vec2f* const uv2, const Texture* const texture, const Vec2i scrSize, Renderer* r);
void mat4ExtractPerspective(const Mat4* m, float* near, float* far, float* aspect, float* fov);
Pixel rgba2222_to_pixel(uint8_t data);

#if DEBUG
extern void show_pixel(float x, float y, uint8_t a, uint8_t b, uint8_t g, uint8_t r);
#endif

int renderFrame(Renderer * r, Renderable ren) {
    Texture * f = ren.impl;
    return rasterizer_draw_pixel_perfect((Vec2i) { 0, 0 }, r, f);
};

int renderSprite(Mat4 transform, Renderer * r, Renderable ren) {
    Sprite * s = ren.impl;
    Mat4 backUp = s->t;

    //Apply parent transform to the local transform
    s->t = mat4MultiplyM( & s->t, & transform);

    //Apply camera translation
    Mat4 newMat = mat4Translate((Vec3f) { -r->camera.x, -r->camera.y, 0 });
    s->t = mat4MultiplyM( & s->t, & newMat);

    rasterizer_draw_transformed(s->t, r, & s->frame);
    s->t = backUp;
    return 0;
};

void renderRenderable(Mat4 transform, Renderer * r, Renderable ren) {
    renderingFunctions[ren.renderableType](transform, r, ren);
};

int renderScene(Mat4 transform, Renderer * r, Renderable ren) {
    Scene * s = ren.impl;
    if (!s->visible)
        return 0;

    //Apply hierarchy transfom
    Mat4 newTransform = mat4MultiplyM( & s->transform, & transform);
    for (int i = 0; i < s->numberOfRenderables; i++) {
        renderRenderable(newTransform, r, s->renderables[i]);
    }
    return 0;
};

float isClockWise(float x1, float y1, float x2, float y2, float x3, float y3) {
    return (y2 - y1) * (x3 - x2) - (y3 - y2) * (x2 - x1);
}

int orient2d( Vec2i a, Vec2i b, Vec2i c)
{
    return (b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x);
}

void backendDrawPixel (Renderer * r, Texture * f, Vec2i pos, Pixel color, float illumination) {
    // If backend specifies something..
    if (r->backEnd->drawPixel != 0) {
        // Draw using the backend
        r->backEnd->drawPixel(f, pos, color, illumination);
    }
    else {
        // By default call this
        texture_draw(f, pos, pixelMul(color, illumination));
    }
}

int renderObject(Mat4 object_transform, Renderer * r, Renderable ren) {

    const Vec2i scrSize = r->frameBuffer.size;
    Object * o = ren.impl;
    Vec2f * tex_coords = o->textCoord;

    // MODEL MATRIX
    Mat4 m = mat4MultiplyM( &o->transform, &object_transform  );
    // Mat4 m = o->transform;

    // VIEW MATRIX
    Mat4 v = r->camera_view;
    Mat4 p = r->camera_projection;

    // get the camera attributes back out of its projection matrix
    float near, far, aspect, fov;
    mat4ExtractPerspective(&p, &near, &far, &aspect, &fov);

    for (int i = 0; i < o->mesh->indexes_count; i += 3) {
        Vec3f *ver1 = &o->mesh->positions[o->mesh->pos_indices[i + 0]];
        Vec3f *ver2 = &o->mesh->positions[o->mesh->pos_indices[i + 1]];
        Vec3f *ver3 = &o->mesh->positions[o->mesh->pos_indices[i + 2]];

        Vec4f a = {ver1->x, ver1->y, ver1->z, 1};
        Vec4f b = {ver2->x, ver2->y, ver2->z, 1};
        Vec4f c = {ver3->x, ver3->y, ver3->z, 1};

        // Print original vertex positions
        // printf("Original vertices:\n");
        // printf("a: (%f, %f, %f, %f), b: (%f, %f, %f, %f), c: (%f, %f, %f, %f)\n", a.x, a.y, a.z, a.w, b.x, b.y, b.z, b.w, c.x, c.y, c.z, c.w);

        a = mat4MultiplyVec4(&a, &m);
        b = mat4MultiplyVec4(&b, &m);
        c = mat4MultiplyVec4(&c, &m);

        // Print transformed vertex positions (after model matrix multiplication)
        // printf("After model matrix multiplication:\n");
        // printf("a: (%f, %f, %f, %f), b: (%f, %f, %f, %f), c: (%f, %f, %f, %f)\n", a.x, a.y, a.z, a.w, b.x, b.y, b.z, b.w, c.x, c.y, c.z, c.w);

        a = mat4MultiplyVec4( &a, &p);
        b = mat4MultiplyVec4( &b, &p);
        c = mat4MultiplyVec4( &c, &p);

        //Triangle is completely behind camera
        if (a.z > -near && b.z > -near && c.z > -near)
            continue;

        // Perspective divide
        persp_divide((Vec3f *)&a);
        persp_divide((Vec3f *)&b);
        persp_divide((Vec3f *)&c);

        // Print vertex positions after perspective divide
        // printf("After perspective divide:\n");
        // printf("a: (%f, %f, %f), b: (%f, %f, %f), c: (%f, %f, %f)\n", a.x, a.y, a.z, b.x, b.y, b.z, c.x, c.y, c.z);

        float clocking = isClockWise(a.x, a.y, b.x, b.y, c.x, c.y);
        if (clocking >= 0)
            continue;

        // Convert to raster space
        to_raster(scrSize, (Vec3f *)&a);
        to_raster(scrSize, (Vec3f *)&b);
        to_raster(scrSize, (Vec3f *)&c);

        // Print vertex positions after rasterization
        // printf("After rasterization:\n");
        // printf("a: (%f, %f, %f), b: (%f, %f, %f), c: (%f, %f, %f)\n", a.x, a.y, a.z, b.x, b.y, b.z, c.x, c.y, c.z);

        float bbox[4];
        tri_bbox((Vec3f *)&a, (Vec3f *)&b, (Vec3f *)&c, bbox);

        // Print bounding box
        // printf("Bounding box: minX: %f, minY: %f, maxX: %f, maxY: %f\n", bbox[0], bbox[1], bbox[2], bbox[3]);

        // Bounding box constraint
        if (bbox[0] > scrSize.x - 1 || bbox[2] < 0 || bbox[1] > scrSize.y - 1 || bbox[3] < 0)
            continue;

        int x0 = MAX(0, (int)bbox[0]);
        int y0 = MAX(0, (int)bbox[1]);
        int x1 = MIN(scrSize.x - 1, (int)bbox[2]);
        int y1 = MIN(scrSize.y - 1, (int)bbox[3]);

        Vec2f tca = tex_coords[o->tex_indices[i + 0]];
        Vec2f tcb = tex_coords[o->tex_indices[i + 1]];
        Vec2f tcc = tex_coords[o->tex_indices[i + 2]];

        // Perspective correct texture coordinates
        tca.x /= a.z;
        tca.y /= a.z;
        tcb.x /= b.z;
        tcb.y /= b.z;
        tcc.x /= c.z;
        tcc.y /= c.z;

        // Rasterize the triangle with the new logic
        rasterize(x0, y0, x1, y1, (Vec3f *)&a, (Vec3f *)&b, (Vec3f *)&c, &tca, &tcb, &tcc, o->material->texture, scrSize, r);

    }

    return 0;
}

int rendererInit(Renderer * r, Vec2i size, BackEnd * backEnd) {
    renderingFunctions[RENDERABLE_SPRITE] = & renderSprite;
    renderingFunctions[RENDERABLE_SCENE] = & renderScene;
    renderingFunctions[RENDERABLE_OBJECT] = & renderObject;

    r->scene = 0;
    r->clear = 1;
    r->clearColor = PIXELBLACK;
    r->backEnd = backEnd;

    r->backEnd->init(r, r->backEnd, (Vec4i) { 0, 0, 0, 0 });

    int e = 0;
    e = texture_init( & (r->frameBuffer), size, backEnd->getFrameBuffer(r, backEnd));
    if (e) return e;

    return 0;
}

int rendererRender(Renderer * r) {

    int pixels = r->frameBuffer.size.x * r->frameBuffer.size.y;
    memset(r->backEnd->getZetaBuffer(r, r->backEnd), 0, pixels * sizeof (PingoDepth));

    r->backEnd->beforeRender(r, r->backEnd);

    //get current framebuffe from backend
    r->frameBuffer.frameBuffer = r->backEnd->getFrameBuffer(r, r->backEnd);

    //Clear draw buffer before rendering
    if (r->clear) {
        memset(r->backEnd->getFrameBuffer(r, r->backEnd), 0, pixels * sizeof (Pixel));
    }

    renderScene(mat4Identity(), r, sceneAsRenderable(r->scene));

    r->backEnd->afterRender(r, r->backEnd);

    return 0;
}

int rendererSetScene(Renderer * r, Scene * s) {
    if (s == 0)
        return 1; //nullptr scene

    r->scene = s;
    return 0;
}

int rendererSetCamera(Renderer * r, Vec4i rect) {
    r->camera = rect;
    r->backEnd->init(r, r->backEnd, rect);
    r->frameBuffer.size = (Vec2i) {
            rect.z, rect.w
};
    return 0;
}

// SCRATCHPIXEL FUNCTIONS
static inline void persp_divide(struct Vec3f* p) {
    if (p->z > Z_THRESHOLD) {
        p->z = Z_THRESHOLD;  // Prevent division by zero
    }
    float inv_z = 1.0f / p->z;  // Use the z value directly without flipping sign
    p->x *= inv_z;  // Normalize x by z
    p->y *= inv_z;  // Normalize y by z
}

static inline void to_raster(const Vec2i size, struct Vec3f* const p) {
    p->x = (p->x + 1) * 0.5f * (float)size.x;
    p->y = (1 - p->y) * 0.5f * (float)size.y;
}

static inline void tri_bbox(const Vec3f* const p0, const Vec3f* const p1, const Vec3f* const p2, float* const bbox) {
    bbox[0] = MIN(MIN(p0->x, p1->x), p2->x);
    bbox[1] = MIN(MIN(p0->y, p1->y), p2->y);
    bbox[2] = MAX(MAX(p0->x, p1->x), p2->x);
    bbox[3] = MAX(MAX(p0->y, p1->y), p2->y);
}

static inline float edge(const Vec3f* const a, const Vec3f* const b, const Vec3f* const test) {
    return (test->x - a->x) * (b->y - a->y) - (test->y - a->y) * (b->x - a->x);
}

static Pixel shade(const Texture* texture, Vec2f uv) {
    if (texture->frameBuffer != NULL) {
        float u = uv.x;
        float v = uv.y;

        // Convert normalized coordinates to texel coordinates
        Vec2i texel;
		texel.x = (int)MIN(u * texture->size.x, texture->size.x - 1);
		texel.y = (int)MIN(v * texture->size.y, texture->size.y - 1);

        // Get the color from the texture at the texel position
        // return texture->frameBuffer[texel.y * texture->size.x + texel.x];
        return texture_read(texture, texel);
    }
}

static inline void rasterize(int x0, int y0, int x1, int y1, const Vec3f* const p0, const Vec3f* const p1, const Vec3f* const p2, const Vec2f* const uv0, const Vec2f* const uv1, const Vec2f* const uv2, const Texture* const texture, const Vec2i scrSize, Renderer* r) {

    // printf("x0: %d, y0: %d, x1: %d, y1: %d\n", x0, y0, x1, y1);
    // printf("p0: (%f, %f, %f), p1: (%f, %f, %f), p2: (%f, %f, %f)\n", p0->x, p0->y, p0->z, p1->x, p1->y, p1->z, p2->x, p2->y, p2->z);
    // printf("uv0: (%f, %f), uv1: (%f, %f), uv2: (%f, %f)\n", uv0->x, uv0->y, uv1->x, uv1->y, uv2->x, uv2->y);
    // printf("scrSize: (%d, %d)\n", scrSize.x, scrSize.y);

    float inv_area = 1.0f / edge(p0, p1, p2);  // Precompute the inverse of the area
    // printf("inv_area: %f\n", inv_area);

    Vec3f pixel, sample;
    pixel.y = y0;

    for (int scrY = y0, row = y0 * scrSize.x; scrY <= y1; ++scrY, pixel.y += 1, row += scrSize.x) {
        pixel.x = x0;
        for (int scrX = x0, index = row + x0; scrX <= x1; ++scrX, pixel.x += 1, ++index) {
            sample.x = pixel.x + 0.5f;
            sample.y = pixel.y + 0.5f;

            // Print sample positions
            // printf("scrX: %d, scrY: %d, sample.x: %f, sample.y: %f\n", scrX, scrY, sample.x, sample.y);

            float w0 = edge(p1, p2, &sample) * inv_area;
            float w1 = edge(p2, p0, &sample) * inv_area;
            float w2 = edge(p0, p1, &sample) * inv_area;

            // Print barycentric weights
            // printf("w0: %f, w1: %f, w2: %f\n", w0, w1, w2);

            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                float one_over_z = w0 / p0->z + w1 / p1->z + w2 / p2->z;
                float z = 1.0f / one_over_z;

                // Print depth value
                // printf("z: %f, one_over_z: %f\n", z, one_over_z);

                if (depth_check(r->backEnd->getZetaBuffer(r, r->backEnd), scrX + scrY * scrSize.x, z)) {
                    // printf("Depth check failed at (scrX: %d, scrY: %d)\n", scrX, scrY);
                    continue;
                }

                depth_write(r->backEnd->getZetaBuffer(r, r->backEnd), scrX + scrY * scrSize.x, z);

                // Interpolate the texture coordinates
                Vec2f uv;
                uv.x = (uv0->x * w0 + uv1->x * w1 + uv2->x * w2) * z;
                uv.y = (uv0->y * w0 + uv1->y * w1 + uv2->y * w2) * z;

                // Print texture coordinates
                // printf("uv.x: %f, uv.y: %f\n", uv.x, uv.y);

                // Shade the pixel and update the color buffer
                Pixel color = shade(texture, uv);

                // Print the color values before drawing
                // printf("Color (r: %u, g: %u, b: %u, a: %u) at (scrX: %d, scrY: %d)\n", color.r, color.g, color.b, color.a, scrX, scrY);

                backendDrawPixel(r, &r->frameBuffer, (Vec2i) { scrX, scrY }, color, 1.0f);
            }
        }
    }
}

// FUNCTIONS BELOW ARE NOT FROM SCRATCHPIXEL 
// BUT WE PUT THEM HERE BECAUSE THEY'RE NOT PINGO EITHER
void mat4ExtractPerspective(const Mat4* m, float* near, float* far, float* aspect, float* fov) {
    // Extract the relevant elements from the matrix
    float w = m->elements[0];  // Element at (0, 0)
    float h = m->elements[5];  // Element at (1, 1)
    float a = m->elements[10]; // Element at (2, 2)
    float b = m->elements[14]; // Element at (2, 3)

    // Calculate the aspect ratio and field of view
    *aspect = w / h;
    *fov = 2.0 * atan(1.0 / h);

    // Calculate the near and far planes
    *far = b / (a + 1.0);
    *near = b / (a - 1.0);
}

// Decode the rgba2222 pixel format
Pixel rgba2222_to_pixel(uint8_t data) {
    uint8_t a = (data >> 6) & 0b11;
    uint8_t b = (data >> 4) & 0b11;
    uint8_t g = (data >> 2) & 0b11;
    uint8_t r = data & 0b11;

    // Map the 2-bit values to 8-bit color values
    static const uint8_t mapping[4] = {0, 85, 170, 255};

    Pixel pixel;
    pixel.r = mapping[r];
    pixel.g = mapping[g];
    pixel.b = mapping[b];
    pixel.a = mapping[a];

    return pixel;
}

