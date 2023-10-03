
#include "pch.h"
#include "Gameplay/AIController.h"
#include <Windows.h>

sAIController::sAIController(sGameInstance* InOwner)
	: Super()
	, Owner(InOwner)
{
}

sAIController::~sAIController()
{
	PossessedActors.clear();
}

void sAIController::BeginPlay()
{
	OnBeginPlay();
}

void sAIController::Tick(const double DeltaTime)
{
	OnTick(DeltaTime);
}

void sAIController::FixedUpdate(const double DeltaTime)
{
	OnFixedUpdate(DeltaTime);
}

sActor* sAIController::GetPossessedActor(std::size_t index) const
{
	return PossessedActors.at(index);
}

bool sAIController::HasPossessedActor(sActor* Actor) const
{
	if (!Actor)
		return false;

	if (std::find(PossessedActors.begin(), PossessedActors.end(), Actor) != PossessedActors.end())
		return true;
	return false;
}

void sAIController::Possess(sActor* Actor)
{
	if (!Actor)
		return;

	Actor->UnPossess();

	if (!HasPossessedActor(Actor))
	{
		PossessedActors.push_back(Actor);
		Actor->Possess(this);

		OnPossess(Actor);
	}
}

void sAIController::UnPossess(sActor* Actor)
{
	if (HasPossessedActor(Actor))
	{
		PossessedActors.erase(std::find(PossessedActors.begin(), PossessedActors.end(), Actor));
		Actor->UnPossess();
		OnUnPossess();
	}
}
