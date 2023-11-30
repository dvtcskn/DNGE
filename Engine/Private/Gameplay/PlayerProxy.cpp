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
#include "Gameplay/PlayerProxy.h"
#include "Gameplay/GameInstance.h"

std::string sProxyController::GetClassNetworkAddress() const
{
	return Owner->GetClassNetworkAddress();
}

IMetaWorld* sProxyController::GetMetaWorld() const
{
	return Owner->GetGameInstance()->GetMetaWorld();
}

sPlayerProxyBase::sPlayerProxyBase(sGameInstance* InOwner, std::string PlayerName, std::string NetAddress, sActor::SharedPtr InPlayerFocusedActor)
	: Owner(InOwner)
	, NetworkAddress(NetAddress)
	, DeferredRemovePlayerActor(false)
	, Controller(sProxyController::Create(this))
	, Name(PlayerName)
{
	InPlayerFocusedActor->UnPossess();
	if (PlayerFocusedActor)
	{
		auto Old = PlayerFocusedActor;
		Old->UnPossess();
		PlayerFocusedActor = nullptr;
	}

	if (PlayerFocusedActor != InPlayerFocusedActor)
	{
		PlayerFocusedActor = InPlayerFocusedActor;
		//PlayerFocusedActor->Possess(Controller.get());
		Controller->Possess(PlayerFocusedActor.get());
	}
}
sPlayerProxyBase::sPlayerProxyBase(sGameInstance* InOwner, std::string PlayerName, std::string NetAddress, sProxyController::SharedPtr InController, sActor::SharedPtr InPlayerFocusedActor)
	: Owner(InOwner)
	, NetworkAddress(NetAddress)
	, DeferredRemovePlayerActor(false)
	, Controller(InController)
	, Name(PlayerName)
{
	Controller->SetPlayerProxyBase(this);

	InPlayerFocusedActor->UnPossess();
	if (PlayerFocusedActor)
	{
		auto Old = PlayerFocusedActor;
		Old->UnPossess();
		PlayerFocusedActor = nullptr;
	}

	if (PlayerFocusedActor != InPlayerFocusedActor)
	{
		PlayerFocusedActor = InPlayerFocusedActor;
		//PlayerFocusedActor->Possess(Controller.get());
		Controller->Possess(PlayerFocusedActor.get());
	}
}

sPlayerProxyBase::~sPlayerProxyBase()
{
	Network::UnregisterRPC(GetClassNetworkAddress());
	Controller->UnPossess(PlayerFocusedActor.get());
	PlayerFocusedActor->UnPossess();
	PlayerFocusedActor->RemoveFromLevel();
	PlayerFocusedActor = nullptr;
	Controller = nullptr;
	Owner = nullptr;
}

sProxyController* sPlayerProxyBase::GetController() const
{
	return Controller.get();
}

void sPlayerProxyBase::BeginPlay()
{
	RegisterRPCfn(GetClassNetworkAddress(), "Player", "SpawnPlayerFocusedActor_Client", eRPCType::Client, true, false, std::bind(&sPlayerProxyBase::SpawnPlayerFocusedActor_Client, this, std::placeholders::_1, std::placeholders::_2), FVector, std::size_t);

	OnBeginPlay();
	PlayerFocusedActor->BeginPlay();
	Controller->BeginPlay();
}

void sPlayerProxyBase::Tick(const double DeltaTime)
{
	if (DeferredRemovePlayerActor && PlayerFocusedActor)
		PlayerFocusedActor = nullptr;

	Controller->Tick(DeltaTime);
	OnTick(DeltaTime);
}

void sPlayerProxyBase::FixedUpdate(const double DeltaTime)
{
	Controller->FixedUpdate(DeltaTime);
	OnFixedUpdate(DeltaTime);
}

void sPlayerProxyBase::SpawnPlayerFocusedActor_Client(FVector Location, std::size_t LayerIndex)
{
	if (PlayerFocusedActor)
	{
		DespawnPlayerFocusedActor();
		PlayerFocusedActor->AddToActiveLevel(Location, LayerIndex);
	}
}

void sPlayerProxyBase::DespawnPlayerFocusedActor()
{
	if (!PlayerFocusedActor)
		return;

	//Engine::WriteToConsole("DespawnPlayerFocusedActor");
	PlayerFocusedActor->RemoveFromLevel(false);
}

void sPlayerProxyBase::RemovePlayerFocusedActor(bool DeferredRemove)
{
	if (!PlayerFocusedActor)
		return;

	PlayerFocusedActor->UnPossess();
	PlayerFocusedActor->RemoveFromLevel();

	if (DeferredRemove)
	{
		DeferredRemovePlayerActor = true;
		PlayerFocusedActor->SetEnabled(false);
		PlayerFocusedActor->Hide(true);
	}
	else
	{
		PlayerFocusedActor = nullptr;
	}
}

void sPlayerProxyBase::OnChangeLevel()
{
	PlayerFocusedActor->RemoveFromLevel();
	//SpawnPlayerFocusedActor();
}
