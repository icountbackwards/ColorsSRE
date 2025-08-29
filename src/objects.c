#include "objects.h"

Vec3* load_png_as_vec3(const char* filename, int* out_width, int* out_height) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 3);

    if (!data) {
        printf("Failed to load image: %s\n", stbi_failure_reason());
        return NULL;
    }

    Vec3* pixels = malloc(sizeof(Vec3) * width * height);
    if (!pixels) {
        stbi_image_free(data);
        return NULL;
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int i = y * width + x;
            pixels[i].x = data[i * 3 + 0];
            pixels[i].y = data[i * 3 + 1];
            pixels[i].z = data[i * 3 + 2];
        }
    }

    stbi_image_free(data);

    if (out_width) *out_width = width;
    if (out_height) *out_height = height;

    return pixels;
}



Texture createTexture(char* filename){
    Texture out;
    out.pixels = load_png_as_vec3(filename, &out.width, &out.height);
    if(!out.pixels){
        out.width = -1;
    }
    return out;
}

Vec3 getTexturePixel(Texture* pTex, float tx, float ty){
    int ix = (int)(pTex->width  * tx);
    int iy = (int)(pTex->height * ty);
    if (ix >= pTex->width)  ix = pTex->width  - 1;
    if (iy >= pTex->height) iy = pTex->height - 1;
    if (ix < 0) {
        ix = 0;
    } 
    if (iy < 0){
        iy = 0;
    }
    return pTex->pixels[iy * pTex->width + ix];
}

MeshData loadOBJ(const char *filename) {
    Vec3 *positions = NULL;
    Vec3 *normals = NULL;
    Vec2 *texcoords = NULL;

    int posCount = 0, normCount = 0, texCount = 0;

    int *faceIndices = NULL;
    int faceIndexCount = 0;

    MeshData mesh = {0};
    mesh.indexCount = -1;

    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Could not open %s\n", filename);
        return mesh;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "v ", 2) == 0) {
            Vec3 p;
            sscanf(line + 2, "%f %f %f", &p.x, &p.y, &p.z);
            positions = realloc(positions, sizeof(Vec3) * (posCount + 1));
            positions[posCount++] = p;
        }
        else if (strncmp(line, "vt ", 3) == 0) {
            Vec2 t;
            sscanf(line + 3, "%f %f", &t.x, &t.y);
            texcoords = realloc(texcoords, sizeof(Vec2) * (texCount + 1));
            texcoords[texCount++] = t;
        }
        else if (strncmp(line, "vn ", 3) == 0) {
            Vec3 n;
            sscanf(line + 3, "%f %f %f", &n.x, &n.y, &n.z);
            normals = realloc(normals, sizeof(Vec3) * (normCount + 1));
            normals[normCount++] = n;
        }
        else if (strncmp(line, "f ", 2) == 0) {
            int v[4][3]; // pos/tex/norm indices
            int count = sscanf(line + 2, "%d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
                               &v[0][0], &v[0][1], &v[0][2],
                               &v[1][0], &v[1][1], &v[1][2],
                               &v[2][0], &v[2][1], &v[2][2],
                               &v[3][0], &v[3][1], &v[3][2]);

            int vertsInFace = (count == 9) ? 3 : 4;

            // Triangulate if needed
            if (vertsInFace == 3) {
                for (int i = 0; i < 3; i++) {
                    faceIndices = realloc(faceIndices, sizeof(int) * (faceIndexCount + 3));
                    faceIndices[faceIndexCount++] = v[i][0] - 1; // pos index
                    faceIndices[faceIndexCount++] = v[i][1] - 1; // tex index
                    faceIndices[faceIndexCount++] = v[i][2] - 1; // norm index
                }
            } else if (vertsInFace == 4) {
                int idxOrder[6] = {0, 1, 2, 0, 2, 3};
                for (int k = 0; k < 6; k++) {
                    int i = idxOrder[k];
                    faceIndices = realloc(faceIndices, sizeof(int) * (faceIndexCount + 3));
                    faceIndices[faceIndexCount++] = v[i][0] - 1;
                    faceIndices[faceIndexCount++] = v[i][1] - 1;
                    faceIndices[faceIndexCount++] = v[i][2] - 1;
                }
            }
        }
    }
    fclose(file);

    // Build final vertex buffer
    int vertexSize = 3 + 3 + 2 + 3; // pos, color, tex, normal
    mesh.vertexCount = (faceIndexCount / 3) * vertexSize;
    mesh.vertices = malloc(sizeof(float) * mesh.vertexCount);
    mesh.indexCount = faceIndexCount / 3;
    mesh.indices = malloc(sizeof(int) * mesh.indexCount);

    for (int i = 0; i < mesh.indexCount; i++) {
        int pi = faceIndices[i * 3 + 0];
        int ti = faceIndices[i * 3 + 1];
        int ni = faceIndices[i * 3 + 2];

        float *vert = &mesh.vertices[i * vertexSize];
        vert[0] = positions[pi].x;
        vert[1] = positions[pi].y;
        vert[2] = positions[pi].z;

        vert[3] = 0.5f; // color R
        vert[4] = 0.5f; // color G
        vert[5] = 0.5f; // color B

        if (ti >= 0 && ti < texCount) {
            vert[6] = texcoords[ti].x;
            vert[7] = texcoords[ti].y;
        } else {
            vert[6] = vert[7] = 0.0f;
        }

        if (ni >= 0 && ni < normCount) {
            vert[8]  = normals[ni].x;
            vert[9]  = normals[ni].y;
            vert[10] = normals[ni].z;
        } else {
            vert[8] = vert[9] = vert[10] = 0.0f;
        }

        mesh.indices[i] = i;
    }

    free(positions);
    free(normals);
    free(texcoords);
    free(faceIndices);
    return mesh;
}