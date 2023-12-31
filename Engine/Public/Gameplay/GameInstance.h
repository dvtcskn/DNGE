/* ---------------------------------------------------------------------------------------
* MIT License
*
* Copyright (c) 2023 Davut Co�kun.
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

#include "PlayerController.h"
#include "Actor.h"
#include "Engine/ClassBody.h"
#include "Utilities/Input.h"
#include "Engine/IMetaWorld.h"
#include "AIController.h"
#include "Player.h"
#include "PlayerProxy.h"

class sGameInstance
{
	sBaseClassBody(sClassConstructor, sGameInstance)
	friend class IServer;
	friend class IClient;
public:
	sGameInstance(IMetaWorld* World);
	virtual ~sGameInstance();

	void BeginPlay();
	void Tick(const double DeltaTime);
	void FixedUpdate(const double DeltaTime);

	sPlayer* GetPlayer(std::size_t index = 0, bool ByIndex = true) const;
	std::vector<sPlayer*> GetPlayers() const;
	std::vector<sPlayerProxyBase*> GetPlayerProxys() const;
	inline sPlayerController* GetPlayerController(std::size_t index) const { return Players[index]->GetPlayerController(); }
	inline sActor* GetPlayerFocusedActor(std::size_t index) const { return Players[index]->GetPlayerFocusedActor(); }

	template<typename T>
	inline T* GetActiveLevel() const { return static_cast<T*>(Owner->GetActiveLevel()); }
	inline ILevel* GetActiveLevel() const { return Owner->GetActiveLevel(); }

	template<typename T>
	inline T* GetMetaWorld() const { return static_cast<T*>(Owner); }
	inline IMetaWorld* GetMetaWorld() const { return Owner; }

	virtual void ResetLevel();
	void OpenLevel(std::string Level);
	void OpenWorld(std::string World);

	virtual sPlayer::SharedPtr ConstructPlayer()
	{
		return sPlayer::Create(this, GetNextPlayerIndex());
	}
	virtual sPlayerProxyBase::SharedPtr ConstructPlayerProxy(std::string PlayerName, std::string NetAddress)
	{
		return nullptr;
	}

	void EnableSplitScreen();
	void DisableSplitScreen();
	inline bool IsSplitScreenEnabled() const { return bIsSplitScreenEnabled; }
	inline ESplitScreenType GetSplitScreenType() const { return SplitScreenType; }
	void SetSplitScreenType(ESplitScreenType InSplitScreenType);

	sPlayer* CreatePlayer();
	sPlayer* CreatePlayer(eNetworkRole Role);
	void AddPlayer(const sPlayer::SharedPtr& InPlayer);
	void RemovePlayer(std::size_t Idx);
	void RemovePlayer(sPlayer* InPlayer);
	void RemoveLastPlayer();

	inline std::size_t GetLimitPlayer() const { return PlayerLimit; }
	void LimitPlayer(std::size_t InPlayerLimit);

	std::int32_t GetPlayerIndex(sPlayer* PC) const;
	inline std::size_t GetPlayerCount() const { return Players.size(); }
	bool IsPlayerIndexExist(std::size_t Index) const;

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

	void SessionCreated();
	void SessionDestroyed();
	void Connected();
	void Disconnected();
	void PlayerConnected(std::string PlayerName, std::string NetAddress);
	void PlayerDisconnected(std::string PlayerName, std::string NetAddress);

	virtual void OnSessionCreated() {}
	virtual void OnSessionDestroyed() {}
	virtual void OnConnected() {}
	virtual void OnDisconnected() {}
	virtual void OnPlayerConnectedToServer(std::string PlayerName, std::string NetAddress) {}
	virtual void OnPlayerDisconnectedFromServer(std::string PlayerName, std::string NetAddress) {}
	virtual void OnPlayerConnected(std::string PlayerName, std::string NetAddress) {}
	virtual void OnPlayerDisconnected(std::string PlayerName, std::string NetAddress) {}

protected:
	std::size_t GetNextPlayerIndex();

private:
	bool bIsInitialized;
	IMetaWorld* Owner;
	std::vector<sPlayer::SharedPtr> Players;
	std::vector<sPlayerProxyBase::SharedPtr> PlayerProxys;
	bool bIsSplitScreenEnabled;
	ESplitScreenType SplitScreenType;
	std::size_t PlayerLimit;
	std::vector<sAIController::SharedPtr> AIControllers;
};
