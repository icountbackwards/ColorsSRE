#pragma once

#include "./math_utils.h"

#define PIPELINE_VARIATION_MESH 1
#define PIPELINE_VARIATION_LIGHT_SOURCE 2

typedef struct {
    float *data;
    int size;
} Vertex;

typedef struct {
    Vertex *vertices;
    int *indices;
    int *layout;
    int vertexAmount;
    int indicesSize;
    int layoutSize;
} VertexShaderOutput;

typedef struct {
    Vec2 position;
    float zval;
    Vec4 color;
    float *data;
    Vec2 texCoord;
    Vec3 normal;
    Vec3 fragPos;
} Fragment;

typedef struct {
    Vertex v1;
    Vertex v2;
    Vertex v3;
    Fragment *fragments;
    int fragmentAmount;
} Primitive;

typedef struct {
    Primitive *primitives;
    int *layout;
    int primitiveAmount;
    int layoutSize;
} PrimitiveAssemblyOutput;

typedef struct {
    Primitive *primitives;
    int *layout;
    int primitiveAmount;
    int layoutSize;
} PrimitiveOutput;

typedef struct{
    Primitive *primitives;
    int size;
} PrimitiveGroup;

typedef struct {
    Primitive *primitives;
    int *layout;
    int primitiveAmount;
    int layoutSize;
} ClippingOutput;

typedef struct {
    Vec2 Max;
    Vec2 Min;
} AABB;

typedef struct {
    Fragment *fragments;
    int fragmentSize;
} TriangleTraversalOutput;

typedef struct {
    Fragment* fragments;
    int fragmentSize;
} FragmentShaderOutput;