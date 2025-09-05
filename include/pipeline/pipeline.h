#pragma once

#include "pipeline_structs.h"
#include "vertex_shader.h"
#include "primitive_processing.h"
#include "rasterizer.h"
#include "fragment_shader.h"
#include "merger.h"

#include "./instance.h"

void draw(VertexBuffer *vbo, UniformBuffer *ubo, Texture* pTextureResource, int pipelineVariation, Instance* instance);

void drawMeshObject(VertexBuffer *vbo, UniformBuffer *ubo, Texture* pTextureResource, Instance* instance);
void drawLightSource(VertexBuffer *vbo, UniformBuffer *ubo, Texture* pTextureResource, Instance* instance);
