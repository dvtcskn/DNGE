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

	virtual void PauseActiveLevel(bool Pause) {}

	virtual void OnResizeWindow(const std::size_t Width, const std::size_t Height) = 0;

	virtual void InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar) = 0;
};
