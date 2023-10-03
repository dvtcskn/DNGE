#pragma once

#include "PrimitiveComponent.h"
#include "Engine/IRigidBody.h"
#include "Engine/AbstractEngine.h"
#include "CameraManager.h"

class sCameraComponent : public sPrimitiveComponent
{
	sClassBody(sClassConstructor, sCameraComponent, sPrimitiveComponent)
public:
	sCameraComponent(std::string InName, sActor* pActor = nullptr);
	virtual ~sCameraComponent();

private:
	virtual void OnBeginPlay() override;
	virtual void OnTick(const double DeltaTime) override;
	virtual void OnFixedUpdate(const double DeltaTime) override;

public:
	void Possess(sCameraManager* Manager);
	void UnPossess();

	void SetViewport(std::optional<sViewport> Viewport);
	void AttachCanvas(ICanvas* Canvas);

	void SetPerspective(float AspectRatio);
	void SetOrthographic(std::size_t Width, std::size_t Height);

public:
	FORCEINLINE sViewportInstance::SharedPtr GetViewportInstance() const { return ViewportInstance; }

	FORCEINLINE sCamera::SharedPtr GetCamera() const { return Camera; }

	FORCEINLINE FVector GetLocation() const { return Camera->GetPosition(); }
	FORCEINLINE FVector GetRotation() const { return Camera->GetRotation(); }
	FORCEINLINE FVector GetFocus() const { return Camera->GetFocus(); }

	FORCEINLINE bool IsOrthographic() const { return Camera->IsOrthographic(); }

	FORCEINLINE float GetFOV() const { return Camera->GetFOV(); }
	FORCEINLINE void SetFOV(float NewFOV) { Camera->SetFOV(NewFOV); }
	FORCEINLINE float GetAspectRatio() const { return Camera->GetAspectRatio(); }
	FORCEINLINE void SetAspectRatio(float AspectRatio) { Camera->SetAspectRatio(AspectRatio); }
	FORCEINLINE float GetNearClip() const { return Camera->GetNearClip(); }
	FORCEINLINE float GetFarClip() const { return Camera->GetFarClip(); }
	FORCEINLINE void SetZRange(float nearZ, float farZ) { Camera->SetZRange(nearZ, farZ); }

	FORCEINLINE bool IsZReversed() const { return Camera->IsZReversed(); }
	FORCEINLINE void SetReverseZ(bool Value) { Camera->SetReverseZ(Value); }
	FORCEINLINE float GetClearDepth() const { return Camera->GetClearDepth(); }

	FORCEINLINE bool IsZInfinite() const { return Camera->IsZInfinite(); }
	FORCEINLINE void SetInfiniteZ(bool Value) { Camera->SetInfiniteZ(Value); }

	FORCEINLINE FVector GetPosition() const { return Camera->GetPosition(); }
	FORCEINLINE FMatrix GetViewMatrix() const { return Camera->GetViewMatrix(); }
	FORCEINLINE FMatrix GetInverseViewMatrix() const { return Camera->GetInverseViewMatrix(); }
	FORCEINLINE FMatrix GetProjMatrix() const { return Camera->GetProjMatrix(); }
	FORCEINLINE FMatrix GetViewProjMatrix() const { return Camera->GetViewProjMatrix(); }
	FORCEINLINE FMatrix GetReprojectionMatrix() const { return Camera->GetReprojectionMatrix(); }

	FORCEINLINE void SetProjMatrix(const FMatrix& ProjMat) { Camera->SetProjMatrix(ProjMat); }
	FORCEINLINE void SetViewMatrix(const FMatrix& InView) { Camera->SetViewMatrix(InView); }

private:
	sCameraManager* OwnedManager;
	sCamera::SharedPtr Camera;
	sViewportInstance::SharedPtr ViewportInstance;
};
