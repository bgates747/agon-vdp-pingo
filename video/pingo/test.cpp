#include <cstdio>
#include <cmath>

#include "test.hpp"

namespace p3d {

// PASSED
// Function to test Vec2f (float) operations
void test_vec2f_operations() {
    Vec2f vecA = {1.5f, 2.3f};
    Vec2f vecB = {3.4f, 4.7f};

    printf("Running Vec2f (float) operations tests...\n");
    printf("Float Vector Operations:\n");
    printf("vecA = (%f, %f)\n", vecA.x, vecA.y);
    printf("vecB = (%f, %f)\n", vecB.x, vecB.y);

    Vec2f vecC = vector2fSum(vecA, vecB);  // Use the correct addition function
    printf("vecA + vecB = (%f, %f)\n", vecC.x, vecC.y);

    Vec2i vecA_int = vecFtoI(vecA);
    Vec2i vecB_int = vecFtoI(vecB);
    printf("Converted A (int) = (%d, %d)\n", vecA_int.x, vecA_int.y);
    printf("Converted B (int) = (%d, %d)\n", vecB_int.x, vecB_int.y);
}

// PASSED
// Function to test Vec2i (int) operations
void test_vec2i_operations() {
    printf("Running Vec2i (int) operations tests...\n");

    // Declare two integer vectors
    Vec2i vecA = {1, 2};
    Vec2i vecB = {3, 4};

    // Perform addition
    Vec2i sumI = vector2ISum(vecA, vecB);

    // Perform conversions
    Vec2f convertedA = vecItoF(vecA);
    Vec2f convertedB = vecItoF(vecB);

    // Print results
    printf("Integer Vector Operations:\n");
    printf("vecA = (%d, %d)\n", vecA.x, vecA.y);
    printf("vecB = (%d, %d)\n", vecB.x, vecB.y);
    printf("vecA + vecB = (%d, %d)\n", sumI.x, sumI.y);
    printf("Converted A (float) = (%f, %f)\n", convertedA.x, convertedA.y);
    printf("Converted B (float) = (%f, %f)\n", convertedB.x, convertedB.y);
}

// Function to test Vec3f operations
void test_vec3f_operations() {
    Vec3f vecA = vec3f(1.5f, 2.3f, 3.1f);
    Vec3f vecB = vec3f(3.4f, 4.7f, 5.6f);
    float scalar = 2.0f;

    printf("Running Vec3f (float) operations tests...\n");
    printf("Float Vector Operations:\n");
    printf("vecA = (%f, %f, %f)\n", vecA.x, vecA.y, vecA.z);
    printf("vecB = (%f, %f, %f)\n", vecB.x, vecB.y, vecB.z);

    // Test vec3fsumV
    Vec3f vecC = vec3fsumV(vecA, vecB);
    printf("vecA + vecB = (%f, %f, %f)\n", vecC.x, vecC.y, vecC.z);

    // Test vec3fsubV
    Vec3f vecD = vec3fsubV(vecA, vecB);
    printf("vecA - vecB = (%f, %f, %f)\n", vecD.x, vecD.y, vecD.z);

    // Test vec3fmul
    Vec3f vecE = vec3fmul(vecA, scalar);
    printf("vecA * %f = (%f, %f, %f)\n", scalar, vecE.x, vecE.y, vecE.z);

    // Test vec3fsum (adding a scalar)
    Vec3f vecF = vec3fsum(vecA, scalar);
    printf("vecA + %f = (%f, %f, %f)\n", scalar, vecF.x, vecF.y, vecF.z);

    // Test vec3Dot
    float dotProduct = vec3Dot(vecA, vecB);
    printf("vecA . vecB = %f\n", dotProduct);

    // Test vec3Cross
    Vec3f vecG = vec3Cross(vecA, vecB);
    printf("vecA x vecB = (%f, %f, %f)\n", vecG.x, vecG.y, vecG.z);

    // Test vec3Normalize
    Vec3f vecH = vec3Normalize(vecA);
    printf("Normalized vecA = (%f, %f, %f)\n", vecH.x, vecH.y, vecH.z);
}

// Function to test Vec3i operations
void test_vec3i_operations() {
    Vec3i vecA = {1, 2, 3};
    Vec3i vecB = {3, 4, 5};
    int scalar = 2;

    printf("Running Vec3i (int) operations tests...\n");
    printf("Integer Vector Operations:\n");
    printf("vecA = (%d, %d, %d)\n", vecA.x, vecA.y, vecA.z);
    printf("vecB = (%d, %d, %d)\n", vecB.x, vecB.y, vecB.z);

    // Test addition of Vec3i
    Vec3i vecC = {vecA.x + vecB.x, vecA.y + vecB.y, vecA.z + vecB.z};
    printf("vecA + vecB = (%d, %d, %d)\n", vecC.x, vecC.y, vecC.z);

    // Test subtraction of Vec3i
    Vec3i vecD = {vecA.x - vecB.x, vecA.y - vecB.y, vecA.z - vecB.z};
    printf("vecA - vecB = (%d, %d, %d)\n", vecD.x, vecD.y, vecD.z);

    // Test multiplication of Vec3i with scalar
    Vec3i vecE = {vecA.x * scalar, vecA.y * scalar, vecA.z * scalar};
    printf("vecA * %d = (%d, %d, %d)\n", scalar, vecE.x, vecE.y, vecE.z);
}

} // namespace p3d