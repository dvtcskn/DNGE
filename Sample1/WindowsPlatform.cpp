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

#include "pch.h"
#include "WindowsPlatform.h"
#include <algorithm>
#include "MetaWorld.h"
#include <Utilities/ConfigManager.h>
#include <Utilities/FileManager.h>
#include "AssetManager.h"

#include <Engine/World2D.h>
#include "Materials.h"

#pragma comment(lib, "Engine.lib")
#pragma comment(lib, "CBGUI.lib")
#pragma comment(lib, "freetype.lib")
#pragma comment(lib, "Pdh.lib")

#pragma comment(lib, "box2d.lib")


#define WINDOW_STYLE_NORMAL					(WS_VISIBLE | WS_OVERLAPPEDWINDOW | WS_CAPTION)
#define WINDOW_STYLE_BORDERLESS				(WS_VISIBLE | WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP)

namespace EngineConsole {
	static void AttachConsoleToEngine()
	{
//#ifdef _DEBUG
		AllocConsole();
		AttachConsole(GetCurrentProcessId());
		FILE* stream;
		freopen_s(&stream, "CONOUT$", "w+", stdout);
		SetConsoleTitle(L"GameTest_Console");
		//fflush(stream);
//#endif
	}
}

RECT ArrangeWindow(const RECT& rect)
{
	RECT result = rect;
	{
		HMONITOR hMonitor;
		MONITORINFO mi;
		hMonitor = MonitorFromRect(&rect, MONITOR_DEFAULTTONEAREST);
		mi.cbSize = sizeof(mi);
		GetMonitorInfo(hMonitor, &mi);

		RECT desktop = mi.rcMonitor;
		const int centreX = (int)desktop.left + (int)(desktop.right - desktop.left) / 2;
		const int centreY = (int)desktop.top + (int)(desktop.bottom - desktop.top) / 2;
		const int winW = rect.right - rect.left;
		const int winH = rect.bottom - rect.top;
		int left = centreX - winW / 2;
		int right = left + winW;
		int top = centreY - winH / 2;
		int bottom = top + winH;
		result.left = std::max(left, (int)desktop.left);
		result.right = std::min(right, (int)desktop.right);
		result.bottom = std::min(bottom, (int)desktop.bottom);
		result.top = std::max(top, (int)desktop.top);

	}
	return result;
}

LRESULT CALLBACK WindowProc(HWND hWnd, std::uint32_t uMsg, WPARAM wParam, LPARAM lParam)
{
	WindowsPlatform* pWP = (WindowsPlatform*)GetWindowLongPtr(hWnd, 0);
	if (pWP)
		return pWP->MsgProc(hWnd, uMsg, wParam, lParam);
	else
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

WindowsPlatform::WindowsPlatform()
	: m_hWnd(NULL)
	, WindowsMode(1)
	, WindowWidth(1366)
	, WindowHeight(768)
	, WindowFullscreen(false)
	, Engine(nullptr)
{
	ConfigManager::Get().ReadGameConfig();

	WindowWidth = (std::uint32_t)ConfigManager::Get().GetGameConfig().Dimensions.Width;
	WindowHeight = (std::uint32_t)ConfigManager::Get().GetGameConfig().Dimensions.Height;
	WindowFullscreen = ConfigManager::Get().GetGameConfig().Fullscreen;

	RECT rect = ArrangeWindow(RECT({ 0, 0, static_cast<LONG>(WindowWidth), static_cast<LONG>(WindowHeight) }));
	CreateViewport(L"GameTest", rect, WindowFullscreen);

	Engine = sEngine::CreateUnique(EGITypes::eD3D12,
#if BulletPhysics
		BulletWorld::Create(PhysicalWorldScale)
#elif PhysXEngine
		PhysXWorld::Create(PhysicalWorldScale)
#else
		sWorld2D::Create()
#endif
		, 0
	);

	Engine->InitWindow(GetHWND(), WindowWidth, WindowHeight, WindowFullscreen);
	Engine->SetInternalBaseRenderResolution(640, 360);
	GameMaterials::InitMaterials();
	AssetManager::Get().InitializeResources();
	pMetaWorld = MetaWorld::Create(this);
	Engine->SetMetaWorld(pMetaWorld);

	HCURSOR hCursor = LoadCursor(NULL, IDC_ARROW);
	SetCursor(hCursor);

	//Engine->Vsync(true);
	//Engine->SetEngineFixedTargetElapsedSeconds(true, 1.0/60.0);
}

WindowsPlatform::~WindowsPlatform()
{
	if (m_hWnd)
	{
		DestroyWindow(m_hWnd);
		m_hWnd = NULL;
	}
	pMetaWorld = nullptr;
	AssetManager::Get().Destroy();
	Engine = nullptr;
}

bool WindowsPlatform::CreateViewport(std::wstring title, RECT rect, bool bfullscreen)
{
	EngineConsole::AttachConsoleToEngine();
	HINSTANCE hInstance = GetModuleHandle(NULL);
	//WNDCLASSEX windowClass = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WindowProc,
	//	0L, sizeof(void*), hInstance, NULL, NULL, NULL, NULL, WINDOW_CLASS_NAME, NULL };

	WNDCLASSEX windowClass;
	memset(&windowClass, 0, sizeof(WNDCLASSEX));
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.cbClsExtra = 0L;
	windowClass.cbWndExtra = sizeof(void*);
	windowClass.hInstance = hInstance;
	windowClass.hIcon = NULL;// LoadIcon(NULL, IDI_APPLICATION);
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.hbrBackground = NULL;// (HBRUSH)GetStockObject(WHITE_BRUSH);
	windowClass.lpszMenuName = NULL;// APP_NAME;
	windowClass.lpszClassName = title.c_str();
	windowClass.hIconSm = NULL;// LoadIcon(NULL, IDI_WINLOGO);

	RegisterClassEx(&windowClass);

	std::uint32_t windowStyle = WINDOW_STYLE_BORDERLESS;

	AdjustWindowRect(&rect, windowStyle, FALSE);

	m_hWnd = CreateWindowEx(
		WS_EX_APPWINDOW,
		title.c_str(),
		title.c_str(),
		windowStyle,
		rect.left,
		rect.top,
		rect.right - rect.left,
		rect.bottom - rect.top,
		GetDesktopWindow(),
		NULL,
		hInstance,
		NULL
	);

	if (!m_hWnd)
	{
#ifdef _DEBUG
		DWORD errorCode = GetLastError();
		DebugBreak();
#endif
		MessageBox(NULL, L"Cannot create window", 0, MB_OK | MB_ICONERROR);
		//return E_FAIL;
		return false;
	}

	ShowWindow(m_hWnd, SW_SHOW);
	SetForegroundWindow(m_hWnd);
	SetFocus(m_hWnd);

	SetWindowLongPtr(m_hWnd, 0, (LONG_PTR)this);
	UpdateWindow(m_hWnd);
	return true;
}

void WindowsPlatform::MessageLoop()
{
	MSG msg = { 0 };

	while (WM_QUIT != msg.message)
	{
		while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)) // PM_NOYIELD
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

		}
		{
			//if (!IsIconic(m_hWnd))
			{
				Engine->EngineInternalTick();
			}
		}
	}
}

void WindowsPlatform::BeginPlay()
{
	Engine->BeginPlay();
}

void WindowsPlatform::Tick(const double fElapsedTimeSeconds)
{
	Engine->Tick((float)fElapsedTimeSeconds);
}

void WindowsPlatform::Render()
{
	Engine->Render();
}

LRESULT WindowsPlatform::MsgProc(HWND hWnd, std::uint32_t uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_KEYDOWN)
	{
		int iKeyPressed = static_cast<int>(wParam);

		switch (iKeyPressed)
		{
		case VK_ESCAPE:
			RequestTermination();
			return true;
			break;

		case VK_TAB:
			return 0;
			break;

		case VK_SPACE:
			break;

		}

	}

	switch (uMsg)
	{
	case WM_DESTROY:
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;

	case WM_SYSKEYDOWN:
		if (wParam == VK_F4)
		{
			PostQuitMessage(0);
			return 0;
		}
		break;

	case WM_ENTERSIZEMOVE:
		break;

	case WM_EXITSIZEMOVE:
		break;

	case WM_KEYDOWN:
		break;

	case WM_SIZE:
		break;
	}

	if (uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST ||
		uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST)
	{
		auto GetMouseLocation = [&]() -> FVector2
		{
			POINT point;
			GetCursorPos(&point);
			ScreenToClient(hWnd, &point);

			return FVector2(static_cast<float>(point.x), static_cast<float>(point.y));
		};

		GMouseInput MouseInput;
		KeyboardBTNEvent.bIsIdle = true;

		if (uMsg)
		{
			switch (uMsg)
			{
			case WM_MOUSEWHEEL:
			{
				MouseInput.State = EMouseState::eScroll;

				const float SpinFactor = 1.0f / 120.0f;
				const short WheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);

				MouseInput.MouseLocation = GetMouseLocation();
				MouseInput.WheelDelta = (short)(static_cast<float>(WheelDelta) * SpinFactor);
				break;
			}
			case WM_MOUSEMOVE:
			{
				MouseInput.State = EMouseState::eMoving;
				MouseInput.MouseLocation = GetMouseLocation();
				break;
			}
			case WM_LBUTTONDOWN:
			{
				MouseInput.Buttons["Left"] = EMouseButtonState::ePressed;
				MouseInput.MouseLocation = GetMouseLocation();
				break;
			}
			case WM_LBUTTONUP:
			{
				MouseInput.Buttons["Left"] = EMouseButtonState::eReleased;
				MouseInput.MouseLocation = GetMouseLocation();
				break;
			}
			case WM_RBUTTONDOWN:
			{
				MouseInput.Buttons["Right"] = EMouseButtonState::ePressed;
				MouseInput.MouseLocation = GetMouseLocation();
				break;
			}
			case WM_RBUTTONUP:
			{
				MouseInput.Buttons["Right"] = EMouseButtonState::eReleased;
				MouseInput.MouseLocation = GetMouseLocation();
				break;
			}
			case WM_MBUTTONDOWN:
			{
				MouseInput.Buttons["Middle"] = EMouseButtonState::ePressed;
				MouseInput.MouseLocation = GetMouseLocation();
				break;
			}
			case WM_MBUTTONUP:
			{
				MouseInput.Buttons["Middle"] = EMouseButtonState::eReleased;
				MouseInput.MouseLocation = GetMouseLocation();
				break;
			}
			case WM_XBUTTONDOWN:
				switch (GET_XBUTTON_WPARAM(wParam))
				{
				case XBUTTON1:
					MouseInput.Buttons["XB1"] = EMouseButtonState::ePressed;
					MouseInput.MouseLocation = GetMouseLocation();
					break;

				case XBUTTON2:
					MouseInput.Buttons["XB2"] = EMouseButtonState::ePressed;
					MouseInput.MouseLocation = GetMouseLocation();
					break;
				}
				break;

			case WM_XBUTTONUP:
				switch (GET_XBUTTON_WPARAM(wParam))
				{
				case XBUTTON1:
					MouseInput.Buttons["XB1"] = EMouseButtonState::eReleased;
					MouseInput.MouseLocation = GetMouseLocation();
					break;

				case XBUTTON2:
					MouseInput.Buttons["XB2"] = EMouseButtonState::eReleased;
					MouseInput.MouseLocation = GetMouseLocation();
					break;
				}
				break;
			case WM_SYSKEYUP:
				break;
			case WM_SYSKEYDOWN:
				break;
			case WM_KEYDOWN:
			{
				KeyboardBTNEvent.bIsIdle = false;
				KeyboardBTNEvent.bIsPressed = true;
				KeyboardBTNEvent.KeyCode = 0;
				if (static_cast<int>(wParam) < 65)
					KeyboardBTNEvent.KeyCode = static_cast<int>(wParam);
				KeyboardBTNEvent.bIsChar = false;
			}
				break;
			case WM_CHAR:
			{
				if (KeyboardBTNEvent.bIsPressed)
				{
					int iKeyPressed = static_cast<int>(wParam);

					if (iKeyPressed < 32)
					{
						KeyboardBTNEvent.KeyCode = iKeyPressed;
					}
					else
					{
						if ((GetAsyncKeyState(VK_CAPITAL) & 0x0001) == 0)
						{
							iKeyPressed = towlower(iKeyPressed);

							if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0)
								iKeyPressed = towupper(iKeyPressed);
						}
						else if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0)
							iKeyPressed = towlower(iKeyPressed);

						KeyboardBTNEvent.bIsIdle = false;
						KeyboardBTNEvent.KeyCode = iKeyPressed;
						KeyboardBTNEvent.bIsChar = true;
					}
				}
			}
				break;
			case WM_KEYUP:
				KeyboardBTNEvent.bIsIdle = true;
				KeyboardBTNEvent.bIsPressed = false;
				KeyboardBTNEvent.KeyCode = static_cast<int>(wParam);
				break;
			}
		}
		Engine->InputProcess(MouseInput, KeyboardBTNEvent);
		KeyboardBTNEvent.KeyCode = 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void WindowsPlatform::RequestTermination()
{
	// This triggers the termination of the application
	PostQuitMessage(0);
}

void WindowsPlatform::WindowMode(const int value)
{
	if (WindowsMode == value)
		return;

	switch (value)
	{
	case 0:		// Fullscreen
	{
		SetWindowLong(m_hWnd, GWL_STYLE, WINDOW_STYLE_BORDERLESS);

		RECT rect = ArrangeWindow(RECT({ 0, 0, static_cast<LONG>(WindowWidth), static_cast<LONG>(WindowHeight) }));
		MoveWindow(m_hWnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, true);

		Engine->FullScreen(true);

		if (!Engine->IsFullScreen())
		{
			SetWindowLong(m_hWnd, GWL_STYLE, WINDOW_STYLE_NORMAL);
			return;
		}

		UpdateWindow(m_hWnd);

		WindowsMode = value;
	}
		break;
	case 1:		// Borderless Windowed
	{
		if (Engine->IsFullScreen())
		{
			Engine->FullScreen(false);
		}

		SetWindowLong(m_hWnd, GWL_STYLE, WINDOW_STYLE_BORDERLESS);

		RECT rect = ArrangeWindow(RECT({ 0, 0, static_cast<LONG>(WindowWidth), static_cast<LONG>(WindowHeight) }));
		AdjustWindowRect(&rect, WINDOW_STYLE_BORDERLESS, FALSE);
		MoveWindow(m_hWnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, true);
		UpdateWindow(m_hWnd);

		WindowsMode = value;
	}
		break;
	case 2:		// Windowed
	{
		if (Engine->IsFullScreen())
		{
			Engine->FullScreen(false);
		}

		SetWindowLong(m_hWnd, GWL_STYLE, WINDOW_STYLE_NORMAL);

		RECT rect = ArrangeWindow(RECT({ 0, 0, static_cast<LONG>(WindowWidth), static_cast<LONG>(WindowHeight) }));
		AdjustWindowRect(&rect, WINDOW_STYLE_NORMAL, FALSE);
		MoveWindow(m_hWnd, rect.left/2, rect.top/2, rect.right - rect.left, rect.bottom - rect.top, true);
		UpdateWindow(m_hWnd);

		WindowsMode = value;
	}
		break;
	default:
		break;
	}
}

void WindowsPlatform::ResizeWindow(std::size_t Width, std::size_t Height)
{
	WindowWidth = (uint32_t)Width;
	WindowHeight = (uint32_t)Height;

	RECT rect = ArrangeWindow(RECT({ 0, 0, static_cast<LONG>(WindowWidth), static_cast<LONG>(WindowHeight) }));

	if (WindowsMode == 0) // Fullscreen
	{
		AdjustWindowRect(&rect, WINDOW_STYLE_BORDERLESS, FALSE);
	}
	else if (WindowsMode == 1) // Borderless Windowed
	{
		AdjustWindowRect(&rect, WINDOW_STYLE_BORDERLESS, FALSE);
	}
	else if (WindowsMode == 2) // Windowed
	{
		AdjustWindowRect(&rect, WINDOW_STYLE_NORMAL, FALSE);
	}

	MoveWindow(m_hWnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, true);

	Engine->ResizeWindow(WindowWidth, WindowHeight);

	ShowWindow(m_hWnd, SW_RESTORE);
	UpdateWindow(m_hWnd);
}

void WindowsPlatform::Vsync(const bool value)
{
	Engine->Vsync(value);
}

void WindowsPlatform::VsyncInterval(const UINT value)
{
	Engine->VsyncInterval(value);
}

HRESULT WindowsPlatform::GetDisplayResolution(int& width, int& height)
{
	if (m_hWnd != INVALID_HANDLE_VALUE)
	{
		HMONITOR monitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTOPRIMARY);
		MONITORINFO info;
		info.cbSize = sizeof(MONITORINFO);

		if (GetMonitorInfo(monitor, &info))
		{
			width = info.rcMonitor.right - info.rcMonitor.left;
			height = info.rcMonitor.bottom - info.rcMonitor.top;
			return S_OK;
		}
	}

	return E_FAIL;
}
