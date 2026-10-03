
#ifndef BINDLESS
#define BINDLESS 0
#endif

#include "ShaderBindings.hlsli"

GUIGeometryVSOut GUIGeometryVS(GUIGeometryVSIn input)			
{														
	GUIGeometryVSOut output;							

	float4 pos = float4(input.position.xyz, 1.0f);
    
#if BINDLESS
	ConstantBuffer<UICBuffer> WBuffer = ResourceDescriptorHeap[GUI.ObjectBuffer];
    pos = mul(pos, WBuffer.GUIMatrix);
#else
    pos = mul(pos, GUIMatrix);
#endif				

	pos.z = 0.0f;
	pos.w = 1.0f;

	output.position = pos;

	output.Color = input.Color;
	output.texCoord = input.texCoord;

	return output;
};
																			
float4 DepthTestVS(float4 pos : POSITION) : SV_POSITION		
{														
	float4 position = float4(pos.xyz, 1.0f);
#if BINDLESS	
	ConstantBuffer<UICBuffer> WBuffer = ResourceDescriptorHeap[GUIDepthPass.ObjectBuffer];
	position = mul(position, WBuffer.GUIMatrix);			
#else
	position = mul(position, GUIMatrix);	
#endif							
																			
	position.z = 0.0f;									
	position.w = 1.0f;									
																					
	return position;									
}														
																					
float4 mainDepthPS() : SV_TARGET								
{														
	return float4(1.0f, 1.0f, 1.0f, 0.0f);				
};

float4 WidgetFlatColorPS(GUIGeometryVSOut Input) : SV_TARGET	
{															
	return Input.Color;
};

float4 WidgetBasePSFlatBlack(GUIGeometryVSOut Input) : SV_TARGET	
{															
	return float4(0.0f, 0.0f, 0.0f, 1.0f);									
};

#if !BINDLESS
Texture2D<float> gFontTexture : register(t0);
SamplerState gLinearSampler : register(s0);
#endif

float4 FontPS(GUIGeometryVSOut input) : SV_Target0											
{
#if BINDLESS
    StructuredBuffer<MaterialInstanceDescriptor> MaterialInstanceContainer = ResourceDescriptorHeap[GUI.StructureIndex];
    MaterialInstanceDescriptor Material = MaterialInstanceContainer[GUI.MaterialIndex];
    Texture2D<float> FontTexture = ResourceDescriptorHeap[Material.AlbedoTextureIdx];
    SamplerState LinearSampler = SamplerDescriptorHeap[Material.SamplerIdx];
	
	float4 alpha = FontTexture.Sample(LinearSampler, input.texCoord);
#else
    float4 alpha = gFontTexture.Sample(gLinearSampler, input.texCoord);	
#endif
	return float4(input.Color.rgb, input.Color.a * smoothstep(0.0 - (1.0f / 64.0f), 1.0 + (1.0f / 64.0f), alpha.a));			
};

float4 RGBtoFloat(float r, float g, float b)																
{																											
	return float4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);												
}																											
																																	
float4 Gradient3(float4 First, float4 Mid, float4 End, float Alpha)											
{																											
	float h = 0.5;																							
	return lerp(lerp(First, Mid, Alpha / h), lerp(Mid, End, (Alpha - h) / (1.0 - h)), step(h, Alpha));		
}																											
																																	
float4 Gradient2(float4 First, float4 End, float Alpha)														
{																											
	return float4(lerp(First, End, Alpha));																	
}

float4 GradientPS(GUIGeometryVSOut Input) : SV_TARGET																										
{		
#if BINDLESS
	ConstantBuffer<CBGradientIdx> WBuffer = ResourceDescriptorHeap[GUI.CBGradientIndex];
    uint GradientIdx = WBuffer.Idx;
#endif																																			
	[forcecase]																																			
	switch (GradientIdx)																																
	{																																					
		case 0: return Gradient2(RGBtoFloat(255.0f, 0.0f, 132.0f), RGBtoFloat(51.0f, 0.0f, 27.0f), Input.texCoord.r);										
		case 1: return Gradient2(RGBtoFloat(67.0f, 198.0f, 172.0f), RGBtoFloat(25.0f, 22.0f, 84.0f), Input.texCoord.r);										
		case 2:	return Gradient2(RGBtoFloat(239.0f, 50.0f, 217.0f), RGBtoFloat(137.0f, 255.0f, 253.0f), Input.texCoord.r);									
		case 3:	return Gradient2(RGBtoFloat(0.0f, 92.0f, 151.0f), RGBtoFloat(54.0f, 55.0f, 149.0f), Input.texCoord.r);										
		case 4:	return float4(lerp(RGBtoFloat(203.0f, 45.0f, 62.0f), RGBtoFloat(239.0f, 71.0f, 58.0f), Input.texCoord.r));									
		case 5:	return Gradient3(RGBtoFloat(15.0f, 12.0f, 41.0f), RGBtoFloat(48.0f, 43.0f, 99.0f), RGBtoFloat(36.0f, 36.0f, 62.0f), Input.texCoord.r);		
		case 6:	return Gradient2(RGBtoFloat(33.0f, 34.0f, 42.0f), RGBtoFloat(58.0f, 96.0f, 115.0f), Input.texCoord.r);										
		case 7:	return float4(lerp(RGBtoFloat(0.0f, 4.0f, 40.0f), RGBtoFloat(0.0f, 78.0f, 146.0f), Input.texCoord.r));										
		case 8:	return float4(lerp(RGBtoFloat(233.0f, 100.0f, 67.0f), RGBtoFloat(144.0f, 78.0f, 149.0f), Input.texCoord.r));								
		case 9:	return float4(lerp(RGBtoFloat(219.0f, 230.0f, 246.0f), RGBtoFloat(197.0f, 121.0f, 109.0f), Input.texCoord.r));								
		case 10: return float4(lerp(RGBtoFloat(211.0f, 204.0f, 227.0f), RGBtoFloat(233.0f, 228.0f, 240.0f), Input.texCoord.r));								
		case 11: return float4(lerp(RGBtoFloat(116.0f, 235.0f, 213.0f), RGBtoFloat(172.0f, 182.0f, 229.0f), Input.texCoord.r));								
		case 12: return float4(lerp(RGBtoFloat(20.0f, 30.0f, 48.0f), RGBtoFloat(36.0f, 59.0f, 85.0f), Input.texCoord.r));									
		case 13: return Gradient2(RGBtoFloat(0.0f, 0.0f, 0.0f), RGBtoFloat(67.0f, 67.0f, 67.0f), Input.texCoord.r);											
		case 14: return float4(lerp(RGBtoFloat(96.0f, 108.0f, 136.0f), RGBtoFloat(63.0f, 76.0f, 107.0f), Input.texCoord.r));								
		case 15: return float4(lerp(RGBtoFloat(96.0f, 108.0f, 136.0f), RGBtoFloat(63.0f, 76.0f, 107.0f), Input.texCoord.r));								
	}																																					
																																												
		return float4(0.0f, 0.0f, 0.0f, 1.0f);																												
};

float4 DefaultTexturedGUIPS(GUIGeometryVSOut Input, in uint bIsFrontFace : SV_IsFrontFace) : SV_TARGET
{
#if BINDLESS
    StructuredBuffer<MaterialInstanceDescriptor> MaterialInstanceContainer = ResourceDescriptorHeap[GUI.StructureIndex];
    MaterialInstanceDescriptor Material = MaterialInstanceContainer[GUI.MaterialIndex];
    Texture2D DiffuseTexture = ResourceDescriptorHeap[Material.AlbedoTextureIdx];
    SamplerState mySampler = SamplerDescriptorHeap[Material.SamplerIdx];
	
    return DiffuseTexture.Sample(mySampler, Input.texCoord);
#else
    return t_Diffuse.Sample(gSampler, Input.texCoord);
#endif
}
