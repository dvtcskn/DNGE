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
#include "VulkanPipeline.h"
#include <stdexcept>
#include "VulkanShader.h"
#include "VulkanException.h"
#include "VulkanShaderStates.h"
#include "VulkanFrameBuffer.h"

VulkanPipeline::VulkanPipeline(VulkanDevice* Device, std::string InName, sPipelineDesc InDesc)
	: Name(InName)
	, Desc(InDesc)
	, Owner(Device)
	, PipelineCreateInfo(VkGraphicsPipelineCreateInfo(VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO))
	, Compiled(false)
{
	Compile();
}

bool VulkanPipeline::Compile(IFrameBuffer* InFrameBuffer)
{
	if (Compiled)
		return false;

	if (!InFrameBuffer)
		return false;

	VulkanFrameBuffer* FB = Cast<VulkanFrameBuffer>(InFrameBuffer);

	std::size_t RenderTargetAttachmentCount = 0;

	std::vector<VkImageView> attachments;
	for (auto& FrameBufferAttachment : FB->RenderTargets)
	{
		attachments.push_back(FrameBufferAttachment->GetView());
		//if (FrameBufferAttachment->IsUAV_Allowed())
		//	attachments.push_back(FrameBufferAttachment->GetUAV_View());
		RenderTargetAttachmentCount++;
	}
	for (auto& FrameBufferAttachment : FB->UnorderedAccessTargets)
	{
		attachments.push_back(FrameBufferAttachment->GetView());
	}
	attachments.push_back(FB->DepthTarget->GetView());

	RenderPass = VulkanRenderpass::Create(Owner, Name + "_RenderPass", FB->GetAttachmentInfo());

	VkFramebufferCreateInfo FramebufferCreateInfo = {};
	FramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	FramebufferCreateInfo.pNext = NULL;
	FramebufferCreateInfo.renderPass = RenderPass->Get();
	FramebufferCreateInfo.pAttachments = attachments.data();
	FramebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	FramebufferCreateInfo.width = FB->GetAttachmentInfo().Desc.Dimensions.X;
	FramebufferCreateInfo.height = FB->GetAttachmentInfo().Desc.Dimensions.Y;
	FramebufferCreateInfo.layers = 1;
	FramebufferCreateInfo.flags = 0;// VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR;

	FrameBufferDimension.Width = FB->GetAttachmentInfo().Desc.Dimensions.X;
	FrameBufferDimension.Height = FB->GetAttachmentInfo().Desc.Dimensions.Y;

	vkCreateFramebuffer(Owner->Get(), &FramebufferCreateInfo, nullptr, &FrameBuffer);

	VulkanBlendState BlendState(Desc.BlendAttribute, RenderTargetAttachmentCount);
	auto Blend = BlendState.Get();
	PipelineCreateInfo.pColorBlendState = &Blend;

	CompilePipeline();

	return true;
}

bool VulkanPipeline::Compile(IRenderTarget* InRT, IDepthTarget* Depth)
{
	if (Compiled)
		return false;

	VulkanRenderTarget* RT = Cast<VulkanRenderTarget>(InRT);
	
	sFrameBufferAttachmentInfo AttachmentInfo;
	AttachmentInfo.Desc = RT->GetDesc();
	AttachmentInfo.AddFrameBuffer(RT->GetFormat());

	std::vector<VkImageView> attachments;
	attachments.push_back(RT->GetView());
	//if (RT->IsUAV_Allowed())
	//	attachments.push_back(RT->GetUAV_View());

	if (Depth)
	{
		attachments.push_back(Cast<VulkanDepthTarget>(Depth)->GetView());
		AttachmentInfo.DepthFormat = Cast<VulkanDepthTarget>(Depth)->GetFormat();
	}

	RenderPass = VulkanRenderpass::Create(Owner, Name + "_RenderPass", AttachmentInfo);

	VkFramebufferCreateInfo FramebufferCreateInfo = {};
	FramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	FramebufferCreateInfo.pNext = NULL;
	FramebufferCreateInfo.renderPass = RenderPass->Get();
	FramebufferCreateInfo.pAttachments = attachments.data();
	FramebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	FramebufferCreateInfo.width = RT->GetDesc().Dimensions.X;
	FramebufferCreateInfo.height = RT->GetDesc().Dimensions.Y;
	FramebufferCreateInfo.layers = 1;

	FrameBufferDimension.Width = RT->GetDesc().Dimensions.X;
	FrameBufferDimension.Height = RT->GetDesc().Dimensions.Y;

	vkCreateFramebuffer(Owner->Get(), &FramebufferCreateInfo, nullptr, &FrameBuffer);

	VulkanBlendState BlendState(Desc.BlendAttribute, 1);
	auto Blend = BlendState.Get();
	PipelineCreateInfo.pColorBlendState = &Blend;

	CompilePipeline();

	return true;
}

bool VulkanPipeline::Compile(std::vector<IRenderTarget*> RTs, IDepthTarget* Depth)
{
	if (Compiled)
		return false;

	sFrameBufferAttachmentInfo AttachmentInfo;

	std::vector<VkImageView> attachments;
	for (auto& RT : RTs)
	{
		VulkanRenderTarget* pRT = Cast<VulkanRenderTarget>(RT);
		attachments.push_back(pRT->GetView());
		//if (pRT->IsUAV_Allowed())
		//	attachments.push_back(pRT->GetUAV_View());
		AttachmentInfo.AddFrameBuffer(pRT->GetFormat());
		AttachmentInfo.Desc = pRT->GetDesc();
	}

	if (Depth)
	{
		attachments.push_back(Cast<VulkanDepthTarget>(Depth)->GetView());
		AttachmentInfo.DepthFormat = Cast<VulkanDepthTarget>(Depth)->GetFormat();
	}

	RenderPass = VulkanRenderpass::Create(Owner, Name + "_RenderPass", AttachmentInfo);

	VkFramebufferCreateInfo FramebufferCreateInfo = {};
	FramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	FramebufferCreateInfo.pNext = NULL;
	FramebufferCreateInfo.renderPass = RenderPass->Get();
	FramebufferCreateInfo.pAttachments = attachments.data();
	FramebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	FramebufferCreateInfo.width = AttachmentInfo.Desc.Dimensions.X;
	FramebufferCreateInfo.height = AttachmentInfo.Desc.Dimensions.Y;
	FramebufferCreateInfo.layers = 1;

	FrameBufferDimension.Width = AttachmentInfo.Desc.Dimensions.X;
	FrameBufferDimension.Height = AttachmentInfo.Desc.Dimensions.Y;

	vkCreateFramebuffer(Owner->Get(), &FramebufferCreateInfo, nullptr, &FrameBuffer);

	VulkanBlendState BlendState(Desc.BlendAttribute, RTs.size());
	auto Blend = BlendState.Get();
	PipelineCreateInfo.pColorBlendState = &Blend;

	CompilePipeline();

	return true;
}

bool VulkanPipeline::Recompile()
{
	return false;
}

void VulkanPipeline::CompilePipeline()
{
	//std::vector<VkShaderModule> shader_modules;

	std::vector<VkPipelineShaderStageCreateInfo> stage_create_infos;

	// Create specialization info from tracked state. This is shared by all shaders.
	//std::vector<uint8_t>                  data{};
	//std::vector<VkSpecializationMapEntry> map_entries{};

	//const auto specialization_constant_state = pipeline_state.get_specialization_constant_state().get_specialization_constant_state();

	//for (const auto specialization_constant : specialization_constant_state)
	//{
	//	map_entries.push_back({ specialization_constant.first, (std::uint32_t)(data.size()), specialization_constant.second.size() });
	//	data.insert(data.end(), specialization_constant.second.begin(), specialization_constant.second.end());
	//}

	//VkSpecializationInfo specialization_info{};
	//specialization_info.mapEntryCount = (std::uint32_t)(map_entries.size());
	//specialization_info.pMapEntries = map_entries.data();
	//specialization_info.dataSize = data.size();
	//specialization_info.pData = data.data();

	//for (const ShaderModule* shader_module : pipeline_state.get_pipeline_layout().get_shader_modules())
	//{
	//	VkPipelineShaderStageCreateInfo stage_create_info{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };

	//	stage_create_info.stage = shader_module->get_stage();
	//	stage_create_info.pName = shader_module->get_entry_point().c_str();

	//	// Create the Vulkan handle
	//	VkShaderModuleCreateInfo vk_create_info{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };

	//	vk_create_info.codeSize = shader_module->get_binary().size() * sizeof(uint32_t);
	//	vk_create_info.pCode = shader_module->get_binary().data();

	//	VkResult result = vkCreateShaderModule(Owner->Get(), &vk_create_info, nullptr, &stage_create_info.module);
	//	if (result != VK_SUCCESS)
	//	{
	//		throw VulkanException{ result };
	//	}

	//	device.get_debug_utils().set_debug_name(device.get_handle(),
	//		VK_OBJECT_TYPE_SHADER_MODULE, reinterpret_cast<uint64_t>(stage_create_info.module),
	//		shader_module->get_debug_name().c_str());

	//	stage_create_info.pSpecializationInfo = &specialization_info;

	//	stage_create_infos.push_back(stage_create_info);
	//	shader_modules.push_back(stage_create_info.module);
	//}

	/*std::vector<VulkanShader::SharedPtr> Shaders;
	for (const auto& Attachment : Desc.ShaderAttachments)
	{
		VulkanShader::SharedPtr Shader = VulkanShader::Create(Owner, Attachment);
		Shaders.push_back(Shader);

		shader_modules.insert(shader_modules.end(), Shader->shader_modules.begin(), Shader->shader_modules.end());
		stage_create_infos.push_back(Shader->shader_stage);
	}*/

	for (const auto& Attachment : Desc.ShaderAttachments)
	{
		VkPipelineShaderStageCreateInfo shader_stage = {};
		shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		switch (Attachment.Type)
		{
		case eShaderType::Vertex:
			shader_stage.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
			break;
		case eShaderType::Domain:
			//language = EShLangVertex;
			//shader_stage.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
			break;
		case eShaderType::Geometry:
			shader_stage.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT;
			break;
		case eShaderType::HULL:
			//shader_stage.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
			break;
		case eShaderType::Pixel:
			shader_stage.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
			break;
		case eShaderType::Compute:
			shader_stage.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT;
			break;
		case eShaderType::Amplification:
			//shader_stage.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
			break;
		case eShaderType::Mesh:
			shader_stage.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT;
			break;
		}

		auto pShader = Owner->CompileShader(Attachment, true);
		//Shaders.push_back(pShader);

		// Create shader module from generated SPIR-V
		VkShaderModule           shader_module = VK_NULL_HANDLE;
		VkShaderModuleCreateInfo module_create_info{};
		module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		module_create_info.codeSize = pShader->GetByteCodeSize();//spirv.size() * sizeof(uint32_t);
		module_create_info.pCode = (std::uint32_t*)pShader->GetByteCode();
		module_create_info.flags = 0;
		VK_CHECK(vkCreateShaderModule(Owner->Get(), &module_create_info, nullptr, &shader_module));

		//shader_stage.stage = stage;
		shader_stage.module = shader_module;
		shader_stage.pName = Attachment.FunctionName.c_str();
		shader_stage.flags = 0;
		//shader_stage.pSpecializationInfo = ;
		assert(shader_stage.module != VK_NULL_HANDLE);
		stage_create_infos.push_back(shader_stage);
	}


	PipelineCreateInfo.stageCount = (std::uint32_t)(stage_create_infos.size());
	PipelineCreateInfo.pStages = stage_create_infos.data();

	VkPipelineInputAssemblyStateCreateInfo input_assembly_state{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };

	switch (Desc.PrimitiveTopologyType)
	{
	case EPrimitiveType::ePOINT_LIST:
		input_assembly_state.topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		break;
	case EPrimitiveType::eTRIANGLE_LIST:
		input_assembly_state.topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		break;
	case EPrimitiveType::eTRIANGLE_STRIP:
		input_assembly_state.topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		break;
	case EPrimitiveType::eLINE_LIST:
		input_assembly_state.topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		break;
	}
	//input_assembly_state.primitiveRestartEnable = false;

	VkPipelineViewportStateCreateInfo viewport_state{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };

	auto VP = Owner->GetViewport();
	VkViewport Viewport = {};
	Viewport.width = VP.Width;
	Viewport.height = VP.Height;
	Viewport.minDepth = VP.MinDepth;
	Viewport.maxDepth = VP.MaxDepth;
	Viewport.x = VP.TopLeftX;
	Viewport.y = VP.TopLeftY;

	VkRect2D Rect = {};
	Rect.extent.width = VP.Width;
	Rect.extent.height = VP.Height;
	Rect.offset.x = 0;
	Rect.offset.y = 0;

	viewport_state.viewportCount = 1;
	viewport_state.pViewports = &Viewport;
	viewport_state.scissorCount = 1;
	viewport_state.pScissors = &Rect;

	VkPipelineMultisampleStateCreateInfo multisample_state{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };

	//multisample_state.sampleShadingEnable = false;
	multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	//multisample_state.minSampleShading = 0.0f;
	multisample_state.alphaToCoverageEnable = Desc.BlendAttribute.balphaToCoverage;
	//multisample_state.alphaToOneEnable = false;
	//multisample_state.pSampleMask = ;

	std::array<VkDynamicState, 9> dynamic_states
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_LINE_WIDTH,
		VK_DYNAMIC_STATE_DEPTH_BIAS,
		VK_DYNAMIC_STATE_BLEND_CONSTANTS,
		VK_DYNAMIC_STATE_DEPTH_BOUNDS,
		VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
		VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
		VK_DYNAMIC_STATE_STENCIL_REFERENCE,
	};

	VkPipelineDynamicStateCreateInfo dynamic_state{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };

	dynamic_state.pDynamicStates = dynamic_states.data();
	dynamic_state.dynamicStateCount = (std::uint32_t)(dynamic_states.size());

	VulkanVertexAttribute VertexAttribute(Desc.VertexLayout);
	auto Vertex = VertexAttribute.Get();
	PipelineCreateInfo.pVertexInputState = &Vertex;
	PipelineCreateInfo.pInputAssemblyState = &input_assembly_state;
	PipelineCreateInfo.pViewportState = &viewport_state;
	VulkanRasterizer Rasterizer(Desc.RasterizerAttribute);
	auto Raster = Rasterizer.Get();
	PipelineCreateInfo.pRasterizationState = &Raster;
	PipelineCreateInfo.pMultisampleState = &multisample_state;
	VulkanDepthStencilState DepthStencilState(Desc.DepthStencilAttribute);
	auto DepthStencil = DepthStencilState.Get();
	PipelineCreateInfo.pDepthStencilState = &DepthStencil;
	PipelineCreateInfo.pDynamicState = &dynamic_state;

	VkDescriptorSetLayout Layouts = Owner->GetPushDescriptorSetLayout();

	VkPipelineLayoutCreateInfo pipeline_layout_info = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipeline_layout_info.pSetLayouts = &Layouts;
	pipeline_layout_info.setLayoutCount = 1;

	/*std::vector<VkPushConstantRange> push_constants;
	{
		VkPushConstantRange push_constant;
		push_constant.offset = 0;
		push_constant.size = sizeof(sMeshConstantBufferAttributes);
		push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		push_constants.push_back(push_constant);
	}
	{
		VkPushConstantRange push_constant;
		push_constant.offset = 0;
		push_constant.size = 256;
		push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		push_constants.push_back(push_constant);
	}

	pipeline_layout_info.pPushConstantRanges = push_constants.data();
	pipeline_layout_info.pushConstantRangeCount = push_constants.size();*/

	vkCreatePipelineLayout(Owner->Get(), &pipeline_layout_info, nullptr, &PipelineLayout);

	PipelineCreateInfo.layout = PipelineLayout;

	PipelineCreateInfo.renderPass = RenderPass->Get();
	PipelineCreateInfo.subpass = RenderPass->SubpassIndex();

	auto result = vkCreateGraphicsPipelines(Owner->Get(), nullptr, /*PipelineCache,*/ 1, &PipelineCreateInfo, nullptr, &Pipeline);

	if (result != VK_SUCCESS)
	{
		throw "Cannot create GraphicsPipeline";
	}

	Compiled = true;
}

VulkanComputePipeline::VulkanComputePipeline(VulkanDevice* InOwner, const std::string& InName, const sComputePipelineDesc& InDesc)
	: Name(InName)
	, Desc(InDesc)
{
	//const ShaderModule* shader_module = pipeline_state.get_pipeline_layout().get_shader_modules().front();

	//if (shader_module->get_stage() != VK_SHADER_STAGE_COMPUTE_BIT)
	//{
	//	throw VulkanException{ VK_ERROR_INVALID_SHADER_NV, "Shader module stage is not compute" };
	//}

	//VkPipelineShaderStageCreateInfo stage{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };

	//stage.stage = shader_module->get_stage();
	//stage.pName = shader_module->get_entry_point().c_str();

	//// Create the Vulkan handle
	//VkShaderModuleCreateInfo vk_create_info{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };

	//vk_create_info.codeSize = shader_module->get_binary().size() * sizeof(uint32_t);
	//vk_create_info.pCode = shader_module->get_binary().data();

	//VkResult result = vkCreateShaderModule(device.get_handle(), &vk_create_info, nullptr, &stage.module);
	//if (result != VK_SUCCESS)
	//{
	//	throw VulkanException{ result };
	//}

	//device.get_debug_utils().set_debug_name(device.get_handle(),
	//	VK_OBJECT_TYPE_SHADER_MODULE, reinterpret_cast<uint64_t>(stage.module),
	//	shader_module->get_debug_name().c_str());

	//// Create specialization info from tracked state.
	//std::vector<uint8_t>                  data{};
	//std::vector<VkSpecializationMapEntry> map_entries{};

	//const auto specialization_constant_state = pipeline_state.get_specialization_constant_state().get_specialization_constant_state();

	//for (const auto specialization_constant : specialization_constant_state)
	//{
	//	map_entries.push_back({ specialization_constant.first, to_u32(data.size()), specialization_constant.second.size() });
	//	data.insert(data.end(), specialization_constant.second.begin(), specialization_constant.second.end());
	//}

	//VkSpecializationInfo specialization_info{};
	//specialization_info.mapEntryCount = to_u32(map_entries.size());
	//specialization_info.pMapEntries = map_entries.data();
	//specialization_info.dataSize = data.size();
	//specialization_info.pData = data.data();

	//stage.pSpecializationInfo = &specialization_info;

	//VkComputePipelineCreateInfo PipelineCreateInfo{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };

	//PipelineCreateInfo.layout = pipeline_state.get_pipeline_layout().get_handle();
	//PipelineCreateInfo.stage = stage;

	//result = vkCreateComputePipelines(device.get_handle(), pipeline_cache, 1, &PipelineCreateInfo, nullptr, &handle);

	//if (result != VK_SUCCESS)
	//{
	//	throw VulkanException{ result, "Cannot create ComputePipelines" };
	//}

	//vkDestroyShaderModule(device.get_handle(), stage.module, nullptr);
}

VulkanComputePipeline::~VulkanComputePipeline()
{
}

bool VulkanComputePipeline::Recompile()
{
	return false;
}
