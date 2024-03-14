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

#include <memory>
#include <map>
#include <mutex>
#include <queue>
#include <wrl/client.h>
#include <string>

#ifdef USING_DIRECTX_HEADERS
#include <directx/d3d12.h>
#else
#include <d3d12.h>
#endif

#include <dxgi.h>
#include <dxgi1_6.h>

#include "GI/AbstractGI/AbstractGIDevice.h"

using namespace Microsoft::WRL;

class D3D12Viewport;
class D3D12Texture;
class D3D12CommandBuffer;
class D3D12CopyCommandBuffer;
struct D3D12DescriptorHandle;
class D3D12DescriptorHeapManager;
class D3D12Fence;

class D3D12Device final : public IAbstractGIDevice
{
	sClassBody(sClassConstructor, D3D12Device, IAbstractGIDevice)
public:
	D3D12Device(std::optional<short> InGPUIndex = std::nullopt);
	virtual ~D3D12Device();
	virtual void InitWindow(void* HWND, std::uint32_t Width, std::uint32_t Height, bool Fullscreen) override final;
	virtual void Present(IRenderTarget* pRT) override final;
	bool GetDeviceIdentification(std::wstring& InVendorID, std::wstring& InDeviceID) const;
	IDXGIAdapter* FindAdapter(const WCHAR* InTargetName) const;
	IDXGIAdapter1* GetAdapter(std::optional<short> Index = std::nullopt) const;

	void ExecuteDirectCommandLists(ID3D12CommandList* pCommandList, bool WaitForCompletion = false);
	void ExecuteComputeCommandLists(ID3D12CommandList* pCommandList, bool WaitForCompletion = false);
	void ExecuteCopyCommandLists(ID3D12CommandList* pCommandList, bool WaitForCompletion = false);

	void GPUSignal(D3D12_COMMAND_LIST_TYPE Type = D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);
	void CPUWait(D3D12_COMMAND_LIST_TYPE Type = D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);

	ID3D12CommandAllocator* RequestCommandAllocator(D3D12_COMMAND_LIST_TYPE Type, std::optional<std::uint64_t> CompletedFenceValue = std::nullopt);
	void DiscardCommandAllocator(D3D12_COMMAND_LIST_TYPE Type, ID3D12CommandAllocator* Allocator, std::optional<std::uint64_t> FenceValue = std::nullopt);

	void GPUFlush(D3D12_COMMAND_LIST_TYPE queueType);
	void GPUFlush();

	void SetHeaps(ID3D12GraphicsCommandList* cmd);
	void AllocateDescriptor(D3D12DescriptorHandle* DescriptorHandle);
	void DeallocateDescriptor(D3D12DescriptorHandle* DescriptorHandle);

	inline D3D_FEATURE_LEVEL GetFeatureLevel() const { return FeatureLevel; }

	virtual void* GetInternalDevice() override final { return Direct3DDevice.Get(); }

	virtual void ResizeWindow(std::size_t Width, std::size_t Height) override final;
	virtual void FullScreen(const bool value) override final;
	virtual void Vsync(const bool value) override final;
	virtual void VsyncInterval(const std::uint32_t value) override final;

	virtual bool IsFullScreen() const override final;
	virtual bool IsVsyncEnabled() const override final;
	virtual std::uint32_t GetVsyncInterval() const override final;

	virtual std::vector<sDisplayMode> GetAllSupportedResolutions() const override final;

	virtual EGITypes GetGIType() const override final { return EGITypes::eD3D12; }
	virtual sGPUInfo GetGPUInfo() const override final { return sGPUInfo(); }

	virtual sScreenDimension GetBackBufferDimension() const override final;
	virtual EFormat GetBackBufferFormat() const override final;
	virtual sViewport GetViewport() const override final;

	virtual IGraphicsCommandContext::SharedPtr CreateGraphicsCommandContext() override final;
	virtual IGraphicsCommandContext::UniquePtr CreateUniqueGraphicsCommandContext() override final;

	virtual IComputeCommandContext::SharedPtr CreateComputeCommandContext() override final;
	virtual IComputeCommandContext::UniquePtr CreateUniqueComputeCommandContext() override final;

	virtual ICopyCommandContext::SharedPtr CreateCopyCommandContext() override final;
	virtual ICopyCommandContext::UniquePtr CreateUniqueCopyCommandContext() override final;

	virtual IConstantBuffer::SharedPtr CreateConstantBuffer(std::string InName, const sBufferDesc& InDesc, std::uint32_t InRootParameterIndex) override final;
	virtual IConstantBuffer::UniquePtr CreateUniqueConstantBuffer(std::string InName, const sBufferDesc& InDesc, std::uint32_t InRootParameterIndex) override final;

	virtual IVertexBuffer::SharedPtr CreateVertexBuffer(std::string InName, const sBufferDesc& InDesc, sBufferSubresource* InSubresource = nullptr) override final;
	virtual IVertexBuffer::UniquePtr CreateUniqueVertexBuffer(std::string InName, const sBufferDesc& InDesc, sBufferSubresource* InSubresource = nullptr) override final;

	virtual IIndexBuffer::SharedPtr CreateIndexBuffer(std::string InName, const sBufferDesc& InDesc, sBufferSubresource* InSubresource = nullptr) override final;
	virtual IIndexBuffer::UniquePtr CreateUniqueIndexBuffer(std::string InName, const sBufferDesc& InDesc, sBufferSubresource* InSubresource = nullptr) override final;

	virtual IFrameBuffer::SharedPtr CreateFrameBuffer(const std::string InName, const sFrameBufferAttachmentInfo& InAttachments) override final;
	virtual IFrameBuffer::UniquePtr CreateUniqueFrameBuffer(const std::string InName, const sFrameBufferAttachmentInfo& InAttachments) override final;

	virtual IRenderTarget::SharedPtr CreateRenderTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc) override final;
	virtual IRenderTarget::UniquePtr CreateUniqueRenderTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc) override final;
	virtual IDepthTarget::SharedPtr CreateDepthTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc) override final;
	virtual IDepthTarget::UniquePtr CreateUniqueDepthTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc) override final;
	virtual IUnorderedAccessTarget::SharedPtr CreateUnorderedAccessTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc, bool InEnableSRV) override final;
	virtual IUnorderedAccessTarget::UniquePtr CreateUniqueUnorderedAccessTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc, bool InEnableSRV) override final;

	virtual IPipeline::SharedPtr CreatePipeline(const std::string& InName, const sPipelineDesc& InDesc) override final;
	virtual IPipeline::UniquePtr CreateUniquePipeline(const std::string& InName, const sPipelineDesc& InDesc) override final;

	virtual IComputePipeline::SharedPtr CreateComputePipeline(const std::string& InName, const sComputePipelineDesc& InDesc) override final;
	virtual IComputePipeline::UniquePtr CreateUniqueComputePipeline(const std::string& InName, const sComputePipelineDesc& InDesc) override final;

	virtual ITexture2D::SharedPtr CreateTexture2D(const std::wstring FilePath, const std::string InName, std::uint32_t DefaultRootParameterIndex = 0) override final;
	virtual ITexture2D::UniquePtr CreateUniqueTexture2D(const std::wstring FilePath, const std::string InName, std::uint32_t DefaultRootParameterIndex = 0) override final;
	virtual ITexture2D::SharedPtr CreateTexture2D(const std::string InName, void* InBuffer, const std::size_t InSize, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex = 0) override final;
	virtual ITexture2D::UniquePtr CreateUniqueTexture2D(const std::string InName, void* InBuffer, const std::size_t InSize, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex = 0) override final;

	virtual ITexture2D::SharedPtr CreateEmptyTexture2D(const std::string InName, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex = 0) override final;
	virtual ITexture2D::UniquePtr CreateUniqueEmptyTexture2D(const std::string InName, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex = 0) override final;

	//virtual ITiledTexture::SharedPtr CreateTiledTexture(const std::string InName, const std::uint32_t InTileX, const std::uint32_t InTileY, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex = 0) override final;
	//virtual ITiledTexture::UniquePtr CreateUniqueTiledTexture(const std::string InName, const std::uint32_t InTileX, const std::uint32_t InTileY, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex = 0) override final;

private:
	void PostInit();

private:
	std::optional<short> GPUIndex;
	ComPtr<ID3D12Device> Direct3DDevice;
	ComPtr<IDXGIFactory4> DxgiFactory;
	ComPtr<ID3D12CommandQueue> GraphicsQueue;
	ComPtr<ID3D12CommandQueue> ComputeQueue;
	ComPtr<ID3D12CommandQueue> CopyQueue;

	D3D_FEATURE_LEVEL FeatureLevel;

	std::unique_ptr<D3D12Viewport> Viewport;

	std::unique_ptr<D3D12DescriptorHeapManager> DescriptorHeapManager;

	std::unique_ptr<D3D12CommandBuffer> IMCommandList;
	std::unique_ptr<D3D12CopyCommandBuffer> IMCopyCommandList;

	bool useGPUBasedValidation;

	bool bTypedUAVLoadSupport_R11G11B10_FLOAT;
	bool bTypedUAVLoadSupport_R16G16B16A16_FLOAT;
	bool bEnhancedBarriersSupport;
	D3D12_RENDER_PASS_TIER RenderPassTier;

	std::uint32_t VendorId;

	std::map<D3D12_COMMAND_LIST_TYPE, D3D12Fence*> Fences;

	struct CommandAllocatorPool
	{
		std::vector<ID3D12CommandAllocator*> m_AllocatorPool;
		std::queue<std::pair<uint64_t, ID3D12CommandAllocator*>> m_ReadyAllocators;
		std::mutex m_AllocatorMutex;

		CommandAllocatorPool() = default;
		~CommandAllocatorPool()
		{
			for (size_t i = 0; i < m_AllocatorPool.size(); ++i)
				m_AllocatorPool[i]->Release();

			m_AllocatorPool.clear();
		}
	};

	std::map<D3D12_COMMAND_LIST_TYPE, CommandAllocatorPool> CMDAllocatorPool;

public:
	FORCEINLINE ID3D12Device* Get() const
	{
		return Direct3DDevice.Get();
	}

	FORCEINLINE ID3D12Device* GetDevice() const
	{
		return Direct3DDevice.Get();
	}

	FORCEINLINE ID3D12CommandQueue* GetGraphicsQueue() const
	{
		return GraphicsQueue.Get();
	}

	FORCEINLINE ID3D12CommandQueue* GetCopyQueue() const
	{
		return CopyQueue.Get();
	}

	FORCEINLINE ID3D12CommandQueue* GetComputeQueue() const
	{
		return ComputeQueue.Get();
	}

	FORCEINLINE D3D12CommandBuffer* GetIMCommandList() const
	{
		return IMCommandList.get();
	}

	FORCEINLINE D3D12CopyCommandBuffer* GetIMCopyCommandList() const
	{
		return IMCopyCommandList.get();
	}

	FORCEINLINE bool Is_DXGI_FORMAT_R11G11B10_FLOAT_Supported() const
	{
		return bTypedUAVLoadSupport_R11G11B10_FLOAT;
	}
	FORCEINLINE bool Is_DXGI_FORMAT_R16G16B16A16_FLOAT_Supported() const
	{
		return bTypedUAVLoadSupport_R16G16B16A16_FLOAT;
	}
	FORCEINLINE bool IsEnhancedBarriersSupported() const
	{
		return bEnhancedBarriersSupport;
	}

	FORCEINLINE ComPtr<IDXGIFactory4> GetFactory() const
	{
		return DxgiFactory;
	}

	FORCEINLINE bool IsNvDeviceID() const
	{
		return VendorId == 0x10DE;
	}

	FORCEINLINE bool IsAMDDeviceID() const
	{
		return VendorId == 0x1002;
	}

	FORCEINLINE bool IsIntelDeviceID() const
	{
		return VendorId == 0x8086;
	}

	FORCEINLINE bool IsSoftwareDevice() const
	{
		return VendorId == 0x1414;
	}

private:
	/*FORCEINLINE D3D12Fence* GetFence() const
	{
		return Fence.get();
	}*/

	D3D12Viewport* GetViewportContext() const;

	FORCEINLINE D3D12DescriptorHeapManager* GetDescriptorHeapManager() const
	{
		return DescriptorHeapManager.get();
	}
};
