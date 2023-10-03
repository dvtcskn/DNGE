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

#include <map>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <DXGIDebug.h>
#include <vector>
#include "Engine/AbstractEngine.h"
#include "D3D11Device.h"

#include <wrl\client.h>
using namespace Microsoft::WRL;

class D3D11Viewport final
{
	sBaseClassBody(sClassConstructor, D3D11Viewport)
public:
	static std::shared_ptr<D3D11Viewport> CreateD3D11Viewport(D3D11Device* InOwner, ComPtr<IDXGIFactory4> InFactory, std::uint32_t InSizeX, std::uint32_t InSizeY, bool IsFullscreen, HWND InHandle)
	{
		return std::make_shared<D3D11Viewport>(InOwner, InFactory, InSizeX, InSizeY, IsFullscreen, InHandle);
	}

	D3D11Viewport(class D3D11Device* InOwner, ComPtr<IDXGIFactory4> InFactory, std::uint32_t InSizeX, std::uint32_t InSizeY, bool IsFullscreen, HWND InHandle);
	~D3D11Viewport();

	void Present(IRenderTarget* pRT);
	void ResizeSwapChain(std::size_t Width, std::size_t Height);

	void FullScreen(const bool value);
	void Vsync(const bool value);
	void VsyncInterval(const UINT value);

	bool IsFullScreen() const { return bIsFullScreen; }
	bool IsVsyncEnabled() const { return bIsVSYNCEnabled; }
	UINT GetVsyncInterval() const { return SyncInterval; }

	std::vector<sDisplayMode> GetAllSupportedResolutions() const;

	FORCEINLINE ID3D11RenderTargetView* GetCurrentBackBufferRT() const { return RenderTargets[0].Get(); }
	FORCEINLINE std::uint32_t GetBackBufferCount() const { return BackBufferCount; }

	FORCEINLINE ComPtr<IDXGISwapChain3> GetSwapChain() const { return SwapChain; }
	FORCEINLINE HWND GetHandle() const { return WindowHandle; }
	FORCEINLINE RECT GetRect() const { return Rect; }
	FORCEINLINE D3D11_VIEWPORT GetD3D11Viewport() { return viewport; }
	FORCEINLINE std::uint32_t GetViewportWidth() const { return SizeX; }
	FORCEINLINE std::uint32_t GetViewportHeight() const { return SizeY; }
	FORCEINLINE sViewport GetViewport() { return sViewport((std::uint32_t)viewport.Width, (std::uint32_t)viewport.Height, (std::uint32_t)viewport.TopLeftX, (std::uint32_t)viewport.TopLeftY, viewport.MinDepth, viewport.MaxDepth); }
	FORCEINLINE EFormat GetBackBufferFormat() const { return BackBufferFormat; }
	FORCEINLINE sScreenDimension GetScreenDimension() const { return sScreenDimension(SizeX, SizeY); }

private:
	HRESULT CreateRenderTarget();

private:
	D3D11Device* Owner;
	ComPtr<IDXGISwapChain3> SwapChain;

	EFormat BackBufferFormat;

	DXGI_PRESENT_PARAMETERS PresentParams;

	RECT Rect;
	D3D11_VIEWPORT viewport;
	std::uint32_t SizeX;
	std::uint32_t SizeY;
	HWND WindowHandle;
	std::uint32_t BackBufferCount;
	std::uint32_t CurrentBackBuffer;

	UINT SyncInterval;
	bool bIsFullScreen;
	bool bIsVSYNCEnabled;

	ComPtr<ID3D11Texture2D> DXGIBackBufferTexture;
	std::vector<ComPtr<ID3D11RenderTargetView>> RenderTargets;
	ComPtr<ID3D11ShaderResourceView> BackBufferShaderResourceView;
};
