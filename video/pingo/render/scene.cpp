#include "scene.hpp"
#include "renderer.hpp"
#include "../math/mat4.hpp"  // Include for mat4Identity()

namespace p3d {

// Constructor to initialize Scene
Scene::Scene(uint16_t sid)
    : Transformable(),  // Call the base constructor for Transformable
      sid(sid),         // Initialize Scene ID
      numberOfRenderables(0),  // Initialize number of renderables
      visible(1)  // Initialize visibility flag
{
    // Initialize the Transformable part
    this->scale = {1.0f, 1.0f, 1.0f};  // Default scale
    this->rotation = {0.0f, 0.0f, 0.0f};  // Default rotation
    this->translation = {0.0f, 0.0f, 0.0f};  // Default translation
    this->is_camera = false;  // Scene is not a camera
    compute_transformation_matrix(*this);  // Initialize the transformation matrix
}

// Adds a Renderable to the Scene
int sceneAddRenderable(Scene* scene, Renderable renderable) {
    if (scene->numberOfRenderables >= MAX_SCENE_RENDERABLES) {
        return 1; // Too many renderables in this scene
    }

    scene->renderables[scene->numberOfRenderables++] = renderable;
    return 0;
}

// Initializes the Scene
int sceneInit(Scene* scene) {
    scene->transform = mat4Identity();  // Initialize the transform matrix to identity
    scene->numberOfRenderables = 0;     // Reset number of renderables
    scene->visible = 1;                 // Set scene to visible

    // Initialize the Transformable part
    initialize_scale(*scene);
    initialize(*scene);

    return 0;
}

// Converts a Scene to a Renderable
Renderable sceneAsRenderable(Scene* scene) {
    return Renderable{RENDERABLE_SCENE, scene};
}

} // namespace p3d
