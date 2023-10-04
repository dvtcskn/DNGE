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

#include <memory>
#include "Engine/AbstractEngine.h"
#include "Core/Math/CoreMath.h"
#include "IRigidBody.h"
#include "Engine/ClassBody.h"
#include "Gameplay/PhysicalComponent.h"

class IPhysicalWorld
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IPhysicalWorld)
public:
	virtual void BeginPlay() = 0;
	virtual void Tick(const double InDeltaTime) = 0;

	virtual void SetGravity(const FVector& Gravity) = 0;
	virtual FVector GetGravity() const = 0;
	virtual void SetWorldOrigin(const FVector& newOrigin) = 0;

	virtual sPhysicalComponent* LineTraceToViewPort(const FVector& InOrigin, const FVector& InDirection) const = 0;
	virtual std::vector<sPhysicalComponent*> QueryAABB(const FBoundingBox& Bounds) const = 0;

	virtual std::size_t GetBodyCount() const = 0;
	virtual sPhysicalComponent* GetPhysicalBody(std::size_t Index) const = 0;
	virtual IRigidBody* GetBody(std::size_t Index) const = 0;

	virtual void SetPhysicsInternalTick(std::optional<double> Tick) = 0;
	virtual std::optional<double> GetPhysicsInternalTick() const = 0;
	virtual EPhysicsEngine GetPhysicsEngineType() const = 0;

	virtual float GetPhysicalWorldScale() const = 0;

	virtual IRigidBody::SharedPtr Create2DBoxBody(sPhysicalComponent* Owner,const sRigidBodyDesc& Desc, const FBounds2D& Bounds) const = 0;
	virtual IRigidBody::SharedPtr Create2DPolygonBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, const std::array<FVector2, 8>& points) const = 0;
	virtual IRigidBody::SharedPtr Create2DCircleBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, float InRadius) const = 0;
	virtual IRigidBody::SharedPtr Create2DEdgeBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, const std::array<FVector2, 4>& points, bool OneSided) const = 0;
	virtual IRigidBody::SharedPtr Create2DChainBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, const std::vector<FVector2>& vertices) const = 0;
	virtual IRigidBody::SharedPtr Create2DChainBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, const std::vector<FVector2>& vertices, const FVector2& prevVertex, const FVector2& nextVertex) const = 0;

	virtual IRigidBody::SharedPtr CreateBoxBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, FVector InHalf) const = 0;
	virtual IRigidBody::SharedPtr CreateSphereBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, float InRadius) const = 0;
	virtual IRigidBody::SharedPtr CreateCapsuleBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, float InRadius, float InHeight) const = 0;
	virtual IRigidBody::SharedPtr CreateCylinderBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, float InRadius, float InHeight) const = 0;
	virtual IRigidBody::SharedPtr CreateConeBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, float InRadius, float InHeight) const = 0;

	virtual IRigidBody::SharedPtr CreateMultiBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, FVector InInertia, float InMass) const = 0;
	virtual IRigidBody::SharedPtr CreateConvexHullBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, const float* points, int numPoints, int stride) const = 0;
	virtual IRigidBody::SharedPtr CreateTriangleMesh(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, const FVector* points, int numPoints, const std::uint32_t* indices, int numIndices) const = 0;

protected:
	void UpdateBodyPhysics();

	void BeginCollision(IRigidBody* contactA, IRigidBody* contactB);
	void EndCollision(IRigidBody* contactA, IRigidBody* contactB);
};
