/* ---------------------------------------------------------------------------------------
* MIT License
*
* Copyright (c) 2022 Davut Coþkun.
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

#include <list>
#include <memory>
#include <thread>
#include <mutex>
#include <map>
#include <string>
#include <cbMath.h>

#include <Engine/Engine.h>

#include "framework.h"

class MetaWorld;

class WindowsPlatform final
{
public:
	WindowsPlatform();
	virtual ~WindowsPlatform();

	void BeginPlay();
	void Render();
	void Tick(const double DeltaTime);
	void MessageLoop();

	HWND GetHWND() { return m_hWnd; }

	void RequestTermination();

	void WindowMode(const int value);
	void ResizeWindow(std::size_t Width, std::size_t Height);
	void Vsync(const bool value);
	void VsyncInterval(const UINT value);

	cbgui::cbDimension GetScreenDimension() const { return cbgui::cbDimension((float)WindowWidth, (float)WindowHeight); }
	std::uint32_t GetWindowWidth() const { return WindowWidth; }
	std::uint32_t GetWindowHeight() const { return WindowHeight; }
	bool IsWindowFullscreen() const { return WindowFullscreen; }

	HRESULT GetDisplayResolution(int& width, int& height);

	LRESULT MsgProc(HWND hWnd, std::uint32_t uMsg, WPARAM wParam, LPARAM lParam);

private:
	bool CreateViewport(std::wstring title, RECT rect, bool bfullscreen);

private:
	std::unique_ptr<sEngine> Engine;

	HWND m_hWnd;

	int WindowsMode;
	std::uint32_t WindowWidth;
	std::uint32_t WindowHeight;
	bool WindowFullscreen;

	GKeyboardChar KeyboardBTNEvent;

	std::shared_ptr<MetaWorld> pMetaWorld;
};
