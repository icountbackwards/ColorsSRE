#pragma once

#include <stdbool.h>
#include <stdlib.h>

#include "pipeline_structs.h"

TriangleTraversalOutput runRasterizer(PrimitiveOutput *primitives, int pipelineVariation, int width, int height);

Fragment *groupFragments(Fragment **parent, Fragment **child, int *parentSize, int childSize);
Primitive rewindPrimitive(Primitive primitive);
bool isTopLeft(Vec2 a, Vec2 b);

int edgeCross1(Vertex a, Vertex b, Vec2 p);
int edgeCross2(Vertex a, Vertex b, Vertex p);