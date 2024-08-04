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

#include "Engine/ClassBody.h"
#include "Engine/AbstractEngine.h"
#include "Core/Math/CoreMath.h"
#include "AbstractGI/Material.h"
#include <random>

class sEmitter;

class ParticleBase
{
	sBaseClassBody(sClassNoDefaults, ParticleBase)
protected:
	ParticleBase(sEmitter* InOwner = nullptr);

public:
	virtual ~ParticleBase();

	void SetOwner(sEmitter* Owner);
	sEmitter* GetOwner() const { return Owner; }

	std::string GetName() const { return Name; }
	void SetName(std::string InName) { Name = InName; }
	
	void SetActive(bool value);
	bool IsActive() const { return bIsActive; }

	virtual void Begin() = 0;
	virtual void Update(float DT) = 0;
	virtual void OnUpdateTransform() = 0;

	EParticleType GetParticleType() const { return EParticleType::eCPU; }

private:
	std::string Name;
	sEmitter* Owner;
	bool bIsActive;
};

struct sParticleShape
{
	std::vector<sParticleVertexLayout> Shape;
	std::vector<std::uint32_t> ShapeIndexes;

	sParticleShape()
	{}
};

struct sParticleDesc
{
	std::uint32_t SpawnRate;
	float MinLifeTime;
	float MaxLifeTime;

	FColor StartColor;
	FColor EndColor;
	FVector MinVelocity;
	FVector MaxVelocity;
	FAngles MinAngle;
	FAngles MaxAngle;
	FVector MinScale;
	FVector MaxScale;

	sParticleDesc()
		: SpawnRate(1)
		, MinLifeTime(1.0f)
		, MaxLifeTime(1.0f)
		, StartColor(FColor::White())
		, EndColor(FColor::White())
		, MinVelocity(FVector(-1.0f, -1.0f, -1.0f))
		, MaxVelocity(FVector(1.0f, 1.0f, 1.0f))
		, MinAngle(FAngles())
		, MaxAngle(FAngles())
		, MinScale(FVector::One())
		, MaxScale(FVector::One())
	{}

	inline void SetLifeTime(float Life) { MinLifeTime = Life; MaxLifeTime = Life; }
	inline void SetColor(FColor Color) { StartColor = Color; EndColor = Color; }
	inline void SetVelocity(FVector Velocity) { MinVelocity = Velocity; MaxVelocity = Velocity; }
	inline void SetAngle(FAngles Angle) { MinAngle = Angle; MaxAngle = Angle; }
	inline void SetScale(FVector Scale) { MinScale = Scale; MaxScale = Scale; }
};

struct sMeshParticleDesc : public sParticleDesc
{
	//std::vector<sParticleShape> Shapes;
	sParticleShape Shape;

	sMeshParticleDesc()
	{}
};

class MeshParticle : public ParticleBase
{
	sClassBody(sClassConstructor, MeshParticle, ParticleBase)
private:
	struct Particle
	{
		sParticleShape Shape;
		FVector Position = FVector(0.0f, 0.0f, 0.0f);
		FVector Velocity = FVector(0.0f, 0.0f, 0.0f);
		FColor Color = FColor::White();
		FAngles Rotation = FAngles();
		FVector Scale = FVector(1.0f, 1.0f, 1.0f);

		float LifeTime = 1.0f;
		float LifeRemaining = 0.0f;

		bool Active = false;
	};

public:
	MeshParticle(sMeshParticleDesc Desc, sEmitter* Owner = nullptr);
	virtual ~MeshParticle();

	virtual void Begin() override;
	virtual void Update(float DT) override;

	//bool IsInstanced() const { return bIsInstanced; }

	std::vector<sParticleVertexLayout::sParticleInstanceLayout> ParticleBufferVertexes;
	//std::vector<std::uint32_t> Indexes;
	bool bIsUpdated;

	sMaterial::sMaterialInstance* MaterialInstance;

	IVertexBuffer::SharedPtr VertexBuffer;
	IVertexBuffer::SharedPtr InstanceBuffer;
	IIndexBuffer::SharedPtr IndexBuffer;
	sObjectDrawParameters ObjectDrawParameters;

private:
	virtual void OnUpdateTransform() override;

private:
	sMeshParticleDesc Desc;

	std::vector<Particle> ParticlePool;
	float Time;
	std::size_t SpawnCounter;
};

class sEmitter
{
	sBaseClassBody(sClassConstructor, sEmitter)
public:
	sEmitter(std::string InName);
	virtual ~sEmitter();

	virtual void BeginPlay();
	virtual void Tick(float DT);

	void AddParticle(const ParticleBase::SharedPtr& Particle);
	void RemoveParticle(ParticleBase* Particle);
	void RemoveParticle(std::size_t Index);

	ParticleBase* GetParticle(std::size_t Index) const;
	inline std::size_t GetParticlesSize() const { return Particles.size(); }

	void SetLocation(FVector Loc);
	void SetRotation(FAngles Rot);
	void SetScale(FVector InScale);

	inline FVector GetLocation() const { return Location; }
	inline FAngles GetRotation() const { return Rotation; }
	inline FVector GetScale() const { return Scale; }

private:
	std::string Name;
	std::vector<ParticleBase::SharedPtr> Particles;

	FVector Location;
	FAngles Rotation;
	FVector Scale;
};
