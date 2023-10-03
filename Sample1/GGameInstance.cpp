
#include "pch.h"
#include "GGameInstance.h"
#include "GPlayerController.h"
#include "GPlayerCharacter.h"
#include "GAIController.h"

GGameInstance::GGameInstance(IMetaWorld* World)
	: Super(World)
{
	AddAIController(GAIController::Create(this));
}

GGameInstance::~GGameInstance()
{
}

void GGameInstance::OnBeginPlay()
{

}

void GGameInstance::OnTick(const double DeltaTime)
{

}

void GGameInstance::InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar)
{
	Super::InputProcess(MouseInput, KeyboardChar);
}
