#pragma once

#include <Gameplay/PlayerController.h>
#include "GPlayerCharacter.h"

class sGameInstance;
class GPlayerController : public sPlayerController
{
	sClassBody(sClassConstructor, GPlayerController, sPlayerController)
public:
	GPlayerController(sPlayer* InOwner);

	virtual ~GPlayerController();

	virtual void OnBeginPlay() override final;
	virtual void OnTick(const double DeltaTime) override final;

	virtual void Save() override final;
	virtual void Load() override final;

	virtual void InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar) override final;

	virtual void OnCharacterDead();

private:
	//virtual void OnNewPlayerAdded() {}
	//virtual void OnPlayerRemoved() {}

	virtual void OnPossess(sActor* Actor);
	virtual void OnUnPossess();
};
