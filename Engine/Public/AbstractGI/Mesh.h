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

#include <vector>
#include "Core/Transform.h"
#include "Core/Archive.h"
#include "Material.h"
#include "Core/Math/CoreMath.h"

class sMeshGeometryContainerManager;
class sMeshGeometryContainer;

struct GeometryDescriptor
{
	std::uint64_t Location;
	std::uint32_t NumberOfElement;
	std::uint32_t ElementSize;

	GeometryDescriptor(std::uint32_t NewNumberOfElement = std::uint32_t(-1), std::uint32_t NewElementSize = std::uint32_t(-1))
		: Location(std::uint64_t(-1))
		, NumberOfElement(NewNumberOfElement)
		, ElementSize(NewElementSize)
	{}

	std::uint32_t GetSize() const { return NumberOfElement * ElementSize; }

	inline bool IsValid() const
	{
		return Location != std::uint64_t(-1) && NumberOfElement != std::uint32_t(-1) && ElementSize != std::uint32_t(-1);
	}

	inline bool constexpr operator ==(const GeometryDescriptor& Other)
	{
		return NumberOfElement == Other.NumberOfElement && ElementSize == Other.ElementSize; //Location == Other.Location;
	}
	inline bool constexpr operator <(const GeometryDescriptor& Other)
	{
		return Location < Other.Location;
	}
	inline bool constexpr operator >(const GeometryDescriptor& Other)
	{
		return Location > Other.Location;
	}
};

class MeshBindlessGeometryHandle
{
	sBaseClassBody(sClassConstructor, MeshBindlessGeometryHandle)
public:
	MeshBindlessGeometryHandle(std::uint32_t NewNumberOfElement = std::uint32_t(-1), std::uint32_t NewElementSize = std::uint32_t(-1))
		: Owner(std::weak_ptr<sMeshGeometryContainer>())
		, StructureIndex(std::uint32_t(-1))
		, Descriptor(GeometryDescriptor(NewNumberOfElement, NewElementSize))
	{}
	MeshBindlessGeometryHandle(std::weak_ptr<sMeshGeometryContainer> NewOwner, std::uint32_t NewStructureIndex, GeometryDescriptor NewGeometryDescriptor)
		: Owner(NewOwner)
		, StructureIndex(NewStructureIndex)
		, Descriptor(NewGeometryDescriptor)
	{}
	~MeshBindlessGeometryHandle()
	{
		Release();
	}

	void Release();
	void UpdateGeometry(void* Geometry);

	inline void Reset(std::weak_ptr<sMeshGeometryContainer> NewOwner, std::uint32_t NewStructureIndex, std::uint64_t NewLocation)
	{
		Owner = NewOwner;
		StructureIndex = NewStructureIndex;
		Descriptor.Location = NewLocation;
	}

	inline void Clear()
	{
		Owner.reset();
		StructureIndex = std::uint32_t(-1);
		Descriptor = GeometryDescriptor();
	}

	inline bool IsValid() const
	{
		return !Owner.expired() && (StructureIndex != std::uint32_t(-1)) && Descriptor.IsValid();
	}

	inline std::uint32_t GetBindlessStructureIndex() const
	{
		return StructureIndex;
	}
	inline std::uint64_t GetLocation() const
	{
		return Descriptor.Location;
	}
	inline std::uint32_t GetElementSize() const
	{
		return Descriptor.ElementSize;
	}
	inline std::uint32_t GetDescriptorSize() const
	{
		return Descriptor.GetSize();
	}

	std::weak_ptr<sMeshGeometryContainer> Owner;
	std::uint32_t StructureIndex;
	GeometryDescriptor Descriptor;
};

class sMeshGeometryContainer : public std::enable_shared_from_this<sMeshGeometryContainer>
{
	sBaseClassBody(sClassConstructor, sMeshGeometryContainer)
public:
	sMeshGeometryContainer(std::uint64_t NewSize)
		: Size(NewSize)
		, NextDescriptorLocation(0)
		, Container(nullptr)
	{
		std::string Name = "MeshGeometryContainer";
		Container = IByteAddressBuffer::Create(Name, Size, false);
	}

	~sMeshGeometryContainer()
	{
		Size = 0;
		NextDescriptorLocation = 0;

		FreedDescriptors.clear();

		Container = nullptr;
	}

	inline bool AllocateHandle(MeshBindlessGeometryHandle* Handle)
	{
		GeometryDescriptor Descriptor = TryToGetAvailableFreeSlot(Handle);
		if (Descriptor.IsValid())
		{
			Handle->Reset(weak_from_this(), Container->GetBindlessIndex(), Descriptor.Location);
		}
		else
		{
			if (NextDescriptorLocation >= Size)
				return false;

			Handle->Reset(weak_from_this(), Container->GetBindlessIndex(), NextDescriptorLocation);
			NextDescriptorLocation += Handle->GetDescriptorSize();
		}

		return true;
	}

	inline GeometryDescriptor TryToGetAvailableFreeSlot(MeshBindlessGeometryHandle* Handle)
	{
		auto Found = std::find(FreedDescriptors.begin(), FreedDescriptors.end(), Handle->Descriptor);
		if (Found == FreedDescriptors.end())
			return GeometryDescriptor();

		GeometryDescriptor Descriptor = *Found;
		FreedDescriptors.erase(Found);
		return Descriptor;
	}

	inline void UpdateGeometry(const GeometryDescriptor& Descriptor, void* Geometry)
	{
		Container->Map(Geometry, Descriptor.Location, Descriptor.GetSize());
	}

	inline void Free(const GeometryDescriptor& Descriptor)
	{
		if (std::find(FreedDescriptors.begin(), FreedDescriptors.end(), Descriptor) != FreedDescriptors.end())
			return;

		FreedDescriptors.push_back(Descriptor);
		std::sort(FreedDescriptors.begin(), FreedDescriptors.end());

		/*
		* clear container slot
		*/
		/*std::vector<std::uint8_t> ZeroData;
		ZeroData.resize(Descriptor.GetSize());
		Container->Map(ZeroData.data(), Descriptor.Location, Descriptor.GetSize());*/
	}

	inline IByteAddressBuffer* GetContainer() const { return Container.get(); }

private:
	std::uint64_t Size;
	std::uint64_t NextDescriptorLocation;
	std::vector<GeometryDescriptor> FreedDescriptors;

	IByteAddressBuffer::SharedPtr Container;
};

class sMeshIndexContainer;
struct MeshIndexBufferDescriptor
{
	MeshIndexBufferDescriptor(std::uint32_t NewElementSize = std::uint32_t(-1), std::uint32_t NewStride = std::uint32_t(-1), std::uint32_t NewLocation = std::uint32_t(-1))
		: ElementSize(NewElementSize)
		, Stride(NewStride)
		, StartIndexLocation(NewLocation)
	{}

	std::uint32_t GetSize() const
	{
		return ElementSize/* * Stride*/;
	};
	std::uint32_t GetStartIndexLocation() const
	{
		return StartIndexLocation;
	};
	std::uint32_t GetElementSize() const
	{
		return ElementSize;
	};
	std::uint32_t GetStride() const
	{
		return Stride;
	};

	inline bool IsValid() const
	{
		return StartIndexLocation != std::uint32_t(-1) && ElementSize != std::uint32_t(-1)/* && Stride != std::uint32_t(-1)*/;
	}

	inline void Clear()
	{
		StartIndexLocation = std::uint32_t(-1);
		ElementSize = std::uint32_t(-1);
		Stride = std::uint32_t(-1);
	}

	inline bool constexpr operator ==(const MeshIndexBufferDescriptor& Other)
	{
		return ElementSize == Other.ElementSize/* && Stride == Other.Stride*/;
	}
	inline bool constexpr operator <(const MeshIndexBufferDescriptor& Other)
	{
		return StartIndexLocation < Other.StartIndexLocation;
	}
	inline bool constexpr operator >(const MeshIndexBufferDescriptor& Other)
	{
		return StartIndexLocation > Other.StartIndexLocation;
	}

	std::uint32_t StartIndexLocation;

private:
	std::uint32_t ElementSize;
	std::uint32_t Stride;
};

class MeshIndexBufferHandle
{
	sBaseClassBody(sClassConstructor, MeshIndexBufferHandle)
public:
	MeshIndexBufferHandle(std::uint32_t NewElementSize = std::uint32_t(-1), std::uint32_t NewStride = std::uint32_t(-1))
		: Owner(std::weak_ptr<sMeshIndexContainer>())
		, Descriptor(MeshIndexBufferDescriptor(NewElementSize, NewStride))
	{}

	~MeshIndexBufferHandle()
	{
		Release();
	}

	void Release();
	void UpdateIndexBuffer(void* IndexData);

	inline void Reset(std::weak_ptr<sMeshIndexContainer> NewOwner, std::uint32_t NewLocation)
	{
		Owner = NewOwner;
		Descriptor.StartIndexLocation = NewLocation;
	}

	inline void Clear()
	{
		Owner.reset();
		Descriptor.Clear();
	}

	inline bool IsValid() const
	{
		return !Owner.expired() && Descriptor.IsValid();
	}

	inline std::uint32_t GetStartIndexLocation() const
	{
		return Descriptor.GetStartIndexLocation();
	}

	inline std::uint32_t GetSize() const
	{
		return Descriptor.GetSize();
	}

	inline MeshIndexBufferDescriptor GetDescriptor() const
	{
		return Descriptor;
	}

	std::weak_ptr<sMeshIndexContainer> Owner;
	MeshIndexBufferDescriptor Descriptor;
};

class sMeshIndexContainer : public std::enable_shared_from_this<sMeshIndexContainer>
{
	sBaseClassBody(sClassConstructor, sMeshIndexContainer);
public:
	sMeshIndexContainer(std::uint64_t NewTotalSize = 1024 * 1024 * 4)
		: TotalSize(NewTotalSize)
		, NextDescriptorLocation(0)
	{
		IndexBuffer = IIndexBuffer::Create("sClassConstructor_IndexBuffer", BufferLayout(TotalSize, sizeof(std::uint32_t)));
	}

	~sMeshIndexContainer()
	{
		Release();
	}

	inline bool AllocateHandle(MeshIndexBufferHandle* Handle)
	{
		MeshIndexBufferDescriptor Descriptor = TryToGetAvailableFreeSlot(Handle);
		if (Descriptor.IsValid())
		{
			Handle->Reset(weak_from_this(), Descriptor.GetStartIndexLocation());
		}
		else
		{
			if (NextDescriptorLocation >= TotalSize)
				return false;

			Handle->Reset(weak_from_this(), NextDescriptorLocation);
			NextDescriptorLocation += Handle->GetSize();
		}

		return true;
	}

	inline MeshIndexBufferDescriptor TryToGetAvailableFreeSlot(MeshIndexBufferHandle* Handle)
	{
		auto Found = std::find(FreedDescriptorIndices.begin(), FreedDescriptorIndices.end(), Handle->Descriptor);
		if (Found == FreedDescriptorIndices.end())
			return MeshIndexBufferDescriptor(std::uint32_t(-1));

		MeshIndexBufferDescriptor Descriptor = *Found;
		FreedDescriptorIndices.erase(Found);
		return Descriptor;
	}

	inline void UpdateIndexBuffer(BufferSubresource* Subresource)
	{
		if (IndexBuffer)
			IndexBuffer->UpdateSubresource(Subresource);
	}

	inline bool DeallocateHandle(MeshIndexBufferHandle* Handle)
	{
		if (std::find(FreedDescriptorIndices.begin(), FreedDescriptorIndices.end(), Handle->Descriptor) != FreedDescriptorIndices.end())
			return false;

		FreedDescriptorIndices.push_back(Handle->Descriptor);
		std::sort(FreedDescriptorIndices.begin(), FreedDescriptorIndices.end());

		Handle->Clear();

		return true;
	}

	inline void Release()
	{
		IndexBuffer = nullptr;
		FreedDescriptorIndices.clear();
		TotalSize = 0;
		NextDescriptorLocation = std::uint32_t(-1);
	}

	IIndexBuffer* GetIndexBuffer() const 
	{
		return IndexBuffer.get();
	}
	void SetIndexBuffer(IGraphicsCommandContext* CommandContext);

private:
	IIndexBuffer::SharedPtr IndexBuffer;
	std::uint64_t TotalSize;
	uint32_t NextDescriptorLocation;
	std::vector<MeshIndexBufferDescriptor> FreedDescriptorIndices;
};

class sMeshGeometryContainerManager
{
	sBaseClassBody(sClassNoDefaults, sMeshGeometryContainerManager);
private:
	sMeshGeometryContainerManager() = default;
	sMeshGeometryContainerManager(const sMeshGeometryContainerManager& Other) = delete;
	sMeshGeometryContainerManager& operator=(const sMeshGeometryContainerManager&) = delete;

public:
	~sMeshGeometryContainerManager()
	{
		Release();
	}

	static sMeshGeometryContainerManager& Get()
	{
		static sMeshGeometryContainerManager instance;
		return instance;
	}

	inline void AllocateHandle(MeshBindlessGeometryHandle* Handle)
	{
		if (!MegaVertexContainer)
		{
			MegaVertexContainer = sMeshGeometryContainer::Create(1024 * 1024 * 128);
		}
		MegaVertexContainer->AllocateHandle(Handle);
	}

	inline void DeallocateHandle(MeshBindlessGeometryHandle* Handle)
	{
		if (MegaVertexContainer)
		{
			MegaVertexContainer->Free(Handle->Descriptor);
			Handle->Clear();
		}
	}

	inline void AllocateHandle(MeshIndexBufferHandle* Handle)
	{
		if (!MegaIndexContainer)
		{
			MegaIndexContainer = sMeshIndexContainer::Create(1024 * 1024 * 128);
		}
		MegaIndexContainer->AllocateHandle(Handle);
	}

	inline void DeallocateHandle(MeshIndexBufferHandle* Handle)
	{
		if (MegaIndexContainer)
		{
			MegaIndexContainer->DeallocateHandle(Handle);
			Handle->Clear();
		}
	}

	IIndexBuffer* GetIndexBuffer() const 
	{
		return MegaIndexContainer->GetIndexBuffer();
	}
	void SetIndexBuffer(IGraphicsCommandContext* CommandContext)
	{
		if (MegaIndexContainer)
			MegaIndexContainer->SetIndexBuffer(CommandContext);
	}

	inline void Release()
	{
		MegaVertexContainer = nullptr;
		MegaIndexContainer = nullptr;
	}

private:
	sMeshGeometryContainer::SharedPtr MegaVertexContainer;
	sMeshIndexContainer::SharedPtr MegaIndexContainer;
};

enum class EBasicMeshType
{
	ePlane,
	eBox
};

enum class EMeshRenderPriority
{
	eDefault,
	InOrder = eDefault,
	LastInTheHierarchy,
	Latest,
};

class IMesh
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IMesh)
public:
	virtual std::string GetName() const = 0;

	virtual IConstantBuffer* GetMeshConstantBuffer() const = 0;
	virtual IVertexBuffer* GetVertexBuffer() const = 0;
	virtual bool HasInstanceBuffer() const = 0;
	virtual IVertexBuffer* GetInstanceBuffer() const = 0;
	virtual IIndexBuffer* GetIndexBuffer() const = 0;

	virtual sObjectDrawParameters GetDrawParameters() const = 0;

	virtual EMeshRenderPriority GeMeshRenderPriority() const { return EMeshRenderPriority::eDefault; }

	virtual bool IsUpdateRequired() const { return false; }
	virtual void UpdateMesh(IGraphicsCommandContext* Context) { }

	virtual std::string GetMaterialName(/*std::int32_t Index = 0*/) const = 0;
	virtual std::vector<sMaterialInstance*> GetMaterialInstances() const = 0;
	virtual std::int32_t GetNumMaterials() const = 0;
	virtual sMaterialInstance* GetMaterialInstance(/*std::int32_t Index = 0*/) const = 0;

	virtual void Serialize(sArchive& archive) = 0;

	virtual const MeshBindlessGeometryHandle* GetGeometryHandle() const { return nullptr; }
	virtual const MeshBindlessGeometryHandle* GetGeometryInstanceHandle() const { return nullptr; }
	virtual const MeshIndexBufferHandle* GetIndexHandle() const { return nullptr; }
};

class sMesh : public IMesh
{
	sClassBody(sClassConstructor, sMesh, IMesh)
public:
	sMesh(const std::string Name, EBasicMeshType MeshType, std::optional<FBoxDimension> Dimension = std::nullopt);
	sMesh(const std::string Name, const std::string Path = "Memory");
	sMesh(const std::string Name, const std::string Path, const sMeshData& Data);

public:
	virtual ~sMesh();

	FORCEINLINE void SetName(std::string InName)  { Name = InName; };
	FORCEINLINE virtual std::string GetName() const override final { return Name; };
	FORCEINLINE std::string GetPath() const { return Path; };

	void SetMeshData(const sMeshData& Data, const std::string Path = "Memory");
	virtual void OnSetMeshData() {};

	virtual IConstantBuffer* GetMeshConstantBuffer() const override final { return MeshConstantBuffer.get(); };
	virtual IVertexBuffer* GetVertexBuffer() const override final { return VertexBuffer.get(); };
	virtual bool HasInstanceBuffer() const override final { return InstanceBuffer != nullptr; };
	virtual IVertexBuffer* GetInstanceBuffer() const override final { return InstanceBuffer.get(); };
	virtual IIndexBuffer* GetIndexBuffer() const override final { return IndexBuffer.get(); };

	std::vector<std::uint32_t> GetIndices() const { return Data.Indices; };
	std::size_t GetIndicesSize() const { return Data.Indices.size(); };
	std::vector<sVertexLayout> GetVertices() const { return Data.Vertices; };
	std::size_t GetVerticesSize() const { return Data.Vertices.size(); };
	std::vector<sVertexLayout::sVertexInstanceLayout> GetInstances() const { return Data.InstanceData; };
	std::size_t GetInstanceSize() const { return Data.InstanceData.size(); };
	virtual sObjectDrawParameters GetDrawParameters() const override final { return Data.DrawParameters; };

	virtual EMeshRenderPriority GeMeshRenderPriority() const override final { return RenderPriority; }
	void SeMeshRenderPriority(const EMeshRenderPriority Priority) { RenderPriority = Priority; }

	void UpdateVertexSubresource(BufferSubresource* Subresource, IGraphicsCommandContext* Context = nullptr);
	void UpdateInstanceubresource(BufferSubresource* Subresource, IGraphicsCommandContext* Context = nullptr);
	void UpdateIndexSubresource(BufferSubresource* Subresource, IGraphicsCommandContext* Context = nullptr);

	virtual bool IsUpdateRequired() const { return PendingVertexBufferSubresource.has_value() || PendingIndexBufferSubresource.has_value(); }
	virtual void UpdateMesh(IGraphicsCommandContext* Context) 
	{
		UpdateVertexBufferWithPendingSubresource(Context);
		UpdateInstanceBufferWithPendingSubresource(Context);
		UpdateIndexBufferWithPendingSubresource(Context);
	}

	void UpdateVertexBufferWithPendingSubresource(IGraphicsCommandContext* Context);
	void UpdateInstanceBufferWithPendingSubresource(IGraphicsCommandContext* Context);
	void UpdateIndexBufferWithPendingSubresource(IGraphicsCommandContext* Context);

	std::optional<BufferSubresource> PendingVertexBufferSubresource;
	std::optional<BufferSubresource> PendingInstanceBufferSubresource;
	std::optional<BufferSubresource> PendingIndexBufferSubresource;

	void SetIndexOffset(std::size_t Offset) { Data.DrawParameters.StartIndexLocation = (std::uint32_t)Offset; };
	void SetVertexOffset(std::size_t Offset) { Data.DrawParameters.BaseVertexLocation = (std::int32_t)Offset; };
	std::size_t GetIndexOffset() { return Data.DrawParameters.StartIndexLocation; };
	std::size_t GetVertexOffset() { return Data.DrawParameters.BaseVertexLocation; };

	virtual std::vector<sMaterialInstance*> GetMaterialInstances() const override final;
	virtual std::int32_t GetNumMaterials() const override final;
	virtual sMaterialInstance* GetMaterialInstance(/*std::int32_t Index = 0*/) const override final;
	void SetMaterial(/*std::int32_t Index,*/ sMaterialInstance* Material);

	FORCEINLINE virtual std::string GetMaterialName(/*std::int32_t Index = 0*/) const override final { return MaterialInstance->GetName(); /*return MaterialInstances.at(Index)->GetName();*/ }

	void SetMeshTransform(const sMeshConstantBufferAttributes& ObjectConstants);
	void SetMeshTransform(FVector Location, FVector Scale, FVector4 Rotation);
	virtual void Serialize(sArchive& archive) override;

	virtual const MeshBindlessGeometryHandle* GetGeometryHandle() const override { return &GeometryHandle; }
	virtual const MeshBindlessGeometryHandle* GetGeometryInstanceHandle() const override { return &GeometryInstanceHandle; }
	virtual const MeshIndexBufferHandle* GetIndexHandle() const override { return &IndexBufferHandle; }

private:
	std::string Name;
	std::string Path;
	EMeshRenderPriority RenderPriority;

	MeshBindlessGeometryHandle GeometryHandle;
	MeshBindlessGeometryHandle GeometryInstanceHandle;
	MeshIndexBufferHandle IndexBufferHandle;
	sMeshData Data;
	IVertexBuffer::SharedPtr VertexBuffer;
	IVertexBuffer::SharedPtr InstanceBuffer;
	IIndexBuffer::SharedPtr IndexBuffer;

	IConstantBuffer::SharedPtr MeshConstantBuffer;

	// Birden fazla Material desteklemek için Draw Callarý Materiale göre böl
	//std::vector<sMaterialInstance*> MaterialInstances;
	sMaterialInstance* MaterialInstance;
};
