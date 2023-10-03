
#include "pch.h"
#include "Gameplay/CircleCollision2DComponent.h"
#include "Engine/AbstractEngine.h"

sCircleCollision2DComponent::sCircleCollision2DComponent(std::string InName, const sRigidBodyDesc& Desc, const FVector2& Origin, float InRadius, sActor* pActor)
	: Super(InName, pActor)
{
	RigidBody = IRigidBody::Create2DCircleBody(this, Desc, Origin, InRadius);
}

sCircleCollision2DComponent::~sCircleCollision2DComponent()
{
	RigidBody = nullptr;
}

void sCircleCollision2DComponent::OnBeginPlay()
{

}

void sCircleCollision2DComponent::OnFixedUpdate(const double DT)
{
	//GPU::DrawBound(GetBounds(), 0.05f);
}

FBoundingBox sCircleCollision2DComponent::GetBounds() const
{
	FVector Min;
	FVector Max;
	RigidBody->GetAabb(Min, Max);
	return FBoundingBox(Min, Max);
};

void sCircleCollision2DComponent::OnUpdateTransform()
{
	if (RigidBody->GetLocation() != GetRelativeLocation() || RigidBody->GetRotation() != GetRelativeRotation())
		RigidBody->SetTransform(GetRelativeLocation(), GetRelativeRotation());
}
