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
		std::int32_t presentQueueFamilyIndex = -1;
		std::int32_t computeQueueFamilyIndex = -1;
		std::int32_t copyQueueFamilyIndex = -1;
	};

public:
	VulkanDevice(const GPUDeviceCreateInfo& DeviceCreateInfo);
	virtual ~VulkanDevice();
	virtual void InitWindow(void* HWND, std::uint32_t Width, std::uint32_t Height, bool Fullscreen) override final;
	virtual void BeginFrame() override final;
	virtual void Present(IRenderTarget* pRT) override final;
	bool GetDeviceIdentification(std::wstring& InVendorID, std::wstring& InDeviceID);

	void ExecuteGraphicsCommandBuffer(VkCommandBuffer pCommandList, bool WaitForCompletion = false);
#ifdef VK_HPP
	void ExecuteGraphicsCommandBuffer(vk::CommandBuffer* pCommandList, bool WaitForCompletion = false);
#endif
	void ExecuteComputeCommandBuffer(VkCommandBuffer pCommandList, bool WaitForCompletion = false);
	void ExecuteCopyCommandBuffer(VkCommandBuffer pCommandList, bool WaitForCompletion = false);

	void ResetCommandBuffer(VkCommandBuffer CommandBuffer);

	VkDescriptorSetLayout GetPushDescriptorSetLayout() const;
	VkDescriptorSetLayout GetBindlessDescriptorSetLayout() const;
	VkDescriptorSet GetBindlessDescriptorSet() const;
	void UpdateBindlessImageDescriptor(uint32_t binding, uint32_t arrayIndex, VkDescriptorImageInfo* imageInfo);
	void UpdateBindlessBufferDescriptor(uint32_t binding, uint32_t arrayIndex, VkDescriptorBufferInfo* bufferInfo);

	VkSemaphore AcquireSignalledSemaphore();
	VkSemaphore AcquireSemaphore();
	VkFence AcquireFence();
	void RecycleFence(VkFence Fence);

	VkCommandBuffer RequestCommandBuffer();
	void ReturnCommandBuffer(VkCommandBuffer commandBuffer);
	bool IsCommandBufferAvailable(VkCommandBuffer commandBuffer) const;

	bool GetAdapter(std::optional<short> Index, VkPhysicalDevice& GPU, QueueFamilyProperties& FamilyProperties, VkPhysicalDeviceProperties& properties) const;

	virtual void* GetInternalDevice() override final { return nullptr; }

	const VkInstance& GetInstance() const { return instance; }

	const VkDevice& Get() const { return Device; }
#ifdef VK_HPP
	const vk::Device& Get_Hpp() const { return Device; }
#endif
	const VkQueue& GetQueue() const { return GraphicsQueue; }
	const VkPhysicalDevice& GetPhysicalDevice() const { return PhysicalDevice; }
	std::int32_t GraphicsQueueIndex() const { return FamilyProperties.graphicsQueueFamilyIndex; }

	uint32_t GetMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties);

	virtual void ResizeWindow(std::size_t Width, std::size_t Height) override final;
	virtual void FullScreen(const bool value) override final;
	virtual void Vsync(const bool value) override final;
	virtual void VsyncInterval(const std::uint32_t value) override final;

	virtual bool IsFullScreen() const override final;
	virtual bool IsVsyncEnabled() const override final;
	virtual std::uint32_t GetVsyncInterval() const override final;

	virtual std::vector<sDisplayMode> GetAllSupportedResolutions() const override final;

	virtual EGITypes GetGIType() const override final { return EGITypes::eVulkan; }
	virtual sGPUInfo GetGPUInfo() const override final { return sGPUInfo(); }

	virtual sScreenDimension GetBackBufferDimension() const override final;
	virtual EFormat GetBackBufferFormat() const override final;
	virtual sViewport GetViewport() const override final;

	void SetPerfMarkerBegin(VkCommandBuffer cmd_buf, const char* name);
	void SetPerfMarkerEnd(VkCommandBuffer cmd_buf);

	void SetResourceName(VkObjectType objectType, uint64_t handle, const char* name);

	VkDeviceMemory AllocateMemory(/*std::string ClassID,*/ VkDeviceSize size, uint32_t memoryTypeIndex);
	VkDeviceMemory AllocateMemory(/*std::string ClassID,*/ VkDeviceSize size, uint32_t type_filter, VkMemoryPropertyFlags properties);
	void FreeMemory(VkDeviceMemory memory);

	virtual IShader* CompileShader(const sShaderAttachment& Attachment, bool Spirv = false) override final;
	virtual IShader* CompileShader(std::wstring InSrcFile, std::string InFunctionName, eShaderType InProfile, bool Spirv = false, std::vector<sShaderDefines> InDefines = std::vector<sShaderDefines>()) override final;
	virtual IShader* CompileShader(const void* InCode, std::size_t Size, std::string InFunctionName, eShaderType InProfile, bool Spirv = false, std::vector<sShaderDefines> InDefines = std::vector<sShaderDefines>()) override final;

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

public:
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

private:
	VkInstance instance = VK_NULL_HANDLE;
	VkDevice Device;
	VkPhysicalDevice PhysicalDevice;
	VkPhysicalDeviceProperties DeviceProperties;
	VkPhysicalDeviceType PhysicalDevice_Type;
	VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties;
	VkPhysicalDeviceLimits deviceLimits;
	VkDebugUtilsMessengerEXT debug_callback = VK_NULL_HANDLE;

	std::unique_ptr<VulkanCommandBuffer> IMCommandBuffer;

	VkQueue GraphicsQueue;
	VkQueue ComputeQueue;

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
	std::unique_ptr<VulkanPushDescriptorPool> PushDescriptorPool;
};
