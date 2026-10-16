/* ---------------------------------------------------------------------------------------
* MIT License
*
* Copyright (c) 2023 Davut Coşkun.
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

#include <vulkan/vulkan.h>
#include <Engine/ClassBody.h>
#include <atomic>
#include <mutex>

class VulkanFence
{
	sBaseClassBody(sClassConstructor, VulkanFence)
public:
	explicit VulkanFence(VkDevice device, bool initiallySignaled = false);
	~VulkanFence();

	VulkanFence(const VulkanFence&) = delete;
	VulkanFence& operator=(const VulkanFence&) = delete;

	VkFence Get() const noexcept
	{
		return Fence;
	}

	bool IsComplete() const;
	bool Signaled() const 
	{
		return IsComplete();
	}

	// Returns false only when the timeout expires.
	bool CpuWait(std::uint64_t timeout = UINT64_MAX) const;
	void Reset();

private:
	VkDevice Device = VK_NULL_HANDLE;
	VkFence Fence = VK_NULL_HANDLE;
	mutable std::mutex Mutex;
};
