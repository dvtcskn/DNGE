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
#include "Gameplay/GameInstance.h"

sGameInstance::sGameInstance(IMetaWorld* World)
	: Owner(World)
	, bIsInitialized(false)
	, bIsSplitScreenEnabled(false)
	, SplitScreenType(ESplitScreenType::Grid)
	, PlayerLimit(4)
{
}

sGameInstance::~sGameInstance()
{
	for (auto& Player : Players)
		Player = nullptr;
	Players.clear();
	for (auto& Player : PlayerProxys)
		Player = nullptr;
	PlayerProxys.clear();
	for (auto& Controllers : AIControllers)
		Controllers = nullptr;
	AIControllers.clear();
	Owner = nullptr;
}

void sGameInstance::BeginPlay()
{
	CreatePlayer();

	for (const auto& Player : Players)
		Player->BeginPlay();

	for (const auto& Player : PlayerProxys)
		Player->BeginPlay();

	for (const auto& Controller : AIControllers)
		Controller->BeginPlay();

	for (const auto& Player : Players)
		Player->SpawnPlayerFocusedActor();

	bIsInitialized = true;

	OnBeginPlay();
}

void sGameInstance::Tick(const double DeltaTime)
{
	for (const auto& Player : Players)
		Player->Tick(DeltaTime);

	for (const auto& Player : PlayerProxys)
		Player->Tick(DeltaTime);

	for (const auto& Controller : AIControllers)
		Controller->Tick(DeltaTime);

	OnTick(DeltaTime);
}

void sGameInstance::FixedUpdate(const double DeltaTime)
{
	for (const auto& Player : Players)
		Player->FixedUpdate(DeltaTime);

	for (const auto& Player : PlayerProxys)
		Player->FixedUpdate(DeltaTime);

	for (const auto& Controller : AIControllers)
		Controller->FixedUpdate(DeltaTime);

	OnFixedUpdate(DeltaTime);
}

sPlayer* sGameInstance::GetPlayer(std::size_t index, bool ByIndex) const
{
	if (ByIndex)
	{
		for (const auto& Player : Players)
		{
			if (Player->GetPlayerIndex() == index)
				return Player.get();
		}
		return nullptr;
	}
	else
	{
		return Players[index].get();
	}
	return nullptr;
}

std::vector<sPlayer*> sGameInstance::GetPlayers() const
{
	std::vector<sPlayer*> Result;
	Result.reserve(Players.size());
	std::transform(Players.cbegin(), Players.cend(), std::back_inserter(Result), [](auto& ptr) { return ptr.get(); });
	return Result;
}

std::vector<sPlayerProxyBase*> sGameInstance::GetPlayerProxys() const
{
	std::vector<sPlayerProxyBase*> Result;
	Result.reserve(PlayerProxys.size());
	std::transform(PlayerProxys.cbegin(), PlayerProxys.cend(), std::back_inserter(Result), [](auto& ptr) { return ptr.get(); });
	return Result;
}

void sGameInstance::InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar)
{
	for (const auto& Player : Players)
		Player->InputProcess(MouseInput, KeyboardChar);
}

void sGameInstance::WindowResized(const std::size_t Width, const std::size_t Height)
{
	OnWindowResized(Width, Height);
	for (const auto& Player : Players)
		Player->WindowResized(Width, Height);
}

void sGameInstance::SessionCreated()
{
	OnSessionCreated();
}

void sGameInstance::SessionDestroyed()
{
	OnSessionDestroyed();
}

void sGameInstance::Connected()
{
	for (auto& Player : PlayerProxys)
		Player = nullptr;
	PlayerProxys.clear();
	GetActiveLevel()->OnConnectedToServer();
	OnConnected();
}

void sGameInstance::Disconnected()
{
	for (auto& Player : PlayerProxys)
		Player = nullptr;
	PlayerProxys.clear();
	GetActiveLevel()->OnDisconnected();
	OnDisconnected();
}

void sGameInstance::PlayerConnected(std::string PlayerName, std::string NetAddress)
{
	if (Network::IsHost())
		return;
	if (!Network::IsConnected())
		return;

	sPlayerProxyBase::SharedPtr Player = ConstructPlayerProxy(PlayerName, NetAddress);
	if (!Player)
		return;

	PlayerProxys.push_back(Player);

	if (bIsInitialized)
		Player->BeginPlay();

	//Player->SpawnPlayerFocusedActor_Client(FVector(75.0f, 250.0f, 0.0f), 3);

	OnPlayerConnected(PlayerName, NetAddress);
}

void sGameInstance::PlayerDisconnected(std::string PlayerName, std::string NetAddress)
{
	if (Network::IsHost())
		return;
	if (!Network::IsConnected())
		return;

	auto erased = std::erase_if(PlayerProxys, [&](const std::shared_ptr<sPlayerProxyBase>& pPlayer) { return pPlayer->GetPlayerName() == PlayerName && pPlayer->GetClassNetworkAddress() == NetAddress; });

	if (erased > 0)
	{

	}
	OnPlayerDisconnected(PlayerName, NetAddress);
}

std::size_t sGameInstance::GetNextPlayerIndex()
{
	std::sort(Players.begin(), Players.end(), [](const sPlayer::SharedPtr& a, const sPlayer::SharedPtr& b) {
		return a->GetPlayerIndex() < b->GetPlayerIndex();
		});

	std::size_t Index = 0;
	for (const auto& Player : Players)
	{
		if (Player->GetPlayerIndex() == Index)
		{
			Index++;
		}
		else
		{
			return Index;
		}
	}
	return Index;
}

void sGameInstance::ResetLevel()
{
	GetActiveLevel()->Reset();

	for (const auto& Player : Players)
		Player->OnLevelReset();
}

void sGameInstance::OpenLevel(std::string Level)
{
	//std::lock_guard<std::mutex> locker(Mutex);
	GetMetaWorld()->GetActiveWorld()->SetActiveLevel(Level);

	for (const auto& Player : Players)
		Player->OnChangeLevel();
}

void sGameInstance::OpenWorld(std::string World)
{
	//std::lock_guard<std::mutex> locker(Mutex);
	GetMetaWorld()->SetActiveWorld(World);

	for (const auto& Player : Players)
		Player->OnChangeLevel();
}

void sGameInstance::EnableSplitScreen()
{
	if (Network::IsConnected() && !Network::IsHost())
		return;

	bIsSplitScreenEnabled = true;
	OnSplitScreenEnabled();
	for (auto& Player : Players)
		Player->SplitScreenEnabled();
}

void sGameInstance::DisableSplitScreen()
{
	bIsSplitScreenEnabled = false;
	OnSplitScreenDisabled();
	for (auto& Player : Players)
		Player->SplitScreenDisabled();
}

void sGameInstance::SetSplitScreenType(ESplitScreenType InSplitScreenType)
{
	if (Network::IsConnected() && !Network::IsHost())
		return;

	SplitScreenType = InSplitScreenType;
	OnSplitScreenTypeChanged(SplitScreenType);
	for (auto& Player : Players)
		Player->SplitScreenTypeChanged(SplitScreenType);
}

sPlayer* sGameInstance::CreatePlayer()
{
	if ((Network::IsConnected() && Players.size() == 1) && !Network::IsHost())
		return nullptr;

	if (GetPlayerCount() < PlayerLimit)
	{
		sPlayer::SharedPtr Player = ConstructPlayer();
		AddPlayer(Player);
		return Player.get();
	}
	return nullptr;
}

sPlayer* sGameInstance::CreatePlayer(eNetworkRole Role)
{
	if ((Network::IsConnected() && Players.size() == 1) && !Network::IsHost())
		return nullptr;

	if (GetPlayerCount() < PlayerLimit)
	{
		sPlayer::SharedPtr Player = ConstructPlayer();
		Player->SetNetworkRole(Role);
		AddPlayer(Player);
		return Player.get();
	}
	return nullptr;
}

void sGameInstance::AddPlayer(const sPlayer::SharedPtr& InPlayer)
{
	if ((Network::IsConnected() && Players.size() == 1) && !Network::IsHost())
		return;

	if (GetPlayerCount() < PlayerLimit)
	{
		Players.push_back(InPlayer);

		std::sort(Players.begin(), Players.end(), [](const sPlayer::SharedPtr& a, const sPlayer::SharedPtr& b) {
			return a->GetPlayerIndex() < b->GetPlayerIndex();
			});

		if (bIsInitialized)
			InPlayer->BeginPlay();
		OnNewPlayerAdded(GetPlayerIndex(InPlayer.get()));
		for (auto& Player : Players)
			Player->NewPlayerAdded();
	}
}

void sGameInstance::RemovePlayer(std::size_t Idx)
{
	if (IsPlayerIndexExist(Idx))
	{
		Players.erase(std::remove_if(Players.begin(), Players.end(), [&](const sPlayer::SharedPtr& Player)
			{
				return Player->GetPlayerIndex() == Idx;
			}), Players.end());

		std::sort(Players.begin(), Players.end(), [](const sPlayer::SharedPtr& a, const sPlayer::SharedPtr& b) {
			return a->GetPlayerIndex() < b->GetPlayerIndex();
			});

		OnPlayerRemoved();
		for (auto& Player : Players)
			Player->PlayerRemoved();
	}
}

void sGameInstance::RemovePlayer(sPlayer* InPlayer)
{
	/*Players.erase(std::find_if(Players.begin(), Players.end(), [&InPlayer](const std::shared_ptr<sPlayer>& pPlayer)
		{
			return InPlayer == pPlayer.get();
		}));*/

	auto erased = std::erase_if(Players, [&InPlayer](const std::shared_ptr<sPlayer>& pPlayer) { return InPlayer == pPlayer.get(); });
	std::sort(Players.begin(), Players.end(), [](const sPlayer::SharedPtr& a, const sPlayer::SharedPtr& b) {
		return a->GetPlayerIndex() < b->GetPlayerIndex();
		});

	if (erased > 0)
	{
		OnPlayerRemoved();
		for (auto& Player : Players)
			Player->PlayerRemoved();
	}
}

void sGameInstance::RemoveLastPlayer()
{
	std::size_t Idx = Players.size() - 1;
	Players[Idx] = nullptr;
	Players.erase(Players.begin() + Idx);
	OnPlayerRemoved();
	for (auto& Player : Players)
		Player->PlayerRemoved();
}

void sGameInstance::LimitPlayer(std::size_t InPlayerLimit)
{
	if (Network::IsConnected())
		return;

	PlayerLimit = InPlayerLimit;

	if (GetPlayerCount() > PlayerLimit)
	{
		std::size_t Limit = GetPlayerCount() - PlayerLimit;
		for (std::size_t i = 0; i < Limit; i++)
			RemoveLastPlayer();
	}
}

std::int32_t sGameInstance::GetPlayerIndex(sPlayer* PC) const
{
	ptrdiff_t pos = std::find_if(Players.begin(), Players.end(), [&PC](const std::shared_ptr<sPlayer>& pPlayer)
		{
			return PC == pPlayer.get();
		}) - Players.begin();

	if (pos >= (ptrdiff_t)Players.size())
	{
		return -1;
	}
	return std::int32_t(pos);
}

bool sGameInstance::IsPlayerIndexExist(std::size_t Index) const
{
	for (const auto& Player : Players)
	{
		if (Player->GetPlayerIndex() == Index)
			return true;
	}
	return false;
}

void sGameInstance::AddAIController(const sAIController::SharedPtr& Controller)
{
	if (Network::IsConnected() && !Network::IsHost())
		return;

	AIControllers.push_back(Controller);
	Controller->BeginPlay();
	OnNewAIControllerAdded();
}

void sGameInstance::RemoveAIController(sAIController* Controller)
{
	auto erased = std::erase_if(AIControllers, [&Controller](const std::shared_ptr<sAIController>& pController) { return Controller == pController.get(); });
	if (erased > 0)
	{
		OnAIControllerRemoved();
	}
}

void sGameInstance::RemoveAIController(std::size_t Idx)
{
	if (Idx < AIControllers.size())
	{
		AIControllers[Idx] = nullptr;
		AIControllers.erase(AIControllers.begin() + Idx);
		OnAIControllerRemoved();
	}
}
