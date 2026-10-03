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
#pragma once

#include <set>
#include <map>
#include <vector>
#include "Engine/ClassBody.h"
#include "Engine/AbstractEngine.h"
#include "VulkanDevice.h"
#include "VulkanFormat.h"
#include <unordered_map>
#include <iostream>
#include <stdexcept>

class VulkanRasterizer final
{
	sBaseClassBody(sClassConstructor, VulkanRasterizer)
public:
	VulkanRasterizer(sRasterizerAttributeDesc InDesc)
		: RasterizerDesc(InDesc)
		, RasterizationState(VkPipelineRasterizationStateCreateInfo(VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO))
	{
		switch (InDesc.FillMode)
		{
		case ERasterizerFillMode::Solid:
			RasterizationState.polygonMode = VkPolygonMode::VK_POLYGON_MODE_FILL;
			break;
		case  ERasterizerFillMode::Wireframe:
			RasterizationState.polygonMode = VkPolygonMode::VK_POLYGON_MODE_LINE;
			break;
		}

		switch (InDesc.CullMode)
		{
		case ERasterizerCullMode::CCW:
			RasterizationState.cullMode = VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT;
			break;
		case ERasterizerCullMode::CW:
			RasterizationState.cullMode = VkCullModeFlagBits::VK_CULL_MODE_FRONT_BIT;
			break;
		case ERasterizerCullMode::None:
			RasterizationState.cullMode = VkCullModeFlagBits::VK_CULL_MODE_NONE;
			break;
		}

		VkPipelineRasterizationStateRasterizationOrderAMD PipelineRasterizationStateRasterizationOrderAMD;
		PipelineRasterizationStateRasterizationOrderAMD.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_RASTERIZATION_ORDER_AMD;
		PipelineRasterizationStateRasterizationOrderAMD.rasterizationOrder = VK_RASTERIZATION_ORDER_RELAXED_AMD;
		//PipelineRasterizationStateRasterizationOrderAMD.pNext = ;

		VkPipelineRasterizationConservativeStateCreateInfoEXT PipelineRasterizationConservativeStateCreateInfoEXT;
		PipelineRasterizationConservativeStateCreateInfoEXT.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT;
		PipelineRasterizationConservativeStateCreateInfoEXT.conservativeRasterizationMode = VkConservativeRasterizationModeEXT::VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT;
		//PipelineRasterizationConservativeStateCreateInfoEXT.extraPrimitiveOverestimationSize = VkPhysicalDeviceConservativeRasterizationPropertiesEXT::primitiveOverestimationSize;
		PipelineRasterizationConservativeStateCreateInfoEXT.flags = 0;
		//PipelineRasterizationConservativeStateCreateInfoEXT.pNext = ;

		//RasterizationState.pNext = &PipelineRasterizationStateRasterizationOrderAMD;

		//RasterizationState.depthClampEnable = ;
		//RasterizationState.rasterizerDiscardEnable = ;

		RasterizationState.frontFace = InDesc.FrontCounterClockwise ? VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE : VkFrontFace::VK_FRONT_FACE_CLOCKWISE;
		RasterizationState.depthBiasEnable = InDesc.DepthBias > 0.0f ? true : false;
		RasterizationState.depthBiasConstantFactor = InDesc.DepthBias;
		RasterizationState.depthBiasClamp = InDesc.DepthBiasClamp;
		RasterizationState.depthBiasSlopeFactor = InDesc.SlopeScaleDepthBias;
		RasterizationState.lineWidth = 1.0f;
	}

	VkPipelineRasterizationStateCreateInfo Get() const { return RasterizationState; }

	virtual	~VulkanRasterizer() = default;

private:
	sRasterizerAttributeDesc RasterizerDesc;
	VkPipelineRasterizationStateCreateInfo RasterizationState;
};

class VulkanStaticSamplerState
{
	sBaseClassBody(sClassConstructor, VulkanStaticSamplerState)
public:
	VulkanStaticSamplerState(sSamplerAttributeDesc InDesc)
		: SamplerDesc(InDesc)
		, SamplerCreateInfo(VkSamplerCreateInfo(VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO))
	{
		SamplerCreateInfo.compareEnable = false; // InDesc.SamplerComparisonFunction != ECompareFunction::Never;

		auto WrapMpde = [&](ESamplerAddressMode Mode) -> VkSamplerAddressMode
		{
			switch (Mode)
			{
			case ESamplerAddressMode::Border: return VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			case ESamplerAddressMode::Clamp: return VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			case ESamplerAddressMode::Mirror: return VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			case ESamplerAddressMode::MirrorOnce: return VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
			case ESamplerAddressMode::Wrap: return VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
			};
			return VkSamplerAddressMode();
		};

		auto Compare = [&](ECompareFunction Mode) -> VkCompareOp
		{
			switch (Mode)
			{
			case ECompareFunction::Less: return VkCompareOp::VK_COMPARE_OP_LESS;
			case ECompareFunction::LessEqual: return VkCompareOp::VK_COMPARE_OP_LESS_OR_EQUAL;
			case ECompareFunction::Greater: return VkCompareOp::VK_COMPARE_OP_GREATER;
			case ECompareFunction::GreaterEqual: return VkCompareOp::VK_COMPARE_OP_GREATER_OR_EQUAL;
			case ECompareFunction::Equal: return VkCompareOp::VK_COMPARE_OP_EQUAL;
			case ECompareFunction::NotEqual: return VkCompareOp::VK_COMPARE_OP_NOT_EQUAL;
			case ECompareFunction::Never: return VkCompareOp::VK_COMPARE_OP_NEVER;
			case ECompareFunction::Always: return VkCompareOp::VK_COMPARE_OP_ALWAYS;
			}
			return VkCompareOp::VK_COMPARE_OP_MAX_ENUM;
		};

		switch (SamplerDesc.Filter)
		{
		case ESamplerFilter::AnisotropicLinear:
			SamplerCreateInfo.magFilter = VkFilter::VK_FILTER_LINEAR;
			SamplerCreateInfo.minFilter = VkFilter::VK_FILTER_LINEAR;
			SamplerCreateInfo.mipmapMode = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR;
			break;
		case ESamplerFilter::AnisotropicPoint:
			SamplerCreateInfo.magFilter = VkFilter::VK_FILTER_LINEAR;
			SamplerCreateInfo.minFilter = VkFilter::VK_FILTER_LINEAR;
			SamplerCreateInfo.mipmapMode = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_NEAREST;
			break;
		case ESamplerFilter::Trilinear:
			SamplerCreateInfo.magFilter = VkFilter::VK_FILTER_LINEAR;
			SamplerCreateInfo.minFilter = VkFilter::VK_FILTER_LINEAR;
			SamplerCreateInfo.mipmapMode = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR;
			break;
		case ESamplerFilter::Bilinear:
			SamplerCreateInfo.magFilter = VkFilter::VK_FILTER_LINEAR;
			SamplerCreateInfo.minFilter = VkFilter::VK_FILTER_LINEAR;
			SamplerCreateInfo.mipmapMode = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_NEAREST;
			break;
		case ESamplerFilter::Point:
			SamplerCreateInfo.magFilter = VkFilter::VK_FILTER_NEAREST;
			SamplerCreateInfo.minFilter = VkFilter::VK_FILTER_NEAREST;
			SamplerCreateInfo.mipmapMode = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_NEAREST;
			break;
		}

		SamplerCreateInfo.flags;
		SamplerCreateInfo.unnormalizedCoordinates;

		if (SamplerDesc.BorderColor.R == 0.f && SamplerDesc.BorderColor.G == 0.f && SamplerDesc.BorderColor.B == 0.f)
		{
			if (SamplerDesc.BorderColor.A == 0.f)
			{
				SamplerCreateInfo.borderColor = VkBorderColor::VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
			}

			if (SamplerDesc.BorderColor.A == 1.f)
			{
				SamplerCreateInfo.borderColor = VkBorderColor::VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
			}
		}

		if (SamplerDesc.BorderColor.R == 1.f && SamplerDesc.BorderColor.G == 1.f && SamplerDesc.BorderColor.B == 1.f)
		{
			if (SamplerDesc.BorderColor.A == 1.f)
			{
				SamplerCreateInfo.borderColor = VkBorderColor::VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			}
		}

		SamplerCreateInfo.addressModeU = WrapMpde(SamplerDesc.AddressU);
		SamplerCreateInfo.addressModeV = WrapMpde(SamplerDesc.AddressV);
		SamplerCreateInfo.addressModeW = WrapMpde(SamplerDesc.AddressW);
		SamplerCreateInfo.mipLodBias = (float)SamplerDesc.MipBias;
		SamplerCreateInfo.compareOp = Compare(SamplerDesc.SamplerComparisonFunction);;
		SamplerCreateInfo.minLod = SamplerDesc.MinMipLevel;
		SamplerCreateInfo.maxLod = SamplerDesc.MaxMipLevel;		
		SamplerCreateInfo.maxAnisotropy = (float)SamplerDesc.MaxAnisotropy;
		SamplerCreateInfo.anisotropyEnable = SamplerDesc.MaxAnisotropy > 1 ? true : false;
	}

	VkSamplerCreateInfo GetCreateInfo() const { return SamplerCreateInfo; }
	VkSampler Get(VkDevice Device) const 
	{
		VkSampler Sampler;
		vkCreateSampler(Device, &SamplerCreateInfo, nullptr, &Sampler);
		return Sampler;
	}

	sSamplerAttributeDesc GetSamplerDesc() const { return SamplerDesc; }

	virtual ~VulkanStaticSamplerState() = default;

private:
	sSamplerAttributeDesc SamplerDesc;
	VkSamplerCreateInfo SamplerCreateInfo;
};

class VulkanSamplerState : public ISamplerState
{
	sClassBody(sClassConstructor, VulkanSamplerState, ISamplerState)
public:
	VulkanSamplerState(VulkanDevice* InOwner, std::string InName, sSamplerAttributeDesc InDesc)
		: Super()
		, Owner(InOwner)
		, Name(InName)
		, SamplerState(VulkanStaticSamplerState(InDesc))
	{
	}

	VulkanSamplerState(VulkanDevice* InOwner, sSamplerAttributeDesc InDesc)
		: Super()
		, Owner(InOwner)
		, Name("VulkanSamplerState_NoName")
		, SamplerState(VulkanStaticSamplerState(InDesc))
	{}

	virtual ~VulkanSamplerState()
	{
		Owner = nullptr;
	}

	virtual std::string GetName() const override { return Name; }
	virtual sSamplerAttributeDesc GetSamplerDesc() const override { return SamplerState.GetSamplerDesc(); }
	virtual std::uint32_t GetBindlessIndex() const override { return std::uint32_t(-1); }

	FORCEINLINE VulkanStaticSamplerState Get() const { return SamplerState;	}

private:
	VulkanDevice* Owner;
	std::string Name;
	VulkanStaticSamplerState SamplerState;
};

class VulkanDepthStencilState
{
	sBaseClassBody(sClassConstructor, VulkanDepthStencilState)
public:
	VulkanDepthStencilState(sDepthStencilAttributeDesc InDesc)
		: DepthStencilStateDesc(InDesc)
		, DepthStencilState(VkPipelineDepthStencilStateCreateInfo(VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO))

	{
		auto Compare = [&](ECompareFunction Mode) -> VkCompareOp
		{
			switch (Mode)
			{
			case ECompareFunction::Less: return VkCompareOp::VK_COMPARE_OP_LESS;
			case ECompareFunction::LessEqual: return VkCompareOp::VK_COMPARE_OP_LESS_OR_EQUAL;
			case ECompareFunction::Greater: return VkCompareOp::VK_COMPARE_OP_GREATER;
			case ECompareFunction::GreaterEqual: return VkCompareOp::VK_COMPARE_OP_GREATER_OR_EQUAL;
			case ECompareFunction::Equal: return VkCompareOp::VK_COMPARE_OP_EQUAL;
			case ECompareFunction::NotEqual: return VkCompareOp::VK_COMPARE_OP_NOT_EQUAL;
			case ECompareFunction::Never: return VkCompareOp::VK_COMPARE_OP_NEVER;
			case ECompareFunction::Always:	return VkCompareOp::VK_COMPARE_OP_ALWAYS;
			}
			return VkCompareOp();
		};

		auto Stencil = [&](EStencilOp Mode) -> VkStencilOp
		{
			switch (Mode)
			{
			case EStencilOp::Keep: return VkStencilOp::VK_STENCIL_OP_KEEP;
			case EStencilOp::Zero: return VkStencilOp::VK_STENCIL_OP_ZERO;
			case EStencilOp::Replace: return VkStencilOp::VK_STENCIL_OP_REPLACE;
			case EStencilOp::SaturatedIncrement: return VkStencilOp::VK_STENCIL_OP_INCREMENT_AND_CLAMP;
			case EStencilOp::SaturatedDecrement:	return VkStencilOp::VK_STENCIL_OP_DECREMENT_AND_CLAMP;
			case EStencilOp::Invert:	return VkStencilOp::VK_STENCIL_OP_INVERT;
			case EStencilOp::Increment: return VkStencilOp::VK_STENCIL_OP_INCREMENT_AND_WRAP;
			case EStencilOp::Decrement: return VkStencilOp::VK_STENCIL_OP_DECREMENT_AND_WRAP;
			}
			return VkStencilOp();
		};

		DepthStencilState.depthTestEnable = (/*InDesc.DepthTest != ECompareFunction::Always ||*/ InDesc.bEnableDepthWrite) ? true : false;
		DepthStencilState.depthCompareOp = Compare(InDesc.DepthTest);
		DepthStencilState.depthWriteEnable = InDesc.bEnableDepthWrite;

		DepthStencilState.depthBoundsTestEnable = false;
		DepthStencilState.minDepthBounds = 0.0f;
		DepthStencilState.maxDepthBounds = 1.0f;

		DepthStencilState.stencilTestEnable = InDesc.bEnableFrontFaceStencil || InDesc.bEnableBackFaceStencil;

		DepthStencilState.front.failOp = Stencil(InDesc.FrontFaceStencilFailStencilOp);
		DepthStencilState.front.passOp = Stencil(InDesc.FrontFacePassStencilOp);
		DepthStencilState.front.depthFailOp = Stencil(InDesc.FrontFaceDepthFailStencilOp);
		DepthStencilState.front.compareOp = Compare(InDesc.FrontFaceStencilTest);
		DepthStencilState.front.compareMask = InDesc.StencilReadMask;
		DepthStencilState.front.writeMask = InDesc.StencilWriteMask;
		DepthStencilState.front.reference = ~0U;

		DepthStencilState.back.failOp = Stencil(InDesc.BackFaceStencilFailStencilOp);
		DepthStencilState.back.passOp = Stencil(InDesc.BackFacePassStencilOp);
		DepthStencilState.back.depthFailOp = Stencil(InDesc.BackFaceDepthFailStencilOp);
		DepthStencilState.back.compareOp = Compare(InDesc.BackFaceStencilTest);
		DepthStencilState.back.compareMask = InDesc.StencilReadMask;
		DepthStencilState.back.writeMask = InDesc.StencilWriteMask;
		DepthStencilState.back.reference = ~0U;
	}

	VkPipelineDepthStencilStateCreateInfo Get() const { return DepthStencilState; }

	virtual ~VulkanDepthStencilState() = default;

private:
	sDepthStencilAttributeDesc DepthStencilStateDesc;
	VkPipelineDepthStencilStateCreateInfo DepthStencilState;
};

class VulkanBlendState
{
	sBaseClassBody(sClassConstructor, VulkanBlendState)
public:
	VulkanBlendState(sBlendAttributeDesc InDesc, std::size_t size)
		: BlendStateDesc(InDesc)
		, BlendState(VkPipelineColorBlendStateCreateInfo(VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO))
	{
		auto BlendOP = [&](EBlendOperation var) -> VkBlendOp
		{
			switch (var)
			{
			case EBlendOperation::Add: return VkBlendOp::VK_BLEND_OP_ADD;
			case EBlendOperation::Subtract: return VkBlendOp::VK_BLEND_OP_SUBTRACT;
			case EBlendOperation::Min: return VkBlendOp::VK_BLEND_OP_MIN;
			case EBlendOperation::Max: return VkBlendOp::VK_BLEND_OP_MAX;
			case EBlendOperation::ReverseSubtract: return VkBlendOp::VK_BLEND_OP_REVERSE_SUBTRACT;
			}
			return VkBlendOp();
		};

		auto BlendFactor = [&](EBlendFactor var) -> VkBlendFactor
		{
			switch (var)
			{
			case EBlendFactor::Zero: return VkBlendFactor::VK_BLEND_FACTOR_ZERO;
			case EBlendFactor::One: return VkBlendFactor::VK_BLEND_FACTOR_ONE;
			case EBlendFactor::SourceColor: return VkBlendFactor::VK_BLEND_FACTOR_SRC_COLOR;
			case EBlendFactor::InverseSourceColor: return VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
			case EBlendFactor::SourceAlpha: return VkBlendFactor::VK_BLEND_FACTOR_SRC_ALPHA;
			case EBlendFactor::InverseSourceAlpha:	return VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			case EBlendFactor::DestAlpha: return VkBlendFactor::VK_BLEND_FACTOR_DST_ALPHA;
			case EBlendFactor::InverseDestAlpha: return VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
			case EBlendFactor::DestColor: return VkBlendFactor::VK_BLEND_FACTOR_DST_COLOR;
			case EBlendFactor::InverseDestColor: return VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
			case EBlendFactor::BlendFactor: return VkBlendFactor::VK_BLEND_FACTOR_CONSTANT_COLOR;
			case EBlendFactor::InverseBlendFactor: return VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
			}
			return VkBlendFactor();
		};

		auto ColorWriteMask = [&](EColorWriteMask var) -> VkColorComponentFlags
		{
			switch (var)
			{
			case EColorWriteMask::NONE: return VkColorComponentFlags();
			case EColorWriteMask::RED: return VK_COLOR_COMPONENT_R_BIT;
			case EColorWriteMask::GREEN: return VK_COLOR_COMPONENT_G_BIT;
			case EColorWriteMask::BLUE:	return VK_COLOR_COMPONENT_B_BIT;
			case EColorWriteMask::ALPHA: return VK_COLOR_COMPONENT_A_BIT;
			case EColorWriteMask::RGB: return VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;
			case EColorWriteMask::RGBA:	return VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			case EColorWriteMask::RG: return VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT;
			case EColorWriteMask::BA: return VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			}
			return VkColorComponentFlags();
		};

		BlendState.logicOpEnable = false;
		BlendState.logicOp = VkLogicOp();

		for (std::size_t i = 0; i < size; i++)
		{
			if (i < BlendStateDesc.RenderTargets.size())
			{
				const auto& RT = BlendStateDesc.RenderTargets[i];
				VkPipelineColorBlendAttachmentState BlendAttachmentState;
				BlendAttachmentState.blendEnable = RT.bBlendEnable;

				BlendAttachmentState.srcColorBlendFactor = BlendFactor(RT.ColorSrcBlend);
				BlendAttachmentState.dstColorBlendFactor = BlendFactor(RT.ColorDestBlend);
				BlendAttachmentState.colorBlendOp = BlendOP(RT.ColorBlendOp);

				BlendAttachmentState.srcAlphaBlendFactor = BlendFactor(RT.AlphaSrcBlend);
				BlendAttachmentState.dstAlphaBlendFactor = BlendFactor(RT.AlphaDestBlend);
				BlendAttachmentState.alphaBlendOp = BlendOP(RT.AlphaBlendOp);

				BlendAttachmentState.colorWriteMask = ColorWriteMask(RT.ColorWriteMask);

				BlendAttachments.push_back(BlendAttachmentState);
			}
			else
			{
				VkPipelineColorBlendAttachmentState BlendAttachmentState = {};
				BlendAttachmentState.blendEnable = false;

				BlendAttachments.push_back(BlendAttachmentState);
			}
		}

		BlendState.pAttachments = BlendAttachments.data();
		BlendState.attachmentCount = (std::uint32_t)BlendAttachments.size();

		BlendState.blendConstants[0] = 1.0f;
		BlendState.blendConstants[1] = 1.0f;
		BlendState.blendConstants[2] = 1.0f;
		BlendState.blendConstants[3] = 1.0f;

		//BlendState.flags = ;
		//BlendState.pNext = ;
	}

	VkPipelineColorBlendStateCreateInfo Get() const { return BlendState; }

	virtual ~VulkanBlendState() = default;

private:
	sBlendAttributeDesc BlendStateDesc;
	VkPipelineColorBlendStateCreateInfo BlendState;
	std::vector<VkPipelineColorBlendAttachmentState> BlendAttachments;
};

struct sBindingStruct
{
	bool Instanced = false;
	std::uint32_t Stride = 0;
	sBindingStruct(bool instanced = false, std::uint32_t stride = 0)
		: Instanced(instanced)
		, Stride(stride)
	{
	}
};

class VulkanVertexAttribute
{
	sBaseClassBody(sClassConstructor, VulkanVertexAttribute)
public:
	VulkanVertexAttribute(std::vector<sVertexAttributeDesc> InDesc)
		: VertexAttributeDescData(InDesc)
		, VertexInputState(VkPipelineVertexInputStateCreateInfo(VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO))
	{
		//VertexInputState.flags = ;
		//VertexInputState.pNext = ;

		std::map<std::size_t, sBindingStruct> BindingList;
		for (std::size_t i = 0; i < VertexAttributeDescData.size(); i++)
		{
			VkVertexInputAttributeDescription VertexInputAttributeDescription = {};
			VertexInputAttributeDescription.location = (std::uint32_t)i;
			VertexInputAttributeDescription.binding = VertexAttributeDescData[i].InputSlot;
			VertexInputAttributeDescription.format = ConvertFormat_Format_To_VkFormat(VertexAttributeDescData[i].format);
			VertexInputAttributeDescription.offset = VertexAttributeDescData[i].offset;
			VertexInputAttributeDescriptions.push_back(VertexInputAttributeDescription);
			BindingList.insert({ VertexAttributeDescData[i].InputSlot, sBindingStruct(VertexAttributeDescData[i].isInstanced, (std::uint32_t)VertexAttributeDescData[i].Stride) });
		}

		for (const auto& Binding : BindingList)
		{
			VkVertexInputBindingDescription VertexInputBindingDescription = {};
			VertexInputBindingDescription.binding = (std::uint32_t)Binding.first;
			VertexInputBindingDescription.inputRate = Binding.second.Instanced ? VkVertexInputRate::VK_VERTEX_INPUT_RATE_INSTANCE : VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;
			VertexInputBindingDescription.stride =  Binding.second.Stride; // 32 : 80
			VertexInputBindingDescriptions.push_back(VertexInputBindingDescription);
		}

		VertexInputState.pVertexAttributeDescriptions = VertexInputAttributeDescriptions.data();
		VertexInputState.vertexAttributeDescriptionCount = (std::uint32_t)VertexInputAttributeDescriptions.size();

		VertexInputState.pVertexBindingDescriptions = VertexInputBindingDescriptions.data();
		VertexInputState.vertexBindingDescriptionCount = (std::uint32_t)VertexInputBindingDescriptions.size();

	}

	std::uint32_t GetStride(std::uint32_t Binding) const
	{
		std::uint32_t Stride = 0;

		return Stride;
	}

	std::uint32_t GetOffset(std::uint32_t Binding) const
	{
		std::uint32_t Offset = 0;

		return Offset;
	}

	VkPipelineVertexInputStateCreateInfo Get() const { return VertexInputState; }

	virtual ~VulkanVertexAttribute() = default;

private:
	std::vector<sVertexAttributeDesc> VertexAttributeDescData;
	std::vector<VkVertexInputAttributeDescription> VertexInputAttributeDescriptions;
	std::vector<VkVertexInputBindingDescription> VertexInputBindingDescriptions;
	VkPipelineVertexInputStateCreateInfo VertexInputState;
};

class VulkanMemoryManager
{
	sBaseClassBody(sClassConstructor, VulkanMemoryManager)
public:
	VulkanMemoryManager(VulkanDevice* device)
		: Device(device)
	{}

	~VulkanMemoryManager()
	{
		Cleanup();
	}

	VkDeviceMemory AllocateMemory(/*std::string ClassID,*/ VkDeviceSize size, uint32_t memoryTypeIndex)
	{
		VkMemoryAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = size;
		allocateInfo.memoryTypeIndex = memoryTypeIndex;

		VkDeviceMemory memory;
		if (vkAllocateMemory(Device->Get(), &allocateInfo, nullptr, &memory) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate device memory.");

		MemoryAllocations[memory] = size;
		TotalAllocatedMemory += size;

		return memory;
	}

	void FreeMemory(VkDeviceMemory memory)
	{
		auto it = MemoryAllocations.find(memory);
		if (it != MemoryAllocations.end())
		{
			TotalAllocatedMemory -= it->second;
			MemoryAllocations.erase(it);
		}

		vkFreeMemory(Device->Get(), memory, nullptr);
	}

	VkDeviceSize GetTotalAllocatedMemory() const
	{
		return TotalAllocatedMemory;
	}

	void ListAllocations() const
	{
		std::cout << "Memory Allocations:\n";
		for (const auto& [memory, size] : MemoryAllocations)
		{
			std::cout << "  Memory: " << memory << " Size: " << size << " bytes\n";
		}
		std::cout << "Total Allocated: " << TotalAllocatedMemory << " bytes\n";
	}

private:
	void Cleanup()
	{
		for (const auto& [memory, size] : MemoryAllocations)
		{
			vkFreeMemory(Device->Get(), memory, nullptr);
		}
		MemoryAllocations.clear();
		TotalAllocatedMemory = 0;
	}

	VulkanDevice* Device;
	std::unordered_map<VkDeviceMemory, VkDeviceSize> MemoryAllocations;
	VkDeviceSize TotalAllocatedMemory = 0;
};

class VulkanBindlessDescriptorPool 
{
	sBaseClassBody(sClassConstructor, VulkanBindlessDescriptorPool)
public:
	VulkanBindlessDescriptorPool(VulkanDevice* device, uint32_t maxImageDescriptors = 16536, uint32_t maxBufferDescriptors = 16536)
		: Device(device)
		, maxImageDescriptors(maxImageDescriptors)
		, maxBufferDescriptors(maxBufferDescriptors)
	{
		CreateDescriptorPool();
		CreateDescriptorSetLayout();
		AllocateDescriptorSets();
	}

	~VulkanBindlessDescriptorPool() 
	{
		vkDestroyDescriptorPool(Device->Get(), bindlessPool, nullptr);
		vkDestroyDescriptorSetLayout(Device->Get(), bindlessLayout, nullptr);
	}

	VkDescriptorSetLayout GetDescriptorSetLayout() const 
	{
		return bindlessLayout;
	}

	VkDescriptorSet GetDescriptorSet() const
	{
		return globalBindlessSet;
	}

	void UpdateImageDescriptor(uint32_t binding, uint32_t arrayIndex, VkDescriptorImageInfo* imageInfo)
	{
		VkWriteDescriptorSet writeDescriptorSet{};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = globalBindlessSet;
		writeDescriptorSet.dstBinding = binding;
		writeDescriptorSet.dstArrayElement = arrayIndex;	// The bindless ?
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSet.pImageInfo = imageInfo;

		vkUpdateDescriptorSets(Device->Get(), 1, &writeDescriptorSet, 0, nullptr);
	}

	void UpdateBufferDescriptor(uint32_t binding, uint32_t arrayIndex, VkDescriptorBufferInfo* bufferInfo)
	{
		VkWriteDescriptorSet writeDescriptorSet{};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = globalBindlessSet;
		writeDescriptorSet.dstBinding = binding;
		writeDescriptorSet.dstArrayElement = arrayIndex;	// The bindless ?
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		writeDescriptorSet.pBufferInfo = bufferInfo;

		vkUpdateDescriptorSets(Device->Get(), 1, &writeDescriptorSet, 0, nullptr);
	}

private:
	void CreateDescriptorPool() 
	{
		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSize.descriptorCount = maxBindlessResources;

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		poolInfo.maxSets = 1;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

		vkCreateDescriptorPool(Device->Get(), &poolInfo, nullptr, &bindlessPool);
	}

	void CreateDescriptorSetLayout() 
	{
		/*auto GetStageFlags = [](const eShaderType ShaderType) -> VkShaderStageFlagBits
			{
				if (ShaderType == eShaderType::Vertex)
					return VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
				else if (ShaderType == eShaderType::Pixel)
					return VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
				else if (ShaderType == eShaderType::Geometry)
					return VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT;
				else if (ShaderType == eShaderType::HULL)
					return VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
				else if (ShaderType == eShaderType::Domain)
					return VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
				else if (ShaderType == eShaderType::Mesh)
					return VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT;
				else if (ShaderType == eShaderType::Amplification)
					return VkShaderStageFlagBits::VK_SHADER_STAGE_TASK_BIT_EXT;
				return VkShaderStageFlagBits();
			};

		std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;
		std::vector<VkPushConstantRange> m_pushConstantRanges;

		for (const auto& Binding : Bindings)
		{
			if (Binding.GetDescriptorType() == EDescriptorType::UniformBuffer)
			{
				descriptorSetLayoutBindings.push_back(VkDescriptorSetLayoutBinding());
				descriptorSetLayoutBindings.back().binding = Binding.Location;
				descriptorSetLayoutBindings.back().descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorSetLayoutBindings.back().descriptorCount = Binding.Size;
				descriptorSetLayoutBindings.back().stageFlags = GetStageFlags(Binding.ShaderType);
				descriptorSetLayoutBindings.back().pImmutableSamplers = nullptr;
			}
			else if (Binding.GetDescriptorType() == EDescriptorType::Sampler)
			{
				descriptorSetLayoutBindings.push_back(VkDescriptorSetLayoutBinding());
				descriptorSetLayoutBindings.back().binding = Binding.Location;
				descriptorSetLayoutBindings.back().descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorSetLayoutBindings.back().descriptorCount = Binding.Size;
				descriptorSetLayoutBindings.back().stageFlags = GetStageFlags(Binding.ShaderType);
				descriptorSetLayoutBindings.back().pImmutableSamplers = nullptr;
			}
			else if (Binding.GetDescriptorType() == EDescriptorType::Texture)
			{
				continue;
			}
			else if (Binding.GetDescriptorType() == EDescriptorType::UAV)
			{
				descriptorSetLayoutBindings.push_back(VkDescriptorSetLayoutBinding());
				descriptorSetLayoutBindings.back().binding = Binding.Location;
				descriptorSetLayoutBindings.back().descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				descriptorSetLayoutBindings.back().descriptorCount = Binding.Size;
				descriptorSetLayoutBindings.back().stageFlags = GetStageFlags(Binding.ShaderType);
				descriptorSetLayoutBindings.back().pImmutableSamplers = nullptr;
			}
			else if (Binding.GetDescriptorType() == EDescriptorType::e32BitConstant)
			{
				m_pushConstantRanges.push_back(VkPushConstantRange());
				m_pushConstantRanges.back().offset = 0;
				m_pushConstantRanges.back().size = Binding.Size;
				m_pushConstantRanges.back().stageFlags = GetStageFlags(Binding.ShaderType);
			}
		}
		
		layoutInfo.pBindings = descriptorSetLayoutBindings.data();

		*/

		//std::vector<VkDescriptorSetLayoutBinding> bindings = {
		//	// Binding 0: Textures
		//	{ 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 500000, VK_SHADER_STAGE_ALL, nullptr },

		//	// Binding 1: StructuredBuffers
		//	{ 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,        100000, VK_SHADER_STAGE_ALL, nullptr },

		//	// Binding 2: UAV Textures
		//	{ 2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,         10000,  VK_SHADER_STAGE_ALL, nullptr }
		//};

		VkDescriptorSetLayoutBinding binding{};
		binding.binding = 0;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		binding.descriptorCount = maxBindlessResources;
		binding.stageFlags = VK_SHADER_STAGE_ALL;

		VkDescriptorBindingFlags bindlessFlags =
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
			VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
			VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;

		VkDescriptorSetLayoutBindingFlagsCreateInfo extendedInfo{};
		extendedInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		extendedInfo.bindingCount = 1;
		extendedInfo.pBindingFlags = &bindlessFlags;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.pNext = &extendedInfo;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &binding;
		layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

		vkCreateDescriptorSetLayout(Device->Get(), &layoutInfo, nullptr, &bindlessLayout);
	}

	void AllocateDescriptorSets() 
	{
		uint32_t maxBinding = maxBindlessResources - 1;

		VkDescriptorSetVariableDescriptorCountAllocateInfo countInfo{};
		countInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
		countInfo.descriptorSetCount = 1;
		countInfo.pDescriptorCounts = &maxBinding;

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = &countInfo;
		allocInfo.descriptorPool = bindlessPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &bindlessLayout;

		vkAllocateDescriptorSets(Device->Get(), &allocInfo, &globalBindlessSet);
	}

	//VkDescriptorPool GenerateDescriptorPool(std::uint32_t numSets) const
	//{
	//	auto AddDescriptorTypeToPool = [](std::vector<VkDescriptorPoolSize>& poolSizes, VkDescriptorType type, std::uint32_t count)
	//		{
	//			auto it = poolSizes.begin();
	//			for (; it != poolSizes.end(); ++it)
	//			{
	//				if (type == it->type)
	//				{
	//					it->descriptorCount += count;
	//					return;
	//				}
	//			}

	//			if (it == poolSizes.end())
	//			{
	//				VkDescriptorPoolSize poolSize;
	//				poolSize.type = type;
	//				poolSize.descriptorCount = count;
	//				poolSizes.push_back(poolSize);
	//			}
	//		};

	//	std::vector<VkDescriptorPoolSize> poolSizes;
	//	std::uint32_t Samplercount = 0;
	//	for (const auto& Binding : BindingDesc)
	//	{
	//		if (Binding.GetDescriptorType() == EDescriptorType::UniformBuffer)
	//		{
	//			AddDescriptorTypeToPool(poolSizes, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, numSets * Binding.Size);
	//		}
	//		else if (Binding.GetDescriptorType() == EDescriptorType::Sampler)
	//		{
	//			Samplercount++;
	//			continue;
	//		}
	//		else if (Binding.GetDescriptorType() == EDescriptorType::Texture)
	//		{
	//			AddDescriptorTypeToPool(poolSizes, VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, numSets * Binding.Size);
	//		}
	//		else if (Binding.GetDescriptorType() == EDescriptorType::UAV)
	//		{
	//			AddDescriptorTypeToPool(poolSizes, VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, numSets * Binding.Size);
	//		}
	//		else if (Binding.GetDescriptorType() == EDescriptorType::e32BitConstant)
	//		{
	//		}
	//	}

	//	if (Samplercount > 0)
	//		AddDescriptorTypeToPool(poolSizes, VK_DESCRIPTOR_TYPE_SAMPLER, numSets * Samplercount);

	//	// no need to add push constants

	//	VkDescriptorPoolCreateInfo poolInfo{};
	//	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	//	poolInfo.pNext = nullptr;
	//	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	//	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	//	poolInfo.pPoolSizes = poolSizes.data();
	//	poolInfo.maxSets = numSets;

	//	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	//	VkResult res = vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &descriptorPool);

	//	return descriptorPool;
	//}

private:
	VulkanDevice* Device;
	VkDescriptorPool bindlessPool;
	VkDescriptorSetLayout bindlessLayout;
	VkDescriptorSet globalBindlessSet;
	uint32_t maxImageDescriptors;
	uint32_t maxBufferDescriptors;

	uint32_t maxBindlessResources = 100000;
};
