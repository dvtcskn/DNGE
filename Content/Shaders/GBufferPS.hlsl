#ifndef BINDLESS
#define BINDLESS 0
#endif

#include "ShaderBindings.hlsli"

struct Output
{
    float4 finalColor : SV_Target0;
};

float4 DefaultTexturedGUIPS(GUIGeometryVSOut Input, in uint bIsFrontFace : SV_IsFrontFace) : SV_TARGET
{
#if BINDLESS
    StructuredBuffer<MaterialInstanceDescriptor> MaterialInstanceContainer = ResourceDescriptorHeap[GeometrySceneDesc.StructureIndex];
    MaterialInstanceDescriptor Material = MaterialInstanceContainer[GeometrySceneDesc.MaterialIndex];
    Texture2D DiffuseTexture = ResourceDescriptorHeap[Material.AlbedoTextureIdx]; 
    SamplerState mySampler = SamplerDescriptorHeap[Material.SamplerIdx];
    return DiffuseTexture.Sample(mySampler, Input.texCoord);
#else
    return gTexture.Sample(gSampler, Input.texCoord);
#endif
}

Output GeometryBackgroundPS(GeometryVSOut Input, in uint bIsFrontFace : SV_IsFrontFace)
{
    Output output;
    
#if BINDLESS
    StructuredBuffer<MaterialInstanceDescriptor> MaterialInstanceContainer = ResourceDescriptorHeap[GeometrySceneDesc.StructureIndex];
    MaterialInstanceDescriptor Material = MaterialInstanceContainer[GeometrySceneDesc.MaterialIndex];
    Texture2D DiffuseTexture = ResourceDescriptorHeap[Material.AlbedoTextureIdx];    
    SamplerState mySampler = SamplerDescriptorHeap[Material.SamplerIdx];
    ConstantBuffer<sTimeBuffer> TimeBuffer = ResourceDescriptorHeap[GeometrySceneDesc.TimeConstantBuffer];

    output.finalColor = DiffuseTexture.Sample(mySampler, float2(Input.texCoord.x * 4 + TimeBuffer.Time, Input.texCoord.y * 4));
#else
    output.finalColor = gTexture.Sample(gSampler, float2(Input.texCoord.x * 4 + Time, Input.texCoord.y * 4));
#endif

    return output;
}

Output GeometryPS(GeometryVSOut Input, in uint bIsFrontFace : SV_IsFrontFace)
{
    Output output;
    
#if BINDLESS
    StructuredBuffer<MaterialInstanceDescriptor> MaterialInstanceContainer = ResourceDescriptorHeap[GeometrySceneDesc.StructureIndex];
    MaterialInstanceDescriptor Material = MaterialInstanceContainer[GeometrySceneDesc.MaterialIndex];
    if (Material.AlbedoTextureIdx >= 999 || Material.SamplerIdx >= 999)
    {
        Output output;
        output.finalColor = float4(0.0, 0.0, 0.0, 1.0);
        return output;
    }
    Texture2D DiffuseTexture = ResourceDescriptorHeap[Material.AlbedoTextureIdx];
    SamplerState mySampler = SamplerDescriptorHeap[Material.SamplerIdx];
    output.finalColor = DiffuseTexture.Sample(mySampler, Input.texCoord);
#else
    output.finalColor = gTexture.Sample(gSampler, Input.texCoord);
#endif

    return output;
}

Output GeometryFlatPS(GeometryVSOut Input, in uint bIsFrontFace : SV_IsFrontFace)
{
    Output output;
    output.finalColor = Input.color;
    return output;
}

float4 LineGeometryFlatPS(LineGeometryVSOut Input, in uint bIsFrontFace : SV_IsFrontFace) : SV_TARGET
{
    return Input.color;
}

Output GeometryAtlasTexturedPS(GeometryVSOut Input, in uint bIsFrontFace : SV_IsFrontFace)
{
    Output output;

#if BINDLESS
    StructuredBuffer<MaterialInstanceDescriptor> MaterialInstanceContainer = ResourceDescriptorHeap[GeometrySceneDesc.StructureIndex];
    MaterialInstanceDescriptor Material = MaterialInstanceContainer[GeometrySceneDesc.MaterialIndex];
    Texture2D DiffuseTexture = ResourceDescriptorHeap[Material.AlbedoTextureIdx];
    SamplerState mySampler = SamplerDescriptorHeap[Material.SamplerIdx];
    output.finalColor =DiffuseTexture.Sample(mySampler, float2(Input.texCoord.x, Input.texCoord.y));
#else
    output.finalColor = gTexture.Sample(gSampler, float2(Flip ? 1.0 - Input.texCoord.x : Input.texCoord.x, Input.texCoord.y));
#endif

    return output;
}
