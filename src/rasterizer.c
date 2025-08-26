#include "rasterizer.h"

AABB getBoundingBox(Primitive *primitive){
    AABB out;
    out.Max.x = getMax(primitive->v1.data[0], getMax(primitive->v2.data[0], primitive->v3.data[0]));
    out.Max.y = getMax(primitive->v1.data[1], getMax(primitive->v2.data[1], primitive->v3.data[1]));
    out.Min.x = getMin(primitive->v1.data[0], getMin(primitive->v2.data[0], primitive->v3.data[0]));
    out.Min.y = getMin(primitive->v1.data[1], getMin(primitive->v2.data[1], primitive->v3.data[1]));
    return out;
}

Fragment *groupFragments(Fragment **parent, Fragment **child, int *parentSize, int childSize){
    //printf("allocated: %d\n", ((*parentSize) + childSize));
    Fragment *out = malloc(sizeof(Fragment) * ((*parentSize) + childSize));
    for(int i = 0; i < (*parentSize); i++){
        out[i] = (*parent)[i];
    }
    for(int i = (*parentSize); i < (*parentSize) + childSize; i++){
        out[i] = (*child)[i];
    }
    (*parentSize) += childSize;
    return out;
}

Primitive rewindPrimitive(Primitive primitive){
    //COUNTER CLOCKWISE
    Primitive out = primitive;
    if((out.v2.data[0] - out.v1.data[0])*(out.v3.data[1] - out.v1.data[1])-
        (out.v3.data[0] - out.v1.data[0])*(out.v2.data[1]-out.v1.data[1]) < 0){
            Vertex temp = out.v2;
            out.v2 = out.v3;
            out.v3 = temp;
        }
    return out;
}

TriangleTraversalOutput runRasterizer(PrimitiveOutput *primitives, int pipelineVariation, int width, int height) {
    TriangleTraversalOutput out;
    out.fragments = malloc(sizeof(Fragment) * height * width);
    out.fragmentSize = 0;

    for (int i = 0; i < primitives->primitiveAmount; i++) {
        primitives->primitives[i] = rewindPrimitive(primitives->primitives[i]);

        AABB aabb = getBoundingBox(&primitives->primitives[i]);
        aabb.Min.x = fmax(0, aabb.Min.x);
        aabb.Min.y = fmax(0, aabb.Min.y);
        aabb.Max.x = fmin(width - 1, aabb.Max.x);
        aabb.Max.y = fmin(height - 1, aabb.Max.y);

        Vertex v0 = primitives->primitives[i].v1;
        Vertex v1 = primitives->primitives[i].v2;
        Vertex v2 = primitives->primitives[i].v3;
        

        if (isinf(v0.data[2]) || isinf(v1.data[2]) || isinf(v2.data[2]) ||
            isnan(v0.data[2]) || isnan(v1.data[2]) || isnan(v2.data[2])) {
            //printf("⚠️ Invalid depth (inf or NaN) in triangle %d. Skipped.\n", i);
            continue;
        }

        float delta_w0_col = (v1.data[1] - v2.data[1]);
        float delta_w1_col = (v2.data[1] - v0.data[1]);
        float delta_w2_col = (v0.data[1] - v1.data[1]);

        float delta_w0_row = (v2.data[0] - v1.data[0]);
        float delta_w1_row = (v0.data[0] - v2.data[0]);
        float delta_w2_row = (v1.data[0] - v0.data[0]);

        float area = edgeCross2(v0, v1, v2);

        Vec2 p0 = {aabb.Min.x, aabb.Min.y};
        float w0_row = edgeCross1(v1, v2, p0);
        float w1_row = edgeCross1(v2, v0, p0);
        float w2_row = edgeCross1(v0, v1, p0);

        float v0z = 1.0f / v0.data[2];
        float v1z = 1.0f / v1.data[2];
        float v2z = 1.0f / v2.data[2];

        float invW0 = 1.0f / v0.data[3];
        float invW1 = 1.0f / v1.data[3];
        float invW2 = 1.0f / v2.data[3];
        float r0, r1, r2, g0, g1, g2, b0, b1, b2;

        float *frag0 = malloc(sizeof(float) * primitives->layout[2]);
        float *frag1 = malloc(sizeof(float) * primitives->layout[2]);
        float *frag2 = malloc(sizeof(float) * primitives->layout[2]);

        //R G B = 4 5 6
        //TX TY = 7 8
        //NX NY NZ = 9 10 11
        //FX FY FZ = 12 13 14
        if(pipelineVariation == PIPELINE_VARIATION_MESH){
            r0 = v0.data[4] * v0z;
            g0 = v0.data[5] * v0z;
            b0 = v0.data[6] * v0z;

            r1 = v1.data[4] * v1z;
            g1 = v1.data[5] * v1z;
            b1 = v1.data[6] * v1z;

            r2 = v2.data[4] * v2z;
            g2 = v2.data[5] * v2z;
            b2 = v2.data[6] * v2z;

            for(int f = 7; f < primitives->layout[2]; f++){
                frag0[f] = v0.data[f] * invW0;
                frag1[f] = v1.data[f] * invW1;
                frag2[f] = v2.data[f] * invW2;
            }
        }
        

        for (int y = (int)aabb.Min.y; y <= (int)aabb.Max.y; y++) {
            float w0 = w0_row;
            float w1 = w1_row;
            float w2 = w2_row;

            for (int x = (int)aabb.Min.x; x <= (int)aabb.Max.x; x++) {
                //TOP LEFT RULE IMP

                Vec2 p0 = {v0.data[0], v0.data[1]};
                Vec2 p1 = {v1.data[0], v1.data[1]};
                Vec2 p2 = {v2.data[0], v2.data[1]};
                bool w0_zero = (w0 == 0) && isTopLeft(p1, p2);
                bool w1_zero = (w1 == 0) && isTopLeft(p2, p0);
                bool w2_zero = (w2 == 0) && isTopLeft(p0, p1);

                bool isInside =
                    (w0 > 0 || w0_zero) &&
                    (w1 > 0 || w1_zero) &&
                    (w2 > 0 || w2_zero);

                if (isInside) {
                    if (out.fragmentSize >= width * height) {
                        //FRAGMENT BUFFER OVERFLOW
                        break;
                    }
                    if(x < 0 || x > width || y < 0 || y > height){
                        continue;
                    }

                    float alpha = w0  / area;
                    float beta = w1 / area;
                    float gamma = w2 / area;

                    // Interpolate 1/z
                    float invZ = alpha * v0z + beta * v1z + gamma * v2z;
                    float z = 1.0f / invZ;

                    float r, g, b, invW;
                    float *frag = malloc(sizeof(float) * primitives->layout[2]);
                    if(pipelineVariation == PIPELINE_VARIATION_MESH){
                        // Perspective-correct interpolation for colors
                        r = (alpha * r0 + beta * r1 + gamma * r2) * z;
                        g = (alpha * g0 + beta * g1 + gamma * g2) * z;
                        b = (alpha * b0 + beta * b1 + gamma * b2) * z;

                        // At each pixel (after computing barycentrics)
                        invW = alpha * invW0 + beta * invW1 + gamma * invW2;

                        for(int f = 7; f < primitives->layout[2]; f++){
                            frag[f] = (alpha * frag0[f] + beta * frag1[f] + gamma * frag2[f]) / invW;
                        }
                    }
                    

                    Fragment newfrag;
                    newfrag.position.x = x;
                    newfrag.position.y = y;
                    newfrag.zval = z;
                    if(pipelineVariation == PIPELINE_VARIATION_MESH){
                        newfrag.color.x = r;
                        newfrag.color.y = g;
                        newfrag.color.z = b;
                        newfrag.color.w = 1.0;
                        newfrag.data = frag;
                    }

                    out.fragments[out.fragmentSize] = newfrag;
                    out.fragmentSize++;
                }

                w0 += delta_w0_col;
                w1 += delta_w1_col;
                w2 += delta_w2_col;
            }

            w0_row += delta_w0_row;
            w1_row += delta_w1_row;
            w2_row += delta_w2_row;
        }
        free(frag0);
        free(frag1);
        free(frag2);
    }

    return out;
}

bool isTopLeft(Vec2 a, Vec2 b) {
    return (a.y == b.y && a.x > b.x) || (a.y > b.y);
}

int edgeCross1(Vertex a, Vertex b, Vec2 p){
    return ((b.data[0] - a.data[0])*(p.y-a.data[1]) - (b.data[1]-a.data[1])*(p.x-a.data[0]));
}

int edgeCross2(Vertex a, Vertex b, Vertex p){
    return ((b.data[0] - a.data[0])*(p.data[1]-a.data[1]) - (b.data[1]-a.data[1])*(p.data[0]-a.data[0]));
}
