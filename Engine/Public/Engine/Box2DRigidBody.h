#pragma once

#include "IRigidBody.h"
#include "World2D.h"
#include "Gameplay/PhysicalComponent.h"

class sBox2DRigidBody : public IRigidBody
{
	sClassBody(sClassConstructor, sBox2DRigidBody, IRigidBody)
private:
	sPhysicalComponent* Owner;
	sWorld2D* World;
	b2Body* Body;
	b2Fixture* Fixture;
	ERigidBodyShape ShapeType;
	sRigidBodyDesc Desc;

public:
	sBox2DRigidBody(sPhysicalComponent* InOwner, IPhysicalWorld* InWorld, b2Body* InBody, const ERigidBodyShape ShapeType, const sRigidBodyDesc& Desc);
	virtual ~sBox2DRigidBody();

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

	float GetRestitutionThreshold() const;
	void SetRestitutionThreshold(float InRestitutionThreshold);

	float GetDensity() const;
	void SetDensity(float InDensity);

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

	FVector GetLinearVelocityFromWorldPoint(const FVector& worldPoint) const;
	FVector GetLinearVelocityFromLocalPoint(const FVector& localPoint) const;

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
