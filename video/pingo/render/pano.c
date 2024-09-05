#include "pano.h"
#include <math.h>  // Include for math functions and constants like M_PI and fmod

// Helper to set yaw angle and update state
void setYaw(Pano* pano, float new_yaw) {
    // Update yaw angle, keep it within the range [0, 2π)
    pano->yaw = fmod(new_yaw, 2 * M_PI);
    if (pano->yaw < 0) pano->yaw += 2 * M_PI;
}

// Helper to compute the horizontal pixel mapping for the current yaw
void computeHorizontalMapping(Pano* pano, int* horizontalMapping) {
    int imageWidth = pano->size.x;
    int viewportWidth = pano->viewport_width;
    float yaw = pano->yaw;

    // Loop through each pixel column in the viewport
    for (int x = 0; x < viewportWidth; x++) {
        // Calculate the longitude for the current pixel in the viewport
        float longitude = ((float)x / viewportWidth) * 2 * M_PI - M_PI;

        // Adjust longitude by the current yaw
        float adjustedLongitude = fmod(longitude + yaw, 2 * M_PI);
        if (adjustedLongitude < 0) adjustedLongitude += 2 * M_PI;

        // Map adjusted longitude back to image coordinates (0 to imageWidth)
        horizontalMapping[x] = (int)((adjustedLongitude + M_PI) / (2 * M_PI) * imageWidth);
    }
}

// Helper to compute the vertical pixel mapping based on FOV
void computeVerticalMapping(Pano* pano, int* verticalMapping) {
    int imageHeight = pano->size.y;
    int viewportHeight = pano->viewport_height;
    float verticalStep = pano->vertical_step;

    // Loop through each pixel row in the viewport
    for (int y = 0; y < viewportHeight; y++) {
        // Compute the corresponding y in the background image
        verticalMapping[y] = (int)(y * verticalStep);
        
        // Ensure the index is within bounds of the image height
        if (verticalMapping[y] >= imageHeight) {
            verticalMapping[y] = imageHeight - 1;
        }
    }
}

// Combined helper function to prepare all parameters for rendering
void preparePanoRendering(Pano* pano, int* horizontalMapping, int* verticalMapping) {
    // Compute the horizontal mapping for the current yaw
    computeHorizontalMapping(pano, horizontalMapping);

    // Compute the vertical mapping based on the current FOV
    computeVerticalMapping(pano, verticalMapping);
}
