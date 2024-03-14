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

#include <map>
#include <memory>
#include <string>
#include <vector>
#include "dx12.h"
#include <DXGIDebug.h>

#include <dxgi.h>
#include <dxgi1_6.h>

#include <wrl\client.h>
using namespace Microsoft::WRL;

#include "Engine/AbstractEngine.h"
#include "D3D12DescriptorHeapManager.h"

class D3D12Device;

class D3D12Viewport final
{
	sBaseClassBody(sClassConstructor, D3D12Viewport)
public:
	static SharedPtr Create(D3D12Device* InOwner, ComPtr<IDXGIFactory4> InFactory, std::uint32_t InSizeX, std::uint32_t InSizeY, bool IsFullscreen, HWND InHandle);

	D3D12Viewport(class D3D12Device* InOwner, ComPtr<IDXGIFactory4> InFactory, std::uint32_t InSizeX, std::uint32_t InSizeY, bool IsFullscreen, HWND InHandle);
	~D3D12Viewport();

	void Present(IRenderTarget* pRT);

	void ResizeSwapChain(std::size_t Width, std::size_t Height);
	void FullScreen(const bool value);
	void Vsync(const bool value);
	void VsyncInterval(const UINT value);

	bool IsFullScreen() const { return bIsFullScreen; }
	bool IsVsyncEnabled() const { return bIsVSYNCEnabled; }
	UINT GetVsyncInterval() const { return SyncInterval; }

	std::vector<sDisplayMode> GetAllSupportedResolutions() const;

	FORCEINLINE ComPtr<IDXGISwapChain3> GetSwapChain() const { return SwapChain; }
	FORCEINLINE HWND GetHandle() const { return WindowHandle; }
	FORCEINLINE const D3D12_VIEWPORT& GetD3D12Viewport() { return viewport; }
	FORCEINLINE std::uint32_t GetViewportWidth() const { return SizeX; }
	FORCEINLINE std::uint32_t GetViewportHeight() const { return SizeY; }
	FORCEINLINE std::uint64_t GetFrameIndex() const { return FrameIndex; }
	FORCEINLINE sViewport GetViewport() { return sViewport((std::uint32_t)viewport.Width, (std::uint32_t)viewport.Height, (std::uint32_t)viewport.TopLeftX, (std::uint32_t)viewport.TopLeftY, viewport.MinDepth, viewport.MaxDepth); }
	FORCEINLINE EFormat GetBackBufferFormat() const { return BackBufferFormat; }
	FORCEINLINE sScreenDimension GetScreenDimension() const { return sScreenDimension(SizeX, SizeY); }

	void CreateRenderTargets();
	void CopyToBackBuffer(IRenderTarget* pRT);

private:
	D3D12Device* Owner;
	HWND WindowHandle;

	EFormat BackBufferFormat;

	D3D12_VIEWPORT viewport;
	std::uint32_t SizeX;
	std::uint32_t SizeY;

	ComPtr<IDXGISwapChain3> SwapChain;

	struct BackBufferRTV
	{
		BackBufferRTV()
			: renderTarget(nullptr)
			, Handle(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
		{}

		~BackBufferRTV()
		{
			Release();
		};

		void Release()
		{
			renderTarget = nullptr;
		}

		ComPtr<ID3D12Resource> renderTarget;
		D3D12DescriptorHandle Handle;
	};
	UINT m_rtvDescriptorSize;
	BackBufferRTV RTV[2];

	UINT SwapChainBufferCount;
	std::uint32_t CurrentBackBuffer;

	std::uint64_t FrameIndex;

	DXGI_PRESENT_PARAMETERS PresentParams;

	UINT SyncInterval;
	bool bIsFullScreen;
	bool bIsVSYNCEnabled;
};
