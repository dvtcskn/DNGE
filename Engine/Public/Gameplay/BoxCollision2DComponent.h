#pragma once

#include "PhysicalComponent.h"
#include "Engine/IRigidBody.h"

class sBoxCollision2DComponent : public sPhysicalComponent
{
	sClassBody(sClassConstructor, sBoxCollision2DComponent, sPhysicalComponent)
public:
	sBoxCollision2DComponent(std::string InName, const sRigidBodyDesc& Desc, const FDimension2D& Dimension, sActor* pActor = nullptr);
	virtual ~sBoxCollision2DComponent();

	virtual bool HasRigidBody() const override final { return RigidBody != nullptr; }
	virtual IRigidBody* GetRigidBody() const override final { return RigidBody.get(); }

	virtual void OnBeginPlay() override;
	virtual void OnFixedUpdate(const double DT) override;

	//void SetDimension(const FDimension2D& Dimension);

	virtual FBoundingBox GetBounds() const override final;

private:
	virtual void OnUpdateTransform() override final;

private:
	IRigidBody::SharedPtr RigidBody;
	FDimension2D Dimension;
};
