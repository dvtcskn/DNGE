
#include "pch.h"
#include "Engine/IRigidBody.h"
#include "Gameplay/PhysicalComponent.h"

void IRigidBody::UpdatePhysics()
{
	if (HasOwner())
	{
		GetOwner()->UpdatePhysics();
		OnUpdatePhysics();
	}
}

void IRigidBody::OnCollisionStart(IRigidBody* Component)
{
	if (!Component)
		return;

	if (HasOwner() && Component->HasOwner())
	{
		GetOwner()->CollisionStart(Component->GetOwner());
	}
}

void IRigidBody::OnCollisionEnd(IRigidBody* Component)
{
	if (!Component)
		return;

	if (HasOwner() && Component->HasOwner())
	{
		GetOwner()->CollisionEnd(Component->GetOwner());
	}
}
