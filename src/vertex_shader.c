#include "vertex_shader.h"

#include <stdlib.h>

VertexShaderOutput runVertexShader(VertexBuffer *vbo, UniformBuffer *ubo, int pipelineVariation){
    int vertexAmount, outDataLayoutSize1, outDataLayoutSize2, outDataSize1, outDataSize2;
    Vertex *vertices1;
    Vertex *vertices2;
    VertexShaderOutput out;
    switch(pipelineVariation){
        case PIPELINE_VARIATION_MESH:
            //GET INDEX OF EACH VERTEX ATTRIBUTE LOCATION (FIRST ELEMENT)
            vertexAmount = getVertexAmount(vbo);
            //printf("Vertex Shader Query: vertexAmount = %d\n\n", vertexAmount);
            int outDataLayout1[20] = {0, 4, 15, 0, 
                                        1, 3, 15, 4, 
                                        2, 2, 15, 7, 
                                        3, 3, 15, 9, 
                                        4, 3, 15, 12}; //VERTEX AMOUNT, LOCATION, SIZE, STRIDE, OFFSET
            outDataLayoutSize1 = 5;
            outDataSize1 = 0;
            for(int i = 0; i < outDataLayoutSize1; i++){
                outDataSize1 += outDataLayout1[1 + i * 4];
            }
            vertices1 = malloc(sizeof(Vertex) * vertexAmount);
            for(int v = 0; v < vertexAmount; v++){
                //INPUT
                Vec3 loc0in = getVertexData3(&vbo->data, v, vbo->layout, 0); // POS
                Vec3 loc1in = getVertexData3(&vbo->data, v, vbo->layout, 1);
                Vec2 loc2in = getVertexData2(&vbo->data, v, vbo->layout, 2);
                Vec3 loc3in = getVertexData3(&vbo->data, v, vbo->layout, 3);

                //OUTPUT
                Vec4 loc0out; // POS
                Vec3 loc1out; // COL
                Vec2 loc2out; // TEXCOORD
                Vec3 loc3out = normalTransform(getMat3(ubo->model), loc3in);
                Vec3 loc4out; //FRAGPOS

                //CODE
                loc0out.x = loc0in.x;
                loc0out.y = loc0in.y;
                loc0out.z = loc0in.z;
                loc0out.w = 1.0;

                //double angle = fmod(getTime(), TWO_PI);
                //if (angle < 0)
                //    angle += TWO_PI_SHORT;
                //loc0out = rotate(loc0out, 0, angle);

                //Vec3 factor = {getTime()* 0.1, 0.0, 0.0};
                //loc0out = translate(loc0out, factor);
                

                loc0out = mat4vec4multiply(ubo->model, loc0out);

                loc4out.x = loc0out.x;
                loc4out.y = loc0out.y;
                loc4out.z = loc0out.z;

                loc0out = mat4vec4multiply(ubo->view, loc0out);
                
                loc0out = mat4vec4multiply(ubo->projection, loc0out);

                loc1out.x = loc1in.x;
                loc1out.y = loc1in.y;
                loc1out.z = loc1in.z;

                loc2out.x = loc2in.x;
                loc2out.y = loc2in.y;

                //MAP OUTPUT
                vertices1[v].data = malloc(sizeof(float) * outDataSize1);
                vertices1[v].size = outDataSize1;
                mapVertexShaderDataOutput4(&vertices1[v], loc0out, 0, outDataLayout1);
                mapVertexShaderDataOutput3(&vertices1[v], loc1out, 1, outDataLayout1);
                mapVertexShaderDataOutput2(&vertices1[v], loc2out, 2, outDataLayout1);
                mapVertexShaderDataOutput3(&vertices1[v], loc3out, 3, outDataLayout1);
                mapVertexShaderDataOutput3(&vertices1[v], loc4out, 4, outDataLayout1);
            }
            out.indices = vbo->indices;
            out.layout = malloc(sizeof(int) * outDataLayoutSize1 * 4);
            for(int i = 0; i < outDataLayoutSize1 * 4; i++){
                out.layout[i] = outDataLayout1[i];
            }
            out.vertices = vertices1;
            out.vertices = malloc(sizeof(Vertex) * vertexAmount);
            for(int i = 0; i < vertexAmount; i++){
                out.vertices[i] = vertices1[i];
            }
            out.vertexAmount = vertexAmount;
            out.indicesSize = vbo->indicesSize;
            out.layoutSize = outDataLayoutSize1;

            free(vertices1);

            return out;
        case PIPELINE_VARIATION_LIGHT_SOURCE:
            //GET INDEX OF EACH VERTEX ATTRIBUTE LOCATION (FIRST ELEMENT)
            vertexAmount = getVertexAmount(vbo);
            //printf("Vertex Shader Query: vertexAmount = %d\n\n", vertexAmount);
            int outDataLayout2[4] = {0, 4, 4, 0}; //VERTEX AMOUNT, LOCATION, SIZE, STRIDE, OFFSET
            outDataLayoutSize2 = vbo->layoutSize;
            outDataSize2 = 0;
            for(int i = 0; i < outDataLayoutSize2; i++){
                outDataSize2 += outDataLayout2[1 + i * 4];
            }
            vertices2 = malloc(sizeof(Vertex) * vertexAmount);
            for(int v = 0; v < vertexAmount; v++){
                //INPUT
                Vec3 loc0in = getVertexData3(&vbo->data, v, vbo->layout, 0); // POS

                //OUTPUT
                Vec4 loc0out; // POS

                //CODE
                loc0out.x = loc0in.x;
                loc0out.y = loc0in.y;
                loc0out.z = loc0in.z;
                loc0out.w = 1.0;


                loc0out = mat4vec4multiply(ubo->model, loc0out);
                loc0out = mat4vec4multiply(ubo->view, loc0out);
                
                loc0out = mat4vec4multiply(ubo->projection, loc0out);


                //MAP OUTPUT
                vertices2[v].data = malloc(sizeof(float) * outDataSize2);
                vertices2[v].size = outDataSize2;
                mapVertexShaderDataOutput4(&vertices2[v], loc0out, 0, outDataLayout2);
            }

            out.indices = vbo->indices;
            out.layout = malloc(sizeof(int) * outDataLayoutSize2 * 4);
            for(int i = 0; i < outDataLayoutSize2 * 4; i++){
                out.layout[i] = outDataLayout2[i];
            }
            out.vertices = vertices2;
            out.vertices = malloc(sizeof(Vertex) * vertexAmount);
            for(int i = 0; i < vertexAmount; i++){
                out.vertices[i] = vertices2[i];
            }
            out.vertexAmount = vertexAmount;
            out.indicesSize = vbo->indicesSize;
            out.layoutSize = outDataLayoutSize2;

            free(vertices2);


            return out;
    }
}

Vec2 getVertexData2(float **data, int index, int* layout, int loc){
    int stride = layout[loc * 4 + 2];
    int offset = layout[loc * 4 + 3];
    Vec2 vec = {(*data)[stride * index + offset], (*data)[stride * index + offset + 1]};
    return vec;
}

Vec3 getVertexData3(float **data, int index, int* layout, int loc){
    int stride = layout[loc * 4 + 2];
    int offset = layout[loc * 4 + 3];
    Vec3 vec = {(*data)[stride * index + offset], (*data)[stride * index + offset + 1], (*data)[stride * index + offset + 2]};
    return vec;
}
Vec4 getVertexData4(float **data, int index, int* layout, int loc){
    int stride = layout[loc * 4 + 2];
    int offset = layout[loc * 4 + 3];
    Vec4 vec = {(*data)[stride * index + offset], (*data)[stride * index + offset + 1], (*data)[stride * index + offset + 2], (*data)[stride * index + offset + 3]};
    return vec;
}

int getVertexAmount(VertexBuffer *vbo){
    return vbo->dataSize / vbo->layout[2];
}

void mapVertexShaderDataOutput2(Vertex *vertex, Vec2 output, int loc, int* layout){
    int offset = layout[loc * 4 + 3];
    vertex->data[offset] = output.x;
    vertex->data[offset + 1] = output.y;
}

void mapVertexShaderDataOutput3(Vertex *vertex, Vec3 output, int loc, int* layout){
    int offset = layout[loc * 4 + 3];
    vertex->data[offset] = output.x;
    vertex->data[offset + 1] = output.y;
    vertex->data[offset + 2] = output.z;
}

void mapVertexShaderDataOutput4(Vertex *vertex, Vec4 output, int loc, int* layout){
    int offset = layout[loc * 4 + 3];
    vertex->data[offset] = output.x;
    vertex->data[offset + 1] = output.y;
    vertex->data[offset + 2] = output.z;
    vertex->data[offset + 3] = output.w;
}
