
#include "pch.h"
#include "Engine/IPhysicalWorld.h"
#include "Gameplay/PhysicalComponent.h"

void IPhysicalWorld::UpdateBodyPhysics()
{
	auto BodyCount = GetBodyCount();

	for (std::size_t i = 0; i < BodyCount; i++)
	{
		auto Body = GetBody(i);
		if (Body)
			Body->UpdatePhysics();
	}
}

void IPhysicalWorld::BeginCollision(IRigidBody* contactA, IRigidBody* contactB)
{
	if (contactA)
		contactA->OnCollisionStart(contactB);
	if (contactB)
		contactB->OnCollisionStart(contactA);
}

void IPhysicalWorld::EndCollision(IRigidBody* contactA, IRigidBody* contactB)
{
	if (contactA)
		contactA->OnCollisionEnd(contactB);
	if (contactB)
		contactB->OnCollisionEnd(contactA);
}
