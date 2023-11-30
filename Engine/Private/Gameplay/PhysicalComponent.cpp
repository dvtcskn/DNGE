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
#include "Gameplay/PlayerProxy.h"

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

void sPhysicalComponent::Replicate(bool bReplicate)
{
	if (IsReplicated() == bReplicate)
		return;

	Super::Replicate(bReplicate);

	if (IsReplicated())
	{
		RegisterRPCfn(GetClassNetworkAddress(), GetName(), "SetLinearVelocity_Client", eRPCType::Client, false, false, std::bind(&sPhysicalComponent::SetLinearVelocity_Client, this, std::placeholders::_1), FVector);
	}
	else
	{
		Network::UnregisterRPC(GetClassNetworkAddress(), GetName());
	}
}

void sPhysicalComponent::UpdatePhysics()
{
	if (HasRigidBody() && bEnablePhysics)
	{
		IRigidBody* RigidBody = GetRigidBody();
		if (RigidBody->GetLocation() != GetRelativeLocation() || RigidBody->GetRotation() != GetRelativeRotation())
			SetTransform(HasComponentOwner() ? RigidBody->GetLocation() - GetRelativeLocation() + GetLocalLocation() : RigidBody->GetLocation(), RigidBody->GetRotation(), GetRelativeScale());
	}
}

void sPhysicalComponent::OnUpdateTransform()
{

}

void sPhysicalComponent::SetCollisionEnabled(bool val)
{
	if (HasRigidBody())
		GetRigidBody()->SetEnabled(val);
}

bool sPhysicalComponent::IsCollisionEnabled() const
{
	return HasRigidBody() ? GetRigidBody()->IsEnabled() : false;
}

void sPhysicalComponent::SetCollisionChannel(ECollisionChannel Type)
{
	if (HasRigidBody())
		GetRigidBody()->SetCollisionChannel(Type);
}

void sPhysicalComponent::SetCollisionChannel(ECollisionChannel Type, std::uint16_t CollideTo)
{
	if (HasRigidBody())
		GetRigidBody()->SetCollisionChannel(Type, CollideTo);
}

ECollisionChannel sPhysicalComponent::GetCollisionChannel() const
{
	return HasRigidBody() ? GetRigidBody()->GetCollisionChannel() : ECollisionChannel::None;
}

void sPhysicalComponent::SetLinearVelocity(const FVector& V)
{
	if (IsReplicated() && Network::IsHost())
	{
		if (HasRigidBody())
			GetRigidBody()->SetLinearVelocity(V);

		Network::CallRPC(GetClassNetworkAddress(), GetName(), "SetLinearVelocity_Client", sArchive(V), false);
	}
	else if (IsReplicated() && Network::IsConnected())
	{
		if (GetOwner()->GetNetworkRole() == eNetworkRole::NetProxy)
		{
			return;
		}

		if (HasRigidBody())
			GetRigidBody()->SetLinearVelocity(V);

		//Network::CallRPC(GetClassNetworkAddress(), GetName(), "SetLinearVelocity_Server", sArchive(V), false);
	}
	else
	{
		if (HasRigidBody())
			GetRigidBody()->SetLinearVelocity(V);
	}
}

void sPhysicalComponent::SetLinearVelocity_Client(const FVector& V)
{
	if (Network::IsHost())
		return;

	if (!IsReplicated())
		return;

	if (HasRigidBody())
		GetRigidBody()->SetLinearVelocity(V);

	//Network::CallRPC(GetClassNetworkAddress(), "SetRelativeLocation_Server", sArchive(GetRelativeLocation()), false);
}

FVector sPhysicalComponent::GetVelocity() const
{
	return HasRigidBody() ? GetRigidBody()->GetLinearVelocity() : FVector::Zero();
}

void sPhysicalComponent::SetLinearDamping(const float v)
{
	if (HasRigidBody())
		GetRigidBody()->SetLinearDamping(v);
}

float sPhysicalComponent::GetLinearDamping() const
{
	return HasRigidBody() ? GetRigidBody()->GetLinearDamping() : 0.0f;
}

void sPhysicalComponent::SetAngularVelocity(float omega)
{
	if (HasRigidBody())
		GetRigidBody()->SetAngularVelocity(omega);
}

float sPhysicalComponent::GetAngularVelocity() const
{
	return HasRigidBody() ? GetRigidBody()->GetAngularVelocity() : 0.0f;
}

void sPhysicalComponent::SetAngularDamping(const float v)
{
	if (HasRigidBody())
		GetRigidBody()->SetAngularDamping(v);
}

float sPhysicalComponent::GetAngularDamping() const
{
	return HasRigidBody() ? GetRigidBody()->GetAngularDamping() : 0.0f;
}

void sPhysicalComponent::ApplyForce(const FVector& force, const FVector& point, bool wake)
{
	if (HasRigidBody())
		GetRigidBody()->ApplyForce(force, point, wake);
}

void sPhysicalComponent::ApplyForceToCenter(const FVector& force, bool wake)
{
	if (HasRigidBody())
		GetRigidBody()->ApplyForceToCenter(force, wake);
}

void sPhysicalComponent::ApplyTorque(float torque, bool wake)
{
	if (HasRigidBody())
		GetRigidBody()->ApplyTorque(torque, wake);
}

void sPhysicalComponent::ApplyLinearImpulse(const FVector& impulse, const FVector& point, bool wake)
{
	if (HasRigidBody())
		GetRigidBody()->ApplyLinearImpulse(impulse, point, wake);
}

void sPhysicalComponent::ApplyLinearImpulseToCenter(const FVector& impulse, bool wake)
{
	if (HasRigidBody())
		GetRigidBody()->ApplyLinearImpulseToCenter(impulse, wake);
}

void sPhysicalComponent::ApplyAngularImpulse(float impulse, bool wake)
{
	if (HasRigidBody())
		GetRigidBody()->ApplyAngularImpulse(impulse, wake);
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

void sPhysicalComponent::Serialize(sArchive& archive)
{
}
