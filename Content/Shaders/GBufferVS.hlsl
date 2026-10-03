#ifndef BINDLESS
#define BINDLESS 0
#endif

#ifndef ENABLE_BINDLESS_VB
#define ENABLE_BINDLESS_VB 1
#endif

#include "ShaderBindings.hlsli"

#if ENABLE_BINDLESS_VB
GeometryVSOut GeometryVS(uint vertexID : SV_VertexID/*, uint instanceID : SV_InstanceID*/)
#else
GeometryVSOut GeometryVS(GeometryVSIn input)
#endif
{
#if ENABLE_BINDLESS_VB
    ByteAddressBuffer vb = ResourceDescriptorHeap[GeometrySceneDesc.VertexBufferStructureIndex];

    uint stride = GeometrySceneDesc.VertexBufferStride;
    uint baseOffset = GeometrySceneDesc.VertexBufferLocation + stride * vertexID;

    GeometryVSIn input;
    input.position = asfloat(vb.Load3(baseOffset + 0));
    input.normal = asfloat(vb.Load3(baseOffset + 12));
    input.texCoord = asfloat(vb.Load2(baseOffset + 24));
    input.color = asfloat(vb.Load4(baseOffset + 32));
    input.tangent = asfloat(vb.Load3(baseOffset + 48));
    input.binormal = asfloat(vb.Load3(baseOffset + 60));
    input.ArrayIndex = asuint(vb.Load(baseOffset + 72));
#endif

    GeometryVSOut output;
    
    float4 pos = float4(input.position.xyz, 1.0f);

#if BINDLESS
    ConstantBuffer<ObjectCBuffer> Object = ResourceDescriptorHeap[GeometrySceneDesc.ObjectConstantBuffer];
    pos = mul(pos, Object.modelMatrix);
#else
    pos = mul(pos, modelMatrix);
#endif

#if BINDLESS
    ConstantBuffer<sCamera> Camera = ResourceDescriptorHeap[GeometrySceneDesc.CameraConstantBuffer];
    pos = mul(pos, Camera.mCameraWorldViewProj);
#else
    pos = mul(pos, mCameraWorldViewProj);
#endif
	
    pos.z = 0.0f;
    pos.w = 1.0f;

    output.position = pos;
    
    output.color = input.color;
    
#if BINDLESS
    output.normal = normalize(mul(float4(input.normal, 0), Object.modelMatrix).xyz);
#else
    output.normal = normalize(mul(float4(input.normal, 0), modelMatrix).xyz);
#endif
    output.tangent = input.tangent;
    output.binormal = input.binormal;
	
    output.texCoord = input.texCoord;
    output.ArrayIndex = input.ArrayIndex;
    
    return output;
}

#if ENABLE_BINDLESS_VB
GeometryVSOut GeometryInstanceVS(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
#else
GeometryVSOut GeometryInstanceVS(GeometryInstanceVSIn input)
#endif
{
#if ENABLE_BINDLESS_VB
    ByteAddressBuffer vb = ResourceDescriptorHeap[GeometrySceneDesc.VertexBufferStructureIndex];

    uint stride = GeometrySceneDesc.VertexBufferStride;
    uint baseOffset = GeometrySceneDesc.VertexBufferLocation + stride * vertexID;

    GeometryInstanceVSIn input;
    input.position = asfloat(vb.Load3(baseOffset + 0));
    input.normal = asfloat(vb.Load3(baseOffset + 12));
    input.texCoord = asfloat(vb.Load2(baseOffset + 24));
    input.color = asfloat(vb.Load4(baseOffset + 32));
    input.tangent = asfloat(vb.Load3(baseOffset + 48));
    input.binormal = asfloat(vb.Load3(baseOffset + 60));
    input.ArrayIndex = asuint(vb.Load(baseOffset + 72));

    uint InstanceStride = GeometrySceneDesc.VertexInstanceBufferStride;
    uint InstanceBaseOffset = GeometrySceneDesc.VertexInstanceBufferLocation + (InstanceStride * instanceID);

    input.InstancePos = asfloat(vb.Load3(InstanceBaseOffset + 0));
    input.InstanceColor = asfloat(vb.Load4(InstanceBaseOffset + 16));
#endif

    GeometryVSOut output;
    
    float4 pos = float4(input.position.xyz, 1.0f);
    pos.xyz = input.InstancePos.xyz + pos.xyz;

#if BINDLESS
    ConstantBuffer<ObjectCBuffer> Object = ResourceDescriptorHeap[GeometrySceneDesc.ObjectConstantBuffer];
    pos = mul(pos, Object.modelMatrix);
#else
    pos = mul(pos, modelMatrix);
#endif

#if BINDLESS
    ConstantBuffer<sCamera> Camera = ResourceDescriptorHeap[GeometrySceneDesc.CameraConstantBuffer];
    pos = mul(pos, Camera.mCameraWorldViewProj);
#else
    pos = mul(pos, mCameraWorldViewProj);
#endif
	
    pos.z = 0.0f;
    pos.w = 1.0f;

    output.position = pos;
    
    float4 color = input.color;
    color.xyzw = input.InstanceColor.xyzw + color.xyzw;

    output.color = color;
    
#if BINDLESS
    output.normal = normalize(mul(float4(input.normal, 0), Object.modelMatrix).xyz);
#else
    output.normal = normalize(mul(float4(input.normal, 0), modelMatrix).xyz);
#endif
    output.tangent = input.tangent;
    output.binormal = input.binormal;
	
    output.texCoord = input.texCoord;
    output.ArrayIndex = input.ArrayIndex;
    
    return output;
}

//#if ENABLE_BINDLESS_VB
//LineGeometryVSOut GeometryVSLine(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
//#else
LineGeometryVSOut GeometryVSLine(LineGeometryVSIn input)
//#endif
{
    LineGeometryVSOut output;
    float4 pos = float4(input.position.xyz, 1.0f);
#if BINDLESS
    ConstantBuffer<sCamera> Constants = ResourceDescriptorHeap[LineSceneDesc.CameraConstantBuffer];
    pos = mul(pos, Constants.mCameraWorldViewProj);
#else
    pos = mul(pos, mCameraWorldViewProj);
#endif
    pos.z = 0.0f;
    pos.w = 1.0f;
    output.position = pos;	
    output.color = input.color;
    
    return output;
};
