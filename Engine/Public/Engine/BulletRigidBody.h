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
#ifndef BT_NO_SIMD_OPERATOR_OVERLOADS
#define BT_NO_SIMD_OPERATOR_OVERLOADS
#endif
#include <btBulletDynamicsCommon.h>
#include "IRigidBody.h"
#include "Engine/BulletWorld.h"

class BulletRigidBody final : public IRigidBody
{
	sClassBody(sClassConstructor, BulletRigidBody, IRigidBody)
private:
	sPhysicalComponent* Owner;
	BulletWorld* World;
	ERigidBodyShape ShapeType;
	sRigidBodyDesc Desc;
	btRigidBody* RigidBody;
	btMotionState* MotionState;
	btVector3 BaseGravity;
	std::vector<btCollisionShape*> ChildShapes;

public:
	BulletRigidBody(sPhysicalComponent* InOwner, IPhysicalWorld* InWorld, btRigidBody* RigidBody, std::vector<btCollisionShape*> ChildShapes, const ERigidBodyShape ShapeType, const sRigidBodyDesc& Desc);
	virtual ~BulletRigidBody();

	virtual bool HasOwner() const override final;
	virtual sPhysicalComponent* GetOwner() const override final;

	virtual void OnUpdatePhysics() override final;

	virtual void SetCollisionChannel(ECollisionChannel Type) override final;
	virtual void SetCollisionChannel(ECollisionChannel Type, std::uint16_t CollideTo) override final;
	virtual ECollisionChannel GetCollisionChannel() const override final;

	virtual ERigidBodyShape GetShape() const override final;
	virtual sRigidBodyDesc GetDesc() const override final;

	virtual void SetTransform(const FVector& position, const FQuaternion& orientation) const override final;
	virtual FVector GetLocation() const override final;
	virtual FQuaternion GetRotation() const override final;

	virtual void GetAabb(FVector& InMýn, FVector& InMax) const override final;

	virtual FVector GetInertia() const override final;

	virtual FMatrix GetTransformMatrix() const override final;

	virtual void SetUserPointer(void* InActor) override final;
	virtual void* GetUserPointer() const override final;

	virtual float GetMass() const override final;
	virtual void SetMass(float InMass) override final;

	virtual float GetFriction() const override final;
	virtual void SetFriction(float InFriction) override final;

	virtual float GetRestitution() const override final;
	virtual void SetRestitution(float InRestitution) override final;

	virtual void SetLinearVelocity(const FVector& v) override final;
	virtual FVector GetLinearVelocity() const override final;

	virtual void SetLinearDamping(const float v) override final;
	virtual float GetLinearDamping() const override final;

	virtual void SetAngularVelocity(float omega) override final;
	virtual float GetAngularVelocity() const override final;
	
	virtual void SetAngularDamping(const float v) override final;
	virtual float GetAngularDamping() const override final;

	virtual void ApplyForce(const FVector& force, const FVector& point, bool wake = true) override final;
	virtual void ApplyForceToCenter(const FVector& force, bool wake = true) override final;
	virtual void ApplyTorque(float torque, bool wake = true) override final;
	virtual void ApplyLinearImpulse(const FVector& impulse, const FVector& point, bool wake = true) override final;
	virtual void ApplyLinearImpulseToCenter(const FVector& impulse, bool wake = true) override final;
	virtual void ApplyAngularImpulse(float impulse, bool wake = true) override final;

	virtual FVector GetWorldCenter() const override final;

	virtual void SetGravityScale(float scale) override final;
	virtual void SetGravityScale(const FVector& scale) override final;
	virtual FVector GetGravityScale() const override final;

	virtual void SetType(ERigidBodyType type) override final;

	virtual void SetBullet(bool flag) override final;
	virtual bool IsBullet() const override final;

	virtual void SetAwake(bool flag) override final;
	virtual bool IsAwake() const override final;

	virtual void SetEnabled(bool flag) override final;
	virtual bool IsEnabled() const override final;

	virtual void SetFixedRotation(bool flag) override final;
	virtual bool IsFixedRotation() const override final;
};
