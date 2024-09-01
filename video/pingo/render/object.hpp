#pragma once

#include "../math/mat4.hpp"
#include "../math/vec3.hpp"
#include "transformable.hpp"  // Include Transformable base struct
#include "mesh.hpp"           // Include Mesh struct
#include "renderable.hpp"     // Include Renderable struct
#include "material.hpp"       // Include Material struct

namespace p3d {

// Object struct inheriting from Transformable
struct Object : public Transformable {
    Mesh* mesh;                // Pointer to the Mesh associated with the Object
    Material* material;        // Pointer to the Material used by the Object
    uint16_t* tex_indices;     // Pointer to texture indices
    Vec2f* textCoord;          // Pointer to texture coordinates

    // Constructor
    Object(Mesh* mesh, Material* material);

    // Method to convert Object to Renderable
    Renderable as_renderable();
};

} // namespace p3d
