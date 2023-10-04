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

#include <vector>
#include <Gameplay/ILevel.h>
#include "MetaWorld.h"
#include <AbstractGI/Material.h>
#include <Gameplay/Actor.h>
#include <Gameplay/MeshComponent.h>
#include <Core/Camera.h>
#include <Gameplay/StaticMesh.h>
#include "GPlayerCharacter.h"
#include "GAIController.h"

struct LevelLayer
{
	LevelLayer() = default;
	~LevelLayer()
	{
		DeferredRemove();
		for (auto& Actor : Actors)
		{
			Actor->RemoveFromLevel(false);
			Actor = nullptr;
		}
		Actors.clear();
		for (auto& StaticMesh : Meshes)
			StaticMesh = nullptr;
		Meshes.clear();
	}

	void Release()
	{
		for (auto& Actor : Actors)
		{
			Actor->RemoveFromLevel();
			Actor = nullptr;
		}
		Actors.clear();
		for (auto& StaticMesh : Meshes)
			StaticMesh = nullptr;
		Meshes.clear();
		DeferredRemove();
	}

	inline void BeginPlay() 
	{
		for (const auto& Actor : Actors)
			Actor->BeginPlay();
	}
	inline void Tick(const double DeltaTime)
	{
		DeferredRemove();

		std::vector<sActor::SharedPtr>::iterator it = Actors.begin();
		while (it != Actors.end())
		{
			if ((*it))
			{
				(*it)->Tick(DeltaTime);
				it++;
			}
			else
			{
				it++;
			}
		}
	}
	inline void FixedUpdate(const double DeltaTime)
	{
		std::vector<sActor::SharedPtr>::iterator it = Actors.begin();
		while (it != Actors.end())
		{
			if ((*it))
			{
				(*it)->FixedUpdate(DeltaTime);
				it++;
			}
			else
			{
				it++;
			}
		}
	}

	inline void InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar)
	{
		for (const auto& Actor : Actors)
			Actor->InputProcess(MouseInput, KeyboardChar);
	}

	inline void AddMesh(const std::shared_ptr<sMesh>& Mesh)
	{
		Meshes.push_back(Mesh);
	}
	inline void RemoveMesh(sMesh* Mesh)
	{
		std::vector<sMesh::SharedPtr>::iterator it = Meshes.begin();
		while (it != Meshes.end())
		{
			if ((*it))
			{
				if ((*it).get() == Mesh)
				{
					it = Meshes.erase(it);
					break;
				}
				else
				{
					it++;
				}
			}
			else
			{
				it++;
			}
		}
	}
	inline void AddActor(const std::shared_ptr<sActor>& Object)
	{
		Actors.push_back(Object);
	}
	inline void RemoveActor(sActor* Actor, bool bDeferredRemove)
	{
		if (bDeferredRemove && Actor)
		{
			auto SharedActor = Actor->shared_from_this();
			if (std::find(RemoveActorsByDeferred.begin(), RemoveActorsByDeferred.end(), SharedActor) != RemoveActorsByDeferred.end())
			{
				SharedActor = nullptr;
				return;
			}
			Actor->SetEnabled(false);
			Actor->Hide(true);
			RemoveActorsByDeferred.push_back(SharedActor);
			SharedActor = nullptr;
		}
		else
		{
			std::vector<sActor::SharedPtr>::iterator it = Actors.begin();
			while (it != Actors.end())
			{
				if ((*it))
				{
					if ((*it).get() == Actor)
					{
						sActor::SharedPtr Temp = (*it);
						it = Actors.erase(it);
						Temp->RemoveFromLevel();
						Temp = nullptr;
						break;
					}
					else
					{
						it++;
					}
				}
				else
				{
					it++;
				}
			}
			Actors.shrink_to_fit();
		}
	}

	inline size_t MeshCount() const
	{
		return Meshes.size();
	}
	inline std::vector<std::shared_ptr<sMesh>> GetAllMeshes() const
	{
		return Meshes;
	}
	inline sMesh* GetMesh(const std::size_t Index) const
	{
		return Meshes.at(Index).get();
	}
	inline size_t ActorCount() const
	{
		return Actors.size();
	}
	inline std::vector<std::shared_ptr<sActor>> GetAllActors() const
	{
		return Actors;
	}
	inline sActor* GetActor(const std::size_t Index) const
	{
		return Actors.at(Index).get();
	}

	std::vector<sActor::SharedPtr> Actors;
	std::vector<sMesh::SharedPtr> Meshes;
private:
	void DeferredRemove()
	{
		if (RemoveActorsByDeferred.size() > 0)
		{
			for (auto& Actor : RemoveActorsByDeferred)
			{
				RemoveActor(Actor.get(), false);
				Actor = nullptr;
			}
			RemoveActorsByDeferred.clear();
		}
	}

	std::vector<sActor::SharedPtr> RemoveActorsByDeferred;
};

class sDefaultLevel : public ILevel
{
	sClassBody(sClassConstructor, sDefaultLevel, ILevel)
public:
	sDefaultLevel(IWorld* World, std::string Name = "DefaultLevel");
	virtual ~sDefaultLevel();

	virtual std::string GetName() const override final;

	virtual void InitLevel() override final;

	virtual void BeginPlay() override final;
	virtual void Tick(const double DeltaTime) override final;
	virtual void FixedUpdate(const double DeltaTime) override final;

	virtual IWorld* GetWorld() const override final { return World; }

	virtual FBoundingBox GetLevelBounds() const override final;

	virtual void Serialize() override final;

	virtual void AddMesh(const std::shared_ptr<sMesh>& Object, std::size_t LayerIndex = 0) override final;
	virtual void RemoveMesh(sMesh* Object, std::size_t LayerIndex = 0) override final;
	virtual void AddActor(const std::shared_ptr<sActor>& Object, std::size_t LayerIndex = 0) override final;
	virtual void RemoveActor(sActor* Object, std::size_t LayerIndex = 0, bool bDeferredRemove = true) override final;

	virtual void SpawnPlayerActor(const std::shared_ptr<sActor>& Actor, std::size_t PlayerIndex) override final;

	virtual size_t LayerCount() const override final;

	virtual size_t MeshCount(std::size_t LayerIndex = 0) const override final;
	virtual	std::vector<std::shared_ptr<sMesh>> GetAllMeshes(std::size_t LayerIndex = 0) const override final;
	virtual sMesh* GetMesh(const std::size_t Index, std::size_t LayerIndex = 0) const override final;
	virtual size_t ActorCount(std::size_t LayerIndex = 0) const override final;
	virtual	std::vector<std::shared_ptr<sActor>> GetAllActors(std::size_t LayerIndex = 0) const override final;
	virtual sActor* GetActor(const std::size_t Index, std::size_t LayerIndex = 0) const override final;

	virtual sActor* GetPlayerFocusedActor(std::size_t Index) const override final;

	virtual void OnResizeWindow(const std::size_t Width, const std::size_t Height) override final;

	virtual void Reset();

	virtual void InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar) override final;

private:
	IWorld* World;
	std::string Name;
	std::vector<std::unique_ptr<LevelLayer>> Layers;
	std::vector<sPlayerSpawn> PlayerSpawnLocations;
};
