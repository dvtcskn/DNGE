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
#include "Gameplay/CircleCollision2DComponent.h"
#include "Engine/AbstractEngine.h"

sCircleCollision2DComponent::sCircleCollision2DComponent(std::string InName, const sRigidBodyDesc& Desc, const FVector2& Origin, float InRadius, sActor* pActor)
	: Super(InName, pActor)
{
	RigidBody = IRigidBody::Create2DCircleBody(this, Desc, Origin, InRadius);
}

sCircleCollision2DComponent::~sCircleCollision2DComponent()
{
	RigidBody = nullptr;
}

void sCircleCollision2DComponent::OnBeginPlay()
{

}

void sCircleCollision2DComponent::OnFixedUpdate(const double DT)
{
	//GPU::DrawBound(GetBounds(), 0.05f);
}

FBoundingBox sCircleCollision2DComponent::GetBounds() const
{
	if (!RigidBody)
		return FBoundingBox();

	FVector Min;
	FVector Max;
	RigidBody->GetAabb(Min, Max);
	return FBoundingBox(Min, Max);
};

void sCircleCollision2DComponent::OnUpdateTransform()
{
	if (!RigidBody)
		return;

	if (RigidBody->GetLocation() != GetRelativeLocation() || RigidBody->GetRotation() != GetRelativeRotation())
		RigidBody->SetTransform(GetRelativeLocation(), GetRelativeRotation());
}
