#pragma once

#include <Gameplay/IWorld.h>
#include <Gameplay/ILevel.h>
#include "DefaultLevel.h"

class MetaWorld;

class sWorld : public IWorld
{
	sClassBody(sClassConstructor, sWorld, IWorld)
public:
	sWorld(MetaWorld* Owner, std::string Name = "DefaultWorld");
	virtual ~sWorld();

	virtual std::string GetName() const override final;

	virtual void InitWorld() override final;

	virtual void BeginPlay() override final;
	virtual void Tick(const double DeltaTime) override final;
	virtual void FixedUpdate(const double DeltaTime) override final;

	virtual void Serialize() override final;

	virtual IMetaWorld* GetMetaWorld() const override final;

	virtual size_t LevelCount() const override final { return Levels.size(); }
	virtual void AddLevel(const ILevel::SharedPtr& Level) override final;
	virtual void SetActiveLevel(const std::size_t index) override final;
	virtual void SetActiveLevel(const std::string& Name) override final;
	virtual	std::vector<ILevel*> GetAllLevels() const override final;
	virtual ILevel* GetLevel(const std::size_t Index) const override final;
	virtual ILevel* GetLevel(const std::string& Name) const override final;
	virtual ILevel* GetActiveLevel() const override final;
	virtual void RemoveLevel(const std::size_t index) override final;
	virtual void RemoveLevel(const std::string& Name) override final;

	virtual sGameInstance* GetGameInstance() const override final;
	virtual sActor* GetPlayerFocusedActor(std::size_t Index) const override final;

	virtual void PauseActiveLevel(bool Pause) override final;

	virtual void OnResizeWindow(const std::size_t Width, const std::size_t Height) override final;

	virtual void InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar) override final;

private:
	MetaWorld* Owner;
	std::string Name;

	std::size_t ActiveLevelIndex;
	std::vector<ILevel::SharedPtr> Levels;

	bool bPauseActiveLevel;
};

