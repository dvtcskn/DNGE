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

#include "pch.h"
#include <algorithm>
#include "D3D11Viewport.h"
#include "D3D11FrameBuffer.h"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#if _DEBUG
	#pragma comment(lib, "dxguid.lib")
#endif

D3D11Viewport::D3D11Viewport(D3D11Device* InOwner, ComPtr<IDXGIFactory4> InFactory, std::uint32_t InSizeX, std::uint32_t InSizeY, bool IsFullscreen, HWND InHandle)
	: Owner(InOwner)
	, SizeX(InSizeX)
	, SizeY(InSizeY)
	, BackBufferFormat(EFormat::BGRA8_UNORM)
	, WindowHandle(InHandle)
	, BackBufferCount(2)
	, CurrentBackBuffer(0)
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
	m_SwapChainDesc.BufferUsage = DXGI_USAGE_SHADER_INPUT | DXGI_USAGE_RENDER_TARGET_OUTPUT;
	m_SwapChainDesc.BufferCount = BackBufferCount;
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
	// Create a SwapChain from a Win32 window.
	HRESULT HR = InFactory->CreateSwapChainForHwnd(
		Owner->GetDevice(),
		InHandle,
		&m_SwapChainDesc,
		&fsSwapChainDesc, nullptr, swapChain1.ReleaseAndGetAddressOf()
	);

	InFactory->MakeWindowAssociation(InHandle, DXGI_MWA_VALID);

	swapChain1.As(&SwapChain);
	//SwapChain->GetBuffer(0, IID_PPV_ARGS(&DXGIBackBuffer));
	//DXGIBackBuffer = nullptr;

	CreateRenderTarget();

	viewport = { 0.0f, 0.0f, (float)InSizeX,  (float)InSizeY, 0.0f, 1.0f };

	GetClientRect(InHandle, &Rect);
}

D3D11Viewport::~D3D11Viewport()
{
	SwapChain = nullptr;

	Owner = nullptr;
	WindowHandle = nullptr;

	for (auto& RT : RenderTargets)
		RT = nullptr;
	RenderTargets.clear();
	BackBufferShaderResourceView = nullptr;
	DXGIBackBufferTexture = nullptr;
}

void D3D11Viewport::Present(IRenderTarget* pRT)
{
	Owner->GetDeviceIMContext()->CopyResource(DXGIBackBufferTexture.Get(), static_cast<D3D11RenderTarget*>(pRT)->GetD3D11Texture());

	SwapChain->Present(!bIsVSYNCEnabled ? 0 : SyncInterval, !bIsVSYNCEnabled && !bIsFullScreen ? DXGI_PRESENT_ALLOW_TEARING : 0);
}

HRESULT D3D11Viewport::CreateRenderTarget()
{
	HRESULT hr = E_FAIL;

	{
		hr = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)DXGIBackBufferTexture.GetAddressOf());
		if (FAILED(hr))
			return hr;
	}

	//for (size_t i = 0; i < BackBufferCount; i++)
	{
		D3D11_RENDER_TARGET_VIEW_DESC Desc;
		Desc.Format = DXGI_FORMAT_UNKNOWN;
		Desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		Desc.Texture2D.MipSlice = 0;

		ComPtr<ID3D11RenderTargetView> Temp = nullptr;
		hr = Owner->GetDevice()->CreateRenderTargetView(DXGIBackBufferTexture.Get(), &Desc, Temp.GetAddressOf());
		if (FAILED(hr))
			return hr;

		RenderTargets.push_back(Temp);
	}

	{
		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
		SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MostDetailedMip = 0;
		SRVDesc.Texture2D.MipLevels = 1;
		hr = Owner->GetDevice()->CreateShaderResourceView(DXGIBackBufferTexture.Get(), &SRVDesc, BackBufferShaderResourceView.GetAddressOf());

		if (FAILED(hr))
			return hr;
	}

	return hr;
}

void D3D11Viewport::FullScreen(const bool value)
{
	if (bIsFullScreen == value)
		return;

	bIsFullScreen = value;

	SwapChain->SetFullscreenState(value, nullptr);

	ResizeSwapChain(SizeX, SizeY);
}

void D3D11Viewport::Vsync(const bool value)
{
	bIsVSYNCEnabled = value;
}

void D3D11Viewport::VsyncInterval(const UINT value)
{
	SyncInterval = value;
}

void D3D11Viewport::ResizeSwapChain(std::size_t Width, std::size_t Height)
{
	SizeX = (UINT)Width;
	SizeY = (UINT)Height;

	ID3D11RenderTargetView *nullRTV = NULL;

	ID3D11DeviceContext *context = Owner->GetDeviceIMContext();
	//Owner->GetImmediateContext(&context);

	context->OMSetRenderTargets(1, &nullRTV, NULL);
	
	for (auto& RT : RenderTargets)
		RT = nullptr;
	RenderTargets.clear();
	BackBufferShaderResourceView = nullptr;
	DXGIBackBufferTexture = nullptr;

	if (SwapChain)
	{
		// Resize the swap chain
		HRESULT hr = SwapChain->ResizeBuffers(BackBufferCount, SizeX, SizeY, ConvertFormat_Format_To_DXGI(BackBufferFormat),
			DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING | DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

		if (hr == S_OK)
			viewport = { 0.0f, 0.0f, (float)SizeX,  (float)SizeY, 0.0f, 1.0f };
		else
			DebugBreak();

		CreateRenderTarget();
	}

	if (bIsFullScreen)
	{
		SwapChain->SetFullscreenState(false, nullptr);
		SwapChain->SetFullscreenState(true, nullptr);
	}
}

std::vector<sDisplayMode> D3D11Viewport::GetAllSupportedResolutions() const
{
	std::vector<sDisplayMode> Result;
	{
		HRESULT HResult = S_OK;
		IDXGIAdapter1* Adapter = Owner->GetAdapter();

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

		// TODO: GetDisplayModeList is a terribly SLOW call.  It can take up to a second per invocation.
		//  We might want to work around some DXGI badness here.
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
