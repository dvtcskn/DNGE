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
	sActor* GetPlayerFocusedActor(std::size_t Index) const;

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

