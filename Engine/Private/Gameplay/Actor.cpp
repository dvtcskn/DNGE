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
#include "Gameplay/Actor.h"
#include "Gameplay/PlayerController.h"
#include "Gameplay/AIController.h"
#include "Engine/IMetaWorld.h"

sActor::sActor(std::string InName, sController* InController)
	: Name(InName)
	, Controller(InController)
	, Level(nullptr)
	, RootComponent(nullptr)
	, bIsHidden(false)
	, bIsEnabled(true)
	, LayerIndex(0)
	, bIsReplicated(false)
{}

sActor::~sActor()
{
	Replicate(false);
	UnPossess();
	RemoveFromLevel();
	Level = nullptr;
	Controller = nullptr;
	RootComponent = nullptr;
}

void sActor::BeginPlay()
{
	OnBeginPlay();
	RootComponent->BeginPlay();
}

void sActor::Tick(const double DeltaTime)
{
	if (!bIsEnabled)
		return;

	OnTick(DeltaTime);
	RootComponent->Tick(DeltaTime);
}

void sActor::FixedUpdate(const double DeltaTime)
{
	if (!bIsEnabled)
		return;

	OnFixedUpdate(DeltaTime);
	RootComponent->FixedUpdate(DeltaTime);
}

void sActor::Replicate(bool bReplicate)
{
	if (bIsReplicated == bReplicate/* || !Controller*/)
		return;

	bIsReplicated = bReplicate;

	if (bIsReplicated)
	{
		//RegisterRPCfn(GetClassNetworkAddress(), GetName(), "AddToActiveLevel_Server", eRPCType::Client, true, std::bind(&sActor::AddToActiveLevel_Server, this, std::placeholders::_1, std::placeholders::_2), FVector, std::size_t);
	}
	else
	{
		Network::UnregisterRPC(GetClassNetworkAddress(), GetName());
	}
}

eNetworkRole sActor::GetNetworkRole() const
{
	return Controller ? Controller->GetNetworkRole() : eNetworkRole::None;
}

std::string sActor::GetClassNetworkAddress() const
{
	return Controller ? Controller->GetClassNetworkAddress() : "-1";
}

bool sActor::AddToLevel(ILevel* pLevel, FVector SpawnLocation, std::size_t InLayerIndex)
{
	if (!pLevel)
		return false;

	if (Level)
	{
		auto ActiveLevel = GetController()->GetMetaWorld()->GetActiveLevel();
		if (Level == ActiveLevel)
		{
			if (IsReplicated() && Network::IsConnected() && !Network::IsHost())
			{
				Network::CallRPC(GetClassNetworkAddress(), GetName(), "AddToLevel_Server", sArchive(SpawnLocation, InLayerIndex), true);
				return true;
			}
			return false;
		}
	}

	Level = pLevel;

	/*if (IsReplicated() && Network::IsConnected() && !Network::IsHost())
	{
		Network::CallRPC(GetClassNetworkAddress(), GetName(), "AddToLevel_Server", sArchive(SpawnLocation, InLayerIndex), true);
		return true;
	}*/

	LayerIndex = InLayerIndex;
	Level->AddActor(shared_from_this(), LayerIndex);
	SetLocation(SpawnLocation);

	SetEnabled(true);
	Hide(false);

	return true;
}

void sActor::AddToLevel_Server(FVector SpawnLocation, std::size_t InLayerIndex)
{
	LayerIndex = InLayerIndex;
	Level->AddActor(shared_from_this(), LayerIndex);
	SetLocation(SpawnLocation);

	SetEnabled(true);
	Hide(false);
}

bool sActor::AddToLevel(std::string InLevel, FVector SpawnLocation, std::size_t InLayerIndex)
{
	if (!HasController())
		return false;

	if (InLevel == "")
		return false;

	if (Level)
	{
		auto ActiveLevel = GetController()->GetMetaWorld()->GetActiveLevel();
		if (Level == ActiveLevel)
		{
			if (IsReplicated() && Network::IsConnected() && !Network::IsHost())
			{
				Network::CallRPC(GetClassNetworkAddress(), GetName(), "AddToLevel_Server", sArchive(SpawnLocation, InLayerIndex), true);
				return true;
			}
			return false;
		}
	}

	Level = GetController()->GetMetaWorld()->GetActiveWorld()->GetLevel(InLevel);

	if (IsReplicated() && Network::IsConnected() && !Network::IsHost())
	{
		Network::CallRPC(GetClassNetworkAddress(), GetName(), "AddToLevel_Server", sArchive(SpawnLocation, InLayerIndex), true);
		return true;
	}

	LayerIndex = InLayerIndex;
	Level->AddActor(shared_from_this(), LayerIndex);
	SetLocation(SpawnLocation);

	SetEnabled(true);
	Hide(false);

	return true;
}

void sActor::AddToNamedLevel_Server(std::string InLevel, FVector SpawnLocation, std::size_t InLayerIndex)
{
	if (!HasController())
		return;

	if (InLevel == "")
		return;

	if (Level)
	{
		auto ActiveLevel = GetController()->GetMetaWorld()->GetActiveLevel();
		if (Level == ActiveLevel)
			return;
	}

	Level = GetController()->GetMetaWorld()->GetActiveWorld()->GetLevel(InLevel);

	LayerIndex = InLayerIndex;
	Level->AddActor(shared_from_this(), LayerIndex);
	SetLocation(SpawnLocation);

	SetEnabled(true);
	Hide(false);
}

bool sActor::AddToActiveLevel(FVector SpawnLocation, std::size_t InLayerIndex)
{
	if (!HasController())
		return false;

	if (Level)
	{
		auto ActiveLevel = GetController()->GetMetaWorld()->GetActiveLevel();
		if (Level == ActiveLevel)
			return false;
	}

	/*if (IsReplicated() && Network::IsConnected() && !Network::IsHost())
	{
		Network::CallRPC(GetClassNetworkAddress(), GetName(), "AddToActiveLevel_Server", sArchive(SpawnLocation, InLayerIndex), true);
		return true;
	}*/

	Engine::WriteToConsole("AddToActiveLevel : " + SpawnLocation.ToString());

	Level = GetController()->GetMetaWorld()->GetActiveLevel();

	LayerIndex = InLayerIndex;
	Level->AddActor(shared_from_this(), LayerIndex);
	SetLocation(SpawnLocation);

	SetEnabled(true);
	Hide(false);

	/*if (IsReplicated() && Network::IsConnected())
	{
		Network::CallRPC(GetClassNetworkAddress(), GetName(), "AddToActiveLevel_Server", sArchive(SpawnLocation, InLayerIndex), true);
		return true;
	}*/

	return true;
}

void sActor::AddToActiveLevel_Server(FVector SpawnLocation, std::size_t InLayerIndex)
{
	if (!HasController())
		return;

	if (Level)
	{
		auto ActiveLevel = GetController()->GetMetaWorld()->GetActiveLevel();
		if (Level == ActiveLevel)
			return;
	}

	Engine::WriteToConsole("AddToActiveLevel_Server : " + SpawnLocation.ToString());
	Level = GetController()->GetMetaWorld()->GetActiveLevel();

	LayerIndex = InLayerIndex;
	Level->AddActor(shared_from_this(), LayerIndex);
	SetLocation(SpawnLocation);

	SetEnabled(true);
	Hide(false);
}

void sActor::AddTag(const std::string& InTag, const std::optional<std::size_t> i)
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

void sActor::SetTag(const std::size_t i, const std::string& InTag)
{
	Tags.at(i) = InTag;
}

bool sActor::HasTag(std::string InTag)
{
	return std::find(Tags.begin(), Tags.end(), InTag) != Tags.end();
}

void sActor::SetRootComponent(const sPrimitiveComponent::SharedPtr& InComponent)
{
	RootComponent = InComponent;
	RootComponent->AttachToActor(this);
}

void sActor::RemoveFromLevel(bool bDeferredRemove)
{
	if (Level)
	{
		auto Old = Level;
		Level = nullptr;
		Old->RemoveActor(this, LayerIndex, bDeferredRemove);
	}
}

FBoundingBox sActor::GetBounds() const
{
	return RootComponent->GetBounds();
}

FVector sActor::GetLocation() const
{
	return RootComponent->GetRelativeLocation();
}

FVector sActor::GetScale() const
{
	return RootComponent->GetRelativeScale();
}

FQuaternion sActor::GetRotation() const
{
	return RootComponent->GetRelativeRotation();
}

FAngles sActor::GetRotationAngles() const
{
	return GetRotation().GetAngles();
}

void sActor::SetLocation(FVector InLocation)
{
	RootComponent->SetRelativeLocation(InLocation);
	OnTransformUpdated();
}

void sActor::SetRollPitchYaw(FAngles RPY)
{
	RootComponent->SetRelativeRotation(FQuaternion(RPY));
	OnTransformUpdated();
}

void sActor::SetScale(FVector InScale)
{
	RootComponent->SetRelativeScale(InScale);
	OnTransformUpdated();
}

void sActor::SetScale(float InScale)
{
	RootComponent->SetRelativeScale(InScale);
	OnTransformUpdated();
}

FVector sActor::GetVelocity() const
{
	return RootComponent->GetVelocity();
}

void sActor::Hide(bool value)
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

void sActor::SetEnabled(bool value)
{
	bIsEnabled = value;
	if (bIsEnabled)
	{
		OnEnabled();
	}
	else
	{
		OnDisabled();
	}
}

void sActor::Possess(sController* InController)
{
	if (sPlayerController* PC = dynamic_cast<sPlayerController*>(InController))
	{
		if (PC->GetPossessedActor() != this)
		{
			PC->Possess(this);
		}
		else
		{
			Controller = InController;
		}
	}
	else if (sAIController* AI = dynamic_cast<sAIController*>(InController))
	{
		if (!AI->HasPossessedActor(this))
		{
			AI->Possess(this);
		}
		else
		{
			Controller = InController;
		}
	}
	else
	{
		Controller = InController;
	}
}

void sActor::UnPossess()
{
	if (Controller)
	{
		auto pController = Controller;
		Controller = nullptr;
		pController->UnPossess(this);
	}
}

void sActor::Serialize(sArchive& archive)
{
}
