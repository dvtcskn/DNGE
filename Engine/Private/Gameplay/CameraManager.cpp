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
#include "Gameplay/CameraManager.h"
#include "Gameplay/PlayerController.h"
#include "Gameplay/GameInstance.h"
#include "Gameplay/CameraComponent.h"
#include <Windows.h>
#include "Gameplay/ICanvas.h"

sCameraManager::sCameraManager(sPlayerController* InOwner)
	: PCOwner(InOwner)
	, AttachedActor(nullptr)
	, PossessedCameraComponent(nullptr)
	, pCamera(sCamera::Create())
	, RotVelocity(FVector::Zero())
	, CameraMovement(FVector::Zero())
	, Direction(FVector::Zero())
	, VelocityDrag(FVector::Zero())
	, Velocity(FVector::Zero())
	, MouseDelta(FVector::Zero())
	, LastMousePosition(FVector2::Zero())
	, Speed(25)
	, DragTimer(0.0f)
	, CameraMovementScaling(40.25f)
	, bEnableCameraVelocity(false)
	, bCameraFocused(false)
	, CameraPitchAngle(0.0f)
	, CameraYawAngle(0.0f)
	//, MouseSensitivityX(1.0f)
	//, MouseSensitivityY(1.0f)
	, ViewportInstance(sViewportInstance::Create())
	, bIsCameraEnabled(true)
	, bIsInputMovementEnabled(true)
{
	ViewportInstance->pCamera = pCamera;
	//GPU::AddViewportInstance(ViewportInstance.get());
}

sCameraManager::~sCameraManager()
{
	ViewportInstance = nullptr;
	AttachedActor = nullptr;
	PossessedCameraComponent = nullptr;
	PCOwner = nullptr;
	DetachFromActor();
	pCamera = nullptr;
}

void sCameraManager::BeginPlay()
{
	if (ViewportInstance)
	{
		for (ICanvas* Canvas : ViewportInstance->Canvases)
		{
			Canvas->BeginPlay();
		}
	}
}

void sCameraManager::Tick(const double DeltaTime)
{
	UpdateCamera(static_cast<float>(DeltaTime));

	/*if (ViewportInstance)
	{
		for (ICanvas* Canvas : ViewportInstance->Canvases)
		{
			Canvas->Tick(DeltaTime);
		}
	}*/
}

void sCameraManager::FixedUpdate(const double DeltaTime)
{
	if (ViewportInstance)
	{
		for (ICanvas* Canvas : ViewportInstance->Canvases)
		{
			Canvas->Tick(DeltaTime);
		}
	}
}

bool sCameraManager::AddCanvasToViewport(ICanvas* Canvas)
{
	if (std::find(ViewportInstance->Canvases.begin(), ViewportInstance->Canvases.end(), Canvas) != ViewportInstance->Canvases.end())
		return false;

	ViewportInstance->Canvases.push_back(Canvas);
	return true;
}

bool sCameraManager::RemoveCanvasFromViewport(ICanvas* Canvas)
{
	if (!ViewportInstance)
		return false;

	if (std::find(ViewportInstance->Canvases.begin(), ViewportInstance->Canvases.end(), Canvas) == ViewportInstance->Canvases.end())
		return false;

	ViewportInstance->Canvases.erase(std::find(ViewportInstance->Canvases.begin(), ViewportInstance->Canvases.end(), Canvas));
	return true;
}

bool sCameraManager::RemoveCanvasFromViewport(std::size_t Index)
{
	if (!ViewportInstance)
		return false;

	if (ViewportInstance->Canvases.size() <= Index)
		return false;

	ViewportInstance->Canvases.erase(ViewportInstance->Canvases.begin() + Index);
	return true;
}

ICanvas* sCameraManager::GetCanvas(std::size_t Index)
{
	if (!ViewportInstance)
		return nullptr;

	if (ViewportInstance->Canvases.size() <= Index)
		return nullptr;

	return ViewportInstance->Canvases.at(Index);
}

void sCameraManager::AttachToActor(sActor* InActor)
{
	if (InActor)
	{
		if (AttachedActor)
			DetachFromActor();

		AttachedActor = InActor;

		std::vector<sCameraComponent*> CameraComponents = AttachedActor->FindComponents<sCameraComponent>();
		if (CameraComponents.size() > 0)
		{
			PossessedCameraComponent = CameraComponents[0];

			//GPU::RemoveViewportInstance(ViewportInstance.get());
			ViewportInstance = nullptr;
			ViewportInstance = PossessedCameraComponent->GetViewportInstance();

			pCamera = nullptr;
			pCamera = PossessedCameraComponent->GetCamera();

			//GPU::AddViewportInstance(ViewportInstance.get());
		}
	}
}

void sCameraManager::DetachFromActor()
{
	DetachActorCamera();
	AttachedActor = nullptr;
}

void sCameraManager::DetachActorCamera()
{
	if (PossessedCameraComponent)
	{
		sCameraComponent* OldComp = PossessedCameraComponent;
		PossessedCameraComponent = nullptr;
		OldComp->UnPossess();

		pCamera = nullptr;
		pCamera = sCamera::Create();
		//GPU::RemoveViewportInstance(ViewportInstance.get());
		ViewportInstance = nullptr;
		ViewportInstance = sViewportInstance::Create();
		ViewportInstance->pCamera = pCamera;
		//GPU::AddViewportInstance(ViewportInstance.get());
	}
}

void sCameraManager::EnableCamera()
{
	if (ViewportInstance)
	{
		bIsCameraEnabled = true;
		ViewportInstance->bIsEnabled = bIsCameraEnabled;
		//GPU::AddViewportInstance(ViewportInstance.get());
	}
}

void sCameraManager::DisableCamera()
{
	//if (ViewportInstance)
	//	GPU::RemoveViewportInstance(ViewportInstance.get());
	bIsCameraEnabled = false;
	if (ViewportInstance)
		ViewportInstance->bIsEnabled = bIsCameraEnabled;
}

void sCameraManager::SplitViewport()
{
	if (ViewportInstance)
	{
		std::size_t PlayerCount = PCOwner->GetPlayerCount();
		if (PlayerCount == 1)
		{
			ViewportInstance->Viewport = sViewport(GPU::GetInternalBaseRenderResolution());
			if (IsOrthographic())
				SetOrthographic(ViewportInstance->Viewport->Width, ViewportInstance->Viewport->Height);
			return;
		}
		std::size_t PlayerIndex = PCOwner->GetPlayerIndex();
		ESplitScreenType SplitScreenType = PCOwner->GetSplitScreenType();

		auto BaseDimension = GPU::GetInternalBaseRenderResolution();
		switch (SplitScreenType)
		{
		case ESplitScreenType::Grid:
		{
			/*
			* 8 Player Support
			* 4 Top
			* 4 Bottom
			*/
			std::size_t Horizontal = PlayerCount == 2 ? 2 : std::ceil((float)PlayerCount / 2.0f);
			std::size_t Vertical = PlayerCount > 2 ? 2 : 1;

			ViewportInstance->Viewport = sViewport(GPU::GetInternalBaseRenderResolution());
			if (IsOrthographic())
				SetOrthographic(ViewportInstance->Viewport->Width / Horizontal, ViewportInstance->Viewport->Height / Vertical);

			std::size_t TopLeftX = (std::uint32_t)(BaseDimension.Width / Horizontal) * ((PlayerIndex > (Horizontal - 1)) ? (std::uint32_t)(PlayerIndex - Horizontal) : (std::uint32_t)(PlayerIndex));
			std::size_t TopLeftY = (PlayerIndex > (Horizontal - 1) ? (std::uint32_t)(BaseDimension.Height / Vertical) : 0);

			ViewportInstance->Viewport = sViewport((std::uint32_t)(BaseDimension.Width / Horizontal), (std::uint32_t)(BaseDimension.Height / Vertical), TopLeftX, TopLeftY);
		}
			break;
		case ESplitScreenType::Horizontal:
			ViewportInstance->Viewport = sViewport(GPU::GetInternalBaseRenderResolution());
			if (IsOrthographic())
				SetOrthographic(ViewportInstance->Viewport->Width / PlayerCount, ViewportInstance->Viewport->Height);
			ViewportInstance->Viewport = sViewport((std::uint32_t)(BaseDimension.Width / PlayerCount), (std::uint32_t)(BaseDimension.Height), (std::uint32_t)(BaseDimension.Width / PlayerCount * PlayerIndex), 0);
			break;
		}
	}
}

void sCameraManager::DisableSplitScreen()
{
	ViewportInstance->Viewport = sViewport(GPU::GetInternalBaseRenderResolution());
}

sPlayerController* sCameraManager::GetPlayerController() const
{
	return PCOwner;
}

void sCameraManager::UpdateCamera(float DeltaTime)
{
	if (PossessedCameraComponent)
		return;

	if (AttachedActor)
	{
		FVector Location = AttachedActor->GetLocation();
		ILevel* Level = AttachedActor->GetOwnedLevel();
		const FBoundingBox& LevelBounds = Level->GetLevelBounds();

		const sScreenDimension ViewportDimension = ViewportInstance->Viewport.has_value() ?
			sScreenDimension(ViewportInstance->Viewport->Width, ViewportInstance->Viewport->Height) : GPU::GetBackBufferDimension();

		if (IsOrthographic())
			Location = (FVector(Location.X - (ViewportDimension.Width / 2), Location.Y - (ViewportDimension.Height / 2), 0.0f));

		if ((Location.X - (ViewportDimension.Width / 2)) < LevelBounds.Min.X)
			Location.X = pCamera->GetPosition().X;
		if ((Location.X + (ViewportDimension.Width / 2)) > LevelBounds.Max.X)
			Location.X = pCamera->GetPosition().X;
		if ((Location.Y - (ViewportDimension.Height / 2)) < LevelBounds.Min.Y)
			Location.Y = pCamera->GetPosition().Y;
		if ((Location.Y + (ViewportDimension.Height / 2)) > LevelBounds.Max.Y)
			Location.Y = pCamera->GetPosition().Y;
		if (!IsOrthographic())
		{
			//if ((Location.Z) > LevelBounds.Min.Z)
			//	Location.Z = pCamera->GetPosition().Z;
			//if ((Location.Z) < LevelBounds.Max.Z)
			//	Location.Z = pCamera->GetPosition().Z;
		}

		//Utilities::WriteToConsole(Location.ToString());

		pCamera->SetPosition(Location);
		pCamera->Update();
	}
	else
	{
		Direction = FVector(DirectX::XMVectorSet(CameraMovement.Y * CameraMovementScaling, CameraMovement.Z * CameraMovementScaling, CameraMovement.X * CameraMovementScaling, 0.0f));

		if (bEnableCameraVelocity)
			UpdateVelocity(DeltaTime);

		FVector vPosDelta = bEnableCameraVelocity ? Velocity * DeltaTime : Direction * 0.001f * Speed * DeltaTime;

		/*if ((vPosDelta.X - (ViewportDimension.Width / 2)) < LevelBounds.Min.X)
			vPosDelta.X = pCamera->GetPosition().X;
		if ((vPosDelta.X + (ViewportDimension.Width / 2)) > LevelBounds.Max.X)
			vPosDelta.X = pCamera->GetPosition().X;
		if ((vPosDelta.Y - (ViewportDimension.Height / 2)) < LevelBounds.Min.Y)
			vPosDelta.Y = pCamera->GetPosition().Y;
		if ((vPosDelta.Y + (ViewportDimension.Height / 2)) > LevelBounds.Max.Y)
			vPosDelta.Y = pCamera->GetPosition().Y;
		if (!bIsOrthographic)
		{
			//if ((vPosDelta.Z) > LevelBounds.Min.Z)
			//	vPosDelta.Z = pCamera->GetPosition().Z;
			//if ((vPosDelta.Z) < LevelBounds.Max.Z)
			//	vPosDelta.Z = pCamera->GetPosition().Z;
		}*/

		if (bCameraFocused)
		{
			POINT ptCurMousePos;
			GetCursorPos(&ptCurMousePos);
			// Calc how far it's moved since last frame
			FVector ptCurMouseDelta = FVector(
				(float)(ptCurMousePos.x - LastMousePosition.X),
				(float)(ptCurMousePos.y - LastMousePosition.Y),
				(float)0);

			// Record current position for next time
			LastMousePosition.X = (float)ptCurMousePos.x;
			LastMousePosition.Y = (float)ptCurMousePos.y;

			// Smooth the relative mouse data over a few frames so it isn't 
			// jerky when moving slowly at low frame rates.
			float fPercentOfNew = 1.0f / 2.0f;
			float fPercentOfOld = 1.0f - fPercentOfNew;

			MouseDelta = MouseDelta * fPercentOfOld + ptCurMouseDelta * fPercentOfNew;

			RotVelocity = (MouseDelta * -1.0f) * 0.01f;

			float fYawDelta = DirectX::XMVectorGetX(RotVelocity);
			float fPitchDelta = DirectX::XMVectorGetY(RotVelocity);

			CameraPitchAngle += fPitchDelta;
			CameraYawAngle += fYawDelta;

			// Limit pitch to straight up or straight down
			CameraPitchAngle = std::max(-fPI / 2.0f, CameraPitchAngle);
			CameraPitchAngle = std::min(+fPI / 2.0f, CameraPitchAngle);
		}

		pCamera->SetTransform(vPosDelta, CameraPitchAngle, CameraYawAngle, 0);
		pCamera->Update();
	}
}

void sCameraManager::UpdateVelocity(float fElapsedTime)
{
	RotVelocity = MouseDelta * 0.01f;
	FVector vAccel = Direction * 0.001f;
	vAccel = vAccel * Speed;

	{
		if (DirectX::XMVectorGetX(DirectX::XMVector3Length(vAccel)) > 0)
		{
			Velocity = vAccel;
			DragTimer = 0.25f;
			VelocityDrag = vAccel / DragTimer;
		}
		else
		{
			if (DragTimer > 0.0f && bEnableCameraVelocity)
			{
				// Drag until timer is <= 0
				Velocity -= VelocityDrag * fElapsedTime;
				DragTimer -= fElapsedTime;
			}
			else
			{
				// Zero velocity
				Velocity = FVector(DirectX::XMVectorReplicate(0));
				DragTimer = 0.0f;
			}
		}
	}
}

void sCameraManager::SetPerspective(float AspectRatio)
{
	ViewportInstance->Viewport = std::nullopt;
	pCamera->SetPerspectiveMatrix((float)dPI_OVER_4, AspectRatio, 0.01f, 4000.0f);
}

void sCameraManager::SetOrthographic(std::size_t Width, std::size_t Height)
{
	ViewportInstance->Viewport = sViewport((std::uint32_t)Width, (std::uint32_t)Height);
	pCamera->SetOrthographic(Width, Height);
}

void sCameraManager::SetFOV(float NewFOV)
{
	pCamera->SetFOV(NewFOV);
}

void sCameraManager::SetAspectRatio(float AspectRatio)
{
	pCamera->SetAspectRatio(AspectRatio);
}

void sCameraManager::SetWorldScale(double sceneScaling, bool zAxisUp)
{
	pCamera->SetWorldScale(sceneScaling, zAxisUp);
}

void sCameraManager::SetViewport(std::optional<sViewport> Viewport)
{
	ViewportInstance->Viewport = Viewport;
}

void sCameraManager::SetRollPitchYaw(const float Roll, const float InPitch, const float InYaw)
{
	CameraPitchAngle = InPitch;
	CameraYawAngle = InYaw;
}

void sCameraManager::SetMousePosition(FVector2 InLastMousePosition)
{
	LastMousePosition = InLastMousePosition;
}

void sCameraManager::SetSpeed(const float InSpeed)
{
	Speed = InSpeed;
}

void sCameraManager::EnableVelocity()
{
	bEnableCameraVelocity = true;
	Velocity = FVector(DirectX::XMVectorReplicate(0));
	DragTimer = 0.0f;
}

void sCameraManager::DisableVelocity()
{
	bEnableCameraVelocity = false;
	Velocity = FVector(DirectX::XMVectorReplicate(0));
	DragTimer = 0.0f;
}

void sCameraManager::WindowResized(const std::size_t Width, const std::size_t Height)
{
	if (pCamera->IsOrthographic())
	{
		SetOrthographic(Width, Height);
	}
}

void sCameraManager::InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar)
{
	if (AttachedActor || PossessedCameraComponent)
		return;

	if (ViewportInstance)
	{
		for (ICanvas* Canvas : ViewportInstance->Canvases)
		{
			Canvas->InputProcess(MouseInput, KeyboardChar);
		}
	}

	if (!bIsInputMovementEnabled)
		return;

	if (KeyboardChar.bIsPressed && KeyboardChar.bIsChar)
	{
		if (KeyboardChar.KeyCode == 'w' || KeyboardChar.KeyCode == 'W')
		{
			MoveForward();
		}
		else if (KeyboardChar.KeyCode == 's' || KeyboardChar.KeyCode == 'S')
		{
			MoveBackward();
		}
		else if (KeyboardChar.KeyCode == 'a' || KeyboardChar.KeyCode == 'A')
		{
			MoveLeft();
		}
		else if (KeyboardChar.KeyCode == 'd' || KeyboardChar.KeyCode == 'D')
		{
			MoveRight();
		}
	}
	else
	{
		if (KeyboardChar.KeyCode == 'w' || KeyboardChar.KeyCode == 'W')
		{
			MoveForward_RELEASE();
		}
		if (KeyboardChar.KeyCode == 's' || KeyboardChar.KeyCode == 'S')
		{
			MoveBackward_RELEASE();
		}
		if (KeyboardChar.KeyCode == 'a' || KeyboardChar.KeyCode == 'A')
		{
			MoveLeft_RELEASE();
		}
		if (KeyboardChar.KeyCode == 'd' || KeyboardChar.KeyCode == 'D')
		{
			MoveRight_RELEASE();
		}
	}

	if (MouseInput.State == EMouseState::eScroll)
	{
		if (MouseInput.WheelDelta >= 1.0f)
		{
			MoveUp();
		}
		else if (MouseInput.WheelDelta <= 1.0f)
		{
			MoveDown();
		}
	}
	else
	{
		MoveUp_RELEASE();
		MoveDown_RELEASE();
	}

	if (MouseInput.Buttons.at("Right") == EMouseButtonState::ePressed)
	{
		SetCapture(GetActiveWindow());
		POINT MousePos;
		GetCursorPos(&MousePos);
		SetCameraFocused(true);
		SetMousePosition(FVector2((float)MousePos.x, (float)MousePos.y));
	}
	else if (MouseInput.Buttons.at("Right") == EMouseButtonState::eReleased)
	{
		ReleaseCapture();
		SetCameraFocused(false);
	}
}
