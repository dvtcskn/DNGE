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
#include "GPlayerCharacter.h"
#include <Engine/IRigidBody.h>
#include <Core/MeshPrimitives.h>
#include <Gameplay/MeshComponent.h>
#include <Gameplay/CameraComponent.h>
#include <Gameplay/BoxCollision2DComponent.h>
#include <Gameplay/CircleCollision2DComponent.h>
#include <AbstractGI/MaterialManager.h>
#include "MetaWorld.h"
#include "SpriteEffectComponent.h"
#include "SpriteAnimationManager.h"
#include "GPlayerController.h"
#include "GAIController.h"

#include "AssetManager.h"
#include <Engine/Box2DRigidBody.h>
//#include <Engine/BulletRigidBody.h>

#include "HelperFunctions.h"

GNetInputManager::GNetInputManager()
	: CurrentInput(nullptr)
{
}

GNetInputManager::~GNetInputManager()
{
	Reset();
}

void GNetInputManager::Reset()
{
	for (auto& Input : Inputs)
		delete Input;
	Inputs.clear();
	CurrentInput = nullptr;
}

void GNetInputManager::Tick(const double DeltaTime)
{
	if (!CurrentInput && Inputs.size() > 0)
	{
		CurrentInput = Inputs.front();
		//Engine::WriteToConsole("Inputs front");
	}

	if (CurrentInput)
	{
		if (CurrentInput->EndFrameCount.has_value())
		{
			if (CurrentInput->FrameCounter >= CurrentInput->EndFrameCount)
			{
				//Engine::WriteToConsole("CurrentInput erased");
				Inputs.erase(std::find(Inputs.begin(), Inputs.end(), CurrentInput));
				delete CurrentInput;
				CurrentInput = nullptr;

				if (Inputs.size() > 0)
					CurrentInput = Inputs.front();
			}
		}
	}

	if (CurrentInput)
	{
		CurrentInput->FrameCounter++;
	}
}

void GNetInputManager::PushBackInput(GNetInputManager::eInput Type, std::uint32_t key, FVector2 InputModifier)
{
	Inputs.push_back(new GNetInputManager::GNetInput(Type, key, InputModifier));
	//Engine::WriteToConsole("PushBackInput");
	if (!CurrentInput && Inputs.size() > 0)
	{
		CurrentInput = Inputs.front();
		//Engine::WriteToConsole("PushBackInput::Inputs front");
	}
}

void GNetInputManager::SetFrameCountForCurrentOrNextInput(GNetInputManager::eInput Type, std::uint32_t FrameCount)
{
	if (CurrentInput)
	{
		if (CurrentInput->Type == Type && !CurrentInput->EndFrameCount.has_value())
		{
			CurrentInput->EndFrameCount = FrameCount;
			//Engine::WriteToConsole("CurrentInput->EndFrameCount SET");
		}
		else
		{
			for (const auto& Input : Inputs)
			{
				if (Input->Type == Type)
				{
					//Engine::WriteToConsole("NextInput->EndFrameCount SET");
					Input->EndFrameCount = FrameCount;
					break;
				}
			}
		}
	}
}

GPlayerCharacter::GPlayerCharacter(std::string InName)
	: Super(InName)
	, HorizontalMoveDirection(0)
	, Move_Speed(6.0f)
	, JumpForce(-8.0f)
	, Health(100)
	, AppleCount(0)
	, CherrieCount(0)
	, bIsAI(false)
	, FrameCounter(0)
{
	AddTag("PlayerCharacter");
	{
		sRigidBodyDesc Desc;
		Desc.Friction = 1.0f;
		Desc.Mass = 1.0f;
		Desc.Restitution = 0.0f;
		Desc.RigidBodyType = ERigidBodyType::Dynamic;

		sBoxCollision2DComponent::SharedPtr BoxCollision2DComponent = sNet_BoxCollision2DComponent::Create("BoxActorCollision", Desc, FDimension2D(14.0f, 20.0f));
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
	pBoxCollision2DComponent->SetAngularDamping(100.0f);
}

GPlayerCharacter::~GPlayerCharacter()
{
	pBoxCollision2DComponent = nullptr;
	AnimManager = nullptr;
}

void GPlayerCharacter::Replicate(bool bReplicate)
{
	if (IsReplicated() == bReplicate)
		return;

	Super::Replicate(bReplicate);

	if (IsReplicated())
	{
		RegisterRPCfn(GetClassNetworkAddress(), GetName(), "OnBindKey_JumpMovementKey", eRPCType::Server, true, true, std::bind(&GPlayerCharacter::Net_OnBindKey_JumpMovementKey, this, std::placeholders::_1, std::placeholders::_2), sDateTime, int);
		RegisterRPCfn(GetClassNetworkAddress(), GetName(), "OnBindKey_RightMovementKey", eRPCType::Server, true, true, std::bind(&GPlayerCharacter::Net_OnBindKey_RightMovementKey, this, std::placeholders::_1, std::placeholders::_2), sDateTime, int);
		RegisterRPCfn(GetClassNetworkAddress(), GetName(), "OnBindKey_LeftMovementKey", eRPCType::Server, true, true, std::bind(&GPlayerCharacter::Net_OnBindKey_LeftMovementKey, this, std::placeholders::_1, std::placeholders::_2), sDateTime, int);

		RegisterRPCfn(GetClassNetworkAddress(), GetName(), "OnBindKey_RightMovementKey_Released", eRPCType::Server, true, true, std::bind(&GPlayerCharacter::Net_OnBindKey_RightMovementKey_Released, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), sDateTime, int, std::uint32_t);
		RegisterRPCfn(GetClassNetworkAddress(), GetName(), "OnBindKey_LeftMovementKey_Released", eRPCType::Server, true, true, std::bind(&GPlayerCharacter::Net_OnBindKey_LeftMovementKey_Released, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), sDateTime, int, std::uint32_t);
		RegisterRPCfn(GetClassNetworkAddress(), GetName(), "OnBindKey_JumpMovementKey_Released", eRPCType::Server, true, true, std::bind(&GPlayerCharacter::Net_OnBindKey_JumpMovementKey_Released, this, std::placeholders::_1, std::placeholders::_2), sDateTime, int);

		RegisterRPCfn(GetClassNetworkAddress(), GetName(), "ReceiveItem_Client", eRPCType::Client, true, false, std::bind(&GPlayerCharacter::ReceiveItem_Client, this, std::placeholders::_1, std::placeholders::_2), std::string, std::uint32_t);
		RegisterRPCfn(GetClassNetworkAddress(), GetName(), "ApplyDamage_Client", eRPCType::Client, true, false, std::bind(&GPlayerCharacter::ApplyDamage_Client, this, std::placeholders::_1, std::placeholders::_2), float, bool);

		RegisterRPCfn(GetClassNetworkAddress(), GetName(), "Net_CheckIfRightMovementKeyPressed_Client", eRPCType::Client, true, false, std::bind(&GPlayerCharacter::Net_CheckIfRightMovementKeyPressed_Client, this));
		RegisterRPCfn(GetClassNetworkAddress(), GetName(), "Net_CheckIfLeftMovementKeyPressed_Client", eRPCType::Client, true, false, std::bind(&GPlayerCharacter::Net_CheckIfLeftMovementKeyPressed_Client, this));
		RegisterRPCfn(GetClassNetworkAddress(), GetName(), "Net_CheckIfRightMovementKeyPressed_Server", eRPCType::Server, true, false, std::bind(&GPlayerCharacter::Net_CheckIfRightMovementKeyPressed_Server, this, std::placeholders::_1), bool);
		RegisterRPCfn(GetClassNetworkAddress(), GetName(), "Net_CheckIfLeftMovementKeyPressed_Server", eRPCType::Server, true, false, std::bind(&GPlayerCharacter::Net_CheckIfLeftMovementKeyPressed_Server, this, std::placeholders::_1), bool);
	}
	else
	{
		Network::UnregisterRPC(GetClassNetworkAddress(), GetName());
	}
}

void GPlayerCharacter::OnBeginPlay()
{
	Replicate(true);
	pBoxCollision2DComponent->Replicate(true);

	if (GetNetworkRole() == eNetworkRole::SimulatedProxy || GetNetworkRole() == eNetworkRole::NetProxy)
		return;

	Engine::WriteToConsole("OnBeginPlay");

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
		//KMButtonInputDesc.bHoldable = true;
		KMButtonInputDesc.Key = Index == 0 ? 'D' : 'L';
		KMButtonInputDesc.fOnPressed = std::bind(&GPlayerCharacter::OnBindKey_RightMovementKey, this, std::placeholders::_1);
		KMButtonInputDesc.fOnReleased = std::bind(&GPlayerCharacter::OnBindKey_RightMovementKey_Released, this, std::placeholders::_1);
		InputController->BindInput(Index == 0 ? "RightMovementKey" : "RightMovementKey2", KMButtonInputDesc);
	}
	{
		sKMButtonInputDesc KMButtonInputDesc;
		//KMButtonInputDesc.bHoldable = true;
		KMButtonInputDesc.Key = Index == 0 ? 'A' : 'J';
		KMButtonInputDesc.fOnPressed = std::bind(&GPlayerCharacter::OnBindKey_LeftMovementKey, this, std::placeholders::_1);
		KMButtonInputDesc.fOnReleased = std::bind(&GPlayerCharacter::OnBindKey_LeftMovementKey_Released, this, std::placeholders::_1);
		InputController->BindInput(Index == 0 ? "LeftMovementKey" : "LeftMovementKey2", KMButtonInputDesc);
	}
	{
		sKMButtonInputDesc KMButtonInputDesc;
		//KMButtonInputDesc.bHoldable = true;
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

void GPlayerCharacter::OnFixedUpdate(const double DeltaTime)
{
	/*if (GetNetworkRole() == eNetworkRole::SimulatedProxy || GetNetworkRole() == eNetworkRole::NetProxy)
	{
		return;
	}*/

	//if (Network::IsConnected() && !Network::IsHost())
	//	return;

	if (IsDead())
		return;

	InputManager.Tick(DeltaTime);

	auto Velocity = pBoxCollision2DComponent->GetVelocity();

	short Horizontal_Move_Direction = GetHorizontalMoveDirection();

	if (Horizontal_Move_Direction != 0)
		FrameCounter++;

	if (GetNetworkRole() == eNetworkRole::SimulatedProxy && (FrameCounter % 120) == 0)
	{
		// Error Correct
		if (Horizontal_Move_Direction == 1)
		{
			Net_CheckIfRightMovementKeyPressed();
		}
		else if (Horizontal_Move_Direction == -1)
		{
			Net_CheckIfLeftMovementKeyPressed();
		}
	}

	if (!bIsOnGround)
	{
		bIsOnGround = IsOnGround(pBoxCollision2DComponent->GetBounds());
		if (bIsOnGround)
		{
			if (Velocity.Y != 0.0f)
			{
				bIsOnGround = false;
				pBoxCollision2DComponent->SetLinearVelocity(FVector(0.0f, Velocity.Y, 0.0f));
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
			pBoxCollision2DComponent->SetLinearVelocity(FVector(Velocity.X, Velocity.Y, 0.0f));
		}
	}

	if (bIsAI)
		return;

	if (bIsOnGround)
	{
		auto Velocity = pBoxCollision2DComponent->GetVelocity();
		if (SpaceBTN)
		{
			if (CheckTag(pBoxCollision2DComponent->GetBounds(), eTagDirectionType::eDown, "RockHead"))
			{
				pBoxCollision2DComponent->SetLinearVelocity(FVector(Velocity.X / 2.0f, JumpForce * 2.0f, 0.0f));
			}
			else
			{
				pBoxCollision2DComponent->SetLinearVelocity(FVector(Velocity.X / 2.0f, JumpForce, 0.0f));
			}
			bIsOnGround = false;
			bIsJumping = true;
		}
		else
		{
			if (Velocity.Y != 0.0f)
			{
				pBoxCollision2DComponent->SetLinearVelocity(FVector(0.0f, Velocity.Y, 0.0f));
			}
			else if (Horizontal_Move_Direction == 0 && Velocity.X != 0.0f)
			{
				pBoxCollision2DComponent->SetLinearVelocity(FVector(0.0f, Velocity.Y, 0.0f));
				//WriteToConsole("Move : " + std::to_string(Time));
			}
			else if (Horizontal_Move_Direction != 0)
			{
				pBoxCollision2DComponent->SetLinearVelocity(FVector(Horizontal_Move_Direction * Move_Speed, Velocity.Y, 0.0f));
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
			pBoxCollision2DComponent->SetLinearVelocity(FVector(0.0f, Velocity.Y, 0.0f));
		}
		else if (Horizontal_Move_Direction >= 1)
		{
			bool bIsBlocked = CheckTag(pBoxCollision2DComponent->GetBounds(), eTagDirectionType::eRight, "Block");
			if (!bIsBlocked && !bIsOnGround)
			{
				bIsBlocked = CheckTag(pBoxCollision2DComponent->GetBounds(), eTagDirectionType::eRight, "Jumpable_Ground");
			}
			pBoxCollision2DComponent->SetLinearVelocity(FVector(bIsBlocked ? 0.0f : Horizontal_Move_Direction * Move_Speed / 2, Velocity.Y, 0.0f));
		}
		else if (Horizontal_Move_Direction <= -1)
		{
			bool bIsBlocked = CheckTag(pBoxCollision2DComponent->GetBounds(), eTagDirectionType::eLeft, "Block");
			if (!bIsBlocked && !bIsOnGround)
			{
				bIsBlocked = CheckTag(pBoxCollision2DComponent->GetBounds(), eTagDirectionType::eLeft, "Jumpable_Ground");
			}
			pBoxCollision2DComponent->SetLinearVelocity(FVector(bIsBlocked ? 0.0f : Horizontal_Move_Direction * Move_Speed / 2, Velocity.Y, 0.0f));
		}
	}
	else if (bIsOnGround && !bIsJumping)
	{
		pBoxCollision2DComponent->SetLinearVelocity(FVector(0.0f, 0.0f, 0.0f));
	}
	else if (Velocity.Y > 0.0f)
	{
		OnFalling();
		pBoxCollision2DComponent->SetLinearVelocity(FVector(Velocity.X / 2.0f, Velocity.Y, 0.0f));
	}
	else if (!bIsOnGround && Velocity.Y == 0.0f)
	{
		OnFalling();
		pBoxCollision2DComponent->SetLinearVelocity(FVector(0.0f, 1.0f, 0.0f));
	}
}

void GPlayerCharacter::MoveLeft()
{
	if (IsDead())
		return;

	if (!IsOnGround(pBoxCollision2DComponent->GetBounds()))
		return;

	HorizontalMoveDirection = 1;
	pBoxCollision2DComponent->SetLinearVelocity(FVector(HorizontalMoveDirection * Move_Speed, 0.0f, 0.0f));
}

void GPlayerCharacter::MoveRight()
{
	if (IsDead())
		return;

	if (!IsOnGround(pBoxCollision2DComponent->GetBounds()))
		return;

	HorizontalMoveDirection = -1;
	bIsOnGround = IsOnGround(pBoxCollision2DComponent->GetBounds());
	pBoxCollision2DComponent->SetLinearVelocity(FVector(HorizontalMoveDirection * Move_Speed, 0.0f, 0.0f));
}

void GPlayerCharacter::MoveJump()
{
	if (IsDead())
		return;

	if (!IsOnGround(pBoxCollision2DComponent->GetBounds()))
		return;

	HorizontalMoveDirection = 0;
	bIsOnGround = IsOnGround(pBoxCollision2DComponent->GetBounds());
	auto Velocity = pBoxCollision2DComponent->GetVelocity();
	pBoxCollision2DComponent->SetLinearVelocity(FVector(Velocity.X / 2, JumpForce, 0.0f));
	bIsJumping = true;
}

bool GPlayerCharacter::GetIsFalling() const
{
	if (IsDead())
		return false;

	auto Velocity = pBoxCollision2DComponent->GetVelocity();
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

	HorizontalMoveDirection = 0;
	pBoxCollision2DComponent->SetLinearVelocity(FVector(0.0f, 0.0f, 0.0f));
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
	if (Network::IsClient())
		return;

	if (Item->GetFruitType() == "Apple")
	{
		AppleCount++;
		Network::CallRPC(GetClassNetworkAddress(), GetName(), "ReceiveItem_Client", sArchive(Item->GetFruitType(), AppleCount));
	}
	else if (Item->GetFruitType() == "Cherries")
	{
		CherrieCount++;
		Network::CallRPC(GetClassNetworkAddress(), GetName(), "ReceiveItem_Client", sArchive(Item->GetFruitType(), CherrieCount));
	}
}

void GPlayerCharacter::ReceiveItem_Client(std::string Item, std::uint32_t Count)
{
	if (Item == "Apple")
	{
		AppleCount = Count;
	}
	else if (Item == "Cherries")
	{
		CherrieCount = Count;
	}
}

void GPlayerCharacter::Reset()
{
	InputManager.Reset();
	pBoxCollision2DComponent->SetCollisionChannel(ECollisionChannel::Default);
	pBoxCollision2DComponent->SetLinearVelocity(FVector(0.0f, 0.0f, 0.0f));

	AppleCount = 0;
	CherrieCount = 0;

	SpaceBTN = false;
	bIsJumping = false;
	bIsOnGround = true;
	K_Pressed = false;

	HorizontalMoveDirection = 0;

	Health = 100.0f;

	pBoxCollision2DComponent->SetCollisionEnabled(true);
	AnimManager->SetAnimationState(EAnimationState::eIdle);
}

void GPlayerCharacter::ApplyDamage(float Damage, bool HitReaction)
{
	if (Network::IsClient())
		return;

	if (AnimManager->GetAnimationState() == EAnimationState::eHit)
		return;

	if (IsDead())
		return;

	Health -= Damage;
	AnimManager->SetAnimationState(EAnimationState::eHit);
	if (Health <= 0.0f)
	{
		InputManager.Reset();
		pBoxCollision2DComponent->SetCollisionEnabled(false);
		AnimManager->SetAnimationState(EAnimationState::eDead);
	}
	else if (HitReaction)
	{
		auto Velocity = pBoxCollision2DComponent->GetVelocity();
		pBoxCollision2DComponent->SetLinearVelocity(FVector(15.0f * (HorizontalMoveDirection == 0 ? HorizontalMoveDirection * -1 : 1.0f), -5.0f, 0.0f));
		SpaceBTN = false;
		bIsJumping = true;
		HorizontalMoveDirection = 0;
	}
	Network::CallRPC(GetClassNetworkAddress(), GetName(), "ApplyDamage_Client", sArchive(Health, HitReaction));
}

void GPlayerCharacter::ApplyDamage_Client(float InHealth, bool HitReaction)
{
	Health = InHealth;
	AnimManager->SetAnimationState(EAnimationState::eHit);
	if (Health <= 0.0f)
	{
		InputManager.Reset();
		pBoxCollision2DComponent->SetCollisionEnabled(false);
		AnimManager->SetAnimationState(EAnimationState::eDead);
	}
	else if (HitReaction)
	{
		auto Velocity = pBoxCollision2DComponent->GetVelocity();
		pBoxCollision2DComponent->SetLinearVelocity(FVector(15.0f * (HorizontalMoveDirection == 0 ? HorizontalMoveDirection * -1 : 1.0f), -5.0f, 0.0f));
		SpaceBTN = false;
		bIsJumping = true;
		HorizontalMoveDirection = 0;
	}
}

void GPlayerCharacter::Net_CheckIfRightMovementKeyPressed()
{
	if (Network::IsHost())
	{
		Network::CallRPC(GetClassNetworkAddress(), GetName(), "Net_CheckIfRightMovementKeyPressed_Client", sArchive());
	}
}

void GPlayerCharacter::Net_CheckIfRightMovementKeyPressed_Client()
{
	if (Network::IsClient() && GetNetworkRole() != eNetworkRole::NetProxy && GetNetworkRole() != eNetworkRole::SimulatedProxy)
	{
		if (HorizontalMoveDirection == 1)
		{
			Network::CallRPC(GetClassNetworkAddress(), GetName(), "Net_CheckIfRightMovementKeyPressed_Server", sArchive(true));
		}
		else
		{
			Network::CallRPC(GetClassNetworkAddress(), GetName(), "Net_CheckIfRightMovementKeyPressed_Server", sArchive(false));
		}
	}
}

void GPlayerCharacter::Net_CheckIfRightMovementKeyPressed_Server(bool val)
{
	if (!val)
	{
		if (InputManager.CurrentInput)
		{
			if (InputManager.CurrentInput->Type == GNetInputManager::eInput::eRight && !InputManager.CurrentInput->EndFrameCount.has_value())
				InputManager.CurrentInput->EndFrameCount = 1;
		}
	}
}

void GPlayerCharacter::Net_CheckIfLeftMovementKeyPressed()
{
	if (Network::IsHost())
	{
		Network::CallRPC(GetClassNetworkAddress(), GetName(), "Net_CheckIfLeftMovementKeyPressed_Client", sArchive());
	}
}

void GPlayerCharacter::Net_CheckIfLeftMovementKeyPressed_Client()
{
	if (Network::IsClient() && GetNetworkRole() != eNetworkRole::NetProxy && GetNetworkRole() != eNetworkRole::SimulatedProxy)
	{
		if (HorizontalMoveDirection == -1)
		{
			Network::CallRPC(GetClassNetworkAddress(), GetName(), "Net_CheckIfLeftMovementKeyPressed_Server", sArchive(true));
		}
		else
		{
			Network::CallRPC(GetClassNetworkAddress(), GetName(), "Net_CheckIfLeftMovementKeyPressed_Server", sArchive(false));
		}
	}
}

void GPlayerCharacter::Net_CheckIfLeftMovementKeyPressed_Server(bool val)
{
	if (!val)
	{
		if (InputManager.CurrentInput)
		{
			if (InputManager.CurrentInput->Type == GNetInputManager::eInput::eLeft && !InputManager.CurrentInput->EndFrameCount.has_value())
				InputManager.CurrentInput->EndFrameCount = 1;
		}
	}
}

void GPlayerCharacter::OnCharacterDead()
{
	SetEnabled(false);
	pBoxCollision2DComponent->SetCollisionChannel(ECollisionChannel::None);
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

std::int32_t GPlayerCharacter::GetHorizontalMoveDirection() const
{
	if (!Network::IsConnected() || GetNetworkRole() == eNetworkRole::Client)
		return HorizontalMoveDirection;
	if (Network::IsHost() && GetNetworkRole() == eNetworkRole::Host)
		return HorizontalMoveDirection;

	if (GetNetworkRole() == eNetworkRole::SimulatedProxy)
		return InputManager.CurrentInput != nullptr ? InputManager.CurrentInput->InputModifier.X : 0;
	return std::uint32_t();
}

void GPlayerCharacter::OnBindKey_RightMovementKey(int key)
{;
	if (GetNetworkRole() == eNetworkRole::SimulatedProxy || GetNetworkRole() == eNetworkRole::NetProxy)
		return;

	if (IsDead())
		return;

	if (HorizontalMoveDirection == -1)
		OnBindKey_LeftMovementKey_Released(0);

	if (Network::IsConnected() && !Network::IsHost()/* && HorizontalMoveDirection == 0*/)
	{
		Network::CallRPC(GetClassNetworkAddress(), GetName(), "OnBindKey_RightMovementKey", sArchive(key));
		//return;
	}

	//if (Network::IsConnected() && !Network::IsHost())
	//	return;

	//if (AnimManager->GetAnimationState() == EAnimationState::eHit)
	//	return;

	HorizontalMoveDirection = 1;
	//Engine::WriteToConsole("OnBindKey_RightMovementKey");
}

void GPlayerCharacter::OnBindKey_RightMovementKey_Released(int key)
{
	if (GetNetworkRole() == eNetworkRole::SimulatedProxy || GetNetworkRole() == eNetworkRole::NetProxy)
		return;

	if (IsDead())
		return;

	if (Network::IsConnected() && !Network::IsHost() /*&& HorizontalMoveDirection == 1*/)
	{
		Network::CallRPC(GetClassNetworkAddress(), GetName(), "OnBindKey_RightMovementKey_Released", sArchive(key, FrameCounter));
		//return;
	}

	FrameCounter = 0;

	//if (AnimManager->GetAnimationState() == EAnimationState::eHit)
	//	return;

	HorizontalMoveDirection = HorizontalMoveDirection == 1 ? 0 : HorizontalMoveDirection;
	//Engine::WriteToConsole("Move_KEy");
}

void GPlayerCharacter::OnBindKey_LeftMovementKey(int key)
{
	if (GetNetworkRole() == eNetworkRole::SimulatedProxy || GetNetworkRole() == eNetworkRole::NetProxy)
		return;

	if (IsDead())
		return;

	if (HorizontalMoveDirection == 1)
		OnBindKey_RightMovementKey_Released(0);

	if (Network::IsConnected() && !Network::IsHost() /*&& HorizontalMoveDirection == 0*/)
	{
		Network::CallRPC(GetClassNetworkAddress(), GetName(), "OnBindKey_LeftMovementKey", sArchive(key));
		//return;
	}

	//if (AnimManager->GetAnimationState() == EAnimationState::eHit)
	//	return;

	HorizontalMoveDirection = -1;
	//Engine::WriteToConsole("Move_KEy");
}

void GPlayerCharacter::OnBindKey_LeftMovementKey_Released(int key)
{
	if (GetNetworkRole() == eNetworkRole::SimulatedProxy || GetNetworkRole() == eNetworkRole::NetProxy)
		return;

	if (IsDead())
		return;

	if (Network::IsConnected() && !Network::IsHost() /*&& HorizontalMoveDirection == -1*/)
	{
		Network::CallRPC(GetClassNetworkAddress(), GetName(), "OnBindKey_LeftMovementKey_Released", sArchive(key, FrameCounter));
		//return;
	}

	FrameCounter = 0;

	//if (AnimManager->GetAnimationState() == EAnimationState::eHit)
	//	return;

	HorizontalMoveDirection = HorizontalMoveDirection == -1 ? 0 : HorizontalMoveDirection;
	//Engine::WriteToConsole("Move_KEy");
}

void GPlayerCharacter::OnBindKey_JumpMovementKey(int key)
{
	if (GetNetworkRole() == eNetworkRole::SimulatedProxy || GetNetworkRole() == eNetworkRole::NetProxy)
		return;

	if (IsDead())
		return;

	if (Network::IsConnected() && !Network::IsHost() && !SpaceBTN)
	{
		Network::CallRPC(GetClassNetworkAddress(), GetName(), "OnBindKey_JumpMovementKey", sArchive(key));
		//return;
	}

	if (AnimManager->GetAnimationState() == EAnimationState::eHit)
		return;

	if (!SpaceBTN && !bIsJumping)
		SpaceBTN = true;
}

void GPlayerCharacter::OnBindKey_JumpMovementKey_Released(int key)
{
	if (GetNetworkRole() == eNetworkRole::SimulatedProxy || GetNetworkRole() == eNetworkRole::NetProxy)
		return;

	if (IsDead())
		return;

	if (Network::IsConnected() && !Network::IsHost())
	{
		Network::CallRPC(GetClassNetworkAddress(), GetName(), "OnBindKey_JumpMovementKey_Released", sArchive(key));
		//return;
	}

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

	HorizontalMoveDirection = (short)value;
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

void GPlayerCharacter::Net_OnBindKey_RightMovementKey(sDateTime Time, int key)
{
	InputManager.PushBackInput(GNetInputManager::eInput::eRight, key, FVector2(1,0));
}

void GPlayerCharacter::Net_OnBindKey_RightMovementKey_Released(sDateTime Time, int key, std::uint32_t inFrameCounter)
{
	InputManager.SetFrameCountForCurrentOrNextInput(GNetInputManager::eInput::eRight, inFrameCounter);
}

void GPlayerCharacter::Net_OnBindKey_LeftMovementKey(sDateTime Time, int key)
{
	InputManager.PushBackInput(GNetInputManager::eInput::eLeft, key, FVector2(-1, 0));
}

void GPlayerCharacter::Net_OnBindKey_LeftMovementKey_Released(sDateTime Time, int key, std::uint32_t inFrameCounter)
{
	InputManager.SetFrameCountForCurrentOrNextInput(GNetInputManager::eInput::eLeft, inFrameCounter);
}

void GPlayerCharacter::Net_OnBindKey_JumpMovementKey(sDateTime Time, int key)
{
	if (!SpaceBTN && !bIsJumping)
	{
		SpaceBTN = true;
		//Engine::WriteToConsole("SpaceBTN == TRUE");
	}
}

void GPlayerCharacter::Net_OnBindKey_JumpMovementKey_Released(sDateTime Time, int key)
{
	SpaceBTN = false;
	//Engine::WriteToConsole("SpaceBTN == FALSE");
}

void GPlayerCharacter::InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar)
{
	if (IsDead())
		return;

	Super::InputProcess(MouseInput, KeyboardChar);
}

