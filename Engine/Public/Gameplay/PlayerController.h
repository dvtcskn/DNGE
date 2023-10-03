#pragma once

#include "PlayerState.h"
#include "Character.h"
#include "Controller.h"
#include "Engine/ClassBody.h"
#include "Utilities/Input.h"
#include "CameraManager.h"

class sInputController;

class sPlayerController : public sController, public std::enable_shared_from_this<sPlayerController>
{
	sClassBody(sClassConstructor, sPlayerController, sController)
	friend class sPlayer;
public:
	sPlayerController(sPlayer* InOwner, const sPlayerState::SharedPtr& InPlayerState = nullptr);
	virtual ~sPlayerController();

	void BeginPlay();
	void Tick(const double DeltaTime);
	void FixedUpdate(const double DeltaTime);

	FORCEINLINE sPlayerState* GetPlayerState() const { return pPlayerState.get(); }

	template<class T>
	inline T* GetPlayer() const
	{
		return static_cast<T*>(Owner);
	}
	sFORCEINLINE sPlayer* GetPlayer() { return Owner; };

	sActor* GetPossessedActor() const;
	virtual void Possess(sActor* Actor) override;
	virtual void UnPossess(sActor* Actor) override;

	bool AddCanvasToViewport(ICanvas* Canvas);
	bool RemoveCanvasFromViewport(ICanvas* Canvas);
	bool RemoveCanvasFromViewport(std::size_t Index);
	ICanvas* GetCanvas(std::size_t Index);

	virtual void Save();
	virtual void Load();

	void SetPlayerState(sPlayerState::SharedPtr PS);

	FORCEINLINE sCameraManager* GetCameraManager() const { return pCameraManager.get(); };

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

	virtual void OnPossess(sActor* Actor) {}
	virtual void OnUnPossess() {}

	virtual void OnBeginPlay() {}
	virtual void OnTick(const double DeltaTime) {}
	virtual void OnFixedUpdate(const double DeltaTime) {}

private:
	sPlayer* Owner;
	sActor* PossessedActor;
	sCameraManager::UniquePtr pCameraManager;
	sPlayerState::SharedPtr pPlayerState;
};
