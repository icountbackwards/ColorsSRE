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

    instance->lightColor = (Vec3){1.0, 1.0, 1.0};
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
    for (int i = 0; i < instance->frameHeight; i++) {
        for (int j = 0; j < instance->frameWidth; j++) {
            (*instance->pPixels)[i * instance->frameWidth + j] = 0xFF000000;
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

void handleKeypress(int key, Instance* instance){
    switch(key){
        case KEY_W:
            instance->cameraPos = plus3(instance->cameraPos, scalarMultiply3(instance->cameraSpeed, instance->cameraFront));
            break;
        case KEY_A:
            instance->cameraPos = plus3(instance->cameraPos, scalarMultiply3(instance->cameraSpeed, cross(instance->cameraFront, instance->cameraUp)));
            break;
        case KEY_S:
            instance->cameraPos = minus3(instance->cameraPos, scalarMultiply3(instance->cameraSpeed, instance->cameraFront));
            break;
        case KEY_D:
            instance->cameraPos = minus3(instance->cameraPos, scalarMultiply3(instance->cameraSpeed, cross(instance->cameraFront, instance->cameraUp)));
            break;
        case KEY_M:
            instance->mouseFreeze = !instance->mouseFreeze;
            instance->mouseDeltaFreeze = true;
            break;
        case KEY_B:
            instance->backtofront = true;
            break;
        case KEY_ESC:
            instance->isRunning = false;
            break;
        case KEY_V:
            instance->isVertical = !instance->isVertical;
            break;
        case KEY_I:
            if(instance->isVertical){
                instance->lightPosition.y += instance->cameraSpeed;
            } else{
                instance->lightPosition.z -= instance->cameraSpeed;
            }
            break;
        case KEY_J:
            //cameraPos = plus3(cameraPos, scalarMultiply3(cameraSpeed, cross(cameraFront, cameraUp)));
            instance->lightPosition.x += instance->cameraSpeed;
            break;
        case KEY_K:
            //cameraPos = minus3(cameraPos, scalarMultiply3(cameraSpeed, cameraFront));
            if(instance->isVertical){
                instance->lightPosition.y -= instance->cameraSpeed;
            } else{
                instance->lightPosition.z += instance->cameraSpeed;
            }
            break;
        case KEY_L:
            //cameraPos = minus3(cameraPos, scalarMultiply3(cameraSpeed, cross(cameraFront, cameraUp)));
            instance->lightPosition.x -= instance->cameraSpeed;
            break;
        case KEY_UP: // same as W
            instance->objrotate0 -= 0.1;
            break;
        case KEY_DOWN: // same as S
            instance->objrotate0 += 0.1;
            break;
        case KEY_LEFT: // same as A
            instance->objrotate1 += 0.1;
            break;
        case KEY_RIGHT: // same as D
            instance->objrotate1 -= 0.1;
            break;
    }
}