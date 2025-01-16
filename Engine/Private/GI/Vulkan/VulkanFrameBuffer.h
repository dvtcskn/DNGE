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
#include "VulkanDevice.h"
#include "VulkanRenderPass.h"
#include "VulkanShaderStates.h"
#include "VulkanFormat.h"

class VulkanRenderTarget final : public IRenderTarget
{
	sClassBody(sClassConstructor, VulkanRenderTarget, IRenderTarget)
public:
	VulkanRenderTarget(VulkanDevice* InOwner, const std::string InName, const EFormat Format, const sFBODesc& Desc, bool InIsSRVAllowed = true, bool InIsUnorderedAccessAllowed = false);
	virtual ~VulkanRenderTarget();

	virtual void SetDefaultRootParameterIndex(std::uint32_t RootParameterIndex) override final {}
	virtual std::uint32_t GetDefaultRootParameterIndex() const override final { return 0; }

	inline sFBODesc GetDesc() const { return Desc; }
	const VkImage& GetImage() const { return Image; }
	const VkImageView& GetView() const { return View; }
	const VkImageView& GetUAV_View() const { return View; }
	virtual void* GetNativeTexture() const override final { return Image; }

	EFormat GetFormat() const { return Format; }
	VkFormat GetVkFormat() const { return ConvertFormat_Format_To_VkFormat(Format); }

	virtual bool IsSRV_Allowed() const override final { return bIsSRVSupported; }
	virtual bool IsUAV_Allowed() const override final { return bIsUAVSupported; }

	VkImageLayout CurrentLayout;

	VkImage	Image;
	VkDescriptorImageInfo DescriptorImageInfo;

private:
	std::string Name;
	EFormat Format;
	sFBODesc Desc;

	VkDeviceMemory Memory;
	VkImageView    View;
	VkImageView    UAV_View;

	bool bIsSRVSupported;
	bool bIsUAVSupported;
};

class VulkanDepthTarget final : public IDepthTarget
{
	sClassBody(sClassConstructor, VulkanDepthTarget, IDepthTarget)
public:
	VulkanDepthTarget(VulkanDevice* InOwner, const std::string InName, const EFormat Format, const sFBODesc& Desc);
	virtual ~VulkanDepthTarget();

	virtual void SetDefaultRootParameterIndex(std::uint32_t RootParameterIndex) override final {}
	virtual std::uint32_t GetDefaultRootParameterIndex() const override final { return 0; }

	const VkImage& GetImage() const { return Image; }
	const VkImageView& GetView() const { return View; }
	virtual void* GetNativeTexture() const override final { return Image; }

	bool IsSRVSupported() const { return bIsSRVSupported; }
	virtual bool IsSRV_Allowed() const override final { return bIsSRVSupported; }
	virtual bool IsUAV_Allowed() const override final { return false; }

	EFormat GetFormat() const { return Format; }
	VkFormat GetVkFormat() const { return ConvertFormat_Format_To_VkFormat(Format); }

	VkImageLayout CurrentLayout;

	VkImage	Image;

private:
	std::string Name;
	EFormat Format;
	sFBODesc Desc;

	VkDeviceMemory Memory;
	VkImageView    View;

	bool bIsSRVSupported;
};

class VulkanUnorderedAccessTarget final : public IUnorderedAccessTarget
{
	sClassBody(sClassConstructor, VulkanUnorderedAccessTarget, IUnorderedAccessTarget)
public:
	VulkanUnorderedAccessTarget(VulkanDevice* InOwner, const std::string InName, const EFormat Format, const sFBODesc& Desc, bool InEnableSRV = true);
	virtual ~VulkanUnorderedAccessTarget();

	virtual void SetDefaultRootParameterIndex(std::uint32_t RootParameterIndex) override final {}
	virtual std::uint32_t GetDefaultRootParameterIndex() const override final { return 0; }

	const VkImage& GetImage() const { return Image; }
	const VkImageView& GetView() const { return View; }
	virtual void* GetNativeTexture() const override final { return Image; }

	bool IsSRVSupported() const { return bIsSRVSupported; }
	virtual bool IsSRV_Allowed() const override final { return bIsSRVSupported; }

	EFormat GetFormat() const { return Format; }
	VkFormat GetVkFormat() const { return ConvertFormat_Format_To_VkFormat(Format); }

	VkImageLayout CurrentLayout;

	VkImage	Image;

private:
	std::string Name;
	EFormat Format;
	sFBODesc Desc;

	VkDeviceMemory Memory;
	VkImageView    View;

	bool bIsSRVSupported;
};

class VulkanFrameBuffer final : public IFrameBuffer
{
	sClassBody(sClassConstructor, VulkanFrameBuffer, IFrameBuffer)
private:
	VulkanDevice* Owner;
	std::string Name;

	sFrameBufferAttachmentInfo AttachmentInfo;

public:
	std::vector<VulkanRenderTarget::SharedPtr> RenderTargets;
	std::vector<VulkanUnorderedAccessTarget::SharedPtr> UnorderedAccessTargets;
	VulkanDepthTarget::SharedPtr DepthTarget;

	//VkFramebuffer FrameBuffer;
	//VulkanRenderpass::SharedPtr RenderPass;

public:
	VulkanFrameBuffer(VulkanDevice* Device, const std::string InName, const sFrameBufferAttachmentInfo& InAttachments);

	virtual ~VulkanFrameBuffer()
	{
		//vkDestroyFramebuffer(Owner->Get(), FrameBuffer, nullptr);

		Owner = nullptr;

		for (auto& RT : RenderTargets)
			RT = nullptr;
		RenderTargets.clear();

		for (auto& UAV : UnorderedAccessTargets)
			UAV = nullptr;
		UnorderedAccessTargets.clear();

		DepthTarget = nullptr;
		//RenderPass = nullptr;
	}


	virtual std::string GetName() const override final
	{
		return Name;
	}

	virtual sFrameBufferAttachmentInfo GetAttachmentInfo() const override final
	{
		return AttachmentInfo;
	}

	virtual std::size_t GetAttachmentCount() const override final
	{
		return AttachmentInfo.GetAttachmentCount();
	}
	virtual std::size_t GetRenderTargetAttachmentCount() const override final
	{
		return AttachmentInfo.GetRenderTargetAttachmentCount();
	}
	VulkanRenderTarget* PrimaryFrameBuffer() const
	{
		return RenderTargets.at(AttachmentInfo.PrimaryFB).get();
	}

	virtual std::vector<IRenderTarget*> GetRenderTargets() const override final
	{
		std::vector<IRenderTarget*> Result;
		Result.reserve(RenderTargets.size());
		std::transform(RenderTargets.cbegin(), RenderTargets.cend(), std::back_inserter(Result), [](auto& ptr) { return ptr.get(); });
		return Result;
	}
	virtual IRenderTarget* GetRenderTarget(std::size_t Index) const override final { return RenderTargets.at(Index).get(); }
	virtual void AttachRenderTarget(const IRenderTarget::SharedPtr& RenderTarget, std::optional<std::size_t> Index = std::nullopt) override final;

	virtual std::vector<IUnorderedAccessTarget*> GetUnorderedAccessTargets() const override final
	{
		std::vector<IUnorderedAccessTarget*> Result;
		Result.reserve(UnorderedAccessTargets.size());
		std::transform(UnorderedAccessTargets.cbegin(), UnorderedAccessTargets.cend(), std::back_inserter(Result), [](auto& ptr) { return ptr.get(); });
		return Result;
	}
	virtual IUnorderedAccessTarget* GetUnorderedAccessTarget(std::size_t Index) const override final { return UnorderedAccessTargets.at(Index).get(); }
	virtual void AttachUnorderedAccessTarget(const IUnorderedAccessTarget::SharedPtr& UnorderedAccessTarget, std::optional<std::size_t> Index = std::nullopt) override final;

	virtual IDepthTarget* GetDepthTarget() const override final { return DepthTarget.get(); }
	virtual void SetDepthTarget(const IDepthTarget::SharedPtr& DepthTarget) override final;
};
