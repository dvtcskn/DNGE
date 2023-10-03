
#include "pch.h"
#include "Gameplay/Player.h"
#include "Gameplay/GameInstance.h"

sPlayer::sPlayer(sGameInstance* InOwner, std::size_t InPlayerIndex)
	: Owner(InOwner)
	, Controller(sPlayerController::Create(this))
	, PlayerFocusedActor(nullptr)
	, PlayerIndex(InPlayerIndex)
	, DeferredRemovePlayerActor(false)
{
	SetPlayerFocusedActor(sActor::Create("DefaultActor"));
}

sPlayer::sPlayer(sGameInstance* InOwner, std::size_t InPlayerIndex, sPlayerController::SharedPtr PlayerController, sActor::SharedPtr InPlayerFocusedActor)
	: Owner(InOwner)
	, Controller(PlayerController)
	, PlayerFocusedActor(nullptr)
	, PlayerIndex(InPlayerIndex)
	, DeferredRemovePlayerActor(false)
{
	SetPlayerFocusedActor(InPlayerFocusedActor);
}

sPlayer::sPlayer(sGameInstance* InOwner, sPlayerController::SharedPtr PlayerController, sActor::SharedPtr InPlayerFocusedActor)
	: Owner(InOwner)
	, Controller(PlayerController)
	, PlayerFocusedActor(nullptr)
	, PlayerIndex(InOwner->GetPlayerCount())
	, DeferredRemovePlayerActor(false)
{
	SetPlayerFocusedActor(InPlayerFocusedActor);
}

sPlayer::~sPlayer()
{
	Controller->UnPossess(PlayerFocusedActor.get());
	Controller = nullptr;
	PlayerFocusedActor = nullptr;
	Owner = nullptr;
}

void sPlayer::BeginPlay()
{
	if (PlayerFocusedActor)
		Owner->GetActiveLevel()->SpawnPlayerActor(PlayerFocusedActor, Owner->GetPlayerIndex(this));

	Controller->BeginPlay();
	OnBeginPlay();
}

void sPlayer::Tick(const double DeltaTime)
{
	if (DeferredRemovePlayerActor && PlayerFocusedActor)
		PlayerFocusedActor = nullptr;

	Controller->Tick(DeltaTime);
	OnTick(DeltaTime);
}

void sPlayer::FixedUpdate(const double DeltaTime)
{
	Controller->FixedUpdate(DeltaTime);
	OnFixedUpdate(DeltaTime);
}

void sPlayer::SetPlayerFocusedActor(const sActor::SharedPtr& InPlayerFocusedActor)
{
	if (!InPlayerFocusedActor)
		return;

	InPlayerFocusedActor->UnPossess();
	if (PlayerFocusedActor)
	{
		auto Old = PlayerFocusedActor;
		Old->UnPossess();
		PlayerFocusedActor = nullptr;
	}

	if (PlayerFocusedActor != InPlayerFocusedActor)
	{
		PlayerFocusedActor = InPlayerFocusedActor;
		//PlayerFocusedActor->Possess(Controller.get());
		Controller->Possess(PlayerFocusedActor.get());
	}
}

void sPlayer::SpawnPlayerFocusedActor()
{
	if (PlayerFocusedActor)
		GetGameInstance()->GetActiveLevel()->SpawnPlayerActor(PlayerFocusedActor, GetPlayerIndex());
}

void sPlayer::RemovePlayerFocusedActor(bool DeferredRemove)
{
	if (!PlayerFocusedActor)
		return;

	PlayerFocusedActor->UnPossess();
	PlayerFocusedActor->RemoveFromLevel();

	if (DeferredRemove)
	{
		DeferredRemovePlayerActor = true;
		PlayerFocusedActor->SetEnabled(false);
		PlayerFocusedActor->Hide(true);
	}
	else
	{
		PlayerFocusedActor = nullptr;
	}
}

std::int32_t sPlayer::GetPlayerIndex() const
{
	return Owner->GetPlayerIndex(const_cast<sPlayer*>(this));
}

std::size_t sPlayer::GetPlayerCount() const
{
	return Owner->GetPlayerCount();
}

ESplitScreenType sPlayer::GetSplitScreenType() const
{
	return Owner->GetSplitScreenType();
}

void sPlayer::InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar)
{
	Controller->InputProcess(MouseInput, KeyboardChar);
}

void sPlayer::WindowResized(const std::size_t Width, const std::size_t Height)
{
	OnWindowResized(Width, Height);
	Controller->WindowResized(Width, Height);
}

void sPlayer::SplitScreenTypeChanged(ESplitScreenType InSplitScreenType)
{
	Controller->SplitScreenTypeChanged(InSplitScreenType);
	OnSplitScreenTypeChanged(InSplitScreenType);
}

void sPlayer::SplitScreenEnabled()
{
	Controller->SplitScreenEnabled();
	OnSplitScreenEnabled();
}

void sPlayer::SplitScreenDisabled()
{
	Controller->SplitScreenDisabled();
	OnSplitScreenDisabled();
}

void sPlayer::NewPlayerAdded()
{
	Controller->NewPlayerAdded();
	OnNewPlayerAdded();
}

void sPlayer::PlayerRemoved()
{
	Controller->PlayerRemoved();
	OnPlayerRemoved();
}

void sPlayer::SetPlayerIndex(std::size_t Index)
{
	PlayerIndex = Index;
}
