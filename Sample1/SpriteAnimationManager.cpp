
#include "pch.h"
#include "SpriteAnimationManager.h"
#include <Gameplay/Actor.h>

sSpriteAnimationManager::sSpriteAnimationManager(std::string Name)
	: Super(Name)
	, AnimationState(EAnimationState::eIdle)
	, fAnimationStarted(nullptr)
	, fAnimationEnd(nullptr)
	, fFrameUpdated(nullptr)
{
	auto SpriteComponent = sSpriteSheetComponent::Create("DefaultSpriteSheetComponent");
	SpriteComponent->AttachToComponent(this);
	SpriteSheetComponent = SpriteComponent.get();

	SpriteComponent->BindFunction_AnimationStarted(std::bind(&sSpriteAnimationManager::AnimationStarted, this));
	SpriteComponent->BindFunction_AnimationEnded(std::bind(&sSpriteAnimationManager::AnimationEnded, this));
	SpriteComponent->BindFunction_FrameUpdated(std::bind(&sSpriteAnimationManager::AnimationFrameUpdated, this, std::placeholders::_1));
}

sSpriteAnimationManager::~sSpriteAnimationManager() 
{
	SpriteSheetComponent = nullptr;
	States.clear();

	fAnimationStarted = nullptr;
	fAnimationEnd = nullptr;
	fFrameUpdated = nullptr;
}

void sSpriteAnimationManager::OnFixedUpdate(const double DeltaTime)
{
	const FVector& Velocity = GetOwner()->GetVelocity();
	const float Length = Velocity.Length();

	if (Length != 0.0f)
	{

	}
	else
	{

	}

	if (AnimationState == EAnimationState::eHit || AnimationState == EAnimationState::eDead)
	{

	}
	else if (Velocity.X == 0.0f && Velocity.Y == 0.0f)
	{
		SetAnimationState(EAnimationState::eIdle);
	}
	else if (Velocity.Y > 0.0f)
	{
		SetAnimationState(EAnimationState::eJump);
	}
	else if (Velocity.Y < 0.0f)
	{
		SetAnimationState(EAnimationState::eFalling);
	}
	else if (Velocity.X > 0.0f)
	{
		SetAnimationState(EAnimationState::eMoving);
	}
	else if (Velocity.X < 0.0f)
	{
		SetAnimationState(EAnimationState::eMoving);
	}

	if (Velocity.X < 0.0f)
	{
		SpriteSheetComponent->Flip(true);
	}
	else if (Velocity.X > 0.0f)
	{
		SpriteSheetComponent->Flip(false);
	}
}

void sSpriteAnimationManager::SetAnimationState(const EAnimationState State)
{
	if (AnimationState == State && SpriteSheetComponent->GetCurrentSpriteSheet())
		return;

	AnimationState = State;
	SpriteSheetComponent->SetSpriteSheet(Graph.States.at(AnimationState));
}

void sSpriteAnimationManager::SetGraph(const sSpriteAnimationGraph& InGraph)
{
	Graph = InGraph;
	SetAnimationState(EAnimationState::eIdle);
}

void sSpriteAnimationManager::AnimationStarted()
{
	if (fAnimationStarted)
		fAnimationStarted();
}

void sSpriteAnimationManager::AnimationEnded()
{
	if (fAnimationEnd)
		fAnimationEnd();

	SetAnimationState(EAnimationState::eIdle);
}

void sSpriteAnimationManager::AnimationFrameUpdated(std::size_t Frame)
{
	if (fFrameUpdated)
		fFrameUpdated(Frame);

	for (const auto& State : States)
	{
		if (AnimationState == State.second.State && State.second.Frame == Frame)
		{
			if (State.second.fOnFrame)
				State.second.fOnFrame();
		}
	}
}

bool sSpriteAnimationManager::IsFlipped() const
{
	return SpriteSheetComponent->IsFlipped();
}

void sSpriteAnimationManager::BindFunctionOnAnimationFrame(std::string Name, std::size_t Frame, EAnimationState State, std::function<void()> fOnFrame)
{
	States.insert(std::make_pair(Name, sAnimFunc(Frame, State, fOnFrame)));
}

void sSpriteAnimationManager::BindFunction_AnimationStarted(std::function<void()> pf)
{
	fAnimationStarted = pf;
}

void sSpriteAnimationManager::BindFunction_AnimationEnded(std::function<void()> pf)
{
	fAnimationEnd = pf;
}

void sSpriteAnimationManager::BindFunction_FrameUpdated(std::function<void(std::size_t)> pf)
{
	fFrameUpdated = pf;
}
