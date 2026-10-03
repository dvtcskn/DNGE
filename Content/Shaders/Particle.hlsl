
#ifndef BINDLESS
#define BINDLESS 0
#endif

#ifndef ENABLE_BINDLESS_VB
#define ENABLE_BINDLESS_VB 1
#endif

#include "ShaderBindings.hlsli"

struct ParticleGeometryVSIn
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD;
    float4 color : COLOR;

    float3 InstancePos : INSTANCEPOS;
    float4 InstanceColor : INSTANCECOLOR;
};

struct ParticleVSOut
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD;
    float4 color : COLOR;
};

#if ENABLE_BINDLESS_VB
ParticleVSOut Particle2DVS(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
#else
ParticleVSOut Particle2DVS(ParticleGeometryVSIn input)
#endif
{
#if ENABLE_BINDLESS_VB
    ByteAddressBuffer vb = ResourceDescriptorHeap[ParticlSceneDesc.VertexBufferStructureIndex];

    uint stride = ParticlSceneDesc.VertexBufferStride;
    uint baseOffset = ParticlSceneDesc.VertexBufferLocation + stride * vertexID;

    ParticleGeometryVSIn input;
    input.position = asfloat(vb.Load3(baseOffset + 0));
    input.texCoord = asfloat(vb.Load2(baseOffset + 12));
    input.color = asfloat(vb.Load4(baseOffset + 20));

    uint InstanceStride = ParticlSceneDesc.VertexInstanceBufferStride;
    uint InstanceBaseOffset = ParticlSceneDesc.VertexInstanceBufferLocation + (InstanceStride * instanceID);

    input.InstancePos = asfloat(vb.Load3(InstanceBaseOffset + 0));
    input.InstanceColor = asfloat(vb.Load4(InstanceBaseOffset + 16));
    //input.InstanceColor.r = 1.0;

#endif
    ParticleVSOut output;
    
    float4 pos = float4(input.position.xyz, 1.0f);
    pos.xyz = input.InstancePos.xyz + pos.xyz;
    
#if BINDLESS
    ConstantBuffer<sCamera> Camera = ResourceDescriptorHeap[ParticlSceneDesc.CameraConstantBuffer];
    pos = mul(pos, Camera.mCameraWorldViewProj);
#else
    pos = mul(pos, Camera.mCameraWorldViewProj);
#endif
    
    float4 color = input.color;
    color.xyzw = input.InstanceColor.xyzw + color.xyzw;
    
    output.position = pos;
    output.texCoord = input.texCoord.xy;
    output.color = input.InstanceColor;
    
    return output;
}

struct Output
{
    float4 finalColor : SV_Target0;
};

#if !BINDLESS
Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);
#endif

Output ParticlePS(ParticleVSOut Input, in uint bIsFrontFace : SV_IsFrontFace)
{
    Output Out;
#if BINDLESS
    StructuredBuffer<MaterialInstanceDescriptor> MaterialInstanceContainer = ResourceDescriptorHeap[ParticlSceneDesc.StructureIndex];
    MaterialInstanceDescriptor Material = MaterialInstanceContainer[ParticlSceneDesc.MaterialIndex];
    Texture2D DiffuseTexture = ResourceDescriptorHeap[Material.AlbedoTextureIdx];
    SamplerState mySampler = SamplerDescriptorHeap[Material.SamplerIdx];
    float4 diffuseColor = DiffuseTexture.Sample(mySampler, Input.texCoord);
#else
    float4 diffuseColor = gTexture.Sample(gSampler, Input.texCoord);
#endif
    Out.finalColor = diffuseColor * Input.color;
    return Out;
}

Output ParticleFlatPS(ParticleVSOut Input, in uint bIsFrontFace : SV_IsFrontFace)
{
    Output Out;
    Out.finalColor = Input.color;
    return Out;
}
