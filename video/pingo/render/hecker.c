#include "hecker.h"

// Initialize gradients structure
void InitializeGradients(Gradients *gradients, const Vertex *pVertices) {
    int Counter;
    float OneOverdX = 1 / (((pVertices[1].position.x - pVertices[2].position.x) *
                           (pVertices[0].position.y - pVertices[2].position.y)) -
                          ((pVertices[0].position.x - pVertices[2].position.x) *
                           (pVertices[1].position.y - pVertices[2].position.y)));
    float OneOverdY = -OneOverdX;

    for (Counter = 0; Counter < 3; Counter++) {
        float OneOverZ = 1 / pVertices[Counter].position.z;
        gradients->aOneOverZ[Counter] = OneOverZ;
        gradients->aUOverZ[Counter] = pVertices[Counter].uv.x * OneOverZ;  // Use uv.x instead of u
        gradients->aVOverZ[Counter] = pVertices[Counter].uv.y * OneOverZ;  // Use uv.y instead of v
    }

    gradients->dOneOverZdX = OneOverdX * (((gradients->aOneOverZ[1] - gradients->aOneOverZ[2]) *
                                           (pVertices[0].position.y - pVertices[2].position.y)) -
                                          ((gradients->aOneOverZ[0] - gradients->aOneOverZ[2]) *
                                           (pVertices[1].position.y - pVertices[2].position.y)));
    gradients->dOneOverZdY = OneOverdY * (((gradients->aOneOverZ[1] - gradients->aOneOverZ[2]) *
                                           (pVertices[0].position.x - pVertices[2].position.x)) -
                                          ((gradients->aOneOverZ[0] - gradients->aOneOverZ[2]) *
                                           (pVertices[1].position.x - pVertices[2].position.x)));

    gradients->dUOverZdX = OneOverdX * (((gradients->aUOverZ[1] - gradients->aUOverZ[2]) *
                                         (pVertices[0].position.y - pVertices[2].position.y)) -
                                        ((gradients->aUOverZ[0] - gradients->aUOverZ[2]) *
                                         (pVertices[1].position.y - pVertices[2].position.y)));
    gradients->dUOverZdY = OneOverdY * (((gradients->aUOverZ[1] - gradients->aUOverZ[2]) *
                                         (pVertices[0].position.x - pVertices[2].position.x)) -
                                        ((gradients->aUOverZ[0] - gradients->aUOverZ[2]) *
                                         (pVertices[1].position.x - pVertices[2].position.x)));

    gradients->dVOverZdX = OneOverdX * (((gradients->aVOverZ[1] - gradients->aVOverZ[2]) *
                                         (pVertices[0].position.y - pVertices[2].position.y)) -
                                        ((gradients->aVOverZ[0] - gradients->aVOverZ[2]) *
                                         (pVertices[1].position.y - pVertices[2].position.y)));
    gradients->dVOverZdY = OneOverdY * (((gradients->aVOverZ[1] - gradients->aVOverZ[2]) *
                                         (pVertices[0].position.x - pVertices[2].position.x)) -
                                        ((gradients->aVOverZ[0] - gradients->aVOverZ[2]) *
                                         (pVertices[1].position.x - pVertices[2].position.x)));
}

// Initialize edge structure
void InitializeEdge(Edge *edge, const Gradients *gradients,
                    const Vertex *pVertices, int Top, int Bottom) {
    edge->y = (int)ceil(pVertices[Top].position.y);
    int YEnd = (int)ceil(pVertices[Bottom].position.y);
    float YPrestep = edge->y - pVertices[Top].position.y;
    float RealHeight = pVertices[Bottom].position.y - pVertices[Top].position.y;
    float RealWidth = pVertices[Bottom].position.x - pVertices[Top].position.x;

    edge->x = ((RealWidth * YPrestep) / RealHeight) + pVertices[Top].position.x;
    edge->XStep = RealWidth / RealHeight;

    float XPrestep = edge->x - pVertices[Top].position.x;

    edge->OneOverZ = gradients->aOneOverZ[Top] +
                     YPrestep * gradients->dOneOverZdY +
                     XPrestep * gradients->dOneOverZdX;
    edge->OneOverZStep = edge->XStep * gradients->dOneOverZdX + gradients->dOneOverZdY;

    edge->UOverZ = gradients->aUOverZ[Top] +
                   YPrestep * gradients->dUOverZdY +
                   XPrestep * gradients->dUOverZdX;
    edge->UOverZStep = edge->XStep * gradients->dUOverZdX + gradients->dUOverZdY;

    edge->VOverZ = gradients->aVOverZ[Top] +
                   YPrestep * gradients->dVOverZdY +
                   XPrestep * gradients->dVOverZdX;
    edge->VOverZStep = edge->XStep * gradients->dVOverZdX + gradients->dVOverZdY;

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

// DrawScanLine function
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
            // Interpolated Z value
            float z = 1 / OneOverZ;

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
            Pixel tex_color = pTexture->pixels[tex_y * pTexture->size.y + tex_x];

            // Debug output for each pixel
            // printf("Pixel (%d, %d): u=%f, v=%f, Texture Pixel = (%d, %d, %d, %d)\n", XStart + Width, pLeft->y, u, v, tex_color.r, tex_color.g, tex_color.b, tex_color.a);

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
    printf("Initial Vertex Positions:\n");
    for (int i = 0; i < 3; i++) {
        printf("V%d: (%f, %f, %f)\n", i, pVertices[i].position.x, pVertices[i].position.y, pVertices[i].position.z);
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
    printf("Sorted Vertices: Top=%d, Middle=%d, Bottom=%d\n", Top, Middle, Bottom);

    Gradients gradients;
    InitializeGradients(&gradients, pVertices);
    printf("Gradients Initialized: dOneOverZdX=%f, dOneOverZdY=%f, dUOverZdX=%f, dUOverZdY=%f\n",
           gradients.dOneOverZdX, gradients.dOneOverZdY, gradients.dUOverZdX, gradients.dUOverZdY);

    Edge TopToBottom, TopToMiddle, MiddleToBottom;
    InitializeEdge(&TopToBottom, &gradients, pVertices, Top, Bottom);
    InitializeEdge(&TopToMiddle, &gradients, pVertices, Top, Middle);
    InitializeEdge(&MiddleToBottom, &gradients, pVertices, Middle, Bottom);

    // Debug: Print edge initialization data
    printf("Edges Initialized:\n");
    printf("TopToBottom: y=%d, Height=%d, XStep=%f\n", TopToBottom.y, TopToBottom.Height, TopToBottom.XStep);
    printf("TopToMiddle: y=%d, Height=%d, XStep=%f\n", TopToMiddle.y, TopToMiddle.Height, TopToMiddle.XStep);
    printf("MiddleToBottom: y=%d, Height=%d, XStep=%f\n", MiddleToBottom.y, MiddleToBottom.Height, MiddleToBottom.XStep);

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
    printf("MiddleIsLeft: %d\n", MiddleIsLeft);

    int Height = TopToMiddle.Height;
    if (Height > 0) {
        printf("Rendering top half from y=%d for %d scanlines\n", TopToMiddle.y, Height);
    } else {
        printf("Skipping top half, no scanlines to draw.\n");
    }

    while (Height-- > 0) {
        DrawScanLine(pDest, &gradients, pLeft, pRight, pTexture);
        EdgeStep(pLeft);
        EdgeStep(pRight);
    }

    Height = MiddleToBottom.Height;
    if (Height > 0) {
        printf("Rendering bottom half from y=%d for %d scanlines\n", MiddleToBottom.y, Height);
    } else {
        printf("Skipping bottom half, no scanlines to draw.\n");
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
