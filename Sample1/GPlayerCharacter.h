#pragma once

#include <Gameplay/Character.h>
#include <Core/MeshPrimitives.h>
#include <Gameplay/MeshComponent.h>
#include <Gameplay/BoxCollision2DComponent.h>
#include <Gameplay/CircleCollision2DComponent.h>
#include "SpriteAnimationManager.h"
#include "SpriteComponent.h"
#include "SpriteSheetComponent.h"
#include <Engine/InputController.h>
#include "Items.h"

class GPlayerCharacter final : public sCharacter
{
	sClassBody(sClassConstructor, GPlayerCharacter, sCharacter)
public:
	GPlayerCharacter(std::string InName = "");

	virtual ~GPlayerCharacter();

	virtual void OnBeginPlay() override final;
	virtual void OnFixedUpdate(const double DeltaTime) override final;

	virtual void InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar) override final;

	float GetJumpForce() const { return JumpForce; }
	void SetJumpForce(float Force) { JumpForce = Force; }

	float GetDensity() const;
	void SetDensity(float D);

	void ReceiveItem(GItem* Item);
	int AppleCount;
	int CherrieCount;

	inline float GetHealth() const { return Health; }

	void Reset();

	void ApplyDamage(float Damage, bool HitReaction = false);

	void MoveLeft();
	void MoveRight();
	void MoveJump();
	void Stop();

	float GetHorizontal_Move_Direction() const { return Horizontal_Move_Direction; }

	bool GetIsFalling() const;
	bool GetIsJumping() const { return bIsJumping; }
	bool GetIsOnGround() const;

//private:
	void OnGrounded();
	void OnJumping();
	void OnFalling();

	void OnBindKey_RightMovementKey(int key);
	void OnBindKey_RightMovementKey_Released(int key);
	void OnBindKey_LeftMovementKey(int key);
	void OnBindKey_LeftMovementKey_Released(int key);
	void OnBindKey_JumpMovementKey(int key);
	void OnBindKey_JumpMovementKey_Released(int key);

	void OnBindButton_HorizontalMovementKey(float value, eGamepadButtons Button, eGamepadPlayer Player);
	void OnBindButton_JumpMovementKey(eGamepadButtons Button, eGamepadPlayer Player);
	void OnBindButton_JumpMovementKey_Released(eGamepadButtons Button, eGamepadPlayer Player);

	bool IsDead() const { return Health <= 0.0f; }

private:
	void OnCharacterDead();

private:
	sBoxCollision2DComponent* pBoxCollision2DComponent;

	//__declspec(align(256)) struct sAnimationConstantBuffer
	//{
	//	//std::uint32_t Index = 0;
	//	std::uint32_t Flip = 0;
	//	//float FlipMaxX = 0;
	//	//float FlipMinX = 0;
	//};

	bool bIsAI;

	bool SpaceBTN = false;
	bool bIsJumping = false;
	bool bIsOnGround = true;
	bool K_Pressed = false;

	short Horizontal_Move_Direction;
	float Move_Speed;

	float JumpForce;

	constexpr bool IsJumping() const { return bIsJumping /*|| AnimaState == AnimationState::Jump*/; }

	float Health;

	sSpriteAnimationManager* AnimManager;
};
