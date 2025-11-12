#include "window.h"

void initWindowObject(WindowObject* windowObject, uint32_t width, uint32_t height, Instance* instance) {
     // This function creates definition for windowObject
    windowObject->width = width;
    windowObject->height = height;

    if (SDL_Init(SDL_INIT_VIDEO)) {
        
    }else{
        printf("SDL_Init Error: %s\n", SDL_GetError());
        exit(1);
    }

    windowObject->window = SDL_CreateWindow("ColorsSRE", width, height, 0);

    if (!windowObject->window) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }

    windowObject->renderer = SDL_CreateRenderer(windowObject->window, NULL);
    if (!windowObject->renderer) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(windowObject->window);
        SDL_Quit();
        exit(1);
    }

    windowObject->framebuffer = SDL_CreateTexture(windowObject->renderer,
    SDL_PIXELFORMAT_ARGB8888,
    SDL_TEXTUREACCESS_STREAMING,
    width, height);

    if (!windowObject->framebuffer) {
        printf("SDL_CreateTexture Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(windowObject->renderer);
        SDL_DestroyWindow(windowObject->window);
        SDL_Quit();
        exit(1);
    }

    windowObject->pixels = (uint32_t*)malloc(sizeof(uint32_t) * width * height);
    if (!windowObject->pixels) {
        printf("Failed to allocate framebuffer memory\n");
        SDL_DestroyTexture(windowObject->framebuffer);
        SDL_DestroyRenderer(windowObject->renderer);
        SDL_DestroyWindow(windowObject->window);
        SDL_Quit();
        exit(1);
    }
    for(int i = 0; i < width * height; i++){
        windowObject->pixels[i] = 0xFF000000;
    }

    windowObject->isRunning = true;
    windowObject->framebufferSize = width * height;

}

void processEvent(WindowObject* windowObject, Instance* instance){
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
    switch (event.type) {
        case SDL_EVENT_QUIT:
            instance->isRunning = false;
            break;

        case SDL_EVENT_KEY_DOWN:
            switch (event.key.key) {
                case SDLK_W:
                    //instance->cameraPos = plus3(instance->cameraPos,scalarMultiply3(instance->cameraSpeed, instance->cameraFront));
                    break;
                case SDLK_A:
                    //instance->cameraPos = plus3(instance->cameraPos,scalarMultiply3(instance->cameraSpeed, cross(instance->cameraFront, instance->cameraUp)));
                    break;
                case SDLK_S:
                    //instance->cameraPos = minus3(instance->cameraPos,scalarMultiply3(instance->cameraSpeed, instance->cameraFront));
                    break;
                case SDLK_D:
                    //instance->cameraPos = minus3(instance->cameraPos,scalarMultiply3(instance->cameraSpeed, cross(instance->cameraFront, instance->cameraUp)));
                    break;
                case SDLK_M:
                    //instance->mouseFreeze = !instance->mouseFreeze;
                    //instance->mouseDeltaFreeze = true;
                    break;
                case SDLK_B:
                    //instance->backtofront = true;
                    break;
                case SDLK_ESCAPE:
                    instance->isRunning = false;
                    break;
                case SDLK_V:
                    //instance->isVertical = !instance->isVertical;
                    break;
                case SDLK_I:
                    instance->lightPosition.y += instance->cameraSpeed;
                    break;
                case SDLK_J:
                    instance->lightPosition.x += instance->cameraSpeed;
                    break;
                case SDLK_K:
                    instance->lightPosition.y -= instance->cameraSpeed;
                    break;
                case SDLK_L:
                    instance->lightPosition.x -= instance->cameraSpeed;
                    break;
                case SDLK_U:
                    instance->lightPosition.z -= instance->cameraSpeed;
                    break;
                case SDLK_H:
                    instance->lightPosition.z += instance->cameraSpeed;
                    break;
                case SDLK_UP:
                    instance->objrotate0 -= 0.1;
                    break;
                case SDLK_DOWN:
                    instance->objrotate0 += 0.1;
                    break;
                case SDLK_LEFT:
                    instance->objrotate1 += 0.1;
                    break;
                case SDLK_RIGHT:
                    instance->objrotate1 -= 0.1;
                    break;
            }
            break;
    }
}
}

void presentScreen(WindowObject* windowObject, Instance* instance) {
    SDL_UpdateTexture(windowObject->framebuffer, NULL, windowObject->pixels,
                      windowObject->width * sizeof(uint32_t));

    SDL_RenderClear(windowObject->renderer);

    SDL_FRect dst = {0, 0, windowObject->width, windowObject->height};
    SDL_RenderTextureRotated(
        windowObject->renderer,
        windowObject->framebuffer,
        NULL,   // source rectangle (NULL = full texture)
        &dst,   // destination rectangle
        0.0,    // rotation
        NULL,   // rotation center
        SDL_FLIP_VERTICAL // flip vertically
    );
    

    SDL_RenderPresent(windowObject->renderer);
}

void destroyWindow(WindowObject* windowObject, Instance* instance){
    // Cleanup and destroy after exiting the application render loop
    SDL_DestroyRenderer(windowObject->renderer);
    SDL_DestroyWindow(windowObject->window);
    SDL_Quit();

    free(instance->frameBuffer);
    free(windowObject->pixels);
}