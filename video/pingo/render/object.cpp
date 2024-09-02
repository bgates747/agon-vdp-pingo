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
TexObject::TexObject(uint16_t oid, Mesh* mesh, fabgl::Bitmap* texture, uint16_t* tex_indices, Vec2f* textCoord)
    : Object(oid), 
      mesh(mesh), 
      texture(texture), 
      tex_indices(tex_indices), 
      textCoord(textCoord) 
{
    this->is_camera = false;
}

// Method to convert TexObject to Renderable
Renderable TexObject::texObject_as_renderable() {
    return Renderable{RENDERABLE_TEXOBJECT, this};
}

} // namespace p3d
