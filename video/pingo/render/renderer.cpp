#include "renderer.hpp"
namespace p3d {

// Renderer constructor
Renderer::Renderer(Scene* scene, Camera* camera, uint16_t width, uint16_t height, fabgl::RGBA2222 clearColor, int clear)
    : scene(scene),
      camera(camera),
      frameBuffer(nullptr),
      background(nullptr),
      z_buffer(nullptr),
      clear(clear),
      clearColor(clearColor) {
        // renderingFunctions[RENDERABLE_SPRITE] = & renderSprite; // TODO: Implement when needed
        renderingFunctions[RENDERABLE_SCENE] = & renderScene;
        renderingFunctions[RENDERABLE_TEXOBJECT] = & renderTexObject;
}

void renderRenderable(Mat4 transform, Renderer * r, Renderable ren) {
    renderingFunctions[ren.renderableType](transform, r, ren);
};

int renderScene(Mat4 transform, Renderer* r, Renderable ren) {
    Scene* scene = static_cast<Scene*>(ren.impl);
    
    if (!scene->visible)
        return 0;

    // Apply hierarchy transform
    Mat4 newTransform = mat4MultiplyM(&scene->transform, &transform);
    for (int i = 0; i < scene->numberOfRenderables; i++) {
        renderRenderable(newTransform, r, scene->renderables[i]);
    }
    return 0;
}

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

void backendDrawPixel (Renderer * r, fabgl::Bitmap * f, Vec2i pos, fabgl::RGBA2222 color, float illumination) {
    f->setPixel(pos.x, pos.y, color);
}

int renderTexObject(Mat4 object_transform, Renderer * r, Renderable ren) {
    Camera * camera = r->camera;
    fabgl::Bitmap frameBuffer = *r->frameBuffer;
    const Vec2i scrSize = {frameBuffer.width, frameBuffer.height};
    float halfX = scrSize.x / 2.0f;
    float halfY = scrSize.y / 2.0f;
    TexObject* o = static_cast<TexObject*>(ren.impl);
    Vec2f* tex_coords = o->textCoord;
    fabgl::Bitmap* texture = o->texture;
    Vec2i tex_size = {texture->width, texture->height};
    PingoDepth * z_buffer = r->z_buffer;

    // MODEL MATRIX
    Mat4 m = mat4MultiplyM( &o->transform, &object_transform  );

    // VIEW MATRIX
    Mat4 v = camera->camera_view;
    Mat4 p = camera->camera_projection;

    for (int i = 0; i < o->mesh->indexes_count; i += 3) {
        Vec3f * ver1 = &o->mesh->positions[o->mesh->pos_indices[i+0]];
        Vec3f * ver2 = &o->mesh->positions[o->mesh->pos_indices[i+1]];
        Vec3f * ver3 = &o->mesh->positions[o->mesh->pos_indices[i+2]];

        Vec2f tca = {0,0};
        Vec2f tcb = {0,0};
        Vec2f tcc = {0,0};

        if (texture != 0) {
            tca = tex_coords[o->tex_indices[i+0]];
            tcb = tex_coords[o->tex_indices[i+1]];
            tcc = tex_coords[o->tex_indices[i+2]];
        }

        Vec4f a =  { ver1->x, ver1->y, ver1->z, 1 };
        Vec4f b =  { ver2->x, ver2->y, ver2->z, 1 };
        Vec4f c =  { ver3->x, ver3->y, ver3->z, 1 };

        a = mat4MultiplyVec4( &a, &m);
        b = mat4MultiplyVec4( &b, &m);
        c = mat4MultiplyVec4( &c, &m);

        float diffuseLight = 1.0; // default to full illumination from all directions
        if (true) { // set to true for lighting effects at the expense of performance
            //Calc Face Normal
            Vec3f na = vec3fsubV(*((Vec3f*)(&a)), *((Vec3f*)(&b)));
            Vec3f nb = vec3fsubV(*((Vec3f*)(&a)), *((Vec3f*)(&c)));
            Vec3f normal = vec3Normalize(vec3Cross(na, nb));
            Vec3f light = vec3Normalize((Vec3f){-3,8,5});
            diffuseLight = (1.0 + vec3Dot(normal, light)) *0.5;
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

        Vec2i a_s = {static_cast<int>(a.x * halfX + halfX), static_cast<int>(a.y * halfY + halfY)};
        Vec2i b_s = {static_cast<int>(b.x * halfX + halfX), static_cast<int>(b.y * halfY + halfY)};
        Vec2i c_s = {static_cast<int>(c.x * halfX + halfX), static_cast<int>(c.y * halfY + halfY)};

        int32_t minX = MIN(MIN(a_s.x, b_s.x), c_s.x);
        int32_t minY = MIN(MIN(a_s.y, b_s.y), c_s.y);
        int32_t maxX = MAX(MAX(a_s.x, b_s.x), c_s.x);
        int32_t maxY = MAX(MAX(a_s.y, b_s.y), c_s.y);

        minX = MIN(MAX(minX, 0), scrSize.x);
        minY = MIN(MAX(minY, 0), scrSize.y);
        maxX = MIN(MAX(maxX, 0), scrSize.x);
        maxY = MIN(MAX(maxY, 0), scrSize.y);

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

        if (texture != 0) {
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
            int ulast = -1;
            int vlast = -1;
            fabgl::RGBA2222 color = {0,0,0,0};

            for (int32_t x = minX; x < maxX; x++, w0 += A12, w1 += A20, w2 += A01) {

                if ((w0 | w1 | w2) < 0)
                    continue;

                float depth =  -( w0 * a.z + w1 * b.z + w2 * c.z ) * areaInverse;
                if (depth < 0.0 || depth > 1.0)
                    continue;

                int idx = (y * scrSize.x) + x;
                if (depth_check(z_buffer, idx, 1-depth))
                    continue;
                depth_write(z_buffer, idx, 1-depth);

                // Invert the y-coordinate for screen space
                int32_t inverted_y = scrSize.y - y - 1;

                if (texture != 0) {
                    // Texture lookup
                    int u = static_cast<int>(-(w0 * tca.x + w1 * tcb.x + w2 * tcc.x) * areaInverse * depth);
                    int v = static_cast<int>(-(w0 * tca.y + w1 * tcb.y + w2 * tcc.y) * areaInverse * depth);
                    if (u != ulast || v != vlast) {
                        ulast = u;
                        vlast = v;
                        color = texture->getPixel2222(u, v);
                    }
                    // TODO: implement diffuse lighting
                    texture->setPixel(x, inverted_y, color);
                } else {
                    // TODO: Implement flat shading
                    // fabgl::RGBA2222 pixel;
                    // pixel.A = 255;
                    // pixel.B = 255;
                    // pixel.G = 255;
                    // pixel.R = 255;
                    // backendDrawPixel(r, &frameBuffer, (Vec2i){x, inverted_y}, pixel, diffuseLight);
                }

            }
        }
    }

    return 0;
};

int rendererRender(Renderer * r) {
    int numpixels = r->frameBuffer->height * r->frameBuffer->width;
    memset(r->z_buffer, 0, numpixels * sizeof (PingoDepth));

    if (r->clear == 1) {
        uint8_t packedColor = p3d::RGBA2222ToUint8(r->clearColor);
        memset(r->frameBuffer, packedColor, numpixels * sizeof(fabgl::RGBA2222));
    }

    renderScene(mat4Identity(), r, sceneAsRenderable(r->scene));
    return 0;
}

// DEPRECATED
// int rendererSetScene(Renderer * r, Scene * scene) {
//     if (scene == 0)
//         return 1; //nullptr scene

//     r->scene = scene;
//     return 0;
// }

// DEPRECATED
// int rendererSetCamera(Renderer * r, Vec4i rect) {
//     r->camera = rect;
//     r->backEnd->init(r, r->backEnd, rect);
//     scrSize = (Vec2i) {rect.z, rect.w};
//     return 0;
// }



} // namespace p3d