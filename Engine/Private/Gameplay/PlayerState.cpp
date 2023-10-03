
#include "pch.h"
#include "Gameplay/PlayerState.h"
#include "Gameplay/PlayerController.h"

sPlayerState::sPlayerState(sPlayerController* InOwner)
	: Owner(InOwner)
{
}

sPlayerState::~sPlayerState()
{
	Owner = nullptr;
}

void sPlayerState::BeginPlay()
{
}

void sPlayerState::Tick(const double DeltaTime)
{
}

void sPlayerState::FixedUpdate(const double DeltaTime)
{
}
