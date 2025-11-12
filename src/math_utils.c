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

Vec3 normalTransform(Mat3 model, Vec3 aNormal) {
    Mat3 inv = mat3Inverse(model);
    Mat3 invT = mat3Transpose(inv);
    return mat3vec3multiply(invT, aNormal);
}

Mat3 mat3Transpose(Mat3 m) {
    return createMat3(
        m.r0.x, m.r1.x, m.r2.x,
        m.r0.y, m.r1.y, m.r2.y,
        m.r0.z, m.r1.z, m.r2.z
    );
}

Mat3 mat3Inverse(Mat3 m) {
    float a00 = m.r0.x, a01 = m.r0.y, a02 = m.r0.z;
    float a10 = m.r1.x, a11 = m.r1.y, a12 = m.r1.z;
    float a20 = m.r2.x, a21 = m.r2.y, a22 = m.r2.z;

    float det =
        a00 * (a11 * a22 - a12 * a21) -
        a01 * (a10 * a22 - a12 * a20) +
        a02 * (a10 * a21 - a11 * a20);

    if (fabs(det) < 1e-8f) {
        // Matrix is not invertible, return identity as fallback
        return createMat3(1, 0, 0,
                           0, 1, 0,
                           0, 0, 1);
    }

    float invDet = 1.0f / det;

    return createMat3(
        (a11 * a22 - a12 * a21) * invDet,
        (a02 * a21 - a01 * a22) * invDet,
        (a01 * a12 - a02 * a11) * invDet,

        (a12 * a20 - a10 * a22) * invDet,
        (a00 * a22 - a02 * a20) * invDet,
        (a02 * a10 - a00 * a12) * invDet,

        (a10 * a21 - a11 * a20) * invDet,
        (a01 * a20 - a00 * a21) * invDet,
        (a00 * a11 - a01 * a10) * invDet
    );
}

Vec3 mat3vec3multiply(Mat3 m, Vec3 v) {
    Vec3 out;
    out.x = m.r0.x * v.x + m.r0.y * v.y + m.r0.z * v.z;
    out.y = m.r1.x * v.x + m.r1.y * v.y + m.r1.z * v.z;
    out.z = m.r2.x * v.x + m.r2.y * v.y + m.r2.z * v.z;
    return out;
}

Mat3 getMat3(Mat4 m) {
    Mat3 out;
    out.r0.x = m.r0.x; out.r0.y = m.r0.y; out.r0.z = m.r0.z;
    out.r1.x = m.r1.x; out.r1.y = m.r1.y; out.r1.z = m.r1.z;
    out.r2.x = m.r2.x; out.r2.y = m.r2.y; out.r2.z = m.r2.z;
    return out;
}

Mat3 createMat3(
    float m00, float m01, float m02,
    float m10, float m11, float m12,
    float m20, float m21, float m22
) {
    Mat3 m;
    m.r0.x = m00; m.r0.y = m01; m.r0.z = m02;
    m.r1.x = m10; m.r1.y = m11; m.r1.z = m12;
    m.r2.x = m20; m.r2.y = m21; m.r2.z = m22;
    return m;
}

Vec3 abs3(Vec3 v){
    Vec3 out;
    if(v.x < 0){
        out.x = v.x * -1;
    }else{
        out.x = v.x;
    }
    if(v.y < 0){
        out.y = v.y * -1;
    }else{
        out.y = v.y;
    }
    if(v.z < 0){
        out.z = v.z * -1;
    }else{
        out.z = v.z;
    }
    return out;
}

Vec3 reflect(Vec3 I, Vec3 N) {
    // assumes N is already normalized
    float dotNI = dot3(N, I);
    Vec3 scaledNormal = scale3(N, 2.0f * dotNI);
    return minus3(I, scaledNormal);
}

Vec3 scale3(Vec3 v, float s) {
    Vec3 out = { v.x * s, v.y * s, v.z * s };
    return out;
}

uint8_t clamp(int value, int min, int max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}