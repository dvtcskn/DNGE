#pragma once

#include <Engine/IMetaWorld.h>
#include "WindowsPlatform.h"
#include <Gameplay/GameInstance.h>

namespace cbgui
{
	class sCanvas;
	class DebugCanvas;
}

class sDefaultLevel;

class MetaWorld : public IMetaWorld
{
	sClassBody(sClassConstructor, MetaWorld, IMetaWorld)
public:
	MetaWorld(WindowsPlatform* Owner);
	virtual ~MetaWorld();

	virtual void BeginPlay() override final;
	virtual void Tick(const double DeltaTime) override final;
	virtual void FixedUpdate(const double DeltaTime) override final;

	virtual void OnResizeWindow(const std::size_t Width, const std::size_t Height) override final;

	virtual size_t WorldCount() const override final { return Worlds.size(); }
	virtual	std::vector<IWorld*> GetAllWorlds() const override final;
	virtual IWorld* GetWorld(const std::size_t Index) const override final;
	virtual IWorld* GetWorld(const std::string& Name) const override final;
	virtual IWorld* GetActiveWorld() const override final;
	virtual void SetActiveWorld(const std::size_t index) override final;
	virtual void SetActiveWorld(const std::string& Name) override final;
	virtual void AddWorld(const std::shared_ptr<IWorld>& World) override final;
	virtual void RemoveWorld(const std::size_t index) override final;
	virtual void RemoveWorld(const std::string& Name) override final;

	virtual ILevel* GetActiveLevel() const override final;
	virtual std::vector<ILevel*> GetLevels(std::size_t WorldIndex = 0) const override final;
	virtual ICanvas* GetActiveCanvas() const override final;
	virtual std::vector<ICanvas*> GetCanvases() const override final;
	virtual sGameInstance* GetGameInstance() const override final { return GameInstance.get(); }

	virtual void PauseActiveWorld(bool Pause) override final;

	virtual void InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar) override final;

	cbgui::cbDimension GetScreenDimension() const { return Owner->GetScreenDimension(); }
	WindowsPlatform* GetOwnerPlatform() const { return Owner; }

private:
	WindowsPlatform* Owner;
	sGameInstance::UniquePtr GameInstance;

	std::vector<IWorld::SharedPtr> Worlds;
	std::size_t ActiveWorldIndex;

	std::shared_ptr<cbgui::sCanvas> Canvas;

	bool bPauseActiveWorld;
};
