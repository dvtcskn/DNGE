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

	virtual FVector GetVelocity() const override final;

	virtual void Serialize(Archive& archive);

	virtual void BindFunctionToCollisionStart(std::function<void(sPhysicalComponent*)> pfCollisionStart) { fCollisionStart = pfCollisionStart; }
	virtual void BindFunctionToCollisionEnd(std::function<void(sPhysicalComponent*)> pfCollisionEnd) { fCollisionEnd = pfCollisionEnd; }

	void SetCollisionEnabled(bool val);
	bool IsCollisionEnabled() const;
	void SetCollisionChannel(ECollisionChannel Type);
	void SetCollisionChannel(ECollisionChannel Type, std::uint16_t CollideTo);
	ECollisionChannel GetCollisionChannel() const;

private:
	void UpdatePhysics();

	virtual void OnUpdateTransform() override;
	virtual void OnUpdatePhysics(double DeltaTime) {}

	void CollisionStart(sPhysicalComponent* Component);
	void CollisionEnd(sPhysicalComponent* Component);

	virtual void OnCollisionStart(sPhysicalComponent* Component) {}
	virtual void OnCollisionEnd(sPhysicalComponent* Component) {}

private:
	bool bEnablePhysics;
	std::function<void(sPhysicalComponent*)> fCollisionStart;
	std::function<void(sPhysicalComponent*)> fCollisionEnd;
};
