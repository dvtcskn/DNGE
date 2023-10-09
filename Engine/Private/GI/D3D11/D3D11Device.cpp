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
#include <iostream>
#include <DXGIDebug.h>
#include "D3D11Device.h"
#include "D3D11Buffer.h"
#include "D3D11Viewport.h"
#include "D3D11Pipeline.h"
#include "D3D11FrameBuffer.h"
#include "D3D11Texture.h"
#include "D3D11ShaderStates.h"
#include "D3D11CommandBuffer.h"
#include "D3D11ComputeCommandContext.h"
#include "Engine/AbstractEngine.h"

#define DEBUG_D3DDEVICE 1

D3D11Device::D3D11Device(std::optional<short> InGPUIndex)
	: VendorId(0)
	, GPUIndex(InGPUIndex)
{
	auto WideStringToString = [](const std::wstring& utf16) -> std::string
	{
		const int utf16Length = (int)utf16.length() + 1;
		const int len = WideCharToMultiByte(0, 0, utf16.data(), utf16Length, 0, 0, 0, 0);
		std::string STR(len, '\0');
		WideCharToMultiByte(0, 0, utf16.data(), utf16Length, STR.data(), len, 0, 0);
		return STR;
	};

	std::uint32_t m_dxgiFactoryFlags = 0;
#if _DEBUG && DEBUG_D3DDEVICE
	{
		ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
		{
			m_dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

			DXGI_INFO_QUEUE_MESSAGE_ID hide[] =
			{
				80,
			};
			DXGI_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
			filter.DenyList.pIDList = hide;
			dxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
		}
	}
#endif

	CreateDXGIFactory2(m_dxgiFactoryFlags, IID_PPV_ARGS(&DXGIFactory));

	std::uint32_t createFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if _DEBUG && DEBUG_D3DDEVICE
	createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	IDXGIAdapter1* pAdapter = GetAdapter(GPUIndex);
	DXGI_ADAPTER_DESC1 AdapterDesc;
	pAdapter->GetDesc1(&AdapterDesc);

	D3D_FEATURE_LEVEL featureLevel[] =
	{
		//D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
		D3D_FEATURE_LEVEL_1_0_CORE,
	};

	UINT numLevelsRequested = 3;
	D3D_FEATURE_LEVEL m_d3dFeatureLevel;

	ComPtr<ID3D11Device> lDirect3DDevice;
	ComPtr<ID3D11DeviceContext> lDirect3DDeviceIMContext;

	SIZE_T MaxVideoMemory = 0;

	UINT FeatureIndex = 0;
	D3D_FEATURE_LEVEL ReqFeatureLevel = featureLevel[FeatureIndex];

	bool bWarp = false;

	if (InGPUIndex.has_value())
	{
		if (InGPUIndex <= -1)
		{
			bWarp = true;
		}
	}

	while (FeatureIndex < 10)
	{
		if (SUCCEEDED(D3D11CreateDevice(
			bWarp ? NULL : pAdapter ? pAdapter : NULL,											// pAdapter
			bWarp ? D3D_DRIVER_TYPE_WARP 
				  : (pAdapter) ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE,			// DriverType
			NULL,																				// Software
			bWarp ? NULL : createFlags,																		// Flags
			&ReqFeatureLevel,																	// pFeatureLevels
			1,																					// FeatureLevels
			D3D11_SDK_VERSION,																	// SDKVersion
			&lDirect3DDevice,																	// ppDevice
			&m_d3dFeatureLevel,																	// pFeatureLevel
			&lDirect3DDeviceIMContext															// ppImContentteContext
		)))
		{
			DXGI_ADAPTER_DESC1 desc;
			pAdapter->GetDesc1(&desc);
			VendorId = desc.VendorId;
			pAdapter->GetDesc1(&desc);
			MaxVideoMemory = desc.DedicatedVideoMemory;
			break;
		}
		else
		{
			FeatureIndex++;
			ReqFeatureLevel = featureLevel[FeatureIndex];
		}
	}

	lDirect3DDevice->QueryInterface(__uuidof (ID3D11Device1), (void**)& Direct3DDevice);
	lDirect3DDeviceIMContext->QueryInterface(__uuidof (ID3D11DeviceContext1), (void**)& Direct3DDeviceIMContext);

	{
		if (GPUIndex.has_value())
		{
			if (GPUIndex <= -1)
			{
				std::cout << "WARP Device" << std::endl;
			}
		}
		std::wcout << AdapterDesc.Description << std::endl;
		std::cout << AdapterDesc.DeviceId << std::endl;
		std::cout << "Vendor Id : " << VendorId << std::endl;
		std::cout << "DedicatedVideoMemory : " << AdapterDesc.DedicatedVideoMemory << std::endl;
		std::cout << "DedicatedSystemMemory : " << AdapterDesc.DedicatedSystemMemory << std::endl;
		std::cout << "SharedSystemMemory : " << AdapterDesc.SharedSystemMemory << std::endl;

		std::string FeatureLevel = m_d3dFeatureLevel == D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_12_2 ? "D3D_FEATURE_LEVEL_12_2" : m_d3dFeatureLevel == D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_12_1 ? "D3D_FEATURE_LEVEL_12_1"
			: m_d3dFeatureLevel == D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_12_0 ? "D3D_FEATURE_LEVEL_12_0" : m_d3dFeatureLevel == D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_1 ? "D3D_FEATURE_LEVEL_11_1"
			: m_d3dFeatureLevel == D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0 ? "D3D_FEATURE_LEVEL_11_0" : m_d3dFeatureLevel == D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_10_1 ? "D3D_FEATURE_LEVEL_10_1"
			: m_d3dFeatureLevel == D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_10_0 ? "D3D_FEATURE_LEVEL_10_0" : m_d3dFeatureLevel == D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_9_3 ? "D3D_FEATURE_LEVEL_9_3"
			: m_d3dFeatureLevel == D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_9_2 ? "D3D_FEATURE_LEVEL_9_2" : m_d3dFeatureLevel == D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_9_1 ? "D3D_FEATURE_LEVEL_9_1"
			: m_d3dFeatureLevel == D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_1_0_CORE ? "D3D_FEATURE_LEVEL_1_0_CORE" : "D3D_FEATURE_LEVEL_Unknown";
		std::cout << FeatureLevel << std::endl;
	};

	D3D11_FEATURE_DATA_D3D11_OPTIONS FeatureData_11 = {};
	Direct3DDevice->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS, &FeatureData_11, sizeof(FeatureData_11));
	D3D11_FEATURE_DATA_D3D11_OPTIONS1 FeatureData_11_1 = {};
	Direct3DDevice->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS1, &FeatureData_11_1, sizeof(FeatureData_11_1));
	D3D11_FEATURE_DATA_D3D11_OPTIONS2 FeatureData_11_2 = {};
	Direct3DDevice->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS2, &FeatureData_11_2, sizeof(FeatureData_11_2));
	D3D11_FEATURE_DATA_D3D11_OPTIONS3 FeatureData_11_3 = {};
	Direct3DDevice->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS3, &FeatureData_11_3, sizeof(FeatureData_11_3));
	//D3D11_FEATURE_DATA_D3D11_OPTIONS4 FeatureData_11_4 = {};
	//Direct3DDevice->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS4, &FeatureData_11_4, sizeof(FeatureData_11_4));
	D3D11_FEATURE_DATA_D3D11_OPTIONS5 FeatureData_11_5 = {};
	Direct3DDevice->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS5, &FeatureData_11_5, sizeof(FeatureData_11_5));

#ifdef _DEBUG
	{
		std::string Name = "Direct3DDevice";
		Direct3DDevice->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(Name) - 1, Name.c_str());
	}
	{
		std::string Name = "Direct3DDeviceIMContext";
		Direct3DDeviceIMContext->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(Name) - 1, Name.c_str());
	}
#endif

	lDirect3DDevice = nullptr;
	lDirect3DDeviceIMContext = nullptr;

	if (pAdapter)
		pAdapter->Release();
}

D3D11Device::~D3D11Device()
{
#if _DEBUG && DEBUG_D3DDEVICE
	Direct3DDeviceIMContext->ClearState();
	Direct3DDeviceIMContext->Flush();

	ID3D11Debug* D3D11Debug = nullptr;
	Direct3DDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)(&D3D11Debug));
#endif

	Direct3DDevice = nullptr;
	Direct3DDeviceIMContext = nullptr;

	Viewport = nullptr;
	DXGIFactory = nullptr;

#if _DEBUG && DEBUG_D3DDEVICE
	if (D3D11Debug)
		D3D11Debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	IDXGIDebug* debugDev = nullptr;
	DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debugDev));
	if (debugDev)
		debugDev->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
#endif
}

D3D11Viewport* D3D11Device::GetViewportContext() const
{
	return Viewport.get();
}

void D3D11Device::InitWindow(void* InHWND, std::uint32_t InWidth, std::uint32_t InHeight, bool bFullscreen)
{
	if (!Viewport)
		Viewport = std::make_unique<D3D11Viewport>(this, DXGIFactory, InWidth, InHeight, bFullscreen, (HWND)InHWND);
}

void D3D11Device::Present(IRenderTarget* pRT)
{
	Viewport->Present(pRT);

	Direct3DDeviceIMContext->DiscardView(Viewport->GetCurrentBackBufferRT());
	Direct3DDeviceIMContext->ClearState();
}

bool D3D11Device::IsFullScreen() const
{
	return Viewport->IsFullScreen();
}

bool D3D11Device::IsVsyncEnabled() const
{
	return Viewport->IsVsyncEnabled();
}

std::uint32_t D3D11Device::GetVsyncInterval() const
{
	return Viewport->GetVsyncInterval();
}

void D3D11Device::ResizeWindow(std::size_t Width, std::size_t Height)
{
	Viewport->ResizeSwapChain(Width, Height);
}

void D3D11Device::FullScreen(const bool value)
{
	Viewport->FullScreen(value);
}

void D3D11Device::Vsync(const bool value)
{
	Viewport->Vsync(value);
}

void D3D11Device::VsyncInterval(const std::uint32_t value)
{
	Viewport->VsyncInterval(value);
}

std::vector<sDisplayMode> D3D11Device::GetAllSupportedResolutions() const
{
	return Viewport->GetAllSupportedResolutions();
}

sScreenDimension D3D11Device::GetBackBufferDimension() const
{
	return Viewport->GetScreenDimension();
}

EFormat D3D11Device::GetBackBufferFormat() const
{
	return Viewport->GetBackBufferFormat();
}

sViewport D3D11Device::GetViewport() const
{
	return Viewport->GetViewport();
}

IDXGIAdapter1* D3D11Device::GetAdapter(std::optional<short> Index)
{
	IDXGIAdapter1* pAdapter;
	std::vector<IDXGIAdapter1*> vAdapters;

	if (Index.has_value())
	{
		if (Index <= -1)
		{
			DXGIFactory->EnumWarpAdapter(IID_PPV_ARGS(&pAdapter));
			return pAdapter;
		}
	}

	for (std::uint32_t i = 0; DXGIFactory->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		DXGI_ADAPTER_DESC1 desc;
		pAdapter->GetDesc1(&desc);
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;
		vAdapters.push_back(pAdapter);
	}

	if (Index != std::nullopt)
	{
		return vAdapters.at(Index.value() >= vAdapters.size() ? vAdapters.size() - 1 : Index.value());
	}

	SIZE_T videoMemory = 0;
	for (auto& Adapter : vAdapters)
	{
		DXGI_ADAPTER_DESC1 desc;
		Adapter->GetDesc1(&desc);
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;

		if (desc.DedicatedVideoMemory > videoMemory)
		{
			videoMemory = desc.DedicatedVideoMemory;

			if (SUCCEEDED(D3D12CreateDevice(Adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
			{
				pAdapter = Adapter;
			}
		}
	}

	// WARP
	//if (!pAdapter)
	//{
	//	if (FAILED(DXGIFactory->EnumWarpAdapter(IID_PPV_ARGS(&pAdapter))))
	//	{
	//		std::cout << "Direct3D Adapter - WARP" << std::endl;
	//	}
	//}

	return pAdapter;
}

bool D3D11Device::GetDeviceIdentification(std::wstring& InVendorID, std::wstring& InDeviceID)
{
	DISPLAY_DEVICE dd;
	dd.cb = sizeof(DISPLAY_DEVICE);
	int i = 0;
	std::wstring id;
	// locate primary display device
	while (EnumDisplayDevices(NULL, i, &dd, 0))
	{
		if (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
		{
			id = dd.DeviceID;
			break;
		}
		i++;
	}
	//if (id == "") return false;
	// get vendor ID
	InVendorID = id.substr(8, 4);
	// get device ID
	InDeviceID = id.substr(17, 4);
	return true;
}

IGraphicsCommandContext::SharedPtr D3D11Device::CreateGraphicsCommandContext()
{
	return D3D11CommandBuffer::Create(this);
}

IGraphicsCommandContext::UniquePtr D3D11Device::CreateUniqueGraphicsCommandContext()
{
	return D3D11CommandBuffer::CreateUnique(this);
}

IComputeCommandContext::SharedPtr D3D11Device::CreateComputeCommandContext()
{
	return D3D11ComputeCommandContext::Create(this);
}

IComputeCommandContext::UniquePtr D3D11Device::CreateUniqueComputeCommandContext()
{
	return D3D11ComputeCommandContext::CreateUnique(this);
}

ICopyCommandContext::SharedPtr D3D11Device::CreateCopyCommandContext()
{
	return D3D11CopyCommandBuffer::Create(this);
}

ICopyCommandContext::UniquePtr D3D11Device::CreateUniqueCopyCommandContext()
{
	return D3D11CopyCommandBuffer::CreateUnique(this);
}

IConstantBuffer::SharedPtr D3D11Device::CreateConstantBuffer(std::string InName, const sBufferDesc& InDesc, std::uint32_t InRootParameterIndex)
{
	return D3D11ConstantBuffer::Create(this, InName, InDesc, InRootParameterIndex);
}

IConstantBuffer::UniquePtr D3D11Device::CreateUniqueConstantBuffer(std::string InName, const sBufferDesc& InDesc, std::uint32_t InRootParameterIndex)
{
	return D3D11ConstantBuffer::CreateUnique(this, InName, InDesc, InRootParameterIndex);
}

IVertexBuffer::SharedPtr D3D11Device::CreateVertexBuffer(std::string InName, const sBufferDesc& InDesc, sBufferSubresource* InSubresource)
{
	return D3D11VertexBuffer::Create(this, InName, InDesc, InSubresource);
}

IVertexBuffer::UniquePtr D3D11Device::CreateUniqueVertexBuffer(std::string InName, const sBufferDesc& InDesc, sBufferSubresource* InSubresource)
{
	return D3D11VertexBuffer::CreateUnique(this, InName, InDesc, InSubresource);
}

IIndexBuffer::SharedPtr D3D11Device::CreateIndexBuffer(std::string InName, const sBufferDesc& InDesc, sBufferSubresource* InSubresource)
{
	return D3D11IndexBuffer::Create(this, InName, InDesc, InSubresource);
}

IIndexBuffer::UniquePtr D3D11Device::CreateUniqueIndexBuffer(std::string InName, const sBufferDesc& InDesc, sBufferSubresource* InSubresource)
{
	return D3D11IndexBuffer::CreateUnique(this, InName, InDesc, InSubresource);
}

IFrameBuffer::SharedPtr D3D11Device::CreateFrameBuffer(const std::string InName, const sFrameBufferAttachmentInfo& InAttachments)
{
	return D3D11FrameBuffer::Create(this, InName, InAttachments);
}

IFrameBuffer::UniquePtr D3D11Device::CreateUniqueFrameBuffer(const std::string InName, const sFrameBufferAttachmentInfo& InAttachments)
{
	return D3D11FrameBuffer::CreateUnique(this, InName, InAttachments);
}

IRenderTarget::SharedPtr D3D11Device::CreateRenderTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc)
{
	return D3D11RenderTarget::Create(this, InName, Format, Desc);
}

IRenderTarget::UniquePtr D3D11Device::CreateUniqueRenderTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc)
{
	return D3D11RenderTarget::CreateUnique(this, InName, Format, Desc);
}

IDepthTarget::SharedPtr D3D11Device::CreateDepthTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc)
{
	return D3D11DepthTarget::Create(this, InName, Format, Desc);
}

IDepthTarget::UniquePtr D3D11Device::CreateUniqueDepthTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc)
{
	return D3D11DepthTarget::CreateUnique(this, InName, Format, Desc);
}

IUnorderedAccessTarget::SharedPtr D3D11Device::CreateUnorderedAccessTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc, bool InEnableSRV)
{
	return D3D11UnorderedAccessTarget::Create(this, InName, Format, Desc, InEnableSRV);
}

IUnorderedAccessTarget::UniquePtr D3D11Device::CreateUniqueUnorderedAccessTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc, bool InEnableSRV)
{
	return D3D11UnorderedAccessTarget::CreateUnique(this, InName, Format, Desc, InEnableSRV);
}

IPipeline::SharedPtr D3D11Device::CreatePipeline(const std::string& InName, const sPipelineDesc& InDesc)
{
	return D3D11Pipeline::Create(this, InName, InDesc);
}

IPipeline::UniquePtr D3D11Device::CreateUniquePipeline(const std::string& InName, const sPipelineDesc& InDesc)
{
	return D3D11Pipeline::CreateUnique(this, InName, InDesc);
}

IComputePipeline::SharedPtr D3D11Device::CreateComputePipeline(const std::string& InName, const sComputePipelineDesc& InDesc)
{
	return D3D11ComputePipeline::CreateUnique(this, InName, InDesc);
}

IComputePipeline::UniquePtr D3D11Device::CreateUniqueComputePipeline(const std::string& InName, const sComputePipelineDesc& InDesc)
{
	return D3D11ComputePipeline::CreateUnique(this, InName, InDesc);
}

ITexture2D::SharedPtr D3D11Device::CreateTexture2D(const std::wstring FilePath, const std::string InName, std::uint32_t DefaultRootParameterIndex)
{
	return D3D11Texture::Create(this, FilePath, InName, DefaultRootParameterIndex);
}

ITexture2D::UniquePtr D3D11Device::CreateUniqueTexture2D(const std::wstring FilePath, const std::string InName, std::uint32_t DefaultRootParameterIndex)
{
	return D3D11Texture::CreateUnique(this, FilePath, InName, DefaultRootParameterIndex);
}

ITexture2D::SharedPtr D3D11Device::CreateTexture2D(const std::string InName, void* InBuffer, const std::size_t InSize, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
{
	return D3D11Texture::Create(this, InName, InBuffer, InSize, InDesc, DefaultRootParameterIndex);
}

ITexture2D::UniquePtr D3D11Device::CreateUniqueTexture2D(const std::string InName, void* InBuffer, const std::size_t InSize, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
{
	return D3D11Texture::CreateUnique(this, InName, InBuffer, InSize, InDesc, DefaultRootParameterIndex);
}

ITexture2D::SharedPtr D3D11Device::CreateEmptyTexture2D(const std::string InName, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
{
	return D3D11Texture::CreateUnique(this, InName, InDesc, DefaultRootParameterIndex);
}

ITexture2D::UniquePtr D3D11Device::CreateUniqueEmptyTexture2D(const std::string InName, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
{
	return D3D11Texture::CreateUnique(this, InName, InDesc, DefaultRootParameterIndex);
}

//ITiledTexture::SharedPtr D3D11Device::CreateTiledTexture(const std::string InName, const std::uint32_t InTileX, const std::uint32_t InTileY, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
//{
//	return D3D11TiledTexture::Create(this, InName, InTileX, InTileY, InDesc, DefaultRootParameterIndex);
//}
//
//ITiledTexture::UniquePtr D3D11Device::CreateUniqueTiledTexture(const std::string InName, const std::uint32_t InTileX, const std::uint32_t InTileY, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
//{
//	return D3D11TiledTexture::CreateUnique(this, InName, InTileX, InTileY, InDesc, DefaultRootParameterIndex);
//}
