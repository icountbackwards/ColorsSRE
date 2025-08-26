#include "fragment_shader.h"

FragmentShaderOutput runFragmentShader(TriangleTraversalOutput *in, Texture* texture, UniformBuffer *ubo, int pipelineVariation) {
    FragmentShaderOutput out;

    switch(pipelineVariation){
        case PIPELINE_VARIATION_LIGHT_SOURCE:
            for(int i = 0; i < in->fragmentSize; i++){
                in->fragments[i].color.x = 255.0;
                in->fragments[i].color.y = 255.0;
                in->fragments[i].color.z = 255.0;

            }
            break;
        case PIPELINE_VARIATION_MESH:
            for(int i = 0; i < in->fragmentSize; i++){
                Vec3 texel = getTexturePixel(texture, in->fragments[i].data[7], in->fragments[i].data[8]);
                in->fragments[i].color.x = (in->fragments[i].color.x * texel.x);
                in->fragments[i].color.y = (in->fragments[i].color.y * texel.y);
                in->fragments[i].color.z = (in->fragments[i].color.z * texel.z);

                Vec3 inColor = (Vec3){in->fragments[i].color.x, in->fragments[i].color.y, in->fragments[i].color.z};
                Vec3 inNormal = (Vec3){in->fragments[i].data[9], in->fragments[i].data[10], in->fragments[i].data[11]};
                Vec3 inFragPos = (Vec3){in->fragments[i].data[12], in->fragments[i].data[13], in->fragments[i].data[14]};

                float ambientStrength = 0.1;
                float specularStrength = 0.5;
                
                Vec3 ambient = scalarMultiply3(ambientStrength, ubo->lightColor);

                Vec3 norm = normalize(inNormal);
                Vec3 lightDir = normalize(minus3(ubo->lightPos, inFragPos));

                Vec3 viewDir = normalize(minus3(ubo->viewPos, inFragPos));
                Vec3 reflectDir = reflect(scalarMultiply3(-1, lightDir), norm);  

                float spec = pow(fmax(dot3(viewDir, reflectDir), 0.0), 128);
                Vec3 specular = scalarMultiply3(specularStrength * spec, ubo->lightColor);

                float diff = fmax(dot3(norm, lightDir), 0.0);
                Vec3 diffuse = scalarMultiply3(diff, ubo->lightColor);

                Vec3 globalLight = {0.4, 0.4, 0.4};

                Vec3 result = multiply3(plus3(plus3(plus3(ambient, diffuse), specular), globalLight), inColor);

                

                in->fragments[i].color.x = result.x * texel.x/ 255.0;
                in->fragments[i].color.y = result.y * texel.y/ 255.0;
                in->fragments[i].color.z = result.z * texel.z/ 255.0;
                free(in->fragments[i].data);
            }
            break;
    }
    
    out.fragments = in->fragments;
    out.fragmentSize = in->fragmentSize;

    
    
    return out;
}