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
#include "Gameplay/CameraComponent.h"
#include "Gameplay/Actor.h"

sCameraComponent::sCameraComponent(std::string InName, sActor* pActor)
	: Super(InName, pActor)
	, OwnedManager(nullptr)
{
	Camera = sCamera::Create();
	ViewportInstance = sViewportInstance::Create();
	ViewportInstance->pCamera = Camera;
}

sCameraComponent::~sCameraComponent()
{
	UnPossess();
	Camera = nullptr;
	ViewportInstance = nullptr;
	OwnedManager = nullptr;
}

void sCameraComponent::OnBeginPlay()
{
}

void sCameraComponent::OnTick(const double DeltaTime)
{
	FVector Location = GetOwner()->GetLocation();
	ILevel* Level = GetOwner()->GetOwnedLevel();
	const FBoundingBox& LevelBounds = Level->GetLevelBounds();

	const sScreenDimension ViewportDimension = ViewportInstance->Viewport.has_value() ?
		sScreenDimension(ViewportInstance->Viewport->Width, ViewportInstance->Viewport->Height) : GPU::GetBackBufferDimension();

	if (Camera->IsOrthographic())
		Location = (FVector(Location.X - (ViewportDimension.Width / 2), Location.Y - (ViewportDimension.Height / 2), 0.0f));

	if ((Location.X - (ViewportDimension.Width / 2)) < LevelBounds.Min.X)
		Location.X = Camera->GetPosition().X;
	if ((Location.X + (ViewportDimension.Width / 2)) > LevelBounds.Max.X)
		Location.X = Camera->GetPosition().X;
	if ((Location.Y - (ViewportDimension.Height / 2)) < LevelBounds.Min.Y)
		Location.Y = Camera->GetPosition().Y;
	if ((Location.Y + (ViewportDimension.Height / 2)) > LevelBounds.Max.Y)
		Location.Y = Camera->GetPosition().Y;
	if (!Camera->IsOrthographic())
	{
		//if ((Location.Z) > LevelBounds.Min.Z)
		//	Location.Z = pCamera->GetPosition().Z;
		//if ((Location.Z) < LevelBounds.Max.Z)
		//	Location.Z = pCamera->GetPosition().Z;
	}

	//Utilities::WriteToConsole(Location.ToString());

	Camera->SetPosition(Location);
	Camera->Update();
}

void sCameraComponent::OnFixedUpdate(const double DeltaTime)
{
}

void sCameraComponent::Possess(sCameraManager* Manager)
{
	if (OwnedManager)
		UnPossess();
	OwnedManager = Manager;
}

void sCameraComponent::UnPossess()
{
	if (OwnedManager)
	{
		sCameraManager* Old = OwnedManager;
		OwnedManager = nullptr;
		Old->DetachActorCamera();
	}
}

void sCameraComponent::SetViewport(std::optional<sViewport> Viewport)
{
	ViewportInstance->Viewport = Viewport;
}

void sCameraComponent::AttachCanvas(ICanvas* Canvas)
{
	ViewportInstance->Canvases.push_back(Canvas);
}

void sCameraComponent::SetPerspective(float AspectRatio) 
{
	//ViewportInstance->Viewport = sViewport(GPU::GetBackBufferDimension());
	ViewportInstance->Viewport = std::nullopt;
	Camera->SetPerspectiveMatrix((float)dPI_OVER_4, AspectRatio, 0.01f, 4000.0f);
}

void sCameraComponent::SetOrthographic(std::size_t Width, std::size_t Height) 
{
	ViewportInstance->Viewport = sViewport((std::uint32_t)Width, (std::uint32_t)Height);
	Camera->SetOrthographic(Width, Height);
}
