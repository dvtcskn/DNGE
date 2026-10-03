
#ifndef BINDLESS
#define BINDLESS 0
#endif

struct GeometryVSIn
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float4 color : COLOR;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    uint ArrayIndex : ARRAYINDEX;
};

struct GeometryInstanceVSIn
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float4 color : COLOR;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    uint ArrayIndex : ARRAYINDEX;

    float3 InstancePos : INSTANCEPOS;
    float4 InstanceColor : INSTANCECOLOR;
};

struct GeometryVSOut
{
    float4 position : SV_Position;
    float3 WorldPos : WORLDPOS; // vertex position
    float2 texCoord : TEXCOORD;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    uint ArrayIndex : ARRAYINDEX;
};

struct LineGeometryVSIn
{
    float3 position : POSITION;
    float4 color : COLOR;
};

struct LineGeometryVSOut
{
    float4 position : SV_Position;
    float4 color : COLOR;
};

struct GUIGeometryVSIn										
{														
	float4 position : POSITION;							
	float2 texCoord : TEXCOORD;							
	float4 Color : COLOR;								
};														
																				
struct GUIGeometryVSOut									
{														
	float4 position : SV_Position;						
	float2 texCoord : TEXCOORD;							
	float4 Color : COLOR;								
};														

struct MaterialInstanceAttributes
{
	float4 BaseColor;

	float Metallic;
	float Roughness;
	float _padding1[2];

	float3 Emissive;
	float _padding2;
};

struct MaterialInstanceDescriptor
{
	uint AlbedoTextureIdx;
    uint SamplerIdx;
	uint NormalTextureIdx;
	uint MetallicRoughnessTextureIdx;
	uint EmissiveTextureIdx;
	uint _padding1[3];

	MaterialInstanceAttributes Attributes;
};

struct GeometrySceneDescriptor
{
	uint VertexBufferStructureIndex;
	uint VertexBufferLocation;
	uint VertexBufferStride;
	uint VertexInstanceBufferLocation;
	uint VertexInstanceBufferStride;
	uint StructureIndex;
	uint MaterialIndex;
	uint CameraConstantBuffer;
	uint ObjectConstantBuffer;
	uint TimeConstantBuffer;
};

#if BINDLESS
struct CBGradientIdx
{
	uint Idx;
};
struct UICBuffer			
{
	matrix GUIMatrix;
};
struct sGUIBuffer
{
    uint ObjectBuffer;
	uint StructureIndex;
	uint MaterialIndex;
	uint CBGradientIndex;
};
ConstantBuffer<sGUIBuffer> GUI : register(b0);

struct sGUIDepthBuffer
{
    uint ObjectBuffer;
};
ConstantBuffer<sGUIDepthBuffer> GUIDepthPass : register(b0);

struct sTimeBuffer
{
    float Time;
};

struct sCamera
{
    matrix mCameraWorldViewProj;
    matrix PrevCameraViewProj;
};

struct LineSceneDescriptor
{
	uint CameraConstantBuffer;
};

ConstantBuffer<LineSceneDescriptor> LineSceneDesc : register(b0);

struct ObjectCBuffer
{
    matrix modelMatrix;
    matrix PrevModelMatrix;
};

struct sMaterial
{
    uint DefaultTexture;
};

ConstantBuffer<GeometrySceneDescriptor> GeometrySceneDesc : register(b0);

struct ParticleSceneDescriptor
{
	uint VertexBufferStructureIndex;
	uint VertexBufferLocation;
	uint VertexBufferStride;
	uint VertexInstanceBufferLocation;
	uint VertexInstanceBufferStride;
    
	uint CameraConstantBuffer;
	uint StructureIndex;
	uint MaterialIndex;
};
ConstantBuffer<ParticleSceneDescriptor> ParticlSceneDesc : register(b0);

#else

cbuffer CBGradientIdx : register(b9)
{
	uint GradientIdx;
};
cbuffer UICBuffer : register(b13)
{
	matrix GUIMatrix;
};

cbuffer sTimeBuffer : register(b11)
{
    double Time;
};

Texture2D t_Diffuse : register(t0);
SamplerState gSampler : register(s0);

cbuffer sCamera
{
    matrix mCameraWorldViewProj;
    matrix PrevCameraViewProj;
};

cbuffer ObjectCBuffer
{
    matrix modelMatrix;
    matrix PrevModelMatrix;
};

Texture2D TextureAtlas : register(t0);
SamplerState SamplerAtlas : register(s0);

#endif


