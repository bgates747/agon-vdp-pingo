#pragma once

#include "../math/mat4.hpp"
#include "../math/vec3.hpp"
#include <cstring>  // For std::memset
#include <cstdio>   // For std::printf
#include <cmath>    // For mathematical constants like PI

namespace p3d {

// Transformable struct definition
struct Transformable {
    Vec3f scale;
    Vec3f rotation;
    Vec3f translation;
    Mat4 transform;
    bool modified;

    Vec3f rotation_loc;
    Vec3f translation_loc;

    bool modified_loc;
    bool is_camera;
};

// Function prototypes for Transformable operations

// Initializes the scale of a Transformable to default values
void initialize_scale(Transformable& t);

// Initializes a Transformable instance to default values
void initialize(Transformable& t);

// Computes the forward direction vector from the rotation of a Transformable
Vec3f get_forward_direction(const Transformable& t);

// Computes the transformation matrix for a Transformable
void compute_transformation_matrix(Transformable& t);

// Computes the local transformation matrix for a Transformable
void compute_transformation_matrix_local(Transformable& t);

// Dumps the current state of a Transformable to the console (for debugging)
void dump(const Transformable& t);

} // namespace p3d
