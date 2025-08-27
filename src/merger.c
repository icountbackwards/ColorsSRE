#include "merger.h"

void runMerger(FragmentShaderOutput *in, Texture* texture, int width, int height, float* depthBuffer, uint32_t* frameBuffer) {
    for (int i = 0; i < in->fragmentSize; i++) {
        int x = (int)in->fragments[i].position.x;
        int y = (int)in->fragments[i].position.y;

        if (x < 0 || x >= width || y < 0 || y >= height) {
            fprintf(stderr, "⚠️ Fragment %d out of bounds: x = %d, y = %d\n", i, x, y);
            continue;
        }

        int index = y * width + x;

        //REVERSE Z IMP
        if (in->fragments[i].zval > depthBuffer[index]) {
            depthBuffer[index] = in->fragments[i].zval;
            
            uint8_t r = clamp(in->fragments[i].color.x, 0, 255) *255;
            uint8_t g = clamp(in->fragments[i].color.y, 0, 255) *255;
            uint8_t b = clamp(in->fragments[i].color.z, 0, 255) *255;
            uint8_t a = clamp(in->fragments[i].color.w, 0, 255) *255;
            

            //uint8_t r = (uint8_t)(tex.x * 255.0);
            //uint8_t g = (uint8_t)(tex.y * 255.0);
            //uint8_t b = (uint8_t)(tex.z * 255.0);
            //uint8_t a = clamp(in->fragments[i].color.w, 0, 255);

            uint32_t interp_color = (a << 24) | (r << 16) | (g << 8) | (b);

            frameBuffer[index] = interp_color;
        }
    }
}