#include <Windows.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>


#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION

#include "data.c"

#include "math_utils.h"
#include "pipeline/pipeline.h"
#include "instance.h"
#include "objects.h"

static boolean quit = FALSE;

struct {
    int width;
    int height;
    uint32_t *pixels;
}frame = { 0 };

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

Vec3 cameraPos = {0.0, 0.0, 5.0};
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

void clearDepthBuffer();
void clearFrameBuffer();
double getTime();

void writepixel(int x, int y, uint32_t col);
void handleMouse(long x, long y);

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
            cameraPos = (Vec3){0.0, 0.0, 5.0};
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
    
    runScreenMapping(&perspectiveDivideOutput, frame.width, frame.height);
    
    TriangleTraversalOutput triangleTraversalOutput = runRasterizer(&perspectiveDivideOutput, pipelineVariation, frame.width, frame.height);
    
    FragmentShaderOutput fragmentShaderOutput = runFragmentShader(&triangleTraversalOutput, pTextureResource, ubo, pipelineVariation);

    runMerger(&fragmentShaderOutput, pTextureResource, frame.width, frame.height, depthBuffer, frame.pixels);

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

void updateUniformBuffer(){

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


