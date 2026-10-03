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
#include "Utilities/FileManager.h"

namespace GameMaterials
{
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
			sPipelineDesc pPipelineDesc = sPipelineDesc::CreateDefaultPipelineDesc(ERenderPass::GBuffer, EBlendStateMode::Opaque, ECompareFunction::LessEqual, true, EVertexLayoutType::DefaultVertexLayout, false, GPU::GetRenderPassIndirectLayoutBindingDesc(ERenderPass::GBuffer));

			pPipelineDesc.Bindings.push_back(sShaderBinding(EDescriptorType::e32BitConstant, eShaderType::All, 0, 10));

			pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(L"..//Content\\Shaders\\GBufferVS.hlsl", "GeometryVS", eShaderType::Vertex));
			pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(L"..//Content\\Shaders\\GBufferPS.hlsl", "GeometryBackgroundPS", eShaderType::Pixel));

			auto DefaultActorMat = sMaterial::Create("BackgoundMat", EMaterialBlendMode::Opaque, pPipelineDesc);
			sMaterialManager::Get().StoreMaterial(DefaultActorMat);

			auto DefaultActorMatInstance = DefaultActorMat->CreateInstance("BackgoundMatInstance");
			DefaultActorMatInstance->AddTexture(ITexture2D::Create(L"E:\\VisualStudioProjects\\DNGE\\Content\\Pixel Adventure 1\\Free\\Background\\Pink.png", "Backgound", 3/* GPU::GetGBufferTextureEntryPoint()*/));

			sSamplerAttributeDesc sampler(ESamplerStateMode::PointBorder);
			sampler.Filter = ESamplerFilter::Point;
			sampler.AddressU = ESamplerAddressMode::Wrap;
			sampler.AddressV = ESamplerAddressMode::Wrap;
			sampler.AddressW = ESamplerAddressMode::Wrap;
			sampler.MipBias = 0;
			sampler.MinMipLevel = -FLT_MAX;
			sampler.MaxMipLevel = FLT_MAX;
			sampler.MaxAnisotropy = 1;
			sampler.BorderColor = FColor::Transparent();
			DefaultActorMatInstance->AddSampler(sampler);
		}
		{
			sPipelineDesc pPipelineDesc = sPipelineDesc::CreateDefaultPipelineDesc(ERenderPass::GBuffer, EBlendStateMode::Opaque, ECompareFunction::LessEqual, true, EVertexLayoutType::DefaultVertexLayout, true, GPU::GetRenderPassIndirectLayoutBindingDesc(ERenderPass::GBuffer));

			pPipelineDesc.Bindings.push_back(sShaderBinding(EDescriptorType::e32BitConstant, eShaderType::All, 0, 10));

			pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(L"..//Content\\Shaders\\GBufferVS.hlsl", "GeometryInstanceVS", eShaderType::Vertex));
			pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(L"..//Content\\Shaders\\GBufferPS.hlsl", "GeometryPS", eShaderType::Pixel));

			auto DefaultActorMat = sMaterial::Create("DefaultTexturedMaterial", EMaterialBlendMode::Opaque, pPipelineDesc);
			sMaterialManager::Get().StoreMaterial(DefaultActorMat);

			sSamplerAttributeDesc sampler(ESamplerStateMode::PointWrap);

			ITexture2D::SharedPtr TextureAtlas = ITexture2D::Create(L"E:\\VisualStudioProjects\\DNGE\\Content\\Pixel Adventure 1\\Free\\Terrain\\Terrain (16x16).png", "TerrainAtlas", 3/* GPU::GetGBufferTextureEntryPoint()*/);

			auto TextureAtlasDesc = TextureAtlas->GetDesc();

			for (std::uint32_t h = 0; h < 176; h += 16)
			{
				for (std::uint32_t w = 0; w < 352; w += 16)
				{
					auto MatInstance = DefaultActorMat->CreateInstance("TerrainMatInstance_" + std::to_string(w) + "x" + std::to_string(h));
					sTextureDesc Desc;
					Desc.Dimensions.X = 16;
					Desc.Dimensions.Y = 16;
					Desc.Format = TextureAtlasDesc.Format;
					ITexture2D::SharedPtr Texture = ITexture2D::CreateEmpty("Terrain_" + std::to_string(w) + "x" + std::to_string(h), Desc, 4);
					Texture->UpdateTexture(TextureAtlas.get(), 0, 0, IntVector2(0, 0), FBounds2D(FDimension2D(16.0f,16.0f), FVector2(8.0f + w,8.0f + h)));
					MatInstance->AddTexture(Texture);
					MatInstance->AddSampler(sampler);
				}
			}

			TextureAtlas = nullptr;
		}

		{
			sPipelineDesc pPipelineDesc = sPipelineDesc::CreateDefaultPipelineDesc(ERenderPass::GBuffer, EBlendStateMode::NonPremultiplied, ECompareFunction::LessEqual, true, EVertexLayoutType::DefaultVertexLayout, false, GPU::GetRenderPassIndirectLayoutBindingDesc(ERenderPass::GBuffer));

			pPipelineDesc.RasterizerAttribute.CullMode = ERasterizerCullMode::None;
			pPipelineDesc.Bindings.push_back(sShaderBinding(EDescriptorType::e32BitConstant, eShaderType::All, 0, 10));

			pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(L"..//Content\\Shaders\\GBufferVS.hlsl", "GeometryVS", eShaderType::Vertex));
			pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(L"..//Content\\Shaders\\GBufferPS.hlsl", "GeometryAtlasTexturedPS", eShaderType::Pixel));

			auto DefaultActorMat = sMaterial::Create("DefaultActorAtlastMat", EMaterialBlendMode::Masked, pPipelineDesc);
			sMaterialManager::Get().StoreMaterial(DefaultActorMat);

			auto DefaultActorMatInstance = DefaultActorMat->CreateInstance("DefaultActorAtlastMatInstance");

			sSamplerAttributeDesc sampler(ESamplerStateMode::PointBorder);
			sampler.Filter = ESamplerFilter::Point;
			sampler.AddressU = ESamplerAddressMode::Clamp;
			sampler.AddressV = ESamplerAddressMode::Clamp;
			sampler.AddressW = ESamplerAddressMode::Clamp;
			sampler.MipBias = 0;
			sampler.MinMipLevel = -FLT_MAX;
			sampler.MaxMipLevel = FLT_MAX;
			sampler.MaxAnisotropy = 1;
			sampler.BorderColor = FColor::Transparent();
			DefaultActorMatInstance->AddSampler(sampler);
		}

		{
			sPipelineDesc pPipelineDesc;
			pPipelineDesc.BlendAttribute = sBlendAttributeDesc(EBlendStateMode::Opaque);
			for (std::uint32_t i = 0; i < 8; i++)
			{
				pPipelineDesc.BlendAttribute.RenderTargets[i].bBlendEnable = true;
				pPipelineDesc.BlendAttribute.RenderTargets[i].ColorSrcBlend = EBlendFactor::SourceAlpha;
				pPipelineDesc.BlendAttribute.RenderTargets[i].ColorDestBlend = EBlendFactor::InverseSourceAlpha;
				pPipelineDesc.BlendAttribute.RenderTargets[i].AlphaDestBlend = EBlendFactor::SourceAlpha;
			}
			pPipelineDesc.BlendAttribute.bUseIndependentRenderTargetBlendStates = true;

			pPipelineDesc.DepthStencilAttribute = sDepthStencilAttributeDesc();
			pPipelineDesc.DepthStencilAttribute.bEnableDepthWrite = false;
			pPipelineDesc.DepthStencilAttribute.bDepthWriteMask = false;
			pPipelineDesc.DepthStencilAttribute.bStencilEnable = true;
			pPipelineDesc.DepthStencilAttribute.DepthTest = ECompareFunction::Always;
			pPipelineDesc.DepthStencilAttribute.FrontFacePassStencilOp = EStencilOp::Keep;
			pPipelineDesc.DepthStencilAttribute.FrontFaceStencilTest = ECompareFunction::Equal;
			pPipelineDesc.DepthStencilAttribute.BackFacePassStencilOp = EStencilOp::Keep;
			pPipelineDesc.DepthStencilAttribute.BackFaceStencilTest = ECompareFunction::Equal;
			pPipelineDesc.PrimitiveTopologyType = EPrimitiveType::TRIANGLE_LIST;
			pPipelineDesc.RasterizerAttribute = sRasterizerAttributeDesc();

			pPipelineDesc.VertexLayout = sVertexAttributeDesc::GetDefaultGUIVertexLayout(false);

			pPipelineDesc.Bindings.clear();
			pPipelineDesc.Bindings.push_back(sShaderBinding(EDescriptorType::e32BitConstant, eShaderType::All, 0, 4));	// Model CB

			pPipelineDesc.ShaderAttachments.clear();
			pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(FileManager::GetShaderFolderW() + L"GUI.hlsl", "GUIGeometryVS", eShaderType::Vertex));
			pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(FileManager::GetShaderFolderW() + L"GUI.hlsl", "WidgetFlatColorPS", eShaderType::Pixel));

			auto DefaultGUIMat = sMaterial::Create("Default_GUI_Mat", EMaterialBlendMode::Opaque, pPipelineDesc);
			sMaterialManager::Get().StoreMaterial(DefaultGUIMat);

			{
				sMaterialInstance::SharedPtr FlatColorInstance = DefaultGUIMat->CreateInstance("Default_GUI_MatInstance");
				FlatColorInstance = nullptr;
			}

			pPipelineDesc.DepthStencilAttribute.DepthTest = ECompareFunction::Always;
			pPipelineDesc.DepthStencilAttribute.FrontFaceStencilTest = ECompareFunction::Equal;
			pPipelineDesc.DepthStencilAttribute.BackFaceStencilTest = ECompareFunction::Equal;

			sSamplerAttributeDesc sampler(ESamplerStateMode::PointBorder);
			sampler.BorderColor = FColor::Transparent();

			pPipelineDesc.Bindings.clear();
			pPipelineDesc.Bindings.push_back(sShaderBinding(EDescriptorType::e32BitConstant, eShaderType::All, 0, 4));
			pPipelineDesc.ShaderAttachments.clear();
			pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(FileManager::GetShaderFolderW() + L"GUI.hlsl", "GUIGeometryVS", eShaderType::Vertex));
			pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(FileManager::GetShaderFolderW() + L"GUI.hlsl", "FontPS", eShaderType::Pixel));

			auto DefaultGUIFontMaterial = sMaterial::Create("Default_Font_Mat", EMaterialBlendMode::Opaque, pPipelineDesc);
			sMaterialManager::Get().StoreMaterial(DefaultGUIFontMaterial);
			sMaterialInstance::SharedPtr FontInstance = DefaultGUIFontMaterial->CreateInstance("Default_Font_Mat_Instance");
			FontInstance->AddSampler(sampler);

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
			std::uint32_t pFontSize = 0;
			FontDesc.Fonts.insert({ eFontType::Regular, cbFontDesc::cbFontTypeDesc((unsigned char*)ptr, Len) });*/

			//FontDesc.AtlasHeight = 4096;
			//FontDesc.AtlasWidth = 4096;
			//FontDesc.Numchars = 256;
			//FontDesc.LightItalicFontLocation = "..//Content//";
			//FontDesc.SDF = true;
			/*FontDesc.fFontTextureUpdate_Callback = [&](const void* Texture, std::uint32_t RowPitch, std::uint32_t MinX, std::uint32_t MinY, std::uint32_t MaxX, std::uint32_t MaxY)
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
			pPipelineDesc.BlendAttribute = sBlendAttributeDesc(EBlendStateMode::Opaque);
			for (std::uint32_t i = 0; i < 8; i++)
			{
				pPipelineDesc.BlendAttribute.RenderTargets[i].bBlendEnable = true;
				pPipelineDesc.BlendAttribute.RenderTargets[i].ColorSrcBlend = EBlendFactor::SourceAlpha;
				pPipelineDesc.BlendAttribute.RenderTargets[i].ColorDestBlend = EBlendFactor::InverseSourceAlpha;
				pPipelineDesc.BlendAttribute.RenderTargets[i].AlphaDestBlend = EBlendFactor::SourceAlpha;
			}
			pPipelineDesc.BlendAttribute.bUseIndependentRenderTargetBlendStates = true;

			pPipelineDesc.DepthStencilAttribute = sDepthStencilAttributeDesc();
			pPipelineDesc.DepthStencilAttribute.bEnableDepthWrite = false;
			pPipelineDesc.DepthStencilAttribute.bDepthWriteMask = false;
			pPipelineDesc.DepthStencilAttribute.bStencilEnable = true;
			pPipelineDesc.DepthStencilAttribute.DepthTest = ECompareFunction::Always;
			pPipelineDesc.DepthStencilAttribute.FrontFacePassStencilOp = EStencilOp::Keep;
			pPipelineDesc.DepthStencilAttribute.FrontFaceStencilTest = ECompareFunction::Equal;
			pPipelineDesc.DepthStencilAttribute.BackFacePassStencilOp = EStencilOp::Keep;
			pPipelineDesc.DepthStencilAttribute.BackFaceStencilTest = ECompareFunction::Equal;
			pPipelineDesc.PrimitiveTopologyType = EPrimitiveType::TRIANGLE_LIST;
			pPipelineDesc.RasterizerAttribute = sRasterizerAttributeDesc();

			pPipelineDesc.VertexLayout = sVertexAttributeDesc::GetDefaultGUIVertexLayout(false);

			pPipelineDesc.Bindings.push_back(sShaderBinding(EDescriptorType::e32BitConstant, eShaderType::All, 0, 4));

			pPipelineDesc.ShaderAttachments.clear();
			pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(FileManager::GetShaderFolderW() + L"GUI.hlsl", "GUIGeometryVS", eShaderType::Vertex));
			pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(FileManager::GetShaderFolderW() + L"GUI.hlsl", "GradientPS", eShaderType::Pixel));

			auto DefaultGUI_GradientMat = sMaterial::Create("Default_GUI_GradientMat", EMaterialBlendMode::Opaque, pPipelineDesc);
			sMaterialManager::Get().StoreMaterial(DefaultGUI_GradientMat);
		}

		{
			sPipelineDesc pPipelineDesc;
			pPipelineDesc.BlendAttribute = sBlendAttributeDesc(EBlendStateMode::Opaque);
			for (std::uint32_t i = 0; i < 8; i++)
			{
				pPipelineDesc.BlendAttribute.RenderTargets[i].bBlendEnable = true;
				pPipelineDesc.BlendAttribute.RenderTargets[i].ColorSrcBlend = EBlendFactor::SourceAlpha;
				pPipelineDesc.BlendAttribute.RenderTargets[i].ColorDestBlend = EBlendFactor::InverseSourceAlpha;
				pPipelineDesc.BlendAttribute.RenderTargets[i].AlphaDestBlend = EBlendFactor::SourceAlpha;
			}
			pPipelineDesc.BlendAttribute.bUseIndependentRenderTargetBlendStates = true;

			pPipelineDesc.DepthStencilAttribute = sDepthStencilAttributeDesc();
			pPipelineDesc.DepthStencilAttribute.bEnableDepthWrite = false;
			pPipelineDesc.DepthStencilAttribute.bDepthWriteMask = false;
			pPipelineDesc.DepthStencilAttribute.bStencilEnable = true;
			pPipelineDesc.DepthStencilAttribute.DepthTest = ECompareFunction::Always;
			pPipelineDesc.DepthStencilAttribute.FrontFacePassStencilOp = EStencilOp::Keep;
			pPipelineDesc.DepthStencilAttribute.FrontFaceStencilTest = ECompareFunction::Equal;
			pPipelineDesc.DepthStencilAttribute.BackFacePassStencilOp = EStencilOp::Keep;
			pPipelineDesc.DepthStencilAttribute.BackFaceStencilTest = ECompareFunction::Equal;
			pPipelineDesc.PrimitiveTopologyType = EPrimitiveType::TRIANGLE_LIST;
			pPipelineDesc.RasterizerAttribute = sRasterizerAttributeDesc();

			pPipelineDesc.VertexLayout = sVertexAttributeDesc::GetDefaultGUIVertexLayout(false);

			pPipelineDesc.Bindings.push_back(sShaderBinding(EDescriptorType::e32BitConstant, eShaderType::All, 0, 4));

			sSamplerAttributeDesc sampler(ESamplerStateMode::PointWrap);

			pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(FileManager::GetShaderFolderW() + L"GUI.hlsl", "GUIGeometryVS", eShaderType::Vertex));
			pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(FileManager::GetShaderFolderW() + L"GUI.hlsl", "DefaultTexturedGUIPS", eShaderType::Pixel));
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
				AppleMatInstance->AddSampler(sampler);

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
				CherrieMatInstance->AddSampler(sampler);

				Cherrie = nullptr;
			}
		}

		//{
		//	sPipelineDesc pPipelineDesc = sPipelineDesc::CreateDefaultPipelineDesc(ERenderPass::Particle, EBlendStateMode::NonPremultiplied, ECompareFunction::GreaterEqual, true, EVertexLayoutType::Particle, true, GPU::GetRenderPassIndirectLayoutBindingDesc(ERenderPass::Particle));

		//	pPipelineDesc.Bindings.push_back(sShaderBinding(EDescriptorType::e32BitConstant, eShaderType::All, 0, 8));

		//	std::vector<sShaderAttachment> ShaderAttachments;
		//	pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(L"..//Content\\Shaders\\Particle.hlsl", "Particle2DVS", eShaderType::Vertex));
		//	pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(L"..//Content\\Shaders\\Particle.hlsl", "ParticlePS", eShaderType::Pixel));

		//	sMaterial::SharedPtr ParticleMat;
		//	ParticleMat = sMaterial::Create("ParticleMat", EMaterialBlendMode::Opaque, pPipelineDesc);
		//	//DefaultEngineMat->BindConstantBuffer(CameraCB);
		//	auto DefaultParticle_MatInstance = ParticleMat->CreateInstance("ParticleMat_MatInstance");
		//	DefaultParticle_MatInstance->AddTexture(L"..//Content\\smoke-particle.png", "ParticleMat_Texture", 2);

		//	sSamplerAttributeDesc sampler(ESamplerStateMode::PointClamp);
		//	DefaultParticle_MatInstance->AddSampler(sampler);

		//	//DefaultParticle_MatInstance->AddTexture(L"..//Content\\Textures\\DefaultWhiteGrid.DDS", "DefaultEngineTexture", 2);
		//	sMaterialManager::Get().StoreMaterial(ParticleMat);
		//}
	};
}
