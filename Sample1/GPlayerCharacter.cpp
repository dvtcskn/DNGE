
#include "pch.h"
#include "GPlayerCharacter.h"
#include <Engine/IRigidBody.h>
#include <Core/MeshPrimitives.h>
#include <Gameplay/MeshComponent.h>
#include <Gameplay/CameraComponent.h>
#include <Gameplay/BoxCollision2DComponent.h>
#include <Gameplay/CircleCollision2DComponent.h>
#include <AbstractGI/MaterialManager.h>
#include "MetaWorld.h"
#include <Utilities/Utilities.h>
#include "SpriteEffectComponent.h"
#include "SpriteAnimationManager.h"
#include "GPlayerController.h"
#include "GAIController.h"

#include "AssetManager.h"
#include <Engine/Box2DRigidBody.h>
//#include <Engine/BulletRigidBody.h>

#include "HelperFunctions.h"

using namespace Utilities;

GPlayerCharacter::GPlayerCharacter(std::string InName)
	: Super(InName)
	, Horizontal_Move_Direction(0)
	, Move_Speed(6.0f)
	, JumpForce(-8.0f)
	, Health(100)
	, AppleCount(0)
	, CherrieCount(0)
	, bIsAI(false)
{
	AddTag("PlayerCharacter");
	{
		sRigidBodyDesc Desc;
		Desc.Friction = 1.0f;
		Desc.Mass = 1.0f;
		Desc.Restitution = 0.0f;
		Desc.RigidBodyType = ERigidBodyType::Dynamic;

		sBoxCollision2DComponent::SharedPtr BoxCollision2DComponent = sBoxCollision2DComponent::Create("BoxActorCollision", Desc, FDimension2D(14.0f, 20.0f));
		pBoxCollision2DComponent = BoxCollision2DComponent.get();

		SetRootComponent(BoxCollision2DComponent);
		SetLocation(FVector(640 / 2, 250, 0.0f));

		pBoxCollision2DComponent->AddTag("PlayerCharacter");

		{
			sSpriteAnimationGraph SpriteSheetGraph;
			{
				SpriteSheetGraph.States.insert({ EAnimationState::eIdle, AssetManager::Get().GetSpriteSheet("IDLE") });
				SpriteSheetGraph.States.insert({ EAnimationState::eMoving, AssetManager::Get().GetSpriteSheet("Running") });
				SpriteSheetGraph.States.insert({ EAnimationState::eJump, AssetManager::Get().GetSpriteSheet("Jump") });
				SpriteSheetGraph.States.insert({ EAnimationState::eFalling, AssetManager::Get().GetSpriteSheet("Fall") });
				SpriteSheetGraph.States.insert({ EAnimationState::eHit, AssetManager::Get().GetSpriteSheet("Hit") });
				SpriteSheetGraph.States.insert({ EAnimationState::eDead, AssetManager::Get().GetSpriteSheet("Dead") });
			}

			auto AnimationManager = sSpriteAnimationManager::Create();
			AnimManager = AnimationManager.get();
			AnimManager->SetGraph(SpriteSheetGraph);
			AnimManager->AttachToComponent(pBoxCollision2DComponent);
			AnimManager->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));

			//AnimManager->BindFunction_AnimationEnded(std::bind(&GPlayerCharacter::OnCharacterDead, this));
			AnimManager->BindFunctionOnAnimationFrame("Dead", 6, EAnimationState::eDead, std::bind(&GPlayerCharacter::OnCharacterDead, this));
		}
	}
		
	auto Body = pBoxCollision2DComponent->GetRigidBody();
	Body->SetFixedRotation(true);
	Body->SetAngularDamping(100.0f);
}

GPlayerCharacter::~GPlayerCharacter()
{
	pBoxCollision2DComponent = nullptr;
	AnimManager = nullptr;
}

void GPlayerCharacter::OnBeginPlay()
{
	auto PC = dynamic_cast<GPlayerController*>(GetController());

	if (!PC)
	{
		auto AIC = dynamic_cast<GAIController*>(GetController());
		if (AIC)
			bIsAI = true;
		return;
	}

	auto Index = PC->GetPlayerIndex();

	auto InputController = Engine::GetInputController();
	{
		sKMButtonInputDesc KMButtonInputDesc;
		KMButtonInputDesc.bHoldable = true;
		KMButtonInputDesc.Key = Index == 0 ? 'D' : 'L';
		KMButtonInputDesc.fOnPressed = std::bind(&GPlayerCharacter::OnBindKey_RightMovementKey, this, std::placeholders::_1);
		KMButtonInputDesc.fOnReleased = std::bind(&GPlayerCharacter::OnBindKey_RightMovementKey_Released, this, std::placeholders::_1);
		InputController->BindInput(Index == 0 ? "RightMovementKey" : "RightMovementKey2", KMButtonInputDesc);
	}
	{
		sKMButtonInputDesc KMButtonInputDesc;
		KMButtonInputDesc.bHoldable = true;
		KMButtonInputDesc.Key = Index == 0 ? 'A' : 'J';
		KMButtonInputDesc.fOnPressed = std::bind(&GPlayerCharacter::OnBindKey_LeftMovementKey, this, std::placeholders::_1);
		KMButtonInputDesc.fOnReleased = std::bind(&GPlayerCharacter::OnBindKey_LeftMovementKey_Released, this, std::placeholders::_1);
		InputController->BindInput(Index == 0 ? "LeftMovementKey" : "LeftMovementKey2", KMButtonInputDesc);
	}
	{
		sKMButtonInputDesc KMButtonInputDesc;
		KMButtonInputDesc.bHoldable = true;
		KMButtonInputDesc.Key = Index == 0 ? ' ' : 'K';
		KMButtonInputDesc.fOnPressed = std::bind(&GPlayerCharacter::OnBindKey_JumpMovementKey, this, std::placeholders::_1);
		KMButtonInputDesc.fOnReleased = std::bind(&GPlayerCharacter::OnBindKey_JumpMovementKey_Released, this, std::placeholders::_1);
		InputController->BindInput(Index == 0 ? "JumpMovementKey" : "JumpMovementKey2", KMButtonInputDesc);
	}
	{
		sGamepadButtonInputDesc ButtonInputDesc;
		ButtonInputDesc.a.fOnPressed = std::bind(&GPlayerCharacter::OnBindButton_JumpMovementKey, this, std::placeholders::_1, std::placeholders::_2);
		ButtonInputDesc.a.fOnReleased = std::bind(&GPlayerCharacter::OnBindButton_JumpMovementKey_Released, this, std::placeholders::_1, std::placeholders::_2);
		ButtonInputDesc.LeftThumbStick.X = std::bind(&GPlayerCharacter::OnBindButton_HorizontalMovementKey, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

		InputController->SetGamePadInputMap(Index == 0 ? eGamepadPlayer::Player1 : eGamepadPlayer::Player2, ButtonInputDesc);
	}
}

//float Time = 0.0f;
void GPlayerCharacter::OnFixedUpdate(const double DeltaTime)
{
	if (IsDead())
		return;
	//Time += DeltaTime;
	auto Body = pBoxCollision2DComponent->GetRigidBody();
	auto Velocity = Body->GetLinearVelocity();

	if (!bIsOnGround)
	{
		bIsOnGround = IsOnGround(pBoxCollision2DComponent->GetBounds());
		if (bIsOnGround)
		{
			if (Velocity.Y != 0.0f)
			{
				bIsOnGround = false;
				Body->SetLinearVelocity(FVector(0.0f, Velocity.Y, 0.0f));
			}
			if (bIsOnGround)
			{
				OnGrounded();
				bIsOnGround = true;
				bIsJumping = false;
			}
		}
	}
	else
	{
		bIsOnGround = IsOnGround(pBoxCollision2DComponent->GetBounds());

		if (Velocity.Y != 0.0f)
		{
			bIsOnGround = false;
			Body->SetLinearVelocity(FVector(Velocity.X, Velocity.Y, 0.0f));
		}
	}

	if (bIsAI)
		return;

	if (bIsOnGround)
	{
		auto Velocity = Body->GetLinearVelocity();
		if (SpaceBTN)
		{
			if (CheckTag(pBoxCollision2DComponent->GetBounds(), eTagDirectionType::eDown, "RockHead"))
			{
				Body->SetLinearVelocity(FVector(Velocity.X / 2.0f, JumpForce * 2.0f, 0.0f));
			}
			else
			{
				Body->SetLinearVelocity(FVector(Velocity.X / 2.0f, JumpForce, 0.0f));
			}
			bIsOnGround = false;
			bIsJumping = true;
		}
		else
		{
			if (Velocity.Y != 0.0f)
			{
				Body->SetLinearVelocity(FVector(0.0f, Velocity.Y, 0.0f));
			}
			else if (Horizontal_Move_Direction == 0 && Velocity.X != 0.0f)
			{
				Body->SetLinearVelocity(FVector(0.0f, Velocity.Y, 0.0f));
				//WriteToConsole("Move : " + std::to_string(Time));
			}
			else if (Horizontal_Move_Direction != 0)
			{
				Body->SetLinearVelocity(FVector(Horizontal_Move_Direction * Move_Speed, Velocity.Y, 0.0f));
				//WriteToConsole("Move : " + std::to_string(Time));
			}
		}
	}
	else if (bIsJumping)
	{
		if (Velocity.Y < 0.0f)
		{
			OnJumping();
		}
		else if (Velocity.Y > 0.0f)
		{
			OnFalling();
		}

		if (CheckTag(pBoxCollision2DComponent->GetBounds(), eTagDirectionType::eUp, "Block"))
		{
			Velocity.Y = 2.0f;
			Body->SetLinearVelocity(FVector(0.0f, Velocity.Y, 0.0f));
		}
		else if (Horizontal_Move_Direction >= 1)
		{
			bool bIsBlocked = CheckTag(pBoxCollision2DComponent->GetBounds(), eTagDirectionType::eRight, "Block");
			if (!bIsBlocked && !bIsOnGround)
			{
				bIsBlocked = CheckTag(pBoxCollision2DComponent->GetBounds(), eTagDirectionType::eRight, "Jumpable_Ground");
			}
			Body->SetLinearVelocity(FVector(bIsBlocked ? 0.0f : Horizontal_Move_Direction * Move_Speed / 2, Velocity.Y, 0.0f));
		}
		else if (Horizontal_Move_Direction <= -1)
		{
			bool bIsBlocked = CheckTag(pBoxCollision2DComponent->GetBounds(), eTagDirectionType::eLeft, "Block");
			if (!bIsBlocked && !bIsOnGround)
			{
				bIsBlocked = CheckTag(pBoxCollision2DComponent->GetBounds(), eTagDirectionType::eLeft, "Jumpable_Ground");
			}
			Body->SetLinearVelocity(FVector(bIsBlocked ? 0.0f : Horizontal_Move_Direction * Move_Speed / 2, Velocity.Y, 0.0f));
		}
	}
	else if (bIsOnGround && !bIsJumping)
	{
		Body->SetLinearVelocity(FVector(0.0f, 0.0f, 0.0f));
	}
	else if (Velocity.Y > 0.0f)
	{
		OnFalling();
		Body->SetLinearVelocity(FVector(Velocity.X / 2.0f, Velocity.Y, 0.0f));
	}
	else if (!bIsOnGround && Velocity.Y == 0.0f)
	{
		OnFalling();
		Body->SetLinearVelocity(FVector(0.0f, 1.0f, 0.0f));
	}
}

void GPlayerCharacter::MoveLeft()
{
	if (IsDead())
		return;

	if (!IsOnGround(pBoxCollision2DComponent->GetBounds()))
		return;

	Horizontal_Move_Direction = 1;
	auto Body = pBoxCollision2DComponent->GetRigidBody();
	auto Velocity = Body->GetLinearVelocity();
	Body->SetLinearVelocity(FVector(Horizontal_Move_Direction * Move_Speed, 0.0f, 0.0f));
}

void GPlayerCharacter::MoveRight()
{
	if (IsDead())
		return;

	if (!IsOnGround(pBoxCollision2DComponent->GetBounds()))
		return;

	Horizontal_Move_Direction = -1;
	bIsOnGround = IsOnGround(pBoxCollision2DComponent->GetBounds());
	auto Body = pBoxCollision2DComponent->GetRigidBody();
	auto Velocity = Body->GetLinearVelocity();
	Body->SetLinearVelocity(FVector(Horizontal_Move_Direction * Move_Speed, 0.0f, 0.0f));
}

void GPlayerCharacter::MoveJump()
{
	if (IsDead())
		return;

	if (!IsOnGround(pBoxCollision2DComponent->GetBounds()))
		return;

	Horizontal_Move_Direction = 0;
	bIsOnGround = IsOnGround(pBoxCollision2DComponent->GetBounds());
	auto Body = pBoxCollision2DComponent->GetRigidBody();
	auto Velocity = Body->GetLinearVelocity();
	Body->SetLinearVelocity(FVector(Velocity.X / 2, JumpForce, 0.0f));
	bIsJumping = true;
}

bool GPlayerCharacter::GetIsFalling() const
{
	if (IsDead())
		return false;

	auto Body = pBoxCollision2DComponent->GetRigidBody();
	auto Velocity = Body->GetLinearVelocity();
	if (Velocity.Y > 0)
	{
		return true;
	}
	return false;
}

bool GPlayerCharacter::GetIsOnGround() const
{
	return IsOnGround(pBoxCollision2DComponent->GetBounds());
}

void GPlayerCharacter::Stop()
{
	if (!IsOnGround(pBoxCollision2DComponent->GetBounds()))
		return;

	Horizontal_Move_Direction = 0;
	auto Body = pBoxCollision2DComponent->GetRigidBody();
	Body->SetLinearVelocity(FVector(0.0f, 0.0f, 0.0f));
}

void GPlayerCharacter::OnGrounded()
{

}

void GPlayerCharacter::OnFalling()
{

}

void GPlayerCharacter::OnJumping()
{
}

float GPlayerCharacter::GetDensity() const
{
	if (Physics::GetActivePhysicsEngineType() == EPhysicsEngine::eBox2D)
	{
		auto Body = pBoxCollision2DComponent->GetRigidBody();
		return static_cast<sBox2DRigidBody*>(Body)->GetDensity();
	}
	return 0.0f;
}

void GPlayerCharacter::SetDensity(float D)
{
	if (Physics::GetActivePhysicsEngineType() == EPhysicsEngine::eBox2D)
	{
		auto Body = pBoxCollision2DComponent->GetRigidBody();
		static_cast<sBox2DRigidBody*>(Body)->SetDensity(D);
	}
}

void GPlayerCharacter::ReceiveItem(GItem* Item)
{
	if (Item->GetName() == "Apple")
	{
		AppleCount++;
	}
	else if (Item->GetName() == "Cherries")
	{
		CherrieCount++;
	}
}

void GPlayerCharacter::Reset()
{
	auto Body = pBoxCollision2DComponent->GetRigidBody();
	Body->SetLinearVelocity(FVector(0.0f, 0.0f, 0.0f));

	AppleCount = 0;
	CherrieCount = 0;

	SpaceBTN = false;
	bIsJumping = false;
	bIsOnGround = true;
	K_Pressed = false;

	Horizontal_Move_Direction = 0;

	Health = 100.0f;

	pBoxCollision2DComponent->SetCollisionEnabled(true);
	AnimManager->SetAnimationState(EAnimationState::eIdle);
}

void GPlayerCharacter::ApplyDamage(float Damage, bool HitReaction)
{
	if (AnimManager->GetAnimationState() == EAnimationState::eHit)
		return;

	if (IsDead())
		return;

	Health -= Damage;
	AnimManager->SetAnimationState(EAnimationState::eHit);
	if (Health <= 0.0f)
	{
		pBoxCollision2DComponent->SetCollisionEnabled(false);
		AnimManager->SetAnimationState(EAnimationState::eDead);
	}
	else if (HitReaction)
	{
		auto Body = pBoxCollision2DComponent->GetRigidBody();
		auto Velocity = Body->GetLinearVelocity();
		Body->SetLinearVelocity(FVector(15.0f * (Horizontal_Move_Direction == 0 ? Horizontal_Move_Direction * -1 : 1.0f), -5.0f, 0.0f));
		SpaceBTN = false;
		bIsJumping = true;
		Horizontal_Move_Direction = 0;
	}
}

void GPlayerCharacter::OnCharacterDead()
{
	SetEnabled(false);
	if (HasController())
	{
		if (auto Controller = dynamic_cast<GPlayerController*>(GetController()))
		{
			Controller->OnCharacterDead();
		}
		else
		{
			GetOwnedLevel()->RemoveActor(this, GetLayerIndex());
		}
	}
}

void GPlayerCharacter::OnBindKey_RightMovementKey(int key)
{
	if (IsDead())
		return;

	//if (AnimManager->GetAnimationState() == EAnimationState::eHit)
	//	return;

	Horizontal_Move_Direction = 1;
}

void GPlayerCharacter::OnBindKey_RightMovementKey_Released(int key)
{
	if (IsDead())
		return;

	//if (AnimManager->GetAnimationState() == EAnimationState::eHit)
	//	return;

	Horizontal_Move_Direction = Horizontal_Move_Direction == 1 ? 0 : Horizontal_Move_Direction;
}

void GPlayerCharacter::OnBindKey_LeftMovementKey(int key)
{
	if (IsDead())
		return;

	//if (AnimManager->GetAnimationState() == EAnimationState::eHit)
	//	return;

	Horizontal_Move_Direction = -1;
}

void GPlayerCharacter::OnBindKey_LeftMovementKey_Released(int key)
{
	if (IsDead())
		return;

	//if (AnimManager->GetAnimationState() == EAnimationState::eHit)
	//	return;

	Horizontal_Move_Direction = Horizontal_Move_Direction == -1 ? 0 : Horizontal_Move_Direction;
}

void GPlayerCharacter::OnBindKey_JumpMovementKey(int key)
{
	if (IsDead())
		return;

	if (AnimManager->GetAnimationState() == EAnimationState::eHit)
		return;

	if (!SpaceBTN && !bIsJumping)
		SpaceBTN = true;
}

void GPlayerCharacter::OnBindKey_JumpMovementKey_Released(int key)
{
	if (IsDead())
		return;

	if (AnimManager->GetAnimationState() == EAnimationState::eHit)
		return;

	SpaceBTN = false;
}

void GPlayerCharacter::OnBindButton_HorizontalMovementKey(float value, eGamepadButtons Button, eGamepadPlayer Player)
{
	if (IsDead())
		return;

	//if (AnimManager->GetAnimationState() == EAnimationState::eHit)
	//	return;

	Horizontal_Move_Direction = (short)value;
}

void GPlayerCharacter::OnBindButton_JumpMovementKey(eGamepadButtons Button, eGamepadPlayer Player)
{
	if (IsDead())
		return;

	if (AnimManager->GetAnimationState() == EAnimationState::eHit)
		return;

	if (!SpaceBTN && !bIsJumping)
		SpaceBTN = true;
}

void GPlayerCharacter::OnBindButton_JumpMovementKey_Released(eGamepadButtons Button, eGamepadPlayer Player)
{
	if (IsDead())
		return;

	if (AnimManager->GetAnimationState() == EAnimationState::eHit)
		return;

	SpaceBTN = false;
}

void GPlayerCharacter::InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar)
{
	if (IsDead())
		return;

	Super::InputProcess(MouseInput, KeyboardChar);
}
