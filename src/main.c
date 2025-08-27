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

int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 600;

static BITMAPINFO frame_bitmap_info;
static HBITMAP frame_bitmap = 0;
static HDC frame_device_context = 0;

UniformBuffer *UniformBufferRegister;
Texture notexture;

MeshData suzanne_flat;
VertexBuffer suzanne_flat_vbo;

Mat4 identityMat4 = {
    {1, 0, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 1}
};

bool debug = true;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
double getTime();

Instance instance;

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

    createInstance(&instance, SCREEN_WIDTH, SCREEN_HEIGHT);
    instance.lastTime = getTime();
    instance.depthBuffer = malloc(sizeof(float) * (instance.frameWidth+1) * (instance.frameHeight+1));

    HWND hwnd = CreateWindow((PCSTR)window_class_name, "ColorsSRE", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, instance.frameWidth + 16 + 1, instance.frameHeight + 39 + 1, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);

    int frameCount = 0;

    VertexBuffer vbo1 = generateVertexBuffer(vbo1data, vbo1indices, vbo1layout, vbo1size, vbo1indicesSize, vbo1layoutsize);
    VertexBuffer lightvbo = generateVertexBuffer(lightdata, lightindices, lightlayout, lightsize, lightindicessize, lightlayoutsize);
    
    UniformBuffer ubo1;

    Texture texture1 = createTexture("../assets/gold.png");

    suzanne_flat = loadOBJ("../assets/suzanne_smooth.obj");
    suzanne_flat_vbo = generateVertexBuffer(suzanne_flat.vertices, suzanne_flat.indices, vbo1layout, suzanne_flat.vertexCount, suzanne_flat.indexCount, vbo1layoutsize);

    while(instance.isRunning){
        static MSG msg = { 0 };
        while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        GetCursorPos(&instance.mousepos);
        if(!instance.mouseFreeze && !instance.mouseDeltaFreeze){
            handleMouse(instance.mousepos.x, instance.mousepos.y, &instance);
        }
        if(instance.mouseDeltaFreeze){
            instance.mouseDeltaFreeze = false;
            instance.mouseDeltaPos.x = 0;
            instance.mouseDeltaPos.y = 0;
            instance.mouseLastPos = instance.mousepos;
        }

        //

        //static unsigned int p = 0;
        //frame.pixels[(p++)%(frame.width*frame.height)] = rand();
        //frame.pixels[rand()%(frame.width*frame.height)] = 0X00ff0000;

        //

        float time = getTime();
        float currentFrame = time;
        instance.deltaTime = currentFrame - instance.lastFrame;
        instance.lastFrame = currentFrame;

        frameCount++;
        double currentTime = time;
        double elapsed = currentTime - instance.lastTime;
        if (elapsed >= 1.0) { // Every 1 second
            printf("FPS: %d\n", frameCount);
            frameCount = 0;
            instance.lastTime = currentTime;
        }

        instance.cameraSpeed = 5.0 * instance.deltaTime;
        Vec3 direction;
        direction.z = cos((instance.yaw * TWO_PI) / 180) * cos ((instance.pitch * TWO_PI) / 180);
        direction.y = -sin((instance.pitch * TWO_PI) / 180);
        direction.x = sin((instance.yaw * TWO_PI) / 180) * cos((instance.pitch * TWO_PI) / 180);
        instance.cameraFront = normalize(direction);    

        if(instance.backtofront){
            instance.cameraFront = (Vec3){0.0, 0.0, -1.0};
            instance.cameraPos = (Vec3){0.0, 0.0, 5.0};
            instance.yaw = -90.0;
            instance.pitch = 0.0;
            instance.backtofront = false;
        }
        
        clearFrameBuffer(&instance);
        clearDepthBuffer(&instance);

        Mat4 model = identityMat4;
        Mat4 view = lookAt(instance.cameraPos, plus3(instance.cameraPos, instance.cameraFront), instance.cameraUp);
        Mat4 projection = perspective(45.0, ((float)instance.frameWidth)/((float)instance.frameHeight), 100.0, 0.1);
        rotate4(&model, instance.objrotate0, 0);
        rotate4(&model, instance.objrotate1, 1);
        rotate4(&model, instance.objrotate2, 2);
        //rotate4(&model, time, i % 2);
        ubo1.model = model;
        ubo1.view = view;
        ubo1.projection = projection;
        ubo1.objectColor = (Vec3){1.0, 1.0, 1.0};
        ubo1.lightColor = instance.lightColor;
        ubo1.lightPos = instance.lightPosition;
        ubo1.viewPos = instance.cameraPos;
        //draw(&vbo1, &ubo1, &texture1, PIPELINE_VARIATION_MESH);
        draw(&suzanne_flat_vbo, &ubo1, &texture1, PIPELINE_VARIATION_MESH, &instance);


        model = identityMat4;
        scale4(&model, (Vec3){0.2, 0.2, 0.2});
        translate4(&model, instance.lightPosition);
        ubo1.model = model;
        ubo1.view = view;
        ubo1.projection = projection;
        draw(&lightvbo, &ubo1, &notexture, PIPELINE_VARIATION_LIGHT_SOURCE, &instance);


        InvalidateRect(hwnd, NULL, FALSE);
        UpdateWindow(hwnd);
    }

    free(instance.depthBuffer);

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam){

    switch(msg){
        case WM_QUIT:
        case WM_DESTROY: {
            instance.isRunning = FALSE;
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
            frame_bitmap = CreateDIBSection(NULL, &frame_bitmap_info, DIB_RGB_COLORS, (void**)&instance.frameBuffer, 0, 0);
            SelectObject(frame_device_context, frame_bitmap);

            instance.frameWidth = LOWORD(lparam);
            instance.frameHeight = HIWORD(lparam);
        }break;

        case WM_KEYDOWN:
            switch (wparam) {
                
                case 'W':
                    instance.cameraPos = plus3(instance.cameraPos, scalarMultiply3(instance.cameraSpeed, instance.cameraFront));
                    break;
                case 'A':
                    instance.cameraPos = plus3(instance.cameraPos, scalarMultiply3(instance.cameraSpeed, cross(instance.cameraFront, instance.cameraUp)));
                    break;
                case 'S':
                    instance.cameraPos = minus3(instance.cameraPos, scalarMultiply3(instance.cameraSpeed, instance.cameraFront));
                    break;
                case 'D':
                    instance.cameraPos = minus3(instance.cameraPos, scalarMultiply3(instance.cameraSpeed, cross(instance.cameraFront, instance.cameraUp)));
                    break;
                case 'M':
                    instance.mouseFreeze = !instance.mouseFreeze;
                    instance.mouseDeltaFreeze = true;
                    break;
                case 'B':
                    instance.backtofront = true;
                    break;
                case VK_ESCAPE:
                    instance.isRunning = false;
                    break;
                case 'V':
                    instance.isVertical = !instance.isVertical;
                    break;
                case 'I':
                    if(instance.isVertical){
                        instance.lightPosition.y += instance.cameraSpeed;
                    } else{
                        instance.lightPosition.z -= instance.cameraSpeed;
                    }
                    break;
                case 'J':
                    //cameraPos = plus3(cameraPos, scalarMultiply3(cameraSpeed, cross(cameraFront, cameraUp)));
                    instance.lightPosition.x += instance.cameraSpeed;
                    break;
                case 'K':
                    //cameraPos = minus3(cameraPos, scalarMultiply3(cameraSpeed, cameraFront));
                    if(instance.isVertical){
                        instance.lightPosition.y -= instance.cameraSpeed;
                    } else{
                        instance.lightPosition.z += instance.cameraSpeed;
                    }
                    break;
                case 'L':
                    //cameraPos = minus3(cameraPos, scalarMultiply3(cameraSpeed, cross(cameraFront, cameraUp)));
                    instance.lightPosition.x -= instance.cameraSpeed;
                    break;
                case VK_UP: // same as W
                    instance.objrotate0 -= 0.1;
                    break;
                case VK_DOWN: // same as S
                    instance.objrotate0 += 0.1;
                    break;
                case VK_LEFT: // same as A
                    instance.objrotate1 += 0.1;
                    break;
                case VK_RIGHT: // same as D
                    instance.objrotate1 -= 0.1;
                    break;
            }
            break;
            

        default: {
            return DefWindowProc(hwnd, msg, wparam, lparam);
        } break;
    }
    return 0;
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





