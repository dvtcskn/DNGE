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
#include "D3D12Device.h"
#include <iostream>
#include <ostream>
#include <algorithm>
#include <deque>
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
#include "D3D12ShaderStates.h"
//#include "D3D12ShaderCompiler.h"
#include "GI/Shared/ShaderCompiler.h"

#include "dx12.h"

#if Pix3_Enabled && _DEBUG
#include <pix3.h>
#endif

//#pragma comment(lib, "WinPixEventRuntime.lib")

#define DEBUG_D3DDEVICE 1

/*class D3D12Device::D3D12CommandAllocatorPool
{
public:
	explicit D3D12CommandAllocatorPool(ID3D12Device* device)
		: Device(device)
	{}

	ID3D12CommandAllocator* RequestAllocator(
		D3D12_COMMAND_LIST_TYPE type,
		UINT64 completedFenceValue)
	{
		std::lock_guard<std::mutex> lock(Mutex);
		auto& pool = Pools[type];

		for (auto it = pool.Ready.begin(); it != pool.Ready.end(); ++it)
		{
			if (it->FenceValue <= completedFenceValue)
			{
				auto* allocator = it->Allocator;
				pool.Ready.erase(it);
				ThrowIfFailed(allocator->Reset());
				return allocator;
			}
		}

		ComPtr<ID3D12CommandAllocator> allocator;
		ThrowIfFailed(Device->CreateCommandAllocator(
			type, IID_PPV_ARGS(&allocator)));

		auto* result = allocator.Get();
		pool.All.push_back(std::move(allocator));
		return result;
	}

	void RetireAllocator(
		D3D12_COMMAND_LIST_TYPE type,
		ID3D12CommandAllocator* allocator,
		UINT64 fenceValue)
	{
		if (!allocator || fenceValue == 0)
			throw std::invalid_argument("Invalid allocator retirement");

		std::lock_guard<std::mutex> lock(Mutex);
		Pools[type].Ready.push_back({ fenceValue, allocator });
	}

private:
	struct RetiredAllocator
	{
		UINT64 FenceValue;
		ID3D12CommandAllocator* Allocator;
	};

	struct TypePool
	{
		std::vector<ComPtr<ID3D12CommandAllocator>> All;
		std::deque<RetiredAllocator> Ready;
	};

	ComPtr<ID3D12Device> Device;
	std::map<D3D12_COMMAND_LIST_TYPE, TypePool> Pools;
	std::mutex Mutex;
};*/

class D3D12Device::D3D12CommandAllocatorPool
{
	sBaseClassBody(sClassConstructor, D3D12CommandAllocatorPool);
public:
	explicit D3D12CommandAllocatorPool(ID3D12Device* InDevice, D3D12_COMMAND_LIST_TYPE InType)
		: Device(InDevice)
		, Type(InType)
		, Size(0)
	{}

	ComPtr<ID3D12CommandAllocator> RequestAllocator(std::uint64_t CompletedFenceValue)
	{
		std::lock_guard<std::mutex> Lock(Mutex);

		if (!ReadyAllocators.empty())
		{
			auto& Front = ReadyAllocators.front();
			if (Front.first <= CompletedFenceValue)
			{
				ComPtr<ID3D12CommandAllocator> Allocator = Front.second;
				ReadyAllocators.pop();
				ThrowIfFailed(Allocator->Reset());
				return Allocator;
			}
		}

		ComPtr<ID3D12CommandAllocator> Allocator;
		ThrowIfFailed(Device->CreateCommandAllocator(Type, IID_PPV_ARGS(&Allocator)));
		++Size;
		return Allocator;
	}

	void DiscardAllocator(std::uint64_t FenceValue, ComPtr<ID3D12CommandAllocator> Allocator)
	{
		std::lock_guard<std::mutex> Lock(Mutex);
		ReadyAllocators.push({ FenceValue, std::move(Allocator) });
	}

	std::size_t GetSize() const
	{
		return Size;
	}

private:
	D3D12_COMMAND_LIST_TYPE Type;
	ID3D12Device* Device = nullptr;
	std::queue<std::pair<std::uint64_t, ComPtr<ID3D12CommandAllocator>>> ReadyAllocators;
	std::mutex Mutex;
	std::size_t Size = 0;
};

class D3D12Device::D3D12CommandAllocatorManager
{
	sBaseClassBody(sClassConstructor, D3D12CommandAllocatorManager);
public:
	D3D12CommandAllocatorManager(ID3D12Device* InDevice)
		: DirectPool(InDevice, D3D12_COMMAND_LIST_TYPE_DIRECT)
		, ComputePool(InDevice, D3D12_COMMAND_LIST_TYPE_COMPUTE)
		, CopyPool(InDevice, D3D12_COMMAND_LIST_TYPE_COPY) 
	{}

	ComPtr<ID3D12CommandAllocator> RequestAllocator(D3D12_COMMAND_LIST_TYPE Type, std::uint64_t CompletedFenceValue)
	{
		switch (Type)
		{
		case D3D12_COMMAND_LIST_TYPE_DIRECT:  
			return DirectPool.RequestAllocator(CompletedFenceValue);
		case D3D12_COMMAND_LIST_TYPE_COMPUTE: 
			return ComputePool.RequestAllocator(CompletedFenceValue);
		case D3D12_COMMAND_LIST_TYPE_COPY:   
			return CopyPool.RequestAllocator(CompletedFenceValue);
		}
		return nullptr;
	}

	void DiscardAllocator(D3D12_COMMAND_LIST_TYPE Type, std::uint64_t FenceValue, ComPtr<ID3D12CommandAllocator> Allocator)
	{
		switch (Type)
		{
		case D3D12_COMMAND_LIST_TYPE_DIRECT:  
			DirectPool.DiscardAllocator(FenceValue, std::move(Allocator));
			break;
		case D3D12_COMMAND_LIST_TYPE_COMPUTE: 
			ComputePool.DiscardAllocator(FenceValue, std::move(Allocator));
			break;
		case D3D12_COMMAND_LIST_TYPE_COPY:   
			CopyPool.DiscardAllocator(FenceValue, std::move(Allocator));
			break;
		}
	}

private:
	D3D12CommandAllocatorPool DirectPool;
	D3D12CommandAllocatorPool ComputePool;
	D3D12CommandAllocatorPool CopyPool;
};

#define CheckD3DFeature(FeatureLevel, var) Direct3DDevice->CheckFeatureSupport(FeatureLevel, &var, sizeof(var))

D3D12Device::D3D12Device(const GPUCreateInfo& DeviceCreateInfo, std::uint32_t InDeviceIndex)
	: bTypedUAVLoadSupport_R11G11B10_FLOAT(false)
	, bTypedUAVLoadSupport_R16G16B16A16_FLOAT(false)
	, useGPUBasedValidation(false)
	, GPUIndex(DeviceCreateInfo.PrimaryGPU.GPUIndex)
	, DeviceType(DeviceCreateInfo.PrimaryGPU.DeviceType)
	, VendorId(0)
	, DeviceIndex(InDeviceIndex)
	, bIsEnhancedBarriersSupported(false)
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

	ComPtr<IDXGIAdapter1> adapter = GetAdapter(DeviceType == EGPUDeviceType::Software ? -1 : GPUIndex);

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
	Direct3DDevice->SetName(AdapterDesc.Description);
#endif

	Direct3DDevice->QueryInterface(IID_PPV_ARGS(&Direct3DDevice14));

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
		ShaderCompiler = DXCShaderCompiler::CreateUnique();
		//pD3D12ShaderCompiler = D3D12ShaderCompiler::CreateUnique();
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

	D3D12_FEATURE_DATA_D3D12_OPTIONS Feature = {};
	CheckD3DFeature(D3D12_FEATURE_D3D12_OPTIONS, Feature);
	D3D12_FEATURE_DATA_D3D12_OPTIONS1 Feature1 = {};
	CheckD3DFeature(D3D12_FEATURE_D3D12_OPTIONS1, Feature1);
	D3D12_FEATURE_DATA_D3D12_OPTIONS2 Feature2 = {};
	CheckD3DFeature(D3D12_FEATURE_D3D12_OPTIONS2, Feature2);
	D3D12_FEATURE_DATA_D3D12_OPTIONS3 Feature3 = {};
	CheckD3DFeature(D3D12_FEATURE_D3D12_OPTIONS3, Feature3);
	D3D12_FEATURE_DATA_D3D12_OPTIONS4 Feature4 = {};
	CheckD3DFeature(D3D12_FEATURE_D3D12_OPTIONS4, Feature4);
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 Feature5 = {};
	CheckD3DFeature(D3D12_FEATURE_D3D12_OPTIONS5, Feature5);
	D3D12_FEATURE_DATA_D3D12_OPTIONS6 Feature6 = {};
	CheckD3DFeature(D3D12_FEATURE_D3D12_OPTIONS6, Feature6);
	D3D12_FEATURE_DATA_D3D12_OPTIONS7 Feature7 = {};
	CheckD3DFeature(D3D12_FEATURE_D3D12_OPTIONS7, Feature7);
	D3D12_FEATURE_DATA_D3D12_OPTIONS8 Feature8 = {};
	CheckD3DFeature(D3D12_FEATURE_D3D12_OPTIONS8, Feature8);
	D3D12_FEATURE_DATA_D3D12_OPTIONS9 Feature9 = {};
	CheckD3DFeature(D3D12_FEATURE_D3D12_OPTIONS9, Feature9);
	D3D12_FEATURE_DATA_D3D12_OPTIONS10 Feature10 = {};
	CheckD3DFeature(D3D12_FEATURE_D3D12_OPTIONS10, Feature10);
	D3D12_FEATURE_DATA_D3D12_OPTIONS11 Feature11 = {};
	CheckD3DFeature(D3D12_FEATURE_D3D12_OPTIONS11, Feature11);
	D3D12_FEATURE_DATA_D3D12_OPTIONS12 Feature12 = {};
	CheckD3DFeature(D3D12_FEATURE_D3D12_OPTIONS12, Feature12);
	D3D12_FEATURE_DATA_D3D12_OPTIONS13 Feature13 = {};
	CheckD3DFeature(D3D12_FEATURE_D3D12_OPTIONS13, Feature13);
	D3D12_FEATURE_DATA_D3D12_OPTIONS14 Feature14 = {};
	CheckD3DFeature(D3D12_FEATURE_D3D12_OPTIONS14, Feature14);
	D3D12_FEATURE_DATA_D3D12_OPTIONS15 Feature15 = {};
	CheckD3DFeature(D3D12_FEATURE_D3D12_OPTIONS15, Feature15);
	D3D12_FEATURE_DATA_D3D12_OPTIONS16 Feature16 = {};
	CheckD3DFeature(D3D12_FEATURE_D3D12_OPTIONS16, Feature16);
	D3D12_FEATURE_DATA_D3D12_OPTIONS17 Feature17 = {};
	CheckD3DFeature(D3D12_FEATURE_D3D12_OPTIONS17, Feature17);
	D3D12_FEATURE_DATA_D3D12_OPTIONS18 Feature18 = {};
	CheckD3DFeature(D3D12_FEATURE_D3D12_OPTIONS18, Feature18);
	D3D12_FEATURE_DATA_D3D12_OPTIONS19 Feature19 = {};
	CheckD3DFeature(D3D12_FEATURE_D3D12_OPTIONS19, Feature19);
	D3D12_FEATURE_DATA_D3D12_OPTIONS20 Feature20 = {};
	CheckD3DFeature(D3D12_FEATURE_D3D12_OPTIONS20, Feature20);
	D3D12_FEATURE_DATA_D3D12_OPTIONS20 Feature21 = {};
	CheckD3DFeature(D3D12_FEATURE_D3D12_OPTIONS21, Feature21);

	D3D12_FEATURE_DATA_FEATURE_LEVELS Feature_FEATURE_LEVELS = {};
	CheckD3DFeature(D3D12_FEATURE_FEATURE_LEVELS, Feature_FEATURE_LEVELS);
	D3D12_FEATURE_DATA_PREDICATION Feature_PREDICATION = {};
	CheckD3DFeature(D3D12_FEATURE_PREDICATION, Feature_PREDICATION);
	D3D12_FEATURE_DATA_PLACED_RESOURCE_SUPPORT_INFO Feature_PLACED_RESOURCE_SUPPORT_INFO = {};
	CheckD3DFeature(D3D12_FEATURE_PLACED_RESOURCE_SUPPORT_INFO, Feature_PLACED_RESOURCE_SUPPORT_INFO);
	D3D12_FEATURE_DATA_HARDWARE_COPY Feature_HARDWARE_COPY = {};
	CheckD3DFeature(D3D12_FEATURE_HARDWARE_COPY, Feature_HARDWARE_COPY);
	D3D12_FEATURE_DATA_PROTECTED_RESOURCE_SESSION_TYPES Feature_PROTECTED_RESOURCE_SESSION_TYPES = {};
	CheckD3DFeature(D3D12_FEATURE_PROTECTED_RESOURCE_SESSION_TYPES, Feature_PROTECTED_RESOURCE_SESSION_TYPES);
	D3D12_FEATURE_DATA_PROTECTED_RESOURCE_SESSION_TYPE_COUNT Feature_PROTECTED_RESOURCE_SESSION_TYPE_COUNT = {};
	CheckD3DFeature(D3D12_FEATURE_PROTECTED_RESOURCE_SESSION_TYPE_COUNT, Feature_PROTECTED_RESOURCE_SESSION_TYPE_COUNT);
	D3D12_FEATURE_DATA_QUERY_META_COMMAND Feature_QUERY_META_COMMAND = {};
	CheckD3DFeature(D3D12_FEATURE_QUERY_META_COMMAND, Feature_QUERY_META_COMMAND);
	D3D12_FEATURE_DATA_DISPLAYABLE Feature_DISPLAYABLE = {};
	CheckD3DFeature(D3D12_FEATURE_DISPLAYABLE, Feature_DISPLAYABLE);
	D3D12_FEATURE_DATA_CROSS_NODE Feature_CROSS_NODE = {};
	CheckD3DFeature(D3D12_FEATURE_CROSS_NODE, Feature_CROSS_NODE);
	D3D12_FEATURE_DATA_SERIALIZATION Feature_SERIALIZATION = {};
	CheckD3DFeature(D3D12_FEATURE_SERIALIZATION, Feature_SERIALIZATION);
	D3D12_FEATURE_DATA_EXISTING_HEAPS Feature_EXISTING_HEAPS = {};
	CheckD3DFeature(D3D12_FEATURE_EXISTING_HEAPS, Feature_EXISTING_HEAPS);
	D3D12_FEATURE_DATA_COMMAND_QUEUE_PRIORITY Feature_COMMAND_QUEUE_PRIORITY = {};
	CheckD3DFeature(D3D12_FEATURE_COMMAND_QUEUE_PRIORITY, Feature_COMMAND_QUEUE_PRIORITY);
	D3D12_FEATURE_DATA_SHADER_CACHE Feature_SHADER_CACHE = {};
	CheckD3DFeature(D3D12_FEATURE_SHADER_CACHE, Feature_SHADER_CACHE);
	D3D12_FEATURE_DATA_ROOT_SIGNATURE Feature_ROOT_SIGNATURE = {};
	CheckD3DFeature(D3D12_FEATURE_ROOT_SIGNATURE, Feature_ROOT_SIGNATURE);
	D3D12_FEATURE_DATA_PROTECTED_RESOURCE_SESSION_SUPPORT Feature_PROTECTED_RESOURCE_SESSION_SUPPORT = {};
	CheckD3DFeature(D3D12_FEATURE_PROTECTED_RESOURCE_SESSION_SUPPORT, Feature_PROTECTED_RESOURCE_SESSION_SUPPORT);
	D3D12_FEATURE_DATA_SHADER_MODEL Feature_SHADER_MODEL = { D3D_HIGHEST_SHADER_MODEL };
	CheckD3DFeature(D3D12_FEATURE_SHADER_MODEL, Feature_SHADER_MODEL);
	D3D12_FEATURE_DATA_GPU_VIRTUAL_ADDRESS_SUPPORT Feature_GPU_VIRTUAL_ADDRESS_SUPPORT = {};
	CheckD3DFeature(D3D12_FEATURE_GPU_VIRTUAL_ADDRESS_SUPPORT, Feature_GPU_VIRTUAL_ADDRESS_SUPPORT);
	D3D12_FEATURE_DATA_FORMAT_INFO Feature_FORMAT_INFO = {};
	CheckD3DFeature(D3D12_FEATURE_FORMAT_INFO, Feature_FORMAT_INFO);
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS Feature_MULTISAMPLE_QUALITY_LEVELS = {};
	CheckD3DFeature(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, Feature_MULTISAMPLE_QUALITY_LEVELS);
	D3D12_FEATURE_DATA_FORMAT_SUPPORT Feature_FORMAT_SUPPORT = {};
	CheckD3DFeature(D3D12_FEATURE_FORMAT_SUPPORT, Feature_FORMAT_SUPPORT);
	D3D12_FEATURE_DATA_ARCHITECTURE Feature_ARCHITECTURE = {};
	CheckD3DFeature(D3D12_FEATURE_ARCHITECTURE, Feature_ARCHITECTURE);
	D3D12_FEATURE_DATA_ARCHITECTURE1 Feature_FEATURE_ARCHITECTURE1 = {};
	CheckD3DFeature(D3D12_FEATURE_ARCHITECTURE1, Feature_FEATURE_ARCHITECTURE1);
	D3D12_FEATURE_DATA_D3D12_OPTIONS Feature_D3D12_OPTIONS = {};
	CheckD3DFeature(D3D12_FEATURE_D3D12_OPTIONS, Feature_D3D12_OPTIONS);

	bIsEnhancedBarriersSupported = static_cast<bool>(Feature12.EnhancedBarriersSupported);

	{
		if (DeviceType == EGPUDeviceType::Software)
		{
			std::cout << "WARP Device" << std::endl;
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

	CommandAllocatorPool = D3D12CommandAllocatorManager::CreateUnique(Direct3DDevice.Get());

	IMCommandList = std::make_unique<D3D12CommandBuffer>(this);
#if _DEBUG
	IMCommandList->Get()->SetName(L"IMCommandList");
#endif

	IMCopyCommandList = D3D12CopyCommandBuffer::CreateUnique(this);
#if _DEBUG
	IMCopyCommandList->Get()->SetName(L"IMCopyCommandList");
#endif

	DescriptorHeapManager = std::make_unique<D3D12DescriptorHeapManager>(this);

	InitWindow(DeviceCreateInfo.pHWND, DeviceCreateInfo.Width, DeviceCreateInfo.Height, DeviceCreateInfo.Fullscreen);
}

D3D12Device::~D3D12Device()
{
	{
		GPUSignal();
		CpuWait(D3D12_COMMAND_LIST_TYPE_DIRECT);
	}
	for (auto& RootSignature : RootSignatureCache)
	{
		RootSignature.second = nullptr;
	}
	RootSignatureCache.clear();

	for (auto& Sampler : SamplerCache)
	{
		Sampler.second = nullptr;
	}
	SamplerCache.clear();

	GPUFlush();

#if _DEBUG && DEBUG_D3DDEVICE
	ID3D12DebugDevice* DebugDevice = nullptr;
	Direct3DDevice->QueryInterface(__uuidof(ID3D12DebugDevice), (void**)(&DebugDevice));
#endif

	ShaderCompiler = nullptr;
	//pD3D12ShaderCompiler = nullptr;

	Viewport = nullptr;

	DescriptorHeapManager = nullptr;
	DxgiFactory = nullptr;
	IMCommandList = nullptr;
	IMCopyCommandList = nullptr;

	for (auto& Fence : Fences)
	{
		delete Fence.second;
		Fence.second = nullptr;
	}
	Fences.clear();

	//for (auto& DeferredCommandList : DeferredCommandLists)
	//{
	//	DeferredCommandList = nullptr;
	//}
	//DeferredCommandLists.clear();

	GraphicsQueue = nullptr;
	ComputeQueue = nullptr;
	CopyQueue = nullptr;

	CommandAllocatorPool = nullptr;

#if _DEBUG && DEBUG_D3DDEVICE
	if (DebugDevice)
		DebugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
	IDXGIDebug* debugDev = nullptr;
	DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debugDev));
	if (debugDev)
		debugDev->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
#endif
	Direct3DDevice14 = nullptr;

	//Direct3DDevice->Release();
	Direct3DDevice = nullptr;
}

void D3D12Device::InitWindow(void* InHWND, std::uint32_t InWidth, std::uint32_t InHeight, bool bFullscreen)
{
	if (IsPrimaryGPU())
	{
		if (!Viewport)
			Viewport = std::make_unique<D3D12Viewport>(this, DxgiFactory, InWidth, InHeight, bFullscreen, (HWND)InHWND);
	}
}

void D3D12Device::BeginFrame()
{
	if (IsPrimaryGPU())
		Viewport->BeginFrame();
}

void D3D12Device::Present(IRenderTarget* pRT)
{
	if (IsPrimaryGPU())
	{
		Viewport->OnPresent(pRT);
	}

	if (DeferredCommandLists.size() > 0)
	{
		for (auto& DeferredCommandList : DeferredCommandLists)
		{
			auto& PendingList = DeferredCommandList.second;
			std::vector<ID3D12CommandList*> lists;
			for (const auto& submission : PendingList)
				lists.push_back(submission.CommandList.Get());

			switch (DeferredCommandList.first)
			{
			case D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT:
			{
#if Pix3_Enabled && _DEBUG
				PIXBeginEvent(GraphicsQueue.Get(), 0, L"D3D12Device::ExecuteGraphicsQueue");
#endif
				GraphicsQueue->ExecuteCommandLists(static_cast<UINT>(lists.size()), lists.data());

#if Pix3_Enabled && _DEBUG
				PIXEndEvent(GraphicsQueue.Get());
#endif 
			}
				break;
			case D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_COMPUTE:
			{
#if Pix3_Enabled && _DEBUG
				PIXBeginEvent(ComputeQueue.Get(), 0, L"D3D12Device::ExecuteComputeQueue");
#endif
				ComputeQueue->ExecuteCommandLists(static_cast<UINT>(lists.size()), lists.data());

#if Pix3_Enabled && _DEBUG
				PIXEndEvent(ComputeQueue.Get());
#endif 
			}
			break;
			case D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_COPY:
			{
#if Pix3_Enabled && _DEBUG
				PIXBeginEvent(CopyQueue.Get(), 0, L"D3D12Device::ExecuteCopyQueue");
#endif
				CopyQueue->ExecuteCommandLists(static_cast<UINT>(lists.size()), lists.data());

#if Pix3_Enabled && _DEBUG
				PIXEndEvent(CopyQueue.Get());
#endif 
			}
			break;
			}
			const auto fenceValue = GPUSignal(DeferredCommandList.first);

			for (const auto& submission : PendingList)
				DiscardCommandAllocator(submission.Type, submission.Allocator, fenceValue);

			PendingList.clear();
		}
		DeferredCommandLists.clear();
	}

#if Pix3_Enabled && _DEBUG
	PIXBeginEvent(GraphicsQueue.Get(), 0, L"D3D12Device::Present");
#endif
	if (IsPrimaryGPU())
	{
		Viewport->Present();
	}
#if Pix3_Enabled && _DEBUG
	PIXEndEvent(GraphicsQueue.Get());
#endif 

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
	if (IsPrimaryGPU())
		Viewport->FullScreen(value);
}

void D3D12Device::Vsync(const bool value)
{
	if (IsPrimaryGPU())
		Viewport->Vsync(value);
}

void D3D12Device::VsyncInterval(const std::uint32_t value)
{
	if (IsPrimaryGPU())
		Viewport->VsyncInterval(value);
}

bool D3D12Device::IsFullScreen() const
{
	if (IsPrimaryGPU())
		return Viewport->IsFullScreen();
	return false;
}

bool D3D12Device::IsVsyncEnabled() const
{
	if (IsPrimaryGPU())
		return Viewport->IsVsyncEnabled();
	return false;
}

std::uint32_t D3D12Device::GetVsyncInterval() const
{
	if (IsPrimaryGPU())
		return Viewport->GetVsyncInterval();
	return -1;
}

void D3D12Device::GPUFlush()
{
	GPUFlush(D3D12_COMMAND_LIST_TYPE_DIRECT);
	GPUFlush(D3D12_COMMAND_LIST_TYPE_COMPUTE);
	GPUFlush(D3D12_COMMAND_LIST_TYPE_COPY);
}

void D3D12Device::WaitForGPU()
{
	GPUSignal(D3D12_COMMAND_LIST_TYPE_DIRECT);
	CpuWait(D3D12_COMMAND_LIST_TYPE_DIRECT);

	GPUSignal(D3D12_COMMAND_LIST_TYPE_COMPUTE);
	CpuWait(D3D12_COMMAND_LIST_TYPE_COMPUTE);

	GPUSignal(D3D12_COMMAND_LIST_TYPE_COPY);
	CpuWait(D3D12_COMMAND_LIST_TYPE_COPY);
}

void D3D12Device::WaitForCPU()
{
	CpuWait(D3D12_COMMAND_LIST_TYPE_DIRECT);
	CpuWait(D3D12_COMMAND_LIST_TYPE_COMPUTE);
	CpuWait(D3D12_COMMAND_LIST_TYPE_COPY);
}

std::vector<sDisplayMode> D3D12Device::GetAllSupportedResolutions() const
{
	if (IsPrimaryGPU())
		return Viewport->GetAllSupportedResolutions();
	return std::vector<sDisplayMode>();
}

sScreenDimension D3D12Device::GetBackBufferDimension() const
{
	if (IsPrimaryGPU())
		return Viewport->GetScreenDimension();
	return sScreenDimension();
}

EFormat D3D12Device::GetBackBufferFormat() const
{
	if (IsPrimaryGPU())
		return Viewport->GetBackBufferFormat();
	return EFormat::UNKNOWN;
}

sViewport D3D12Device::GetViewport() const
{
	if (IsPrimaryGPU())
		return Viewport->GetViewport();
	return sViewport();
}

std::uint32_t D3D12Device::GetBackBufferSize() const
{
	if (!Viewport)
		return std::uint32_t(-1);

	return Viewport->GetBackBufferCount();
}

std::uint32_t D3D12Device::GetCurrentBackBufferIndex() const
{
	if (!Viewport)
		return std::uint32_t(-1);

	return Viewport->GetCurrentBackBufferIndex();
}

std::shared_ptr<D3D12RootSignature> D3D12Device::CreateRootSignature(const std::vector<sShaderBinding>& Bindings, const sIndirectLayoutBindingDesc& IndirectDesc)
{
	std::size_t Hash = sDescriptorSetLayoutBindingVectorHash{}(Bindings);
	if (RootSignatureCache.contains(Hash))
	{
		return RootSignatureCache[Hash];
	}
	
	D3D12RootSignature::SharedPtr RootSignature = D3D12RootSignature::Create(Direct3DDevice.Get(), Bindings, IndirectDesc);
	RootSignatureCache.insert({ Hash, RootSignature });
	return RootSignature;
}

IShader* D3D12Device::CompileD3D12Shader(const sShaderAttachment& Attachment, bool Spirv)
{
	if (sShaderManager::Get().IsShaderExist(Attachment.GetLocation(), Attachment.FunctionName))
		return sShaderManager::Get().GetShader(Attachment.GetLocation(), Attachment.FunctionName);

	IShader::SharedPtr pShader = ShaderCompiler->Compile(Attachment, Spirv);
	//IShader::SharedPtr pShader = pD3D12ShaderCompiler->Compile(Attachment);
	sShaderManager::Get().StoreShader(pShader);
	return pShader.get();
}

IShader* D3D12Device::CompileD3D12Shader(std::wstring InSrcFile, std::string InFunctionName, eShaderType InProfile, bool Spirv, std::vector<sShaderDefines> InDefines)
{
	if (sShaderManager::Get().IsShaderExist(InSrcFile, InFunctionName))
		return sShaderManager::Get().GetShader(InSrcFile, InFunctionName);

	IShader::SharedPtr pShader = ShaderCompiler->Compile(InSrcFile, InFunctionName, InProfile, Spirv, InDefines);
	//IShader::SharedPtr pShader = pD3D12ShaderCompiler->Compile(InSrcFile, InFunctionName, InProfile, InDefines);
	sShaderManager::Get().StoreShader(pShader);
	return pShader.get();
}

IShader* D3D12Device::CompileD3D12Shader(const void* InCode, std::size_t Size, std::string InFunctionName, eShaderType InProfile, bool Spirv, std::vector<sShaderDefines> InDefines)
{
	if (sShaderManager::Get().IsShaderExist(L"", InFunctionName))
		return sShaderManager::Get().GetShader(L"", InFunctionName);

	IShader::SharedPtr pShader = ShaderCompiler->Compile(InCode, Size, InFunctionName, InProfile, Spirv, InDefines);
	//IShader::SharedPtr pShader = pD3D12ShaderCompiler->Compile(InCode, Size, InFunctionName, InProfile, InDefines);
	sShaderManager::Get().StoreShader(pShader);
	return pShader.get();
}

IShader::SharedPtr D3D12Device::CompileShader(const sShaderAttachment& Attachment)
{
	bool Spirv = false;
	if (sShaderManager::Get().IsShaderExist(Attachment.GetLocation(), Attachment.FunctionName))
		return sShaderManager::Get().GetShaderAsShared(Attachment.GetLocation(), Attachment.FunctionName);

	IShader::SharedPtr pShader = ShaderCompiler->Compile(Attachment, Spirv);
	//IShader::SharedPtr pShader = pD3D12ShaderCompiler->Compile(Attachment);
	sShaderManager::Get().StoreShader(pShader);
	return pShader;
}

IShader::SharedPtr D3D12Device::CompileShader(std::wstring InSrcFile, std::string InFunctionName, eShaderType InProfile, std::vector<sShaderDefines> InDefines)
{
	bool Spirv = false;
	if (sShaderManager::Get().IsShaderExist(InSrcFile, InFunctionName))
		return sShaderManager::Get().GetShaderAsShared(InSrcFile, InFunctionName);

	IShader::SharedPtr pShader = ShaderCompiler->Compile(InSrcFile, InFunctionName, InProfile, Spirv, InDefines);
	//IShader::SharedPtr pShader = pD3D12ShaderCompiler->Compile(InSrcFile, InFunctionName, InProfile, InDefines);
	sShaderManager::Get().StoreShader(pShader);
	return pShader;
}

IShader::SharedPtr D3D12Device::CompileShader(const void* InCode, std::size_t Size, std::string InFunctionName, eShaderType InProfile, std::vector<sShaderDefines> InDefines)
{
	bool Spirv = false;
	if (sShaderManager::Get().IsShaderExist(L"", InFunctionName))
		return sShaderManager::Get().GetShaderAsShared(L"", InFunctionName);

	IShader::SharedPtr pShader = ShaderCompiler->Compile(InCode, Size, InFunctionName, InProfile, Spirv, InDefines);
	//IShader::SharedPtr pShader = pD3D12ShaderCompiler->Compile(InCode, Size, InFunctionName, InProfile, InDefines);
	sShaderManager::Get().StoreShader(pShader);
	return pShader;
}

bool D3D12Device::IsCommandBufferPendingForExecute(ICommandContext* CommandContext) const
{
	if (!CommandContext)
		return false;

	switch (CommandContext->GetCommandContextType())
	{
		case ECommandContextType::Graphics:
		{
			if (!DeferredCommandLists.contains(D3D12_COMMAND_LIST_TYPE_DIRECT))
				return false;
			for (const auto& Deferred : DeferredCommandLists.at(D3D12_COMMAND_LIST_TYPE_DIRECT))
			{
				if (Deferred.CommandBuffer == CommandContext)
					return true;
			}
			break;
		}
		case ECommandContextType::Compute:
		{
			if (!DeferredCommandLists.contains(D3D12_COMMAND_LIST_TYPE_COMPUTE))
				return false;
			for (const auto& Deferred : DeferredCommandLists.at(D3D12_COMMAND_LIST_TYPE_COMPUTE))
			{
				if (Deferred.CommandBuffer == CommandContext)
					return true;
			}
			break;
		}
		case ECommandContextType::Copy:
		{
			if (!DeferredCommandLists.contains(D3D12_COMMAND_LIST_TYPE_COPY))
				return false;
			for (const auto& Deferred : DeferredCommandLists.at(D3D12_COMMAND_LIST_TYPE_COPY))
			{
				if (Deferred.CommandBuffer == CommandContext)
					return true;
			}
			break;
		}
	}
	return false;
}

void D3D12Device::OnCommandBufferDestroyed(ICommandContext* CommandContext)
{
	switch (CommandContext->GetCommandContextType())
	{
		case ECommandContextType::Graphics:
		{
			for (auto& DeferredCommandList : DeferredCommandLists[D3D12_COMMAND_LIST_TYPE_DIRECT])
			{
				if (DeferredCommandList.CommandBuffer == CommandContext)
				{
					DeferredCommandList.CommandBuffer = nullptr;
					break;
				}
			}
		}
		break;
		case ECommandContextType::Compute:
		{
			for (auto& DeferredCommandList : DeferredCommandLists[D3D12_COMMAND_LIST_TYPE_COMPUTE])
			{
				if (DeferredCommandList.CommandBuffer == CommandContext)
				{
					DeferredCommandList.CommandBuffer = nullptr;
					break;
				}
			}
		}
		break;
		case ECommandContextType::Copy:
		{
			for (auto& DeferredCommandList : DeferredCommandLists[D3D12_COMMAND_LIST_TYPE_COPY])
			{
				if (DeferredCommandList.CommandBuffer == CommandContext)
				{
					DeferredCommandList.CommandBuffer = nullptr;
					break;
				}
			}
		}
		break;
	}
}

std::uint64_t D3D12Device::ExecuteDirectCommandLists(ECommandContextExecuteType ExecuteType, std::uint32_t Order, D3D12CommandBuffer* CommandList, bool WaitForCompletion)
{
	std::lock_guard<std::mutex> lock(Mutex);
	if (ExecuteType == ECommandContextExecuteType::Immediate)
	{
#if Pix3_Enabled && _DEBUG
		PIXBeginEvent(GraphicsQueue.Get(), 0, L"D3D12Device::ExecuteDirectCommandLists");
#endif
		ID3D12CommandList* pCommandList = CommandList->Get();
		GraphicsQueue->ExecuteCommandLists(1, &pCommandList);
		const auto fenceValue = GPUSignal(D3D12_COMMAND_LIST_TYPE_DIRECT);

		if (WaitForCompletion)
			CpuWaitForFence(D3D12_COMMAND_LIST_TYPE_DIRECT, fenceValue);

#if Pix3_Enabled && _DEBUG
		PIXEndEvent(GraphicsQueue.Get());
#endif

		return fenceValue;

	}
	else if (ExecuteType == ECommandContextExecuteType::Deferred)
	{
		PendingSubmission Submission;
		Submission.CommandBuffer = CommandList;
		Submission.CommandList = CommandList->GetAsCOM();
		Submission.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		Submission.Allocator = CommandList->GetAllocator();
		if (DeferredCommandLists[D3D12_COMMAND_LIST_TYPE_DIRECT].size() > Order)
		{
			auto Begin = DeferredCommandLists[D3D12_COMMAND_LIST_TYPE_DIRECT].begin();
			DeferredCommandLists[D3D12_COMMAND_LIST_TYPE_DIRECT].insert(Begin + Order, Submission);
		}
		else
		{
			DeferredCommandLists[D3D12_COMMAND_LIST_TYPE_DIRECT].push_back(Submission);
		}
	}

	return std::uint64_t(-1);
}

std::uint64_t D3D12Device::ExecuteComputeCommandLists(ECommandContextExecuteType ExecuteType, std::uint32_t Order, D3D12ComputeCommandContext* CommandList, bool WaitForCompletion)
{
	std::lock_guard<std::mutex> lock(Mutex);
	if (ExecuteType == ECommandContextExecuteType::Immediate)
	{
#if Pix3_Enabled && _DEBUG
		PIXBeginEvent(ComputeQueue.Get(), 0, L"D3D12Device::ExecuteComputeCommandLists");
#endif
		ID3D12CommandList* pCommandList = CommandList->Get();
		ComputeQueue->ExecuteCommandLists(1, &pCommandList);
		const auto fenceValue = GPUSignal(D3D12_COMMAND_LIST_TYPE_COMPUTE);

		if (WaitForCompletion)
			CpuWaitForFence(D3D12_COMMAND_LIST_TYPE_COMPUTE, fenceValue);

#if Pix3_Enabled && _DEBUG
		PIXEndEvent(ComputeQueue.Get());
#endif

		return fenceValue; 
	}
	else if (ExecuteType == ECommandContextExecuteType::Deferred)
	{
		PendingSubmission Submission;
		Submission.CommandBuffer = CommandList;
		Submission.CommandList = CommandList->GetAsCOM();
		Submission.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		Submission.Allocator = CommandList->GetAllocator();
		if (DeferredCommandLists[D3D12_COMMAND_LIST_TYPE_COMPUTE].size() > Order)
		{
			auto Begin = DeferredCommandLists[D3D12_COMMAND_LIST_TYPE_COMPUTE].begin();
			DeferredCommandLists[D3D12_COMMAND_LIST_TYPE_COMPUTE].insert(Begin + Order, Submission);
		}
		else
		{
			DeferredCommandLists[D3D12_COMMAND_LIST_TYPE_COMPUTE].push_back(Submission);
		}
	}
	return std::uint64_t(-1);
}

std::uint64_t D3D12Device::ExecuteCopyCommandLists(ECommandContextExecuteType ExecuteType, std::uint32_t Order, D3D12CopyCommandBuffer* CommandList, bool WaitForCompletion)
{
	std::lock_guard<std::mutex> lock(Mutex);
	if (ExecuteType == ECommandContextExecuteType::Immediate)
	{
#if Pix3_Enabled && _DEBUG
		PIXBeginEvent(CopyQueue.Get(), 0, L"D3D12Device::ExecuteCopyCommandLists");
#endif
		ID3D12CommandList* pCommandList = CommandList->Get();
		CopyQueue->ExecuteCommandLists(1, &pCommandList);
		const auto fenceValue = GPUSignal(D3D12_COMMAND_LIST_TYPE_COPY);

		if (WaitForCompletion)
			CpuWaitForFence(D3D12_COMMAND_LIST_TYPE_COPY, fenceValue);

#if Pix3_Enabled && _DEBUG
		PIXEndEvent(CopyQueue.Get());
#endif

		return fenceValue;
	}
	else if (ExecuteType == ECommandContextExecuteType::Deferred)
	{
		PendingSubmission Submission;
		Submission.CommandBuffer = CommandList;
		Submission.CommandList = CommandList->GetAsCOM();
		Submission.Type = D3D12_COMMAND_LIST_TYPE_COPY;
		Submission.Allocator = CommandList->GetAllocator();
		if (DeferredCommandLists[D3D12_COMMAND_LIST_TYPE_COPY].size() > Order)
		{
			auto Begin = DeferredCommandLists[D3D12_COMMAND_LIST_TYPE_COPY].begin();
			DeferredCommandLists[D3D12_COMMAND_LIST_TYPE_COPY].insert(Begin + Order, Submission);
		}
		else
		{
			DeferredCommandLists[D3D12_COMMAND_LIST_TYPE_COPY].push_back(Submission);
		}
	}
	return std::uint64_t(-1);
}

std::uint64_t D3D12Device::GPUSignal(D3D12_COMMAND_LIST_TYPE Type)
{
	switch (Type)
	{
	case D3D12_COMMAND_LIST_TYPE_BUNDLE:
	case D3D12_COMMAND_LIST_TYPE_DIRECT:
		return Fences.at(D3D12_COMMAND_LIST_TYPE_DIRECT)->Signal(GraphicsQueue.Get());
	case D3D12_COMMAND_LIST_TYPE_COMPUTE:
		return Fences.at(D3D12_COMMAND_LIST_TYPE_COMPUTE)->Signal(ComputeQueue.Get());
	case D3D12_COMMAND_LIST_TYPE_COPY:
		return Fences.at(D3D12_COMMAND_LIST_TYPE_COPY)->Signal(CopyQueue.Get());
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

	return std::uint64_t(-1);
}

void D3D12Device::CpuWaitForFence(D3D12_COMMAND_LIST_TYPE Type, std::uint64_t FenceValue)
{
	Fences.at(Type == D3D12_COMMAND_LIST_TYPE_BUNDLE ? D3D12_COMMAND_LIST_TYPE_DIRECT : Type)->CpuWaitForFence(FenceValue);
}

void D3D12Device::CpuWait(D3D12_COMMAND_LIST_TYPE Type)
{
	Fences.at(Type == D3D12_COMMAND_LIST_TYPE_BUNDLE ? D3D12_COMMAND_LIST_TYPE_DIRECT : Type)->CpuWait();
}

ComPtr<ID3D12CommandAllocator> D3D12Device::RequestCommandAllocator(D3D12_COMMAND_LIST_TYPE cmdType)
{
	return CommandAllocatorPool->RequestAllocator(cmdType == D3D12_COMMAND_LIST_TYPE_BUNDLE ? D3D12_COMMAND_LIST_TYPE_DIRECT : cmdType, Fences.at(cmdType)->GetCompletedValue());
}

void D3D12Device::DiscardCommandAllocator(D3D12_COMMAND_LIST_TYPE cmdType, ComPtr<ID3D12CommandAllocator> Allocator, std::uint64_t FenceValue)
{
	CommandAllocatorPool->DiscardAllocator(cmdType == D3D12_COMMAND_LIST_TYPE_BUNDLE ? D3D12_COMMAND_LIST_TYPE_DIRECT : cmdType, FenceValue, Allocator);
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

void D3D12Device::SetHeaps(ID3D12GraphicsCommandList* cmd)
{
	DescriptorHeapManager->SetHeaps(cmd);
}

ID3D12DescriptorHeap* D3D12Device::GetHeap() const
{
	return DescriptorHeapManager->GetHeap();
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

IGraphicsCommandContext::SharedPtr D3D12Device::CreateGraphicsCommandContext()
{
	return D3D12CommandBuffer::Create(this);
}

IGraphicsCommandContext::UniquePtr D3D12Device::CreateUniqueGraphicsCommandContext()
{
	return D3D12CommandBuffer::CreateUnique(this);
}

IComputeCommandContext::SharedPtr D3D12Device::CreateComputeCommandContext()
{
	return D3D12ComputeCommandContext::Create(this);
}

IComputeCommandContext::UniquePtr D3D12Device::CreateUniqueComputeCommandContext()
{
	return D3D12ComputeCommandContext::CreateUnique(this);
}

ICopyCommandContext::SharedPtr D3D12Device::CreateCopyCommandContext()
{
	return D3D12CopyCommandBuffer::Create(this);
}

ICopyCommandContext::UniquePtr D3D12Device::CreateUniqueCopyCommandContext()
{
	return D3D12CopyCommandBuffer::CreateUnique(this);
}

IConstantBuffer::SharedPtr D3D12Device::CreateConstantBuffer(std::string InName, const BufferLayout& InDesc, std::uint32_t InRootParameterIndex)
{
	return D3D12ConstantBuffer::Create(this, InName, InDesc, InRootParameterIndex);
}

IConstantBuffer::UniquePtr D3D12Device::CreateUniqueConstantBuffer(std::string InName, const BufferLayout& InDesc, std::uint32_t InRootParameterIndex)
{
	return D3D12ConstantBuffer::CreateUnique(this, InName, InDesc, InRootParameterIndex);
}

IVertexBuffer::SharedPtr D3D12Device::CreateVertexBuffer(std::string InName, const BufferLayout& InDesc, BufferSubresource* InSubresource)
{
	return D3D12VertexBuffer::Create(this, InName, InDesc, InSubresource);
}

IVertexBuffer::UniquePtr D3D12Device::CreateUniqueVertexBuffer(std::string InName, const BufferLayout& InDesc, BufferSubresource* InSubresource)
{
	return D3D12VertexBuffer::CreateUnique(this, InName, InDesc, InSubresource);
}

IIndexBuffer::SharedPtr D3D12Device::CreateIndexBuffer(std::string InName, const BufferLayout& InDesc, BufferSubresource* InSubresource)
{
	return D3D12IndexBuffer::Create(this, InName, InDesc, InSubresource);
}

IIndexBuffer::UniquePtr D3D12Device::CreateUniqueIndexBuffer(std::string InName, const BufferLayout& InDesc, BufferSubresource* InSubresource)
{
	return D3D12IndexBuffer::CreateUnique(this, InName, InDesc, InSubresource);
}

IByteAddressBuffer::SharedPtr D3D12Device::CreateByteAddressBuffer(std::string InName, std::uint64_t Size, bool bReadWriteAllowed)
{
	return D3D12ByteAddressBuffer::Create(this, InName, Size, bReadWriteAllowed);
}

IByteAddressBuffer::UniquePtr D3D12Device::CreateUniqueByteAddressBuffer(std::string InName, std::uint64_t Size, bool bReadWriteAllowed)
{
	return D3D12ByteAddressBuffer::CreateUnique(this, InName, Size, bReadWriteAllowed);
}

IStructuredBuffer::SharedPtr D3D12Device::CreateStructuredBuffer(std::string InName, const BufferLayout& InDesc, bool bSRVAllowed)
{
	return D3D12StructuredBuffer::Create(this, InName, InDesc, bSRVAllowed);
}

IStructuredBuffer::UniquePtr D3D12Device::CreateUniqueStructuredBuffer(std::string InName, const BufferLayout& InDesc, bool bSRVAllowed)
{
	return D3D12StructuredBuffer::CreateUnique(this, InName, InDesc, bSRVAllowed);
}

IIndirectBuffer::SharedPtr D3D12Device::CreateIndirectBuffer(std::string InName, BufferLayout NewLayout)
{
	return D3D12IndirectBuffer::Create(this, InName, NewLayout);
}

IIndirectBuffer::UniquePtr D3D12Device::CreateUniqueIndirectBuffer(std::string InName, BufferLayout NewLayout)
{
	return D3D12IndirectBuffer::CreateUnique(this, InName, NewLayout);
}

IFrameBuffer::SharedPtr D3D12Device::CreateFrameBuffer(const std::string InName, const sFrameBufferAttachmentInfo& InAttachments)
{
	return D3D12FrameBuffer::Create(this, InName, InAttachments);
}

IFrameBuffer::UniquePtr D3D12Device::CreateUniqueFrameBuffer(const std::string InName, const sFrameBufferAttachmentInfo& InAttachments)
{
	return D3D12FrameBuffer::CreateUnique(this, InName, InAttachments);
}

ISamplerState::SharedPtr D3D12Device::CreateSamplerState(const std::string InName, const sSamplerAttributeDesc& InDesc)
{
	std::size_t Hash = sSamplerAttributeDescHash{}(InDesc);
	if (SamplerCache.contains(Hash))
	{
		return SamplerCache[Hash];
	}

	ISamplerState::SharedPtr Sampler = D3D12SamplerState::Create(this, InName, InDesc);
	SamplerCache.insert({ Hash, Sampler });
	return Sampler;
}

ISamplerState::UniquePtr D3D12Device::CreateUniqueSamplerState(const std::string InName, const sSamplerAttributeDesc& InDesc)
{
	return D3D12SamplerState::CreateUnique(this, InName, InDesc);
}

IRenderTarget::SharedPtr D3D12Device::CreateRenderTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc)
{
	return D3D12RenderTarget::Create(this, InName, Format, Desc);
}

IRenderTarget::UniquePtr D3D12Device::CreateUniqueRenderTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc)
{
	return D3D12RenderTarget::CreateUnique(this, InName, Format, Desc);
}

IDepthTarget::SharedPtr D3D12Device::CreateDepthTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc)
{
	return D3D12DepthTarget::Create(this, InName, Format, Desc);
}

IDepthTarget::UniquePtr D3D12Device::CreateUniqueDepthTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc)
{
	return D3D12DepthTarget::CreateUnique(this, InName, Format, Desc);
}

IUnorderedAccessTarget::SharedPtr D3D12Device::CreateUnorderedAccessTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc, bool InEnableSRV)
{
	return D3D12UnorderedAccessTarget::Create(this, InName, Format, Desc, InEnableSRV);
}

IUnorderedAccessTarget::UniquePtr D3D12Device::CreateUniqueUnorderedAccessTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc, bool InEnableSRV)
{
	return D3D12UnorderedAccessTarget::CreateUnique(this, InName, Format, Desc, InEnableSRV);
}

IPipeline::SharedPtr D3D12Device::CreatePipeline(const std::string& InName, const sPipelineDesc& InDesc)
{
	return D3D12Pipeline::Create(this, InName, InDesc);
}

IPipeline::UniquePtr D3D12Device::CreateUniquePipeline(const std::string& InName, const sPipelineDesc& InDesc)
{
	return D3D12Pipeline::CreateUnique(this, InName, InDesc);
}

IComputePipeline::SharedPtr D3D12Device::CreateComputePipeline(const std::string& InName, const sComputePipelineDesc& InDesc)
{
	return D3D12ComputePipeline::Create(this, InName, InDesc);
}

IComputePipeline::UniquePtr D3D12Device::CreateUniqueComputePipeline(const std::string& InName, const sComputePipelineDesc& InDesc)
{
	return D3D12ComputePipeline::CreateUnique(this, InName, InDesc);
}

ITexture2D::SharedPtr D3D12Device::CreateTexture2D(const std::wstring FilePath, const std::string InName, std::uint32_t DefaultRootParameterIndex)
{
	return D3D12Texture::Create(this, FilePath, InName, DefaultRootParameterIndex);
}

ITexture2D::UniquePtr D3D12Device::CreateUniqueTexture2D(const std::wstring FilePath, const std::string InName, std::uint32_t DefaultRootParameterIndex)
{
	return D3D12Texture::CreateUnique(this, FilePath, InName, DefaultRootParameterIndex);
}

ITexture2D::SharedPtr D3D12Device::CreateTexture2D(const std::string InName, void* InBuffer, const std::size_t InSize, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
{
	return D3D12Texture::Create(this, InName, InBuffer, InSize, InDesc, DefaultRootParameterIndex);
}

ITexture2D::UniquePtr D3D12Device::CreateUniqueTexture2D(const std::string InName, void* InBuffer, const std::size_t InSize, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
{
	return D3D12Texture::CreateUnique(this, InName, InBuffer, InSize, InDesc, DefaultRootParameterIndex);
}

ITexture2D::SharedPtr D3D12Device::CreateEmptyTexture2D(const std::string InName, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
{
	return D3D12Texture::Create(this, InName, InDesc, DefaultRootParameterIndex);
}

ITexture2D::UniquePtr D3D12Device::CreateUniqueEmptyTexture2D(const std::string InName, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
{
	return D3D12Texture::CreateUnique(this, InName, InDesc, DefaultRootParameterIndex);
}

//ITiledTexture::SharedPtr D3D12Device::CreateTiledTexture(const std::string InName, const std::uint32_t InTileX, const std::uint32_t InTileY, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
//{
//	DebugBreak();
//	return nullptr;
//	//return D3D11TiledTexture::Create(this, InName, InTileX, InTileY, InDesc, DefaultRootParameterIndex);
//}
//
//ITiledTexture::UniquePtr D3D12Device::CreateUniqueTiledTexture(const std::string InName, const std::uint32_t InTileX, const std::uint32_t InTileY, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
//{
//	DebugBreak();
//	return nullptr;
//	//return D3D11TiledTexture::CreateUnique(this, InName, InTileX, InTileY, InDesc, DefaultRootParameterIndex);
//}
