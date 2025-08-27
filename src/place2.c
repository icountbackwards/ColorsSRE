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
#include "window.h"

#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL.h>

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

double sdlGetTime();

Instance instance;
WindowObject windowObject;

int main(int argc, char *argv[]){

    //static WNDCLASS window_class = { 0 };
    //static const wchar_t window_class_name[] = L"WindowClass";
    //window_class.lpszClassName = (PCSTR)window_class_name;
    //window_class.lpfnWndProc = WindowProc;
    //window_class.hInstance = hInstance;
    //RegisterClass(&window_class);
    //frame_bitmap_info.bmiHeader.biSize = sizeof(frame_bitmap_info.bmiHeader);
    //frame_bitmap_info.bmiHeader.biPlanes = 1;
    //frame_bitmap_info.bmiHeader.biBitCount = 32;
    //frame_bitmap_info.bmiHeader.biCompression = BI_RGB;
    //frame_device_context = CreateCompatibleDC(0);

    createInstance(&instance, SCREEN_WIDTH, SCREEN_HEIGHT);
    instance.lastTime = sdlGetTime();
    instance.depthBuffer = malloc(sizeof(float) * (instance.frameWidth+1) * (instance.frameHeight+1));
    initWindowObject(&windowObject, SCREEN_WIDTH, SCREEN_HEIGHT, &instance);

    //HWND hwnd = CreateWindow((PCSTR)window_class_name, "ColorsSRE", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, instance.frameWidth + 16 + 1, instance.frameHeight + 39 + 1, NULL, NULL, hInstance, NULL);

    //ShowWindow(hwnd, nCmdShow);

    int frameCount = 0;

    VertexBuffer vbo1 = generateVertexBuffer(vbo1data, vbo1indices, vbo1layout, vbo1size, vbo1indicesSize, vbo1layoutsize);
    VertexBuffer lightvbo = generateVertexBuffer(lightdata, lightindices, lightlayout, lightsize, lightindicessize, lightlayoutsize);
    
    UniformBuffer ubo1;

    Texture texture1 = createTexture("../assets/gold.png");

    suzanne_flat = loadOBJ("../assets/suzanne_smooth.obj");
    suzanne_flat_vbo = generateVertexBuffer(suzanne_flat.vertices, suzanne_flat.indices, vbo1layout, suzanne_flat.vertexCount, suzanne_flat.indexCount, vbo1layoutsize);

    while(instance.isRunning){
        //static MSG msg = { 0 };
        //while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
        //    TranslateMessage(&msg);
        //    DispatchMessage(&msg);
        //}
        processEvent(&windowObject, &instance);

        SDL_GetMouseState(&instance.mousepos.x, &instance.mousepos.y); 
        //GetCursorPos(&instance.mousepos);
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

        double time = sdlGetTime();
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

        //InvalidateRect(hwnd, NULL, FALSE);
        //UpdateWindow(hwnd);
        presentScreen(&windowObject, &instance);
    }

    free(instance.depthBuffer);
    destroyWindow(&windowObject, &instance);

    return 0;
}

double sdlGetTime() {
    static Uint64 start = 0;
    static Uint64 frequency = 0;

    if (start == 0) {
        start = SDL_GetPerformanceCounter();
        frequency = SDL_GetPerformanceFrequency();
    }

    Uint64 now = SDL_GetPerformanceCounter();
    return (double)(now - start) / (double)frequency;
}





