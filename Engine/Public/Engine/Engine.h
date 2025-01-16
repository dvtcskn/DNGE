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
#include "Core/Math/CoreMath.h"
#include "Utilities/Input.h"
#include "Engine/AbstractEngine.h"
#include "Engine/StepTimer.h"
#include "IMetaWorld.h"
#include "Engine/IPhysicalWorld.h"

class sEngine final
{
	sBaseClassBody(sClassConstructor, sEngine);
public:
	sEngine(const GPUDeviceCreateInfo& CreateInfo, const IPhysicalWorld::SharedPtr& PhysicalWorld);
	//sEngine(const EGITypes GIType, const IPhysicalWorld::SharedPtr& PhysicalWorld, std::optional<short> GPUIndex = std::nullopt, void* InHWND = nullptr);
	~sEngine();

	//bool InitWindow(void* HWND, std::uint32_t InWidth, std::uint32_t InHeight, bool bFullscreen);

	void EngineInternalTick();

	void BeginPlay();
	void PhysicsTick(const double DeltaTime);
	void FixedTick(const double DeltaTime);
	/*
	* With Renderer Tick
	*/
	void BeginFrame();
	void Tick(const double DeltaTime);
	void Render();
	void Present();

	bool SetMetaWorld(const std::shared_ptr<IMetaWorld>& MetaWorld);
	void DestroyWorld();
	IMetaWorld* GetMetaWorld() const;

	void FullScreen(const bool value);
	bool IsFullScreen() const;

	bool IsVsyncEnabled() const;
	void Vsync(const bool value);
	void VsyncInterval(const std::uint32_t value);
	std::uint32_t GetVsyncInterval() const;

	/*
	* Affects the Engine tick,
	* Renderer, Renderer tick and Present
	* Limits FPS
	*/
	void SetEngineFixedTargetElapsedSeconds(bool Enable, double DeltaTime = 1.0/60.0);
	/*
	* Affects PhysicsTick and FixedTick rate
	*/
	void SetFixedTargetElapsedSeconds(double DeltaTime);

	sGPUInfo GetGPUInfo() const;

	std::vector<sDisplayMode> GetAllSupportedResolutions() const;

	sScreenDimension GetScreenDimension() const;
	void ResizeWindow(std::size_t Width, std::size_t Height);

	sScreenDimension GetInternalBaseRenderResolution() const;
	void SetInternalBaseRenderResolution(std::size_t Width, std::size_t Height);

	void SetGBufferClearMode(EGBufferClear Mode);
	EGBufferClear GetGBufferClearMode() const;

	void InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar);

private:
	bool bWindowInitialized;
	StepTimer mStepTimer;
	StepTimer FixedStepTimer;
	sScreenDimension ScreenDimension;

	std::shared_ptr<IMetaWorld> MetaWorld;
};
