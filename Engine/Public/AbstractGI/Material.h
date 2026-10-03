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
#pragma once

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <utility>

#include "Engine/ClassBody.h"
#include "Engine/AbstractEngine.h"

class sMaterial;
class sMaterialInstance;
class sMaterialInstanceContainerManager;
class sMaterialInstanceContainer;

enum class eMaterialLayoutType : std::uint8_t
{
	Default,
	PBR = Default,

	All,
};

struct MaterialInstanceAttributes
{
	FVector4 BaseColor = FVector4(1.0f, 1.0f, 1.0f, 1.0f);

	float Metallic = 0.0f;
	float Roughness = 0.0f;
	float _padding1[2];

	FVector Emissive = FVector(1.0f, 1.0f, 1.0f);
	float _padding2;
};

struct IMaterialInstanceDescriptor {};
struct MaterialInstanceDescriptor : public IMaterialInstanceDescriptor
{
	std::uint32_t AlbedoTextureIdx = std::uint32_t(-1);
	std::uint32_t SamplerIdx = std::uint32_t(-1);
	std::uint32_t NormalTextureIdx = std::uint32_t(-1);
	std::uint32_t MetallicRoughnessTextureIdx = std::uint32_t(-1);
	std::uint32_t EmissiveTextureIdx = std::uint32_t(-1);
	std::uint32_t _padding1[3];

	MaterialInstanceAttributes Attributes;
};

class MaterialInstanceBindlessHandle
{
	sBaseClassBody(sClassConstructor, MaterialInstanceBindlessHandle)
public:
	MaterialInstanceBindlessHandle(eMaterialLayoutType NewLayoutType)
		: LayoutType(NewLayoutType)
		, Owner(std::weak_ptr<sMaterialInstanceContainer>())
		, StructureIndex(std::uint32_t(-1))
		, BindlessIndex(std::uint32_t(-1))
	{}
	MaterialInstanceBindlessHandle(std::weak_ptr<sMaterialInstanceContainer> NewOwner, eMaterialLayoutType NewLayoutType, std::uint32_t NewStructureIndex, std::uint32_t NewBindlessIndex)
		: Owner(NewOwner)
		, LayoutType(NewLayoutType)
		, StructureIndex(NewStructureIndex)
		, BindlessIndex(NewBindlessIndex)
	{}
	~MaterialInstanceBindlessHandle();

	void Release();

	inline void Reset(std::weak_ptr<sMaterialInstanceContainer> NewOwner, eMaterialLayoutType NewLayoutType, std::uint32_t NewStructureIndex, std::uint32_t NewBindlessIndex)
	{
		Owner = NewOwner;
		StructureIndex = NewStructureIndex;
		BindlessIndex = NewBindlessIndex;
		LayoutType = NewLayoutType;
	}
	void UpdateMaterialInstanceDescriptor(const IMaterialInstanceDescriptor* Data);

	inline std::uint32_t GetBindlessStructureIndex() const
	{
		return StructureIndex;
	}
	inline std::uint32_t GetBindlessIndex() const
	{
		return BindlessIndex;
	}

	inline eMaterialLayoutType GetLayoutType() const
	{
		return LayoutType;
	}

	std::weak_ptr<sMaterialInstanceContainer> Owner;
	std::uint32_t StructureIndex;
	std::uint32_t BindlessIndex;
	eMaterialLayoutType LayoutType;
};

/*
* to do : make base and template
*/
class sMaterialInstanceContainer : public std::enable_shared_from_this<sMaterialInstanceContainer>
{
	sBaseClassBody(sClassConstructor, sMaterialInstanceContainer)
public:
	sMaterialInstanceContainer(sMaterialInstanceContainerManager* NewOwner, uint32_t NewSize, eMaterialLayoutType NewLayoutType)
		: Owner(NewOwner)
		, Size(NewSize)
		, AllocatedDescriptorCount(0)
		, Container(nullptr)
		, LayoutType(NewLayoutType)
	{
		std::string Name = "MaterialInstanceContainer_";
		BufferLayout Layout;
		Layout.Size = Size;
		switch (LayoutType)
		{
		case eMaterialLayoutType::Default:
			Name += "Default";
			Layout.Stride = sizeof(MaterialInstanceDescriptor);
			break;
		}
		Container = IStructuredBuffer::Create(Name, Layout, true);
	}

	~sMaterialInstanceContainer()
	{
		Size = 0;
		AllocatedDescriptorCount = 0;

		FreedDescriptorIndices.clear();
		LayoutType = eMaterialLayoutType::All;

		Container = nullptr;
		Owner = nullptr;
	}

	inline bool AllocateMaterialInstanceDescriptor(MaterialInstanceBindlessHandle* Handle)
	{
		if (FreedDescriptorIndices.size() > 0)
		{
			uint32_t BindlessIndex = FreedDescriptorIndices.back();
			FreedDescriptorIndices.pop_back();
			Handle->Reset(weak_from_this(), LayoutType, Container->GetBindlessIndex(), BindlessIndex);
		}
		else
		{
			if (AllocatedDescriptorCount >= Size)
				return false;

			Handle->Reset(weak_from_this(), LayoutType, Container->GetBindlessIndex(), AllocatedDescriptorCount);
			AllocatedDescriptorCount++;
		}

		return true;
	}

	inline void UpdateMaterialInstanceDescriptor(MaterialInstanceBindlessHandle* Handle, const IMaterialInstanceDescriptor* Data)
	{
		if (Container)
			Container->Map(Data, Handle->GetBindlessIndex());
	}

	inline void Free(MaterialInstanceBindlessHandle* Handle)
	{
		if (std::find(FreedDescriptorIndices.begin(), FreedDescriptorIndices.end(), Handle->GetBindlessIndex()) != FreedDescriptorIndices.end())
			return;

		FreedDescriptorIndices.push_back(Handle->GetBindlessIndex());
		std::sort(FreedDescriptorIndices.begin(), FreedDescriptorIndices.end());

		switch (LayoutType)
		{
		case eMaterialLayoutType::Default:
		{
			if (Container)
			{
				MaterialInstanceDescriptor Descriptor;
				Container->Map(&Descriptor, Handle->GetBindlessIndex());
			}
		}
			break;
		}
	}

	inline eMaterialLayoutType GetLayoutType() const { return LayoutType; }
	inline IStructuredBuffer* GetContainer() const { return Container.get(); }

private:
	std::uint32_t Size;
	std::uint32_t AllocatedDescriptorCount;
	std::vector<std::uint32_t> FreedDescriptorIndices;

	IStructuredBuffer::SharedPtr Container;
	eMaterialLayoutType LayoutType;

	sMaterialInstanceContainerManager* Owner;
};

class sMaterialInstanceContainerManager
{
	sBaseClassBody(sClassNoDefaults, sMaterialInstanceContainerManager);
private:
	sMaterialInstanceContainerManager() = default;
	sMaterialInstanceContainerManager(const sMaterialInstanceContainerManager& Other) = delete;
	sMaterialInstanceContainerManager& operator=(const sMaterialInstanceContainerManager&) = delete;

public:
	~sMaterialInstanceContainerManager()
	{
		Release();
	}

	static sMaterialInstanceContainerManager& Get()
	{
		static sMaterialInstanceContainerManager instance;
		return instance;
	}

	inline void AllocateMaterialInstanceDescriptor(MaterialInstanceBindlessHandle* Handle)
	{
		if (Containers.contains(Handle->GetLayoutType()))
		{
			Containers[Handle->GetLayoutType()]->AllocateMaterialInstanceDescriptor(Handle);
		}
		else
		{
			Containers.insert({ Handle->GetLayoutType(), sMaterialInstanceContainer::Create(this, 16384, Handle->GetLayoutType()) });
			Containers[Handle->GetLayoutType()]->AllocateMaterialInstanceDescriptor(Handle);
		}
	}

	inline void DeallocateMaterialInstanceDescriptor(MaterialInstanceBindlessHandle* Handle)
	{
		if (Containers.contains(Handle->GetLayoutType()))
		{
			Containers[Handle->GetLayoutType()]->Free(Handle);
		}
	}

	inline void Release()
	{
		for (auto& Container : Containers)
		{
			Container.second = nullptr;
		}
		Containers.clear();
	}

private:
	std::map<eMaterialLayoutType, sMaterialInstanceContainer::SharedPtr> Containers;
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

struct IMaterialBindingLayout
{
	IMaterialBindingLayout() = default;

	virtual const IMaterialInstanceDescriptor* GetDescriptor() const
	{
		return nullptr;
	}

	virtual eMaterialLayoutType GetLayoutType() const
	{
		return eMaterialLayoutType::All;
	}

	virtual std::vector<std::uint32_t> GetAllBindlessIndices() const
	{
		return std::vector<std::uint32_t>();
	}

	virtual std::size_t TextureBindingSize() const
	{
		return 0;
	}

	virtual bool AddTextureBinding(std::size_t Index, std::uint32_t TextureBindlessIndex)
	{
		return false;
	}

	virtual std::size_t SamplerBindingSize() const
	{
		return 0;
	}

	virtual bool AddSamplerBinding(std::size_t Index, std::uint32_t SamplerBindlessIndex)
	{
		return false;
	}

	virtual MaterialInstanceAttributes GetAttributes() const
	{
		return MaterialInstanceAttributes();
	}

	virtual bool SetAttributes(MaterialInstanceAttributes Attributes)
	{
		return false;
	}

	virtual std::size_t ConstantBufferBindingSize() const
	{
		return 0;
	}

	virtual bool AddConstantBufferBinding(std::size_t Index, std::uint32_t ConstantBufferBindlessIndex)
	{
		return false;
	}

	virtual std::size_t UAVBindingSize() const
	{
		return 0;
	}

	virtual bool AddUAVBinding(std::size_t Index, std::uint32_t UAVBindlessIndex)
	{
		return false;
	}
};

struct sMaterialBindingDefaultLayout : public IMaterialBindingLayout
{
	sMaterialBindingDefaultLayout() = default;

	MaterialInstanceDescriptor Descriptor;

	virtual const IMaterialInstanceDescriptor* GetDescriptor() const override final
	{
		return &Descriptor;
	}

	virtual eMaterialLayoutType GetLayoutType() const
	{
		return eMaterialLayoutType::Default;
	}

	virtual std::vector<std::uint32_t> GetAllBindlessIndices() const override final
	{
		std::vector<std::uint32_t> Indices;
		if (Descriptor.AlbedoTextureIdx != std::uint32_t(-1))
			Indices.push_back(Descriptor.AlbedoTextureIdx);
		if (Descriptor.NormalTextureIdx != std::uint32_t(-1))
			Indices.push_back(Descriptor.NormalTextureIdx);
		if (Descriptor.MetallicRoughnessTextureIdx != std::uint32_t(-1))
			Indices.push_back(Descriptor.MetallicRoughnessTextureIdx);
		if (Descriptor.EmissiveTextureIdx != std::uint32_t(-1))
			Indices.push_back(Descriptor.EmissiveTextureIdx);
		return Indices;
	}

	virtual std::size_t TextureBindingSize() const override final
	{
		return GetAllBindlessIndices().size();
	}

	virtual bool AddTextureBinding(std::size_t Index, std::uint32_t TextureBindlessIndex) override final
	{
		switch (Index)
		{
		case 0:
			Descriptor.AlbedoTextureIdx = TextureBindlessIndex;
			return true;
		case 1:
			Descriptor.NormalTextureIdx = TextureBindlessIndex;
			return true;
		case 2:
			Descriptor.MetallicRoughnessTextureIdx = TextureBindlessIndex;
			return true;
		case 3:
			Descriptor.EmissiveTextureIdx = TextureBindlessIndex;
			return true;
		}
		return false;
	}

	virtual std::size_t SamplerBindingSize() const
	{
		if (Descriptor.SamplerIdx != std::uint32_t(-1))
			return 1;
		return 0;
	}

	virtual bool AddSamplerBinding(std::size_t Index, std::uint32_t SamplerBindlessIndex)
	{
		switch (Index)
		{
		case 0:
			Descriptor.SamplerIdx = SamplerBindlessIndex;
			return true;
		}
		return false;
	}

	virtual MaterialInstanceAttributes GetAttributes() const override final
	{
		return Descriptor.Attributes;
	}

	virtual bool SetAttributes(MaterialInstanceAttributes Attributes) override final
	{
		Descriptor.Attributes = Attributes;
		return true;
	}

	virtual std::size_t ConstantBufferBindingSize() const override final
	{
		return 0;
	}

	virtual bool AddConstantBufferBinding(std::size_t Index, std::uint32_t ConstantBufferBindlessIndex) override final
	{
		return false;
	}

	virtual std::size_t UAVBindingSize() const override final
	{
		return 0;
	}

	virtual bool AddUAVBinding(std::size_t Index, std::uint32_t UAVBindlessIndex) override final
	{
		return false;
	}
};

class sMaterialInstance : public std::enable_shared_from_this<sMaterialInstance>
{
	sBaseClassBody(sClassNoDefaults, sMaterialInstance)
private:
	sMaterial* Parent;
	std::string Name;
	std::string Path;

	MaterialInstanceBindlessHandle Handle;
	IMaterialBindingLayout* BindingLayout;

public:
	std::vector<ITexture2D::SharedPtr> Textures;
	std::vector<ISamplerState::SharedPtr> Samplers;
	std::vector<IConstantBuffer::SharedPtr> ConstantBuffers;
	std::vector<IStructuredBuffer::SharedPtr> UnorderedAccessBuffers;

public:
	sMaterialInstance(sMaterial* InParent, std::string& InName);

	virtual ~sMaterialInstance()
	{
		Handle.Release();
		if (BindingLayout)
		{
			delete BindingLayout;
			BindingLayout = nullptr;
		}

		for (auto& Texture : Textures)
		{
			Texture = nullptr;
		}
		Textures.clear();

		for (auto& Sampler : Samplers)
		{
			Sampler = nullptr;
		}
		Samplers.clear();

		for (auto& ConstantBuffer : ConstantBuffers)
		{
			ConstantBuffer = nullptr;
		}
		ConstantBuffers.clear();

		for (auto& UnorderedAccessBuffer : UnorderedAccessBuffers)
		{
			UnorderedAccessBuffer = nullptr;
		}
		UnorderedAccessBuffers.clear();

		Parent = nullptr;
	}

	FORCEINLINE std::string GetName() const { return Name; };
	FORCEINLINE std::string GetPath() const { return Path; };
	FORCEINLINE void SetPath(std::string InPath) { Path = InPath; };

	FORCEINLINE sMaterial* GetParent() const { return Parent; }

	virtual void SetMaterialInstanceBindingLayout(IMaterialBindingLayout* NewBindingLayout);

	std::uint32_t GetStructureId() const;
	std::uint32_t GetId() const;

	ERenderPass GetRenderPass() const;
	FORCEINLINE std::size_t GetTextureSize() const { return BindingLayout ? BindingLayout->TextureBindingSize() : Textures.size(); }
	FORCEINLINE ITexture2D* GetTexture(std::size_t index) const { return Textures.at(index).get(); };
	FORCEINLINE std::size_t GetTextureBindlessIndex(std::size_t index) const { return Textures.at(index)->GetBindlessIndex(); };
	FORCEINLINE std::size_t GetSamplerSize() const { return BindingLayout ? BindingLayout->SamplerBindingSize() : Samplers.size(); }
	FORCEINLINE ISamplerState* GetSampler(std::size_t index) const { return Samplers.at(index).get(); };
	FORCEINLINE std::size_t GetSamplerBindlessIndex(std::size_t index) const { return Samplers.at(index)->GetBindlessIndex(); };
	FORCEINLINE std::size_t GetConstantBufferSize() const { return BindingLayout ? BindingLayout->ConstantBufferBindingSize() : ConstantBuffers.size(); }
	FORCEINLINE IConstantBuffer* GetConstantBuffer(std::size_t index) const { return ConstantBuffers.at(index).get(); };
	FORCEINLINE std::size_t GetConstantBuffeBindlessIndex(std::size_t index) const { return ConstantBuffers.at(index)->GetBindlessIndex(); };
	FORCEINLINE std::size_t GetStructuredBufferSize() const { return BindingLayout ? BindingLayout->UAVBindingSize() : UnorderedAccessBuffers.size(); }
	FORCEINLINE IStructuredBuffer* GetUnorderedAccessBuffer(std::size_t index) const { return UnorderedAccessBuffers.at(index).get();	};
	FORCEINLINE std::size_t GetUnorderedAccessBufferBindlessIndex(std::size_t index) const { return UnorderedAccessBuffers.at(index)->GetBindlessIndex(); };

	bool IsCompiled() const;
	bool Compile(IFrameBuffer* FrameBuffer = nullptr);
	bool Compile(IRenderTarget* RT, IDepthTarget* Depth = nullptr);
	bool Compile(std::vector<IRenderTarget*> RTs, IDepthTarget* Depth = nullptr);

	void ApplyMaterialInstance(IGraphicsCommandContext* InCMDBuffer = nullptr) const;

	bool AddTexture(const ITexture2D::SharedPtr& Texture);
	bool AddTexture(std::string InName, void* InData, size_t InSize, sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex);
	bool AddTexture(std::wstring InPath, std::string InName, std::uint32_t DefaultRootParameterIndex);

	bool AddSampler(const ISamplerState::SharedPtr& Sampler);
	bool AddSampler(const sSamplerAttributeDesc& Desc);
	bool AddSampler(std::uint32_t Index, std::uint32_t BindlessIndex);

	bool SetAttributes(MaterialInstanceAttributes Attributes);

	std::vector<std::uint32_t> GetTextureHeapIndices() const;
	std::vector<std::uint32_t> GetConstantBufferHeapIndices() const;

	bool BindConstantBuffer(IConstantBuffer::SharedPtr InCB);
	bool BindUnorderedAccessBuffer(IStructuredBuffer::SharedPtr UAV);

	IConstantBuffer::SharedPtr GetConstantBuffer(std::string Name) const;

	void UpdateTexture(std::size_t TextureIndex, const void* pSrcData, std::size_t RowPitch, std::size_t MinX, std::size_t MinY, std::size_t MaxX, std::size_t MaxY);

	bool IsIndirectCommandAvailable() const;

	void Serialize() const;
};

class sMaterial : public std::enable_shared_from_this<sMaterial>
{
	sBaseClassBody(sClassConstructor, sMaterial)
public:
	static sMaterial::SharedPtr CreateMaterial(std::string InName, EMaterialBlendMode InBlendMode, sPipelineDesc InPipelineDesc)
	{
		auto Mat = Create(InName, InBlendMode, InPipelineDesc);
		return Mat;
	}

	static sMaterial::SharedPtr CreateMaterial(std::string InName, EMaterialBlendMode InBlendMode, sPipelineDesc InPipelineDesc, std::vector<sShaderBinding> InDescriptorSetLayout,
		std::vector<sShaderAttachment> InAttachments, std::vector<sVertexAttributeDesc> InVertexLayout = std::vector<sVertexAttributeDesc>())
	{
		auto Mat = Create(InName, InBlendMode, InPipelineDesc, InDescriptorSetLayout, InAttachments, InVertexLayout);
		return Mat;
	}

	sMaterial(std::string InName, EMaterialBlendMode InBlendMode, sPipelineDesc InPipelineDesc);
	sMaterial(std::string InName, EMaterialBlendMode InBlendMode, sPipelineDesc InPipelineDesc, std::vector<sShaderBinding> InDescriptorSetLayout,
		std::vector<sShaderAttachment> InAttachments, std::vector<sVertexAttributeDesc> InVertexLayout = std::vector<sVertexAttributeDesc>());

	virtual ~sMaterial();
	sMaterialInstance::SharedPtr CreateInstance(std::string InName);

	FORCEINLINE std::string GetName() const { return Name; };
	FORCEINLINE std::string GetPath() const { return Path; };
	FORCEINLINE void SetPath(std::string InPath) { Path = InPath; };
	FORCEINLINE IPipeline::SharedPtr GetPipeline() const { return Pipeline; };

	ERenderPass GetRenderPass() const;

	bool AddSampler(const ISamplerState::SharedPtr& Sampler);
	bool AddSampler(const sSamplerAttributeDesc& Desc);

	FORCEINLINE bool IsCompiled() const { return Pipeline->IsCompiled(); }
	bool Compile(IFrameBuffer* FrameBuffer = nullptr);
	bool Compile(IRenderTarget* RT, IDepthTarget* Depth = nullptr);
	bool Compile(std::vector<IRenderTarget*> RTs, IDepthTarget* Depth = nullptr);
	bool Recompile();

	bool IsIndirectCommandAvailable() const;

	FORCEINLINE std::vector<sShaderBinding> GetDescriptorSetLayoutBindings() const { return Pipeline->GetPipelineDesc().Bindings; };
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

	FORCEINLINE sMaterialInstance::SharedPtr GetInstanceById(std::uint32_t Index) const
	{
		for (auto& Instance : Instances)
		{
			if (Instance->GetId() == Index)
			{
				return Instance;
			}
		}
		return nullptr;
	};

	FORCEINLINE void DestroyInstance(std::string InstanceName)
	{
		sMaterialInstance::SharedPtr MatInstance = nullptr;
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

	void ApplyMaterial(IGraphicsCommandContext* InCMDBuffer = nullptr) const;

	EMaterialUsage MaterialUsage;
	EMaterialBlendMode BlendMode;

	void Serialize();

private:
	std::string Name;
	std::string Path;
	IPipeline::SharedPtr Pipeline;

	std::vector<sMaterialInstance::SharedPtr> Instances;
	std::map<std::size_t, ISamplerState::SharedPtr> Samplers;
};
