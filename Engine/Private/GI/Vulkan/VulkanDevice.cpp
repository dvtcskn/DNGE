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
#include <iostream>
#include "VulkanDevice.h"
#include "Engine/AbstractEngine.h"

#define DEBUG_D3DDEVICE 1

VulkanDevice::VulkanDevice(std::optional<short> GPUIndex)
	: Super()
	, VendorId(0)
{
//	instance = vk::su::createInstance("Vulkan", "VulkanDeferredRenderer", vk::su::getInstanceExtensions());
//#if !defined(NDEBUG)
//	vk::UniqueDebugReportCallbackEXT debugReportCallback = vk::su::createDebugReportCallback(instance);
//#endif
//
//	std::vector<vk::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();
//	PhysicalDevice = physicalDevices[0];
//	PhysicalDeviceProperties = PhysicalDevice.getProperties();
//	PhysicalDeviceMemoryProperties = PhysicalDevice.getMemoryProperties();
//	deviceLimits = PhysicalDeviceProperties.limits;
//	assert(!physicalDevices.empty());
//
//	SetupQueueGraphicsFamily();
//
//	// create a UniqueDevice
//	float queuePriority = 0.0f;
//	vk::DeviceQueueCreateInfo deviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), static_cast<uint32_t>(FamilyProperties.graphicsQueueFamilyIndex), 1, &queuePriority);
//
//	device = vk::su::createDevice(physicalDevices[0], static_cast<uint32_t>(FamilyProperties.graphicsQueueFamilyIndex), vk::su::getDeviceExtensions());
//	GraphicsQueue = device.getQueue(static_cast<uint32_t>(FamilyProperties.graphicsQueueFamilyIndex), 0);
//
//	vk::CommandPoolCreateInfo CMDPoolInfo;
//	CMDPoolInfo.queueFamilyIndex = static_cast<uint32_t>(FamilyProperties.graphicsQueueFamilyIndex);
//	//CMDPoolInfo.flags = ;
//	CMDBufferPool = device.createCommandPoolUnique(CMDPoolInfo);
//
//	SetupProperties();
}

VulkanDevice::~VulkanDevice()
{
}

void VulkanDevice::InitWindow(void* InHWND, std::uint32_t InWidth, std::uint32_t InHeight, bool bFullscreen)
{

}

void VulkanDevice::SetupProperties()
{
}

void VulkanDevice::SetupQueueGraphicsFamily()
{
}

void VulkanDevice::Present(IRenderTarget* pRT)
{

}

bool VulkanDevice::IsFullScreen() const
{
	return false;
}

bool VulkanDevice::IsVsyncEnabled() const
{
	return false;
}

std::uint32_t VulkanDevice::GetVsyncInterval() const
{
	return 0;
}

void VulkanDevice::ResizeWindow(std::size_t Width, std::size_t Height)
{
}

void VulkanDevice::FullScreen(const bool value)
{
}

void VulkanDevice::Vsync(const bool value)
{
}

void VulkanDevice::VsyncInterval(const std::uint32_t value)
{
}

std::vector<sDisplayMode> VulkanDevice::GetAllSupportedResolutions() const
{
	return std::vector<sDisplayMode>();
}

sScreenDimension VulkanDevice::GetBackBufferDimension() const
{
	return sScreenDimension();
}

EFormat VulkanDevice::GetBackBufferFormat() const
{
	return EFormat::UNKNOWN;
}

sViewport VulkanDevice::GetViewport() const
{
	return sViewport();
}

bool VulkanDevice::GetDeviceIdentification(std::wstring& InVendorID, std::wstring& InDeviceID)
{
	return true;
}

IGraphicsCommandContext::SharedPtr VulkanDevice::CreateGraphicsCommandContext()
{
	return nullptr;
}

IGraphicsCommandContext::UniquePtr VulkanDevice::CreateUniqueGraphicsCommandContext()
{
	return nullptr;
}

IComputeCommandContext::SharedPtr VulkanDevice::CreateComputeCommandContext()
{
	return IComputeCommandContext::SharedPtr();
}

IComputeCommandContext::UniquePtr VulkanDevice::CreateUniqueComputeCommandContext()
{
	return IComputeCommandContext::UniquePtr();
}

ICopyCommandContext::SharedPtr VulkanDevice::CreateCopyCommandContext()
{
	return ICopyCommandContext::SharedPtr();
}

ICopyCommandContext::UniquePtr VulkanDevice::CreateUniqueCopyCommandContext()
{
	return ICopyCommandContext::UniquePtr();
}

IConstantBuffer::SharedPtr VulkanDevice::CreateConstantBuffer(std::string InName, const sBufferDesc& InDesc, std::uint32_t InRootParameterIndex)
{
	return nullptr;
}

IConstantBuffer::UniquePtr VulkanDevice::CreateUniqueConstantBuffer(std::string InName, const sBufferDesc& InDesc, std::uint32_t InRootParameterIndex)
{
	return nullptr;
}

IVertexBuffer::SharedPtr VulkanDevice::CreateVertexBuffer(std::string InName, const sBufferDesc& InDesc, sBufferSubresource* InSubresource)
{
	return nullptr;
}

IVertexBuffer::UniquePtr VulkanDevice::CreateUniqueVertexBuffer(std::string InName, const sBufferDesc& InDesc, sBufferSubresource* InSubresource)
{
	return nullptr;
}

IIndexBuffer::SharedPtr VulkanDevice::CreateIndexBuffer(std::string InName, const sBufferDesc& InDesc, sBufferSubresource* InSubresource)
{
	return nullptr;
}

IIndexBuffer::UniquePtr VulkanDevice::CreateUniqueIndexBuffer(std::string InName, const sBufferDesc& InDesc, sBufferSubresource* InSubresource)
{
	return nullptr;
}

IFrameBuffer::SharedPtr VulkanDevice::CreateFrameBuffer(const std::string InName, const sFrameBufferAttachmentInfo& InAttachments)
{
	return nullptr;
}

IFrameBuffer::UniquePtr VulkanDevice::CreateUniqueFrameBuffer(const std::string InName, const sFrameBufferAttachmentInfo& InAttachments)
{
	return nullptr;
}

IRenderTarget::SharedPtr VulkanDevice::CreateRenderTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc)
{
	return IRenderTarget::SharedPtr();
}

IRenderTarget::UniquePtr VulkanDevice::CreateUniqueRenderTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc)
{
	return IRenderTarget::UniquePtr();
}

IDepthTarget::SharedPtr VulkanDevice::CreateDepthTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc)
{
	return IDepthTarget::SharedPtr();
}

IDepthTarget::UniquePtr VulkanDevice::CreateUniqueDepthTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc)
{
	return IDepthTarget::UniquePtr();
}

IUnorderedAccessTarget::SharedPtr VulkanDevice::CreateUnorderedAccessTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc, bool InEnableSRV)
{
	return IUnorderedAccessTarget::SharedPtr();
}

IUnorderedAccessTarget::UniquePtr VulkanDevice::CreateUniqueUnorderedAccessTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc, bool InEnableSRV)
{
	return IUnorderedAccessTarget::UniquePtr();
}

IPipeline::SharedPtr VulkanDevice::CreatePipeline(const std::string& InName, const sPipelineDesc& InDesc)
{
	return nullptr;
}

IPipeline::UniquePtr VulkanDevice::CreateUniquePipeline(const std::string& InName, const sPipelineDesc& InDesc)
{
	return nullptr;
}

IComputePipeline::SharedPtr VulkanDevice::CreateComputePipeline(const std::string& InName, const sComputePipelineDesc& InDesc)
{
	return nullptr;
}

IComputePipeline::UniquePtr VulkanDevice::CreateUniqueComputePipeline(const std::string& InName, const sComputePipelineDesc& InDesc)
{
	return nullptr;
}

ITexture2D::SharedPtr VulkanDevice::CreateTexture2D(const std::wstring FilePath, const std::string InName, std::uint32_t DefaultRootParameterIndex)
{
	return nullptr;
}

ITexture2D::UniquePtr VulkanDevice::CreateUniqueTexture2D(const std::wstring FilePath, const std::string InName, std::uint32_t DefaultRootParameterIndex)
{
	return nullptr;
}

ITexture2D::SharedPtr VulkanDevice::CreateTexture2D(const std::string InName, void* InBuffer, const std::size_t InSize, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
{
	return nullptr;
}

ITexture2D::UniquePtr VulkanDevice::CreateUniqueTexture2D(const std::string InName, void* InBuffer, const std::size_t InSize, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
{
	return nullptr;
}

ITexture2D::SharedPtr VulkanDevice::CreateEmptyTexture2D(const std::string InName, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
{
	return nullptr;
}

ITexture2D::UniquePtr VulkanDevice::CreateUniqueEmptyTexture2D(const std::string InName, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
{
	return nullptr;
}

//ITiledTexture::SharedPtr VulkanDevice::CreateTiledTexture(const std::string InName, const std::uint32_t InTileX, const std::uint32_t InTileY, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
//{
//	return nullptr;
//}
//
//ITiledTexture::UniquePtr VulkanDevice::CreateUniqueTiledTexture(const std::string InName, const std::uint32_t InTileX, const std::uint32_t InTileY, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
//{
//	return nullptr;
//}
