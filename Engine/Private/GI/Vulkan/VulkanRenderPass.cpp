
#include "pch.h"
#include "VulkanRenderpass.h"
#include "VulkanFormat.h"

VulkanRenderpass::VulkanRenderpass(VulkanDevice* InDevice, std::string InName, const sFrameBufferAttachmentInfo& InInfos)
	: Name(InName)
	, Device(InDevice)
	, Info(InInfos)
{
	auto GetAttachmentInfos = [](std::vector<sFrameBufferAttachmentInfo::sFrameBuffer> FrameBuffer) -> std::vector<VkAttachmentDescription>
	{
		std::vector<VkAttachmentDescription> attachmentDescs;
		for (uint32_t i = 0; i < FrameBuffer.size(); i++)
		{
			attachmentDescs.push_back(VkAttachmentDescription());
			attachmentDescs[i].format = ConvertFormat_Format_To_VkFormat(FrameBuffer[i].Format);
			attachmentDescs[i].samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
			attachmentDescs[i].loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachmentDescs[i].storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;

			if (FrameBuffer[i].AttachmentType == eFrameBufferAttachmentType::eDepth)
			{
				attachmentDescs[i].stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachmentDescs[i].stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;

				attachmentDescs[i].initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
				attachmentDescs[i].finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			}
			else
			{
				attachmentDescs[i].stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachmentDescs[i].stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;

				attachmentDescs[i].initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
				attachmentDescs[i].finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}
		}

		return attachmentDescs;
	};

	std::vector<VkAttachmentDescription> attachmentDescs = GetAttachmentInfos(InInfos.GetAttachments(true));

	std::vector<VkAttachmentReference> colorReferences;
	VkAttachmentReference depthReference = {};
	{
		uint32_t AttachmentIndex = 0;
		for (const auto& Info : InInfos.FrameBuffer)
		{
			colorReferences.push_back({ AttachmentIndex, VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
			AttachmentIndex++;
		}
		if (InInfos.DepthFormat != EFormat::UNKNOWN)
		{
			depthReference.attachment = AttachmentIndex;
			depthReference.layout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
	}

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.pColorAttachments = colorReferences.data();
	subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());

	if (InInfos.DepthFormat != EFormat::UNKNOWN)
		subpass.pDepthStencilAttachment = &depthReference;

	// Use subpass dependencies for attachment layout transitions
	/*std::array<vk::SubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
	dependencies[0].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	dependencies[0].srcAccessMask = vk::AccessFlagBits::eMemoryRead;
	dependencies[0].dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
	dependencies[0].dependencyFlags = vk::DependencyFlagBits::eByRegion;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	dependencies[1].dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
	dependencies[1].srcAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
	dependencies[1].dstAccessMask = vk::AccessFlagBits::eMemoryRead;
	dependencies[1].dependencyFlags = vk::DependencyFlagBits::eByRegion;*/

	std::uint32_t NumDependencies = 0;
	VkSubpassDependency SubpassDependencies[2];

	// DepthReadSubpass
	//{
	//	vk::SubpassDependency& SubpassDep = SubpassDependencies[NumDependencies++];
	//	SubpassDep.srcSubpass = 0;
	//	SubpassDep.dstSubpass = 1;
	//	SubpassDep.srcStageMask = vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests; // VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	//	SubpassDep.dstStageMask = vk::PipelineStageFlagBits::eFragmentShader; // VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	//	SubpassDep.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite; // VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	//	SubpassDep.dstAccessMask = vk::AccessFlagBits::eInputAttachmentRead; //VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
	//	SubpassDep.dependencyFlags = vk::DependencyFlagBits::eByRegion; 
	//}
	
	// Subpass dependencies

	std::array<VkSubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VkAccessFlagBits::VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VkAccessFlagBits::VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT;

	//VkSubpassDependency dependency{};
	//dependency.srcSubpass = 0;// VK_SUBPASS_EXTERNAL;
	//dependency.dstSubpass = 0;
	//dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	//dependency.srcAccessMask = 0;
	//dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	//dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	//dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT | VK_DEPENDENCY_VIEW_LOCAL_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pAttachments = attachmentDescs.data();
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = dependencies.data();

	//renderPassInfo.dependencyCount = 0;// static_cast<uint32_t>(dependencies.size());
	//renderPassInfo.pDependencies = dependencies.data();

	vkCreateRenderPass(Device->Get(), &renderPassInfo, nullptr, &RenderPass);
}

VulkanRenderpass::~VulkanRenderpass()
{
	//Device->Get()->destroyRenderPass(renderPass);
	Device = nullptr;
}
