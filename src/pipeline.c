#include "pipeline.h"


void draw(VertexBuffer *vbo, UniformBuffer *ubo, Texture* pTextureResource, int pipelineVariation, Instance* instance){
    
    VertexShaderOutput vertexShaderOutput = runVertexShader(vbo, ubo, pipelineVariation);
    
    PrimitiveAssemblyOutput primitiveAssemblyOutput = runPrimitiveAssembly(vertexShaderOutput);
    
    ClippingOutput primitiveClippingOutput = runPrimitiveClipping(primitiveAssemblyOutput);
    
    PrimitiveOutput perspectiveDivideOutput = runPerspectiveDivide(primitiveClippingOutput);
    
    runScreenMapping(&perspectiveDivideOutput, instance->frameWidth, instance->frameHeight);
    
    TriangleTraversalOutput triangleTraversalOutput = runRasterizer(&perspectiveDivideOutput, pipelineVariation, instance->frameWidth, instance->frameHeight);
    
    FragmentShaderOutput fragmentShaderOutput = runFragmentShader(&triangleTraversalOutput, pTextureResource, ubo, pipelineVariation);

    runMerger(&fragmentShaderOutput, pTextureResource, instance->frameWidth, instance->frameHeight, instance->depthBuffer, instance->frameBuffer);

    free(vertexShaderOutput.vertices);
    free(vertexShaderOutput.layout);    
    free(primitiveAssemblyOutput.primitives);
    free(primitiveClippingOutput.primitives);
    for(int i = 0; i < primitiveClippingOutput.primitiveAmount; i++){
        free(perspectiveDivideOutput.primitives[i].v1.data);
        free(perspectiveDivideOutput.primitives[i].v2.data);
        free(perspectiveDivideOutput.primitives[i].v3.data);
    }
    free(perspectiveDivideOutput.primitives);
    free(triangleTraversalOutput.fragments);
    //free(fragmentShaderOutput.fragments);
    
    
}