#pragma once

#include <displaycontroller.h>

#include "renderable.hpp"
#include "../math/vec4.hpp"

namespace p3d {

struct Scene;
struct Renderer{
    Vec4i camera;
    Scene * scene;

    fabgl::Bitmap * frameBuffer;
    fabgl::RGBA2222 clearColor;
    int clear;

    Mat4 camera_projection;
    Mat4 camera_view;
};

extern int rendererRender(Renderer *);

extern int rendererInit(Renderer *, Vec2i size, struct tag_BackEnd * backEnd);

extern int rendererSetScene(Renderer *r, Scene * scene);

extern int rendererSetCamera(Renderer *r, Vec4i camera);

} // namespace p3d
