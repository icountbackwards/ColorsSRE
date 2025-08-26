#include "pipeline.h"

int edgeCross1(Vertex a, Vertex b, Vec2 p){
    return ((b.data[0] - a.data[0])*(p.y-a.data[1]) - (b.data[1]-a.data[1])*(p.x-a.data[0]));
}

int edgeCross2(Vertex a, Vertex b, Vertex p){
    return ((b.data[0] - a.data[0])*(p.data[1]-a.data[1]) - (b.data[1]-a.data[1])*(p.data[0]-a.data[0]));
}
