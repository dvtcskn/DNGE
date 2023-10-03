
#include "pch.h"
#include "Gameplay/BoxCollision2DComponent.h"
#include "Engine/AbstractEngine.h"

sBoxCollision2DComponent::sBoxCollision2DComponent(std::string InName, const sRigidBodyDesc& Desc, const FDimension2D& InDimension, sActor* pActor)
	: Super(InName, pActor)
	, Dimension(InDimension)
{
	RigidBody = IRigidBody::Create2DBoxBody(this, Desc, FBounds2D(Dimension, FVector2::Zero()));
}

sBoxCollision2DComponent::~sBoxCollision2DComponent()
{
	RigidBody = nullptr;
}

void sBoxCollision2DComponent::OnBeginPlay()
{

}

void sBoxCollision2DComponent::OnFixedUpdate(const double DT)
{
	//GPU::DrawBound(GetBounds(), 0.05f);
}

FBoundingBox sBoxCollision2DComponent::GetBounds() const
{
	FVector Min;
	FVector Max;
	RigidBody->GetAabb(Min, Max);
	return FBoundingBox(Min, Max);
};

void sBoxCollision2DComponent::OnUpdateTransform()
{
	if (RigidBody->GetLocation() != GetRelativeLocation() || RigidBody->GetRotation() != GetRelativeRotation())
		RigidBody->SetTransform(GetRelativeLocation(), GetRelativeRotation());
}
