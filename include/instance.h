#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "math_utils.h"
#include "objects.h"

#define KEY_W 0
#define KEY_A 1
#define KEY_S 2
#define KEY_D 3
#define KEY_I 4
#define KEY_J 5
#define KEY_K 6
#define KEY_L 7
#define KEY_UP 8
#define KEY_DOWN 9
#define KEY_LEFT 10
#define KEY_RIGHT 11
#define KEY_B 12
#define KEY_M 13
#define KEY_V 14
#define KEY_ESC 15



typedef struct {
    int frameWidth;
    int frameHeight;

    uint32_t* frameBuffer;
    float* depthBuffer;

    bool isRunning;

    double lastTime;

    bool mouseFreeze;
    bool mouseDeltaFreeze;
    Vec2 mousepos;
    Vec2 mouseDeltaPos;
    Vec2 mouseLastPos;
    float yaw;
    float pitch;
    float objrotate0;
    float objrotate1;
    float objrotate2;
    float mouseSensitivity;
    Vec3 cameraPos;
    Vec3 cameraFront;
    Vec3 cameraUp;
    Vec3 lightPosition;
    bool isVertical;
    float cameraSpeed;
    bool backtofront;
    float deltaTime;
    float lastFrame;
    UniformBuffer *UniformBufferRegister;
    

    Vec3 lightColor;

    uint32_t** pPixels;
    Texture texture;
    VertexBuffer meshvbo;
    VertexBuffer lightvbo;
} Instance;

void createInstance(Instance* instance, int width, int height);

void clearDepthBuffer(Instance* instance);
void clearFrameBuffer(Instance* instance);

void handleMouse(long x, long y, Instance* instance);
void handleKeypress(int key, Instance* instance);


