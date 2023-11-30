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
#include "Gameplay/PrimitiveComponent.h"
#include "Gameplay/Actor.h"
#include <functional>

sPrimitiveComponent::sPrimitiveComponent(std::string InName, sActor* pActor)
	: Name(InName)
	, Owner(pActor)
	, bIsHidden(false)
	, bIsEnabled(true)
	, ComponentOwner(nullptr)
	, Location(FVector::Zero())
	, Rotation(FVector4(0.0f, 0.0f, 0.0f, 1.0f))
	, Scale(FVector(1.0f, 1.0f, 1.0f))
	, bIsReplicated(false)
{
}

sPrimitiveComponent::~sPrimitiveComponent()
{
	ComponentOwner = nullptr;

	for (auto& Child : Children)
		Child = nullptr;
	Children.clear();

	Owner = nullptr;
}

void sPrimitiveComponent::BeginPlay()
{
	OnBeginPlay();
	for (auto& Child : Children)
		Child->BeginPlay();
}

void sPrimitiveComponent::Tick(const double DeltaTime)
{
	OnTick(DeltaTime);
	for (auto& Child : Children)
		Child->Tick(DeltaTime);
}

void sPrimitiveComponent::FixedUpdate(const double DeltaTime)
{
	OnFixedUpdate(DeltaTime);
	for (auto& Child : Children)
		Child->FixedUpdate(DeltaTime);
}

std::string sPrimitiveComponent::GetClassNetworkAddress() const
{
	return Owner ? Owner->GetClassNetworkAddress() : ComponentOwner->GetClassNetworkAddress();
}

bool sPrimitiveComponent::HasOwner() const
{
	if (Owner != nullptr)
		return true;

	if (ComponentOwner)
		return ComponentOwner->HasOwner();

	return false;
}

sActor* sPrimitiveComponent::GetOwner() const
{
	if (Owner)
		return Owner;

	if (ComponentOwner)
		return ComponentOwner->GetOwner();

	return nullptr;
}

bool sPrimitiveComponent::AttachToActor(sActor* InParent)
{
	if (!InParent)
		return false;
	Owner = InParent;
	OnAttach();
	return true;
}

void sPrimitiveComponent::DetachFromActor()
{
	Owner = nullptr;
	OnDetach();
}

void sPrimitiveComponent::AddTag(const std::string& InTag, const std::optional<std::size_t> i)
{
	if (!i.has_value())
	{
		Tags.push_back(InTag);
	}
	else
	{
		if (i.value() >= Tags.size())
			return;
		Tags.insert(Tags.begin() + i.value(), InTag);
	}
}

void sPrimitiveComponent::SetTag(const std::size_t i, const std::string& InTag)
{
	Tags.at(i) = InTag;
}

bool sPrimitiveComponent::HasTag(std::string InTag)
{
	return std::find(Tags.begin(), Tags.end(), InTag) != Tags.end();
}

void sPrimitiveComponent::Replicate(bool bReplicate)
{
	if (bIsReplicated == bReplicate)
		return;

	bIsReplicated = bReplicate;

	if (bIsReplicated)
	{
		RegisterRPCfn(GetClassNetworkAddress(), GetName(), "SetRelativeLocation_Client", eRPCType::Client, false, false, std::bind(&sPrimitiveComponent::SetRelativeLocation_Client, this, std::placeholders::_1), FVector);
		RegisterRPCfn(GetClassNetworkAddress(), GetName(), "SetTransform_Client", eRPCType::Client, false, false, std::bind(&sPrimitiveComponent::SetTransform_Client, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), FVector, FVector4, FVector);
	}
	else
	{
		Network::UnregisterRPC(GetClassNetworkAddress(), GetName());
	}
}

void sPrimitiveComponent::Enable()
{
	bIsEnabled = true;
	OnEnabled();
}

void sPrimitiveComponent::Disable()
{
	bIsEnabled = false;
	OnDisabled();
}

void sPrimitiveComponent::Hide(bool value)
{
	bIsHidden = value;
	if (bIsHidden)
	{
		OnHidden();
	}
	else
	{
		OnVisible();
	}
}

bool sPrimitiveComponent::IsEnabled() const 
{
	return Owner ? Owner->IsEnabled() ? bIsEnabled : false : bIsEnabled;
}

bool sPrimitiveComponent::IsHidden() const 
{
	return Owner ? Owner->IsHidden() ? bIsHidden : false : bIsHidden;
}

bool sPrimitiveComponent::AttachComponent(sPrimitiveComponent::SharedPtr Component)
{
	Children.push_back(Component);
	OnChildAttached(Component.get());
	return true;
}

void sPrimitiveComponent::DetachComponent(sPrimitiveComponent* Component)
{
	Children.erase(std::find_if(Children.begin(), Children.end(), [Component](const sPrimitiveComponent::SharedPtr& Child)
		{
			return Child.get() == Component;
		}));
	OnChildDetached();
}

void sPrimitiveComponent::DetachComponent(std::size_t index)
{
	auto Child = Children.at(index);
	if (Child)
		Child->DetachFromComponent();
}

bool sPrimitiveComponent::AttachToComponent(sPrimitiveComponent* InParent)
{
	ComponentOwner = InParent;
	InParent->AttachComponent(shared_from_this());
	UpdateTransform();

	OnAttachToComponent();

	return true;
}

void sPrimitiveComponent::DetachFromComponent()
{
	auto pComponentOwner = ComponentOwner;
	ComponentOwner = nullptr;
	OnDetachFromComponent();
	pComponentOwner->DetachComponent(this);
}

sPrimitiveComponent* sPrimitiveComponent::GetComponentAttachmentRoot() const
{
	std::function<sPrimitiveComponent* (sPrimitiveComponent*)> GetParent;
	GetParent = [&](sPrimitiveComponent* pParent) -> sPrimitiveComponent*
	{
		if (pParent->HasComponentOwner())
			return GetParent(pParent->GetComponentOwner());
		return pParent;
	};

	if (HasComponentOwner())
		return GetParent(GetComponentOwner());
	return const_cast<sPrimitiveComponent*>(this);
}

void sPrimitiveComponent::Serialize(sArchive& archive)
{
	for (auto& Child : Children)
		Child->Serialize(archive);
}

void sPrimitiveComponent::SetRelativeLocation(const FVector& V)
{
	if (IsReplicated() && Network::IsHost())
	{
		Location = V;
		UpdateTransform();

		Network::CallRPC(GetClassNetworkAddress(), GetName(), "SetRelativeLocation_Client", sArchive(V), false);
	}
	else if (IsReplicated() && Network::IsConnected())
	{
		Location = V;
		UpdateTransform();
	}
	else
	{
		Location = V;
		UpdateTransform();
	}
}

void sPrimitiveComponent::SetRelativeLocation_Client(const FVector& V)
{
	if (Network::IsHost())
		return;

	if (!IsReplicated())
		return;

	Location = V;
	UpdateTransform();
}

FVector sPrimitiveComponent::GetRelativeLocation() const
{
	if (!HasComponentOwner())
		return Location;

	return Location + GetComponentOwner()->GetRelativeLocation();
}

void sPrimitiveComponent::SetRelativeRotation(const FVector4& V)
{
	Rotation = V; 
	UpdateTransform();
}

void sPrimitiveComponent::SetRollPitchYaw(FAngles RPY)
{
	SetRelativeRotation(FQuaternion(RPY));
}

FQuaternion sPrimitiveComponent::GetRelativeRotation() const
{
	if (!HasComponentOwner())
		return Rotation;

	return GetComponentOwner()->GetRelativeRotation() * FQuaternion(Rotation);
}

FAngles sPrimitiveComponent::GetRelativeRotationAngles() const
{
	return GetRelativeRotation().GetAngles();
}

void sPrimitiveComponent::SetRelativeScale(const FVector& V)
{
	Scale = V; 
	UpdateTransform();
}

FVector sPrimitiveComponent::GetRelativeScale() const
{
	if (!HasComponentOwner())
		return Scale;

	return GetComponentOwner()->GetRelativeScale() * Scale;
}

FVector sPrimitiveComponent::GetVelocity() const
{
	if (HasComponentOwner())
		return GetComponentOwner()->GetVelocity();

	return FVector::Zero();
}

void sPrimitiveComponent::SetTransform(const FVector& InLocation, const FVector4& InRotation, const FVector InScale)
{
	if (IsReplicated() && Network::IsHost())
	{
		if (Location != InLocation || Rotation != InRotation || Scale != InScale)
		{
			Location = InLocation;
			Rotation = InRotation;
			Scale = InScale;

			UpdateTransform();
		}

		Network::CallRPC(GetClassNetworkAddress(), GetName(), "SetTransform_Client", sArchive(InLocation, InRotation, InScale), false);
	}
	else if (IsReplicated() && Network::IsConnected())
	{
		if (Location != InLocation || Rotation != InRotation || Scale != InScale)
		{
			Location = InLocation;
			Rotation = InRotation;
			Scale = InScale;

			UpdateTransform();
		}
	}
	else
	{
		if (Location != InLocation || Rotation != InRotation || Scale != InScale)
		{
			Location = InLocation;
			Rotation = InRotation;
			Scale = InScale;

			UpdateTransform();
		}
	}
}

void sPrimitiveComponent::SetTransform_Client(const FVector& InLocation, const FVector4& InRotation, const FVector InScale)
{
	if (Network::IsHost())
		return;

	if (!IsReplicated())
		return;

	if (Location != InLocation || Rotation != InRotation || Scale != InScale)
	{
		Location = InLocation;
		Rotation = InRotation;
		Scale = InScale;

		UpdateTransform();
	}
}

void sPrimitiveComponent::UpdateTransform()
{
	OnUpdateTransform();
	for (auto& Child : Children)
		Child->UpdateTransform();
}
