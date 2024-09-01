#pragma once

#include "../math/mat4.hpp"
#include "mesh.hpp"
#include "renderable.hpp"
#include "material.hpp"

namespace p3d {

struct Object {
    Mesh * mesh;
    Mat4 transform;
    Material * material;
    uint16_t * tex_indices;
    Vec2f * textCoord;
};

Renderable object_as_renderable(Object * object);

} // namespace p3d