/* ---------------------------------------------------------------------------------------
* MIT License
*
* Copyright (c) 2023 Davut Coþkun.
* All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
* ---------------------------------------------------------------------------------------
*/

#include "pch.h"
#include "Gameplay/AIController.h"
#include "Gameplay/GameInstance.h"
#include <Windows.h>

sAIController::sAIController(sGameInstance* InOwner)
	: Super()
	, Owner(InOwner)
	, Name(GetClassID())
	, NetworkRole(eNetworkRole::None)
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

std::string sAIController::GetClassNetworkAddress() const
{
	return "0::" + Name;
}

IMetaWorld* sAIController::GetMetaWorld() const
{
	return Owner->GetMetaWorld();
}

eNetworkRole sAIController::GetNetworkRole() const
{
	return NetworkRole;
}

void sAIController::SetNetworkRole(eNetworkRole Role)
{
	NetworkRole = Role;
}
