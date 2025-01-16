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
#include "VulkanTexture.h"
#include "VulkanException.h"
#include "VulkanFormat.h"
#include "Utilities/FileManager.h"
#include "Utilities/stb_image.h"
#include "DDSTextureLoader_Vulkan.h"
#include "VulkanCommandBuffer.h"
#include "VulkanCommandBuffer.h"
#include <fstream>
#include <span>

static std::uint32_t vkBytesPerPixel(VkFormat Format)
{
	return (std::uint32_t)Vulkan_BitsPerPixel(Format) / 8;
}

VulkanTexture::VulkanTexture(VulkanDevice* Device, const std::wstring FilePath, const std::string InName, std::uint32_t InRootParameterIndex)
	: Owner(Device)
	, Name(InName)
	, Path(FilePath)
	, Desc(sTextureDesc())
	, CurrentLayout(VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	, RootParameterIndex(InRootParameterIndex)
{
	auto WideStringToString = [](const std::wstring& s) -> std::string
		{
			int len;
			int slength = (int)s.length() + 1;
			len = WideCharToMultiByte(0, 0, s.c_str(), slength, 0, 0, 0, 0);
			std::string r(len, '\0');
			WideCharToMultiByte(0, 0, s.c_str(), slength, &r[0], len, 0, 0);
			return r;
		};

	std::string str = WideStringToString(FilePath);
	std::string Ext = std::string(str.end() - 4, str.end());

	bool DDS = false;
	if (Ext.find("DDS") != std::string::npos || Ext.find("dds") != std::string::npos)
		DDS = true;

	if (DDS)
	{
		std::unique_ptr<uint8_t[]> decodedData;

		std::vector<TextureSubresource> subresources;
		HRESULT HR = Vulkan::LoadDDSTextureFromFile(Device, FilePath.c_str(), &Image, Memory, Desc,
				decodedData, subresources);

		if (HR != S_OK)
			throw std::runtime_error("DDS Texture load failed!");

		VkDeviceSize Size = Desc.Dimensions.X * Desc.Dimensions.Y * 4;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		{
			VkBufferCreateInfo bufferInfo = {};
			bufferInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = Size;// Desc.Dimensions.X* Vulkan_BitsPerPixel(ConvertFormat_Format_To_VkFormat(Desc.Format));
			bufferInfo.usage = VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			bufferInfo.sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;

			vkCreateBuffer(Device->Get(), &bufferInfo, nullptr, &stagingBuffer);

			VkMemoryRequirements memRequirements;
			// Get memory requirements for the staging buffer (alignment, memory type bits)
			vkGetBufferMemoryRequirements(Device->Get(), stagingBuffer, &memRequirements);

			// Allocate memory for the buffer
			VkMemoryAllocateInfo alloc_info{
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.allocationSize = memRequirements.size,
				.memoryTypeIndex = Owner->GetMemoryType(memRequirements.memoryTypeBits,
													VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) };

			VK_CHECK(vkAllocateMemory(Owner->Get(), &alloc_info, nullptr, &stagingBufferMemory));

			// Bind the buffer with the allocated memory
			VK_CHECK(vkBindBufferMemory(Owner->Get(), stagingBuffer, stagingBufferMemory, 0));
		}

		void* data;
		VK_CHECK(vkMapMemory(Owner->Get(), stagingBufferMemory, 0, Size, 0, &data));
		memcpy(data, subresources[0].pData, static_cast<size_t>(Size));
		vkUnmapMemory(Owner->Get(), stagingBufferMemory);

		TransitionImageLayout(Image, ConvertFormat_Format_To_VkFormat(Desc.Format), VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, Desc.MipLevels);
		//{
		//	Owner->GetIMCommandBuffer()->BeginRecordCommandList();
		//	{
		//		Owner->GetIMCommandBuffer()->TransitionTo(
		//			Image,
		//			VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED,
		//			VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		//			0,														// srcAccessMask
		//			0,                                                      // dstAccessMask
		//			VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,					// srcStage
		//			VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT                  // dstStage
		//		);
		//		CurrentLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		//	}
		//	Owner->GetIMCommandBuffer()->FinishRecordCommandList();
		//	Owner->GetIMCommandBuffer()->ExecuteWithWait();
		//}
		CopyBufferToImage(stagingBuffer, Image, static_cast<uint32_t>(Desc.Dimensions.X), static_cast<uint32_t>(Desc.Dimensions.Y));

		//transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

		vkDestroyBuffer(Device->Get(), stagingBuffer, nullptr);
		vkFreeMemory(Device->Get(), stagingBufferMemory, nullptr);

		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = Image;
		viewInfo.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = ConvertFormat_Format_To_VkFormat(Desc.Format);
		viewInfo.subresourceRange.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = Desc.MipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		vkCreateImageView(Device->Get(), &viewInfo, nullptr, &View);
	}
	else
	{
		std::string PathSTR = FileManager::WideStringToString(FilePath);

		int texWidth = 0, texHeight = 0, texChannels = 0;
		stbi_uc* pixels = stbi_load(PathSTR.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		Desc.MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
		Desc.Dimensions.X = texWidth;
		Desc.Dimensions.Y = texHeight;
		Desc.Format = EFormat::RGBA8_UNORM; // ?

		if (!pixels) {
			throw std::runtime_error("failed to load texture image!");
		}

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		{
			VkBufferCreateInfo bufferInfo = {};
			bufferInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = imageSize;
			bufferInfo.usage = VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			bufferInfo.sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;

			vkCreateBuffer(Owner->Get(), &bufferInfo, nullptr, &stagingBuffer);

			VkMemoryRequirements memRequirements;
			// Get memory requirements for the staging buffer (alignment, memory type bits)
			vkGetBufferMemoryRequirements(Owner->Get(), stagingBuffer, &memRequirements);

			// Allocate memory for the buffer
			VkMemoryAllocateInfo alloc_info{
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.allocationSize = memRequirements.size,
				.memoryTypeIndex = Owner->GetMemoryType(memRequirements.memoryTypeBits,
													VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) };

			VK_CHECK(vkAllocateMemory(Owner->Get(), &alloc_info, nullptr, &stagingBufferMemory));

			// Bind the buffer with the allocated memory
			VK_CHECK(vkBindBufferMemory(Owner->Get(), stagingBuffer, stagingBufferMemory, 0));
		}

		void* data;
		VK_CHECK(vkMapMemory(Owner->Get(), stagingBufferMemory, 0, imageSize, 0, &data));
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(Owner->Get(), stagingBufferMemory);

		stbi_image_free(pixels);

		CreateImage(Desc.Dimensions.X, Desc.Dimensions.Y, Desc.MipLevels, ConvertFormat_Format_To_VkFormat(Desc.Format), VkImageTiling::VK_IMAGE_TILING_OPTIMAL,
			VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT, VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, Image, Memory);

		TransitionImageLayout(Image, ConvertFormat_Format_To_VkFormat(Desc.Format), VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, Desc.MipLevels);
		//{
		//	Owner->GetIMCommandBuffer()->BeginRecordCommandList();
		//	{
		//		Owner->GetIMCommandBuffer()->TransitionTo(
		//			Image,
		//			VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED,
		//			VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		//			0,														// srcAccessMask
		//			0,                                                      // dstAccessMask
		//			VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,					// srcStage
		//			VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT                  // dstStage
		//		);
		//		CurrentLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		//	}
		//	Owner->GetIMCommandBuffer()->FinishRecordCommandList();
		//	Owner->GetIMCommandBuffer()->ExecuteWithWait();
		//}
		CopyBufferToImage(stagingBuffer, Image, static_cast<uint32_t>(Desc.Dimensions.X), static_cast<uint32_t>(Desc.Dimensions.Y));

		//transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

		vkDestroyBuffer(Owner->Get(), stagingBuffer, nullptr);
		vkFreeMemory(Owner->Get(), stagingBufferMemory, nullptr);

		GenerateMipmaps(Image, ConvertFormat_Format_To_VkFormat(Desc.Format), Desc.Dimensions.X, Desc.Dimensions.Y, Desc.MipLevels);

		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = Image;
		viewInfo.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = ConvertFormat_Format_To_VkFormat(Desc.Format);
		viewInfo.subresourceRange.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = Desc.MipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		vkCreateImageView(Owner->Get(), &viewInfo, nullptr, &View);
	}

	{
		DescriptorImageInfo.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		DescriptorImageInfo.imageView = View;
		VulkanSamplerState Sampler = VulkanSamplerState(sSamplerAttributeDesc(ESamplerStateMode::eAnisotropicWrap));
		DescriptorImageInfo.sampler = Sampler.Get(Owner->Get());
	}

	{
		std::string STR = std::string("VulkanTexture::") + Name.c_str();
		Device->SetResourceName(VkObjectType::VK_OBJECT_TYPE_IMAGE, reinterpret_cast<std::uint64_t>(Image), STR.c_str());
	}
}

VulkanTexture::VulkanTexture(VulkanDevice* Device, const std::string InName, void* InBuffer, const std::size_t __InSize, const sTextureDesc& InDesc, std::uint32_t InRootParameterIndex)
	: Owner(Device)
	, Name(InName)
	, Path(L"")
	, Desc(InDesc)
	, RootParameterIndex(InRootParameterIndex)
{
	Desc.MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(InDesc.Dimensions.X, InDesc.Dimensions.Y)))) + 1;

	VkDeviceSize InSize = InDesc.Dimensions.X * InDesc.Dimensions.Y * 4;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	{
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = InDesc.Dimensions.X * InDesc.Dimensions.Y; //Vulkan_BitsPerPixel(ConvertFormat_Format_To_VkFormat(InDesc.Format));
		bufferInfo.usage = VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;

		vkCreateBuffer(Device->Get(), &bufferInfo, nullptr, &stagingBuffer);

		VkMemoryRequirements memRequirements;
		// Get memory requirements for the staging buffer (alignment, memory type bits)
		vkGetBufferMemoryRequirements(Device->Get(), stagingBuffer, &memRequirements);

		// Allocate memory for the buffer
		VkMemoryAllocateInfo alloc_info{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = Owner->GetMemoryType(memRequirements.memoryTypeBits,
												VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) };

		VK_CHECK(vkAllocateMemory(Owner->Get(), &alloc_info, nullptr, &stagingBufferMemory));

		// Bind the buffer with the allocated memory
		VK_CHECK(vkBindBufferMemory(Owner->Get(), stagingBuffer, stagingBufferMemory, 0));
	}

	void* data;
	VK_CHECK(vkMapMemory(Owner->Get(), stagingBufferMemory, 0, InDesc.Dimensions.X * InDesc.Dimensions.Y, 0, &data));
	memcpy(data, InBuffer, static_cast<size_t>(InDesc.Dimensions.X * InDesc.Dimensions.Y/* Vulkan_BitsPerPixel(ConvertFormat_Format_To_VkFormat(InDesc.Format))*/));
	vkUnmapMemory(Owner->Get(), stagingBufferMemory);

	CreateImage(Desc.Dimensions.X, Desc.Dimensions.Y, Desc.MipLevels, ConvertFormat_Format_To_VkFormat(InDesc.Format), VkImageTiling::VK_IMAGE_TILING_OPTIMAL,
		VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT, VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, Image, Memory);

	TransitionImageLayout(Image, ConvertFormat_Format_To_VkFormat(InDesc.Format), VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, Desc.MipLevels);
	//{
	//	Owner->GetIMCommandBuffer()->BeginRecordCommandList();
	//	{
	//		Owner->GetIMCommandBuffer()->TransitionTo(
	//			Image,
	//			VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED,
	//			VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	//			0,														// srcAccessMask
	//			0,                                                      // dstAccessMask
	//			VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,					// srcStage
	//			VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT                  // dstStage
	//		);
	//		CurrentLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	//	}
	//	Owner->GetIMCommandBuffer()->FinishRecordCommandList();
	//	Owner->GetIMCommandBuffer()->ExecuteWithWait();
	//}
	CopyBufferToImage(stagingBuffer, Image, static_cast<uint32_t>(Desc.Dimensions.X), static_cast<uint32_t>(Desc.Dimensions.Y));

	//transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

	vkDestroyBuffer(Device->Get(), stagingBuffer, nullptr);
	vkFreeMemory(Device->Get(), stagingBufferMemory, nullptr);

	GenerateMipmaps(Image, ConvertFormat_Format_To_VkFormat(InDesc.Format), Desc.Dimensions.X, Desc.Dimensions.Y, Desc.MipLevels);

	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = Image;
	viewInfo.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = ConvertFormat_Format_To_VkFormat(InDesc.Format);
	viewInfo.subresourceRange.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = Desc.MipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	vkCreateImageView(Device->Get(), &viewInfo, nullptr, &View);

	{
		DescriptorImageInfo.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		DescriptorImageInfo.imageView = View;
		VulkanSamplerState Sampler = VulkanSamplerState(sSamplerAttributeDesc(ESamplerStateMode::eAnisotropicWrap));
		DescriptorImageInfo.sampler = Sampler.Get(Owner->Get());
	}

	{
		std::string STR = std::string("VulkanTexture::") + Name.c_str();
		Device->SetResourceName(VkObjectType::VK_OBJECT_TYPE_IMAGE, reinterpret_cast<std::uint64_t>(Image), STR.c_str());
	}
}

VulkanTexture::VulkanTexture(VulkanDevice* Device, const std::string InName, const sTextureDesc& InDesc, std::uint32_t InRootParameterIndex)
	: Owner(Device)
	, Name(InName)
	, Path(L"")
	, Desc(InDesc)
	, RootParameterIndex(InRootParameterIndex)
{
	Desc.MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(InDesc.Dimensions.X, InDesc.Dimensions.Y)))) + 1;

	CreateImage(Desc.Dimensions.X, Desc.Dimensions.Y, Desc.MipLevels, ConvertFormat_Format_To_VkFormat(InDesc.Format), VkImageTiling::VK_IMAGE_TILING_OPTIMAL,
		VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT, VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, Image, Memory);

	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = Image;
	viewInfo.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = ConvertFormat_Format_To_VkFormat(InDesc.Format);
	viewInfo.subresourceRange.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = Desc.MipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	vkCreateImageView(Device->Get(), &viewInfo, nullptr, &View);

	TransitionImageLayout(Image, ConvertFormat_Format_To_VkFormat(InDesc.Format), VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, Desc.MipLevels);
	TransitionImageLayout(Image, ConvertFormat_Format_To_VkFormat(InDesc.Format), VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, Desc.MipLevels);
	//{
	//	Owner->GetIMCommandBuffer()->BeginRecordCommandList();
	//	{
	//		Owner->GetIMCommandBuffer()->TransitionTo(
	//			Image,
	//			VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED,
	//			VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	//			0,														// srcAccessMask
	//			0,                                                      // dstAccessMask
	//			VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,					// srcStage
	//			VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT                  // dstStage
	//		);
	//		CurrentLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	//	}
	//	Owner->GetIMCommandBuffer()->FinishRecordCommandList();
	//	Owner->GetIMCommandBuffer()->ExecuteWithWait();
	//}
	{
		DescriptorImageInfo.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		DescriptorImageInfo.imageView = View;
		VulkanSamplerState Sampler = VulkanSamplerState(sSamplerAttributeDesc(ESamplerStateMode::eAnisotropicWrap));
		DescriptorImageInfo.sampler = Sampler.Get(Owner->Get());
	}

	{
		std::string STR = std::string("VulkanTexture::") + Name.c_str();
		Device->SetResourceName(VkObjectType::VK_OBJECT_TYPE_IMAGE, reinterpret_cast<std::uint64_t>(Image), STR.c_str());
	}
}

void VulkanTexture::UpdateTexture(ITexture2D* SourceTexture, std::size_t SourceArrayIndex, std::size_t ArrayIndex, const std::optional<IntVector2> Dest, const std::optional<FBounds2D> TargetBounds)
{
	VulkanTexture* Texure = static_cast<VulkanTexture*>(SourceTexture);
	{
		Owner->GetIMCommandBuffer()->BeginRecordCommandList();
		Owner->GetIMCommandBuffer()->TransitionTo(
			Image,
			CurrentLayout,
			VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			0,														// srcAccessMask
			0,                                                      // dstAccessMask
			VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,					// srcStage
			VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT                  // dstStage
		);
		CurrentLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		Owner->GetIMCommandBuffer()->FinishRecordCommandList();
		Owner->GetIMCommandBuffer()->ExecuteWithWait();
	}
	{
		Owner->GetIMCommandBuffer()->BeginRecordCommandList();
		Owner->GetIMCommandBuffer()->TransitionTo(
			Texure->Image,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VkImageLayout::VK_IMAGE_LAYOUT_GENERAL,
			0,														// srcAccessMask
			0,                                                      // dstAccessMask
			VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,					// srcStage
			VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT                  // dstStage
		);
		Texure->CurrentLayout = VK_IMAGE_LAYOUT_GENERAL;
		Owner->GetIMCommandBuffer()->FinishRecordCommandList();
		Owner->GetIMCommandBuffer()->ExecuteWithWait();
	}
	VkImageCopy Copy = {};
	Copy.dstOffset.x = 0;
	Copy.dstOffset.y = 0;
	Copy.dstOffset.z = 0;
	Copy.dstSubresource.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
	Copy.dstSubresource.baseArrayLayer = 0;
	Copy.dstSubresource.layerCount = 1;
	Copy.dstSubresource.mipLevel = 0;
	Copy.extent.width = TargetBounds.has_value() ? TargetBounds->GetWidth() : Texure->Desc.Dimensions.X;
	Copy.extent.height = TargetBounds.has_value() ? TargetBounds->GetHeight() : Texure->Desc.Dimensions.Y;
	Copy.extent.depth = 1;
	Copy.srcOffset.x = TargetBounds.has_value() ? TargetBounds.value().Min.X : 0;
	Copy.srcOffset.y = TargetBounds.has_value() ? TargetBounds.value().Min.Y : 0;
	Copy.srcOffset.z = 0;
	Copy.srcSubresource.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
	Copy.srcSubresource.baseArrayLayer = 0;
	Copy.srcSubresource.layerCount = 1;
	Copy.srcSubresource.mipLevel = 0;

	Owner->GetIMCommandBuffer()->BeginRecordCommandList();
	vkCmdCopyImage(Owner->GetIMCommandBuffer()->Get(), Texure->Image, Texure->CurrentLayout, Image, CurrentLayout, 1, &Copy);
	Owner->GetIMCommandBuffer()->FinishRecordCommandList();
	Owner->GetIMCommandBuffer()->ExecuteWithWait();

	{
		Owner->GetIMCommandBuffer()->BeginRecordCommandList();
		Owner->GetIMCommandBuffer()->TransitionTo(
			Image,
			VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			0,														// srcAccessMask
			0,                                                      // dstAccessMask
			VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,					// srcStage
			VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT                  // dstStage
		);
		CurrentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		Owner->GetIMCommandBuffer()->FinishRecordCommandList();
		Owner->GetIMCommandBuffer()->ExecuteWithWait();
	}
	{
		Owner->GetIMCommandBuffer()->BeginRecordCommandList();
		Owner->GetIMCommandBuffer()->TransitionTo(
			Texure->Image,
			VkImageLayout::VK_IMAGE_LAYOUT_GENERAL,
			VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			0,														// srcAccessMask
			0,                                                      // dstAccessMask
			VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,					// srcStage
			VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT                  // dstStage
		);
		Texure->CurrentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		Owner->GetIMCommandBuffer()->FinishRecordCommandList();
		Owner->GetIMCommandBuffer()->ExecuteWithWait();
	}
}

void VulkanTexture::UpdateTexture(const std::wstring FilePath, std::size_t ArrayIndex, const std::optional<IntVector2> Dest, const std::optional<FBounds2D> TargetBounds)
{
	std::cout << "VulkanTexture::UpdateTexture_Unimplemented(FilePath, ArrayIndex, Dest, TargetBounds)" << std::endl;
}

void VulkanTexture::UpdateTexture(const void* pSrcData, const std::size_t InSize, const FDimension2D& Dimension, std::size_t ArrayIndex, const std::optional<IntVector2>  Dest, const std::optional<FBounds2D> TargetBounds)
{
	std::cout << "VulkanTexture::UpdateTexture_Unimplemented(pSrcData, InSize, Dimension, ArrayIndex, Dest, TargetBounds)" << std::endl;
}

void VulkanTexture::UpdateTexture(const void* pSrcData, std::size_t RowPitch, std::size_t MinX, std::size_t MinY, std::size_t MaxX, std::size_t MaxY, IGraphicsCommandContext* InCommandBuffer)
{
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	std::vector<TextureSubresource> subresources;
	TextureSubresource Subresource;
	Subresource.pData = const_cast<void*>(pSrcData);
	Subresource.RowPitch = RowPitch;
	Subresource.SlicePitch = 0;
	subresources.push_back(Subresource);

	VkDeviceSize Size = (MaxX - MinX) * (MaxY - MinY);

	{
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = Size;// Desc.Dimensions.X* Vulkan_BitsPerPixel(ConvertFormat_Format_To_VkFormat(Desc.Format));
		bufferInfo.usage = VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;

		vkCreateBuffer(Owner->Get(), &bufferInfo, nullptr, &stagingBuffer);

		VkMemoryRequirements memRequirements;
		// Get memory requirements for the staging buffer (alignment, memory type bits)
		vkGetBufferMemoryRequirements(Owner->Get(), stagingBuffer, &memRequirements);

		// Allocate memory for the buffer
		VkMemoryAllocateInfo alloc_info{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = Owner->GetMemoryType(memRequirements.memoryTypeBits,
												VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) };

		VK_CHECK(vkAllocateMemory(Owner->Get(), &alloc_info, nullptr, &stagingBufferMemory));

		// Bind the buffer with the allocated memory
		VK_CHECK(vkBindBufferMemory(Owner->Get(), stagingBuffer, stagingBufferMemory, 0));
	}

	void* data;
	VK_CHECK(vkMapMemory(Owner->Get(), stagingBufferMemory, 0, Size, 0, &data));
	memcpy(data, subresources[0].pData, static_cast<size_t>(Size));
	vkUnmapMemory(Owner->Get(), stagingBufferMemory);

	TransitionImageLayout(Image, ConvertFormat_Format_To_VkFormat(Desc.Format), VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, Desc.MipLevels);
	//if (CurrentLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL || CurrentLayout == VK_IMAGE_LAYOUT_UNDEFINED)
	//{
	//	Owner->GetIMCommandBuffer()->BeginRecordCommandList();
	//	{
	//		Owner->GetIMCommandBuffer()->TransitionTo(
	//			Image,
	//			VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED,
	//			VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	//			0,														// srcAccessMask
	//			0,                                                      // dstAccessMask
	//			VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,					// srcStage
	//			VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT                  // dstStage
	//		);
	//		CurrentLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	//	}
	//	Owner->GetIMCommandBuffer()->FinishRecordCommandList();
	//	Owner->GetIMCommandBuffer()->ExecuteWithWait();
	//}

	CopyBufferToImage(stagingBuffer, Image, MaxX - MinX, MaxY - MinY, MinX, MinY);

	//if (CurrentLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	//{
	//	Owner->GetIMCommandBuffer()->BeginRecordCommandList();
	//	{
	//		Owner->GetIMCommandBuffer()->TransitionTo(
	//			Image,
	//			CurrentLayout,
	//			VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	//			0,														// srcAccessMask
	//			0,                                                      // dstAccessMask
	//			VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,					// srcStage
	//			VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT                  // dstStage
	//		);
	//		CurrentLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	//	}
	//	Owner->GetIMCommandBuffer()->FinishRecordCommandList();
	//	Owner->GetIMCommandBuffer()->ExecuteWithWait();
	//}

	//transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

	vkDestroyBuffer(Owner->Get(), stagingBuffer, nullptr);
	vkFreeMemory(Owner->Get(), stagingBufferMemory, nullptr);
}

void VulkanTexture::SaveToFile(std::wstring InPath) const
{
}

void VulkanTexture::GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
{
	VkCommandBuffer commandBuffer = Owner->GetIMCommandBuffer()->Get();

	VkCommandBufferBeginInfo cmdBufInfo = {};
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkBeginCommandBuffer(commandBuffer, &cmdBufInfo);

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;

	for (uint32_t i = 1; i < mipLevels; i++)
	{
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT, VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &barrier);

		VkImageBlit blit = {};
		blit.srcOffsets[0] = VkOffset3D(0, 0, 0);
		blit.srcOffsets[1] = VkOffset3D(mipWidth, mipHeight, 1);
		blit.srcSubresource.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = VkOffset3D(0, 0, 0);
		blit.dstOffsets[1] = VkOffset3D(mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1);
		blit.dstSubresource.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(commandBuffer, image, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VkFilter::VK_FILTER_LINEAR);

		barrier.oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &barrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &barrier);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	// Create fence to ensure that the command buffer has finished executing
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	VkFence fence;
	vkCreateFence(Owner->Get(), &fenceInfo, nullptr, &fence);

	// Submit to the queue
	vkQueueSubmit(Owner->GetQueue(), 1, &submitInfo, fence);

	// Wait for the fence to signal that command buffer has finished executing
	vkWaitForFences(Owner->Get(), 1, &fence, VK_TRUE, 1000000000);
	vkDestroyFence(Owner->Get(), fence, nullptr);
}

void VulkanTexture::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
{
	VkCommandBuffer commandBuffer = Owner->GetIMCommandBuffer()->Get();

	VkCommandBufferBeginInfo cmdBufInfo = {};
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkBeginCommandBuffer(commandBuffer, &cmdBufInfo);

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
	{
		barrier.srcAccessMask = 0;// VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
	{
		barrier.srcAccessMask = 0;// VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &barrier);

	CurrentLayout = newLayout;

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	// Create fence to ensure that the command buffer has finished executing
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	VkFence fence;
	vkCreateFence(Owner->Get(), &fenceInfo, nullptr, &fence);

	// Submit to the queue
	vkQueueSubmit(Owner->GetQueue(), 1, &submitInfo, fence);

	// Wait for the fence to signal that command buffer has finished executing
	vkWaitForFences(Owner->Get(), 1, &fence, VK_TRUE, 1000000000);
	vkDestroyFence(Owner->Get(), fence, nullptr);
}

void VulkanTexture::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, std::int32_t OffsetX, std::int32_t OffsetY)
{
	VkCommandBuffer commandBuffer = Owner->GetIMCommandBuffer()->Get();

	VkCommandBufferBeginInfo cmdBufInfo = {};
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkBeginCommandBuffer(commandBuffer, &cmdBufInfo);

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { OffsetX, OffsetY, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	// Create fence to ensure that the command buffer has finished executing
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	VkFence fence;
	vkCreateFence(Owner->Get(), &fenceInfo, nullptr, &fence);

	// Submit to the queue
	vkQueueSubmit(Owner->GetQueue(), 1, &submitInfo, fence);

	// Wait for the fence to signal that command buffer has finished executing
	vkWaitForFences(Owner->Get(), 1, &fence, VK_TRUE, 1000000000);
	vkDestroyFence(Owner->Get(), fence, nullptr);
}

void VulkanTexture::CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VkImageType::VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = mipLevels;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;

	vkCreateImage(Owner->Get(), &imageInfo, nullptr, &image);

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(Owner->Get(), image, &memRequirements);

	// Allocate memory for the buffer
	VkMemoryAllocateInfo alloc_info{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = Owner->GetMemoryType(memRequirements.memoryTypeBits,	properties) };

	VK_CHECK(vkAllocateMemory(Owner->Get(), &alloc_info, nullptr, &imageMemory));

	vkBindImageMemory(Owner->Get(), image, imageMemory, 0);
}
