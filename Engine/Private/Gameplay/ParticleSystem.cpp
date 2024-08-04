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

#include "pch.h"
#include "Gameplay/ParticleSystem.h"
#include "Core/MeshPrimitives.h"

sEmitter::sEmitter(std::string InName)
	: Name(InName)
	, Location(FVector::Zero())
	, Scale(FVector::One())
	, Rotation(FAngles())
{
}

sEmitter::~sEmitter()
{
	for (auto& Particle : Particles)
		Particle = nullptr;
	Particles.clear();
}

void sEmitter::BeginPlay()
{
	//for (const auto& Particle : Particles)
	//	Particle->Begin();
}

void sEmitter::Tick(float DT)
{
	for (const auto& Particle : Particles)
		Particle->Update(DT);
}

void sEmitter::AddParticle(const ParticleBase::SharedPtr& Particle)
{
	if (std::find(Particles.begin(), Particles.end(), Particle) == Particles.end())
	{
		Particles.push_back(Particle);
		Particle->SetOwner(this);
		Particle->Begin();
	}
}

void sEmitter::RemoveParticle(ParticleBase* Particle)
{
	//if (std::find(Particles.begin(), Particles.end(), Particle) != Particles.end())
	//	Particles.erase(std::remove(Particles.begin(), Particles.end(), Particle), Particles.end());

	auto erased = std::erase_if(Particles, [&Particle](const std::shared_ptr<ParticleBase>& pParticle) { return Particle == pParticle.get(); });
	if (erased > 0)
	{

	}
}

void sEmitter::RemoveParticle(std::size_t Index)
{
	if (Index < Particles.size())
		Particles.erase(Particles.begin() + Index);
}

ParticleBase* sEmitter::GetParticle(std::size_t Index) const
{
	if (Index < Particles.size())
		return Particles[Index].get();
	return nullptr;
}

void sEmitter::SetLocation(FVector Loc)
{
	Location = Loc;
	for (const auto& Particle : Particles)
		Particle->OnUpdateTransform();
}

void sEmitter::SetRotation(FAngles Rot)
{
	Rotation = Rot;
	for (const auto& Particle : Particles)
		Particle->OnUpdateTransform();
}

void sEmitter::SetScale(FVector InScale)
{
	Scale = InScale;
	for (const auto& Particle : Particles)
		Particle->OnUpdateTransform();
}

ParticleBase::ParticleBase(sEmitter* InOwner)
	: Owner(InOwner)
	, bIsActive(true)
{
}

ParticleBase::~ParticleBase()
{
	Owner = nullptr;
}

void ParticleBase::SetOwner(sEmitter* InOwner)
{
	Owner = InOwner;
}

void ParticleBase::SetActive(bool value)
{
	bIsActive = value;
}

MeshParticle::MeshParticle(sMeshParticleDesc InDesc, sEmitter* InOwner)
	: Super(InOwner)
	, Desc(InDesc)
	, Time(1.0f)
	, SpawnCounter(0)
	, bIsUpdated(false)
	, MaterialInstance(nullptr)
	//, bIsInstanced(true)
{
	ParticlePool.resize((Desc.SpawnRate * Desc.MaxLifeTime + 2));

	{
		BufferLayout Layout;
		Layout.Size = Desc.Shape.Shape.size() * sizeof(sParticleVertexLayout);
		Layout.Stride = sizeof(sParticleVertexLayout);
		VertexBuffer = IVertexBuffer::CreateUnique("MeshParticle_VB", Layout);
	}
	{
		BufferLayout Layout;
		Layout.Size = Desc.Shape.ShapeIndexes.size() * sizeof(std::uint32_t);
		Layout.Stride = sizeof(std::uint32_t);
		IndexBuffer = IIndexBuffer::CreateUnique("MeshParticle_IB", Layout);
	}
	{
		{
			BufferSubresource VertexResource;
			VertexResource.pSysMem = Desc.Shape.Shape.data();
			VertexResource.Size = Desc.Shape.Shape.size() * sizeof(sParticleVertexLayout);
			VertexResource.Location = 0;
			VertexBuffer->UpdateSubresource(&VertexResource);
		}
		{
			BufferSubresource IndexResource;
			IndexResource.pSysMem = Desc.Shape.ShapeIndexes.data();
			IndexResource.Size = Desc.Shape.ShapeIndexes.size() * sizeof(std::uint32_t);
			IndexResource.Location = 0;
			IndexBuffer->UpdateSubresource(&IndexResource);
		}
	}

	ObjectDrawParameters.IndexCountPerInstance = Desc.Shape.ShapeIndexes.size();
	ObjectDrawParameters.InstanceCount = (Desc.SpawnRate * Desc.MaxLifeTime + 2);

	{
		BufferLayout Layout;
		Layout.Size = Desc.Shape.Shape.size() * (Desc.SpawnRate * Desc.MaxLifeTime + 2) * sizeof(sParticleVertexLayout::sParticleInstanceLayout);
		Layout.Stride = sizeof(sParticleVertexLayout::sParticleInstanceLayout);
		InstanceBuffer = IVertexBuffer::CreateUnique("MeshInstanceParticle_VB", Layout);
	}
}

MeshParticle::~MeshParticle()
{
	MaterialInstance = nullptr;
	VertexBuffer = nullptr;
	InstanceBuffer = nullptr;
	IndexBuffer = nullptr;

	ParticlePool.clear();

	ParticleBufferVertexes.clear();
	//Indexes.clear();
}

void MeshParticle::Begin()
{
}

void MeshParticle::Update(float DT)
{
	bIsUpdated = false;
	if (!IsActive())
		return;

	Time += DT;
	if (Time >= 4.0f / Desc.SpawnRate)
	{
		Time = 0.0f;

		{
			auto& particle = ParticlePool[SpawnCounter];
			{
				if (!particle.Active)
				{
					particle.Active = true;
					//particle.Shape = Desc.Shapes[0];
					particle.Shape = Desc.Shape;
					auto Owner = GetOwner();
					particle.Position = Owner ? Owner->GetLocation() : FVector::Zero();

					particle.Velocity = Lerp(Desc.MinVelocity, Desc.MaxVelocity, FVector(Engine::RandomValueInRange(0.0f, 1.0f)));
					particle.Color = Desc.StartColor;
					{
						const float Roll = Lerp(Desc.MinAngle.Roll, Desc.MaxAngle.Roll, (Engine::RandomValueInRange(0.0f, 1.0f)));
						const float Yaw = Lerp(Desc.MinAngle.Yaw, Desc.MaxAngle.Yaw, (Engine::RandomValueInRange(0.0f, 1.0f)));
						const float Pitch = Lerp(Desc.MinAngle.Pitch, Desc.MaxAngle.Pitch, (Engine::RandomValueInRange(0.0f, 1.0f)));
						particle.Rotation = FAngles(Pitch, Yaw, Roll);
					}
					particle.Scale = Lerp(Desc.MinScale, Desc.MaxScale, FVector(Engine::RandomValueInRange(0.0f, 1.0f)));

					particle.LifeTime = Lerp(Desc.MinLifeTime, Desc.MaxLifeTime, (Engine::RandomValueInRange(0.0f, 1.0f)));
					particle.LifeRemaining = particle.LifeTime;
				}
			}
			if ((SpawnCounter + 1) >= ParticlePool.size())
				SpawnCounter = 0;
			else
				SpawnCounter += 1;
		}
	}

	//std::int32_t ActiveParticleCount = 0;
	{
		for (std::size_t i = 0; i < ParticlePool.size(); i++)
		{
			auto& particle = ParticlePool[i];

			{
				if (!particle.Active)
					continue;

				if (particle.LifeRemaining <= 0.0f)
				{
					particle.Active = false;
					continue;
				}

				particle.LifeRemaining -= DT;
				particle.Position += particle.Velocity * DT;
				//ActiveParticleCount++;
				bIsUpdated = true;

				float Alpha = (particle.LifeTime - particle.LifeRemaining) / particle.LifeTime;
				{
					const float R = Lerp(Desc.StartColor.R, Desc.EndColor.R, Alpha);
					const float G = Lerp(Desc.StartColor.G, Desc.EndColor.G, Alpha);
					const float B = Lerp(Desc.StartColor.B, Desc.EndColor.B, Alpha);
					const float A = Lerp(Desc.StartColor.A, Desc.EndColor.A, Alpha);
					particle.Color = FColor(R, G, B, A);
				}
			}
		}
	}

	{
		if (bIsUpdated)
		{
			ParticleBufferVertexes.clear();
			//for (std::size_t i = 0; i < ParticlePool.size(); i++)
			//for (std::size_t i = ParticlePool.size(); i-- > 0; )
			//{
			//	const auto& Particle = ParticlePool[i];
			//	if (Particle.Active)
			//	{
			//		for (const auto& Shape : Particle.Shape.Shape)
			//		{
			//			ParticleBufferVertexes.push_back(sParticleVertexBufferEntry(Shape.position + Particle.Position, Shape.texCoord, Particle.Color));
			//		}
			//	}
			//	else
			//	{
			//		for (const auto& Shape : Particle.Shape.Shape)
			//		{
			//			//ParticleBufferVertexes.push_back(sParticleVertexBufferEntry(FVector::Zero(), FVector2D::Zero()));
			//		}
			//	}
			//}

			//for (std::size_t i = 0; i < ParticlePool.size(); i++)
			for (std::size_t i = ParticlePool.size(); i-- > 0; )
			{
				const auto& Particle = ParticlePool[i];
				if (Particle.Active)
				{
					ParticleBufferVertexes.push_back(sParticleVertexLayout::sParticleInstanceLayout(Particle.Position, Particle.Color));
				}
				else
				{
					for (const auto& Shape : Particle.Shape.Shape)
					{
						//ParticleBufferVertexes.push_back(sParticleVertexBufferEntry(FVector::Zero(), FVector2D::Zero()));
					}
				}
			}
		}

		{
			//Indexes.clear();
			//Indexes = MeshPrimitives::GeneratePlaneIndices(ActiveParticleCount);

			//ObjectDrawParameters.IndexCountPerInstance = Indexes.size();
		}
	}
}

void MeshParticle::OnUpdateTransform()
{
	InstanceBuffer = nullptr;
}
