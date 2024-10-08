#pragma once

#include "../math/mat4.h"

#include "mesh.h"
#include "renderable.h"
#include "material.h"

typedef struct Object {
    Mesh * mesh;
    Mat4 transform;
    Material * material;
    uint16_t * tex_indices;
    Vec2f * textCoord;
} Object;

Renderable object_as_renderable(Object * object);

