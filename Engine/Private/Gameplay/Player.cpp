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
{
	SetPlayerFocusedActor(sActor::Create("DefaultActor"));
}

sPlayer::sPlayer(sGameInstance* InOwner, std::size_t InPlayerIndex, sPlayerController::SharedPtr PlayerController, sActor::SharedPtr InPlayerFocusedActor)
	: Owner(InOwner)
	, Controller(PlayerController)
	, PlayerFocusedActor(nullptr)
	, PlayerIndex(InPlayerIndex)
	, DeferredRemovePlayerActor(false)
{
	SetPlayerFocusedActor(InPlayerFocusedActor);
}

sPlayer::sPlayer(sGameInstance* InOwner, sPlayerController::SharedPtr PlayerController, sActor::SharedPtr InPlayerFocusedActor)
	: Owner(InOwner)
	, Controller(PlayerController)
	, PlayerFocusedActor(nullptr)
	, PlayerIndex(InOwner->GetPlayerCount())
	, DeferredRemovePlayerActor(false)
{
	SetPlayerFocusedActor(InPlayerFocusedActor);
}

sPlayer::~sPlayer()
{
	Controller->UnPossess(PlayerFocusedActor.get());
	Controller = nullptr;
	PlayerFocusedActor = nullptr;
	Owner = nullptr;
}

void sPlayer::BeginPlay()
{
	if (PlayerFocusedActor)
		Owner->GetActiveLevel()->SpawnPlayerActor(PlayerFocusedActor, Owner->GetPlayerIndex(this));

	Controller->BeginPlay();
	OnBeginPlay();
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

void sPlayer::SpawnPlayerFocusedActor()
{
	if (PlayerFocusedActor)
		GetGameInstance()->GetActiveLevel()->SpawnPlayerActor(PlayerFocusedActor, GetPlayerIndex());
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

std::int32_t sPlayer::GetPlayerIndex() const
{
	return Owner->GetPlayerIndex(const_cast<sPlayer*>(this));
}

std::size_t sPlayer::GetPlayerCount() const
{
	return Owner->GetPlayerCount();
}

ESplitScreenType sPlayer::GetSplitScreenType() const
{
	return Owner->GetSplitScreenType();
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
