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

class sGameInstance;
class sAIController : public sController, public std::enable_shared_from_this<sAIController>
{
	sClassBody(sClassConstructor, sAIController, sController)
public:
	sAIController(sGameInstance* InOwner);
	virtual ~sAIController();

	void BeginPlay();
	void Tick(const double DeltaTime);
	void FixedUpdate(const double DeltaTime);

	sActor* GetPossessedActor(std::size_t index) const;
	template<typename T>
	inline T* GetPossessedActor(std::size_t index) const { return static_cast<T*>(PossessedActors.at(index)); }
	bool HasPossessedActor(sActor* Actor) const;
	virtual void Possess(sActor* Actor) override;
	virtual void UnPossess(sActor* Actor) override;

	inline std::size_t PossessedActorCount() const { return PossessedActors.size(); }
	inline const std::vector<sActor*>& GetPossessedActors() const { return PossessedActors; }

	inline sGameInstance* GetOwner() const { return Owner; }

private:
	virtual void OnPossess(sActor* Actor) {}
	virtual void OnUnPossess() {}

	virtual void OnBeginPlay() {}
	virtual void OnTick(const double DeltaTime) {}
	virtual void OnFixedUpdate(const double DeltaTime) {}

private:
	sGameInstance* Owner;
	std::vector<sActor*> PossessedActors;
};
