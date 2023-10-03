#pragma once

#include <Gameplay/GameInstance.h>
#include "GPlayerState.h"
#include "GPlayer.h"

class GGameInstance : public sGameInstance
{
	sClassBody(sClassConstructor, GGameInstance, sGameInstance)
public:
	GGameInstance(IMetaWorld* Owner);
	virtual ~GGameInstance();

	virtual void OnBeginPlay() override final;
	virtual void OnTick(const double DeltaTime) override final;

	virtual sPlayer::SharedPtr ConstructPlayer() override final
	{
		return GPlayer::Create(this, GetPlayerCount());
	}

	virtual void InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar) override final;

private:

};
