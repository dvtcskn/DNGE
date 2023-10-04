/* ---------------------------------------------------------------------------------------
* MIT License
*
* Copyright (c) 2023 Davut Co�kun.
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
#include "Gameplay/PlayerController.h"
#include "Gameplay/GameInstance.h"
#include "Engine/InputController.h"
#include "Gameplay/CameraComponent.h"
#include <Windows.h>

sPlayerController::sPlayerController(sPlayer* InOwner, const sPlayerState::SharedPtr& InPlayerState)
	: Super()
	, Owner(InOwner)
	, pPlayerState(InPlayerState)
	, PossessedActor(nullptr)
{
	if (!pPlayerState)
		pPlayerState = sPlayerState::Create(this);

	pCameraManager = sCameraManager::CreateUnique(this);
	pCameraManager->SetSpeed(900000);
}

sPlayerController::~sPlayerController()
{
	Owner = nullptr;
	UnPossess(PossessedActor);
	PossessedActor = nullptr;
	pPlayerState = nullptr;
	pCameraManager = nullptr;
}

void sPlayerController::BeginPlay()
{
	pPlayerState->BeginPlay();
	if (pCameraManager)
		pCameraManager->BeginPlay();

	OnBeginPlay();
}

void sPlayerController::Tick(const double DeltaTime)
{
	pPlayerState->Tick(DeltaTime);
	if (pCameraManager)
		pCameraManager->Tick(DeltaTime);

	OnTick(DeltaTime);
}

void sPlayerController::FixedUpdate(const double DeltaTime)
{
	pPlayerState->FixedUpdate(DeltaTime);
	if (pCameraManager)
		pCameraManager->FixedUpdate(DeltaTime);

	OnFixedUpdate(DeltaTime);
}

sActor* sPlayerController::GetPossessedActor() const
{
	return PossessedActor;
}

void sPlayerController::Possess(sActor* Actor)
{
	if (!Actor)
		return;

	Actor->UnPossess();
	if (PossessedActor)
		PossessedActor->UnPossess();

	if (PossessedActor != Actor)
	{
		PossessedActor = Actor;
		PossessedActor->Possess(this);

		if (pCameraManager)
			pCameraManager->AttachToActor(PossessedActor);

		OnPossess(PossessedActor);
	}
}

void sPlayerController::UnPossess(sActor* Actor)
{
	if (PossessedActor)
	{
		auto pActor = PossessedActor;
		PossessedActor = nullptr;
		pActor->UnPossess();

		if (pCameraManager)
			pCameraManager->DetachFromActor();

		OnUnPossess();
	}
}

bool sPlayerController::AddCanvasToViewport(ICanvas* Canvas)
{
	if (pCameraManager)
		return pCameraManager->AddCanvasToViewport(Canvas);
	return false;
}

bool sPlayerController::RemoveCanvasFromViewport(ICanvas* Canvas)
{
	if (pCameraManager)
		return pCameraManager->RemoveCanvasFromViewport(Canvas);
	return false;
}

bool sPlayerController::RemoveCanvasFromViewport(std::size_t Index)
{
	if (pCameraManager)
		return pCameraManager->RemoveCanvasFromViewport(Index);
	return false;
}

ICanvas* sPlayerController::GetCanvas(std::size_t Index)
{
	if (pCameraManager)
		return pCameraManager->GetCanvas(Index);
	return nullptr;
}

void sPlayerController::SetPlayerState(sPlayerState::SharedPtr PS)
{
	pPlayerState = nullptr;
	pPlayerState = PS;
}

std::int32_t sPlayerController::GetPlayerIndex() const
{
	return Owner->GetPlayerIndex();
}

std::size_t sPlayerController::GetPlayerCount() const
{
	return Owner->GetPlayerCount();
}

ESplitScreenType sPlayerController::GetSplitScreenType() const
{
	return Owner->GetSplitScreenType();
}

void sPlayerController::Save()
{

}

void sPlayerController::Load()
{

}

void sPlayerController::InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar)
{
	if (pCameraManager)
		pCameraManager->InputProcess(MouseInput, KeyboardChar);
}

void sPlayerController::WindowResized(const std::size_t Width, const std::size_t Height)
{
	OnWindowResized(Width, Height);
	if (pCameraManager)
		pCameraManager->WindowResized(Width, Height);
}

void sPlayerController::SplitScreenTypeChanged(ESplitScreenType InSplitScreenType)
{
	OnSplitScreenTypeChanged(InSplitScreenType);
	if (pCameraManager)
		pCameraManager->SplitViewport();
}

void sPlayerController::SplitScreenEnabled()
{
	OnSplitScreenEnabled();
	if (pCameraManager)
	{
		if (!pCameraManager->IsCameraEnabled())
			pCameraManager->EnableCamera();
		pCameraManager->SplitViewport();
	}
}

void sPlayerController::SplitScreenDisabled()
{
	OnSplitScreenDisabled();
	if (GetPlayerIndex() != 0)
		if (pCameraManager)
			pCameraManager->DisableCamera();
}

void sPlayerController::NewPlayerAdded()
{
	OnNewPlayerAdded();
	if (pCameraManager)
		pCameraManager->SplitViewport();
}

void sPlayerController::PlayerRemoved()
{
	OnPlayerRemoved();
	if (pCameraManager)
		pCameraManager->SplitViewport();
}
