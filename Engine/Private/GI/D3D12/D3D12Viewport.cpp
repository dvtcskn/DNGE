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
#include <algorithm>
#include "D3D12FrameBuffer.h"
#include "D3D12Viewport.h"
#include "D3D12Device.h"
#include "GI/D3DShared/D3DShared.h"
#include "D3D12CommandBuffer.h"
#include "D3D12Fence.h"
#include "Utilities/FileManager.h"
#include <dwmapi.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "Dwmapi.lib")

#if Pix3_Enabled && _DEBUG
#include <pix3.h>
#endif

RECT clientRectToScreenSpace(HWND const hWnd)
{
	RECT rc{ 0 };
	if (GetClientRect(hWnd, &rc) == TRUE)
	{
		if (MapWindowPoints(hWnd, nullptr, reinterpret_cast<POINT*>(&rc), 2) == 0)
		{
		}
	}

	return rc;
}

bool isDirectFlip(IDXGISwapChain* swapChain, RECT monitorExtents)
{
	bool bResult = false;

	IDXGIOutput* dxgiOutput = nullptr;
	if (SUCCEEDED(swapChain->GetContainingOutput(&dxgiOutput)))
	{
		IDXGIOutput6* dxgiOutput6 = nullptr;
		if (SUCCEEDED(dxgiOutput->QueryInterface(IID_PPV_ARGS(&dxgiOutput6))))
		{
			UINT hwSupportFlags = 0;
			if (SUCCEEDED(dxgiOutput6->CheckHardwareCompositionSupport(&hwSupportFlags)))
			{
				// check support in fullscreen mode
				if (hwSupportFlags & DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_FULLSCREEN)
				{
					DXGI_SWAP_CHAIN_DESC desc{};
					if (SUCCEEDED(swapChain->GetDesc(&desc)))
					{
						RECT windowExtents = clientRectToScreenSpace(desc.OutputWindow);
						bResult |= memcmp(&windowExtents, &monitorExtents, sizeof(windowExtents)) == 0;
					}
				}

				// check support in windowed mode
				if (hwSupportFlags & DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_WINDOWED)
				{
					bResult |= true;
				}
			}

			dxgiOutput6->Release();
		}

		dxgiOutput->Release();
	}
	return bResult;
}

void D3D12Viewport::BackBufferRTV::Release()
{
	if (renderTarget)
	{
		renderTarget.Reset();
		//renderTarget = nullptr;
	}
}

D3D12Viewport::SharedPtr D3D12Viewport::Create(D3D12Device* InOwner, ComPtr<IDXGIFactory7> InFactory, std::uint32_t InSizeX, std::uint32_t InSizeY, bool bInIsFullscreen, HWND InHandle)
{
	return std::make_shared<D3D12Viewport>(InOwner, InFactory, InSizeX, InSizeY, bInIsFullscreen, InHandle);
}

D3D12Viewport::D3D12Viewport(D3D12Device* InOwner, ComPtr<IDXGIFactory7> InFactory, std::uint32_t InSizeX, std::uint32_t InSizeY, bool IsFullscreen, HWND InHandle)
	: Owner(InOwner)
	, SizeX(InSizeX)
	, SizeY(InSizeY)
	, BackBufferFormat(EFormat::BGRA8_UNORM)
	, WindowHandle(InHandle)
	, SwapChainBufferCount(2)
	, CurrentBackBuffer(0)
	, m_rtvDescriptorSize(0)
	, SyncInterval(1)
	, bIsFullScreen(IsFullscreen)
	, bIsVSYNCEnabled(false)
	, FrameIndex(0)
{
	// Query all connected outputs
	EnumerateOutputs();

	// Find current output window is being displayed to
	FindCurrentOutput();

	// Find all HDR modes supported by current display
	EnumerateHDRModes();

	// See if display mode requested in config is supported and return it, or default to LDR
	CurrentDisplayMode = CheckAndGetDisplayModeRequested(DisplayMode::DISPLAYMODE_LDR);

	if (BackBufferFormat == EFormat::UNKNOWN)
	{
		BackBufferFormat = GetDisplayFormat(CurrentDisplayMode);
	}

	PopulateHDRMetadataBasedOnDisplayMode();

	m_rtvDescriptorSize = Owner->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	viewport = { 0.0f, 0.0f, (float)InSizeX,  (float)InSizeY, 0.0f, 1.0f };

	PresentParams.DirtyRectsCount = 0;
	PresentParams.pDirtyRects = 0;
	PresentParams.pScrollRect = 0;
	PresentParams.pScrollOffset = 0;

	SecureZeroMemory(&SwapChainDesc, sizeof(SwapChainDesc));
	SwapChainDesc.Width = InSizeX;
	SwapChainDesc.Height = InSizeY;
	SwapChainDesc.Format = ConvertFormat_Format_To_DXGI(BackBufferFormat);
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.BufferCount = SwapChainBufferCount;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SampleDesc.Quality = 0;
	SwapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING | DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	SwapChainFullscreenDesc = {};
	SwapChainFullscreenDesc.Windowed = !bIsFullScreen;

	Owner->AllocateDescriptor(&RTV[0].Handle);
	Owner->AllocateDescriptor(&RTV[1].Handle);

	CreateSwapChain(InFactory.Get());
}

D3D12Viewport::~D3D12Viewport()
{
	{
		//Owner->GPUSignal();
		//Owner->CpuWait(D3D12_COMMAND_LIST_TYPE_DIRECT);

		SwapChain->SetFullscreenState(false, nullptr);
	}

	SupportedDisplayModes.clear();

	CurrentOutput = nullptr;
	for (auto& output : AttachedOutputs)
		output = nullptr;
	AttachedOutputs.clear();

	SwapChain->AddRef();
	SwapChain = nullptr;

	for (std::size_t i = 0; i < SwapChainBufferCount; i++)
		RTV[i].Release();

	WindowHandle = nullptr;
	Owner = nullptr;
}

void D3D12Viewport::CreateSwapChain(IDXGIFactory7* InFactory)
{
	if (SwapChain)
		return;

	ComPtr<IDXGISwapChain1> swapChain1;
	HRESULT HR = InFactory->CreateSwapChainForHwnd(
		Owner->GetGraphicsQueue(),
		WindowHandle,
		&SwapChainDesc,
		&SwapChainFullscreenDesc, nullptr, swapChain1.ReleaseAndGetAddressOf()
	);
	swapChain1.As(&SwapChain);
	swapChain1->Release();
	//SwapChain->GetBuffer(0, IID_PPV_ARGS(&DXGIBackBuffer));
	//DXGIBackBuffer = nullptr;

	//RECT Rect;
	//GetClientRect(InHandle, &Rect);

	SwapChain->AddRef();

	InFactory->MakeWindowAssociation(WindowHandle, DXGI_MWA_NO_WINDOW_CHANGES);

	CreateRenderTargets();

	CurrentBackBuffer = SwapChain->GetCurrentBackBufferIndex();

	SetHDRMetadataAndColorspace();
}

void D3D12Viewport::RecreateSwapChain()
{
	IDXGIFactory7* Factory = nullptr;
	if (SUCCEEDED(SwapChain->GetParent(IID_PPV_ARGS(&Factory))))
	{
		for (std::size_t i = 0; i < SwapChainBufferCount; i++)
			RTV[i].Release();
		auto RefCount = SwapChain.Reset();
		CreateSwapChain(Factory);
	}
}

void D3D12Viewport::SetSwapChain(ComPtr<IDXGISwapChain4> InSwapChain)
{
	for (std::size_t i = 0; i < SwapChainBufferCount; i++)
		RTV[i].Release();
	SwapChain = InSwapChain;
	if (!SwapChain)
		return;

	IDXGIFactory7* Factory = nullptr;
	if (SUCCEEDED(SwapChain->GetParent(IID_PPV_ARGS(&Factory))))
		Factory->MakeWindowAssociation(WindowHandle, DXGI_MWA_NO_WINDOW_CHANGES);

	CurrentBackBuffer = SwapChain->GetCurrentBackBufferIndex();

	CreateRenderTargets();

	SetHDRMetadataAndColorspace();
}

DXGI_FORMAT D3D12Viewport::GetDXGIFormat() const
{
	return ConvertFormat_Format_To_DXGI(BackBufferFormat);
}

void D3D12Viewport::CreateRenderTargets()
{
	for (std::size_t i = 0; i < SwapChainBufferCount; i++)
		RTV[i].Release();

	const auto Device = Owner->GetDevice();

	{
		for (UINT n = 0; n < SwapChainBufferCount; n++)
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(RTV[n].Handle.GetCPU());

			SwapChain->GetBuffer(n, IID_PPV_ARGS(&RTV[n].renderTarget));
			Device->CreateRenderTargetView(RTV[n].renderTarget.Get(), nullptr, rtvHandle);
		}
	}
}

void D3D12Viewport::BeginFrame()
{
	CurrentBackBuffer = SwapChain->GetCurrentBackBufferIndex();
}

void D3D12Viewport::OnPresent(IRenderTarget* pRT)
{
	if (!pRT)
		return;

	auto CMDList = Owner->GetIMCommandList();
	CMDList->BeginRecordCommandList();
#if Pix3_Enabled && _DEBUG
	PIXBeginEvent(CMDList->Get(), 0, L"D3D12Viewport::OnPresent");
#endif

	CMDList->CopyResource(RTV[CurrentBackBuffer].renderTarget.Get(), D3D12_RESOURCE_STATE_PRESENT, static_cast<D3D12RenderTarget*>(pRT)->GetD3D12Texture(), static_cast<D3D12RenderTarget*>(pRT)->CurrentState);

#if Pix3_Enabled && _DEBUG
	PIXEndEvent(CMDList->Get());
#endif
	CMDList->FinishRecordCommandList();
	CMDList->ExecuteCommandList(ECommandContextExecuteType::Deferred, std::uint32_t(-1));
}

void D3D12Viewport::Present()
{
	SwapChain->Present(!bIsVSYNCEnabled ? 0 : SyncInterval, !bIsVSYNCEnabled && !bIsFullScreen ? DXGI_PRESENT_ALLOW_TEARING : 0);

	//if (Owner->IsSoftwareDevice())
	//	Owner->CpuWait(D3D12_COMMAND_LIST_TYPE_DIRECT);

	++FrameIndex;
}

void D3D12Viewport::ResizeSwapChain(std::size_t Width, std::size_t Height)
{
	SizeX = (uint32_t)Width;
	SizeY = (uint32_t)Height;
	viewport = { 0.0f, 0.0f, (float)SizeX,  (float)SizeY, 0.0f, 1.0f };

	for (std::size_t i = 0; i < SwapChainBufferCount; i++)
		RTV[i].Release();

	Owner->GPUSignal();
	Owner->CpuWait(D3D12_COMMAND_LIST_TYPE_DIRECT);

	SwapChain->ResizeBuffers(SwapChainBufferCount, SizeX, SizeY, ConvertFormat_Format_To_DXGI(BackBufferFormat),
		DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING | DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

	CurrentBackBuffer = SwapChain->GetCurrentBackBufferIndex();

	CreateRenderTargets();

	if (bIsFullScreen)
	{
		SwapChain->SetFullscreenState(false, nullptr);
		SwapChain->SetFullscreenState(true, nullptr);
	}

	SetHDRMetadataAndColorspace();
}

void D3D12Viewport::FullScreen(const bool value)
{
	if (bIsFullScreen == value)
		return;

	bIsFullScreen = value;

	SwapChain->SetFullscreenState(value, nullptr);

	ResizeSwapChain(SizeX, SizeY);
}

void D3D12Viewport::Vsync(const bool value)
{
	bIsVSYNCEnabled = value;
}

void D3D12Viewport::VsyncInterval(const UINT value)
{
	SyncInterval = value;
}

DisplayMode D3D12Viewport::CheckAndGetDisplayModeRequested(DisplayMode DispMode)
{
	for (const auto& it : SupportedDisplayModes)
	{
		if (it == DispMode)
			return it;
	}

	if (DispMode == DisplayMode::DISPLAYMODE_FSHDR_2084) {

		Engine::WriteToConsole(L"FSHDR PQ not supported, trying HDR10 PQ");
		return CheckAndGetDisplayModeRequested(DisplayMode::DISPLAYMODE_HDR10_2084);
	}
	else if (DispMode == DisplayMode::DISPLAYMODE_FSHDR_SCRGB) {

		Engine::WriteToConsole(L"FSHDR SCRGB not supported, trying HDR10 SCRGB");
		return CheckAndGetDisplayModeRequested(DisplayMode::DISPLAYMODE_HDR10_SCRGB);
	}

	if (DispMode != DisplayMode::DISPLAYMODE_LDR)
	{
		Engine::WriteToConsole(L"HDR modes not supported, defaulting to LDR");
	}

	return DisplayMode::DISPLAYMODE_LDR;
}

void D3D12Viewport::PopulateHDRMetadataBasedOnDisplayMode()
{
	switch (CurrentDisplayMode)
	{
	case DisplayMode::DISPLAYMODE_LDR:
		// Values set here make no difference on HDR wide gamut monitors
		// Monitors will not undersell their capabilities, if they can go beyond rec709 gamut and 100 nits.

		// [0, 1] in respective RGB channel maps to display gamut.
		HDR.RedPrimary[0] = 0.64f;
		HDR.RedPrimary[1] = 0.33f;
		HDR.GreenPrimary[0] = 0.30f;
		HDR.GreenPrimary[1] = 0.60f;
		HDR.BluePrimary[0] = 0.15f;
		HDR.BluePrimary[1] = 0.06f;
		HDR.WhitePoint[0] = 0.3127f;
		HDR.WhitePoint[1] = 0.3290f;

		// [0, 1] actually maps to display brightness.
		// This gets ignored, writing it for completeness
		HDR.MinLuminance = 0.0f;
		HDR.MaxLuminance = 100.0f;

		// Scene dependent
		HDR.MaxContentLightLevel = 2000.0f;
		HDR.MaxFrameAverageLightLevel = 500.0f;
		break;

	case DisplayMode::DISPLAYMODE_HDR10_2084:
		// Values set here either get clipped at display capabilities or tone and gamut mapped on the display to fit its brightness and gamut range.

		// rec 2020 primaries
		HDR.RedPrimary[0] = 0.708f;
		HDR.RedPrimary[1] = 0.292f;
		HDR.GreenPrimary[0] = 0.170f;
		HDR.GreenPrimary[1] = 0.797f;
		HDR.BluePrimary[0] = 0.131f;
		HDR.BluePrimary[1] = 0.046f;
		HDR.WhitePoint[0] = 0.3127f;
		HDR.WhitePoint[1] = 0.3290f;

		// Max nits of 500 is actually low
		// Can set this value to 1000, 2000 and 4000 based on target display and content contrast range
		// However we are trying to make sure HDR10 mode doesn't look bad on HDR displays with only 300 nits brightness, hence the low max lum value.
		HDR.MinLuminance = 0.0f;
		HDR.MaxLuminance = 500.0f;

		// Scene dependent
		HDR.MaxContentLightLevel = 2000.0f;
		HDR.MaxFrameAverageLightLevel = 500.0f;
		break;

	case DisplayMode::DISPLAYMODE_HDR10_SCRGB:
		// Same as above

		// rec 709 primaries
		HDR.RedPrimary[0] = 0.64f;
		HDR.RedPrimary[1] = 0.33f;
		HDR.GreenPrimary[0] = 0.30f;
		HDR.GreenPrimary[1] = 0.60f;
		HDR.BluePrimary[0] = 0.15f;
		HDR.BluePrimary[1] = 0.06f;
		HDR.WhitePoint[0] = 0.3127f;
		HDR.WhitePoint[1] = 0.3290f;

		// Same comment as HDR10_2084.
		HDR.MinLuminance = 0.0f;
		HDR.MaxLuminance = 500.0f;

		// Scene dependent
		HDR.MaxContentLightLevel = 2000.0f;
		HDR.MaxFrameAverageLightLevel = 500.0f;
		break;

	case DisplayMode::DISPLAYMODE_FSHDR_2084:
	case DisplayMode::DISPLAYMODE_FSHDR_SCRGB:
		// FS HDR modes should already have the monitor's primaries queried through backend API's like DXGI or VK ext

		// Scene dependent
		HDR.MaxContentLightLevel = 2000.0f;
		HDR.MaxFrameAverageLightLevel = 500.0f;
		break;
	}
}

void D3D12Viewport::EnumerateOutputs()
{
	IDXGIAdapter1* Adapter = Owner->GetAdapter();

	ComPtr<IDXGIOutput> pOutput = nullptr;

	for (UINT outputIndex = 0; Adapter->EnumOutputs(outputIndex, &pOutput) != DXGI_ERROR_NOT_FOUND; outputIndex++)
	{
		ComPtr<IDXGIOutput6> pOutput6;
		ThrowIfFailed(pOutput->QueryInterface(__uuidof(IDXGIOutput6), (void**)&pOutput6));
		AttachedOutputs.push_back(pOutput6);
	}
}

void D3D12Viewport::FindCurrentOutput()
{
	RECT windowRect;
	GetWindowRect(WindowHandle, &windowRect);

	float bestIntersectArea = -1.0f;
	for (auto output : AttachedOutputs)
	{
		DXGI_OUTPUT_DESC outputDesc;
		ThrowIfFailed(output->GetDesc(&outputDesc));
		RECT outputRect = outputDesc.DesktopCoordinates;

		if (IntersectWindowAndOutput(windowRect, outputRect, bestIntersectArea))
		{
			CurrentOutput = output;
		}
	}
}

void D3D12Viewport::EnumerateHDRModes()
{
	// If we are attached via remote desktop, there will be no current output, so just return (will assume LDR display)
	if (!CurrentOutput)
		return;

	SupportedDisplayModes.clear();

	DXGI_OUTPUT_DESC1 outputDesc1;
	ThrowIfFailed(CurrentOutput->GetDesc1(&outputDesc1));

	if (outputDesc1.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020)
	{
		SupportedDisplayModes.push_back(DisplayMode::DISPLAYMODE_HDR10_2084);
		SupportedDisplayModes.push_back(DisplayMode::DISPLAYMODE_HDR10_SCRGB);

		// Init HDR metadata with queried values in DXGI Output Desc
		// Will be used as is for FS HDR mdoe
		HDR.RedPrimary[0] = outputDesc1.RedPrimary[0];
		HDR.RedPrimary[1] = outputDesc1.RedPrimary[1];
		HDR.GreenPrimary[0] = outputDesc1.GreenPrimary[0];
		HDR.GreenPrimary[1] = outputDesc1.GreenPrimary[1];
		HDR.BluePrimary[0] = outputDesc1.BluePrimary[0];
		HDR.BluePrimary[0] = outputDesc1.BluePrimary[1];
		HDR.WhitePoint[0] = outputDesc1.WhitePoint[0];
		HDR.WhitePoint[1] = outputDesc1.WhitePoint[1];
		HDR.MinLuminance = outputDesc1.MinLuminance;
		HDR.MaxLuminance = outputDesc1.MaxLuminance;

#if AGS_Enable
		CheckFSHDRSupport();
#endif
	}
}

#if AGS_Enable
void D3D12Viewport::CheckFSHDRSupport()
{
	// Check FS2 HDR feature if AGS is enabled
	if (auto AGS = Owner->GetAGSContext())
	{
		//Assert(ASSERT_WARNING, GetDevice()->GetImpl()->GetAGSGpuInfo()->numDevices == 1,
		//	L"Following AGS Freesync Premium Pro HDR feature enablement assumes single GPU setup");

		auto AGS_GPUInfo = Owner->GetAGSGpuInfo();
		for (int32_t i = 0; i < AGS_GPUInfo.numDevices; i++)
		{
			const AGSDeviceInfo& device = AGS_GPUInfo.devices[i];

			if (FileManager::StringToWstring(std::string(device.adapterString)) == std::wstring(Owner->GetDeviceName()))
			{
				// Find display, app window is rendering to in ags display list
				// Unfortunately we cannot use CurrentOutput
				int        displayIndexAGS = -1;
				float      bestIntersectArea = -1.0f;
				for (int j = 0; j < device.numDisplays; j++)
				{
					RECT windowRect;
					GetWindowRect(WindowHandle, &windowRect);
					AGSRect AGSmonitorRect = device.displays[j].currentResolution;
					RECT    monitorRect = { AGSmonitorRect.offsetX,
										AGSmonitorRect.offsetY,
										AGSmonitorRect.offsetX + AGSmonitorRect.width,
										AGSmonitorRect.offsetY + AGSmonitorRect.height };
					if (IntersectWindowAndOutput(windowRect, monitorRect, bestIntersectArea))
					{
						displayIndexAGS = j;
					}
				}

				//Assert(ASSERT_ERROR, displayIndexAGS != -1, L"AGS could not find monitor GPU is rendering to.");

				// Check for FS2 HDR support
				if (displayIndexAGS != -1 && device.displays[displayIndexAGS].freesyncHDR)
				{
					SupportedDisplayModes.push_back(DisplayMode::DISPLAYMODE_FSHDR_2084);
					SupportedDisplayModes.push_back(DisplayMode::DISPLAYMODE_FSHDR_SCRGB);
					break;
				}
			}
		}
	}
}
#endif

void D3D12Viewport::SetHDRMetadataAndColorspace()
{
	DXGI_HDR_METADATA_HDR10 HDR10MetaData = {};

	// Chroma values are normalized to 50,000.
	HDR10MetaData.RedPrimary[0] = static_cast<UINT16>(HDR.RedPrimary[0] * 50000.0f);
	HDR10MetaData.RedPrimary[1] = static_cast<UINT16>(HDR.RedPrimary[1] * 50000.0f);
	HDR10MetaData.GreenPrimary[0] = static_cast<UINT16>(HDR.GreenPrimary[0] * 50000.0f);
	HDR10MetaData.GreenPrimary[1] = static_cast<UINT16>(HDR.GreenPrimary[1] * 50000.0f);
	HDR10MetaData.BluePrimary[0] = static_cast<UINT16>(HDR.BluePrimary[0] * 50000.0f);
	HDR10MetaData.BluePrimary[1] = static_cast<UINT16>(HDR.BluePrimary[1] * 50000.0f);
	HDR10MetaData.WhitePoint[0] = static_cast<UINT16>(HDR.WhitePoint[0] * 50000.0f);
	HDR10MetaData.WhitePoint[1] = static_cast<UINT16>(HDR.WhitePoint[1] * 50000.0f);

	// Max luminance value is absolute.
	HDR10MetaData.MaxMasteringLuminance = static_cast<UINT>(HDR.MaxLuminance);

	// Min luminance value is normalized to 10,000.
	HDR10MetaData.MinMasteringLuminance = static_cast<UINT>(HDR.MinLuminance * 10000.0f);

	// Max content and frame average light level values are absolute.
	HDR10MetaData.MaxContentLightLevel = static_cast<UINT16>(HDR.MaxContentLightLevel);
	HDR10MetaData.MaxFrameAverageLightLevel = static_cast<UINT16>(HDR.MaxFrameAverageLightLevel);

	// Set HDR meta data.
	ThrowIfFailed(SwapChain->SetHDRMetaData(DXGI_HDR_METADATA_TYPE_HDR10, sizeof(DXGI_HDR_METADATA_HDR10), &HDR10MetaData));

	// Set color space.
	switch (CurrentDisplayMode)
	{
	case DisplayMode::DISPLAYMODE_LDR:
		ThrowIfFailed(SwapChain->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709));
		break;
	case DisplayMode::DISPLAYMODE_FSHDR_2084:
	case DisplayMode::DISPLAYMODE_HDR10_2084:
		// FS HDR mode will only use PQ rec2020 as a final swapchain back buffer "transport container" to the driver
		// It is not to be confused as its required colour space and transfer function
		// We tone and gamut map FS HDR to display's native capabilities
		ThrowIfFailed(SwapChain->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020));
		break;
	case DisplayMode::DISPLAYMODE_FSHDR_SCRGB:
	case DisplayMode::DISPLAYMODE_HDR10_SCRGB:
		ThrowIfFailed(SwapChain->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709));
		break;
	}
}

bool D3D12Viewport::IntersectWindowAndOutput(const RECT& windowRect, const RECT& outputRect, float& bestIntersectArea)
{
	LONG ax1 = windowRect.left;
	LONG ay1 = windowRect.top;
	LONG ax2 = windowRect.right;
	LONG ay2 = windowRect.bottom;

	LONG bx1 = outputRect.left;
	LONG by1 = outputRect.top;
	LONG bx2 = outputRect.right;
	LONG by2 = outputRect.bottom;

	// Compute the intersection
	LONG intersectArea = ComputeIntersectionArea(ax1, ay1, ax2, ay2, bx1, by1, bx2, by2);
	if (intersectArea > bestIntersectArea)
	{
		bestIntersectArea = static_cast<float>(intersectArea);
		return true;
	}

	return false;
}

std::vector<sDisplayDesc> D3D12Viewport::GetAllSupportedResolutions() const
{
	std::vector<sDisplayDesc> Result;
	HRESULT HResult = S_OK;
	IDXGIAdapter1* Adapter = Owner->GetAdapter();
	IDXGIOutput* Output;
	UINT iOutput = 0;

	while (DXGI_ERROR_NOT_FOUND != Adapter->EnumOutputs(iOutput++, &Output))
	{
		// get the description of the adapter
		DXGI_ADAPTER_DESC AdapterDesc;
		Adapter->GetDesc(&AdapterDesc);

		DXGI_OUTPUT_DESC desc;
		HResult = Output->GetDesc(&desc);

		MONITORINFOEXW monInfoEx;
		monInfoEx.cbSize = sizeof(MONITORINFOEXW);
		GetMonitorInfoW(desc.Monitor, &monInfoEx);

		DISPLAY_DEVICEW dispDev;
		dispDev.cb = sizeof(DISPLAY_DEVICEW);
		EnumDisplayDevicesW(monInfoEx.szDevice, 0, &dispDev, 0);

		DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		std::uint32_t NumModes = 0;
		HResult = Output->GetDisplayModeList(Format, 0, &NumModes, NULL);
		if (HResult == DXGI_ERROR_NOT_FOUND)
		{
			break;
		}
		else if (HResult == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE)
		{
			break;
		}

		DXGI_MODE_DESC* ModeList = new DXGI_MODE_DESC[NumModes];
		Output->GetDisplayModeList(Format, 0, &NumModes, ModeList);

		for (std::uint32_t m = 0; m < NumModes; m++)
		{
			sDisplayDesc Mode;
			std::wstring ID = dispDev.DeviceID;
			ID.insert(0, L" (");
			ID.append(L")");
			std::wstring Name = dispDev.DeviceString + ID;
			Mode.Name = FileManager::WideStringToString(Name);
			Mode.Width = ModeList[m].Width;
			Mode.Height = ModeList[m].Height;
			Mode.RefreshRate.Denominator = ModeList[m].RefreshRate.Denominator;
			Mode.RefreshRate.Numerator = ModeList[m].RefreshRate.Numerator;
			Result.push_back(Mode);
		}

		delete[] ModeList;
	}
	return Result;
};

void D3D12Viewport::GetRefreshRate(double* outRefreshRate)
{
	double dwsRate = 1000.0;
	*outRefreshRate = 1000.0;

	bool         bIsPotentialDirectFlip = false;
	IDXGIOutput* dxgiOutput = nullptr;
	BOOL         isFullscreen = false;
	SwapChain->GetFullscreenState(&isFullscreen, &dxgiOutput);

	if (!isFullscreen)
	{
		DWM_TIMING_INFO compositionTimingInfo{};
		compositionTimingInfo.cbSize = sizeof(DWM_TIMING_INFO);
		double  monitorRefreshRate = 0.0f;
		HRESULT hr = DwmGetCompositionTimingInfo(nullptr, &compositionTimingInfo);
		if (SUCCEEDED(hr))
		{
			dwsRate = double(compositionTimingInfo.rateRefresh.uiNumerator) / compositionTimingInfo.rateRefresh.uiDenominator;
		}
		SwapChain->GetContainingOutput(&dxgiOutput);
	}

	// if FS this should be the monitor used for FS, in windowed the window containing the main portion of the output
	if (dxgiOutput)
	{
		IDXGIOutput1* dxgiOutput1 = nullptr;
		if (SUCCEEDED(dxgiOutput->QueryInterface(IID_PPV_ARGS(&dxgiOutput1))))
		{
			DXGI_OUTPUT_DESC outputDes{};
			if (SUCCEEDED(dxgiOutput->GetDesc(&outputDes)))
			{
				MONITORINFOEXW info{};
				info.cbSize = sizeof(info);
				if (GetMonitorInfoW(outputDes.Monitor, &info) != 0)
				{
					bIsPotentialDirectFlip = isDirectFlip(SwapChain.Get(), info.rcMonitor);

					UINT32 numPathArrayElements, numModeInfoArrayElements;
					if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &numPathArrayElements, &numModeInfoArrayElements) == ERROR_SUCCESS)
					{
						std::vector<DISPLAYCONFIG_PATH_INFO> pathArray(numPathArrayElements);
						std::vector<DISPLAYCONFIG_MODE_INFO> modeInfoArray(numModeInfoArrayElements);
						if (QueryDisplayConfig(
							QDC_ONLY_ACTIVE_PATHS, &numPathArrayElements, pathArray.data(), &numModeInfoArrayElements, modeInfoArray.data(), nullptr) ==
							ERROR_SUCCESS)
						{
							bool rateFound = false;
							// iterate through all the paths until find the exact source to match
							for (size_t i = 0; i < pathArray.size() && !rateFound; i++)
							{
								const DISPLAYCONFIG_PATH_INFO& path = pathArray[i];
								DISPLAYCONFIG_SOURCE_DEVICE_NAME sourceName;
								sourceName.header = {
									DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME, sizeof(sourceName), path.sourceInfo.adapterId, path.sourceInfo.id };

								if (DisplayConfigGetDeviceInfo(&sourceName.header) == ERROR_SUCCESS)
								{
									if (wcscmp(info.szDevice, sourceName.viewGdiDeviceName) == 0)
									{
										const DISPLAYCONFIG_RATIONAL& rate = path.targetInfo.refreshRate;
										if (rate.Denominator > 0)
										{
											double refrate = (double)rate.Numerator / (double)rate.Denominator;
											*outRefreshRate = refrate;

											// we found a valid rate?
											rateFound = (refrate > 0.0);
										}
									}
								}
							}
						}
					}
				}
			}
			dxgiOutput1->Release();
		}
		dxgiOutput->Release();

		if (!bIsPotentialDirectFlip)
		{
			*outRefreshRate = std::min(*outRefreshRate, dwsRate);
		}
	}
}
