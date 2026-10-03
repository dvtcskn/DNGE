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
#include "AbstractGI/Mesh.h"
#include "Core/MeshPrimitives.h"

void MeshBindlessGeometryHandle::Release()
{
	if (!Owner.expired())
	{
		if (std::shared_ptr<sMeshGeometryContainer> PTR = Owner.lock())
			PTR->Free(Descriptor);
	}

	Owner.reset();

	StructureIndex = std::uint32_t(-1);
	Descriptor = GeometryDescriptor();
}

void MeshBindlessGeometryHandle::UpdateGeometry(void* Geometry)
{
	if (!Owner.expired())
	{
		if (std::shared_ptr<sMeshGeometryContainer> PTR = Owner.lock())
			PTR->UpdateGeometry(Descriptor, Geometry);
	}
}

void MeshIndexBufferHandle::Release()
{
	if (!Owner.expired())
	{
		if (std::shared_ptr<sMeshIndexContainer> PTR = Owner.lock())
			PTR->DeallocateHandle(this);
	}

	Owner.reset();

	Descriptor = std::uint32_t(-1);
}

void MeshIndexBufferHandle::UpdateIndexBuffer(void* IndexData)
{
	if (!Owner.expired())
	{
		if (std::shared_ptr<sMeshIndexContainer> PTR = Owner.lock())
		{
			BufferSubresource Subresource;
			Subresource.Location = GetStartIndexLocation() * sizeof(std::uint32_t);
			Subresource.pSysMem = IndexData;
			Subresource.Size = GetSize() * sizeof(std::uint32_t);
			PTR->UpdateIndexBuffer(&Subresource);
		}
	}
}

void sMeshIndexContainer::SetIndexBuffer(IGraphicsCommandContext* CommandContext)
{
	CommandContext->SetIndexBuffer(IndexBuffer.get());
}

sMesh::sMesh(const std::string InName, EBasicMeshType MeshType, std::optional<FBoxDimension> Dimension)
	: Name(InName)
	, Path("Generated")
	, VertexBuffer(nullptr)
	, IndexBuffer(nullptr)
	, Data(sMeshData())
	, MaterialInstance(nullptr)
	, InstanceBuffer(nullptr)
	, RenderPriority(EMeshRenderPriority::eDefault)
{
	if (MeshType == EBasicMeshType::ePlane)
	{
		const auto Plane = MeshPrimitives::Create2DPlaneVerticesFromDimension(Dimension.has_value() ? FDimension2D(Dimension->Width, Dimension->Height) : FDimension2D(1.0f, 1.0f));
		const auto PlaneTC = MeshPrimitives::GeneratePlaneTextureCoordinate(0.0f);
		for (std::size_t i = 0; i < Plane.size(); i++)
		{
			auto& Verts = Plane.at(i);
			auto& TC = PlaneTC.at(i);
			sVertexLayout VBE;
			VBE.position = FVector(Verts.X, Verts.Y, 0.0f);
			VBE.texCoord = TC;
			VBE.Color = FColor::White();
			Data.Vertices.push_back(VBE);
		}
		Data.Indices = MeshPrimitives::GeneratePlaneIndices(1);
		Data.DrawParameters.IndexCountPerInstance = (uint32_t)Data.Indices.size();
	}
	else if (MeshType == EBasicMeshType::eBox)
	{
		Data = MeshPrimitives::CreateBox(/*Dimension*/);
	}

	GeometryHandle = MeshBindlessGeometryHandle((std::uint32_t)Data.Vertices.size(), sizeof(sVertexLayout));
	sMeshGeometryContainerManager::Get().AllocateHandle(&GeometryHandle);
	GeometryHandle.UpdateGeometry(Data.Vertices.data());

	IndexBufferHandle = MeshIndexBufferHandle((std::uint32_t)Data.Indices.size(), sizeof(std::uint32_t));
	sMeshGeometryContainerManager::Get().AllocateHandle(&IndexBufferHandle);
	IndexBufferHandle.UpdateIndexBuffer(Data.Indices.data());

	Data.DrawParameters.StartIndexLocation = IndexBufferHandle.GetStartIndexLocation();

	{
		auto Vertices = Data.Vertices;
		BufferSubresource Subresource = BufferSubresource(Vertices.data(), Vertices.size() * sizeof(sVertexLayout));
		VertexBuffer = (IVertexBuffer::Create(Name + "_VertexBuffer", BufferLayout(Vertices.size() * sizeof(sVertexLayout), sizeof(sVertexLayout)), &Subresource));
	}
	{
		auto Indices = Data.Indices;
		BufferSubresource Subresource = BufferSubresource(Indices.data(), Indices.size() * sizeof(std::uint32_t));
		IndexBuffer = (IIndexBuffer::Create(Name + "_IndexBuffer", BufferLayout(Indices.size() * sizeof(std::uint32_t), sizeof(std::uint32_t)), &Subresource));
	}
	{
		MeshConstantBuffer = IConstantBuffer::Create(Name + "_MeshConstantBuffer", BufferLayout(sizeof(sMeshConstantBufferAttributes), 0), 1);  // 1
		{
			sMeshConstantBufferAttributes ObjectConstants;
			ObjectConstants.modelMatrix = FMatrix::Identity();
			ObjectConstants.PrevModelMatrix = FMatrix::Identity();
			MeshConstantBuffer->Map(&ObjectConstants);
		}
	}
}

sMesh::sMesh(const std::string InName, const std::string InPath)
	: Name(InName)
	, Path(InPath)
	, VertexBuffer(nullptr)
	, IndexBuffer(nullptr)
	, Data(sMeshData())
	, MaterialInstance(nullptr)
	, InstanceBuffer(nullptr)
	, RenderPriority(EMeshRenderPriority::eDefault)
{
	{
		MeshConstantBuffer = IConstantBuffer::Create(Name + "_MeshConstantBuffer", BufferLayout(sizeof(sMeshConstantBufferAttributes), 0), 1); // 1
		{
			sMeshConstantBufferAttributes ObjectConstants;
			ObjectConstants.modelMatrix = FMatrix::Identity();
			ObjectConstants.PrevModelMatrix = FMatrix::Identity();
			MeshConstantBuffer->Map(&ObjectConstants);
		}
	}
}

sMesh::sMesh(const std::string InName, const std::string InPath, const sMeshData& pData)
	: Name(InName)
	, Path(InPath)
	, Data(pData)
	, MaterialInstance(nullptr)
	, InstanceBuffer(nullptr)
	, RenderPriority(EMeshRenderPriority::eDefault)
	, GeometryHandle(MeshBindlessGeometryHandle((std::uint32_t)pData.Vertices.size(), sizeof(sVertexLayout)))
	, GeometryInstanceHandle(MeshBindlessGeometryHandle((std::uint32_t)pData.InstanceData.size(), sizeof(sVertexLayout::sVertexInstanceLayout)))
	, IndexBufferHandle(MeshIndexBufferHandle((std::uint32_t)pData.Indices.size(), sizeof(std::uint32_t)))
{
	sMeshGeometryContainerManager::Get().AllocateHandle(&GeometryHandle);
	GeometryHandle.UpdateGeometry(Data.Vertices.data());
	if (pData.InstanceData.size() > 0)
	{
		sMeshGeometryContainerManager::Get().AllocateHandle(&GeometryInstanceHandle);
		GeometryInstanceHandle.UpdateGeometry(Data.InstanceData.data());
	}
	sMeshGeometryContainerManager::Get().AllocateHandle(&IndexBufferHandle);
	IndexBufferHandle.UpdateIndexBuffer(Data.Indices.data());

	Data.DrawParameters.StartIndexLocation = IndexBufferHandle.GetStartIndexLocation();

	{
		auto Vertices = Data.Vertices;
		BufferSubresource Subresource = BufferSubresource(Vertices.data(), Vertices.size() * sizeof(sVertexLayout));
		VertexBuffer = (IVertexBuffer::Create(Name + "_VertexBuffer", BufferLayout(Vertices.size() * sizeof(sVertexLayout), sizeof(sVertexLayout)), &Subresource));
	}
	if (Data.InstanceData.size() > 0)
	{
		auto Vertices = Data.InstanceData;
		BufferSubresource Subresource = BufferSubresource(Vertices.data(), Vertices.size() * sizeof(sVertexLayout::sVertexInstanceLayout));
		InstanceBuffer = (IVertexBuffer::Create(Name + "_InstanceBuffer", BufferLayout(Vertices.size() * sizeof(sVertexLayout::sVertexInstanceLayout), sizeof(sVertexLayout::sVertexInstanceLayout)), &Subresource));
	}
	{
		auto Indices = Data.Indices;
		BufferSubresource Subresource = BufferSubresource(Indices.data(), Indices.size() * sizeof(std::uint32_t));
		IndexBuffer = (IIndexBuffer::Create(Name + "_IndexBuffer", BufferLayout(Indices.size() * sizeof(std::uint32_t), sizeof(std::uint32_t)), &Subresource));
	}
	{
		MeshConstantBuffer = IConstantBuffer::Create(Name + "_MeshConstantBuffer", BufferLayout(sizeof(sMeshConstantBufferAttributes), 0), 1); // 1
		{
			sMeshConstantBufferAttributes ObjectConstants;
			ObjectConstants.modelMatrix = FMatrix::Identity();
			ObjectConstants.PrevModelMatrix = FMatrix::Identity();
			MeshConstantBuffer->Map(&ObjectConstants);
		}
	}
}

sMesh::~sMesh()
{
	VertexBuffer = nullptr;
	InstanceBuffer = nullptr;
	IndexBuffer = nullptr;
	MeshConstantBuffer = nullptr;

	MaterialInstance = nullptr;

	PendingVertexBufferSubresource = std::nullopt;
	PendingIndexBufferSubresource = std::nullopt;

	sMeshGeometryContainerManager::Get().DeallocateHandle(&GeometryHandle);
	sMeshGeometryContainerManager::Get().DeallocateHandle(&GeometryInstanceHandle);
	sMeshGeometryContainerManager::Get().DeallocateHandle(&IndexBufferHandle);
}

void sMesh::SetMeshData(const sMeshData& pData, const std::string InPath)
{
	Data = pData;
	Path = InPath;

	sMeshGeometryContainerManager::Get().DeallocateHandle(&GeometryHandle);
	sMeshGeometryContainerManager::Get().DeallocateHandle(&GeometryInstanceHandle);
	sMeshGeometryContainerManager::Get().DeallocateHandle(&IndexBufferHandle);

	GeometryHandle = MeshBindlessGeometryHandle((std::uint32_t)pData.Vertices.size(), sizeof(sVertexLayout));
	sMeshGeometryContainerManager::Get().AllocateHandle(&GeometryHandle);
	GeometryHandle.UpdateGeometry(Data.Vertices.data());

	if (Data.InstanceData.size() > 0)
	{
		GeometryInstanceHandle = MeshBindlessGeometryHandle((std::uint32_t)pData.InstanceData.size(), sizeof(sVertexLayout::sVertexInstanceLayout));
		sMeshGeometryContainerManager::Get().AllocateHandle(&GeometryInstanceHandle);
		GeometryInstanceHandle.UpdateGeometry(Data.InstanceData.data());
	}

	IndexBufferHandle = MeshIndexBufferHandle((std::uint32_t)pData.Indices.size(), sizeof(std::uint32_t));
	sMeshGeometryContainerManager::Get().AllocateHandle(&IndexBufferHandle);
	IndexBufferHandle.UpdateIndexBuffer(Data.Indices.data());

	{
		auto Vertices = Data.Vertices;
		BufferSubresource Subresource = BufferSubresource(Vertices.data(), Vertices.size() * sizeof(sVertexLayout));
		VertexBuffer = (IVertexBuffer::Create(Name + "_VertexBuffer", BufferLayout(Vertices.size() * sizeof(sVertexLayout), sizeof(sVertexLayout)), &Subresource));
	}
	if (Data.InstanceData.size() > 0)
	{
		auto Vertices = Data.InstanceData;
		BufferSubresource Subresource = BufferSubresource(Vertices.data(), Vertices.size() * sizeof(sVertexLayout::sVertexInstanceLayout));
		InstanceBuffer = (IVertexBuffer::Create(Name + "_InstanceBuffer", BufferLayout(Vertices.size() * sizeof(sVertexLayout::sVertexInstanceLayout), sizeof(sVertexLayout::sVertexInstanceLayout)), &Subresource));
	}
	{
		auto Indices = Data.Indices;
		BufferSubresource Subresource = BufferSubresource(Indices.data(), Indices.size() * sizeof(std::uint32_t));
		IndexBuffer = (IIndexBuffer::Create(Name + "_IndexBuffer", BufferLayout(Indices.size() * sizeof(std::uint32_t), sizeof(std::uint32_t)), &Subresource));
	}
}

std::vector<sMaterialInstance*> sMesh::GetMaterialInstances() const
{
	return std::vector<sMaterialInstance*>{ MaterialInstance };
}

std::int32_t sMesh::GetNumMaterials() const
{
	return 1;
}

sMaterialInstance* sMesh::GetMaterialInstance(/*std::int32_t Index*/) const
{
	return MaterialInstance;
}

void sMesh::SetMaterial(/*std::int32_t Index, */sMaterialInstance* Material)
{
	MaterialInstance = Material;
}

void sMesh::SetMeshTransform(const sMeshConstantBufferAttributes& ObjectConstants)
{
	MeshConstantBuffer->Map(&ObjectConstants);
}

void sMesh::SetMeshTransform(FVector Location, FVector Scale, FVector4 Rotation)
{
	sMeshConstantBufferAttributes ObjectConstants;
	ObjectConstants.PrevModelMatrix = ObjectConstants.modelMatrix;
	ObjectConstants.modelMatrix = ToMatrixWithScale(Location, Scale, Rotation);

	MeshConstantBuffer->Map(&ObjectConstants);
}

void sMesh::UpdateVertexSubresource(BufferSubresource* Subresource, IGraphicsCommandContext* Context)
{
	VertexBuffer->UpdateSubresource(Subresource, Context);
}

void sMesh::UpdateInstanceubresource(BufferSubresource* Subresource, IGraphicsCommandContext* Context)
{
	if (InstanceBuffer)
		InstanceBuffer->UpdateSubresource(Subresource, Context);
}

void sMesh::UpdateIndexSubresource(BufferSubresource* Subresource, IGraphicsCommandContext* Context)
{
	IndexBuffer->UpdateSubresource(Subresource, Context);
}

void sMesh::UpdateVertexBufferWithPendingSubresource(IGraphicsCommandContext* Context)
{
	if (PendingVertexBufferSubresource.has_value())
		Context->UpdateBufferSubresource(VertexBuffer.get(), &PendingVertexBufferSubresource.value());
	PendingVertexBufferSubresource = std::nullopt;
}

void sMesh::UpdateInstanceBufferWithPendingSubresource(IGraphicsCommandContext* Context)
{
	if (!InstanceBuffer)
		return;

	if (PendingInstanceBufferSubresource.has_value())
		Context->UpdateBufferSubresource(InstanceBuffer.get(), &PendingInstanceBufferSubresource.value());
	PendingInstanceBufferSubresource = std::nullopt;
}

void sMesh::UpdateIndexBufferWithPendingSubresource(IGraphicsCommandContext* Context)
{
	if (PendingVertexBufferSubresource.has_value())
		Context->UpdateBufferSubresource(IndexBuffer.get(), &PendingIndexBufferSubresource.value());
	PendingIndexBufferSubresource = std::nullopt;
}

void sMesh::Serialize(sArchive& archive)
{
}
