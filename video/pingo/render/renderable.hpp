#pragma once

#include "../math/mat4.hpp"

namespace p3d {

class Renderer; 

enum RenderableType {
    RENDERABLE_SCENE = 0,
    RENDERABLE_SPRITE,
    RENDERABLE_TEXOBJECT,
    RENDERABLE_COUNT,
};

struct Renderable {
    RenderableType renderableType;
    void* impl;
};

extern int (*renderingFunctions[RENDERABLE_COUNT])(Mat4 transform, Renderer*, Renderable);

} // namespace p3d