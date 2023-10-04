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

#include "IPhysicalWorld.h"
#include <btBulletDynamicsCommon.h>
#include <BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include <BulletSoftBody/btDefaultSoftBodySolver.h>
#include <BulletSoftBody/btSoftBodyHelpers.h>
#include <BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h>
#include <BulletCollision/CollisionShapes/btConvex2dShape.h>
#include <BulletCollision/CollisionShapes/btBox2dShape.h>
#include <BulletCollision/CollisionDispatch/btBox2dBox2dCollisionAlgorithm.h>
#include <BulletCollision/CollisionDispatch/btConvex2dConvex2dAlgorithm.h>
#include <BulletCollision/NarrowPhaseCollision/btMinkowskiPenetrationDepthSolver.h>
#include <BulletCollision/NarrowPhaseCollision/btRaycastCallback.h>

class BulletWorld : public IPhysicalWorld
{
	sClassBody(sClassConstructor, BulletWorld, IPhysicalWorld)
public:
	BulletWorld(const float PhysicalWorldScale = 0.05f);
	virtual ~BulletWorld();

	virtual void BeginPlay() override final;
	virtual void Tick(const double InDeltaTime) override final;

	virtual std::size_t GetBodyCount() const override final;
	virtual sPhysicalComponent* GetPhysicalBody(std::size_t Index) const override final;
	virtual IRigidBody* GetBody(std::size_t Index) const override final;

	virtual FVector GetGravity() const override final;
	virtual void SetGravity(const FVector& Gravity) override final;
	virtual void SetWorldOrigin(const FVector& newOrigin) override final;

	virtual sPhysicalComponent* LineTraceToViewPort(const FVector& InOrigin, const FVector& InDirection) const override final;
	virtual std::vector<sPhysicalComponent*> QueryAABB(const FBoundingBox& Bounds) const override final;

	virtual void SetPhysicsInternalTick(std::optional<double> Tick) override final;
	virtual std::optional<double> GetPhysicsInternalTick() const override final { return InternalTick; }
	virtual EPhysicsEngine GetPhysicsEngineType() const override final { return EPhysicsEngine::eBulletPhysics; }

	virtual float GetPhysicalWorldScale() const override final { return PhysicalWorldScale; }

	virtual IRigidBody::SharedPtr Create2DBoxBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FBounds2D& Bounds) const override final;
	virtual IRigidBody::SharedPtr Create2DPolygonBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, const std::array<FVector2, 8>& points) const override final;
	virtual IRigidBody::SharedPtr Create2DCircleBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, float InRadius) const override final;
	virtual IRigidBody::SharedPtr Create2DEdgeBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, const std::array<FVector2, 4>& points, bool OneSided) const override final;
	virtual IRigidBody::SharedPtr Create2DChainBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, const std::vector<FVector2>& vertices) const override final;
	virtual IRigidBody::SharedPtr Create2DChainBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, const std::vector<FVector2>& vertices, const FVector2& prevVertex, const FVector2& nextVertex) const override final;

	virtual IRigidBody::SharedPtr CreateBoxBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, FVector InHalf) const override final;
	virtual IRigidBody::SharedPtr CreateSphereBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, float InRadius) const override final;
	virtual IRigidBody::SharedPtr CreateCapsuleBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, float InRadius, float InHeight) const override final;
	virtual IRigidBody::SharedPtr CreateCylinderBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, float InRadius, float InHeight) const override final;
	virtual IRigidBody::SharedPtr CreateConeBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, float InRadius, float InHeight) const override final;

	virtual IRigidBody::SharedPtr CreateMultiBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, FVector InInertia, float InMass) const override final;
	virtual IRigidBody::SharedPtr CreateConvexHullBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, const float* points, int numPoints, int stride) const override final;
	virtual IRigidBody::SharedPtr CreateTriangleMesh(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, const FVector* points, int numPoints, const std::uint32_t* indices, int numIndices) const override final;
	
	//btSoftRigidDynamicsWorld* GetWorld() { return World.get(); };
	btDiscreteDynamicsWorld* GetWorld() const { return World.get(); };

	void DestroyBody(btRigidBody* Body);

private:
	void DeferredDestroy();

private:
	std::unique_ptr<btDiscreteDynamicsWorld> World;
	//std::unique_ptr<btSoftRigidDynamicsWorld> World;
	std::unique_ptr<btCollisionDispatcher> dispatcher;
	std::unique_ptr<btCollisionConfiguration> collisionConfig;
	std::unique_ptr<btBroadphaseInterface> broadphase;
	std::unique_ptr<btConstraintSolver> solver;
	//std::unique_ptr<btSoftBodySolver> softbodySolver;

	std::unique_ptr<btConvex2dConvex2dAlgorithm::CreateFunc> m_convexAlgo2d;
	std::unique_ptr<btVoronoiSimplexSolver> m_simplexSolver;
	std::unique_ptr<btMinkowskiPenetrationDepthSolver> m_pdSolver;
	std::unique_ptr<btBox2dBox2dCollisionAlgorithm::CreateFunc> m_box2dbox2dAlgo;

	std::optional<double> InternalTick;

	float PhysicalWorldScale;

	std::vector<btRigidBody*> DeferredDestroyBodyList;
};
