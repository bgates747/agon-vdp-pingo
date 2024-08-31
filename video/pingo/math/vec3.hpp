#pragma once

#include <cmath>
namespace p3d {

// Integer-based 3D vector
struct Vec3i {
    int x;
    int y;
    int z;
};

// Floating-point-based 3D vector
struct Vec3f {
    float x;
    float y;
    float z;
};

// Function prototypes for Vec3f operations
Vec3f vec3f(float x, float y, float z);
Vec3f vec3fmul(const Vec3f& a, float b);
Vec3f vec3fsumV(const Vec3f& a, const Vec3f& b);
Vec3f vec3fsubV(const Vec3f& a, const Vec3f& b);
Vec3f vec3fsum(const Vec3f& a, float b);
float vec3Dot(const Vec3f& a, const Vec3f& b);
Vec3f vec3Cross(const Vec3f& a, const Vec3f& b);
Vec3f vec3Normalize(const Vec3f& v);

} // namespace p3d