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

#include "IWorld.h"

class sActor;
class sMesh;
class sStaticMesh;
class sMaterial;
class sActorComponent;
class sMeshComponent;

struct sObjectSpawnNode
{
	std::string Name = "";
	std::int32_t PlayerIndex = -1;
	FVector Location = FVector::Zero();
	std::size_t LayerIndex = 0;

	sObjectSpawnNode() = default;
	sObjectSpawnNode(std::string InName, std::int32_t PlayerIdx, FVector Loc, std::size_t Layer)
		: Name(InName)
		, Location(Loc)
		, LayerIndex(Layer)
		, PlayerIndex(PlayerIdx)
	{}
};

class ILevel
{
	sBaseClassBody(sClassDefaultProtectedConstructor, ILevel)
public:
	virtual std::string GetName() const = 0;

	virtual void InitLevel() = 0;
	virtual void DestroyLevel() {}

	virtual void BeginPlay() = 0;
	virtual void Tick(const double DeltaTime) = 0;
	virtual void FixedUpdate(const double DeltaTime) = 0;

	virtual void PauseLayer(std::size_t LayerIndex, bool Pause) {}
	virtual void Reset() {}

	virtual FBoundingBox GetLevelBounds() const = 0;

	virtual IWorld* GetWorld() const = 0;
	
	virtual void Serialize() = 0;

	virtual sObjectSpawnNode GetSpawnNode(std::string Name, std::int32_t PlayerIndex = -1) const = 0;

	virtual void AddMesh(const std::shared_ptr<sMesh>& Object, std::size_t LayerIndex = 0) = 0;
	virtual void RemoveMesh(sMesh* Object, std::size_t LayerIndex = 0) = 0;
	virtual void AddActor(const std::shared_ptr<sActor>& Object, std::size_t LayerIndex = 0) = 0;
	virtual void RemoveActor(sActor* Object, std::size_t LayerIndex = 0, bool bDeferredRemove = true) = 0;

	virtual size_t LayerCount() const { return 1; }

	virtual size_t MeshCount(std::size_t LayerIndex = 0) const = 0;
	virtual	std::vector<std::shared_ptr<sMesh>> GetAllMeshes(std::size_t LayerIndex = 0) const = 0;
	virtual sMesh* GetMesh(const std::size_t Index, std::size_t LayerIndex = 0) const = 0;
	virtual size_t ActorCount(std::size_t LayerIndex = 0) const = 0;
	virtual	std::vector<std::shared_ptr<sActor>> GetAllActors(std::size_t LayerIndex = 0) const = 0;
	virtual sActor* GetActor(const std::size_t Index, std::size_t LayerIndex = 0) const = 0;

	virtual void OnResizeWindow(const std::size_t Width, const std::size_t Height) = 0;

	virtual void OnConnectedToServer() {}
	virtual void OnDisconnected() {}

	virtual void InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar) = 0;
};
