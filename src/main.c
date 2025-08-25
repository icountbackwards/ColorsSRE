//TODO:
//1. TESTING REQUIRED -> FUNCTION groupVertices(). Note: No reordering of vertices established, possiblity for
//                                                       wrong triangulation and incorrect primitive construction.

#include <Windows.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <math.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <string.h>

#include "data.c"

#define TWO_PI 6.28318530718
#define TWO_PI_SHORT 6.28
#define PI 3.14159265358979323846f

#define PIPELINE_VARIATION_MESH 1
#define PIPELINE_VARIATION_LIGHT_SOURCE 2

static boolean quit = FALSE;

struct {
    int width;
    int height;
    uint32_t *pixels;
}frame = { 0 };

typedef struct
{
    float x;
    float y;
}Vec2;

typedef struct
{
    float x;
    float y;
    float z;
}Vec3;

typedef struct
{
    float x;
    float y;
    float z;
    float w;
}Vec4;

typedef struct {
    float *data;
    int dataSize;
    int *layout;
    int layoutSize;
    int *indices;
    int indicesSize;
} VertexBuffer;

typedef struct {
    float *data;
    int size;
} Vertex;

typedef struct {
    Vertex *vertices;
    int *indices;
    int *layout;
    int vertexAmount;
    int indicesSize;
    int layoutSize;
} VertexShaderOutput;

typedef struct {
    Vec2 position;
    float zval;
    Vec4 color;
    float *data;
    Vec2 texCoord;
    Vec3 normal;
    Vec3 fragPos;
} Fragment;

typedef struct {
    Vertex v1;
    Vertex v2;
    Vertex v3;
    Fragment *fragments;
    int fragmentAmount;
} Primitive;

typedef struct {
    Primitive *primitives;
    int *layout;
    int primitiveAmount;
    int layoutSize;
} PrimitiveAssemblyOutput;

typedef struct {
    Primitive *primitives;
    int *layout;
    int primitiveAmount;
    int layoutSize;
} PrimitiveOutput;

typedef struct{
    Primitive *primitives;
    int size;
} PrimitiveGroup;

typedef struct {
    Primitive *primitives;
    int *layout;
    int primitiveAmount;
    int layoutSize;
} ClippingOutput;

typedef struct {
    Vec2 Max;
    Vec2 Min;
} AABB;

typedef struct {
    Vec4 r0;
    Vec4 r1;
    Vec4 r2;
} Mat3;

typedef struct {
    Vec4 r0;
    Vec4 r1;
    Vec4 r2;
    Vec4 r3;
} Mat4;

typedef struct {
    Mat4 model;
    Mat4 view;
    Mat4 projection;
    Vec3 objectColor;
    Vec3 lightColor;
    Vec3 lightPos;
    Vec3 viewPos;
} UniformBuffer;

typedef struct {
    Fragment *fragments;
    int fragmentSize;
} TriangleTraversalOutput;

typedef struct {
    Fragment* fragments;
    int fragmentSize;
} FragmentShaderOutput;

typedef struct {
    Vec3* pixels;
    int width;
    int height;
} Texture;

typedef struct {
    float *vertices; // interleaved
    int vertexCount; // number of floats
    int *indices;
    int indexCount;
} MeshData;

int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 600;

static BITMAPINFO frame_bitmap_info;
static HBITMAP frame_bitmap = 0;
static HDC frame_device_context = 0;

double lastTime = 0;

bool mouseFreeze = false;
bool mouseDeltaFreeze = false;
POINT mousepos;
POINT mouseDeltaPos;
POINT mouseLastPos;
float yaw = -90.0;
float pitch = 0.0;

float objrotate0 = 0;
float objrotate1 = 0;
float objrotate2 = 0;

float mouseSensitivity = 0.5;

Vec3 cameraPos = {0.0, 0.0, 3.0};
Vec3 cameraFront = {0.0, 0.0, -1.0};
Vec3 cameraUp = {0.0, 1.0, 0.0};

Vec3 lightPosition = {-1.2, 1.5, 0.5};
bool isVertical = false;

float cameraSpeed = 0.7;

bool backtofront = false;

Vec3 cubePositions[10] = {
    {0.0, 0.0, 0.0},
    {2.0, 5.0, -15.0},
    {-1.5, -2.2, -2.5},
    {-3.8, -2.0, -12.3},
    {2.4, -0.4, -3.5},
    {-1.7, 3.0, -7.5},
    {1.3, -2.0, -2.5},
    {1.5, 2.0, -2.5},
    {1.5, 0.2, -1.5},
    {-1.3, 1.0, -1.5}
};

float deltaTime = 0.0;
float lastFrame = 0.0;

UniformBuffer *UniformBufferRegister;
Texture notexture;

MeshData suzanne_flat;
VertexBuffer suzanne_flat_vbo;

float *depthBuffer;

Vertex nullvertex = {0};

Mat4 identityMat4 = {
    {1, 0, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 1}
};

bool debug = true;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

VertexBuffer generateVertexBuffer(float* data, int* indices, int* layout, int datasize, int indicesSize, int layoutSize);

void draw(VertexBuffer *vbo, UniformBuffer *ubo, Texture* pTextureResource, int pipelineVariation);

int getVertexAmount(VertexBuffer *vbo);
Vec2 getVertexData2(float **data, int index, int* layout, int loc);
Vec3 getVertexData3(float **data, int index, int* layout, int loc);
Vec4 getVertexData4(float **data, int index, int* layout, int loc);
Vertex getIntersectingPoint(Vertex v1, Vertex v2, Vec3 clipPlane);
void mapVertexShaderDataOutput3(Vertex *vertex, Vec3 output, int loc, int* layout);
void mapVertexShaderDataOutput4(Vertex *vertex, Vec4 output, int loc, int* layout);
bool isInside(Vertex v, Vec3 clipPlane);
PrimitiveGroup groupPrimitives(PrimitiveGroup parent, PrimitiveGroup child);
PrimitiveGroup groupVertices(Vertex *vertices, int amount);
void bindPrimitiveGroup(Primitive **primitiveArray, PrimitiveGroup primitiveGroup);
float dot3(Vec3 v1, Vec3 v2);
float abs_(float n);
Vec3 minus3(Vec3 front, Vec3 back);
Vec3 plus3(Vec3 front, Vec3 back);
Vec3 normalize(Vec3 v);

Vec3 scalarMultiply3(float scalar, Vec3 vector);
float getMax(float n1, float n2);
float getMin(float n1, float n2);
AABB getBoundingBox(Primitive *primitive);
int edgeCross1(Vertex a, Vertex b, Vec2 p);
int edgeCross2(Vertex a, Vertex b, Vertex p);
Fragment *groupFragments(Fragment **parent, Fragment **child, int *parentSize, int childSize);
Primitive rewindPrimitive(Primitive primitive);
void clearDepthBuffer();
void clearFrameBuffer();
double getTime();
Vec3* load_png_as_vec3(const char* filename, int* out_width, int* out_height);
bool isTopLeft(Vec2 a, Vec2 b);
static inline uint8_t clamp(int value, int min, int max);
Vec3 getTexturePixel(Texture* pTex, float tx, float ty);

Mat4 mat4mat4Multiply(Mat4 left, Mat4 right);
Vec4 mat4vec4multiply(Mat4 left, Vec4 right);
Vec3 cross(Vec3 a, Vec3 b);
Vec3 multiply3(Vec3 v1, Vec3 v2);

void rotate4(Mat4 *mat, float angle, int axis);
void translate4(Mat4 *mat, Vec3 factor);
void scale4(Mat4 *mat, Vec3 scale);
Mat4 perspective(float fovy, float aspect, float znear, float zfar);
Mat4 lookAt(Vec3 cameraPos, Vec3 invDirection, Vec3 cameraUp);

void writepixel(int x, int y, uint32_t col);
void handleMouse(long x, long y);

VertexShaderOutput runVertexShader(VertexBuffer *vbo, UniformBuffer *ubo, int pipelineVariation);
PrimitiveAssemblyOutput runPrimitiveAssembly(VertexShaderOutput vertices);
ClippingOutput runPrimitiveClipping(PrimitiveAssemblyOutput primitives);
PrimitiveOutput runPerspectiveDivide(ClippingOutput primitives);
void runScreenMapping(PrimitiveOutput *primitives);
TriangleTraversalOutput runRasterizer(PrimitiveOutput *primitives, int pipelineVariation);
FragmentShaderOutput runFragmentShader(TriangleTraversalOutput *in, Texture* texture, UniformBuffer *ubo, int pipelineVariation);
void runMerger(FragmentShaderOutput *in, Texture* texture);
MeshData loadOBJ(const char *filename);

Texture createTexture(char* filename);

Vec4 scale(Vec4 vector, Vec3 factor);
Vec4 translate(Vec4 vector, Vec3 factor);
Vec4 rotate(Vec4 vector, int axis, double angle);

void printVec2(Vec2 v);
void printVec3(Vec3 v);
void printVec4(Vec4 v);
void printVertex(Vertex v, int layoutSize);
void printVertexShaderOutput(VertexShaderOutput out);
void printPrimitive(Primitive p, int layoutSize);
void printPrimitiveOutput(PrimitiveOutput out);
void printTriangleTraversalOutput(TriangleTraversalOutput out);
void printFragmentShaderOutput(FragmentShaderOutput out);
void printPrimitiveOutput2(ClippingOutput out);
void printPrimitiveOutput3(PrimitiveAssemblyOutput out);

void printMat4(Mat4 m) {
    printf("[[%f %f %f %f]\n", m.r0.x, m.r0.y, m.r0.z, m.r0.w);
    printf(" [%f %f %f %f]\n", m.r1.x, m.r1.y, m.r1.z, m.r1.w);
    printf(" [%f %f %f %f]\n", m.r2.x, m.r2.y, m.r2.z, m.r2.w);
    printf(" [%f %f %f %f]]\n", m.r3.x, m.r3.y, m.r3.z, m.r3.w);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow){

    static WNDCLASS window_class = { 0 };
    static const wchar_t window_class_name[] = L"WindowClass";
    window_class.lpszClassName = (PCSTR)window_class_name;
    window_class.lpfnWndProc = WindowProc;
    window_class.hInstance = hInstance;

    RegisterClass(&window_class);

    frame_bitmap_info.bmiHeader.biSize = sizeof(frame_bitmap_info.bmiHeader);
    frame_bitmap_info.bmiHeader.biPlanes = 1;
    frame_bitmap_info.bmiHeader.biBitCount = 32;
    frame_bitmap_info.bmiHeader.biCompression = BI_RGB;
    frame_device_context = CreateCompatibleDC(0);

    frame.width = SCREEN_WIDTH;
    frame.height = SCREEN_HEIGHT;

    HWND hwnd = CreateWindow((PCSTR)window_class_name, "Win32", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, SCREEN_WIDTH + 16 + 1, SCREEN_HEIGHT + 39 + 1, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);

    lastTime = getTime();
    int frameCount = 0;

    depthBuffer = malloc(sizeof(float) * (frame.width+1) * (frame.height+1));

    VertexBuffer vbo1 = generateVertexBuffer(vbo1data, vbo1indices, vbo1layout, vbo1size, vbo1indicesSize, vbo1layoutsize);
    VertexBuffer lightvbo = generateVertexBuffer(lightdata, lightindices, lightlayout, lightsize, lightindicessize, lightlayoutsize);
    
    UniformBuffer ubo1;

    Texture texture1 = createTexture("../assets/gold.png");

    printf("tex width: %d\n", texture1.width);
    printf("tex height: %d\n", texture1.height);

    suzanne_flat = loadOBJ("../assets/suzanne_smooth.obj");
    suzanne_flat_vbo = generateVertexBuffer(suzanne_flat.vertices, suzanne_flat.indices, vbo1layout, suzanne_flat.vertexCount, suzanne_flat.indexCount, vbo1layoutsize);

    int run = 0;
    while(!quit){
        if(run == 1){
            debug = false;
        }
        static MSG msg = { 0 };
        while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        GetCursorPos(&mousepos);
        if(!mouseFreeze && !mouseDeltaFreeze){
            handleMouse(mousepos.x, mousepos.y);
        }
        if(mouseDeltaFreeze){
            mouseDeltaFreeze = false;
            mouseDeltaPos.x = 0;
            mouseDeltaPos.y = 0;
            mouseLastPos = mousepos;
        }

        //

        //static unsigned int p = 0;
        //frame.pixels[(p++)%(frame.width*frame.height)] = rand();
        //frame.pixels[rand()%(frame.width*frame.height)] = 0X00ff0000;

        //
        
        clearFrameBuffer();
        clearDepthBuffer();

        Mat4 model = identityMat4;

        float time = getTime();
        
        float currentFrame = time;
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        cameraSpeed = 5.0 * deltaTime;

        
        Vec3 direction;
        direction.z = cos((yaw * TWO_PI) / 180) * cos ((pitch * TWO_PI) / 180);
        direction.y = -sin((pitch * TWO_PI) / 180);
        direction.x = sin((yaw * TWO_PI) / 180) * cos((pitch * TWO_PI) / 180);
        cameraFront = normalize(direction);    

        if(backtofront){
            cameraFront = (Vec3){0.0, 0.0, -1.0};
            cameraPos = (Vec3){0.0, 0.0, 3.0};
            yaw = -90.0;
            pitch = 0.0;
            backtofront = false;
        }
        Mat4 view = lookAt(cameraPos, plus3(cameraPos, cameraFront), cameraUp);

        //REVERSE Z IMP
        Mat4 projection = perspective(45.0, ((float)frame.width)/((float)frame.height), 100.0, 0.1);
        //Mat4 projection = perspective(45.0, ((float)frame.width)/((float)frame.height), 0.1, 100.0);


        for(int i = 0; i < 1; i++){
            model = identityMat4;
            translate4(&model, cubePositions[i]);
            rotate4(&model, objrotate0, 0);
            rotate4(&model, objrotate1, 1);
            rotate4(&model, objrotate2, 2);
            //rotate4(&model, time, i % 2);

            ubo1.model = model;
            ubo1.view = view;
            ubo1.projection = projection;
            ubo1.objectColor = (Vec3){1.0, 0.5, 1.0};
            ubo1.lightColor = (Vec3){1.0, 1.0, 1.0};
            ubo1.lightPos = lightPosition;
            ubo1.viewPos = cameraPos;

            //draw(&vbo1, &ubo1, &texture1, PIPELINE_VARIATION_MESH);
            draw(&suzanne_flat_vbo, &ubo1, &texture1, PIPELINE_VARIATION_MESH);
        }


        model = identityMat4;
        scale4(&model, (Vec3){0.2, 0.2, 0.2});
        translate4(&model, lightPosition);
        ubo1.model = model;
        ubo1.view = view;
        ubo1.projection = projection;
        draw(&lightvbo, &ubo1, &notexture, PIPELINE_VARIATION_LIGHT_SOURCE);
        

        frameCount++;
        double currentTime = time;
        double elapsed = currentTime - lastTime;
        if (elapsed >= 1.0) { // Every 1 second
            printf("FPS: %d\n", frameCount);
            frameCount = 0;
            lastTime = currentTime;
        }

        InvalidateRect(hwnd, NULL, FALSE);
        UpdateWindow(hwnd);

        run++;
    }

    free(depthBuffer);

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam){

    switch(msg){
        case WM_QUIT:
        case WM_DESTROY: {
            quit = TRUE;
        } break;

        case WM_PAINT: {
            static PAINTSTRUCT paint;
            static HDC device_context;
            device_context = BeginPaint(hwnd, &paint);
            BitBlt(device_context,
                    paint.rcPaint.left, paint.rcPaint.top,
                    paint.rcPaint.right - paint.rcPaint.left, paint.rcPaint.bottom - paint.rcPaint.top,
                    frame_device_context,
                    paint.rcPaint.left, paint.rcPaint.top,
                SRCCOPY);
            EndPaint(hwnd, &paint);
        }break;

        case WM_SIZE:{
            frame_bitmap_info.bmiHeader.biWidth = LOWORD(lparam);
            frame_bitmap_info.bmiHeader.biHeight = HIWORD(lparam);
            
            if(frame_bitmap) DeleteObject(frame_bitmap);
            frame_bitmap = CreateDIBSection(NULL, &frame_bitmap_info, DIB_RGB_COLORS, (void**)&frame.pixels, 0, 0);
            SelectObject(frame_device_context, frame_bitmap);

            frame.width = LOWORD(lparam);
            frame.height = HIWORD(lparam);
        }break;

        case WM_KEYDOWN:
            switch (wparam) {
                
                case 'W':
                    cameraPos = plus3(cameraPos, scalarMultiply3(cameraSpeed, cameraFront));
                    break;
                case 'A':
                    cameraPos = plus3(cameraPos, scalarMultiply3(cameraSpeed, cross(cameraFront, cameraUp)));
                    break;
                case 'S':
                    cameraPos = minus3(cameraPos, scalarMultiply3(cameraSpeed, cameraFront));
                    break;
                case 'D':
                    cameraPos = minus3(cameraPos, scalarMultiply3(cameraSpeed, cross(cameraFront, cameraUp)));
                    break;
                case 'M':
                    mouseFreeze = !mouseFreeze;
                    mouseDeltaFreeze = true;
                    break;
                case 'B':
                    backtofront = true;
                    break;
                case VK_ESCAPE:
                    quit = true;
                    break;
                case 'V':
                    isVertical = !isVertical;
                    break;
                case 'I':
                    if(isVertical){
                        lightPosition.y += cameraSpeed;
                    } else{
                        lightPosition.z -= cameraSpeed;
                    }
                    break;
                case 'J':
                    //cameraPos = plus3(cameraPos, scalarMultiply3(cameraSpeed, cross(cameraFront, cameraUp)));
                    lightPosition.x += cameraSpeed;
                    break;
                case 'K':
                    //cameraPos = minus3(cameraPos, scalarMultiply3(cameraSpeed, cameraFront));
                    if(isVertical){
                        lightPosition.y -= cameraSpeed;
                    } else{
                        lightPosition.z += cameraSpeed;
                    }
                    break;
                case 'L':
                    //cameraPos = minus3(cameraPos, scalarMultiply3(cameraSpeed, cross(cameraFront, cameraUp)));
                    lightPosition.x -= cameraSpeed;
                    break;
                case VK_UP: // same as W
                    objrotate0 -= 0.1;
                    break;
                case VK_DOWN: // same as S
                    objrotate0 += 0.1;
                    break;
                case VK_LEFT: // same as A
                    objrotate1 += 0.1;
                    break;
                case VK_RIGHT: // same as D
                    objrotate1 -= 0.1;
                    break;
            }
            break;
            

        default: {
            return DefWindowProc(hwnd, msg, wparam, lparam);
        } break;
    }
    return 0;
}

VertexBuffer generateVertexBuffer(float* data, int* indices, int* layout, int datasize, int indicesSize, int layoutSize){
    VertexBuffer newvbo;
    newvbo.data = data;
    newvbo.indices = indices;
    newvbo.layout = layout;
    newvbo.dataSize = datasize;
    newvbo.indicesSize = indicesSize;
    newvbo.layoutSize = layoutSize;
    return newvbo;
}

void draw(VertexBuffer *vbo, UniformBuffer *ubo, Texture* pTextureResource, int pipelineVariation){
    
    VertexShaderOutput vertexShaderOutput = runVertexShader(vbo, ubo, pipelineVariation);
    
    PrimitiveAssemblyOutput primitiveAssemblyOutput = runPrimitiveAssembly(vertexShaderOutput);
    
    ClippingOutput primitiveClippingOutput = runPrimitiveClipping(primitiveAssemblyOutput);
    
    PrimitiveOutput perspectiveDivideOutput = runPerspectiveDivide(primitiveClippingOutput);
    
    runScreenMapping(&perspectiveDivideOutput);
    
    TriangleTraversalOutput triangleTraversalOutput = runRasterizer(&perspectiveDivideOutput, pipelineVariation);
    
    FragmentShaderOutput fragmentShaderOutput = runFragmentShader(&triangleTraversalOutput, pTextureResource, ubo, pipelineVariation);

    runMerger(&fragmentShaderOutput, pTextureResource);

    free(vertexShaderOutput.vertices);
    free(vertexShaderOutput.layout);    
    free(primitiveAssemblyOutput.primitives);
    free(primitiveClippingOutput.primitives);
    for(int i = 0; i < primitiveClippingOutput.primitiveAmount; i++){
        free(perspectiveDivideOutput.primitives[i].v1.data);
        free(perspectiveDivideOutput.primitives[i].v2.data);
        free(perspectiveDivideOutput.primitives[i].v3.data);
    }
    free(perspectiveDivideOutput.primitives);
    free(triangleTraversalOutput.fragments);
    //free(fragmentShaderOutput.fragments);
    
    
}

int getVertexAmount(VertexBuffer *vbo){
    return vbo->dataSize / vbo->layout[2];
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

void updateUniformBuffer(){

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

Mat3 mat3_create(
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

// Transpose of a 3x3
Mat3 mat3_transpose(Mat3 m) {
    return mat3_create(
        m.r0.x, m.r1.x, m.r2.x,
        m.r0.y, m.r1.y, m.r2.y,
        m.r0.z, m.r1.z, m.r2.z
    );
}

// Inverse of a 3x3
Mat3 mat3_inverse(Mat3 m) {
    float a00 = m.r0.x, a01 = m.r0.y, a02 = m.r0.z;
    float a10 = m.r1.x, a11 = m.r1.y, a12 = m.r1.z;
    float a20 = m.r2.x, a21 = m.r2.y, a22 = m.r2.z;

    float det =
        a00 * (a11 * a22 - a12 * a21) -
        a01 * (a10 * a22 - a12 * a20) +
        a02 * (a10 * a21 - a11 * a20);

    if (fabs(det) < 1e-8f) {
        // Matrix is not invertible, return identity as fallback
        return mat3_create(1, 0, 0,
                           0, 1, 0,
                           0, 0, 1);
    }

    float invDet = 1.0f / det;

    return mat3_create(
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

// Multiply Mat3 * Vec3
Vec3 mat3_mul_vec3(Mat3 m, Vec3 v) {
    Vec3 out;
    out.x = m.r0.x * v.x + m.r0.y * v.y + m.r0.z * v.z;
    out.y = m.r1.x * v.x + m.r1.y * v.y + m.r1.z * v.z;
    out.z = m.r2.x * v.x + m.r2.y * v.y + m.r2.z * v.z;
    return out;
}

Mat3 mat3_from_mat4(Mat4 m) {
    Mat3 out;
    out.r0.x = m.r0.x; out.r0.y = m.r0.y; out.r0.z = m.r0.z;
    out.r1.x = m.r1.x; out.r1.y = m.r1.y; out.r1.z = m.r1.z;
    out.r2.x = m.r2.x; out.r2.y = m.r2.y; out.r2.z = m.r2.z;
    return out;
}

Vec3 normal_transform(Mat3 model, Vec3 aNormal) {
    Mat3 inv = mat3_inverse(model);
    Mat3 invT = mat3_transpose(inv);
    return mat3_mul_vec3(invT, aNormal);
}

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
                Vec3 loc3out = normal_transform(mat3_from_mat4(ubo->model), loc3in);
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

float dot3(Vec3 v1, Vec3 v2){
    return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
}

float abs_(float n){
    if(n >= 0){
        return n;
    }else{
         return n * -1;
    }
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



Vec3 minus3(Vec3 front, Vec3 back){
    Vec3 out;
    out.x = front.x - back.x;
    out.y = front.y - back.y;
    out.z = front.z - back.z;
    return out;
}

Vec3 scalarMultiply3(float scalar, Vec3 vector){
    Vec3 out;
    out.x = scalar * vector.x;
    out.y = scalar * vector.y;
    out.z = scalar * vector.z;
    return out;
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

void runScreenMapping(PrimitiveOutput *primitives){
    for(int i = 0; i < primitives->primitiveAmount; i++){
        primitives->primitives[i].v1.data[0] = (frame.width / 2) * primitives->primitives[i].v1.data[0] + (frame.width / 2);
        primitives->primitives[i].v1.data[1] = (frame.height / 2) * primitives->primitives[i].v1.data[1] + (frame.height / 2);
        primitives->primitives[i].v1.data[2] = 0.5 * primitives->primitives[i].v1.data[2] + 0.5;

        primitives->primitives[i].v2.data[0] = (frame.width / 2) * primitives->primitives[i].v2.data[0] + (frame.width / 2);
        primitives->primitives[i].v2.data[1] = (frame.height / 2) * primitives->primitives[i].v2.data[1] + (frame.height / 2);
        primitives->primitives[i].v2.data[2] = 0.5 * primitives->primitives[i].v2.data[2] + 0.5;

        primitives->primitives[i].v3.data[0] = (frame.width / 2) * primitives->primitives[i].v3.data[0] + (frame.width / 2);
        primitives->primitives[i].v3.data[1] = (frame.height / 2) * primitives->primitives[i].v3.data[1] + (frame.height / 2);
        primitives->primitives[i].v3.data[2] = 0.5 * primitives->primitives[i].v3.data[2] + 0.5;
    }
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

AABB getBoundingBox(Primitive *primitive){
    AABB out;
    out.Max.x = getMax(primitive->v1.data[0], getMax(primitive->v2.data[0], primitive->v3.data[0]));
    out.Max.y = getMax(primitive->v1.data[1], getMax(primitive->v2.data[1], primitive->v3.data[1]));
    out.Min.x = getMin(primitive->v1.data[0], getMin(primitive->v2.data[0], primitive->v3.data[0]));
    out.Min.y = getMin(primitive->v1.data[1], getMin(primitive->v2.data[1], primitive->v3.data[1]));
    return out;
}

int edgeCross1(Vertex a, Vertex b, Vec2 p){
    return ((b.data[0] - a.data[0])*(p.y-a.data[1]) - (b.data[1]-a.data[1])*(p.x-a.data[0]));
}
int edgeCross2(Vertex a, Vertex b, Vertex p){
    return ((b.data[0] - a.data[0])*(p.data[1]-a.data[1]) - (b.data[1]-a.data[1])*(p.data[0]-a.data[0]));
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

TriangleTraversalOutput runRasterizer(PrimitiveOutput *primitives, int pipelineVariation) {
    TriangleTraversalOutput out;
    out.fragments = malloc(sizeof(Fragment) * frame.height * frame.width);
    out.fragmentSize = 0;

    for (int i = 0; i < primitives->primitiveAmount; i++) {
        primitives->primitives[i] = rewindPrimitive(primitives->primitives[i]);

        AABB aabb = getBoundingBox(&primitives->primitives[i]);
        aabb.Min.x = fmax(0, aabb.Min.x);
        aabb.Min.y = fmax(0, aabb.Min.y);
        aabb.Max.x = fmin(frame.width - 1, aabb.Max.x);
        aabb.Max.y = fmin(frame.height - 1, aabb.Max.y);

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
                    if (out.fragmentSize >= frame.width * frame.height) {
                        //FRAGMENT BUFFER OVERFLOW
                        break;
                    }
                    if(x < 0 || x > frame.width || y < 0 || y > frame.height){
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

Vec3 multiply3(Vec3 v1, Vec3 v2){
    return (Vec3){v1.x * v2.x, v1.y * v2.y, v1.z * v2.z};
}

Vec3 vec3_scale(Vec3 v, float s) {
    Vec3 out = { v.x * s, v.y * s, v.z * s };
    return out;
}

Vec3 reflect(Vec3 I, Vec3 N) {
    // assumes N is already normalized
    float dotNI = dot3(N, I);
    Vec3 scaledNormal = vec3_scale(N, 2.0f * dotNI);
    return minus3(I, scaledNormal);
}

FragmentShaderOutput runFragmentShader(TriangleTraversalOutput *in, Texture* texture, UniformBuffer *ubo, int pipelineVariation) {
    FragmentShaderOutput out;

    switch(pipelineVariation){
        case PIPELINE_VARIATION_LIGHT_SOURCE:
            for(int i = 0; i < in->fragmentSize; i++){
                in->fragments[i].color.x = 255.0;
                in->fragments[i].color.y = 255.0;
                in->fragments[i].color.z = 255.0;

            }
            break;
        case PIPELINE_VARIATION_MESH:
            for(int i = 0; i < in->fragmentSize; i++){
                Vec3 texel = getTexturePixel(texture, in->fragments[i].data[7], in->fragments[i].data[8]);
                in->fragments[i].color.x = (in->fragments[i].color.x * texel.x);
                in->fragments[i].color.y = (in->fragments[i].color.y * texel.y);
                in->fragments[i].color.z = (in->fragments[i].color.z * texel.z);

                Vec3 inColor = (Vec3){in->fragments[i].color.x, in->fragments[i].color.y, in->fragments[i].color.z};
                Vec3 inNormal = (Vec3){in->fragments[i].data[9], in->fragments[i].data[10], in->fragments[i].data[11]};
                Vec3 inFragPos = (Vec3){in->fragments[i].data[12], in->fragments[i].data[13], in->fragments[i].data[14]};

                float ambientStrength = 0.1;
                float specularStrength = 0.5;
                
                Vec3 ambient = scalarMultiply3(ambientStrength, ubo->lightColor);

                Vec3 norm = normalize(inNormal);
                Vec3 lightDir = normalize(minus3(ubo->lightPos, inFragPos));

                Vec3 viewDir = normalize(minus3(ubo->viewPos, inFragPos));
                Vec3 reflectDir = reflect(scalarMultiply3(-1, lightDir), norm);  

                float spec = pow(max(dot3(viewDir, reflectDir), 0.0), 128);
                Vec3 specular = scalarMultiply3(specularStrength * spec, ubo->lightColor);

                float diff = max(dot3(norm, lightDir), 0.0);
                Vec3 diffuse = scalarMultiply3(diff, ubo->lightColor);

                Vec3 globalLight = {0.4, 0.4, 0.4};

                Vec3 result = multiply3(plus3(plus3(plus3(ambient, diffuse), specular), globalLight), inColor);

                

                in->fragments[i].color.x = result.x * texel.x/ 255.0;
                in->fragments[i].color.y = result.y * texel.y/ 255.0;
                in->fragments[i].color.z = result.z * texel.z/ 255.0;
                
                if(debug && i < 20){
                    printVec4(in->fragments[i].color);
                }
                free(in->fragments[i].data);
            }
            break;
    }
    
    out.fragments = in->fragments;
    out.fragmentSize = in->fragmentSize;

    
    
    return out;
}

void clearDepthBuffer() {
    for (int i = 0; i < frame.height+1; i++) {
        for (int j = 0; j < frame.width+1; j++) {
            //REVERSE Z IMP
            depthBuffer[i * frame.width + j] = 0.0f;
            //depthBuffer[i * frame.width + j] = 100.0f;
        }
    }
}

void clearFrameBuffer() {
    for (int i = 0; i < frame.height+1; i++) {
        for (int j = 0; j < frame.width+1; j++) {
            frame.pixels[i * frame.width + j] = 0x00000000;
        }
    }
}
void runMerger(FragmentShaderOutput *in, Texture* texture) {
    for (int i = 0; i < in->fragmentSize; i++) {
        int x = (int)in->fragments[i].position.x;
        int y = (int)in->fragments[i].position.y;

        if (x < 0 || x >= frame.width || y < 0 || y >= frame.height) {
            fprintf(stderr, "⚠️ Fragment %d out of bounds: x = %d, y = %d\n", i, x, y);
            continue;
        }

        int index = y * frame.width + x;

        //REVERSE Z IMP
        if (in->fragments[i].zval > depthBuffer[index]) {
            depthBuffer[index] = in->fragments[i].zval;
            
            uint8_t r = clamp(in->fragments[i].color.x, 0, 255);
            uint8_t g = clamp(in->fragments[i].color.y, 0, 255);
            uint8_t b = clamp(in->fragments[i].color.z, 0, 255);
            uint8_t a = clamp(in->fragments[i].color.w, 0, 255);
            

            //uint8_t r = (uint8_t)(tex.x * 255.0);
            //uint8_t g = (uint8_t)(tex.y * 255.0);
            //uint8_t b = (uint8_t)(tex.z * 255.0);
            //uint8_t a = clamp(in->fragments[i].color.w, 0, 255);

            uint32_t interp_color = (a << 24) | (r << 16) | (g << 8) | (b);

            frame.pixels[index] = interp_color;
        }
    }
}

static inline uint8_t clamp(int value, int min, int max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
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

double getTime() {
    static LARGE_INTEGER frequency;
    static LARGE_INTEGER start;
    static int initialized = 0;

    if (!initialized) {
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&start);
        initialized = 1;
    }

    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);

    return (double)(now.QuadPart - start.QuadPart) / (double)frequency.QuadPart;
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

Mat4 perspective2(float fovy, float aspect, float znear, float zfar){
    float f = 1 / tan(fovy / 2);
    Mat4 out = (Mat4){
        (Vec4){1 / (aspect * tan(fovy / 2)), 0, 0, 0},
        (Vec4){0, f, 0, 0},
        (Vec4){0, 0, (zfar + znear)/(znear - zfar), (2 * zfar * znear)/(znear - zfar)},
        (Vec4){0, 0, 1, 0}
    };
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

void printVec2(Vec2 v) {
    printf("Vec2(x: %f, y: %f)\n", v.x, v.y);
}

void printVec3(Vec3 v) {
    printf("Vec3(x: %f, y: %f, z: %f)\n", v.x, v.y, v.z);
}

void printVec4(Vec4 v) {
    printf("Vec4(x: %f, y: %f, z: %f, w: %f)\n", v.x, v.y, v.z, v.w);
}
void printVertex(Vertex v, int layoutSize) {
    printf("Vertex: ");
    for (int i = 0; i < layoutSize; i++) {
        printf("%f ", v.data[i]);
    }
    printf("\n");
}
void printVertexShaderOutput(VertexShaderOutput out) {
    printf("\n--- Vertex Shader Output ---\n");
    for (int i = 0; i < out.vertexAmount; i++) {
        printf("Vertex %d: ", i);
        for (int j = 0; j < out.vertices->size; j++) {
            printf("%f ", out.vertices[i].data[j]);
        }
        printf("\n");
    }

    printf("Indices: ");
    for (int i = 0; i < out.indicesSize; i++) {
        printf("%d ", out.indices[i]);
    }
    printf("\nLayout: ");
    for (int i = 0; i < out.vertices->size; i++) {
        printf("%d ", out.layout[i]);
    }
    printf("\n");
}

void printPrimitive(Primitive p, int layoutSize) {
    printf("Primitive:\n");
    printf("  Vertex 1: ");
    for (int i = 0; i < layoutSize; i++) printf("%f ", p.v1.data[i]);
    printf("\n  Vertex 2: ");
    for (int i = 0; i < layoutSize; i++) printf("%f ", p.v2.data[i]);
    printf("\n  Vertex 3: ");
    for (int i = 0; i < layoutSize; i++) printf("%f ", p.v3.data[i]);
    printf("\n");
}

void printPrimitiveOutput(PrimitiveOutput out) {
    printf("\n--- Perspective Divide Output ---\n");
    for (int i = 0; i < out.primitiveAmount; i++) {
        printf("Primitive %d:\n", i);
        printPrimitive(out.primitives[i], out.primitives->v1.size);
    }
}

void printPrimitiveOutput2(ClippingOutput out) {
    printf("\n--- Primitive Clipping Output ---\n");
    for (int i = 0; i < out.primitiveAmount; i++) {
        printf("Primitive %d:\n", i);
        printPrimitive(out.primitives[i], out.primitives->v1.size);
    }
}

void printPrimitiveOutput3(PrimitiveAssemblyOutput out) {
    printf("\n--- Primitive Assembly Output ---\n");
    for (int i = 0; i < out.primitiveAmount; i++) {
        printf("Primitive %d:\n", i);
        printPrimitive(out.primitives[i], out.primitives->v1.size);
    }
}

void printTriangleTraversalOutput(TriangleTraversalOutput out) {
    printf("\n--- Triangle Traversal Output (Fragments) ---\n");
    for (int i = 0; i < 100; i++) {
        printf("Fragment %d: ", i);
        printVec2(out.fragments[i].position);
        printf(" z: %f ", out.fragments[i].zval);
        printVec4(out.fragments[i].color);
    }
}
void handleMouse(long x, long y){

    mouseDeltaPos.x = x - mouseLastPos.x;
    mouseDeltaPos.y = y - mouseLastPos.y;
    mouseLastPos.x = x;
    mouseLastPos.y = y;

    mouseDeltaPos.x *= mouseSensitivity;
    mouseDeltaPos.y *= mouseSensitivity;

    yaw += mouseDeltaPos.x * 15.0 * deltaTime;
    pitch += mouseDeltaPos.y * 15.0 * deltaTime;

    if(pitch > 89.0){
        pitch = 89.0;
    }
    if(pitch < -89.0){
        pitch = -89.0;
    }




}

Vec3 normalize(Vec3 v){
    float length = sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
    return (Vec3){v.x/length, v.y/length, v.z/length};
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

Vec3 cross(Vec3 a, Vec3 b) {
    Vec3 result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;
    return result;
}

Vec3 plus3(Vec3 front, Vec3 back){
    Vec3 result;
    result.x = front.x + back.x;
    result.y = front.y + back.y;
    result.z = front.z + back.z;
    return result;
}

Vec3* load_png_as_vec3(const char* filename, int* out_width, int* out_height) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 3);

    if (!data) {
        printf("Failed to load image: %s\n", stbi_failure_reason());
        return NULL;
    }

    Vec3* pixels = malloc(sizeof(Vec3) * width * height);
    if (!pixels) {
        stbi_image_free(data);
        return NULL;
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int i = y * width + x;
            pixels[i].x = data[i * 3 + 0];
            pixels[i].y = data[i * 3 + 1];
            pixels[i].z = data[i * 3 + 2];
        }
    }

    stbi_image_free(data);

    if (out_width) *out_width = width;
    if (out_height) *out_height = height;

    return pixels;
}

bool isTopLeft(Vec2 a, Vec2 b) {
    return (a.y == b.y && a.x > b.x) || (a.y > b.y);
}

Texture createTexture(char* filename){
    Texture out;
    out.pixels = load_png_as_vec3(filename, &out.width, &out.height);
    return out;
}

Vec3 getTexturePixel(Texture* pTex, float tx, float ty){
    int ix = (int)(pTex->width  * tx);
    int iy = (int)(pTex->height * ty);
    if (ix >= pTex->width)  ix = pTex->width  - 1;
    if (iy >= pTex->height) iy = pTex->height - 1;
    if (ix < 0) {
        ix = 0;
    } 
    if (iy < 0){
        iy = 0;
    }
    return pTex->pixels[iy * pTex->width + ix];
}

MeshData loadOBJ(const char *filename) {
    Vec3 *positions = NULL;
    Vec3 *normals = NULL;
    Vec2 *texcoords = NULL;

    int posCount = 0, normCount = 0, texCount = 0;

    int *faceIndices = NULL;
    int faceIndexCount = 0;

    MeshData mesh = {0};

    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Could not open %s\n", filename);
        return mesh;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "v ", 2) == 0) {
            Vec3 p;
            sscanf(line + 2, "%f %f %f", &p.x, &p.y, &p.z);
            positions = realloc(positions, sizeof(Vec3) * (posCount + 1));
            positions[posCount++] = p;
        }
        else if (strncmp(line, "vt ", 3) == 0) {
            Vec2 t;
            sscanf(line + 3, "%f %f", &t.x, &t.y);
            texcoords = realloc(texcoords, sizeof(Vec2) * (texCount + 1));
            texcoords[texCount++] = t;
        }
        else if (strncmp(line, "vn ", 3) == 0) {
            Vec3 n;
            sscanf(line + 3, "%f %f %f", &n.x, &n.y, &n.z);
            normals = realloc(normals, sizeof(Vec3) * (normCount + 1));
            normals[normCount++] = n;
        }
        else if (strncmp(line, "f ", 2) == 0) {
            int v[4][3]; // pos/tex/norm indices
            int count = sscanf(line + 2, "%d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
                               &v[0][0], &v[0][1], &v[0][2],
                               &v[1][0], &v[1][1], &v[1][2],
                               &v[2][0], &v[2][1], &v[2][2],
                               &v[3][0], &v[3][1], &v[3][2]);

            int vertsInFace = (count == 9) ? 3 : 4;

            // Triangulate if needed
            if (vertsInFace == 3) {
                for (int i = 0; i < 3; i++) {
                    faceIndices = realloc(faceIndices, sizeof(int) * (faceIndexCount + 3));
                    faceIndices[faceIndexCount++] = v[i][0] - 1; // pos index
                    faceIndices[faceIndexCount++] = v[i][1] - 1; // tex index
                    faceIndices[faceIndexCount++] = v[i][2] - 1; // norm index
                }
            } else if (vertsInFace == 4) {
                int idxOrder[6] = {0, 1, 2, 0, 2, 3};
                for (int k = 0; k < 6; k++) {
                    int i = idxOrder[k];
                    faceIndices = realloc(faceIndices, sizeof(int) * (faceIndexCount + 3));
                    faceIndices[faceIndexCount++] = v[i][0] - 1;
                    faceIndices[faceIndexCount++] = v[i][1] - 1;
                    faceIndices[faceIndexCount++] = v[i][2] - 1;
                }
            }
        }
    }
    fclose(file);

    // Build final vertex buffer
    int vertexSize = 3 + 3 + 2 + 3; // pos, color, tex, normal
    mesh.vertexCount = (faceIndexCount / 3) * vertexSize;
    mesh.vertices = malloc(sizeof(float) * mesh.vertexCount);
    mesh.indexCount = faceIndexCount / 3;
    mesh.indices = malloc(sizeof(int) * mesh.indexCount);

    for (int i = 0; i < mesh.indexCount; i++) {
        int pi = faceIndices[i * 3 + 0];
        int ti = faceIndices[i * 3 + 1];
        int ni = faceIndices[i * 3 + 2];

        float *vert = &mesh.vertices[i * vertexSize];
        vert[0] = positions[pi].x;
        vert[1] = positions[pi].y;
        vert[2] = positions[pi].z;

        vert[3] = 0.5f; // color R
        vert[4] = 0.5f; // color G
        vert[5] = 0.5f; // color B

        if (ti >= 0 && ti < texCount) {
            vert[6] = texcoords[ti].x;
            vert[7] = texcoords[ti].y;
        } else {
            vert[6] = vert[7] = 0.0f;
        }

        if (ni >= 0 && ni < normCount) {
            vert[8]  = normals[ni].x;
            vert[9]  = normals[ni].y;
            vert[10] = normals[ni].z;
        } else {
            vert[8] = vert[9] = vert[10] = 0.0f;
        }

        mesh.indices[i] = i;
    }

    free(positions);
    free(normals);
    free(texcoords);
    free(faceIndices);
    return mesh;
}
