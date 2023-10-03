#pragma once

#include <memory>
#include <string>
#include <map>
#include <vector>

#include "Engine/ClassBody.h"
#include "Engine/AbstractEngine.h"

enum class EMaterialType
{
	Actor,
	UI,
	PostProcess,
};

enum class EMaterialBlendMode
{
	Opaque,
	Masked,
};

enum class EMaterialUsage
{
	BeforPostProcess,
	AfterPostProcess,
};

__declspec(align(16))
struct sMaterialAttributes
{
	FVector4 DiffuseColor;
	std::uint32_t Masked;
	std::uint32_t Metalic;
	std::uint32_t Specular;
	std::uint32_t Roughness;
	std::uint32_t PBR_NumRadianceMipLevels;
	bool bIsSolid;

	explicit sMaterialAttributes()
		: Masked(NULL)
		, Metalic(NULL)
		, Specular(NULL)
		, Roughness(NULL)
		, PBR_NumRadianceMipLevels(NULL)
		, DiffuseColor(FVector4(0.0f, 0.0f, 0.0f, 0.0f))
		, bIsSolid(false)
	{}
};

class sMaterial : public std::enable_shared_from_this<sMaterial>
{
	sBaseClassBody(sClassConstructor, sMaterial)
public:
	class sMaterialInstance : public std::enable_shared_from_this<sMaterialInstance>
	{
		sBaseClassBody(sClassNoDefaults, sMaterialInstance)
	private:
		friend sMaterial;

		sMaterial* Parent;
		std::string Name;
		std::string Path;
		std::string ParentName;

	public:
		std::vector<ITexture2D::SharedPtr> Textures;
		std::vector<IConstantBuffer::SharedPtr> ConstantBuffers;

	public:
		sMaterialInstance(sMaterial* InParent, std::string& InName)
			: Parent(InParent)
			, Name(InName)
			, ParentName(InParent->GetName())
			, Path(InName)
		{}

		virtual ~sMaterialInstance()
		{
			for (auto& Texture : Textures)
			{
				Texture = nullptr;
			}
			Textures.clear();

			for (auto& ConstantBuffer : ConstantBuffers)
			{
				ConstantBuffer = nullptr;
			}
			ConstantBuffers.clear();

			Parent = nullptr;
		}

		FORCEINLINE std::string GetPath() const { return Path; };
		FORCEINLINE void SetPath(std::string InPath) { Path = InPath; };

		FORCEINLINE sMaterial* GetParent() const
		{
			return Parent;
		}

		FORCEINLINE std::string GetName() const { return Name; };

		FORCEINLINE ITexture2D* GetTexture(std::uint32_t index) const { return Textures.at(index).get(); };
		FORCEINLINE IConstantBuffer* GetConstantBuffer(std::uint32_t index) const { return ConstantBuffers.at(index).get(); };

		void AddTexture(const ITexture2D::SharedPtr& Texture);
		void AddTexture(std::string InName, void* InData, size_t InSize, sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex);
		void AddTexture(std::wstring InPath, std::string InName, std::uint32_t DefaultRootParameterIndex);

		void ApplyMaterialInstance(IGraphicsCommandContext* InCMDBuffer = nullptr) const;

		void BindConstantBuffer(IConstantBuffer::SharedPtr InCB);

		IConstantBuffer::SharedPtr GetConstantBuffer(std::string Name) const;

		void UpdateTexture(std::size_t TextureIndex, const void* pSrcData, std::size_t RowPitch, std::size_t MinX, std::size_t MinY, std::size_t MaxX, std::size_t MaxY);

		void Serialize() const;
	};

public:
	static sMaterial::SharedPtr CreateMaterial(std::string InName, EMaterialBlendMode InBlendMode, sPipelineDesc InPipelineDesc, EMaterialType InType = EMaterialType::Actor)
	{
		auto Mat = Create(InName, InBlendMode, InPipelineDesc, InType);
		return Mat;
	}

	static sMaterial::SharedPtr CreateMaterial(std::string InName, EMaterialBlendMode InBlendMode, sPipelineDesc InPipelineDesc, std::vector<sDescriptorSetLayoutBinding> InDescriptorSetLayout,
		std::vector<sShaderAttachment> InAttachments, std::vector<sVertexAttributeDesc> InVertexLayout = std::vector<sVertexAttributeDesc>(), EMaterialType InType = EMaterialType::Actor)
	{
		auto Mat = Create(InName, InBlendMode, InPipelineDesc, InDescriptorSetLayout, InAttachments, InVertexLayout, InType);
		return Mat;
	}

	sMaterial(std::string InName, EMaterialBlendMode InBlendMode, sPipelineDesc InPipelineDesc, EMaterialType InType = EMaterialType::Actor);
	sMaterial(std::string InName, EMaterialBlendMode InBlendMode, sPipelineDesc InPipelineDesc, std::vector<sDescriptorSetLayoutBinding> InDescriptorSetLayout,
		std::vector<sShaderAttachment> InAttachments, std::vector<sVertexAttributeDesc> InVertexLayout = std::vector<sVertexAttributeDesc>(), EMaterialType InType = EMaterialType::Actor);

	virtual ~sMaterial();
	sMaterialInstance::SharedPtr CreateInstance(std::string InName);

	void RecompileProgram();

	FORCEINLINE std::string GetName() const { return Name; };
	FORCEINLINE std::string GetPath() const { return Path; };
	FORCEINLINE void SetPath(std::string InPath) { Path = InPath; };
	FORCEINLINE IPipeline::SharedPtr GetPipeline() const { return Pipeline; };

	FORCEINLINE std::vector<sDescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings() const { return Pipeline->GetPipelineDesc().DescriptorSetLayout; };
	FORCEINLINE std::vector<sMaterialInstance::SharedPtr> GetInstances() const { return Instances; };

	FORCEINLINE sMaterialInstance::SharedPtr GetInstance(std::string InName) const
	{
		for (auto& Instance : Instances)
		{
			if (Instance->GetName() == InName)
			{
				return Instance;
			}
		}
		return nullptr;
	};

	FORCEINLINE sMaterialInstance::SharedPtr GetInstance(std::size_t Index) const
	{
		if (Index < Instances.size())
			return Instances.at(Index);
		return nullptr;
	};

	FORCEINLINE void DestroyInstance(std::string InstanceName)
	{
		sMaterial::sMaterialInstance::SharedPtr MatInstance = nullptr;
		for (auto& Instance : Instances)
		{
			if (Instance->GetName() == InstanceName)
			{
				MatInstance = Instance;
			}
		}
		Instances.erase(std::find(Instances.begin(), Instances.end(), MatInstance));
		MatInstance = nullptr;
	}

	void BindConstantBuffer(IConstantBuffer::SharedPtr InCB);
	IConstantBuffer::SharedPtr GetConstantBuffer(std::string Name) const;

	void ApplyMaterial(IGraphicsCommandContext* InCMDBuffer = nullptr) const;

	EMaterialType MaterialType;
	EMaterialUsage MaterialUsage;
	EMaterialBlendMode BlendMode;

	void Serialize();

private:
	friend sMaterialInstance;

	std::string Name;
	std::string Path;
	IPipeline::SharedPtr Pipeline;

	std::vector<sMaterialInstance::SharedPtr> Instances;
	std::vector<IConstantBuffer::SharedPtr> ConstantBuffers;
};
