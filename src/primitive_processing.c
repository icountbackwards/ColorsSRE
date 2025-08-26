#include "primitive_processing.h"

PrimitiveAssemblyOutput runPrimitiveAssembly(VertexShaderOutput vsout){
    PrimitiveAssemblyOutput out;
    out.primitives = malloc(sizeof(Primitive) * (vsout.vertexAmount/3));
    int primitiveAmount = 0;
    for (int i = 0; i < vsout.indicesSize; i += 3){
        Primitive newprimitive;
        newprimitive.v1 = vsout.vertices[vsout.indices[i]];
        newprimitive.v2 = vsout.vertices[vsout.indices[i + 1]];
        newprimitive.v3 = vsout.vertices[vsout.indices[i + 2]];
        out.primitives[i/3] = newprimitive;
        primitiveAmount++;
    }
    out.primitiveAmount = primitiveAmount;
    out.layout = vsout.layout;
    out.layoutSize = vsout.layoutSize;
    return out;
}

bool isInside(Vertex v, Vec3 clipPlane){
    float x = v.data[0];
    float y = v.data[1];
    float z = v.data[2];
    float w = v.data[3];
    float absw = abs_(w);
    if (clipPlane.x == 1)  return x <= absw;
    if (clipPlane.x == -1) return x >= absw * -1;
    if (clipPlane.y == 1)  return y <= absw;
    if (clipPlane.y == -1) return y >= absw * -1;
    if (clipPlane.z == 1)  return z <= absw;
    if (clipPlane.z == -1) return z >= absw * -1;

    assert(0 && "Invalid clip plane direction");
    return false;
}

Vertex getIntersectingPoint(Vertex v1, Vertex v2, Vec3 clipPlane){
    Vec4 v1_;
    Vec4 v2_;
    v1_.x = v1.data[0];
    v1_.y = v1.data[1];
    v1_.z = v1.data[2];
    v1_.w = v1.data[3];
    v2_.x = v2.data[0];
    v2_.y = v2.data[1];
    v2_.z = v2.data[2];
    v2_.w = v2.data[3];

    float f1, f2;
    if (clipPlane.x != 0) {
        float sign = clipPlane.x > 0 ? 1.0f : -1.0f;
        f1 = v1_.x - sign * v1_.w;
        f2 = v2_.x - sign * v2_.w;
    } else if (clipPlane.y != 0) {
        float sign = clipPlane.y > 0 ? 1.0f : -1.0f;
        f1 = v1_.y - sign * v1_.w;
        f2 = v2_.y - sign * v2_.w;
    } else if (clipPlane.z != 0) {
        float sign = clipPlane.z > 0 ? 1.0f : -1.0f;
        f1 = v1_.z - sign * v1_.w;
        f2 = v2_.z - sign * v2_.w;
    }

    float t;
    if (f1 == f2) t = 0.0f; // avoid div-by-zero
    else t = f1 / (f1 - f2);

    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    Vertex intersect;
    intersect.size = v1.size;
    intersect.data = malloc(sizeof(float) * intersect.size);

    for(int i = 0; i < intersect.size; i++){
        intersect.data[i] = (1-t) * v1.data[i] + t * v2.data[i];
    }
    
    

    return intersect;
}

PrimitiveGroup groupVertices(Vertex *vertices, int amount){
    PrimitiveGroup group;
    group.size = amount - 2;
    group.primitives = malloc(sizeof(Primitive) * (group.size));
    for(int i = 0; i < group.size; i++){
        Primitive newprim;
        newprim.v1 = vertices[0];
        newprim.v2 = vertices[i + 1];
        newprim.v3 = vertices[i + 2];
        group.primitives[i] = newprim;
    }
    return group;
}

PrimitiveGroup groupPrimitives(PrimitiveGroup parent, PrimitiveGroup child){
    PrimitiveGroup out;
    out.size = parent.size + child.size;
    out.primitives = malloc(sizeof(Primitive) * out.size);
    for(int i = 0; i < parent.size; i++){
        out.primitives[i] = parent.primitives[i]; 
    }
    for(int i = 0; i < child.size; i++){
        out.primitives[parent.size + i] = child.primitives[i];
    }
    return out;
}

void bindPrimitiveGroup(Primitive **primitiveArray, PrimitiveGroup primitiveGroup){
    *primitiveArray = malloc(sizeof(Primitive) * primitiveGroup.size);
    for(int i = 0; i < primitiveGroup.size; i++){
        (*primitiveArray)[i] = primitiveGroup.primitives[i];
    }
}

ClippingOutput runPrimitiveClipping(PrimitiveAssemblyOutput primitives){
    Vertex nullvertex = {0};
    Vec3 right;
    right.x = -1;
    right.y = 0;
    right.z = 0;
    Vec3 left;
    left.x = 1;
    left.y = 0;
    left.z = 0;
    Vec3 top;
    top.x = 0;
    top.y = -1;
    top.z = 0;
    Vec3 bot;
    bot.x = 0;
    bot.y = 1;
    bot.z = 0;
    Vec3 front;
    front.x = 0;
    front.y = 0;
    front.z = -1;
    Vec3 back;
    back.x = 0;
    back.y = 0;
    back.z = 1;

    Vec3 *clipPlanes = malloc(sizeof(Vec3) * 6);
    clipPlanes[0] = right;
    clipPlanes[1] = left;
    clipPlanes[2] = top;
    clipPlanes[3] = bot;
    clipPlanes[4] = front;
    clipPlanes[5] = back;
    ClippingOutput out;

    PrimitiveGroup grouped;
    grouped.size = 0;
    
    Vertex *outputList;
    Vertex *inputList;
    Primitive subjectPolygon;
    int outputlistAmount;
    int newlistamount;

    Vertex currentPoint;
    Vertex prevPoint;
    Vertex intersectingPoint;

    outputList = malloc(sizeof(Vertex) * 7);
    inputList = malloc(sizeof(Vertex) * 7);

    out.primitives = malloc(sizeof(Primitive) * primitives.primitiveAmount * 5);
    int primIndex = 0;

    int count = 0;

    for(int p = 0; p < primitives.primitiveAmount; p++){
        for(int i = 0; i < 6; i++){
            outputList[i] = nullvertex;
        }
        subjectPolygon = primitives.primitives[p];

        outputList[0] = subjectPolygon.v1;
        outputList[1] = subjectPolygon.v2;
        outputList[2] = subjectPolygon.v3;

        outputlistAmount = 3;

        for(int c = 0; c < 6; c++){
            for(int i = 0; i < outputlistAmount; i++){
                inputList[i] = nullvertex;
            }
            for(int a = 0; a < outputlistAmount; a++){
                inputList[a] = outputList[a];
            }
            for(int i = 0; i < outputlistAmount; i++){
                outputList[i] = nullvertex;
            }

            newlistamount = 0; 

            for(int i = 0; i < outputlistAmount; i++){
                currentPoint = inputList[i];
                prevPoint = inputList[(i + outputlistAmount - 1) % outputlistAmount];
                intersectingPoint = getIntersectingPoint(currentPoint, prevPoint, clipPlanes[c]);

                bool cur = isInside(currentPoint, clipPlanes[c]);
                bool prev = isInside(prevPoint, clipPlanes[c]);

                if(cur){
                    if(!prev){
                        outputList[newlistamount] = intersectingPoint;
                        newlistamount++;
                        count++;
                    }
                    outputList[newlistamount] = currentPoint;
                    newlistamount++;
                    count++;
                    
                }
                else if(prev){
                    if(!cur){
                        outputList[newlistamount] = intersectingPoint;
                        newlistamount++;
                        count++;
                    }
                }
                
            }
            outputlistAmount = newlistamount;

            
        }

        grouped = groupVertices(outputList, outputlistAmount);
        for(int i = 0; i < grouped.size; i++){
            out.primitives[primIndex] = grouped.primitives[i];
            primIndex++;
        }
    }
    

    
    out.primitiveAmount = primIndex;
    
    
    out.layout = primitives.layout; 
    out.layoutSize = primitives.layoutSize;

    free(inputList);
    free(outputList);
    free(clipPlanes);


    free(grouped.primitives);
    return out;
}

PrimitiveOutput runPerspectiveDivide(ClippingOutput primitives){
    
    PrimitiveOutput out;
    out.layoutSize = primitives.layoutSize;
    out.primitiveAmount = primitives.primitiveAmount;
    out.primitives = malloc(sizeof(Primitive) * primitives.primitiveAmount);
    out.layout = primitives.layout;

    for(int i = 0; i < primitives.primitiveAmount; i++){
        int vertexSize = primitives.primitives[i].v1.size;
        out.primitives[i].v1.size = vertexSize;
        out.primitives[i].v2.size = vertexSize;
        out.primitives[i].v3.size = vertexSize;
        out.primitives[i].v1.data = malloc(sizeof(float) * (vertexSize));
        out.primitives[i].v2.data = malloc(sizeof(float) * (vertexSize));
        out.primitives[i].v3.data = malloc(sizeof(float) * (vertexSize));
        for (int j = 0; j < 4; j++){
            if(j == 3){
                out.primitives[i].v1.data[j] = primitives.primitives[i].v1.data[j];
                out.primitives[i].v2.data[j] = primitives.primitives[i].v2.data[j];
                out.primitives[i].v3.data[j] = primitives.primitives[i].v3.data[j]; 
            }else{
                out.primitives[i].v1.data[j] = primitives.primitives[i].v1.data[j] / primitives.primitives[i].v1.data[3];
                out.primitives[i].v2.data[j] = primitives.primitives[i].v2.data[j] / primitives.primitives[i].v2.data[3];
                out.primitives[i].v3.data[j] = primitives.primitives[i].v3.data[j] / primitives.primitives[i].v3.data[3]; 
            }
            
        }
        if(out.layoutSize > 1){
            for(int k = 4; k < vertexSize; k++){
                out.primitives[i].v1.data[k] = primitives.primitives[i].v1.data[k];
                out.primitives[i].v2.data[k] = primitives.primitives[i].v2.data[k];
                out.primitives[i].v3.data[k] = primitives.primitives[i].v3.data[k];
            }
        }
        
    }
    
    return out;
}

void runScreenMapping(PrimitiveOutput *primitives, int width, int height){
    for(int i = 0; i < primitives->primitiveAmount; i++){
        primitives->primitives[i].v1.data[0] = (width / 2) * primitives->primitives[i].v1.data[0] + (width / 2);
        primitives->primitives[i].v1.data[1] = (height / 2) * primitives->primitives[i].v1.data[1] + (height / 2);
        primitives->primitives[i].v1.data[2] = 0.5 * primitives->primitives[i].v1.data[2] + 0.5;

        primitives->primitives[i].v2.data[0] = (width / 2) * primitives->primitives[i].v2.data[0] + (width / 2);
        primitives->primitives[i].v2.data[1] = (height / 2) * primitives->primitives[i].v2.data[1] + (height / 2);
        primitives->primitives[i].v2.data[2] = 0.5 * primitives->primitives[i].v2.data[2] + 0.5;

        primitives->primitives[i].v3.data[0] = (width / 2) * primitives->primitives[i].v3.data[0] + (width / 2);
        primitives->primitives[i].v3.data[1] = (height / 2) * primitives->primitives[i].v3.data[1] + (height / 2);
        primitives->primitives[i].v3.data[2] = 0.5 * primitives->primitives[i].v3.data[2] + 0.5;
    }
}