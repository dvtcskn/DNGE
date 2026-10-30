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
#include <algorithm>
#include "VulkanViewport.h"
#include "VulkanException.h"
#include "VulkanFormat.h"
#include "VulkanFrameBuffer.h"
#include "VulkanCommandBuffer.h"

#define GET_INSTANCE_PROC_ADDR(name) name = (PFN_##name)vkGetInstanceProcAddr(Device->GetInstance(), #name);
#define GET_DEVICE_PROC_ADDR(name) name = (PFN_##name)vkGetDeviceProcAddr(Device->Get(), #name);

VulkanViewport::VulkanViewport(VulkanDevice* InDevice, HWND* InHandle)
	: Device(InDevice)
	, bRecreateSurface(false)
	, bIsFullScreen(false)
	, bVSYNC(false)
	, Vsync_Interval(0)
	, Format(EFormat::UNKNOWN)
	, SizeX(0)
	, SizeY(0)
	, FrameIndex(0)
	, Viewport(sViewport())
{
	Surface = VK_NULL_HANDLE;

	if (!InHandle)
		bRecreateSurface = true;

	VkWin32SurfaceCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hwnd = (HWND)InHandle;
	createInfo.hinstance = GetModuleHandle(nullptr);

	if (vkCreateWin32SurfaceKHR(Device->GetInstance(), &createInfo, nullptr, &Surface) != VK_SUCCESS) {
		Engine::WriteToConsole("failed to create window surface!");
	}
}

VulkanViewport::~VulkanViewport()
{
	vkDestroySwapchainKHR(Device->Get(), Swapchain, nullptr);
	for (auto& Image : SwapchainImages)
		Image.Destroy(Device->Get());
	SwapchainImages.clear();
	vkDestroySurfaceKHR(Device->GetInstance(), Surface, nullptr);

	vkSetHdrMetadataEXT = nullptr;
	vkGetPhysicalDeviceSurfaceCapabilities2KHR = nullptr;
	vkGetPhysicalDeviceSurfaceFormats2KHR = nullptr;

	Handle = nullptr;
	Device = nullptr;
}

void VulkanViewport::InitWindow(ViewportCreateInfo CreateInfo)
{
	SizeX = CreateInfo.Width;
	SizeY = CreateInfo.Height;

	Viewport = { SizeX, SizeY, 0, 0, 0.0f, 1.0f };

	Handle = (HWND*)CreateInfo.pHWND;

	if (bRecreateSurface)
	{
		vkDestroySurfaceKHR(Device->GetInstance(), Surface, nullptr);

		Surface = VK_NULL_HANDLE;

		bRecreateSurface = false;

		VkWin32SurfaceCreateInfoKHR SurfaceCreateInfo{};
		SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		SurfaceCreateInfo.hwnd = (HWND)Handle;
		SurfaceCreateInfo.hinstance = GetModuleHandle(nullptr);

		if (vkCreateWin32SurfaceKHR(Device->GetInstance(), &SurfaceCreateInfo, nullptr, &Surface) != VK_SUCCESS) {
			Engine::WriteToConsole("failed to create window surface!");
		}
	}

	VkSurfaceCapabilitiesKHR surface_properties;
	VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device->GetPhysicalDevice(), Surface, &surface_properties));

	std::vector<VkFormat> const& preferred_formats = { VK_FORMAT_B8G8R8A8_UNORM/*2D Render only*/, VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_A8B8G8R8_SRGB_PACK32};

	{
		uint32_t surface_format_count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(Device->GetPhysicalDevice(), Surface, &surface_format_count, nullptr);
		assert(0 < surface_format_count);
		std::vector<VkSurfaceFormatKHR> supported_surface_formats(surface_format_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(Device->GetPhysicalDevice(), Surface, &surface_format_count, supported_surface_formats.data());

		auto it = std::find_if(supported_surface_formats.begin(), supported_surface_formats.end(), [&](VkSurfaceFormatKHR surface_format)
			{
				if (CreateInfo.SwapChainFormat.has_value())
				{
					return surface_format.format == ConvertFormat_Format_To_VkFormat(CreateInfo.SwapChainFormat.value());
				}
				else
				{
					return std::any_of(preferred_formats.begin(), preferred_formats.end(), [&surface_format](VkFormat format) { return format == surface_format.format; });
				}
			}
		);

		SurfaceFormat = it != supported_surface_formats.end() ? *it : supported_surface_formats[0];
	}

	GET_INSTANCE_PROC_ADDR(vkGetPhysicalDeviceSurfaceFormats2KHR);
	GET_INSTANCE_PROC_ADDR(vkGetPhysicalDeviceSurfaceCapabilities2KHR);
	GET_DEVICE_PROC_ADDR(vkSetHdrMetadataEXT);

	QuerySwapChainSupport();
	// Find all HDR modes supported by current display and pick surface format
	EnumerateDisplayModesAndFormats(formats2);

	VkExtent2D swapchain_size;
	if (surface_properties.currentExtent.width == 0xFFFFFFFF)
	{
		swapchain_size.width = SizeX;
		swapchain_size.height = SizeY;
	}
	else
	{
		swapchain_size = surface_properties.currentExtent;
	}

	// VSYNC = VK_PRESENT_MODE_FIFO_KHR  
	// NO VSYNC = VSYNC VK_PRESENT_MODE_IMMEDIATE_KHR 
	// Half VSYNC ? = VK_PRESENT_MODE_FIFO_RELAXED_KHR
	// FIFO must be supported by all implementations.
	VkPresentModeKHR swapchain_present_mode = VK_PRESENT_MODE_FIFO_KHR;

	// Determine the number of VkImage's to use in the swapchain.
	// Ideally, we desire to own 1 image at a time, the rest of the images can
	// either be rendered to and/or being queued up for display.
	uint32_t desired_swapchain_images = surface_properties.minImageCount + 1;
	if ((surface_properties.maxImageCount > 0) && (desired_swapchain_images > surface_properties.maxImageCount))
	{
		// Application must settle for fewer images than desired.
		desired_swapchain_images = surface_properties.maxImageCount;
	}

	// Figure out a suitable surface transform.
	VkSurfaceTransformFlagBitsKHR pre_transform;
	if (surface_properties.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		pre_transform = surface_properties.currentTransform;
	}

	VkSwapchainKHR old_swapchain = Swapchain;

	// one bitmask needs to be set according to the priority of presentation engine
	VkCompositeAlphaFlagBitsKHR composite = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	if (surface_properties.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
	{
		composite = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	}
	else if (surface_properties.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR)
	{
		composite = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
	}
	else if (surface_properties.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR)
	{
		composite = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
	}
	else if (surface_properties.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR)
	{
		composite = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
	}

	HWND HWND_application_window = GetActiveWindow();

	surface_full_screen_exclusive_Win32_info_EXT.sType = VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT;
	surface_full_screen_exclusive_Win32_info_EXT.pNext = nullptr;
	surface_full_screen_exclusive_Win32_info_EXT.hmonitor = MonitorFromWindow(HWND_application_window, MONITOR_DEFAULTTONEAREST);

	surface_full_screen_exclusive_info_EXT.sType = VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT;
	surface_full_screen_exclusive_info_EXT.pNext = &surface_full_screen_exclusive_Win32_info_EXT;
	/*
	*  Borderless = VK_FULL_SCREEN_EXCLUSIVE_ALLOWED_EXT
	*  Windowed = VK_FULL_SCREEN_EXCLUSIVE_DISALLOWED_EXT
	*  Application Controlled = VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT
	* 
	*  VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT 
	*  specifies that the application will manage full-screen exclusive mode by using the vkAcquireFullScreenExclusiveModeEXT and vkReleaseFullScreenExclusiveModeEXT commands.
	*/
	surface_full_screen_exclusive_info_EXT.fullScreenExclusive = VK_FULL_SCREEN_EXCLUSIVE_DEFAULT_EXT;

	VkSwapchainCreateInfoKHR info{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext = &surface_full_screen_exclusive_info_EXT,
		.surface = Surface,                            // The surface onto which images will be presented
		.minImageCount = desired_swapchain_images,                   // Minimum number of images in the swapchain (number of buffers)
		.imageFormat = SurfaceFormat.format,                              // Format of the swapchain images (e.g., VK_FORMAT_B8G8R8A8_SRGB)
		.imageColorSpace = SurfaceFormat.colorSpace,                          // Color space of the images (e.g., VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		.imageExtent = swapchain_size,                             // Resolution of the swapchain images (width and height)
		.imageArrayLayers = 1,                                          // Number of layers in each image (usually 1 unless stereoscopic)
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,        // How the images will be used (as color attachments)
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,                  // Access mode of the images (exclusive to one queue family)
		.preTransform = pre_transform,                              // Transform to apply to images (e.g., rotation)
		.compositeAlpha = composite,                                  // Alpha blending to apply (e.g., opaque, pre-multiplied)
		.presentMode = swapchain_present_mode,                     // Presentation mode (e.g., vsync settings)
		.clipped = true,                                       // Whether to clip obscured pixels (improves performance)
		.oldSwapchain = old_swapchain                               // Handle to the old swapchain, if replacing an existing one
	};

	SwapchainCreateInfo = info;
	Format = ConvertFormat_VkFormat_To_Format(SurfaceFormat.format);

	VK_CHECK(vkCreateSwapchainKHR(Device->Get(), &SwapchainCreateInfo, nullptr, &Swapchain));

	if (old_swapchain != VK_NULL_HANDLE)
	{
		for (auto& images : SwapchainImages)
		{
			images.Destroy(Device->Get());
		}
		SwapchainImages.clear();

		vkDestroySwapchainKHR(Device->Get(), old_swapchain, nullptr);
	}

	CreateBackBufferTexture();

	// Can only do this for Freesync Premium Pro HDR display on AMD hardware
	if (strstr(Device->GetDeviceName().c_str(), "AMD") != nullptr)
	{
		EnumerateHDRMetadata(Device->GetPhysicalDevice(), physicalDeviceSurfaceInfo2, capabilities2);
	}

	// Set primaies based on displaymode
	PopulateHDRMetadataBasedOnDisplayMode();

	SetHDRMetadataAndColorspace();

	if (CreateInfo.bFullscreen)
		FullScreen(true);
}

VkImage VulkanViewport::GetCurrentBackBufferImage() const
{
	return SwapchainImages[FrameIndex].Image;
}

VkImageCreateInfo VulkanViewport::GetCurrentBackBufferImageCreateInfo() const
{
	return SwapchainImages[FrameIndex].ImageCreateInfo;
}

void VulkanViewport::RecreateSwapChain()
{
}

void VulkanViewport::CreateBackBufferTexture()
{
	uint32_t image_count;
	VK_CHECK(vkGetSwapchainImagesKHR(Device->Get(), Swapchain, &image_count, nullptr));

	std::vector<VkImage> Images(image_count);
	VK_CHECK(vkGetSwapchainImagesKHR(Device->Get(), Swapchain, &image_count, Images.data()));

	for (size_t i = 0; i < image_count; i++)
	{
		VkImageCreateInfo ImageInfo = {};
		ImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		ImageInfo.pNext = nullptr;
		ImageInfo.flags = 0;
		ImageInfo.imageType = VK_IMAGE_TYPE_2D;
		ImageInfo.format = SurfaceFormat.format;
		ImageInfo.extent.width = SizeX;
		ImageInfo.extent.height = SizeY;
		ImageInfo.extent.depth = 1;
		ImageInfo.mipLevels = 1;
		ImageInfo.arrayLayers = 1;
		ImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		ImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		ImageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		VkImageViewCreateInfo view_info{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.image = Images[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = SurfaceFormat.format,
			.subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1} };

		VkImageView image_view;
		VK_CHECK(vkCreateImageView(Device->Get(), &view_info, nullptr, &image_view));

		SwapchainImages.push_back(SwapchainImage(Images[i], ImageInfo, image_view, VK_IMAGE_LAYOUT_UNDEFINED));
	}
}

void VulkanViewport::SetSwapChain(VkSwapchainKHR NewSwapchain)
{
	if (NewSwapchain != VK_NULL_HANDLE)
	{
		Swapchain = NewSwapchain;
		CreateBackBufferTexture();
	}
	else
	{
		for (auto& Image : SwapchainImages)
			Image.Destroy(Device->Get());
		SwapchainImages.clear();
		//vkDestroySwapchainKHR(Device->Get(), Swapchain, nullptr);
		Swapchain = NewSwapchain;
	}
}

VkResult VulkanViewport::AcquireNextImageIndex(uint32_t* image)
{
	VkSemaphore acquire_semaphore = Device->AcquireSemaphore();
	VkResult result = vkAcquireNextImageKHR(Device->Get(), Swapchain, UINT64_MAX, acquire_semaphore, VK_NULL_HANDLE, image);
	if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR)
	{
		AcquiredSemaphore = acquire_semaphore;
		bAcquireSemaphoreWaited = false;
	}
	return result;
}

void VulkanViewport::BeginFrame()
{
	VkResult Result = AcquireNextImageIndex(&FrameIndex);

	if (Result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		if (!ResizeSwapChain(SizeX, SizeY))
		{
			Engine::WriteToConsole("Resize failed");
		}
		Result = AcquireNextImageIndex(&FrameIndex);
	}

	if (Result != VK_SUCCESS && Result != VK_SUBOPTIMAL_KHR)
	{
		throw ("Failed to get swapchain image.");
	}
}

void VulkanViewport::Present(IRenderTarget* pRT)
{
	if (SwapchainImages[FrameIndex].CurrentImageLayout != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
	{
		auto CMD = Device->GetIMCommandBuffer();
		CMD->BeginRecordCommandList();

		CMD->TransitionTo(
			SwapchainImages[FrameIndex].Image,
			SwapchainImages[FrameIndex].CurrentImageLayout,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			0,														// srcAccessMask
			0,                                                      // dstAccessMask
			VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,					// srcStage
			VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT                  // dstStage
		);

		CMD->FinishRecordCommandList();
		VkSemaphore acquire_semaphore = GetPendingAcquireSemaphore();
		if (acquire_semaphore != VK_NULL_HANDLE)
		{
			if (!CMD->ExecuteCommandListWithWaitSemaphore(acquire_semaphore))
				throw std::runtime_error("Failed to submit swapchain image transition.");
			MarkAcquireSemaphoreWaited();
		}
		else
		{
			CMD->ExecuteCommandList();
		}

		SwapchainImages[FrameIndex].CurrentImageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	}

	//Device->WaitForGPU();

	VkSemaphore acquire_semaphore = GetPendingAcquireSemaphore();
	if (acquire_semaphore != VK_NULL_HANDLE)
	{
		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		VkSubmitInfo waitSubmit{};
		waitSubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		waitSubmit.waitSemaphoreCount = 1;
		waitSubmit.pWaitSemaphores = &acquire_semaphore;
		waitSubmit.pWaitDstStageMask = &waitStage;
		VK_CHECK(vkQueueSubmit(Device->GetQueue(), 1, &waitSubmit, VK_NULL_HANDLE));
		MarkAcquireSemaphoreWaited();
		VK_CHECK(vkQueueWaitIdle(Device->GetQueue()));
	}

	VkPresentInfoKHR present{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.swapchainCount = 1,
		.pSwapchains = &Swapchain,
		.pImageIndices = &FrameIndex,
	};

	VkResult Result = vkQueuePresentKHR(Device->GetQueue(), &present);

	if (Result == VK_SUBOPTIMAL_KHR || Result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		if (!ResizeSwapChain(SizeX, SizeY))
		{
			Engine::WriteToConsole("Resize failed");
			vkQueueWaitIdle(Device->GetQueue());
		}
	}
	else if (Result != VK_SUCCESS)
	{
		throw ("Failed to present swapchain image.");
	}
}

void VulkanViewport::CopyToBackBuffer(IRenderTarget* pRT)
{
	if (SwapchainImages[FrameIndex].CurrentImageLayout != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
	{
		auto CMD = Device->GetIMCommandBuffer();
		CMD->BeginRecordCommandList();

		CMD->TransitionTo(
			SwapchainImages[FrameIndex].Image,
			SwapchainImages[FrameIndex].CurrentImageLayout,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			0,														// srcAccessMask
			0,                                                      // dstAccessMask
			VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,					// srcStage
			VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT                  // dstStage
		);

		CMD->FinishRecordCommandList();
		VkSemaphore acquire_semaphore = GetPendingAcquireSemaphore();
		if (acquire_semaphore != VK_NULL_HANDLE)
		{
			if (!CMD->ExecuteCommandListWithWaitSemaphore(acquire_semaphore))
				throw std::runtime_error("Failed to submit swapchain image transition.");
			MarkAcquireSemaphoreWaited();
		}
		else
		{
			CMD->ExecuteCommandList();
		}

		SwapchainImages[FrameIndex].CurrentImageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	}

	auto CMDList = Device->GetIMCommandBuffer();
	CMDList->BeginRecordCommandList();

	CMDList->CopyResource(SwapchainImages[FrameIndex].Image, VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, static_cast<VulkanRenderTarget*>(pRT)->Image, static_cast<VulkanRenderTarget*>(pRT)->CurrentLayout, {SizeX, SizeY});

	CMDList->FinishRecordCommandList();
	CMDList->ExecuteCommandList(ECommandContextExecuteType::Deferred, std::uint32_t(-1));
}

void VulkanViewport::FullScreen(const bool value)
{
	if (bIsFullScreen == value)
		return;

	bIsFullScreen = value;

	/*
	*  Borderless = VK_FULL_SCREEN_EXCLUSIVE_ALLOWED_EXT
	*  Windowed = VK_FULL_SCREEN_EXCLUSIVE_DISALLOWED_EXT
	*  Application Controlled = VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT
	*
	*  VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT
	*  specifies that the application will manage full-screen exclusive mode by using the vkAcquireFullScreenExclusiveModeEXT and vkReleaseFullScreenExclusiveModeEXT commands.
	*/
	surface_full_screen_exclusive_info_EXT.fullScreenExclusive = VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT;

	vkDeviceWaitIdle(Device->Get());        // pause the renderer

	// basically destroy everything swapchain related
	//teardown_framebuffers();                 
	ViewportCreateInfo Viewport;
	Viewport.SwapChainFormat = Format;
	Viewport.Width = SizeX;
	Viewport.Height = SizeY;
	Viewport.bFullscreen = value;
	Viewport.pHWND = Handle;
	InitWindow(Viewport);
	// Step: 2) recreate the frame buffers using the newly created swapchain
	//init_framebuffers();

	// Step: 3) remember: ALWAYS change the application window mode BEFORE acquire the full screen exclusive mode!
	// full screen exclusive
	if (value)
	{
		//VkResult result = vkReleaseFullScreenExclusiveModeEXT(Device->Get(), swapchain);
		//if (result != VK_SUCCESS)
		{
			//Engine::WriteToConsole("vkReleaseFullScreenExclusiveModeEXT: " + std::to_string(result));
		}
	}
}

void VulkanViewport::Vsync(const bool value)
{
	bVSYNC = value;
}

void VulkanViewport::VsyncInterval(const std::uint32_t value)
{
	Vsync_Interval = value;
}

bool VulkanViewport::ResizeSwapChain(std::size_t Width, std::size_t Height)
{
	if (Device->Get() == VK_NULL_HANDLE)
	{
		return false;
	}

	VkSurfaceCapabilitiesKHR surface_properties;
	VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device->GetPhysicalDevice(), Surface, &surface_properties));

	// Only rebuild the swapchain if the dimensions have changed
	if (surface_properties.currentExtent.width == SizeX &&
		surface_properties.currentExtent.height == SizeY)
	{
		return false;
	}

	vkDeviceWaitIdle(Device->Get());

	ViewportCreateInfo Viewport;
	Viewport.SwapChainFormat = Format;
	Viewport.Width = (std::uint32_t)Width;
	Viewport.Height = (std::uint32_t)Width;
	Viewport.bFullscreen = IsFullScreen();
	Viewport.pHWND = Handle;
	InitWindow(Viewport);

	return true;
}

void VulkanViewport::QuerySwapChainSupport()
{
	physicalDeviceSurfaceInfo2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
	physicalDeviceSurfaceInfo2.pNext = nullptr;
	physicalDeviceSurfaceInfo2.surface = Surface;

	if (strstr(Device->GetDeviceName().c_str(), "AMD") != nullptr)
	{
		capabilities2.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR;
		capabilities2.pNext = &displayNativeHdrSurfaceCapabilitiesAMD;

		displayNativeHdrSurfaceCapabilitiesAMD.sType = VK_STRUCTURE_TYPE_DISPLAY_NATIVE_HDR_SURFACE_CAPABILITIES_AMD;
		displayNativeHdrSurfaceCapabilitiesAMD.pNext = nullptr;

		// Calling this to check Local dimming support
		vkGetPhysicalDeviceSurfaceCapabilities2KHR(Device->GetPhysicalDevice(), &physicalDeviceSurfaceInfo2, &capabilities2);

		swapchainDisplayNativeHdrCreateInfoAMD.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_DISPLAY_NATIVE_HDR_CREATE_INFO_AMD;
		swapchainDisplayNativeHdrCreateInfoAMD.pNext = nullptr;
		swapchainDisplayNativeHdrCreateInfoAMD.localDimmingEnable = displayNativeHdrSurfaceCapabilitiesAMD.localDimmingSupport;
	}

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormats2KHR(Device->GetPhysicalDevice(), &physicalDeviceSurfaceInfo2, &formatCount, nullptr);
	if (formatCount != 0)
	{
		formats2.resize(formatCount);
		for (auto& surfaceFormat : formats2)
		{
			surfaceFormat.sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR;
			surfaceFormat.pNext = nullptr;
		}
		vkGetPhysicalDeviceSurfaceFormats2KHR(Device->GetPhysicalDevice(), &physicalDeviceSurfaceInfo2, &formatCount, formats2.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(Device->GetPhysicalDevice(), Surface, &presentModeCount, nullptr);
	if (presentModeCount != 0)
	{
		presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(Device->GetPhysicalDevice(), Surface, &presentModeCount, presentModes.data());
	}
}

std::unordered_map<DisplayMode, VkSurfaceFormatKHR> VulkanViewport::GetAvailableFormats(const std::vector<VkSurfaceFormat2KHR>& surfaceFormats2)
{
	std::unordered_map<DisplayMode, VkSurfaceFormatKHR> modes;
	for (const auto& surfaceFormat2 : surfaceFormats2)
	{
		VkSurfaceFormatKHR surfaceFormat = surfaceFormat2.surfaceFormat;
		if ((surfaceFormat.format == VK_FORMAT_R8G8B8A8_SRGB || surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM || surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB) && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			modes[DisplayMode::DISPLAYMODE_LDR] = surfaceFormat;
		}
		else if (surfaceFormat.format == VK_FORMAT_A2B10G10R10_UNORM_PACK32
			&& surfaceFormat.colorSpace == VK_COLOR_SPACE_DISPLAY_NATIVE_AMD)
		{
			modes[DisplayMode::DISPLAYMODE_FSHDR_2084] = surfaceFormat;
		}
		else if (surfaceFormat.format == VK_FORMAT_R16G16B16A16_SFLOAT && surfaceFormat.colorSpace == VK_COLOR_SPACE_DISPLAY_NATIVE_AMD)
		{
			modes[DisplayMode::DISPLAYMODE_FSHDR_SCRGB] = surfaceFormat;
		}
		else if (surfaceFormat.format == VK_FORMAT_A2B10G10R10_UNORM_PACK32
			&& surfaceFormat.colorSpace == VK_COLOR_SPACE_HDR10_ST2084_EXT)
		{
			// NOTE: we don't have any preference for the format between the two
			modes[DisplayMode::DISPLAYMODE_HDR10_2084] = surfaceFormat;
		}
		else if (surfaceFormat.format == VK_FORMAT_R16G16B16A16_SFLOAT && surfaceFormat.colorSpace == VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT)
		{
			modes[DisplayMode::DISPLAYMODE_HDR10_SCRGB] = surfaceFormat;
		}
	}

	return modes;
}

DisplayMode VulkanViewport::CheckAndGetDisplayModeRequested(DisplayMode DispMode)
{
	for (const auto& it : SupportedDisplayModes)
	{
		if (it == DispMode)
			return it;
	}

	if (DispMode == DisplayMode::DISPLAYMODE_FSHDR_2084) {

		Engine::WriteToConsole(L"FSHDR PQ not supported, trying HDR10 PQ");
		return CheckAndGetDisplayModeRequested(DisplayMode::DISPLAYMODE_HDR10_2084);
	}
	else if (DispMode == DisplayMode::DISPLAYMODE_FSHDR_SCRGB) {

		Engine::WriteToConsole(L"FSHDR SCRGB not supported, trying HDR10 SCRGB");
		return CheckAndGetDisplayModeRequested(DisplayMode::DISPLAYMODE_HDR10_SCRGB);
	}

	if (DispMode != DisplayMode::DISPLAYMODE_LDR)
	{
		Engine::WriteToConsole(L"HDR modes not supported, defaulting to LDR");
	}

	return DisplayMode::DISPLAYMODE_LDR;
}

void VulkanViewport::EnumerateDisplayModesAndFormats(const std::vector<VkSurfaceFormat2KHR>& formats2)
{
	SupportedDisplayModes.clear();

	std::unordered_map<DisplayMode, VkSurfaceFormatKHR> modes = GetAvailableFormats(formats2);

	for (const auto& it : modes)
	{
		SupportedDisplayModes.push_back(it.first);
	}

	// See if display mode requested in config is supported and return it, or default to LDR
	CurrentDisplayMode = CheckAndGetDisplayModeRequested(DisplayMode::DISPLAYMODE_LDR);

	auto found = modes.find(CurrentDisplayMode);

	Engine::Assert(AssertLevel::ASSERT_CRITICAL, found != modes.end(), L"Unable to find a suitable swapchain format.");
	CurrentDisplayMode = found->first;
	auto m_SurfaceFormat = found->second;

	// Set format based on display mode
	//m_SwapChainFormat = ConvertFormat(m_SurfaceFormat.format);
}

void VulkanViewport::EnumerateHDRMetadata(VkPhysicalDevice PhysicalDevice, VkPhysicalDeviceSurfaceInfo2KHR& physicalDeviceSurfaceInfo2, VkSurfaceCapabilities2KHR& capabilities2)
{
	VkHdrMetadataEXT hdrMetadata = {};
	hdrMetadata.sType = VK_STRUCTURE_TYPE_HDR_METADATA_EXT;
	hdrMetadata.pNext = nullptr;

	VkDisplayNativeHdrSurfaceCapabilitiesAMD* displayNativeHdrSurfaceCapabilitiesAMD = reinterpret_cast<VkDisplayNativeHdrSurfaceCapabilitiesAMD*>(capabilities2.pNext);
	displayNativeHdrSurfaceCapabilitiesAMD->pNext = &hdrMetadata;

	// Must requry FS HDR display capabilities
	// After VkSwapchainDisplayNativeHdrCreateInfoAMD is set through swapchain creation when attached to swapchain pnext.
	// This fills in data into VkHdrMetadataEXT attached as pNext in VkSurfaceCapabilities2KHR->VkDisplayNativeHdrSurfaceCapabilitiesAMD
	vkGetPhysicalDeviceSurfaceCapabilities2KHR(PhysicalDevice, &physicalDeviceSurfaceInfo2, &capabilities2);

	HDR.RedPrimary[0] = hdrMetadata.displayPrimaryRed.x;
	HDR.RedPrimary[1] = hdrMetadata.displayPrimaryRed.y;
	HDR.GreenPrimary[0] = hdrMetadata.displayPrimaryGreen.x;
	HDR.GreenPrimary[1] = hdrMetadata.displayPrimaryGreen.y;
	HDR.BluePrimary[0] = hdrMetadata.displayPrimaryBlue.x;
	HDR.BluePrimary[1] = hdrMetadata.displayPrimaryBlue.y;
	HDR.WhitePoint[0] = hdrMetadata.whitePoint.x;
	HDR.WhitePoint[1] = hdrMetadata.whitePoint.y;
	HDR.MinLuminance = hdrMetadata.minLuminance;
	HDR.MaxLuminance = hdrMetadata.maxLuminance;
}

void VulkanViewport::SetHDRMetadataAndColorspace()
{
	VkHdrMetadataEXT hdrMetadata = {};
	hdrMetadata.sType = VK_STRUCTURE_TYPE_HDR_METADATA_EXT;
	hdrMetadata.pNext = nullptr;

	hdrMetadata.displayPrimaryRed.x = HDR.RedPrimary[0];
	hdrMetadata.displayPrimaryRed.y = HDR.RedPrimary[1];
	hdrMetadata.displayPrimaryGreen.x = HDR.GreenPrimary[0];
	hdrMetadata.displayPrimaryGreen.y = HDR.GreenPrimary[1];
	hdrMetadata.displayPrimaryBlue.x = HDR.BluePrimary[0];
	hdrMetadata.displayPrimaryBlue.y = HDR.BluePrimary[1];
	hdrMetadata.whitePoint.x = HDR.WhitePoint[0];
	hdrMetadata.whitePoint.y = HDR.WhitePoint[1];

	hdrMetadata.maxLuminance = HDR.MaxLuminance;
	hdrMetadata.minLuminance = HDR.MinLuminance;

	hdrMetadata.maxContentLightLevel = HDR.MaxContentLightLevel;
	hdrMetadata.maxFrameAverageLightLevel = HDR.MaxFrameAverageLightLevel;

	if (vkSetHdrMetadataEXT)
		vkSetHdrMetadataEXT(Device->Get(), 1, &Swapchain, &hdrMetadata);
}

void VulkanViewport::PopulateHDRMetadataBasedOnDisplayMode()
{
	switch (CurrentDisplayMode)
	{
	case DisplayMode::DISPLAYMODE_LDR:
		// Values set here make no difference on HDR wide gamut monitors
		// Monitors will not undersell their capabilities, if they can go beyond rec709 gamut and 100 nits.

		// [0, 1] in respective RGB channel maps to display gamut.
		HDR.RedPrimary[0] = 0.64f;
		HDR.RedPrimary[1] = 0.33f;
		HDR.GreenPrimary[0] = 0.30f;
		HDR.GreenPrimary[1] = 0.60f;
		HDR.BluePrimary[0] = 0.15f;
		HDR.BluePrimary[1] = 0.06f;
		HDR.WhitePoint[0] = 0.3127f;
		HDR.WhitePoint[1] = 0.3290f;

		// [0, 1] actually maps to display brightness.
		// This gets ignored, writing it for completeness
		HDR.MinLuminance = 0.0f;
		HDR.MaxLuminance = 100.0f;

		// Scene dependent
		HDR.MaxContentLightLevel = 2000.0f;
		HDR.MaxFrameAverageLightLevel = 500.0f;
		break;

	case DisplayMode::DISPLAYMODE_HDR10_2084:
		// Values set here either get clipped at display capabilities or tone and gamut mapped on the display to fit its brightness and gamut range.

		// rec 2020 primaries
		HDR.RedPrimary[0] = 0.708f;
		HDR.RedPrimary[1] = 0.292f;
		HDR.GreenPrimary[0] = 0.170f;
		HDR.GreenPrimary[1] = 0.797f;
		HDR.BluePrimary[0] = 0.131f;
		HDR.BluePrimary[1] = 0.046f;
		HDR.WhitePoint[0] = 0.3127f;
		HDR.WhitePoint[1] = 0.3290f;

		// Max nits of 500 is actually low
		// Can set this value to 1000, 2000 and 4000 based on target display and content contrast range
		// However we are trying to make sure HDR10 mode doesn't look bad on HDR displays with only 300 nits brightness, hence the low max lum value.
		HDR.MinLuminance = 0.0f;
		HDR.MaxLuminance = 500.0f;

		// Scene dependent
		HDR.MaxContentLightLevel = 2000.0f;
		HDR.MaxFrameAverageLightLevel = 500.0f;
		break;

	case DisplayMode::DISPLAYMODE_HDR10_SCRGB:
		// Same as above

		// rec 709 primaries
		HDR.RedPrimary[0] = 0.64f;
		HDR.RedPrimary[1] = 0.33f;
		HDR.GreenPrimary[0] = 0.30f;
		HDR.GreenPrimary[1] = 0.60f;
		HDR.BluePrimary[0] = 0.15f;
		HDR.BluePrimary[1] = 0.06f;
		HDR.WhitePoint[0] = 0.3127f;
		HDR.WhitePoint[1] = 0.3290f;

		// Same comment as HDR10_2084.
		HDR.MinLuminance = 0.0f;
		HDR.MaxLuminance = 500.0f;

		// Scene dependent
		HDR.MaxContentLightLevel = 2000.0f;
		HDR.MaxFrameAverageLightLevel = 500.0f;
		break;

	case DisplayMode::DISPLAYMODE_FSHDR_2084:
	case DisplayMode::DISPLAYMODE_FSHDR_SCRGB:
		// FS HDR modes should already have the monitor's primaries queried through backend API's like DXGI or VK ext

		// Scene dependent
		HDR.MaxContentLightLevel = 2000.0f;
		HDR.MaxFrameAverageLightLevel = 500.0f;
		break;
	}
}
