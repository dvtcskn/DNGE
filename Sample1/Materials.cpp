/* ---------------------------------------------------------------------------------------
* MIT License
*
* Copyright (c) 2023 Davut Coþkun.
* All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
* ---------------------------------------------------------------------------------------
*/

#include "pch.h"
#include "Materials.h"
#include <cbgui.h>
#include "cbString.h"

namespace GameMaterials
{
	static std::string WidgetBaseVS = "												\
							cbuffer UICBuffer : register(b13)					\
							{														\
								matrix WidgetMatrix;								\
							};														\
																					\
							struct GeometryVSIn										\
							{														\
								float4 position : POSITION;							\
								float2 texCoord : TEXCOORD;							\
								float4 Color : COLOR;								\
							};														\
																					\
							struct GeometryVSOut									\
							{														\
								float4 position : SV_Position;						\
								float2 texCoord : TEXCOORD;							\
								float4 Color : COLOR;								\
							};														\
																					\
							GeometryVSOut GeometryVS(GeometryVSIn input)			\
							{														\
								GeometryVSOut output;								\
																					\
								float4 pos = float4(input.position.xyz, 1.0f);		\
								pos = mul(pos, WidgetMatrix);						\
																					\
								pos.z = 0.0f;										\
								pos.w = 1.0f;										\
																					\
								output.position = pos;								\
																					\
								output.Color = input.Color;							\
								output.texCoord = input.texCoord;					\
																					\
								return output;										\
							}";

	static std::string WidgetBasePS_Flat = "											\
							struct GeometryVSOut										\
							{															\
								float4 position : SV_Position;							\
								float2 texCoord : TEXCOORD;								\
								float4 Color : COLOR;									\
							};															\
																						\
							float4 WidgetFlatColorPS(GeometryVSOut Input) : SV_TARGET	\
							{															\
								return Input.Color;										\
							}";

	static std::string WidgetBasePS_Font = "																		\
							struct GeometryVSOut																	\
							{																						\
								float4 position : SV_Position;														\
								float2 texCoord : TEXCOORD;															\
								float4 Color : COLOR;																\
							};																						\
																													\
							Texture2D<float> gFontTexture : register(t0);											\
							SamplerState gLinearSampler : register(s0);												\
																													\
							float4 FontPS(GeometryVSOut input) : SV_Target0											\
							{																						\
								float4 alpha = gFontTexture.Sample(gLinearSampler, input.texCoord);															\
								return float4(input.Color.rgb, input.Color.a * smoothstep(0.0 - (1.0f / 64.0f), 1.0 + (1.0f / 64.0f), alpha.a));			\
							}";

	std::string WidgetBasePS_Gradient = "																													\
							struct GeometryVSOut																											\
							{																																\
								float4 position : SV_Position;																								\
								float2 texCoord : TEXCOORD;																									\
								float4 Color : COLOR;																										\
							};																																\
							cbuffer CBGradientIdx : register(b9)																		\
							{																											\
								uint GradientIdx;																						\
							};																											\
																																		\
							float4 RGBtoFloat(float r, float g, float b)																\
							{																											\
								return float4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);												\
							}																											\
																																		\
							float4 Gradient3(float4 First, float4 Mid, float4 End, float Alpha)											\
							{																											\
								float h = 0.5;																							\
								return lerp(lerp(First, Mid, Alpha / h), lerp(Mid, End, (Alpha - h) / (1.0 - h)), step(h, Alpha));		\
							}																											\
																																		\
							float4 Gradient2(float4 First, float4 End, float Alpha)														\
							{																											\
								return float4(lerp(First, End, Alpha));																	\
							}																											\
							float4 GradientPS(GeometryVSOut Input) : SV_TARGET																										\
							{																																						\
								[forcecase]																																			\
								switch (GradientIdx)																																\
								{																																					\
								case 0: return Gradient2(RGBtoFloat(255.0f, 0.0f, 132.0f), RGBtoFloat(51.0f, 0.0f, 27.0f), Input.texCoord.r);										\
								case 1: return Gradient2(RGBtoFloat(67.0f, 198.0f, 172.0f), RGBtoFloat(25.0f, 22.0f, 84.0f), Input.texCoord.r);										\
								case 2:	return Gradient2(RGBtoFloat(239.0f, 50.0f, 217.0f), RGBtoFloat(137.0f, 255.0f, 253.0f), Input.texCoord.r);									\
								case 3:	return Gradient2(RGBtoFloat(0.0f, 92.0f, 151.0f), RGBtoFloat(54.0f, 55.0f, 149.0f), Input.texCoord.r);										\
								case 4:	return float4(lerp(RGBtoFloat(203.0f, 45.0f, 62.0f), RGBtoFloat(239.0f, 71.0f, 58.0f), Input.texCoord.r));									\
								case 5:	return Gradient3(RGBtoFloat(15.0f, 12.0f, 41.0f), RGBtoFloat(48.0f, 43.0f, 99.0f), RGBtoFloat(36.0f, 36.0f, 62.0f), Input.texCoord.r);		\
								case 6:	return Gradient2(RGBtoFloat(33.0f, 34.0f, 42.0f), RGBtoFloat(58.0f, 96.0f, 115.0f), Input.texCoord.r);										\
								case 7:	return float4(lerp(RGBtoFloat(0.0f, 4.0f, 40.0f), RGBtoFloat(0.0f, 78.0f, 146.0f), Input.texCoord.r));										\
								case 8:	return float4(lerp(RGBtoFloat(233.0f, 100.0f, 67.0f), RGBtoFloat(144.0f, 78.0f, 149.0f), Input.texCoord.r));								\
								case 9:	return float4(lerp(RGBtoFloat(219.0f, 230.0f, 246.0f), RGBtoFloat(197.0f, 121.0f, 109.0f), Input.texCoord.r));								\
								case 10: return float4(lerp(RGBtoFloat(211.0f, 204.0f, 227.0f), RGBtoFloat(233.0f, 228.0f, 240.0f), Input.texCoord.r));								\
								case 11: return float4(lerp(RGBtoFloat(116.0f, 235.0f, 213.0f), RGBtoFloat(172.0f, 182.0f, 229.0f), Input.texCoord.r));								\
								case 12: return float4(lerp(RGBtoFloat(20.0f, 30.0f, 48.0f), RGBtoFloat(36.0f, 59.0f, 85.0f), Input.texCoord.r));									\
								case 13: return Gradient2(RGBtoFloat(0.0f, 0.0f, 0.0f), RGBtoFloat(67.0f, 67.0f, 67.0f), Input.texCoord.r);											\
								case 14: return float4(lerp(RGBtoFloat(96.0f, 108.0f, 136.0f), RGBtoFloat(63.0f, 76.0f, 107.0f), Input.texCoord.r));								\
								case 15: return float4(lerp(RGBtoFloat(96.0f, 108.0f, 136.0f), RGBtoFloat(63.0f, 76.0f, 107.0f), Input.texCoord.r));								\
								}																																					\
																																													\
								return float4(0.0f, 0.0f, 0.0f, 1.0f);																												\
							}";

	static bool bIsInitialized = false;

	void fFontTextureUpdate_Callback(const void* Texture, std::size_t RowPitch, std::size_t MinX, std::size_t MinY, std::size_t MaxX, std::size_t MaxY)
	{
		sMaterialManager::Get().GetMaterialInstance("Default_Font_Mat", "Default_Font_Mat_Instance")->UpdateTexture(0, Texture, RowPitch, MinX, MinY, MaxX, MaxY);
	};

	void InitMaterials()
	{
		if (bIsInitialized)
			return;

		bIsInitialized = true;

		{
			sPipelineDesc pPipelineDesc;
			pPipelineDesc.BlendAttribute = sBlendAttributeDesc(EBlendStateMode::eOpaque);
			pPipelineDesc.DepthStencilAttribute = sDepthStencilAttributeDesc();
			pPipelineDesc.DepthStencilAttribute.DepthTest = ECompareFunction::eLessEqual;
			//pPipelineDesc.DepthStencilAttribute.DepthTest = ECompareFunction::eAlways;
			pPipelineDesc.PrimitiveTopologyType = EPrimitiveType::eTRIANGLE_LIST;
			pPipelineDesc.RasterizerAttribute = sRasterizerAttributeDesc();
			pPipelineDesc.RasterizerAttribute.CullMode = ERasterizerCullMode::eNone;

			std::vector<sVertexAttributeDesc> VertexLayout =
			{
				{ "POSITION",	EFormat::RGB32_FLOAT,   0, offsetof(sVertexLayout, position),    false, sizeof(sVertexLayout) },
				{ "NORMAL",		EFormat::RGB32_FLOAT,   0, offsetof(sVertexLayout, normal),      false, sizeof(sVertexLayout) },
				{ "TEXCOORD",	EFormat::RG32_FLOAT,    0, offsetof(sVertexLayout, texCoord),    false, sizeof(sVertexLayout) },
				{ "COLOR",		EFormat::RGBA32_FLOAT,  0, offsetof(sVertexLayout, Color),       false, sizeof(sVertexLayout) },
				{ "TANGENT",	EFormat::RGB32_FLOAT,   0, offsetof(sVertexLayout, tangent),     false, sizeof(sVertexLayout) },
				{ "BINORMAL",	EFormat::RGB32_FLOAT,   0, offsetof(sVertexLayout, binormal),    false, sizeof(sVertexLayout) },
				{ "ARRAYINDEX",	EFormat::R32_UINT,	    0, offsetof(sVertexLayout, ArrayIndex),  false, sizeof(sVertexLayout) },
			};
			pPipelineDesc.VertexLayout = VertexLayout;

			pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eUniformBuffer, eShaderType::Vertex, 13));	// Model CB
			pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eUniformBuffer, eShaderType::Vertex, 12));
			pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eUniformBuffer, eShaderType::Pixel,  11));
			pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eTexture, eShaderType::Pixel, 0));

			sSamplerAttributeDesc sampler(ESamplerStateMode::ePointBorder);
			sampler.Filter = ESamplerFilter::ePoint;
			sampler.AddressU = ESamplerAddressMode::eWrap;
			sampler.AddressV = ESamplerAddressMode::eWrap;
			sampler.AddressW = ESamplerAddressMode::eWrap;
			sampler.MipBias = 0;
			sampler.MinMipLevel = -FLT_MAX;
			sampler.MaxMipLevel = FLT_MAX;
			sampler.MaxAnisotropy = 1;
			sampler.BorderColor = FColor::Transparent();

			pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(sampler, eShaderType::Pixel, 0));

			pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(L"..//Content\\Shaders\\GBufferVS.hlsl", "GeometryVS", eShaderType::Vertex));
			pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(L"..//Content\\Shaders\\GBufferPS.hlsl", "GeometryBackgroundPS", eShaderType::Pixel));

			auto DefaultActorMat = sMaterial::Create("BackgoundMat", EMaterialBlendMode::Opaque, pPipelineDesc);
			sMaterialManager::Get().StoreMaterial(DefaultActorMat);

			auto DefaultActorMatInstance = DefaultActorMat->CreateInstance("BackgoundMatInstance");
			DefaultActorMatInstance->AddTexture(ITexture2D::Create(L"E:\\VisualStudioProjects\\DNGE\\Content\\Pixel Adventure 1\\Free\\Background\\Pink.png", "Backgound", 3/* GPU::GetGBufferTextureEntryPoint()*/));
		}
		{
			sPipelineDesc pPipelineDesc;
			pPipelineDesc.BlendAttribute = sBlendAttributeDesc(EBlendStateMode::eOpaque);
			pPipelineDesc.DepthStencilAttribute = sDepthStencilAttributeDesc();
			pPipelineDesc.DepthStencilAttribute.DepthTest = ECompareFunction::eLessEqual;
			//pPipelineDesc.DepthStencilAttribute.DepthTest = ECompareFunction::eAlways;
			pPipelineDesc.PrimitiveTopologyType = EPrimitiveType::eTRIANGLE_LIST;
			pPipelineDesc.RasterizerAttribute = sRasterizerAttributeDesc();
			pPipelineDesc.RasterizerAttribute.CullMode = ERasterizerCullMode::eNone;

			std::vector<sVertexAttributeDesc> VertexLayout =
			{
				{ "POSITION",	EFormat::RGB32_FLOAT,   0, offsetof(sVertexLayout, position),    false, sizeof(sVertexLayout) },
				{ "NORMAL",		EFormat::RGB32_FLOAT,   0, offsetof(sVertexLayout, normal),      false, sizeof(sVertexLayout) },
				{ "TEXCOORD",	EFormat::RG32_FLOAT,    0, offsetof(sVertexLayout, texCoord),    false, sizeof(sVertexLayout) },
				{ "COLOR",		EFormat::RGBA32_FLOAT,  0, offsetof(sVertexLayout, Color),       false, sizeof(sVertexLayout) },
				{ "TANGENT",	EFormat::RGB32_FLOAT,   0, offsetof(sVertexLayout, tangent),     false, sizeof(sVertexLayout) },
				{ "BINORMAL",	EFormat::RGB32_FLOAT,   0, offsetof(sVertexLayout, binormal),    false, sizeof(sVertexLayout) },
				{ "ARRAYINDEX",	EFormat::R32_UINT,	    0, offsetof(sVertexLayout, ArrayIndex),  false, sizeof(sVertexLayout) },
				{ "INSTANCEPOS",	EFormat::RGB32_FLOAT,	1, offsetof(sVertexLayout::sVertexInstanceLayout, position),	  true, sizeof(sVertexLayout::sVertexInstanceLayout) },
				{ "INSTANCECOLOR",	EFormat::RGBA32_FLOAT,	1, offsetof(sVertexLayout::sVertexInstanceLayout, Color),		  true, sizeof(sVertexLayout::sVertexInstanceLayout) },
			};
			pPipelineDesc.VertexLayout = VertexLayout;

			pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eUniformBuffer, eShaderType::Vertex, 13));	// Model CB
			pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eUniformBuffer, eShaderType::Vertex, 12));
			pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eUniformBuffer, eShaderType::Pixel, 11));
			pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eUniformBuffer, eShaderType::Pixel, 10));
			pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eTexture, eShaderType::Pixel, 0));

			sSamplerAttributeDesc sampler(ESamplerStateMode::ePointWrap);

			pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(sampler, eShaderType::Pixel, 0));

			pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(L"..//Content\\Shaders\\GBufferVS.hlsl", "GeometryInstanceVS", eShaderType::Vertex));
			pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(L"..//Content\\Shaders\\GBufferPS.hlsl", "GeometryPS", eShaderType::Pixel));

			auto DefaultActorMat = sMaterial::Create("DefaultTexturedMaterial", EMaterialBlendMode::Opaque, pPipelineDesc);
			sMaterialManager::Get().StoreMaterial(DefaultActorMat);

			ITexture2D::SharedPtr TextureAtlas = ITexture2D::Create(L"E:\\VisualStudioProjects\\DNGE\\Content\\Pixel Adventure 1\\Free\\Terrain\\Terrain (16x16).png", "TerrainAtlas", 3/* GPU::GetGBufferTextureEntryPoint()*/);

			auto TextureAtlasDesc = TextureAtlas->GetDesc();

			for (std::size_t h = 0; h < 176; h += 16)
			{
				for (std::size_t w = 0; w < 352; w += 16)
				{
					auto MatInstance = DefaultActorMat->CreateInstance("TerrainMatInstance_" + std::to_string(w) + "x" + std::to_string(h));
					sTextureDesc Desc;
					Desc.Dimensions.X = 16;
					Desc.Dimensions.Y = 16;
					Desc.Format = TextureAtlasDesc.Format;
					ITexture2D::SharedPtr Texture = ITexture2D::CreateEmpty("Terrain_" + std::to_string(w) + "x" + std::to_string(h), Desc, 4);
					Texture->UpdateTexture(TextureAtlas.get(), 0, 0, IntVector2(0, 0), FBounds2D(FDimension2D(16.0f,16.0f), FVector2(8.0f + w,8.0f + h)));
					MatInstance->AddTexture(Texture);
				}
			}

			TextureAtlas = nullptr;
		}

		{
			sPipelineDesc pPipelineDesc;
			pPipelineDesc.BlendAttribute = sBlendAttributeDesc(EBlendStateMode::eNonPremultiplied);
			pPipelineDesc.DepthStencilAttribute = sDepthStencilAttributeDesc();
			pPipelineDesc.DepthStencilAttribute.DepthTest = ECompareFunction::eLessEqual;
			//pPipelineDesc.DepthStencilAttribute.DepthTest = ECompareFunction::eAlways;
			pPipelineDesc.PrimitiveTopologyType = EPrimitiveType::eTRIANGLE_LIST;
			pPipelineDesc.RasterizerAttribute = sRasterizerAttributeDesc();
			pPipelineDesc.RasterizerAttribute.CullMode = ERasterizerCullMode::eNone;

			std::vector<sVertexAttributeDesc> VertexLayout =
			{
				{ "POSITION",	EFormat::RGB32_FLOAT,   0, offsetof(sVertexLayout, position),    false, sizeof(sVertexLayout) },
				{ "NORMAL",		EFormat::RGB32_FLOAT,   0, offsetof(sVertexLayout, normal),      false, sizeof(sVertexLayout) },
				{ "TEXCOORD",	EFormat::RG32_FLOAT,    0, offsetof(sVertexLayout, texCoord),    false, sizeof(sVertexLayout) },
				{ "COLOR",		EFormat::RGBA32_FLOAT,  0, offsetof(sVertexLayout, Color),       false, sizeof(sVertexLayout) },
				{ "TANGENT",	EFormat::RGB32_FLOAT,   0, offsetof(sVertexLayout, tangent),     false, sizeof(sVertexLayout) },
				{ "BINORMAL",	EFormat::RGB32_FLOAT,   0, offsetof(sVertexLayout, binormal),    false, sizeof(sVertexLayout) },
				{ "ARRAYINDEX",	EFormat::R32_UINT,	    0, offsetof(sVertexLayout, ArrayIndex),  false, sizeof(sVertexLayout) },
			};
			pPipelineDesc.VertexLayout = VertexLayout;

			pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eUniformBuffer, eShaderType::Vertex, 13));	// Model CB
			pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eUniformBuffer, eShaderType::Vertex, 12));
			pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eUniformBuffer, eShaderType::Pixel, 11));
			pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eUniformBuffer, eShaderType::Pixel, 10));
			pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eTexture, eShaderType::Pixel, 0));

			sSamplerAttributeDesc sampler(ESamplerStateMode::ePointBorder);
			sampler.Filter = ESamplerFilter::ePoint;
			sampler.AddressU = ESamplerAddressMode::eClamp;
			sampler.AddressV = ESamplerAddressMode::eClamp;
			sampler.AddressW = ESamplerAddressMode::eClamp;
			sampler.MipBias = 0;
			sampler.MinMipLevel = -FLT_MAX;
			sampler.MaxMipLevel = FLT_MAX;
			sampler.MaxAnisotropy = 1;
			sampler.BorderColor = FColor::Transparent();

			pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(sampler, eShaderType::Pixel, 0));

			pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(L"..//Content\\Shaders\\GBufferVS.hlsl", "GeometryVS", eShaderType::Vertex));
			pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(L"..//Content\\Shaders\\GBufferPS.hlsl", "GeometryAtlasTexturedPS", eShaderType::Pixel));

			auto DefaultActorMat = sMaterial::Create("DefaultActorAtlastMat", EMaterialBlendMode::Masked, pPipelineDesc);
			sMaterialManager::Get().StoreMaterial(DefaultActorMat);

			auto DefaultActorMatInstance = DefaultActorMat->CreateInstance("DefaultActorAtlastMatInstance");
		}

		{
			sPipelineDesc pPipelineDesc;
			pPipelineDesc.BlendAttribute = sBlendAttributeDesc(EBlendStateMode::eOpaque);
			for (std::size_t i = 0; i < 8; i++)
			{
				pPipelineDesc.BlendAttribute.RenderTargets[i].bBlendEnable = true;
				pPipelineDesc.BlendAttribute.RenderTargets[i].ColorSrcBlend = EBlendFactor::eSourceAlpha;
				pPipelineDesc.BlendAttribute.RenderTargets[i].ColorDestBlend = EBlendFactor::eInverseSourceAlpha;
				pPipelineDesc.BlendAttribute.RenderTargets[i].AlphaDestBlend = EBlendFactor::eSourceAlpha;
			}
			pPipelineDesc.BlendAttribute.bUseIndependentRenderTargetBlendStates = true;

			pPipelineDesc.DepthStencilAttribute = sDepthStencilAttributeDesc();
			pPipelineDesc.DepthStencilAttribute.bEnableDepthWrite = false;
			pPipelineDesc.DepthStencilAttribute.bDepthWriteMask = false;
			pPipelineDesc.DepthStencilAttribute.bStencilEnable = true;
			pPipelineDesc.DepthStencilAttribute.DepthTest = ECompareFunction::eAlways;
			pPipelineDesc.DepthStencilAttribute.FrontFacePassStencilOp = EStencilOp::eKeep;
			pPipelineDesc.DepthStencilAttribute.FrontFaceStencilTest = ECompareFunction::eEqual;
			pPipelineDesc.DepthStencilAttribute.BackFacePassStencilOp = EStencilOp::eKeep;
			pPipelineDesc.DepthStencilAttribute.BackFaceStencilTest = ECompareFunction::eEqual;
			pPipelineDesc.PrimitiveTopologyType = EPrimitiveType::eTRIANGLE_LIST;
			pPipelineDesc.RasterizerAttribute = sRasterizerAttributeDesc();

			std::vector<sVertexAttributeDesc> VertexLayout =
			{
				{ "POSITION",	EFormat::RGBA32_FLOAT, 0,	offsetof(cbgui::cbGeometryVertexData, position),   false, sizeof(cbgui::cbGeometryVertexData) },
				{ "TEXCOORD",	EFormat::RG32_FLOAT,   0,	offsetof(cbgui::cbGeometryVertexData, texCoord),   false, sizeof(cbgui::cbGeometryVertexData) },
				{ "COLOR",		EFormat::RGBA32_FLOAT, 0,	offsetof(cbgui::cbGeometryVertexData, Color),	   false, sizeof(cbgui::cbGeometryVertexData) },
			};
			pPipelineDesc.VertexLayout = VertexLayout;

			pPipelineDesc.DescriptorSetLayout.clear();
			pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eUniformBuffer, eShaderType::Vertex, 13));	// Model CB

			pPipelineDesc.ShaderAttachments.clear();
			pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment((void*)WidgetBaseVS.data(), WidgetBaseVS.length(), "GeometryVS", eShaderType::Vertex));
			pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment((void*)WidgetBasePS_Flat.data(), WidgetBasePS_Flat.length(), "WidgetFlatColorPS", eShaderType::Pixel));

			auto DefaultGUIMat = sMaterial::Create("Default_GUI_Mat", EMaterialBlendMode::Opaque, pPipelineDesc);
			sMaterialManager::Get().StoreMaterial(DefaultGUIMat);

			{
				sMaterial::sMaterialInstance::SharedPtr FlatColorInstance = DefaultGUIMat->CreateInstance("Default_GUI_MatInstance");
				FlatColorInstance = nullptr;
			}

			pPipelineDesc.DepthStencilAttribute.DepthTest = ECompareFunction::eAlways;
			pPipelineDesc.DepthStencilAttribute.FrontFaceStencilTest = ECompareFunction::eEqual;
			pPipelineDesc.DepthStencilAttribute.BackFaceStencilTest = ECompareFunction::eEqual;

			sSamplerAttributeDesc sampler(ESamplerStateMode::ePointBorder);
			sampler.BorderColor = FColor::Transparent();

			pPipelineDesc.DescriptorSetLayout.clear();
			pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eUniformBuffer, eShaderType::Vertex, 13));	// Model CB
			pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eTexture, eShaderType::Pixel, 0));
			pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(sampler, eShaderType::Pixel, 0));

			pPipelineDesc.ShaderAttachments.clear();
			pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment((void*)WidgetBaseVS.data(), WidgetBaseVS.length(), "GeometryVS", eShaderType::Vertex));
			pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment((void*)WidgetBasePS_Font.data(), WidgetBasePS_Font.length(), "FontPS", eShaderType::Pixel));

			auto DefaultGUIFontMaterial = sMaterial::Create("Default_Font_Mat", EMaterialBlendMode::Opaque, pPipelineDesc);
			sMaterialManager::Get().StoreMaterial(DefaultGUIFontMaterial);
			sMaterial::sMaterialInstance::SharedPtr FontInstance = DefaultGUIFontMaterial->CreateInstance("Default_Font_Mat_Instance");

			cbgui::cbFontDesc FontDesc(cbgui::cbFontDesc("DejaVu Sans"));

			FontDesc.FontSize = 24;
			//FontDesc.DefaultSpaceWidth = 3;
			FontDesc.DefaultFontLocation = "c:/Windows/Fonts/";
			FontDesc.Fonts.insert({ cbgui::eFontType::Regular, cbgui::cbFontDesc::cbFontLoadDesc("DejaVuSans.ttf") });
			FontDesc.Fonts.insert({ cbgui::eFontType::Bold, cbgui::cbFontDesc::cbFontLoadDesc("DejaVuSans-Bold.ttf") });
			FontDesc.Fonts.insert({ cbgui::eFontType::BoldItalic,cbgui::cbFontDesc::cbFontLoadDesc("DejaVuSans-BoldOblique.ttf") });
			FontDesc.Fonts.insert({ cbgui::eFontType::Italic,cbgui::cbFontDesc::cbFontLoadDesc("DejaVuSans-Oblique.ttf") });
			FontDesc.Fonts.insert({ cbgui::eFontType::Light, cbgui::cbFontDesc::cbFontLoadDesc("DejaVuSans-ExtraLight.ttf") });

			/*auto hDC = CreateDC(L"DISPLAY", NULL, NULL, NULL);
			LOGFONT logFont = {};
			logFont.lfWeight = 700;
			wcscpy_s(logFont.lfFaceName, L"arial");
			HFONT hFont = CreateFontIndirect(&logFont);
			if (!hFont)
			{
				fwprintf(stderr, L"ERROR: Could not create font\n");
			}
			if (!SelectObject(hDC, hFont))
			{
				fwprintf(stderr, L"ERROR: Could not select object\n");
			}

			auto Len = GetFontData(hDC, 0, 0, NULL, 0);
			auto hGlobal = GlobalAlloc(GMEM_MOVEABLE, Len);

			void* ptr = GlobalLock(hGlobal);
			if (GetFontData(hDC, 0, 0, ptr, Len) == GDI_ERROR)
			{
				fwprintf(stderr, L"ERROR: Could not get font data\n");
			}

			GlobalUnlock(hGlobal);

			unsigned char* pFontData = nullptr;
			DWORD NAME = 0x66637474;
			std::size_t pFontSize = 0;
			FontDesc.Fonts.insert({ eFontType::Regular, cbFontDesc::cbFontTypeDesc((unsigned char*)ptr, Len) });*/

			//FontDesc.AtlasHeight = 4096;
			//FontDesc.AtlasWidth = 4096;
			//FontDesc.Numchars = 256;
			//FontDesc.LightItalicFontLocation = "..//Content//";
			//FontDesc.SDF = true;
			/*FontDesc.fFontTextureUpdate_Callback = [&](const void* Texture, std::size_t RowPitch, std::size_t MinX, std::size_t MinY, std::size_t MaxX, std::size_t MaxY)
			{
				DefaultGUIFontMaterial->GetInstance("Default_Font_Mat_Instance")->UpdateTexture(0, Texture, RowPitch, MinX, MinY, MaxX, MaxY);
			};*/

			FontDesc.fFontTextureUpdate_Callback = std::bind(&fFontTextureUpdate_Callback, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6);
			cbFontResources::Get().AddFreeTypeFont(FontDesc);

			auto pFont = cbFontResources::Get().GetFontFamily("DejaVu Sans");
			auto Atlas = pFont->GetTexture();

			sTextureDesc Desc;
			Desc.Dimensions.X = pFont->GetDesc().AtlasWidth;
			Desc.Dimensions.Y = pFont->GetDesc().AtlasHeight;
			Desc.MipLevels = 1;
			Desc.Format = EFormat::R8_UNORM;
			FontInstance->AddTexture("Font_Mat", &Atlas[0], 0, Desc, 1);
		}

		{
			sPipelineDesc pPipelineDesc;
			pPipelineDesc.BlendAttribute = sBlendAttributeDesc(EBlendStateMode::eOpaque);
			for (std::size_t i = 0; i < 8; i++)
			{
				pPipelineDesc.BlendAttribute.RenderTargets[i].bBlendEnable = true;
				pPipelineDesc.BlendAttribute.RenderTargets[i].ColorSrcBlend = EBlendFactor::eSourceAlpha;
				pPipelineDesc.BlendAttribute.RenderTargets[i].ColorDestBlend = EBlendFactor::eInverseSourceAlpha;
				pPipelineDesc.BlendAttribute.RenderTargets[i].AlphaDestBlend = EBlendFactor::eSourceAlpha;
			}
			pPipelineDesc.BlendAttribute.bUseIndependentRenderTargetBlendStates = true;

			pPipelineDesc.DepthStencilAttribute = sDepthStencilAttributeDesc();
			pPipelineDesc.DepthStencilAttribute.bEnableDepthWrite = false;
			pPipelineDesc.DepthStencilAttribute.bDepthWriteMask = false;
			pPipelineDesc.DepthStencilAttribute.bStencilEnable = true;
			pPipelineDesc.DepthStencilAttribute.DepthTest = ECompareFunction::eAlways;
			pPipelineDesc.DepthStencilAttribute.FrontFacePassStencilOp = EStencilOp::eKeep;
			pPipelineDesc.DepthStencilAttribute.FrontFaceStencilTest = ECompareFunction::eEqual;
			pPipelineDesc.DepthStencilAttribute.BackFacePassStencilOp = EStencilOp::eKeep;
			pPipelineDesc.DepthStencilAttribute.BackFaceStencilTest = ECompareFunction::eEqual;
			pPipelineDesc.PrimitiveTopologyType = EPrimitiveType::eTRIANGLE_LIST;
			pPipelineDesc.RasterizerAttribute = sRasterizerAttributeDesc();

			std::vector<sVertexAttributeDesc> VertexLayout =
			{
				{ "POSITION",	EFormat::RGBA32_FLOAT, 0,	offsetof(cbgui::cbGeometryVertexData, position),   false, sizeof(cbgui::cbGeometryVertexData) },
				{ "TEXCOORD",	EFormat::RG32_FLOAT,   0,	offsetof(cbgui::cbGeometryVertexData, texCoord),   false, sizeof(cbgui::cbGeometryVertexData) },
				{ "COLOR",		EFormat::RGBA32_FLOAT, 0,	offsetof(cbgui::cbGeometryVertexData, Color),	   false, sizeof(cbgui::cbGeometryVertexData) },
			};
			pPipelineDesc.VertexLayout = VertexLayout;

			pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eUniformBuffer, eShaderType::Vertex, 13));	// Model CB
			pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eUniformBuffer, eShaderType::Pixel, 11));

			pPipelineDesc.ShaderAttachments.clear();
			pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment((void*)WidgetBaseVS.data(), WidgetBaseVS.length(), "GeometryVS", eShaderType::Vertex));
			pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment((void*)WidgetBasePS_Gradient.data(), WidgetBasePS_Gradient.length(), "GradientPS", eShaderType::Pixel));

			auto DefaultGUI_GradientMat = sMaterial::Create("Default_GUI_GradientMat", EMaterialBlendMode::Opaque, pPipelineDesc);
			sMaterialManager::Get().StoreMaterial(DefaultGUI_GradientMat);
		}

		{
			sPipelineDesc pPipelineDesc;
			pPipelineDesc.BlendAttribute = sBlendAttributeDesc(EBlendStateMode::eOpaque);
			for (std::size_t i = 0; i < 8; i++)
			{
				pPipelineDesc.BlendAttribute.RenderTargets[i].bBlendEnable = true;
				pPipelineDesc.BlendAttribute.RenderTargets[i].ColorSrcBlend = EBlendFactor::eSourceAlpha;
				pPipelineDesc.BlendAttribute.RenderTargets[i].ColorDestBlend = EBlendFactor::eInverseSourceAlpha;
				pPipelineDesc.BlendAttribute.RenderTargets[i].AlphaDestBlend = EBlendFactor::eSourceAlpha;
			}
			pPipelineDesc.BlendAttribute.bUseIndependentRenderTargetBlendStates = true;

			pPipelineDesc.DepthStencilAttribute = sDepthStencilAttributeDesc();
			pPipelineDesc.DepthStencilAttribute.bEnableDepthWrite = false;
			pPipelineDesc.DepthStencilAttribute.bDepthWriteMask = false;
			pPipelineDesc.DepthStencilAttribute.bStencilEnable = true;
			pPipelineDesc.DepthStencilAttribute.DepthTest = ECompareFunction::eAlways;
			pPipelineDesc.DepthStencilAttribute.FrontFacePassStencilOp = EStencilOp::eKeep;
			pPipelineDesc.DepthStencilAttribute.FrontFaceStencilTest = ECompareFunction::eEqual;
			pPipelineDesc.DepthStencilAttribute.BackFacePassStencilOp = EStencilOp::eKeep;
			pPipelineDesc.DepthStencilAttribute.BackFaceStencilTest = ECompareFunction::eEqual;
			pPipelineDesc.PrimitiveTopologyType = EPrimitiveType::eTRIANGLE_LIST;
			pPipelineDesc.RasterizerAttribute = sRasterizerAttributeDesc();

			std::vector<sVertexAttributeDesc> VertexLayout =
			{
				{ "POSITION",	EFormat::RGBA32_FLOAT, 0,	offsetof(cbgui::cbGeometryVertexData, position),   false, sizeof(cbgui::cbGeometryVertexData) },
				{ "TEXCOORD",	EFormat::RG32_FLOAT,   0,	offsetof(cbgui::cbGeometryVertexData, texCoord),   false, sizeof(cbgui::cbGeometryVertexData) },
				{ "COLOR",		EFormat::RGBA32_FLOAT, 0,	offsetof(cbgui::cbGeometryVertexData, Color),	   false, sizeof(cbgui::cbGeometryVertexData) },
			};
			pPipelineDesc.VertexLayout = VertexLayout;

			pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eUniformBuffer, eShaderType::Vertex, 13));	// Model CB
			pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eTexture, eShaderType::Pixel, 0));

			sSamplerAttributeDesc sampler(ESamplerStateMode::ePointWrap);
			pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(sampler, eShaderType::Pixel, 0));

			pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment((void*)WidgetBaseVS.data(), WidgetBaseVS.length(), "GeometryVS", eShaderType::Vertex));
			pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(L"..//Content\\Shaders\\GBufferPS.hlsl", "DefaultTexturedGUIPS", eShaderType::Pixel));

			auto DefaultActorMat = sMaterial::Create("Default_GUI_TexturedMaterial", EMaterialBlendMode::Opaque, pPipelineDesc);
			sMaterialManager::Get().StoreMaterial(DefaultActorMat);

			{
				ITexture2D::SharedPtr Apple = ITexture2D::Create(L"E:\\VisualStudioProjects\\DNGE\\Content\\Pixel Adventure 1\\Free\\Items\\Fruits\\Apple.png", "Apple", 1);
				auto TextureAppleDesc = Apple->GetDesc();

				auto AppleMatInstance = DefaultActorMat->CreateInstance("AppleMatInstance");
				sTextureDesc Desc;
				Desc.Dimensions.X = 12;
				Desc.Dimensions.Y = 14;
				Desc.Format = TextureAppleDesc.Format;
				ITexture2D::SharedPtr Texture = ITexture2D::CreateEmpty("Apple_" + std::to_string(12) + "x" + std::to_string(14), Desc, 1);
				Texture->UpdateTexture(Apple.get(), 0, 0, IntVector2(0, 0), FBounds2D(FDimension2D(12, 14), FVector2(16, 16)));
				AppleMatInstance->AddTexture(Texture);

				Apple = nullptr;
			}

			{
				ITexture2D::SharedPtr Cherrie = ITexture2D::Create(L"E:\\VisualStudioProjects\\DNGE\\Content\\Pixel Adventure 1\\Free\\Items\\Fruits\\Cherries.png", "Cherrie", 1);
				auto TextureCherrieeDesc = Cherrie->GetDesc();

				auto CherrieMatInstance = DefaultActorMat->CreateInstance("CherrieMatInstance");
				sTextureDesc Desc;
				Desc.Dimensions.X = 12;
				Desc.Dimensions.Y = 14;
				Desc.Format = TextureCherrieeDesc.Format;
				ITexture2D::SharedPtr Texture = ITexture2D::CreateEmpty("Cherrie", Desc, 1);
				Texture->UpdateTexture(Cherrie.get(), 0, 0, IntVector2(0, 0), FBounds2D(FDimension2D(12, 14), FVector2(16, 16)));
				CherrieMatInstance->AddTexture(Texture);

				Cherrie = nullptr;
			}
		}

		//{
		//	sPipelineDesc pPipelineDesc;
		//	pPipelineDesc.BlendAttribute = sBlendAttributeDesc(EBlendStateMode::eNonPremultiplied);
		//	pPipelineDesc.DepthStencilAttribute = sDepthStencilAttributeDesc(true, true);
		//	pPipelineDesc.DepthStencilAttribute.DepthTest = ECompareFunction::eGreaterEqual;
		//	//pPipelineDesc.DepthStencilAttribute.DepthTest = ECompareFunction::eLess;
		//	//pPipelineDesc.DepthStencilAttribute.DepthTest = ECompareFunction::eAlways;
		//	pPipelineDesc.PrimitiveTopologyType = EPrimitiveType::eTRIANGLE_LIST;
		//	pPipelineDesc.RasterizerAttribute = sRasterizerAttributeDesc();

		//	std::vector<sVertexAttributeDesc> VertexLayout =
		//	{
		//		{ "POSITION",		 EFormat::RGB32_FLOAT,   0, offsetof(sParticleVertexLayout, position),	false, sizeof(sParticleVertexLayout) },
		//		{ "TEXCOORD",		 EFormat::RG32_FLOAT,    0, offsetof(sParticleVertexLayout, texCoord),	false, sizeof(sParticleVertexLayout) },
		//		{ "COLOR",			 EFormat::RGBA32_FLOAT,  0, offsetof(sParticleVertexLayout, Color),		false, sizeof(sParticleVertexLayout) },
		//		{ "INSTANCEPOS",	 EFormat::RGB32_FLOAT,	 1, offsetof(sParticleVertexLayout::sParticleInstanceLayout, position),		true, sizeof(sParticleVertexLayout::sParticleInstanceLayout) },
		//		{ "INSTANCECOLOR",	 EFormat::RGBA32_FLOAT,	 1, offsetof(sParticleVertexLayout::sParticleInstanceLayout, Color),		true, sizeof(sParticleVertexLayout::sParticleInstanceLayout) },
		//	};
		//	pPipelineDesc.VertexLayout = VertexLayout;

		//	pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eUniformBuffer, eShaderType::Vertex, 13));	// Model CB
		//	pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eUniformBuffer, eShaderType::Vertex, 12));
		//	pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eTexture, eShaderType::Pixel, 0));
		//	pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eSampler, eShaderType::Pixel, 0));
		//	//pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eUniformBuffer, eShaderType::Pixel, 9));
		//	//pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eTexture, eShaderType::Pixel, 1));
		//	//pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eTexture, eShaderType::Pixel, 2));
		//	//pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eTexture, eShaderType::Pixel, 3));
		//	//pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eTexture, eShaderType::Pixel, 4));

		//	std::vector<sShaderAttachment> ShaderAttachments;
		//	pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(L"..//Content\\Shaders\\Particle.hlsl", "ParticleVS", eShaderType::Vertex));
		//	pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(L"..//Content\\Shaders\\Particle.hlsl", "ParticlePS", eShaderType::Pixel));

		//	sMaterial::SharedPtr ParticleMat;
		//	ParticleMat = sMaterial::Create("ParticleMat", EMaterialBlendMode::Opaque, pPipelineDesc);
		//	//DefaultEngineMat->BindConstantBuffer(CameraCB);
		//	auto DefaultParticle_MatInstance = ParticleMat->CreateInstance("ParticleMat_MatInstance");
		//	DefaultParticle_MatInstance->AddTexture(L"..//Content\\smoke-particle.png", "ParticleMat_Texture", 2);
		//	//DefaultParticle_MatInstance->AddTexture(L"..//Content\\Textures\\DefaultWhiteGrid.DDS", "DefaultEngineTexture", 2);
		//	sMaterialManager::Get().StoreMaterial(ParticleMat);
		//}
	};
}
