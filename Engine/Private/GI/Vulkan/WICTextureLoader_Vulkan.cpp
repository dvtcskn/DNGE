//--------------------------------------------------------------------------------------
// File: WICTextureLoader12.cpp
//
// Function for loading a WIC image and creating a Direct3D runtime texture for it
// (auto-generating mipmaps if possible)
//
// Note: Assumes application has already called CoInitializeEx
//
// Note these functions are useful for images created as simple 2D textures. For
// more complex resources, DDSTextureLoader is an excellent light-weight runtime loader.
// For a full-featured DDS file reader, writer, and texture processing pipeline see
// the 'Texconv' sample and the 'DirectXTex' library.
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkId=248926
// http://go.microsoft.com/fwlink/?LinkID=615561
//--------------------------------------------------------------------------------------

// We could load multi-frame images (TIFF/GIF) into a texture array.
// For now, we just load the first frame (note: DirectXTex supports multi-frame images)

#include "pch.h"
#include "WICTextureLoader_Vulkan.h"
#include "VulkanFormat.h"
#include "VulkanException.h"
#include "GI/D3DShared/D3DShared.h"

#include <algorithm>
#include <cassert>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <iterator>
#include <new>
#include <tuple>

#include <wincodec.h>

#include <wrl\client.h>

#ifdef _MSC_VER
// Off by default warnings
#pragma warning(disable : 4619 4616 4061 4062 4623 4626 5027)
// C4619/4616 #pragma warning warnings
// C4061 enumerator 'x' in switch of enum 'y' is not explicitly handled by a case label
// C4062 enumerator 'x' in switch of enum 'y' is not handled
// C4623 default constructor was implicitly defined as deleted
// C4626 assignment operator was implicitly defined as deleted
// C5027 move assignment operator was implicitly defined as deleted
#endif

#ifdef __clang__
#pragma clang diagnostic ignored "-Wtautological-type-limit-compare"
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#pragma clang diagnostic ignored "-Wswitch"
#pragma clang diagnostic ignored "-Wswitch-enum"
#pragma clang diagnostic ignored "-Wunused-macros"
#endif

#define VK_NO_STATE_OBJECT_HELPERS
#define VK_NO_CHECK_FEATURE_SUPPORT_CLASS

#define	VK_REQ_TEXTURE2D_U_OR_V_DIMENSION	( 16384 )

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace
{
    //-------------------------------------------------------------------------------------
    // WIC Pixel Format Translation Data
    //-------------------------------------------------------------------------------------
    struct WICTranslate
    {
        const GUID&         wic;
        DXGI_FORMAT         format;

        constexpr WICTranslate(const GUID& wg, DXGI_FORMAT fmt) noexcept :
            wic(wg),
            format(fmt) {}
    };

    constexpr WICTranslate g_WICFormats[] =
    {
        { GUID_WICPixelFormat128bppRGBAFloat,       DXGI_FORMAT_R32G32B32A32_FLOAT },

        { GUID_WICPixelFormat64bppRGBAHalf,         DXGI_FORMAT_R16G16B16A16_FLOAT },
        { GUID_WICPixelFormat64bppRGBA,             DXGI_FORMAT_R16G16B16A16_UNORM },

        { GUID_WICPixelFormat32bppRGBA,             DXGI_FORMAT_R8G8B8A8_UNORM },
        { GUID_WICPixelFormat32bppBGRA,             DXGI_FORMAT_B8G8R8A8_UNORM },
        { GUID_WICPixelFormat32bppBGR,              DXGI_FORMAT_B8G8R8X8_UNORM },

        { GUID_WICPixelFormat32bppRGBA1010102XR,    DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM },
        { GUID_WICPixelFormat32bppRGBA1010102,      DXGI_FORMAT_R10G10B10A2_UNORM },

        { GUID_WICPixelFormat16bppBGRA5551,         DXGI_FORMAT_B5G5R5A1_UNORM },
        { GUID_WICPixelFormat16bppBGR565,           DXGI_FORMAT_B5G6R5_UNORM },

        { GUID_WICPixelFormat32bppGrayFloat,        DXGI_FORMAT_R32_FLOAT },
        { GUID_WICPixelFormat16bppGrayHalf,         DXGI_FORMAT_R16_FLOAT },
        { GUID_WICPixelFormat16bppGray,             DXGI_FORMAT_R16_UNORM },
        { GUID_WICPixelFormat8bppGray,              DXGI_FORMAT_R8_UNORM },

        { GUID_WICPixelFormat8bppAlpha,             DXGI_FORMAT_A8_UNORM },

        { GUID_WICPixelFormat96bppRGBFloat,         DXGI_FORMAT_R32G32B32_FLOAT },
    };

    //-------------------------------------------------------------------------------------
    // WIC Pixel Format nearest conversion table
    //-------------------------------------------------------------------------------------
    struct WICConvert
    {
        const GUID& source;
        const GUID& target;

        constexpr WICConvert(const GUID& src, const GUID& tgt) noexcept :
            source(src),
            target(tgt) {}
    };

    constexpr WICConvert g_WICConvert[] =
    {
        // Note target GUID in this conversion table must be one of those directly supported formats (above).

        { GUID_WICPixelFormatBlackWhite,            GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM

        { GUID_WICPixelFormat1bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
        { GUID_WICPixelFormat2bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
        { GUID_WICPixelFormat4bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
        { GUID_WICPixelFormat8bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM

        { GUID_WICPixelFormat2bppGray,              GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM
        { GUID_WICPixelFormat4bppGray,              GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM

        { GUID_WICPixelFormat16bppGrayFixedPoint,   GUID_WICPixelFormat16bppGrayHalf }, // DXGI_FORMAT_R16_FLOAT
        { GUID_WICPixelFormat32bppGrayFixedPoint,   GUID_WICPixelFormat32bppGrayFloat }, // DXGI_FORMAT_R32_FLOAT

        { GUID_WICPixelFormat16bppBGR555,           GUID_WICPixelFormat16bppBGRA5551 }, // DXGI_FORMAT_B5G5R5A1_UNORM

        { GUID_WICPixelFormat32bppBGR101010,        GUID_WICPixelFormat32bppRGBA1010102 }, // DXGI_FORMAT_R10G10B10A2_UNORM

        { GUID_WICPixelFormat24bppBGR,              GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
        { GUID_WICPixelFormat24bppRGB,              GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
        { GUID_WICPixelFormat32bppPBGRA,            GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
        { GUID_WICPixelFormat32bppPRGBA,            GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM

        { GUID_WICPixelFormat48bppRGB,              GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
        { GUID_WICPixelFormat48bppBGR,              GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
        { GUID_WICPixelFormat64bppBGRA,             GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
        { GUID_WICPixelFormat64bppPRGBA,            GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
        { GUID_WICPixelFormat64bppPBGRA,            GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM

        { GUID_WICPixelFormat48bppRGBFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
        { GUID_WICPixelFormat48bppBGRFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
        { GUID_WICPixelFormat64bppRGBAFixedPoint,   GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
        { GUID_WICPixelFormat64bppBGRAFixedPoint,   GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
        { GUID_WICPixelFormat64bppRGBFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
        { GUID_WICPixelFormat64bppRGBHalf,          GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
        { GUID_WICPixelFormat48bppRGBHalf,          GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT

        { GUID_WICPixelFormat128bppPRGBAFloat,      GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT
        { GUID_WICPixelFormat128bppRGBFloat,        GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT
        { GUID_WICPixelFormat128bppRGBAFixedPoint,  GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT
        { GUID_WICPixelFormat128bppRGBFixedPoint,   GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT
        { GUID_WICPixelFormat32bppRGBE,             GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT

        { GUID_WICPixelFormat32bppCMYK,             GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
        { GUID_WICPixelFormat64bppCMYK,             GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
        { GUID_WICPixelFormat40bppCMYKAlpha,        GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
        { GUID_WICPixelFormat80bppCMYKAlpha,        GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM

        { GUID_WICPixelFormat32bppRGB,              GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
        { GUID_WICPixelFormat64bppRGB,              GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
        { GUID_WICPixelFormat64bppPRGBAHalf,        GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT

        { GUID_WICPixelFormat96bppRGBFixedPoint,   GUID_WICPixelFormat96bppRGBFloat }, // DXGI_FORMAT_R32G32B32_FLOAT

        // We don't support n-channel formats
    };

    BOOL WINAPI InitializeWICFactory(PINIT_ONCE, PVOID, PVOID* ifactory) noexcept
    {
        return SUCCEEDED(CoCreateInstance(
            CLSID_WICImagingFactory2,
            nullptr,
            CLSCTX_INPROC_SERVER,
            __uuidof(IWICImagingFactory2),
            ifactory)) ? TRUE : FALSE;
    }

    IWICImagingFactory2* GetWIC() noexcept
    {
        static INIT_ONCE s_initOnce = INIT_ONCE_STATIC_INIT;

        IWICImagingFactory2* factory = nullptr;
        if (!InitOnceExecuteOnce(
            &s_initOnce,
            InitializeWICFactory,
            nullptr,
            reinterpret_cast<LPVOID*>(&factory)))
        {
            return nullptr;
        }

        return factory;
    }

    //---------------------------------------------------------------------------------
    inline uint32_t CountMips(uint32_t width, uint32_t height) noexcept
    {
        if (width == 0 || height == 0)
            return 0;

        uint32_t count = 1;
        while (width > 1 || height > 1)
        {
            width >>= 1;
            height >>= 1;
            count++;
        }
        return count;
    }

    //--------------------------------------------------------------------------------------
    DXGI_FORMAT MakeSRGB(_In_ DXGI_FORMAT format) noexcept
    {
        switch (format)
        {
        case DXGI_FORMAT_R8G8B8A8_UNORM:
            return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

        case DXGI_FORMAT_BC1_UNORM:
            return DXGI_FORMAT_BC1_UNORM_SRGB;

        case DXGI_FORMAT_BC2_UNORM:
            return DXGI_FORMAT_BC2_UNORM_SRGB;

        case DXGI_FORMAT_BC3_UNORM:
            return DXGI_FORMAT_BC3_UNORM_SRGB;

        case DXGI_FORMAT_B8G8R8A8_UNORM:
            return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

        case DXGI_FORMAT_B8G8R8X8_UNORM:
            return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;

        case DXGI_FORMAT_BC7_UNORM:
            return DXGI_FORMAT_BC7_UNORM_SRGB;

        default:
            return format;
        }
    }

    //---------------------------------------------------------------------------------
    DXGI_FORMAT WICToDXGI(const GUID& guid) noexcept
    {
        for (size_t i = 0; i < std::size(g_WICFormats); ++i)
        {
            if (memcmp(&g_WICFormats[i].wic, &guid, sizeof(GUID)) == 0)
                return g_WICFormats[i].format;
        }

        return DXGI_FORMAT_UNKNOWN;
    }

    //---------------------------------------------------------------------------------
    size_t WICBitsPerPixel(REFGUID targetGuid) noexcept
    {
        auto pWIC = GetWIC();
        if (!pWIC)
            return 0;

        ComPtr<IWICComponentInfo> cinfo;
        if (FAILED(pWIC->CreateComponentInfo(targetGuid, cinfo.GetAddressOf())))
            return 0;

        WICComponentType type;
        if (FAILED(cinfo->GetComponentType(&type)))
            return 0;

        if (type != WICPixelFormat)
            return 0;

        ComPtr<IWICPixelFormatInfo> pfinfo;
        if (FAILED(cinfo.As(&pfinfo)))
            return 0;

        UINT bpp;
        if (FAILED(pfinfo->GetBitsPerPixel(&bpp)))
            return 0;

        return bpp;
    }

    //---------------------------------------------------------------------------------
    void FitPowerOf2(UINT origx, UINT origy, UINT& targetx, UINT& targety, size_t maxsize)
    {
        const float origAR = float(origx) / float(origy);

        if (origx > origy)
        {
            size_t x;
            for (x = maxsize; x > 1; x >>= 1) { if (x <= targetx) break; }
            targetx = UINT(x);

            float bestScore = FLT_MAX;
            for (size_t y = maxsize; y > 0; y >>= 1)
            {
                const float score = fabsf((float(x) / float(y)) - origAR);
                if (score < bestScore)
                {
                    bestScore = score;
                    targety = UINT(y);
                }
            }
        }
        else
        {
            size_t y;
            for (y = maxsize; y > 1; y >>= 1) { if (y <= targety) break; }
            targety = UINT(y);

            float bestScore = FLT_MAX;
            for (size_t x = maxsize; x > 0; x >>= 1)
            {
                const float score = fabsf((float(x) / float(y)) - origAR);
                if (score < bestScore)
                {
                    bestScore = score;
                    targetx = UINT(x);
                }
            }
        }
    }

    HRESULT CreateTextureResource(
        _In_ VulkanDevice* Device,
        size_t width,
        size_t height,
        size_t depth,
        size_t mipCount,
        size_t arraySize,
        DXGI_FORMAT format,
        _Outptr_ VkImage* texture,
        _Outptr_ VkDeviceMemory& Memory,
        _Outptr_ sTextureDesc& TextureDesc) noexcept
    {
        if (!Device)
            return E_POINTER;

        HRESULT hr = E_FAIL;

        VkImageType ImageType = VkImageType::VK_IMAGE_TYPE_2D;

        VkFormat VulkanFormat = ConvertFormat_Format_To_VkFormat(ConvertFormat_DXGI_To_Format(format));

        TextureDesc.ArraySize = arraySize;
        TextureDesc.Dimensions.X = width;
        TextureDesc.Dimensions.Y = height;
        TextureDesc.Format = ConvertFormat_DXGI_To_Format(format);
        TextureDesc.MipLevels = mipCount;

        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = ImageType;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = depth;
        imageInfo.mipLevels = mipCount;
        imageInfo.arrayLayers = arraySize;
        imageInfo.format = VulkanFormat;
        imageInfo.tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;

        vkCreateImage(Device->Get(), &imageInfo, nullptr, texture);

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(Device->Get(), *texture, &memRequirements);

        // Allocate memory for the buffer
        VkMemoryAllocateInfo alloc_info{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memRequirements.size,
            .memoryTypeIndex = Device->GetMemoryType(memRequirements.memoryTypeBits,	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) };

        VK_CHECK(vkAllocateMemory(Device->Get(), &alloc_info, nullptr, &Memory));

        vkBindImageMemory(Device->Get(), *texture, Memory, 0);

        hr = S_OK;
        return hr;
    }

    //---------------------------------------------------------------------------------
    HRESULT CreateTextureFromWIC(_In_ VulkanDevice* Device,
        _In_ IWICBitmapFrameDecode *frame,
        size_t maxsize,
        unsigned int loadFlags,
        _Outptr_ VkImage* texture,
        _Outptr_ VkDeviceMemory& Memory,
        _Outptr_ sTextureDesc& TextureDesc,
        std::unique_ptr<uint8_t[]>& decodedData,
        TextureSubresource& subresource) noexcept
    {
        UINT width, height;
        HRESULT hr = frame->GetSize(&width, &height);
        if (FAILED(hr))
            return hr;

        assert(width > 0 && height > 0);

        if (maxsize > UINT32_MAX)
            return E_INVALIDARG;

        if (!maxsize)
        {
            maxsize = size_t(VK_REQ_TEXTURE2D_U_OR_V_DIMENSION);
        }

        UINT twidth = width;
        UINT theight = height;
        if (loadFlags & Vulkan::WIC_LOADER_FIT_POW2)
        {
            FitPowerOf2(width, height, twidth, theight, maxsize);
        }
        else if (width > maxsize || height > maxsize)
        {
            const float ar = static_cast<float>(height) / static_cast<float>(width);
            if (width > height)
            {
                twidth = static_cast<UINT>(maxsize);
                theight = std::max<UINT>(1, static_cast<UINT>(static_cast<float>(maxsize) * ar));
            }
            else
            {
                theight = static_cast<UINT>(maxsize);
                twidth = std::max<UINT>(1, static_cast<UINT>(static_cast<float>(maxsize) / ar));
            }
            assert(twidth <= maxsize && theight <= maxsize);
        }

        if (loadFlags & Vulkan::WIC_LOADER_MAKE_SQUARE)
        {
            twidth = std::max<UINT>(twidth, theight);
            theight = twidth;
        }

        // Determine format
        WICPixelFormatGUID pixelFormat;
        hr = frame->GetPixelFormat(&pixelFormat);
        if (FAILED(hr))
            return hr;

        WICPixelFormatGUID convertGUID;
        memcpy_s(&convertGUID, sizeof(WICPixelFormatGUID), &pixelFormat, sizeof(GUID));

        size_t bpp = 0;

        DXGI_FORMAT format = WICToDXGI(pixelFormat);
        if (format == DXGI_FORMAT_UNKNOWN)
        {
            for (size_t i = 0; i < std::size(g_WICConvert); ++i)
            {
                if (memcmp(&g_WICConvert[i].source, &pixelFormat, sizeof(WICPixelFormatGUID)) == 0)
                {
                    memcpy_s(&convertGUID, sizeof(WICPixelFormatGUID), &g_WICConvert[i].target, sizeof(GUID));

                    format = WICToDXGI(g_WICConvert[i].target);
                    assert(format != DXGI_FORMAT_UNKNOWN);
                    bpp = WICBitsPerPixel(convertGUID);
                    break;
                }
            }

            if (format == DXGI_FORMAT_UNKNOWN)
                return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
        }
        else
        {
            bpp = WICBitsPerPixel(pixelFormat);
        }

        if (loadFlags & Vulkan::WIC_LOADER_FORCE_RGBA32)
        {
            memcpy_s(&convertGUID, sizeof(WICPixelFormatGUID), &GUID_WICPixelFormat32bppRGBA, sizeof(GUID));
            format = DXGI_FORMAT_R8G8B8A8_UNORM;
            bpp = 32;
        }

        if (!bpp)
            return E_FAIL;

        // Handle sRGB formats
        if (loadFlags & Vulkan::WIC_LOADER_FORCE_SRGB)
        {
            format = MakeSRGB(format);
        }
        else if (!(loadFlags & Vulkan::WIC_LOADER_IGNORE_SRGB))
        {
            ComPtr<IWICMetadataQueryReader> metareader;
            if (SUCCEEDED(frame->GetMetadataQueryReader(metareader.GetAddressOf())))
            {
                GUID containerFormat;
                if (SUCCEEDED(metareader->GetContainerFormat(&containerFormat)))
                {
                    bool sRGB = false;

                    PROPVARIANT value;
                    PropVariantInit(&value);

                    // Check for colorspace chunks
                    if (memcmp(&containerFormat, &GUID_ContainerFormatPng, sizeof(GUID)) == 0)
                    {
                        if (SUCCEEDED(metareader->GetMetadataByName(L"/sRGB/RenderingIntent", &value)) && value.vt == VT_UI1)
                        {
                            sRGB = true;
                        }
                        else if (SUCCEEDED(metareader->GetMetadataByName(L"/gAMA/ImageGamma", &value)) && value.vt == VT_UI4)
                        {
                            sRGB = (value.uintVal == 45455);
                        }
                        else
                        {
                            sRGB = (loadFlags & Vulkan::WIC_LOADER_SRGB_DEFAULT) != 0;
                        }
                    }
                    else if (SUCCEEDED(metareader->GetMetadataByName(L"System.Image.ColorSpace", &value)) && value.vt == VT_UI2)
                    {
                        sRGB = (value.uiVal == 1);
                    }
                    else
                    {
                        sRGB = (loadFlags & Vulkan::WIC_LOADER_SRGB_DEFAULT) != 0;
                    }

                    std::ignore = PropVariantClear(&value);

                    if (sRGB)
                        format = MakeSRGB(format);
                }
            }
        }

        // Allocate memory for decoded image
        const uint64_t rowBytes = (uint64_t(twidth) * uint64_t(bpp) + 7u) / 8u;
        const uint64_t numBytes = rowBytes * uint64_t(theight);

        if (rowBytes > UINT32_MAX || numBytes > UINT32_MAX)
            return HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);

        auto const rowPitch = static_cast<size_t>(rowBytes);
        auto const imageSize = static_cast<size_t>(numBytes);

        decodedData.reset(new (std::nothrow) uint8_t[imageSize]);
        if (!decodedData)
            return E_OUTOFMEMORY;

        // Load image data
        if (memcmp(&convertGUID, &pixelFormat, sizeof(GUID)) == 0
            && twidth == width
            && theight == height)
        {
            // No format conversion or resize needed
            hr = frame->CopyPixels(nullptr, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), decodedData.get());
            if (FAILED(hr))
                return hr;
        }
        else if (twidth != width || theight != height)
        {
            // Resize
            auto pWIC = GetWIC();
            if (!pWIC)
                return E_NOINTERFACE;

            ComPtr<IWICBitmapScaler> scaler;
            hr = pWIC->CreateBitmapScaler(scaler.GetAddressOf());
            if (FAILED(hr))
                return hr;

            hr = scaler->Initialize(frame, twidth, theight, WICBitmapInterpolationModeFant);
            if (FAILED(hr))
                return hr;

            WICPixelFormatGUID pfScaler;
            hr = scaler->GetPixelFormat(&pfScaler);
            if (FAILED(hr))
                return hr;

            if (memcmp(&convertGUID, &pfScaler, sizeof(GUID)) == 0)
            {
                // No format conversion needed
                hr = scaler->CopyPixels(nullptr, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), decodedData.get());
                if (FAILED(hr))
                    return hr;
            }
            else
            {
                ComPtr<IWICFormatConverter> FC;
                hr = pWIC->CreateFormatConverter(FC.GetAddressOf());
                if (FAILED(hr))
                    return hr;

                BOOL canConvert = FALSE;
                hr = FC->CanConvert(pfScaler, convertGUID, &canConvert);
                if (FAILED(hr) || !canConvert)
                {
                    return E_UNEXPECTED;
                }

                hr = FC->Initialize(scaler.Get(), convertGUID, WICBitmapDitherTypeErrorDiffusion, nullptr, 0, WICBitmapPaletteTypeMedianCut);
                if (FAILED(hr))
                    return hr;

                hr = FC->CopyPixels(nullptr, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), decodedData.get());
                if (FAILED(hr))
                    return hr;
            }
        }
        else
        {
            // Format conversion but no resize
            auto pWIC = GetWIC();
            if (!pWIC)
                return E_NOINTERFACE;

            ComPtr<IWICFormatConverter> FC;
            hr = pWIC->CreateFormatConverter(FC.GetAddressOf());
            if (FAILED(hr))
                return hr;

            BOOL canConvert = FALSE;
            hr = FC->CanConvert(pixelFormat, convertGUID, &canConvert);
            if (FAILED(hr) || !canConvert)
            {
                return E_UNEXPECTED;
            }

            hr = FC->Initialize(frame, convertGUID, WICBitmapDitherTypeErrorDiffusion, nullptr, 0, WICBitmapPaletteTypeMedianCut);
            if (FAILED(hr))
                return hr;

            hr = FC->CopyPixels(nullptr, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), decodedData.get());
            if (FAILED(hr))
                return hr;
        }

        // Count the number of mips
        const uint32_t mipCount = (loadFlags & (Vulkan::WIC_LOADER_MIP_AUTOGEN | Vulkan::WIC_LOADER_MIP_RESERVE))
            ? CountMips(twidth, theight) : 1u;

        hr = CreateTextureResource(Device, twidth, theight, 1, static_cast<UINT16>(mipCount), 1, format, texture, Memory, TextureDesc );

        if (FAILED(hr))
        {
            return hr;
        }

        subresource.pData = decodedData.get();
        subresource.RowPitch = static_cast<LONG>(rowPitch);
        subresource.SlicePitch = static_cast<LONG>(imageSize);

        return hr;
    }
} // anonymous namespace


//--------------------------------------------------------------------------------------
_Use_decl_annotations_
HRESULT Vulkan::LoadWICTextureFromMemory(
    _In_ VulkanDevice* Device,
    _In_reads_bytes_(wicDataSize) const uint8_t* wicData,
    size_t wicDataSize,
    _Outptr_ VkImage* texture,
    _Outptr_ VkDeviceMemory& Memory,
    _Outptr_ sTextureDesc& TextureDesc,
    std::unique_ptr<uint8_t[]>& decodedData,
    TextureSubresource& subresource,
    size_t maxsize) noexcept
{
    return LoadWICTextureFromMemoryEx(
        Device,
        wicData,
        wicDataSize,
        maxsize,
        WIC_LOADER_DEFAULT,
        texture,
        Memory,
        TextureDesc,
        decodedData,
        subresource);
}


//--------------------------------------------------------------------------------------
_Use_decl_annotations_
HRESULT Vulkan::LoadWICTextureFromMemoryEx(
    _In_ VulkanDevice* Device,
    _In_reads_bytes_(wicDataSize) const uint8_t* wicData,
    size_t wicDataSize,
    size_t maxsize,
    WIC_LOADER_FLAGS loadFlags,
    _Outptr_ VkImage* texture,
    _Outptr_ VkDeviceMemory& Memory,
    _Outptr_ sTextureDesc& TextureDesc,
    std::unique_ptr<uint8_t[]>& decodedData,
    TextureSubresource& subresource) noexcept
{
    if (texture)
    {
        *texture = nullptr;
    }

    if (!Device || !wicData || !texture)
        return E_INVALIDARG;

    if (!wicDataSize)
        return E_FAIL;

    if (wicDataSize > UINT32_MAX)
        return HRESULT_FROM_WIN32(ERROR_FILE_TOO_LARGE);

    auto pWIC = GetWIC();
    if (!pWIC)
        return E_NOINTERFACE;

    // Create input stream for memory
    ComPtr<IWICStream> stream;
    HRESULT hr = pWIC->CreateStream(stream.GetAddressOf());
    if (FAILED(hr))
        return hr;

    hr = stream->InitializeFromMemory(const_cast<uint8_t*>(wicData), static_cast<DWORD>(wicDataSize));
    if (FAILED(hr))
        return hr;

    // Initialize WIC
    ComPtr<IWICBitmapDecoder> decoder;
    hr = pWIC->CreateDecoderFromStream(stream.Get(), nullptr, WICDecodeMetadataCacheOnDemand, decoder.GetAddressOf());
    if (FAILED(hr))
        return hr;

    ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, frame.GetAddressOf());
    if (FAILED(hr))
        return hr;

    hr = CreateTextureFromWIC(Device,
        frame.Get(), maxsize,
        loadFlags,
        texture, Memory, TextureDesc, decodedData, subresource);
    if (FAILED(hr))
        return hr;

    _Analysis_assume_(*texture != nullptr);

#if _DEBUG
    //Device->SetResourceName(VkObjectType::VK_OBJECT_TYPE_IMAGE, reinterpret_cast<std::uint64_t>(*texture), "WICTextureLoader::Memory");
#endif
    return hr;
}


//--------------------------------------------------------------------------------------
_Use_decl_annotations_
HRESULT Vulkan::LoadWICTextureFromFile(
    _In_ VulkanDevice* Device,
    _In_z_ const wchar_t* szFileName,
    _Outptr_ VkImage* texture,
    _Outptr_ VkDeviceMemory& Memory,
    _Outptr_ sTextureDesc& TextureDesc,
    std::unique_ptr<uint8_t[]>& decodedData,
    TextureSubresource& subresource,
    size_t maxsize) noexcept
{
    return LoadWICTextureFromFileEx(
        Device,
        szFileName,
        maxsize,
        WIC_LOADER_DEFAULT,
        texture,
        Memory,
        TextureDesc,
        decodedData,
        subresource);
}


//--------------------------------------------------------------------------------------
_Use_decl_annotations_
HRESULT Vulkan::LoadWICTextureFromFileEx(
        _In_ VulkanDevice* Device,
        _In_z_ const wchar_t* szFileName,
        size_t maxsize,
        WIC_LOADER_FLAGS loadFlags,
        _Outptr_ VkImage* texture,
        _Outptr_ VkDeviceMemory& Memory,
        _Outptr_ sTextureDesc& TextureDesc,
        std::unique_ptr<uint8_t[]>& decodedData,
        TextureSubresource& subresource) noexcept
{
    if (texture)
    {
        *texture = nullptr;
    }

    if (!Device || !szFileName || !texture)
        return E_INVALIDARG;

    auto pWIC = GetWIC();
    if (!pWIC)
        return E_NOINTERFACE;

    // Initialize WIC
    ComPtr<IWICBitmapDecoder> decoder;
    HRESULT hr = pWIC->CreateDecoderFromFilename(szFileName,
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnDemand,
        decoder.GetAddressOf());
    if (FAILED(hr))
        return hr;

    ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, frame.GetAddressOf());
    if (FAILED(hr))
        return hr;

    hr = CreateTextureFromWIC(Device, frame.Get(), maxsize,
        loadFlags,
        texture, Memory, TextureDesc,  decodedData, subresource);

    if (SUCCEEDED(hr))
    {
#if _DEBUG
        //Device->SetResourceName(VkObjectType::VK_OBJECT_TYPE_IMAGE, reinterpret_cast<std::uint64_t>(*texture), "WICTextureLoader");
#endif
    }

    return hr;
}
