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

VulkanViewport::VulkanViewport(VulkanDevice* InDevice, HWND* InHandle)
	: Device(InDevice)
	, bRecreateSurface(false)
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
	vkDestroySurfaceKHR(Device->GetInstance(), Surface, nullptr);
}

void VulkanViewport::InitWindow(HWND* InHandle, std::uint32_t InWidth, std::uint32_t InHeight, bool bFullscreen)
{
	SizeX = InWidth;
	SizeY = InHeight;

	Viewport = { SizeX, SizeY, 0, 0, 0.0f, 1.0f };

	Handle = InHandle;

	if (bRecreateSurface)
	{
		vkDestroySurfaceKHR(Device->GetInstance(), Surface, nullptr);

		Surface = VK_NULL_HANDLE;

		bRecreateSurface = false;

		VkWin32SurfaceCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hwnd = (HWND)Handle;
		createInfo.hinstance = GetModuleHandle(nullptr);

		if (vkCreateWin32SurfaceKHR(Device->GetInstance(), &createInfo, nullptr, &Surface) != VK_SUCCESS) {
			Engine::WriteToConsole("failed to create window surface!");
		}
	}

	VkSurfaceCapabilitiesKHR surface_properties;
	VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device->GetPhysicalDevice(), Surface, &surface_properties));

	std::vector<VkFormat> const& preferred_formats = { VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_A8B8G8R8_SRGB_PACK32 };

	VkSurfaceFormatKHR format;
	{
		uint32_t surface_format_count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(Device->GetPhysicalDevice(), Surface, &surface_format_count, nullptr);
		assert(0 < surface_format_count);
		std::vector<VkSurfaceFormatKHR> supported_surface_formats(surface_format_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(Device->GetPhysicalDevice(), Surface, &surface_format_count, supported_surface_formats.data());

		auto it = std::find_if(supported_surface_formats.begin(),
			supported_surface_formats.end(),
			[&preferred_formats](VkSurfaceFormatKHR surface_format) {
				return std::any_of(preferred_formats.begin(),
					preferred_formats.end(),
					[&surface_format](VkFormat format) { return format == surface_format.format; });
			});

		format = it != supported_surface_formats.end() ? *it : supported_surface_formats[0];
	}

	VkExtent2D swapchain_size;
	if (surface_properties.currentExtent.width == 0xFFFFFFFF)
	{
		swapchain_size.width = InWidth;
		swapchain_size.height = InHeight;
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
		.imageFormat = format.format,                              // Format of the swapchain images (e.g., VK_FORMAT_B8G8R8A8_SRGB)
		.imageColorSpace = format.colorSpace,                          // Color space of the images (e.g., VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
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

	Format = ConvertFormat_VkFormat_To_Format(format.format);

	VK_CHECK(vkCreateSwapchainKHR(Device->Get(), &info, nullptr, &Swapchain));

	if (old_swapchain != VK_NULL_HANDLE)
	{
		for (auto& images : SwapchainImages)
		{
			images.Destroy(Device->Get());
		}
		SwapchainImages.clear();

		vkDestroySwapchainKHR(Device->Get(), old_swapchain, nullptr);
	}

	uint32_t image_count;
	VK_CHECK(vkGetSwapchainImagesKHR(Device->Get(), Swapchain, &image_count, nullptr));

	std::vector<VkImage> Images(image_count);
	VK_CHECK(vkGetSwapchainImagesKHR(Device->Get(), Swapchain, &image_count, Images.data()));

	for (size_t i = 0; i < image_count; i++)
	{
		VkImageViewCreateInfo view_info{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.image = Images[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = format.format,
			.subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1} };

		VkImageView image_view;
		VK_CHECK(vkCreateImageView(Device->Get(), &view_info, nullptr, &image_view));

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		VkFence fence;
		VkResult result = vkCreateFence(Device->Get(), &fenceInfo, nullptr, &fence);

		SwapchainImages.push_back(SwapchainImage(Images[i], image_view, fence, VK_IMAGE_LAYOUT_UNDEFINED));
	}
}

VkResult VulkanViewport::AcquireNextImageIndex(uint32_t* image)
{
	VkSemaphore acquire_semaphore = Device->AcquireSemaphore();

	constexpr bool bWaitBefore = true;

	VkResult Result_Wait = {};
	VkResult Result_Reset = {};

	if (bWaitBefore)
	{
		Result_Wait = vkWaitForFences(Device->Get(), 1, &SwapchainImages[*image].Fence, true, UINT64_MAX);
		Result_Reset = vkResetFences(Device->Get(), 1, &SwapchainImages[*image].Fence);
	}

	VkResult Result = vkAcquireNextImageKHR(Device->Get(), Swapchain, UINT64_MAX, acquire_semaphore, SwapchainImages[*image].Fence, image);

	if (!bWaitBefore)
	{
		Result_Wait = vkWaitForFences(Device->Get(), 1, &SwapchainImages[*image].Fence, true, UINT64_MAX);
		Result_Reset = vkResetFences(Device->Get(), 1, &SwapchainImages[*image].Fence);
	}

	return Result;
}

void VulkanViewport::BeginFrame()
{
	VkResult Result = AcquireNextImageIndex(&FrameIndex);

	if (Result == VK_SUBOPTIMAL_KHR || Result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		if (!ResizeSwapChain(SizeX, SizeY))
		{
			Engine::WriteToConsole("Resize failed");
		}
		vkQueueWaitIdle(Device->GetQueue());
		Result = AcquireNextImageIndex(&FrameIndex);
	}

	if (Result != VK_SUCCESS)
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
		CMD->ExecuteCommandList();

		SwapchainImages[FrameIndex].CurrentImageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	}

	CopyToBackBuffer(pRT);

	{
		VkSemaphore acquire_semaphore = Device->AcquireSignalledSemaphore();
		//if (!acquire_semaphore)
		//	acquire_semaphore = Device->AcquireSemaphore(SemaphoreSignalStatus::Undefined);

		VkPresentInfoKHR present{
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &acquire_semaphore,
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
}

void VulkanViewport::CopyToBackBuffer(IRenderTarget* pRT)
{
	auto CMDList = Device->GetIMCommandBuffer();
	CMDList->BeginRecordCommandList();

	CMDList->CopyResource(SwapchainImages[FrameIndex].Image, VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, static_cast<VulkanRenderTarget*>(pRT)->Image, static_cast<VulkanRenderTarget*>(pRT)->CurrentLayout, {SizeX, SizeY});

	CMDList->FinishRecordCommandList();
	CMDList->ExecuteCommandList();
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

	InitWindow(Handle, SizeX, SizeY, value);
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

void VulkanViewport::VsyncInterval(const std::size_t value)
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

	InitWindow(Handle, Width, Width, IsFullScreen());

	return true;
}
