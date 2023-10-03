#pragma once

#include "Engine/ClassBody.h"
#include "Utilities/Input.h"
#include "Engine/IMetaWorld.h"
#include "GPlayerController.h"
#include <Gameplay/Player.h>
#include "GCanvas.h"

class GPlayer : public sPlayer
{
	sClassBody(sClassConstructor, GPlayer, sPlayer)
public:
	GPlayer(sGameInstance* InOwner, std::size_t InPlayerIndex);
	virtual ~GPlayer();

	virtual void OnBeginPlay() override;
	virtual void OnTick(const double DeltaTime) override;
	virtual void OnFixedUpdate(const double DeltaTime) override;

	virtual void OnCharacterDead();

private:
	void StartScreenFadeOut();
	void GameOverFadeOut();

private:
	cbgui::GCanvas::SharedPtr Canvas;
};
