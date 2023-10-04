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
