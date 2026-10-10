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
#include "ParticleRenderer.h"
#include "Gameplay/GameInstance.h"
#include "Gameplay/StaticMesh.h"
#include "Gameplay/MeshComponent.h"

#include "Utilities/FileManager.h"

struct ParticleSceneDescriptor : public IBindlessSceneDescriptor
{
	std::uint32_t VertexBufferStructureIndex = std::uint32_t(-1);
	std::uint32_t VertexBufferLocation = std::uint32_t(-1);
	std::uint32_t VertexBufferStride = std::uint32_t(-1);
	std::uint32_t VertexInstanceBufferLocation = std::uint32_t(-1);
	std::uint32_t VertexInstanceBufferStride = std::uint32_t(-1);

	std::uint32_t CameraConstantBuffer = std::uint32_t(-1);
	std::uint32_t StructureIndex = std::uint32_t(-1);
	std::uint32_t MaterialIndex = std::uint32_t(-1);
};

struct ParticleIndirectCommand
{
	ParticleSceneDescriptor Descriptor;
	sObjectDrawParameters DrawArgs;
};

struct sParticleDescriptor : public IBindlessSceneContainer
{
	ParticleSceneDescriptor Descriptor;

	sParticleDescriptor(ParticleSceneDescriptor NewDescriptor)
		: Descriptor(NewDescriptor)
	{}

	virtual const IBindlessSceneDescriptor* GetDescriptor() const override final
	{
		return &Descriptor;
	}

	virtual std::uint32_t Size() const override final
	{
		return 8;
	}

	virtual std::vector<std::uint32_t> GetAllBindlessIndices() const override final
	{
		std::vector<std::uint32_t> BindlessIndices;
		BindlessIndices.push_back(Descriptor.VertexBufferStructureIndex);
		BindlessIndices.push_back(Descriptor.VertexBufferLocation);
		BindlessIndices.push_back(Descriptor.VertexBufferStride);
		BindlessIndices.push_back(Descriptor.VertexInstanceBufferLocation);
		BindlessIndices.push_back(Descriptor.VertexInstanceBufferStride);

		BindlessIndices.push_back(Descriptor.CameraConstantBuffer);
		BindlessIndices.push_back(Descriptor.StructureIndex);
		BindlessIndices.push_back(Descriptor.MaterialIndex);
		return BindlessIndices;
	}

	virtual std::vector<std::uint32_t> GetAllMaterialInstanceBindlessIndices() const override final
	{
		std::vector<std::uint32_t> BindlessIndices;
		BindlessIndices.push_back(Descriptor.StructureIndex);
		BindlessIndices.push_back(Descriptor.MaterialIndex);
		return BindlessIndices;
	}

	virtual std::uint32_t GetMaterialInstanceSize() const override final
	{
		return 2;
	}

	virtual std::vector<std::uint32_t> GetAllConstantBufferBindlessIndices() const override final
	{
		std::vector<std::uint32_t> BindlessIndices;
		BindlessIndices.push_back(Descriptor.CameraConstantBuffer);
		return BindlessIndices;
	}

	virtual std::uint32_t GetConstantBuffereSize() const override final
	{
		return 1;
	}
};

ParticleRenderer::ParticleRenderer(std::size_t Width, std::size_t Height, IGraphicsCommandContext::SharedPtr CMD)
	: GraphicsCommandContext(CMD ? CMD : IGraphicsCommandContext::Create())
	, ScreenDimension(sScreenDimension(Width, Height))
	, bEnableExecuteIndirect(true)
{
	sFBODesc Desc;
	Desc.Dimensions.X = (std::uint32_t)ScreenDimension.Width;
	Desc.Dimensions.Y = (std::uint32_t)ScreenDimension.Height;
	Depth = IDepthTarget::Create("ParticleDepth", GPU::GetDefaultDepthFormat(), Desc);

	{
		sPipelineDesc pPipelineDesc;
		pPipelineDesc.BlendAttribute = sBlendAttributeDesc(EBlendStateMode::NonPremultiplied);
		pPipelineDesc.DepthStencilAttribute = sDepthStencilAttributeDesc(ECompareFunction::GreaterEqual, true, true);
		//pPipelineDesc.DepthStencilAttribute.DepthTest = ECompareFunction::GreaterEqual;
		//pPipelineDesc.DepthStencilAttribute.DepthTest = ECompareFunction::Less;
		//pPipelineDesc.DepthStencilAttribute.DepthTest = ECompareFunction::Always;
		pPipelineDesc.PrimitiveTopologyType = EPrimitiveType::TRIANGLE_LIST;
		pPipelineDesc.RasterizerAttribute = sRasterizerAttributeDesc();

		pPipelineDesc.VertexLayout = sVertexAttributeDesc::GetDefaultParticleVertexLayout();

		pPipelineDesc.Bindings.push_back(sShaderBinding(EDescriptorType::e32BitConstant, eShaderType::All, 0, 8));
		IndirectLayoutBindingDesc = sIndirectLayoutBindingDesc(EDrawTypes::DrawIndexedInstanced, 2, sizeof(ParticleIndirectCommand), 1);
		pPipelineDesc.IndirectLayoutBindingDesc = IndirectLayoutBindingDesc;

		std::vector<sShaderAttachment> ShaderAttachments;
		pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(FileManager::GetShaderFolderW() + L"Particle.hlsl", "Particle2DVS", eShaderType::Vertex));
		pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(FileManager::GetShaderFolderW() + L"Particle.hlsl", "ParticleFlatPS", eShaderType::Pixel));

		DefaultParticle_EngineMat = sMaterial::Create("DefaultEngineMat", EMaterialBlendMode::Opaque, pPipelineDesc);
		//DefaultEngineMat->BindConstantBuffer(CameraCB);
	}

	DefaultParticle_MatInstance = DefaultParticle_EngineMat->CreateInstance("DefaultParticle_EngineMatInstance");

	IndirectBuffer = IIndirectBuffer::Create("GeometryPassIndirectBuffer", BufferLayout(1024 * 1024 * 4, sizeof(ParticleIndirectCommand)));
}

ParticleRenderer::~ParticleRenderer()
{
	IndirectBuffer = nullptr;
	DefaultParticle_EngineMat = nullptr;
	DefaultParticle_MatInstance = nullptr;
	GraphicsCommandContext = nullptr;
	Depth = nullptr;
}

void ParticleRenderer::BeginPlay()
{
}

void ParticleRenderer::Tick(const double DeltaTime)
{

}

void ParticleRenderer::Render(const ILevel* Level, std::uint32_t CameraBindlessIndex, IRenderTarget* pRT, std::optional<sViewport> Viewport)
{
	if (!Level || !pRT)
		return;

	std::vector<ParticleIndirectCommand> IndirectCommands;
	std::uint64_t ArgumentOffset = 0;
	sMaterial* LastMaterial = nullptr;

	auto FlushIndirectCommands = [&](IGraphicsCommandContext* CMD)
		{
			if (!bEnableExecuteIndirect || IndirectCommands.empty())
				return;

			const std::size_t CommandCount = IndirectCommands.size();
			IndirectBuffer->SetArgument(IndirectCommands.data(), CommandCount, ArgumentOffset);

			ArgumentOffset += sizeof(ParticleIndirectCommand) * CommandCount;
			IndirectCommands.clear();

			CMD->ExecuteIndirect(IndirectBuffer.get());
		};

	{
		GraphicsCommandContext->BeginRecordCommandList(ECommandContextBeginState::Render);

		GraphicsCommandContext->SetRenderTarget(pRT, Depth.get());

		if (Viewport.has_value())
			GraphicsCommandContext->SetViewport(*Viewport);
		else
			GraphicsCommandContext->SetViewport(sViewport(ScreenDimension));

		GraphicsCommandContext->SetScissorRect(0, 0, (std::uint32_t)ScreenDimension.Height, (std::uint32_t)ScreenDimension.Width);

		auto IndexBuffer = sMeshGeometryContainerManager::Get().GetIndexBuffer();
		GraphicsCommandContext->SetIndexBuffer(IndexBuffer);

		auto LayerCount = Level->LayerCount();

		for (std::size_t Layer = 0; Layer < LayerCount; Layer++)
		{
			auto EmitterCount = Level->EmitterCount(Layer);
			for (std::size_t i = 0; i < EmitterCount; i++)
			{
				auto Emitter = Level->GetEmitter(i, Layer);
				for (std::size_t P = 0; P < Emitter->GetParticlesSize(); P++)
				{
					MeshParticle* Particle = Cast<MeshParticle>(Emitter->GetParticle(P));
					if (Particle)
					{
						const auto& pMaterialInstance = Particle->MaterialInstance;
						if (pMaterialInstance)
						{
							if (!pMaterialInstance->IsCompiled())
								pMaterialInstance->Compile(pRT, Depth.get());

							sMaterial* pMaterial = pMaterialInstance->GetParent();
							{
								FlushIndirectCommands(GraphicsCommandContext.get());
								if (LastMaterial != pMaterial)
								{
									LastMaterial = pMaterial;
									LastMaterial->ApplyMaterial(GraphicsCommandContext.get());
								}
								pMaterialInstance->ApplyMaterialInstance(GraphicsCommandContext.get());
							}
						}
						else
						{
							if (LastMaterial != DefaultParticle_EngineMat.get())
							{
								if (!DefaultParticle_EngineMat->IsCompiled())
									DefaultParticle_EngineMat->Compile(pRT, Depth.get());

								FlushIndirectCommands(GraphicsCommandContext.get());
								LastMaterial = DefaultParticle_EngineMat.get();
								LastMaterial->ApplyMaterial(GraphicsCommandContext.get());
							}
							DefaultParticle_MatInstance->ApplyMaterialInstance(GraphicsCommandContext.get());
						}

						{
							//GraphicsCommandContext->SetVertexBuffer(Particle->VertexBuffer.get());
							//GraphicsCommandContext->SetVertexBuffer(Particle->InstanceBuffer.get(), 1);
							//GraphicsCommandContext->SetIndexBuffer(Particle->IndexBuffer.get());

							//if (Particle->bIsUpdated)
							{
								/*if (Particle->ParticleBufferVertexes.size() > 0)
								{
									BufferSubresource VertexResource;
									VertexResource.pSysMem = Particle->ParticleBufferVertexes.data();
									VertexResource.Size = Particle->ParticleBufferVertexes.size() * sizeof(sParticleVertexLayout::sParticleInstanceLayout);
									VertexResource.Location = 0;
									GraphicsCommandContext->UpdateBufferSubresource(Particle->InstanceBuffer.get(), &VertexResource);
								}*/
								/*if (Particle->ParticleBufferVertexes.size() > 0)
								{
									BufferSubresource VertexResource;
									VertexResource.pSysMem = Particle->ParticleBufferVertexes.data();
									VertexResource.Size = Particle->ParticleBufferVertexes.size() * sizeof(sParticleVertexBufferEntry);
									VertexResource.Location = 0;
									GraphicsCommandContext->UpdateBufferSubresource(Particle->VertexBuffer.get(), &VertexResource);
								}
								if (Particle->Indexes.size() > 0)
								{
									BufferSubresource IndexResource;
									IndexResource.pSysMem = Particle->Indexes.data();
									IndexResource.Size = Particle->Indexes.size() * sizeof(std::uint32_t);
									IndexResource.Location = 0;
									GraphicsCommandContext->UpdateBufferSubresource(Particle->IndexBuffer.get(), &IndexResource);
								}*/
								//Particle->bIsUpdated = false;
							}

							ParticleSceneDescriptor SceneDescriptor;

							if (Particle->GeometryHandle.IsValid())
							{
								SceneDescriptor.VertexBufferStructureIndex = Particle->GeometryHandle.GetBindlessStructureIndex();
								SceneDescriptor.VertexBufferLocation = static_cast<std::uint32_t>(Particle->GeometryHandle.GetLocation());
								SceneDescriptor.VertexBufferStride = static_cast<std::uint32_t>(Particle->GeometryHandle.GetElementSize());

								if (Particle->GeometryInstanceHandle.IsValid())
								{
									SceneDescriptor.VertexInstanceBufferLocation = static_cast<std::uint32_t>(Particle->GeometryInstanceHandle.GetLocation());
									SceneDescriptor.VertexInstanceBufferStride = static_cast<std::uint32_t>(Particle->GeometryInstanceHandle.GetElementSize());
								}
							}
							else
							{
								// invalid vertex-buffer
								return;
							}

							SceneDescriptor.CameraConstantBuffer = CameraBindlessIndex;
							if (pMaterialInstance)
							{
								SceneDescriptor.StructureIndex = pMaterialInstance->GetStructureId();
								SceneDescriptor.MaterialIndex = pMaterialInstance->GetId();
							}

							if (bEnableExecuteIndirect && LastMaterial->IsIndirectCommandAvailable())
							{
								ParticleIndirectCommand Command;
								Command.Descriptor = SceneDescriptor;
								Command.DrawArgs = Particle->ObjectDrawParameters;
								IndirectCommands.push_back(Command);
							}
							else
							{
								//GraphicsCommandContext->Set32BitConstants(0, &SceneDescriptor, 3, 0);
								sParticleDescriptor Descriptor(SceneDescriptor);
								GraphicsCommandContext->SetBindlessDescriptor(0, &Descriptor);

								GraphicsCommandContext->DrawIndexedInstanced(Particle->ObjectDrawParameters);
							}
						}
					}
				}
			}
		}

		FlushIndirectCommands(GraphicsCommandContext.get());
		GraphicsCommandContext->FinishRecordCommandList();
		GraphicsCommandContext->ExecuteCommandList(ECommandContextExecuteType::Deferred, 2);
	}
}

void ParticleRenderer::SetRenderSize(std::size_t InWidth, std::size_t InHeight)
{
	ScreenDimension.Width = InWidth;
	ScreenDimension.Height = InHeight;
}

void ParticleRenderer::OnInputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar)
{
}
