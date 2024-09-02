#pragma once

#include <cstdint>
#include "transformable.hpp"
#include "renderable.hpp"
#include "../math/mat3.hpp"
#include "../math/mat4.hpp"

struct Transformable;
struct Renderable;

namespace p3d {

#define MAX_SCENE_RENDERABLES 32

struct Scene {
    uint16_t sid;  // Scene ID
    // Transformable stuff
    Vec3f scale;
    Vec3f rotation;
    Vec3f translation;
    Mat4 transform;
    bool modified;
    Vec3f rotation_loc;
    Vec3f translation_loc;
    bool modified_loc;
    bool is_camera;

    // Scene stuff
    uint8_t numberOfRenderables;
    Renderable renderables[MAX_SCENE_RENDERABLES];
    uint8_t visible;

    // Constructor
    Scene(uint16_t sid);
};

extern int sceneInit(Scene * s);
extern int sceneAddRenderable(Scene * scene, Renderable renderable);

extern Renderable sceneAsRenderable(Scene * scene);

extern int sceneInit(Scene * scene);
extern int sceneAddRenderable(Scene * scene, Renderable renderable);

extern Renderable sceneAsRenderable(Scene * scene);

} // namespace p3d