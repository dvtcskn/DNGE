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

class sPlayerProxyBase;
class sProxyController : public sController, public std::enable_shared_from_this<sProxyController>
{
	sClassBody(sClassConstructor, sProxyController, sController)
public:
	sProxyController(sPlayerProxyBase* Proxy = nullptr)
		: Super()
		, Owner(Proxy)
		, PossessedActor(nullptr)
		, Name(GetClassID())
	{

	}
	virtual ~sProxyController()
	{
		Owner = nullptr;
		UnPossess(PossessedActor);
		PossessedActor = nullptr;
	}

	virtual void BeginPlay()
	{

	}
	virtual void Tick(const double DeltaTime)
	{

	}
	virtual void FixedUpdate(const double DeltaTime)
	{

	}

	virtual std::string GetName() const override final { return Name; }
	inline void SetName(std::string InName) { Name = InName; }
	virtual std::string GetClassNetworkAddress() const override;

	template<class T>
	inline T* GetPlayerProxyBase() const
	{
		return static_cast<T*>(Owner);
	}
	sFORCEINLINE sPlayerProxyBase* GetPlayerProxyBase() { return Owner; };
	void SetPlayerProxyBase(sPlayerProxyBase* InOwner) { Owner = InOwner; };

	sActor* GetPossessedActor() const
	{
		return PossessedActor;
	}
	template<class T>
	inline T* GetPossessedActor() const
	{
		return static_cast<T*>(PossessedActor);
	}
	virtual void Possess(sActor* Actor) override
	{
		if (!Actor)
			return;

		Actor->UnPossess();
		if (PossessedActor)
			PossessedActor->UnPossess();

		if (PossessedActor != Actor)
		{
			PossessedActor = Actor;
			PossessedActor->Possess(this);

			OnPossess(PossessedActor);
		}
	}
	virtual void UnPossess(sActor* Actor = nullptr) override
	{
		if (PossessedActor)
		{
			auto pActor = PossessedActor;
			PossessedActor = nullptr;
			pActor->UnPossess();

			OnUnPossess();
		}
	}

	virtual IMetaWorld* GetMetaWorld() const override final;

	virtual eNetworkRole GetNetworkRole() const override
	{
		return eNetworkRole::NetProxy;
	}

private:
	virtual void OnPossess(sActor* Actor) {}
	virtual void OnUnPossess() {}

private:
	std::string Name;
	sPlayerProxyBase* Owner;
	sActor* PossessedActor;
};

class sGameInstance;
class sPlayerProxyBase
{
	sBaseClassBody(sClassConstructor, sPlayerProxyBase)
	friend class sGameInstance;
public:
	sPlayerProxyBase(sGameInstance* InOwner, std::string PlayerName, std::string NetAddress, sActor::SharedPtr PlayerFocusedActor);
	sPlayerProxyBase(sGameInstance* InOwner, std::string PlayerName, std::string NetAddress, sProxyController::SharedPtr Controller, sActor::SharedPtr PlayerFocusedActor);
	virtual ~sPlayerProxyBase();

	template<class T>
	inline T* GetGameInstance() const
	{
		return static_cast<T*>(Owner);
	}
	sFORCEINLINE sGameInstance* GetGameInstance() { return Owner; };

	template<class T>
	inline T* GetController() const
	{
		return static_cast<T*>(Controller);
	}
	sProxyController* GetController() const;

	template<class T>
	inline T* GetPlayerFocusedActor() const
	{
		return static_cast<T*>(PlayerFocusedActor.get());
	}
	inline sActor* GetPlayerFocusedActor() const { return PlayerFocusedActor.get(); }
	inline std::string GetPlayerName() const { return Name; }
	inline std::string GetClassNetworkAddress() const { return NetworkAddress; }

private:
	void BeginPlay();
	void Tick(const double DeltaTime);
	void FixedUpdate(const double DeltaTime);

	virtual void OnBeginPlay() {}
	virtual void OnTick(const double DeltaTime) {}
	virtual void OnFixedUpdate(const double DeltaTime) {}

	void SpawnPlayerFocusedActor_Client(FVector Location, std::size_t LayerIndex);
	void DespawnPlayerFocusedActor();
	void RemovePlayerFocusedActor(bool DeferredRemove = true);

	void OnChangeLevel();

	//virtual void OnLevelReset() {}

private:
	std::string Name;
	sGameInstance* Owner;
	sProxyController::SharedPtr Controller;
	sActor::SharedPtr PlayerFocusedActor;
	bool DeferredRemovePlayerActor;
	std::string NetworkAddress;
};
