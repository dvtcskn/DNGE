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

VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice* InOwner)
	: Owner(InOwner)
	, StencilRef(0)
	, bIsClosed(false)
	, bWaitForCompletion(false)
	, bIsRenderPassActive(false)
{
	auto Device = Owner->Get();

	CommandBuffer = Owner->RequestCommandBuffer();

	/*VkCommandBufferAllocateInfo cmd_buf_info{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = CommandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1 };*/
	//VK_CHECK(vkAllocateCommandBuffers(Device, &cmd_buf_info, &CommandBuffer));

	/*auto allocInfo = vk::CommandBufferAllocateInfo()
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandPool(CommandPool)
		.setCommandBufferCount(1);

	auto Res = Owner->Get_Hpp().allocateCommandBuffers(&allocInfo, &CommandBuffer);
	if (Res != vk::Result::eSuccess)
	{

	}*/

	//CommandBuffer.end();

	bIsClosed = true;

	// The push descriptor update function is part of an extension so it has to be manually loaded
	vkCmdPushDescriptorSetKHR = reinterpret_cast<PFN_vkCmdPushDescriptorSetKHR>(vkGetDeviceProcAddr(Owner->Get(), "vkCmdPushDescriptorSetKHR"));
	if (!vkCmdPushDescriptorSetKHR)
	{
		throw std::runtime_error("Could not get a valid function pointer for vkCmdPushDescriptorSetKHR");
	}
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
	if (!bIsClosed)
		Close();

	//Owner->Get_Hpp().freeCommandBuffers(CommandPool, CommandBuffer);
	//Owner->Get_Hpp().destroyCommandPool(CommandPool);
	//vkFreeCommandBuffers(Owner->Get(), CommandPool, 1, &CommandBuffer);
	//vkDestroyCommandPool(Owner->Get(), CommandPool, nullptr);
	ClearCMDStates();

	vkCmdPushDescriptorSetKHR = nullptr;
}

void VulkanCommandBuffer::BeginRecordCommandList(const ERenderPass RenderPass)
{
	Open();

	if (RenderPass != ERenderPass::eNONE)
	{
		//Owner->SetHeaps(CommandList.Get());
	}
	else
	{
		//Owner->CPUWait();
	}
}

void VulkanCommandBuffer::Open()
{
	//if (!CommandPool)
	//	CommandPool = Owner->RequestCommandPool();

	if (!Owner->IsCommandBufferAvailable(CommandBuffer))
	{
		auto Old = CommandBuffer;
		CommandBuffer = Owner->RequestCommandBuffer();
		Owner->ReturnCommandBuffer(Old);
	}
	else
	{
		Owner->ResetCommandBuffer(CommandBuffer);
		//VkResult result = vkResetCommandBuffer(CommandBuffer, 0);
		//if (result != VK_SUCCESS)
		//	throw std::runtime_error("Failed to reset command buffer");
	}

	auto beginInfo = vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

	CommandBuffer.begin(beginInfo);

	bIsClosed = false;
	Sets.clear();

	//ClearState(); 
}

void VulkanCommandBuffer::FinishRecordCommandList()
{
	//ClearCMDStates();
	Close();
}

void VulkanCommandBuffer::Close()
{
	if (bIsRenderPassActive)
		vkCmdEndRenderPass(CommandBuffer);

	auto Result = vkEndCommandBuffer(CommandBuffer); 
	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to end command buffer");

	//CommandBuffer.end();
	bIsClosed = true;
	bIsRenderPassActive = false;
	Sets.clear();
}

void VulkanCommandBuffer::ExecuteCommandList()
{
	Owner->ExecuteGraphicsCommandBuffer(&CommandBuffer, bWaitForCompletion);

	//Owner->ReturnCommandPool(CommandPool);
	//CommandPool = VK_NULL_HANDLE;
	bWaitForCompletion = false;
}

void VulkanCommandBuffer::ClearState()
{
	StencilRef = 0;
}

void VulkanCommandBuffer::ExecuteWithWait()
{
	Owner->ExecuteGraphicsCommandBuffer(&CommandBuffer, true);

	//Owner->ReturnCommandPool(CommandPool);
	//CommandPool = VK_NULL_HANDLE;
	bWaitForCompletion = false;
}

void VulkanCommandBuffer::TransitionTo(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags2 srcAccessMask, VkAccessFlags2 dstAccessMask, VkPipelineStageFlags2 srcStage, VkPipelineStageFlags2 dstStage)
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
			.levelCount = 1,                                // Number of mip levels affected
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

void VulkanCommandBuffer::CopyResource(VkImage pDstResource, VkImageLayout DestState, VkImage pSrcResource, VkImageLayout SrcState, sDimension2D Dimension)
{
	TransitionTo(
		pDstResource,
		DestState,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		0,														// srcAccessMask
		0,                                                      // dstAccessMask
		VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,					// srcStage
		VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT                  // dstStage
	);
	TransitionTo(
		pSrcResource,
		SrcState,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		0,														// srcAccessMask
		0,                                                      // dstAccessMask
		VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,					// srcStage
		VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT                  // dstStage
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
		DestState,
		0,														// srcAccessMask
		0,                                                      // dstAccessMask
		VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,					// srcStage
		VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT                  // dstStage
	);
	TransitionTo(
		pSrcResource,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		0,														// srcAccessMask
		0,                                                      // dstAccessMask
		VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,					// srcStage
		VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT                  // dstStage
	);

	bWaitForCompletion = true;
}

void VulkanCommandBuffer::Draw(std::uint32_t VertexCount, std::uint32_t VertexStartOffset)
{
	if (!bIsRenderPassActive)
		return;
	BindDescriptorSets();
	CommandBuffer.draw(VertexCount, 1, VertexStartOffset, 0);
}

void VulkanCommandBuffer::DrawInstanced(std::uint32_t VertexCountPerInstance, std::uint32_t InstanceCount, std::uint32_t StartVertexLocation, std::uint32_t StartInstanceLocation)
{
	if (!bIsRenderPassActive)
		return;
	BindDescriptorSets();
	CommandBuffer.draw(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

void VulkanCommandBuffer::DrawIndexedInstanced(std::uint32_t IndexCountPerInstance, std::uint32_t InstanceCount, std::uint32_t StartIndexLocation, std::int32_t BaseVertexLocation, std::uint32_t StartInstanceLocation)
{
	if (!bIsRenderPassActive)
		return;
	BindDescriptorSets();
	CommandBuffer.drawIndexed(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

void VulkanCommandBuffer::DrawIndexedInstanced(const sObjectDrawParameters& DrawParameters)
{
	if (!bIsRenderPassActive)
		return;
	BindDescriptorSets();
	CommandBuffer.drawIndexed(DrawParameters.IndexCountPerInstance, DrawParameters.InstanceCount, DrawParameters.StartIndexLocation, DrawParameters.BaseVertexLocation, DrawParameters.StartInstanceLocation);
}

void VulkanCommandBuffer::ExecuteIndirect(IIndirectBuffer* IndirectBuffer)
{
	Engine::WriteToConsole("Unimplemented VulkanCommandBuffer::ExecuteIndirect");
}

void VulkanCommandBuffer::SetViewport(const sViewport& Viewport)
{
	vk::Viewport VP = {};
	VP.width = Viewport.Width;
	VP.height = Viewport.Height;
	VP.minDepth = Viewport.MinDepth;
	VP.maxDepth = Viewport.MaxDepth;
	VP.x = Viewport.TopLeftX;
	VP.y = Viewport.TopLeftY;
	CommandBuffer.setViewport(0, VP);
}

void VulkanCommandBuffer::SetScissorRect(std::uint32_t X, std::uint32_t Y, std::uint32_t Z, std::uint32_t W)
{
	vk::Rect2D Rect;
	Rect.extent.width = W - Y;
	Rect.extent.height = Z - X;
	Rect.offset.x = 0;
	Rect.offset.y = 0;
	CommandBuffer.setScissor(0, Rect);
}

void VulkanCommandBuffer::SetStencilRef(std::uint32_t Ref)
{
	StencilRef = Ref;
	CommandBuffer.setStencilReference(vk::StencilFaceFlagBits::eVkStencilFrontAndBack, StencilRef);
}

void VulkanCommandBuffer::ClearFrameBuffer(IFrameBuffer* pFB)
{
	//CommandBuffer.clearAttachments();
}

void VulkanCommandBuffer::ClearRenderTarget(IRenderTarget* pFB, IDepthTarget* DepthTarget)
{
	//CommandBuffer.clearColorImage();
}

void VulkanCommandBuffer::ClearRenderTargets(std::vector<IRenderTarget*> pRTs, IDepthTarget* DepthTarget)
{
	Engine::WriteToConsole("Unimplemented VulkanCommandBuffer::ClearRenderTargets");
	//CommandBuffer.clearColorImage();
}

void VulkanCommandBuffer::ClearDepthTarget(IDepthTarget* DepthTarget)
{
	//CommandBuffer.clearDepthStencilImage();
}

void VulkanCommandBuffer::SetFrameBuffer(IFrameBuffer* pFB, std::optional<std::size_t> FBOIndex)
{
	/*VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.clearValueCount = ;
	renderPassInfo.pClearValues = ;
	renderPassInfo.renderPass = ;
	renderPassInfo.renderArea = ;
	renderPassInfo.framebuffer = ;

	bIsRenderPassActive = true;*/
}

void VulkanCommandBuffer::SetRenderTarget(IRenderTarget* pRT, IDepthTarget* DepthTarget)
{
	//VulkanRenderTarget* Texture = static_cast<VulkanRenderTarget*>(pRT);
	//TransitionTo(
	//	Texture->Image,
	//	Texture->CurrentLayout,
	//	VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	//	0,														// srcAccessMask
	//	0,                                                      // dstAccessMask
	//	VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,					// srcStage
	//	VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT                  // dstStage
	//);
	//Texture->CurrentLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
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
		0,														// srcAccessMask
		0,                                                      // dstAccessMask
		VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,					// srcStage
		VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT                  // dstStage
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

void VulkanCommandBuffer::SetUnorderedAccessBufferAsResource(IUnorderedAccessBuffer* pUAV, std::uint32_t RootParameterIndex)
{
	Engine::WriteToConsole("Unimplemented VulkanCommandBuffer::SetUnorderedAccessBufferAsResource");
}

void VulkanCommandBuffer::SetUnorderedAccessBuffersAsResource(std::vector<IUnorderedAccessBuffer*> UAVs, std::uint32_t RootParameterIndex)
{
	Engine::WriteToConsole("Unimplemented VulkanCommandBuffer::SetUnorderedAccessBuffersAsResource");
}

void VulkanCommandBuffer::SetPipeline(IPipeline* pipeline)
{
	Pipeline = static_cast<VulkanPipeline*>(pipeline);

	//if (bIsRenderPassActive)
	//	vkCmdEndRenderPass(CommandBuffer);

	if (!bIsRenderPassActive)
	{
		// Set clear color values.
		std::vector<VkClearValue> clear_value;
		for (std::size_t i = 0; i < Pipeline->RenderPass->Info.GetAttachmentCount(); i++)
			clear_value.push_back({ {0.0f, 0.0f, 0.0f, 0.0f} });

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.clearValueCount = clear_value.size();
		renderPassInfo.pClearValues = clear_value.data();
		renderPassInfo.renderPass = Pipeline->RenderPass->Get();
		renderPassInfo.renderArea.extent.width = Pipeline->FrameBufferDimension.Width;
		renderPassInfo.renderArea.extent.height = Pipeline->FrameBufferDimension.Height;
		renderPassInfo.framebuffer = Pipeline->FrameBuffer;
		vkCmdBeginRenderPass(CommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		bIsRenderPassActive = true;
	}

	//CommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, static_cast<VulkanPipeline*>(Pipeline)->Get(), nullptr);
	vkCmdBindPipeline(CommandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline->Get());
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
	if (Sets.size() == 0)
		return;

	std::vector<VkWriteDescriptorSet> WriteDescriptorSets;
	for (auto& Set : Sets)
	{
		VkWriteDescriptorSet DescSet = {};
		DescSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		DescSet.pNext = nullptr;
		DescSet.descriptorCount = 1;
		DescSet.dstArrayElement = 0;
		DescSet.dstBinding = Set.Location;
		DescSet.dstSet = 0;
		DescSet.pTexelBufferView = nullptr;
		if (Set.IsBuffer)
		{
			DescSet.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			DescSet.pBufferInfo = &Set.DescriptorBufferInfo;
			DescSet.pImageInfo = nullptr;
		}
		else
		{
			DescSet.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			DescSet.pImageInfo = &Set.DescriptorImageInfo;
			DescSet.pBufferInfo = nullptr;
		}
		WriteDescriptorSets.push_back(DescSet);
	}

	//vkCmdPushDescriptorSet(CommandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline->GetLayout(), 0, WriteDescriptorSets.size(), WriteDescriptorSets.data());
	vkCmdPushDescriptorSetKHR(CommandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline->GetLayout(), 0, WriteDescriptorSets.size(), WriteDescriptorSets.data());

	Sets.clear();
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
	if (bIsRenderPassActive)
		return;

	/*VulkanUploadBuffer::SharedPtr UploadBuffer = VulkanUploadBuffer::Create(Owner, "Temp", Buffer->GetSize());
	UploadBuffer->Map();

	memcpy((BYTE*)UploadBuffer->pData, pSrcData, Size);

	UploadBuffer->Unmap();

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = Location;
	copyRegion.dstOffset = Location;
	copyRegion.size = UploadBuffer->GetSize();

	vkCmdCopyBuffer(CommandBuffer, UploadBuffer->GetBuffer(), static_cast<VulkanVertexBuffer*>(Buffer)->GetBuffer(), 1, &copyRegion);
	bWaitForCompletion = true;

	UploadBuffer = nullptr;*/

	static_cast<VulkanVertexBuffer*>(Buffer)->Map(pSrcData);
	static_cast<VulkanVertexBuffer*>(Buffer)->Unmap();
	bWaitForCompletion = true;
}

void VulkanCommandBuffer::UpdateBufferSubresource(IIndexBuffer* Buffer, BufferSubresource* Subresource)
{
	UpdateBufferSubresource(Buffer, Subresource->Location, Subresource->Size, Subresource->pSysMem);
}

void VulkanCommandBuffer::UpdateBufferSubresource(IIndexBuffer* Buffer, std::size_t Location, std::size_t Size, const void* pSrcData)
{
	if (bIsRenderPassActive)
		return;

	//VulkanUploadBuffer::SharedPtr UploadBuffer = VulkanUploadBuffer::Create(Owner, "Temp", Buffer->GetSize());
	//UploadBuffer->Map();

	////memcpy((BYTE*)UploadBuffer->GetData() + Location, pSrcData, Size);

	//UploadBuffer->Unmap();

	//VkBufferCopy copyRegion = {};
	//copyRegion.srcOffset = Location;
	//copyRegion.dstOffset = Location;
	//copyRegion.size = Size;

	//vkCmdCopyBuffer(CommandBuffer, UploadBuffer->GetBuffer(), static_cast<VulkanIndexBuffer*>(Buffer)->GetBuffer(), 1, &copyRegion);
	//bWaitForCompletion = true;
	//
	//UploadBuffer = nullptr;

	static_cast<VulkanIndexBuffer*>(Buffer)->Map(pSrcData);
	static_cast<VulkanIndexBuffer*>(Buffer)->Unmap();
	bWaitForCompletion = true;
}

void VulkanCommandBuffer::ClearCMDStates()
{
	//CommandBuffer.();
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
