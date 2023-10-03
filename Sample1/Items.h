#pragma once

#include <Gameplay/Actor.h>
#include <Gameplay/BoxCollision2DComponent.h>
#include <Gameplay/Controller.h>
#include "SpriteAnimationManager.h"
#include "SpriteComponent.h"
#include "SpriteSheetComponent.h"

class GItem : public sActor
{
	sClassBody(sClassConstructor, GItem, sActor)
public:
	GItem(std::string Fruit);
	virtual ~GItem();

	virtual void OnBeginPlay() override;
	virtual void OnFixedUpdate(const double DeltaTime) override;

private:
	void AnimationStarted();
	void AnimationEnded();
	void AnimFrameUpdated(std::size_t Frame);

	void OnDestroyed();

	virtual void OnTransformUpdated() override;

private:
	sSpriteSheetComponent* SpriteSheetComponent;
	FBounds2D Bound;
	bool bDestroyed;
};
