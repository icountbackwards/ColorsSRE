#pragma once

#include <stdint.h>

#include "pipeline_structs.h"
#include "./objects.h"

void runMerger(FragmentShaderOutput *in, Texture* texture, int width, int height, float* depthBuffer, uint32_t* frameBuffer);