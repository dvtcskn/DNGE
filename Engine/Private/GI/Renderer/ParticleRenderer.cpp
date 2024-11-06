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

ParticleRenderer::ParticleRenderer(std::size_t Width, std::size_t Height, IGraphicsCommandContext::SharedPtr CMD)
	: GraphicsCommandContext(CMD ? CMD : IGraphicsCommandContext::Create())
	, ScreenDimension(sScreenDimension(Width, Height))
{
	{
		BufferLayout BufferDesc;
		BufferDesc.Size = sizeof(sParticleCameraBuffer);
		CameraCB = IConstantBuffer::Create("sParticleCameraBuffer" + std::to_string(0), BufferDesc, 0);
	}

	sFBODesc Desc;
	Desc.Dimensions.X = (std::uint32_t)ScreenDimension.Width;
	Desc.Dimensions.Y = (std::uint32_t)ScreenDimension.Height;
	Depth = IDepthTarget::Create("ParticleDepth", GPU::GetDefaultDepthFormat(), Desc);

	{
		sPipelineDesc pPipelineDesc;
		pPipelineDesc.BlendAttribute = sBlendAttributeDesc(EBlendStateMode::eNonPremultiplied);
		pPipelineDesc.DepthStencilAttribute = sDepthStencilAttributeDesc(true, true);
		pPipelineDesc.DepthStencilAttribute.DepthTest = ECompareFunction::eGreaterEqual;
		//pPipelineDesc.DepthStencilAttribute.DepthTest = ECompareFunction::eLess;
		//pPipelineDesc.DepthStencilAttribute.DepthTest = ECompareFunction::eAlways;
		pPipelineDesc.PrimitiveTopologyType = EPrimitiveType::eTRIANGLE_LIST;
		pPipelineDesc.RasterizerAttribute = sRasterizerAttributeDesc();

		pPipelineDesc.NumRenderTargets = 1;
		pPipelineDesc.RTVFormats[0] = EFormat::BGRA8_UNORM;
		pPipelineDesc.DSVFormat = GPU::GetDefaultDepthFormat();

		std::vector<sVertexAttributeDesc> VertexLayout =
		{
			{ "POSITION",		EFormat::RGB32_FLOAT,	0, offsetof(sParticleVertexLayout, position),    false },
			{ "TEXCOORD",		EFormat::RG32_FLOAT,	0, offsetof(sParticleVertexLayout, texCoord),    false },
			{ "COLOR",			EFormat::RGBA32_FLOAT,	0, offsetof(sParticleVertexLayout, Color),       false },
			{ "INSTANCEPOS",	EFormat::RGB32_FLOAT,	1, offsetof(sParticleVertexLayout::sParticleInstanceLayout, position),	  true },
			{ "INSTANCECOLOR",	EFormat::RGBA32_FLOAT,	1, offsetof(sParticleVertexLayout::sParticleInstanceLayout, Color),		  true },
		};
		pPipelineDesc.VertexLayout = VertexLayout;

		pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eUniformBuffer, eShaderType::Vertex, 0));
		pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eUniformBuffer, eShaderType::Vertex, 10));
		pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eUniformBuffer, eShaderType::Pixel, 0));
		//pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eTexture, eShaderType::Pixel, 1));
		//pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eTexture, eShaderType::Pixel, 2));
		//pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eTexture, eShaderType::Pixel, 3));
		//pPipelineDesc.DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eTexture, eShaderType::Pixel, 4));

		pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(FileManager::GetShaderFolderW() + L"Particle.hlsl", "ParticleVS", eShaderType::Vertex));
		pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(FileManager::GetShaderFolderW() + L"Particle.hlsl", "ParticleFlatPS", eShaderType::Pixel));

		DefaultParticle_EngineMat = sMaterial::Create("DefaultEngineMat", EMaterialBlendMode::Opaque, pPipelineDesc);
		//DefaultEngineMat->BindConstantBuffer(CameraCB);
	}

	DefaultParticle_MatInstance = DefaultParticle_EngineMat->CreateInstance("DefaultParticle_EngineMatInstance");
}

ParticleRenderer::~ParticleRenderer()
{
	DefaultParticle_EngineMat = nullptr;
	DefaultParticle_MatInstance = nullptr;
	GraphicsCommandContext = nullptr;
	Depth = nullptr;
	CameraCB = nullptr;
}

void ParticleRenderer::BeginPlay()
{
}

void ParticleRenderer::Tick(const double DeltaTime)
{

}

void ParticleRenderer::Render(const ILevel* Level, ICamera* pCamera, IRenderTarget* pRT, std::optional<sViewport> Viewport)
{
	sMaterial* LastMaterial = nullptr;

	{
		GraphicsCommandContext->BeginRecordCommandList(ERenderPass::eGBuffer);

		UpdateCameraBuffer(pCamera, GraphicsCommandContext.get());

		GraphicsCommandContext->SetRenderTarget(pRT, Depth.get());

		if (Viewport.has_value())
			GraphicsCommandContext->SetViewport(*Viewport);
		else
			GraphicsCommandContext->SetViewport(sViewport(ScreenDimension));

		GraphicsCommandContext->SetScissorRect(0, 0, (std::uint32_t)ScreenDimension.Height, (std::uint32_t)ScreenDimension.Width);

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
							sMaterial* pMaterial = pMaterialInstance->GetParent();
							{
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
								LastMaterial = DefaultParticle_EngineMat.get();
								LastMaterial->ApplyMaterial(GraphicsCommandContext.get());
							}
							DefaultParticle_MatInstance->ApplyMaterialInstance(GraphicsCommandContext.get());
						}

						{
							GraphicsCommandContext->SetVertexBuffer(Particle->VertexBuffer.get());
							GraphicsCommandContext->SetVertexBuffer(Particle->InstanceBuffer.get(), 1);
							GraphicsCommandContext->SetIndexBuffer(Particle->IndexBuffer.get());

							if (Particle->bIsUpdated)
							{
								if (Particle->ParticleBufferVertexes.size() > 0)
								{
									BufferSubresource VertexResource;
									VertexResource.pSysMem = Particle->ParticleBufferVertexes.data();
									VertexResource.Size = Particle->ParticleBufferVertexes.size() * sizeof(sParticleVertexLayout::sParticleInstanceLayout);
									VertexResource.Location = 0;
									GraphicsCommandContext->UpdateBufferSubresource(Particle->InstanceBuffer.get(), &VertexResource);
								}
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
								Particle->bIsUpdated = false;
							}

							GraphicsCommandContext->SetConstantBuffer(CameraCB.get());

							GraphicsCommandContext->DrawIndexedInstanced(Particle->ObjectDrawParameters);
						}
					}
				}
			}
		}

		GraphicsCommandContext->FinishRecordCommandList();
		GraphicsCommandContext->ExecuteCommandList();
	}
}

void ParticleRenderer::UpdateCameraBuffer(ICamera* pCamera, IGraphicsCommandContext* CMD)
{
	CameraBuffer.PrevViewProjMatrix = CameraBuffer.ViewProjMatrix;
	CameraBuffer.ViewProjMatrix = pCamera->GetViewProjMatrix();
	CameraCB->Map(&CameraBuffer, CMD);
}

void ParticleRenderer::SetRenderSize(std::size_t InWidth, std::size_t InHeight)
{
	ScreenDimension.Width = InWidth;
	ScreenDimension.Height = InHeight;
}

void ParticleRenderer::OnInputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar)
{
}
