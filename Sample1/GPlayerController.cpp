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
#include "GPlayerController.h"
#include "GPlayerState.h"
#include <Engine/AbstractEngine.h>
#include <Gameplay/BoxCollision2DComponent.h>
#include <Engine/InputController.h>
#include <Gameplay/GameInstance.h>
#include "GPlayer.h"

GPlayerController::GPlayerController(sPlayer* InOwner)
	: Super(InOwner, GPlayerState::Create(this))
{
	if (auto pCameraManager = GetCameraManager())
	{
		pCameraManager->DisableInputMovement();
		pCameraManager->DisableVelocity();
		auto Dim = GPU::GetInternalBaseRenderResolution();
		//pCameraManager->SetOrthographic(Dim.Width, Dim.Height);
		pCameraManager->SetOrthographic(Dim.Width, Dim.Height);
	}
}

GPlayerController::~GPlayerController()
{
	Canvas = nullptr;
}

void GPlayerController::OnBeginPlay()
{
	if (GetNetworkRole() != eNetworkRole::SimulatedProxy || GetNetworkRole() != eNetworkRole::NetProxy)
	{
		Canvas = cbgui::GCanvas::Create(GetMetaWorld(), (GPlayerCharacter*)GetPossessedActor());
		AddCanvasToViewport(Canvas.get());

		Canvas->fOnMenuScreenFadeOut = std::bind(&GPlayerController::StartScreenFadeOut, this);
		Canvas->fOnGameOverScreenFadeOut = std::bind(&GPlayerController::GameOverFadeOut, this);
	}
	Replicate(true);
}

void GPlayerController::OnTick(const double DeltaTime)
{
}

void GPlayerController::Save()
{
	Super::Save();
}

void GPlayerController::Load()
{
	Super::Load();
}

void GPlayerController::OnPossess(sActor* Actor)
{
	if (!GetPlayer()->GetGameInstance()->IsSplitScreenEnabled())
	{
		if (auto pCameraManager = GetCameraManager())
		{
			pCameraManager->DetachActorCamera();
			pCameraManager->DetachFromActor();
		}
	}
}

void GPlayerController::OnUnPossess()
{

}

void GPlayerController::InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar)
{
	Super::InputProcess(MouseInput, KeyboardChar);
}

void GPlayerController::OnCharacterDead()
{
	GetPossessedActor()->SetEnabled(false);
	GetPossessedActor()->Hide(true);
	GetPossessedActor()->RemoveFromLevel();
	GetPlayer<GPlayer>()->OnCharacterDead();

	if (Canvas)
		Canvas->GameOver();

	//UnPossess();
}

void GPlayerController::Replicate(bool bReplicate)
{
	if (IsReplicated() == bReplicate)
		return;

	Super::Replicate(bReplicate);

	if (IsReplicated())
	{
		RegisterRPCfn(GetClassNetworkAddress(), GetName(), "OnLevelReset_Client", eRPCType::Client, true, false, std::bind(&GPlayerController::OnLevelReset_Client, this));
	}
}

void GPlayerController::GameOverFadeOut()
{
	if (Network::IsClient() || GetNetworkRole() == eNetworkRole::SimulatedProxy || GetNetworkRole() == eNetworkRole::NetProxy)
		return;

	if (GetNetworkRole() != eNetworkRole::SimulatedProxy || GetNetworkRole() != eNetworkRole::NetProxy)
	{
		GetMetaWorld()->PauseActiveWorld(false);
		GetPlayer()->GetGameInstance()->ResetLevel();
	}

	GetPossessedActor<GPlayerCharacter>()->Reset();
	GetPossessedActor()->SetEnabled(true);
	GetPossessedActor()->Hide(false);
	GetPlayer()->SpawnPlayerFocusedActor();
}

void GPlayerController::StartScreenFadeOut()
{
}

void GPlayerController::OnPlayerNetworkRoleChanged()
{
	if (GetNetworkRole() == eNetworkRole::SimulatedProxy || GetNetworkRole() == eNetworkRole::NetProxy)
	{
		if (auto pCameraManager = GetCameraManager())
		{
			pCameraManager->RemoveCanvasFromViewport(Canvas.get());
		}
		Canvas = nullptr;
	}
}

void GPlayerController::OnSplitScreenEnabled()
{
	if (auto pCameraManager = GetCameraManager())
	{
		pCameraManager->AttachToActor(GetPossessedActor());
	}
}

void GPlayerController::OnSplitScreenDisabled()
{
	if (auto pCameraManager = GetCameraManager())
	{
		pCameraManager->DetachActorCamera();
		pCameraManager->DetachFromActor();
	}
}

void GPlayerController::OnLevelReset()
{
	Network::CallRPC(GetClassNetworkAddress(), GetName(), "OnLevelReset_Client", sArchive());
}

void GPlayerController::OnLevelReset_Client()
{
	if (Canvas && Network::IsClient())
		Canvas->GameOverFadeOut();

	GetPossessedActor<GPlayerCharacter>()->Reset();
	GetPossessedActor()->SetEnabled(true);
	GetPossessedActor()->Hide(false);
}

GProxyController::GProxyController(sPlayerProxyBase* Proxy)
	: Super(Proxy)
{
	SetName("sPlayerController");
}

GProxyController::~GProxyController()
{
	Network::UnregisterRPC(GetClassNetworkAddress(), GetName());
}

void GProxyController::BeginPlay()
{
	Super::BeginPlay();
	RegisterRPCfn(GetClassNetworkAddress(), GetName(), "OnLevelReset_Client", eRPCType::Client, true, false, std::bind(&GProxyController::OnLevelReset_Client, this));
}

void GProxyController::OnLevelReset_Client()
{
	GetPossessedActor<GPlayerCharacter>()->Reset();
	GetPossessedActor()->SetEnabled(true);
	GetPossessedActor()->Hide(false);
}
