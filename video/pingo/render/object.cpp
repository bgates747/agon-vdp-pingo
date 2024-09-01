#include "object.hpp"

namespace p3d {

// Constructor to initialize Object
Object::Object(Mesh* mesh, Material* material, uint16_t oid) 
    : Transformable(),  // Call the base constructor for Transformable
      mesh(mesh),
      material(material),
      tex_indices(nullptr),
      textCoord(nullptr)
{
    this->scale = {1.0f, 1.0f, 1.0f};  // Default scale
    this->rotation = {0.0f, 0.0f, 0.0f};  // Default rotation
    this->translation = {0.0f, 0.0f, 0.0f};  // Default translation
    this->oid = oid;  // Set the Object ID
    compute_transformation_matrix(*this);  // Initialize the transformation matrix
}

// Method to convert Object to Renderable
Renderable Object::as_renderable() {
    Renderable renderable;
    renderable.renderableType = RENDERABLE_OBJECT;
    renderable.impl = static_cast<void*>(this);
    return renderable;
}

} // namespace p3d