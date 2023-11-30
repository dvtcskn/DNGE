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
#include "SpriteSheetComponent.h"
#include "Sprite.h"
#include <Gameplay/Actor.h>
#include <Core/MeshPrimitives.h>
#include <AbstractGI/MaterialManager.h>
#include <Gameplay/BoxCollision2DComponent.h>
#include "SpriteEffectComponent.h"
#include "SpriteAnimationManager.h"

sSpriteSheetComponent::sSpriteSheetComponent(const std::string Name)
	: Super(Name)
	, MeshContainer(sSpriteSheetComponentMeshContainer::CreateUnique(this))
	, FrameCount(0)
	, CurrentSpriteIndex(0)
	, bIsAnimationActive(true)
	, bFlip(false)
	, CurrentKeyFrame(nullptr)
	, AnimSeqFrameCount(0)
	//, ElapsedTime(0.0f)
	//, FrameTime(0.0f)
	, Offset(FVector::Zero())
	, SpriteSheet(nullptr)
	, fAnimationStarted(nullptr)
	, fAnimationEnd(nullptr)
	, fFrameUpdated(nullptr)
{
	sBufferDesc BufferDesc;
	BufferDesc.Size = sizeof(sAnimationFlip);
	AnimationCB = IConstantBuffer::Create("AnimationFlip", BufferDesc, 4);

	sSpriteEffectComponent::SharedPtr pSpriteEffectComponent = sSpriteEffectComponent::Create("SpriteEffectComponent");
	pSpriteEffectComponent->AttachToComponent(this);
	SpriteEffectComponent = pSpriteEffectComponent.get();

	MeshConstantBuffer = IConstantBuffer::Create(Name, sBufferDesc(sizeof(sMeshConstantBufferAttributes), 0), 1);
	{
		ObjectConstants.modelMatrix = FMatrix::Identity();
		ObjectConstants.PrevModelMatrix = FMatrix::Identity();
		MeshConstantBuffer->Map(&ObjectConstants);
	}

	UpdateTransform();
}

sSpriteSheetComponent::sSpriteSheetComponent(sSpriteSheet* pSpriteSheet)
	: Super(pSpriteSheet->GetName())
	, MeshContainer(sSpriteSheetComponentMeshContainer::CreateUnique(this))
	, FrameCount(0)
	, CurrentSpriteIndex(0)
	, bIsAnimationActive(true)
	, bFlip(false)
	, CurrentKeyFrame(nullptr)
	, AnimSeqFrameCount(0)
	//, ElapsedTime(0.0f)
	//, FrameTime(0.0f)
	, Offset(FVector::Zero())
	, SpriteSheet(pSpriteSheet)
	, fAnimationStarted(nullptr)
	, fAnimationEnd(nullptr)
	, fFrameUpdated(nullptr)
{
	sBufferDesc BufferDesc;
	BufferDesc.Size = sizeof(sAnimationFlip);
	AnimationCB = IConstantBuffer::Create("AnimationFlip", BufferDesc, 4);

	sSpriteEffectComponent::SharedPtr pSpriteEffectComponent = sSpriteEffectComponent::Create("SpriteEffectComponent");
	pSpriteEffectComponent->AttachToComponent(this);
	SpriteEffectComponent = pSpriteEffectComponent.get();

	MeshConstantBuffer = IConstantBuffer::Create(GetName(), sBufferDesc(sizeof(sMeshConstantBufferAttributes), 0), 1);
	{
		ObjectConstants.modelMatrix = FMatrix::Identity();
		ObjectConstants.PrevModelMatrix = FMatrix::Identity();
		MeshConstantBuffer->Map(&ObjectConstants);
	}

	UpdateTransform();
}

sSpriteSheetComponent::~sSpriteSheetComponent()
{
	MeshConstantBuffer = nullptr;
	CurrentKeyFrame = nullptr;
	MeshContainer = nullptr;
	SpriteSheet = nullptr;

	SpriteEffectComponent = nullptr;

	fAnimationStarted = nullptr;
	fAnimationEnd = nullptr;
	fFrameUpdated = nullptr;

	AnimationCB = nullptr;
}

void sSpriteSheetComponent::OnBeginPlay()
{
}

void sSpriteSheetComponent::OnFixedUpdate(const double DeltaTime)
{
	Run((float)DeltaTime);
}

void sSpriteSheetComponent::OnUpdateTransform()
{
	if (HasComponentOwner())
	{
		ObjectConstants.PrevModelMatrix = ObjectConstants.modelMatrix;
		ObjectConstants.modelMatrix = ToMatrixWithScale(GetRelativeLocation(), GetRelativeScale(), GetRelativeRotation());
		//auto RelRot = GetRelativeRotation();

		MeshConstantBuffer->Map(&ObjectConstants);
	}
	else
	{
		ObjectConstants.PrevModelMatrix = ObjectConstants.modelMatrix;
		ObjectConstants.modelMatrix = ToMatrixWithScale(GetRelativeLocation(), GetRelativeScale(), GetRelativeRotation());

		MeshConstantBuffer->Map(&ObjectConstants);
	}
}

bool sSpriteSheetComponent::IsFlipped() const
{
	return bFlip;
	//if (CurrentKeyFrame)
	//	return CurrentKeyFrame->Sprite->IsFlipped();
	//return false;
}

void sSpriteSheetComponent::Flip(bool value)
{
	if (bFlip == value)
		return;
	bFlip = value;

	if (bFlip)
	{
		//SetRelativeRotation(FQuaternion(FAngles(180.0f, 0.0f, 0.0f)));

		AnimationFlip.Flip = true;
		AnimationCB->Map(&AnimationFlip);
	}
	else
	{
		//SetRelativeRotation(FQuaternion(FAngles(0.0f, 0.0f, 0.0f)));

		AnimationFlip.Flip = false;
		AnimationCB->Map(&AnimationFlip);
	}

	//if (CurrentKeyFrame)
	//	CurrentKeyFrame->Sprite->Flip(value);
}

sSprite* sSpriteSheetComponent::GetCurrentSprite() const
{
	return CurrentKeyFrame->Sprite.get();
}

std::size_t sSpriteSheetComponent::GetCurrentSpriteIndex() const
{
	return CurrentSpriteIndex;
}

bool sSpriteSheetComponent::IsAnimationEnded() const
{
	return AnimSeqFrameCount >= SpriteSheet->GetNumFrames();
}

bool sSpriteSheetComponent::IsAnimationRunning() const
{
	return bIsAnimationActive;
}

void sSpriteSheetComponent::Run(const float DeltaTime)
{
	if (!bIsAnimationActive)
		return;

	FrameCount++;
	if (FrameCount >= SpriteSheet->FPS) // 10
	{
		FrameCount = 0;
		UpdateFrame();
	}
	//return;

	//FrameTime += DeltaTime;
	//if (FrameTime >= (1.0f / (float)Graph.States.at(AnimationState)->FPS)) // 10
	//{
	//	UpdateFrameAtTime(FrameTime);
	//	FrameTime = 0.0f;
	//}
}

void sSpriteSheetComponent::UpdateFrame(std::optional<std::size_t> Frame)
{
	const std::size_t Duration = SpriteSheet->GetNumFrames();

	if (AnimSeqFrameCount == 0)
		OnAnimationStarted();

	if (Frame.has_value())
	{
		if (*Frame > Duration)
		{
			if (AnimSeqFrameCount != Duration)
			{
				AnimSeqFrameCount = Duration;
				OnFrameUpdated();
			}
		}
		else
		{
			const std::size_t CorrectedFrame = *Frame == 0 ? 1 : *Frame;
			if (AnimSeqFrameCount != CorrectedFrame)
			{
				AnimSeqFrameCount = CorrectedFrame;
				OnFrameUpdated();
			}
		}
	}
	else
	{
		AnimSeqFrameCount++;
		OnFrameUpdated();
	}

	auto KeyFrame = SpriteSheet->GetKeyFrameAtFrame(AnimSeqFrameCount == 0 ? 0 : AnimSeqFrameCount - 1);

	if (CurrentKeyFrame != KeyFrame)
	{
		CurrentSpriteIndex = SpriteSheet->GetKeyFrameIndexAtFrame(AnimSeqFrameCount == 0 ? 0 : AnimSeqFrameCount - 1);
		CurrentKeyFrame = KeyFrame;
		OnAnimationUpdated();
	}

	//Utilities::WriteToConsole(KeyFrame->Sprite->GetName() + " " + "Frame : " + std::to_string(CurrentSpriteIndex));

	if (AnimSeqFrameCount >= Duration)
	{
		if (SpriteSheet->bIsAnimationInLoop)
		{
			Reset();
			Play();
			OnAnimationUpdated();
		}
		else
		{
			bIsAnimationActive = false;
			OnAnimationEnded();
		}
	}
}

void sSpriteSheetComponent::SetSpriteSheet(sSpriteSheet* pSpriteSheet)
{
	if (SpriteSheet == pSpriteSheet)
		return;

	SpriteSheet = pSpriteSheet;
	//ElapsedTime += (1.0f / (float)Graph.States.at(AnimationState)->FPS);
	UpdateFrame(1);
	OnAnimationUpdated();
	Play();
}

//void sSpriteSheetComponent::UpdateFrameAtTime(float Time)
//{
//	const float Duration = Graph.States.at(AnimationState)->GetTotalDurationInSeconds();
//
//	ElapsedTime += Time;
//	AnimSeqFrameCount++;
//	OnFrameUpdated();
//
//	auto KeyFrame = Graph.States.at(AnimationState)->GetKeyFrameAtTime(ElapsedTime);
//
//	if (CurrentKeyFrame != KeyFrame)
//	{
//		CurrentSpriteIndex = Graph.States.at(AnimationState)->GetKeyFrameIndexAtTime(ElapsedTime);
//		CurrentKeyFrame = KeyFrame;
//		OnAnimationUpdated();
//	}
//
//	Utilities::WriteToConsole(KeyFrame->Sprite->GetName() + " " + "Frame : " + std::to_string(CurrentSpriteIndex));
//
//	if (ElapsedTime >= Duration)
//	{
//		if (Graph.States.at(AnimationState)->bIsAnimationInLoop)
//		{
//			Reset();
//			ElapsedTime = 0.0;
//			Play();
//			OnAnimationUpdated();
//		}
//		else
//		{
//			bIsAnimationActive = false;
//		}
//	}
//}

void sSpriteSheetComponent::OnFrameUpdated()
{
	if (fFrameUpdated)
		fFrameUpdated(AnimSeqFrameCount);
}

void sSpriteSheetComponent::OnAnimationUpdated()
{
	if (!HasOwner() || !HasComponentOwner() || !CurrentKeyFrame)
		return;
	auto SpriteBound = CurrentKeyFrame->Sprite->GetBound();
	SetRelativeLocation(FVector(0.0f/*(bFlip ? 1.0f : (-1.0f)) * ((SpriteBound.GetDimension().Width - SpriteSheet->WidthOffset) / 2.0f)*/,
		(-1.0f) * ((SpriteBound.GetDimension().Height - GetOwner()->GetBounds().GetDimension().Height) / 2.0f), 0.0f) + Offset);
}

void sSpriteSheetComponent::OnAttachToComponent()
{
	OnAnimationUpdated();
}

void sSpriteSheetComponent::OnDetachFromComponent()
{
}

void sSpriteSheetComponent::OnAnimationStarted()
{
	if (fAnimationStarted)
		fAnimationStarted();
}

void sSpriteSheetComponent::OnAnimationEnded()
{
	if (fAnimationEnd)
		fAnimationEnd();
}

void sSpriteSheetComponent::Play()
{
	bIsAnimationActive = true;
}

void sSpriteSheetComponent::Pause()
{
	bIsAnimationActive = false;
}

void sSpriteSheetComponent::Stop()
{
	bIsAnimationActive = false;
	FrameCount = 0;
	AnimSeqFrameCount = 0;
	CurrentSpriteIndex = 0;
}

void sSpriteSheetComponent::Reset()
{
	bIsAnimationActive = true;
	FrameCount = 0;
	AnimSeqFrameCount = 0;
	CurrentSpriteIndex = 0;
}

void sSpriteSheetComponent::ForceEndAnimation()
{
	bIsAnimationActive = false;
	AnimSeqFrameCount = SpriteSheet->GetNumFrames();
	CurrentSpriteIndex = SpriteSheet->GetKeyFrameIndexAtFrame(AnimSeqFrameCount);
}

void sSpriteSheetComponent::BindFunction_AnimationStarted(std::function<void()> pf)
{
	fAnimationStarted = pf;
}

void sSpriteSheetComponent::BindFunction_AnimationEnded(std::function<void()> pf)
{
	fAnimationEnd = pf;
}

void sSpriteSheetComponent::BindFunction_FrameUpdated(std::function<void(std::size_t)> pf)
{
	fFrameUpdated = pf;
}

void sSpriteSheetComponent::Serialize(sArchive& archive)
{
	//if (archive.IsReadMode())
	//{
	//	archive >> DefaultMaterialName;
	//	archive >> bIsDirty;
	//	archive >> Data;
	//	archive >> Data.Indices;
	//	archive >> Data.Vertices;

	//	DirectX::FVector3 Loc;
	//	DirectX::FVector3 Scale;
	//	DirectX::FQuaternion Rot;
	//	ObjectConstants.modelMatrix.Decompose(Scale, Rot, Loc);

	//	mTransform(Rot, Loc, Scale);

	//	AABB.MoveTo(mTransform.GetLocation());
	//	RigidBody->Teleport(Loc, (DirectX::FVector4&)mTransform.GetRotation());

	//	UpdateModelCB();
	//}
	//else
	//{
	//	archive << DefaultMaterialName;
	//	archive << bIsDirty;
	//	//archive << mTransform.GetLocation();
	//	//archive << mTransform.GetScale3D();
	//	//archive << DirectX::FVector4(mTransform.GetRotation());
	//	archive << ObjectConstants.modelMatrix;
	//	archive << ObjectConstants.mInverseTransposemodelMatrix;
	//}
}
