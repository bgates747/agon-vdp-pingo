#include <cstdio>

#include "test.hpp"

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

// Function to run all Vec2 tests
void run_all_tests() {
    test_vec2f_operations();
    test_vec2i_operations();
}
