#pragma once

#include <cmath>             // Include for mathematical functions like PI

#include "transformable.hpp" // Include for Transformable struct
#include "../math/mat4.hpp"  // Include for Mat4 operations
#include "../math/vec3.hpp"  // Include for Vec3f operations

namespace p3d {

// Camera struct inheriting from Transformable
struct Camera : public Transformable {
    float fov;          // Field of view in radians
    float aspectRatio;  // Aspect ratio (width / height)
    float nearClip;     // Near clipping plane
    float farClip;      // Far clipping plane

    Mat4 camera_projection; // Projection matrix
    Mat4 camera_view;       // View matrix

    // Constructor
    Camera(const Mat4& transformMatrix, float fov, float aspectRatio, float nearClip, float farClip);

    // Methods specific to Camera
    void update_projection_matrix();
};

} // namespace p3d
