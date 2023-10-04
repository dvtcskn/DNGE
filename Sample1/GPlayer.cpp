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
#include "GPlayer.h"
#include "GPlayerCharacter.h"

GPlayer::GPlayer(sGameInstance* InOwner, std::size_t InPlayerIndex)
	: Super(InOwner, InPlayerIndex, GPlayerController::Create(this), GPlayerCharacter::Create())
{
	Canvas = cbgui::GCanvas::Create(GetGameInstance()->GetMetaWorld(), (GPlayerCharacter*)GetPlayerFocusedActor());
	GetPlayerController()->AddCanvasToViewport(Canvas.get());

	Canvas->fOnMenuScreenFadeOut = std::bind(&GPlayer::StartScreenFadeOut, this);
	Canvas->fOnGameOverScreenFadeOut = std::bind(&GPlayer::GameOverFadeOut, this);
}

GPlayer::~GPlayer()
{
	Canvas = nullptr;
}

void GPlayer::OnBeginPlay()
{
}

void GPlayer::OnTick(const double DeltaTime)
{
}

void GPlayer::OnFixedUpdate(const double DeltaTime)
{
}

void GPlayer::OnCharacterDead()
{
	//RemovePlayerFocusedActor();

	GetPlayerFocusedActor()->SetEnabled(false);
	GetPlayerFocusedActor()->Hide(true);
	GetPlayerFocusedActor()->RemoveFromLevel();
	GetGameInstance()->GetMetaWorld()->PauseActiveWorld(true);
	Canvas->GameOver();
}

void GPlayer::StartScreenFadeOut()
{

}

void GPlayer::GameOverFadeOut()
{
	GetGameInstance()->GetMetaWorld()->PauseActiveWorld(false);
	GetGameInstance()->GetMetaWorld()->GetActiveLevel()->Reset();

	GetPlayerFocusedActor<GPlayerCharacter>()->Reset();
	GetPlayerFocusedActor()->SetEnabled(true);
	GetPlayerFocusedActor()->Hide(false);
	SpawnPlayerFocusedActor();
}
