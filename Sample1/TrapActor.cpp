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

#include "pch.h"
#include "TrapActor.h"
#include "AssetManager.h"
#include <Gameplay/CircleCollision2DComponent.h>
#include "GPlayerCharacter.h"
#include "HelperFunctions.h"

GTrapActorBase::GTrapActorBase(std::string InName, sController* InController)
	: Super(InName, InController)
	, Damage(10)
{
}

GTrapActorBase::~GTrapActorBase()
{
}

void GTrapActorBase::OnBeginPlay()
{
}

void GTrapActorBase::OnFixedUpdate(const double DeltaTime)
{
}

void GTrapActorBase::MoveLeft()
{
}

void GTrapActorBase::MoveRight()
{
}

void GTrapActorBase::MoveJump()
{
}

void GTrapActorBase::Stop()
{
}

GSawTrapActor::GSawTrapActor(std::string InName, sController* InController)
	: Super(InName, InController)
	, StartLoc(FVector2::Zero())
	, EndLoc(FVector2::Zero())
	, bReverse(false)
	, Time(0.0f)
	, Speed(3.5f)
{
	auto Sprite = AssetManager::Get().GetSpriteSheet("Saw");
	auto Dim = Sprite->GetSpriteBound(0).GetDimension();

	auto SpriteComponent = sSpriteSheetComponent::Create("DefaultSpriteSheetComponent");
	SetRootComponent(SpriteComponent);
	SpriteComponent->SetSpriteSheet(Sprite);
	SpriteComponent->Play();

	{
		sRigidBodyDesc Desc;
		Desc.Friction = 0.0f;
		Desc.Mass = 1.0f;
		Desc.Restitution = 0.0f;
		Desc.RigidBodyType = ERigidBodyType::Kinematic;

		sCircleCollision2DComponent::SharedPtr Trap_BoxActorCollision = sCircleCollision2DComponent::Create("Saw_BoxActorCollision", Desc, FVector2::Zero(), Dim.Width / 2.0f);
		Trap_BoxActorCollision->AttachToComponent(SpriteComponent.get());
		Trap_BoxActorCollision->AddTag("Trap");
		Trap_BoxActorCollision->AddTag("Saw");
		Trap_BoxActorCollision->AddTag("Jumpable_Ground");
		//Trap_BoxActorCollision->GetRigidBody()->SetCollisionChannel(ECollisionType::Character);

		Trap_BoxActorCollision->BindFunctionToCollisionStart(std::bind(&GSawTrapActor::OnCollisionStart, this, std::placeholders::_1));
		Trap_BoxActorCollision->BindFunctionToCollisionEnd(std::bind(&GSawTrapActor::OnCollisionEnd, this, std::placeholders::_1));
	}
}

GSawTrapActor::~GSawTrapActor()
{
}

void GSawTrapActor::OnBeginPlay()
{
	Super::OnBeginPlay();
}

void GSawTrapActor::OnFixedUpdate(const double DeltaTime)
{
	Super::OnFixedUpdate(DeltaTime);

	Time = bReverse ? Time - DeltaTime / Speed : Time + DeltaTime / Speed;
	auto Loc = Lerp(FVector(StartLoc.X, StartLoc.Y, 0.0f), FVector(EndLoc.X, EndLoc.Y, 0.0f), SmoothStep(0.0f, 1.0f, (float)Time));
	SetLocation(FVector(Loc.X, Loc.Y, 0.0f));

	if (Time <= 0.0f && bReverse)
	{
		Time = 0.0f;
		bReverse = false;
	}
	else if (Time >= 1.0f)
	{
		Time = 1.0f;
		bReverse = true;
	}
}

void GSawTrapActor::SetRange(FVector2 InStartLoc, FVector2 InEndLoc)
{
	StartLoc = InStartLoc;
	SetLocation(FVector(StartLoc.X, StartLoc.Y, 0.0f));
	EndLoc = InEndLoc;
}

void GSawTrapActor::SetSpeed(float InSpeed)
{
	Speed = InSpeed;
}

void GSawTrapActor::OnCollisionStart(sPhysicalComponent* Component)
{
	if (Component)
	{
		Component->GetOwner<GPlayerCharacter>()->ApplyDamage(GetDamage(), true);
	}
}

void GSawTrapActor::OnCollisionEnd(sPhysicalComponent* Component)
{

}

GRockHeadActor::GRockHeadActor(std::string InName, sController* InController)
	: Super(InName, InController)
	, StartLoc(FVector2::Zero())
	, EndLoc(FVector2::Zero())
	, bIsActive(false)
	, Time(0.0f)
	, Speed(0.9f)
	, State(eRockHeadState::idle)
{
	SetDamage(100.0f);

	auto Sprite = AssetManager::Get().GetSpriteSheet("Rock_Head_Blink");
	auto Dim = Sprite->GetSpriteBound(0).GetDimension();

	auto SpriteComponent = sSpriteSheetComponent::Create("DefaultSpriteSheetComponent");
	SetRootComponent(SpriteComponent);
	SpriteComponent->SetSpriteSheet(Sprite);
	SpriteComponent->Play();

	SpriteComponent->BindFunction_AnimationStarted(std::bind(&GRockHeadActor::AnimationStarted, this));
	SpriteComponent->BindFunction_AnimationEnded(std::bind(&GRockHeadActor::AnimationEnded, this));
	SpriteComponent->BindFunction_FrameUpdated(std::bind(&GRockHeadActor::AnimFrameUpdated, this, std::placeholders::_1));

	{
		/* Simple Collision Level */
		sRigidBodyDesc Desc;
		Desc.Friction = 0.0f;
		Desc.Mass = 1.0f;
		Desc.Restitution = 0.0f;
		Desc.RigidBodyType = ERigidBodyType::Kinematic;

		sBoxCollision2DComponent::SharedPtr Trap_BoxActorCollision = sBoxCollision2DComponent::Create("Rock_Head_BoxActorCollision", Desc, Dim);
		BoxCollision2DComponent = Trap_BoxActorCollision.get();
		Trap_BoxActorCollision->AttachToComponent(SpriteComponent.get());
		Trap_BoxActorCollision->AddTag("Trap");
		Trap_BoxActorCollision->AddTag("RockHead");
		Trap_BoxActorCollision->AddTag("Jumpable_Ground");
		//Trap_BoxActorCollision->SetCollisionChannel(ECollisionChannel::Channel_4);

		Trap_BoxActorCollision->BindFunctionToCollisionStart(std::bind(&GRockHeadActor::OnCollisionStart, this, std::placeholders::_1));
		Trap_BoxActorCollision->BindFunctionToCollisionEnd(std::bind(&GRockHeadActor::OnCollisionEnd, this, std::placeholders::_1));
	}
}

GRockHeadActor::~GRockHeadActor()
{
	BoxCollision2DComponent = nullptr;
}

void GRockHeadActor::OnBeginPlay()
{
	Super::OnBeginPlay();
}

void GRockHeadActor::OnFixedUpdate(const double DeltaTime)
{
	Super::OnFixedUpdate(DeltaTime);

	if (State == eRockHeadState::idle && !bIsActive && Time <= 0.0f && CheckTag(BoxCollision2DComponent->GetBounds(), eTagDirectionType::eUp, "PlayerCharacter", true, BoxCollision2DComponent->GetBounds().GetWidth()))
	{
		bIsActive = true;
		State = eRockHeadState::Top;
	}

	if (State != eRockHeadState::idle)
	{
		Time = State == eRockHeadState::Top ? Time + DeltaTime / Speed : Time - DeltaTime / Speed;
		auto Loc = Lerp(FVector(StartLoc.X, StartLoc.Y, 0.0f), FVector(EndLoc.X, EndLoc.Y, 0.0f), SmoothStep(0.0f, 1.0f, (float)Time));
		SetLocation(FVector(Loc.X, Loc.Y, 0.0f));
	}

	if (Time <= 0.0f)
	{
		Time = 0.0f;

		if (State == eRockHeadState::bottom)
		{
			OnHit_Bottom();
		}
	}
	else if (Time >= 1.0f)
	{
		Time = 1.0f;
		OnHit_Top();
	}
}

void GRockHeadActor::SetRange(FVector2 InStartLoc, FVector2 InEndLoc)
{
	StartLoc = InStartLoc;
	SetLocation(FVector(StartLoc.X, StartLoc.Y, 0.0f));
	EndLoc = InEndLoc;
}

void GRockHeadActor::SetSpeed(float InSpeed)
{
	Speed = InSpeed;
}

void GRockHeadActor::OnCollisionStart(sPhysicalComponent* Component)
{
	if (Component)
	{
		//Component->GetOwner<GPlayerCharacter>()->ApplyDamage(GetDamage());
	}
}

void GRockHeadActor::OnCollisionEnd(sPhysicalComponent* Component)
{
}

void GRockHeadActor::OnHit_Top()
{
	auto Sprite = AssetManager::Get().GetSpriteSheet("Rock_Head_Top_Hit");
	GetRootComponent<sSpriteSheetComponent>()->SetSpriteSheet(Sprite);

	auto Loc = GetLocation();
	SetLocation(FVector(Loc.X, Loc.Y - 2, 0.0f));
	BoxCollision2DComponent->SetRelativeLocation(FVector(0.0f, -2.0f, 0.0f));

	auto Bound = BoxCollision2DComponent->GetBounds();
	auto Dimension3D = Bound.GetDimension();
	auto Center = Bound.GetCenter();
	FBounds2D Bounds(FDimension2D(Dimension3D.Width, 10.0f), FVector2(Center.X, Bound.Min.Y));
	auto Result = Physics::QueryAABB(Bounds);
	for (const auto& Res : Result)
	{
		if (Res->HasTag("PlayerCharacter"))
		{
			Res->GetOwner<GPlayerCharacter>()->ApplyDamage(100.0f);
			break;
		}
	}
}

void GRockHeadActor::OnHit_Bottom()
{
	auto Sprite = AssetManager::Get().GetSpriteSheet("Rock_Head_Bottom_Hit");
	GetRootComponent<sSpriteSheetComponent>()->SetSpriteSheet(Sprite);

	auto Loc = GetLocation();
	SetLocation(FVector(Loc.X, Loc.Y + 2, 0.0f));
	BoxCollision2DComponent->SetRelativeLocation(FVector(0.0f, 2.0f, 0.0f));

	auto Bound = BoxCollision2DComponent->GetBounds();
	auto Dimension3D = Bound.GetDimension();
	auto Center = Bound.GetCenter();
	FBounds2D Bounds(FDimension2D(Dimension3D.Width - 2, 20.0f), FVector2(Center.X, Bound.Max.Y));
	auto Result = Physics::QueryAABB(Bounds);
	for (const auto& Res : Result)
	{
		if (Res->HasTag("PlayerCharacter"))
		{
			Res->GetOwner<GPlayerCharacter>()->ApplyDamage(100.0f);
			break;
		}
	}
}

void GRockHeadActor::AnimationStarted()
{
}

void GRockHeadActor::AnimationEnded()
{
	std::string Name = GetRootComponent<sSpriteSheetComponent>()->GetCurrentSpriteSheet()->GetName();
	if (Name == "Rock_Head_Top_Hit")
	{
		auto Sprite = AssetManager::Get().GetSpriteSheet("Rock_Head_Blink");
		GetRootComponent<sSpriteSheetComponent>()->SetSpriteSheet(Sprite);
		Time = 1.0f;
		bIsActive = false;
		State = eRockHeadState::bottom;

		auto Loc = GetLocation();
		SetLocation(FVector(Loc.X, Loc.Y + 2, 0.0f));
		BoxCollision2DComponent->SetRelativeLocation(FVector(0.0f, 2.0f, 0.0f));
	}
	else if (Name == "Rock_Head_Bottom_Hit")
	{
		auto Sprite = AssetManager::Get().GetSpriteSheet("Rock_Head_Blink");
		GetRootComponent<sSpriteSheetComponent>()->SetSpriteSheet(Sprite);
		State = eRockHeadState::idle;

		auto Loc = GetLocation();
		SetLocation(FVector(Loc.X, Loc.Y - 2, 0.0f));
		BoxCollision2DComponent->SetRelativeLocation(FVector(0.0f, -2.0f, 0.0f));
	}
}

void GRockHeadActor::AnimFrameUpdated(std::size_t Frame)
{
}
