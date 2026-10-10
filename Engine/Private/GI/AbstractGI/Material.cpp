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
#include "AbstractGI/Material.h"
#include "AbstractGI/TextureManager.h"

MaterialInstanceBindlessHandle::~MaterialInstanceBindlessHandle()
{
	Release();
}

void MaterialInstanceBindlessHandle::Release()
{
	if (!Owner.expired())
	{
		if (std::shared_ptr<sMaterialInstanceContainer> PTR = Owner.lock())
			PTR->Free(this);
	}

	Owner.reset();

	StructureIndex = std::uint32_t(-1);
	BindlessIndex = std::uint32_t(-1);
}

void MaterialInstanceBindlessHandle::UpdateMaterialInstanceDescriptor(const IMaterialInstanceDescriptor* Data)
{
	if (!Owner.expired())
	{
		if (std::shared_ptr<sMaterialInstanceContainer> PTR = Owner.lock())
			PTR->UpdateMaterialInstanceDescriptor(this, Data);
	}
}

sMaterialInstance::sMaterialInstance(sMaterial* InParent, std::string& InName)
	: Parent(InParent)
	, Name(InName)
	, Path(InName)
	, Handle(MaterialInstanceBindlessHandle(eMaterialLayoutType::Default))
	, BindingLayout(new sMaterialBindingDefaultLayout())
{
	sMaterialInstanceContainerManager::Get().AllocateMaterialInstanceDescriptor(&Handle);
}

bool sMaterialInstance::IsCompiled() const 
{
	return Parent->IsCompiled();
}

bool sMaterialInstance::Compile(IFrameBuffer* FrameBuffer)
{
	return Parent->Compile(FrameBuffer);
}

bool sMaterialInstance::Compile(IRenderTarget* RT, IDepthTarget* Depth)
{
	return Parent->Compile(RT, Depth);
}

bool sMaterialInstance::Compile(std::vector<IRenderTarget*> RTs, IDepthTarget* Depth)
{
	return Parent->Compile(RTs, Depth);
}

bool sMaterialInstance::Recompile()
{
	return Parent->Recompile();
}

void sMaterialInstance::ApplyMaterialInstance(IGraphicsCommandContext* InCMDBuffer) const
{
	//if (GPU::IsBindlessRendererEnabled())
	//{

	//}
	//else
	//{
	//	//std::uint32_t i = 0;
	//	for (auto& Texture : Textures)
	//	{
	//		InCMDBuffer->SetTexture2D(Texture.get()/*, Texture->GetDefaultRootParameterIndex() + i*/);
	//		//i++;
	//	}

	//	for (auto& ConstantBuffer : ConstantBuffers)
	//	{
	//		InCMDBuffer->SetConstantBuffer(ConstantBuffer.get());
	//	}

	//	for (auto& UAV : UnorderedAccessBuffers)
	//	{
	//		InCMDBuffer->SetUnorderedAccessBufferAsResource(UAV.get());
	//	}
	//}
}

void sMaterialInstance::SetMaterialInstanceBindingLayout(IMaterialBindingLayout* NewBindingLayout)
{
	if (!NewBindingLayout)
		return;

	MaterialInstanceAttributes Attributes;
	if (BindingLayout)
	{
		Attributes = BindingLayout->GetAttributes();
		delete BindingLayout;
	}
	BindingLayout = NewBindingLayout;
	BindingLayout->SetAttributes(Attributes);

	Handle.Release();
	Handle = MaterialInstanceBindlessHandle(BindingLayout->GetLayoutType());
	sMaterialInstanceContainerManager::Get().AllocateMaterialInstanceDescriptor(&Handle);

	for (std::size_t i = 0; i < Textures.size(); i++)
	{
		auto Texture = Textures[i];
		if (BindingLayout->AddTextureBinding(i, Texture->GetBindlessIndex()))
		{
			Handle.UpdateMaterialInstanceDescriptor(BindingLayout->GetDescriptor());
		}
		else
		{
			Engine::WriteToConsole("Material Binding Layout size too small, couldn't add Texture Binding.");
			break;
		}
	}

	for (std::size_t i = 0; i < ConstantBuffers.size(); i++)
	{
		auto ConstantBuffer = ConstantBuffers[i];
		if (BindingLayout->AddConstantBufferBinding(i, ConstantBuffer->GetBindlessIndex()))
		{
			Handle.UpdateMaterialInstanceDescriptor(BindingLayout->GetDescriptor());
		}
		else
		{
			Engine::WriteToConsole("Material Binding Layout size too small, couldn't add Constant Buffer Binding.");
			break;
		}
	}

	for (std::size_t i = 0; i < UnorderedAccessBuffers.size(); i++)
	{
		auto UAV = UnorderedAccessBuffers[i];
		if (BindingLayout->AddUAVBinding(i, UAV->GetBindlessIndex()))
		{
			Handle.UpdateMaterialInstanceDescriptor(BindingLayout->GetDescriptor());
		}
		else
		{
			Engine::WriteToConsole("Material Binding Layout size too small, couldn't add UAV Binding.");
			break;
		}
	}
}

std::uint32_t sMaterialInstance::GetStructureId() const
{
	return Handle.GetBindlessStructureIndex();
}

std::uint32_t sMaterialInstance::GetId() const
{
	//std::hash<std::string> StringHasher;
	//return StringHasher(Name);
	return Handle.GetBindlessIndex();
}

ERenderPass sMaterialInstance::GetRenderPass() const
{
	return Parent->GetRenderPass();
}

bool sMaterialInstance::AddTexture(const ITexture2D::SharedPtr& Texture)
{
	if (!BindingLayout || !Texture)
		return false;
	if (BindingLayout->AddTextureBinding((std::uint32_t)Textures.size(), Texture->GetBindlessIndex()))
	{
		Textures.push_back(Texture);
		Handle.UpdateMaterialInstanceDescriptor(BindingLayout->GetDescriptor());
		return true;
	}
	return false;
}

bool sMaterialInstance::AddTexture(std::wstring InPath, std::string InName, std::uint32_t DefaultRootParameterIndex)
{
	if (!BindingLayout)
		return false;
	auto Texture = ITexture2D::Create(InPath, InName, DefaultRootParameterIndex);
	if (!Texture)
		return false;
	if (BindingLayout->AddTextureBinding((std::uint32_t)Textures.size(), Texture->GetBindlessIndex()))
	{
		Textures.push_back(Texture);
		Handle.UpdateMaterialInstanceDescriptor(BindingLayout->GetDescriptor());
		return true;
	}
	else
	{
		Texture = nullptr;
		sTextureManager::Get().DestroyTexture(InPath, InName);
		return false;
	}

	return true;
}

bool sMaterialInstance::AddTexture(std::string InName, void* InData, size_t InSize, sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
{
	if (!BindingLayout)
		return false;
	auto Texture = ITexture2D::Create(Name + "_" + std::to_string(Textures.size()), InData, InSize, InDesc, DefaultRootParameterIndex);
	if (!Texture)
		return false;
	if (BindingLayout->AddTextureBinding((std::uint32_t)Textures.size(), Texture->GetBindlessIndex()))
	{
		Textures.push_back(Texture);
		Handle.UpdateMaterialInstanceDescriptor(BindingLayout->GetDescriptor());
		return true;
	}
	else
	{
		Texture = nullptr;
		return false;
	}
	return false;
}

bool sMaterialInstance::AddSampler(const ISamplerState::SharedPtr& Sampler)
{
	if (!BindingLayout || !Sampler)
		return false;
	if (BindingLayout->AddSamplerBinding((std::uint32_t)Samplers.size(), Sampler->GetBindlessIndex()))
	{
		Samplers.push_back(Sampler);
		Handle.UpdateMaterialInstanceDescriptor(BindingLayout->GetDescriptor());
		return true;
	}
	return false;
}

bool sMaterialInstance::AddSampler(const sSamplerAttributeDesc& Desc)
{
	if (!BindingLayout)
		return false;
	auto Sampler = ISamplerState::Create(Name + "_Sampler_" + std::to_string(Samplers.size()), Desc);
	if (!Sampler)
		return false;
	if (BindingLayout->AddSamplerBinding((std::uint32_t)Samplers.size(), Sampler->GetBindlessIndex()))
	{
		Samplers.push_back(Sampler);
		Handle.UpdateMaterialInstanceDescriptor(BindingLayout->GetDescriptor());
		return true;
	}
	else
	{
		Sampler = nullptr;
	}
	return false;
}

bool sMaterialInstance::AddSampler(std::uint32_t Index, std::uint32_t BindlessIndex)
{
	if (!BindingLayout)
		return false;
	if (BindingLayout->AddSamplerBinding(Index, BindlessIndex))
	{
		Handle.UpdateMaterialInstanceDescriptor(BindingLayout->GetDescriptor());
		return true;
	}
	return false;
}

bool sMaterialInstance::SetAttributes(MaterialInstanceAttributes Attributes)
{
	if (!BindingLayout)
		return false;

	if (BindingLayout->SetAttributes(Attributes))
	{
		Handle.UpdateMaterialInstanceDescriptor(BindingLayout->GetDescriptor());
		return true;
	}
	return false;
}

std::vector<std::uint32_t> sMaterialInstance::GetTextureHeapIndices() const
{
	std::vector<std::uint32_t> Indices;
	for (const auto& Texture : Textures)
	{
		Indices.push_back(Texture->GetBindlessIndex());
	}
	return Indices;
}

std::vector<std::uint32_t> sMaterialInstance::GetConstantBufferHeapIndices() const
{
	std::vector<std::uint32_t> Indices;
	for (const auto& ConstantBuffer : ConstantBuffers)
	{
		Indices.push_back(ConstantBuffer->GetBindlessIndex());
	}
	return Indices;
}

bool sMaterialInstance::BindConstantBuffer(IConstantBuffer::SharedPtr InCB)
{
	if (!BindingLayout || !InCB)
		return false;
	if (BindingLayout->AddConstantBufferBinding((std::uint32_t)ConstantBuffers.size(), InCB->GetBindlessIndex()))
	{
		ConstantBuffers.push_back(InCB);
		Handle.UpdateMaterialInstanceDescriptor(BindingLayout->GetDescriptor());
		return true;
	}
	return false;
}

bool sMaterialInstance::BindUnorderedAccessBuffer(IStructuredBuffer::SharedPtr UAV)
{
	if (!BindingLayout || !UAV)
		return false;
	if (BindingLayout->AddUAVBinding((std::uint32_t)UnorderedAccessBuffers.size(), UAV->GetBindlessIndex()))
	{
		UnorderedAccessBuffers.push_back(UAV);
		Handle.UpdateMaterialInstanceDescriptor(BindingLayout->GetDescriptor());
		return true;
	}
	return false;
}

IConstantBuffer::SharedPtr sMaterialInstance::GetConstantBuffer(std::string Name) const
{
	for (auto CBs : ConstantBuffers)
	{
		if (CBs->GetName() == Name)
		{
			return CBs;
		}
	}
	return nullptr;
}

void sMaterialInstance::UpdateTexture(std::size_t TextureIndex, const void* pSrcData, std::size_t RowPitch, std::size_t MinX, std::size_t MinY, std::size_t MaxX, std::size_t MaxY)
{
	Textures[(uint32_t)TextureIndex]->UpdateTexture(pSrcData, RowPitch, MinX, MinY, MaxX, MaxY);
}

bool sMaterialInstance::IsIndirectCommandAvailable() const
{
	return Parent && Parent->IsIndirectCommandAvailable();
}

void sMaterialInstance::Serialize() const
{
	/*MaterialInstanceAsset Asset;
	Asset.Name = Name;
	Asset.ParentName = ParentName;
	Asset.Path = "..\\Content\\Assets\\Materials\\" + Name + "_" + ParentName + "_Instance.DNGEAsset";;
	Asset.TextureSize = Textures.size();

	for (auto& Texture : Textures)
	{
		MaterialInstanceAsset::CTexture CTexture;
		CTexture.Location = Texture.second->GetPath();
		CTexture.Slot = Texture.first;
		Asset.TextureLocations.push_back(CTexture);
	}

	Asset.Write();*/
}

sMaterial::sMaterial(std::string InName, EMaterialBlendMode InBlendMode, sPipelineDesc InPipelineDesc)
	: Name(InName)
	, MaterialUsage(EMaterialUsage::BeforPostProcess)
	, BlendMode(InBlendMode)
	, Path(InName)
{
	Pipeline = IPipeline::Create(InName, InPipelineDesc);
}

sMaterial::sMaterial(std::string InName, EMaterialBlendMode InBlendMode, sPipelineDesc InPipelineDesc, std::vector<sShaderBinding> InDescriptorSetLayout,
	std::vector<sShaderAttachment> InAttachments, std::vector<sVertexAttributeDesc> InVertexLayout)
	: Name(InName)
	, MaterialUsage(EMaterialUsage::BeforPostProcess)
	, BlendMode(InBlendMode)
	, Path(InName)
{
	Pipeline = IPipeline::Create(InName, InPipelineDesc);
}

sMaterial::~sMaterial()
{
	for (auto& Instance : Instances)
	{
		Instance = nullptr;
	}
	Instances.clear();

	for (auto& Sampler : Samplers)
	{
		Sampler.second = nullptr;
	}
	Samplers.clear();

	Pipeline = nullptr;
}

sMaterialInstance::SharedPtr sMaterial::CreateInstance(std::string InName)
{
	auto Mat = GetInstance(InName);
	if (Mat)
	{
		return Mat;
	}

	auto Instance = std::make_shared<sMaterialInstance>(this, InName);
	Instances.push_back(Instance);
	return Instance;
}

ERenderPass sMaterial::GetRenderPass() const
{
	return Pipeline->GetRenderPass();
}

bool sMaterial::AddSampler(const ISamplerState::SharedPtr& Sampler)
{
	if (!Sampler)
		return false;
	Samplers.insert({ Samplers.size(), Sampler });
	return true;
}

bool sMaterial::AddSampler(const sSamplerAttributeDesc& Desc)
{
	auto Sampler = ISamplerState::Create(Name + "_Sampler_" + std::to_string(Samplers.size()), Desc);
	if (!Sampler)
		return false;
	Samplers.insert({ Samplers.size(), Sampler });
	return true;
}

bool sMaterial::Compile(IFrameBuffer* FrameBuffer)
{
	return Pipeline->Compile(FrameBuffer);
}

bool sMaterial::Compile(IRenderTarget* RT, IDepthTarget* Depth)
{
	return Pipeline->Compile(RT, Depth);
}

bool sMaterial::Compile(std::vector<IRenderTarget*> RTs, IDepthTarget* Depth)
{
	return Pipeline->Compile(RTs, Depth);
}

bool sMaterial::Recompile()
{
	return Pipeline->Recompile();
}

bool sMaterial::IsIndirectCommandAvailable() const
{
	return Pipeline && Pipeline->IsIndirectCommandAvailable();
}

void sMaterial::ApplyMaterial(IGraphicsCommandContext* InCMDBuffer) const
{
	InCMDBuffer->SetPipeline(Pipeline.get());
}

void sMaterial::Serialize()
{
	/*MaterialAsset mMaterialAsset;
	mMaterialAsset.Name = Name;
	mMaterialAsset.Path = "..\\Content\\Assets\\Materials\\" + Name + ".DNGEAsset";
	mMaterialAsset.MaterialType = MaterialType;
	mMaterialAsset.MaterialUsage = MaterialUsage;
	mMaterialAsset.BlendMode = BlendMode;

	mMaterialAsset.mPipelineAsset.Name = Name + "_" + "Pipeline";
	mMaterialAsset.mPipelineAsset.pPipelineDesc = Pipeline->GetInterface()->GetPipelineDesc();

	mMaterialAsset.mPipelineAsset.DescriptorSetLayout = Pipeline->GetRootSignature();
	mMaterialAsset.mPipelineAsset.ShaderAttachments = Pipeline->GetShaderAttachments();
	mMaterialAsset.mPipelineAsset.VertexLayout = Pipeline->GetVertexAttribute();

	mMaterialAsset.Write();*/
}
