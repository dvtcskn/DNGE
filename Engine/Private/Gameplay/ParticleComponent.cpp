
#include "pch.h"
#include "Gameplay/ParticleComponent.h"

ParticleComponent::ParticleComponent(std::string InName, sActor* pActor)
	: Super(InName, pActor)
{
}

ParticleComponent::~ParticleComponent()
{
	Emitter = nullptr;
}

void ParticleComponent::SetParticle(const sEmitter::SharedPtr& InEmitter)
{
	Emitter = nullptr;
	Emitter = InEmitter;
}

void ParticleComponent::OnUpdateTransform()
{
	Emitter->SetLocation(GetLocalLocation());
	Emitter->SetRotation(GetLocalRotationAngles());
	Emitter->SetScale(GetLocalScale());
}
