#pragma once

#include "vec2.hpp"

namespace p3d {
struct Mat3 {
    float elements[9];
};

/* Returns identity
 |1|0|x|
 |0|1|y|
 |0|0|1|
*/
Mat3 mat3Identity();

/* Builds a clean translation matrix with x and y translation along relative axes
 |1|0|x|
 |0|1|y|
 |0|0|1|
*/
Mat3 mat3Translate(Vec2f l);

/* Builds a clean rotation matrix of Θ angle
 | c(Θ) | -s(Θ)  | 0 |
 | s(Θ) | c(Θ)   | 0 |
 | 0    | 0      | 1 |
*/
Mat3 mat3Rotate(float theta);


/* Builds a clean scale matrix of x, y scaling factors
 | x | 0 | 0 |
 | 0 | y | 0 |
 | 0 | 0 | 1 |
*/
Mat3 mat3Scale(Vec2f s);

//Multiply 2 component vector b 3x3 matrix
Vec2f mat3Multiply(Vec2f * v, Mat3 * t);

//Multiply 3x3 matrix with 3x3 matrix (v*t)
Mat3  mat3MultiplyM( Mat3 *v, Mat3 *t);

//Calculate homogeneous inverse of matrix
Mat3 mat3Inverse( Mat3 *v );

/* Calculate a complete matrix transformation with translation rotation and scale working as expected
 * Rotation and scaled are applied in reference to the provided origin
 */
Mat3 mat3Complete( Vec2f origin, Vec2f translation, Vec2f scale, float rotation );

//Calculate determinant of matrix
float mat3Determinant(Mat3 * m);

//If a matrix has only translation some optimization can be done during rendering.
int mat3IsOnlyTranslation(Mat3 *m);

//If a matrix has only translation and doubles the size some optimization can be done during rendering.
int mat3IsOnlyTranslationDoubled(Mat3 *m);

} // namespace p3d
