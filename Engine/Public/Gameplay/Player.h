#pragma once

#include "Engine/ClassBody.h"
#include "Utilities/Input.h"
#include "Engine/IMetaWorld.h"
#include "PlayerController.h"
#include "Actor.h"

class sGameInstance;
class sPlayer
{
	sBaseClassBody(sClassConstructor, sPlayer)
	friend sGameInstance;
public:
	sPlayer(sGameInstance* InOwner, std::size_t InPlayerIndex);
	sPlayer(sGameInstance* InOwner, std::size_t InPlayerIndex, sPlayerController::SharedPtr PlayerController, sActor::SharedPtr PlayerFocusedActor);
	sPlayer(sGameInstance* InOwner, sPlayerController::SharedPtr PlayerController, sActor::SharedPtr PlayerFocusedActor);
	virtual ~sPlayer();

	void BeginPlay();
	void Tick(const double DeltaTime);
	void FixedUpdate(const double DeltaTime);

	template<class T>
	inline T* GetGameInstance(const bool Dynamic = false) const
	{
		return static_cast<T*>(Owner);
	}
	sFORCEINLINE sGameInstance* GetGameInstance() { return Owner; };

	template<class T>
	inline T* GetPlayerController() const
	{
		return static_cast<T*>(Controller.get());
	}
	sFORCEINLINE sPlayerController* GetPlayerController() { return Controller.get(); };

	void SetPlayerFocusedActor(const sActor::SharedPtr& InPlayerFocusedActor);
	template<class T>
	inline T* GetPlayerFocusedActor() const
	{
		return static_cast<T*>(PlayerFocusedActor.get());
	}
	inline sActor* GetPlayerFocusedActor() const { return PlayerFocusedActor.get(); }
	void SpawnPlayerFocusedActor();
	void RemovePlayerFocusedActor(bool DeferredRemove = true);

	std::int32_t GetPlayerIndex() const;
	std::size_t GetPlayerCount() const;
	ESplitScreenType GetSplitScreenType() const;

	virtual void InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar);

private:
	void WindowResized(const std::size_t Width, const std::size_t Height);
	virtual void OnWindowResized(const std::size_t Width, const std::size_t Height) {}

	void SplitScreenTypeChanged(ESplitScreenType InSplitScreenType);
	virtual void OnSplitScreenTypeChanged(ESplitScreenType InSplitScreenType) {}
	void SplitScreenEnabled();
	void SplitScreenDisabled();
	virtual void OnSplitScreenEnabled() {}
	virtual void OnSplitScreenDisabled() {}
	void NewPlayerAdded();
	void PlayerRemoved();
	virtual void OnNewPlayerAdded() {}
	virtual void OnPlayerRemoved() {}

	virtual void OnBeginPlay() {}
	virtual void OnTick(const double DeltaTime) {}
	virtual void OnFixedUpdate(const double DeltaTime) {}

	void SetPlayerIndex(std::size_t Index);

private:
	sGameInstance* Owner;
	std::size_t PlayerIndex;
	sPlayerController::SharedPtr Controller;
	sActor::SharedPtr PlayerFocusedActor;
	bool DeferredRemovePlayerActor;
};
