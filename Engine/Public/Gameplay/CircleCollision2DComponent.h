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

#pragma once

#include "PhysicalComponent.h"
#include "Engine/IRigidBody.h"

class sCircleCollision2DComponent : public sPhysicalComponent
{
	sClassBody(sClassConstructor, sCircleCollision2DComponent, sPhysicalComponent)
public:
	sCircleCollision2DComponent(std::string InName, const sRigidBodyDesc& Desc, const FVector2& Origin, float InRadius, sActor* pActor = nullptr);
	virtual ~sCircleCollision2DComponent();

	virtual bool HasRigidBody() const override final { return RigidBody != nullptr; }
	virtual IRigidBody* GetRigidBody() const override final { return RigidBody.get(); }

	//void SetDimension(const FDimension2D& Dimension);

	virtual void OnBeginPlay() override;
	virtual void OnFixedUpdate(const double DT) override;

	virtual FBoundingBox GetBounds() const override final;

private:
	virtual void OnUpdateTransform() override final;

private:
	IRigidBody::SharedPtr RigidBody;
};
