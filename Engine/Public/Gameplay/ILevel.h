#pragma once

#include "IWorld.h"

class sActor;
class sMesh;
class sStaticMesh;
class sMaterial;
class sActorComponent;
class sMeshComponent;

struct sPlayerSpawn
{
	std::size_t PlayerIndex = 0;
	FVector Location = FVector::Zero();
	std::size_t LayerIndex = 0;

	sPlayerSpawn() = default;
	sPlayerSpawn(std::size_t PlayerIdx, FVector Loc, std::size_t Layer)
		: PlayerIndex(PlayerIdx)
		, Location(Loc)
		, LayerIndex(Layer)
	{}
};

class ILevel
{
	sBaseClassBody(sClassDefaultProtectedConstructor, ILevel)
public:
	virtual std::string GetName() const = 0;

	virtual void InitLevel() = 0;

	virtual void BeginPlay() = 0;
	virtual void Tick(const double DeltaTime) = 0;
	virtual void FixedUpdate(const double DeltaTime) = 0;

	virtual void PauseLayer(std::size_t LayerIndex, bool Pause) {}
	virtual void Reset() {}

	virtual FBoundingBox GetLevelBounds() const = 0;

	virtual IWorld* GetWorld() const = 0;
	
	virtual void Serialize() = 0;

	virtual void AddMesh(const std::shared_ptr<sMesh>& Object, std::size_t LayerIndex = 0) = 0;
	virtual void RemoveMesh(sMesh* Object, std::size_t LayerIndex = 0) = 0;
	virtual void AddActor(const std::shared_ptr<sActor>& Object, std::size_t LayerIndex = 0) = 0;
	virtual void RemoveActor(sActor* Object, std::size_t LayerIndex = 0, bool bDeferredRemove = true) = 0;

	virtual void SpawnPlayerActor(const std::shared_ptr<sActor>& Actor, std::size_t PlayerIndex) = 0;

	virtual size_t LayerCount() const { return 1; }

	virtual size_t MeshCount(std::size_t LayerIndex = 0) const = 0;
	virtual	std::vector<std::shared_ptr<sMesh>> GetAllMeshes(std::size_t LayerIndex = 0) const = 0;
	virtual sMesh* GetMesh(const std::size_t Index, std::size_t LayerIndex = 0) const = 0;
	virtual size_t ActorCount(std::size_t LayerIndex = 0) const = 0;
	virtual	std::vector<std::shared_ptr<sActor>> GetAllActors(std::size_t LayerIndex = 0) const = 0;
	virtual sActor* GetActor(const std::size_t Index, std::size_t LayerIndex = 0) const = 0;

	virtual sActor* GetPlayerFocusedActor(std::size_t Index) const = 0;

	virtual void OnResizeWindow(const std::size_t Width, const std::size_t Height) = 0;

	virtual void InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar) = 0;
};
