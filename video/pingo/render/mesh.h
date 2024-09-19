#pragma once

#include <stdint.h>

#include "../math/vec2.h"
#include "../math/vec3.h"

typedef struct {
    Vec3f position;  // Vertex position (x, y, z)
    Vec2f uv;        // UV coordinates (x=u, y=v)
    // Vec3f normal;    // Normal vector (x, y, z)
} Vertex;

typedef struct Mesh {
    int indexes_count;
    Vertex *vertices;
} Mesh;