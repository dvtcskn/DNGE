/* ---------------------------------------------------------------------------------------
* MIT License
*
* Copyright (c) 2023 Davut Co�kun.
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

#include <vector>
#include "IPhysicalWorld.h"
#include "Gameplay/Actor.h"
#include <box2d/box2d.h>

const int32 k_maxContactPoints = 2048;

struct ContactPoint
{
	b2Fixture* fixtureA;
	b2Fixture* fixtureB;
	b2Vec2 normal;
	b2Vec2 position;
	b2PointState state;
	float normalImpulse;
	float tangentImpulse;
	float separation;
};

class sWorld2D : public IPhysicalWorld, public b2ContactListener
{
	sClassBody(sClassConstructor, sWorld2D, IPhysicalWorld)
private:
	// This is called when a joint in the world is implicitly destroyed
	// because an attached body is destroyed. This gives us a chance to
	// nullify the mouse joint.
	class DestructionListener : public b2DestructionListener
	{
	public:
		void SayGoodbye(b2Fixture* fixture) override;
		void SayGoodbye(b2Joint* joint) override;

		sWorld2D* World2D;
	};
public:
	sWorld2D(const float PhysicalWorldScale = 0.05f);
	virtual ~sWorld2D();

	virtual void BeginPlay() override final {}
	virtual void Tick(const double InDeltaTime) override final;

	// Let derived tests know that a joint was destroyed.
	virtual void JointDestroyed(b2Joint* joint) { B2_NOT_USED(joint); }

	// Callbacks for derived classes.
	virtual void BeginContact(b2Contact* contact)  override;
	virtual void EndContact(b2Contact* contact)  override;
	virtual void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
	virtual void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;

	virtual FVector GetGravity() const override final;
	virtual void SetGravity(const FVector& Gravity) override final;
	virtual void SetWorldOrigin(const FVector& newOrigin) override final;

	void DestroyBody(b2Body* Body);

	void SetAllowSleeping(const bool val) { m_world->SetAllowSleeping(val); }
	void SetWarmStarting(const bool val) { m_world->SetWarmStarting(val); }
	void SetContinuousPhysics(const bool val) { m_world->SetContinuousPhysics(val); }
	void SetSubStepping(const bool val) { m_world->SetSubStepping(val); }

	void SetVelocityIterations(int32 VelocityIterations) { m_velocityIterations = VelocityIterations; }
	void SetPositionIterations(int32 PositionIterations) { m_positionIterations = PositionIterations; }

	virtual sPhysicalComponent* LineTraceToViewPort(const FVector& InOrigin, const FVector& InDirection) const override final;
	virtual std::vector<sPhysicalComponent*> QueryAABB(const FBoundingBox& Bounds) const override final;

	virtual std::size_t GetBodyCount() const override final;
	virtual sPhysicalComponent* GetPhysicalBody(std::size_t Index) const override final;
	virtual IRigidBody* GetBody(std::size_t Index) const override final;

	virtual void SetPhysicsInternalTick(std::optional<double> Tick) override final;
	virtual std::optional<double> GetPhysicsInternalTick() const override final { return InternalTick; }
	virtual EPhysicsEngine GetPhysicsEngineType() const override final { return EPhysicsEngine::eBox2D; }

	virtual float GetPhysicalWorldScale() const override final { return PhysicalWorldScale; }

	virtual IRigidBody::SharedPtr Create2DBoxBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FBounds2D& Bounds) override final;
	virtual IRigidBody::SharedPtr Create2DPolygonBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, const std::array<FVector2, 8>& points) override final;
	virtual IRigidBody::SharedPtr Create2DCircleBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, float InRadius) override final;
	virtual IRigidBody::SharedPtr Create2DEdgeBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, const std::array<FVector2, 4>& points, bool OneSided) override final;
	virtual IRigidBody::SharedPtr Create2DChainBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, const std::vector<FVector2>& vertices) override final;
	virtual IRigidBody::SharedPtr Create2DChainBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, const std::vector<FVector2>& vertices, const FVector2& prevVertex, const FVector2& nextVertex) override final;

	virtual IRigidBody::SharedPtr CreateBoxBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, FVector InHalf) override final;
	virtual IRigidBody::SharedPtr CreateSphereBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, float InRadius) override final;
	virtual IRigidBody::SharedPtr CreateCapsuleBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, float InRadius, float InHeight) override final;
	virtual IRigidBody::SharedPtr CreateCylinderBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, float InRadius, float InHeight) override final;
	virtual IRigidBody::SharedPtr CreateConeBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, float InRadius, float InHeight) override final;

	virtual IRigidBody::SharedPtr CreateMultiBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, FVector InInertia, float InMass) override final;
	virtual IRigidBody::SharedPtr CreateConvexHullBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, const float* points, int numPoints, int stride) override final;
	virtual IRigidBody::SharedPtr CreateTriangleMesh(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, const FVector* points, int numPoints, const std::uint32_t* indices, int numIndices) override final;

private:
	void DeferredDestroy();

private:
	friend class DestructionListener;
	friend class BoundaryListener;
	friend class ContactListener;

	b2World* m_world;
	DestructionListener m_destructionListener;

	ContactPoint m_points[k_maxContactPoints];
	int32 m_pointCount;

	int32 m_velocityIterations;
	int32 m_positionIterations;

	std::optional<double> InternalTick;

	float PhysicalWorldScale;

	std::vector<b2Body*> DeferredDestroyBodyList;
};
