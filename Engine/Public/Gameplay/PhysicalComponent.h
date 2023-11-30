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

#include "PrimitiveComponent.h"
#include "Engine/IRigidBody.h"

class sPhysicalComponent : public sPrimitiveComponent
{
	sClassBody(sClassNoDefaults, sPhysicalComponent, sPrimitiveComponent)
	friend class IPhysicalWorld;
	friend class IRigidBody;
protected:
	sPhysicalComponent(std::string InName, sActor* pActor = nullptr);
public:
	virtual ~sPhysicalComponent();

	void EnablePhysics() { bEnablePhysics = true; }
	void DisablePhysics() { bEnablePhysics = false; }
	bool IsPhysicsEnabled() const { return bEnablePhysics; }

	virtual bool HasRigidBody() const = 0;
	virtual IRigidBody* GetRigidBody() const = 0;

	virtual void Serialize(sArchive& archive);

	virtual void BindFunctionToCollisionStart(std::function<void(sPhysicalComponent*)> pfCollisionStart) { fCollisionStart = pfCollisionStart; }
	virtual void BindFunctionToCollisionEnd(std::function<void(sPhysicalComponent*)> pfCollisionEnd) { fCollisionEnd = pfCollisionEnd; }

	void SetCollisionEnabled(bool val);
	bool IsCollisionEnabled() const;
	void SetCollisionChannel(ECollisionChannel Type);
	void SetCollisionChannel(ECollisionChannel Type, std::uint16_t CollideTo);
	ECollisionChannel GetCollisionChannel() const;

	void SetLinearVelocity(const FVector& v);
	virtual FVector GetVelocity() const override final;

	void SetLinearDamping(const float v);
	float GetLinearDamping() const;

	void SetAngularVelocity(float omega);
	float GetAngularVelocity() const;

	void SetAngularDamping(const float v);
	float GetAngularDamping() const;

	void ApplyForce(const FVector& force, const FVector& point, bool wake = true);
	void ApplyForceToCenter(const FVector& force, bool wake = true);
	void ApplyTorque(float torque, bool wake = true);
	void ApplyLinearImpulse(const FVector& impulse, const FVector& point, bool wake = true);
	void ApplyLinearImpulseToCenter(const FVector& impulse, bool wake = true);
	void ApplyAngularImpulse(float impulse, bool wake = true);

	virtual void Replicate(bool bReplicate) override;

private:
	void UpdatePhysics();

	virtual void OnUpdateTransform() override;
	virtual void OnUpdatePhysics(double DeltaTime) {}

	void CollisionStart(sPhysicalComponent* Component);
	void CollisionEnd(sPhysicalComponent* Component);

	virtual void OnCollisionStart(sPhysicalComponent* Component) {}
	virtual void OnCollisionEnd(sPhysicalComponent* Component) {}

private:
	void SetLinearVelocity_Client(const FVector& v);

private:
	bool bEnablePhysics;
	std::function<void(sPhysicalComponent*)> fCollisionStart;
	std::function<void(sPhysicalComponent*)> fCollisionEnd;
};
