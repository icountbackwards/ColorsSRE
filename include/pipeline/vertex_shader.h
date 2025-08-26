#pragma once

#include "pipeline_structs.h"
#include "./instance.h"

VertexShaderOutput runVertexShader(VertexBuffer *vbo, UniformBuffer *ubo, int pipelineVariation);

Vec2 getVertexData2(float **data, int index, int* layout, int loc);
Vec3 getVertexData3(float **data, int index, int* layout, int loc);
Vec4 getVertexData4(float **data, int index, int* layout, int loc);

int getVertexAmount(VertexBuffer *vbo);

void mapVertexShaderDataOutput2(Vertex *vertex, Vec2 output, int loc, int* layout);
void mapVertexShaderDataOutput3(Vertex *vertex, Vec3 output, int loc, int* layout);
void mapVertexShaderDataOutput4(Vertex *vertex, Vec4 output, int loc, int* layout);