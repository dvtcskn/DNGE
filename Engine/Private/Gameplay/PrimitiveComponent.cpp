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

void sPrimitiveComponent::Serialize(Archive& archive)
{
	for (auto& Child : Children)
		Child->Serialize(archive);
}

void sPrimitiveComponent::SetRelativeLocation(const FVector& V)
{
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
	//if (!HasComponentOwner())
		return FVector::Zero();

	//return GetComponentOwner()->GetVelocity();
}

void sPrimitiveComponent::SetTransform(const FVector& InLocation, const FVector4& InRotation, const std::optional<FVector> InScale)
{
	if (Location != InLocation || Rotation != InRotation)
	{
		Location = InLocation;
		Rotation = InRotation;

		if (InScale.has_value())
		{
			if (Scale != *InScale)
				Scale = *InScale;
		}
		UpdateTransform();
	}
	else if (InScale.has_value())
	{
		if (Scale != *InScale)
		{
			Scale = *InScale;
			UpdateTransform();
		}
	}
}

void sPrimitiveComponent::UpdateTransform()
{
	OnUpdateTransform();
	for (auto& Child : Children)
		Child->UpdateTransform();
}
