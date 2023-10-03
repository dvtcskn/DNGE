
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
