#pragma once

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