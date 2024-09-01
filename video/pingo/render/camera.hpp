#pragma once

#include "transformable.hpp" // Include for Transformable struct
#include "../math/mat4.hpp"  // Include for Mat4 operations
#include "../math/vec3.hpp"  // Include for Vec3f operations
#include <memory>            // Include for smart pointers
#include <cmath>             // Include for mathematical functions like PI

namespace p3d {

// Camera struct inheriting from Transformable
struct Camera : public Transformable {
    float fov;          // Field of view in radians
    float aspectRatio;  // Aspect ratio (width / height)
    float nearClip;     // Near clipping plane
    float farClip;      // Far clipping plane

    Mat4 projectionMatrix; // Projection matrix

    // Constructor
    Camera(const Vec3f& position, float fov, float aspectRatio, float nearClip, float farClip);

    // Methods specific to Camera
    void update_projection_matrix();
};

// Constructor to initialize Camera settings
Camera::Camera(const Vec3f& position, float fov, float aspectRatio, float nearClip, float farClip)
    : Transformable(),  // Call base constructor for Transformable
      fov(fov),
      aspectRatio(aspectRatio),
      nearClip(nearClip),
      farClip(farClip) 
{
    this->translation = position;  // Set initial position
    this->scale = {1.0f, 1.0f, 1.0f};  // Set default scale
    this->rotation = {0.0f, 0.0f, 0.0f};  // Initialize rotation
    this->is_camera = true;  // Mark this Transformable as a camera

    update_projection_matrix(); // Initialize the projection matrix
    compute_transformation_matrix(*this); // Compute the initial transformation matrix
}

// Method to update the projection matrix
void Camera::update_projection_matrix() {
    projectionMatrix = mat4Perspective(fov, aspectRatio, nearClip, farClip);
}

} // namespace p3d
