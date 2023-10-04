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
