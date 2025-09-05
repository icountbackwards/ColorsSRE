
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

int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 600;

UniformBuffer *UniformBufferRegister;
Texture notexture;

Mat4 identityMat4 = {
    {1, 0, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 1}
};

bool debug = true;

double getTime();
void handleApplicationArguments(int argc, char* argv[]);

void loadDefaultMeshObject();
void loadDefaultTextureImage();
void loadMeshObject(char* argv);

Instance instance;
WindowObject windowObject;

int main(int argc, char *argv[]){
    
    createInstance(&instance, SCREEN_WIDTH, SCREEN_HEIGHT);
    instance.lastTime = getTime();
    instance.depthBuffer = malloc(sizeof(float) * (instance.frameWidth+1) * (instance.frameHeight+1));

    initWindowObject(&windowObject, SCREEN_WIDTH, SCREEN_HEIGHT, &instance);
    instance.pPixels = &windowObject.pixels;

    int frameCount = 0; 

    instance.lightvbo = generateVertexBuffer(lightdata, lightindices, lightlayout, lightsize, lightindicessize, lightlayoutsize);
    
    UniformBuffer ubo1;

    handleApplicationArguments(argc, argv);

    Texture texture1 = createTexture("../assets/gold.png");
    

    while(instance.isRunning){
        processEvent(&windowObject, &instance);

        SDL_GetMouseState(&instance.mousepos.x, &instance.mousepos.y);
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
        draw(&instance.meshvbo, &ubo1, &instance.texture, PIPELINE_VARIATION_MESH, &instance);


        model = identityMat4;
        scale4(&model, (Vec3){0.2, 0.2, 0.2});
        translate4(&model, instance.lightPosition);
        ubo1.model = model;
        ubo1.view = view;
        ubo1.projection = projection;
        draw(&instance.lightvbo, &ubo1, &notexture, PIPELINE_VARIATION_LIGHT_SOURCE, &instance);

        presentScreen(&windowObject, &instance);
    }

    free(instance.depthBuffer);
    destroyWindow(&windowObject, &instance);

    return 0;
}

double getTime() {
    static uint64_t start = 0;
    if (start == 0) {
        start = SDL_GetPerformanceCounter();
    }

    uint64_t now = SDL_GetPerformanceCounter();
    uint64_t freq = SDL_GetPerformanceFrequency();

    return (double)(now - start) / (double)freq;
}

void handleApplicationArguments(int argc, char* argv[]){
    /*
        This function handles the command line arguments passed when running the application
        Argument format: 
            1. Render default mesh with default texture (No arguments needed) => ColorsSRE.exe
            2. Render custom mesh with default texture => ColorsSRE.exe MESH_LOCATION_DIRECTORY/Mesh.obj default_texture
            3. Render default mesh with custom texture => ColorsSRE.exe default_mesh TEXTURE_LOCATION_DIRECTORY/TextureImage.png
            4. Render custom mesh with custom texture => ColorsSRE.exe MESH_LOCATION_DIRECTORY/Mesh.obj TEXTURE_LOCATION_DIRECTORY/TextureImage.png\
            5. ColorsSRE.exe default_mesh default_texture also works
            6. Above second and third arguments are ignored
        s
        Note: If the argument(s) passed is invalid, default asset will be used instead
    */

    if (argc == 1) {
        //Format 1
        printf("No parameters were provided, using default assets\n");
        loadDefaultMeshObject();
        loadDefaultTextureImage();
    } else {
        if(strcmp(argv[1], "default_mesh") == 0){
            printf("using default mesh\n");
            loadDefaultMeshObject();
        }else{
            loadMeshObject(argv[1]);
            if(instance.meshvbo.indicesSize == -1){
                loadDefaultMeshObject();
            }
        }
        if(strcmp(argv[2], "default_texture") == 0){
            printf("using default texture\n");
            loadDefaultTextureImage();
        }else{
            instance.texture = createTexture(argv[2]);
            if(instance.texture.width == -1){
                printf("Using default texture image instead\n");
                loadDefaultTextureImage();
            }
        }
    }

}

void loadDefaultMeshObject(){
    instance.meshvbo = loadObjFile("../assets/suzanne_smooth.obj");
    instance.meshvbo.layout = vbo1layout;
    instance.meshvbo.layoutSize = vbo1layoutsize;
}

void loadDefaultTextureImage(){
    instance.texture = createTexture("../assets/gold.png");
}

void loadMeshObject(char* argv){
    instance.meshvbo = loadObjFile(argv);
    instance.meshvbo.layout = vbo1layout;
    instance.meshvbo.layoutSize = vbo1layoutsize;
    if(instance.meshvbo.indicesSize == -1){
        loadDefaultMeshObject();
        printf("Using default mesh object instead\n");
    }
}



