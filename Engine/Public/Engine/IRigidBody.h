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
#include "Core/Math/CoreMath.h"
#include "Engine/ClassBody.h"
#include <array>
#include <vector>

enum class ERigidBodyShape
{
	Box2D,
	Polygon2D,
	Circle2D,
	Edge2D,
	Chain2D,
	Box3D,
	Sphere,
	Capsule,
	Cylinder,
	Cone,
	ConvexHull,
	MultiBody,
	TriangleMesh,
};

enum class ERigidBodyType
{
	Static,
	Kinematic,
	Dynamic,
};

enum class ECollisionChannel : std::uint16_t
{
	None		= 0x00,
	OverlapOnly	= None,
	Default		= 0x01,
	Channel_0	= 0x02,
	Channel_1	= 0x03,
	Channel_2	= 0x04,
	Channel_3	= 0x05,
	Channel_4	= 0x06,
	Channel_5	= 0x07,
	Channel_6	= 0x08,
	Channel_7	= 0x09,
	Channel_8	= 0x10,
	Channel_9	= 0x11,
	Channel_10	= 0x12,
	Channel_11	= 0x13,
	Channel_12	= 0x14,
	Channel_13	= 0x15,
	Channel_14	= 0x16,
};

struct sRigidBodyDesc
{
	void* UserPointer = nullptr;
	ERigidBodyType RigidBodyType = ERigidBodyType::Static;
	float Mass = 0.0f;
	float Friction = 0.0f;
	float Restitution = 0.0f;
};

class sPhysicalComponent;

class IRigidBody
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IRigidBody)
	friend class IPhysicalWorld;
public:
	static IRigidBody::SharedPtr Create2DBoxBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FBounds2D& Bounds);
	static IRigidBody::SharedPtr Create2DPolygonBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, const std::array<FVector2, 8>& points);
	static IRigidBody::SharedPtr Create2DCircleBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, float InRadius);
	static IRigidBody::SharedPtr Create2DEdgeBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, const std::array<FVector2, 4>& points, bool OneSided);
	static IRigidBody::SharedPtr Create2DChainBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, const std::vector<FVector2>& vertices);
	static IRigidBody::SharedPtr Create2DChainBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, const std::vector<FVector2>& vertices, const FVector2& prevVertex, const FVector2& nextVertex);

	static IRigidBody::SharedPtr CreateBoxBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, FVector InHalf);
	static IRigidBody::SharedPtr CreateSphereBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, float InRadius);
	static IRigidBody::SharedPtr CreateCapsuleBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, float InRadius, float InHeight);
	static IRigidBody::SharedPtr CreateCylinderBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, float InRadius, float InHeight);
	static IRigidBody::SharedPtr CreateConeBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, float InRadius, float InHeight);

	static IRigidBody::SharedPtr CreateMultiBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, FVector InInertia, float InMass);
	static IRigidBody::SharedPtr CreateConvexHullBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, const float* points, int numPoints, int stride);
	static IRigidBody::SharedPtr CreateTriangleMesh(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, const FVector* points, int numPoints, const std::uint32_t* indices, int numIndices);

public:
	virtual bool HasOwner() const = 0;
	virtual sPhysicalComponent* GetOwner() const = 0;

	virtual ERigidBodyShape GetShape() const = 0;
	virtual sRigidBodyDesc GetDesc() const = 0;

	virtual void SetTransform(const FVector& position, const FQuaternion& orientation) const = 0;
	virtual FVector GetLocation() const = 0;
	virtual FQuaternion GetRotation() const = 0;

	virtual void GetAabb(FVector& InMýn, FVector& InMax) const = 0;

	virtual void SetCollisionChannel(ECollisionChannel Type) = 0;
	virtual void SetCollisionChannel(ECollisionChannel Type, std::uint16_t CollideTo) = 0;
	virtual ECollisionChannel GetCollisionChannel() const = 0;

	virtual FVector GetInertia() const = 0;

	virtual FMatrix GetTransformMatrix() const = 0;

	virtual void SetUserPointer(void* InActor) = 0;
	virtual void* GetUserPointer() const = 0;

	virtual float GetMass() const = 0;
	virtual void SetMass(float InMass) = 0;

	virtual float GetFriction() const = 0;
	virtual void SetFriction(float InFriction) = 0;

	virtual float GetRestitution() const = 0;
	virtual void SetRestitution(float InRestitution) = 0;

	virtual void SetLinearVelocity(const FVector& v) = 0;
	virtual FVector GetLinearVelocity() const = 0;

	virtual void SetLinearDamping(const float v) = 0;
	virtual float GetLinearDamping() const = 0;

	virtual void SetAngularVelocity(float omega) = 0;
	virtual float GetAngularVelocity() const = 0;

	virtual void SetAngularDamping(const float v) = 0;
	virtual float GetAngularDamping() const = 0;

	virtual void ApplyForce(const FVector& force, const FVector& point, bool wake = true) = 0;
	virtual void ApplyForceToCenter(const FVector& force, bool wake = true) = 0;
	virtual void ApplyTorque(float torque, bool wake = true) = 0;
	virtual void ApplyLinearImpulse(const FVector& impulse, const FVector& point, bool wake = true) = 0;
	virtual void ApplyLinearImpulseToCenter(const FVector& impulse, bool wake = true) = 0;
	virtual void ApplyAngularImpulse(float impulse, bool wake = true) = 0;

	virtual FVector GetWorldCenter() const = 0;

	virtual void SetGravityScale(float Yscale) = 0;
	virtual void SetGravityScale(const FVector& scale) = 0;
	virtual FVector GetGravityScale() const = 0;

	virtual void SetType(ERigidBodyType type) = 0;

	virtual void SetBullet(bool flag) = 0;
	virtual bool IsBullet() const = 0;

	virtual void SetAwake(bool flag) = 0;
	virtual bool IsAwake() const = 0;

	virtual void SetEnabled(bool flag) = 0;
	virtual bool IsEnabled() const = 0;

	virtual void SetFixedRotation(bool flag) = 0;
	virtual bool IsFixedRotation() const = 0;

protected:
	void UpdatePhysics();

private:
	virtual void OnUpdatePhysics() = 0;
	virtual void OnCollisionStart(IRigidBody* Component);
	virtual void OnCollisionEnd(IRigidBody* Component);
};
