// AMD Cauldron code
// 
// Copyright(c) 2020 Advanced Micro Devices, Inc.All rights reserved.
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "Tonemappers.hlsl"

#ifndef BINDLESS
#define BINDLESS 0
#endif

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
#if BINDLESS
struct TonemappingAttributes
{
    float u_exposure;
    uint  u_toneMapper;
    uint  u_gamma2;
};
struct PerFrame
{
    uint HDRTexture;
    uint Sampler;
    uint Attributes;
};
ConstantBuffer<PerFrame> PerFrameCB : register(b0);
#else
cbuffer cbPerFrame : register(b0)
{
    float u_exposure : packoffset(c0.x);
    int   u_toneMapper : packoffset(c0.y);
    int   u_gamma2 : packoffset(c0.z);
}
#endif

//--------------------------------------------------------------------------------------
// I/O Structures
//--------------------------------------------------------------------------------------
struct VERTEX
{
    float2 vTexcoord : TEXCOORD;
};

//--------------------------------------------------------------------------------------
// Texture definitions
//--------------------------------------------------------------------------------------
#if !BINDLESS
Texture2D        HDR              :register(t0);
SamplerState     samLinearWrap    :register(s0);
#endif


float3 Pattern(float2 vTexcoord)
{
    if (vTexcoord.x < .5)
        return float3(.5, .5, .5);

    uint y = vTexcoord.y * 720;
    if ((y & 1) == 1)
        return float3(1.0, 1.0, 1.0);


    return float3(0, 0, 0);
}

float3 ApplyGamma(float3 color)
{
    return pow(abs(color.rgb), 1.0f / 2.2f);
}

float3 Tonemap(float3 color, float exposure, int tonemapper)
{
    color *= exposure;

    switch (tonemapper)
    {
    case 0: return AMDTonemapper(color);
    case 1: return DX11DSK(color);
    case 2: return Reinhard(color);
    case 3: return Uncharted2Tonemap(color);
    case 4: return ACESFilm(color);
    case 5: return color;
    default: return float3(1, 1, 1);
    }
}

//--------------------------------------------------------------------------------------
// Main function
//--------------------------------------------------------------------------------------

#if BINDLESS
float4 mainPS(VERTEX Input) : SV_Target
{
    ConstantBuffer<TonemappingAttributes> Frame = ResourceDescriptorHeap[PerFrameCB.Attributes];

    if (Frame.u_exposure < 0)
    {
        Texture2D HDRTexture = ResourceDescriptorHeap[PerFrameCB.HDRTexture]; 
        SamplerState mySampler = SamplerDescriptorHeap[PerFrameCB.Sampler];
        return HDRTexture.Sample(mySampler, Input.vTexcoord);
    }

    Texture2D HDRTexture = ResourceDescriptorHeap[PerFrameCB.HDRTexture]; 
    SamplerState mySampler = SamplerDescriptorHeap[PerFrameCB.Sampler];
    float4 texColor = HDRTexture.Sample(mySampler, Input.vTexcoord);

    float3 color = Tonemap(texColor.rgb, Frame.u_exposure, Frame.u_toneMapper);
    if (Frame.u_gamma2 == 1)
        color = sqrt(color);
    return float4(color, 1);
}
#else
float4 mainPS(VERTEX Input) : SV_Target
{
    if (Frame.u_exposure < 0)
    {
        return HDR.Sample(samLinearWrap, Input.vTexcoord);
    }

    float4 texColor = HDR.Sample(samLinearWrap, Input.vTexcoord);

    float3 color = Tonemap(texColor.rgb, u_exposure, u_toneMapper);
    if (u_gamma2 == 1)
        color = sqrt(color);
    return float4(color, 1);
}
#endif
