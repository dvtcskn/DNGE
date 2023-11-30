/* ---------------------------------------------------------------------------------------
* MIT License
*
* Copyright (c) 2023 Davut Co�kun.
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
#include "Renderer.h"
#include <ranges>
#include "Gameplay/GameInstance.h"
#include "Gameplay/StaticMesh.h"
#include "Gameplay/MeshComponent.h"

class sRenderer::sGBuffer
{
	sBaseClassBody(sClassConstructor, sRenderer::sGBuffer);
public:
	sGBuffer(std::size_t Width, std::size_t Height, IGraphicsCommandContext::SharedPtr CMD = nullptr)
		: GBuffer(nullptr)
		, GraphicsCommandContext(CMD ? CMD : IGraphicsCommandContext::Create())
		, ScreenDimension(sScreenDimension(Width, Height))
	{
		sBufferDesc BufferDesc;
		BufferDesc.Size = sizeof(sTimeBuffer);
		TimeCB = IConstantBuffer::Create("TimeCB", BufferDesc, 3);

		{
			sFrameBufferAttachmentInfo AttachmentInfo;
			AttachmentInfo.Desc.Dimensions.X = (std::uint32_t)ScreenDimension.Width;
			AttachmentInfo.Desc.Dimensions.Y = (std::uint32_t)ScreenDimension.Height;
			AttachmentInfo.AddFrameBuffer(GPU::GetBackBufferFormat()); // finalColor
			AttachmentInfo.DepthFormat = GPU::GetDefaultDepthFormat();
			GBuffer = IFrameBuffer::Create("GBuffer", AttachmentInfo);
		}

		{
			sPipelineDesc pPipelineDesc;
			pPipelineDesc.BlendAttribute = sBlendAttributeDesc(EBlendStateMode::eOpaque);
			pPipelineDesc.DepthStencilAttribute = sDepthStencilAttributeDesc(true, true);
			pPipelineDesc.DepthStencilAttribute.DepthTest = ECompareFunction::eGreaterEqual;
			pPipelineDesc.PrimitiveTopologyType = EPrimitiveType::eTRIANGLE_LIST;
			pPipelineDesc.RasterizerAttribute = sRasterizerAttributeDesc();

			pPipelineDesc.NumRenderTargets = 1;
			pPipelineDesc.RTVFormats[0] = EFormat::BGRA8_UNORM;
			pPipelineDesc.DSVFormat = EFormat::R32G8X24_Typeless;

			std::vector<sVertexAttributeDesc> VertexLayout =
			{
				{ "POSITION",	EFormat::RGB32_FLOAT,   0, offsetof(sVertexBufferEntry, position),    false },
				{ "NORMAL",		EFormat::RGB32_FLOAT,   0, offsetof(sVertexBufferEntry, normal),      false },
				{ "TEXCOORD",	EFormat::RG32_FLOAT,    0, offsetof(sVertexBufferEntry, texCoord),    false },
				{ "COLOR",		EFormat::RGBA32_FLOAT,  0, offsetof(sVertexBufferEntry, Color),		  false },
				{ "TANGENT",	EFormat::RGB32_FLOAT,   0, offsetof(sVertexBufferEntry, tangent),     false },
				{ "BINORMAL",	EFormat::RGB32_FLOAT,   0, offsetof(sVertexBufferEntry, binormal),    false },
				{ "ARRAYINDEX",	EFormat::R32_UINT,	    0, offsetof(sVertexBufferEntry, ArrayIndex),  false },
			};
			pPipelineDesc.VertexLayout = VertexLayout;

			pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eUniformBuffer, eShaderType::Vertex, 0));
			pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eUniformBuffer, eShaderType::Vertex, 10));
			pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eTexture, eShaderType::Pixel, 0));
			pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eSampler, eShaderType::Pixel, 0));
			pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eUniformBuffer, eShaderType::Pixel, 0));

			pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(L"..//Content\\Shaders\\GBufferVS.hlsl", "GeometryVS", eShaderType::Vertex));
			pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(L"..//Content\\Shaders\\GBufferPS.hlsl", "GeometryPS", eShaderType::Pixel));

			DefaultEngineMat = sMaterial::Create("DefaultEngineMat", EMaterialBlendMode::Opaque, pPipelineDesc);
		}

		DefaultMatInstance = DefaultEngineMat->CreateInstance("DefaultEngineMatInstance");
		//DefaultMatInstance->AddTexture(L"..//Content\\Textures\\DefaultGrid.DDS", "DefaultEngineTexture", 2);
	}
	~sGBuffer()
	{
		DefaultEngineMat = nullptr;
		DefaultMatInstance = nullptr;
		GraphicsCommandContext = nullptr;
		for (auto& CameraCB : CameraCBs)
			CameraCB = nullptr;
		CameraCBs.clear();
		GBuffer = nullptr;

		TimeCB = nullptr;
	}

	void ClearGBuffer()
	{
		GraphicsCommandContext->BeginRecordCommandList();

		GraphicsCommandContext->ClearFrameBuffer(GBuffer.get());

		GraphicsCommandContext->FinishRecordCommandList();
		GraphicsCommandContext->ExecuteCommandList();
	}

	void Tick(double DeltaTime)
	{
		TimeBuffer.Time += DeltaTime;
		TimeCB->Map(&TimeBuffer);
	}

	IRenderTarget* Render(ILevel* Level, std::size_t index, ICamera* pCamera, std::optional<sViewport> Viewport)
	{
		//std::size_t MeshCompHashCode = sMeshComponent::GetStaticHashCode();
		//std::size_t MeshHashCode = IMesh::GetStaticHashCode();

		sMaterial* LastMaterial = nullptr;
		std::vector<IMesh*> BlendedMeshes;
		std::vector<IMesh*> LatestMeshes;

		std::function<void(EMaterialBlendMode, IMesh*, IGraphicsCommandContext*)> Draw;
		Draw = [&](EMaterialBlendMode BlendMode, IMesh* Mesh, IGraphicsCommandContext* CMD)
			{
				if (Mesh->GeMeshRenderPriority() == EMeshRenderPriority::Latest)
				{
					LatestMeshes.push_back(Mesh);
					return;
				}
				const auto& pMaterialInstace = Mesh->GetMaterialInstance();
				if (pMaterialInstace)
				{
					sMaterial* pMaterial = pMaterialInstace->GetParent();
					if (BlendMode == EMaterialBlendMode::Opaque && pMaterial->BlendMode == EMaterialBlendMode::Masked)
					{
						BlendedMeshes.push_back(Mesh);
						return;
					}
					{
						if (LastMaterial != pMaterial)
						{
							LastMaterial = pMaterial;
							LastMaterial->ApplyMaterial(CMD);
						}
						pMaterialInstace->ApplyMaterialInstance(CMD);
					}
				}
				else
				{
					if (LastMaterial != DefaultEngineMat.get())
					{
						LastMaterial = DefaultEngineMat.get();
						LastMaterial->ApplyMaterial(CMD);
					}
					DefaultMatInstance->ApplyMaterialInstance(CMD);
				}

				{
					CMD->SetVertexBuffer(Mesh->GetVertexBuffer());
					CMD->SetIndexBuffer(Mesh->GetIndexBuffer());

					if (Mesh->IsUpdateRequired())
						Mesh->UpdateMesh(CMD);

					CMD->SetConstantBuffer(CameraCBs[index].get());
					CMD->SetConstantBuffer(Mesh->GetMeshConstantBuffer());
					for (std::size_t i = 0; i < Mesh->GetSecondaryConstantBufferCount(); i++)
						CMD->SetConstantBuffer(Mesh->GetSecondaryConstantBuffer(i));
					CMD->SetConstantBuffer(TimeCB.get());

					CMD->DrawIndexedInstanced(Mesh->GetDrawParameters());
				}
			};

		std::function<void(EMaterialBlendMode, sPrimitiveComponent*, IGraphicsCommandContext*)> fDraw;
		fDraw = [&](EMaterialBlendMode BlendMode, sPrimitiveComponent* Component, IGraphicsCommandContext* CMD) -> void
			{
				if (!Component)
					return;

				if (Component->IsHidden())
					return;

				if (auto MeshComponent = Cast<IMeshComponent>(Component))
					Draw(BlendMode, MeshComponent->GetMesh(), CMD);

				std::size_t ChildrenSize = Component->GetChildrenSize();
				for (std::size_t i = 0; i < ChildrenSize; i++)
				{
					fDraw(BlendMode, Component->GetChild(i), CMD);
				}
			};

		{
			GraphicsCommandContext->BeginRecordCommandList(ERenderPass::eGBuffer);

			UpdateCameraBuffer(pCamera, index, GraphicsCommandContext.get());

			//GraphicsCommandContext->ClearFrameBuffer(GBuffer.get());
			GraphicsCommandContext->SetFrameBuffer(GBuffer.get());

			if (Viewport.has_value())
				GraphicsCommandContext->SetViewport(*Viewport);
			else
				GraphicsCommandContext->SetViewport(sViewport(ScreenDimension));

			GraphicsCommandContext->SetScissorRect(0, 0, (std::uint32_t)ScreenDimension.Height, (std::uint32_t)ScreenDimension.Width);

			auto LayerCount = Level->LayerCount();

			for (size_t Layer = 0; Layer < LayerCount; Layer++)
			{
				auto MeshCount = Level->MeshCount(Layer);
				for (size_t i = 0; i < MeshCount; i++)
				{
					Draw(EMaterialBlendMode::Opaque, Level->GetMesh(i, Layer), GraphicsCommandContext.get());
				}

				auto ActorCount = Level->ActorCount(Layer);
				for (size_t i = 0; i < ActorCount; i++)
				{
					auto Obj = Level->GetActor(i, Layer);
					if (Obj->IsHidden())
						continue;

					fDraw(EMaterialBlendMode::Opaque, Obj->GetRootComponent(), GraphicsCommandContext.get());
				}

				for (const auto& Mesh : BlendedMeshes)
				{
					Draw(EMaterialBlendMode::Masked, Mesh, GraphicsCommandContext.get());
				}
				BlendedMeshes.clear();

				for (const auto& Mesh : LatestMeshes)
				{
					Draw(EMaterialBlendMode::Opaque, Mesh, GraphicsCommandContext.get());
				}
				LatestMeshes.clear();

				for (const auto& Mesh : BlendedMeshes)
				{
					Draw(EMaterialBlendMode::Masked, Mesh, GraphicsCommandContext.get());
				}
				BlendedMeshes.clear();
			}

			GraphicsCommandContext->FinishRecordCommandList();
			GraphicsCommandContext->ExecuteCommandList();
		}

		return GBuffer->GetRenderTarget(0);
	}

	void CopyToFrameBuffer(IRenderTarget* RT)
	{
		GraphicsCommandContext->BeginRecordCommandList();
		GraphicsCommandContext->CopyRenderTarget(RT, GBuffer->GetRenderTarget(0));
		GraphicsCommandContext->FinishRecordCommandList();
		GraphicsCommandContext->ExecuteCommandList();
	}

	void UpdateCameraBuffer(ICamera* pCamera, std::size_t index, IGraphicsCommandContext* CMD = nullptr)
	{
		CameraBuffer.PrevViewProjMatrix = CameraBuffer.ViewProjMatrix;
		CameraBuffer.ViewProjMatrix = pCamera->GetViewProjMatrix();
		CameraCBs[index]->Map(&CameraBuffer, CMD);
	}
	
	void AddCameraConstantBuffer(std::size_t Count = 1)
	{
		for (std::size_t i = 0; i < Count; i++)
		{
			sBufferDesc BufferDesc;
			BufferDesc.Size = sizeof(sCameraBuffer);
			IConstantBuffer::SharedPtr CameraCB = IConstantBuffer::Create("CameraCB_" + std::to_string(CameraCBs.size()), BufferDesc, 0);
			CameraCBs.push_back(CameraCB);
		}
	}

	void RemoveLastCameraConstantBuffer()
	{
		CameraCBs[CameraCBs.size() - 1] = nullptr;
		CameraCBs.pop_back();
	}

	void DestroyAllCameraConstantBuffers()
	{
		for (auto& CameraCB : CameraCBs)
			CameraCB = nullptr;
		CameraCBs.clear();
	}

	void SetRenderSize(std::size_t InWidth, std::size_t InHeight)
	{
		ScreenDimension.Width = InWidth;
		ScreenDimension.Height = InHeight;

		sFrameBufferAttachmentInfo AttachmentInfo;
		AttachmentInfo.Desc.Dimensions.X = (std::uint32_t)ScreenDimension.Width;
		AttachmentInfo.Desc.Dimensions.Y = (std::uint32_t)ScreenDimension.Height;
		AttachmentInfo.AddFrameBuffer(GPU::GetBackBufferFormat()); // finalColor
		AttachmentInfo.DepthFormat = GPU::GetDefaultDepthFormat();
		GBuffer = IFrameBuffer::Create("GBuffer", AttachmentInfo);
	}

	IFrameBuffer* GetGBuffer() const { return GBuffer.get(); };
	IRenderTarget* GetMotionVector() const { return GBuffer->GetRenderTarget(4); };
	IDepthTarget* GetDepth() const { return GBuffer->GetDepthTarget(); };
	IConstantBuffer* GetCameraConstantBuffer(std::size_t index) const { return CameraCBs[index].get(); }
	std::size_t GetCameraConstantBufferSize() const { return CameraCBs.size(); }
	sScreenDimension GetScreenDimension() const { return ScreenDimension; }

private:
	IFrameBuffer::SharedPtr GBuffer;
	sScreenDimension ScreenDimension;

	IGraphicsCommandContext::SharedPtr GraphicsCommandContext;
	sMaterial::SharedPtr DefaultEngineMat;
	sMaterial::sMaterialInstance::SharedPtr DefaultMatInstance;
	std::vector<IConstantBuffer::SharedPtr> CameraCBs;

	__declspec(align(256)) struct sCameraBuffer
	{
		FMatrix ViewProjMatrix;
		FMatrix PrevViewProjMatrix;
	};
	static_assert((sizeof(sCameraBuffer) % 256) == 0, "Constant Buffer size must be 256-byte aligned");

	sCameraBuffer CameraBuffer;

	__declspec(align(256)) struct sTimeBuffer
	{
		double Time;
	};
	static_assert((sizeof(sTimeBuffer) % 256) == 0, "Constant Buffer size must be 256-byte aligned");

	sTimeBuffer TimeBuffer;
	IConstantBuffer::SharedPtr TimeCB;
};

sRenderer::sRenderer(std::size_t Width, std::size_t Height)
	: FinalRenderTarget(nullptr)
	, GraphicsCommandContext(IGraphicsCommandContext::Create())
	, World(nullptr)
	, GBufferClearMode(EGBufferClear::Driver)
	, ScreenDimension(sScreenDimension(Width, Height))
	, InternalBaseRenderResolution(sScreenDimension(Width, Height))
	, GBuffer(sGBuffer::CreateUnique(Width, Height))
	, LineRenderer(sLineRenderer::Create(Width, Height))
	, CanvasRenderer(sCanvasRenderer::Create(Width, Height))
	, PostProcessRenderer(sPostProcessRenderer::CreateUnique(Width, Height))
	, ToneMapping(sToneMapping::CreateUnique(Width, Height))
	, bIsTonmapperEnabled(true)
{
}

sRenderer::~sRenderer()
{
	for (auto& vPP : PostProcess)
	{
		for (auto& PP : vPP.second)
		{
			PP = nullptr;
		}
	}
	PostProcess.clear();

	ViewportInstances.clear();
	FinalRenderTarget = nullptr;
	GraphicsCommandContext = nullptr;
	World = nullptr;
	GBuffer = nullptr;
	CanvasRenderer = nullptr;
	ToneMapping = nullptr;
	PostProcessRenderer = nullptr;
	LineRenderer = nullptr;
}

void sRenderer::BeginPlay()
{
	LineRenderer->BeginPlay();
	CanvasRenderer->BeginPlay();
	PostProcessRenderer->BeginPlay();
}

void sRenderer::Tick(const double DeltaTime)
{
	if (!World)
		return;

	GBuffer->Tick(DeltaTime);
	LineRenderer->Tick(DeltaTime);
}

void sRenderer::AddViewportInstance(sViewportInstance* ViewportInstance, std::optional<std::size_t> Priority)
{
	if (Priority.has_value())
	{
		if (*Priority == ViewportInstances.size())
			ViewportInstances.insert(ViewportInstances.begin() + *Priority, ViewportInstance);
		else
			ViewportInstances.insert(ViewportInstances.begin() + ViewportInstances.size(), ViewportInstance);
		GBuffer->AddCameraConstantBuffer();
	}
	else
	{
		ViewportInstances.push_back(ViewportInstance);
		GBuffer->AddCameraConstantBuffer();
	}
}

void sRenderer::RemoveViewportInstance(sViewportInstance* ViewportInstance)
{
	if (std::find(ViewportInstances.begin(), ViewportInstances.end(), ViewportInstance) != ViewportInstances.end())
	{
		ViewportInstances.erase(std::find(ViewportInstances.begin(), ViewportInstances.end(), ViewportInstance));
		GBuffer->RemoveLastCameraConstantBuffer();
	}
}

void sRenderer::RemoveViewportInstance(std::size_t Index)
{
	if (Index < ViewportInstances.size())
	{
		ViewportInstances.erase(ViewportInstances.begin() + Index);
		GBuffer->RemoveLastCameraConstantBuffer();
	}
}

void sRenderer::SetViewportInstancePriority(sViewportInstance* ViewportInstance, std::size_t Priority)
{
	RemoveViewportInstance(ViewportInstance);
	if (Priority == ViewportInstances.size())
		ViewportInstances.insert(ViewportInstances.begin() + Priority, ViewportInstance);
	else
		ViewportInstances.insert(ViewportInstances.begin() + ViewportInstances.size(), ViewportInstance);
	GBuffer->AddCameraConstantBuffer();
}

void sRenderer::SetMetaWorld(IMetaWorld* pMetaWorld)
{
	World = pMetaWorld;
}

void sRenderer::RemoveWorld()
{
	World = nullptr;
}

IMetaWorld* sRenderer::GetMetaWorld() const
{
	return World;
}

void sRenderer::Render()
{
	if (!World)
		return;

	if (GBufferClearMode == EGBufferClear::Driver)
	{
		GBuffer->ClearGBuffer();
	}
	FinalRenderTarget = GBuffer->GetGBuffer()->GetRenderTarget(0);

	auto GameInstance = World->GetGameInstance();
	std::size_t PlayerCount = GameInstance->GetPlayerCount();

	if ((PlayerCount + ViewportInstances.size()) != GBuffer->GetCameraConstantBufferSize())
	{
		GBuffer->DestroyAllCameraConstantBuffers();
		GBuffer->AddCameraConstantBuffer(PlayerCount + ViewportInstances.size());
	}

	/*
	* To Do:
	* Batch Command Context 
	*/
	if (PlayerCount > 0)
	{
		std::size_t Count = GameInstance->IsSplitScreenEnabled() ? PlayerCount : PlayerCount > 0 ? 1 : 0;
		for (std::size_t i = 0; i < Count; i++)
		{
			auto Player = GameInstance->GetPlayer(i, false);
			sViewportInstance* ViewportInstance = Player->GetViewportInstance();
			if (!ViewportInstance)
				continue;
			if (!ViewportInstance->bIsEnabled)
				continue;

			FinalRenderTarget = GBuffer->Render(World->GetActiveLevel(), i, ViewportInstance->pCamera.get(), ViewportInstance->Viewport);
			LineRenderer->Render(FinalRenderTarget, GBuffer->GetCameraConstantBuffer(i), ViewportInstance->pCamera.get(), ViewportInstance->Viewport);
		}
	}
	{
		for (std::size_t i = 0; i < ViewportInstances.size(); i++)
		{
			sViewportInstance* ViewportInstance = ViewportInstances[i];
			FinalRenderTarget = GBuffer->Render(World->GetActiveLevel(), i, ViewportInstance->pCamera.get(), ViewportInstance->Viewport);
		}
	}

	for (const auto& PP : PostProcess[EPostProcessRenderOrder::BeforeTonemap])
	{
		PostProcessRenderer->Render(PP.get(), FinalRenderTarget, std::nullopt);
		FinalRenderTarget = PP->GetFrameBuffer();
	}

	if (bIsTonmapperEnabled)
	{
		PostProcessRenderer->Render(ToneMapping.get(), FinalRenderTarget, std::nullopt);
		FinalRenderTarget = ToneMapping->GetFrameBuffer();
	}

	for (const auto& PP : PostProcess[EPostProcessRenderOrder::AfterTonemap])
	{
		PostProcessRenderer->Render(PP.get(), FinalRenderTarget, std::nullopt);
		FinalRenderTarget = PP->GetFrameBuffer();
	}

	for (const auto& PP : PostProcess[EPostProcessRenderOrder::BeforeUI])
	{
		PostProcessRenderer->Render(PP.get(), FinalRenderTarget, std::nullopt);
		FinalRenderTarget = PP->GetFrameBuffer();
	}

	if (PlayerCount > 0)
	{
		for (std::size_t i = 0; i < PlayerCount; i++)
		{
			auto Player = GameInstance->GetPlayer(i, false);
			if (Player->GetNetworkRole() == eNetworkRole::SimulatedProxy || Player->GetNetworkRole() == eNetworkRole::NetProxy)
				continue;
			sViewportInstance* ViewportInstance = Player->GetViewportInstance();
			if (!ViewportInstance)
				continue;
			if (!ViewportInstance->bIsEnabled)
				continue;

			if (ViewportInstance->Canvases.size() > 0)
			{
				if (ViewportInstance->Viewport.has_value())
				{
					std::uint32_t X = (std::uint32_t)ScreenDimension.Width / (std::uint32_t)InternalBaseRenderResolution.Width;
					std::uint32_t Y = (std::uint32_t)ScreenDimension.Height / (std::uint32_t)InternalBaseRenderResolution.Height;
					sViewport Viewport = sViewport((std::uint32_t)ScreenDimension.Width, (std::uint32_t)ScreenDimension.Height,
						ViewportInstance->Viewport->TopLeftX * X, ViewportInstance->Viewport->TopLeftY * Y);

					CanvasRenderer->Render(ViewportInstance->Canvases, FinalRenderTarget, Viewport);
				}
				else
				{
					CanvasRenderer->Render(ViewportInstance->Canvases, FinalRenderTarget, ViewportInstance->Viewport);
				}
			}
		}
	}
	for (sViewportInstance* ViewportInstance : ViewportInstances/*std::ranges::views::reverse(ViewportInstances)*/)
	{
		if (ViewportInstance->Canvases.size() > 0)
		{
			if (ViewportInstance->Viewport.has_value())
			{
				std::uint32_t X = (std::uint32_t)ScreenDimension.Width / (std::uint32_t)InternalBaseRenderResolution.Width;
				std::uint32_t Y = (std::uint32_t)ScreenDimension.Height / (std::uint32_t)InternalBaseRenderResolution.Height;
				sViewport Viewport = sViewport((std::uint32_t)ScreenDimension.Width, (std::uint32_t)ScreenDimension.Height,
					ViewportInstance->Viewport->TopLeftX * X, ViewportInstance->Viewport->TopLeftY * Y);

				CanvasRenderer->Render(ViewportInstance->Canvases, FinalRenderTarget, Viewport);
			}
			else
			{
				CanvasRenderer->Render(ViewportInstance->Canvases, FinalRenderTarget, ViewportInstance->Viewport);
			}
		}
	}
	CanvasRenderer->Render(World->GetCanvases(), FinalRenderTarget, std::nullopt);

	for (const auto& PP : PostProcess[EPostProcessRenderOrder::AfterUI])
	{
		PostProcessRenderer->Render(PP.get(), FinalRenderTarget, std::nullopt);
		FinalRenderTarget = PP->GetFrameBuffer();
	}
}

void sRenderer::OnResizeWindow(std::size_t InWidth, std::size_t InHeight)
{
	if (ScreenDimension == InternalBaseRenderResolution)
	{
		ScreenDimension.Width = InWidth;
		ScreenDimension.Height = InHeight;
		InternalBaseRenderResolution = ScreenDimension;
	} 
	else 
	{
		ScreenDimension.Width = InWidth;
		ScreenDimension.Height = InHeight;
	}

	GBuffer->SetRenderSize(InternalBaseRenderResolution.Width, InternalBaseRenderResolution.Height);
	LineRenderer->SetRenderSize(InternalBaseRenderResolution.Width, InternalBaseRenderResolution.Height);

	PostProcessRenderer->SetRenderSize(ScreenDimension.Width, ScreenDimension.Height);
	CanvasRenderer->SetRenderSize(ScreenDimension.Width, ScreenDimension.Height);
	ToneMapping->SetFrameBufferSize(ScreenDimension.Width, ScreenDimension.Height);
}

void sRenderer::OnInputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar)
{
	LineRenderer->OnInputProcess(MouseInput, KeyboardChar);
	CanvasRenderer->OnInputProcess(MouseInput, KeyboardChar);
	PostProcessRenderer->OnInputProcess(MouseInput, KeyboardChar);
}

void sRenderer::DrawLine(const FVector& Start, const FVector& End, const FColor& Color, std::optional<float> Time)
{
	if (LineRenderer)
		LineRenderer->DrawLine(Start, End, Color, Time);
}

void sRenderer::DrawBound(const FBoundingBox& Box, const FColor& Color, std::optional<float> Time)
{
	if (LineRenderer)
		LineRenderer->DrawBound(Box, Color, Time);
}

void sRenderer::SetInternalBaseRenderResolution(std::size_t Width, std::size_t Height)
{
	InternalBaseRenderResolution = sScreenDimension(Width, Height);
	GBuffer->SetRenderSize(InternalBaseRenderResolution.Width, InternalBaseRenderResolution.Height);
	LineRenderer->SetRenderSize(InternalBaseRenderResolution.Width, InternalBaseRenderResolution.Height);
}

void sRenderer::SetTonemapper(int Val)
{
	ToneMapping->SetTonemapper(Val);
}

int sRenderer::GetTonemapperIndex() const
{
	return ToneMapping->GetTonemapperIndex();
}

void sRenderer::SetGBufferClearMode(EGBufferClear Mode)
{
	GBufferClearMode = Mode;
}

void sRenderer::AddPostProcess(const EPostProcessRenderOrder Order, const std::shared_ptr<sPostProcess>& PP)
{
	PostProcess[Order].push_back(PP);
}

void sRenderer::RemovePostProcess(const EPostProcessRenderOrder Order, const int Val)
{
	PostProcess[Order].erase(PostProcess[Order].begin() + Val);
}
