
#include "pch.h"
#include "AbstractGI/Material.h"

void sMaterial::sMaterialInstance::AddTexture(const ITexture2D::SharedPtr& Texture)
{
	Textures.push_back(Texture);
}

void sMaterial::sMaterialInstance::AddTexture(std::wstring InPath, std::string InName, std::uint32_t DefaultRootParameterIndex)
{
	Textures.push_back(ITexture2D::Create(InPath, InName, DefaultRootParameterIndex));
}

void sMaterial::sMaterialInstance::AddTexture(std::string InName, void* InData, size_t InSize, sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
{
	Textures.push_back(ITexture2D::Create(Name + "_" + std::to_string(Textures.size()), InData, InSize, InDesc, DefaultRootParameterIndex));
}

void sMaterial::sMaterialInstance::BindConstantBuffer(IConstantBuffer::SharedPtr InCB)
{
	ConstantBuffers.push_back(InCB);
}

IConstantBuffer::SharedPtr sMaterial::sMaterialInstance::GetConstantBuffer(std::string Name) const
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

void sMaterial::sMaterialInstance::UpdateTexture(std::size_t TextureIndex, const void* pSrcData, std::size_t RowPitch, std::size_t MinX, std::size_t MinY, std::size_t MaxX, std::size_t MaxY)
{
	Textures[(uint32_t)TextureIndex]->UpdateTexture(pSrcData, RowPitch, MinX, MinY, MaxX, MaxY);
}

void sMaterial::sMaterialInstance::ApplyMaterialInstance(IGraphicsCommandContext* InCMDBuffer) const
{
	for (auto& Texture : Textures)
	{
		InCMDBuffer->SetTexture2D(Texture.get());
	}

	for (auto& ConstantBuffer : ConstantBuffers)
	{
		InCMDBuffer->SetConstantBuffer(ConstantBuffer.get());
	}
}

void sMaterial::sMaterialInstance::Serialize() const
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

sMaterial::sMaterialInstance::SharedPtr sMaterial::CreateInstance(std::string InName)
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

sMaterial::sMaterial(std::string InName, EMaterialBlendMode InBlendMode, sPipelineDesc InPipelineDesc, EMaterialType InType)
	: Name(InName)
	, MaterialType(InType)
	, MaterialUsage(EMaterialUsage::BeforPostProcess)
	, BlendMode(InBlendMode)
	, Path(InName)
{
	Pipeline = IPipeline::Create(InName, InPipelineDesc);
}

sMaterial::sMaterial(std::string InName, EMaterialBlendMode InBlendMode, sPipelineDesc InPipelineDesc, std::vector<sDescriptorSetLayoutBinding> InDescriptorSetLayout,
	std::vector<sShaderAttachment> InAttachments, std::vector<sVertexAttributeDesc> InVertexLayout, EMaterialType InType)
	: Name(InName)
	, MaterialType(InType)
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

	for (auto& ConstantBuffer : ConstantBuffers)
	{
		ConstantBuffer = nullptr;
	}
	ConstantBuffers.clear();

	Pipeline = nullptr;
}

void sMaterial::RecompileProgram()
{
	Pipeline->Recompile();
}

void sMaterial::ApplyMaterial(IGraphicsCommandContext* InCMDBuffer) const
{
	InCMDBuffer->SetPipeline(Pipeline.get());

	for (auto& ConstantBuffer : ConstantBuffers)
	{
		InCMDBuffer->SetConstantBuffer(ConstantBuffer.get());
	}
}

IConstantBuffer::SharedPtr sMaterial::GetConstantBuffer(std::string Name) const
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

void sMaterial::BindConstantBuffer(IConstantBuffer::SharedPtr InCB)
{
	ConstantBuffers.push_back(InCB);
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
