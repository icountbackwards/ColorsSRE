#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL3/SDL.h>
#include "instance.h"

struct WindowObject {
    //This struct stores the variables related to the screen display, as well as SDL handles and the framebuffer array
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* framebuffer;
    uint32_t* pixels;
    uint32_t width;
    uint32_t height;
    uint32_t framebufferSize; // Size of uint32_t* pixels => width * height
    bool isRunning; // the application render loop quits if this is false
};

typedef struct WindowObject WindowObject;

void initWindowObject(WindowObject* windowObject, uint32_t width, uint32_t height, Instance* instance); // Creates definition for the WindowObject struct
void processEvent(WindowObject* windowObject, Instance* instance); // Application event and input handling
void presentScreen(WindowObject* windowObject, Instance* instance); // Presenting the rendered framebuffer onto the screen
void destroyWindow(WindowObject* windowObject, Instance* instance); // Cleanup and destroy after the application quits