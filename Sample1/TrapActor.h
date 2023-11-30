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

#include <Gameplay/Actor.h>
#include <Gameplay/BoxCollision2DComponent.h>
#include <Gameplay/Controller.h>
#include "SpriteAnimationManager.h"
#include "SpriteComponent.h"
#include "SpriteSheetComponent.h"
#include "NetComponents.h"

class GTrapActorBase : public sActor
{
	sClassBody(sClassConstructor, GTrapActorBase, sActor)
public:
	GTrapActorBase(std::string InName = "", sController* InController = nullptr);
	virtual ~GTrapActorBase();

	virtual void OnBeginPlay() override;
	virtual void OnFixedUpdate(const double DeltaTime) override;

	void MoveLeft();
	void MoveRight();
	void MoveJump();
	void Stop();

	virtual void OnGrounded() {}
	virtual void OnJumping() {}
	virtual void OnFalling() {}

	inline float GetDamage() const { return Damage; }
	inline void SetDamage(float dmg) { Damage = dmg; }

	virtual std::string GetClassNetworkAddress() const override { return "Level::" + GetName(); }

private:
	float Damage;
};

class GSawTrapActor : public GTrapActorBase
{
	sClassBody(sClassConstructor, GSawTrapActor, GTrapActorBase)
public:
	GSawTrapActor(std::string InName = "", sController* InController = nullptr);
	virtual ~GSawTrapActor();

	virtual void OnBeginPlay() override final;
	virtual void OnFixedUpdate(const double DeltaTime) override final;

	void SetRange(FVector2 StartLoc, FVector2 EndLoc);
	void SetSpeed(float Speed);

private:
	void OnCollisionStart(sPhysicalComponent* Component);
	void OnCollisionEnd(sPhysicalComponent* Component);

private:
	FVector2 StartLoc;
	FVector2 EndLoc;
	bool bReverse;

	float Speed;
	double Time;
};

class GRockHeadActor : public GTrapActorBase
{
	sClassBody(sClassConstructor, GRockHeadActor, GTrapActorBase)
public:
	GRockHeadActor(std::string InName = "", sController* InController = nullptr);
	virtual ~GRockHeadActor();

	virtual void OnBeginPlay() override final;
	virtual void OnFixedUpdate(const double DeltaTime) override final;

	void SetRange(FVector2 StartLoc, FVector2 EndLoc);
	void SetSpeed(float Speed);

	virtual void Replicate(bool bReplicate) override;

private:
	void OnCollisionStart(sPhysicalComponent* Component);
	void OnCollisionEnd(sPhysicalComponent* Component);

	void OnHit_Top();
	void OnHit_Top_Client();
	void OnHit_Bottom();
	void OnHit_Bottom_Client();

	void AnimationStarted();
	void AnimationEnded();
	void AnimFrameUpdated(std::size_t Frame);

private:
	sBoxCollision2DComponent* BoxCollision2DComponent;

	FVector2 StartLoc;
	FVector2 EndLoc;
	bool bIsActive;

	float Speed;
	double Time;

	enum class eRockHeadState
	{
		idle,
		Top,
		bottom,
	};

	eRockHeadState State;
};
