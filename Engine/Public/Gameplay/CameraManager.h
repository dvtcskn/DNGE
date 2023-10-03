#pragma once

#include <DirectXMath.h>
#include "Actor.h"
#include "Engine/ClassBody.h"
#include "Utilities/Input.h"
#include "Core/Camera.h"

class sCameraComponent;

class sPlayerController;
class sCameraManager
{
	sBaseClassBody(sClassConstructor, sCameraManager)
public:
	sCameraManager(sPlayerController* InOwner);

	virtual ~sCameraManager();

	virtual void BeginPlay();
	virtual void Tick(const double DeltaTime);
	virtual void FixedUpdate(const double DeltaTime);

	virtual void UpdateCamera(float DeltaTime);

	void EnableCamera();
	void DisableCamera();
	void SplitViewport();

	sPlayerController* GetPlayerController() const;

	void AttachToActor(sActor* InActor);
	void DetachFromActor();
	void DetachActorCamera();

	void SetFOV(float NewFOV);
	void SetAspectRatio(float AspectRatio);

	void SetPerspective(float AspectRatio);
	void SetOrthographic(std::size_t Width, std::size_t Height);

	void SetWorldScale(double sceneScaling, bool zAxisUp = false);
	void SetRollPitchYaw(const float Roll, const float InPitch, const float InYaw);

	void SetMousePosition(FVector2 LastMousePosition);

	void SetSpeed(const float InSpeed);
	void EnableVelocity();
	void DisableVelocity();

	FVector2 ConvertScreenToWorld(const FVector2& ps) const;
	FVector2 ConvertWorldToScreen(const FVector2& pw) const;

	//const sCameraSceneBuffer* GetCameraSceneBuffer() const { return CameraSceneBuffer; }

	bool AddCanvasToViewport(ICanvas* Canvas);
	bool RemoveCanvasFromViewport(ICanvas* Canvas);
	bool RemoveCanvasFromViewport(std::size_t Index);
	ICanvas* GetCanvas(std::size_t Index);

	FORCEINLINE sViewportInstance* GetViewportInstance() const { return ViewportInstance.get(); }
	void SetViewport(std::optional<sViewport> Viewport);

	void WindowResized(const std::size_t Width, const std::size_t Height);

public:
	FORCEINLINE sCamera::SharedPtr GetCamera() const { return pCamera; }
	FORCEINLINE bool IsCameraEnabled() const { return bIsCameraEnabled; }

	FORCEINLINE bool IsInputMovementEnabled() { return bIsInputMovementEnabled; }
	FORCEINLINE void EnableInputMovement() { bIsInputMovementEnabled = true; }
	FORCEINLINE void DisableInputMovement() { bIsInputMovementEnabled = false; }

	FORCEINLINE FVector GetLocation() const { return pCamera->GetPosition(); }
	FORCEINLINE FVector GetRotation() const { return pCamera->GetRotation(); }
	FORCEINLINE FVector GetFocus() const { return pCamera->GetFocus(); }

	FORCEINLINE bool IsOrthographic() const { return pCamera->IsOrthographic(); }

	FORCEINLINE float GetFOV() const { return pCamera->GetFOV(); }
	FORCEINLINE float GetAspectRatio() const { return pCamera->GetAspectRatio(); }
	FORCEINLINE float GetNearClip() const { return pCamera->GetNearClip(); }
	FORCEINLINE float GetFarClip() const { return pCamera->GetFarClip(); }
	FORCEINLINE void SetZRange(float nearZ, float farZ) { pCamera->SetZRange(nearZ, farZ); }

	FORCEINLINE bool IsZReversed() const { return pCamera->IsZReversed(); }
	FORCEINLINE void SetReverseZ(bool Value) { pCamera->SetReverseZ(Value); }
	FORCEINLINE float GetClearDepth() const { return pCamera->GetClearDepth(); }

	FORCEINLINE bool IsZInfinite() const { return pCamera->IsZInfinite(); }
	FORCEINLINE void SetInfiniteZ(bool Value) { pCamera->SetInfiniteZ(Value); }

	FORCEINLINE FVector GetPosition() const { return pCamera->GetPosition(); }
	FORCEINLINE FMatrix GetViewMatrix() const { return pCamera->GetViewMatrix(); }
	FORCEINLINE FMatrix GetInverseViewMatrix() const { return pCamera->GetInverseViewMatrix(); }
	FORCEINLINE FMatrix GetProjMatrix() const { return pCamera->GetProjMatrix(); }
	FORCEINLINE FMatrix GetViewProjMatrix() const { return pCamera->GetViewProjMatrix(); }
	FORCEINLINE FMatrix GetReprojectionMatrix() const { return pCamera->GetReprojectionMatrix(); }

	FORCEINLINE void SetProjMatrix(const FMatrix& ProjMat) { pCamera->SetProjMatrix(ProjMat); }
	FORCEINLINE void SetViewMatrix(const FMatrix& InView) { pCamera->SetViewMatrix(InView); }

	FORCEINLINE float GetPitch() const { return CameraPitchAngle; };
	FORCEINLINE float GetYaw() const { return CameraYawAngle; };

	FORCEINLINE void SetCameraFocused(bool Val) { bCameraFocused = Val; }
	FORCEINLINE bool GetCameraFocused() const { return bCameraFocused; }

public:
	virtual void InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar);

	FORCEINLINE void MoveUp() {	CameraMovement.Z = IsOrthographic() ? -1.0f : 1.0f; }
	FORCEINLINE void MoveUp_RELEASE() { CameraMovement.Z = 0.0f; }
	FORCEINLINE void MoveDown() { CameraMovement.Z = IsOrthographic() ? 1.0f : -1.0f; }
	FORCEINLINE void MoveDown_RELEASE() { CameraMovement.Z = 0.0f; }
	FORCEINLINE void MoveForward() { CameraMovement.X = -1.0f; }
	FORCEINLINE void MoveForward_RELEASE() { CameraMovement.X = CameraMovement.X == 1.0f ? 1.0f : 0.0f; }
	FORCEINLINE void MoveBackward() { CameraMovement.X = 1.0f; }
	FORCEINLINE void MoveBackward_RELEASE() { CameraMovement.X = CameraMovement.X == -1.0f ? -1.0f : 0.0f; }
	FORCEINLINE void MoveLeft() { CameraMovement.Y = -1.0f; }
	FORCEINLINE void MoveLeft_RELEASE() { CameraMovement.Y = CameraMovement.Y == 1.0f ? 1.0f : 0.0f; }
	FORCEINLINE void MoveRight() { CameraMovement.Y = 1.0f; }
	FORCEINLINE void MoveRight_RELEASE() { CameraMovement.Y = CameraMovement.Y == -1.0f ? -1.0f : 0.0f; }

private:
	void UpdateVelocity(float fElapsedTime);

private:
	sPlayerController* PCOwner;
	sCamera::SharedPtr pCamera;
	sActor* AttachedActor;
	sCameraComponent* PossessedCameraComponent;
	sViewportInstance::SharedPtr ViewportInstance;

	FVector2 LastMousePosition;
	//float MouseSensitivityX;
	//float MouseSensitivityY;
	FVector MouseDelta;
	float DragTimer;

	float CameraPitchAngle;
	float CameraYawAngle;

	FVector Velocity;
	FVector Direction;
	FVector VelocityDrag;
	FVector CameraMovement;
	FVector RotVelocity;

	float Speed;
	float CameraMovementScaling;

	bool bEnableCameraVelocity;
	bool bCameraFocused;
	bool bIsCameraEnabled;

	bool bIsInputMovementEnabled;
};
