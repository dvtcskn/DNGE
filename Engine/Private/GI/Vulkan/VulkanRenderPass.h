#pragma once

#include <memory>
#include "VulkanDevice.h"
#include "VulkanFrameBuffer.h"

class VulkanRenderpass
{
	sBaseClassBody(sClassConstructor, VulkanRenderpass)
public:
	VulkanRenderpass(VulkanDevice* InDevice, std::string InName, const sFrameBufferAttachmentInfo& InInfos);

public:
	virtual ~VulkanRenderpass();

	FORCEINLINE const VkRenderPass& Get() { return RenderPass; };
	FORCEINLINE std::uint32_t SubpassIndex() { return 0; };

	sFrameBufferAttachmentInfo Info;

private:
	VulkanDevice* Device;
	VkRenderPass RenderPass;
	std::string Name;
};
