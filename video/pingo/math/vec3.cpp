#include "vec3.hpp"

// Function to create a Vec3f
Vec3f vec3f(float x, float y, float z) {
    return Vec3f{x, y, z};
}

// Function to multiply a Vec3f by a scalar
Vec3f vec3fmul(const Vec3f& a, float b) {
    return Vec3f{a.x * b, a.y * b, a.z * b};
}

// Function to add two Vec3f vectors
Vec3f vec3fsumV(const Vec3f& a, const Vec3f& b) {
    return Vec3f{a.x + b.x, a.y + b.y, a.z + b.z};
}

// Function to subtract two Vec3f vectors
Vec3f vec3fsubV(const Vec3f& a, const Vec3f& b) {
    return Vec3f{a.x - b.x, a.y - b.y, a.z - b.z};
}

// Function to add a scalar to a Vec3f
Vec3f vec3fsum(const Vec3f& a, float b) {
    return Vec3f{a.x + b, a.y + b, a.z + b};
}

// Function to calculate the dot product of two Vec3f vectors
float vec3Dot(const Vec3f& a, const Vec3f& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Function to calculate the cross product of two Vec3f vectors
Vec3f vec3Cross(const Vec3f& a, const Vec3f& b) {
    return Vec3f{
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

// Function to normalize a Vec3f vector
Vec3f vec3Normalize(const Vec3f& v) {
    float magnitude = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    if (magnitude != 0) {
        return Vec3f{v.x / magnitude, v.y / magnitude, v.z / magnitude};
    }
    return Vec3f{0, 0, 0};  // Return a zero vector if the magnitude is zero
}
