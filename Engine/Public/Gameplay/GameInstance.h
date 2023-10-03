#pragma once

#include "PlayerController.h"
#include "Actor.h"
#include "Engine/ClassBody.h"
#include "Utilities/Input.h"
#include "Engine/IMetaWorld.h"
#include "AIController.h"
#include "Player.h"

class sGameInstance
{
	sBaseClassBody(sClassConstructor, sGameInstance)
public:
	sGameInstance(IMetaWorld* World);
	virtual ~sGameInstance();

	void BeginPlay();
	void Tick(const double DeltaTime);
	void FixedUpdate(const double DeltaTime);

	inline sPlayer* GetPlayer(std::size_t index) const { return Players[index].get(); }
	inline sPlayerController* GetPlayerController(std::size_t index) const { return Players[index]->GetPlayerController(); }
	inline sActor* GetPlayerFocusedActor(std::size_t index) const { return Players[index]->GetPlayerFocusedActor(); }

	template<typename T>
	inline T* GetActiveLevel() const { return static_cast<T*>(Owner->GetActiveLevel()); }
	inline ILevel* GetActiveLevel() const { return Owner->GetActiveLevel(); }

	template<typename T>
	inline T* GetMetaWorld() const { return static_cast<T*>(Owner); }
	inline IMetaWorld* GetMetaWorld() const { return Owner; }

	virtual sPlayer::SharedPtr ConstructPlayer()
	{
		return sPlayer::Create(this, GetPlayerCount());
	}

	void EnableSplitScreen();
	void DisableSplitScreen();
	inline bool IsSplitScreenEnabled() const { return bIsSplitScreenEnabled; }
	inline ESplitScreenType GetSplitScreenType() const { return SplitScreenType; }
	void SetSplitScreenType(ESplitScreenType InSplitScreenType);

	void CreatePlayer();
	void AddPlayer(const sPlayer::SharedPtr& InPlayer);
	void RemovePlayer(std::size_t Idx);
	void RemovePlayer(sPlayer* InPlayer);
	void RemoveLastPlayer();

	inline std::size_t GetLimitPlayer() const { return PlayerLimit; }
	void LimitPlayer(std::size_t InPlayerLimit);

	std::int32_t GetPlayerIndex(sPlayer* PC) const;
	inline std::size_t GetPlayerCount() const { return Players.size(); }

	inline std::size_t GetAIControllersCount() const { return AIControllers.size(); }
	sAIController* GetAIController(std::size_t Idx = 0) const { return AIControllers.at(Idx).get(); }
	void AddAIController(const sAIController::SharedPtr& Controller);
	void RemoveAIController(sAIController* Controller);
	void RemoveAIController(std::size_t Idx);

	virtual void InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar);

	void WindowResized(const std::size_t Width, const std::size_t Height);

private:
	virtual void OnWindowResized(const std::size_t Width, const std::size_t Height) {}
	virtual void OnBeginPlay() {}
	virtual void OnTick(const double DeltaTime) {}
	virtual void OnFixedUpdate(const double DeltaTime) {}

	virtual void OnSplitScreenTypeChanged(ESplitScreenType InSplitScreenType) {}
	virtual void OnSplitScreenEnabled() {}
	virtual void OnSplitScreenDisabled() {}
	virtual void OnNewPlayerAdded(std::size_t Idx) {}
	virtual void OnPlayerRemoved() {}

	virtual void OnNewAIControllerAdded() {}
	virtual void OnAIControllerRemoved() {}

private:
	IMetaWorld* Owner;
	std::vector<sPlayer::SharedPtr> Players;
	bool bIsSplitScreenEnabled;
	ESplitScreenType SplitScreenType;
	std::size_t PlayerLimit;
	std::vector<sAIController::SharedPtr> AIControllers;
};
