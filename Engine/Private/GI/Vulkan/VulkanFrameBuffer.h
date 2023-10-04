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

#include <memory>
#include <vector>
#include "Engine/AbstractEngine.h"

class VulkanFrameBuffer final : public IFrameBuffer
{
	sClassBody(sClassConstructor, VulkanFrameBuffer, IFrameBuffer)
private:



public:
	VulkanFrameBuffer(const std::string InName, const sFrameBufferAttachmentInfo& InAttachments);

	virtual ~VulkanFrameBuffer()
	{

	}

	virtual std::string GetName() const override final
	{
		return "";
	}
	virtual sFrameBufferAttachmentInfo GetAttachmentInfo() const override final
	{
		return sFrameBufferAttachmentInfo();
	}
	virtual std::size_t GetAttachmentCount() const override final
	{
		return 0;
	}
	virtual std::size_t GetRenderTargetAttachmentCount() const override final
	{
		return 0;
	}

	virtual std::vector<IRenderTarget*> GetRenderTargets() const override final { return std::vector<IRenderTarget*>(); }
	virtual IRenderTarget* GetRenderTarget(std::size_t Index) const override final { return nullptr; }
	virtual void AttachRenderTarget(const IRenderTarget::SharedPtr& RenderTarget, std::optional<std::size_t> Index = std::nullopt) override final { }

	virtual std::vector<IUnorderedAccessTarget*> GetUnorderedAccessTargets() const override final { return std::vector<IUnorderedAccessTarget*>(); }
	virtual IUnorderedAccessTarget* GetUnorderedAccessTarget(std::size_t Index) const override final { return nullptr; }
	virtual void AttachUnorderedAccessTarget(const IUnorderedAccessTarget::SharedPtr& UnorderedAccessTarget, std::optional<std::size_t> Index = std::nullopt) override final { }

	virtual IDepthTarget* GetDepthTarget() const override final { return nullptr; }
	virtual void SetDepthTarget(const IDepthTarget::SharedPtr& DepthTarget) override final { }
};
