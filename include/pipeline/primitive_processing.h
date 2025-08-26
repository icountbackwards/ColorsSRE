#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#include "pipeline_structs.h"

PrimitiveAssemblyOutput runPrimitiveAssembly(VertexShaderOutput vertices);
ClippingOutput runPrimitiveClipping(PrimitiveAssemblyOutput primitives);
PrimitiveOutput runPerspectiveDivide(ClippingOutput primitives);
void runScreenMapping(PrimitiveOutput *primitives, int width, int height);

Vertex getIntersectingPoint(Vertex v1, Vertex v2, Vec3 clipPlane);
bool isInside(Vertex v, Vec3 clipPlane);
PrimitiveGroup groupPrimitives(PrimitiveGroup parent, PrimitiveGroup child);
PrimitiveGroup groupVertices(Vertex *vertices, int amount);
void bindPrimitiveGroup(Primitive **primitiveArray, PrimitiveGroup primitiveGroup);