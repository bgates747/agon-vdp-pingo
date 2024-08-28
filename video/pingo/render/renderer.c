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

#define Z_THRESHOLD -0.000001f

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

#define MIN(a, b)(((a) < (b)) ? (a) : (b))
#define MAX(a, b)(((a) > (b)) ? (a) : (b))

int edgeFunction(const Vec2f * a, const Vec2f * b, const Vec2f * c) {
    return (c->x - a->x) * (b->y - a->y) - (c->y - a->y) * (b->x - a->x);
}

float isClockWise(float x1, float y1, float x2, float y2, float x3, float y3) {
    return (y2 - y1) * (x3 - x2) - (y3 - y2) * (x2 - x1);
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
        texture_draw(f, pos, pixelMul(color,illumination));
    }
}

int renderObject(Mat4 object_transform, Renderer * r, Renderable ren) {

    const Vec2i scrSize = r->frameBuffer.size;
    Object * o = ren.impl;
    Vec2f * tex_coords = o->textCoord;

    // MODEL MATRIX
    Mat4 m = mat4MultiplyM( &o->transform, &object_transform  );

    // VIEW MATRIX
    Mat4 v = r->camera_view;
    Mat4 p = r->camera_projection;

    // CAMERA NORMAL
    Vec3f cameraNormal = { 
        v.elements[2],  // forward.x
        v.elements[6],  // forward.y
        v.elements[10]  // forward.z
    };
    cameraNormal = vec3Normalize(cameraNormal);

    for (int i = 0; i < o->mesh->indexes_count; i += 3) {
        Vec3f * ver1 = &o->mesh->positions[o->mesh->pos_indices[i+0]];
        Vec3f * ver2 = &o->mesh->positions[o->mesh->pos_indices[i+1]];
        Vec3f * ver3 = &o->mesh->positions[o->mesh->pos_indices[i+2]];

        Vec4f a =  { ver1->x, ver1->y, ver1->z, 1 };
        Vec4f b =  { ver2->x, ver2->y, ver2->z, 1 };
        Vec4f c =  { ver3->x, ver3->y, ver3->z, 1 };

        a = mat4MultiplyVec4( &a, &m);
        b = mat4MultiplyVec4( &b, &m);
        c = mat4MultiplyVec4( &c, &m);

        // FACE NORMAL
        Vec3f na = vec3fsubV(*((Vec3f*)(&a)), *((Vec3f*)(&b)));
        Vec3f nb = vec3fsubV(*((Vec3f*)(&a)), *((Vec3f*)(&c)));
        Vec3f cameraNormal = vec3Normalize(vec3Cross(na, nb));

        // Cull triangles facing away from camera
        float faceCamDot = vec3Dot(cameraNormal, (Vec3f){0,0,1});
        if (faceCamDot < 0)
            continue;

        float diffuseLight = 1.0; // default to full illumination from all directions
        if (false) { // set to true for lighting effects at the expense of performance
            Vec3f light = vec3Normalize((Vec3f){-3,8,5});
            diffuseLight = (1.0 + vec3Dot(cameraNormal, light)) *0.5;
            diffuseLight = MIN(1.0, MAX(diffuseLight, 0));
        }

        a = mat4MultiplyVec4( &a, &v);
        b = mat4MultiplyVec4( &b, &v);
        c = mat4MultiplyVec4( &c, &v);

        a = mat4MultiplyVec4( &a, &p);
        b = mat4MultiplyVec4( &b, &p);
        c = mat4MultiplyVec4( &c, &p);

        //Triangle is completely behind camera
        if (a.z > 0 && b.z > 0 && c.z > 0)
           continue;

        // convert to device coordinates by perspective division
        a.w = 1.0 / a.w;
        b.w = 1.0 / b.w;
        c.w = 1.0 / c.w;
        a.x *= a.w; a.y *= a.w; a.z *= a.w;
        b.x *= b.w; b.y *= b.w; b.z *= b.w;
        c.x *= c.w; c.y *= c.w; c.z *= c.w;

        float clocking = isClockWise(a.x, a.y, b.x, b.y, c.x, c.y);
        if (clocking >= 0)
            continue;

        //Compute Screen coordinates
        float halfX = scrSize.x/2;
        float halfY = scrSize.y/2;
        Vec2i a_s = {a.x * halfX + halfX,  a.y * halfY + halfY};
        Vec2i b_s = {b.x * halfX + halfX,  b.y * halfY + halfY};
        Vec2i c_s = {c.x * halfX + halfX,  c.y * halfY + halfY};

        int32_t minX = MIN(MIN(a_s.x, b_s.x), c_s.x);
        int32_t minY = MIN(MIN(a_s.y, b_s.y), c_s.y);
        int32_t maxX = MAX(MAX(a_s.x, b_s.x), c_s.x);
        int32_t maxY = MAX(MAX(a_s.y, b_s.y), c_s.y);

        minX = MIN(MAX(minX, 0), r->frameBuffer.size.x);
        minY = MIN(MAX(minY, 0), r->frameBuffer.size.y);
        maxX = MIN(MAX(maxX, 0), r->frameBuffer.size.x);
        maxY = MIN(MAX(maxY, 0), r->frameBuffer.size.y);

        // Barycentric coordinates at minX/minY corner
        Vec2i minTriangle = { minX, minY };

        int32_t area =  orient2d( a_s, b_s, c_s);
        if (area == 0)
            continue;
        float areaInverse = 1.0/area;

        int32_t A01 = ( a_s.y - b_s.y); //Barycentric coordinates steps
        int32_t B01 = ( b_s.x - a_s.x); //Barycentric coordinates steps
        int32_t A12 = ( b_s.y - c_s.y); //Barycentric coordinates steps
        int32_t B12 = ( c_s.x - b_s.x); //Barycentric coordinates steps
        int32_t A20 = ( c_s.y - a_s.y); //Barycentric coordinates steps
        int32_t B20 = ( a_s.x - c_s.x); //Barycentric coordinates steps

        int32_t w0_row = orient2d( b_s, c_s, minTriangle);
        int32_t w1_row = orient2d( c_s, a_s, minTriangle);
        int32_t w2_row = orient2d( a_s, b_s, minTriangle);

        Vec2f tca = {0,0};
        Vec2f tcb = {0,0};
        Vec2f tcc = {0,0};

        if (o->material != 0) {
            tca = tex_coords[o->tex_indices[i+0]];
            tcb = tex_coords[o->tex_indices[i+1]];
            tcc = tex_coords[o->tex_indices[i+2]];            
            // Perspective correct texture coordinates
            tca.x /= a.z;
            tca.y /= a.z;
            tcb.x /= b.z;
            tcb.y /= b.z;
            tcc.x /= c.z;
            tcc.y /= c.z;
        }

        for (int16_t y = minY; y < maxY; y++, w0_row += B12,w1_row += B20,w2_row += B01) {
            int32_t w0 = w0_row;
            int32_t w1 = w1_row;
            int32_t w2 = w2_row;

            for (int32_t x = minX; x < maxX; x++, w0 += A12, w1 += A20, w2 += A01) {

                if ((w0 | w1 | w2) < 0)
                    continue;

                float depth =  -( w0 * a.z + w1 * b.z + w2 * c.z ) * areaInverse;
                if (depth < 0.0 || depth > 1.0)
                    continue;

                if (depth_check(r->backEnd->getZetaBuffer(r,r->backEnd), x + y * scrSize.x, 1-depth ))
                    continue;

                depth_write(r->backEnd->getZetaBuffer(r,r->backEnd), x + y * scrSize.x, 1- depth );

                // Invert the y-coordinate for screen space
                int32_t inverted_y = scrSize.y - y - 1;

                if (o->material != 0) {
                    // Texture lookup
                    float textCoordx = -(w0 * tca.x + w1 * tcb.x + w2 * tcc.x) * areaInverse * depth;
                    float textCoordy = -(w0 * tca.y + w1 * tcb.y + w2 * tcc.y) * areaInverse * depth;

                    Pixel text = texture_readF(o->material->texture, (Vec2f){textCoordx,textCoordy});
        #if DEBUG
                    //show_pixel(textCoordx, textCoordy, text.a, text.b, text.g, text.r);
        #endif

                    backendDrawPixel(r, &r->frameBuffer, (Vec2i){x, inverted_y}, text, diffuseLight);
                } else {
                    Pixel pixel;
                    pixel.a = 255;
                    pixel.b = 255;
                    pixel.g = 255;
                    pixel.r = 255;
                    backendDrawPixel(r, &r->frameBuffer, (Vec2i){x, inverted_y}, pixel, diffuseLight);
                }

            }
        }
    }

    return 0;
};

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
    memset(r->backEnd->getZetaBuffer(r,r->backEnd), 0, pixels * sizeof (PingoDepth));

    r->backEnd->beforeRender(r, r->backEnd);

    //get current framebuffe from backend
    r->frameBuffer.frameBuffer = r->backEnd->getFrameBuffer(r, r->backEnd);

    //Clear draw buffer before rendering
    if (r->clear) {
        memset(r->backEnd->getFrameBuffer(r,r->backEnd), 0, pixels * sizeof (Pixel));
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
    float inv_area = 1.0f / edge(p0, p1, p2);

    Vec3f pixel, sample;
    pixel.y = y0;

    for (int scrY = y0, row = y0 * scrSize.x; scrY <= y1; ++scrY, pixel.y += 1, row += scrSize.x) {
        pixel.x = x0;
        for (int scrX = x0, index = row + x0; scrX <= x1; ++scrX, pixel.x += 1, ++index) {
            sample.x = pixel.x + 0.5f;
            sample.y = pixel.y + 0.5f;

            float w0 = edge(p1, p2, &sample) * inv_area;
            float w1 = edge(p2, p0, &sample) * inv_area;
            float w2 = edge(p0, p1, &sample) * inv_area;

            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                float one_over_z = w0 / p0->z + w1 / p1->z + w2 / p2->z;
                float z = 1.0f / one_over_z;

                if (depth_check(r->backEnd->getZetaBuffer(r, r->backEnd), scrX + scrY * scrSize.x, z)) {
                    continue;
                }

                depth_write(r->backEnd->getZetaBuffer(r, r->backEnd), scrX + scrY * scrSize.x, z);

                // Interpolate the texture coordinates
                Vec2f uv;
                uv.x = (uv0->x * w0 + uv1->x * w1 + uv2->x * w2) * z;
                uv.y = (uv0->y * w0 + uv1->y * w1 + uv2->y * w2) * z;

                // Shade the pixel and update the color buffer
                Pixel color = shade(texture, uv);

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

