#pragma once

#include <cmath> 

// Integer-based 2D vector
struct Vec2i {
    int x;
    int y;
};

// Floating-point-based 2D vector
struct Vec2f {
    float x;
    float y;
};

// Function prototypes
Vec2f vector2fSum(const Vec2f& l, const Vec2f& r);
Vec2i vector2ISum(const Vec2i& l, const Vec2i& r);
Vec2f vecItoF(const Vec2i& v);
Vec2i vecFtoI(const Vec2f& v);