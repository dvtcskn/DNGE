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
