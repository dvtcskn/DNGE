#include "pch.h"
#include "GPlayerState.h"

GPlayerState::GPlayerState(sPlayerController* InOwner)
	: Super(InOwner)
{
}

GPlayerState::~GPlayerState()
{
}

void GPlayerState::BeginPlay()
{
	Super::BeginPlay();
}

void GPlayerState::Tick(const double DeltaTime)
{
	Super::Tick(DeltaTime);
}
