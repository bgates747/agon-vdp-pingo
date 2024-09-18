#include "hecker.h"

// code shamelessly stolen from https://chrishecker.com/images/4/41/Gdmtex1.pdf
// src/orig/DIVFLFL.CPP

#ifndef Z_THRESHOLD
#define Z_THRESHOLD 0.000001f
#endif

// Macro to safely perform division, preventing division by values near zero
#define SAFE_DIV(numerator, denominator) ((fabs(denominator) > Z_THRESHOLD) ? ((numerator) / (denominator)) : 0.0f)

// Initialize gradients structure with register-efficient optimizations
void InitializeGradients(Gradients *gradients, const Vertex *pVertices) {
    // Load vertex positions and texture coordinates directly into local variables for register efficiency
    float pos0x = pVertices[0].position.x;
    float pos0y = pVertices[0].position.y;
    float pos0z = pVertices[0].position.z;
    float pos1x = pVertices[1].position.x;
    float pos1y = pVertices[1].position.y;
    float pos1z = pVertices[1].position.z;
    float pos2x = pVertices[2].position.x;
    float pos2y = pVertices[2].position.y;
    float pos2z = pVertices[2].position.z;

    float uv0x = pVertices[0].uv.x;
    float uv0y = pVertices[0].uv.y;
    float uv1x = pVertices[1].uv.x;
    float uv1y = pVertices[1].uv.y;
    float uv2x = pVertices[2].uv.x;
    float uv2y = pVertices[2].uv.y;

    // Calculate differences in x and y once and store them
    float deltaX1 = pos1x - pos2x;
    float deltaX0 = pos0x - pos2x;
    float deltaY1 = pos1y - pos2y;
    float deltaY0 = pos0y - pos2y;

    // Combine calculation of 1/dX and 1/dY early to minimize intermediate memory usage
    float denominator = (deltaX1 * deltaY0) - (deltaX0 * deltaY1);
    float OneOverdX = SAFE_DIV(1.0f, denominator);
    float OneOverdY = -OneOverdX;

    // Precompute and store 1/z for each vertex
    float OneOverZ0 = SAFE_DIV(1.0f, pos0z);
    float OneOverZ1 = SAFE_DIV(1.0f, pos1z);
    float OneOverZ2 = SAFE_DIV(1.0f, pos2z);

    gradients->aOneOverZ[0] = OneOverZ0;
    gradients->aOneOverZ[1] = OneOverZ1;
    gradients->aOneOverZ[2] = OneOverZ2;

    // Precompute U/z and V/z for each vertex and store
    gradients->aUOverZ[0] = uv0x * OneOverZ0;
    gradients->aUOverZ[1] = uv1x * OneOverZ1;
    gradients->aUOverZ[2] = uv2x * OneOverZ2;

    gradients->aVOverZ[0] = uv0y * OneOverZ0;
    gradients->aVOverZ[1] = uv1y * OneOverZ1;
    gradients->aVOverZ[2] = uv2y * OneOverZ2;

    // Calculate gradients for 1/z, U/z, and V/z with respect to x and y
    // Combine similar calculations to minimize loading the same variables repeatedly
    float dOneOverZ1_Z2 = OneOverZ1 - OneOverZ2;
    float dOneOverZ0_Z2 = OneOverZ0 - OneOverZ2;

    gradients->dOneOverZdX = OneOverdX * ((dOneOverZ1_Z2 * deltaY0) - (dOneOverZ0_Z2 * deltaY1));
    gradients->dOneOverZdY = OneOverdY * ((dOneOverZ1_Z2 * deltaX0) - (dOneOverZ0_Z2 * deltaX1));

    // U/z gradients
    float dUOverZ1_U2 = gradients->aUOverZ[1] - gradients->aUOverZ[2];
    float dUOverZ0_U2 = gradients->aUOverZ[0] - gradients->aUOverZ[2];

    gradients->dUOverZdX = OneOverdX * ((dUOverZ1_U2 * deltaY0) - (dUOverZ0_U2 * deltaY1));
    gradients->dUOverZdY = OneOverdY * ((dUOverZ1_U2 * deltaX0) - (dUOverZ0_U2 * deltaX1));

    // V/z gradients
    float dVOverZ1_V2 = gradients->aVOverZ[1] - gradients->aVOverZ[2];
    float dVOverZ0_V2 = gradients->aVOverZ[0] - gradients->aVOverZ[2];

    gradients->dVOverZdX = OneOverdX * ((dVOverZ1_V2 * deltaY0) - (dVOverZ0_V2 * deltaY1));
    gradients->dVOverZdY = OneOverdY * ((dVOverZ1_V2 * deltaX0) - (dVOverZ0_V2 * deltaX1));
}

// Initialize edge structure with register-efficient optimizations
void InitializeEdge(Edge *edge, const Gradients *gradients,
                    const Vertex *pVertices, int Top, int Bottom) {
    // Load vertex positions into local variables for efficiency
    float TopPosX = pVertices[Top].position.x;
    float TopPosY = pVertices[Top].position.y;
    float BottomPosX = pVertices[Bottom].position.x;
    float BottomPosY = pVertices[Bottom].position.y;

    // Load gradients for Top vertex
    float TopOneOverZ = gradients->aOneOverZ[Top];
    float TopUOverZ = gradients->aUOverZ[Top];
    float TopVOverZ = gradients->aVOverZ[Top];

    // Calculate Y coordinates, height, and step values early
    edge->y = (int)ceil(TopPosY);
    int YEnd = (int)ceil(BottomPosY);
    float YPrestep = edge->y - TopPosY;
    float RealHeight = BottomPosY - TopPosY;
    float RealWidth = BottomPosX - TopPosX;

    // Calculate initial x position and x step
    edge->x = SAFE_DIV((RealWidth * YPrestep), RealHeight) + TopPosX;
    edge->XStep = SAFE_DIV(RealWidth, RealHeight);

    // Precalculate XPrestep
    float XPrestep = edge->x - TopPosX;

    // Calculate 1/z, U/z, and V/z, minimizing repeated access to gradients
    float dOneOverZdX = gradients->dOneOverZdX;
    float dOneOverZdY = gradients->dOneOverZdY;

    edge->OneOverZ = TopOneOverZ + YPrestep * dOneOverZdY + XPrestep * dOneOverZdX;
    edge->OneOverZStep = edge->XStep * dOneOverZdX + dOneOverZdY;

    // Calculate U/z values
    float dUOverZdX = gradients->dUOverZdX;
    float dUOverZdY = gradients->dUOverZdY;

    edge->UOverZ = TopUOverZ + YPrestep * dUOverZdY + XPrestep * dUOverZdX;
    edge->UOverZStep = edge->XStep * dUOverZdX + dUOverZdY;

    // Calculate V/z values
    float dVOverZdX = gradients->dVOverZdX;
    float dVOverZdY = gradients->dVOverZdY;

    edge->VOverZ = TopVOverZ + YPrestep * dVOverZdY + XPrestep * dVOverZdX;
    edge->VOverZStep = edge->XStep * dVOverZdX + dVOverZdY;

    // Calculate the edge height
    edge->Height = YEnd - edge->y;
}

// Step function for Edge
void EdgeStep(Edge *edge) {
    edge->x += edge->XStep;
    edge->y++;
    edge->Height--;

    edge->OneOverZ += edge->OneOverZStep;
    edge->UOverZ += edge->UOverZStep;
    edge->VOverZ += edge->VOverZStep;
}

// DrawScanLine function with SAFE_DIV macro applied
void DrawScanLine(Texture *pDest, const Gradients *gradients, const Edge *pLeft, const Edge *pRight, const Texture *pTexture) {
    int XStart = (int)ceil(pLeft->x);
    float XPrestep = XStart - pLeft->x;
    Pixel *pDestPixel = pDest->pixels + ((pLeft->y) * pDest->size.x) + XStart;
    int Width = (int)ceil(pRight->x) - XStart;

    // Interpolated values at the start of the scanline
    float OneOverZ = pLeft->OneOverZ + XPrestep * gradients->dOneOverZdX;
    float UOverZ = pLeft->UOverZ + XPrestep * gradients->dUOverZdX;
    float VOverZ = pLeft->VOverZ + XPrestep * gradients->dVOverZdX;

    if (Width > 0) {
        while (Width--) {
            // Interpolated Z value using SAFE_DIV
            float z = SAFE_DIV(1.0f, OneOverZ);

            // Interpolated texture coordinates
            float u = UOverZ * z;
            float v = VOverZ * z;

            // Ensure texture coordinates are within bounds [0, 1]
            u = fmodf(u, 1.0f);
            if (u < 0) u += 1.0f;
            v = fmodf(v, 1.0f);
            if (v < 0) v += 1.0f;

            // Sample the texture at interpolated (u, v) coordinates
            int tex_x = (int)(u * pTexture->size.x);
            int tex_y = (int)(v * pTexture->size.y);
            Pixel tex_color = pTexture->pixels[tex_y * pTexture->size.x + tex_x];

            // Draw the pixel
            *pDestPixel = tex_color;
            pDestPixel++;

            // Increment interpolated values for next pixel
            OneOverZ += gradients->dOneOverZdX;
            UOverZ += gradients->dUOverZdX;
            VOverZ += gradients->dVOverZdX;
        }
    }
}

// TextureMapTriangle function with additional debug output
void TextureMapTriangle(Texture *pDest, const Vertex *pVertices, Texture *pTexture) {
    int Top, Middle, Bottom;
    int MiddleCompare, BottomCompare;
    float Y0 = pVertices[0].position.y;
    float Y1 = pVertices[1].position.y;
    float Y2 = pVertices[2].position.y;

    // Debug: Print initial vertex positions
    // printf("Initial Vertex Positions:\n");
    for (int i = 0; i < 3; i++) {
        // printf("V%d: (%f, %f, %f)\n", i, pVertices[i].position.x, pVertices[i].position.y, pVertices[i].position.z);
    }

    // Sort vertices in y
    if (Y0 < Y1) {
        if (Y2 < Y0) {
            Top = 2; Middle = 0; Bottom = 1;
            MiddleCompare = 0; BottomCompare = 1;
        } else {
            Top = 0;
            if (Y1 < Y2) {
                Middle = 1; Bottom = 2;
                MiddleCompare = 1; BottomCompare = 2;
            } else {
                Middle = 2; Bottom = 1;
                MiddleCompare = 2; BottomCompare = 1;
            }
        }
    } else {
        if (Y2 < Y1) {
            Top = 2; Middle = 1; Bottom = 0;
            MiddleCompare = 1; BottomCompare = 0;
        } else {
            Top = 1;
            if (Y0 < Y2) {
                Middle = 0; Bottom = 2;
                MiddleCompare = 0; BottomCompare = 2;
            } else {
                Middle = 2; Bottom = 0;
                MiddleCompare = 2; BottomCompare = 0;
            }
        }
    }

    // Debug: Print sorted vertex indices
    // printf("Sorted Vertices: Top=%d, Middle=%d, Bottom=%d\n", Top, Middle, Bottom);

    Gradients gradients;
    InitializeGradients(&gradients, pVertices);
    // printf("Gradients Initialized: dOneOverZdX=%f, dOneOverZdY=%f, dUOverZdX=%f, dUOverZdY=%f\n", gradients.dOneOverZdX, gradients.dOneOverZdY, gradients.dUOverZdX, gradients.dUOverZdY);

    Edge TopToBottom, TopToMiddle, MiddleToBottom;
    InitializeEdge(&TopToBottom, &gradients, pVertices, Top, Bottom);
    InitializeEdge(&TopToMiddle, &gradients, pVertices, Top, Middle);
    InitializeEdge(&MiddleToBottom, &gradients, pVertices, Middle, Bottom);

    // Debug: Print edge initialization data
    // printf("Edges Initialized:\n");
    // printf("TopToBottom: y=%d, Height=%d, XStep=%f\n", TopToBottom.y, TopToBottom.Height, TopToBottom.XStep);
    // printf("TopToMiddle: y=%d, Height=%d, XStep=%f\n", TopToMiddle.y, TopToMiddle.Height, TopToMiddle.XStep);
    // printf("MiddleToBottom: y=%d, Height=%d, XStep=%f\n", MiddleToBottom.y, MiddleToBottom.Height, MiddleToBottom.XStep);

    Edge *pLeft, *pRight;
    int MiddleIsLeft;

    // The triangle is clockwise, so if bottom > middle then middle is right
    if (BottomCompare > MiddleCompare) {
        MiddleIsLeft = 0;
        pLeft = &TopToBottom; pRight = &TopToMiddle;
    } else {
        MiddleIsLeft = 1;
        pLeft = &TopToMiddle; pRight = &TopToBottom;
    }

    // Debug: Print which side is left and which is right
    // printf("MiddleIsLeft: %d\n", MiddleIsLeft);

    int Height = TopToMiddle.Height;
    if (Height > 0) {
        // printf("Rendering top half from y=%d for %d scanlines\n", TopToMiddle.y, Height);
    } else {
        // printf("Skipping top half, no scanlines to draw.\n");
    }

    while (Height-- > 0) {
        DrawScanLine(pDest, &gradients, pLeft, pRight, pTexture);
        EdgeStep(pLeft);
        EdgeStep(pRight);
    }

    Height = MiddleToBottom.Height;
    if (Height > 0) {
        // printf("Rendering bottom half from y=%d for %d scanlines\n", MiddleToBottom.y, Height);
    } else {
        // printf("Skipping bottom half, no scanlines to draw.\n");
    }

    if (MiddleIsLeft) {
        pLeft = &MiddleToBottom; pRight = &TopToBottom;
    } else {
        pLeft = &TopToBottom; pRight = &MiddleToBottom;
    }

    while (Height-- > 0) {
        DrawScanLine(pDest, &gradients, pLeft, pRight, pTexture);
        EdgeStep(pLeft);
        EdgeStep(pRight);
    }
}
