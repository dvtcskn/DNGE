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

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#if _DEBUG
	#pragma comment(lib, "dxguid.lib")
#endif

D3D12Viewport::SharedPtr D3D12Viewport::Create(D3D12Device* InOwner, ComPtr<IDXGIFactory4> InFactory, std::uint32_t InSizeX, std::uint32_t InSizeY, bool bInIsFullscreen, HWND InHandle)
{
	return std::make_shared<D3D12Viewport>(InOwner, InFactory, InSizeX, InSizeY, bInIsFullscreen, InHandle);
}

D3D12Viewport::D3D12Viewport(D3D12Device* InOwner, ComPtr<IDXGIFactory4> InFactory, std::uint32_t InSizeX, std::uint32_t InSizeY, bool IsFullscreen, HWND InHandle)
	: Owner(InOwner)
	, SizeX(InSizeX)
	, SizeY(InSizeY)
	, BackBufferFormat(EFormat::BGRA8_UNORM)
	, WindowHandle(InHandle)
	, SwapChainBufferCount(2)
	, CurrentBackBuffer(0)
	, FrameIndex(0)
	, m_rtvDescriptorSize(0)
	, SyncInterval(1)
	, bIsFullScreen(IsFullscreen)
	, bIsVSYNCEnabled(false)
{
	PresentParams.DirtyRectsCount = 0;
	PresentParams.pDirtyRects = 0;
	PresentParams.pScrollRect = 0;
	PresentParams.pScrollOffset = 0;

	DXGI_SWAP_CHAIN_DESC1 m_SwapChainDesc;
	SecureZeroMemory(&m_SwapChainDesc, sizeof(m_SwapChainDesc));
	m_SwapChainDesc.Width = InSizeX;
	m_SwapChainDesc.Height = InSizeY;
	m_SwapChainDesc.Format = ConvertFormat_Format_To_DXGI(BackBufferFormat);
	m_SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	m_SwapChainDesc.BufferCount = SwapChainBufferCount;
	m_SwapChainDesc.SampleDesc.Count = 1;
	m_SwapChainDesc.SampleDesc.Quality = 0;
	m_SwapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	m_SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	m_SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	m_SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING | DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc;
	fsSwapChainDesc = {};
	fsSwapChainDesc.Windowed = !bIsFullScreen;

	ComPtr<IDXGISwapChain1> swapChain1;
	HRESULT HR = InFactory->CreateSwapChainForHwnd(
		Owner->GetGraphicsQueue(),
		InHandle,
		&m_SwapChainDesc,
		&fsSwapChainDesc, nullptr, swapChain1.ReleaseAndGetAddressOf()
	);

	swapChain1.As(&SwapChain);
	//SwapChain->GetBuffer(0, IID_PPV_ARGS(&DXGIBackBuffer));
	//DXGIBackBuffer = nullptr;

	InFactory->MakeWindowAssociation(InHandle, DXGI_MWA_VALID);

	FrameIndex = SwapChain->GetCurrentBackBufferIndex();

	viewport = { 0.0f, 0.0f, (float)InSizeX,  (float)InSizeY, 0.0f, 1.0f };

	//RECT Rect;
	//GetClientRect(InHandle, &Rect);

	Owner->AllocateDescriptor(&RTV[0].Handle);
	Owner->AllocateDescriptor(&RTV[1].Handle);

	m_rtvDescriptorSize = Owner->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	CreateRenderTargets();
}

D3D12Viewport::~D3D12Viewport()
{
	{
		Owner->GPUSignal();
		Owner->CPUWait();

		SwapChain->SetFullscreenState(false, nullptr);
	}

	RTV->Release();

	SwapChain = nullptr;

	Owner = nullptr;
	WindowHandle = nullptr;
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
			//rtvHandle.Offset(1, m_rtvDescriptorSize);
		}
	}
}

void D3D12Viewport::Present(IRenderTarget* pRT)
{
	CopyToBackBuffer(pRT);

	SwapChain->Present(!bIsVSYNCEnabled ? 0 : SyncInterval, !bIsVSYNCEnabled && !bIsFullScreen ? DXGI_PRESENT_ALLOW_TEARING : 0);

	FrameIndex = SwapChain->GetCurrentBackBufferIndex();

	//Owner->GPUSignal();
	Owner->CPUWait();
}

void D3D12Viewport::CopyToBackBuffer(IRenderTarget* pRT)
{
	auto CMDList = Owner->GetIMCommandList();
	CMDList->BeginRecordCommandList();

	CMDList->CopyResource(RTV[FrameIndex].renderTarget.Get(), D3D12_RESOURCE_STATE_PRESENT, static_cast<D3D12RenderTarget*>(pRT)->GetD3D12Texture(), static_cast<D3D12RenderTarget*>(pRT)->CurrentState);

	CMDList->FinishRecordCommandList();
	CMDList->ExecuteCommandList();
}

void D3D12Viewport::ResizeSwapChain(std::size_t Width, std::size_t Height)
{
	SizeX = (uint32_t)Width;
	SizeY = (uint32_t)Height;
	viewport = { 0.0f, 0.0f, (float)SizeX,  (float)SizeY, 0.0f, 1.0f };

	for (std::size_t i = 0; i < SwapChainBufferCount; i++)
		RTV[i].Release();

	Owner->GPUSignal();
	Owner->CPUWait();

	SwapChain->ResizeBuffers(SwapChainBufferCount, SizeX, SizeY, ConvertFormat_Format_To_DXGI(BackBufferFormat),
		DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING | DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

	FrameIndex = SwapChain->GetCurrentBackBufferIndex();

	CreateRenderTargets();

	if (bIsFullScreen)
	{
		SwapChain->SetFullscreenState(false, nullptr);
		SwapChain->SetFullscreenState(true, nullptr);
	}
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

std::vector<sDisplayMode> D3D12Viewport::GetAllSupportedResolutions() const
{
	std::vector<sDisplayMode> Result;
	{
		HRESULT HResult = S_OK;
		IDXGIAdapter1* Adapter = Owner->GetAdapter();

		//Owner->GetAdapter(Adapter);

		// get the description of the adapter
		DXGI_ADAPTER_DESC AdapterDesc;
		Adapter->GetDesc(&AdapterDesc);

		IDXGIOutput* Output;
		HResult = Adapter->EnumOutputs(0, &Output);
		if (DXGI_ERROR_NOT_FOUND == HResult)
		{
			return std::vector<sDisplayMode>();
		}
		if (FAILED(HResult))
		{
			return std::vector<sDisplayMode>();
		}

		DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		std::uint32_t NumModes = 0;
		HResult = Output->GetDisplayModeList(Format, 0, &NumModes, NULL);
		if (HResult == DXGI_ERROR_NOT_FOUND)
		{
			return  std::vector<sDisplayMode>();
		}
		else if (HResult == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE)
		{
			return  std::vector<sDisplayMode>();
		}

		DXGI_MODE_DESC* ModeList = new DXGI_MODE_DESC[NumModes];
		Output->GetDisplayModeList(Format, 0, &NumModes, ModeList);

		for (std::uint32_t m = 0; m < NumModes; m++)
		{
			sDisplayMode Mode;
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
