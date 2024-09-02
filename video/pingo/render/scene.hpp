#pragma once

#include <cstdint>
#include "transformable.hpp"
#include "renderable.hpp"
#include "../math/mat3.hpp"
#include "../math/mat4.hpp"

namespace p3d {

#define MAX_SCENE_RENDERABLES 32

struct Scene : public Transformable {  // Scene inherits from Transformable
    uint16_t sid;  // Scene ID

    // Scene-specific members
    uint8_t numberOfRenderables;
    Renderable renderables[MAX_SCENE_RENDERABLES];
    uint8_t visible;

    // Constructor
    Scene(uint16_t sid);
};

// Function prototypes for Scene operations

// Initializes the scene
extern int sceneInit(Scene* s);

// Adds a Renderable to the Scene
extern int sceneAddRenderable(Scene* scene, Renderable renderable);

// Converts a Scene to a Renderable
extern Renderable sceneAsRenderable(Scene* scene);

} // namespace p3d
