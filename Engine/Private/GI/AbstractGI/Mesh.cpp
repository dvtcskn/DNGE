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

sMesh::sMesh(const std::string InName, EBasicMeshType MeshType, std::optional<FBoxDimension> Dimension)
	: Name(InName)
	, Path("Generated")
	, VertexBuffer(nullptr)
	, IndexBuffer(nullptr)
	, Data(sMeshData())
	, MaterialInstance(nullptr)
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

	{
		auto Vertices = Data.Vertices;
		BufferSubresource Subresource = BufferSubresource(Vertices.data(), Vertices.size() * sizeof(sVertexLayout));
		VertexBuffer = (IVertexBuffer::Create(Name, BufferLayout(Vertices.size() * sizeof(sVertexLayout), sizeof(sVertexLayout)), &Subresource));
	}
	{
		auto Indices = Data.Indices;
		BufferSubresource Subresource = BufferSubresource(Indices.data(), Indices.size() * sizeof(std::uint32_t));
		IndexBuffer = (IIndexBuffer::Create(Name, BufferLayout(Indices.size() * sizeof(std::uint32_t), sizeof(std::uint32_t)), &Subresource));
	}
	{
		MeshConstantBuffer = IConstantBuffer::Create(Name, BufferLayout(sizeof(sMeshConstantBufferAttributes), 0), 1);
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
	, RenderPriority(EMeshRenderPriority::eDefault)
{
	{
		MeshConstantBuffer = IConstantBuffer::Create(Name, BufferLayout(sizeof(sMeshConstantBufferAttributes), 0), 1);
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
	, RenderPriority(EMeshRenderPriority::eDefault)
{
	{
		auto Vertices = Data.Vertices;
		BufferSubresource Subresource = BufferSubresource(Vertices.data(), Vertices.size() * sizeof(sVertexLayout));
		VertexBuffer = (IVertexBuffer::Create(Name, BufferLayout(Vertices.size() * sizeof(sVertexLayout), sizeof(sVertexLayout)), &Subresource));
	}
	{
		auto Indices = Data.Indices;
		BufferSubresource Subresource = BufferSubresource(Indices.data(), Indices.size() * sizeof(std::uint32_t));
		IndexBuffer = (IIndexBuffer::Create(Name, BufferLayout(Indices.size() * sizeof(std::uint32_t), sizeof(std::uint32_t)), &Subresource));
	}
	{
		MeshConstantBuffer = IConstantBuffer::Create(Name, BufferLayout(sizeof(sMeshConstantBufferAttributes), 0), 1);
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
	IndexBuffer = nullptr;
	MeshConstantBuffer = nullptr;

	MaterialInstance = nullptr;

	PendingVertexBufferSubresource = std::nullopt;
	PendingIndexBufferSubresource = std::nullopt;
}

void sMesh::SetMeshData(const sMeshData& pData, const std::string InPath)
{
	Data = pData;
	Path = InPath;
	{
		auto Vertices = Data.Vertices;
		BufferSubresource Subresource = BufferSubresource(Vertices.data(), Vertices.size() * sizeof(sVertexLayout));
		VertexBuffer = (IVertexBuffer::Create(Name, BufferLayout(Vertices.size() * sizeof(sVertexLayout), sizeof(sVertexLayout)), &Subresource));
	}
	{
		auto Indices = Data.Indices;
		BufferSubresource Subresource = BufferSubresource(Indices.data(), Indices.size() * sizeof(std::uint32_t));
		IndexBuffer = (IIndexBuffer::Create(Name, BufferLayout(Indices.size() * sizeof(std::uint32_t), sizeof(std::uint32_t)), &Subresource));
	}
}

std::vector<sMaterial::sMaterialInstance*> sMesh::GetMaterialInstances() const
{
	return std::vector<sMaterial::sMaterialInstance*>{ MaterialInstance };
}

std::int32_t sMesh::GetNumMaterials() const
{
	return 1;
}

sMaterial::sMaterialInstance* sMesh::GetMaterialInstance(/*std::int32_t Index*/) const
{
	return MaterialInstance;
}

void sMesh::SetMaterial(/*std::int32_t Index, */sMaterial::sMaterialInstance* Material)
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

void sMesh::UpdateIndexBufferWithPendingSubresource(IGraphicsCommandContext* Context)
{
	if (PendingVertexBufferSubresource.has_value())
		Context->UpdateBufferSubresource(IndexBuffer.get(), &PendingIndexBufferSubresource.value());
	PendingIndexBufferSubresource = std::nullopt;
}

void sMesh::Serialize(sArchive& archive)
{
}
