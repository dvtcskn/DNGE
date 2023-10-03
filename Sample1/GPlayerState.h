#pragma once

#include <Gameplay/PlayerState.h>

class sPlayerController;
class GPlayerState : public sPlayerState
{
	sClassBody(sClassConstructor, GPlayerState, sPlayerState)
public:
	GPlayerState(sPlayerController* InOwner);

	virtual ~GPlayerState();

	virtual void BeginPlay() override final;
	virtual void Tick(const double DeltaTime) override final;
};