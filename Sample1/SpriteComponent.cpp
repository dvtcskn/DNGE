
#include "pch.h"
#include "SpriteComponent.h"
#include "Sprite.h"
#include <Gameplay/Actor.h>
#include <Core/MeshPrimitives.h>
#include <AbstractGI/MaterialManager.h>
#include <Gameplay/BoxCollision2DComponent.h>
#include <Utilities/Utilities.h>

sSpriteComponent::sSpriteComponent(const std::string Name)
	: Super(Name)
	, MeshContainer(sSpriteComponentMeshContainer::CreateUnique(this))
	, bFlip(false)
	, Sprite(nullptr)
	, Offset(FVector::Zero())
{
	MeshConstantBuffer = IConstantBuffer::Create(Name, sBufferDesc(sizeof(sMeshConstantBufferAttributes), 0), 1);
	{
		ObjectConstants.modelMatrix = FMatrix::Identity();
		ObjectConstants.PrevModelMatrix = FMatrix::Identity();
		MeshConstantBuffer->Map(&ObjectConstants);
	}

	UpdateTransform();
}

sSpriteComponent::sSpriteComponent(sSprite* pSprite)
	: Super(pSprite->GetName())
	, MeshContainer(sSpriteComponentMeshContainer::CreateUnique(this))
	, bFlip(false)
	, Sprite(pSprite)
	, Offset(FVector::Zero())
{
	MeshConstantBuffer = IConstantBuffer::Create(Name, sBufferDesc(sizeof(sMeshConstantBufferAttributes), 0), 1);
	{
		ObjectConstants.modelMatrix = FMatrix::Identity();
		ObjectConstants.PrevModelMatrix = FMatrix::Identity();
		MeshConstantBuffer->Map(&ObjectConstants);
	}

	UpdateTransform();
}

sSpriteComponent::~sSpriteComponent()
{
	MeshConstantBuffer = nullptr;
	Sprite = nullptr;
	MeshContainer = nullptr;
}

void sSpriteComponent::OnBeginPlay()
{
}

void sSpriteComponent::OnTick(const double DeltaTime)
{

}

void sSpriteComponent::SetSprite(sSprite* pSprite)
{
	Sprite = nullptr;
	Sprite = pSprite;
	UpdateOffset();
}

void sSpriteComponent::OnUpdateTransform()
{
	if (HasComponentOwner())
	{
		ObjectConstants.PrevModelMatrix = ObjectConstants.modelMatrix;
		ObjectConstants.modelMatrix = ToMatrixWithScale(GetRelativeLocation(), GetRelativeScale(), GetRelativeRotation());

		MeshConstantBuffer->Map(&ObjectConstants);
	}
	else
	{
		ObjectConstants.PrevModelMatrix = ObjectConstants.modelMatrix;
		ObjectConstants.modelMatrix = ToMatrixWithScale(GetRelativeLocation(), GetRelativeScale(), GetRelativeRotation());

		MeshConstantBuffer->Map(&ObjectConstants);
	}
}

bool sSpriteComponent::IsFlipped() const
{
	return bFlip;
}

void sSpriteComponent::Flip(bool value)
{
	if (bFlip == value)
		return;
	bFlip = value;

	if (bFlip)
	{
		SetRelativeRotation(FQuaternion(FAngles(0.0f, 180.0f, 0.0f)));
	}
	else
	{
		SetRelativeRotation(FQuaternion(FAngles(0.0f, 0.0f, 0.0f)));
	}
}

void sSpriteComponent::UpdateOffset()
{
	if (!HasComponentOwner() || !Sprite)
		return;

	auto SpriteBound = Sprite->GetBound();
	SetRelativeLocation(FVector(0.0f, (-1.0f) * ((SpriteBound.GetDimension().Height - GetOwner()->GetBounds().GetDimension().Height) / 2.0f), 0.0f) + Offset);
}

void sSpriteComponent::OnAttachToComponent()
{
	UpdateOffset();
}

void sSpriteComponent::OnDetachFromComponent()
{
}

void sSpriteComponent::Serialize(Archive& archive)
{
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
