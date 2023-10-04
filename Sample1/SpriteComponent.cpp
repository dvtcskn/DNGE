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
#include "SpriteComponent.h"
#include "Sprite.h"
#include <Gameplay/Actor.h>
#include <Core/MeshPrimitives.h>
#include <AbstractGI/MaterialManager.h>
#include <Gameplay/BoxCollision2DComponent.h>
#include <Utilities/Utilities.h>

sSpriteComponent::sSpriteComponent(const std::string Name)
	: Super(Name)
	, MeshContainer(sSpriteComponentMeshContainer::CreateUnique(this))
	, bFlip(false)
	, Sprite(nullptr)
	, Offset(FVector::Zero())
{
	MeshConstantBuffer = IConstantBuffer::Create(Name, sBufferDesc(sizeof(sMeshConstantBufferAttributes), 0), 1);
	{
		ObjectConstants.modelMatrix = FMatrix::Identity();
		ObjectConstants.PrevModelMatrix = FMatrix::Identity();
		MeshConstantBuffer->Map(&ObjectConstants);
	}

	UpdateTransform();
}

sSpriteComponent::sSpriteComponent(sSprite* pSprite)
	: Super(pSprite->GetName())
	, MeshContainer(sSpriteComponentMeshContainer::CreateUnique(this))
	, bFlip(false)
	, Sprite(pSprite)
	, Offset(FVector::Zero())
{
	MeshConstantBuffer = IConstantBuffer::Create(Name, sBufferDesc(sizeof(sMeshConstantBufferAttributes), 0), 1);
	{
		ObjectConstants.modelMatrix = FMatrix::Identity();
		ObjectConstants.PrevModelMatrix = FMatrix::Identity();
		MeshConstantBuffer->Map(&ObjectConstants);
	}

	UpdateTransform();
}

sSpriteComponent::~sSpriteComponent()
{
	MeshConstantBuffer = nullptr;
	Sprite = nullptr;
	MeshContainer = nullptr;
}

void sSpriteComponent::OnBeginPlay()
{
}

void sSpriteComponent::OnTick(const double DeltaTime)
{

}

void sSpriteComponent::SetSprite(sSprite* pSprite)
{
	Sprite = nullptr;
	Sprite = pSprite;
	UpdateOffset();
}

void sSpriteComponent::OnUpdateTransform()
{
	if (HasComponentOwner())
	{
		ObjectConstants.PrevModelMatrix = ObjectConstants.modelMatrix;
		ObjectConstants.modelMatrix = ToMatrixWithScale(GetRelativeLocation(), GetRelativeScale(), GetRelativeRotation());

		MeshConstantBuffer->Map(&ObjectConstants);
	}
	else
	{
		ObjectConstants.PrevModelMatrix = ObjectConstants.modelMatrix;
		ObjectConstants.modelMatrix = ToMatrixWithScale(GetRelativeLocation(), GetRelativeScale(), GetRelativeRotation());

		MeshConstantBuffer->Map(&ObjectConstants);
	}
}

bool sSpriteComponent::IsFlipped() const
{
	return bFlip;
}

void sSpriteComponent::Flip(bool value)
{
	if (bFlip == value)
		return;
	bFlip = value;

	if (bFlip)
	{
		SetRelativeRotation(FQuaternion(FAngles(0.0f, 180.0f, 0.0f)));
	}
	else
	{
		SetRelativeRotation(FQuaternion(FAngles(0.0f, 0.0f, 0.0f)));
	}
}

void sSpriteComponent::UpdateOffset()
{
	if (!HasComponentOwner() || !Sprite)
		return;

	auto SpriteBound = Sprite->GetBound();
	SetRelativeLocation(FVector(0.0f, (-1.0f) * ((SpriteBound.GetDimension().Height - GetOwner()->GetBounds().GetDimension().Height) / 2.0f), 0.0f) + Offset);
}

void sSpriteComponent::OnAttachToComponent()
{
	UpdateOffset();
}

void sSpriteComponent::OnDetachFromComponent()
{
}

void sSpriteComponent::Serialize(Archive& archive)
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
