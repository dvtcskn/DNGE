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
#include "Core/Transform.h"
#include "Core/Archive.h"
#include "AbstractGI/Material.h"
#include "Core/Math/CoreMath.h"
#include "PrimitiveComponent.h"
#include "Engine/AbstractEngine.h"
#include "ILevel.h"

class sController;

class sActor : public std::enable_shared_from_this<sActor>
{
	sBaseClassBody(sClassConstructor, sActor)
public:
	sActor(std::string InName = "", sController* InController = nullptr);
	virtual ~sActor();

	inline std::string GetTag(const std::size_t i = 0) const { return Tags.at(i); }
	inline std::vector<std::string> GetTags() const { return Tags; }
	void AddTag(const std::string& InTag, const std::optional<std::size_t> i = std::nullopt);
	void SetTag(const std::size_t i, const std::string& InTag);
	inline void RemoveTag(const std::size_t i) { Tags.erase(Tags.begin() + i); }
	bool HasTag(std::string InTag);

	void BeginPlay();
	void Tick(const double DeltaTime);
	void FixedUpdate(const double DeltaTime);

	inline void SetName(std::string InName) { Name = InName; };
	inline std::string GetName() const { return Name; };
	virtual std::string GetClassNetworkAddress() const;

	FBoundingBox GetBounds() const;
	FVector GetLocation() const;
	FVector GetScale() const;
	FQuaternion GetRotation() const;
	FAngles GetRotationAngles() const;
	void SetLocation(FVector InLocation);
	void SetRollPitchYaw(FAngles RPY);
	void SetScale(FVector InScale);
	void SetScale(float InScale);
	virtual FVector GetVelocity() const;

	virtual void Replicate(bool bReplicate);
	inline bool IsReplicated() const { return bIsReplicated; }
	virtual eNetworkRole GetNetworkRole() const;

	void Hide(bool value);
	void SetEnabled(bool value);

	inline bool IsEnabled() const { return bIsEnabled; }
	inline bool IsHidden() const { return bIsHidden; }

	void SetRootComponent(const sPrimitiveComponent::SharedPtr& InComponent);
	inline sPrimitiveComponent* GetRootComponent() const { return RootComponent.get(); };
	template<typename T>
	inline T* GetRootComponent() const { return static_cast<T*>(RootComponent.get()); };

	template<class T>
	inline T* FindComponent() const
	{
		if (auto Component = dynamic_cast<T*>(RootComponent.get()))
			return Component;
		return RootComponent->FindComponent<T>();
	}
	template<class T>
	inline std::vector<T*> FindComponents() const
	{
		std::vector<T*> Components;
		if (auto Component = dynamic_cast<T*>(RootComponent.get()))
			Components.push_back(Component);
		std::vector<T*> Temp = RootComponent->FindComponents<T>();
		Components.insert(Components.end(), Temp.begin(), Temp.end());
		return Components;
	}

	bool AddToLevel(ILevel* pLevel, FVector SpawnLocation, std::size_t LayerIndex = 0);
	bool AddToLevel(std::string Level, FVector SpawnLocation, std::size_t LayerIndex = 0);
	bool AddToActiveLevel(FVector SpawnLocation, std::size_t LayerIndex = 0);
	inline ILevel* GetOwnedLevel() const { return Level; };
	inline std::size_t GetLayerIndex() const { return LayerIndex; };
	void RemoveFromLevel(bool bDeferredRemove = true);

	inline bool HasController() const { return Controller != nullptr; }
	inline sController* GetController() const { return Controller; }
	template<typename T>
	inline T* GetController() const { return static_cast<T*>(Controller); }

	void Possess(sController* Controller);
	void UnPossess();

	virtual void Serialize(sArchive& archive);

	virtual void InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar) { };

private:
	virtual void OnBeginPlay() {}
	virtual void OnTick(const double DeltaTime) {}
	virtual void OnFixedUpdate(const double DeltaTime) {}

	virtual void OnEnabled() {}
	virtual void OnDisabled() {}
	virtual void OnHidden() {}
	virtual void OnVisible() {}

	virtual void OnTransformUpdated() {};

	void AddToLevel_Server(FVector SpawnLocation, std::size_t LayerIndex);
	void AddToActiveLevel_Server(FVector SpawnLocation, std::size_t LayerIndex);
	void AddToNamedLevel_Server(std::string Level, FVector SpawnLocation, std::size_t LayerIndex);

private:
	sPrimitiveComponent::SharedPtr RootComponent;
	std::string Name;

	std::vector<std::string> Tags;

	ILevel* Level;
	std::size_t LayerIndex;

	sController* Controller;

	/*FVector Location;
	FVector4 Rotation;
	FVector Scale;*/

	bool bIsHidden;
	bool bIsEnabled;

	bool bIsReplicated;
};
