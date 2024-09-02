#pragma once

#include "../math/mat4.hpp"
#include "../math/vec3.hpp"
#include "transformable.hpp"  // Include Transformable base struct
#include "mesh.hpp"           // Include Mesh struct
#include "renderable.hpp"     // Include Renderable struct
#include "material.hpp"       // Include Material struct
#include <displaycontroller.h> // Include for fabgl::Bitmap

namespace p3d {

// Object struct inheriting from Transformable
struct Object : public Transformable {
    uint16_t oid;  // Object ID

    // Constructor
    Object(uint16_t oid);
};

// TexObject struct inheriting from Object
struct TexObject : public Object {
    Mesh* mesh;                // Pointer to the Mesh associated with the Object
    fabgl::Bitmap* texture;    // Pointer to the texture bitmap used by the Object
    uint16_t* tex_indices;     // Pointer to texture indices
    Vec2f* textCoord;          // Pointer to texture coordinates

    // Constructor
    TexObject(uint16_t oid, Mesh* mesh, fabgl::Bitmap* texture, uint16_t* tex_indices, Vec2f* textCoord);

    // Method to convert Object to Renderable
    Renderable texObject_as_renderable();
};

} // namespace p3d
