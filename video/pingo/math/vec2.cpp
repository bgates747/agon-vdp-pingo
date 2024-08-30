#include "vec2.hpp"

// Function to add two Vec2f vectors
Vec2f vector2fSum(const Vec2f& l, const Vec2f& r) {
    Vec2f result;
    result.x = l.x + r.x;
    result.y = l.y + r.y;
    return result;
}

// Function to add two Vec2i vectors
Vec2i vector2ISum(const Vec2i& l, const Vec2i& r) {
    Vec2i result;
    result.x = l.x + r.x;
    result.y = l.y + r.y;
    return result;
}

// Function to convert a Vec2i to a Vec2f
Vec2f vecItoF(const Vec2i& v) {
    Vec2f result;
    result.x = static_cast<float>(v.x);
    result.y = static_cast<float>(v.y);
    return result;
}

// Function to convert a Vec2f to a Vec2i
Vec2i vecFtoI(const Vec2f& v) {
    Vec2i result;
    result.x = static_cast<int>(v.x);
    result.y = static_cast<int>(v.y);
    return result;
}