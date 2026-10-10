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
#include "Renderer.h"
#include <ranges>
#include "Gameplay/GameInstance.h"
#include "Gameplay/StaticMesh.h"
#include "Gameplay/MeshComponent.h"
#include "Utilities/FileManager.h"
#include "AbstractGI/MaterialManager.h"
#include "Utilities/TimerProfiler.h"

struct GeometrySceneDescriptor : public IBindlessSceneDescriptor
{
	std::uint32_t VertexBufferStructureIndex = std::uint32_t(-1);
	std::uint32_t VertexBufferLocation = std::uint32_t(-1);
	std::uint32_t VertexBufferStride = std::uint32_t(-1);
	std::uint32_t VertexInstanceBufferLocation = std::uint32_t(-1);
	std::uint32_t VertexInstanceBufferStride = std::uint32_t(-1);
	std::uint32_t StructureIndex = std::uint32_t(-1);
	std::uint32_t MaterialIndex = std::uint32_t(-1);
	std::uint32_t CameraConstantBuffer = std::uint32_t(-1);
	std::uint32_t ObjectConstantBuffer = std::uint32_t(-1);
	std::uint32_t TimeConstantBuffer = std::uint32_t(-1);

	inline bool IsValid(bool bCheckForInstances = false) const
	{
		return (VertexBufferStructureIndex != std::uint32_t(-1) &&
			VertexBufferLocation != std::uint32_t(-1) &&
			VertexBufferStride != std::uint32_t(-1) &&
			StructureIndex != std::uint32_t(-1) &&
			MaterialIndex != std::uint32_t(-1) &&
			CameraConstantBuffer != std::uint32_t(-1) &&
			ObjectConstantBuffer != std::uint32_t(-1) &&
			TimeConstantBuffer != std::uint32_t(-1)) && 
			(!bCheckForInstances || (VertexInstanceBufferLocation != std::uint32_t(-1) && VertexInstanceBufferStride != std::uint32_t(-1)));
	}
};

struct GeometrySceneIndirectCommand
{
	GeometrySceneDescriptor Descriptor;
	sObjectDrawParameters DrawArgs;
};

struct sGeometrySceneDescriptor : public IBindlessSceneContainer
{
	GeometrySceneDescriptor Descriptor;

	sGeometrySceneDescriptor(GeometrySceneDescriptor NewDescriptor)
		: Descriptor(NewDescriptor)
	{}

	virtual const IBindlessSceneDescriptor* GetDescriptor() const override final
	{
		return &Descriptor;
	}

	virtual std::uint32_t Size() const override final
	{
		return 10;
	}

	virtual std::vector<std::uint32_t> GetAllBindlessIndices() const override final
	{
		std::vector<std::uint32_t> BindlessIndices;
		BindlessIndices.push_back(Descriptor.VertexBufferStructureIndex);
		BindlessIndices.push_back(Descriptor.VertexBufferLocation);
		BindlessIndices.push_back(Descriptor.VertexBufferStride);
		BindlessIndices.push_back(Descriptor.VertexInstanceBufferLocation);
		BindlessIndices.push_back(Descriptor.VertexInstanceBufferStride);
		BindlessIndices.push_back(Descriptor.StructureIndex);
		BindlessIndices.push_back(Descriptor.MaterialIndex);
		BindlessIndices.push_back(Descriptor.CameraConstantBuffer);
		BindlessIndices.push_back(Descriptor.ObjectConstantBuffer);
		BindlessIndices.push_back(Descriptor.TimeConstantBuffer);
		return BindlessIndices;
	}
};

class sRenderer::sGBuffer
{
	sBaseClassBody(sClassConstructor, sRenderer::sGBuffer);
public:
	sGBuffer(std::size_t Width, std::size_t Height, IGraphicsCommandContext::SharedPtr CMD = nullptr)
		: GBuffer(nullptr)
		, GraphicsCommandContext(CMD ? CMD : IGraphicsCommandContext::Create())
		, ScreenDimension(sScreenDimension(Width, Height))
		, TimeBuffer(sTimeBuffer())
		, IndirectBuffer(nullptr)
		, bEnableExecuteIndirect(true)
		, CustomScreenDimension(sScreenDimension(0, 0))
		, MipBias(-1.0f)
		, bForceRecompileMaterials(false)
	{
		BufferLayout BufferDesc;
		BufferDesc.Size = sizeof(sTimeBuffer);
		TimeCB = IConstantBuffer::Create("TimeCB", BufferDesc, 2);

		{
			sFrameBufferAttachmentInfo AttachmentInfo;
			AttachmentInfo.Desc.Dimensions.X = (std::uint32_t)ScreenDimension.Width;
			AttachmentInfo.Desc.Dimensions.Y = (std::uint32_t)ScreenDimension.Height;
			AttachmentInfo.AddFrameBuffer(GPU::GetBackBufferFormat()/*, EFrameBufferAttachmentType::RT_SRV_UAV*/); // finalColor
			AttachmentInfo.DepthFormat = GPU::GetDefaultDepthFormat();
			GBuffer = IFrameBuffer::Create("GBuffer", AttachmentInfo);
		}

		{
			IndirectLayoutBindingDesc = sIndirectLayoutBindingDesc(EDrawTypes::DrawIndexedInstanced, 2, sizeof(GeometrySceneIndirectCommand), 1);
			IndirectBuffer = IIndirectBuffer::Create("GeometryPassIndirectBuffer", BufferLayout(1024 * 1024 * 4, sizeof(GeometrySceneIndirectCommand)));
		}

		{
			sPipelineDesc pPipelineDesc = sPipelineDesc::CreateDefaultPipelineDesc(ERenderPass::GBuffer, EBlendStateMode::Opaque, ECompareFunction::GreaterEqual, true, EVertexLayoutType::DefaultVertexLayout, false, IndirectLayoutBindingDesc);

			pPipelineDesc.Bindings.push_back(sShaderBinding(EDescriptorType::e32BitConstant, eShaderType::All, 0, 10));
			std::vector<sShaderAttachment> ShaderAttachments;
			pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(FileManager::GetShaderFolderW() + L"GBufferVS.hlsl", "GeometryVS", eShaderType::Vertex));
			pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(FileManager::GetShaderFolderW() + L"GBufferPS.hlsl", "GeometryPS", eShaderType::Pixel));

			DefaultEngineMat = sMaterial::Create("DefaultEngineMat", EMaterialBlendMode::Opaque, pPipelineDesc);
			DefaultEngineMat->Compile(GBuffer.get());
		}

		DefaultMatInstance = DefaultEngineMat->CreateInstance("DefaultEngineMatInstance");
		if (FileManager::fileExists(FileManager::GetTextureFolderW() + L"DefaultWhiteGrid.DDS"))
		{
			DefaultMatInstance->AddTexture(FileManager::GetTextureFolderW() + L"DefaultWhiteGrid.DDS", "DefaultEngineTexture", 3);
			DefaultMatInstance->AddSampler(sSamplerAttributeDesc(ESamplerStateMode::AnisotropicLinear));
			//MatInstance->AddTexture(L"..//Content\\Textures\\bfn.DDS", 3);
		}
	}

	~sGBuffer()
	{
		IndirectBuffer = nullptr;
		DefaultEngineMat = nullptr;
		DefaultMatInstance = nullptr;
		GraphicsCommandContext = nullptr;
		for (auto& CameraCB : CameraCBs)
			CameraCB.Release();
		CameraCBs.clear();
		GBuffer = nullptr;

		TimeCB = nullptr;
	}

	void ClearGBuffer()
	{
		if (MaterialsToRecompile.size() > 0)
		{
			GPU::WaitForGPU();
			for (auto& Mat : MaterialsToRecompile)
				Mat->Recompile();
			MaterialsToRecompile.clear();
		}
		GraphicsCommandContext->BeginRecordCommandList(ECommandContextBeginState::Render);

		GraphicsCommandContext->BeginProfile(ERenderPass::GBuffer, true);

		GraphicsCommandContext->ClearFrameBuffer(GBuffer.get());

		//GraphicsCommandContext->FinishRecordCommandList();
		//GraphicsCommandContext->ExecuteCommandList();
	}

	void Tick(double DeltaTime)
	{
		TimeBuffer.Time += (float)DeltaTime;
		TimeCB->Map(&TimeBuffer);
	}

	sIndirectLayoutBindingDesc GetIndirectLayoutBindingDesc() const
	{
		return IndirectLayoutBindingDesc;
	}

	/*IRenderTarget* Render(ILevel* Level, std::size_t index, ICamera* pCamera, std::optional<sViewport> Viewport)
	{
		std::vector<IndirectCommand> IndirectCommands;
		std::uint64_t ArgumentOffset = 0;

		sMaterial* LastMaterial = nullptr;
		std::vector<IMesh*> BlendedMeshes;
		std::vector<IMesh*> LatestMeshes;

		auto ExecuteRemainingIndirectCommands = [&](IGraphicsCommandContext* CMD) -> void
			{
				if (bEnableExecuteIndirect && IndirectCommands.size() > 0)
				{
					IndirectBuffer->SetArgument(IndirectCommands.data(), IndirectCommands.size(), ArgumentOffset);
					ArgumentOffset += sizeof(IndirectCommand) * IndirectCommands.size();
					IndirectCommands.clear();

					CMD->ExecuteIndirect(IndirectBuffer.get());
				}
			};

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
					if (!pMaterialInstace->IsCompiled())
						pMaterialInstace->Compile(GBuffer.get());

					sMaterial* pMaterial = pMaterialInstace->GetParent();
					if (BlendMode == EMaterialBlendMode::Opaque && pMaterial->BlendMode == EMaterialBlendMode::Masked)
					{
						BlendedMeshes.push_back(Mesh);
						return;
					}
					{
						if (LastMaterial != pMaterial)
						{
							if (bEnableExecuteIndirect)
								ExecuteRemainingIndirectCommands(CMD);

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
						if (!DefaultEngineMat->IsCompiled())
							DefaultEngineMat->Compile(GBuffer.get());

						if (bEnableExecuteIndirect)
							ExecuteRemainingIndirectCommands(CMD);

						LastMaterial = DefaultEngineMat.get();
						LastMaterial->ApplyMaterial(CMD);
					}
					DefaultMatInstance->ApplyMaterialInstance(CMD);
				}

				{
					if (Mesh->IsUpdateRequired())
						Mesh->UpdateMesh(CMD);

					GeometrySceneDescriptor GeometrySceneDescriptor;
					if (auto GeometryHandle = Mesh->GetGeometryHandle())
					{
						if (GeometryHandle->IsValid())
						{
							GeometrySceneDescriptor.VertexBufferStructureIndex = GeometryHandle->GetBindlessStructureIndex();
							GeometrySceneDescriptor.VertexBufferLocation = (std::uint32_t)GeometryHandle->GetLocation();
							GeometrySceneDescriptor.VertexBufferStride = (std::uint32_t)GeometryHandle->GetElementSize();
						}
						if (auto GeometryInstanceHandle = Mesh->GetGeometryInstanceHandle())
						{
							if (GeometryInstanceHandle->IsValid())
							{
								GeometrySceneDescriptor.VertexInstanceBufferLocation = (std::uint32_t)GeometryInstanceHandle->GetLocation();
								GeometrySceneDescriptor.VertexInstanceBufferStride = GeometryInstanceHandle->GetElementSize();
							}
						}
					}
					GeometrySceneDescriptor.StructureIndex = pMaterialInstace ? pMaterialInstace->GetStructureId() : DefaultMatInstance->GetStructureId();
					GeometrySceneDescriptor.MaterialIndex = pMaterialInstace ? pMaterialInstace->GetId() : DefaultMatInstance->GetId();
					GeometrySceneDescriptor.CameraConstantBuffer = CameraCBs[index]->GetBindlessIndex();
					GeometrySceneDescriptor.ObjectConstantBuffer = Mesh->GetMeshConstantBuffer()->GetBindlessIndex();
					GeometrySceneDescriptor.TimeConstantBuffer = TimeCB->GetBindlessIndex();


					if (bEnableExecuteIndirect)
					{
						IndirectCommand Command;
						Command.Descriptor = GeometrySceneDescriptor;
						Command.DrawArgs = Mesh->GetDrawParameters();

						IndirectCommands.push_back(Command);

					}
					else
					{
						//CMD->Set32BitConstants(0, &GeometrySceneDescriptor, 5, 0);					
						sGeometrySceneDescriptor SceneDescriptor(GeometrySceneDescriptor);
						CMD->SetBindlessDescriptor(0, &SceneDescriptor);
						CMD->DrawIndexedInstanced(Mesh->GetDrawParameters());
					}
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
			GraphicsCommandContext->BeginRecordCommandList(ERenderPass::GBuffer);

			UpdateCameraBuffer(pCamera, index, GraphicsCommandContext.get());

			//GraphicsCommandContext->ClearFrameBuffer(GBuffer.get());
			GraphicsCommandContext->SetFrameBuffer(GBuffer.get());

			if (Viewport.has_value())
				GraphicsCommandContext->SetViewport(*Viewport);
			else
				GraphicsCommandContext->SetViewport(sViewport(ScreenDimension));

			GraphicsCommandContext->SetScissorRect(0, 0, (std::uint32_t)ScreenDimension.Height, (std::uint32_t)ScreenDimension.Width);

			auto IndexBuffer = sMeshGeometryContainerManager::Get().GetIndexBuffer();
			GraphicsCommandContext->SetIndexBuffer(IndexBuffer);

			auto LayerCount = Level->LayerCount();

			for (size_t Layer = 0; Layer < LayerCount; Layer++)
			{
				auto MeshCount = Level->MeshCount(Layer);
				for (size_t i = 0; i < MeshCount; i++)
				{
					Draw(EMaterialBlendMode::Opaque, Level->GetMesh(i, Layer), GraphicsCommandContext.get());
				}
				if (IndirectCommands.size() > 0)
					ExecuteRemainingIndirectCommands(GraphicsCommandContext.get());

				auto ActorCount = Level->ActorCount(Layer);
				for (size_t i = 0; i < ActorCount; i++)
				{
					auto Obj = Level->GetActor(i, Layer);
					if (Obj->IsHidden())
						continue;

					fDraw(EMaterialBlendMode::Opaque, Obj->GetRootComponent(), GraphicsCommandContext.get());
				}
				if (IndirectCommands.size() > 0)
					ExecuteRemainingIndirectCommands(GraphicsCommandContext.get());

				for (const auto& Mesh : BlendedMeshes)
				{
					Draw(EMaterialBlendMode::Masked, Mesh, GraphicsCommandContext.get());
				}
				BlendedMeshes.clear();
				if (IndirectCommands.size() > 0)
					ExecuteRemainingIndirectCommands(GraphicsCommandContext.get());

				for (const auto& Mesh : LatestMeshes)
				{
					Draw(EMaterialBlendMode::Opaque, Mesh, GraphicsCommandContext.get());
				}
				LatestMeshes.clear();
				if (IndirectCommands.size() > 0)
					ExecuteRemainingIndirectCommands(GraphicsCommandContext.get());

				for (const auto& Mesh : BlendedMeshes)
				{
					Draw(EMaterialBlendMode::Masked, Mesh, GraphicsCommandContext.get());
				}
				BlendedMeshes.clear();
				if (IndirectCommands.size() > 0)
					ExecuteRemainingIndirectCommands(GraphicsCommandContext.get());
			}

			GraphicsCommandContext->FinishRecordCommandList();
			// Deferred means Execute later, Batch command
			GraphicsCommandContext->ExecuteCommandList(ECommandContextExecuteType::Deferred);
		}

		return GBuffer->GetRenderTarget(0);
	}*/

	IRenderTarget* Render(ILevel* Level, std::size_t index, ICamera* pCamera, std::optional<sViewport> Viewport)
	{
		if (!Level || !pCamera)
			return nullptr;

		std::vector<GeometrySceneIndirectCommand> IndirectCommands;
		std::uint64_t ArgumentOffset = 0;

		std::vector<IMesh*> BlendedMeshes;
		std::vector<IMesh*> LatestMeshes;
		sMaterial* LastMaterial = nullptr;

		auto FlushIndirectCommands = [&](IGraphicsCommandContext* CMD)
			{
				if (!bEnableExecuteIndirect || IndirectCommands.empty())
					return;

				const std::size_t CommandCount = IndirectCommands.size();
				IndirectBuffer->SetArgument(IndirectCommands.data(), CommandCount, ArgumentOffset);

				ArgumentOffset += sizeof(GeometrySceneIndirectCommand) * CommandCount;
				IndirectCommands.clear();

				CMD->ExecuteIndirect(IndirectBuffer.get());
			};

		auto DrawMesh = [&](EMaterialBlendMode BlendMode, IMesh* Mesh, IGraphicsCommandContext* CMD, bool bDeferLatest)
			{
				if (!Mesh)
					return;

				if (bDeferLatest &&
					Mesh->GeMeshRenderPriority() == EMeshRenderPriority::Latest)
				{
					LatestMeshes.push_back(Mesh);
					return;
				}

				auto MaterialInstance = Mesh->GetMaterialInstance();
				sMaterial* Material = nullptr;

				if (MaterialInstance)
				{
					if (!MaterialInstance->IsCompiled())
						MaterialInstance->Compile(GBuffer.get());

					if (bForceRecompileMaterials)
						MaterialsToRecompile.push_back(MaterialInstance);

					Material = MaterialInstance->GetParent();

					if (BlendMode == EMaterialBlendMode::Opaque && Material && Material->BlendMode == EMaterialBlendMode::Masked)
					{
						BlendedMeshes.push_back(Mesh);
						return;
					}
				}

				if (Material)
				{
					if (LastMaterial != Material)
					{
						FlushIndirectCommands(CMD);
						LastMaterial = Material;
						LastMaterial->ApplyMaterial(CMD);
					}

					MaterialInstance->ApplyMaterialInstance(CMD);
				}
				else
				{
					if (!DefaultEngineMat->IsCompiled())
						DefaultEngineMat->Compile(GBuffer.get());

					if (LastMaterial != DefaultEngineMat.get())
					{
						FlushIndirectCommands(CMD);
						LastMaterial = DefaultEngineMat.get();
						LastMaterial->ApplyMaterial(CMD);
					}

					DefaultMatInstance->ApplyMaterialInstance(CMD);
				}

				if (Mesh->IsUpdateRequired())
					Mesh->UpdateMesh(CMD);

				GeometrySceneDescriptor Descriptor;

				bool bInstance = false;
				if (auto GeometryHandle = Mesh->GetGeometryHandle(); GeometryHandle && GeometryHandle->IsValid())
				{
					Descriptor.VertexBufferStructureIndex =	GeometryHandle->GetBindlessStructureIndex();
					Descriptor.VertexBufferLocation = static_cast<std::uint32_t>(GeometryHandle->GetLocation());
					Descriptor.VertexBufferStride = static_cast<std::uint32_t>(GeometryHandle->GetElementSize());

					if (auto InstanceHandle = Mesh->GetGeometryInstanceHandle(); InstanceHandle && InstanceHandle->IsValid())
					{
						Descriptor.VertexInstanceBufferLocation = static_cast<std::uint32_t>(InstanceHandle->GetLocation());
						Descriptor.VertexInstanceBufferStride =	static_cast<std::uint32_t>(InstanceHandle->GetElementSize());
						bInstance = true;
					}
				}
				else
				{
					// invalid vertex-buffer
					return;
				}

				//auto IndexBuffer = Mesh->GetIndexBuffer();
				//GraphicsCommandContext->SetIndexBuffer(IndexBuffer);
				//auto VertexBuffer = Mesh->GetVertexBuffer();
				//GraphicsCommandContext->SetVertexBuffer(VertexBuffer);

				Descriptor.StructureIndex = MaterialInstance ? MaterialInstance->GetStructureId() : DefaultMatInstance->GetStructureId();
				Descriptor.MaterialIndex = MaterialInstance	? MaterialInstance->GetId()	: DefaultMatInstance->GetId();
				Descriptor.CameraConstantBuffer = CameraCBs[index].Get()->GetBindlessIndex();
				Descriptor.ObjectConstantBuffer = Mesh->GetMeshConstantBuffer()->GetBindlessIndex();
				Descriptor.TimeConstantBuffer = TimeCB->GetBindlessIndex();

				if (!Descriptor.IsValid(bInstance))
					throw std::runtime_error("Invalid GeometrySceneDescriptor for mesh: " + Mesh->GetName());
					
				if (bEnableExecuteIndirect && LastMaterial->IsIndirectCommandAvailable())
				{
					GeometrySceneIndirectCommand Command;
					Command.Descriptor = Descriptor;
					Command.DrawArgs = Mesh->GetDrawParameters();
					IndirectCommands.push_back(Command);
				}
				else
				{
					sGeometrySceneDescriptor SceneDescriptor(Descriptor);
					CMD->SetBindlessDescriptor(0, &SceneDescriptor);
					auto DrawParams = Mesh->GetDrawParameters();
					//DrawParams.StartIndexLocation = 0;
					CMD->DrawIndexedInstanced(DrawParams);
				}
			};

		auto DrawComponent = [&](auto&& Self, EMaterialBlendMode BlendMode, sPrimitiveComponent* Component,	IGraphicsCommandContext* CMD) -> void
			{
				if (!Component || Component->IsHidden())
					return;

				if (auto MeshComponent = Cast<IMeshComponent>(Component))
					DrawMesh(BlendMode, MeshComponent->GetMesh(), CMD, true);

				const std::size_t ChildCount = Component->GetChildrenSize();
				for (std::size_t ChildIndex = 0; ChildIndex < ChildCount; ++ChildIndex)
					Self(Self, BlendMode, Component->GetChild(ChildIndex), CMD);
			};

		auto DrawQueuedMeshes = [&](std::vector<IMesh*>& Meshes, EMaterialBlendMode BlendMode, IGraphicsCommandContext* CMD)
			{
				for (IMesh* Mesh : Meshes)
					DrawMesh(BlendMode, Mesh, CMD, false);

				Meshes.clear();
				FlushIndirectCommands(CMD);
			};

		GraphicsCommandContext->BeginRecordCommandList(ECommandContextBeginState::Render);
		GraphicsCommandContext->BeginProfile(ERenderPass::GBuffer, true);

		UpdateCameraBuffer(pCamera, index, GraphicsCommandContext.get());
		GraphicsCommandContext->SetFrameBuffer(GBuffer.get());

		if (CustomScreenDimension.IsValid())
		{
			GraphicsCommandContext->SetViewport(sViewport(CustomScreenDimension));
			GraphicsCommandContext->SetScissorRect(0, 0, static_cast<std::uint32_t>(CustomScreenDimension.Height), static_cast<std::uint32_t>(CustomScreenDimension.Width));
		}
		else
		{
			if (Viewport.has_value())
				GraphicsCommandContext->SetViewport(*Viewport);
			else
				GraphicsCommandContext->SetViewport(sViewport(ScreenDimension));
			GraphicsCommandContext->SetScissorRect(0, 0, static_cast<std::uint32_t>(ScreenDimension.Height), static_cast<std::uint32_t>(ScreenDimension.Width));
		}

		auto IndexBuffer = sMeshGeometryContainerManager::Get().GetIndexBuffer();
		GraphicsCommandContext->SetIndexBuffer(IndexBuffer);

		const std::size_t LayerCount = Level->LayerCount();
		for (std::size_t Layer = 0; Layer < LayerCount; ++Layer)
		{
			const std::size_t MeshCount = Level->MeshCount(Layer);
			for (std::size_t MeshIndex = 0; MeshIndex < MeshCount; ++MeshIndex)
			{
				DrawMesh(EMaterialBlendMode::Opaque, Level->GetMesh(MeshIndex, Layer), GraphicsCommandContext.get(), true);
			}
			FlushIndirectCommands(GraphicsCommandContext.get());

			const std::size_t ActorCount = Level->ActorCount(Layer);
			for (std::size_t ActorIndex = 0; ActorIndex < ActorCount; ++ActorIndex)
			{
				auto Actor = Level->GetActor(ActorIndex, Layer);
				if (!Actor || Actor->IsHidden())
					continue;

				DrawComponent(DrawComponent, EMaterialBlendMode::Opaque, Actor->GetRootComponent(),	GraphicsCommandContext.get());
			}
			FlushIndirectCommands(GraphicsCommandContext.get());

			DrawQueuedMeshes(BlendedMeshes,	EMaterialBlendMode::Masked,	GraphicsCommandContext.get());

			DrawQueuedMeshes(LatestMeshes, EMaterialBlendMode::Opaque, GraphicsCommandContext.get());

			DrawQueuedMeshes(BlendedMeshes, EMaterialBlendMode::Masked,	GraphicsCommandContext.get());
		}

		GraphicsCommandContext->EndProfile();
		GraphicsCommandContext->FinishRecordCommandList();
		GraphicsCommandContext->ExecuteCommandList(ECommandContextExecuteType::Deferred, 0);

		auto Result = GraphicsCommandContext->GetProfileResult();
		if (Result.IsValid())
			Engine::GetActiveCanvas()->AddRendererProfileResult(Result);

		bForceRecompileMaterials = false;

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

		CameraBuffer.PrevJitter = CameraBuffer.CurrJitter;
		const FVector4& Vector = pCamera->GetJitteredProjMatrix().r[2];
		CameraBuffer.CurrJitter = FVector2(Vector.X, Vector.Y);
		//Engine::WriteToConsole("Prev : " + CameraBuffer.PrevJitter.ToString() + " | Current : " + CameraBuffer.CurrJitter.ToString());

		CameraBuffer.MipBias = MipBias;
		CameraCBs[index].Get()->Map(&CameraBuffer, CMD);
	}
	
	void AddCameraConstantBuffer(std::size_t Count = 1)
	{
		for (std::size_t i = 0; i < Count; i++)
		{
			TDoubleBuffer<IConstantBuffer> DoubleBuffer;
			{
				BufferLayout BufferDesc;
				BufferDesc.Size = sizeof(sCameraBuffer);
				IConstantBuffer::SharedPtr CameraCB = IConstantBuffer::Create("CameraCB_" + std::to_string(CameraCBs.size()), BufferDesc, 0); // 0
				DoubleBuffer.First = CameraCB;
			}
			{
				BufferLayout BufferDesc;
				BufferDesc.Size = sizeof(sCameraBuffer);
				IConstantBuffer::SharedPtr CameraCB = IConstantBuffer::Create("CameraCB_" + std::to_string(CameraCBs.size() + 1), BufferDesc, 0); // 0
				DoubleBuffer.Second = CameraCB;
			}
			CameraCBs.push_back(DoubleBuffer);
		}
	}

	void RemoveLastCameraConstantBuffer()
	{
		//CameraCBs[CameraCBs.size() - 1] = nullptr;
		CameraCBs.pop_back();
	}

	void DestroyAllCameraConstantBuffers()
	{
		//for (auto& CameraCB : CameraCBs)
		//	CameraCB = nullptr;
		CameraCBs.clear();
	}

	void SetRenderSize(std::size_t InWidth, std::size_t InHeight)
	{
		ScreenDimension.Width = InWidth;
		ScreenDimension.Height = InHeight;

		sFrameBufferAttachmentInfo AttachmentInfo;
		AttachmentInfo.Desc.Dimensions.X = (std::uint32_t)ScreenDimension.Width;
		AttachmentInfo.Desc.Dimensions.Y = (std::uint32_t)ScreenDimension.Height;
		AttachmentInfo.AddFrameBuffer(GPU::GetBackBufferFormat()/*, EFrameBufferAttachmentType::RT_SRV_UAV*/); // finalColor
		AttachmentInfo.DepthFormat = GPU::GetDefaultDepthFormat();
		GBuffer = IFrameBuffer::Create("GBuffer", AttachmentInfo);
	}

	void OnUpdateUpscalerPreset(sScreenDimension NewCustomScreenDimension, float NewMipBias)
	{
		CustomScreenDimension = NewCustomScreenDimension;
		MipBias = NewMipBias;
	}

	IGraphicsCommandContext* GetCommandContext() const { return GraphicsCommandContext.get(); };
	IFrameBuffer* GetGBuffer() const { return GBuffer.get(); };
	IRenderTarget* GetMotionVector() const { return GBuffer->GetRenderTarget(4); };
	IDepthTarget* GetDepth() const { return GBuffer->GetDepthTarget(); };
	IConstantBuffer* GetCameraConstantBuffer(std::size_t index) const { return CameraCBs[index].Get(); }
	std::size_t GetCameraConstantBufferSize() const { return CameraCBs.size(); }
	sScreenDimension GetScreenDimension() const { return ScreenDimension; }

	void ForceRecompileMaterials()
	{
		bForceRecompileMaterials = true;
	}

private:
	IFrameBuffer::SharedPtr GBuffer;
	sScreenDimension ScreenDimension;

	IGraphicsCommandContext::SharedPtr GraphicsCommandContext;
	IIndirectBuffer::SharedPtr IndirectBuffer;
	sIndirectLayoutBindingDesc IndirectLayoutBindingDesc;
	bool bEnableExecuteIndirect;
	sMaterial::SharedPtr DefaultEngineMat;
	sMaterialInstance::SharedPtr DefaultMatInstance;

	std::vector<TDoubleBuffer<IConstantBuffer>> CameraCBs;

	bool bForceRecompileMaterials;
	std::vector<sMaterialInstance*> MaterialsToRecompile;

	__declspec(align(256)) struct sCameraBuffer
	{
		FMatrix ViewProjMatrix;
		FMatrix PrevViewProjMatrix;
		FVector2 CurrJitter;
		FVector2 PrevJitter;
		float MipBias;
	};
	static_assert((sizeof(sCameraBuffer) % 256) == 0, "Constant Buffer size must be 256-byte aligned");

	sCameraBuffer CameraBuffer;

	/*__declspec(align(256))*/ struct sTimeBuffer
	{
		float Time;
	};
	sTimeBuffer TimeBuffer;
	IConstantBuffer::SharedPtr TimeCB;
	//static_assert((sizeof(sTimeBuffer) % 256) == 0, "Constant Buffer size must be 256-byte aligned");
	sScreenDimension CustomScreenDimension;
	float MipBias;
};

sRenderer::sRenderer(std::size_t Width, std::size_t Height)
	: FinalRenderTarget(nullptr)
	, GraphicsCommandContext(IGraphicsCommandContext::Create())
	, World(nullptr)
	, GBufferClearMode(ERendererClear::Driver)
	, ScreenDimension(sScreenDimension(Width, Height))
	, InternalBaseRenderResolution(sScreenDimension(Width, Height))
	, GBuffer(sGBuffer::CreateUnique(Width, Height))
	, LineRenderer(sLineRenderer::Create(Width, Height))
	, CanvasRenderer(sCanvasRenderer::Create(Width, Height))
	, PostProcessRenderer(sPostProcessRenderer::CreateUnique(Width, Height))
	, ToneMapping(sToneMapping::CreateUnique(Width, Height))
	, bIsTonmapperEnabled(true)
	, pParticleRenderer(ParticleRenderer::Create(Width, Height))
	, FSR(sFSR::CreateUnique(Width, Height))
	, UpscalerType(ERendererUpscalerType::None)
	, bSoftwareDeviceUpscallerSupport(false)
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
	pParticleRenderer = nullptr;
	FSR = nullptr;
}

void sRenderer::BeginPlay()
{
	LineRenderer->BeginPlay();
	CanvasRenderer->BeginPlay();
	PostProcessRenderer->BeginPlay();
	pParticleRenderer->BeginPlay();
	if (FSR)
	{
		if (UpscalerType == ERendererUpscalerType::FSR && (GPU::GetDeviceType() != EGPUDeviceType::Software || bSoftwareDeviceUpscallerSupport))
		{
			FSR->OnEnabled(true, false);
			FSR->OnUpdatePreset = std::bind(&sGBuffer::OnUpdateUpscalerPreset, GBuffer.get(), std::placeholders::_1, std::placeholders::_2);
		}
		FSR->BeginPlay();
	}
}

void sRenderer::Tick(const double DeltaTime)
{
	if (!World)
		return;

	GBuffer->Tick(DeltaTime);
	LineRenderer->Tick(DeltaTime);
	pParticleRenderer->Tick(DeltaTime);
	if (FSR)
	{
		FSR->Tick(DeltaTime);
	}
}

void sRenderer::RegisterMaterial(sMaterial* Material)
{
	if (!Material->IsCompiled())
		CompileMaterial(Material, false);

	switch (Material->GetRenderPass())
	{
	case ERenderPass::GBuffer:
		break;
	case ERenderPass::Line:
		break;
	case ERenderPass::Particle:
		break;
	case ERenderPass::PostProcess:
		break;
	case ERenderPass::UI:
		break;
	}
}

void sRenderer::CompileMaterial(sMaterial* Material, bool bRecompile)
{
	switch (Material->GetRenderPass())
	{
	case ERenderPass::GBuffer:
	{
		if (!Material->IsCompiled())
			Material->Compile(GBuffer->GetGBuffer());
		break;
	}
	case ERenderPass::Line:
		break;
	case ERenderPass::Particle:
		break;
	case ERenderPass::PostProcess:
		break;
	case ERenderPass::UI:
		break;
	}
}

void sRenderer::CompilePipeline(IPipeline* Pipeline, bool bRecompile)
{
	switch (Pipeline->GetRenderPass())
	{
	case ERenderPass::GBuffer:
		break;
	case ERenderPass::Line:
		break;
	case ERenderPass::Particle:
		break;
	case ERenderPass::PostProcess:
		break;
	case ERenderPass::UI:
		break;
	}
}

sIndirectLayoutBindingDesc sRenderer::GetRenderPassIndirectLayoutBindingDesc(ERenderPass RenderPass) const
{
	switch (RenderPass)
	{
		case ERenderPass::GBuffer:
			return GBuffer->GetIndirectLayoutBindingDesc();
		case ERenderPass::Line:
			break;
		case ERenderPass::Particle:
			return pParticleRenderer->GetIndirectLayoutBindingDesc();
		case ERenderPass::PostProcess:
			return PostProcessRenderer->GetIndirectLayoutBindingDesc();
		case ERenderPass::UI:
			break;
	}
	return sIndirectLayoutBindingDesc();
}

IFrameBuffer* sRenderer::GetFrameBuffer(ERenderPass RenderPass) const
{
	switch (RenderPass)
	{
	case ERenderPass::GBuffer:
		return GBuffer->GetGBuffer();
	case ERenderPass::Line:
		break;
	case ERenderPass::Particle:
		break;
	case ERenderPass::PostProcess:
		break;
	case ERenderPass::UI:
		break;
	}
	return nullptr;
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

void sRenderer::BeginFrame()
{
}

/*
* To Do:
* fix Command Context for multiple call
*/
void sRenderer::Render()
{
	if (!World)
		return;

	if (GBufferClearMode == ERendererClear::Driver  /*|| GBufferClearMode == ERendererClear::Sky*/)
	{
		GBuffer->ClearGBuffer();
	}
	FinalRenderTarget = GBuffer->GetGBuffer()->GetRenderTarget(0);

	auto GameInstance = World->GetGameInstance();
	std::size_t PlayerCount = 0;
	if (GameInstance)
	{
		PlayerCount = GameInstance->GetPlayerCount();

		if ((PlayerCount + ViewportInstances.size()) != GBuffer->GetCameraConstantBufferSize())
		{
			GBuffer->DestroyAllCameraConstantBuffers();
			GBuffer->AddCameraConstantBuffer(PlayerCount + ViewportInstances.size());
		}
	}

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
			LineRenderer->Render(FinalRenderTarget, GBuffer->GetCameraConstantBuffer(i)->GetBindlessIndex(), ViewportInstance->Viewport);
			pParticleRenderer->Render(World->GetActiveLevel(), GBuffer->GetCameraConstantBuffer(i)->GetBindlessIndex(), FinalRenderTarget, ViewportInstance->Viewport);
			if (UpscalerType == ERendererUpscalerType::FSR && FSR && (GPU::GetDeviceType() != EGPUDeviceType::Software || bSoftwareDeviceUpscallerSupport))
			{
				FSR->Render(FinalRenderTarget, GBuffer->GetGBuffer()->GetRenderTarget(4), GBuffer->GetGBuffer()->GetRenderTarget(5), GBuffer->GetDepth(), ViewportInstance->pCamera.get());
				FinalRenderTarget = FSR->GetOutputRenderTarget();
			}
		}
	}

	{
		for (std::size_t i = 0; i < ViewportInstances.size(); i++)
		{
			sViewportInstance* ViewportInstance = ViewportInstances[i];
			FinalRenderTarget = GBuffer->Render(World->GetActiveLevel(), i, ViewportInstance->pCamera.get(), ViewportInstance->Viewport);
			pParticleRenderer->Render(World->GetActiveLevel(), GBuffer->GetCameraConstantBuffer(i)->GetBindlessIndex(), FinalRenderTarget, ViewportInstance->Viewport);
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
	{
		auto CommandContext = CanvasRenderer->GetCommandContext();
		CommandContext->EndProfile();

		CommandContext->FinishRecordCommandList();
		CommandContext->ExecuteCommandList(ECommandContextExecuteType::Deferred, 40);

		auto Result = CommandContext->GetProfileResult();
		if (Result.IsValid())
			Engine::GetActiveCanvas()->AddRendererProfileResult(Result);
	}

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
	if (FSR)
	{
		FSR->SetRenderSize(ScreenDimension.Width, ScreenDimension.Height);
	}
}

void sRenderer::OnInputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar)
{
	LineRenderer->OnInputProcess(MouseInput, KeyboardChar);
	CanvasRenderer->OnInputProcess(MouseInput, KeyboardChar);
	PostProcessRenderer->OnInputProcess(MouseInput, KeyboardChar);
	if (UpscalerType == ERendererUpscalerType::FSR && FSR && (GPU::GetDeviceType() != EGPUDeviceType::Software || bSoftwareDeviceUpscallerSupport))
	{
		FSR->OnInputProcess(MouseInput, KeyboardChar);
	}

	if (KeyboardChar.KeyCode == 32 && KeyboardChar.bIsPressed /*&& KeyboardChar.bIsChar*/)
	{
		//GPU::WaitForGPU();
		//GBuffer->ForceRecompileMaterials();
		//Engine::WriteToConsole("ForceRecompileMaterials");
	}
	else if (KeyboardChar.KeyCode == 8 && KeyboardChar.bIsPressed /*&& KeyboardChar.bIsChar*/)
	{
		GPU::WaitForGPU();
		SetUpscalerType(ERendererUpscalerType::None);
		Engine::WriteToConsole("SetUpscalerType::NONE");
	}
	else if (KeyboardChar.KeyCode == 13 && KeyboardChar.bIsPressed /*&& KeyboardChar.bIsChar*/)
	{
		GPU::WaitForGPU();
		SetUpscalerType(ERendererUpscalerType::FSR);
		Engine::WriteToConsole("SetUpscalerType::FSR");
	}
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
	if (UpscalerType == ERendererUpscalerType::FSR && FSR && (GPU::GetDeviceType() != EGPUDeviceType::Software || bSoftwareDeviceUpscallerSupport))
		FSR->SetRenderSize(InternalBaseRenderResolution.Width, InternalBaseRenderResolution.Height);
}

void sRenderer::SetUpscalerType(ERendererUpscalerType NewUpscalerType)
{
	UpscalerType = NewUpscalerType;

	if (UpscalerType == ERendererUpscalerType::FSR && FSR && (GPU::GetDeviceType() != EGPUDeviceType::Software || bSoftwareDeviceUpscallerSupport))
	{
		FSR->OnEnabled(true, false);
		FSR->OnUpdatePreset = std::bind(&sGBuffer::OnUpdateUpscalerPreset, GBuffer.get(), std::placeholders::_1, std::placeholders::_2);
	}
	else
	{
		FSR->OnDisabled();
		FSR->OnUpdatePreset = nullptr;
	}
}

void sRenderer::SetUpscaleMode(ERendererUpscaleMode UpscaleMode)
{
	if (UpscalerType == ERendererUpscalerType::FSR && FSR && (GPU::GetDeviceType() != EGPUDeviceType::Software || bSoftwareDeviceUpscallerSupport))
		FSR->SetUpscaleMode(UpscaleMode);
}

void sRenderer::SetFSRSharpness(float Sharpness)
{
	if (UpscalerType == ERendererUpscalerType::FSR && FSR && (GPU::GetDeviceType() != EGPUDeviceType::Software || bSoftwareDeviceUpscallerSupport))
		FSR->SetUpscaleSharpness(Sharpness);
}

void sRenderer::SetEnableFrameGen(bool bEnable, std::uint32_t Multiplier)
{
	if (UpscalerType == ERendererUpscalerType::FSR && FSR && (GPU::GetDeviceType() != EGPUDeviceType::Software || bSoftwareDeviceUpscallerSupport))
		FSR->SetFrameGenerationEnabled(bEnable);
}

void sRenderer::SetTonemapper(int Val)
{
	ToneMapping->SetTonemapper(Val);
}

int sRenderer::GetTonemapperIndex() const
{
	return ToneMapping->GetTonemapperIndex();
}

void sRenderer::SetRendererClearMode(ERendererClear Mode)
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
