
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

struct GUIGeometryVSOut
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD;
    float4 Color : COLOR;
};

struct Output
{
    float4 finalColor : SV_Target0;
};

cbuffer sTimeBuffer : register(b0)
{
    double Time;
};

cbuffer sAnimationConstantBuffer : register(b1)
{
    //uint LayerIndex;
    uint Flip;
    //float FlipMaxX = 0;
    //float FlipMinX = 0;
};

Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);

float4 DefaultTexturedGUIPS(GUIGeometryVSOut Input, in uint bIsFrontFace : SV_IsFrontFace) : SV_TARGET
{
    return gTexture.Sample(gSampler, Input.texCoord);
}

Output GeometryBackgroundPS(GeometryVSOut Input, in uint bIsFrontFace : SV_IsFrontFace) : SV_TARGET
{
    Output output;
    
    output.finalColor = gTexture.Sample(gSampler, float2(Input.texCoord.x * 4 + Time, Input.texCoord.y * 4));
    
    return output;
}

Output GeometryPS(GeometryVSOut Input, in uint bIsFrontFace : SV_IsFrontFace) : SV_TARGET
{
    Output output;
    
    output.finalColor = gTexture.Sample(gSampler, Input.texCoord);
    
    return output;
}

Output GeometryFlatPS(GeometryVSOut Input, in uint bIsFrontFace : SV_IsFrontFace) : SV_TARGET
{
    Output output;
    output.finalColor = Input.color;
    return output;
}

struct LineGeometryVSOut
{
    float4 position : SV_Position;
    float4 color : COLOR;
};

float4 LineGeometryFlatPS(LineGeometryVSOut Input, in uint bIsFrontFace : SV_IsFrontFace) : SV_TARGET
{
    return Input.color;
}

Output GeometryAtlasTexturedPS(GeometryVSOut Input, in uint bIsFrontFace : SV_IsFrontFace) : SV_TARGET
{
    Output output;

    output.finalColor = gTexture.Sample(gSampler, float2(Flip ? 1.0 - Input.texCoord.x : Input.texCoord.x, Input.texCoord.y));
    
    return output;
}
