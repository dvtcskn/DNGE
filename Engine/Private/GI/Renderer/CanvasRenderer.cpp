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
#include <functional>
#include <cbgui.h>
#include "Core/Math/CoreMath.h"
#include "CanvasRenderer.h"
#include "Renderer.h"

__declspec(align(256)) struct OnScreenWidgetMatrix
{
	FMatrix Matrix;
	FMatrix Offset[3];
};
static_assert((sizeof(OnScreenWidgetMatrix) % 256) == 0, "Constant Buffer size must be 256-byte aligned");

sCanvasRenderer::sCanvasRenderer(std::size_t Width, std::size_t Height)
	: Super()
	, LastMaterial(nullptr)
	, CMD(IGraphicsCommandContext::Create())
	, bShowLines(false)
	, bEnableStencilClipping(true)
	, ScreenDimension(sScreenDimension(Width, Height))
{
	std::string DepthTestVSShader = "														\
							cbuffer UICBuffer : register(b0)						\
							{														\
								matrix WidgetMatrix;								\
							}														\
																					\
							float4 mainVS(float4 pos : POSITION) : SV_POSITION		\
							{														\
								float4 position = float4(pos.xyz, 1.0f);			\
								position = mul(position, WidgetMatrix);				\
																					\
								position.z = 0.0f;									\
								position.w = 1.0f;									\
																					\
								return position;									\
							}														\
																					\
							float4 mainPS() : SV_TARGET								\
							{														\
								return float4(1.0f, 1.0f, 1.0f, 0.0f);				\
							}";

	std::string WidgetBaseVS = "															\
							cbuffer UICBuffer : register(b0)						\
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

	std::string WidgetBasePS_Flat = "															\
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

	std::string WidgetBasePS_FlatBlack = "												\
							struct GeometryVSOut										\
							{															\
								float4 position : SV_Position;							\
								float2 texCoord : TEXCOORD;								\
								float4 Color : COLOR;									\
							};															\
																						\
							float4 WidgetFlatColorPS(GeometryVSOut Input) : SV_TARGET	\
							{															\
								return float4(0.0f, 0.0f, 0.0f, 1.0f);					\
							}";
	{
		sPipelineDesc pPipelineDesc;
		pPipelineDesc.BlendAttribute = sBlendAttributeDesc();
		pPipelineDesc.DepthStencilAttribute = sDepthStencilAttributeDesc();
		pPipelineDesc.DepthStencilAttribute.DepthTest = ECompareFunction::eAlways;
		pPipelineDesc.PrimitiveTopologyType = EPrimitiveType::eLINE_LIST;
		pPipelineDesc.RasterizerAttribute = sRasterizerAttributeDesc();
		pPipelineDesc.RasterizerAttribute.bEnableLineAA = true;
		pPipelineDesc.RasterizerAttribute.CullMode = ERasterizerCullMode::eNone;

		pPipelineDesc.NumRenderTargets = 1;
		pPipelineDesc.RTVFormats[0] = EFormat::BGRA8_UNORM;
		pPipelineDesc.DSVFormat = GPU::GetDefaultDepthFormat();

		std::vector<sVertexAttributeDesc> VertexLayout =
		{
			{ "POSITION",	EFormat::RGBA32_FLOAT, 0,	offsetof(cbgui::cbGeometryVertexData, position),   false },
			{ "TEXCOORD",	EFormat::RG32_FLOAT,   0,	offsetof(cbgui::cbGeometryVertexData, texCoord),   false },
			{ "COLOR",		EFormat::RGBA32_FLOAT, 0,	offsetof(cbgui::cbGeometryVertexData, Color),	   false },
		};
		pPipelineDesc.VertexLayout = VertexLayout;

		pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eUniformBuffer, eShaderType::Vertex, 0));

		std::vector<sShaderAttachment> ShaderAttachments;
		pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment((void*)WidgetBaseVS.data(), WidgetBaseVS.length(), "GeometryVS", eShaderType::Vertex));
		pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment((void*)WidgetBasePS_FlatBlack.data(), WidgetBasePS_FlatBlack.length(), "WidgetFlatColorPS", eShaderType::Pixel));

		DefaultUILineMaterial = sMaterial::Create("Line", EMaterialBlendMode::Opaque, pPipelineDesc);

		{
			sMaterial::sMaterialInstance::SharedPtr FlatColorInstance = DefaultUILineMaterial->CreateInstance("LineInstance");
			DefaultLineColorMatStyle = UIMaterialStyle::Create("Image", UIMaterial::Create(FlatColorInstance.get()));
			FlatColorInstance = nullptr;
		}
	}

	Depth = IDepthTarget::CreateUnique("CanvasDepthBuffer", GPU::GetDefaultDepthFormat(), sFBODesc(sFBODesc::sFBODimension((std::uint32_t)ScreenDimension.Width, (std::uint32_t)ScreenDimension.Height)));

	/*{
		sFrameBufferAttachmentInfo AttachmentInfo;
		AttachmentInfo.Desc.Dimensions.X = (std::uint32_t)ScreenDimension.Width;
		AttachmentInfo.Desc.Dimensions.Y = (std::uint32_t)ScreenDimension.Height;
		AttachmentInfo.AddFrameBuffer(GPU::GetBackBufferFormat());
		AttachmentInfo.DepthFormat = GPU::GetDefaultDepthFormat();
		CanvasFBO = IFrameBuffer::Create("Deferred_Backbuffer", AttachmentInfo);
	}*/

	{
		sPipelineDesc pPipelineDesc;
		pPipelineDesc.BlendAttribute = sBlendAttributeDesc();

		pPipelineDesc.DepthStencilAttribute.bEnableDepthWrite = false;
		pPipelineDesc.DepthStencilAttribute.DepthTest = ECompareFunction::eAlways;
		pPipelineDesc.DepthStencilAttribute.bDepthWriteMask = false;
		pPipelineDesc.DepthStencilAttribute.bStencilEnable = true;

		pPipelineDesc.DepthStencilAttribute.FrontFaceStencilFailStencilOp = EStencilOp::eKeep;
		pPipelineDesc.DepthStencilAttribute.FrontFaceDepthFailStencilOp = EStencilOp::eKeep;
		pPipelineDesc.DepthStencilAttribute.FrontFacePassStencilOp = EStencilOp::eReplace;
		pPipelineDesc.DepthStencilAttribute.FrontFaceStencilTest = ECompareFunction::eAlways;

		pPipelineDesc.DepthStencilAttribute.BackFaceStencilFailStencilOp = EStencilOp::eKeep;
		pPipelineDesc.DepthStencilAttribute.BackFaceDepthFailStencilOp = EStencilOp::eKeep;
		pPipelineDesc.DepthStencilAttribute.BackFacePassStencilOp = EStencilOp::eReplace;
		pPipelineDesc.DepthStencilAttribute.BackFaceStencilTest = ECompareFunction::eAlways;

		pPipelineDesc.PrimitiveTopologyType = EPrimitiveType::eTRIANGLE_STRIP;
		pPipelineDesc.RasterizerAttribute = sRasterizerAttributeDesc(ERasterizerCullMode::eCCW);

		std::vector<sVertexAttributeDesc> VertexLayout =
		{
			{ "POSITION", EFormat::RGBA32_FLOAT, 0, 0, false },
		};
		pPipelineDesc.VertexLayout = VertexLayout;

		pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eUniformBuffer, eShaderType::Vertex, 0));

		pPipelineDesc.NumRenderTargets = 0;
		pPipelineDesc.DSVFormat = GPU::GetDefaultDepthFormat();

		pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment((void*)DepthTestVSShader.data(), DepthTestVSShader.length(), "mainVS", eShaderType::Vertex));

		DepthMainPipeline = IPipeline::CreateUnique("DepthMainPipeline", pPipelineDesc);
	}

	{
		sPipelineDesc pPipelineDesc;
		pPipelineDesc.BlendAttribute = sBlendAttributeDesc();

		pPipelineDesc.DepthStencilAttribute.bEnableDepthWrite = false;
		pPipelineDesc.DepthStencilAttribute.DepthTest = ECompareFunction::eAlways;
		pPipelineDesc.DepthStencilAttribute.bDepthWriteMask = false;
		pPipelineDesc.DepthStencilAttribute.bStencilEnable = true;

		pPipelineDesc.DepthStencilAttribute.FrontFaceStencilFailStencilOp = EStencilOp::eKeep;
		pPipelineDesc.DepthStencilAttribute.FrontFaceDepthFailStencilOp = EStencilOp::eKeep;
		pPipelineDesc.DepthStencilAttribute.FrontFacePassStencilOp = EStencilOp::eSaturatedIncrement;
		pPipelineDesc.DepthStencilAttribute.FrontFaceStencilTest = ECompareFunction::eAlways;

		pPipelineDesc.DepthStencilAttribute.BackFaceStencilFailStencilOp = EStencilOp::eKeep;
		pPipelineDesc.DepthStencilAttribute.BackFaceDepthFailStencilOp = EStencilOp::eKeep;
		pPipelineDesc.DepthStencilAttribute.BackFacePassStencilOp = EStencilOp::eSaturatedIncrement;
		pPipelineDesc.DepthStencilAttribute.BackFaceStencilTest = ECompareFunction::eAlways;

		pPipelineDesc.PrimitiveTopologyType = EPrimitiveType::eTRIANGLE_STRIP;
		pPipelineDesc.RasterizerAttribute = sRasterizerAttributeDesc(ERasterizerCullMode::eCCW);

		std::vector<sVertexAttributeDesc> VertexLayout =
		{
			{ "POSITION", EFormat::RGBA32_FLOAT, 0, 0, false },
		};
		pPipelineDesc.VertexLayout = VertexLayout;

		pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eUniformBuffer, eShaderType::Vertex, 0));

		pPipelineDesc.NumRenderTargets = 0;
		pPipelineDesc.DSVFormat = GPU::GetDefaultDepthFormat();

		pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment((void*)DepthTestVSShader.data(), DepthTestVSShader.length(), "mainVS", eShaderType::Vertex));

		DepthPipeline = IPipeline::CreateUnique("DepthPipeline", pPipelineDesc);
	}

	{
		sBufferDesc Desc;
		Desc.Size = sizeof(cbgui::cbVector4) * 4;
		Desc.Stride = sizeof(cbgui::cbVector4);
		DepthVertexBuffer = IVertexBuffer::CreateUnique("DepthVertexBuffer", Desc);
	}

	{
		sBufferDesc BufferDesc;
		BufferDesc.Size = sizeof(OnScreenWidgetMatrix);
		WidgetConstantBuffer = IConstantBuffer::Create("DepthTest", BufferDesc, 0);

		OnScreenWidgetMatrix WidgetMatrix;
		WidgetMatrix.Matrix = (FMatrix&)cbgui::GetViewportTransform((int)ScreenDimension.Width, (int)ScreenDimension.Height);
		WidgetConstantBuffer->Map(&WidgetMatrix);
	}
}

sCanvasRenderer::~sCanvasRenderer()
{
	//CanvasFBO = nullptr;
	CMD = nullptr;
	Depth = nullptr;

	LastMaterial = nullptr;

	WidgetConstantBuffer = nullptr;

	DefaultLineColorMatStyle = nullptr;
	DefaultUILineMaterial = nullptr;

	DepthVertexBuffer = nullptr;
}

void sCanvasRenderer::BeginPlay()
{
}

void sCanvasRenderer::Tick(const double DeltaTime)
{
}

void sCanvasRenderer::Render(ICanvas* Canvas, IRenderTarget* pFB, std::optional<sViewport> Viewport)
{
	using namespace cbgui;

	std::vector<ICanvas::WidgetHierarchy*> DrawLatest;
	std::vector<ICanvas::WidgetHierarchy*> LastInTheHierarchy;

	std::function<void(IVertexBuffer*, IIndexBuffer*, ICanvas::WidgetHierarchy*, std::optional<cbgui::cbIntBounds>, const sViewport&, const eZOrderMode&)> fDraw;
	fDraw = [&](IVertexBuffer* VertexBuffer, IIndexBuffer* IndexBuffer, ICanvas::WidgetHierarchy* Node, std::optional<cbgui::cbIntBounds> ScissorsRect, const sViewport& VP, const eZOrderMode& Mode) -> void
		{
			if (!Node->Widget->IsVisible())
				return;

			//const cbgui::cbIntBounds& Bounds(ScissorsRect.has_value() ? *ScissorsRect : Node->Widget->GetBounds());

			Draw(VertexBuffer, IndexBuffer, Node, Node->Widget->GetCulledBounds()/*Bounds,*/, VP);

			for (const auto& pChild : Node->Nodes)
			{
				if (!pChild->Widget->IsVisible())
				{
					continue;
				}
				if (Mode == eZOrderMode::InOrder && pChild->Widget->GetZOrderMode() == eZOrderMode::Latest)
				{
					DrawLatest.push_back(pChild);
					continue;
				}
				else if (Mode == eZOrderMode::InOrder && pChild->Widget->GetZOrderMode() == eZOrderMode::LastInTheHierarchy)
				{
					LastInTheHierarchy.push_back(pChild);
					continue;
				}
				fDraw(VertexBuffer, IndexBuffer, pChild, std::nullopt,/*cbIntBounds(pChild->Widget->GetBounds()).Crop(Bounds),*/ VP, Mode);
			}
		};

	{
		CMD->BeginRecordCommandList(ERenderPass::eUI);

		sViewport CanvasViewport = Viewport.has_value() ? *Viewport : sViewport(ScreenDimension);

		CMD->SetViewport(CanvasViewport);

		CMD->ClearDepthTarget(Depth.get());
		CMD->SetRenderTarget(pFB, Depth.get());

		{
			IVertexBuffer* VertexBuffer = Canvas->GetVertexBuffer();
			IIndexBuffer* IndexBuffer = Canvas->GetIndexBuffer();
			CMD->SetVertexBuffer(VertexBuffer);
			CMD->SetIndexBuffer(IndexBuffer);

			{
				const std::vector<ICanvas::WidgetHierarchy*>& Widgets = Canvas->GetWidgetHierarchy();
				for (const auto& Widget : Widgets)
				{
					if (!Widget)
						continue;

					if (!Widget->Widget->IsVisible())
						continue;

					if (Widget->Widget->GetZOrderMode() == eZOrderMode::Latest)
					{
						DrawLatest.push_back(Widget);
						continue;
					}
					fDraw(VertexBuffer, IndexBuffer, Widget, std::nullopt, CanvasViewport, eZOrderMode::InOrder);

					if (LastInTheHierarchy.size() > 0)
					{
						for (auto& pWidget : LastInTheHierarchy)
						{
							fDraw(VertexBuffer, IndexBuffer, pWidget, std::nullopt /*pWidget->Widget->GetCulledBounds()*/, CanvasViewport, eZOrderMode::LastInTheHierarchy);
						}
						LastInTheHierarchy.clear();
					}
					/*
					* Required for stencil clipping
					*/
					if (bEnableStencilClipping)
					{
						if (CMD->GetStencilRef() != 0)
							CMD->SetStencilRef(0);
					}
				}
			}

			if (DrawLatest.size() > 0)
			{
				for (auto& Widget : DrawLatest)
				{
					fDraw(VertexBuffer, IndexBuffer, Widget, std::nullopt /*pWidget->Widget->GetCulledBounds()*/, CanvasViewport, eZOrderMode::Latest);
				}
				DrawLatest.clear();
			}
		};

		CMD->FinishRecordCommandList();
		CMD->ExecuteCommandList();
	}
	LastMaterial = nullptr;
}

void sCanvasRenderer::Render(const std::vector<ICanvas*>& Canvases, IRenderTarget* pFB, std::optional<sViewport> Viewport)
{
	using namespace cbgui;

	std::vector<ICanvas::WidgetHierarchy*> DrawLatest;
	std::vector<ICanvas::WidgetHierarchy*> LastInTheHierarchy;

	std::function<void(IVertexBuffer*, IIndexBuffer*, ICanvas::WidgetHierarchy*, std::optional<cbgui::cbIntBounds>, const sViewport&, const eZOrderMode&)> fDraw;
	fDraw = [&](IVertexBuffer* VertexBuffer, IIndexBuffer* IndexBuffer, ICanvas::WidgetHierarchy* Node, std::optional<cbgui::cbIntBounds> ScissorsRect, const sViewport& VP, const eZOrderMode& Mode) -> void
	{
		if (!Node->Widget->IsVisible())
			return;

		//const cbgui::cbIntBounds& Bounds(ScissorsRect.has_value() ? *ScissorsRect : Node->Widget->GetBounds());

		Draw(VertexBuffer, IndexBuffer, Node, Node->Widget->GetCulledBounds()/*Bounds,*/, VP);

		for (const auto& pChild : Node->Nodes)
		{
			if (!pChild->Widget->IsVisible())
			{
				continue;
			}
			if (Mode == eZOrderMode::InOrder && pChild->Widget->GetZOrderMode() == eZOrderMode::Latest)
			{
				DrawLatest.push_back(pChild);
				continue;
			}
			else if (Mode == eZOrderMode::InOrder && pChild->Widget->GetZOrderMode() == eZOrderMode::LastInTheHierarchy)
			{
				LastInTheHierarchy.push_back(pChild);
				continue;
			}
			fDraw(VertexBuffer, IndexBuffer, pChild, std::nullopt,/*cbIntBounds(pChild->Widget->GetBounds()).Crop(Bounds),*/ VP, Mode);
		}
	};

	{
		CMD->BeginRecordCommandList(ERenderPass::eUI);

		sViewport CanvasViewport = Viewport.has_value() ? *Viewport : sViewport(ScreenDimension);

		CMD->SetViewport(CanvasViewport);

		CMD->ClearDepthTarget(Depth.get());
		CMD->SetRenderTarget(pFB, Depth.get());

		for (const auto& Canvas : Canvases)
		{
			IVertexBuffer* VertexBuffer = Canvas->GetVertexBuffer();
			IIndexBuffer* IndexBuffer = Canvas->GetIndexBuffer();
			CMD->SetVertexBuffer(VertexBuffer);
			CMD->SetIndexBuffer(IndexBuffer);

			{
				const std::vector<ICanvas::WidgetHierarchy*>& Widgets = Canvas->GetWidgetHierarchy();
				for (const auto& Widget : Widgets)
				{
					if (!Widget)
						continue;

					if (!Widget->Widget->IsVisible())
						continue;

					if (Widget->Widget->GetZOrderMode() == eZOrderMode::Latest)
					{
						DrawLatest.push_back(Widget);
						continue;
					}
					fDraw(VertexBuffer, IndexBuffer, Widget, std::nullopt, CanvasViewport, eZOrderMode::InOrder);

					if (LastInTheHierarchy.size() > 0)
					{
						for (auto& pWidget : LastInTheHierarchy)
						{
							fDraw(VertexBuffer, IndexBuffer, pWidget, std::nullopt /*pWidget->Widget->GetCulledBounds()*/, CanvasViewport, eZOrderMode::LastInTheHierarchy);
						}
						LastInTheHierarchy.clear();
					}
					/*
					* Required for stencil clipping
					*/
					if (bEnableStencilClipping)
					{
						if (CMD->GetStencilRef() != 0)
							CMD->SetStencilRef(0);
					}
				}
			}

			if (DrawLatest.size() > 0)
			{
				for (auto& Widget : DrawLatest)
				{
					fDraw(VertexBuffer, IndexBuffer, Widget, std::nullopt /*pWidget->Widget->GetCulledBounds()*/, CanvasViewport, eZOrderMode::Latest);
				}
				DrawLatest.clear();
			}
		};

		CMD->FinishRecordCommandList();
		CMD->ExecuteCommandList();
	}
	LastMaterial = nullptr;
}

void sCanvasRenderer::Draw(IVertexBuffer* VertexBuffer, IIndexBuffer* IndexBuffer, ICanvas::WidgetHierarchy* Node, const cbgui::cbIntBounds& ScissorsRect, const sViewport& VP)
{
	if (Node->Widget->HasGeometry())
	{
		if (!Node->MaterialStyle)
			return;

		if (ScissorsRect.IsValid())
		{
			std::uint32_t X = (std::uint32_t)ScreenDimension.Width / VP.Width;
			std::uint32_t Y = (std::uint32_t)ScreenDimension.Height / VP.Height;
			CMD->SetScissorRect((std::uint32_t)VP.TopLeftY + (std::uint32_t)ScissorsRect.GetTop() / Y,
								(std::uint32_t)VP.TopLeftX + (std::uint32_t)ScissorsRect.GetLeft() / X,
								(std::uint32_t)VP.TopLeftY + (std::uint32_t)ScissorsRect.GetBottom() / Y, 
								(std::uint32_t)VP.TopLeftX + (std::uint32_t)ScissorsRect.GetRight() / X);
		}
		else
		{
			return;
		}

		//CMD->SetScissorRect(0, 0, 9999, 9999);

		const auto& GeometryDrawData = Node->Widget->GetGeometryDrawData();
		const auto& pMat = Node->MaterialStyle;
		const auto& pMaterialInstace = pMat->GetMaterial(GeometryDrawData.StyleState)->Material;
		sMaterial* pMaterial = pMaterialInstace->GetParent();
		if (LastMaterial != pMaterial)
		{
			LastMaterial = pMaterial;
			LastMaterial->ApplyMaterial(CMD.get());
			CMD->SetConstantBuffer(WidgetConstantBuffer.get());
		}
		pMaterialInstace->ApplyMaterialInstance(CMD.get());

		if (Node->bVertexDirty || Node->bIndexDirty)
		{
			if (Node->bVertexDirty)
			{
				const auto& Data = Node->Widget->GetVertexData();
				if (Data.size() > 0)
					CMD->UpdateBufferSubresource(VertexBuffer, Node->DrawParams.VertexOffset * sizeof(cbgui::cbGeometryVertexData), Data.size() * sizeof(cbgui::cbGeometryVertexData), Data.data());
				Node->bVertexDirty = false;
			}
			if (Node->bIndexDirty)
			{
				const auto& IndexData = Node->Widget->GetIndexData();
				if (IndexData.size() > 0)
					CMD->UpdateBufferSubresource(IndexBuffer, Node->DrawParams.IndexOffset * sizeof(std::uint32_t), GeometryDrawData.IndexCount * sizeof(std::uint32_t), IndexData.data());
				Node->bIndexDirty = false;
			}
		}
		CMD->DrawIndexedInstanced((std::uint32_t)GeometryDrawData.DrawCount, 1, (std::uint32_t)Node->DrawParams.IndexOffset, (std::uint32_t)Node->DrawParams.VertexOffset, 0);
	}
	else
	{
		/*
		* Required for stencil clipping
		*/
		if (bEnableStencilClipping)
		{
			if (Node->Widget->GetRotation() != 0.0f && Node->Widget->HasAnyChildren())
			{
				if (DepthPass(Node->Widget, VP /*sViewport(ScreenDimension)*/))
				{
					CMD->SetVertexBuffer(VertexBuffer);
					CMD->SetIndexBuffer(IndexBuffer);

					CMD->SetStencilRef(2);
				}
			}
		}

		if (!bShowLines)
			return;

		if (ScissorsRect.IsValid())
		{
			CMD->SetScissorRect((std::uint32_t)ScissorsRect.GetTop(), (std::uint32_t)ScissorsRect.GetLeft(), (std::uint32_t)ScissorsRect.GetBottom(), (std::uint32_t)ScissorsRect.GetRight());
		}
		else
		{
			return;
		}

		const auto& GeometryDrawData = Node->Widget->GetGeometryDrawData(true);
		if (GeometryDrawData.GeometryType == "NONE")
			return;

		const auto pMaterialInstace = DefaultLineColorMatStyle->GetMaterial(GeometryDrawData.StyleState)->Material;
		sMaterial* pMaterial = pMaterialInstace->GetParent();
		if (LastMaterial != pMaterial)
		{
			LastMaterial = pMaterial;
			LastMaterial->ApplyMaterial(CMD.get());
			CMD->SetConstantBuffer(WidgetConstantBuffer.get());
		}
		pMaterialInstace->ApplyMaterialInstance(CMD.get());

		if (Node->bVertexDirty || Node->bIndexDirty)
		{
			if (Node->bVertexDirty)
			{
				const auto& Data = Node->Widget->GetVertexData(true);
				if (Data.size() > 0)
					CMD->UpdateBufferSubresource(VertexBuffer, Node->DrawParams.VertexOffset * sizeof(cbgui::cbGeometryVertexData), Data.size() * sizeof(cbgui::cbGeometryVertexData), Data.data());
				Node->bVertexDirty = false;
			}
			if (Node->bIndexDirty)
			{
				const auto& IndexData = Node->Widget->GetIndexData(true);
				if (IndexData.size() > 0)
					CMD->UpdateBufferSubresource(IndexBuffer, Node->DrawParams.IndexOffset * sizeof(std::uint32_t), GeometryDrawData.IndexCount * sizeof(std::uint32_t), IndexData.data());
				Node->bIndexDirty = false;
			}
		}
		CMD->DrawIndexedInstanced((std::uint32_t)GeometryDrawData.DrawCount, 1, (std::uint32_t)Node->DrawParams.IndexOffset, (std::uint32_t)Node->DrawParams.VertexOffset, 0);
	}
}

bool sCanvasRenderer::DepthPass(cbgui::cbWidgetObj* Widget, const sViewport& VP)
{
	if (Widget->HasOwner())
		return false;

	LastMaterial = nullptr;

	{
		CMD->SetStencilRef(1);
		CMD->SetVertexBuffer(DepthVertexBuffer.get());

		std::vector<cbgui::cbVector4> Data;
		Data.push_back(cbgui::cbVector4::Zero());
		Data.push_back(cbgui::cbVector4(cbgui::cbVector((float)VP.Width, 0), cbgui::cbVector(0.0f, 1.0f)));
		Data.push_back(cbgui::cbVector4(cbgui::cbVector(0, (float)VP.Height), cbgui::cbVector(0.0f, 1.0f)));
		Data.push_back(cbgui::cbVector4(cbgui::cbVector((float)VP.Width, (float)VP.Height), cbgui::cbVector(0.0f, 1.0f)));
		CMD->UpdateBufferSubresource(DepthVertexBuffer.get(), 0 * sizeof(cbgui::cbVector4), Data.size() * sizeof(cbgui::cbVector4), Data.data());

		CMD->SetConstantBuffer(WidgetConstantBuffer.get());
		CMD->SetScissorRect(0, 0, (std::uint32_t)VP.Height, (std::uint32_t)VP.Width);
		CMD->SetPipeline(DepthMainPipeline.get());
		CMD->Draw(4);
	}
	{
		CMD->SetStencilRef(0);
		CMD->SetVertexBuffer(DepthVertexBuffer.get());

		const auto Bounds = Widget->GetBounds();
		std::vector<cbgui::cbVector4> Data;
		Data.push_back(cbgui::cbVector4(Bounds.GetCorner(0), cbgui::cbVector(0.0f, 1.0f)));
		Data.push_back(cbgui::cbVector4(Bounds.GetCorner(1), cbgui::cbVector(0.0f, 1.0f)));
		Data.push_back(cbgui::cbVector4(Bounds.GetCorner(3), cbgui::cbVector(0.0f, 1.0f)));
		Data.push_back(cbgui::cbVector4(Bounds.GetCorner(2), cbgui::cbVector(0.0f, 1.0f)));
		for (auto& pData : Data)
			pData = cbgui::RotateVectorAroundPoint(pData, Widget->GetRotatorOrigin(), Widget->GetRotation());
		CMD->UpdateBufferSubresource(DepthVertexBuffer.get(), 0 * sizeof(cbgui::cbVector4), Data.size() * sizeof(cbgui::cbVector4), Data.data());

		CMD->SetConstantBuffer(WidgetConstantBuffer.get());
		CMD->SetScissorRect(0, 0, (std::uint32_t)VP.Height, (std::uint32_t)VP.Width);
		CMD->SetPipeline(DepthPipeline.get());
		CMD->Draw(4);
	}

	return true;
}

void sCanvasRenderer::SetRenderSize(std::size_t InWidth, std::size_t InHeight)
{
	ScreenDimension.Width = InWidth;
	ScreenDimension.Height = InHeight;

	/*{
		sFrameBufferAttachmentInfo AttachmentInfo;
		AttachmentInfo.Desc.Dimensions.X = (std::uint32_t)ScreenDimension.Width;
		AttachmentInfo.Desc.Dimensions.Y = (std::uint32_t)ScreenDimension.Height;
		AttachmentInfo.AddFrameBuffer(GPU::GetBackBufferFormat());
		AttachmentInfo.DepthFormat = GPU::GetDefaultDepthFormat();
		CanvasFBO = IFrameBuffer::Create("Deferred_Backbuffer", AttachmentInfo);
	}*/

	{
		OnScreenWidgetMatrix WidgetMatrix;
		WidgetMatrix.Matrix = (FMatrix&)cbgui::GetViewportTransform((int)ScreenDimension.Width, (int)ScreenDimension.Height);

		WidgetConstantBuffer->Map(&WidgetMatrix.Matrix);
	}
}

void sCanvasRenderer::OnInputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar)
{
	if (!KeyboardChar.bIsIdle)
	{
		if (KeyboardChar.KeyCode == 38 && KeyboardChar.bIsPressed && !KeyboardChar.bIsChar)
		{
			bShowLines = true;
		}
		else if (KeyboardChar.KeyCode == 40 && KeyboardChar.bIsPressed && !KeyboardChar.bIsChar)
		{
			bShowLines = false;
		}
		else if (KeyboardChar.KeyCode == 109 && KeyboardChar.bIsPressed && !KeyboardChar.bIsChar)
		{
			bEnableStencilClipping = false;
		}
		else if (KeyboardChar.KeyCode == 107 && KeyboardChar.bIsPressed && !KeyboardChar.bIsChar)
		{
			bEnableStencilClipping = true;
		}
	}
}
