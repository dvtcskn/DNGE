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

#ifndef Pix3_Enabled
#define Pix3_Enabled 0
#endif
#ifndef AGS_Enable
#define AGS_Enable 1
#endif

#if AGS_Enable
#include "amd_ags.h"
#endif

class D3D12Viewport;
class D3D12Texture;
class D3D12CommandBuffer;
class D3D12CopyCommandBuffer;
class D3D12ComputeCommandContext;
class D3D12DescriptorHeapManager;
class D3D12Fence;
class D3D12ShaderCompiler;
class DXCShaderCompiler;
class D3D12RootSignature;
class D3D12Pipeline;
struct D3D12DescriptorHandle;

class D3D12Device final : public IAbstractGIDevice
{
	sClassBody(sClassConstructor, D3D12Device, IAbstractGIDevice)
public:
	D3D12Device(const GPUCreateInfo& DeviceCreateInfo, std::uint32_t InDeviceIndex);
	virtual ~D3D12Device();
	virtual void InitWindow(void* HWND, std::uint32_t Width, std::uint32_t Height, bool Fullscreen) override final;
	virtual void BeginFrame() override final;
	virtual void RecreateSwapChain() override final;
	virtual void Present(IRenderTarget* pRT) override final;
	bool GetDeviceIdentification(std::wstring& InVendorID, std::wstring& InDeviceID) const;
	IDXGIAdapter* FindAdapter(const WCHAR* InTargetName) const;
	IDXGIAdapter1* FindAndGetAdapter(std::optional<short> Index = std::nullopt) const;

	bool IsCommandBufferPendingForExecute(ICommandContext* CommandContext) const;
	void OnCommandBufferDestroyed(ICommandContext* CommandContext);

	std::uint64_t ExecuteDirectCommandLists(ECommandContextExecuteType ExecuteType, std::uint32_t Order, D3D12CommandBuffer* pCommandList, bool WaitForCompletion = false);
	std::uint64_t ExecuteComputeCommandLists(ECommandContextExecuteType ExecuteType, std::uint32_t Order, D3D12ComputeCommandContext* pCommandList, bool WaitForCompletion = false);
	std::uint64_t ExecuteCopyCommandLists(ECommandContextExecuteType ExecuteType, std::uint32_t Order, D3D12CopyCommandBuffer* pCommandList, bool WaitForCompletion = false);

	std::uint64_t GPUSignal(D3D12_COMMAND_LIST_TYPE Type = D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);
	void WaitForFence(D3D12_COMMAND_LIST_TYPE Type, std::uint64_t FenceValue);
	void CpuWait(D3D12_COMMAND_LIST_TYPE Type);

	ComPtr<ID3D12CommandAllocator> RequestCommandAllocator(D3D12_COMMAND_LIST_TYPE Type);
	void DiscardCommandAllocator(D3D12_COMMAND_LIST_TYPE Type, ComPtr<ID3D12CommandAllocator> Allocator, std::uint64_t FenceValue);

	void GPUFlush(D3D12_COMMAND_LIST_TYPE queueType);

	void SetHeaps(ID3D12GraphicsCommandList* cmd);
	ID3D12DescriptorHeap* GetHeap() const;

	void AllocateDescriptor(D3D12DescriptorHandle* DescriptorHandle);
	void DeallocateDescriptor(D3D12DescriptorHandle* DescriptorHandle);

	inline D3D_FEATURE_LEVEL GetFeatureLevel() const { return FeatureLevel; }

	virtual EGPUDeviceType GetDeviceType() override final { return DeviceType; }
	virtual void* GetInternalDevice() override final { return Direct3DDevice.Get(); }
	virtual void* GetInternalSwapChain() override final;

	virtual void ResizeWindow(std::size_t Width, std::size_t Height) override final;
	virtual void FullScreen(const bool value) override final;
	virtual void Vsync(const bool value) override final;
	virtual void VsyncInterval(const std::uint32_t value) override final;
	virtual std::vector<sDisplayDesc> GetAllSupportedResolutions() const override final;
	virtual DisplayMode GetDisplayMode() const override final;

	virtual void GPUFlush();
	virtual void WaitForGPU() override final;
	virtual void WaitForCPU() override final;
	virtual void WaitForCPUFence(std::uint64_t Value) override final;
	virtual std::uint64_t GPUFenceSignal() override final;

	virtual bool IsFullScreen() const override final;
	virtual bool IsVsyncEnabled() const override final;
	virtual std::uint32_t GetVsyncInterval() const override final;

	virtual EGITypes GetGIType() const override final { return EGITypes::D3D12; }
	virtual sGPUInfo GetGPUInfo() const override final { return sGPUInfo(); }

	virtual sScreenDimension GetBackBufferDimension() const override final;
	virtual EFormat GetBackBufferFormat() const override final;
	virtual sViewport GetViewport() const override final;

	virtual std::uint32_t GetBackBufferSize() const override final;
	virtual std::uint32_t GetCurrentBackBufferIndex() const override final;

	std::shared_ptr<D3D12RootSignature> CreateRootSignature(const std::vector<sShaderBinding>& Bindings, const sIndirectLayoutBindingDesc& IndirectDesc);

	IShader* CompileD3D12Shader(const sShaderAttachment& Attachment, bool Spirv = false);
	IShader* CompileD3D12Shader(std::wstring InSrcFile, std::string InFunctionName, eShaderType InProfile, bool Spirv = false, std::vector<sShaderDefines> InDefines = std::vector<sShaderDefines>());
	IShader* CompileD3D12Shader(const void* InCode, std::size_t Size, std::string InFunctionName, eShaderType InProfile, bool Spirv = false, std::vector<sShaderDefines> InDefines = std::vector<sShaderDefines>());

	virtual IShader::SharedPtr CompileShader(const sShaderAttachment& Attachment) override final;
	virtual IShader::SharedPtr CompileShader(std::wstring InSrcFile, std::string InFunctionName, eShaderType InProfile, std::vector<sShaderDefines> InDefines = std::vector<sShaderDefines>()) override final;
	virtual IShader::SharedPtr CompileShader(const void* InCode, std::size_t Size, std::string InFunctionName, eShaderType InProfile, std::vector<sShaderDefines> InDefines = std::vector<sShaderDefines>()) override final;

	virtual IGraphicsCommandContext::SharedPtr CreateGraphicsCommandContext() override final;
	virtual IGraphicsCommandContext::UniquePtr CreateUniqueGraphicsCommandContext() override final;

	virtual IComputeCommandContext::SharedPtr CreateComputeCommandContext() override final;
	virtual IComputeCommandContext::UniquePtr CreateUniqueComputeCommandContext() override final;

	virtual ICopyCommandContext::SharedPtr CreateCopyCommandContext() override final;
	virtual ICopyCommandContext::UniquePtr CreateUniqueCopyCommandContext() override final;

	virtual IConstantBuffer::SharedPtr CreateConstantBuffer(std::string InName, const BufferLayout& InDesc, std::uint32_t InRootParameterIndex) override final;
	virtual IConstantBuffer::UniquePtr CreateUniqueConstantBuffer(std::string InName, const BufferLayout& InDesc, std::uint32_t InRootParameterIndex) override final;

	virtual IVertexBuffer::SharedPtr CreateVertexBuffer(std::string InName, const BufferLayout& InDesc, BufferSubresource* InSubresource = nullptr) override final;
	virtual IVertexBuffer::UniquePtr CreateUniqueVertexBuffer(std::string InName, const BufferLayout& InDesc, BufferSubresource* InSubresource = nullptr) override final;

	virtual IIndexBuffer::SharedPtr CreateIndexBuffer(std::string InName, const BufferLayout& InDesc, BufferSubresource* InSubresource = nullptr) override final;
	virtual IIndexBuffer::UniquePtr CreateUniqueIndexBuffer(std::string InName, const BufferLayout& InDesc, BufferSubresource* InSubresource = nullptr) override final;

	virtual IByteAddressBuffer::SharedPtr CreateByteAddressBuffer(std::string InName, std::uint64_t Size, bool bReadWriteAllowed) override final;
	virtual IByteAddressBuffer::UniquePtr CreateUniqueByteAddressBuffer(std::string InName, std::uint64_t Size, bool bReadWriteAllowed) override final;

	virtual IStructuredBuffer::SharedPtr CreateStructuredBuffer(std::string InName, const BufferLayout& InDesc, bool bSRVAllowed = true) override final;
	virtual IStructuredBuffer::UniquePtr CreateUniqueStructuredBuffer(std::string InName, const BufferLayout& InDesc, bool bSRVAllowed = true) override final;

	virtual IIndirectBuffer::SharedPtr CreateIndirectBuffer(std::string InName, BufferLayout NewLayout) override final;
	virtual IIndirectBuffer::UniquePtr CreateUniqueIndirectBuffer(std::string InName, BufferLayout NewLayout) override final;

	virtual IFrameBuffer::SharedPtr CreateFrameBuffer(const std::string InName, const sFrameBufferAttachmentInfo& InAttachments) override final;
	virtual IFrameBuffer::UniquePtr CreateUniqueFrameBuffer(const std::string InName, const sFrameBufferAttachmentInfo& InAttachments) override final;

	virtual ISamplerState::SharedPtr CreateSamplerState(const std::string InName, const sSamplerAttributeDesc& InDesc) override final;
	virtual ISamplerState::UniquePtr CreateUniqueSamplerState(const std::string InName, const sSamplerAttributeDesc& InDesc) override final;

	virtual IRenderTarget::SharedPtr CreateRenderTarget(const std::string InName, const sFrameBuffer& InDesc, const sFBODesc& FBODesc) override final;
	virtual IRenderTarget::UniquePtr CreateUniqueRenderTarget(const std::string InName, const sFrameBuffer& InDesc, const sFBODesc& FBODesc) override final;
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
	std::optional<std::uint32_t> GPUIndex;
	EGPUDeviceType DeviceType;
	std::uint32_t DeviceIndex;

	ComPtr<ID3D12Device> Direct3DDevice;
	ComPtr<IDXGIFactory7> DxgiFactory;
	ComPtr<IDXGIAdapter1> Adapter;
	ComPtr<ID3D12CommandQueue> GraphicsQueue;
	ComPtr<ID3D12CommandQueue> ComputeQueue;
	ComPtr<ID3D12CommandQueue> CopyQueue;

	ComPtr<ID3D12Device14> Direct3DDevice14;

	struct PendingSubmission
	{
		ICommandContext* CommandBuffer = nullptr;
		ComPtr<ID3D12CommandList> CommandList;
		D3D12_COMMAND_LIST_TYPE Type;
		ComPtr<ID3D12CommandAllocator> Allocator;
	};
	std::map<D3D12_COMMAND_LIST_TYPE, std::vector<PendingSubmission>> DeferredCommandLists;
	std::mutex Mutex;

	D3D_FEATURE_LEVEL FeatureLevel;

	std::unique_ptr<D3D12Viewport> Viewport;

	std::unique_ptr<D3D12DescriptorHeapManager> DescriptorHeapManager;

	std::unique_ptr<D3D12CommandBuffer> IMCommandList;
	std::unique_ptr<D3D12CopyCommandBuffer> IMCopyCommandList;

	bool useGPUBasedValidation;

	bool bTypedUAVLoadSupport_R11G11B10_FLOAT;
	bool bTypedUAVLoadSupport_R16G16B16A16_FLOAT;

	bool bIsEnhancedBarriersSupported;

	std::uint32_t VendorId;

	std::map<D3D12_COMMAND_LIST_TYPE, D3D12Fence*> Fences;

	std::unique_ptr<DXCShaderCompiler> ShaderCompiler;
	//std::unique_ptr<D3D12ShaderCompiler> pD3D12ShaderCompiler;

	std::map<std::size_t, ISamplerState::SharedPtr> SamplerCache;
	std::map<std::size_t, std::shared_ptr<D3D12RootSignature>> RootSignatureCache;
	//std::map<std::string, std::shared_ptr<D3D12Pipeline>> PipelineCache;

#if AGS_Enable
	AGSContext*					m_agsContext = nullptr;
	AGSGPUInfo                  m_agsGPUInfo = {};
	AGSDX12ExtensionsSupported	m_agsDeviceExtensions = {};
public:
	FORCEINLINE AGSContext* GetAGSContext() const
	{
		return m_agsContext;
	}
	FORCEINLINE AGSGPUInfo GetAGSGpuInfo() const
	{
		return m_agsGPUInfo;
	}
#endif

public:
	FORCEINLINE ID3D12Device* Get() const
	{
		return Direct3DDevice.Get();
	}
	FORCEINLINE ID3D12Device14* GetDevice() const
	{
		return Direct3DDevice14.Get();
	}
	FORCEINLINE IDXGIAdapter1* GetAdapter() const
	{
		return Adapter.Get();
	}

	FORCEINLINE std::wstring GetDeviceName() const
	{
		if (Adapter)
		{
			DXGI_ADAPTER_DESC1 AdapterDesc;
			Adapter->GetDesc1(&AdapterDesc);
			return AdapterDesc.Description;
		}
		return std::wstring();
	}

	FORCEINLINE D3D12Viewport* GetD3D12Viewport() const
	{
		return Viewport.get();
	}

	FORCEINLINE bool IsPrimaryGPU() const
	{
		return DeviceIndex == 0;
	}

	FORCEINLINE std::uint32_t GetDeviceIndex() const
	{
		return DeviceIndex;
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

	FORCEINLINE bool IsEnhancedBarriersSupported() const
	{
		return false; //bIsEnhancedBarriersSupported;
	}

	FORCEINLINE bool Is_DXGI_FORMAT_R11G11B10_FLOAT_Supported() const
	{
		return bTypedUAVLoadSupport_R11G11B10_FLOAT;
	}
	FORCEINLINE bool Is_DXGI_FORMAT_R16G16B16A16_FLOAT_Supported() const
	{
		return bTypedUAVLoadSupport_R16G16B16A16_FLOAT;
	}

	FORCEINLINE ComPtr<IDXGIFactory7> GetFactory() const
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

	FORCEINLINE D3D12DescriptorHeapManager* GetDescriptorHeapManager() const
	{
		return DescriptorHeapManager.get();
	}

private:
	class D3D12CommandAllocatorPool;
	class D3D12CommandAllocatorManager;
	std::unique_ptr<D3D12CommandAllocatorManager> CommandAllocatorPool;
};
