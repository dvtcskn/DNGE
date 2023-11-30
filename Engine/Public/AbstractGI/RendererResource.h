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

class sRendererResource
{
	sBaseClassBody(sClassDefaultProtectedConstructor, sRendererResource)
public:
	virtual std::string GetName() const = 0;

	virtual void Serialize(sArchive& archive) = 0;
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

class IMesh : public sRendererResource
{
	sClassBody(sClassDefaultProtectedConstructor, IMesh, sRendererResource)
public:
	virtual IConstantBuffer* GetMeshConstantBuffer() const = 0;
	virtual IVertexBuffer* GetVertexBuffer() const = 0;
	virtual IIndexBuffer* GetIndexBuffer() const = 0;

	virtual std::size_t GetSecondaryConstantBufferCount() const { return 0; }
	virtual IConstantBuffer* GetSecondaryConstantBuffer(std::size_t Index) const { return nullptr; }

	virtual sObjectDrawParameters GetDrawParameters() const = 0;

	virtual EMeshRenderPriority GeMeshRenderPriority() const { return EMeshRenderPriority::eDefault; }

	virtual bool IsUpdateRequired() const { return false; }
	virtual void UpdateMesh(IGraphicsCommandContext* Context) { }

	virtual std::string GetMaterialName(/*std::int32_t Index = 0*/) const = 0;
	virtual std::vector<sMaterial::sMaterialInstance*> GetMaterialInstances() const = 0;
	virtual std::int32_t GetNumMaterials() const = 0;
	virtual sMaterial::sMaterialInstance* GetMaterialInstance(/*std::int32_t Index = 0*/) const = 0;
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
	virtual IIndexBuffer* GetIndexBuffer() const override final { return IndexBuffer.get(); };

	std::vector<std::uint32_t> GetIndices() const { return Data.Indices; };
	std::size_t GetIndicesSize() const { return Data.Indices.size(); };
	std::vector<sVertexBufferEntry> GetVertices() const { return Data.Vertices; };
	std::size_t GetVerticesSize() const { return Data.Vertices.size(); };
	virtual sObjectDrawParameters GetDrawParameters() const override final { return Data.DrawParameters; };

	virtual EMeshRenderPriority GeMeshRenderPriority() const override final { return RenderPriority; }
	void SeMeshRenderPriority(const EMeshRenderPriority Priority) { RenderPriority = Priority; }

	void UpdateVertexSubresource(sBufferSubresource* Subresource, IGraphicsCommandContext* Context = nullptr);
	void UpdateIndexSubresource(sBufferSubresource* Subresource, IGraphicsCommandContext* Context = nullptr);

	virtual bool IsUpdateRequired() const { return PendingVertexBufferSubresource.has_value() || PendingIndexBufferSubresource.has_value(); }
	virtual void UpdateMesh(IGraphicsCommandContext* Context) 
	{
		UpdateVertexBufferWithPendingSubresource(Context);
		UpdateIndexBufferWithPendingSubresource(Context);
	}

	void UpdateVertexBufferWithPendingSubresource(IGraphicsCommandContext* Context);
	void UpdateIndexBufferWithPendingSubresource(IGraphicsCommandContext* Context);

	std::optional<sBufferSubresource> PendingVertexBufferSubresource;
	std::optional<sBufferSubresource> PendingIndexBufferSubresource;

	void SetIndexOffset(std::size_t Offset) { Data.DrawParameters.StartIndexLocation = (std::uint32_t)Offset; };
	void SetVertexOffset(std::size_t Offset) { Data.DrawParameters.BaseVertexLocation = (std::int32_t)Offset; };
	std::size_t GetIndexOffset() { return Data.DrawParameters.StartIndexLocation; };
	std::size_t GetVertexOffset() { return Data.DrawParameters.BaseVertexLocation; };

	virtual std::vector<sMaterial::sMaterialInstance*> GetMaterialInstances() const override final;
	virtual std::int32_t GetNumMaterials() const override final;
	virtual sMaterial::sMaterialInstance* GetMaterialInstance(/*std::int32_t Index = 0*/) const override final;
	void SetMaterial(/*std::int32_t Index,*/ sMaterial::sMaterialInstance* Material);

	FORCEINLINE virtual std::string GetMaterialName(/*std::int32_t Index = 0*/) const override final { return MaterialInstance->GetName(); /*return MaterialInstances.at(Index)->GetName();*/ }

	void SetMeshTransform(const sMeshConstantBufferAttributes& ObjectConstants);
	void SetMeshTransform(FVector Location, FVector Scale, FVector4 Rotation);
	virtual void Serialize(sArchive& archive) override;

private:
	std::string Name;
	std::string Path;
	EMeshRenderPriority RenderPriority;

	sMeshData Data;
	IVertexBuffer::SharedPtr VertexBuffer;
	IIndexBuffer::SharedPtr IndexBuffer;

	IConstantBuffer::SharedPtr MeshConstantBuffer;

	// Birden fazla Material desteklemek için Draw Callarý Materiale göre böl
	//std::vector<sMaterial::sMaterialInstance*> MaterialInstances;
	sMaterial::sMaterialInstance* MaterialInstance;
};
