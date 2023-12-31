/* ---------------------------------------------------------------------------------------
* MIT License
*
* Copyright (c) 2023 Davut Co�kun.
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

#include "pch.h"
#include "MetaWorld.h"
#include "Canvas.h"
#include "DefaultLevel.h"
#include "GGameInstance.h"
#include "World.h"

#define ENABLE_EDITOR 0

MetaWorld::MetaWorld(WindowsPlatform* pOwner)
	: Owner(pOwner)
	, GameInstance(GGameInstance::CreateUnique(this))
	, ActiveWorldIndex(0)
	, Canvas(cbgui::sCanvas::Create(this))
	, bPauseActiveWorld(false)
{
	AddWorld(sWorld::Create(this));
}

MetaWorld::~MetaWorld()
{
	for (auto& World : Worlds)
		World = nullptr;
	Worlds.clear();

	Canvas = nullptr;
	GameInstance = nullptr;
	Owner = nullptr;
}

void MetaWorld::BeginPlay()
{
	GameInstance->BeginPlay();
	auto ActiveWorld = GetActiveWorld();
	if (ActiveWorld && !bPauseActiveWorld)
		ActiveWorld->BeginPlay();
	Canvas->BeginPlay();
}

void MetaWorld::Tick(const double DeltaTime)
{
	GameInstance->Tick(DeltaTime);
	auto ActiveWorld = GetActiveWorld();
	if (ActiveWorld && !bPauseActiveWorld)
		ActiveWorld->Tick(DeltaTime);
	Canvas->Tick(DeltaTime);
}

void MetaWorld::FixedUpdate(const double DeltaTime)
{
	GameInstance->FixedUpdate(DeltaTime);
	auto ActiveWorld = GetActiveWorld();
	if (ActiveWorld && !bPauseActiveWorld)
		ActiveWorld->FixedUpdate(DeltaTime);
	Canvas->FixedUpdate(DeltaTime);
}

void MetaWorld::OnResizeWindow(const std::size_t Width, const std::size_t Height)
{
	GameInstance->WindowResized(Width, Height);
	auto ActiveWorld = GetActiveWorld();
	if (ActiveWorld)
		ActiveWorld->OnResizeWindow(Width, Height);
	Canvas->ResizeWindow(Width, Height);
}

void MetaWorld::PauseActiveWorld(bool Pause)
{
	bPauseActiveWorld = Pause;
}

void MetaWorld::InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar)
{
	GameInstance->InputProcess(MouseInput, KeyboardChar);
	auto ActiveWorld = GetActiveWorld();
	if (ActiveWorld && !bPauseActiveWorld)
		ActiveWorld->InputProcess(MouseInput, KeyboardChar);
	Canvas->InputProcess(MouseInput, KeyboardChar);
}

std::vector<IWorld*> MetaWorld::GetAllWorlds() const
{
	std::vector<IWorld*> Result;
	Result.reserve(Worlds.size());
	std::transform(Worlds.cbegin(), Worlds.cend(), std::back_inserter(Result), [](auto& ptr) { return ptr.get(); });
	return Result;
}

IWorld* MetaWorld::GetWorld(const std::size_t Index) const
{
	if (Index < WorldCount())
		return Worlds.at(Index).get();
	return nullptr;
}

IWorld* MetaWorld::GetWorld(const std::string& Name) const
{
	for (const auto& World : Worlds)
	{
		if (World->GetName() == Name)
			return World.get();
	}
	return nullptr;
}

IWorld* MetaWorld::GetActiveWorld() const
{
	if (ActiveWorldIndex < WorldCount())
		return Worlds.at(ActiveWorldIndex).get();
	return nullptr;
}

void MetaWorld::SetActiveWorld(const std::size_t index)
{
	if (index < WorldCount())
		ActiveWorldIndex = index;
}

void MetaWorld::SetActiveWorld(const std::string& Name)
{
	for (std::size_t i = 0; i < Worlds.size(); i++)
	{
		if (Worlds[i]->GetName() == Name)
		{
			ActiveWorldIndex = i;
			break;
		}
	}
}

void MetaWorld::AddWorld(const std::shared_ptr<IWorld>& World)
{
	if (std::find(Worlds.begin(), Worlds.end(), World) == Worlds.end())
		Worlds.push_back(World);
}

void MetaWorld::RemoveWorld(const std::size_t index)
{
	if (index < WorldCount())
		Worlds.erase(Worlds.begin() + index);
}

void MetaWorld::RemoveWorld(const std::string& Name)
{
	Worlds.erase(std::find_if(Worlds.begin(), Worlds.end(), [Name](const IWorld::SharedPtr& Level)
		{
			return Name == Level->GetName();
		}));
}

ILevel* MetaWorld::GetActiveLevel() const
{
	auto ActiveWorld = GetActiveWorld();
	if (ActiveWorld)
		return ActiveWorld->GetActiveLevel();
	return nullptr;
}

std::vector<ILevel*> MetaWorld::GetLevels(std::size_t WorldIndex) const
{
	auto ActiveWorld = GetActiveWorld();
	if (ActiveWorld)
		return ActiveWorld->GetAllLevels();
	return std::vector<ILevel*>();
}

ICanvas* MetaWorld::GetActiveCanvas() const
{
	return Canvas.get();
}

std::vector<ICanvas*> MetaWorld::GetCanvases() const
{
	return std::vector<ICanvas*>{ Canvas.get() };
}
