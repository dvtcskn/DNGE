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
