#pragma once

#include <stdlib.h>

#include <string.h>
#include "stb_image.h"

#include "math_utils.h"

typedef struct {
    Vec3* pixels;
    int width;
    int height;
} Texture;

typedef struct {
    float *vertices; // interleaved
    int vertexCount; // number of floats
    int *indices;
    int indexCount;
} MeshData;

MeshData loadOBJ(const char *filename);
Texture createTexture(char* filename);
Vec3* load_png_as_vec3(const char* filename, int* out_width, int* out_height);
Vec3 getTexturePixel(Texture* pTex, float tx, float ty);