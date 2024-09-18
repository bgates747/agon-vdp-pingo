#pragma once

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "pixel.h"
#include "texture.h"
#include "mesh.h"

// code shamelessly stolen from https://chrishecker.com/images/4/41/Gdmtex1.pdf
// src/orig/DIVFLFL.CPP

// Define the Gradients struct
typedef struct {
    float aOneOverZ[3]; // 1/z for each vertex
    float aUOverZ[3];   // u/z for each vertex
    float aVOverZ[3];   // v/z for each vertex
    float dOneOverZdX, dOneOverZdY; // d(1/z)/dX, d(1/z)/dY
    float dUOverZdX, dUOverZdY; // d(u/z)/dX, d(u/z)/dY
    float dVOverZdX, dVOverZdY; // d(v/z)/dX, d(v/z)/dY
} Gradients;

// Define the Edge struct
typedef struct {
    float x, XStep;   // fractional x and dX/dY
    int y, Height;    // current y and vert count
    float OneOverZ, OneOverZStep; // 1/z and step
    float UOverZ, UOverZStep;     // u/z and step
    float VOverZ, VOverZStep;     // v/z and step
} Edge;

// Function prototypes
void InitializeGradients(Gradients *gradients, const Vertex *pVertices);
void InitializeEdge(Edge *edge, const Gradients *gradients, const Vertex *pVertices, int Top, int Bottom);
void EdgeStep(Edge *edge);
void DrawScanLine(Texture *pDest, const Gradients *gradients, const Edge *pLeft, const Edge *pRight, const Texture *pTexture);
void TextureMapTriangle(Texture *pDest, const Vertex *pVertices, Texture *pTexture);
