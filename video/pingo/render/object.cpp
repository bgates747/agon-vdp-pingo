#include "object.hpp"

namespace p3d {

// Constructor to initialize Object
Object::Object(uint16_t oid) 
    : Transformable(),  // Call the base constructor for Transformable
      oid(oid)  // Initialize Object ID
{
    this->scale = {1.0f, 1.0f, 1.0f};  // Default scale
    this->rotation = {0.0f, 0.0f, 0.0f};  // Default rotation
    this->translation = {0.0f, 0.0f, 0.0f};  // Default translation
    compute_transformation_matrix(*this);  // Initialize the transformation matrix
}

// Constructor to initialize TexObject
TexObject::TexObject(Mesh* mesh, Material* material, uint16_t oid) 
    : Object(oid),  // Call the base constructor for Object
      mesh(mesh),
      material(material),
      tex_indices(nullptr),
      textCoord(nullptr) {}

// Method to convert TexObject to Renderable
Renderable TexObject::as_renderable() {
    Renderable renderable;
    renderable.renderableType = RENDERABLE_OBJECT;
    renderable.impl = static_cast<void*>(this);
    return renderable;
}

} // namespace p3d
