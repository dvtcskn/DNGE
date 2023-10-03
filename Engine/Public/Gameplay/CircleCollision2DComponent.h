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
