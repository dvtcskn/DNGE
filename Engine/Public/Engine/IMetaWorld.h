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
#include "..//Utilities/Input.h"
#include "ClassBody.h"
#include "Core/Camera.h"

class ILevel;
class IWorld;
class ICanvas;
class sGameInstance;

class IMetaWorld
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IMetaWorld)
public:
	virtual void BeginPlay() = 0;
	virtual void Tick(const double DeltaTime) = 0;
	virtual void FixedUpdate(const double DeltaTime) = 0;

	virtual void OnResizeWindow(const std::size_t Width, const std::size_t Height) = 0;

	virtual size_t WorldCount() const { return 1; }
	virtual	std::vector<IWorld*> GetAllWorlds() const = 0;
	virtual IWorld* GetWorld(const std::size_t Index) const = 0;
	template<class T>
	inline T* GetWorld(const std::size_t Index) const { return static_cast<T*>(GetWorld(Index)); }
	virtual IWorld* GetWorld(const std::string& Name) const = 0;
	template<class T>
	inline T* GetWorld(const std::string& Name) const { return static_cast<T*>(GetWorld(Name)); }
	virtual IWorld* GetActiveWorld() const = 0;
	template<class T>
	inline T* GetActiveWorld() const { return static_cast<T*>(GetActiveWorld()); }
	virtual void SetActiveWorld(const std::size_t index) = 0;
	virtual void SetActiveWorld(const std::string& Name) = 0;
	virtual void AddWorld(const std::shared_ptr<IWorld>& World) = 0;
	virtual void RemoveWorld(const std::size_t index) = 0;
	virtual void RemoveWorld(const std::string& Name) = 0;

	virtual ILevel* GetActiveLevel() const = 0;
	template<class T>
	inline T* GetActiveLevel() const { return static_cast<T*>(GetActiveLevel()); }
	virtual std::vector<ILevel*> GetLevels(std::size_t WorldIndex = 0) const = 0;
	virtual ICanvas* GetActiveCanvas() const = 0;
	template<class T>
	inline T* GetActiveCanvas() const { return static_cast<T*>(GetActiveCanvas()); }
	virtual std::vector<ICanvas*> GetCanvases() const = 0;
	virtual sGameInstance* GetGameInstance() const = 0;
	template<class T>
	inline T* GetGameInstance() const { return static_cast<T*>(GetGameInstance()); }

	virtual void PauseActiveWorld(bool Pause) {}

	virtual void InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar) = 0;
};
