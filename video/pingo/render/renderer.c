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
#include <esp_heap_caps.h>

#ifndef MIN
#define MIN(a, b)(((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b)(((a) > (b)) ? (a) : (b))
#endif
#ifndef Z_THRESHOLD
#define Z_THRESHOLD 0.000001f
#endif
#if DEBUG
extern void show_pixel(float x, float y, uint8_t a, uint8_t b, uint8_t g, uint8_t r);
#endif


int rendererInit(Renderer * r, Vec2i size, BackEnd * backEnd) {
    printf("Initalizing Renderer\n");
    renderingFunctions[RENDERABLE_SPRITE] = & renderSprite;
    renderingFunctions[RENDERABLE_SCENE] = & renderScene;
    renderingFunctions[RENDERABLE_OBJECT] = & renderObject;
    // renderingFunctions[RENDERABLE_OBJECT] = & renderObjectHecker;

    r->scene = 0;
    r->backEnd = backEnd;

    r->frameBuffer.size = size;
    printf("Frame buffer initialized\n");

    int zsize = sizeof(PingoDepth) * size.x * size.y;
    r->z_buffer = (PingoDepth*) heap_caps_malloc(zsize, MALLOC_CAP_SPIRAM);
    printf("Z buffer initialized\n");
    printf("Renderer initialized\n");
    return 0;
}

int rendererSetScene(Renderer * r, Scene * s) {
    if (s == 0)
        return 1; //nullptr scene

    r->scene = s;
    return 0;
}

int rendererSetCamera(Renderer * r, Vec2i rect) {
    r->camera.rect = rect;
    r->frameBuffer.size = rect;
    return 0;
}

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
    Mat4 newMat = mat4Translate((Vec3f) { -r->camera.translation.x, -r->camera.translation.y, 0 });
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

int rendererRender(Renderer * r) {
    int num_pixels = r->frameBuffer.size.x * r->frameBuffer.size.y;

    memset(r->z_buffer, 0, num_pixels * sizeof (PingoDepth));

    Pixel* framePixels = r->frameBuffer.pixels;
    if (r->clear == REND_CLEAR) {
        memset(framePixels, r->clearColor.c, num_pixels * sizeof(Pixel));
    } else if (r->clear == REND_BACKGROUND) {
        Pixel* backgroundPixels = r->background.pixels;
        memcpy(framePixels, backgroundPixels, num_pixels * sizeof(Pixel));
    }

    renderScene(mat4Identity(), r, sceneAsRenderable(r->scene));

    return 0;
}

static inline void rasterize(int x0, int y0, int x1, int y1, const Vec3f* const p0, const Vec3f* const p1, const Vec3f* const p2, const Vec2f* const uv0, const Vec2f* const uv1, const Vec2f* const uv2, const Texture* const texture, const Vec2i scrSize, Renderer* r, float near, float diffuseLight) {
    float inv_area = 1.0f / edge(p0, p1, p2);

    Vec3f sample;
    Vec2f intersections[2];
    int intersection_count;

    // Iterate over scanlines within the bounding box
    for (int scrY = y0; scrY <= y1; ++scrY) {
        // Find the intersection points of the current scanline with the triangle edges
        find_scanline_intersections(p0, p1, p2, scrY, intersections, &intersection_count);

        // Continue only if exactly two intersection points are found
        if (intersection_count != 2) continue;

        // Sort the intersections by x-coordinate
        int x_start = (int)MAX(x0, (int)intersections[0].x);
        int x_end = (int)MIN(x1, (int)intersections[1].x);

        // Iterate over pixels between the intersections on the current scanline
        for (int scrX = x_start, index = scrY * scrSize.x + x_start; scrX <= x_end; ++scrX, ++index) {
            sample.x = scrX;
            sample.y = scrY;

            // Barycentric coordinates for pixel coverage
            float w0 = edge(p1, p2, &sample) * inv_area;
            float w1 = edge(p2, p0, &sample) * inv_area;
            float w2 = edge(p0, p1, &sample) * inv_area;

            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                float inv_z = w0 / p0->z + w1 / p1->z + w2 / p2->z;
                if (inv_z < -near) {
                    continue;
                }
                float z = 1.0f / inv_z;

                if (depth_check(r->z_buffer, scrX + scrY * scrSize.x, -inv_z)) {
                    continue;
                }

                depth_write(r->z_buffer, scrX + scrY * scrSize.x, -inv_z);

                Pixel color = {255};
                if (texture) {
                    // Interpolate the texture coordinates
                    Vec2f uv;
                    uv.x = (uv0->x * w0 + uv1->x * w1 + uv2->x * w2) * z;
                    uv.y = (uv0->y * w0 + uv1->y * w1 + uv2->y * w2) * z;

                    // Shade the pixel and update the color buffer
                    color = shade(texture, uv);
                }

                backendDrawPixel(r, &r->frameBuffer, (Vec2i) { scrX, scrY }, color, diffuseLight);
            }
        }
    }
}

int orient2d( Vec2i a,  Vec2i b,  Vec2i c)
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
        // texture_draw(f, pos, pixelMul(color,illumination));
        texture_draw(f, pos, color);
    }
}

// SCRATCHPIXEL FUNCTIONS
// https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/perspective-correct-interpolation-vertex-attributes.html
static inline void persp_divide(struct Vec3f* p) {
    if (p->z > -Z_THRESHOLD) {
        p->z = -Z_THRESHOLD;
    }
    float inv_z = -1.0f / p->z;
    p->x *= inv_z;
    p->y *= inv_z;
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
    if (texture->pixels != NULL) {
        float u = uv.x;
        float v = uv.y;

        // Convert normalized coordinates to texel coordinates
        Vec2i texel;
		texel.x = (int)MIN(u * texture->size.x, texture->size.x - 1);
		texel.y = (int)MIN(v * texture->size.y, texture->size.y - 1);

        // Get the color from the texture at the texel position
        return texture_read(texture, texel);
    }
}

// NOT SCRATCHPIXEL BUT NOT PINGO EITHER
// Helper function to find intersection of a line segment with the screen bounds
static inline int clip_edge(float y, const Vec3f* const v0, const Vec3f* const v1, Vec2f* out) {
    // Check if the edge is horizontal (skip as no intersection along the scanline)
    if (v0->y == v1->y) return 0;

    // Calculate intersection x-coordinate of the line segment with the horizontal line y
    float t = (y - v0->y) / (v1->y - v0->y);
    if (t < 0 || t > 1) return 0; // Intersection not within the segment

    out->x = v0->x + t * (v1->x - v0->x);
    out->y = y;
    return 1; // Valid intersection found
}

// Function to find scanline intersections for triangle edges
static void find_scanline_intersections(const Vec3f* p0, const Vec3f* p1, const Vec3f* p2, int scanline_y, Vec2f* out_intersections, int* count) {
    // Initialize intersection count
    *count = 0;
    
    // Check intersection with the first edge (p0-p1)
    if (clip_edge((float)scanline_y, p0, p1, &out_intersections[*count])) {
        (*count)++;
    }

    // Check intersection with the second edge (p1-p2)
    if (clip_edge((float)scanline_y, p1, p2, &out_intersections[*count])) {
        (*count)++;
    }

    // Check intersection with the third edge (p2-p0)
    if (*count < 2 && clip_edge((float)scanline_y, p2, p0, &out_intersections[*count])) {
        (*count)++;
    }

    // Sort intersections by x-coordinate if two valid intersections are found
    if (*count == 2 && out_intersections[0].x > out_intersections[1].x) {
        Vec2f temp = out_intersections[0];
        out_intersections[0] = out_intersections[1];
        out_intersections[1] = temp;
    }
}

int renderObject(Mat4 object_transform, Renderer *r, Renderable ren) {
    const Vec2i screenSize = r->frameBuffer.size;
    Object *object = ren.impl;
    
    // Fetch mesh and vertex data
    Mesh *mesh = object->mesh;
    Vertex *vertices = mesh->vertices;
    int idx_count = mesh->indexes_count;

    // Precompute model-view-projection matrix once
    Mat4 mtxMdl = mat4MultiplyM(&object->transform, &object_transform);
    Mat4 mtxMdlVw = mat4MultiplyM(&mtxMdl, &r->camera.view);
    Mat4 mtxMdlVwProj = mat4MultiplyM(&mtxMdlVw, &r->camera.projection);
    float nearPlane = r->camera.near;

    // Use a pointer to iterate over the vertices array
    Vertex *v_ptr = vertices;  // Start pointer at the first vertex

    // Process each triangle
    for (int i = 0; i < idx_count; i += 3) {
        // Access the vertices directly via the pointer
        Vertex *v1 = v_ptr++;
        Vertex *v2 = v_ptr++;
        Vertex *v3 = v_ptr++;

        // Convert positions to Vec4f and apply transformation
        Vec4f ver1 = {v1->position.x, v1->position.y, v1->position.z, 1};
        Vec4f ver2 = {v2->position.x, v2->position.y, v2->position.z, 1};
        Vec4f ver3 = {v3->position.x, v3->position.y, v3->position.z, 1};

        ver1 = mat4MultiplyVec4(&ver1, &mtxMdlVwProj);
        ver2 = mat4MultiplyVec4(&ver2, &mtxMdlVwProj);
        ver3 = mat4MultiplyVec4(&ver3, &mtxMdlVwProj);

        // Early culling if behind near plane
        if (ver1.z > -nearPlane && ver2.z > -nearPlane && ver3.z > -nearPlane)
            continue;

        // Perspective divide
        persp_divide((Vec3f *)&ver1);
        persp_divide((Vec3f *)&ver2);
        persp_divide((Vec3f *)&ver3);

        // Backface culling
        if ((ver2.y - ver1.y) * (ver3.x - ver2.x) - (ver3.y - ver2.y) * (ver2.x - ver1.x) >= 0)
            continue;

        // Convert to raster space
        to_raster(screenSize, (Vec3f *)&ver1);
        to_raster(screenSize, (Vec3f *)&ver2);
        to_raster(screenSize, (Vec3f *)&ver3);

        // Compute bounding box
        float bbx[4];
        tri_bbox((Vec3f *)&ver1, (Vec3f *)&ver2, (Vec3f *)&ver3, bbx);

        // Bounding box constraints
        if (bbx[0] > screenSize.x - 1 || bbx[2] < 0 || bbx[1] > screenSize.y - 1 || bbx[3] < 0)
            continue;

        int xMin = MAX(0, (int)bbx[0]);
        int yMin = MAX(0, (int)bbx[1]);
        int xMax = MIN(screenSize.x - 1, (int)bbx[2]);
        int yMax = MIN(screenSize.y - 1, (int)bbx[3]);

        // Texture coordinates processing
        Vec2f textureCoord1 = v1->uv;
        Vec2f textureCoord2 = v2->uv;
        Vec2f textureCoord3 = v3->uv;

        // Perspective correct
        textureCoord1.x /= ver1.z;
        textureCoord1.y /= ver1.z;
        textureCoord2.x /= ver2.z;
        textureCoord2.y /= ver2.z;
        textureCoord3.x /= ver3.z;
        textureCoord3.y /= ver3.z;

        // Rasterize the triangle
        rasterize(xMin, yMin, xMax, yMax, 
                  (Vec3f *)&ver1, 
                  (Vec3f *)&ver2, 
                  (Vec3f *)&ver3, 
                  &textureCoord1, &textureCoord2, &textureCoord3, 
                  object->material->texture, screenSize, r, nearPlane, 1.0);
    }

    return 0;
}
