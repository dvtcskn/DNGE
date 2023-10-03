#pragma once

#include <DirectXMath.h>
#include "AnimationStates.h"
#include <map>
#include <vector>
#include <memory>
#include <optional>
#include <Engine/ClassBody.h>
#include <Gameplay/PrimitiveComponent.h>
#include "Sprite.h"
#include "SpriteComponent.h"
#include "SpriteSheetComponent.h"

struct sSpriteAnimationGraph
{
	std::map<EAnimationState, sSpriteSheet*> States;
};

struct sAnimFunc
{
	std::size_t Frame;
	EAnimationState State;
	std::function<void()> fOnFrame;

	sAnimFunc() = default;
	sAnimFunc(std::size_t InFrame, EAnimationState InState, std::function<void()> InfOnFrame)
		: Frame(InFrame)
		, State(InState)
		, fOnFrame(InfOnFrame)
	{}
	~sAnimFunc()
	{
		fOnFrame = nullptr;
	}
};

class sSpriteAnimationManager : public sPrimitiveComponent
{
	sClassBody(sClassConstructor, sSpriteAnimationManager, sPrimitiveComponent)
public:
	sSpriteAnimationManager(std::string Name = "SpriteAnimationManager");
	virtual ~sSpriteAnimationManager();

	virtual void OnFixedUpdate(const double DeltaTime) override final;

	EAnimationState GetAnimationState() const { return AnimationState; }
	void SetAnimationState(const EAnimationState State);

	void SetGraph(const sSpriteAnimationGraph& Graph);

	void AnimationStarted();
	void AnimationEnded();
	void AnimationFrameUpdated(std::size_t Frame);

	bool IsFlipped() const;

	void BindFunctionOnAnimationFrame(std::string Name, std::size_t Frame, EAnimationState State, std::function<void()> fOnFrame);

	std::map<std::string, sAnimFunc> States;

	void BindFunction_AnimationStarted(std::function<void()> pf);
	void BindFunction_AnimationEnded(std::function<void()> pf);
	void BindFunction_FrameUpdated(std::function<void(std::size_t)> pf);

private:
	EAnimationState AnimationState;
	sSpriteAnimationGraph Graph;

	sSpriteSheetComponent* SpriteSheetComponent;

	std::function<void()> fAnimationStarted;
	std::function<void()> fAnimationEnd;
	std::function<void(std::size_t)> fFrameUpdated;
};
