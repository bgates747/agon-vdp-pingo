#include "scene.hpp"
#include "renderer.hpp"

namespace p3d {

// Constructor to initialize Scene
Scene::Scene(uint16_t sid)
    : sid(sid),  // Initialize Scene ID
    numberOfRenderables(0),  // Initialize number of renderables
    visible(1)  // Initialize visibility flag
{
    this->is_camera = false;
}

int sceneAddRenderable(Scene * scene, Renderable renderable) {
    if (scene->numberOfRenderables >= MAX_SCENE_RENDERABLES) {
        return 1; //Too many renderables in this scene
    }

    scene->renderables[scene->numberOfRenderables++] = renderable;
    return 0;
}

int sceneInit(Scene * scene) {
    scene->transform = mat4Identity();
    scene->numberOfRenderables = 0;
    scene->visible = 1;

    return 0;
}

extern Renderable sceneAsRenderable(Scene * scene) {
    return (Renderable){.renderableType = RENDERABLE_SCENE, .impl = scene};
}

} // namespace p3d