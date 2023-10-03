#pragma once

#include <memory>
#include <vector>

#ifndef PhysX5Enabled
#define PhysX5Enabled 1
#endif

#if PhysX5Enabled

#include "ClassBody.h"
#include "IPhysicalWorld.h"
#include <PxPhysicsAPI.h>
#include <extensions/PxExtensionsAPI.h>
#include <PxShape.h>
#include <foundation/PxString.h>
#include <foundation/PxThread.h>
#include "Utilities/Utilities.h"

class PhysXWorld : public IPhysicalWorld
{
	sClassBody(sClassConstructor, PhysXWorld, IPhysicalWorld)
	class DeletionListener : public physx::PxDeletionListener
	{
	public:
		DeletionListener(PhysXWorld* InParent)
			: Parent(InParent)
		{}

		~DeletionListener() 
		{
			Parent = nullptr;
		}

		virtual void onRelease(const physx::PxBase* observed, void* userData, physx::PxDeletionEventFlag::Enum deletionEvent)
		{
			PX_UNUSED(userData);
			PX_UNUSED(deletionEvent);

			if (observed->is<physx::PxRigidActor>())
			{
				const physx::PxRigidActor* actor = static_cast<const physx::PxRigidActor*>(observed);

				//removeRenderActorsFromPhysicsActor(actor);

				std::vector<physx::PxRigidActor*>::iterator actorIter = std::find(Parent->mPhysicsActors.begin(), Parent->mPhysicsActors.end(), actor);
				if (actorIter != Parent->mPhysicsActors.end())
				{
					Parent->mPhysicsActors.erase(actorIter);
				}

			}
		};

	private:
		friend PhysXWorld;
		PhysXWorld* Parent;
	};

	class PhysXErrorCallback : public physx::PxErrorCallback
	{
	public:
		PhysXErrorCallback() {}
		~PhysXErrorCallback() {}

		virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line)
		{
			if (code == physx::PxErrorCode::eDEBUG_INFO)
			{
				char buffer[1024];
				sprintf_s(buffer, "%s\n", message);
				physx::PxPrintString(buffer);

			}
			else
			{
				const char* errorCode = NULL;

				switch (code)
				{
				case physx::PxErrorCode::eINVALID_PARAMETER:
					errorCode = "invalid parameter";
					break;
				case physx::PxErrorCode::eINVALID_OPERATION:
					errorCode = "invalid operation";
					break;
				case physx::PxErrorCode::eOUT_OF_MEMORY:
					errorCode = "out of memory";
					break;
				case physx::PxErrorCode::eDEBUG_INFO:
					errorCode = "info";
					break;
				case physx::PxErrorCode::eDEBUG_WARNING:
					errorCode = "warning";
					break;
				case physx::PxErrorCode::ePERF_WARNING:
					errorCode = "performance warning";
					break;
				case physx::PxErrorCode::eABORT:
					errorCode = "abort";
					break;
				default:
					errorCode = "unknown error";
					break;
				}

				PX_ASSERT(errorCode);
				if (errorCode)
				{
					char buffer[1024];
					sprintf_s(buffer, "%s (%d) : %s : %s\n", file, line, errorCode, message);

					physx::PxPrintString(buffer);

					// in debug builds halt execution for abort codes
					PX_ASSERT(code != PxErrorCode::eABORT);

					// in release builds we also want to halt execution 
					// and make sure that the error message is flushed  
					while (code == physx::PxErrorCode::eABORT)
					{
						physx::PxPrintString(buffer);
						physx::PxThread::sleep(1000);
					}
				}
			}
		}
	};

	class SimulationEventCallback : public physx::PxSimulationEventCallback
	{
	public:
		SimulationEventCallback() = default;
		~SimulationEventCallback() = default;

		void onContactNotify(unsigned int arraySizes, void** shape0Array, void** shape1Array, void** actor0Array, void** actor1Array, float* positionArray, float* normalArray) {};
		void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) { PX_UNUSED((constraints, count)); };
		void onWake(physx::PxActor** actors, physx::PxU32 count) { PX_UNUSED((actors, count)); };
		void onSleep(physx::PxActor** actors, physx::PxU32 count) { PX_UNUSED((actors, count)); };
		void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) { PX_UNUSED((pairs, count)); };
		void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) {};
		void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) { PX_UNUSED(bodyBuffer);  PX_UNUSED(poseBuffer);  PX_UNUSED(count); };
	};


private:
	physx::PxFoundation* mFoundation;
	physx::PxPhysics* mPhysics;
	physx::PxCooking* mCooking;
	physx::PxScene* mScene;
	physx::PxDefaultCpuDispatcher* mCpuDispatcher;
	physx::PxPvd* mPvd;
	physx::PxPvdTransport* mTransport;
	physx::PxPvdInstrumentationFlags mPvdFlags;

	physx::PxDefaultErrorCallback ErrorCallBack;

	std::vector<physx::PxRigidActor*> mPhysicsActors;

#if PX_SUPPORT_GPU_PHYSX
	physx::PxCudaContextManager* mCudaContextManager;
#endif

	std::unique_ptr<DeletionListener> pDeletionListener;
	std::unique_ptr<PhysXErrorCallback> pPhysXErrorCallback;
	std::unique_ptr<SimulationEventCallback> pSimulationEventCallback;
	physx::PxDefaultAllocator Allocator;

	bool isPhysXSimulating;

	std::optional<double> InternalTick;

	float PhysicalWorldScale;

private:
	physx::PxRigidDynamic* createDynamic(physx::PxMaterial& Material, const physx::PxTransform& t, const physx::PxGeometry& geometry, const physx::PxVec3& velocity = physx::PxVec3(0));
	
public:
	PhysXWorld(const float PhysicalWorldScale = 0.05f);
	virtual ~PhysXWorld();

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
	virtual EPhysicsEngine GetPhysicsEngineType() const override final { return EPhysicsEngine::ePhysX; }

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

	//btDiscreteDynamicsWorld* GetWorld() { return World.get(); };
	void WaitForSim();

	void RemoveBody(physx::PxRigidActor* InBody);
};
#endif
