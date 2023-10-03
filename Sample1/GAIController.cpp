
#include "pch.h"
#include "GAIController.h"
#include "GPlayerCharacter.h"
#include "GGameInstance.h"
#include "DefaultLevel.h"
#include <Utilities/Utilities.h>

GAIController::GAIController(sGameInstance* InOwner)
	: Super(InOwner)
{
}

GAIController::~GAIController()
{
}

void GAIController::OnBeginPlay()
{
}

void GAIController::OnTick(const double DeltaTime)
{

}

void GAIController::OnFixedUpdate(const double DeltaTime)
{
	/*auto Level = GetOwner()->GetActiveLevel<sDefaultLevel>();
	GPlayerCharacter* PlayerFocusedActor = static_cast<GPlayerCharacter*>(GetOwner()->GetPlayerFocusedActor(0));

	if (!PlayerFocusedActor)
		return;

	FBoundingBox PFABounds = PlayerFocusedActor->GetBounds();
	FVector PFALoc = PlayerFocusedActor->GetLocation();

	std::size_t Count = PossessedActorCount();

	for (std::size_t i = 0; i < Count; i++)
	{
		
	}*/
}

void GAIController::OnPossess(sActor* Actor)
{
}

void GAIController::OnUnPossess()
{
}
