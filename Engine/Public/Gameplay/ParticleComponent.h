#pragma once

#include "PrimitiveComponent.h"
#include "ParticleSystem.h"

class ParticleComponent : public sPrimitiveComponent
{
	sClassBody(sClassConstructor, ParticleComponent, sPrimitiveComponent)
public:
	ParticleComponent(std::string InName, sActor* pActor = nullptr);
	virtual ~ParticleComponent();

	void SetParticle(const sEmitter::SharedPtr& Emitter);
	sEmitter::SharedPtr GetEmitter() const { return Emitter; }

private:
	virtual void OnUpdateTransform() override final;

private:
	sEmitter::SharedPtr Emitter;
};
