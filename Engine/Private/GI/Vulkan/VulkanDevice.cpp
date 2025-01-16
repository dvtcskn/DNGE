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
#include "VulkanDevice.h"
#include "Engine/AbstractEngine.h"
#include "Utilities/FileManager.h"
#include <queue>

//#define VK_NO_PROTOTYPES
//#include "volk.h"

#include "VulkanException.h"
#include "VulkanBuffer.h"
#include "VulkanCommandBuffer.h"
#include "VulkanComputeCommandContext.h"
#include "VulkanFrameBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanShaderStates.h"
#include "VulkanTexture.h"
#include "VulkanViewport.h"
//#include "VulkanShader.h"
#include "GI/Shared/ShaderCompiler.h"

#pragma comment(lib, "vulkan-1.lib")

/*
Prefers `VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT`.

VMA_MEMORY_USAGE_GPU_ONLY = 1,

Guarantees `VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT` and `VK_MEMORY_PROPERTY_HOST_COHERENT_BIT`.

VMA_MEMORY_USAGE_CPU_ONLY = 2,

Guarantees `VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT`, prefers `VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT`.

VMA_MEMORY_USAGE_CPU_TO_GPU = 3,

Guarantees `VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT`, prefers `VK_MEMORY_PROPERTY_HOST_CACHED_BIT`.

VMA_MEMORY_USAGE_GPU_TO_CPU = 4,

Prefers not `VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT`.

VMA_MEMORY_USAGE_CPU_COPY = 5,
*/

/*
* VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
* bit specifies that memory allocated with this type is the most efficient for device access. This property will be set if and only if the memory type belongs to a heap with the VK_MEMORY_HEAP_DEVICE_LOCAL_BIT set.
*
* VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
* Mapable/CPU
*
* VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
* bit specifies that the host cache management commands vkFlushMappedMemoryRanges and vkInvalidateMappedMemoryRanges are not needed to manage availability and visibility on the host.
*
* VK_MEMORY_PROPERTY_HOST_CACHED_BIT
* bit specifies that memory allocated with this type is cached on the host. Host memory accesses to uncached memory are slower than to cached memory, however uncached memory is always host coherent.
*
* VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD
* bit specifies that device accesses to allocations of this memory type are automatically made available and visible on the device. If paired with VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, memory domain operations are also performed automatically between host and device.
*
* VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD
* bit specifies that memory allocated with this type is not cached on the device. Uncached device memory is always device coherent.
*
* VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV
* bit specifies that external devices can access this memory directly.
*/

/*
* VK_BUFFER_USAGE_TRANSFER_SRC_BIT
* specifies that the buffer can be used as the source of a transfer command (see the definition of VK_PIPELINE_STAGE_TRANSFER_BIT).
*
* VK_BUFFER_USAGE_TRANSFER_DST_BIT
* specifies that the buffer can be used as the destination of a transfer command.
*
* VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT
* specifies that the buffer can be used to create a VkBufferView suitable for occupying a VkDescriptorSet slot of type VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER.
*
* VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT
* specifies that the buffer can be used to create a VkBufferView suitable for occupying a VkDescriptorSet slot of type VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER.
*
* VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
* specifies that the buffer can be used in a VkDescriptorBufferInfo suitable for occupying a VkDescriptorSet slot either of type VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER or VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC.
*
* VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
* specifies that the buffer can be used in a VkDescriptorBufferInfo suitable for occupying a VkDescriptorSet slot either of type VK_DESCRIPTOR_TYPE_STORAGE_BUFFER or VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC.
*
* VK_BUFFER_USAGE_INDEX_BUFFER_BIT
* specifies that the buffer is suitable for passing as the buffer parameter to vkCmdBindIndexBuffer2 and vkCmdBindIndexBuffer.
*
* VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
* specifies that the buffer is suitable for passing as an element of the pBuffers array to vkCmdBindVertexBuffers.
*
* VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT
* specifies that the buffer is suitable for passing as the buffer parameter to vkCmdDrawIndirect, vkCmdDrawIndexedIndirect, vkCmdDrawMeshTasksIndirectNV, vkCmdDrawMeshTasksIndirectCountNV, vkCmdDrawMeshTasksIndirectEXT, vkCmdDrawMeshTasksIndirectCountEXT, vkCmdDrawClusterIndirectHUAWEI, or vkCmdDispatchIndirect. It is also suitable for passing as the buffer member of VkIndirectCommandsStreamNV, or sequencesCountBuffer or sequencesIndexBuffer or preprocessedBuffer member of VkGeneratedCommandsInfoNV. It is also suitable for passing as the underlying buffer of either the preprocessAddress or sequenceCountAddress members of VkGeneratedCommandsInfoEXT.
*
* VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT
* specifies that the buffer is suitable to contain sampler and combined image sampler descriptors when bound as a descriptor buffer. Buffers containing combined image sampler descriptors must also specify VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT.
*
* VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT
* specifies that the buffer is suitable to contain resource descriptors when bound as a descriptor buffer.
*
*/

class VulkanDevice::VulkanSyncManager
{
	sBaseClassBody(sClassConstructor, VulkanDevice::VulkanSyncManager)
public:
	VulkanSyncManager(VulkanDevice* device)
		: Device(device)
		, SemaphoreCounter(-1)
	{
		VkSemaphoreTypeCreateInfo SemaphoreTypeCreateInfo = {};
		SemaphoreTypeCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
		SemaphoreTypeCreateInfo.pNext = VK_NULL_HANDLE;
		SemaphoreTypeCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_BINARY; // VK_SEMAPHORE_TYPE_TIMELINE;
		SemaphoreTypeCreateInfo.initialValue = 0;
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreInfo.pNext = &SemaphoreTypeCreateInfo;
		semaphoreInfo.flags = 0;
		
		{
			VkSemaphore semaphore;
			VkResult result = vkCreateSemaphore(Device->Get(), &semaphoreInfo, nullptr, &semaphore);
			SemaphorePool.push_back(semaphore);
		}
		{
			VkSemaphore semaphore;
			VkResult result = vkCreateSemaphore(Device->Get(), &semaphoreInfo, nullptr, &semaphore);
			SemaphorePool.push_back(semaphore);
		}
		{
			VkSemaphore semaphore;
			VkResult result = vkCreateSemaphore(Device->Get(), &semaphoreInfo, nullptr, &semaphore);
			SemaphorePool.push_back(semaphore);
		}
	}

	~VulkanSyncManager() 
	{
		for (auto semaphore : SemaphorePool)
			vkDestroySemaphore(Device->Get(), semaphore, nullptr);
		SemaphorePool.clear();

		WaitForPastFence();

		for (auto& fence : FencePool)
			vkDestroyFence(Device->Get(), fence, nullptr);
	}

	VkSemaphore AcquireSemaphore() const
	{
		SemaphoreCounter++;
		if (SemaphoreCounter == SemaphorePool.size())
			SemaphoreCounter = 0;
		return SemaphorePool[SemaphoreCounter];
	}

	VkSemaphore AcquireSignalledSemaphore() const
	{
		return SemaphorePool[SemaphoreCounter];
	}

	std::size_t AvailableSemaphoresSize() const
	{
		return SemaphorePool.size();
	}

	bool IsSignalledSemaphoresAvailable() const
	{
		return SemaphoreCounter >= 0;
	}

	VkFence AcquireFence()
	{
		std::lock_guard<std::mutex> Lock(Mutex);
		if (!FencePool.empty())
		{
			VkFence fence = FencePool.back();
			FencePool.pop_back();
			return fence;
		}

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		VkFence fence;
		VkResult result = vkCreateFence(Device->Get(), &fenceInfo, nullptr, &fence);
		assert(result == VK_SUCCESS);
		vkResetFences(Device->Get(), 1, &fence);
		return fence;
	}

	void AddUsedFence(VkFence fence)
	{
		std::lock_guard<std::mutex> Lock(Mutex);
		UsedFencePool.push_back(fence);
	}

	void WaitForPastFence()
	{
		std::lock_guard<std::mutex> Lock(Mutex);
		if (UsedFencePool.size() > 0)
		{
			for (auto& fence : UsedFencePool)
			{
				RecycleFence(fence);
			}
			UsedFencePool.clear();
		}
	}

	void RecycleFence(VkFence fence)
	{
		std::lock_guard<std::mutex> Lock(Mutex);
		vkWaitForFences(Device->Get(), 1, &fence, VK_TRUE, UINT64_MAX);
		vkResetFences(Device->Get(), 1, &fence);
		FencePool.push_back(fence);
	}

private:
	VulkanDevice* Device;
	mutable std::int64_t SemaphoreCounter;
	std::vector<VkSemaphore> SemaphorePool;
	std::vector<VkFence> FencePool;
	std::vector<VkFence> UsedFencePool;
	std::mutex Mutex;
};

class VulkanDevice::VkCommandPoolManager
{
	sBaseClassBody(sClassConstructor, VulkanDevice::VkCommandPoolManager)
public:
	VkCommandPoolManager(VulkanDevice* device, uint32_t queueFamilyIndex)
		: Device(device)
		, QueueFamilyIndex(queueFamilyIndex)
	{}

	~VkCommandPoolManager()
	{
		Clear();
	}

	VkCommandPool RequestCommandPool()
	{
		std::lock_guard<std::mutex> Lock(Mutex);

		if (!Pool.empty())
		{
			auto& CommandPool = Pool.front();
			Pool.pop();
			vkResetCommandPool(Device->Get(), CommandPool, 0);
			return CommandPool;
		}

		VkCommandPool NewCommandPool = CreateCommandPool();
		return NewCommandPool;
	}

	void ReturnCommandPool(VkCommandPool CommandPool)
	{
		std::lock_guard<std::mutex> Lock(Mutex);
		Pool.push(CommandPool);
	}

	void Clear()
	{
		std::lock_guard<std::mutex> Lock(Mutex);
		while (!Pool.empty())
		{
			auto& CMDPool = Pool.front();
			vkResetCommandPool(Device->Get(), CMDPool, 0);
			vkDestroyCommandPool(Device->Get(), CMDPool, nullptr);
			Pool.pop();
		}
	}

private:
	VkCommandPool CreateCommandPool()
	{
		VkCommandPoolCreateInfo Info{};
		Info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		Info.queueFamilyIndex = QueueFamilyIndex;
		Info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VkCommandPool CommandPool;
		if (vkCreateCommandPool(Device->Get(), &Info, nullptr, &CommandPool) != VK_SUCCESS)
			throw std::runtime_error("Failed to create command pool");

		return CommandPool;
	}

private:
	VulkanDevice* Device;
	uint32_t QueueFamilyIndex;
	std::queue<VkCommandPool> Pool;
	std::mutex Mutex;
};

class VulkanDevice::VkCommandBufferManager
{
	sBaseClassBody(sClassConstructor, VulkanDevice::VkCommandBufferManager)
public:
	VkCommandBufferManager(VulkanDevice* device)
		: Device(device)
	{
		CommandPool = Device->CommandPoolManager->RequestCommandPool();
	}

	~VkCommandBufferManager()
	{
		for (auto& [commandBuffer, fence] : CommandBuffers) 
		{
			if (fence != VK_NULL_HANDLE)
				vkDestroyFence(Device->Get(), fence, nullptr);
		}

		vkDestroyCommandPool(Device->Get(), CommandPool, nullptr);
	}

	bool IsCommandBufferAvailable(VkCommandBuffer commandBuffer) const
	{
		VkFence fence = GetFence(commandBuffer);
		VkResult result = vkWaitForFences(Device->Get(), 1, &fence, true, 33 * 1000 * 1000LL);
		if (result == VkResult::VK_TIMEOUT)
			return false;
		return true;
	}

	VkFence GetFence(VkCommandBuffer commandBuffer) const
	{
		auto it = CommandBuffers.find(commandBuffer);
		if (it != CommandBuffers.end())
			return it->second;
		throw std::runtime_error("Command buffer not managed by this pool.");
		return VkFence();
	}

	VkCommandBuffer GetAvailableCommandBuffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) 
	{
		std::lock_guard<std::mutex> Lock(Mutex);
		while (!AvailableCommandBuffers.empty()) 
		{
			VkCommandBuffer commandBuffer = AvailableCommandBuffers.front();
			AvailableCommandBuffers.pop();

			const bool result = IsCommandBufferAvailable(commandBuffer);
			if (result) 
			{
				ResetCommandBuffer(commandBuffer);
				return commandBuffer;
			}
			else if (!result) 
			{
				continue;
			}
			else 
			{
				throw std::runtime_error("Failed to get fence status.");
			}
		}

		return AllocateCommandBuffer(level);
	}

	void RecycleCommandBuffer(VkCommandBuffer commandBuffer)
	{
		std::lock_guard<std::mutex> Lock(Mutex);
		AvailableCommandBuffers.push(commandBuffer);
	}

	void FreeCommandBuffer(VkCommandBuffer commandBuffer) 
	{
		std::lock_guard<std::mutex> Lock(Mutex);
		auto it = CommandBuffers.find(commandBuffer);
		if (it != CommandBuffers.end()) {
			if (it->second != VK_NULL_HANDLE) {
				vkDestroyFence(Device->Get(), it->second, nullptr);
			}
			CommandBuffers.erase(it);
		}

		vkFreeCommandBuffers(Device->Get(), CommandPool, 1, &commandBuffer);
	}

	void WaitForFence(VkCommandBuffer commandBuffer, uint64_t timeout = UINT64_MAX) 
	{
		std::lock_guard<std::mutex> Lock(Mutex);
		VkFence fence = GetFence(commandBuffer);
		if (vkWaitForFences(Device->Get(), 1, &fence, VK_TRUE, timeout) != VK_SUCCESS)
			throw std::runtime_error("Failed to wait for fence.");
	}

	void ResetCommandBuffer(VkCommandBuffer commandBuffer)
	{
		std::lock_guard<std::mutex> Lock(Mutex);
		if (vkResetCommandBuffer(commandBuffer, 0) != VK_SUCCESS)
			throw std::runtime_error("Failed to reset command buffer.");

		VkFence fence = GetFence(commandBuffer);
		if (vkResetFences(Device->Get(), 1, &fence) != VK_SUCCESS)
			throw std::runtime_error("Failed to reset fence.");
	}

	void Reset() 
	{
		std::lock_guard<std::mutex> Lock(Mutex);
		if (vkResetCommandPool(Device->Get(), CommandPool, 0) != VK_SUCCESS)
			throw std::runtime_error("Failed to reset command pool.");

		for (auto& [commandBuffer, fence] : CommandBuffers) 
		{
			if (fence != VK_NULL_HANDLE) 
			{
				if (vkResetFences(Device->Get(), 1, &fence) != VK_SUCCESS)
					throw std::runtime_error("Failed to reset fence.");
			}
		}
	}

private:
	VkCommandBuffer AllocateCommandBuffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = CommandPool;
		allocInfo.level = level;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		if (vkAllocateCommandBuffers(Device->Get(), &allocInfo, &commandBuffer) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate command buffer.");

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = 0;

		VkFence fence;
		if (vkCreateFence(Device->Get(), &fenceInfo, nullptr, &fence) != VK_SUCCESS)
			throw std::runtime_error("Failed to create fence.");

		CommandBuffers[commandBuffer] = fence;
		return commandBuffer;
	}

private:
	VulkanDevice* Device;
	VkCommandPool CommandPool;
	std::unordered_map<VkCommandBuffer, VkFence> CommandBuffers;
	std::queue<VkCommandBuffer> AvailableCommandBuffers;
	std::mutex Mutex;
};

#define DEBUG_D3DDEVICE 1
#define VK_DEBUG
#define VK_VALIDATION_LAYERS

#if defined(VK_DEBUG) || defined(VK_VALIDATION_LAYERS)
std::unordered_map< VkInstance, PFN_vkCreateDebugUtilsMessengerEXT > CreateDebugUtilsMessengerEXTDispatchTable;

static PFN_vkSetDebugUtilsObjectNameEXT     s_vkSetDebugUtilsObjectName = nullptr;
static PFN_vkCmdBeginDebugUtilsLabelEXT     s_vkCmdBeginDebugUtilsLabel = nullptr;
static PFN_vkCmdEndDebugUtilsLabelEXT       s_vkCmdEndDebugUtilsLabel = nullptr;

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pMessenger)
{
	auto dispatched_cmd = CreateDebugUtilsMessengerEXTDispatchTable.at(instance);
	return dispatched_cmd(instance, pCreateInfo, pAllocator, pMessenger);
}
#endif

#if defined(VK_DEBUG) || defined(VK_VALIDATION_LAYERS)
/// @brief A debug callback called from Vulkan validation layers.
static VKAPI_ATTR VkBool32 VKAPI_CALL FDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
	VkDebugUtilsMessageTypeFlagsEXT             message_types,
	const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
	void* user_data)
{
	if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		Engine::WriteToConsole(std::string(std::to_string(callback_data->messageIdNumber) + " Validation Layer: Error: " + callback_data->pMessageIdName + " : " + callback_data->pMessage));
	}
	else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		Engine::WriteToConsole(std::string(std::to_string(callback_data->messageIdNumber) + " Validation Layer: Warning: " + callback_data->pMessageIdName + " : " + callback_data->pMessage));
	}
	else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
	{
		Engine::WriteToConsole(std::string(std::to_string(callback_data->messageIdNumber) + " Validation Layer: Information: " + callback_data->pMessageIdName + " : " + callback_data->pMessage));
	}
	else if (message_severity & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
	{
		Engine::WriteToConsole(std::string(std::to_string(callback_data->messageIdNumber) + "{} Validation Layer: Performance warning: " + callback_data->pMessageIdName + " : " + callback_data->pMessage));
	}
	else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
	{
		Engine::WriteToConsole(std::string(std::to_string(callback_data->messageIdNumber) + "{} Validation Layer: Verbose: " + callback_data->pMessageIdName + " : " + callback_data->pMessage));
	}
	return VK_FALSE;
}
#endif

bool validate_extensions(const std::vector<const char*>& required, const std::vector<VkExtensionProperties>& available)
{
	bool all_found = true;

	for (const auto* extension_name : required)
	{
		bool found = false;
		for (const auto& available_extension : available)
		{
			if (strcmp(available_extension.extensionName, extension_name) == 0)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			// Output an error message for the missing extension
			Engine::WriteToConsole("Error: Required extension not found: " + *extension_name);
			all_found = false;
		}
	}

	return all_found;
}

bool validate_layers(const std::vector<const char*>& required, const std::vector<VkLayerProperties>& available)
{
	bool all_found = true;

	for (const auto* layer_name : required)
	{
		bool found = false;
		for (const auto& available_layer : available)
		{
			if (strcmp(available_layer.layerName, layer_name) == 0)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			Engine::WriteToConsole("Error: Required layer not found: " + *layer_name);
			all_found = false;
		}
	}

	return all_found;
}

std::vector<const char*> get_optimal_validation_layers(const std::vector<VkLayerProperties>& supported_instance_layers)
{
	std::vector<std::vector<const char*>> validation_layer_priority_list =
	{
		// The preferred validation layer is "VK_LAYER_KHRONOS_validation"
		{"VK_LAYER_KHRONOS_validation"},

		// Otherwise we fallback to using the LunarG meta layer
		{"VK_LAYER_LUNARG_standard_validation"},

		// Otherwise we attempt to enable the individual layers that compose the LunarG meta layer since it doesn't exist
		{
			"VK_LAYER_GOOGLE_threading",
			"VK_LAYER_LUNARG_parameter_validation",
			"VK_LAYER_LUNARG_object_tracker",
			"VK_LAYER_LUNARG_core_validation",
			"VK_LAYER_GOOGLE_unique_objects",
			"VK_LAYER_RENDERDOC_Capture",
		},

		// Otherwise as a last resort we fallback to attempting to enable the LunarG core layer
		{"VK_LAYER_LUNARG_core_validation"} };

	for (auto& validation_layers : validation_layer_priority_list)
	{
		if (validate_layers(validation_layers, supported_instance_layers))
		{
			return validation_layers;
		}

		Engine::WriteToConsole("Couldn't enable validation layers (see log for error) - falling back");
	}

	// Else return nothing
	return {};
}

uint32_t GetQueueFamilyIndex(std::vector<VkQueueFamilyProperties> queue_family_properties, VkQueueFlagBits queue_flag)
{
	// Dedicated queue for compute
	// Try to find a queue family index that supports compute but not graphics
	if (queue_flag & VK_QUEUE_COMPUTE_BIT)
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(queue_family_properties.size()); i++)
		{
			if ((queue_family_properties[i].queueFlags & queue_flag) && !(queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
			{
				return i;
			}
		}
	}

	// Dedicated queue for transfer
	// Try to find a queue family index that supports transfer but not graphics and compute
	if (queue_flag & VK_QUEUE_TRANSFER_BIT)
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(queue_family_properties.size()); i++)
		{
			if ((queue_family_properties[i].queueFlags & queue_flag) && !(queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && !(queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT))
			{
				return i;
			}
		}
	}

	// For other queue types or if no separate compute queue is present, return the first one to support the requested flags
	for (uint32_t i = 0; i < static_cast<uint32_t>(queue_family_properties.size()); i++)
	{
		if (queue_family_properties[i].queueFlags & queue_flag)
		{
			return i;
		}
	}

	throw std::runtime_error("Could not find a matching queue family index");
}

VulkanDevice::VulkanDevice(const GPUDeviceCreateInfo& DeviceCreateInfo) 
	: Super()
{
	Engine::WriteToConsole("Initializing Vulkan instance.");

	uint32_t instance_extension_count;
	VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr));

	std::vector<VkExtensionProperties> available_instance_extensions(instance_extension_count);
	VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, available_instance_extensions.data()));

	std::vector<const char*> required_instance_extensions{ VK_KHR_SURFACE_EXTENSION_NAME };

#if defined(VK_DEBUG) || defined(VK_VALIDATION_LAYERS)
	bool has_debug_utils = false;
	for (const auto& ext : available_instance_extensions)
	{
		if (strncmp(ext.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME, strlen(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) == 0)
		{
			has_debug_utils = true;
			break;
		}
	}
	if (has_debug_utils)
	{
		required_instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	else
	{
		Engine::WriteToConsole("{VK_EXT_DEBUG_UTILS_EXTENSION_NAME} is not available; disabling debug utils messenger");
	}
#endif

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	required_instance_extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
	required_instance_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_METAL_EXT)
	required_instance_extensions.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
	required_instance_extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
	required_instance_extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
	required_instance_extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_DISPLAY_KHR)
	required_instance_extensions.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
#else
#	pragma error Platform not supported
#endif

	required_instance_extensions.push_back("VK_KHR_get_surface_capabilities2");
	if (!validate_extensions(required_instance_extensions, available_instance_extensions))
	{
		throw std::runtime_error("Required instance extensions are missing.");
	}

	uint32_t instance_layer_count;
	VK_CHECK(vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr));

	std::vector<VkLayerProperties> supported_validation_layers(instance_layer_count);
	VK_CHECK(vkEnumerateInstanceLayerProperties(&instance_layer_count, supported_validation_layers.data()));

	std::vector<const char*> requested_validation_layers{};

#if defined(VK_DEBUG) || defined(VK_VALIDATION_LAYERS)
	// Determine the optimal validation layers to enable that are necessary for useful debugging
	std::vector<const char*> optimal_validation_layers = get_optimal_validation_layers(supported_validation_layers);
	requested_validation_layers.insert(requested_validation_layers.end(), optimal_validation_layers.begin(), optimal_validation_layers.end());
#endif

	if (validate_layers(requested_validation_layers, supported_validation_layers))
	{
		Engine::WriteToConsole("Enabled Validation Layers:");
		for (const auto& layer : requested_validation_layers)
		{
			Engine::WriteToConsole("	\t " + *layer);
		}
	}
	else
	{
		throw std::runtime_error("Required validation layers are missing.");
	}

	VkApplicationInfo app{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "Game Engine",
		.pEngineName = "DNGE",
		.apiVersion = VK_MAKE_VERSION(1, 3, 0) };

	VkInstanceCreateInfo instance_info{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &app,
		.enabledLayerCount = (uint32_t)requested_validation_layers.size(),
		.ppEnabledLayerNames = requested_validation_layers.data(),
		.enabledExtensionCount = (uint32_t)required_instance_extensions.size(),
		.ppEnabledExtensionNames = required_instance_extensions.data() };

#if defined(VK_DEBUG) || defined(VK_VALIDATION_LAYERS)
	VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
	if (has_debug_utils)
	{
		debug_messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
		debug_messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		debug_messenger_create_info.pfnUserCallback = FDebugCallback;

		instance_info.pNext = &debug_messenger_create_info;
	}
#endif

	VK_CHECK(vkCreateInstance(&instance_info, nullptr, &instance));
	//instance = vk::createInstance(instance_info);
	//instance = vk::raii::Instance(context, instance_info);

	Viewport = VulkanViewport::CreateUnique(this, static_cast<HWND*>(DeviceCreateInfo.pHWND));

	bool Result = GetAdapter(DeviceCreateInfo.GPUIndex, PhysicalDevice, FamilyProperties, DeviceProperties);
	if (!Result)
	{
		std::uint32_t gpu_count = 0;
		VK_CHECK(vkEnumeratePhysicalDevices(instance, &gpu_count, nullptr));

		for (std::uint32_t i = 0; i < gpu_count; i++)
		{
			Result = GetAdapter(i, PhysicalDevice, FamilyProperties, DeviceProperties);
			if (Result)
				break;
		}
	}

	std::uint32_t device_extension_count;
	VK_CHECK(vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &device_extension_count, nullptr));

	std::vector<VkExtensionProperties> device_extensions(device_extension_count);
	VK_CHECK(vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &device_extension_count, device_extensions.data()));

	// Since this sample has visual output, the device needs to support the swapchain extension
	std::vector<const char*> required_device_extensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	//required_device_extensions.push_back("VK_EXT_full_screen_exclusive");
	if (!validate_extensions(required_device_extensions, device_extensions))
	{
		Engine::WriteToConsole("Required device extensions are missing");
	}

	{
		// Query for Vulkan 1.3 features
		VkPhysicalDeviceFeatures2                       query_device_features2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
		VkPhysicalDeviceVulkan12Features vulkan12Features = {};
		vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		query_device_features2.pNext = &vulkan12Features;
		VkPhysicalDeviceVulkan13Features                query_vulkan13_features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
		VkPhysicalDeviceExtendedDynamicStateFeaturesEXT query_extended_dynamic_state_features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT };
		vulkan12Features.pNext = &query_vulkan13_features;
		query_vulkan13_features.pNext = &query_extended_dynamic_state_features;

		vkGetPhysicalDeviceFeatures2(PhysicalDevice, &query_device_features2);

		// Check if Physical device supports Vulkan 1.3 features
		if (!query_vulkan13_features.dynamicRendering)
			Engine::WriteToConsole("(Vulkan 1.3) Dynamic Rendering feature is missing");

		if (!query_vulkan13_features.synchronization2)
			Engine::WriteToConsole("(Vulkan 1.3) Synchronization2 feature is missing");

		if (!query_extended_dynamic_state_features.extendedDynamicState)
			Engine::WriteToConsole("(Vulkan 1.3) Extended Dynamic State feature is missing");

	}

	// Enable only specific Vulkan 1.3 features
	VkPhysicalDeviceExtendedDynamicStateFeaturesEXT enable_extended_dynamic_state_features = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
		.pNext = VK_NULL_HANDLE,
		.extendedDynamicState = VK_TRUE };

	VkPhysicalDeviceVulkan13Features enable_vulkan13_features = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
		.pNext = &enable_extended_dynamic_state_features,
		.synchronization2 = VK_TRUE,
		.dynamicRendering = VK_TRUE,
	};

	/*VkPhysicalDeviceDescriptorIndexingFeatures DescriptorIndexingFeatures{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
		.pNext = &enable_vulkan13_features,
		.descriptorBindingPartiallyBound = true,
		.runtimeDescriptorArray = true,
	};*/

	VkPhysicalDeviceVulkan12Features enable_vulkan12_features = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
		.pNext = &enable_vulkan13_features,
		//.timelineSemaphore = VK_TRUE,
	};

	enable_vulkan12_features.descriptorBindingPartiallyBound = true;
	enable_vulkan12_features.runtimeDescriptorArray = true;
	enable_vulkan12_features.shaderSampledImageArrayNonUniformIndexing = true;
	enable_vulkan12_features.descriptorBindingSampledImageUpdateAfterBind = true;
	enable_vulkan12_features.shaderUniformBufferArrayNonUniformIndexing = true;
	enable_vulkan12_features.descriptorBindingUniformBufferUpdateAfterBind = true;
	enable_vulkan12_features.shaderStorageBufferArrayNonUniformIndexing = true;
	enable_vulkan12_features.descriptorBindingStorageBufferUpdateAfterBind = true;
	enable_vulkan12_features.descriptorBindingVariableDescriptorCount = true;

	VkPhysicalDevicePushDescriptorPropertiesKHR pushDescriptorProperties{};
	pushDescriptorProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR;

	VkPhysicalDeviceFeatures2 enable_device_features2{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
		.pNext = &enable_vulkan12_features };
	// Create the logical device

	vkGetPhysicalDeviceFeatures2(PhysicalDevice, &enable_device_features2);

	std::vector<VkDeviceQueueCreateInfo> DeviceQueueCreateInfos;

	float queue_priority = 1.0f;

	// Create one queue
	VkDeviceQueueCreateInfo queue_info{
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = static_cast<std::uint32_t>(FamilyProperties.graphicsQueueFamilyIndex),
		.queueCount = 1,
		.pQueuePriorities = &queue_priority };
	DeviceQueueCreateInfos.push_back(queue_info);

	if (FamilyProperties.computeQueueFamilyIndex >= 0)
	{
		VkDeviceQueueCreateInfo compute_queue_info{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = static_cast<std::uint32_t>(FamilyProperties.computeQueueFamilyIndex),
			.queueCount = 1,
			.pQueuePriorities = &queue_priority };
		DeviceQueueCreateInfos.push_back(compute_queue_info);
	}

	if (FamilyProperties.copyQueueFamilyIndex >= 0)
	{
		VkDeviceQueueCreateInfo copy_queue_info{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = static_cast<std::uint32_t>(FamilyProperties.copyQueueFamilyIndex),
			.queueCount = 1,
			.pQueuePriorities = &queue_priority };
		DeviceQueueCreateInfos.push_back(copy_queue_info);
	}

	//VkPhysicalDeviceFeatures PhysicalDeviceFeatures = {};
	//PhysicalDeviceFeatures.shaderFloat64 = true;
	//vkGetPhysicalDeviceFeatures(PhysicalDevice, &PhysicalDeviceFeatures);

	required_device_extensions.push_back(VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME);
	//required_device_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
	required_device_extensions.push_back(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);

	VkDeviceCreateInfo device_info{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &enable_device_features2,
		.queueCreateInfoCount = (std::uint32_t)DeviceQueueCreateInfos.size(),
		.pQueueCreateInfos = DeviceQueueCreateInfos.data(),
		.enabledExtensionCount = (std::uint32_t)(required_device_extensions.size()),
		.ppEnabledExtensionNames = required_device_extensions.data(),
		//.pEnabledFeatures = &PhysicalDeviceFeatures
	};

	VkResult DeviceCreate_Result = vkCreateDevice(PhysicalDevice, &device_info, nullptr, &Device);

	vkGetDeviceQueue(Device, FamilyProperties.graphicsQueueFamilyIndex, 0, &GraphicsQueue);
	if (FamilyProperties.computeQueueFamilyIndex >= 0)
		vkGetDeviceQueue(Device, FamilyProperties.computeQueueFamilyIndex, 0, &ComputeQueue);

#if defined(VK_DEBUG) || defined(VK_VALIDATION_LAYERS)
	PFN_vkVoidFunction temp_fp;
	temp_fp = vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (!temp_fp) throw "Failed to load vkCreateDebugUtilsMessengerEXT"; // check shouldn't be necessary (based on spec)
	CreateDebugUtilsMessengerEXTDispatchTable[instance] = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(temp_fp);
#endif

#if defined(VK_DEBUG) || defined(VK_VALIDATION_LAYERS)
	if (has_debug_utils)
	{
		VK_CHECK(vkCreateDebugUtilsMessengerEXT(instance, &debug_messenger_create_info, nullptr, &debug_callback));
	}
	s_vkSetDebugUtilsObjectName = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(Device, "vkSetDebugUtilsObjectNameEXT");
	s_vkCmdBeginDebugUtilsLabel = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetDeviceProcAddr(Device, "vkCmdBeginDebugUtilsLabelEXT");
	s_vkCmdEndDebugUtilsLabel = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetDeviceProcAddr(Device, "vkCmdEndDebugUtilsLabelEXT");
#endif

	InitWindow(DeviceCreateInfo.pHWND, DeviceCreateInfo.Width, DeviceCreateInfo.Height, DeviceCreateInfo.Fullscreen);

	{
		BindlessDescriptorPool = VulkanBindlessDescriptorPool::CreateUnique(this);
		PushDescriptorPool = VulkanPushDescriptorPool::CreateUnique(this);
	}
	{
		ShaderCompiler = DXCShaderCompiler::CreateUnique();
		//pShaderCompiler = VulkanShaderCompiler::CreateUnique();
	}
	{
		MemoryManager = VulkanMemoryManager::CreateUnique(this);
		CommandPoolManager = VkCommandPoolManager::CreateUnique(this, FamilyProperties.graphicsQueueFamilyIndex);
		CommandBufferManager = VkCommandBufferManager::CreateUnique(this);
		SyncManager = VulkanSyncManager::CreateUnique(this);
		{
			IMCommandBuffer = VulkanCommandBuffer::CreateUnique(this);
		}
	}
}

VulkanDevice::~VulkanDevice()
{
	// destroy a vk::Device
	//Device.destroy();

	ShaderCompiler = nullptr;
	//pShaderCompiler = nullptr;

	Viewport = nullptr;

	BindlessDescriptorPool = nullptr;
	PushDescriptorPool = nullptr;

	IMCommandBuffer = nullptr;
	CommandPoolManager = nullptr;
	CommandBufferManager = nullptr;
	MemoryManager = nullptr;
	SyncManager = nullptr;
	vkDestroyInstance(instance, nullptr);
}

void VulkanDevice::InitWindow(void* InHWND, std::uint32_t InWidth, std::uint32_t InHeight, bool bFullscreen)
{
	Viewport->InitWindow(static_cast<HWND*>(InHWND), InWidth, InHeight, bFullscreen);
}

void VulkanDevice::BeginFrame()
{
	Viewport->BeginFrame();
}

bool VulkanDevice::GetAdapter(std::optional<short> Index, VkPhysicalDevice& GPU, QueueFamilyProperties& FamilyProperties, VkPhysicalDeviceProperties& properties) const
{
	VkPhysicalDevice* OutGPU = nullptr;
	QueueFamilyProperties OutFamilyProperties;
	VkPhysicalDeviceProperties DeviceProperties;
	bool bFound = false;

	std::uint32_t gpu_count = 0;
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &gpu_count, nullptr));

	if (gpu_count < 1)
	{
		throw std::runtime_error("No physical device found.");
	}

	std::vector<VkPhysicalDevice> gpus(gpu_count);
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &gpu_count, gpus.data()));

	if (Index.has_value())
	{
		if (Index.value() < 0)
			return false;
	}

	for (std::size_t i = 0; i < gpus.size(); i++)
	{
		VkPhysicalDevice physical_device = gpus[i];

		// Check if the device supports Vulkan 1.3
		VkPhysicalDeviceProperties device_properties;
		vkGetPhysicalDeviceProperties(physical_device, &device_properties);

		if (device_properties.apiVersion < VK_API_VERSION_1_3)
		{
			Engine::WriteToConsole("Physical device '{" + std::string(device_properties.deviceName) + "}' does not support Vulkan 1.3, skipping.");
			//continue;
		}

		// Find a queue family that supports graphics and presentation
		std::uint32_t queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

		if (queue_family_count < 1)
		{
			Engine::WriteToConsole("No queue family found : { " + std::string(device_properties.deviceName) + " }");
			continue;
		}

		std::vector<VkQueueFamilyProperties> queue_family_properties(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_family_properties.data());

		for (std::uint32_t i = 0; i < queue_family_count; i++)
		{
			VkBool32 supports_present = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, Viewport->GetSurface(), &supports_present);

			if ((queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && supports_present)
			{
				OutFamilyProperties.graphicsQueueFamilyIndex = i;
				OutFamilyProperties.presentQueueFamilyIndex = i;
			}

			if (queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT &&
				!(queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
				OutFamilyProperties.computeQueueFamilyIndex = i;
			}

			if (queue_family_properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT &&
				!(queue_family_properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)) {
				OutFamilyProperties.copyQueueFamilyIndex = i;
			}

		}

		if (OutFamilyProperties.graphicsQueueFamilyIndex >= 0)
		{
			bFound = true;
			OutGPU = &physical_device;
			DeviceProperties = device_properties;
		}

		if (Index.has_value())
		{
			if (Index.value() == i)
				break;
		}
	}

	if (OutFamilyProperties.graphicsQueueFamilyIndex < 0)
	{
		Engine::WriteToConsole("Failed to find a suitable GPU with Vulkan 1.3 support.");
	}

	if (bFound)
	{
		GPU = *OutGPU;
		FamilyProperties = OutFamilyProperties;
		properties = DeviceProperties;
		return true;
	}
	return false;
}

void VulkanDevice::Present(IRenderTarget* pRT)
{
	Viewport->Present(pRT);
}

bool VulkanDevice::IsFullScreen() const
{
	return Viewport->IsFullScreen();
}

bool VulkanDevice::IsVsyncEnabled() const
{
	return Viewport->IsVsyncEnabled();
}

std::uint32_t VulkanDevice::GetVsyncInterval() const
{
	return Viewport->GetVsyncInterval();
}

uint32_t VulkanDevice::GetMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties)
{
	// Structure to hold the physical device's memory properties
	VkPhysicalDeviceMemoryProperties mem_properties;
	vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &mem_properties);

	// Iterate over all memory types available on the physical device
	for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
	{
		// Check if the current memory type is acceptable based on the type_filter
		// The type_filter is a bitmask where each bit represents a memory type that is suitable
		if (type_filter & (1 << i))
		{
			// Check if the memory type has all the desired property flags
			// properties is a bitmask of the required memory properties
			if ((mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				// Found a suitable memory type; return its index
				return i;
			}
		}
	}

	// If no suitable memory type was found, throw an exception
	throw std::runtime_error("Failed to find suitable memory type.");
}

void VulkanDevice::ResizeWindow(std::size_t Width, std::size_t Height)
{
	Viewport->ResizeSwapChain(Width, Height);
}

void VulkanDevice::FullScreen(const bool value)
{
	Viewport->FullScreen(value);
}

void VulkanDevice::Vsync(const bool value)
{
	Viewport->Vsync(value);
}

void VulkanDevice::VsyncInterval(const std::uint32_t value)
{
	Viewport->VsyncInterval(value);
}

std::vector<sDisplayMode> VulkanDevice::GetAllSupportedResolutions() const
{
	std::vector<sDisplayMode> SupportedResolutions;

	uint32_t planeCount = 0;
	vkGetPhysicalDeviceDisplayPlanePropertiesKHR(PhysicalDevice, &planeCount, nullptr);

	std::vector<VkDisplayPlanePropertiesKHR> planeProperties(planeCount);
	vkGetPhysicalDeviceDisplayPlanePropertiesKHR(PhysicalDevice, &planeCount, planeProperties.data());

	for (uint32_t planeIndex = 0; planeIndex < planeCount; ++planeIndex) 
	{
		uint32_t displayCount = 0;
		vkGetDisplayPlaneSupportedDisplaysKHR(PhysicalDevice, planeIndex, &displayCount, nullptr);

		std::vector<VkDisplayKHR> displays(displayCount);
		vkGetDisplayPlaneSupportedDisplaysKHR(PhysicalDevice, planeIndex, &displayCount, displays.data());

		for (VkDisplayKHR display : displays) 
		{
			sDisplayMode DisplayMode;
			VkDisplayPropertiesKHR displayProperties;
			vkGetPhysicalDeviceDisplayPropertiesKHR(PhysicalDevice, &displayCount, &displayProperties);

			DisplayMode.Name = displayProperties.displayName;
			DisplayMode.Width = displayProperties.physicalResolution.width;
			DisplayMode.Height = displayProperties.physicalResolution.height;

			uint32_t modeCount = 0;
			vkGetDisplayModePropertiesKHR(PhysicalDevice, display, &modeCount, nullptr);

			std::vector<VkDisplayModePropertiesKHR> modeProperties(modeCount);
			vkGetDisplayModePropertiesKHR(PhysicalDevice, display, &modeCount, modeProperties.data());

			for (const auto& mode : modeProperties) 
			{
				DisplayMode.RefreshRate.Numerator = mode.parameters.refreshRate;
				DisplayMode.RefreshRate.Denominator = 1000;
				SupportedResolutions.push_back(DisplayMode);
			}
		}
	}

	return SupportedResolutions;
}

sScreenDimension VulkanDevice::GetBackBufferDimension() const
{
	return Viewport->GetScreenDimension();
}

EFormat VulkanDevice::GetBackBufferFormat() const
{
	return Viewport->GetFormat();
}

sViewport VulkanDevice::GetViewport() const
{
	return Viewport->GetViewport();
}

void VulkanDevice::SetPerfMarkerBegin(VkCommandBuffer cmd_buf, const char* name)
{
#if defined(VK_DEBUG) || defined(VK_VALIDATION_LAYERS)
	if (s_vkCmdBeginDebugUtilsLabel)
	{
		VkDebugUtilsLabelEXT label = {};
		label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
		label.pLabelName = name;
		const float color[] = { 1.0f, 0.0f, 0.0f, 1.0f };
		memcpy(label.color, color, sizeof(color));
		s_vkCmdBeginDebugUtilsLabel(cmd_buf, &label);
	}
#endif
}

void VulkanDevice::SetPerfMarkerEnd(VkCommandBuffer cmd_buf)
{
#if defined(VK_DEBUG) || defined(VK_VALIDATION_LAYERS)
	if (s_vkCmdEndDebugUtilsLabel)
	{
		s_vkCmdEndDebugUtilsLabel(cmd_buf);
	}
#endif
}

void VulkanDevice::SetResourceName(VkObjectType objectType, uint64_t handle, const char* name)
{
#if defined(VK_DEBUG) || defined(VK_VALIDATION_LAYERS)
	if (s_vkSetDebugUtilsObjectName && handle && name)
	{
		VkDebugUtilsObjectNameInfoEXT nameInfo = {};
		nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		nameInfo.objectType = objectType;
		nameInfo.objectHandle = handle;
		nameInfo.pObjectName = name;
		s_vkSetDebugUtilsObjectName(Device, &nameInfo);
	}
#endif
}

VkDeviceMemory VulkanDevice::AllocateMemory(/*std::string ClassID,*/ VkDeviceSize size, uint32_t memoryTypeIndex)
{
	return MemoryManager->AllocateMemory(/*ClassID,*/ size, memoryTypeIndex);
}

VkDeviceMemory VulkanDevice::AllocateMemory(/*std::string ClassID,*/ VkDeviceSize size, uint32_t type_filter, VkMemoryPropertyFlags properties)
{
	return MemoryManager->AllocateMemory(/*ClassID, */size, GetMemoryType(type_filter, properties));
}

void VulkanDevice::FreeMemory(VkDeviceMemory memory)
{
	MemoryManager->FreeMemory(memory);
}

bool VulkanDevice::GetDeviceIdentification(std::wstring& InVendorID, std::wstring& InDeviceID)
{
	InDeviceID = std::to_wstring(DeviceProperties.deviceID);
	InVendorID = std::to_wstring(DeviceProperties.vendorID);
	return true;
}

VkCommandBuffer VulkanDevice::RequestCommandBuffer()
{
	return CommandBufferManager->GetAvailableCommandBuffer();
}

void VulkanDevice::ReturnCommandBuffer(VkCommandBuffer commandPool)
{
	CommandBufferManager->RecycleCommandBuffer(commandPool);
}

bool VulkanDevice::IsCommandBufferAvailable(VkCommandBuffer commandBuffer) const
{
	return CommandBufferManager->IsCommandBufferAvailable(commandBuffer);
}

void VulkanDevice::ExecuteGraphicsCommandBuffer(VkCommandBuffer CommandBuffer, bool WaitForCompletion)
{
	if (SyncManager->IsSignalledSemaphoresAvailable())
	{
		VkSemaphore Wait = SyncManager->AcquireSignalledSemaphore();
		VkSemaphore Signal = SyncManager->AcquireSemaphore();
		VkFence QueueFence = CommandBufferManager->GetFence(CommandBuffer);

		VkResult Result = vkGetFenceStatus(Device, QueueFence);
		if (Result == VkResult::VK_SUCCESS)
			vkResetFences(Get(), 1, &QueueFence);

		VkPipelineStageFlags wait_stage{ VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT };

		VkSubmitInfo info{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &Wait,
			.pWaitDstStageMask = &wait_stage,
			.commandBufferCount = 1,
			.pCommandBuffers = &CommandBuffer,
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &Signal };

		/*std::vector<uint64_t> m_WaitSemaphoreValues;
		m_WaitSemaphoreValues.push_back(0);
		std::vector<uint64_t> m_SignalSemaphoreValues;
		m_SignalSemaphoreValues.push_back(0);
		VkTimelineSemaphoreSubmitInfo TimelineSemaphoreSubmitInfo;
		TimelineSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
		TimelineSemaphoreSubmitInfo.waitSemaphoreValueCount = m_WaitSemaphoreValues.size();
		TimelineSemaphoreSubmitInfo.pWaitSemaphoreValues = m_SignalSemaphoreValues.data();
		TimelineSemaphoreSubmitInfo.signalSemaphoreValueCount = m_SignalSemaphoreValues.size();
		TimelineSemaphoreSubmitInfo.pSignalSemaphoreValues = m_SignalSemaphoreValues.data();
		info.pNext = &TimelineSemaphoreSubmitInfo;*/

		VK_CHECK(vkQueueSubmit(GraphicsQueue, 1, &info, QueueFence));

		if (WaitForCompletion)
		{
			vkWaitForFences(Device, 1, &QueueFence, true, UINT64_MAX /*33 * 1000 * 1000*/);
		}
	}
	else
	{
		VkFence QueueFence = CommandBufferManager->GetFence(CommandBuffer);

		VkResult Result = vkGetFenceStatus(Device, QueueFence);
		if (Result == VkResult::VK_SUCCESS)
			vkResetFences(Get(), 1, &QueueFence);

		VkSubmitInfo info{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.commandBufferCount = 1,
			.pCommandBuffers = &CommandBuffer,
		};

		VK_CHECK(vkQueueSubmit(GraphicsQueue, 1, &info, QueueFence));

		if (WaitForCompletion)
		{
			vkWaitForFences(Device, 1, &QueueFence, true, UINT64_MAX /*33 * 1000 * 1000*/);
		}
	}
}

#ifdef VK_HPP
void VulkanDevice::ExecuteGraphicsCommandBuffer(vk::CommandBuffer* pCommandList, bool WaitForCompletion)
{
	VkCommandBuffer CommandBuffer = *pCommandList;
	ExecuteGraphicsCommandBuffer(CommandBuffer, WaitForCompletion);
}
#endif

void VulkanDevice::ExecuteComputeCommandBuffer(VkCommandBuffer CommandBuffer, bool WaitForCompletion)
{
	if (!ComputeQueue)
		return;

	auto Wait = SyncManager->AcquireSignalledSemaphore();
	auto Signal = SyncManager->AcquireSemaphore();
	auto QueueFence = CommandBufferManager->GetFence(CommandBuffer);

	auto Result = vkGetFenceStatus(Device, QueueFence);
	if (Result == VkResult::VK_SUCCESS)
		vkResetFences(Get(), 1, &QueueFence);

	VkPipelineStageFlags wait_stage{ VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT };

	VkSubmitInfo info{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &Wait,
		.pWaitDstStageMask = &wait_stage,
		.commandBufferCount = 1,
		.pCommandBuffers = &CommandBuffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &Signal };

	VK_CHECK(vkQueueSubmit(ComputeQueue, 1, &info, QueueFence));
}

void VulkanDevice::ExecuteCopyCommandBuffer(VkCommandBuffer pCommandList, bool WaitForCompletion)
{
	/*auto Wait = SyncManager->AcquireSemaphore();
	auto Signal = SyncManager->AcquireSemaphore();
	auto QueueFence = CommandBufferManager->GetFence(CommandBuffer);

	auto Result = vkGetFenceStatus(Device, QueueFence);
	if (Result == VkResult::VK_SUCCESS)
		vkResetFences(Get(), 1, &QueueFence);

	VkPipelineStageFlags wait_stage{ VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT };

	VkSubmitInfo info{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &Wait,
		.pWaitDstStageMask = &wait_stage,
		.commandBufferCount = 1,
		.pCommandBuffers = &CommandBuffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &Signal };

	VK_CHECK(vkQueueSubmit(ComputeQueue, 1, &info, QueueFence));

	SyncManager->RecycleSemaphore(Wait);
	SyncManager->RecycleSemaphore(Signal);*/
}

void VulkanDevice::ResetCommandBuffer(VkCommandBuffer CommandBuffer)
{
	CommandBufferManager->ResetCommandBuffer(CommandBuffer);
}

VkDescriptorSetLayout VulkanDevice::GetPushDescriptorSetLayout() const
{
	return PushDescriptorPool->GetDescriptorSetLayout();
}

VkDescriptorSetLayout VulkanDevice::GetBindlessDescriptorSetLayout() const
{
	return BindlessDescriptorPool->GetDescriptorSetLayout();
}

VkDescriptorSet VulkanDevice::GetBindlessDescriptorSet() const
{
	return BindlessDescriptorPool->GetDescriptorSet();
}

void VulkanDevice::UpdateBindlessImageDescriptor(uint32_t binding, uint32_t arrayIndex, VkDescriptorImageInfo* imageInfo)
{
	BindlessDescriptorPool->UpdateImageDescriptor(binding, arrayIndex, imageInfo);
}

void VulkanDevice::UpdateBindlessBufferDescriptor(uint32_t binding, uint32_t arrayIndex, VkDescriptorBufferInfo* bufferInfo)
{
	BindlessDescriptorPool->UpdateBufferDescriptor(binding, arrayIndex, bufferInfo);
}

VkSemaphore VulkanDevice::AcquireSignalledSemaphore()
{
	return SyncManager->AcquireSignalledSemaphore();
}

VkSemaphore VulkanDevice::AcquireSemaphore()
{
	return SyncManager->AcquireSemaphore();
}

VkFence VulkanDevice::AcquireFence()
{
	return SyncManager->AcquireFence();
}

void VulkanDevice::RecycleFence(VkFence Fence)
{
	SyncManager->RecycleFence(Fence);
}

IShader* VulkanDevice::CompileShader(const sShaderAttachment& Attachment, bool Spirv)
{
	if (sShaderManager::Get().IsShaderExist(Attachment.GetLocation(), Attachment.FunctionName))
		return sShaderManager::Get().GetShader(Attachment.GetLocation(), Attachment.FunctionName);

	IShader::SharedPtr pShader = ShaderCompiler->Compile(Attachment, Spirv);
	//IShader::SharedPtr pShader = pShaderCompiler->Compile(Attachment);
	sShaderManager::Get().StoreShader(pShader);
	return pShader.get();
}

IShader* VulkanDevice::CompileShader(std::wstring InSrcFile, std::string InFunctionName, eShaderType InProfile, bool Spirv, std::vector<sShaderDefines> InDefines)
{
	if (sShaderManager::Get().IsShaderExist(InSrcFile, InFunctionName))
		return sShaderManager::Get().GetShader(InSrcFile, InFunctionName);

	IShader::SharedPtr pShader = ShaderCompiler->Compile(InSrcFile, InFunctionName, InProfile, Spirv, InDefines);
	//IShader::SharedPtr pShader = pShaderCompiler->Compile(InSrcFile, InFunctionName, InProfile, InDefines);
	sShaderManager::Get().StoreShader(pShader);
	return pShader.get();
}

IShader* VulkanDevice::CompileShader(const void* InCode, std::size_t Size, std::string InFunctionName, eShaderType InProfile, bool Spirv, std::vector<sShaderDefines> InDefines)
{
	if (sShaderManager::Get().IsShaderExist(L"", InFunctionName))
		return sShaderManager::Get().GetShader(L"", InFunctionName);

	IShader::SharedPtr pShader = ShaderCompiler->Compile(InCode, Size, InFunctionName, InProfile, Spirv, InDefines);
	//IShader::SharedPtr pShader = pShaderCompiler->Compile(InCode, Size, InFunctionName, InProfile, InDefines);
	sShaderManager::Get().StoreShader(pShader);
	return pShader.get();
}

IGraphicsCommandContext::SharedPtr VulkanDevice::CreateGraphicsCommandContext()
{
	return VulkanCommandBuffer::Create(this);
}

IGraphicsCommandContext::UniquePtr VulkanDevice::CreateUniqueGraphicsCommandContext()
{
	return VulkanCommandBuffer::CreateUnique(this);
}

IComputeCommandContext::SharedPtr VulkanDevice::CreateComputeCommandContext()
{
	return VulkanComputeCommandContext::Create(this);
}

IComputeCommandContext::UniquePtr VulkanDevice::CreateUniqueComputeCommandContext()
{
	return VulkanComputeCommandContext::CreateUnique(this);
}

ICopyCommandContext::SharedPtr VulkanDevice::CreateCopyCommandContext()
{
	return VulkanCopyCommandBuffer::Create(this);
}

ICopyCommandContext::UniquePtr VulkanDevice::CreateUniqueCopyCommandContext()
{
	return VulkanCopyCommandBuffer::CreateUnique(this);
}

IConstantBuffer::SharedPtr VulkanDevice::CreateConstantBuffer(std::string InName, const BufferLayout& InDesc, std::uint32_t InRootParameterIndex)
{
	return VulkanConstantBuffer::Create(this, InName, InDesc, InRootParameterIndex);
}

IConstantBuffer::UniquePtr VulkanDevice::CreateUniqueConstantBuffer(std::string InName, const BufferLayout& InDesc, std::uint32_t InRootParameterIndex)
{
	return VulkanConstantBuffer::CreateUnique(this, InName, InDesc, InRootParameterIndex);
}

IVertexBuffer::SharedPtr VulkanDevice::CreateVertexBuffer(std::string InName, const BufferLayout& InDesc, BufferSubresource* InSubresource)
{
	return VulkanVertexBuffer::Create(this, InName, InDesc, InSubresource);
}

IVertexBuffer::UniquePtr VulkanDevice::CreateUniqueVertexBuffer(std::string InName, const BufferLayout& InDesc, BufferSubresource* InSubresource)
{
	return VulkanVertexBuffer::CreateUnique(this, InName, InDesc, InSubresource);
}

IIndexBuffer::SharedPtr VulkanDevice::CreateIndexBuffer(std::string InName, const BufferLayout& InDesc, BufferSubresource* InSubresource)
{
	return VulkanIndexBuffer::Create(this, InName, InDesc, InSubresource);
}

IIndexBuffer::UniquePtr VulkanDevice::CreateUniqueIndexBuffer(std::string InName, const BufferLayout& InDesc, BufferSubresource* InSubresource)
{
	return VulkanIndexBuffer::CreateUnique(this, InName, InDesc, InSubresource);
}

IFrameBuffer::SharedPtr VulkanDevice::CreateFrameBuffer(const std::string InName, const sFrameBufferAttachmentInfo& InAttachments)
{
	return VulkanFrameBuffer::Create(this, InName, InAttachments);
}

IFrameBuffer::UniquePtr VulkanDevice::CreateUniqueFrameBuffer(const std::string InName, const sFrameBufferAttachmentInfo& InAttachments)
{
	return VulkanFrameBuffer::CreateUnique(this, InName, InAttachments);
}

IRenderTarget::SharedPtr VulkanDevice::CreateRenderTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc)
{
	return VulkanRenderTarget::Create(this, InName, Format, Desc);
}

IRenderTarget::UniquePtr VulkanDevice::CreateUniqueRenderTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc)
{
	return VulkanRenderTarget::CreateUnique(this, InName, Format, Desc);
}

IDepthTarget::SharedPtr VulkanDevice::CreateDepthTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc)
{
	return VulkanDepthTarget::Create(this, InName, Format, Desc);
}

IDepthTarget::UniquePtr VulkanDevice::CreateUniqueDepthTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc)
{
	return VulkanDepthTarget::CreateUnique(this, InName, Format, Desc);
}

IUnorderedAccessTarget::SharedPtr VulkanDevice::CreateUnorderedAccessTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc, bool InEnableSRV)
{
	return VulkanUnorderedAccessTarget::Create(this, InName, Format, Desc, InEnableSRV);
}

IUnorderedAccessTarget::UniquePtr VulkanDevice::CreateUniqueUnorderedAccessTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc, bool InEnableSRV)
{
	return VulkanUnorderedAccessTarget::CreateUnique(this, InName, Format, Desc, InEnableSRV);
}

IPipeline::SharedPtr VulkanDevice::CreatePipeline(const std::string& InName, const sPipelineDesc& InDesc)
{
	return VulkanPipeline::Create(this, InName, InDesc);
}

IPipeline::UniquePtr VulkanDevice::CreateUniquePipeline(const std::string& InName, const sPipelineDesc& InDesc)
{
	return VulkanPipeline::CreateUnique(this, InName, InDesc);
}

IComputePipeline::SharedPtr VulkanDevice::CreateComputePipeline(const std::string& InName, const sComputePipelineDesc& InDesc)
{
	return VulkanComputePipeline::Create(this, InName, InDesc);
}

IComputePipeline::UniquePtr VulkanDevice::CreateUniqueComputePipeline(const std::string& InName, const sComputePipelineDesc& InDesc)
{
	return VulkanComputePipeline::CreateUnique(this, InName, InDesc);
}

ITexture2D::SharedPtr VulkanDevice::CreateTexture2D(const std::wstring FilePath, const std::string InName, std::uint32_t DefaultRootParameterIndex)
{
	return VulkanTexture::Create(this, FilePath, InName, DefaultRootParameterIndex);
}

ITexture2D::UniquePtr VulkanDevice::CreateUniqueTexture2D(const std::wstring FilePath, const std::string InName, std::uint32_t DefaultRootParameterIndex)
{
	return VulkanTexture::CreateUnique(this, FilePath, InName, DefaultRootParameterIndex);
}

ITexture2D::SharedPtr VulkanDevice::CreateTexture2D(const std::string InName, void* InBuffer, const std::size_t InSize, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
{
	return VulkanTexture::Create(this, InName, InBuffer, InSize, InDesc, DefaultRootParameterIndex);
}

ITexture2D::UniquePtr VulkanDevice::CreateUniqueTexture2D(const std::string InName, void* InBuffer, const std::size_t InSize, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
{
	return VulkanTexture::CreateUnique(this, InName, InBuffer, InSize, InDesc, DefaultRootParameterIndex);
}

ITexture2D::SharedPtr VulkanDevice::CreateEmptyTexture2D(const std::string InName, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
{
	return VulkanTexture::Create(this, InName, InDesc, DefaultRootParameterIndex);
}

ITexture2D::UniquePtr VulkanDevice::CreateUniqueEmptyTexture2D(const std::string InName, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
{
	return VulkanTexture::CreateUnique(this, InName, InDesc, DefaultRootParameterIndex);
}

//ITiledTexture::SharedPtr VulkanDevice::CreateTiledTexture(const std::string InName, const std::uint32_t InTileX, const std::uint32_t InTileY, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
//{
//	return nullptr;
//}
//
//ITiledTexture::UniquePtr VulkanDevice::CreateUniqueTiledTexture(const std::string InName, const std::uint32_t InTileX, const std::uint32_t InTileY, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
//{
//	return nullptr;
//}
