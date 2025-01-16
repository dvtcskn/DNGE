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
#include "VulkanBuffer.h"
#include "VulkanCommandBuffer.h"
#include "VulkanException.h"

VulkanUploadBuffer::VulkanUploadBuffer(VulkanDevice* Device, std::string InName, const std::uint32_t InSize)
	: Name(InName + "_UploadBuffer")
	, Owner(Device)
	, Size(InSize)
	, Buffer(VK_NULL_HANDLE)
	, pData(nullptr)
	, bIsMapped(false)
{
	VkBufferCreateInfo bufferInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.flags = 0,
		.size = Size,
		.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE };

	VK_CHECK(vkCreateBuffer(Owner->Get(), &bufferInfo, nullptr, &Buffer));

	// Get memory requirements
	VkMemoryRequirements memory_requirements;
	vkGetBufferMemoryRequirements(Owner->Get(), Buffer, &memory_requirements);

	// Allocate memory for the buffer
	VkMemoryAllocateInfo alloc_info{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memory_requirements.size,
		.memoryTypeIndex = Owner->GetMemoryType(memory_requirements.memoryTypeBits,
											VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) };

	VK_CHECK(vkAllocateMemory(Owner->Get(), &alloc_info, nullptr, &Memory));

	// Bind the buffer with the allocated memory
	VK_CHECK(vkBindBufferMemory(Owner->Get(), Buffer, Memory, 0));
}

VulkanUploadBuffer::~VulkanUploadBuffer()
{
	Unmap();
	vkDestroyBuffer(Owner->Get(), Buffer, nullptr);
	vkFreeMemory(Owner->Get(), Memory, nullptr);
}

void VulkanUploadBuffer::Map(const void* Ptr, IGraphicsCommandContext* InCMDBuffer)
{
	if (!bIsMapped)
	{
		VK_CHECK(vkMapMemory(Owner->Get(), Memory, 0, Size, 0, &pData));
		bIsMapped = true;
		if (Ptr)
			memcpy(pData, Ptr, Size);
	}
	else
	{
		if (Ptr)
			memcpy(pData, Ptr, Size);
	}
}

void VulkanUploadBuffer::Unmap()
{
	if (bIsMapped)
		vkUnmapMemory(Owner->Get(), Memory);
}

VulkanConstantBuffer::VulkanConstantBuffer(VulkanDevice* Device, std::string InName, const BufferLayout& InDesc, std::uint32_t InRootParameterIndex)
	: Name(InName)
	, Owner(Device)
	, Desc(InDesc)
	, Buffer(VK_NULL_HANDLE)
	, RootParameterIndex(InRootParameterIndex)
	, pData(nullptr)
	, bIsMapped(false)
{
	VkDeviceSize buffer_size = GetSize();

	// Create the vertex buffer
	VkBufferCreateInfo buffer_info{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.flags = 0,
		.size = buffer_size,
		.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE };

	VK_CHECK(vkCreateBuffer(Owner->Get(), &buffer_info, nullptr, &Buffer));

	// Get memory requirements
	VkMemoryRequirements memory_requirements;
	vkGetBufferMemoryRequirements(Owner->Get(), Buffer, &memory_requirements);

	// Allocate memory for the buffer
	VkMemoryAllocateInfo alloc_info{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memory_requirements.size,
		.memoryTypeIndex = Owner->GetMemoryType(memory_requirements.memoryTypeBits,
											VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) };

	VK_CHECK(vkAllocateMemory(Owner->Get(), &alloc_info, nullptr, &Memory));

	// Bind the buffer with the allocated memory
	VK_CHECK(vkBindBufferMemory(Owner->Get(), Buffer, Memory, 0));

	{
		DescriptorInfo.offset = 0;
		DescriptorInfo.range = buffer_size;// VK_WHOLE_SIZE;
		DescriptorInfo.buffer = Buffer;
	}

#if _DEBUG
	{
		std::string STR = std::string("VulkanConstantBuffer::") + Name.c_str();
		Device->SetResourceName(VkObjectType::VK_OBJECT_TYPE_BUFFER, reinterpret_cast<std::uint64_t>(Buffer), STR.c_str());
	}
#endif
}

VulkanConstantBuffer::~VulkanConstantBuffer()
{
	Unmap();
	vkDestroyBuffer(Owner->Get(), Buffer, nullptr);
	vkFreeMemory(Owner->Get(), Memory, nullptr);
}

void VulkanConstantBuffer::Map(const void* Ptr, IGraphicsCommandContext* InCMDBuffer)
{
	if (!bIsMapped)
	{
		VK_CHECK(vkMapMemory(Owner->Get(), Memory, 0, GetSize(), 0, &pData));
		bIsMapped = true;
		if (Ptr)
			memcpy(pData, Ptr, GetSize());
	}
	else
	{
		if (Ptr)
			memcpy(pData, Ptr, GetSize());
	}
}

void VulkanConstantBuffer::Unmap()
{
	if (bIsMapped)
		vkUnmapMemory(Owner->Get(), Memory);
}

std::uint32_t VulkanConstantBuffer::GetSize() const
{
	return 128;
	return (Desc.Size + 255) & ~255;
}

VulkanVertexBuffer::VulkanVertexBuffer(VulkanDevice* Device, std::string InName, const BufferLayout& InDesc, BufferSubresource* InSubresource)
	: Name(InName)
	, Owner(Device)
	, Desc(InDesc)
	, Buffer(VK_NULL_HANDLE)
	, pData(nullptr)
	, bIsMapped(false)
{
	VkDeviceSize buffer_size = Desc.Size;

	// Create the vertex buffer
	VkBufferCreateInfo buffer_info{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.flags = 0,
		.size = buffer_size,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE };

	VK_CHECK(vkCreateBuffer(Owner->Get(), &buffer_info, nullptr, &Buffer));

	// Get memory requirements
	VkMemoryRequirements memory_requirements;
	vkGetBufferMemoryRequirements(Owner->Get(), Buffer, &memory_requirements);

	// Allocate memory for the buffer
	VkMemoryAllocateInfo alloc_info{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memory_requirements.size,
		.memoryTypeIndex = Owner->GetMemoryType(memory_requirements.memoryTypeBits,
											VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) };

	VK_CHECK(vkAllocateMemory(Owner->Get(), &alloc_info, nullptr, &Memory));

	// Bind the buffer with the allocated memory
	VK_CHECK(vkBindBufferMemory(Owner->Get(), Buffer, Memory, 0));

	UploadBuffer = VulkanUploadBuffer::CreateUnique(Device, InName, InDesc.Size);
	UploadBuffer->Map();

	UpdateSubresource(InSubresource);

#if _DEBUG
	std::string STR = std::string("VulkanVertexBuffer::") + Name.c_str();
	Device->SetResourceName(VkObjectType::VK_OBJECT_TYPE_BUFFER, reinterpret_cast<std::uint64_t>(Buffer), STR.c_str());
#endif
}

VulkanVertexBuffer::~VulkanVertexBuffer()
{
	UploadBuffer = nullptr;
	vkDestroyBuffer(Owner->Get(), Buffer, nullptr);
	vkFreeMemory(Owner->Get(), Memory, nullptr);
}

void VulkanVertexBuffer::Map(const void* Ptr, IGraphicsCommandContext* InCMDBuffer)
{
	if (!bIsMapped)
	{
		VK_CHECK(vkMapMemory(Owner->Get(), Memory, 0, Desc.Size, 0, &pData));
		bIsMapped = true;
		if (Ptr)
			memcpy(pData, Ptr, Desc.Size);
	}
	else
	{
		if (Ptr)
			memcpy(pData, Ptr, Desc.Size);
	}
}

void VulkanVertexBuffer::Unmap()
{
	if (bIsMapped)
		vkUnmapMemory(Owner->Get(), Memory);
}

void VulkanVertexBuffer::UpdateSubresource(BufferSubresource* Subresource, IGraphicsCommandContext* InCMDBuffer)
{
	if (!Subresource)
		return;

	if (InCMDBuffer)
	{
		static_cast<VulkanCommandBuffer*>(InCMDBuffer)->UpdateBufferSubresource(this, Subresource);
	}
	else
	{
		Owner->GetIMCommandBuffer()->BeginRecordCommandList();
		Owner->GetIMCommandBuffer()->UpdateBufferSubresource(this, Subresource);
		Owner->GetIMCommandBuffer()->FinishRecordCommandList();
		Owner->GetIMCommandBuffer()->ExecuteCommandList();
	}
}

VulkanIndexBuffer::VulkanIndexBuffer(VulkanDevice* Device, std::string InName, const BufferLayout& InDesc, BufferSubresource* InSubresource)
	: Name(InName)
	, Owner(Device)
	, Desc(InDesc)
	, Buffer(VK_NULL_HANDLE)
	, pData(nullptr)
	, bIsMapped(false)
{
	VkDeviceSize buffer_size = Desc.Size;

	// Create the vertex buffer
	VkBufferCreateInfo buffer_info{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.flags = 0,
		.size = buffer_size,
		.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE };

	VK_CHECK(vkCreateBuffer(Owner->Get(), &buffer_info, nullptr, &Buffer));

	// Get memory requirements
	VkMemoryRequirements memory_requirements;
	vkGetBufferMemoryRequirements(Owner->Get(), Buffer, &memory_requirements);

	// Allocate memory for the buffer
	VkMemoryAllocateInfo alloc_info{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memory_requirements.size,
		.memoryTypeIndex = Owner->GetMemoryType(memory_requirements.memoryTypeBits,
											VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) };

	VK_CHECK(vkAllocateMemory(Owner->Get(), &alloc_info, nullptr, &Memory));

	// Bind the buffer with the allocated memory
	VK_CHECK(vkBindBufferMemory(Owner->Get(), Buffer, Memory, 0));

	UploadBuffer = VulkanUploadBuffer::CreateUnique(Device, InName, InDesc.Size);
	UploadBuffer->Map();

	UpdateSubresource(InSubresource);

#if _DEBUG
	std::string STR = std::string("VulkanIndexBuffer::") + Name.c_str();
	Device->SetResourceName(VkObjectType::VK_OBJECT_TYPE_BUFFER, reinterpret_cast<std::uint64_t>(Buffer), STR.c_str());
#endif
}

VulkanIndexBuffer::~VulkanIndexBuffer()
{
	UploadBuffer = nullptr;
	vkDestroyBuffer(Owner->Get(), Buffer, nullptr);
	vkFreeMemory(Owner->Get(), Memory, nullptr);
}

void VulkanIndexBuffer::UpdateSubresource(BufferSubresource* Subresource, IGraphicsCommandContext* InCMDBuffer)
{
	if (!Subresource)
		return;

	if (InCMDBuffer)
	{
		static_cast<VulkanCommandBuffer*>(InCMDBuffer)->UpdateBufferSubresource(this, Subresource);
	}
	else
	{
		Owner->GetIMCommandBuffer()->BeginRecordCommandList();
		Owner->GetIMCommandBuffer()->UpdateBufferSubresource(this, Subresource);
		Owner->GetIMCommandBuffer()->FinishRecordCommandList();
		Owner->GetIMCommandBuffer()->ExecuteCommandList();
	}
}

void VulkanIndexBuffer::Map(const void* Ptr, IGraphicsCommandContext* InCMDBuffer)
{
	if (!bIsMapped)
	{
		VK_CHECK(vkMapMemory(Owner->Get(), Memory, 0, Desc.Size, 0, &pData));
		bIsMapped = true;
		if (Ptr)
			memcpy(pData, Ptr, Desc.Size);
	}
	else
	{
		if (Ptr)
			memcpy(pData, Ptr, Desc.Size);
	}
}

void VulkanIndexBuffer::Unmap()
{
	if (bIsMapped)
		vkUnmapMemory(Owner->Get(), Memory);
}

VulkanUnorderedAccessBuffer::VulkanUnorderedAccessBuffer(VulkanDevice* InDevice, std::string InName, const BufferLayout& InDesc, bool bSRVAllowed)
{
}

void VulkanUnorderedAccessBuffer::Map(const void* Ptr, IGraphicsCommandContext* InCMDBuffer)
{
}
