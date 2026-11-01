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
#include "VulkanCommandBuffer.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanFrameBuffer.h"
#include "VulkanException.h"

#ifndef PROFILE
#define PROFILE 0
#endif

VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice* InOwner)
	: Owner(InOwner)
	, StencilRef(0)
	, bWaitForCompletion(false)
	, bIsRenderPassActive(false)
	, State(ECommandContextState::Waiting)
{
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
	Owner->CancelPendingCommandBuffer(this);
	WaitForGPU();
	Pipeline = nullptr;
	Sets.clear();
	if (CommandBuffer)
		Owner->ReturnCommandBuffer(GetCommandContextType(), Get());
	CommandBuffer = nullptr;

	ClearState();
	Owner = nullptr;
}

void VulkanCommandBuffer::SetName(std::string Name)
{

}

ECommandContextBeginResult VulkanCommandBuffer::BeginRecordCommandList(const ECommandContextBeginState Begin)
{
	if (Owner->IsCommandBufferPendingForExecute(this))
		return ECommandContextBeginResult::Failed_WaitingForExecute;

	bool bRestarted = false;
	if (State == ECommandContextState::End)
	{
		Engine::WriteToConsole("WARNING : VulkanCommandBuffer is recording again. Previous records will be deleted!");
		bRestarted = true;
	}
	if (State == ECommandContextState::Begin)
	{
		if (bIsRenderPassActive)
		{
			vkCmdEndRendering(CommandBuffer);
			//vkCmdEndRenderPass(CommandBuffer);
			bIsRenderPassActive = false;
		}
		return ECommandContextBeginResult::AlreadyStarted;
	}
	if (State == ECommandContextState::WaitingForExecute)
		return ECommandContextBeginResult::Failed_WaitingForExecute;

	Sets.clear();

	Open();

	if (Begin == ECommandContextBeginState::Render)
	{

	}
	else if (Begin == ECommandContextBeginState::ApiRender)
	{
	}
	else
	{
		//Owner->CPUWait();
	}

	return bRestarted ? ECommandContextBeginResult::Restarted : ECommandContextBeginResult::Started;
}

void VulkanCommandBuffer::Open()
{
	if (!CommandBuffer)
	{
		CommandBuffer = Owner->RequestCommandBuffer(GetCommandContextType());
	}
	else
	{
		const VkResult resetResult = vkResetCommandBuffer(CommandBuffer, 0);
		if (resetResult != VK_SUCCESS)
			throw std::runtime_error("vkResetCommandBuffer failed with VkResult " + std::to_string(resetResult));
	}

	auto beginInfo = vk::CommandBufferBeginInfo{};

	CommandBuffer.begin(beginInfo);
	State = ECommandContextState::Begin;
	//vkBeginCommandBuffer();
	
	Sets.clear();
	Pipeline = nullptr;
	bWaitForCompletion = false;
}

bool VulkanCommandBuffer::FinishRecordCommandList()
{
	Close();
	return true;
}

void VulkanCommandBuffer::Close()
{
	if (bIsRenderPassActive)
	{
		vkCmdEndRendering(CommandBuffer);
		//vkCmdEndRenderPass(CommandBuffer);
		bIsRenderPassActive = false;
	}

	auto Result = vkEndCommandBuffer(CommandBuffer); 
	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to end command buffer");

	//CommandBuffer.end();

	Pipeline = nullptr;
	State = ECommandContextState::End;
	Sets.clear();
}

bool VulkanCommandBuffer::ExecuteCommandList(ECommandContextExecuteType ExecuteType, std::uint32_t Order)
{
	if (State != ECommandContextState::End)
		return false;

	const auto fenceValue = Owner->ExecuteGraphicsCommandBuffer(ExecuteType, Order, this, ExecuteType == ECommandContextExecuteType::Immediate);

	if (ExecuteType == ECommandContextExecuteType::Immediate)
	{
		OnExecuteCommandList(fenceValue);
	}
	else if (ExecuteType == ECommandContextExecuteType::Deferred)
	{
		State = ECommandContextState::WaitingForExecute;
	}

	bWaitForCompletion = false;

	return true;
}

bool VulkanCommandBuffer::ExecuteCommandListWithWaitSemaphore(VkSemaphore WaitSemaphore, VkPipelineStageFlags WaitStage)
{
	if (State != ECommandContextState::End || WaitSemaphore == VK_NULL_HANDLE)
		return false;

	const auto fenceValue = Owner->ExecuteGraphicsCommandBuffer(
		ECommandContextExecuteType::Immediate,
		std::uint32_t(-1),
		this,
		true,
		WaitSemaphore,
		WaitStage);
	OnExecuteCommandList(fenceValue);
	bWaitForCompletion = false;
	return true;
}

void VulkanCommandBuffer::OnExecuteCommandList(std::uint64_t FenceValue)
{
	State = ECommandContextState::Waiting;
	LastFenceValue = FenceValue;
	Owner->ReturnCommandBuffer(GetCommandContextType(), Get(), FenceValue);
	CommandBuffer = nullptr;
	//Result = ProfileResult();
}

void VulkanCommandBuffer::TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange)
{
	if (oldLayout == newLayout)
		return;

	VkImageMemoryBarrier2 barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	barrier.pNext = nullptr;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange = subresourceRange;

	auto GetStageMask = [](VkImageLayout layout) -> VkPipelineStageFlags2
	{
		switch (layout)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			return VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			return VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			return VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			return VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			return VK_PIPELINE_STAGE_2_TRANSFER_BIT;
		case VK_IMAGE_LAYOUT_GENERAL:
			return VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		default:
			return VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		}
	};
	auto GetAccessMask = [](VkImageLayout layout) -> VkAccessFlags2
	{
		switch (layout)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			return VK_ACCESS_2_NONE;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			return VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			return VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			return VK_ACCESS_2_SHADER_READ_BIT;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			return VK_ACCESS_2_TRANSFER_READ_BIT;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			return VK_ACCESS_2_TRANSFER_WRITE_BIT;
		case VK_IMAGE_LAYOUT_GENERAL:
			return VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
		default:
			return VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
		}
	};

	barrier.srcStageMask = GetStageMask(oldLayout);
	barrier.srcAccessMask = GetAccessMask(oldLayout);
	barrier.dstStageMask = GetStageMask(newLayout);
	barrier.dstAccessMask = GetAccessMask(newLayout);

	// Package the barrier into a VkDependencyInfo structure
	VkDependencyInfo dependencyInfo{};
	dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	dependencyInfo.pNext = nullptr;
	dependencyInfo.dependencyFlags = 0;
	dependencyInfo.memoryBarrierCount = 0;
	dependencyInfo.pMemoryBarriers = nullptr;
	dependencyInfo.bufferMemoryBarrierCount = 0;
	dependencyInfo.pBufferMemoryBarriers = nullptr;
	dependencyInfo.imageMemoryBarrierCount = 1;
	dependencyInfo.pImageMemoryBarriers = &barrier;

	// Execute the modern synchronization2 barrier command
	vkCmdPipelineBarrier2(CommandBuffer, &dependencyInfo);
}

void VulkanCommandBuffer::TransitionTo(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags2 srcAccessMask, VkAccessFlags2 dstAccessMask, VkPipelineStageFlags2 srcStage, VkPipelineStageFlags2 dstStage, std::uint32_t Mips)
{
	// Initialize the VkImageMemoryBarrier2 structure
	VkImageMemoryBarrier2 image_barrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,

		// Specify the pipeline stages and access masks for the barrier
		.srcStageMask = srcStage,             // Source pipeline stage mask
		.srcAccessMask = srcAccessMask,        // Source access mask
		.dstStageMask = dstStage,             // Destination pipeline stage mask
		.dstAccessMask = dstAccessMask,        // Destination access mask

		// Specify the old and new layouts of the image
		.oldLayout = oldLayout,        // Current layout of the image
		.newLayout = newLayout,        // Target layout of the image

		// We are not changing the ownership between queues
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,

		// Specify the image to be affected by this barrier
		.image = image,

		// Define the subresource range (which parts of the image are affected)
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,        // Affects the color aspect of the image
			.baseMipLevel = 0,                                // Start at mip level 0
			.levelCount = Mips,                                // Number of mip levels affected
			.baseArrayLayer = 0,                                // Start at array layer 0
			.layerCount = 1                                 // Number of array layers affected
		} };

	// Initialize the VkDependencyInfo structure
	VkDependencyInfo dependency_info{
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.dependencyFlags = 0,// VK_DEPENDENCY_BY_REGION_BIT,                    // No special dependency flags
		.imageMemoryBarrierCount = 1,                    // Number of image memory barriers
		.pImageMemoryBarriers = &image_barrier        // Pointer to the image memory barrier(s)
	};

	// Record the pipeline barrier into the command buffer
	vkCmdPipelineBarrier2(CommandBuffer, &dependency_info);
}

void VulkanCommandBuffer::PipelineBarrier(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage, uint32_t mipLevels)
{
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

	barrier.srcAccessMask = srcAccessMask;
	barrier.dstAccessMask = dstAccessMask;

	vkCmdPipelineBarrier(CommandBuffer, sourceStage, destinationStage, VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &barrier);
}

void VulkanCommandBuffer::WaitForGPU()
{
	if (LastFenceValue != 0)
		Owner->WaitForCPUFence(LastFenceValue);
}

void VulkanCommandBuffer::Set32BitConstant(std::uint32_t RootParameterIndex, std::uint32_t SrcData, std::uint32_t DestOffsetIn32BitValues)
{
	vkCmdPushConstants(CommandBuffer, Pipeline->GetLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, DestOffsetIn32BitValues, sizeof(std::uint32_t), &SrcData);
}

void VulkanCommandBuffer::Set32BitConstants(std::uint32_t RootParameterIndex, const void* pSrcData, std::uint32_t Num32BitValuesToSet, std::uint32_t DestOffsetIn32BitValues)
{
	// Wrong
	// to do fix
	return;
	vkCmdPushConstants(CommandBuffer, Pipeline->GetLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, DestOffsetIn32BitValues, Num32BitValuesToSet * sizeof(std::uint32_t), &pSrcData);
}

void VulkanCommandBuffer::SetBindlessDescriptor(std::uint32_t RootParameterIndex, IBindlessSceneContainer* Container)
{
	if (!Container)
		return;

	vkCmdPushConstants(CommandBuffer, Pipeline->GetLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, Container->GetOffset(), Container->Size() * sizeof(std::uint32_t), Container->GetDescriptor());
}

void VulkanCommandBuffer::Draw(std::uint32_t VertexCount, std::uint32_t VertexStartOffset)
{
	if (!bIsRenderPassActive)
		return;

	CommandBuffer.draw(VertexCount, 1, VertexStartOffset, 0);
}

void VulkanCommandBuffer::DrawInstanced(std::uint32_t VertexCountPerInstance, std::uint32_t InstanceCount, std::uint32_t StartVertexLocation, std::uint32_t StartInstanceLocation)
{
	if (!bIsRenderPassActive)
		return;

	CommandBuffer.draw(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

void VulkanCommandBuffer::DrawIndexedInstanced(std::uint32_t IndexCountPerInstance, std::uint32_t InstanceCount, std::uint32_t StartIndexLocation, std::int32_t BaseVertexLocation, std::uint32_t StartInstanceLocation)
{
	if (!bIsRenderPassActive)
		return;

	CommandBuffer.drawIndexed(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

void VulkanCommandBuffer::DrawIndexedInstanced(const sObjectDrawParameters& DrawParameters)
{
	if (!bIsRenderPassActive)
		return;

	CommandBuffer.drawIndexed(DrawParameters.IndexCountPerInstance, DrawParameters.InstanceCount, DrawParameters.StartIndexLocation, DrawParameters.BaseVertexLocation, DrawParameters.StartInstanceLocation);
}

void VulkanCommandBuffer::ExecuteIndirect(IIndirectBuffer* IndirectBuffer)
{
	if (!IndirectBuffer)
		return;

	VulkanIndirectBuffer* ArgumentBuffer = static_cast<VulkanIndirectBuffer*>(IndirectBuffer);
	const VkBuffer& Buffer = ArgumentBuffer->GetBuffer();
	std::uint64_t ArgumentOffset = ArgumentBuffer->GetOffset();
	//CommandBuffer.DrawIndexedIndirect(,);
	vkCmdDrawIndexedIndirect(
		CommandBuffer,										// Your active VkCommandBuffer
		Buffer,												// The Vulkan buffer containing the commands
		(std::uint32_t)ArgumentOffset,						// Byte offset into the buffer where parameters begin
		(UINT)ArgumentBuffer->GetCurrentCommandSize(),		// Number of draws to execute
		(std::uint32_t)ArgumentBuffer->GetStride()			// Stride between successive sets of parameters
	);
}

void VulkanCommandBuffer::SetViewport(const sViewport& Viewport)
{
	vk::Viewport VP = {};
	VP.width = (float)Viewport.Width;
	VP.height = -(float)Viewport.Height;
	VP.minDepth = Viewport.MinDepth;
	VP.maxDepth = Viewport.MaxDepth;
	VP.x = 0.0;// (float)Viewport.TopLeftX;
	VP.y = (float)Viewport.Height; //(float)Viewport.TopLeftY;
	CommandBuffer.setViewport(0, VP);
}

void VulkanCommandBuffer::SetScissorRect(std::uint32_t X, std::uint32_t Y, std::uint32_t Z, std::uint32_t W)
{
	vk::Rect2D Rect;
	Rect.extent.width = W - Y;
	Rect.extent.height = Z - X;
	Rect.offset.x = Y;
	Rect.offset.y = X;
	CommandBuffer.setScissor(0, Rect);
}

void VulkanCommandBuffer::SetStencilRef(std::uint32_t Ref)
{
	StencilRef = Ref;
	vkCmdSetStencilReference(CommandBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, StencilRef);
}

void VulkanCommandBuffer::ClearFrameBuffer(IFrameBuffer* pFB)
{
	if (bIsRenderPassActive)
	{
		std::vector<VkClearAttachment> ClearAttachments;
		for (auto& RT : pFB->GetRenderTargets())
		{
			VkClearAttachment clearAttachment{};
			clearAttachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;		// Use VK_IMAGE_ASPECT_COLOR_BIT for color
			clearAttachment.colorAttachment = 0;						// Ignored for depth/stencil; index of color attachment if clearing color
			clearAttachment.clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };		// RGBA clear value
			ClearAttachments.push_back(clearAttachment);
		}

		if (pFB->GetDepthTarget())
		{
			VkClearAttachment clearAttachment{};
			clearAttachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;		// Use VK_IMAGE_ASPECT_DEPTH_BIT for depth
			clearAttachment.colorAttachment = 0;						// Ignored for depth/stencil; index of color attachment if clearing color
			clearAttachment.clearValue.depthStencil = { 0.0f, 0 };		// 0.0f for Reverse-Z depth clear
			ClearAttachments.push_back(clearAttachment);
		}

		auto Info = pFB->GetAttachmentInfo();
		VkClearRect clearRect{};
		clearRect.rect.offset = { 0, 0 };
		clearRect.rect.extent = { Info.Desc.Dimensions.X, Info.Desc.Dimensions.Y };
		clearRect.baseArrayLayer = 0;
		clearRect.layerCount = 1;									// Covers single layer (or all layers in a cubemap/array)

		vkCmdClearAttachments(
			CommandBuffer,
			(std::uint32_t)ClearAttachments.size(),         // attachmentCount
			ClearAttachments.data(),						// pAttachments
			1,												// rectCount
			&clearRect										// pRects
		);
	}
	else
	{
		VkClearColorValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

		VkImageSubresourceRange COLOR_Range{};
		COLOR_Range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		COLOR_Range.baseMipLevel = 0;
		COLOR_Range.levelCount = 1;
		COLOR_Range.baseArrayLayer = 0;
		COLOR_Range.layerCount = 1;

		for (auto& RT : pFB->GetRenderTargets())
		{
			VulkanRenderTarget* Texture = static_cast<VulkanRenderTarget*>(RT);
			Texture->AsResource(EResourceState::Common, this);
			vkCmdClearColorImage(CommandBuffer, Texture->Image, Texture->CurrentLayout, &clearColor, 1, &COLOR_Range);
		}

		if (pFB->GetDepthTarget())
		{
			VkImageSubresourceRange DEPTH_Range{};
			DEPTH_Range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			DEPTH_Range.baseMipLevel = 0;
			DEPTH_Range.levelCount = 1;
			DEPTH_Range.baseArrayLayer = 0;
			DEPTH_Range.layerCount = 1;
			VkClearDepthStencilValue ClearDepthStencilValue = { 0.0f, 0 };
			VulkanDepthTarget* Depth = static_cast<VulkanDepthTarget*>(pFB->GetDepthTarget());
			TransitionImageLayout(Depth->Image, Depth->CurrentLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, DEPTH_Range);
			Depth->CurrentLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			vkCmdClearDepthStencilImage(CommandBuffer, Depth->Image, Depth->CurrentLayout, &ClearDepthStencilValue, 1, &DEPTH_Range);
		}
	}
}

void VulkanCommandBuffer::ClearRenderTarget(IRenderTarget* pRT, IDepthTarget* DepthTarget)
{
	VkClearColorValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

	VkImageSubresourceRange COLOR_Range{};
	COLOR_Range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	COLOR_Range.baseMipLevel = 0;
	COLOR_Range.levelCount = 1;
	COLOR_Range.baseArrayLayer = 0;
	COLOR_Range.layerCount = 1;

	VulkanRenderTarget* Texture = static_cast<VulkanRenderTarget*>(pRT);
	Texture->AsResource(EResourceState::Common, this);
	vkCmdClearColorImage(CommandBuffer, Texture->Image, Texture->CurrentLayout, &clearColor, 1, &COLOR_Range);

	if (DepthTarget)
	{
		VkImageSubresourceRange DEPTH_Range{};
		DEPTH_Range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		DEPTH_Range.baseMipLevel = 0;
		DEPTH_Range.levelCount = 1;
		DEPTH_Range.baseArrayLayer = 0;
		DEPTH_Range.layerCount = 1;
		VkClearDepthStencilValue ClearDepthStencilValue = { 0.0f, 0 };
		VulkanDepthTarget* Depth = static_cast<VulkanDepthTarget*>(DepthTarget);
		TransitionImageLayout(Depth->Image, Depth->CurrentLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, DEPTH_Range);
		Depth->CurrentLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		vkCmdClearDepthStencilImage(CommandBuffer, Depth->Image, Depth->CurrentLayout, &ClearDepthStencilValue, 1, &DEPTH_Range);
	}
}

void VulkanCommandBuffer::ClearRenderTargets(std::vector<IRenderTarget*> pRTs, IDepthTarget* DepthTarget)
{
	VkClearColorValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

	VkImageSubresourceRange COLOR_Range{};
	COLOR_Range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	COLOR_Range.baseMipLevel = 0;
	COLOR_Range.levelCount = 1;
	COLOR_Range.baseArrayLayer = 0;
	COLOR_Range.layerCount = 1;

	for (auto& RT : pRTs)
	{
		VulkanRenderTarget* Texture = static_cast<VulkanRenderTarget*>(RT);
		Texture->AsResource(EResourceState::Common, this);
		vkCmdClearColorImage(CommandBuffer, Texture->Image, Texture->CurrentLayout, &clearColor, 1, &COLOR_Range);
	}

	if (DepthTarget)
	{
		VkImageSubresourceRange DEPTH_Range{};
		DEPTH_Range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		DEPTH_Range.baseMipLevel = 0;
		DEPTH_Range.levelCount = 1;
		DEPTH_Range.baseArrayLayer = 0;
		DEPTH_Range.layerCount = 1;
		VkClearDepthStencilValue ClearDepthStencilValue = { 0.0f, 0 };
		VulkanDepthTarget* Depth = static_cast<VulkanDepthTarget*>(DepthTarget);
		TransitionImageLayout(Depth->Image, Depth->CurrentLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, DEPTH_Range);
		Depth->CurrentLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		vkCmdClearDepthStencilImage(CommandBuffer, Depth->Image, Depth->CurrentLayout, &ClearDepthStencilValue, 1, &DEPTH_Range);
	}
}

void VulkanCommandBuffer::ClearDepthTarget(IDepthTarget* DepthTarget)
{
	VkClearColorValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

	VkImageSubresourceRange DEPTH_Range{};
	DEPTH_Range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	DEPTH_Range.baseMipLevel = 0;
	DEPTH_Range.levelCount = 1;
	DEPTH_Range.baseArrayLayer = 0;
	DEPTH_Range.layerCount = 1;
	VkClearDepthStencilValue ClearDepthStencilValue = { 0.0f, 0 };
	VulkanDepthTarget* Depth = static_cast<VulkanDepthTarget*>(DepthTarget);
	TransitionImageLayout(Depth->Image, Depth->CurrentLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, DEPTH_Range);
	Depth->CurrentLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	vkCmdClearDepthStencilImage(CommandBuffer, Depth->Image, Depth->CurrentLayout, &ClearDepthStencilValue, 1, &DEPTH_Range);
}

void VulkanCommandBuffer::SetFrameBuffer(IFrameBuffer* pFB, std::optional<std::size_t> FBOIndex)
{
	if (bIsRenderPassActive)
	{
		vkCmdEndRendering(CommandBuffer);
		//vkCmdEndRenderPass(CommandBuffer);
		bIsRenderPassActive = false;
	}
	VkImageSubresourceRange COLOR_Range{};
	COLOR_Range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	COLOR_Range.baseMipLevel = 0;
	COLOR_Range.levelCount = 1;
	COLOR_Range.baseArrayLayer = 0;
	COLOR_Range.layerCount = 1;
	
	for (auto& RT : pFB->GetRenderTargets())
	{
		VulkanRenderTarget* VRT = static_cast<VulkanRenderTarget*>(RT);
		if (VRT->CurrentLayout != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			TransitionImageLayout(
				VRT->Image,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				COLOR_Range
			);
			VRT->CurrentLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
	}

	VkImageSubresourceRange DEPTH_Range{};
	DEPTH_Range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	DEPTH_Range.baseMipLevel = 0;
	DEPTH_Range.levelCount = 1;
	DEPTH_Range.baseArrayLayer = 0;
	DEPTH_Range.layerCount = 1;

	VulkanDepthTarget* Depth = static_cast<VulkanDepthTarget*>(pFB->GetDepthTarget());
	if (Depth->CurrentLayout != VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
	{
		TransitionImageLayout(
			Depth->Image,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			DEPTH_Range
		);
		Depth->CurrentLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
	}
}

void VulkanCommandBuffer::SetRenderTarget(IRenderTarget* pRT, IDepthTarget* DepthTarget)
{
	VulkanRenderTarget* RenderTarget = static_cast<VulkanRenderTarget*>(pRT);
	VulkanDepthTarget* Depth = static_cast<VulkanDepthTarget*>(DepthTarget);

	if (RenderTarget)
	{
		VkImageSubresourceRange COLOR_Range{};
		COLOR_Range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		COLOR_Range.baseMipLevel = 0;
		COLOR_Range.levelCount = 1;
		COLOR_Range.baseArrayLayer = 0;
		COLOR_Range.layerCount = 1;

		if (RenderTarget->CurrentLayout != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			TransitionImageLayout(
				RenderTarget->Image,
				RenderTarget->CurrentLayout,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				COLOR_Range
			);
			RenderTarget->CurrentLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
	}

	if (Depth)
	{
		VkImageSubresourceRange COLOR_Range{};
		COLOR_Range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		COLOR_Range.baseMipLevel = 0;
		COLOR_Range.levelCount = 1;
		COLOR_Range.baseArrayLayer = 0;
		COLOR_Range.layerCount = 1;

		if (Depth->CurrentLayout != VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
		{
			TransitionImageLayout(
				Depth->Image,
				Depth->CurrentLayout,
				VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
				COLOR_Range
			);
			Depth->CurrentLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		}
	}

	// Transition the render target to the appropriate layout
	//TransitionTo(
	//	RenderTarget->Image,
	//	RenderTarget->CurrentLayout,
	//	VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	//	VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
	//	VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
	//	VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
	//	VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
	//	1
	//);
	//RenderTarget->CurrentLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//// Transition the depth target to the appropriate layout if it exists
	//if (Depth)
	//{
	//	TransitionTo(
	//		Depth->Image,
	//		Depth->CurrentLayout,
	//		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	//		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
	//		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
	//		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
	//		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
	//		1
	//	);
	//	Depth->CurrentLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	//}

	//// Set the render pass and framebuffer
	//VkRenderPassBeginInfo renderPassInfo = {};
	//renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	//renderPassInfo.renderPass = Pipeline->RenderPass->Get();
	//renderPassInfo.framebuffer = Pipeline->FrameBuffer;
	//renderPassInfo.renderArea.extent.width = Pipeline->FrameBufferDimension.Width;
	//renderPassInfo.renderArea.extent.height = Pipeline->FrameBufferDimension.Height;
	//renderPassInfo.clearValueCount = 2;

	//std::array<VkClearValue, 2> clearValues = {};
	//clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
	//clearValues[1].depthStencil = { 1.0f, 0 };

	//renderPassInfo.pClearValues = clearValues.data();

	//vkCmdBeginRenderPass(CommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	//bIsRenderPassActive = true;
}

void VulkanCommandBuffer::SetRenderTargets(std::vector<IRenderTarget*> pRTs, IDepthTarget* DepthTarget)
{
	Engine::WriteToConsole("Unimplemented VulkanCommandBuffer::SetRenderTargets");
}

void VulkanCommandBuffer::SetFrameBufferAsResource(IFrameBuffer* pFB, std::uint32_t RootParameterIndex)
{
	Engine::WriteToConsole("Unimplemented VulkanCommandBuffer::SetFrameBufferAsResource");
}

void VulkanCommandBuffer::SetFrameBufferAsResource(IFrameBuffer* pFB, std::uint32_t FBOIndex, std::uint32_t RootParameterIndex)
{
	Engine::WriteToConsole("Unimplemented VulkanCommandBuffer::SetFrameBufferAsResource with FBOIndex");
}

void VulkanCommandBuffer::SetRenderTargetAsResource(IRenderTarget* pRT, std::uint32_t RootParameterIndex)
{
	VulkanRenderTarget* Texture = static_cast<VulkanRenderTarget*>(pRT);
	if (!Texture->IsSRV_Allowed())
		return;

	TransitionTo(
		Texture->Image,
		Texture->CurrentLayout,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		1
	);

	Texture->CurrentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	Sets.push_back(DescCopy());
	Sets.back().IsBuffer = false;
	Sets.back().DescriptorImageInfo = Texture->DescriptorImageInfo;
	Sets.back().Location = 0;
}

void VulkanCommandBuffer::SetRenderTargetsAsResource(std::vector<IRenderTarget*> RTs, std::uint32_t RootParameterIndex)
{
	Engine::WriteToConsole("Unimplemented VulkanCommandBuffer::SetRenderTargetsAsResource");
}

void VulkanCommandBuffer::CopyFrameBuffer(IFrameBuffer* Dest, std::size_t DestFBOIndex, IFrameBuffer* Source, std::uint32_t SourceFBOIndex)
{
	Engine::WriteToConsole("Unimplemented VulkanCommandBuffer::CopyFrameBuffer");
}

void VulkanCommandBuffer::CopyFrameBufferDepth(IFrameBuffer* Dest, IFrameBuffer* Source)
{
	Engine::WriteToConsole("Unimplemented VulkanCommandBuffer::CopyFrameBufferDepth");
}

void VulkanCommandBuffer::CopyRenderTarget(IRenderTarget* Dest, IRenderTarget* Source)
{
	Engine::WriteToConsole("Unimplemented VulkanCommandBuffer::CopyRenderTarget");
}

void VulkanCommandBuffer::CopyDepthBuffer(IDepthTarget* Dest, IDepthTarget* Source)
{
	Engine::WriteToConsole("Unimplemented VulkanCommandBuffer::CopyDepthBuffer");
	VulkanDepthTarget* DestFBO = static_cast<VulkanDepthTarget*>(Dest);
	VulkanDepthTarget* SourceFBO = static_cast<VulkanDepthTarget*>(Source);


	//vkCmdCopyImage(,);
}

void VulkanCommandBuffer::SetUnorderedAccessBufferAsResource(IStructuredBuffer* pUAV, std::optional<std::uint32_t> RootParameterIndex)
{
	Engine::WriteToConsole("Unimplemented VulkanCommandBuffer::SetUnorderedAccessBufferAsResource");
}

void VulkanCommandBuffer::SetUnorderedAccessBuffersAsResource(std::vector<IStructuredBuffer*> UAVs, std::optional<std::uint32_t> RootParameterIndex)
{
	Engine::WriteToConsole("Unimplemented VulkanCommandBuffer::SetUnorderedAccessBuffersAsResource");
}

void VulkanCommandBuffer::SetPipeline(IPipeline* pipeline)
{
	if (Pipeline == pipeline)
		return;

	Pipeline = static_cast<VulkanPipeline*>(pipeline);

	//if (bIsRenderPassActive)
	//	vkCmdEndRenderPass(CommandBuffer);

	if (!bIsRenderPassActive)
	{
		// Set clear color values.
		/*std::vector<VkClearValue> clear_value;
		for (std::size_t i = 0; i < Pipeline->RenderPass->Info.GetAttachmentCount(); i++)
			clear_value.push_back({ {0.0f, 0.0f, 0.0f, 0.0f} });

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.clearValueCount = (std::uint32_t)clear_value.size();
		renderPassInfo.pClearValues = clear_value.data();
		renderPassInfo.renderPass = Pipeline->RenderPass->Get();
		renderPassInfo.renderArea.extent.width = Pipeline->FrameBufferDimension.Width;
		renderPassInfo.renderArea.extent.height = Pipeline->FrameBufferDimension.Height;
		renderPassInfo.framebuffer = Pipeline->FrameBuffer;
		vkCmdBeginRenderPass(CommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);*/

		//if (bIsRenderPassActive)
		//	vkCmdEndRendering(CommandBuffer);

		vkCmdBeginRendering(CommandBuffer, &Pipeline->RenderingInfo);

		bIsRenderPassActive = true;
	}

	//CommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, static_cast<VulkanPipeline*>(Pipeline)->Get(), nullptr);
	vkCmdBindPipeline(CommandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline->Get());

	BindDescriptorSets();
}

void VulkanCommandBuffer::SetConstantBuffer(IConstantBuffer* CB, std::optional<std::uint32_t> InRootParameterIndex)
{
	VulkanConstantBuffer* ConstantBuffer = static_cast<VulkanConstantBuffer*>(CB);
	
	const bool enablePushConstants = false;
	if (!enablePushConstants)
	{
		std::uint32_t Root = ConstantBuffer->GetDefaultRootParameterIndex();
		if (Root == 0)
			Root = 13;
		else if (Root == 1)
			Root = 12;
		else if (Root == 2)
			Root = 11;
		else if (Root == 3)
			Root = 10;

		Sets.push_back(DescCopy());
		Sets.back().IsBuffer = true;
		Sets.back().DescriptorBufferInfo = ConstantBuffer->DescriptorInfo;
		Sets.back().Location = Root;// ConstantBuffer->GetDefaultRootParameterIndex();
	}
	else
	{
		//CommandBuffer.pushConstants(Pipeline->GetLayout(), vk::ShaderStageFlagBits::eAll, 0, ConstantBuffer->GetSize(), ConstantBuffer->GetBuffer());

		/*VkPushConstantsInfo PushConstantsInfo = {};
		PushConstantsInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PUSH_CONSTANTS_INFO;
		PushConstantsInfo.layout = Pipeline->GetLayout();
		PushConstantsInfo.offset = 0;
		PushConstantsInfo.pValues = ConstantBuffer->GetData();
		PushConstantsInfo.size = ConstantBuffer->GetSize();
		PushConstantsInfo.stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_ALL;*/
		//vkCmdPushConstants2(CommandBuffer, &PushConstantsInfo);
	}
}

void VulkanCommandBuffer::SetTexture2D(ITexture2D* Texture2D, std::optional<std::uint32_t> InRootParameterIndex)
{
	VulkanTexture* Texture = static_cast<VulkanTexture*>(Texture2D);

	Sets.push_back(DescCopy());
	Sets.back().IsBuffer = false;
	Sets.back().DescriptorImageInfo = Texture->DescriptorImageInfo;
	Sets.back().Location = 0;// Texture->RootParameterIndex;
}

void VulkanCommandBuffer::BindDescriptorSets()
{
	//if (Sets.size() == 0)
	//	return;

	//std::vector<VkWriteDescriptorSet> WriteDescriptorSets;
	//for (auto& Set : Sets)
	//{
	//	VkWriteDescriptorSet DescSet = {};
	//	DescSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	//	DescSet.pNext = nullptr;
	//	DescSet.descriptorCount = 1;
	//	DescSet.dstArrayElement = 0;
	//	DescSet.dstBinding = Set.Location;
	//	DescSet.dstSet = 0;
	//	DescSet.pTexelBufferView = nullptr;
	//	if (Set.IsBuffer)
	//	{
	//		DescSet.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	//		DescSet.pBufferInfo = &Set.DescriptorBufferInfo;
	//		DescSet.pImageInfo = nullptr;
	//	}
	//	else
	//	{
	//		DescSet.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	//		DescSet.pImageInfo = &Set.DescriptorImageInfo;
	//		DescSet.pBufferInfo = nullptr;
	//	}
	//	WriteDescriptorSets.push_back(DescSet);
	//}

	////vkCmdPushDescriptorSet(CommandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline->GetLayout(), 0, WriteDescriptorSets.size(), WriteDescriptorSets.data());
	//vkCmdPushDescriptorSetKHR(CommandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline->GetLayout(), 0, (std::uint32_t)WriteDescriptorSets.size(), WriteDescriptorSets.data());

	//Sets.clear();

	VkDescriptorSet BindlessDescriptorSet = Owner->GetBindlessDescriptorSet();
	vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline->GetLayout(), 0, 1, &BindlessDescriptorSet, 0, nullptr);

}

void VulkanCommandBuffer::SetVertexBuffer(IVertexBuffer* VBuffer, std::uint32_t Slot)
{
	VkBuffer Buffer = static_cast<VulkanVertexBuffer*>(VBuffer)->GetBuffer();
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(CommandBuffer, Slot, 1, &Buffer, offsets);
}

void VulkanCommandBuffer::SetIndexBuffer(IIndexBuffer* Buffer)
{
	CommandBuffer.bindIndexBuffer(static_cast<VulkanIndexBuffer*>(Buffer)->GetBuffer(), 0, vk::IndexType::eUint32);
}

void VulkanCommandBuffer::UpdateBufferSubresource(IVertexBuffer* Buffer, BufferSubresource* Subresource)
{
	UpdateBufferSubresource(Buffer, Subresource->Location, Subresource->Size, Subresource->pSysMem);
}

void VulkanCommandBuffer::UpdateBufferSubresource(IVertexBuffer* Buffer, std::size_t Location, std::size_t Size, const void* pSrcData)
{
	static_cast<VulkanVertexBuffer*>(Buffer)->MapEx(pSrcData, (std::uint32_t)Location, (std::uint32_t)Size);
}

void VulkanCommandBuffer::UpdateBufferSubresource(IIndexBuffer* Buffer, BufferSubresource* Subresource)
{
	UpdateBufferSubresource(Buffer, Subresource->Location, Subresource->Size, Subresource->pSysMem);
}

void VulkanCommandBuffer::UpdateBufferSubresource(IIndexBuffer* Buffer, std::size_t Location, std::size_t Size, const void* pSrcData)
{
	static_cast<VulkanIndexBuffer*>(Buffer)->MapEx(pSrcData, (std::uint32_t)Location, (std::uint32_t)Size);
}

void VulkanCommandBuffer::CopyResource(VkImage pDstResource, VkImageLayout DestState, VkImage pSrcResource, VkImageLayout SrcState, sDimension2D Dimension)
{
	TransitionTo(
		pDstResource,
		DestState,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	);
	TransitionTo(
		pSrcResource,
		SrcState,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
	);

	VkImageCopy Copy{};
	Copy.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	Copy.srcOffset = { 0, 0, 0 };
	Copy.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	Copy.dstOffset = { 0, 0, 0 };
	Copy.extent = { Dimension.Width, Dimension.Height, 1 };
	vkCmdCopyImage(CommandBuffer, pSrcResource, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, pDstResource, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &Copy);

	TransitionTo(
		pDstResource,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		DestState
	);
	TransitionTo(
		pSrcResource,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	);

	bWaitForCompletion = true;
}

void VulkanCommandBuffer::ClearState()
{
	StencilRef = 0;
}

VulkanCopyCommandBuffer::VulkanCopyCommandBuffer(VulkanDevice* InDevice)
{

}

VulkanCopyCommandBuffer::~VulkanCopyCommandBuffer()
{
}

void VulkanCopyCommandBuffer::BeginRecordCommandList()
{
}

void VulkanCopyCommandBuffer::FinishRecordCommandList()
{
}

void VulkanCopyCommandBuffer::ExecuteCommandList()
{
}

void VulkanCopyCommandBuffer::CopyFrameBuffer(IFrameBuffer* Dest, std::size_t DestFBOIndex, IFrameBuffer* Source, std::uint32_t SourceFBOIndex)
{
}

void VulkanCopyCommandBuffer::CopyFrameBufferDepth(IFrameBuffer* Dest, IFrameBuffer* Source)
{
}

void VulkanCopyCommandBuffer::UpdateBufferSubresource(IVertexBuffer* Buffer, BufferSubresource* Subresource)
{
}

void VulkanCopyCommandBuffer::UpdateBufferSubresource(IVertexBuffer* Buffer, std::size_t Location, std::size_t Size, const void* pSrcData)
{
}

void VulkanCopyCommandBuffer::UpdateBufferSubresource(IIndexBuffer* Buffer, BufferSubresource* Subresource)
{
}

void VulkanCopyCommandBuffer::UpdateBufferSubresource(IIndexBuffer* Buffer, std::size_t Location, std::size_t Size, const void* pSrcData)
{
}

void VulkanCopyCommandBuffer::ClearState()
{
}
