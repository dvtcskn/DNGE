
#include "pch.h"
#include "WindowsApplication.h"
#include <Windows.h>

#include <combaseapi.h>

#if defined(DEBUG) | defined(_DEBUG)
	#ifndef  _CRTDBG_MAP_ALLOC
		#define _CRTDBG_MAP_ALLOC
	#endif // ! _CRTDBG_MAP_ALLOC

	#include <stdlib.h>
	#include <crtdbg.h>
#endif

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
	//_CrtSetBreakAlloc(0);
#endif

	HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);

	std::unique_ptr<WindowsApplication> App = std::make_unique<WindowsApplication>();
	App->RunEngine();
	App = nullptr;

	CoUninitialize();

#if defined(DEBUG) | defined(_DEBUG)
	//_CrtDumpMemoryLeaks();
#endif

	return 0;
}
