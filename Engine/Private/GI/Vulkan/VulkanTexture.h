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

#include <string>
#include "Engine/ClassBody.h"
#include "Engine/AbstractEngine.h"
#include "VulkanShaderStates.h"
#include "VulkanDevice.h"

class VulkanTexture final : public ITexture2D
{
	sClassBody(sClassConstructor, VulkanTexture, ITexture2D)
public:
	VulkanTexture(VulkanDevice* Device, const std::wstring FilePath, const std::string InName = std::string(), std::uint32_t RootParameterIndex = 0);
	VulkanTexture(VulkanDevice* Device, const std::string InName, void* InBuffer, const std::size_t InSize, const sTextureDesc& InDesc, std::uint32_t RootParameterIndex = 0);
	VulkanTexture(VulkanDevice* Device, const std::string InName, const sTextureDesc& InDesc, std::uint32_t RootParameterIndex = 0);

	virtual ~VulkanTexture()
	{
	}

	virtual std::string GetName() const override final { return Name; }
	virtual std::wstring GetPath() const override final { return Path; }

	virtual sTextureDesc GetDesc() const override final { return Desc; }

	virtual void SetDefaultRootParameterIndex(std::uint32_t inRootParameterIndex) override final { }
	virtual std::uint32_t GetDefaultRootParameterIndex() const override final { return 0; }

	virtual void UpdateTexture(ITexture2D* SourceTexture, std::size_t SourceArrayIndex, std::size_t ArrayIndex, const std::optional<IntVector2> Dest = std::nullopt, const std::optional<FBounds2D> TargetBounds = std::nullopt) override final;
	virtual void UpdateTexture(const std::wstring FilePath, std::size_t ArrayIndex, const std::optional<IntVector2> Dest = std::nullopt, const std::optional<FBounds2D> TargetBounds = std::nullopt) override final;
	virtual void UpdateTexture(const void* pSrcData, const std::size_t InSize, const FDimension2D& Dimension, std::size_t ArrayIndex, const std::optional<IntVector2> Dest = std::nullopt, const std::optional<FBounds2D> TargetBounds = std::nullopt) override final;

	virtual void UpdateTexture(const void* pSrcData, std::size_t RowPitch, std::size_t MinX, std::size_t MinY, std::size_t MaxX, std::size_t MaxY, IGraphicsCommandContext* InCommandBuffer = nullptr) override final;

	virtual void SaveToFile(std::wstring InPath) const override final;

	VkImageLayout CurrentLayout;
	std::uint32_t RootParameterIndex;
	VkDescriptorImageInfo DescriptorImageInfo;

private:
	void GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
	void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, std::int32_t OffsetX = 0, std::int32_t OffsetY = 0);

	void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

private:
	VulkanDevice* Owner;
	std::string Name;
	std::wstring Path;
	sTextureDesc Desc;

	VkImage Image;
	VkImageView View;
	VkDeviceMemory Memory;
};
