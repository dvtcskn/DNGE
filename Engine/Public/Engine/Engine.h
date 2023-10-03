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
	sEngine(const EGITypes GIType, const IPhysicalWorld::SharedPtr& PhysicalWorld, std::optional<short> GPUIndex = std::nullopt);
	~sEngine();

	void InitWindow(void* Handle, std::uint32_t InWidth, std::uint32_t InHeight, bool bFullscreen);

	void EngineInternalTick();

	void BeginPlay();
	void PhysicsTick(const double DeltaTime);
	void FixedTick(const double DeltaTime);
	/*
	* With Renderer Tick
	*/
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
	StepTimer mStepTimer;
	StepTimer FixedStepTimer;
	sScreenDimension ScreenDimension;

	std::shared_ptr<IMetaWorld> MetaWorld;
};
