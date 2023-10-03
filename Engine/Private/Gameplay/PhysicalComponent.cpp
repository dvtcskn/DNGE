
#include "pch.h"
#include "Gameplay/PhysicalComponent.h"
#include "Gameplay/Actor.h"

sPhysicalComponent::sPhysicalComponent(std::string InName, sActor* pActor)
	: Super(InName, pActor)
	, bEnablePhysics(true)
{
}

sPhysicalComponent::~sPhysicalComponent()
{
	fCollisionStart = nullptr;
	fCollisionEnd = nullptr;
}

void sPhysicalComponent::UpdatePhysics()
{
	if (HasRigidBody() && bEnablePhysics)
	{
		IRigidBody* RigidBody = GetRigidBody();
		if (RigidBody->GetLocation() != GetRelativeLocation() || RigidBody->GetRotation() != GetRelativeRotation())
			SetTransform(HasComponentOwner() ? RigidBody->GetLocation() - GetRelativeLocation() + GetLocalLocation() : RigidBody->GetLocation(), RigidBody->GetRotation());
	}
}

void sPhysicalComponent::OnUpdateTransform()
{

}

void sPhysicalComponent::SetCollisionEnabled(bool val)
{
	GetRigidBody()->SetEnabled(val);
}

bool sPhysicalComponent::IsCollisionEnabled() const
{
	return GetRigidBody()->IsEnabled();
}

void sPhysicalComponent::SetCollisionChannel(ECollisionChannel Type)
{
	GetRigidBody()->SetCollisionChannel(Type);
}

void sPhysicalComponent::SetCollisionChannel(ECollisionChannel Type, std::uint16_t CollideTo)
{
	GetRigidBody()->SetCollisionChannel(Type, CollideTo);
}

ECollisionChannel sPhysicalComponent::GetCollisionChannel() const
{
	return GetRigidBody()->GetCollisionChannel();
}

FVector sPhysicalComponent::GetVelocity() const
{
	return GetRigidBody()->GetLinearVelocity();
}

void sPhysicalComponent::CollisionStart(sPhysicalComponent* Component)
{
	if (fCollisionStart)
		fCollisionStart(Component); 
	OnCollisionStart(Component); 
}

void sPhysicalComponent::CollisionEnd(sPhysicalComponent* Component) 
{
	if (fCollisionEnd)
		fCollisionEnd(Component); 
	OnCollisionEnd(Component);
}

void sPhysicalComponent::Serialize(Archive& archive)
{
}
