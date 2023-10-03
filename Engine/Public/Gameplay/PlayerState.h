#pragma once

#include <memory>
#include "Engine/ClassBody.h"

class sPlayerController;
class sPlayerState
{
	sBaseClassBody(sClassConstructor, sPlayerState)
public:
	sPlayerState(sPlayerController* InOwner);

	virtual ~sPlayerState();

	virtual void BeginPlay();
	virtual void Tick(const double DeltaTime);
	virtual void FixedUpdate(const double DeltaTime);

	sPlayerController* GetOwner() const { return Owner; };

private:
	sPlayerController* Owner;
};