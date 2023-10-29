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
#include "Core/Math/CoreMath.h"
#include "Core/Archive.h"

class sActor;

class sPrimitiveComponent : public std::enable_shared_from_this<sPrimitiveComponent>
{
	sBaseClassBody(sClassConstructor, sPrimitiveComponent)
	friend class sActor;
public:
	sPrimitiveComponent(std::string InName, sActor* pActor = nullptr)
		: Name(InName)
		, Owner(pActor)
		, bIsHidden(false)
		, bIsEnabled(true) 
		, ComponentOwner(nullptr)
		, Location(FVector::Zero())
		, Rotation(FVector4(0.0f, 0.0f, 0.0f, 1.0f))
		, Scale(FVector(1.0f, 1.0f, 1.0f))
	{}
	virtual ~sPrimitiveComponent()
	{
		ComponentOwner = nullptr;

		for (auto& Child : Children)
			Child = nullptr;
		Children.clear();

		Owner = nullptr;
	}

	inline std::string GetName() const { return Name; };
	inline void SetName(std::string inName) { Name = inName; };

	void BeginPlay();
	void Tick(const double DeltaTime);
	void FixedUpdate(const double DeltaTime);

	inline std::string GetTag(const std::size_t i = 0) const { return Tags.at(i); }
	inline std::vector<std::string> GetTags() const { return Tags; }
	void AddTag(const std::string& InTag, const std::optional<std::size_t> i = std::nullopt);
	void SetTag(const std::size_t i, const std::string& InTag);
	inline void RemoveTag(const std::size_t i) { Tags.erase(Tags.begin() + i); }
	bool HasTag(std::string InTag);

	bool HasOwner() const;
	sActor* GetOwner() const;
	template<class T>
	inline T* GetOwner() const
	{
		return static_cast<T*>(GetOwner());
	}
	bool AttachToActor(sActor* InParent);
	void DetachFromActor();

	bool HasComponentOwner() const { return ComponentOwner; }
	sPrimitiveComponent* GetComponentOwner() const { return ComponentOwner; }
	template<class T>
	inline T* GetComponentOwner() const
	{
		return static_cast<T*>(GetComponentOwner());
	}
	sPrimitiveComponent* GetComponentAttachmentRoot() const;
	template<class T>
	inline T* GetComponentAttachmentRoot() const
	{
		return static_cast<T*>(GetComponentAttachmentRoot());
	}

	template<class T>
	inline T* FindComponent() const
	{
		for (const auto& Child : Children)
		{
			if (auto Component = dynamic_cast<T*>(Child.get()))
			{
				return Component;
			}
			else
			{
				T* Temp = Child->FindComponent<T>();
				if (!Temp)
					continue;
				return Temp;
			}
		}
		return nullptr;
	}
	template<class T>
	inline std::vector<T*> FindComponents() const
	{
		std::vector<T*> Components;
		for (const auto& Child : Children)
		{
			if (auto Component = dynamic_cast<T*>(Child.get()))
			{
				Components.push_back(Component);
				std::vector<T*> Temp = Component->FindComponents<T>();
				if (Temp.size() > 0)
					Components.insert(Components.end(), Temp.begin(), Temp.end());
			}
			else
			{
				std::vector<T*> Temp = Child->FindComponents<T>();
				if (Temp.size() > 0)
					Components.insert(Components.end(), Temp.begin(), Temp.end());
			}
		}
		return Components;
	}

	inline bool HasAnyChildren() const { return Children.size() > 0; }
	inline std::size_t GetChildrenSize() const { return Children.size(); }
	inline sPrimitiveComponent* GetChild(const std::size_t index) const { return Children[index].get(); }
	inline std::vector<sPrimitiveComponent*> GetAllChildren() const 
	{
		std::vector<sPrimitiveComponent*> Result;
		Result.reserve(Children.size());
		std::transform(Children.cbegin(), Children.cend(), std::back_inserter(Result), [](auto& ptr) { return ptr.get(); });
		return Result;
	}

	bool AttachToComponent(sPrimitiveComponent* InParent);
	void DetachComponent(std::size_t index);
	void DetachFromComponent();

	void Enable();
	void Disable();
	void Hide(bool value);

	bool IsEnabled() const;
	bool IsHidden() const;

	void SetRelativeLocation(const FVector& V);
	FVector GetRelativeLocation() const;
	void SetRelativeRotation(const FVector4& V);
	void SetRollPitchYaw(FAngles RPY);
	FQuaternion GetRelativeRotation() const;
	FAngles GetRelativeRotationAngles() const;
	void SetRelativeScale(const FVector& V);
	FVector GetRelativeScale() const;
	virtual FVector GetVelocity() const;

	inline FVector GetLocalLocation() const { return Location; }
	inline FQuaternion GetLocalRotation() const { return Rotation; }
	inline FAngles GetLocalRotationAngles() const { return FQuaternion(Rotation).GetAngles(); }
	inline FVector GetLocalScale() const { return Scale; }

	inline virtual FBoundingBox GetBounds() const { return FBoundingBox(FBoxDimension(), GetRelativeLocation()); }

	void SetTransform(const FVector& Location, const FVector4& Rotation, const std::optional<FVector> Scale = std::nullopt);

	virtual void Serialize(sArchive& archive);

private:
	bool AttachComponent(sPrimitiveComponent::SharedPtr Component);
protected:
	void DetachComponent(sPrimitiveComponent* Component);

protected:
	void UpdateTransform();
private:
	virtual void OnUpdateTransform() {};

private:
	virtual void OnBeginPlay() {}
	virtual void OnTick(const double DeltaTime) {}
	virtual void OnFixedUpdate(const double DeltaTime) {}

	virtual void OnAttach() {};
	virtual void OnDetach() {};

	virtual void OnEnabled() {}
	virtual void OnDisabled() {}
	virtual void OnHidden() {}
	virtual void OnVisible() {}

	virtual void OnAttachToComponent() {};
	virtual void OnDetachFromComponent() {};

	virtual void OnChildAttached(sPrimitiveComponent* ChildComponent) {}
	virtual void OnChildDetached() {}

protected:
	sActor* Owner;
	std::string Name;
	std::vector<std::string> Tags;

	bool bIsHidden;
	bool bIsEnabled;

	sPrimitiveComponent* ComponentOwner;
	FVector Location;
	FVector4 Rotation;
	FVector Scale;

	std::vector<sPrimitiveComponent::SharedPtr> Children;
};
