#pragma once

#include <Gameplay/Actor.h>
#include <Gameplay/BoxCollision2DComponent.h>
#include <Gameplay/Controller.h>
#include "SpriteAnimationManager.h"
#include "SpriteComponent.h"
#include "SpriteSheetComponent.h"

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

private:
	void OnCollisionStart(sPhysicalComponent* Component);
	void OnCollisionEnd(sPhysicalComponent* Component);

	void OnHit_Top();
	void OnHit_Bottom();

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
