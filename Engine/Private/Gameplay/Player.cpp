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
#include "Gameplay/Player.h"
#include "Gameplay/GameInstance.h"

sPlayer::sPlayer(sGameInstance* InOwner, std::size_t InPlayerIndex)
	: Owner(InOwner)
	, Controller(sPlayerController::Create(this))
	, PlayerFocusedActor(nullptr)
	, PlayerIndex(InPlayerIndex)
	, DeferredRemovePlayerActor(false)
	, Name("Player_" + std::to_string(InPlayerIndex))
	, bIsReplicated(false)
	, NetworkRole(eNetworkRole::None)
{
	SetPlayerFocusedActor(sActor::Create("DefaultActor"));
}

sPlayer::sPlayer(sGameInstance* InOwner, std::size_t InPlayerIndex, sPlayerController::SharedPtr PlayerController, sActor::SharedPtr InPlayerFocusedActor)
	: Owner(InOwner)
	, Controller(PlayerController)
	, PlayerFocusedActor(nullptr)
	, PlayerIndex(InPlayerIndex)
	, DeferredRemovePlayerActor(false)
	, Name("Player_" + std::to_string(InPlayerIndex))
	, bIsReplicated(false)
	, NetworkRole(eNetworkRole::None)
{
	SetPlayerFocusedActor(InPlayerFocusedActor);
}

sPlayer::sPlayer(sGameInstance* InOwner, sPlayerController::SharedPtr PlayerController, sActor::SharedPtr InPlayerFocusedActor)
	: Owner(InOwner)
	, Controller(PlayerController)
	, PlayerFocusedActor(nullptr)
	, PlayerIndex(InOwner->GetPlayerCount())
	, DeferredRemovePlayerActor(false)
	, Name("Player_" + std::to_string(PlayerIndex))
	, bIsReplicated(false)
	, NetworkRole(eNetworkRole::None)
{
	SetPlayerFocusedActor(InPlayerFocusedActor);
}

sPlayer::~sPlayer()
{
	if (bIsReplicated)
		Replicate(false);

	Controller->UnPossess(PlayerFocusedActor.get());
	PlayerFocusedActor->UnPossess();
	PlayerFocusedActor->RemoveFromLevel();
	PlayerFocusedActor = nullptr;
	Controller = nullptr;
	Owner = nullptr;
}

void sPlayer::BeginPlay()
{
	OnBeginPlay();
	Controller->BeginPlay();
}

void sPlayer::Tick(const double DeltaTime)
{
	if (DeferredRemovePlayerActor && PlayerFocusedActor)
		PlayerFocusedActor = nullptr;

	Controller->Tick(DeltaTime);
	OnTick(DeltaTime);
}

void sPlayer::FixedUpdate(const double DeltaTime)
{
	Controller->FixedUpdate(DeltaTime);
	OnFixedUpdate(DeltaTime);
}

void sPlayer::Replicate(bool bReplicate)
{
	if (bIsReplicated == bReplicate)
		return;

	bIsReplicated = bReplicate;

	if (bIsReplicated)
	{
		RegisterRPCfn(GetClassNetworkAddress(), "Player", "SpawnPlayerFocusedActor_Client", eRPCType::Client, true, false, std::bind(&sPlayer::SpawnPlayerFocusedActor_Client, this, std::placeholders::_1, std::placeholders::_2), FVector, std::size_t);
		RegisterRPCfn(GetClassNetworkAddress(), "Player", "SpawnPlayerFocusedActor_Server", eRPCType::Server, true, false, std::bind(&sPlayer::SpawnPlayerFocusedActor_Server, this));
	}
	else
	{
		Network::UnregisterRPC(GetClassNetworkAddress());
	}
}

void sPlayer::SpawnPlayerFocusedActor()
{
	if (PlayerFocusedActor)
	{
		DespawnPlayerFocusedActor();

		if (IsReplicated() && Network::IsConnected() && !Network::IsHost())
		{
			//Engine::WriteToConsole("SpawnPlayerFocusedActor_Client" + GetClassNetworkAddress() + " " + GetPlayerName());
			Network::CallRPC(GetClassNetworkAddress(), "Player", "SpawnPlayerFocusedActor_Server", sArchive(), true);
		}
		else if (IsReplicated() && Network::IsHost())
		{
			//Engine::WriteToConsole("SpawnPlayerFocusedActor_Server");
			auto Spawn = GetGameInstance()->GetActiveLevel()->GetSpawnNode("PlayerSpawn", GetPlayerIndex());
			if (PlayerFocusedActor->GetOwnedLevel() != GetGameInstance()->GetActiveLevel())
				PlayerFocusedActor->AddToActiveLevel(Spawn.Location, Spawn.LayerIndex);

			Network::CallRPC(GetClassNetworkAddress(), "Player", "SpawnPlayerFocusedActor_Client", sArchive(Spawn.Location, Spawn.LayerIndex), true);
		}
		else
		{
			//Engine::WriteToConsole("SpawnPlayerFocusedActor");
			auto Spawn = GetGameInstance()->GetActiveLevel()->GetSpawnNode("PlayerSpawn", GetPlayerIndex());
			PlayerFocusedActor->AddToActiveLevel(Spawn.Location, Spawn.LayerIndex);
		}
	}
}

void sPlayer::SpawnPlayerFocusedActor_Server()
{
	if (PlayerFocusedActor)
	{
		auto Spawn = GetGameInstance()->GetActiveLevel()->GetSpawnNode("PlayerSpawn", GetPlayerIndex());
		//Engine::WriteToConsole("SpawnPlayerFocusedActor_Server Player Name : " + Name + "| Location : " + Spawn.Location.ToString() + "| Layer : " + std::to_string(Spawn.LayerIndex));
		PlayerFocusedActor->AddToActiveLevel(Spawn.Location, Spawn.LayerIndex);
		Network::CallRPC(GetClassNetworkAddress(), "Player", "SpawnPlayerFocusedActor_Client", sArchive(Spawn.Location, Spawn.LayerIndex), true);
	}
}

void sPlayer::SpawnPlayerFocusedActor_Client(FVector Location, std::size_t LayerIndex)
{
	if (PlayerFocusedActor)
	{
		//Engine::WriteToConsole("SpawnPlayerFocusedActor_Client Player Name : " + Name + "| Location : " + Location.ToString() + "| Layer : " + std::to_string(LayerIndex));
		PlayerFocusedActor->AddToActiveLevel(Location, LayerIndex);
	}
}

void sPlayer::OnLevelReset()
{
	SpawnPlayerFocusedActor();
	Controller->OnLevelReset();
}

void sPlayer::DespawnPlayerFocusedActor()
{
	if (!PlayerFocusedActor)
		return;

	//Engine::WriteToConsole("DespawnPlayerFocusedActor");
	PlayerFocusedActor->RemoveFromLevel(false);
}

void sPlayer::SetPlayerFocusedActor(const sActor::SharedPtr& InPlayerFocusedActor)
{
	if (!InPlayerFocusedActor)
		return;

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

void sPlayer::RemovePlayerFocusedActor(bool DeferredRemove)
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

sViewportInstance* sPlayer::GetViewportInstance() const
{
	if (Controller)
		return Controller->GetViewportInstance();
	return nullptr;
}

std::int32_t sPlayer::GetPlayerIndex() const
{
	return PlayerIndex;
	//return Owner->GetPlayerIndex(const_cast<sPlayer*>(this));
}

std::size_t sPlayer::GetPlayerCount() const
{
	return Owner->GetPlayerCount();
}

ESplitScreenType sPlayer::GetSplitScreenType() const
{
	return Owner->GetSplitScreenType();
}

void sPlayer::SetPlayerName(std::string InName)
{
	Engine::WriteToConsole("Old Name : " + Name + " " + "Old Name : " + InName);
	Name = InName;
	Controller->OnPlayerNameChanged();
	if (Network::IsConnected())
	{
		//Network::CallRPCFromClient("OnNameChanged");
	}
}

std::string sPlayer::GetPlayerName() const
{
	return Name;
}

std::string sPlayer::GetClassNetworkAddress() const
{
	return std::to_string(PlayerIndex);
}

void sPlayer::SetNetworkRole(eNetworkRole Role)
{
	if (NetworkRole == Role)
		return;
	NetworkRole = Role;
	OnNetworkRoleChanged();
	Controller->OnPlayerNetworkRoleChanged();
}

void sPlayer::InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar)
{
	Controller->InputProcess(MouseInput, KeyboardChar);
}

void sPlayer::WindowResized(const std::size_t Width, const std::size_t Height)
{
	OnWindowResized(Width, Height);
	Controller->WindowResized(Width, Height);
}

void sPlayer::SplitScreenTypeChanged(ESplitScreenType InSplitScreenType)
{
	Controller->SplitScreenTypeChanged(InSplitScreenType);
	OnSplitScreenTypeChanged(InSplitScreenType);
}

void sPlayer::SplitScreenEnabled()
{
	Controller->SplitScreenEnabled();
	OnSplitScreenEnabled();
}

void sPlayer::SplitScreenDisabled()
{
	Controller->SplitScreenDisabled();
	OnSplitScreenDisabled();
}

void sPlayer::NewPlayerAdded()
{
	Controller->NewPlayerAdded();
	OnNewPlayerAdded();
}

void sPlayer::PlayerRemoved()
{
	Controller->PlayerRemoved();
	OnPlayerRemoved();
}

void sPlayer::SetPlayerIndex(std::size_t Index)
{
	PlayerIndex = Index;
}

void sPlayer::OnChangeLevel()
{
	PlayerFocusedActor->RemoveFromLevel();
	//SpawnPlayerFocusedActor();
}
