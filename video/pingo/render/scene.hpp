#pragma once

#include <cstdint>
#include "renderable.hpp"
#include "../math/mat3.hpp"
#include "../math/mat4.hpp"

namespace p3d {

#define MAX_SCENE_RENDERABLES 32

struct Scene {
    uint8_t numberOfRenderables;
    Renderable renderables[MAX_SCENE_RENDERABLES];
    Mat4 transform;
    uint8_t visible;
};

extern int sceneInit(Scene * s);
extern int sceneAddRenderable(Scene * scene, Renderable renderable);

extern Renderable sceneAsRenderable(Scene * scene);

} // namespace p3d