#include "instance.h"

void createInstance(Instance* instance, int width, int height){
    instance->frameWidth = width;
    instance->frameHeight = height;
    instance->isRunning = true;

    instance->lastTime = 0;

    instance->mouseFreeze = false;
    instance->mouseDeltaFreeze = false;
    instance->mousepos;
    instance->mouseDeltaPos;
    instance->mouseLastPos;
    instance->yaw = -90.0;
    instance->pitch = 0.0;

    instance->objrotate0 = 0;
    instance->objrotate1 = 0;
    instance->objrotate2 = 0;

    instance->mouseSensitivity = 0.5;

    instance->cameraPos = (Vec3){0.0, 0.0, 5.0};
    instance->cameraFront = (Vec3){0.0, 0.0, -1.0};
    instance->cameraUp = (Vec3){0.0, 1.0, 0.0};

    instance->lightPosition = (Vec3){-1.2, 1.5, 0.5};
    instance->isVertical = false;

    instance-> cameraSpeed = 0.7;

    instance->backtofront = false;

    instance->deltaTime = 0.0;
    instance->lastFrame = 0.0;
}

void clearDepthBuffer(Instance* instance) {
    for (int i = 0; i < instance->frameHeight+1; i++) {
        for (int j = 0; j < instance->frameWidth+1; j++) {
            //REVERSE Z IMP
            instance->depthBuffer[i * instance->frameWidth + j] = 0.0f;
            //depthBuffer[i * frame.width + j] = 100.0f;
        }
    }
}

void clearFrameBuffer(Instance* instance) {
    for (int i = 0; i < instance->frameHeight+1; i++) {
        for (int j = 0; j < instance->frameWidth+1; j++) {
            instance->frameBuffer[i * instance->frameWidth + j] = 0x00000000;
        }
    }
}

void handleMouse(long x, long y, Instance* instance){

    instance->mouseDeltaPos.x = x - instance->mouseLastPos.x;
    instance->mouseDeltaPos.y = y - instance->mouseLastPos.y;
    instance->mouseLastPos.x = x;
    instance->mouseLastPos.y = y;
    instance->mouseDeltaPos.x *= instance->mouseSensitivity;
    instance->mouseDeltaPos.y *= instance->mouseSensitivity;
    instance->yaw += instance->mouseDeltaPos.x * 15.0 * instance->deltaTime;
    instance->pitch += instance->mouseDeltaPos.y * 15.0 * instance->deltaTime;

    if(instance->pitch > 89.0){
        instance->pitch = 89.0;
    }
    if(instance->pitch < -89.0){
        instance->pitch = -89.0;
    }

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