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
#include <dxgi.h>
#include "Engine/AbstractEngine.h"
#include "Utilities/Exception.h"
#include <assert.h>

namespace
{
	inline DXGI_FORMAT GetUAVFormat(DXGI_FORMAT defaultFormat)
	{
		switch (defaultFormat)
		{
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			return DXGI_FORMAT_R8G8B8A8_UNORM;

		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			return DXGI_FORMAT_B8G8R8A8_UNORM;

		case DXGI_FORMAT_B8G8R8X8_TYPELESS:
		case DXGI_FORMAT_B8G8R8X8_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			return DXGI_FORMAT_B8G8R8X8_UNORM;

		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_R32_FLOAT:
			return DXGI_FORMAT_R32_FLOAT;

#ifdef _DEBUG
		case DXGI_FORMAT_R32G8X24_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		case DXGI_FORMAT_D16_UNORM:

			//assert(false, "Requested a UAV Format for a depth stencil Format.");
#endif

		default:
			return defaultFormat;
		}
	}

	inline auto DXGI_BitsPerPixel = [](_In_ DXGI_FORMAT fmt) -> size_t
	{
		switch (fmt)
		{
		case DXGI_FORMAT_R32G32B32A32_TYPELESS:
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
		case DXGI_FORMAT_R32G32B32A32_UINT:
		case DXGI_FORMAT_R32G32B32A32_SINT:
			return 128;

		case DXGI_FORMAT_R32G32B32_TYPELESS:
		case DXGI_FORMAT_R32G32B32_FLOAT:
		case DXGI_FORMAT_R32G32B32_UINT:
		case DXGI_FORMAT_R32G32B32_SINT:
			return 96;

		case DXGI_FORMAT_R16G16B16A16_TYPELESS:
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
		case DXGI_FORMAT_R16G16B16A16_UNORM:
		case DXGI_FORMAT_R16G16B16A16_UINT:
		case DXGI_FORMAT_R16G16B16A16_SNORM:
		case DXGI_FORMAT_R16G16B16A16_SINT:
		case DXGI_FORMAT_R32G32_TYPELESS:
		case DXGI_FORMAT_R32G32_FLOAT:
		case DXGI_FORMAT_R32G32_UINT:
		case DXGI_FORMAT_R32G32_SINT:
		case DXGI_FORMAT_R32G8X24_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		case DXGI_FORMAT_Y416:
		case DXGI_FORMAT_Y210:
		case DXGI_FORMAT_Y216:
			return 64;

		case DXGI_FORMAT_R10G10B10A2_TYPELESS:
		case DXGI_FORMAT_R10G10B10A2_UNORM:
		case DXGI_FORMAT_R10G10B10A2_UINT:
		case DXGI_FORMAT_R11G11B10_FLOAT:
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_R8G8B8A8_UINT:
		case DXGI_FORMAT_R8G8B8A8_SNORM:
		case DXGI_FORMAT_R8G8B8A8_SINT:
		case DXGI_FORMAT_R16G16_TYPELESS:
		case DXGI_FORMAT_R16G16_FLOAT:
		case DXGI_FORMAT_R16G16_UNORM:
		case DXGI_FORMAT_R16G16_UINT:
		case DXGI_FORMAT_R16G16_SNORM:
		case DXGI_FORMAT_R16G16_SINT:
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R32_FLOAT:
		case DXGI_FORMAT_R32_UINT:
		case DXGI_FORMAT_R32_SINT:
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
		case DXGI_FORMAT_R8G8_B8G8_UNORM:
		case DXGI_FORMAT_G8R8_G8B8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM:
		case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8X8_TYPELESS:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		case DXGI_FORMAT_AYUV:
		case DXGI_FORMAT_Y410:
		case DXGI_FORMAT_YUY2:
			return 32;

		case DXGI_FORMAT_P010:
		case DXGI_FORMAT_P016:
			return 24;

		case DXGI_FORMAT_R8G8_TYPELESS:
		case DXGI_FORMAT_R8G8_UNORM:
		case DXGI_FORMAT_R8G8_UINT:
		case DXGI_FORMAT_R8G8_SNORM:
		case DXGI_FORMAT_R8G8_SINT:
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_R16_FLOAT:
		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_R16_UNORM:
		case DXGI_FORMAT_R16_UINT:
		case DXGI_FORMAT_R16_SNORM:
		case DXGI_FORMAT_R16_SINT:
		case DXGI_FORMAT_B5G6R5_UNORM:
		case DXGI_FORMAT_B5G5R5A1_UNORM:
		case DXGI_FORMAT_A8P8:
		case DXGI_FORMAT_B4G4R4A4_UNORM:
			return 16;

		case DXGI_FORMAT_NV12:
		case DXGI_FORMAT_420_OPAQUE:
		case DXGI_FORMAT_NV11:
			return 12;

		case DXGI_FORMAT_R8_TYPELESS:
		case DXGI_FORMAT_R8_UNORM:
		case DXGI_FORMAT_R8_UINT:
		case DXGI_FORMAT_R8_SNORM:
		case DXGI_FORMAT_R8_SINT:
		case DXGI_FORMAT_A8_UNORM:
		case DXGI_FORMAT_AI44:
		case DXGI_FORMAT_IA44:
		case DXGI_FORMAT_P8:
			return 8;

		case DXGI_FORMAT_R1_UNORM:
			return 1;

		case DXGI_FORMAT_BC1_TYPELESS:
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGI_FORMAT_BC4_TYPELESS:
		case DXGI_FORMAT_BC4_UNORM:
		case DXGI_FORMAT_BC4_SNORM:
			return 4;

		case DXGI_FORMAT_BC2_TYPELESS:
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGI_FORMAT_BC3_TYPELESS:
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGI_FORMAT_BC5_TYPELESS:
		case DXGI_FORMAT_BC5_UNORM:
		case DXGI_FORMAT_BC5_SNORM:
		case DXGI_FORMAT_BC6H_TYPELESS:
		case DXGI_FORMAT_BC6H_UF16:
		case DXGI_FORMAT_BC6H_SF16:
		case DXGI_FORMAT_BC7_TYPELESS:
		case DXGI_FORMAT_BC7_UNORM:
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			return 8;
		}
		return 0;
	};

	inline auto ConvertFormat_DXGI_To_Format = [](const DXGI_FORMAT InFormat) -> EFormat
	{
		switch (InFormat)
		{
		case DXGI_FORMAT_R8G8B8A8_UINT:	return EFormat::RGBA8_UINT;
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:	return EFormat::BGRA8_TYPELESS;
		case DXGI_FORMAT_B8G8R8A8_UNORM: return EFormat::BGRA8_UNORM;
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return EFormat::BGRA8_UNORM_SRGB;
		case DXGI_FORMAT_BC1_UNORM: return EFormat::BC1_UNORM;
		case DXGI_FORMAT_BC1_UNORM_SRGB: return EFormat::BC1_UNORM_SRGB;
		case DXGI_FORMAT_BC2_UNORM:	return EFormat::BC2_UNORM;
		case DXGI_FORMAT_BC2_UNORM_SRGB: return EFormat::BC2_UNORM_SRGB;
		case DXGI_FORMAT_BC3_UNORM:	return EFormat::BC3_UNORM;
		case DXGI_FORMAT_BC3_UNORM_SRGB: return EFormat::BC3_UNORM_SRGB;
		case DXGI_FORMAT_R8_UINT: return EFormat::R8_UINT;
		case DXGI_FORMAT_R8_UNORM: return EFormat::R8_UNORM;
		case DXGI_FORMAT_R8_SNORM: return EFormat::R8_SNORM;
		case DXGI_FORMAT_R8G8_UINT:	return EFormat::RG8_UINT;
		case DXGI_FORMAT_R8G8_UNORM: return EFormat::RG8_UNORM;
		case DXGI_FORMAT_R16_UINT: return EFormat::R16_UINT;
		case DXGI_FORMAT_R16_UNORM: return EFormat::R16_UNORM;
		case DXGI_FORMAT_R16_FLOAT:	return EFormat::R16_FLOAT;
		case DXGI_FORMAT_R16_TYPELESS: return EFormat::R16_Typeless;
		case DXGI_FORMAT_R8G8B8A8_UNORM: return EFormat::RGBA8_UNORM;
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return EFormat::SRGBA8_UNORM;
		case DXGI_FORMAT_R10G10B10A2_UNORM: return EFormat::R10G10B10A2_UNORM;
		case DXGI_FORMAT_R11G11B10_FLOAT: return EFormat::R11G11B10_FLOAT;
		case DXGI_FORMAT_R16G16_UINT: return EFormat::RG16_UINT;
		case DXGI_FORMAT_R16G16_FLOAT: return EFormat::RG16_FLOAT;
		case DXGI_FORMAT_R32_UINT: return EFormat::R32_UINT;			
		case DXGI_FORMAT_R32_FLOAT: return EFormat::R32_FLOAT;
		case DXGI_FORMAT_R16G16B16A16_FLOAT: return EFormat::RGBA16_FLOAT;		
		case DXGI_FORMAT_R16G16B16A16_UNORM: return EFormat::RGBA16_UNORM;			
		case DXGI_FORMAT_R16G16B16A16_SNORM: return EFormat::RGBA16_SNORM;			
		case DXGI_FORMAT_R32G32_UINT: return EFormat::RG32_UINT;			
		case DXGI_FORMAT_R32G32_FLOAT: return EFormat::RG32_FLOAT;			
		case DXGI_FORMAT_R32G32B32_UINT: return EFormat::RGB32_UINT;			
		case DXGI_FORMAT_R32G32B32_FLOAT: return EFormat::RGB32_FLOAT;			
		case DXGI_FORMAT_R32G32B32A32_UINT:	return EFormat::RGBA32_UINT;			
		case DXGI_FORMAT_R32G32B32A32_FLOAT: return EFormat::RGBA32_FLOAT;			
		case DXGI_FORMAT_R32_TYPELESS: return EFormat::R32_Typeless;			
		case DXGI_FORMAT_R24G8_TYPELESS: return EFormat::R24G8_Typeless;			
		case DXGI_FORMAT_R32G8X24_TYPELESS: return EFormat::R32G8X24_Typeless;
		case DXGI_FORMAT_D16_UNORM: return EFormat::D16_UNORM;
		case DXGI_FORMAT_D32_FLOAT: return EFormat::D32_FLOAT;
		case DXGI_FORMAT_D24_UNORM_S8_UINT: return EFormat::D24_UNORM_S8_UINT;
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT: return EFormat::D32_FLOAT_S8X24_UINT;
		}
		return EFormat::UNKNOWN;
	};

	inline auto ConvertFormat_Format_To_DXGI = [](const EFormat InFormat) -> DXGI_FORMAT
	{
		switch (InFormat)
		{
		case EFormat::RGBA8_UINT: return DXGI_FORMAT_R8G8B8A8_UINT;
		case EFormat::RGBA16_UINT: return DXGI_FORMAT_R16G16B16A16_UINT;			
		case EFormat::BC1_UNORM: return DXGI_FORMAT_BC1_UNORM;			
		case EFormat::BC1_UNORM_SRGB: return DXGI_FORMAT_BC1_UNORM_SRGB;			
		case EFormat::BC2_UNORM: return DXGI_FORMAT_BC2_UNORM;			
		case EFormat::BC2_UNORM_SRGB: return DXGI_FORMAT_BC2_UNORM_SRGB;			
		case EFormat::BC3_UNORM: return DXGI_FORMAT_BC3_UNORM;			
		case EFormat::BC3_UNORM_SRGB: return DXGI_FORMAT_BC3_UNORM_SRGB;			
		case EFormat::BGRA8_UNORM: return DXGI_FORMAT_B8G8R8A8_UNORM;			
		case EFormat::BGRA8_UNORM_SRGB: return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;			
		case EFormat::BGRA8_TYPELESS: return DXGI_FORMAT_B8G8R8A8_TYPELESS;			
		case EFormat::R8_UINT: return DXGI_FORMAT_R8_UINT;			
		case EFormat::R8_UNORM: return DXGI_FORMAT_R8_UNORM;			
		case EFormat::R8_SNORM: return DXGI_FORMAT_R8_SNORM;			
		case EFormat::RG8_UINT: return DXGI_FORMAT_R8G8_UINT;			
		case EFormat::RG8_UNORM: return DXGI_FORMAT_R8G8_UNORM;			
		case EFormat::R16_UINT: return DXGI_FORMAT_R16_UINT;			
		case EFormat::R16_UNORM: return DXGI_FORMAT_R16_UNORM;			
		case EFormat::R16_FLOAT: return DXGI_FORMAT_R16_FLOAT;			
		case EFormat::R16_Typeless: return DXGI_FORMAT_R16_TYPELESS;			
		case EFormat::RGBA8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;			
		case EFormat::SRGBA8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;			
		case EFormat::R10G10B10A2_UNORM: return DXGI_FORMAT_R10G10B10A2_UNORM;			
		case EFormat::R11G11B10_FLOAT: return DXGI_FORMAT_R11G11B10_FLOAT;			
		case EFormat::RG16_UINT: return DXGI_FORMAT_R16G16_UINT;			
		case EFormat::RG16_FLOAT: return DXGI_FORMAT_R16G16_FLOAT;			
		case EFormat::R32_UINT: return DXGI_FORMAT_R32_UINT;			
		case EFormat::R32_FLOAT: return DXGI_FORMAT_R32_FLOAT;			
		case EFormat::R32_Typeless:	return DXGI_FORMAT_R32_TYPELESS;			
		case EFormat::RGBA16_FLOAT:	return DXGI_FORMAT_R16G16B16A16_FLOAT;			
		case EFormat::RGBA16_UNORM:	return DXGI_FORMAT_R16G16B16A16_UNORM;			
		case EFormat::RGBA16_SNORM:	return DXGI_FORMAT_R16G16B16A16_SNORM;			
		case EFormat::RG32_UINT: return DXGI_FORMAT_R32G32_UINT;			
		case EFormat::RG32_FLOAT: return DXGI_FORMAT_R32G32_FLOAT;			
		case EFormat::RGB32_UINT: return DXGI_FORMAT_R32G32B32_UINT;			
		case EFormat::RGB32_FLOAT: return DXGI_FORMAT_R32G32B32_FLOAT;			
		case EFormat::RGBA32_UINT: return DXGI_FORMAT_R32G32B32A32_UINT;			
		case EFormat::RGBA32_FLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;		
		case EFormat::R24G8_Typeless: return DXGI_FORMAT_R24G8_TYPELESS;			
		case EFormat::R32G8X24_Typeless: return DXGI_FORMAT_R32G8X24_TYPELESS;
		case EFormat::D16_UNORM: return DXGI_FORMAT_D16_UNORM;
		case EFormat::D32_FLOAT: return DXGI_FORMAT_D32_FLOAT;
		case EFormat::D24_UNORM_S8_UINT: return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case EFormat::D32_FLOAT_S8X24_UINT: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		}
		return DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	};

	inline constexpr std::vector<EFormat> GetAllFormats()
	{
		std::vector<EFormat> Formats;
		{
			Formats.push_back(EFormat::RGBA8_UINT);
			Formats.push_back(EFormat::BGRA8_TYPELESS);
			Formats.push_back(EFormat::BGRA8_UNORM);
			Formats.push_back(EFormat::BGRA8_UNORM_SRGB);
			Formats.push_back(EFormat::BC1_UNORM);
			Formats.push_back(EFormat::BC1_UNORM_SRGB);
			Formats.push_back(EFormat::BC2_UNORM);
			Formats.push_back(EFormat::BC2_UNORM_SRGB);
			Formats.push_back(EFormat::BC3_UNORM);
			Formats.push_back(EFormat::BC3_UNORM_SRGB);
			Formats.push_back(EFormat::R8_UINT);
			Formats.push_back(EFormat::R8_UNORM);
			Formats.push_back(EFormat::R8_SNORM);
			Formats.push_back(EFormat::RG8_UINT);
			Formats.push_back(EFormat::RG8_UNORM);
			Formats.push_back(EFormat::R16_UINT);
			Formats.push_back(EFormat::R16_UNORM);
			Formats.push_back(EFormat::R16_FLOAT);
			Formats.push_back(EFormat::R16_Typeless);
			Formats.push_back(EFormat::RGBA8_UNORM);
			Formats.push_back(EFormat::SRGBA8_UNORM);
			Formats.push_back(EFormat::R10G10B10A2_UNORM);
			Formats.push_back(EFormat::R11G11B10_FLOAT);
			Formats.push_back(EFormat::RG16_UINT);
			Formats.push_back(EFormat::RG16_FLOAT);
			Formats.push_back(EFormat::R32_UINT);
			Formats.push_back(EFormat::R32_FLOAT);
			Formats.push_back(EFormat::RGBA16_FLOAT);
			Formats.push_back(EFormat::RGBA16_UNORM);
			Formats.push_back(EFormat::RGBA16_SNORM);
			Formats.push_back(EFormat::RG32_UINT);
			Formats.push_back(EFormat::RG32_FLOAT);
			Formats.push_back(EFormat::RGB32_UINT);
			Formats.push_back(EFormat::RGB32_FLOAT);
			Formats.push_back(EFormat::RGBA32_UINT);
			Formats.push_back(EFormat::RGBA32_FLOAT);
			Formats.push_back(EFormat::R32_Typeless);
			Formats.push_back(EFormat::R24G8_Typeless);
			Formats.push_back(EFormat::R32G8X24_Typeless);
			Formats.push_back(EFormat::D16_UNORM);
			Formats.push_back(EFormat::D32_FLOAT);
			Formats.push_back(EFormat::D24_UNORM_S8_UINT);
			Formats.push_back(EFormat::D32_FLOAT_S8X24_UINT);
		}
		return Formats;
	};

	inline constexpr std::vector<DXGI_FORMAT> GetAllDXGIFormats()
	{
		std::vector<DXGI_FORMAT> Formats;
		{
			Formats.push_back(DXGI_FORMAT_R8G8B8A8_UINT);
			Formats.push_back(DXGI_FORMAT_R16G16B16A16_UINT);
			Formats.push_back(DXGI_FORMAT_BC1_UNORM);
			Formats.push_back(DXGI_FORMAT_BC1_UNORM_SRGB);
			Formats.push_back(DXGI_FORMAT_BC2_UNORM);
			Formats.push_back(DXGI_FORMAT_BC2_UNORM_SRGB);
			Formats.push_back(DXGI_FORMAT_BC3_UNORM);
			Formats.push_back(DXGI_FORMAT_BC3_UNORM_SRGB);
			Formats.push_back(DXGI_FORMAT_B8G8R8A8_UNORM);
			Formats.push_back(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);
			Formats.push_back(DXGI_FORMAT_B8G8R8A8_TYPELESS);
			Formats.push_back(DXGI_FORMAT_R8_UINT);
			Formats.push_back(DXGI_FORMAT_R8_UNORM);
			Formats.push_back(DXGI_FORMAT_R8_SNORM);
			Formats.push_back(DXGI_FORMAT_R8G8_UINT);
			Formats.push_back(DXGI_FORMAT_R8G8_UNORM);
			Formats.push_back(DXGI_FORMAT_R16_UINT);
			Formats.push_back(DXGI_FORMAT_R16_UNORM);
			Formats.push_back(DXGI_FORMAT_R16_FLOAT);
			Formats.push_back(DXGI_FORMAT_R16_TYPELESS);
			Formats.push_back(DXGI_FORMAT_R8G8B8A8_UNORM);
			Formats.push_back(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
			Formats.push_back(DXGI_FORMAT_R10G10B10A2_UNORM);
			Formats.push_back(DXGI_FORMAT_R11G11B10_FLOAT);
			Formats.push_back(DXGI_FORMAT_R16G16_UINT);
			Formats.push_back(DXGI_FORMAT_R16G16_FLOAT);
			Formats.push_back(DXGI_FORMAT_R32_UINT);
			Formats.push_back(DXGI_FORMAT_R32_FLOAT);
			Formats.push_back(DXGI_FORMAT_R32_TYPELESS);
			Formats.push_back(DXGI_FORMAT_R16G16B16A16_FLOAT);
			Formats.push_back(DXGI_FORMAT_R16G16B16A16_UNORM);
			Formats.push_back(DXGI_FORMAT_R16G16B16A16_SNORM);
			Formats.push_back(DXGI_FORMAT_R32G32_UINT);
			Formats.push_back(DXGI_FORMAT_R32G32_FLOAT);
			Formats.push_back(DXGI_FORMAT_R32G32B32_UINT);
			Formats.push_back(DXGI_FORMAT_R32G32B32_FLOAT);
			Formats.push_back(DXGI_FORMAT_R32G32B32A32_UINT);
			Formats.push_back(DXGI_FORMAT_R32G32B32A32_FLOAT);
			Formats.push_back(DXGI_FORMAT_R24G8_TYPELESS);
			Formats.push_back(DXGI_FORMAT_R32G8X24_TYPELESS);
			Formats.push_back(DXGI_FORMAT_D16_UNORM);
			Formats.push_back(DXGI_FORMAT_D32_FLOAT);
			Formats.push_back(DXGI_FORMAT_D24_UNORM_S8_UINT);
			Formats.push_back(DXGI_FORMAT_D32_FLOAT_S8X24_UINT);
		}
		return Formats;
	};

	inline bool IsValidDepthFormat(const DXGI_FORMAT Format)
	{
		return Format == DXGI_FORMAT_R16_TYPELESS || Format == DXGI_FORMAT_R32_TYPELESS || Format == DXGI_FORMAT_R24G8_TYPELESS || Format == DXGI_FORMAT_R32G8X24_TYPELESS;
	};

	inline bool IsDepthSRVSupported(const DXGI_FORMAT Format)
	{
		return Format == DXGI_FORMAT_R16_TYPELESS || Format == DXGI_FORMAT_R32_TYPELESS;
	};

	inline auto GetDepthSRVFormat = [](const DXGI_FORMAT Format) -> DXGI_FORMAT
	{
		if (Format == DXGI_FORMAT::DXGI_FORMAT_R16_TYPELESS)
		{
			return DXGI_FORMAT_R16_UINT;
		}
		else if (Format == DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS)
		{
			return DXGI_FORMAT_R32_UINT;
		}
		return DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	};
	inline auto GetDepthViewFormat = [](const DXGI_FORMAT Format) -> DXGI_FORMAT
	{
		if (Format == DXGI_FORMAT::DXGI_FORMAT_R16_TYPELESS)
		{
			return DXGI_FORMAT_D16_UNORM;
		}
		else if (Format == DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS)
		{
			return DXGI_FORMAT_D32_FLOAT;
		}
		else if (Format == DXGI_FORMAT::DXGI_FORMAT_R24G8_TYPELESS)
		{
			return DXGI_FORMAT_D24_UNORM_S8_UINT;
		}
		else if (Format == DXGI_FORMAT::DXGI_FORMAT_R32G8X24_TYPELESS)
		{
			return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		}
		return DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	};
}
