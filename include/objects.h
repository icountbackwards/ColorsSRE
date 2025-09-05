#pragma once

#include <stdlib.h>

#include <string.h>
#include "stb_image.h"

#include "math_utils.h"

typedef struct {
    float *data;
    int dataSize;
    int *layout;
    int layoutSize;
    int *indices;
    int indicesSize;
} VertexBuffer;

typedef struct {
    Mat4 model;
    Mat4 view;
    Mat4 projection;
    Vec3 objectColor;
    Vec3 lightColor;
    Vec3 lightPos;
    Vec3 viewPos;
} UniformBuffer;

typedef struct {
    Vec3* pixels;
    int width;
    int height;
} Texture;

VertexBuffer loadObjFile(const char* filename);
Texture createTexture(char* filename);
Vec3* loadPngFile(const char* filename, int* out_width, int* out_height);
Vec3 getTexturePixel(Texture* pTex, float tx, float ty);
VertexBuffer generateVertexBuffer(float* data, int* indices, int* layout, int datasize, int indicesSize, int layoutSize);
