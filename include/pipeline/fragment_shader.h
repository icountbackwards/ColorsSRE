#pragma once

#include <stdlib.h>

#include "pipeline_structs.h"
#include "./instance.h"
#include "./objects.h"

FragmentShaderOutput runFragmentShader(TriangleTraversalOutput *in, Texture* texture, UniformBuffer *ubo, int pipelineVariation);