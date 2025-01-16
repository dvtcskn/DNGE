
cbuffer PerFrameConstants : register(b13)
{
    matrix mCameraWorldViewProj;
    matrix PrevCameraViewProj;
};

struct GeometryVSIn
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

ParticleVSOut ParticleVS(GeometryVSIn input)
{
    ParticleVSOut output;
    
    float4 pos = float4(input.position.xyz, 1.0f);
    pos.xyz = input.InstancePos.xyz + pos.xyz;
    
    pos = mul(pos, mCameraWorldViewProj);
    
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

Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);

Output ParticlePS(ParticleVSOut Input, in uint bIsFrontFace : SV_IsFrontFace)
{
    Output Out;
    float4 diffuseColor = gTexture.Sample(gSampler, Input.texCoord);
    Out.finalColor = diffuseColor * Input.color;
    return Out;
}

Output ParticleFlatPS(ParticleVSOut Input, in uint bIsFrontFace : SV_IsFrontFace)
{
    Output Out;
    Out.finalColor = Input.color;
    return Out;
}
