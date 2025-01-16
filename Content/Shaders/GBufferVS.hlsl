
#define WithModelMatrix 1

cbuffer PerFrameConstants : register(b13)
{
    matrix mCameraWorldViewProj;
    matrix PrevCameraViewProj;
};

cbuffer ObjectCBuffer : register(b12)
{
    matrix modelMatrix;
    matrix PrevModelMatrix;
};

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
    float2 texCoord : TEXCOORD;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    uint ArrayIndex : ARRAYINDEX;
};

GeometryVSOut GeometryVS(GeometryVSIn input)
{
    GeometryVSOut output;
    
    float4 pos = float4(input.position.xyz, 1.0f);
#if WithModelMatrix
    pos = mul(pos, modelMatrix);
#endif
    pos = mul(pos, mCameraWorldViewProj);
	
    pos.z = 0.0f;
    pos.w = 1.0f;

    output.position = pos;
    
    output.color = input.color;
    
    output.normal = normalize(mul(float4(input.normal, 0), modelMatrix).xyz);
    output.tangent = input.tangent;
    output.binormal = input.binormal;
	
    output.texCoord = input.texCoord;
    output.ArrayIndex = input.ArrayIndex;
    
    return output;
}

GeometryVSOut GeometryInstanceVS(GeometryInstanceVSIn input)
{
    GeometryVSOut output;
    
    float4 pos = float4(input.position.xyz, 1.0f);
    pos.xyz = input.InstancePos.xyz + pos.xyz;
#if WithModelMatrix
    pos = mul(pos, modelMatrix);
#endif
    pos = mul(pos, mCameraWorldViewProj);
	
    pos.z = 0.0f;
    pos.w = 1.0f;

    output.position = pos;
    
    float4 color = input.color;
    color.xyzw = input.InstanceColor.xyzw + color.xyzw;

    output.color = color;
    
    output.normal = normalize(mul(float4(input.normal, 0), modelMatrix).xyz);
    output.tangent = input.tangent;
    output.binormal = input.binormal;
	
    output.texCoord = input.texCoord;
    output.ArrayIndex = input.ArrayIndex;
    
    return output;
}

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

LineGeometryVSOut GeometryVSLine(LineGeometryVSIn input)
{
    LineGeometryVSOut output;
    float4 pos = float4(input.position.xyz, 1.0f);
    pos = mul(pos, mCameraWorldViewProj);
    pos.z = 0.0f;
    pos.w = 1.0f;
    output.position = pos;	
    output.color = input.color;
    
    return output;
};
