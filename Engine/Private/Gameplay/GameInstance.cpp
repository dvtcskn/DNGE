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
	, bIsSplitScreenEnabled(true)
	, SplitScreenType(ESplitScreenType::Grid)
	, PlayerLimit(4)
{}

sGameInstance::~sGameInstance()
{
	for (auto& Player : Players)
		Player = nullptr;
	Players.clear();
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

	for (const auto& Controller : AIControllers)
		Controller->BeginPlay();

	OnBeginPlay();
}

void sGameInstance::Tick(const double DeltaTime)
{
	for (const auto& Player : Players)
		Player->Tick(DeltaTime);

	for (const auto& Controller : AIControllers)
		Controller->Tick(DeltaTime);

	OnTick(DeltaTime);
}

void sGameInstance::FixedUpdate(const double DeltaTime)
{
	for (const auto& Player : Players)
		Player->FixedUpdate(DeltaTime);

	for (const auto& Controller : AIControllers)
		Controller->FixedUpdate(DeltaTime);

	OnFixedUpdate(DeltaTime);
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

void sGameInstance::EnableSplitScreen()
{
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
	SplitScreenType = InSplitScreenType;
	OnSplitScreenTypeChanged(SplitScreenType);
	for (auto& Player : Players)
		Player->SplitScreenTypeChanged(SplitScreenType);
}

void sGameInstance::CreatePlayer()
{
	sPlayer::SharedPtr Player = ConstructPlayer();
	AddPlayer(Player); 
	Player = nullptr;
}

void sGameInstance::AddPlayer(const sPlayer::SharedPtr& InPlayer)
{
	if (GetPlayerCount() < PlayerLimit)
	{
		Players.push_back(InPlayer);
		OnNewPlayerAdded(GetPlayerIndex(InPlayer.get()));
		for (auto& Player : Players)
			Player->NewPlayerAdded();
	}
}

void sGameInstance::RemovePlayer(std::size_t Idx)
{
	if (Idx < Players.size())
	{
		Players[Idx] = nullptr;
		Players.erase(Players.begin() + Idx);
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

void sGameInstance::AddAIController(const sAIController::SharedPtr& Controller)
{
	AIControllers.push_back(Controller);
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
