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

#include <vector>
#include "Engine/AbstractEngine.h"
#include "VulkanDevice.h"

namespace
{
	inline VkFormat GetVulkanUAVFormat(VkFormat defaultFormat)
	{
		switch (defaultFormat)
		{
		case VK_FORMAT_R8G8B8A8_UNORM:
		case VK_FORMAT_R8G8B8A8_SRGB:
			return VK_FORMAT_B8G8R8A8_UNORM;

		case VK_FORMAT_B8G8R8A8_UNORM:
		case VK_FORMAT_B8G8R8A8_SRGB:
			return VK_FORMAT_B8G8R8A8_UNORM;

		case VK_FORMAT_R32_SFLOAT:
			return VK_FORMAT_R32_SFLOAT;

		case VK_FORMAT_D32_SFLOAT_S8_UINT:
		case VK_FORMAT_D32_SFLOAT:
		case VK_FORMAT_D24_UNORM_S8_UINT:
		case VK_FORMAT_D16_UNORM:
			assert(false); //  "Requested a UAV Format for a depth stencil Format."

		default:
			return defaultFormat;
		}
	}

	inline size_t Vulkan_BitsPerPixel(_In_ VkFormat Format)
	{
		switch (Format)
		{
		case VK_FORMAT_B8G8R8A8_SRGB:
		case VK_FORMAT_B8G8R8A8_UNORM:
		case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
		case VK_FORMAT_X8_D24_UNORM_PACK32:
		case VK_FORMAT_D24_UNORM_S8_UINT:
		case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
		case VK_FORMAT_D32_SFLOAT:
		case VK_FORMAT_R16G16_UNORM:
		case VK_FORMAT_R16G16_SFLOAT:
		case VK_FORMAT_R32_UINT:
		case VK_FORMAT_R32_SINT:
		case VK_FORMAT_R8G8B8A8_UNORM:
		case VK_FORMAT_R8G8B8A8_UINT:
		case VK_FORMAT_R8G8B8A8_SNORM:
		case VK_FORMAT_R16G16_UINT:
		case VK_FORMAT_R32_SFLOAT:
			return 32;
		case VK_FORMAT_R8_UNORM:
		case VK_FORMAT_R8_UINT:
			return 8;
		case VK_FORMAT_R16_UNORM:
		case VK_FORMAT_D16_UNORM:
		case VK_FORMAT_R16_UINT:
		case VK_FORMAT_R16_SINT:
		case VK_FORMAT_R16_SFLOAT:
		case VK_FORMAT_R5G6B5_UNORM_PACK16:
		case VK_FORMAT_R8G8_UNORM:
			return 16;
		case VK_FORMAT_R16G16B16A16_SFLOAT:
		case VK_FORMAT_R32G32_SFLOAT:
		case VK_FORMAT_R32G32_UINT:
		case VK_FORMAT_R32G32_SINT:
		case VK_FORMAT_R16G16B16A16_UNORM:
		case VK_FORMAT_R16G16B16A16_SNORM:
		case VK_FORMAT_R16G16B16A16_UINT:
		case VK_FORMAT_R16G16B16A16_SINT:
		case VK_FORMAT_R64_UINT:
		case VK_FORMAT_R64_SINT:
			return 64;
		case VK_FORMAT_R32G32B32A32_SFLOAT:
		case VK_FORMAT_R32G32B32A32_UINT:
			return 128;
		case VK_FORMAT_D32_SFLOAT_S8_UINT:
			return 8;
		case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
		case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
		case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
		case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
			return 4;
		case VK_FORMAT_BC2_UNORM_BLOCK:
		case VK_FORMAT_BC2_SRGB_BLOCK:
		case VK_FORMAT_BC3_UNORM_BLOCK:
		case VK_FORMAT_BC3_SRGB_BLOCK:
		case VK_FORMAT_BC4_UNORM_BLOCK:
		case VK_FORMAT_BC4_SNORM_BLOCK:
		case VK_FORMAT_BC5_UNORM_BLOCK:
		case VK_FORMAT_BC5_SNORM_BLOCK:
			return 8;
		case VK_FORMAT_BC6H_UFLOAT_BLOCK:
		case VK_FORMAT_BC6H_SFLOAT_BLOCK:
		case VK_FORMAT_BC7_UNORM_BLOCK:
		case VK_FORMAT_BC7_SRGB_BLOCK:
			return 8;
		};
		return 0;
	};

	inline EFormat ConvertFormat_VkFormat_To_Format(const VkFormat InFormat)
	{
		switch (InFormat)
		{
		case VK_FORMAT_R8G8B8A8_UINT:	return EFormat::RGBA8_UINT;
		case VK_FORMAT_B8G8R8A8_UNORM: return EFormat::BGRA8_UNORM;
		case VK_FORMAT_B8G8R8A8_SRGB: return EFormat::BGRA8_UNORM_SRGB;
			//case VK_FORMAT_BC1_UNORM: return EFormat::BC1_UNORM;
			//case VK_FORMAT_BC1_UNORM_SRGB: return EFormat::BC1_UNORM_SRGB;
			//case VK_FORMAT_BC2_UNORM:	return EFormat::BC2_UNORM;
			//case VK_FORMAT_BC2_UNORM_SRGB: return EFormat::BC2_UNORM_SRGB;
			//case VK_FORMAT_BC3_UNORM:	return EFormat::BC3_UNORM;
			//case VK_FORMAT_BC3_UNORM_SRGB: return EFormat::BC3_UNORM_SRGB;
		case VK_FORMAT_R8_UINT: return EFormat::R8_UINT;
		case VK_FORMAT_R8_UNORM: return EFormat::R8_UNORM;
		case VK_FORMAT_R8_SNORM: return EFormat::R8_SNORM;
		case VK_FORMAT_R8G8_UINT:	return EFormat::RG8_UINT;
		case VK_FORMAT_R8G8_UNORM: return EFormat::RG8_UNORM;
		case VK_FORMAT_R16_UINT: return EFormat::R16_UINT;
		case VK_FORMAT_R16_UNORM: return EFormat::R16_UNORM;
		case VK_FORMAT_R16_SFLOAT:	return EFormat::R16_FLOAT;
			//case VK_FORMAT_R16_TYPELESS: return EFormat::R16_Typeless;
		case VK_FORMAT_R8G8B8A8_UNORM: return EFormat::RGBA8_UNORM;
		case VK_FORMAT_R8G8B8A8_SRGB: return EFormat::SRGBA8_UNORM;
		case VK_FORMAT_A2R10G10B10_UNORM_PACK32: return EFormat::R10G10B10A2_UNORM;
		case VK_FORMAT_B10G11R11_UFLOAT_PACK32: return EFormat::R11G11B10_FLOAT;
		case VK_FORMAT_R16G16_UINT: return EFormat::RG16_UINT;
		case VK_FORMAT_R16G16_SFLOAT: return EFormat::RG16_FLOAT;
		case VK_FORMAT_R32_UINT: return EFormat::R32_UINT;
		case VK_FORMAT_R32_SINT: return EFormat::R32_SINT;
		case VK_FORMAT_R32_SFLOAT: return EFormat::R32_FLOAT;
		case VK_FORMAT_R16G16B16A16_SFLOAT: return EFormat::RGBA16_FLOAT;
		case VK_FORMAT_R16G16B16A16_UNORM: return EFormat::RGBA16_UNORM;
		case VK_FORMAT_R16G16B16A16_SNORM: return EFormat::RGBA16_SNORM;
		case VK_FORMAT_R32G32_UINT: return EFormat::RG32_UINT;
		case VK_FORMAT_R32G32_SINT: return EFormat::RG32_SINT;
		case VK_FORMAT_R32G32_SFLOAT: return EFormat::RG32_FLOAT;
		case VK_FORMAT_R32G32B32_UINT: return EFormat::RGB32_UINT;
		case VK_FORMAT_R32G32B32_SINT: return EFormat::RGB32_SINT;
		case VK_FORMAT_R32G32B32_SFLOAT: return EFormat::RGB32_FLOAT;
		case VK_FORMAT_R32G32B32A32_UINT:	return EFormat::RGBA32_UINT;
		case VK_FORMAT_R32G32B32A32_SINT:	return EFormat::RGBA32_SINT;
		case VK_FORMAT_R32G32B32A32_SFLOAT: return EFormat::RGBA32_FLOAT;
			//case VK_FORMAT_R32_TYPELESS: return EFormat::R32_Typeless;
			//case VK_FORMAT_R24G8_TYPELESS: return EFormat::R24G8_Typeless;
			//case VK_FORMAT_R32G8X24_TYPELESS: return EFormat::R32G8X24_Typeless;
		case VK_FORMAT_D16_UNORM: return EFormat::D16_UNORM;
		case VK_FORMAT_D32_SFLOAT: return EFormat::D32_FLOAT;
		case VK_FORMAT_D24_UNORM_S8_UINT: return EFormat::D24_UNORM_S8_UINT;
		case VK_FORMAT_D32_SFLOAT_S8_UINT: return EFormat::D32_FLOAT_S8X24_UINT;
		}
		return EFormat::UNKNOWN;
	};

	inline VkFormat ConvertFormat_Format_To_VkFormat(const EFormat InFormat)
	{
		switch (InFormat)
		{
		case EFormat::RGBA8_UINT: return VK_FORMAT_R8G8B8A8_UINT;
		case EFormat::RGBA16_UINT: return VK_FORMAT_R16G16B16A16_UINT;
			//case EFormat::BC1_UNORM: return VK_FORMAT_BC1_UNORM;
			//case EFormat::BC1_UNORM_SRGB: return VK_FORMAT_BC1_UNORM_SRGB;
			//case EFormat::BC2_UNORM: return VK_FORMAT_BC2_UNORM;
			//case EFormat::BC2_UNORM_SRGB: return VK_FORMAT_BC2_UNORM_SRGB;
			//case EFormat::BC3_UNORM: return VK_FORMAT_BC3_UNORM;
			//case EFormat::BC3_UNORM_SRGB: return VK_FORMAT_BC3_UNORM_SRGB;
		case EFormat::BGRA8_UNORM: return VK_FORMAT_B8G8R8A8_UNORM;
		case EFormat::BGRA8_UNORM_SRGB: return VK_FORMAT_B8G8R8A8_SRGB;
			//case EFormat::BGRA8_TYPELESS: return VK_FORMAT_B8G8R8A8_TYPELESS;
		case EFormat::R8_UINT: return VK_FORMAT_R8_UINT;
		case EFormat::R8_UNORM: return VK_FORMAT_R8_UNORM;
		case EFormat::R8_SNORM: return VK_FORMAT_R8_SNORM;
		case EFormat::RG8_UINT: return VK_FORMAT_R8G8_UINT;
		case EFormat::RG8_UNORM: return VK_FORMAT_R8G8_UNORM;
		case EFormat::R16_UINT: return VK_FORMAT_R16_UINT;
		case EFormat::R16_UNORM: return VK_FORMAT_R16_UNORM;
		case EFormat::R16_FLOAT: return VK_FORMAT_R16_SFLOAT;
			//case EFormat::R16_Typeless: return VK_FORMAT_R16_TYPELESS;
		case EFormat::RGBA8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
		case EFormat::SRGBA8_UNORM: return VK_FORMAT_R8G8B8A8_SRGB;
		case EFormat::R10G10B10A2_UNORM: return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
		case EFormat::R11G11B10_FLOAT: return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
		case EFormat::RG16_UINT: return VK_FORMAT_R16G16_UINT;
		case EFormat::RG16_FLOAT: return VK_FORMAT_R16G16_SFLOAT;
		case EFormat::R32_UINT: return VK_FORMAT_R32_UINT;
		case EFormat::R32_SINT: return VK_FORMAT_R32_SINT;
		case EFormat::R32_FLOAT: return VK_FORMAT_R32_SFLOAT;
			//case EFormat::R32_Typeless:	return VK_FORMAT_R32_TYPELESS;
		case EFormat::RGBA16_FLOAT:	return VK_FORMAT_R16G16B16A16_SFLOAT;
		case EFormat::RGBA16_UNORM:	return VK_FORMAT_R16G16B16A16_UNORM;
		case EFormat::RGBA16_SNORM:	return VK_FORMAT_R16G16B16A16_SNORM;
		case EFormat::RG32_UINT: return VK_FORMAT_R32G32_UINT;
		case EFormat::RG32_SINT: return VK_FORMAT_R32G32_SINT;
		case EFormat::RG32_FLOAT: return VK_FORMAT_R32G32_SFLOAT;
		case EFormat::RGB32_UINT: return VK_FORMAT_R32G32B32_UINT;
		case EFormat::RGB32_SINT: return VK_FORMAT_R32G32B32_SINT;
		case EFormat::RGB32_FLOAT: return VK_FORMAT_R32G32B32_SFLOAT;
		case EFormat::RGBA32_UINT: return VK_FORMAT_R32G32B32A32_UINT;
		case EFormat::RGBA32_SINT: return VK_FORMAT_R32G32B32A32_SINT;
		case EFormat::RGBA32_FLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
			//case EFormat::R24G8_Typeless: return VK_FORMAT_R24G8_TYPELESS;
			//case EFormat::R32G8X24_Typeless: return VK_FORMAT_R32G8X24_TYPELESS;
		case EFormat::D16_UNORM: return VK_FORMAT_D16_UNORM;
		case EFormat::D32_FLOAT: return VK_FORMAT_D32_SFLOAT;
		case EFormat::D24_UNORM_S8_UINT: return VK_FORMAT_D24_UNORM_S8_UINT;
		case EFormat::D32_FLOAT_S8X24_UINT: return VK_FORMAT_D32_SFLOAT_S8_UINT;
		}
		return VkFormat::VK_FORMAT_UNDEFINED;
	};

	inline constexpr std::vector<VkFormat> GetAllVulkanFormats()
	{
		std::vector<VkFormat> Formats;
		{
			Formats.push_back(VK_FORMAT_R8G8B8A8_UINT);
			Formats.push_back(VK_FORMAT_R16G16B16A16_UINT);
			//Formats.push_back(VK_FORMAT_BC1_UNORM);
			//Formats.push_back(VK_FORMAT_BC1_UNORM_SRGB);
			//Formats.push_back(VK_FORMAT_BC2_UNORM);
			//Formats.push_back(VK_FORMAT_BC2_UNORM_SRGB);
			//Formats.push_back(VK_FORMAT_BC3_UNORM);
			//Formats.push_back(VK_FORMAT_BC3_UNORM_SRGB);
			Formats.push_back(VK_FORMAT_B8G8R8A8_UNORM);
			Formats.push_back(VK_FORMAT_B8G8R8A8_SRGB);
			//Formats.push_back(VK_FORMAT_B8G8R8A8_TYPELESS);
			Formats.push_back(VK_FORMAT_R8_UINT);
			Formats.push_back(VK_FORMAT_R8_UNORM);
			Formats.push_back(VK_FORMAT_R8_SNORM);
			Formats.push_back(VK_FORMAT_R8G8_UINT);
			Formats.push_back(VK_FORMAT_R8G8_UNORM);
			Formats.push_back(VK_FORMAT_R16_UINT);
			Formats.push_back(VK_FORMAT_R16_UNORM);
			Formats.push_back(VK_FORMAT_R16_SFLOAT);
			//Formats.push_back(VK_FORMAT_R16_TYPELESS);
			Formats.push_back(VK_FORMAT_R8G8B8A8_UNORM);
			Formats.push_back(VK_FORMAT_R8G8B8A8_SRGB);
			Formats.push_back(VK_FORMAT_A2R10G10B10_UNORM_PACK32);
			Formats.push_back(VK_FORMAT_B10G11R11_UFLOAT_PACK32);
			Formats.push_back(VK_FORMAT_R16G16_UINT);
			Formats.push_back(VK_FORMAT_R16G16_SFLOAT);
			Formats.push_back(VK_FORMAT_R32_UINT);
			Formats.push_back(VK_FORMAT_R32_SINT);
			Formats.push_back(VK_FORMAT_R32_SFLOAT);
			//Formats.push_back(VK_FORMAT_R32_TYPELESS);
			Formats.push_back(VK_FORMAT_R16G16B16A16_SFLOAT);
			Formats.push_back(VK_FORMAT_R16G16B16A16_UNORM);
			Formats.push_back(VK_FORMAT_R16G16B16A16_SNORM);
			Formats.push_back(VK_FORMAT_R32G32_UINT);
			Formats.push_back(VK_FORMAT_R32G32_SINT);
			Formats.push_back(VK_FORMAT_R32G32_SFLOAT);
			Formats.push_back(VK_FORMAT_R32G32B32_UINT);
			Formats.push_back(VK_FORMAT_R32G32B32_SINT);
			Formats.push_back(VK_FORMAT_R32G32B32_SFLOAT);
			Formats.push_back(VK_FORMAT_R32G32B32A32_UINT);
			Formats.push_back(VK_FORMAT_R32G32B32A32_SINT);
			Formats.push_back(VK_FORMAT_R32G32B32A32_SFLOAT);
			//Formats.push_back(VK_FORMAT_R24G8_TYPELESS);
			//Formats.push_back(VK_FORMAT_R32G8X24_TYPELESS);
			Formats.push_back(VK_FORMAT_D16_UNORM);
			Formats.push_back(VK_FORMAT_D32_SFLOAT);
			Formats.push_back(VK_FORMAT_D24_UNORM_S8_UINT);
			Formats.push_back(VK_FORMAT_D32_SFLOAT_S8_UINT);
		}
		return Formats;
	};

	inline bool IsValidVulkanDepthOnlyFormat(const VkFormat Format)
	{
		return Format == VK_FORMAT_D32_SFLOAT || Format == VK_FORMAT_D16_UNORM;
	};

	inline bool IsValidVulkanDepthStencilFormat(const VkFormat Format)
	{
		return Format == VK_FORMAT_D24_UNORM_S8_UINT || Format == VK_FORMAT_D32_SFLOAT_S8_UINT || Format == VK_FORMAT_D16_UNORM_S8_UINT || Format == VK_FORMAT_S8_UINT;
	};

	inline bool IsValidVulkanDepthFormat(const VkFormat Format)
	{
		return IsValidVulkanDepthOnlyFormat(Format) || IsValidVulkanDepthStencilFormat(Format);
	};

	inline bool IsVulkanDepthSRVSupported(const VkFormat Format)
	{
		return Format == VK_FORMAT_R16_SFLOAT || Format == VK_FORMAT_R32_SFLOAT;
	};
}
