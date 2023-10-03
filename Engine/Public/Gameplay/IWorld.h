#pragma once

#include <memory>
#include <vector>
#include "Engine/ClassBody.h"
#include "Utilities/Input.h"

class sGameInstance;
class sActor;
class ILevel;
class IMetaWorld;

class IWorld
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IWorld)
public:
	virtual std::string GetName() const = 0;

	virtual void InitWorld() = 0;

	virtual void BeginPlay() = 0;
	virtual void Tick(const double DeltaTime) = 0;
	virtual void FixedUpdate(const double DeltaTime) = 0;

	virtual void Serialize() = 0;

	template<class T>
	inline T* GetMetaWorld() const { return static_cast<T*>(GetMetaWorld()); }
	virtual IMetaWorld* GetMetaWorld() const = 0;

	virtual size_t LevelCount() const { return 1; }
	virtual void AddLevel(const std::shared_ptr<ILevel>& Level) = 0;
	virtual void SetActiveLevel(const std::size_t index) = 0;
	virtual void SetActiveLevel(const std::string& Name) = 0;
	virtual	std::vector<ILevel*> GetAllLevels() const = 0;
	virtual ILevel* GetLevel(const std::size_t Index) const = 0;
	template<class T>
	inline T* GetLevel(const std::size_t Index) const { return static_cast<T*>(GetLevel(Index)); }
	virtual ILevel* GetLevel(const std::string& Name) const = 0;
	template<class T>
	inline T* GetLevel(const std::string& Name) const { return static_cast<T*>(GetLevel(Name)); }
	virtual ILevel* GetActiveLevel() const = 0;
	template<class T>
	inline T* GetActiveLevel() const { return static_cast<T*>(GetActiveLevel()); }
	virtual void RemoveLevel(const std::size_t index) = 0;
	virtual void RemoveLevel(const std::string& Name) = 0;

	virtual sGameInstance* GetGameInstance() const = 0;
	virtual sActor* GetPlayerFocusedActor(std::size_t Index) const = 0;

	virtual void PauseActiveLevel(bool Pause) {}

	virtual void OnResizeWindow(const std::size_t Width, const std::size_t Height) = 0;

	virtual void InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar) = 0;
};
