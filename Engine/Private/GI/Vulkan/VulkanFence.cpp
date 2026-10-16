
#include "pch.h"
#include "VulkanFence.h"

#include <stdexcept>
#include <string>

namespace
{
	void CheckVkResult(VkResult result, const char* operation)
	{
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error(
				std::string(operation) + " failed with VkResult " +
				std::to_string(result));
		}
	}
}

VulkanFence::VulkanFence(VkDevice device, bool initiallySignaled)
	: Device(device)
{
	if (Device == VK_NULL_HANDLE)
		throw std::invalid_argument("Cannot create a Vulkan fence with a null device.");

	VkFenceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	createInfo.flags = initiallySignaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

	CheckVkResult(vkCreateFence(Device, &createInfo, nullptr, &Fence),
		"vkCreateFence");
}

VulkanFence::~VulkanFence()
{
	if (Fence != VK_NULL_HANDLE)
		vkDestroyFence(Device, Fence, nullptr);
}

bool VulkanFence::IsComplete() const
{
	std::lock_guard<std::mutex> lock(Mutex);

	const VkResult result = vkGetFenceStatus(Device, Fence);
	if (result == VK_SUCCESS)
		return true;
	if (result == VK_NOT_READY)
		return false;

	CheckVkResult(result, "vkGetFenceStatus");
	return false;
}

bool VulkanFence::CpuWait(std::uint64_t timeout) const
{
	std::lock_guard<std::mutex> lock(Mutex);

	const VkResult result = vkWaitForFences(
		Device, 1, &Fence, VK_TRUE, timeout);

	if (result == VK_SUCCESS)
		return true;
	if (result == VK_TIMEOUT)
		return false;

	CheckVkResult(result, "vkWaitForFences");
	return false;
}

void VulkanFence::Reset()
{
	std::lock_guard<std::mutex> lock(Mutex);
	CheckVkResult(vkResetFences(Device, 1, &Fence), "vkResetFences");
}
