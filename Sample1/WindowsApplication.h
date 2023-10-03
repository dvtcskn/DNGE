#pragma once

#include <memory>
#include "WindowsPlatform.h"

class WindowsApplication
{
private:
	std::unique_ptr<WindowsPlatform> pWindowsPlatform;
public:
	WindowsApplication()
	{
		pWindowsPlatform = std::make_unique<WindowsPlatform>();
	}

	virtual ~WindowsApplication()
	{
		pWindowsPlatform = nullptr;
	}

	void RunEngine()
	{
		StartModules();
		EngineLoop();
	}

	void StartModules()
	{
		pWindowsPlatform->BeginPlay();
	}

	void EngineLoop()
	{
		pWindowsPlatform->MessageLoop();
	}
};
