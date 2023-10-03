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
#include <iostream>
#include <ostream>
#include "D3D12Device.h"
#include <algorithm>
#include <assert.h>
#include "Engine/AbstractEngine.h"
#include "D3D12Buffer.h"
#include "dxgidebug.h"
#include "D3D12Viewport.h"
#include "D3D12DescriptorHeapManager.h"
#include "D3D12CommandBuffer.h"
#include "D3D12ComputeCommandContext.h"
#include "D3D12Texture.h"
#include "D3D12Pipeline.h"
#include "GI/D3DShared/D3DShared.h"
#include "D3D12Fence.h"

#define DEBUG_D3DDEVICE 1
#define CheckD3DFeature(FeatureLevel, var) Direct3DDevice->CheckFeatureSupport(FeatureLevel, &var, sizeof(var))

D3D12Device::D3D12Device(std::optional<short> InGPUIndex)
	: bTypedUAVLoadSupport_R11G11B10_FLOAT(false)
	, bTypedUAVLoadSupport_R16G16B16A16_FLOAT(false)
	, useGPUBasedValidation(false)
	, GPUIndex(InGPUIndex)
	, VendorId(0)
{
	HRESULT HR = E_FAIL;
	bool useDebugLayers = false;
	DWORD dxgiFactoryFlags = 0;

#if _DEBUG && DEBUG_D3DDEVICE
	useDebugLayers = true;

	if (useDebugLayers)
	{
		Microsoft::WRL::ComPtr<ID3D12Debug> debugInterface;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface))))
		{
			debugInterface->EnableDebugLayer();

			if (useGPUBasedValidation)
			{
				Microsoft::WRL::ComPtr<ID3D12Debug1> debugInterface1;
				if (SUCCEEDED((debugInterface->QueryInterface(IID_PPV_ARGS(&debugInterface1)))))
				{
					debugInterface1->SetEnableGPUBasedValidation(true);
				}
			}
		}
		else
		{
			std::cout << "Unable to enable D3D12 debug validation layer." << std::endl;
		}
		debugInterface = nullptr;
	}
#endif

	Microsoft::WRL::ComPtr<ID3D12Device> pDevice;
	CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(DxgiFactory.GetAddressOf()));

	//D3D12EnableExperimentalFeatures();

	ComPtr<IDXGIAdapter1> adapter = GetAdapter(GPUIndex);

	DXGI_ADAPTER_DESC1 AdapterDesc;
	adapter->GetDesc1(&AdapterDesc);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_12_2,
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

	UINT FeatureIndex = 0;
	D3D_FEATURE_LEVEL ReqFeatureLevel = featureLevels[FeatureIndex];

	HRESULT hr;

	while (FeatureIndex < 5)
	{
		hr = D3D12CreateDevice(
			adapter.Get(),
			ReqFeatureLevel,
			IID_PPV_ARGS(Direct3DDevice.ReleaseAndGetAddressOf())
		);
		if (hr == S_OK)
		{
			break;
		}
		else
		{
			FeatureIndex++;
			ReqFeatureLevel = featureLevels[FeatureIndex];
		}
	}
	ThrowIfFailed(hr);

	VendorId = AdapterDesc.VendorId;
	FeatureLevel = ReqFeatureLevel;
#if _DEBUG
	Direct3DDevice->SetName(L"Direct3DDevice");
#endif

	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.NodeMask = 0;
		ThrowIfFailed(Direct3DDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&GraphicsQueue)));
#if _DEBUG
		GraphicsQueue->SetName(L"GraphicsQueue");
#endif
	}
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
		queueDesc.NodeMask = 0;
		ThrowIfFailed(Direct3DDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&CopyQueue)));
#if _DEBUG
		CopyQueue->SetName(L"CopyQueue");
#endif
	}
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		queueDesc.NodeMask = 0;
		ThrowIfFailed(Direct3DDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&ComputeQueue)));
#if _DEBUG
		ComputeQueue->SetName(L"ComputeQueue");
#endif
	}

	{
		Fences.insert({ D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12Fence::CreateNew(Direct3DDevice.Get()) });
		Fences.insert({ D3D12_COMMAND_LIST_TYPE_COPY, D3D12Fence::CreateNew(Direct3DDevice.Get()) });
		Fences.insert({ D3D12_COMMAND_LIST_TYPE_COMPUTE, D3D12Fence::CreateNew(Direct3DDevice.Get()) });
	}

	HR = Direct3DDevice->GetDeviceRemovedReason();
	assert(HR != E_FAIL);

	D3D12_FEATURE_DATA_D3D12_OPTIONS FeatureData = {};
	if (SUCCEEDED(CheckD3DFeature(D3D12_FEATURE_D3D12_OPTIONS, FeatureData)))
	{
		if (FeatureData.TypedUAVLoadAdditionalFormats)
		{
			D3D12_FEATURE_DATA_FORMAT_SUPPORT Support =
			{
				DXGI_FORMAT_R11G11B10_FLOAT, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE
			};

			if (SUCCEEDED(Direct3DDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &Support, sizeof(Support))) &&
				(Support.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD) != 0)
			{
				bTypedUAVLoadSupport_R11G11B10_FLOAT = true;
			}

			Support.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

			if (SUCCEEDED(Direct3DDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &Support, sizeof(Support))) &&
				(Support.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD) != 0)
			{
				bTypedUAVLoadSupport_R16G16B16A16_FLOAT = true;
			}
		}
	}

	D3D12_FEATURE_DATA_D3D12_OPTIONS12 Feature12;
	CheckD3DFeature(D3D12_FEATURE_D3D12_OPTIONS12, Feature12);
	bEnhancedBarriersSupport = Feature12.EnhancedBarriersSupported;

	D3D12_FEATURE_DATA_D3D12_OPTIONS5 Feature5;
	CheckD3DFeature(D3D12_FEATURE_D3D12_OPTIONS5, Feature5);
	RenderPassTier = Feature5.RenderPassesTier;

	{
		if (GPUIndex.has_value())
		{
			if (GPUIndex <= -1)
			{
				std::cout << "WARP Device" << std::endl;
			}
		}
		std::wcout << AdapterDesc.Description << std::endl;
		std::cout << "DeviceId : " << AdapterDesc.DeviceId << std::endl;
		std::cout << "Vendor Id : " << VendorId << std::endl;
		std::cout << "DedicatedVideoMemory : " << AdapterDesc.DedicatedVideoMemory << std::endl;
		std::cout << "DedicatedSystemMemory : " << AdapterDesc.DedicatedSystemMemory << std::endl;
		std::cout << "SharedSystemMemory : " << AdapterDesc.SharedSystemMemory << std::endl;

		std::string FeatureLevelSTR = ReqFeatureLevel == D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_12_2 ? "D3D_FEATURE_LEVEL_12_2" : ReqFeatureLevel == D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_12_1 ? "D3D_FEATURE_LEVEL_12_1"
			: ReqFeatureLevel == D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_12_0 ? "D3D_FEATURE_LEVEL_12_0" : ReqFeatureLevel == D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_1 ? "D3D_FEATURE_LEVEL_11_1"
			: ReqFeatureLevel == D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0 ? "D3D_FEATURE_LEVEL_11_0" : ReqFeatureLevel == D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_10_1 ? "D3D_FEATURE_LEVEL_10_1"
			: ReqFeatureLevel == D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_10_0 ? "D3D_FEATURE_LEVEL_10_0" : ReqFeatureLevel == D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_9_3 ? "D3D_FEATURE_LEVEL_9_3"
			: ReqFeatureLevel == D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_9_2 ? "D3D_FEATURE_LEVEL_9_2" : ReqFeatureLevel == D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_9_1 ? "D3D_FEATURE_LEVEL_9_1"
			: ReqFeatureLevel == D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_1_0_CORE ? "D3D_FEATURE_LEVEL_1_0_CORE" : "D3D_FEATURE_LEVEL_Unknown";
		std::cout << FeatureLevelSTR << std::endl;
	};

#if _DEBUG && DEBUG_D3DDEVICE
	ComPtr<ID3D12InfoQueue> d3dInfoQueue;
	if (SUCCEEDED(Direct3DDevice.As(&d3dInfoQueue)))
	{
		d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);

		D3D12_MESSAGE_ID hide[] =
		{
			D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
			D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,

			D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_WRONGSWAPCHAINBUFFERREFERENCE,
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE,
		};
		D3D12_INFO_QUEUE_FILTER filter = {};
		filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
		filter.DenyList.pIDList = hide;
		d3dInfoQueue->AddStorageFilterEntries(&filter);
	}
#endif

	PostInit();
}

D3D12Device::~D3D12Device()
{
	GPUFlush();

#if _DEBUG && DEBUG_D3DDEVICE
	ID3D12DebugDevice* DebugDevice = nullptr;
	Direct3DDevice->QueryInterface(__uuidof(ID3D12DebugDevice), (void**)(&DebugDevice));
#endif

	Viewport = nullptr;

	DescriptorHeapManager = nullptr;
	DxgiFactory = nullptr;
	IMCommandList = nullptr;
	IMCopyCommandList = nullptr;

	for (auto Fence : Fences)
	{
		delete Fence.second;
		Fence.second = nullptr;
	}
	Fences.clear();

	CMDAllocatorPool.clear();

	GraphicsQueue = nullptr;
	ComputeQueue = nullptr;
	CopyQueue = nullptr;

#if _DEBUG && DEBUG_D3DDEVICE
	if (DebugDevice)
		DebugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
	IDXGIDebug* debugDev = nullptr;
	DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debugDev));
	if (debugDev)
		debugDev->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
#endif
	//Direct3DDevice->Release();
	Direct3DDevice = nullptr;
}

void D3D12Device::PostInit()
{
	IMCommandList = std::make_unique<D3D12CommandBuffer>(this);
#if _DEBUG
	IMCommandList->Get()->SetName(L"IMCommandList");
#endif

	IMCopyCommandList = D3D12CopyCommandBuffer::CreateUnique(this);
#if _DEBUG
	IMCopyCommandList->Get()->SetName(L"IMCopyCommandList");
#endif

	DescriptorHeapManager = std::make_unique<D3D12DescriptorHeapManager>(this);
}

D3D12Viewport* D3D12Device::GetViewportContext() const
{
	return Viewport.get();
}

void D3D12Device::InitWindow(void* InHWND, std::uint32_t InWidth, std::uint32_t InHeight, bool bFullscreen)
{
	if (!Viewport)
		Viewport = std::make_unique<D3D12Viewport>(this, DxgiFactory, InWidth, InHeight, bFullscreen, (HWND)InHWND);
}

void D3D12Device::Present(IRenderTarget* pRT)
{
	Viewport->Present(pRT);

	// If we are doing GPU Validation, flush every frame
	if (useGPUBasedValidation)
		GPUFlush();
}

void D3D12Device::ResizeWindow(std::size_t Width, std::size_t Height)
{
	GPUFlush();
	Viewport->ResizeSwapChain(Width, Height);
}

void D3D12Device::FullScreen(const bool value)
{
	GPUFlush();
	Viewport->FullScreen(value);
}

void D3D12Device::Vsync(const bool value)
{
	Viewport->Vsync(value);
}

void D3D12Device::VsyncInterval(const std::uint32_t value)
{
	Viewport->VsyncInterval(value);
}

bool D3D12Device::IsFullScreen() const
{
	return Viewport->IsFullScreen();
}

bool D3D12Device::IsVsyncEnabled() const
{
	return Viewport->IsVsyncEnabled();
}

std::uint32_t D3D12Device::GetVsyncInterval() const
{
	return Viewport->GetVsyncInterval();
}

std::vector<sDisplayMode> D3D12Device::GetAllSupportedResolutions() const
{
	return Viewport->GetAllSupportedResolutions();
}

sScreenDimension D3D12Device::GetBackBufferDimension() const
{
	return Viewport->GetScreenDimension();
}

EFormat D3D12Device::GetBackBufferFormat() const
{
	return Viewport->GetBackBufferFormat();
}

sViewport D3D12Device::GetViewport() const
{
	return Viewport->GetViewport();
}

void D3D12Device::ExecuteDirectCommandLists(ID3D12CommandList* pCommandList, bool WaitForCompletion)
{
	GraphicsQueue->ExecuteCommandLists(1, &pCommandList);
	GPUSignal(D3D12_COMMAND_LIST_TYPE_DIRECT);
	if (WaitForCompletion)
		CPUWait(D3D12_COMMAND_LIST_TYPE_DIRECT);
}

void D3D12Device::ExecuteCopyCommandLists(ID3D12CommandList* pCommandList, bool WaitForCompletion)
{
	CopyQueue->ExecuteCommandLists(1, &pCommandList);
	GPUSignal(D3D12_COMMAND_LIST_TYPE_COPY);
	if (WaitForCompletion)
		CPUWait(D3D12_COMMAND_LIST_TYPE_COPY);
}

void D3D12Device::ExecuteComputeCommandLists(ID3D12CommandList* pCommandList, bool WaitForCompletion)
{
	ComputeQueue->ExecuteCommandLists(1, &pCommandList);
	GPUSignal(D3D12_COMMAND_LIST_TYPE_COMPUTE);
	if (WaitForCompletion)
		CPUWait(D3D12_COMMAND_LIST_TYPE_COMPUTE);
}

void D3D12Device::GPUSignal(D3D12_COMMAND_LIST_TYPE Type)
{
	switch (Type)
	{
	case D3D12_COMMAND_LIST_TYPE_BUNDLE:
	case D3D12_COMMAND_LIST_TYPE_DIRECT:
		Fences.at(D3D12_COMMAND_LIST_TYPE_DIRECT)->Signal(GraphicsQueue.Get());
		break;
	case D3D12_COMMAND_LIST_TYPE_COMPUTE:
		Fences.at(D3D12_COMMAND_LIST_TYPE_COMPUTE)->Signal(ComputeQueue.Get());
		break;
	case D3D12_COMMAND_LIST_TYPE_COPY:
		Fences.at(D3D12_COMMAND_LIST_TYPE_COPY)->Signal(CopyQueue.Get());
		break;
	case D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE:
		break;
	case D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS:
		break;
	case D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE:
		break;
	case D3D12_COMMAND_LIST_TYPE_NONE:
		break;
	default:
		break;
	}
}

void D3D12Device::CPUWait(D3D12_COMMAND_LIST_TYPE Type)
{
	if (Type == D3D12_COMMAND_LIST_TYPE_BUNDLE)
		Fences.at(D3D12_COMMAND_LIST_TYPE_DIRECT)->CpuWait();
	else
		Fences.at(Type)->CpuWait();
}

ID3D12CommandAllocator* D3D12Device::RequestCommandAllocator(D3D12_COMMAND_LIST_TYPE cmdType, std::optional<std::uint64_t> CompletedFenceValue)
{
	std::lock_guard<std::mutex> LockGuard(CMDAllocatorPool[cmdType == D3D12_COMMAND_LIST_TYPE_BUNDLE ? D3D12_COMMAND_LIST_TYPE_DIRECT : cmdType].m_AllocatorMutex);

	ID3D12CommandAllocator* pAllocator = nullptr;

	const D3D12_COMMAND_LIST_TYPE Type = cmdType == D3D12_COMMAND_LIST_TYPE_BUNDLE ? D3D12_COMMAND_LIST_TYPE_DIRECT : cmdType;

	if (!CMDAllocatorPool[Type].m_ReadyAllocators.empty())
	{
		std::pair<uint64_t, ID3D12CommandAllocator*>& AllocatorPair = CMDAllocatorPool[Type].m_ReadyAllocators.front();

		if (CompletedFenceValue.has_value() ? AllocatorPair.first <= CompletedFenceValue : AllocatorPair.first <= Fences.at(Type)->GetCompletedValue())
		{
			pAllocator = AllocatorPair.second;
			pAllocator->Reset();
			CMDAllocatorPool[Type].m_ReadyAllocators.pop();
		}
	}

	// If no allocator's were ready to be reused, create a new one
	if (pAllocator == nullptr)
	{
		Direct3DDevice->CreateCommandAllocator(Type, IID_PPV_ARGS(&pAllocator));
		wchar_t AllocatorName[32];
		swprintf(AllocatorName, 32, L"CommandAllocator %zu", CMDAllocatorPool[Type].m_AllocatorPool.size());
		pAllocator->SetName(AllocatorName);
		CMDAllocatorPool[Type].m_AllocatorPool.push_back(pAllocator);
	}

	return pAllocator;
}

void D3D12Device::DiscardCommandAllocator(D3D12_COMMAND_LIST_TYPE cmdType, ID3D12CommandAllocator* Allocator, std::optional<std::uint64_t> FenceValue)
{
	std::lock_guard<std::mutex> LockGuard(CMDAllocatorPool[cmdType == D3D12_COMMAND_LIST_TYPE_BUNDLE ? D3D12_COMMAND_LIST_TYPE_DIRECT : cmdType].m_AllocatorMutex);

	const D3D12_COMMAND_LIST_TYPE Type = cmdType == D3D12_COMMAND_LIST_TYPE_BUNDLE ? D3D12_COMMAND_LIST_TYPE_DIRECT : cmdType;
	// That fence value indicates we are free to reset the allocator
	if (FenceValue.has_value())
		CMDAllocatorPool[Type].m_ReadyAllocators.push(std::make_pair(*FenceValue, Allocator));
	else
		CMDAllocatorPool[Type].m_ReadyAllocators.push(std::make_pair(Fences.at(Type)->GetFenceCounter(), Allocator));
}

void D3D12Device::GPUFlush(D3D12_COMMAND_LIST_TYPE queueType)
{
	ID3D12Fence* pFence;
	ThrowIfFailed(Direct3DDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence)));

	ID3D12CommandQueue* queue = nullptr;
	switch (queueType)
	{
	case D3D12_COMMAND_LIST_TYPE_BUNDLE:
	case D3D12_COMMAND_LIST_TYPE_DIRECT:
		queue = GetGraphicsQueue();
		break;
	case D3D12_COMMAND_LIST_TYPE_COMPUTE:
		queue = GetComputeQueue();
		break;
	case D3D12_COMMAND_LIST_TYPE_COPY:
		queue = GetCopyQueue();
		break;
	case D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE:
		break;
	case D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS:
		break;
	case D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE:
		break;
	case D3D12_COMMAND_LIST_TYPE_NONE:
		break;
	default:
		break;
	}

	if (!queue)
		return;

	ThrowIfFailed(queue->Signal(pFence, 1));

	HANDLE mHandleFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	pFence->SetEventOnCompletion(1, mHandleFenceEvent);
	WaitForSingleObject(mHandleFenceEvent, INFINITE);

	CloseHandle(mHandleFenceEvent);
	pFence->Release();
	//delete pFence;
}

void D3D12Device::GPUFlush()
{
	GPUFlush(D3D12_COMMAND_LIST_TYPE_COMPUTE);
	GPUFlush(D3D12_COMMAND_LIST_TYPE_DIRECT);
	GPUFlush(D3D12_COMMAND_LIST_TYPE_COPY);
}

void D3D12Device::SetHeaps(ID3D12GraphicsCommandList* cmd)
{
	DescriptorHeapManager->SetHeaps(cmd);
}

void D3D12Device::AllocateDescriptor(D3D12DescriptorHandle* DescriptorHandle)
{
	DescriptorHeapManager->AllocateDescriptor(DescriptorHandle);
}

void D3D12Device::DeallocateDescriptor(D3D12DescriptorHandle* DescriptorHandle)
{
	DescriptorHeapManager->DeallocateDescriptor(DescriptorHandle);
}

IDXGIAdapter* D3D12Device::FindAdapter(const WCHAR* InTargetName) const
{
	IDXGIAdapter* targetAdapter = NULL;
	IDXGIFactory* IDXGIFactory_0001 = NULL;
	HRESULT hres = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&IDXGIFactory_0001);
	if (hres != S_OK)
	{
		return targetAdapter;
	}

	unsigned int adapterNo = 0;
	while (SUCCEEDED(hres))
	{
		IDXGIAdapter* pAdapter = NULL;
		hres = IDXGIFactory_0001->EnumAdapters(adapterNo, (IDXGIAdapter**)&pAdapter);

		if (SUCCEEDED(hres))
		{
			DXGI_ADAPTER_DESC aDesc;
			pAdapter->GetDesc(&aDesc);

			// If no name is specified, return the first adapater.  This is the same behaviour as the 
			// default specified for D3D11CreateDevice when no adapter is specified.
			if (wcslen(InTargetName) == 0)
			{
				targetAdapter = pAdapter;
				break;
			}

			std::wstring aName = aDesc.Description;
			if (aName.find(InTargetName) != std::string::npos)
			{
				targetAdapter = pAdapter;
			}
			else
			{
				pAdapter->Release();
			}
		}

		adapterNo++;
	}

	if (IDXGIFactory_0001)
		IDXGIFactory_0001->Release();

	return targetAdapter;
}

IDXGIAdapter1* D3D12Device::GetAdapter(std::optional<short> Index) const
{
	IDXGIAdapter1* pAdapter;
	std::vector<IDXGIAdapter1*> vAdapters;

	if (Index.has_value())
	{
		if (Index <= -1)
		{
			DxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pAdapter));
			return pAdapter;
		}
	}

	for (std::uint32_t i = 0; DxgiFactory->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
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

	return pAdapter;
}

bool D3D12Device::GetDeviceIdentification(std::wstring& InVendorID, std::wstring& InDeviceID) const
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

IGraphicsCommandContext::SharedPtr D3D12Device::CreateGraphicsCommandContext() const
{
	return D3D12CommandBuffer::Create(const_cast<D3D12Device*>(this));
}

IGraphicsCommandContext::UniquePtr D3D12Device::CreateUniqueGraphicsCommandContext() const
{
	return D3D12CommandBuffer::CreateUnique(const_cast<D3D12Device*>(this));
}

IComputeCommandContext::SharedPtr D3D12Device::CreateComputeCommandContext() const
{
	return D3D12ComputeCommandContext::Create(const_cast<D3D12Device*>(this));
}

IComputeCommandContext::UniquePtr D3D12Device::CreateUniqueComputeCommandContext() const
{
	return D3D12ComputeCommandContext::CreateUnique(const_cast<D3D12Device*>(this));
}

ICopyCommandContext::SharedPtr D3D12Device::CreateCopyCommandContext() const
{
	return D3D12CopyCommandBuffer::Create(const_cast<D3D12Device*>(this));
}

ICopyCommandContext::UniquePtr D3D12Device::CreateUniqueCopyCommandContext() const
{
	return D3D12CopyCommandBuffer::CreateUnique(const_cast<D3D12Device*>(this));
}

IConstantBuffer::SharedPtr D3D12Device::CreateConstantBuffer(std::string InName, const sBufferDesc& InDesc, std::uint32_t InRootParameterIndex) const
{
	return D3D12ConstantBuffer::Create(const_cast<D3D12Device*>(this), InName, InDesc, InRootParameterIndex);
}

IConstantBuffer::UniquePtr D3D12Device::CreateUniqueConstantBuffer(std::string InName, const sBufferDesc& InDesc, std::uint32_t InRootParameterIndex) const
{
	return D3D12ConstantBuffer::CreateUnique(const_cast<D3D12Device*>(this), InName, InDesc, InRootParameterIndex);
}

IVertexBuffer::SharedPtr D3D12Device::CreateVertexBuffer(std::string InName, const sBufferDesc& InDesc, sBufferSubresource* InSubresource) const
{
	return D3D12VertexBuffer::Create(const_cast<D3D12Device*>(this), InName, InDesc, InSubresource);
}

IVertexBuffer::UniquePtr D3D12Device::CreateUniqueVertexBuffer(std::string InName, const sBufferDesc& InDesc, sBufferSubresource* InSubresource) const
{
	return D3D12VertexBuffer::CreateUnique(const_cast<D3D12Device*>(this), InName, InDesc, InSubresource);
}

IIndexBuffer::SharedPtr D3D12Device::CreateIndexBuffer(std::string InName, const sBufferDesc& InDesc, sBufferSubresource* InSubresource) const
{
	return D3D12IndexBuffer::Create(const_cast<D3D12Device*>(this), InName, InDesc, InSubresource);
}

IIndexBuffer::UniquePtr D3D12Device::CreateUniqueIndexBuffer(std::string InName, const sBufferDesc& InDesc, sBufferSubresource* InSubresource) const
{
	return D3D12IndexBuffer::CreateUnique(const_cast<D3D12Device*>(this), InName, InDesc, InSubresource);
}

IFrameBuffer::SharedPtr D3D12Device::CreateFrameBuffer(const std::string InName, const sFrameBufferAttachmentInfo& InAttachments) const
{
	return D3D12FrameBuffer::Create(const_cast<D3D12Device*>(this), InName, InAttachments);
}

IFrameBuffer::UniquePtr D3D12Device::CreateUniqueFrameBuffer(const std::string InName, const sFrameBufferAttachmentInfo& InAttachments) const
{
	return D3D12FrameBuffer::CreateUnique(const_cast<D3D12Device*>(this), InName, InAttachments);
}

IRenderTarget::SharedPtr D3D12Device::CreateRenderTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc)
{
	return D3D12RenderTarget::Create(const_cast<D3D12Device*>(this), InName, Format, Desc);
}

IRenderTarget::UniquePtr D3D12Device::CreateUniqueRenderTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc)
{
	return D3D12RenderTarget::CreateUnique(const_cast<D3D12Device*>(this), InName, Format, Desc);
}

IDepthTarget::SharedPtr D3D12Device::CreateDepthTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc)
{
	return D3D12DepthTarget::Create(const_cast<D3D12Device*>(this), InName, Format, Desc);
}

IDepthTarget::UniquePtr D3D12Device::CreateUniqueDepthTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc)
{
	return D3D12DepthTarget::CreateUnique(const_cast<D3D12Device*>(this), InName, Format, Desc);
}

IUnorderedAccessTarget::SharedPtr D3D12Device::CreateUnorderedAccessTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc, bool InEnableSRV)
{
	return D3D12UnorderedAccessTarget::Create(const_cast<D3D12Device*>(this), InName, Format, Desc, InEnableSRV);
}

IUnorderedAccessTarget::UniquePtr D3D12Device::CreateUniqueUnorderedAccessTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc, bool InEnableSRV)
{
	return D3D12UnorderedAccessTarget::CreateUnique(const_cast<D3D12Device*>(this), InName, Format, Desc, InEnableSRV);
}

IPipeline::SharedPtr D3D12Device::CreatePipeline(const std::string& InName, const sPipelineDesc& InDesc) const
{
	return D3D12Pipeline::Create(const_cast<D3D12Device*>(this), InName, InDesc);
}

IPipeline::UniquePtr D3D12Device::CreateUniquePipeline(const std::string& InName, const sPipelineDesc& InDesc) const
{
	return D3D12Pipeline::CreateUnique(const_cast<D3D12Device*>(this), InName, InDesc);
}

IComputePipeline::SharedPtr D3D12Device::CreateComputePipeline(const std::string& InName, const sComputePipelineDesc& InDesc) const
{
	return D3D12ComputePipeline::Create(const_cast<D3D12Device*>(this), InName, InDesc);
}

IComputePipeline::UniquePtr D3D12Device::CreateUniqueComputePipeline(const std::string& InName, const sComputePipelineDesc& InDesc) const
{
	return D3D12ComputePipeline::CreateUnique(const_cast<D3D12Device*>(this), InName, InDesc);
}

ITexture2D::SharedPtr D3D12Device::CreateTexture2D(const std::wstring FilePath, const std::string InName, std::uint32_t DefaultRootParameterIndex) const
{
	return D3D12Texture::Create(const_cast<D3D12Device*>(this), FilePath, InName, DefaultRootParameterIndex);
}

ITexture2D::UniquePtr D3D12Device::CreateUniqueTexture2D(const std::wstring FilePath, const std::string InName, std::uint32_t DefaultRootParameterIndex) const
{
	return D3D12Texture::CreateUnique(const_cast<D3D12Device*>(this), FilePath, InName, DefaultRootParameterIndex);
}

ITexture2D::SharedPtr D3D12Device::CreateTexture2D(const std::string InName, void* InBuffer, const std::size_t InSize, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex) const
{
	return D3D12Texture::Create(const_cast<D3D12Device*>(this), InName, InBuffer, InSize, InDesc, DefaultRootParameterIndex);
}

ITexture2D::UniquePtr D3D12Device::CreateUniqueTexture2D(const std::string InName, void* InBuffer, const std::size_t InSize, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex) const
{
	return D3D12Texture::CreateUnique(const_cast<D3D12Device*>(this), InName, InBuffer, InSize, InDesc, DefaultRootParameterIndex);
}

ITexture2D::SharedPtr D3D12Device::CreateEmptyTexture2D(const std::string InName, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex) const
{
	return D3D12Texture::Create(const_cast<D3D12Device*>(this), InName, InDesc, DefaultRootParameterIndex);
}

ITexture2D::UniquePtr D3D12Device::CreateUniqueEmptyTexture2D(const std::string InName, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex) const
{
	return D3D12Texture::CreateUnique(const_cast<D3D12Device*>(this), InName, InDesc, DefaultRootParameterIndex);
}

//ITiledTexture::SharedPtr D3D12Device::CreateTiledTexture(const std::string InName, const std::uint32_t InTileX, const std::uint32_t InTileY, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex) const
//{
//	DebugBreak();
//	return nullptr;
//	//return D3D11TiledTexture::Create(const_cast<D3D12Device*>(this), InName, InTileX, InTileY, InDesc, DefaultRootParameterIndex);
//}
//
//ITiledTexture::UniquePtr D3D12Device::CreateUniqueTiledTexture(const std::string InName, const std::uint32_t InTileX, const std::uint32_t InTileY, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex) const
//{
//	DebugBreak();
//	return nullptr;
//	//return D3D11TiledTexture::CreateUnique(const_cast<D3D12Device*>(this), InName, InTileX, InTileY, InDesc, DefaultRootParameterIndex);
//}
