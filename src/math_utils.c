#include "math_utils.h"

float abs_(float n){
    if(n >= 0){
        return n;
    }else{
         return n * -1;
    }
}

Vec3 minus3(Vec3 front, Vec3 back){
    Vec3 out;
    out.x = front.x - back.x;
    out.y = front.y - back.y;
    out.z = front.z - back.z;
    return out;
}

Vec3 plus3(Vec3 front, Vec3 back){
    Vec3 result;
    result.x = front.x + back.x;
    result.y = front.y + back.y;
    result.z = front.z + back.z;
    return result;
}

Vec3 scalarMultiply3(float scalar, Vec3 vector){
    Vec3 out;
    out.x = scalar * vector.x;
    out.y = scalar * vector.y;
    out.z = scalar * vector.z;
    return out;
}

Vec3 multiply3(Vec3 v1, Vec3 v2){
    return (Vec3){v1.x * v2.x, v1.y * v2.y, v1.z * v2.z};
}

float dot3(Vec3 v1, Vec3 v2){
    return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
}

Vec3 cross(Vec3 a, Vec3 b) {
    Vec3 result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;
    return result;
}

int edgeCross1(Vertex a, Vertex b, Vec2 p){
    return ((b.data[0] - a.data[0])*(p.y-a.data[1]) - (b.data[1]-a.data[1])*(p.x-a.data[0]));
}
int edgeCross2(Vertex a, Vertex b, Vertex p){
    return ((b.data[0] - a.data[0])*(p.data[1]-a.data[1]) - (b.data[1]-a.data[1])*(p.data[0]-a.data[0]));
}

Vec3 normalize(Vec3 v){
    float length = sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
    return (Vec3){v.x/length, v.y/length, v.z/length};
}

float getMax(float n1, float n2){
    if(n1 >= n2){
        return n1;
    }
    return n2;
}

float getMin(float n1, float n2){
    if(n1 <= n2){
        return n1;
    }
    return n2;
}

Mat4 mat4mat4Multiply(Mat4 left, Mat4 right){
    Mat4 out;
    out.r0 = (Vec4){
        left.r0.x * right.r0.x + left.r0.y * right.r1.x + left.r0.z * right.r2.x + left.r0.w * right.r3.x,
        left.r0.x * right.r0.y + left.r0.y * right.r1.y + left.r0.z * right.r2.y + left.r0.w * right.r3.y,
        left.r0.x * right.r0.z + left.r0.y * right.r1.z + left.r0.z * right.r2.z + left.r0.w * right.r3.z,
        left.r0.x * right.r0.w + left.r0.y * right.r1.w + left.r0.z * right.r2.w + left.r0.w * right.r3.w
    };
    out.r1 = (Vec4){
        left.r1.x * right.r0.x + left.r1.y * right.r1.x + left.r1.z * right.r2.x + left.r1.w * right.r3.x,
        left.r1.x * right.r0.y + left.r1.y * right.r1.y + left.r1.z * right.r2.y + left.r1.w * right.r3.y,
        left.r1.x * right.r0.z + left.r1.y * right.r1.z + left.r1.z * right.r2.z + left.r1.w * right.r3.z,
        left.r1.x * right.r0.w + left.r1.y * right.r1.w + left.r1.z * right.r2.w + left.r1.w * right.r3.w
    };
    out.r2 = (Vec4){
        left.r2.x * right.r0.x + left.r2.y * right.r1.x + left.r2.z * right.r2.x + left.r2.w * right.r3.x,
        left.r2.x * right.r0.y + left.r2.y * right.r1.y + left.r2.z * right.r2.y + left.r2.w * right.r3.y,
        left.r2.x * right.r0.z + left.r2.y * right.r1.z + left.r2.z * right.r2.z + left.r2.w * right.r3.z,
        left.r2.x * right.r0.w + left.r2.y * right.r1.w + left.r2.z * right.r2.w + left.r2.w * right.r3.w
    };
    out.r3 = (Vec4){
        left.r3.x * right.r0.x + left.r3.y * right.r1.x + left.r3.z * right.r2.x + left.r3.w * right.r3.x,
        left.r3.x * right.r0.y + left.r3.y * right.r1.y + left.r3.z * right.r2.y + left.r3.w * right.r3.y,
        left.r3.x * right.r0.z + left.r3.y * right.r1.z + left.r3.z * right.r2.z + left.r3.w * right.r3.z,
        left.r3.x * right.r0.w + left.r3.y * right.r1.w + left.r3.z * right.r2.w + left.r3.w * right.r3.w
    };
    return out;
}

Vec4 mat4vec4multiply(Mat4 left, Vec4 right){
    return (Vec4){
        left.r0.x * right.x + left.r0.y * right.y + left.r0.z * right.z + left.r0.w * right.w,
        left.r1.x * right.x + left.r1.y * right.y + left.r1.z * right.z + left.r1.w * right.w,
        left.r2.x * right.x + left.r2.y * right.y + left.r2.z * right.z + left.r2.w * right.w,
        left.r3.x * right.x + left.r3.y * right.y + left.r3.z * right.z + left.r3.w * right.w
    };
}

void rotate4(Mat4 *mat, float angle, int axis){
    Mat4 trans;
    switch(axis){
        case 0: // x axis
            trans = (Mat4){
                (Vec4){1, 0, 0, 0},
                (Vec4){0, cos(angle), -1 * sin(angle), 0},
                (Vec4){0, sin(angle), cos(angle), 0},
                (Vec4){0, 0, 0, 1}
            };
            (*mat) = mat4mat4Multiply(trans, (*mat));
            break;
        case 1: // y axis
            trans = (Mat4){
                (Vec4){cos(angle), 0, sin(angle), 0},
                (Vec4){0, 1, 0, 0},
                (Vec4){-1 * sin(angle), 0, cos(angle), 0},
                (Vec4){0, 0, 0, 1}
            };
            (*mat) = mat4mat4Multiply(trans, (*mat));
            break;
        case 2: // z axis
            trans = (Mat4){
                (Vec4){cos(angle), sin(angle), 0, 0},
                (Vec4){-1 * sin(angle), cos(angle), 0, 0},
                (Vec4){0, 0, 1, 0},
                (Vec4){0, 0, 0, 1}
            };
            (*mat) = mat4mat4Multiply(trans, (*mat));
            break;
    }
}
void translate4(Mat4 *mat, Vec3 factor){
    Mat4 trans = (Mat4){
        (Vec4){1, 0, 0, factor.x},
        (Vec4){0, 1, 0, factor.y},
        (Vec4){0, 0, 1, factor.z},
        (Vec4){0, 0, 0, 1}
    };
    (*mat) = mat4mat4Multiply(trans, (*mat));
}
void scale4(Mat4 *mat, Vec3 scale){
    Mat4 trans = (Mat4){
        (Vec4){scale.x, 0, 0, 0},
        (Vec4){0, scale.y, 0, 0},
        (Vec4){0, 0, scale.z, 0},
        (Vec4){0, 0, 0, 1}
    };
    (*mat) = mat4mat4Multiply(trans, (*mat));
}

Vec4 scale(Vec4 vector, Vec3 factor){
    Vec4 out;
    out.x = vector.x * factor.x;
    out.y = vector.y * factor.y;
    out.z = vector.z * factor.z;
    out.w = vector.w;
    return out;
}
Vec4 translate(Vec4 vector, Vec3 factor){
    Vec4 out;
    out.x = vector.x + factor.x;
    out.y = vector.y + factor.y;
    out.z = vector.z + factor.z;
    out.w = vector.w;
    return out;
}
Vec4 rotate(Vec4 vector, int axis, double angle){
    Vec4 out;
    if(angle  > TWO_PI){
        printf("more than angle two pi\n");
    }
    switch(axis){
        case 0: //X AXIS
            
            out.x = vector.x;
            out.y = cos(angle) * vector.y - sin(angle) * vector.z;
            out.z = sin(angle) * vector.y + cos(angle) * vector.z;
            out.w = vector.w;
    
            break;

        case 1: //Y AXIS
            
            out.x = cos(angle) * vector.x + sin(angle) * vector.z;
            out.y = vector.y;
            out.z = cos(angle) * vector.z - sin(angle) * vector.x;
            out.w = vector.w;
    
            break;

        case 2: //Z AXIS
            
            out.x = cos(angle) * vector.x - sin(angle) * vector.y;
            out.y = sin(angle) * vector.x + cos(angle) * vector.y;
            out.z = vector.z;
            out.w = vector.w;
    
            break;
    }
    return out;
}

Mat4 perspective(float fovy, float aspect, float znear, float zfar){
    float fovy_rad = fovy * (PI / 180.0f);
    float f = 1 / tan(fovy_rad / 2);
    Mat4 out = (Mat4){
        (Vec4){f/aspect, 0, 0, 0},
        (Vec4){0, f, 0, 0},
        (Vec4){0, 0, (zfar + znear)/(znear - zfar), (2 * zfar * znear)/(znear - zfar)},
        (Vec4){0, 0, -1, 0}
    };
    return out;
}

Mat4 lookAt(Vec3 cameraPos, Vec3 target, Vec3 up) {
    Vec3 forward = normalize(minus3(target, cameraPos));       // Forward (Z-)
    Vec3 right   = normalize(cross(up, forward));              // Right (X)
    Vec3 trueUp  = cross(forward, right);                      // Ortho up (Y)

    // Rotation matrix (camera basis vectors)
    Mat4 rotation = {
        (Vec4){ right.x,   right.y,   right.z,   0.0 },
        (Vec4){ trueUp.x,  trueUp.y,  trueUp.z,  0.0 },
        (Vec4){-forward.x, -forward.y,-forward.z,0.0 }, // negate forward for RH system
        (Vec4){ 0.0,       0.0,       0.0,       1.0 }
    };

    // Translation matrix
    Mat4 translation = {
        (Vec4){ 1.0, 0.0, 0.0, -cameraPos.x },
        (Vec4){ 0.0, 1.0, 0.0, -cameraPos.y },
        (Vec4){ 0.0, 0.0, 1.0, -cameraPos.z },
        (Vec4){ 0.0, 0.0, 0.0, 1.0 }
    };

    return mat4mat4Multiply(rotation, translation);
}

void printVec2(Vec2 v) {
    printf("Vec2(x: %f, y: %f)\n", v.x, v.y);
}

void printVec3(Vec3 v) {
    printf("Vec3(x: %f, y: %f, z: %f)\n", v.x, v.y, v.z);
}

void printVec4(Vec4 v) {
    printf("Vec4(x: %f, y: %f, z: %f, w: %f)\n", v.x, v.y, v.z, v.w);
}


