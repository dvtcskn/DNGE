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

#include "pch.h"
#include "VulkanFrameBuffer.h"
#include "VulkanException.h"
#include "VulkanCommandBuffer.h"
#include "VulkanFormat.h"

VulkanRenderTarget::VulkanRenderTarget(VulkanDevice* InOwner, const std::string InName, const EFormat InFormat, const sFBODesc& InDesc, bool InIsSRVAllowed, bool InIsUnorderedAccessAllowed)
	: Super()
	, Name(InName)
	, Format(InFormat)
	, Desc(InDesc)
	, bIsSRVSupported(InIsSRVAllowed)
	, bIsUAVSupported(InIsUnorderedAccessAllowed)
	, CurrentLayout(VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED)
{
	VkFormat VulkanFormat = ConvertFormat_Format_To_VkFormat(Format);

	// Create VkImage
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent = { Desc.Dimensions.X, Desc.Dimensions.Y, 1 };
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = VulkanFormat;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL; // Optimal tiling for GPU access
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Will transition later
	imageInfo.usage = InIsSRVAllowed ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
		: VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT; // As render target and texture
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; // No MSAA
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(InOwner->Get(), &imageInfo, nullptr, &Image) != VK_SUCCESS)
		throw std::runtime_error("Failed to create image!");

	// Allocate memory for VkImage
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(InOwner->Get(), Image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = InOwner->GetMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(InOwner->Get(), &allocInfo, nullptr, &Memory) != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate image memory!");

	vkBindImageMemory(InOwner->Get(), Image, Memory, 0);

	// Create VkImageView
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = Image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // Assuming a 2D image
	viewInfo.format = VulkanFormat;

	// Specify how the image will be used
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;
	
	if (vkCreateImageView(InOwner->Get(), &viewInfo, nullptr, &View) != VK_SUCCESS)
		throw std::runtime_error("Failed to create image view!");

	if (bIsSRVSupported)
	{
		{
			DescriptorImageInfo.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			DescriptorImageInfo.imageView = View;
			VulkanSamplerState Sampler = VulkanSamplerState(sSamplerAttributeDesc(ESamplerStateMode::eAnisotropicWrap));
			DescriptorImageInfo.sampler = Sampler.Get(InOwner->Get());
		}
	}

	if (bIsUAVSupported)
	{
		// Create VkImageView
		VkImageViewCreateInfo UAV_viewInfo{};
		UAV_viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		UAV_viewInfo.image = Image;
		UAV_viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // Adjust for 1D, 3D, or cubemap
		UAV_viewInfo.format = VulkanFormat;
		UAV_viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		UAV_viewInfo.subresourceRange.baseMipLevel = 0;
		UAV_viewInfo.subresourceRange.levelCount = 1;
		UAV_viewInfo.subresourceRange.baseArrayLayer = 0;
		UAV_viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(InOwner->Get(), &UAV_viewInfo, nullptr, &UAV_View) != VK_SUCCESS)
			throw std::runtime_error("Failed to create UAV image view!");
	}

	{
		std::string STR = std::string("VulkanRenderTarget::") + Name.c_str();
		InOwner->SetResourceName(VkObjectType::VK_OBJECT_TYPE_IMAGE, reinterpret_cast<std::uint64_t>(Image), STR.c_str());
	}
}

VulkanRenderTarget::~VulkanRenderTarget()
{
}

VulkanDepthTarget::VulkanDepthTarget(VulkanDevice* InOwner, const std::string InName, const EFormat InFormat, const sFBODesc& InDesc)
	: Name(InName)
	, Format(InFormat)
	, Desc(InDesc)
	, bIsSRVSupported(false)
	, Image(VK_NULL_HANDLE)
	, Memory(VK_NULL_HANDLE)
	, View(VK_NULL_HANDLE)
	, CurrentLayout(VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED)
{
	bIsSRVSupported = IsDepthSRVSupported(Format);

	/*
	* Format: VK_FORMAT_D32_SFLOAT_S8_UINT
	* Usage : VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
	*/

	VkFormat DepthFormat = ConvertFormat_Format_To_VkFormat(Format);

	if (!IsValidVulkanDepthFormat(DepthFormat))
		throw std::runtime_error("Invalid Depth Format");

	VkImageCreateInfo image_create_info{};
	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_create_info.imageType = VK_IMAGE_TYPE_2D;
	image_create_info.format = DepthFormat;
	image_create_info.extent = { Desc.Dimensions.X, Desc.Dimensions.Y, 1 };
	image_create_info.mipLevels = 1;
	image_create_info.arrayLayers = 1;
	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_CHECK(vkCreateImage(InOwner->Get(), &image_create_info, nullptr, &Image));
	VkMemoryRequirements memReqs{};
	vkGetImageMemoryRequirements(InOwner->Get(), Image, &memReqs);

	VkMemoryAllocateInfo memory_allocation{};
	memory_allocation.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_allocation.allocationSize = memReqs.size;
	memory_allocation.memoryTypeIndex = InOwner->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK(vkAllocateMemory(InOwner->Get(), &memory_allocation, nullptr, &Memory));
	VK_CHECK(vkBindImageMemory(InOwner->Get(), Image, Memory, 0));

	VkImageViewCreateInfo image_view_create_info{};
	image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	image_view_create_info.image = Image;
	image_view_create_info.format = DepthFormat;
	image_view_create_info.subresourceRange.baseMipLevel = 0;
	image_view_create_info.subresourceRange.levelCount = 1;
	image_view_create_info.subresourceRange.baseArrayLayer = 0;
	image_view_create_info.subresourceRange.layerCount = 1;
	image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

	// Stencil aspect should only be set on depth + stencil formats (VK_FORMAT_D16_UNORM_S8_UINT..VK_FORMAT_D32_SFLOAT_S8_UINT
	if (DepthFormat >= VK_FORMAT_D16_UNORM_S8_UINT)
	{
		image_view_create_info.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	VK_CHECK(vkCreateImageView(InOwner->Get(), &image_view_create_info, nullptr, &View));

	if (IsVulkanDepthSRVSupported(DepthFormat))
	{

	}

	{
		std::string STR = std::string("VulkanDepthTarget::") + Name.c_str();
		InOwner->SetResourceName(VkObjectType::VK_OBJECT_TYPE_IMAGE, reinterpret_cast<std::uint64_t>(Image), STR.c_str());
	}
}

VulkanDepthTarget::~VulkanDepthTarget()
{
}

VulkanUnorderedAccessTarget::VulkanUnorderedAccessTarget(VulkanDevice* InOwner, const std::string InName, const EFormat InFormat, const sFBODesc& InDesc, bool InEnableSRV)
	: Super()
	, Name(InName)
	, Format(InFormat)
	, Desc(InDesc)
	, bIsSRVSupported(InEnableSRV)
	, CurrentLayout(VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED)
{
	VkFormat VulkanFormat = ConvertFormat_Format_To_VkFormat(Format);

	// Create VkImage
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D; // Adjust for 1D, 3D if necessary
	imageInfo.extent = { Desc.Dimensions.X, Desc.Dimensions.Y, 1 };
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = VulkanFormat;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT; // Storage and sampling
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(InOwner->Get(), &imageInfo, nullptr, &Image) != VK_SUCCESS)
		throw std::runtime_error("Failed to create UAV image!");

	// Allocate memory for VkImage
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(InOwner->Get(), Image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = InOwner->GetMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(InOwner->Get(), &allocInfo, nullptr, &Memory) != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate UAV image memory!");

	vkBindImageMemory(InOwner->Get(), Image, Memory, 0);

	// Create VkImageView
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = Image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // Adjust for 1D, 3D, or cubemap
	viewInfo.format = VulkanFormat;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(InOwner->Get(), &viewInfo, nullptr, &View) != VK_SUCCESS)
		throw std::runtime_error("Failed to create UAV image view!");

	if (bIsSRVSupported)
	{
	}

	{
		std::string STR = std::string("VulkanUnorderedAccessTarget::") + Name.c_str();
		InOwner->SetResourceName(VkObjectType::VK_OBJECT_TYPE_IMAGE, reinterpret_cast<std::uint64_t>(Image), STR.c_str());
	}
}

VulkanUnorderedAccessTarget::~VulkanUnorderedAccessTarget()
{
}

VulkanFrameBuffer::VulkanFrameBuffer(VulkanDevice* InOwner, const std::string InName, const sFrameBufferAttachmentInfo& InAttachments)
	: Owner(InOwner)
	, AttachmentInfo(InAttachments)
	, Name(InName)
{
	const auto& FDesc = AttachmentInfo.Desc;

	for (std::size_t i = 0; i < AttachmentInfo.FrameBuffer.size(); i++)
	{
		const auto& FB = AttachmentInfo.FrameBuffer[i];

		if (FB.AttachmentType == eFrameBufferAttachmentType::eUAV)
		{
			UnorderedAccessTargets.push_back(VulkanUnorderedAccessTarget::Create(Owner, Name + "_UAV_" + std::to_string(UnorderedAccessTargets.size()), FB.Format, FDesc, false));
		}
		else if (FB.AttachmentType == eFrameBufferAttachmentType::eUAV_SRV)
		{
			UnorderedAccessTargets.push_back(VulkanUnorderedAccessTarget::Create(Owner, Name + "_UAV_" + std::to_string(UnorderedAccessTargets.size()), FB.Format, FDesc, true));
		}
		else if (FB.AttachmentType == eFrameBufferAttachmentType::eRT)
		{
			RenderTargets.push_back(VulkanRenderTarget::Create(Owner, Name + "_RenderTarget_" + std::to_string(RenderTargets.size()), FB.Format, FDesc, false, false));
		}
		else if (FB.AttachmentType == eFrameBufferAttachmentType::eRT_SRV)
		{
			RenderTargets.push_back(VulkanRenderTarget::Create(Owner, Name + "_RenderTarget_" + std::to_string(RenderTargets.size()), FB.Format, FDesc, true, false));
		}
		else if (FB.AttachmentType == eFrameBufferAttachmentType::eRT_UAV)
		{
			RenderTargets.push_back(VulkanRenderTarget::Create(Owner, Name + "_RenderTarget_" + std::to_string(RenderTargets.size()), FB.Format, FDesc, false, true));
		}
		else if (FB.AttachmentType == eFrameBufferAttachmentType::eRT_SRV_UAV)
		{
			RenderTargets.push_back(VulkanRenderTarget::Create(Owner, Name + "_RenderTarget_" + std::to_string(RenderTargets.size()), FB.Format, FDesc, true, true));
		}
		else if (FB.AttachmentType == eFrameBufferAttachmentType::eDepth)
		{

		}
		else if (FB.AttachmentType == eFrameBufferAttachmentType::eDepth_SRV)
		{

		}
		else if (FB.AttachmentType == eFrameBufferAttachmentType::eDepth_UAV)
		{

		}
		else if (FB.AttachmentType == eFrameBufferAttachmentType::eDepth_SRV_UAV)
		{

		}
	}

	if (IsValidDepthFormat(AttachmentInfo.DepthFormat))
	{
		DepthTarget = VulkanDepthTarget::Create(Owner, Name + "_DepthTarget", AttachmentInfo.DepthFormat, FDesc);
	}

	/*std::vector<VkImageView> attachments;
	for (auto& FrameBufferAttachment : RenderTargets)
	{
		attachments.push_back(FrameBufferAttachment->GetView());
		//if (FrameBufferAttachment->IsUAV_Allowed())
		//	attachments.push_back(FrameBufferAttachment->GetUAV_View());
	}
	for (auto& FrameBufferAttachment : UnorderedAccessTargets)
	{
		attachments.push_back(FrameBufferAttachment->GetView());
	}
	attachments.push_back(DepthTarget->GetView());

	RenderPass = VulkanRenderpass::Create(Owner, Name + "_RenderPass", GetAttachmentInfo());

	VkFramebufferCreateInfo FramebufferCreateInfo = {};
	FramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	FramebufferCreateInfo.pNext = NULL;
	FramebufferCreateInfo.renderPass = RenderPass->Get();
	FramebufferCreateInfo.pAttachments = attachments.data();
	FramebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	FramebufferCreateInfo.width = GetAttachmentInfo().Desc.Dimensions.X;
	FramebufferCreateInfo.height = GetAttachmentInfo().Desc.Dimensions.Y;
	FramebufferCreateInfo.layers = 1;
	FramebufferCreateInfo.flags = 0;// VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR;

	vkCreateFramebuffer(Owner->Get(), &FramebufferCreateInfo, nullptr, &FrameBuffer);*/
}

void VulkanFrameBuffer::AttachRenderTarget(const IRenderTarget::SharedPtr& RenderTarget, std::optional<std::size_t> Index)
{
	if (auto RT = std::dynamic_pointer_cast<VulkanRenderTarget>(RenderTarget))
	{
		eFrameBufferAttachmentType AttachmentType;
		if (!RT->IsSRV_Allowed() && !RT->IsUAV_Allowed())
		{
			AttachmentType = eFrameBufferAttachmentType::eRT;
		}
		else if (RT->IsSRV_Allowed() && !RT->IsUAV_Allowed())
		{
			AttachmentType = eFrameBufferAttachmentType::eRT_SRV;
		}
		else if (!RT->IsSRV_Allowed() && RT->IsUAV_Allowed())
		{
			AttachmentType = eFrameBufferAttachmentType::eRT_UAV;
		}
		else if (RT->IsSRV_Allowed() && RT->IsUAV_Allowed())
		{
			AttachmentType = eFrameBufferAttachmentType::eRT_SRV_UAV;
		}

		if (Index.has_value())
			RenderTargets.insert(RenderTargets.begin() + Index.value(), RT);
		else
			RenderTargets.push_back(RT);

		if (Index.has_value())
			AttachmentInfo.FrameBuffer.insert(AttachmentInfo.FrameBuffer.begin() + Index.value(), sFrameBufferAttachmentInfo::sFrameBuffer(RT->GetFormat(), AttachmentType));
		else
			AttachmentInfo.AddFrameBuffer(RT->GetFormat(), AttachmentType);
	}
}

void VulkanFrameBuffer::AttachUnorderedAccessTarget(const IUnorderedAccessTarget::SharedPtr& UnorderedAccessTarget, std::optional<std::size_t> Index)
{
	if (auto ST = std::dynamic_pointer_cast<VulkanUnorderedAccessTarget>(UnorderedAccessTarget))
	{
		eFrameBufferAttachmentType AttachmentType;
		if (!ST->IsSRV_Allowed())
		{
			AttachmentType = eFrameBufferAttachmentType::eUAV;
		}
		else if (ST->IsSRV_Allowed())
		{
			AttachmentType = eFrameBufferAttachmentType::eUAV_SRV;
		}

		if (Index.has_value())
			UnorderedAccessTargets.insert(UnorderedAccessTargets.begin() + Index.value(), ST);
		else
			UnorderedAccessTargets.push_back(ST);

		if (Index.has_value())
			AttachmentInfo.FrameBuffer.insert(AttachmentInfo.FrameBuffer.begin() + Index.value(), sFrameBufferAttachmentInfo::sFrameBuffer(ST->GetFormat(), AttachmentType));
		else
			AttachmentInfo.AddFrameBuffer(ST->GetFormat(), AttachmentType);
	}
}

void VulkanFrameBuffer::SetDepthTarget(const IDepthTarget::SharedPtr& InDepthTarget)
{
	if (auto DT = std::dynamic_pointer_cast<VulkanDepthTarget>(InDepthTarget))
	{
		DepthTarget = nullptr;
		DepthTarget = DT;
		AttachmentInfo.DepthFormat = DepthTarget->GetFormat();
	}
}
