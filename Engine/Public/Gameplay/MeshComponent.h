#pragma once

#include "PhysicalComponent.h"
#include <vector>
#include <memory>
#include "Core/Math/CoreMath.h"
#include "AbstractGI/RendererResource.h"

class IMeshComponent : public sPrimitiveComponent
{
	sClassBody(sClassNoDefaults, IMeshComponent, sPrimitiveComponent)
protected:
	IMeshComponent(std::string InName, sActor* pActor = nullptr)
		: Super(InName, pActor)
	{}
public:
	virtual ~IMeshComponent() = default;

	virtual IMesh* GetMesh() const = 0;
	virtual std::string GetMeshName() const = 0;
};

class sMeshComponent : public IMeshComponent
{
	sClassBody(sClassConstructor, sMeshComponent, IMeshComponent)
public:
	sMeshComponent(const std::string Name, const std::string Path, const sMeshData& InData);
	sMeshComponent(const std::string Name, const std::string Path, sActor* Owner, const sMeshData& InData);
	sMeshComponent(const std::string Name, EBasicMeshType MeshType, std::optional<FBoxDimension> Dimension = std::nullopt);
	sMeshComponent(const std::string Name, sActor* Owner, EBasicMeshType MeshType, std::optional<FBoxDimension> Dimension = std::nullopt);

	virtual ~sMeshComponent();

	virtual IMesh* GetMesh() const override final { return Mesh.get(); }
	virtual std::string GetMeshName() const override final { return Mesh->GetName(); }
	std::string GetPath() const { return Mesh->GetPath(); }

	void SetMaterial(/*std::int32_t Index,*/ sMaterial::sMaterialInstance* Material);

	virtual void Serialize(Archive& archive) override;

private:
	virtual void OnUpdateTransform() override final;

private:
	sMesh::UniquePtr Mesh;
	sMeshConstantBufferAttributes ObjectConstants;
};
