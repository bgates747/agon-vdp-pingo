#include "camera.hpp"

namespace p3d {

// Constructor to initialize Camera settings with a given transformation matrix
Camera::Camera(const Mat4& transformMatrix, float fov, float aspectRatio, float nearClip, float farClip)
    : Transformable(),  // Call base constructor for Transformable
      fov(fov),
      aspectRatio(aspectRatio),
      nearClip(nearClip),
      farClip(farClip)
{
    this->transform = transformMatrix;  // Set the transform matrix
    this->scale = {1.0f, 1.0f, 1.0f};  // Ensure the scale is set to 1
    this->rotation = {0.0f, 0.0f, 0.0f};  // Initialize rotation to zero
    this->translation = {0.0f, 0.0f, 0.0f};  // Initialize translation to origin
    this->is_camera = true;  // Mark this Transformable as a camera

    update_projection_matrix(); // Initialize the projection matrix
}

// Method to update the projection matrix
void Camera::update_projection_matrix() {
    camera_projection = mat4Perspective(fov, aspectRatio, nearClip, farClip);
}

} // namespace p3d
