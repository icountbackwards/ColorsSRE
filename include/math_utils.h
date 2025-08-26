#pragma once

#include <math.h>
#include <stdio.h>

#include "pipeline/pipeline_structs.h"

#define TWO_PI 6.28318530718
#define TWO_PI_SHORT 6.28
#define PI 3.14159265358979323846f

typedef struct
{
    float x;
    float y;
}Vec2;

typedef struct
{
    float x;
    float y;
    float z;
}Vec3;

typedef struct
{
    float x;
    float y;
    float z;
    float w;
}Vec4;

typedef struct {
    Vec4 r0;
    Vec4 r1;
    Vec4 r2;
} Mat3;

typedef struct {
    Vec4 r0;
    Vec4 r1;
    Vec4 r2;
    Vec4 r3;
} Mat4;

float abs_(float n);
Vec3 minus3(Vec3 front, Vec3 back);
Vec3 plus3(Vec3 front, Vec3 back);
Vec3 scalarMultiply3(float scalar, Vec3 vector);
Vec3 multiply3(Vec3 v1, Vec3 v2);

float dot3(Vec3 v1, Vec3 v2);
Vec3 cross(Vec3 a, Vec3 b);
int edgeCross1(Vertex a, Vertex b, Vec2 p);
int edgeCross2(Vertex a, Vertex b, Vertex p);
Vec3 normalize(Vec3 v);

float getMax(float n1, float n2);
float getMin(float n1, float n2);

Mat4 mat4mat4Multiply(Mat4 left, Mat4 right);
Vec4 mat4vec4multiply(Mat4 left, Vec4 right);

void rotate4(Mat4 *mat, float angle, int axis);
void translate4(Mat4 *mat, Vec3 factor);
void scale4(Mat4 *mat, Vec3 scale);
Vec4 scale(Vec4 vector, Vec3 factor);
Vec4 translate(Vec4 vector, Vec3 factor);
Vec4 rotate(Vec4 vector, int axis, double angle);

Mat4 perspective(float fovy, float aspect, float znear, float zfar);
Mat4 lookAt(Vec3 cameraPos, Vec3 invDirection, Vec3 cameraUp);

void printVec2(Vec2 v);
void printVec3(Vec3 v);
void printVec4(Vec4 v);


