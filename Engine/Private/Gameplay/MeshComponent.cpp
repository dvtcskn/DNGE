
#include "pch.h"
#include "Gameplay/MeshComponent.h"
#include "Gameplay/Actor.h"

sMeshComponent::sMeshComponent(const std::string InName, const std::string InPath, const sMeshData& InData)
	: Super(InName)
{
	Mesh = sMesh::CreateUnique(InName, InPath, InData);
	ObjectConstants.PrevModelMatrix = FMatrix::Identity();
	ObjectConstants.modelMatrix = FMatrix::Identity();
	UpdateTransform();
}

sMeshComponent::sMeshComponent(const std::string InName, const std::string InPath, sActor* Owner, const sMeshData& InData)
	: Super(InName, Owner)
{
	Mesh = sMesh::CreateUnique(InName, InPath, InData);
	ObjectConstants.PrevModelMatrix = FMatrix::Identity();
	ObjectConstants.modelMatrix = FMatrix::Identity();
	UpdateTransform();
}

sMeshComponent::sMeshComponent(const std::string InName, EBasicMeshType MeshType, std::optional<FBoxDimension> Dimension)
	: Super(InName)
{
	Mesh = sMesh::CreateUnique(InName, MeshType, Dimension);
	ObjectConstants.PrevModelMatrix = FMatrix::Identity();
	ObjectConstants.modelMatrix = FMatrix::Identity();
	UpdateTransform();
}

sMeshComponent::sMeshComponent(const std::string InName, sActor* Owner, EBasicMeshType MeshType, std::optional<FBoxDimension> Dimension)
	: Super(InName, Owner)
{
	Mesh = sMesh::CreateUnique(InName, MeshType, Dimension);
	ObjectConstants.PrevModelMatrix = FMatrix::Identity();
	ObjectConstants.modelMatrix = FMatrix::Identity();
	UpdateTransform();
}

sMeshComponent::~sMeshComponent()
{
	Mesh = nullptr;
}

void sMeshComponent::SetMaterial(sMaterial::sMaterialInstance* Material)
{
	Mesh->SetMaterial(Material);
}

void sMeshComponent::OnUpdateTransform()
{
	ObjectConstants.PrevModelMatrix = ObjectConstants.modelMatrix;

	//ObjectConstants.modelMatrix = DirectX::XMMatrixTransformation2D(FVector4::Zero(), 0.0f, GetRelativeScale(), GetRelativeLocation(), atan2f(RelRot.Z, RelRot.W) * 2.0f, GetRelativeLocation());
	//ObjectConstants.modelMatrix = DirectX::XMMatrixTransformation2D(FVector4::Zero(), 0.0f, GetRelativeScale(), FVector4::Zero(), RelRot.GetAngles().Roll, GetRelativeLocation());
	//ObjectConstants.modelMatrix = DirectX::XMMatrixAffineTransformation(GetRelativeScale(), FVector4::Zero(), GetRelativeRotation(), GetRelativeLocation());
	//ObjectConstants.modelMatrix = DirectX::XMMatrixTransformation(FVector4::Zero(), FVector4::Zero(), GetRelativeScale(), FVector4::Zero(), GetRelativeRotation(), GetRelativeLocation());
	ObjectConstants.modelMatrix = ToMatrixWithScale(GetRelativeLocation(), GetRelativeScale(), GetRelativeRotation());

	Mesh->SetMeshTransform(ObjectConstants);
}

void sMeshComponent::Serialize(Archive& archive)
{
	Mesh->Serialize(archive);
	//if (archive.IsReadMode())
	//{
	//	archive >> DefaultMaterialName;
	//	archive >> bIsDirty;
	//	archive >> Data;
	//	archive >> Data.Indices;
	//	archive >> Data.Vertices;

	//	DirectX::FVector3 Loc;
	//	DirectX::FVector3 Scale;
	//	DirectX::FQuaternion Rot;
	//	ObjectConstants.modelMatrix.Decompose(Scale, Rot, Loc);

	//	mTransform(Rot, Loc, Scale);

	//	AABB.MoveTo(mTransform.GetLocation());
	//	RigidBody->Teleport(Loc, (DirectX::FVector4&)mTransform.GetRotation());

	//	UpdateModelCB();
	//}
	//else
	//{
	//	archive << DefaultMaterialName;
	//	archive << bIsDirty;
	//	//archive << mTransform.GetLocation();
	//	//archive << mTransform.GetScale3D();
	//	//archive << DirectX::FVector4(mTransform.GetRotation());
	//	archive << ObjectConstants.modelMatrix;
	//	archive << ObjectConstants.mInverseTransposemodelMatrix;
	//}
}
