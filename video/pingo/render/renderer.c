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

static inline void rasterize(int x0, int y0, int x1, int y1, const Vertex* const ver1, const Vertex* const ver2, const Vertex* const ver3, const Texture* const texture, const Vec2i scrSize, Renderer* r, float near, float diffuseLight) {
    float inv_area = 1.0f / edge(&ver1->position, &ver2->position, &ver3->position);

    Vec3f sample;
    Vec2f intersections[2];
    int intersection_count;

    // Iterate over scanlines within the bounding box
    for (int scrY = y0; scrY <= y1; ++scrY) {
        // Find the intersection points of the current scanline with the triangle edges
        find_scanline_intersections(&ver1->position, &ver2->position, &ver3->position, scrY, intersections, &intersection_count);

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
            float w1 = edge(&ver2->position, &ver3->position, &sample) * inv_area;
            float w2 = edge(&ver3->position, &ver1->position, &sample) * inv_area;
            float w3 = edge(&ver1->position, &ver2->position, &sample) * inv_area;

            if (w1 >= 0 && w2 >= 0 && w3 >= 0) {
                float inv_z = w1 / ver1->position.z + w2 / ver2->position.z + w3 / ver3->position.z;
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
                    // Interpolate the texture coordinates using the UV coordinates in Vertex
                    Vec2f uv;
                    uv.x = (ver1->uv.x * w1 + ver2->uv.x * w2 + ver3->uv.x * w3) * z;
                    uv.y = (ver1->uv.y * w1 + ver2->uv.y * w2 + ver3->uv.y * w3) * z;

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

static inline void tri_bbox(const Vertex* const ver1, const Vertex* const ver2, const Vertex* const ver3, float* const bbox) {
    bbox[0] = MIN(MIN(ver1->position.x, ver2->position.x), ver3->position.x); // x min
    bbox[1] = MIN(MIN(ver1->position.y, ver2->position.y), ver3->position.y); // y min
    bbox[2] = MAX(MAX(ver1->position.x, ver2->position.x), ver3->position.x); // x max
    bbox[3] = MAX(MAX(ver1->position.y, ver2->position.y), ver3->position.y); // y max
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
static inline int clip_edge(float y, const Vec3f* const ver1, const Vec3f* const ver2, Vec2f* out) {
    // Check if the edge is horizontal (skip as no intersection along the scanline)
    if (ver1->y == ver2->y) return 0;

    // Calculate intersection x-coordinate of the line segment with the horizontal line y
    float t = (y - ver1->y) / (ver2->y - ver1->y);
    if (t < 0 || t > 1) return 0; // Intersection not within the segment

    out->x = ver1->x + t * (ver2->x - ver1->x);
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

static inline void process_vertex(Vertex* v, const Mat4* transform, const Vec2i screenSize) {
    // Matrix multiplication (apply the model-view-projection matrix) on the vertex position
    float x = v->position.x * transform->elements[0] + v->position.y * transform->elements[1] + v->position.z * transform->elements[2] + 1.0f * transform->elements[3];
    float y = v->position.x * transform->elements[4] + v->position.y * transform->elements[5] + v->position.z * transform->elements[6] + 1.0f * transform->elements[7];
    float z = v->position.x * transform->elements[8] + v->position.y * transform->elements[9] + v->position.z * transform->elements[10] + 1.0f * transform->elements[11];
    float w = v->position.x * transform->elements[12] + v->position.y * transform->elements[13] + v->position.z * transform->elements[14] + 1.0f * transform->elements[15];
    
    Vec4f transformed = {x, y, z, w}; // Create a Vec4f to store the transformed position

    // Perspective divide
    if (transformed.z > -Z_THRESHOLD) {
        transformed.z = -Z_THRESHOLD;
    }
    float inv_z = -1.0f / transformed.z;
    transformed.x *= inv_z;
    transformed.y *= inv_z;

    // Convert to raster space
    transformed.x = (transformed.x + 1) * 0.5f * (float)screenSize.x;
    transformed.y = (1 - transformed.y) * 0.5f * (float)screenSize.y;

    // Update the vertex position with the transformed values
    v->position.x = transformed.x;
    v->position.y = transformed.y;
    v->position.z = transformed.z; // Optional: Update z if needed for rasterization depth
}

int renderObject(Mat4 object_transform, Renderer *r, Renderable ren) {
    const Vec2i screenSize = r->frameBuffer.size;
    Object *object = ren.impl;
    Mesh *mesh = object->mesh;
    Vertex *vertices = mesh->vertices;
    int idx_count = mesh->indexes_count;
    Mat4 mtxMdl = mat4MultiplyM(&object->transform, &object_transform);
    Mat4 mtxMdlVw = mat4MultiplyM(&mtxMdl, &r->camera.view);
    Mat4 mtxMdlVwProj = mat4MultiplyM(&mtxMdlVw, &r->camera.projection);
    float nearPlane = r->camera.near;
    Vertex *v_ptr = vertices;

    for (int i = 0; i < idx_count; i += 3) {
        Vertex ver1 = *v_ptr++;
        process_vertex(&ver1, &mtxMdlVwProj, screenSize);  

        Vertex ver2 = *v_ptr++;
        process_vertex(&ver2, &mtxMdlVwProj, screenSize);  
        
        Vertex ver3 = *v_ptr++;
        process_vertex(&ver3, &mtxMdlVwProj, screenSize);  

        // Near plane culling
        if (ver1.position.z > -nearPlane && ver2.position.z > -nearPlane && ver3.position.z > -nearPlane)
            continue;

        // Backface culling
        if ((ver2.position.y - ver1.position.y) * (ver3.position.x - ver2.position.x) - 
            (ver3.position.y - ver2.position.y) * (ver2.position.x - ver1.position.x) >= 0)
            continue;

        float bbx[4];
        tri_bbox(&ver1, &ver2, &ver3, bbx);  
        // Bounding box culling
        if (bbx[0] > screenSize.x - 1 || bbx[2] < 0 || bbx[1] > screenSize.y - 1 || bbx[3] < 0)
            continue;

        int xMin = MAX(0, (int)bbx[0]);
        int yMin = MAX(0, (int)bbx[1]);
        int xMax = MIN(screenSize.x - 1, (int)bbx[2]);
        int yMax = MIN(screenSize.y - 1, (int)bbx[3]);

        // Perspective correct UV coordinates
        ver1.uv.x /= ver1.position.z;
        ver1.uv.y /= ver1.position.z;
        ver2.uv.x /= ver2.position.z;
        ver2.uv.y /= ver2.position.z;
        ver3.uv.x /= ver3.position.z;
        ver3.uv.y /= ver3.position.z;

        // Rasterize the triangle
        rasterize(xMin, yMin, xMax, yMax, &ver1, &ver2, &ver3, object->material->texture, screenSize, r, nearPlane, 1.0f);
    }

    return 0;
}
