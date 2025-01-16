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

#include <map>
#include "Engine/ClassBody.h"
#include "Engine/AbstractEngine.h"
#include "VulkanDevice.h"
#include "VulkanRenderPass.h"

class VulkanPipeline final : public IPipeline
{
	sClassBody(sClassConstructor, VulkanPipeline, IPipeline)
public:
	VulkanPipeline(VulkanDevice* Device, std::string InName, sPipelineDesc InDesc);

	virtual ~VulkanPipeline()
	{
		vkDestroyFramebuffer(Owner->Get(), FrameBuffer, nullptr);
		vkDestroyPipeline(Owner->Get(), Pipeline, nullptr);
		if (PipelineCache != VK_NULL_HANDLE)
			vkDestroyPipelineCache(Owner->Get(), PipelineCache, nullptr);
		RenderPass = nullptr;
		Compiled = false;
		Owner = nullptr;
	}

	VkPipeline Get() const { return	Pipeline; }
	VkPipelineLayout GetLayout() const { return PipelineLayout; }

	virtual sPipelineDesc GetPipelineDesc() const  override final { return Desc; }

	virtual bool IsCompiled() const override final
	{
		return Compiled;
	}

	virtual bool Compile(IFrameBuffer* FrameBuffer = nullptr) override final;
	virtual bool Compile(IRenderTarget* RT, IDepthTarget* Depth = nullptr) override final;
	virtual bool Compile(std::vector<IRenderTarget*> RTs, IDepthTarget* Depth = nullptr) override final;

	virtual bool Recompile() override final;

	VkFramebuffer FrameBuffer;
	VulkanRenderpass::SharedPtr RenderPass;
	sDimension2D FrameBufferDimension;

private:
	void CompilePipeline();

protected:
	VulkanDevice* Owner;
	std::string Name;
	sPipelineDesc Desc;
	bool Compiled;

	VkPipelineLayout PipelineLayout;
	VkGraphicsPipelineCreateInfo PipelineCreateInfo;

	VkPipeline Pipeline;
	VkPipelineCache PipelineCache;
};

class VulkanComputePipeline final : public IComputePipeline
{
    sClassBody(sClassConstructor, VulkanComputePipeline, IComputePipeline)
public:
    VulkanComputePipeline(VulkanDevice* InOwner, const std::string& InName, const sComputePipelineDesc& InDesc);
    virtual ~VulkanComputePipeline();

    virtual sComputePipelineDesc GetPipelineDesc() const override final { return Desc; }

    virtual bool Recompile() override final;

private:
    std::string Name;
    sComputePipelineDesc Desc;
};
