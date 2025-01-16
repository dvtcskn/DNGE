//--------------------------------------------------------------------------------------
// File: DDSTextureLoader12.cpp
//
// Functions for loading a DDS texture and creating a Direct3D runtime resource for it
//
// Note these functions are useful as a light-weight runtime loader for DDS files. For
// a full-featured DDS file reader, writer, and texture processing pipeline see
// the 'Texconv' sample and the 'DirectXTex' library.
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkId=248926
// http://go.microsoft.com/fwlink/?LinkID=615561
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "DDSTextureLoader_Vulkan.h"
#include "VulkanFormat.h"
#include "VulkanException.h"
#include "GI/D3DShared/D3DShared.h"

#include <algorithm>
#include <cassert>
#include <memory>
#include <new>

#ifndef _WIN32
#include <fstream>
#include <filesystem>
#endif

#include <dxgi.h>

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

using namespace Vulkan;

//--------------------------------------------------------------------------------------
// Macros
//--------------------------------------------------------------------------------------
#ifndef VK_MAKEFOURCC
#define VK_MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) |       \
                ((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24 ))
#endif /* defined(VK_MAKEFOURCC) */

// HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW)
#define VK_HRESULT_E_ARITHMETIC_OVERFLOW static_cast<HRESULT>(0x80070216L)

// HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED)
#define VK_HRESULT_E_NOT_SUPPORTED static_cast<HRESULT>(0x80070032L)

// HRESULT_FROM_WIN32(ERROR_HANDLE_EOF)
#define VK_HRESULT_E_HANDLE_EOF static_cast<HRESULT>(0x80070026L)

// HRESULT_FROM_WIN32(ERROR_INVALID_DATA)
#define VK_HRESULT_E_INVALID_DATA static_cast<HRESULT>(0x8007000DL)

//--------------------------------------------------------------------------------------
// DDS file structure definitions
//
// See DDS.h in the 'Texconv' sample and the 'DirectXTex' library
//--------------------------------------------------------------------------------------
#pragma pack(push,1)

constexpr uint32_t VK_DDS_MAGIC = 0x20534444; // "DDS "

struct VK_DDS_PIXELFORMAT
{
    uint32_t    size;
    uint32_t    flags;
    uint32_t    fourCC;
    uint32_t    RGBBitCount;
    uint32_t    RBitMask;
    uint32_t    GBitMask;
    uint32_t    BBitMask;
    uint32_t    ABitMask;
};

#define VK_DDS_FOURCC      0x00000004  // DDPF_FOURCC
#define VK_DDS_RGB         0x00000040  // DDPF_RGB
#define VK_DDS_LUMINANCE   0x00020000  // DDPF_LUMINANCE
#define VK_DDS_ALPHA       0x00000002  // DDPF_ALPHA
#define VK_DDS_BUMPDUDV    0x00080000  // DDPF_BUMPDUDV

#define VK_DDS_HEADER_FLAGS_VOLUME         0x00800000  // DDSD_DEPTH

#define VK_DDS_HEIGHT 0x00000002 // DDSD_HEIGHT

#define VK_DDS_CUBEMAP_POSITIVEX 0x00000600 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEX
#define VK_DDS_CUBEMAP_NEGATIVEX 0x00000a00 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEX
#define VK_DDS_CUBEMAP_POSITIVEY 0x00001200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEY
#define VK_DDS_CUBEMAP_NEGATIVEY 0x00002200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEY
#define VK_DDS_CUBEMAP_POSITIVEZ 0x00004200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEZ
#define VK_DDS_CUBEMAP_NEGATIVEZ 0x00008200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEZ

#define VK_DDS_CUBEMAP_ALLFACES ( VK_DDS_CUBEMAP_POSITIVEX | VK_DDS_CUBEMAP_NEGATIVEX |\
                               VK_DDS_CUBEMAP_POSITIVEY | VK_DDS_CUBEMAP_NEGATIVEY |\
                               VK_DDS_CUBEMAP_POSITIVEZ | VK_DDS_CUBEMAP_NEGATIVEZ )

#define VK_DDS_CUBEMAP 0x00000200 // DDSCAPS2_CUBEMAP

#define	VK_REQ_MIP_LEVELS	( 15 )
#define	VK_REQ_TEXTURE1D_U_DIMENSION	( 16384 )
#define	VK_REQ_TEXTURE1D_ARRAY_AXIS_DIMENSION	( 2048 )
#define	VK_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION	( 2048 )
#define	VK_REQ_TEXTURECUBE_DIMENSION	( 16384 )
#define	VK_REQ_TEXTURE2D_U_OR_V_DIMENSION	( 16384 )
#define	VK_D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION	( 2048 )
#define	VK_REQ_SUBRESOURCES	( 30720 )

enum VK_DDS_MISC_FLAGS2
{
    DDS_MISC_FLAGS2_ALPHA_MODE_MASK = 0x7L,
};

struct VK_DDS_HEADER
{
    uint32_t        size;
    uint32_t        flags;
    uint32_t        height;
    uint32_t        width;
    uint32_t        pitchOrLinearSize;
    uint32_t        depth; // only if VK_DDS_HEADER_FLAGS_VOLUME is set in flags
    uint32_t        mipMapCount;
    uint32_t        reserved1[11];
    VK_DDS_PIXELFORMAT ddspf;
    uint32_t        caps;
    uint32_t        caps2;
    uint32_t        caps3;
    uint32_t        caps4;
    uint32_t        reserved2;
};

struct VK_DDS_HEADER_DXT10
{
    DXGI_FORMAT     dxgiFormat;
    uint32_t        resourceDimension;
    uint32_t        miscFlag; // see D3D11_RESOURCE_MISC_FLAG
    uint32_t        arraySize;
    uint32_t        miscFlags2;
};

enum VK_RESOURCE_DIMENSION : std::uint32_t
{
    RESOURCE_DIMENSION_UNKNOWN = 0,
    RESOURCE_DIMENSION_BUFFER = 1,
    RESOURCE_DIMENSION_TEXTURE1D = 2,
    RESOURCE_DIMENSION_TEXTURE2D = 3,
    RESOURCE_DIMENSION_TEXTURE3D = 4
};

#pragma pack(pop)

//--------------------------------------------------------------------------------------
namespace
{
#ifdef _WIN32
    struct handle_closer { void operator()(HANDLE h) noexcept { if (h) CloseHandle(h); } };

    using ScopedHandle = std::unique_ptr<void, handle_closer>;

    inline HANDLE safe_handle(HANDLE h) noexcept { return (h == INVALID_HANDLE_VALUE) ? nullptr : h; }
#endif

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
    HRESULT LoadTextureDataFromMemory(
        _In_reads_(ddsDataSize) const uint8_t* ddsData,
        size_t ddsDataSize,
        const VK_DDS_HEADER** header,
        const uint8_t** bitData,
        size_t* bitSize) noexcept
    {
        if (!header || !bitData || !bitSize)
        {
            return E_POINTER;
        }

        *bitSize = 0;

        if (ddsDataSize > UINT32_MAX)
        {
            return E_FAIL;
        }

        if (ddsDataSize < (sizeof(uint32_t) + sizeof(VK_DDS_HEADER)))
        {
            return E_FAIL;
        }

        // DDS files always start with the same magic number ("DDS ")
        auto const dwMagicNumber = *reinterpret_cast<const uint32_t*>(ddsData);
        if (dwMagicNumber != VK_DDS_MAGIC)
        {
            return E_FAIL;
        }

        auto hdr = reinterpret_cast<const VK_DDS_HEADER*>(ddsData + sizeof(uint32_t));

        // Verify header to validate DDS file
        if (hdr->size != sizeof(VK_DDS_HEADER) ||
            hdr->ddspf.size != sizeof(VK_DDS_PIXELFORMAT))
        {
            return E_FAIL;
        }

        // Check for DX10 extension
        bool bDXT10Header = false;
        if ((hdr->ddspf.flags & VK_DDS_FOURCC) &&
            (VK_MAKEFOURCC('D', 'X', '1', '0') == hdr->ddspf.fourCC))
        {
            // Must be long enough for both headers and magic value
            if (ddsDataSize < (sizeof(uint32_t) + sizeof(VK_DDS_HEADER) + sizeof(VK_DDS_HEADER_DXT10)))
            {
                return E_FAIL;
            }

            bDXT10Header = true;
        }

        // setup the pointers in the process request
        *header = hdr;
        auto offset = sizeof(uint32_t)
            + sizeof(VK_DDS_HEADER)
            + (bDXT10Header ? sizeof(VK_DDS_HEADER_DXT10) : 0u);
        *bitData = ddsData + offset;
        *bitSize = ddsDataSize - offset;

        return S_OK;
    }


    //--------------------------------------------------------------------------------------
    HRESULT LoadTextureDataFromFile(
        _In_z_ const wchar_t* fileName,
        std::unique_ptr<uint8_t[]>& ddsData,
        const VK_DDS_HEADER** header,
        const uint8_t** bitData,
        size_t* bitSize) noexcept
    {
        if (!header || !bitData || !bitSize)
        {
            return E_POINTER;
        }

        *bitSize = 0;

    #ifdef _WIN32
            // open the file
        ScopedHandle hFile(safe_handle(CreateFile2(
            fileName,
            GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING,
            nullptr)));

        if (!hFile)
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }

        // Get the file size
        FILE_STANDARD_INFO fileInfo;
        if (!GetFileInformationByHandleEx(hFile.get(), FileStandardInfo, &fileInfo, sizeof(fileInfo)))
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }

        // File is too big for 32-bit allocation, so reject read
        if (fileInfo.EndOfFile.HighPart > 0)
        {
            return E_FAIL;
        }

        // Need at least enough data to fill the header and magic number to be a valid DDS
        if (fileInfo.EndOfFile.LowPart < (sizeof(uint32_t) + sizeof(VK_DDS_HEADER)))
        {
            return E_FAIL;
        }

        // create enough space for the file data
        ddsData.reset(new (std::nothrow) uint8_t[fileInfo.EndOfFile.LowPart]);
        if (!ddsData)
        {
            return E_OUTOFMEMORY;
        }

        // read the data in
        DWORD bytesRead = 0;
        if (!ReadFile(hFile.get(),
            ddsData.get(),
            fileInfo.EndOfFile.LowPart,
            &bytesRead,
            nullptr
        ))
        {
            ddsData.reset();
            return HRESULT_FROM_WIN32(GetLastError());
        }

        if (bytesRead < fileInfo.EndOfFile.LowPart)
        {
            ddsData.reset();
            return E_FAIL;
        }

        size_t len = fileInfo.EndOfFile.LowPart;

    #else // !WIN32
        std::ifstream inFile(std::filesystem::path(fileName), std::ios::in | std::ios::binary | std::ios::ate);
        if (!inFile)
            return E_FAIL;

        std::streampos fileLen = inFile.tellg();
        if (!inFile)
            return E_FAIL;

        // Need at least enough data to fill the header and magic number to be a valid DDS
        if (fileLen < (sizeof(uint32_t) + sizeof(VK_DDS_HEADER)))
            return E_FAIL;

        ddsData.reset(new (std::nothrow) uint8_t[size_t(fileLen)]);
        if (!ddsData)
            return E_OUTOFMEMORY;

        inFile.seekg(0, std::ios::beg);
        if (!inFile)
        {
            ddsData.reset();
            return E_FAIL;
        }

        inFile.read(reinterpret_cast<char*>(ddsData.get()), fileLen);
        if (!inFile)
        {
            ddsData.reset();
            return E_FAIL;
        }

        inFile.close();

        size_t len = fileLen;
    #endif

        // DDS files always start with the same magic number ("DDS ")
        auto const dwMagicNumber = *reinterpret_cast<const uint32_t*>(ddsData.get());
        if (dwMagicNumber != VK_DDS_MAGIC)
        {
            ddsData.reset();
            return E_FAIL;
        }

        auto hdr = reinterpret_cast<const VK_DDS_HEADER*>(ddsData.get() + sizeof(uint32_t));
        hdr->size;
        // Verify header to validate DDS file
        if (hdr->size != sizeof(VK_DDS_HEADER) ||
            hdr->ddspf.size != sizeof(VK_DDS_PIXELFORMAT))
        {
            ddsData.reset();
            return E_FAIL;
        }

        // Check for DX10 extension
        bool bDXT10Header = false;
        if ((hdr->ddspf.flags & VK_DDS_FOURCC) &&
            (VK_MAKEFOURCC('D', 'X', '1', '0') == hdr->ddspf.fourCC))
        {
            // Must be long enough for both headers and magic value
            if (len < (sizeof(uint32_t) + sizeof(VK_DDS_HEADER) + sizeof(VK_DDS_HEADER_DXT10)))
            {
                ddsData.reset();
                return E_FAIL;
            }

            bDXT10Header = true;
        }

        // setup the pointers in the process request
        *header = hdr;
        auto offset = sizeof(uint32_t) + sizeof(VK_DDS_HEADER)
            + (bDXT10Header ? sizeof(VK_DDS_HEADER_DXT10) : 0u);
        *bitData = ddsData.get() + offset;
        *bitSize = len - offset;

        return S_OK;
    }


    //--------------------------------------------------------------------------------------
    // Return the BPP for a particular format
    //--------------------------------------------------------------------------------------
    size_t BitsPerPixel(_In_ DXGI_FORMAT fmt) noexcept
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
        case DXGI_FORMAT_V408:
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
        case DXGI_FORMAT_P208:
        case DXGI_FORMAT_V208:
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

        default:
            return 0;
        }
    }


    //--------------------------------------------------------------------------------------
    // Get surface information for a particular format
    //--------------------------------------------------------------------------------------
    HRESULT GetSurfaceInfo(
        _In_ size_t width,
        _In_ size_t height,
        _In_ DXGI_FORMAT fmt,
        size_t* outNumBytes,
        _Out_opt_ size_t* outRowBytes,
        _Out_opt_ size_t* outNumRows) noexcept
    {
        uint64_t numBytes = 0;
        uint64_t rowBytes = 0;
        uint64_t numRows = 0;

        bool bc = false;
        bool packed = false;
        bool planar = false;
        size_t bpe = 0;
        switch (fmt)
        {
        case DXGI_FORMAT_BC1_TYPELESS:
        case DXGI_FORMAT_BC1_UNORM:
        case DXGI_FORMAT_BC1_UNORM_SRGB:
        case DXGI_FORMAT_BC4_TYPELESS:
        case DXGI_FORMAT_BC4_UNORM:
        case DXGI_FORMAT_BC4_SNORM:
            bc = true;
            bpe = 8;
            break;

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
            bc = true;
            bpe = 16;
            break;

        case DXGI_FORMAT_R8G8_B8G8_UNORM:
        case DXGI_FORMAT_G8R8_G8B8_UNORM:
        case DXGI_FORMAT_YUY2:
            packed = true;
            bpe = 4;
            break;

        case DXGI_FORMAT_Y210:
        case DXGI_FORMAT_Y216:
            packed = true;
            bpe = 8;
            break;

        case DXGI_FORMAT_NV12:
        case DXGI_FORMAT_420_OPAQUE:
            if ((height % 2) != 0)
            {
                // Requires a height alignment of 2.
                return E_INVALIDARG;
            }
            planar = true;
            bpe = 2;
            break;

        case DXGI_FORMAT_P208:
            planar = true;
            bpe = 2;
            break;

        case DXGI_FORMAT_P010:
        case DXGI_FORMAT_P016:
            if ((height % 2) != 0)
            {
                // Requires a height alignment of 2.
                return E_INVALIDARG;
            }
            planar = true;
            bpe = 4;
            break;

        default:
            break;
        }

        if (bc)
        {
            uint64_t numBlocksWide = 0;
            if (width > 0)
            {
                numBlocksWide = std::max<uint64_t>(1u, (uint64_t(width) + 3u) / 4u);
            }
            uint64_t numBlocksHigh = 0;
            if (height > 0)
            {
                numBlocksHigh = std::max<uint64_t>(1u, (uint64_t(height) + 3u) / 4u);
            }
            rowBytes = numBlocksWide * bpe;
            numRows = numBlocksHigh;
            numBytes = rowBytes * numBlocksHigh;
        }
        else if (packed)
        {
            rowBytes = ((uint64_t(width) + 1u) >> 1) * bpe;
            numRows = uint64_t(height);
            numBytes = rowBytes * height;
        }
        else if (fmt == DXGI_FORMAT_NV11)
        {
            rowBytes = ((uint64_t(width) + 3u) >> 2) * 4u;
            numRows = uint64_t(height) * 2u; // Direct3D makes this simplifying assumption, although it is larger than the 4:1:1 data
            numBytes = rowBytes * numRows;
        }
        else if (planar)
        {
            rowBytes = ((uint64_t(width) + 1u) >> 1) * bpe;
            numBytes = (rowBytes * uint64_t(height)) + ((rowBytes * uint64_t(height) + 1u) >> 1);
            numRows = height + ((uint64_t(height) + 1u) >> 1);
        }
        else
        {
            const size_t bpp = BitsPerPixel(fmt);
            if (!bpp)
                return E_INVALIDARG;

            rowBytes = (uint64_t(width) * bpp + 7u) / 8u; // round up to nearest byte
            numRows = uint64_t(height);
            numBytes = rowBytes * height;
        }

    #if defined(_M_IX86) || defined(_M_ARM) || defined(_M_HYBRID_X86_ARM64)
        static_assert(sizeof(size_t) == 4, "Not a 32-bit platform!");
        if (numBytes > UINT32_MAX || rowBytes > UINT32_MAX || numRows > UINT32_MAX)
            return VK_HRESULT_E_ARITHMETIC_OVERFLOW;
    #else
        static_assert(sizeof(size_t) == 8, "Not a 64-bit platform!");
    #endif

        if (outNumBytes)
        {
            *outNumBytes = static_cast<size_t>(numBytes);
        }
        if (outRowBytes)
        {
            *outRowBytes = static_cast<size_t>(rowBytes);
        }
        if (outNumRows)
        {
            *outNumRows = static_cast<size_t>(numRows);
        }

        return S_OK;
    }


    //--------------------------------------------------------------------------------------
#define ISBITMASK( r,g,b,a ) ( ddpf.RBitMask == r && ddpf.GBitMask == g && ddpf.BBitMask == b && ddpf.ABitMask == a )

    DXGI_FORMAT GetDXGIFormat(const VK_DDS_PIXELFORMAT& ddpf) noexcept
    {
        if (ddpf.flags & VK_DDS_RGB)
        {
            // Note that sRGB formats are written using the "DX10" extended header

            switch (ddpf.RGBBitCount)
            {
            case 32:
                if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
                {
                    return DXGI_FORMAT_R8G8B8A8_UNORM;
                }

                if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000))
                {
                    return DXGI_FORMAT_B8G8R8A8_UNORM;
                }

                if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0))
                {
                    return DXGI_FORMAT_B8G8R8X8_UNORM;
                }

                // No DXGI format maps to ISBITMASK(0x000000ff,0x0000ff00,0x00ff0000,0) aka D3DFMT_X8B8G8R8

                // Note that many common DDS reader/writers (including D3DX) swap the
                // the RED/BLUE masks for 10:10:10:2 formats. We assume
                // below that the 'backwards' header mask is being used since it is most
                // likely written by D3DX. The more robust solution is to use the 'DX10'
                // header extension and specify the DXGI_FORMAT_R10G10B10A2_UNORM format directly

                // For 'correct' writers, this should be 0x000003ff,0x000ffc00,0x3ff00000 for RGB data
                if (ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000))
                {
                    return DXGI_FORMAT_R10G10B10A2_UNORM;
                }

                // No DXGI format maps to ISBITMASK(0x000003ff,0x000ffc00,0x3ff00000,0xc0000000) aka D3DFMT_A2R10G10B10

                if (ISBITMASK(0x0000ffff, 0xffff0000, 0, 0))
                {
                    return DXGI_FORMAT_R16G16_UNORM;
                }

                if (ISBITMASK(0xffffffff, 0, 0, 0))
                {
                    // Only 32-bit color channel format in D3D9 was R32F
                    return DXGI_FORMAT_R32_FLOAT; // D3DX writes this out as a FourCC of 114
                }
                break;

            case 24:
                // No 24bpp DXGI formats aka D3DFMT_R8G8B8
                break;

            case 16:
                if (ISBITMASK(0x7c00, 0x03e0, 0x001f, 0x8000))
                {
                    return DXGI_FORMAT_B5G5R5A1_UNORM;
                }
                if (ISBITMASK(0xf800, 0x07e0, 0x001f, 0))
                {
                    return DXGI_FORMAT_B5G6R5_UNORM;
                }

                // No DXGI format maps to ISBITMASK(0x7c00,0x03e0,0x001f,0) aka D3DFMT_X1R5G5B5

                if (ISBITMASK(0x0f00, 0x00f0, 0x000f, 0xf000))
                {
                    return DXGI_FORMAT_B4G4R4A4_UNORM;
                }

                // NVTT versions 1.x wrote this as RGB instead of LUMINANCE
                if (ISBITMASK(0x00ff, 0, 0, 0xff00))
                {
                    return DXGI_FORMAT_R8G8_UNORM;
                }
                if (ISBITMASK(0xffff, 0, 0, 0))
                {
                    return DXGI_FORMAT_R16_UNORM;
                }

                // No DXGI format maps to ISBITMASK(0x0f00,0x00f0,0x000f,0) aka D3DFMT_X4R4G4B4

                // No 3:3:2:8 or paletted DXGI formats aka D3DFMT_A8R3G3B2, D3DFMT_A8P8, etc.
                break;

            case 8:
                // NVTT versions 1.x wrote this as RGB instead of LUMINANCE
                if (ISBITMASK(0xff, 0, 0, 0))
                {
                    return DXGI_FORMAT_R8_UNORM;
                }

                // No 3:3:2 or paletted DXGI formats aka D3DFMT_R3G3B2, D3DFMT_P8
                break;

            default:
                return DXGI_FORMAT_UNKNOWN;
            }
        }
        else if (ddpf.flags & VK_DDS_LUMINANCE)
        {
            switch (ddpf.RGBBitCount)
            {
            case 16:
                if (ISBITMASK(0xffff, 0, 0, 0))
                {
                    return DXGI_FORMAT_R16_UNORM; // D3DX10/11 writes this out as DX10 extension
                }
                if (ISBITMASK(0x00ff, 0, 0, 0xff00))
                {
                    return DXGI_FORMAT_R8G8_UNORM; // D3DX10/11 writes this out as DX10 extension
                }
                break;

            case 8:
                if (ISBITMASK(0xff, 0, 0, 0))
                {
                    return DXGI_FORMAT_R8_UNORM; // D3DX10/11 writes this out as DX10 extension
                }

                // No DXGI format maps to ISBITMASK(0x0f,0,0,0xf0) aka D3DFMT_A4L4

                if (ISBITMASK(0x00ff, 0, 0, 0xff00))
                {
                    return DXGI_FORMAT_R8G8_UNORM; // Some DDS writers assume the bitcount should be 8 instead of 16
                }
                break;

            default:
                return DXGI_FORMAT_UNKNOWN;
            }
        }
        else if (ddpf.flags & VK_DDS_ALPHA)
        {
            if (8 == ddpf.RGBBitCount)
            {
                return DXGI_FORMAT_A8_UNORM;
            }
        }
        else if (ddpf.flags & VK_DDS_BUMPDUDV)
        {
            switch (ddpf.RGBBitCount)
            {
            case 32:
                if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
                {
                    return DXGI_FORMAT_R8G8B8A8_SNORM; // D3DX10/11 writes this out as DX10 extension
                }
                if (ISBITMASK(0x0000ffff, 0xffff0000, 0, 0))
                {
                    return DXGI_FORMAT_R16G16_SNORM; // D3DX10/11 writes this out as DX10 extension
                }

                // No DXGI format maps to ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000) aka D3DFMT_A2W10V10U10
                break;

            case 16:
                if (ISBITMASK(0x00ff, 0xff00, 0, 0))
                {
                    return DXGI_FORMAT_R8G8_SNORM; // D3DX10/11 writes this out as DX10 extension
                }
                break;

            default:
                return DXGI_FORMAT_UNKNOWN;
            }

            // No DXGI format maps to DDPF_BUMPLUMINANCE aka D3DFMT_L6V5U5, D3DFMT_X8L8V8U8
        }
        else if (ddpf.flags & VK_DDS_FOURCC)
        {
            if (VK_MAKEFOURCC('D', 'X', 'T', '1') == ddpf.fourCC)
            {
                return DXGI_FORMAT_BC1_UNORM;
            }
            if (VK_MAKEFOURCC('D', 'X', 'T', '3') == ddpf.fourCC)
            {
                return DXGI_FORMAT_BC2_UNORM;
            }
            if (VK_MAKEFOURCC('D', 'X', 'T', '5') == ddpf.fourCC)
            {
                return DXGI_FORMAT_BC3_UNORM;
            }

            // While pre-multiplied alpha isn't directly supported by the DXGI formats,
            // they are basically the same as these BC formats so they can be mapped
            if (VK_MAKEFOURCC('D', 'X', 'T', '2') == ddpf.fourCC)
            {
                return DXGI_FORMAT_BC2_UNORM;
            }
            if (VK_MAKEFOURCC('D', 'X', 'T', '4') == ddpf.fourCC)
            {
                return DXGI_FORMAT_BC3_UNORM;
            }

            if (VK_MAKEFOURCC('A', 'T', 'I', '1') == ddpf.fourCC)
            {
                return DXGI_FORMAT_BC4_UNORM;
            }
            if (VK_MAKEFOURCC('B', 'C', '4', 'U') == ddpf.fourCC)
            {
                return DXGI_FORMAT_BC4_UNORM;
            }
            if (VK_MAKEFOURCC('B', 'C', '4', 'S') == ddpf.fourCC)
            {
                return DXGI_FORMAT_BC4_SNORM;
            }

            if (VK_MAKEFOURCC('A', 'T', 'I', '2') == ddpf.fourCC)
            {
                return DXGI_FORMAT_BC5_UNORM;
            }
            if (VK_MAKEFOURCC('B', 'C', '5', 'U') == ddpf.fourCC)
            {
                return DXGI_FORMAT_BC5_UNORM;
            }
            if (VK_MAKEFOURCC('B', 'C', '5', 'S') == ddpf.fourCC)
            {
                return DXGI_FORMAT_BC5_SNORM;
            }

            // BC6H and BC7 are written using the "DX10" extended header

            if (VK_MAKEFOURCC('R', 'G', 'B', 'G') == ddpf.fourCC)
            {
                return DXGI_FORMAT_R8G8_B8G8_UNORM;
            }
            if (VK_MAKEFOURCC('G', 'R', 'G', 'B') == ddpf.fourCC)
            {
                return DXGI_FORMAT_G8R8_G8B8_UNORM;
            }

            if (VK_MAKEFOURCC('Y', 'U', 'Y', '2') == ddpf.fourCC)
            {
                return DXGI_FORMAT_YUY2;
            }

            // Check for D3DFORMAT enums being set here
            switch (ddpf.fourCC)
            {
            case 36: // D3DFMT_A16B16G16R16
                return DXGI_FORMAT_R16G16B16A16_UNORM;

            case 110: // D3DFMT_Q16W16V16U16
                return DXGI_FORMAT_R16G16B16A16_SNORM;

            case 111: // D3DFMT_R16F
                return DXGI_FORMAT_R16_FLOAT;

            case 112: // D3DFMT_G16R16F
                return DXGI_FORMAT_R16G16_FLOAT;

            case 113: // D3DFMT_A16B16G16R16F
                return DXGI_FORMAT_R16G16B16A16_FLOAT;

            case 114: // D3DFMT_R32F
                return DXGI_FORMAT_R32_FLOAT;

            case 115: // D3DFMT_G32R32F
                return DXGI_FORMAT_R32G32_FLOAT;

            case 116: // D3DFMT_A32B32G32R32F
                return DXGI_FORMAT_R32G32B32A32_FLOAT;

            // No DXGI format maps to D3DFMT_CxV8U8

            default:
                return DXGI_FORMAT_UNKNOWN;
            }
        }

        return DXGI_FORMAT_UNKNOWN;
    }

#undef ISBITMASK


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


    //--------------------------------------------------------------------------------------
    inline DXGI_FORMAT MakeLinear(_In_ DXGI_FORMAT format) noexcept
    {
        switch (format)
        {
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            return DXGI_FORMAT_R8G8B8A8_UNORM;

        case DXGI_FORMAT_BC1_UNORM_SRGB:
            return DXGI_FORMAT_BC1_UNORM;

        case DXGI_FORMAT_BC2_UNORM_SRGB:
            return DXGI_FORMAT_BC2_UNORM;

        case DXGI_FORMAT_BC3_UNORM_SRGB:
            return DXGI_FORMAT_BC3_UNORM;

        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
            return DXGI_FORMAT_B8G8R8A8_UNORM;

        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
            return DXGI_FORMAT_B8G8R8X8_UNORM;

        case DXGI_FORMAT_BC7_UNORM_SRGB:
            return DXGI_FORMAT_BC7_UNORM;

        default:
            return format;
        }
    }


    //--------------------------------------------------------------------------------------
    inline bool IsDepthStencil(DXGI_FORMAT fmt) noexcept
    {
        switch (fmt)
        {
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
            return true;

        default:
            return false;
        }
    }


    //--------------------------------------------------------------------------------------
    inline void AdjustPlaneResource(
        _In_ DXGI_FORMAT fmt,
        _In_ size_t height,
        _In_ size_t slicePlane,
        _Inout_ TextureSubresource& res) noexcept
    {
        switch (fmt)
        {
        case DXGI_FORMAT_NV12:
        case DXGI_FORMAT_P010:
        case DXGI_FORMAT_P016:
            if (!slicePlane)
            {
                // Plane 0
                res.SlicePitch = res.RowPitch * static_cast<LONG>(height);
            }
            else
            {
                // Plane 1
                res.pData = (void*)(reinterpret_cast<const uint8_t*>(res.pData) + uintptr_t(res.RowPitch) * height);
                res.SlicePitch = res.RowPitch * ((static_cast<LONG>(height) + 1) >> 1);
            }
            break;

        case DXGI_FORMAT_NV11:
            if (!slicePlane)
            {
                // Plane 0
                res.SlicePitch = res.RowPitch * static_cast<LONG>(height);
            }
            else
            {
                // Plane 1
                res.pData = (void*)(reinterpret_cast<const uint8_t*>(res.pData) + uintptr_t(res.RowPitch) * height);
                res.RowPitch = (res.RowPitch >> 1);
                res.SlicePitch = res.RowPitch * static_cast<LONG>(height);
            }
            break;

        default:
            break;
        }
    }


    //--------------------------------------------------------------------------------------
    HRESULT FillInitData(_In_ size_t width,
        _In_ size_t height,
        _In_ size_t depth,
        _In_ size_t mipCount,
        _In_ size_t arraySize,
        _In_ size_t numberOfPlanes,
        _In_ DXGI_FORMAT format,
        _In_ size_t maxsize,
        _In_ size_t bitSize,
        _In_reads_bytes_(bitSize) const uint8_t* bitData,
        _Out_ size_t& twidth,
        _Out_ size_t& theight,
        _Out_ size_t& tdepth,
        _Out_ size_t& skipMip,
        std::vector<TextureSubresource>& initData)
    {
        if (!bitData)
        {
            return E_POINTER;
        }

        skipMip = 0;
        twidth = 0;
        theight = 0;
        tdepth = 0;

        size_t NumBytes = 0;
        size_t RowBytes = 0;
        const uint8_t* pEndBits = bitData + bitSize;

        initData.clear();

        for (size_t p = 0; p < numberOfPlanes; ++p)
        {
            const uint8_t* pSrcBits = bitData;

            for (size_t j = 0; j < arraySize; j++)
            {
                size_t w = width;
                size_t h = height;
                size_t d = depth;
                for (size_t i = 0; i < mipCount; i++)
                {
                    HRESULT hr = GetSurfaceInfo(w, h, format, &NumBytes, &RowBytes, nullptr);
                    if (FAILED(hr))
                        return hr;

                    if (NumBytes > UINT32_MAX || RowBytes > UINT32_MAX)
                        return VK_HRESULT_E_ARITHMETIC_OVERFLOW;

                    if ((mipCount <= 1) || !maxsize || (w <= maxsize && h <= maxsize && d <= maxsize))
                    {
                        if (!twidth)
                        {
                            twidth = w;
                            theight = h;
                            tdepth = d;
                        }

                        TextureSubresource res = TextureSubresource((void*)pSrcBits, RowBytes, NumBytes);

                        AdjustPlaneResource(format, h, p, res);

                        initData.emplace_back(res);
                    }
                    else if (!j)
                    {
                        // Count number of skipped mipmaps (first item only)
                        ++skipMip;
                    }

                    if (pSrcBits + (NumBytes*d) > pEndBits)
                    {
                        return VK_HRESULT_E_HANDLE_EOF;
                    }

                    pSrcBits += NumBytes * d;

                    w = w >> 1;
                    h = h >> 1;
                    d = d >> 1;
                    if (w == 0)
                    {
                        w = 1;
                    }
                    if (h == 0)
                    {
                        h = 1;
                    }
                    if (d == 0)
                    {
                        d = 1;
                    }
                }
            }
        }

        return initData.empty() ? E_FAIL : S_OK;
    }


    //--------------------------------------------------------------------------------------
    HRESULT CreateTextureResource(
        _In_ VulkanDevice* Device,
        VK_RESOURCE_DIMENSION resDim,
        size_t width,
        size_t height,
        size_t depth,
        size_t mipCount,
        size_t arraySize,
        DXGI_FORMAT format,
        DDS_LOADER_FLAGS loadFlags,
        _Outptr_ VkImage* texture, 
        _Outptr_ VkDeviceMemory& Memory,
        _Outptr_ sTextureDesc& TextureDesc) noexcept
    {
        if (!Device)
            return E_POINTER;

        HRESULT hr = E_FAIL;

        if (loadFlags & DDS_LOADER_FORCE_SRGB)
        {
            format = MakeSRGB(format);
        }
        else if (loadFlags & DDS_LOADER_IGNORE_SRGB)
        {
            format = MakeLinear(format);
        }

        VkImageType ImageType;
        switch (resDim)
        {
        case RESOURCE_DIMENSION_UNKNOWN:
        case RESOURCE_DIMENSION_BUFFER:
            ImageType = VkImageType::VK_IMAGE_TYPE_MAX_ENUM;
            break;
        case RESOURCE_DIMENSION_TEXTURE1D:
            ImageType = VkImageType::VK_IMAGE_TYPE_1D;
            break;
        case RESOURCE_DIMENSION_TEXTURE2D:
            ImageType = VkImageType::VK_IMAGE_TYPE_2D;
            break;
        case RESOURCE_DIMENSION_TEXTURE3D:
            ImageType = VkImageType::VK_IMAGE_TYPE_3D;
            break;
        default:
            ImageType = VkImageType::VK_IMAGE_TYPE_MAX_ENUM;
            break;
        }

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

    //--------------------------------------------------------------------------------------
    HRESULT CreateTextureFromDDS(_In_ VulkanDevice* Device,
        _In_ const VK_DDS_HEADER* header,
        _In_reads_bytes_(bitSize) const uint8_t* bitData,
        size_t bitSize,
        size_t maxsize,
        DDS_LOADER_FLAGS loadFlags,
        _Outptr_ VkImage* texture,
        _Outptr_ VkDeviceMemory& Memory,
        _Outptr_ sTextureDesc& TextureDesc,
        std::vector<TextureSubresource>& subresources,
        _Out_opt_ bool* outIsCubeMap) noexcept(false)
    {
        HRESULT hr = S_OK;

        const UINT width = header->width;
        UINT height = header->height;
        UINT depth = header->depth;

        VK_RESOURCE_DIMENSION resDim = VK_RESOURCE_DIMENSION::RESOURCE_DIMENSION_UNKNOWN;
        UINT arraySize = 1;
        DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
        bool isCubeMap = false;

        size_t mipCount = header->mipMapCount;
        if (0 == mipCount)
        {
            mipCount = 1;
        }

        if ((header->ddspf.flags & VK_DDS_FOURCC) &&
            (VK_MAKEFOURCC('D', 'X', '1', '0') == header->ddspf.fourCC))
        {
            auto d3d10ext = reinterpret_cast<const VK_DDS_HEADER_DXT10*>(reinterpret_cast<const char*>(header) + sizeof(VK_DDS_HEADER));

            arraySize = d3d10ext->arraySize;
            if (arraySize == 0)
            {
                return VK_HRESULT_E_INVALID_DATA;
            }

            switch (d3d10ext->dxgiFormat)
            {
            case DXGI_FORMAT_NV12:
            case DXGI_FORMAT_P010:
            case DXGI_FORMAT_P016:
            case DXGI_FORMAT_420_OPAQUE:
                if ((d3d10ext->resourceDimension != VK_RESOURCE_DIMENSION::RESOURCE_DIMENSION_TEXTURE2D)
                    || (width % 2) != 0 || (height % 2) != 0)
                {
                    return VK_HRESULT_E_NOT_SUPPORTED;
                }
                break;

            case DXGI_FORMAT_YUY2:
            case DXGI_FORMAT_Y210:
            case DXGI_FORMAT_Y216:
            case DXGI_FORMAT_P208:
                if ((width % 2) != 0)
                {
                    return VK_HRESULT_E_NOT_SUPPORTED;
                }
                break;

            case DXGI_FORMAT_NV11:
                if ((width % 4) != 0)
                {
                    return VK_HRESULT_E_NOT_SUPPORTED;
                }
                break;

            case DXGI_FORMAT_AI44:
            case DXGI_FORMAT_IA44:
            case DXGI_FORMAT_P8:
            case DXGI_FORMAT_A8P8:
                return VK_HRESULT_E_NOT_SUPPORTED;

            case DXGI_FORMAT_V208:
                if ((d3d10ext->resourceDimension != VK_RESOURCE_DIMENSION::RESOURCE_DIMENSION_TEXTURE2D)
                    || (height % 2) != 0)
                {
                    return VK_HRESULT_E_NOT_SUPPORTED;
                }
                break;

            default:
                if (BitsPerPixel(d3d10ext->dxgiFormat) == 0)
                {
                    return VK_HRESULT_E_NOT_SUPPORTED;
                }
            }

            format = d3d10ext->dxgiFormat;

            switch (d3d10ext->resourceDimension)
            {
            case VK_RESOURCE_DIMENSION::RESOURCE_DIMENSION_TEXTURE1D:
                // D3DX writes 1D textures with a fixed Height of 1
                if ((header->flags & VK_DDS_HEIGHT) && height != 1)
                {
                    return VK_HRESULT_E_INVALID_DATA;
                }
                height = depth = 1;
                break;

            case VK_RESOURCE_DIMENSION::RESOURCE_DIMENSION_TEXTURE2D:
                if (d3d10ext->miscFlag & 0x4 /* RESOURCE_MISC_TEXTURECUBE */)
                {
                    arraySize *= 6;
                    isCubeMap = true;
                }
                depth = 1;
                break;

            case VK_RESOURCE_DIMENSION::RESOURCE_DIMENSION_TEXTURE3D:
                if (!(header->flags & VK_DDS_HEADER_FLAGS_VOLUME))
                {
                    return VK_HRESULT_E_INVALID_DATA;
                }

                if (arraySize > 1)
                {
                    return VK_HRESULT_E_NOT_SUPPORTED;
                }
                break;

            default:
                return VK_HRESULT_E_NOT_SUPPORTED;
            }

            resDim = static_cast<VK_RESOURCE_DIMENSION>(d3d10ext->resourceDimension);
        }
        else
        {
            format = GetDXGIFormat(header->ddspf);

            if (format == DXGI_FORMAT_UNKNOWN)
            {
                return VK_HRESULT_E_NOT_SUPPORTED;
            }

            if (header->flags & VK_DDS_HEADER_FLAGS_VOLUME)
            {
                resDim = VK_RESOURCE_DIMENSION::RESOURCE_DIMENSION_TEXTURE3D;
            }
            else
            {
                if (header->caps2 & VK_DDS_CUBEMAP)
                {
                    // We require all six faces to be defined
                    if ((header->caps2 & VK_DDS_CUBEMAP_ALLFACES) != VK_DDS_CUBEMAP_ALLFACES)
                    {
                        return VK_HRESULT_E_NOT_SUPPORTED;
                    }

                    arraySize = 6;
                    isCubeMap = true;
                }

                depth = 1;
                resDim = VK_RESOURCE_DIMENSION::RESOURCE_DIMENSION_TEXTURE2D;

                // Note there's no way for a legacy Direct3D 9 DDS to express a '1D' texture
            }

            assert(BitsPerPixel(format) != 0);
        }

        // Bound sizes (for security purposes we don't trust DDS file metadata larger than the Direct3D hardware requirements)
        if (mipCount > VK_REQ_MIP_LEVELS)
        {
            return VK_HRESULT_E_NOT_SUPPORTED;
        }

        switch (resDim)
        {
        case VK_RESOURCE_DIMENSION::RESOURCE_DIMENSION_TEXTURE1D:
            if ((arraySize > VK_REQ_TEXTURE1D_ARRAY_AXIS_DIMENSION) ||
                (width > VK_REQ_TEXTURE1D_U_DIMENSION))
            {
                return VK_HRESULT_E_NOT_SUPPORTED;
            }
            break;

        case VK_RESOURCE_DIMENSION::RESOURCE_DIMENSION_TEXTURE2D:
            if (isCubeMap)
            {
                // This is the right bound because we set arraySize to (NumCubes*6) above
                if ((arraySize > VK_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION) ||
                    (width > VK_REQ_TEXTURECUBE_DIMENSION) ||
                    (height > VK_REQ_TEXTURECUBE_DIMENSION))
                {
                    return VK_HRESULT_E_NOT_SUPPORTED;
                }
            }
            else if ((arraySize > VK_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION) ||
                (width > VK_REQ_TEXTURE2D_U_OR_V_DIMENSION) ||
                (height > VK_REQ_TEXTURE2D_U_OR_V_DIMENSION))
            {
                return VK_HRESULT_E_NOT_SUPPORTED;
            }
            break;

        case VK_RESOURCE_DIMENSION::RESOURCE_DIMENSION_TEXTURE3D:
            if ((arraySize > 1) ||
                (width > VK_D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION) ||
                (height > VK_D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION) ||
                (depth > VK_D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION))
            {
                return VK_HRESULT_E_NOT_SUPPORTED;
            }
            break;

        default:
            return VK_HRESULT_E_NOT_SUPPORTED;
        }

        const UINT numberOfPlanes = 1;// D3D12GetFormatPlaneCount(d3dDevice, format);
        if (!numberOfPlanes)
            return E_INVALIDARG;

        if ((numberOfPlanes > 1) && IsDepthStencil(format))
        {
            // DirectX 12 uses planes for stencil, DirectX 11 does not
            return VK_HRESULT_E_NOT_SUPPORTED;
        }

        if (outIsCubeMap != nullptr)
        {
            *outIsCubeMap = isCubeMap;
        }

        // Create the texture
        size_t numberOfResources = (resDim == VK_RESOURCE_DIMENSION::RESOURCE_DIMENSION_TEXTURE3D)
            ? 1 : arraySize;
        numberOfResources *= mipCount;
        numberOfResources *= numberOfPlanes;

        if (numberOfResources > VK_REQ_SUBRESOURCES)
            return E_INVALIDARG;

        subresources.reserve(numberOfResources);

        size_t skipMip = 0;
        size_t twidth = 0;
        size_t theight = 0;
        size_t tdepth = 0;
        hr = FillInitData(width, height, depth, mipCount, arraySize,
            numberOfPlanes, format,
            maxsize, bitSize, bitData,
            twidth, theight, tdepth, skipMip, subresources);

        if (SUCCEEDED(hr))
        {
            size_t reservedMips = mipCount;
            if (loadFlags & DDS_LOADER_MIP_RESERVE)
            {
                reservedMips = std::min<size_t>(VK_REQ_MIP_LEVELS,
                    CountMips(width, height));
            }

            hr = CreateTextureResource(Device, resDim, twidth, theight, tdepth, reservedMips - skipMip, arraySize,
                format, /*resFlags,*/ loadFlags, texture, Memory, TextureDesc);

            if (FAILED(hr) && !maxsize && (mipCount > 1))
            {
                subresources.clear();

                maxsize = static_cast<size_t>(
                    (resDim == VK_RESOURCE_DIMENSION::RESOURCE_DIMENSION_TEXTURE3D)
                    ? VK_D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION
                    : VK_REQ_TEXTURE2D_U_OR_V_DIMENSION);

                hr = FillInitData(width, height, depth, mipCount, arraySize,
                    numberOfPlanes, format,
                    maxsize, bitSize, bitData,
                    twidth, theight, tdepth, skipMip, subresources);
                if (SUCCEEDED(hr))
                {
                    hr = CreateTextureResource(Device, resDim, twidth, theight, tdepth, mipCount - skipMip, arraySize,
                        format, /*resFlags,*/ loadFlags, texture, Memory, TextureDesc);
                }
            }
        }

        if (FAILED(hr))
        {
            subresources.clear();
        }

        return hr;
    }

    //--------------------------------------------------------------------------------------
    DDS_ALPHA_MODE GetAlphaMode(_In_ const VK_DDS_HEADER* header) noexcept
    {
        if (header->ddspf.flags & VK_DDS_FOURCC)
        {
            if (VK_MAKEFOURCC('D', 'X', '1', '0') == header->ddspf.fourCC)
            {
                auto d3d10ext = reinterpret_cast<const VK_DDS_HEADER_DXT10*>(reinterpret_cast<const uint8_t*>(header) + sizeof(VK_DDS_HEADER));
                auto const mode = static_cast<DDS_ALPHA_MODE>(d3d10ext->miscFlags2 & DDS_MISC_FLAGS2_ALPHA_MODE_MASK);
                switch (mode)
                {
                case DDS_ALPHA_MODE_STRAIGHT:
                case DDS_ALPHA_MODE_PREMULTIPLIED:
                case DDS_ALPHA_MODE_OPAQUE:
                case DDS_ALPHA_MODE_CUSTOM:
                    return mode;

                case DDS_ALPHA_MODE_UNKNOWN:
                default:
                    break;
                }
            }
            else if ((VK_MAKEFOURCC('D', 'X', 'T', '2') == header->ddspf.fourCC)
                || (VK_MAKEFOURCC('D', 'X', 'T', '4') == header->ddspf.fourCC))
            {
                return DDS_ALPHA_MODE_PREMULTIPLIED;
            }
        }

        return DDS_ALPHA_MODE_UNKNOWN;
    }
} // anonymous namespace

//--------------------------------------------------------------------------------------
_Use_decl_annotations_
HRESULT LoadDDSTextureFromMemoryEx(
    VulkanDevice* Device,
    const uint8_t* ddsData,
    size_t ddsDataSize,
    size_t maxsize,
    DDS_LOADER_FLAGS loadFlags,
    VkImage* texture,
    VkDeviceMemory& Memory,
    sTextureDesc& TextureDesc,
    std::vector<TextureSubresource>& subresources,
    DDS_ALPHA_MODE* alphaMode,
    bool* isCubeMap)
{
    if (texture)
    {
        *texture = nullptr;
    }
    if (alphaMode)
    {
        *alphaMode = DDS_ALPHA_MODE_UNKNOWN;
    }
    if (isCubeMap)
    {
        *isCubeMap = false;
    }

    if (!Device || !ddsData || !texture)
    {
        return E_INVALIDARG;
    }

    // Validate DDS file in memory
    const VK_DDS_HEADER* header = nullptr;
    const uint8_t* bitData = nullptr;
    size_t bitSize = 0;

    HRESULT hr = LoadTextureDataFromMemory(ddsData,
        ddsDataSize,
        &header,
        &bitData,
        &bitSize
    );
    if (FAILED(hr))
    {
        return hr;
    }

    hr = CreateTextureFromDDS(Device,
        header, bitData, bitSize, maxsize,
        loadFlags,
        texture, Memory, TextureDesc, subresources, isCubeMap);

    if (SUCCEEDED(hr))
    {
        //SetDebugObjectName(*texture, L"DDSTextureLoader");

        if (alphaMode)
            *alphaMode = GetAlphaMode(header);
    }

    return hr;
}

_Use_decl_annotations_
HRESULT Vulkan::LoadDDSTextureFromMemory(
    VulkanDevice* Device,
    const uint8_t* ddsData,
    size_t ddsDataSize,
    VkImage* texture,
    VkDeviceMemory& Memory,
    sTextureDesc& TextureDesc,
    std::vector<TextureSubresource>& subresources,
    size_t maxsize,
    DDS_ALPHA_MODE* alphaMode,
    bool* isCubeMap)
{
    return LoadDDSTextureFromMemoryEx(
        Device,
        ddsData,
        ddsDataSize,
        maxsize,
        DDS_LOADER_DEFAULT,
        texture, 
        Memory,
        TextureDesc,
        subresources,
        alphaMode,
        isCubeMap);
}

//--------------------------------------------------------------------------------------
_Use_decl_annotations_
HRESULT LoadDDSTextureFromFileEx(
    VulkanDevice* Device,
    const wchar_t* fileName,
    size_t maxsize,
    DDS_LOADER_FLAGS loadFlags,
    VkImage* texture,
    VkDeviceMemory& Memory,
    sTextureDesc& TextureDesc,
    std::unique_ptr<uint8_t[]>& ddsData,
    std::vector<TextureSubresource>& subresources,
    DDS_ALPHA_MODE* alphaMode,
    bool* isCubeMap)
{
    if (texture)
    {
        *texture = nullptr;
    }
    if (alphaMode)
    {
        *alphaMode = DDS_ALPHA_MODE_UNKNOWN;
    }
    if (isCubeMap)
    {
        *isCubeMap = false;
    }

    if (!Device || !fileName || !texture)
    {
        return E_INVALIDARG;
    }

    const VK_DDS_HEADER* header = nullptr;
    const uint8_t* bitData = nullptr;
    size_t bitSize = 0;

    HRESULT hr = LoadTextureDataFromFile(fileName,
        ddsData,
        &header,
        &bitData,
        &bitSize
    );
    if (FAILED(hr))
    {
        return hr;
    }

    hr = CreateTextureFromDDS(Device,
        header, bitData, bitSize, maxsize,
        loadFlags,
        texture, Memory, TextureDesc, subresources, isCubeMap);

    if (SUCCEEDED(hr))
    {
        //SetDebugTextureInfo(fileName, *texture);

        if (alphaMode)
            *alphaMode = GetAlphaMode(header);
    }

    return hr;
}

_Use_decl_annotations_
HRESULT Vulkan::LoadDDSTextureFromFile(
    VulkanDevice* Device,
    const wchar_t* fileName,
    VkImage* texture,
    VkDeviceMemory& Memory,
    sTextureDesc& TextureDesc,
    std::unique_ptr<uint8_t[]>& ddsData,
    std::vector<TextureSubresource>& subresources,
    size_t maxsize,
    DDS_ALPHA_MODE* alphaMode,
    bool* isCubeMap)
{
    return LoadDDSTextureFromFileEx(
        Device,
        fileName,
        maxsize,
        DDS_LOADER_DEFAULT,
        texture,
        Memory,
        TextureDesc,
        ddsData,
        subresources,
        alphaMode,
        isCubeMap);
}
