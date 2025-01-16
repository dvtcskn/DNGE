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
#include <vector>
#include "Engine/AbstractEngine.h"
#include "VulkanDevice.h"

class VulkanViewport final
{
	sBaseClassBody(sClassConstructor, VulkanViewport)
public:
	VulkanViewport(VulkanDevice* Device, HWND* Handle);
	~VulkanViewport();

	void InitWindow(HWND* Handle, std::uint32_t InWidth, std::uint32_t InHeight, bool bFullscreen);

	void BeginFrame();
	void Present(IRenderTarget* pRT);
	bool ResizeSwapChain(std::size_t Width, std::size_t Height);

	void FullScreen(const bool value);
	void Vsync(const bool value);
	void VsyncInterval(const std::size_t value);

	bool IsFullScreen() const { return bIsFullScreen; }
	bool IsVsyncEnabled() const { return bVSYNC; }
	std::size_t GetVsyncInterval() const { return Vsync_Interval; }

	FORCEINLINE std::uint32_t GetBackBufferCount() const { return (std::uint32_t)SwapchainImages.size(); }

	FORCEINLINE const sViewport& GetViewport() { return Viewport; }
	FORCEINLINE std::uint32_t GetViewportWidth() const { return SizeX; }
	FORCEINLINE std::uint32_t GetViewportHeight() const { return SizeY; }
	FORCEINLINE sScreenDimension GetScreenDimension() const { return sScreenDimension(SizeX, SizeY); }

	const VkSurfaceKHR& GetSurface() const { return Surface; }

	EFormat GetFormat() const { return Format; }

private:
	VkResult AcquireNextImageIndex(uint32_t* image);
	void CopyToBackBuffer(IRenderTarget* pRT);

private:
	VulkanDevice* Device;
	VkSurfaceKHR Surface;
	VkSwapchainKHR Swapchain;

	struct SwapchainImage
	{
		VkImage Image;
		VkImageView View;
		VkImageLayout CurrentImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkFence Fence;

		SwapchainImage(VkImage image = VK_NULL_HANDLE, VkImageView view = VK_NULL_HANDLE, VkFence fence = VK_NULL_HANDLE, VkImageLayout currentImageLayout = VK_IMAGE_LAYOUT_UNDEFINED)
			: Image(image)
			, View(view)
			, CurrentImageLayout(currentImageLayout)
			, Fence(fence)
		{}

		void Destroy(VkDevice Device)
		{
			vkDestroyImageView(Device, View, nullptr);
			View = VK_NULL_HANDLE;
			vkDestroyImage(Device, Image, nullptr);
			Image = VK_NULL_HANDLE;
			vkDestroyFence(Device, Fence, nullptr);
			Fence = VK_NULL_HANDLE;
		}
	};

	std::vector<SwapchainImage> SwapchainImages;

private:
	HWND* Handle;
	bool bRecreateSurface;

	bool bIsFullScreen;
	bool bVSYNC;
	std::int32_t Vsync_Interval;
	VkSurfaceFullScreenExclusiveInfoEXT      surface_full_screen_exclusive_info_EXT;
	VkSurfaceFullScreenExclusiveWin32InfoEXT surface_full_screen_exclusive_Win32_info_EXT;

	std::uint32_t FrameIndex;

	EFormat Format;
	std::uint32_t SizeX;
	std::uint32_t SizeY;
	sViewport Viewport;
};
