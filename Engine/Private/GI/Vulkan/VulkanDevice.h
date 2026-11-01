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
#include <string>
#include "GI/AbstractGI/AbstractGIDevice.h"

#include "VulkanFence.h"

#ifndef WIN32
#define WIN32
#endif // !WIN32

#ifndef VK_HPP
#define VK_HPP 1
#endif // !VK_HPP

#ifndef VK_RAII_HPP
#define VK_RAII_HPP 0
#endif // !VK_RAII_HPP

#if VK_HPP && !VK_RAII_HPP
	#include <vulkan/vulkan.hpp>
#elif VK_RAII_HPP
	#include <vulkan/vulkan_raii.hpp>
#else
	#include <vulkan/vulkan.h>
#endif // !VK_HPP

class VulkanViewport;
class VulkanShaderCompiler;
class DXCShaderCompiler;
class VulkanMemoryManager;
class VulkanCommandBuffer;
class VulkanBindlessDescriptorPool;
class VulkanDescriptorSet;
class VulkanPushDescriptorPool;

class VulkanDevice final : public IAbstractGIDevice
{
	sClassBody(sClassConstructor, VulkanDevice, IAbstractGIDevice)
private:
	struct QueueFamilyProperties
	{
		std::int32_t graphicsQueueFamilyIndex = -1;
		std::int32_t computeQueueFamilyIndex = -1;
		std::int32_t copyQueueFamilyIndex = -1;

		// frame interpolation
		std::int32_t ImageAcquireQueueFamilyIndex = -1;
		std::int32_t presentQueueFamilyIndex = -1;
		std::int32_t AsyncComputeQueueFamilyIndex = -1;
	};

public:
	VulkanDevice(const GPUCreateInfo& DeviceCreateInfo, std::optional<ViewportCreateInfo> WindowCreateInfo = std::nullopt);
	virtual ~VulkanDevice();
	void InitWindow(ViewportCreateInfo CreateInfo);

	virtual void BeginFrame() override final;
	virtual void RecreateSwapChain() override final;
	virtual void Present(IRenderTarget* pRT) override final;
	bool GetDeviceIdentification(std::wstring& InVendorID, std::wstring& InDeviceID);

	std::uint64_t ExecuteGraphicsCommandBuffer(ECommandContextExecuteType ExecuteType, std::uint32_t Order, VulkanCommandBuffer* CommandList, bool WaitForCompletion = false, VkSemaphore WaitSemaphore = VK_NULL_HANDLE, VkPipelineStageFlags WaitStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
	void ExecuteComputeCommandBuffer(VkCommandBuffer pCommandList, bool WaitForCompletion = false);
	void ExecuteCopyCommandBuffer(VkCommandBuffer pCommandList, bool WaitForCompletion = false);

	std::uint64_t GPUSignal(ECommandContextType CommandContextType);

	//void ResetCommandBuffer(VkCommandBuffer CommandBuffer);

	bool IsCommandBufferPendingForExecute(VulkanCommandBuffer* CommandBuffer);
	void CancelPendingCommandBuffer(VulkanCommandBuffer* CommandBuffer);

	VkDescriptorSetLayout GetBindlessDescriptorSetLayout() const;
	VkDescriptorSet GetBindlessDescriptorSet() const;
	void UpdateBindlessImageDescriptor(uint32_t& HeapIndex, VkDescriptorImageInfo* imageInfo);
	void UpdateBindlessStorageImageDescriptor(uint32_t& HeapIndex, VkDescriptorImageInfo* imageInfo);
	void UpdateBindlessSamplerDescriptor(uint32_t& HeapIndex, VkDescriptorImageInfo* imageInfo);
	void UpdateBindlessBufferDescriptor(uint32_t& HeapIndex, VkDescriptorBufferInfo* bufferInfo);

	VkSemaphore AcquireSignalledSemaphore();
	VkSemaphore AcquireSemaphore();

	VkCommandBuffer RequestCommandBuffer(ECommandContextType Type);
	void ReturnCommandBuffer(ECommandContextType Type, VkCommandBuffer CommandBuffer, std::uint64_t FenceValue = 0);

	//VkCommandBuffer RequestCommandBuffer();
	//void ReturnCommandBuffer(VkCommandBuffer commandBuffer);
	//bool IsCommandBufferAvailable(VkCommandBuffer commandBuffer) const;

	bool GetAdapter(std::optional<std::uint32_t> Index, VkPhysicalDevice& GPU, QueueFamilyProperties& FamilyProperties, VkPhysicalDeviceProperties& properties) const;

	virtual EGPUDeviceType GetDeviceType() override final {	return EGPUDeviceType::Hardware; }
	virtual void* GetInternalDevice() override final { return Device; }
	virtual void* GetInternalSwapChain() override final;

	const VkInstance& GetInstance() const { return instance; }

	const VkDevice& Get() const { return Device; }
#ifdef VK_HPP
	const vk::Device& Get_Hpp() const { return Device; }
#endif
	const VkQueue& GetQueue() const { return GraphicsQueue; }
	const VkPhysicalDevice& GetPhysicalDevice() const { return PhysicalDevice; }
	std::int32_t GraphicsQueueIndex() const { return FamilyProperties.graphicsQueueFamilyIndex; }

	uint32_t GetMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties);

	virtual std::uint64_t GetFrameIndex() const override final { return 0; }
	virtual void ResizeWindow(std::size_t Width, std::size_t Height) override final;
	virtual void FullScreen(const bool value) override final;
	virtual void Vsync(const bool value) override final;
	virtual void VsyncInterval(const std::uint32_t value) override final;
	virtual DisplayMode GetDisplayMode() const override final;
	virtual HDRMetadata GetHDRMetaData() const override final;

	virtual bool IsFullScreen() const override final;
	virtual bool IsVsyncEnabled() const override final;
	virtual std::uint32_t GetVsyncInterval() const override final;

	virtual void GPUFlush();
	virtual void WaitForGPU() override final;
	virtual void WaitForCPU() override final;
	virtual void WaitForCPUFence(std::uint64_t Value) override final;
	virtual std::uint64_t GPUFenceSignal() override final;

	virtual std::vector<sDisplayDesc> GetAllSupportedResolutions() const override final;

	virtual EGITypes GetGIType() const override final { return EGITypes::Vulkan; }
	virtual sGPUInfo GetGPUInfo() const override final { return sGPUInfo(); }

	virtual sScreenDimension GetBackBufferDimension() const override final;
	virtual EFormat GetBackBufferFormat() const override final;
	virtual sViewport GetViewport() const override final;

	virtual std::uint32_t GetBackBufferSize() const override final;
	virtual std::uint32_t GetCurrentBackBufferIndex() const override final;

	void SetPerfMarkerBegin(VkCommandBuffer cmd_buf, const char* name);
	void SetPerfMarkerEnd(VkCommandBuffer cmd_buf);

	void SetResourceName(VkObjectType objectType, uint64_t handle, const char* name);

	VkDeviceMemory AllocateMemory(/*std::string ClassID,*/ VkDeviceSize size, uint32_t memoryTypeIndex);
	VkDeviceMemory AllocateMemory(/*std::string ClassID,*/ VkDeviceSize size, uint32_t type_filter, VkMemoryPropertyFlags properties);
	void FreeMemory(VkDeviceMemory memory);

	IShader* CompileVkShader(const sShaderAttachment& Attachment, bool Spirv = false);
	IShader* CompileVkShader(std::wstring InSrcFile, std::string InFunctionName, eShaderType InProfile, bool Spirv = false, std::vector<sShaderDefines> InDefines = std::vector<sShaderDefines>());
	IShader* CompileVkShader(const void* InCode, std::size_t Size, std::string InFunctionName, eShaderType InProfile, bool Spirv = false, std::vector<sShaderDefines> InDefines = std::vector<sShaderDefines>());

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

public:
	FORCEINLINE VulkanViewport* GetVulkanViewport() const
	{
		return Viewport.get();
	}

	FORCEINLINE bool IsNvDeviceID() const
	{
		return DeviceProperties.vendorID == 0x10DE;
	}

	FORCEINLINE bool IsAMDDeviceID() const
	{
		return DeviceProperties.vendorID == 0x1002;
	}

	FORCEINLINE bool IsIntelDeviceID() const
	{
		return DeviceProperties.vendorID == 0x8086;
	}

	FORCEINLINE bool IsSoftwareDevice() const
	{
		return DeviceProperties.vendorID == 0x1414;
	}

	FORCEINLINE VulkanCommandBuffer* GetIMCommandBuffer() const
	{
		return IMCommandBuffer.get();
	}

	FORCEINLINE bool IsPrimaryGPU() const
	{
		return bIsPrimary;
	}

	FORCEINLINE std::uint32_t GetGPUAdapterIndex() const
	{
		return GPUAdapterIndex.has_value() ? GPUAdapterIndex.value() : std::uint32_t(-1);
	}

	FORCEINLINE std::string GetDeviceName() const
	{
		return DeviceName;
	}

	PFN_vkCreateIndirectCommandsLayoutEXT GetIndirectCommandsCreateLayout()
	{
		return IndirectCommandsCreateLayout;
	}
	PFN_vkDestroyIndirectCommandsLayoutEXT GetIndirectCommandsDestroyLayout()
	{
		return IndirectCommandsDestroyLayout;
	}
	PFN_vkGetGeneratedCommandsMemoryRequirementsEXT GetGeneratedCommandsMemoryRequirements()
	{
		return GeneratedCommandsMemoryRequirements;
	}
	PFN_vkCmdExecuteGeneratedCommandsEXT GetCmdExecuteGeneratedCommands()
	{
		return CmdExecuteGeneratedCommands;
	}

	VkQueue GetComputeQueue() const 
	{
		return ComputeQueue;
	}
	std::int32_t GetComputeQueueIndex() const
	{
		return FamilyProperties.computeQueueFamilyIndex;
	}

	VkQueue GetCopyQueue() const
	{
		return CopyQueue;
	}
	std::int32_t GetCopyQueueIndex() const
	{
		return FamilyProperties.copyQueueFamilyIndex;
	}

	VkQueue GetPresentQueue() const 
	{
		return PresentQueue;
	}
	std::int32_t GetPresentQueueIndex() const
	{
		return FamilyProperties.presentQueueFamilyIndex;
	}

	VkQueue GetImageAcquireQueue() const 
	{
		return ImageAcquireQueue;
	}
	std::int32_t GetImageAcquireQueueIndex() const
	{
		return FamilyProperties.ImageAcquireQueueFamilyIndex;
	}
	VkQueue GetAsyncComputeQueue() const
	{
		return AsyncComputeQueue;
	}
	std::int32_t GetAsyncComputeQueueIndex() const
	{
		return FamilyProperties.AsyncComputeQueueFamilyIndex;
	}


private:
	std::string DeviceName;
	std::uint32_t InstanceApiVersion;
	VkInstance instance = VK_NULL_HANDLE;
	VkDevice Device;
	VkPhysicalDevice PhysicalDevice;
	VkPhysicalDeviceProperties DeviceProperties;
	VkPhysicalDeviceType PhysicalDevice_Type;
	VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties;
	VkPhysicalDeviceLimits deviceLimits;
	VkDebugUtilsMessengerEXT debug_callback = VK_NULL_HANDLE;

	EGPUDeviceType DeviceType;
	std::optional<std::uint32_t> GPUAdapterIndex;
	bool bIsPrimary;

	std::unique_ptr<VulkanCommandBuffer> IMCommandBuffer;

	VkQueue GraphicsQueue;
	VkQueue ComputeQueue;
	VkQueue CopyQueue;

	// frame interpolation
	VkQueue PresentQueue;
	VkQueue ImageAcquireQueue;
	VkQueue AsyncComputeQueue;

	std::map<std::uint64_t, std::unique_ptr<VulkanFence>> SubmissionFences;
	std::uint64_t NextFenceValue = 0;
	std::uint64_t CompletedFenceValue = 0;

	struct PendingSubmission
	{
		ICommandContext* CommandBuffer = nullptr;
		VkCommandBuffer CommandList;
		ECommandContextType Type;
	};
	struct CommandPoolEntry
	{
		ECommandContextType Type;
		VkCommandPool Pool;
	};
	struct CommandBufferEntry
	{
		ECommandContextType Type;
		VkCommandPool Pool;
		VkCommandBuffer Buffer;
		std::uint64_t FenceValue = 0;
		bool IsLeased = false;
	};
	std::map<ECommandContextType, std::vector<PendingSubmission>> DeferredCommandLists;
	std::vector<CommandPoolEntry> CommandPools;
	std::vector<CommandBufferEntry> CommandBuffers;
	std::mutex Mutex;

	std::uint64_t SubmitCommandBuffers(VkQueue Queue, const std::vector<VkCommandBuffer>& CommandBuffers, bool WaitForCompletion, VkSemaphore WaitSemaphore = VK_NULL_HANDLE, VkPipelineStageFlags WaitStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
	void CollectCompletedFences();

	bool deviceProperties2Available = false;
	bool deviceFormatProperties2Available = false;

	std::map<std::size_t, ISamplerState::SharedPtr> SamplerCache;

	PFN_vkCreateIndirectCommandsLayoutEXT           IndirectCommandsCreateLayout = nullptr;
	PFN_vkDestroyIndirectCommandsLayoutEXT          IndirectCommandsDestroyLayout = nullptr;
	PFN_vkGetGeneratedCommandsMemoryRequirementsEXT GeneratedCommandsMemoryRequirements = nullptr;
	PFN_vkCmdExecuteGeneratedCommandsEXT            CmdExecuteGeneratedCommands = nullptr;

private:
	QueueFamilyProperties FamilyProperties;
	std::unique_ptr<VulkanViewport> Viewport;
	std::uint64_t uniform_buffer_alignment;

	std::unique_ptr<DXCShaderCompiler> ShaderCompiler;
	//std::unique_ptr<VulkanShaderCompiler> pShaderCompiler;

	std::unique_ptr<VulkanMemoryManager> MemoryManager;
	
	class VkCommandPoolManager;
	std::unique_ptr<VkCommandPoolManager> CommandPoolManager;
	class VkCommandBufferManager;
	std::unique_ptr<VkCommandBufferManager> CommandBufferManager;

	class VulkanSyncManager;
	std::unique_ptr<VulkanSyncManager> SyncManager;

	std::unique_ptr<VulkanBindlessDescriptorPool> BindlessDescriptorPool;
};
