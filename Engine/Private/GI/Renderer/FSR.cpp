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
#include "FSR.h"
#include "FSRVulkanContext.h"

#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_6.h>
#include "GI/D3D12/D3D12Device.h"
#include "GI/D3D12/D3D12Viewport.h"

#include <dx12/ffx_api_dx12.hpp>
#include <dx12/ffx_api_framegeneration_dx12.hpp>
#include <ffx_dx12.h>
#pragma comment(lib, "amd_fidelityfx_loader_dx12.lib")

#ifndef FSR_DEBUG
#define FSR_DEBUG 1
#endif

#ifndef FSR_Upscale_DEBUG
#define FSR_Upscale_DEBUG 0
#endif

#ifndef FSR_FrameGen_DEBUG
#define FSR_FrameGen_DEBUG 0
#endif

namespace
{
    FfxApiReturnCodes waitCallback(wchar_t* fenceName, uint64_t fenceValueToWaitFor)
    {
        Engine::Print(L"waiting on '%ls' with value %llu", fenceName, fenceValueToWaitFor);
        return FFX_API_RETURN_OK;
    }

    inline FfxApiSurfaceFormat GetFfxSurfaceFormat(EFormat format)
    {
        switch (format)
        {
        //case (EFormat::RGBA32_TYPELESS):
        //    return FFX_API_SURFACE_FORMAT_R32G32B32A32_TYPELESS;
        case (EFormat::RGBA32_UINT):
            return FFX_API_SURFACE_FORMAT_R32G32B32A32_UINT;
        case (EFormat::RGBA32_FLOAT):
            return FFX_API_SURFACE_FORMAT_R32G32B32A32_FLOAT;
        //case (EFormat::RGBA16_TYPELESS):
        //    return FFX_API_SURFACE_FORMAT_R16G16B16A16_TYPELESS;
        case (EFormat::RGBA16_FLOAT):
            return FFX_API_SURFACE_FORMAT_R16G16B16A16_FLOAT;
        case (EFormat::RGB32_FLOAT):
            return FFX_API_SURFACE_FORMAT_R32G32B32_FLOAT;
        //case (EFormat::RG32_TYPELESS):
        //    return FFX_API_SURFACE_FORMAT_R32G32_TYPELESS;
        case (EFormat::RG32_FLOAT):
            return FFX_API_SURFACE_FORMAT_R32G32_FLOAT;
        case (EFormat::R8_UINT):
            return FFX_API_SURFACE_FORMAT_R8_UINT;
        case (EFormat::R32_UINT):
            return FFX_API_SURFACE_FORMAT_R32_UINT;
        //case (EFormat::RGBA8_TYPELESS):
        //    return FFX_API_SURFACE_FORMAT_R8G8B8A8_TYPELESS;
        case (EFormat::RGBA8_UNORM):
            return FFX_API_SURFACE_FORMAT_R8G8B8A8_UNORM;
        //case (EFormat::RGBA8_SNORM):
        //    return FFX_API_SURFACE_FORMAT_R8G8B8A8_SNORM;
        //case (EFormat::RGBA8_SRGB):
        //    return FFX_API_SURFACE_FORMAT_R8G8B8A8_SRGB;
        //case (EFormat::BGRA8_TYPELESS):
        //    return FFX_API_SURFACE_FORMAT_B8G8R8A8_TYPELESS;
        case (EFormat::BGRA8_UNORM):
            return FFX_API_SURFACE_FORMAT_B8G8R8A8_UNORM;
        //case (EFormat::BGRA8_SRGB):
        //    return FFX_API_SURFACE_FORMAT_B8G8R8A8_SRGB;
        //case (EFormat::RG11B10_FLOAT):
        //    return FFX_API_SURFACE_FORMAT_R11G11B10_FLOAT;
        //case (EFormat::RGB9E5_SHAREDEXP):
        //    return FFX_API_SURFACE_FORMAT_R9G9B9E5_SHAREDEXP;
        case (EFormat::RGB10A2_UNORM):
            return FFX_API_SURFACE_FORMAT_R10G10B10A2_UNORM;
        //case (EFormat::RGB10A2_TYPELESS):
        //    return FFX_API_SURFACE_FORMAT_R10G10B10A2_TYPELESS;
        //case (EFormat::RG16_TYPELESS):
        //    return FFX_API_SURFACE_FORMAT_R16G16_TYPELESS;
        case (EFormat::RG16_FLOAT):
            return FFX_API_SURFACE_FORMAT_R16G16_FLOAT;
        case (EFormat::RG16_UINT):
            return FFX_API_SURFACE_FORMAT_R16G16_UINT;
        //case (EFormat::RG16_SINT):
        //    return FFX_API_SURFACE_FORMAT_R16G16_SINT;
        //case (EFormat::R16_TYPELESS):
        //    return FFX_API_SURFACE_FORMAT_R16_TYPELESS;
        case (EFormat::R16_FLOAT):
            return FFX_API_SURFACE_FORMAT_R16_FLOAT;
        case (EFormat::R16_UINT):
            return FFX_API_SURFACE_FORMAT_R16_UINT;
        case (EFormat::R16_UNORM):
            return FFX_API_SURFACE_FORMAT_R16_UNORM;
        //case (EFormat::R16_SNORM):
        //    return FFX_API_SURFACE_FORMAT_R16_SNORM;
        //case (EFormat::R8_TYPELESS):
        //    return FFX_API_SURFACE_FORMAT_R8_TYPELESS;
        case (EFormat::R8_UNORM):
            return FFX_API_SURFACE_FORMAT_R8_UNORM;
        //case (EFormat::R8_SNORM):
        //    return FFX_API_SURFACE_FORMAT_R8_SNORM;
        //case EFormat::RG8_TYPELESS:
        //    return FFX_API_SURFACE_FORMAT_R8G8_TYPELESS;
        case EFormat::RG8_UNORM:
            return FFX_API_SURFACE_FORMAT_R8G8_UNORM;
        case EFormat::RG8_UINT:
            return FFX_API_SURFACE_FORMAT_R8G8_UINT;
        case EFormat::R32_Typeless:
            return FFX_API_SURFACE_FORMAT_R32_TYPELESS;
        case EFormat::R32_FLOAT:
        case EFormat::D32_FLOAT:
            return FFX_API_SURFACE_FORMAT_R32_FLOAT;
        case (EFormat::UNKNOWN):
            return FFX_API_SURFACE_FORMAT_UNKNOWN;
        }
		return FFX_API_SURFACE_FORMAT_UNKNOWN;
    }

    static FfxApiBackbufferTransferFunction GetFfxApiBackbufferTransferFunction(const DisplayMode displayMode)
    {
        switch (displayMode)
        {
        case DisplayMode::DISPLAYMODE_LDR:
            return FfxApiBackbufferTransferFunction::FFX_API_BACKBUFFER_TRANSFER_FUNCTION_SRGB;
        case DisplayMode::DISPLAYMODE_HDR10_2084:
            return FfxApiBackbufferTransferFunction::FFX_API_BACKBUFFER_TRANSFER_FUNCTION_PQ;
        case DisplayMode::DISPLAYMODE_HDR10_SCRGB:
            return FfxApiBackbufferTransferFunction::FFX_API_BACKBUFFER_TRANSFER_FUNCTION_SCRGB;
        default:
            Engine::Assert(AssertLevel::ASSERT_CRITICAL, false, L"FFXInterface: Cauldron: Unsupported display mode requested. Please implement.");
        }
        return FfxApiBackbufferTransferFunction();
    }
}

class D3D12FSRContext : public IFSRContext
{
    sClassBody(sClassConstructor, D3D12FSRContext, IFSRContext)
public:
    D3D12FSRContext()
        : Super()
    {}

    virtual ~D3D12FSRContext()
    {}

    virtual void* GetDevice() const override final
    {
        D3D12Device* Device = (D3D12Device*)GPU::GetDevice();
        return Device->Get();
    }

    virtual const wchar_t* GetVersionQueryName(ffxStructType_t createDescType) const override final
    {
        switch (createDescType)
        {
        case FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE:
            return L"Upscale";
        case FFX_API_CREATE_CONTEXT_DESC_TYPE_FRAMEGENERATION:
            return L"FrameGeneration";
        case FFX_API_CREATE_CONTEXT_DESC_TYPE_FRAMEGENERATIONSWAPCHAIN_VERSION_DX12:
            return L"FrameGenerationSwapchainDX12";
        default:
            return L"Unknown";
        }
    }

    virtual void QueryVersionNames(ffxStructType_t createDescType, std::vector<uint64_t>& versionIds, std::vector<std::string>& versionNames) const override final
    {       

        ffxQueryDescGetVersions versionQuery{};
        versionQuery.header.type = FFX_API_QUERY_DESC_TYPE_GET_VERSIONS;
        versionQuery.createDescType = createDescType;
        versionQuery.device = GetDevice();

        uint64_t versionCount = 0;
        versionQuery.outputCount = &versionCount;
        ffxReturnCode_t retCode = ffxQuery(nullptr, &versionQuery.header);
        Engine::Assert(AssertLevel::ASSERT_WARNING, retCode == FFX_API_RETURN_OK, L"ffxQuery(nullptr,GetVersions%sCount) returned %d", GetVersionQueryName(createDescType), retCode);

        versionIds.resize(versionCount);
        std::vector<const char*> queriedNames(versionCount);
        versionQuery.versionIds = versionIds.data();
        versionQuery.versionNames = queriedNames.data();
        retCode = ffxQuery(nullptr, &versionQuery.header);
        Engine::Assert(AssertLevel::ASSERT_WARNING, retCode == FFX_API_RETURN_OK, L"ffxQuery(nullptr,GetVersions%sIdsNames) returned %d", GetVersionQueryName(createDescType), retCode);

        versionNames.assign(queriedNames.begin(), queriedNames.end());
    }

    virtual FfxApiResource GetResource(IRenderTarget* RT, FfxApiResourceState State) override final
    {
        return ffxApiGetResourceDX12(RT ? (ID3D12Resource*)RT->GetNativeTexture() : nullptr, State);
    }

    virtual FfxApiResource GetDepthResource(IDepthTarget* Depth, FfxApiResourceState State) override final
    {
        return ffxApiGetResourceDX12(Depth ? (ID3D12Resource*)Depth->GetNativeTexture() : nullptr, State);
    }

    virtual FfxApiResource GetCurrentBackBuffer(FfxApiResourceState State) override final
    {
        ffx::DispatchDescFrameGeneration dispatchFg{};
        D3D12Device* Device = (D3D12Device*)GPU::GetDevice();
        D3D12Viewport* Viewport = (D3D12Viewport*)Device->GetD3D12Viewport();
        auto pSwapchainBackbuffer = Viewport->GetCurrentSwapchainTexture();
        return ffxApiGetResourceDX12(pSwapchainBackbuffer, State);
    }

    virtual ffx::ReturnCode Query(ffx::QueryDescUpscaleGetResourceRequirements& ResourceRequirements, ffx::CreateContextDescOverrideVersion OverrideVersion) override final
    {
        struct ffxCreateBackendDX12Desc backendDX12Desc = {};
        backendDX12Desc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12;
        backendDX12Desc.device = (ID3D12Device*)GPU::GetInternalDevice();

        return ffx::Query(ResourceRequirements, OverrideVersion, backendDX12Desc);
    }

    virtual ffx::ReturnCode QueryUpscaleMode(ffx::QueryDescUpscaleGetUpscaleRatioFromQualityMode UpscaleRatioFromQualityMode, std::optional<ffx::CreateContextDescOverrideVersion> OverrideVersion = std::nullopt) override final
    {
        struct ffxCreateBackendDX12Desc backendDX12Desc = {};
        backendDX12Desc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12;
        backendDX12Desc.device = (ID3D12Device*)GPU::GetInternalDevice();

        if (OverrideVersion.has_value())
            return ffx::Query(UpscaleRatioFromQualityMode, OverrideVersion.value(), backendDX12Desc);
        return ffx::Query(UpscaleRatioFromQualityMode, backendDX12Desc);
    }

    virtual ffx::ReturnCode CreateUpscalingContext(ffx::Context& UpscalingContext, ffx::CreateContextDescUpscale Desc, ffxCreateContextDescUpscaleVersion Version, std::optional<ffxOverrideVersion> OverrideVersion = std::nullopt) override final
    {
        ffx::CreateBackendDX12Desc backendDesc = {};
        backendDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12;
        backendDesc.device = (ID3D12Device*)GPU::GetInternalDevice();

        if (OverrideVersion.has_value())
        {
            OverrideVersion->header.pNext = &backendDesc.header;
            return ffx::CreateContext(UpscalingContext, nullptr, Desc, backendDesc, Version, OverrideVersion.value());
        }
        return ffx::CreateContext(UpscalingContext, nullptr, Desc, backendDesc, Version);
    }

    virtual ffx::ReturnCode CreateFrameGenContext(ffx::Context& UpscalingContext, ffx::CreateContextDescFrameGeneration Desc, ffx::CreateContextDescFrameGenerationVersion Version, std::optional<ffxOverrideVersion> OverrideVersion = std::nullopt) override final
    {
        ffx::CreateBackendDX12Desc backendDesc = {};
        backendDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12;
        backendDesc.device = (ID3D12Device*)GPU::GetInternalDevice();

        if (OverrideVersion.has_value())
            return ffx::CreateContext(UpscalingContext, nullptr, Desc, backendDesc, Version, OverrideVersion.value());
        return ffx::CreateContext(UpscalingContext, nullptr, Desc, backendDesc, Version);
    }

    virtual ffx::ReturnCode CreateHudlessFrameGenContext(ffx::Context& UpscalingContext, ffx::CreateContextDescFrameGeneration Desc, ffx::CreateContextDescFrameGenerationHudless FgHudless, ffx::CreateContextDescFrameGenerationVersion Version, std::optional<ffxOverrideVersion> OverrideVersion = std::nullopt) override final
    {
        ffx::CreateBackendDX12Desc backendDesc = {};
        backendDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12;
        backendDesc.device = (ID3D12Device*)GPU::GetInternalDevice();

        if (OverrideVersion.has_value())
            return ffx::CreateContext(UpscalingContext, nullptr, Desc, backendDesc, Version, FgHudless, OverrideVersion.value());
        return ffx::CreateContext(UpscalingContext, nullptr, Desc, backendDesc, Version, FgHudless);
    }

    virtual ffx::ReturnCode GetSwapChainGPUMemoryUsage(FfxApiEffectMemoryUsage* MemoryUsage, FfxApiDimensions2D Dimension, uint32_t Format, uint32_t Flags) override final
    {
        ffx::ReturnCode retCode = ffx::ReturnCode::ErrorParameter;
        D3D12Device* Device = (D3D12Device*)GPU::GetDevice();
        D3D12Viewport* Viewport = (D3D12Viewport*)Device->GetD3D12Viewport();
        IDXGISwapChain4* dxgiSwapchain = Viewport->GetSwapChainPtr();
        DXGI_SWAP_CHAIN_DESC1 desc1;
        dxgiSwapchain->GetDesc1(&desc1);
        ffx::QueryFrameGenerationSwapChainGetGPUMemoryUsageDX12V2 swapChainGetGPUMemoryUsageV2{};
        swapChainGetGPUMemoryUsageV2.device = GPU::GetInternalDevice();
        swapChainGetGPUMemoryUsageV2.displaySize = { desc1.Width, desc1.Height };
        swapChainGetGPUMemoryUsageV2.backBufferFormat = ffxApiGetSurfaceFormatDX12(desc1.Format);
        swapChainGetGPUMemoryUsageV2.backBufferCount = desc1.BufferCount;

        swapChainGetGPUMemoryUsageV2.uiResourceSize = Dimension;
        swapChainGetGPUMemoryUsageV2.uiResourceFormat = Format;
        swapChainGetGPUMemoryUsageV2.flags = Flags;
        swapChainGetGPUMemoryUsageV2.gpuMemoryUsageFrameGenerationSwapchain = MemoryUsage;

        return ffx::Query(swapChainGetGPUMemoryUsageV2);
    }

    virtual void CreateSwapChainContext(ffx::Context& SwapChainContext, bool bGetLatencyWaitableObject = false, HANDLE LatencyWaitableObj = 0) override final
    {
        //#if FSR_D3D12
        D3D12Device* Device = (D3D12Device*)GPU::GetDevice();
        D3D12Viewport* Viewport = (D3D12Viewport*)Device->GetD3D12Viewport();
        IDXGISwapChain4* dxgiSwapchain = Viewport->GetSwapChainPtr();
        dxgiSwapchain->AddRef();
        dxgiSwapchain->AddRef();
        //cauldron::GetSwapChain()->GetImpl()->SetDXGISwapChain(nullptr);
        Viewport->SetSwapChain(nullptr);

        ffx::CreateContextDescFrameGenerationSwapChainForHwndDX12 createSwapChainDesc{};
        HWND WindowHandle = nullptr;
        dxgiSwapchain->GetHwnd(&WindowHandle);
        createSwapChainDesc.hwnd = WindowHandle;
        DXGI_SWAP_CHAIN_DESC1 desc1;
        dxgiSwapchain->GetDesc1(&desc1);
        createSwapChainDesc.desc = &desc1;
        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc;
        dxgiSwapchain->GetFullscreenDesc(&fullscreenDesc);
        createSwapChainDesc.fullscreenDesc = &fullscreenDesc;
        dxgiSwapchain->GetParent(IID_PPV_ARGS(&createSwapChainDesc.dxgiFactory));
        createSwapChainDesc.gameQueue = Device->GetGraphicsQueue();

        if (bGetLatencyWaitableObject)
        {
            createSwapChainDesc.desc->Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
        }

        dxgiSwapchain->Release();
        dxgiSwapchain->Release();
        dxgiSwapchain = nullptr;
        createSwapChainDesc.swapchain = &dxgiSwapchain;

        ffx::CreateContextDescFrameGenerationSwapChainVersionDX12 headerVersion = {};
        headerVersion.version = FFX_FRAMEGENERATION_SWAPCHAIN_DX12_VERSION;

        ffx::ReturnCode retCode = ffx::CreateContext(SwapChainContext, nullptr, createSwapChainDesc, headerVersion);
        Engine::Assert(AssertLevel::ASSERT_CRITICAL, retCode == ffx::ReturnCode::Ok, L"Couldn't create the ffxapi fg swapchain (dx12): %d", (uint32_t)retCode);
        createSwapChainDesc.dxgiFactory->Release();

        Viewport->SetSwapChain(dxgiSwapchain);

        if (bGetLatencyWaitableObject)
        {
            LatencyWaitableObj = dxgiSwapchain->GetFrameLatencyWaitableObject();
        }

        // In case the app is handling Alt-Enter manually we need to update the window association after creating a different swapchain
        IDXGIFactory7* factory = nullptr;
        if (SUCCEEDED(dxgiSwapchain->GetParent(IID_PPV_ARGS(&factory))))
        {
            factory->MakeWindowAssociation(WindowHandle, DXGI_MWA_NO_WINDOW_CHANGES);
            factory->Release();
        }

        auto RefCount = dxgiSwapchain->Release();
        //RefCount = dxgiSwapchain->Release();

        // Lets do the same for HDR as well as it will need to be re initialized since swapchain was re created
        Viewport->SetHDRMetadataAndColorspace();
    }

    virtual ffx::ReturnCode GetSwapChainGetGPUMemoryUsage(ffx::Context SwapChainContext, FfxApiEffectMemoryUsage& gpuMemoryUsageFrameGenerationSwapchain) override final
    {
        ffx::QueryFrameGenerationSwapChainGetGPUMemoryUsageDX12 frameGenSwapchainGetGPUMemoryUsage{};
        frameGenSwapchainGetGPUMemoryUsage.gpuMemoryUsageFrameGenerationSwapchain = &gpuMemoryUsageFrameGenerationSwapchain;
        return ffx::Query(SwapChainContext, frameGenSwapchainGetGPUMemoryUsage);
    }

    virtual void ConfigureSwapChain(ffx::Context& SwapChainContext, FfxApiResource uiColor, bool bDoublebufferInSwapchain) override final
    {
        ffx::ConfigureDescFrameGenerationSwapChainRegisterUiResourceDX12 uiConfig{};
        uiConfig.uiResource = uiColor;
        uiConfig.flags = bDoublebufferInSwapchain ? FFX_FRAMEGENERATION_UI_COMPOSITION_FLAG_ENABLE_INTERNAL_UI_DOUBLE_BUFFERING : 0;
        ffx::Configure(SwapChainContext, uiConfig);
    }

    virtual ffx::ReturnCode QuerySwapChainCommandList(ffx::Context& SwapChainContext, void** CommandList) override final
    {
        ffx::QueryDescFrameGenerationSwapChainInterpolationCommandListDX12 queryCmdList{};
        queryCmdList.pOutCommandList = CommandList;
        return ffx::Query(SwapChainContext, queryCmdList);
    }

    virtual ffx::ReturnCode QuerySwapChainInterpolationTexture(ffx::Context& SwapChainContext, FfxApiResource* Texture) override final
    {
        ffx::QueryDescFrameGenerationSwapChainInterpolationTextureDX12 queryFiTexture{};
        queryFiTexture.pOutTexture = Texture;
        return ffx::Query(SwapChainContext, queryFiTexture);
    }
};

sFSR::sFSR(std::size_t Width, std::size_t Height)
	: Super()
    , RenderDimension(sScreenDimension(Width, Height), sScreenDimension(Width, Height), sScreenDimension(Width, Height))
	, GraphicsCommandContext(IGraphicsCommandContext::Create())
    , UpscaleMode(ERendererUpscaleMode::NativeAA)
    , bGenerateReactiveMask(false)
    , FSRCopyCommandContext(IGraphicsCommandContext::Create())
    , FrameGenCommandContext(IGraphicsCommandContext::Create())
    , UIRenderMode(UICompositionMode::PreUiBackbuffer)
{
    if (GPU::GetGIType() == EGITypes::D3D11)
    {
        Engine::WriteToConsole("FSR 3/4 does not support D3D11.");
        return;
    }

    if (GPU::GetGIType() == EGITypes::D3D12)
    {
        FSRContext = D3D12FSRContext::CreateUnique();
    }
    else if (GPU::GetGIType() == EGITypes::Vulkan)
    {
        FSRContext = VulkanFSRContext::CreateUnique();
    }

    Output = IRenderTarget::Create("FSR Output", sFrameBuffer(GPU::GetBackBufferFormat(), EFrameBufferAttachmentType::RT_SRV_UAV), sFBODesc(sFBODesc::sFBODimension((std::uint32_t)Width, (std::uint32_t)Height)));
    if (bGenerateReactiveMask)
        GeneratedReactiveMask = IRenderTarget::Create("FSR GeneratedReactiveMask", sFrameBuffer(GPU::GetBackBufferFormat(), EFrameBufferAttachmentType::RT_SRV_UAV), sFBODesc(sFBODesc::sFBODimension((std::uint32_t)Width, (std::uint32_t)Height)));

    HudlessTextures.First = IRenderTarget::Create("HudlessTextures_First", sFrameBuffer(GPU::GetBackBufferFormat(), EFrameBufferAttachmentType::RT_SRV_UAV), sFBODesc(sFBODesc::sFBODimension((std::uint32_t)Width, (std::uint32_t)Height)));
    HudlessTextures.Second = IRenderTarget::Create("HudlessTextures_Second", sFrameBuffer(GPU::GetBackBufferFormat(), EFrameBufferAttachmentType::RT_SRV_UAV), sFBODesc(sFBODesc::sFBODimension((std::uint32_t)Width, (std::uint32_t)Height)));

    InterpolationOutput = IRenderTarget::Create("FSR Interpolation Output", sFrameBuffer(GPU::GetBackBufferFormat(), EFrameBufferAttachmentType::RT_SRV_UAV), sFBODesc(sFBODesc::sFBODimension((std::uint32_t)Width, (std::uint32_t)Height)));

    GraphicsCommandContext->SetName("GraphicsCommandContext_FSR_Upscale");
    FrameGenCommandContext->SetName("FrameGenCommandContext");
    FSRCopyCommandContext->SetName("FSRCopyCommandContext");

    /*{
        struct ffxQueryDescGetVersions versionQuery = { 0 };
        versionQuery.header.type = FFX_API_QUERY_DESC_TYPE_GET_VERSIONS;
        ffxReturnCode_t retCode_t;

        versionQuery.createDescType = FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE;
        versionQuery.device = GPU::GetInternalDevice();

        uint64_t versionCount = 0;
        versionQuery.outputCount = &versionCount;
        retCode_t = ffxQuery(nullptr, &versionQuery.header);
        Engine::Assert(AssertLevel::ASSERT_WARNING, retCode_t == FFX_API_RETURN_OK, L"ffxQuery(nullptr,GetVersionsUpscaleCount) returned %d", retCode_t);

        FsrVersionIds.resize(versionCount);
        FsrVersionNames.resize(versionCount);
        versionQuery.versionIds = FsrVersionIds.data();
        versionQuery.versionNames = FsrVersionNames.data();
        retCode_t = ffxQuery(nullptr, &versionQuery.header);
        Engine::Assert(AssertLevel::ASSERT_WARNING, retCode_t == FFX_API_RETURN_OK, L"ffxQuery(nullptr,GetVersionsUpscaleIdsNames) returned %d", retCode_t);
    }*/

    FSRContext->QueryVersionNames(FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE, FsrVersionIds, FsrVersionNames);
    FSRContext->QueryVersionNames(FFX_API_CREATE_CONTEXT_DESC_TYPE_FRAMEGENERATION, FgVersionIds, FgVersionNames);

    for (const auto& Name : FsrVersionNames)
        Engine::WriteToConsole("Supported FSR Upscale Version : " + Name);
    for (const auto& Name : FgVersionNames)
        Engine::WriteToConsole("Supported FSR Framegen Version : " + Name);

    ffx::CreateContextDescOverrideVersion versionOverride{};
    //ffx::QueryDescUpscaleGetResourceRequirements upscaleResourceRequirementsDesc{};
    ffx::ReturnCode retCode;

    for (size_t index = 0; index < FsrVersionIds.size(); index++)
    {
        versionOverride.versionId = FsrVersionIds[index];
        versionOverride.header.type = FFX_API_DESC_TYPE_OVERRIDE_VERSION;
        ffx::QueryDescUpscaleGetResourceRequirements upscaleResourceRequirementsDesc{};
        {
            retCode = FSRContext->Query(upscaleResourceRequirementsDesc, versionOverride);
            Engine::Assert(AssertLevel::ASSERT_CRITICAL, retCode == ffx::ReturnCode::Ok,
                L"ffxQuery(nullptr,GetResourceRequirements,%S,backend) returned %d", FsrVersionNames[index].c_str(), (uint32_t)retCode);
        }
        Engine::Print(L"Upscaler %S: required_resources: %llu optional_resources: %llu",
            FsrVersionNames[index].c_str(), upscaleResourceRequirementsDesc.required_resources, upscaleResourceRequirementsDesc.optional_resources);
    }
}

sFSR::~sFSR()
{
    HudlessTextures.Release();
	GraphicsCommandContext = nullptr;

    Output = nullptr;
    GeneratedReactiveMask = nullptr;

    // Destroy the FSR
    SetUpscaleEnabled(false);
    SetFrameGenerationEnabled(false);
    FSRContext = nullptr;
}

void sFSR::BeginPlay()
{
    UpdatePreset(nullptr);
}

void sFSR::Tick(const double DeltaTime)
{
    DT = (float)DeltaTime;
}

void sFSR::OnEnabled(bool bEnableUpscale, bool bEnableFrameGen)
{
    JitterFrame = 0;

    if (bEnableUpscale)
    {
        SetUpscaleEnabled(true);
        UpdatePreset(nullptr);
    }
    if (bEnableFrameGen)
    {
        SetFrameGenerationEnabled(true);
    }
}

void sFSR::OnDisabled()
{
    JitterFrame = 0;

    if (UpscalingContext)
    {
        SetUpscaleEnabled(false);
        if (OnUpdatePreset)
            OnUpdatePreset(sScreenDimension(0, 0), -1.0f);
    }
    if (FrameGenContext)
    {
        SetFrameGenerationEnabled(false);
    }
}

void sFSR::SetUpscaleEnabled(bool bEnabled)
{
    if (GPU::GetGIType() == EGITypes::D3D11)
    {
        Engine::WriteToConsole("FSR 3/4 does not support D3D11.");
        return;
    }

    if (bUpscalingActive == bEnabled)
        return;

    if (bEnabled)
    {
        bUpscalingActive = true;
        ffx::CreateContextDescUpscale createFsr{};

        createFsr.maxUpscaleSize = { (std::uint32_t)RenderDimension.UpscaleWidth, (std::uint32_t)RenderDimension.UpscaleHeight };
        createFsr.maxRenderSize = { (std::uint32_t)RenderDimension.RenderWidth, (std::uint32_t)RenderDimension.RenderHeight };
        createFsr.flags = FFX_UPSCALE_ENABLE_AUTO_EXPOSURE;
        if (bInvertedDepth)
        {
            createFsr.flags |= FFX_UPSCALE_ENABLE_DEPTH_INVERTED | FFX_UPSCALE_ENABLE_DEPTH_INFINITE;
        }
        if (UpscaleMode == ERendererUpscaleMode::DynamicResolution)
        {
            createFsr.flags |= FFX_UPSCALE_ENABLE_DYNAMIC_RESOLUTION;
        }
        createFsr.flags |= FFX_UPSCALE_ENABLE_HIGH_DYNAMIC_RANGE;
        createFsr.flags |= ColorSpace == FSRColorSpace::NonLinearColorSpace ? FFX_UPSCALE_ENABLE_NON_LINEAR_COLORSPACE : 0;
        createFsr.fpMessage = nullptr;

#if FSR_DEBUG && FSR_Upscale_DEBUG
        createFsr.flags |= FFX_UPSCALE_ENABLE_DEBUG_CHECKING;
        createFsr.flags |= FFX_UPSCALE_ENABLE_DEBUG_VISUALIZATION;
#endif
        //Before creating any of FSR contexts, query VRAM size
        struct FfxApiEffectMemoryUsage gpuMemoryUsageUpscaler = {0};
        ffxQueryDescUpscaleGetGPUMemoryUsageV2 upscalerGetGPUMemoryUsageV2 = { 0 };
        upscalerGetGPUMemoryUsageV2.header.type = FFX_API_QUERY_DESC_TYPE_UPSCALE_GPU_MEMORY_USAGE_V2;
        upscalerGetGPUMemoryUsageV2.device = FSRContext->GetDevice();
        upscalerGetGPUMemoryUsageV2.maxRenderSize = { (std::uint32_t)RenderDimension.RenderWidth, (std::uint32_t)RenderDimension.RenderHeight };
        upscalerGetGPUMemoryUsageV2.maxUpscaleSize = { (std::uint32_t)RenderDimension.UpscaleWidth, (std::uint32_t)RenderDimension.UpscaleHeight };
        upscalerGetGPUMemoryUsageV2.flags = createFsr.flags;
        upscalerGetGPUMemoryUsageV2.gpuMemoryUsageUpscaler = &gpuMemoryUsageUpscaler;

        uint32_t renderResolution[2] = { 0, 0 };
        ffx::QueryDescUpscaleGetRenderResolutionFromQualityMode renderResDesc;
        renderResDesc.qualityMode = static_cast<uint32_t>(UpscaleMode);
        renderResDesc.displayHeight = (std::uint32_t)RenderDimension.DisplayHeight;
        renderResDesc.displayWidth = (std::uint32_t)RenderDimension.DisplayWidth;
        renderResDesc.pOutRenderWidth = &renderResolution[0];
        renderResDesc.pOutRenderHeight = &renderResolution[1];

        ffxCreateContextDescUpscaleVersion headerVersion = {};
        headerVersion.version = FFX_UPSCALER_VERSION;

        // lifetime of this must last until after CreateContext call!
        struct ffxOverrideVersion versionOverride = { 0 };
        versionOverride.header.type = FFX_API_DESC_TYPE_OVERRIDE_VERSION;
        ffx::ReturnCode UpscalingRetCode = ffx::ReturnCode();
        if (FsrVersionIndex < FsrVersionIds.size() && bOverrideFSRUpscaleVersion)
        {
            versionOverride.versionId = FsrVersionIds[FsrVersionIndex];
            upscalerGetGPUMemoryUsageV2.header.pNext = &versionOverride.header;
            ffxReturnCode_t retCode_t = ffxQuery(nullptr, &upscalerGetGPUMemoryUsageV2.header);
            Engine::Assert(AssertLevel::ASSERT_WARNING, retCode_t == FFX_API_RETURN_OK,
                L"ffxQuery(nullptr,UpscaleGetGPUMemoryUsageV2, %S) returned %d", FsrVersionNames[FsrVersionIndex].c_str(), retCode_t);
            Engine::Print(L"Upscaler version %S Query GPUMemoryUsageV2 VRAM totalUsageInBytes %f MB aliasableUsageInBytes %f MB", FsrVersionNames[FsrVersionIndex].c_str(), gpuMemoryUsageUpscaler.totalUsageInBytes / 1048576.f, gpuMemoryUsageUpscaler.aliasableUsageInBytes / 1048576.f);

            //versionOverride.header.pNext = &backendDesc.header;
            renderResDesc.header.pNext = &versionOverride.header;
            // Query render resolution from display resolution via driver override path (null-context)
            retCode_t = ffxQuery(nullptr, &renderResDesc.header);
            Engine::Assert(AssertLevel::ASSERT_WARNING, retCode_t == FFX_API_RETURN_OK,
                L"ffxQuery(nullptr,QueryDescUpscaleGetRenderResolutionFromQualityMode, %S) returned %d", FsrVersionNames[FsrVersionIndex].c_str(), retCode_t);
            Engine::Print(L"Upscaler version %S QueryDescUpscaleGetRenderResolutionFromQualityMode qualityMode %d renderResolution %d x %d", FsrVersionNames[FsrVersionIndex].c_str(), static_cast<uint32_t>(UpscaleMode), renderResolution[0], renderResolution[1]);

            //ffx::ReturnCode retCode = ffx::CreateContext(UpscalingContext, nullptr, createFsr, backendDesc, headerVersion, versionOverride);
            UpscalingRetCode = FSRContext->CreateUpscalingContext(UpscalingContext, createFsr, headerVersion, versionOverride);
        }
        else
        {
            ffx::ReturnCode retCode = ffx::Query(upscalerGetGPUMemoryUsageV2);
            Engine::Assert(AssertLevel::ASSERT_WARNING, retCode == ffx::ReturnCode::Ok,
                L"ffxQuery(nullptr,UpscaleGetGPUMemoryUsageV2) returned %d", (uint32_t)retCode);
            Engine::Print(L"Default Upscaler Query GPUMemoryUsageV2 totalUsageInBytes %f MB aliasableUsageInBytes %f MB", gpuMemoryUsageUpscaler.totalUsageInBytes / 1048576.f, gpuMemoryUsageUpscaler.aliasableUsageInBytes / 1048576.f);

            ffxReturnCode_t retCode_t = ffxQuery(nullptr, &renderResDesc.header);
            Engine::Assert(AssertLevel::ASSERT_WARNING, retCode_t == FFX_API_RETURN_OK,
                L"ffxQuery(nullptr,QueryDescUpscaleGetRenderResolutionFromQualityMode, %S) returned %d", FsrVersionNames[FsrVersionIndex].c_str(), retCode_t);
            Engine::Print(L"Default Upscaler QueryDescUpscaleGetRenderResolutionFromQualityMode qualityMode %d renderResolution %d x %d", static_cast<uint32_t>(UpscaleMode), renderResolution[0], renderResolution[1]);

            UpscalingRetCode = FSRContext->CreateUpscalingContext(UpscalingContext, createFsr, headerVersion);
        }
        ffxQueryGetProviderVersion getVersion = { 0 };
        getVersion.header.type = FFX_API_QUERY_DESC_TYPE_GET_PROVIDER_VERSION;

        ffxReturnCode_t retCode_t = ffxQuery(&UpscalingContext, &getVersion.header);
        Engine::Assert(AssertLevel::ASSERT_WARNING, retCode_t == FFX_API_RETURN_OK, L"ffxQuery(UpscalingContext,GetProviderVersion) returned %d", retCode_t);

        CurrentUpscaleContextVersionId = getVersion.versionId;
        CurrentUpscaleContextVersionName = getVersion.versionName;

        Engine::Print(L"Upscaler Context versionid 0x%016llx, %S", CurrentUpscaleContextVersionId, CurrentUpscaleContextVersionName);

        for (uint32_t i = 0; i < FsrVersionIds.size(); i++)
        {
            if (FsrVersionIds[i] == CurrentUpscaleContextVersionId)
            {
                FsrVersionIndex = i;
            }
        }
    }
    else
    {
        bUpscalingActive = false;
        ffx::DestroyContext(UpscalingContext);
        UpscalingContext = nullptr;
    }
}

void sFSR::SetUpscaleSharpness(float NewSharpness)
{
    if (GPU::GetGIType() == EGITypes::D3D11)
    {
        Engine::WriteToConsole("FSR 3/4 does not support D3D11.");
        return;
    }
    Sharpness = NewSharpness;
}

void sFSR::SetUpscaleMode(ERendererUpscaleMode Mode)
{
    if (GPU::GetGIType() == EGITypes::D3D11)
    {
        Engine::WriteToConsole("FSR 3/4 does not support D3D11.");
        return;
    }
    UpscaleMode = Mode;
    UpdatePreset(nullptr);
}

void sFSR::SetFrameGenerationEnabled(bool bEnabled)
{
    if (GPU::GetGIType() == EGITypes::D3D11)
    {
        Engine::WriteToConsole("FSR 3/4 does not support D3D11.");
        return;
    }

    if (bFrameInterpolationActive == bEnabled)
        return;

    //{
    //    ffx::ReturnCode retCode;
    //    FfxApiEffectMemoryUsage gpuMemoryUsageSwapChainV2{};
    //    ffxQueryFrameGenerationSwapChainGetGPUMemoryUsageDX12V2 swapChainGetGPUMemoryUsageV2{};
    //    D3D12Device* Device = (D3D12Device*)GPU::GetDevice();
    //    D3D12Viewport* Viewport = (D3D12Viewport*)Device->GetD3D12Viewport();
    //    swapChainGetGPUMemoryUsageV2.device = GPU::GetInternalDevice();
    //    IDXGISwapChain4* dxgiSwapchain = Viewport->GetSwapChainPtr();
    //    DXGI_SWAP_CHAIN_DESC1 desc1;
    //    dxgiSwapchain->GetDesc1(&desc1);
    //    FfxApiDimensions2D swapChainDescDisplaySize;
    //    swapChainDescDisplaySize.width = desc1.Width;
    //    swapChainDescDisplaySize.height = desc1.Height;
    //    swapChainGetGPUMemoryUsageV2.displaySize = swapChainDescDisplaySize;
    //    swapChainGetGPUMemoryUsageV2.backBufferFormat = ffxApiGetSurfaceFormatDX12(desc1.Format);
    //    swapChainGetGPUMemoryUsageV2.backBufferCount = desc1.BufferCount;

    //    //FfxApiResource uiColor = SDKWrapper::ffxGetResourceApi(m_pUiTexture[0]->GetResource(), FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
    //    FfxApiDimensions2D uiResourceSize;
    //    uiResourceSize.width = /*(UIRenderMode == UICompositionMode::UiTexture) ? uiColor.description.width :*/ 0;
    //    uiResourceSize.height = /*(UIRenderMode == UICompositionMode::UiTexture) ? uiColor.description.height :*/ 0;
    //    swapChainGetGPUMemoryUsageV2.uiResourceSize = uiResourceSize;
    //    swapChainGetGPUMemoryUsageV2.uiResourceFormat = /*(UIRenderMode == UICompositionMode::UiTexture) ? uiColor.description.format :*/ FFX_API_SURFACE_FORMAT_UNKNOWN;
    //    swapChainGetGPUMemoryUsageV2.flags = bDoublebufferInSwapchain ? FFX_FRAMEGENERATION_UI_COMPOSITION_FLAG_ENABLE_INTERNAL_UI_DOUBLE_BUFFERING : 0;
    //    swapChainGetGPUMemoryUsageV2.gpuMemoryUsageFrameGenerationSwapchain = &gpuMemoryUsageSwapChainV2;

    //    retCode = ffx::Query(swapChainGetGPUMemoryUsageV2);
    //    Engine::Assert(AssertLevel::ASSERT_WARNING, retCode == ffx::ReturnCode::Ok,
    //        L"ffxQuery(nullptr,SwapChainGetGPUMemoryUsageDX12V2) returned %d", (uint32_t)retCode);
    //    Engine::Print(L"Swapchain Query GPUMemoryUsageV2 VRAM totalUsageInBytes %f MB aliasableUsageInBytes %f MB", gpuMemoryUsageSwapChainV2.totalUsageInBytes / 1048576.f, gpuMemoryUsageSwapChainV2.aliasableUsageInBytes / 1048576.f);
    //}

    {
        FfxApiResource uiColor = FfxApiResource();//ffxApiGetResourceDX12((ID3D12Resource*)m_pUiTexture[0]->GetResource(), FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
        const bool useUiTexture = UIRenderMode == UICompositionMode::UiTexture;
        const FfxApiDimensions2D uiResourceSize = {
            useUiTexture ? uiColor.description.width : 0,
            useUiTexture ? uiColor.description.height : 0 };
        const uint32_t uiResourceFormat = useUiTexture ? uiColor.description.format : FFX_API_SURFACE_FORMAT_UNKNOWN;
        const uint32_t uiFlags = bDoublebufferInSwapchain ? FFX_FRAMEGENERATION_UI_COMPOSITION_FLAG_ENABLE_INTERNAL_UI_DOUBLE_BUFFERING : 0;

        FfxApiEffectMemoryUsage gpuMemoryUsageSwapChainV2 = {};
        ffx::ReturnCode retCode = ffx::ReturnCode::ErrorParameter;
//#if defined(_VK)
//        const VkSwapchainCreateInfoKHR* swapChainInfo = GetSwapChain()->GetImpl()->GetCreateInfo();
//        ffx::QueryFrameGenerationSwapChainGetGPUMemoryUsageVKV2 swapChainGetGPUMemoryUsageV2{};
//        swapChainGetGPUMemoryUsageV2.device = GetDevice()->GetImpl()->VKDevice();
//        swapChainGetGPUMemoryUsageV2.physicalDevice = GetDevice()->GetImpl()->VKPhysicalDevice();
//        swapChainGetGPUMemoryUsageV2.displaySize = { swapChainInfo->imageExtent.width, swapChainInfo->imageExtent.height };
//        swapChainGetGPUMemoryUsageV2.backBufferFormat = ffxApiGetSurfaceFormatVK(swapChainInfo->imageFormat);
//        swapChainGetGPUMemoryUsageV2.backBufferCount = swapChainInfo->minImageCount;
//        swapChainGetGPUMemoryUsageV2.composeOnGameQueue = 0;
//#elif defined(_DX12)
        /*D3D12Device* Device = (D3D12Device*)GPU::GetDevice();
        D3D12Viewport* Viewport = (D3D12Viewport*)Device->GetD3D12Viewport();
        IDXGISwapChain4* dxgiSwapchain = Viewport->GetSwapChainPtr();
        DXGI_SWAP_CHAIN_DESC1 desc1;
        dxgiSwapchain->GetDesc1(&desc1);
        ffx::QueryFrameGenerationSwapChainGetGPUMemoryUsageDX12V2 swapChainGetGPUMemoryUsageV2{};
        swapChainGetGPUMemoryUsageV2.device = GPU::GetInternalDevice();
        swapChainGetGPUMemoryUsageV2.displaySize = { desc1.Width, desc1.Height };
        swapChainGetGPUMemoryUsageV2.backBufferFormat = ffxApiGetSurfaceFormatDX12(desc1.Format);
        swapChainGetGPUMemoryUsageV2.backBufferCount = desc1.BufferCount;
//#endif
        swapChainGetGPUMemoryUsageV2.uiResourceSize = uiResourceSize;
        swapChainGetGPUMemoryUsageV2.uiResourceFormat = uiResourceFormat;
        swapChainGetGPUMemoryUsageV2.flags = uiFlags;
        swapChainGetGPUMemoryUsageV2.gpuMemoryUsageFrameGenerationSwapchain = &gpuMemoryUsageSwapChainV2;

        retCode = ffx::Query(swapChainGetGPUMemoryUsageV2);*/
        retCode = FSRContext->GetSwapChainGPUMemoryUsage(&gpuMemoryUsageSwapChainV2, uiResourceSize, uiResourceFormat, uiFlags);
        Engine::Assert(AssertLevel::ASSERT_WARNING, retCode == ffx::ReturnCode::Ok, L"ffxQuery(nullptr,SwapChainGetGPUMemoryUsageV2) returned %d", (uint32_t)retCode);
        Engine::Print(L"Swapchain Query GPUMemoryUsageV2 VRAM totalUsageInBytes %f MB aliasableUsageInBytes %f MB", gpuMemoryUsageSwapChainV2.totalUsageInBytes / 1048576.f, gpuMemoryUsageSwapChainV2.aliasableUsageInBytes / 1048576.f);
    }

    if (bEnabled)
    {
        std::lock_guard<std::mutex> Lock(FGContextMutex);
        bFrameInterpolationActive = true;
        EnableFrameInterpolationSwapchain(true);

        //query FG Swapchain VRAM size
        {
            ffx::ReturnCode retCode;
            FfxApiEffectMemoryUsage gpuMemoryUsageFrameGenerationSwapchain;
            retCode = FSRContext->GetSwapChainGetGPUMemoryUsage(SwapChainContext, gpuMemoryUsageFrameGenerationSwapchain);

            Engine::Assert(AssertLevel::ASSERT_WARNING, retCode == ffx::ReturnCode::Ok,
                L"ffxQuery(m_SwapChainContext, SwapChainGetGPUMemoryUsageVK) returned %d", (uint32_t)retCode);
            Engine::Print(L"Swapchain Query GPUMemoryUsage VRAM totalUsageInBytes %f MB aliasableUsageInBytes %f MB", gpuMemoryUsageFrameGenerationSwapchain.totalUsageInBytes / 1048576.f, gpuMemoryUsageFrameGenerationSwapchain.aliasableUsageInBytes / 1048576.f);
        }

        ffx::CreateContextDescFrameGeneration createFg{};
        createFg.displaySize = { (std::uint32_t)RenderDimension.DisplayWidth, (std::uint32_t)RenderDimension.DisplayHeight };
        createFg.maxRenderSize = { (std::uint32_t)RenderDimension.DisplayWidth, (std::uint32_t)RenderDimension.DisplayHeight };
        if (bInvertedDepth)
            createFg.flags |= FFX_FRAMEGENERATION_ENABLE_DEPTH_INVERTED | FFX_FRAMEGENERATION_ENABLE_DEPTH_INFINITE;
        createFg.flags |= FFX_FRAMEGENERATION_ENABLE_HIGH_DYNAMIC_RANGE;

        if (bEnableAsyncCompute)
        {
            createFg.flags |= FFX_FRAMEGENERATION_ENABLE_ASYNC_WORKLOAD_SUPPORT;
        }

#if FSR_DEBUG && FSR_FrameGen_DEBUG
        createFg.flags |= FFX_FRAMEGENERATION_ENABLE_DEBUG_CHECKING;
#endif

        createFg.backBufferFormat = GetFfxSurfaceFormat(GPU::GetBackBufferFormat());
        FrameGenerationConfig.flags = 0u;
#if FSR_DEBUG && FSR_FrameGen_DEBUG
        FrameGenerationConfig.flags |= bDrawFrameGenerationDebugTearLines ? FFX_FRAMEGENERATION_FLAG_DRAW_DEBUG_TEAR_LINES : 0;
        FrameGenerationConfig.flags |= bDrawFrameGenerationDebugResetIndicators ? FFX_FRAMEGENERATION_FLAG_DRAW_DEBUG_RESET_INDICATORS : 0;
        FrameGenerationConfig.flags |= bDrawFrameGenerationDebugPacingLines ? FFX_FRAMEGENERATION_FLAG_DRAW_DEBUG_PACING_LINES : 0;
        FrameGenerationConfig.flags |= bDrawFrameGenerationDebugView ? FFX_FRAMEGENERATION_FLAG_DRAW_DEBUG_VIEW : 0;
#endif
        FrameGenerationConfig.flags |= !SwapChainContext ? FFX_FRAMEGENERATION_FLAG_NO_SWAPCHAIN_CONTEXT_NOTIFY : 0;

        //Before creating FG context, query VRAM size
        FfxApiEffectMemoryUsage gpuMemoryUsageFrameGenerationV2;
        ffx::QueryDescFrameGenerationGetGPUMemoryUsageV2 frameGenGetGPUMemoryUsageV2{};
        frameGenGetGPUMemoryUsageV2.device = FSRContext->GetDevice();
        frameGenGetGPUMemoryUsageV2.maxRenderSize = createFg.maxRenderSize;
        frameGenGetGPUMemoryUsageV2.displaySize = createFg.displaySize;
        frameGenGetGPUMemoryUsageV2.createFlags = createFg.flags;
        frameGenGetGPUMemoryUsageV2.dispatchFlags = FrameGenerationConfig.flags;
        frameGenGetGPUMemoryUsageV2.backBufferFormat = createFg.backBufferFormat;
        frameGenGetGPUMemoryUsageV2.gpuMemoryUsageFrameGeneration = &gpuMemoryUsageFrameGenerationV2;

        ffx::CreateContextDescFrameGenerationVersion headerVersion{};
        headerVersion.version = FFX_FRAMEGENERATION_VERSION;
        ffx::ReturnCode retCode;
        if (UIRenderMode == UICompositionMode::PreUiBackbuffer)
        {
            ffx::CreateContextDescFrameGenerationHudless createFgHudless{};
            createFgHudless.hudlessBackBufferFormat = GetFfxSurfaceFormat(HudlessTextures.Get()->GetFormat());
            frameGenGetGPUMemoryUsageV2.backBufferFormat = createFgHudless.hudlessBackBufferFormat;

            if (FgVersionIndex < FgVersionIds.size() && bOverrideFSRFrameGenVersion)
            {
                struct ffxOverrideVersion versionOverride = { 0 };
                versionOverride.header.type = FFX_API_DESC_TYPE_OVERRIDE_VERSION;
                versionOverride.versionId = FgVersionIds[FgVersionIndex];
                retCode = ffx::Query(frameGenGetGPUMemoryUsageV2, versionOverride);
                Engine::Assert(AssertLevel::ASSERT_WARNING, retCode == ffx::ReturnCode::Ok, L"ffx::Query(FrameGenerationGetGPUMemoryUsageV2, %S) returned %d", FsrVersionNames[FsrVersionIndex].c_str(), (uint32_t)retCode);
                retCode = FSRContext->CreateHudlessFrameGenContext(FrameGenContext, createFg, createFgHudless, headerVersion, versionOverride);
            }
            else
            {
                retCode = ffx::Query(frameGenGetGPUMemoryUsageV2);
                Engine::Assert(AssertLevel::ASSERT_WARNING, retCode == ffx::ReturnCode::Ok, L"ffx::Query(FrameGenerationGetGPUMemoryUsageV2) returned %d", (uint32_t)retCode);
                retCode = FSRContext->CreateHudlessFrameGenContext(FrameGenContext, createFg, createFgHudless, headerVersion);
            }
        }
        else
        {
            frameGenGetGPUMemoryUsageV2.hudlessBackBufferFormat = FFX_API_SURFACE_FORMAT_UNKNOWN;

            if (FgVersionIndex < FgVersionIds.size() && bOverrideFSRFrameGenVersion)
            {
                struct ffxOverrideVersion versionOverride = { 0 };
                versionOverride.header.type = FFX_API_DESC_TYPE_OVERRIDE_VERSION;
                versionOverride.versionId = FgVersionIds[FgVersionIndex];
                retCode = ffx::Query(frameGenGetGPUMemoryUsageV2, versionOverride);
                Engine::Assert(AssertLevel::ASSERT_WARNING, retCode == ffx::ReturnCode::Ok, L"ffx::Query(FrameGenerationGetGPUMemoryUsageV2, %S) returned %d", FsrVersionNames[FsrVersionIndex].c_str(), (uint32_t)retCode);
                retCode = FSRContext->CreateFrameGenContext(FrameGenContext, createFg, headerVersion, versionOverride);
            }
            else
            {
                retCode = ffx::Query(frameGenGetGPUMemoryUsageV2);
                Engine::Assert(AssertLevel::ASSERT_WARNING, retCode == ffx::ReturnCode::Ok, L"ffx::Query(FrameGenerationGetGPUMemoryUsageV2) returned %d", (uint32_t)retCode);
                retCode = FSRContext->CreateFrameGenContext(FrameGenContext, createFg, headerVersion);
            }
        }

        Engine::Assert(AssertLevel::ASSERT_CRITICAL, retCode == ffx::ReturnCode::Ok, L"Couldn't create the ffxapi framegen context: %d", (uint32_t)retCode);

        Engine::Print(L"FrameGenerationGetGPUMemoryUsageV2 VRAM totalUsageInBytes %f MB aliasableUsageInBytes %f MB", gpuMemoryUsageFrameGenerationV2.totalUsageInBytes / 1048576.f, gpuMemoryUsageFrameGenerationV2.aliasableUsageInBytes / 1048576.f);

        void* ffxSwapChain = GPU::GetInternalSwapChain();

        // Configure frame generation
        //FfxApiResource hudLessResource = SDKWrapper::ffxGetResourceApi(m_pHudLessTexture[m_curUiTextureIndex]->GetResource(), FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
        FfxApiResource hudLessResource = FSRContext->GetResource(HudlessTextures.Get(), FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);

        FrameGenerationConfig.frameGenerationEnabled = false;
        FrameGenerationConfig.allowAsyncWorkloads = bEnableAsyncCompute;
        FrameGenerationConfig.frameGenerationCallback = [](ffxDispatchDescFrameGeneration* params, void* pUserCtx) -> ffxReturnCode_t
            {
                return ffxDispatch(reinterpret_cast<ffxContext*>(pUserCtx), &params->header);
            };
        FrameGenerationConfig.frameGenerationCallbackUserContext = &FrameGenContext;
        if (UIRenderMode == UICompositionMode::UiCallback)
        {
            //FrameGenerationConfig.presentCallback = nullptr;
            //FrameGenerationConfig.presentCallbackUserContext = nullptr;
            FrameGenerationConfig.presentCallback = [](ffxCallbackDescFrameGenerationPresent* params, void* self) -> auto { return reinterpret_cast<sFSR*>(self)->UiCompositionCallback(params); };
            FrameGenerationConfig.presentCallbackUserContext = this;
        }
        else
        {
            FrameGenerationConfig.presentCallback = nullptr;
            FrameGenerationConfig.presentCallbackUserContext = nullptr;
        }
        FrameGenerationConfig.swapChain = ffxSwapChain;
        FrameGenerationConfig.HUDLessColor = (UIRenderMode == UICompositionMode::PreUiBackbuffer) ? hudLessResource : FfxApiResource({});

        FrameGenerationConfig.frameID = FrameID;

        retCode = ffx::Configure(FrameGenContext, FrameGenerationConfig);
        Engine::Assert(AssertLevel::ASSERT_CRITICAL, retCode == ffx::ReturnCode::Ok, L"ffx::Configure(FrameGenContext,FrameGenerationConfig) returned %d", (uint32_t)retCode);
    }
    else
    {
        bFrameInterpolationActive = false;
        ffx::ReturnCode retCode;
        if (FrameGenContext != nullptr)
        {
            // Acquire lock before destroying FG context
            std::unique_lock<std::mutex> Lock(Mutex);

            void* ffxSwapChain = GPU::GetInternalSwapChain();
            // Disable frame generation before destroying context.
            // Internally calls waitForPresents(), which flushes interpolation and UI composition GPU work, to avoid
            // D3D12 debug layer error #921: OBJECT_DELETED_WHILE_STILL_IN_USE
            FrameGenerationConfig.frameGenerationEnabled = false;
            FrameGenerationConfig.swapChain = ffxSwapChain;
            FrameGenerationConfig.presentCallback = nullptr;
            FrameGenerationConfig.HUDLessColor = FfxApiResource({});
            retCode = ffx::Configure(FrameGenContext, FrameGenerationConfig);
            Engine::Assert(AssertLevel::ASSERT_CRITICAL, !!retCode, L"Configuring FSR FG before destroy failed: %d", (uint32_t)retCode);

            ffx::DestroyContext(FrameGenContext);
            FrameGenContext = nullptr;
        }

        EnableFrameInterpolationSwapchain(false);
    }
}

void sFSR::SetAsyncComputeEnabled(bool bEnabled)
{
    if (GPU::GetGIType() == EGITypes::D3D11)
    {
        Engine::WriteToConsole("FSR 3/4 does not support D3D11.");
        return;
    }
    if (bEnableAsyncCompute == bEnabled)
        return;

    const bool bRecreateFrameGenerationContext = FrameGenContext != nullptr;
    if (bRecreateFrameGenerationContext)
        SetFrameGenerationEnabled(false);

    bEnableAsyncCompute = bEnabled;

    if (bRecreateFrameGenerationContext)
        SetFrameGenerationEnabled(true);
}

void sFSR::EnableFrameInterpolationSwapchain(bool enabled)
{
    if (GPU::GetGIType() == EGITypes::D3D11)
    {
        Engine::WriteToConsole("FSR 3/4 does not support D3D11.");
        return;
    }
    if (enabled)
    {
        GPU::WaitForCPU();
        GPU::WaitForGPU();
        bEnableFrameInterpolationSwapchain = true;
        FSRContext->CreateSwapChainContext(SwapChainContext, bGetLatencyWaitableObject, LatencyWaitableObj);

/*#elif FSR_VULKAN
        // Create frameinterpolation swapchain
        cauldron::SwapChain* pSwapchain = cauldron::GetFramework()->GetSwapChain();
        VkSwapchainKHR       currentSwapchain = pSwapchain->GetImpl()->VKSwapChain();

        ffx::CreateContextDescFrameGenerationSwapChainVK createSwapChainDesc{};
        createSwapChainDesc.physicalDevice = cauldron::GetDevice()->GetImpl()->VKPhysicalDevice();
        createSwapChainDesc.device = cauldron::GetDevice()->GetImpl()->VKDevice();
        createSwapChainDesc.swapchain = &currentSwapchain;
        createSwapChainDesc.createInfo = *cauldron::GetFramework()->GetSwapChain()->GetImpl()->GetCreateInfo();
        createSwapChainDesc.allocator = nullptr;
        createSwapChainDesc.gameQueue.queue = cauldron::GetDevice()->GetImpl()->VKCmdQueue(cauldron::CommandQueue::Graphics);
        createSwapChainDesc.gameQueue.familyIndex = cauldron::GetDevice()->GetImpl()->VKCmdQueueFamily(cauldron::CommandQueue::Graphics);
        createSwapChainDesc.gameQueue.submitFunc = nullptr;  // this queue is only used in vkQueuePresentKHR, hence doesn't need a callback

        createSwapChainDesc.asyncComputeQueue.queue = pAsyncComputeQueue->queue;
        createSwapChainDesc.asyncComputeQueue.familyIndex = pAsyncComputeQueue->family;
        createSwapChainDesc.asyncComputeQueue.submitFunc = nullptr;

        createSwapChainDesc.presentQueue.queue = pPresentQueue->queue;
        createSwapChainDesc.presentQueue.familyIndex = pPresentQueue->family;
        createSwapChainDesc.presentQueue.submitFunc = nullptr;

        createSwapChainDesc.imageAcquireQueue.queue = pImageAcquireQueue->queue;
        createSwapChainDesc.imageAcquireQueue.familyIndex = pImageAcquireQueue->family;
        createSwapChainDesc.imageAcquireQueue.submitFunc = nullptr;

        // make sure swapchain is not holding a ref to real swapchain
        cauldron::GetFramework()->GetSwapChain()->GetImpl()->SetVKSwapChain(VK_NULL_HANDLE);

        auto convertQueueInfo = [](VkQueueInfoFFXAPI queueInfo) {
            VkQueueInfoFFX info;
            info.queue = queueInfo.queue;
            info.familyIndex = queueInfo.familyIndex;
            info.submitFunc = queueInfo.submitFunc;
            return info;
            };

        VkFrameInterpolationInfoFFX frameInterpolationInfo = {};
        frameInterpolationInfo.device = createSwapChainDesc.device;
        frameInterpolationInfo.physicalDevice = createSwapChainDesc.physicalDevice;
        frameInterpolationInfo.pAllocator = createSwapChainDesc.allocator;
        frameInterpolationInfo.gameQueue = convertQueueInfo(createSwapChainDesc.gameQueue);
        frameInterpolationInfo.asyncComputeQueue = convertQueueInfo(createSwapChainDesc.asyncComputeQueue);
        frameInterpolationInfo.presentQueue = convertQueueInfo(createSwapChainDesc.presentQueue);
        frameInterpolationInfo.imageAcquireQueue = convertQueueInfo(createSwapChainDesc.imageAcquireQueue);

        ffx::ReturnCode retCode = ffx::CreateContext(SwapChainContext, nullptr, createSwapChainDesc);

        ffx::QueryDescSwapchainReplacementFunctionsVK replacementFunctions{};
        ffx::Query(SwapChainContext, replacementFunctions);
        cauldron::GetDevice()->GetImpl()->SetSwapchainMethodsAndContext(nullptr,
            nullptr,
            replacementFunctions.pOutGetSwapchainImagesKHR,
            replacementFunctions.pOutAcquireNextImageKHR,
            replacementFunctions.pOutQueuePresentKHR,
            replacementFunctions.pOutSetHdrMetadataEXT,
            replacementFunctions.pOutCreateSwapchainFFXAPI,
            replacementFunctions.pOutDestroySwapchainFFXAPI,
            nullptr,
            replacementFunctions.pOutGetLastPresentCountFFXAPI,
            SwapChainContext,
            &frameInterpolationInfo);

        // Set frameinterpolation swapchain to engine
        cauldron::GetFramework()->GetSwapChain()->GetImpl()->SetVKSwapChain(currentSwapchain, true);

        // we need to re initialize HDR info since swapchain was re created
        cauldron::GetSwapChain()->SetHDRMetadataAndColorspace();
#endif  // defined(FFX_API_DX12)*/
    }
    else if (!enabled && SwapChainContext != nullptr)
    {
        bEnableFrameInterpolationSwapchain = false;
        // This doesn't call proxy swapchain destructor. There is still a swapchain ref held by cauldron
        ffx::DestroyContext(SwapChainContext);
        SwapChainContext = nullptr;
        if (LatencyWaitableObj)
        {
            CloseHandle(LatencyWaitableObj);
            LatencyWaitableObj = 0;
        }
        GPU::RecreateSwapChain();
    }
}

ffxQueryGetProviderVersion sFSR::GetVersion()
{
    if (GPU::GetGIType() == EGITypes::D3D11)
    {
        Engine::WriteToConsole("FSR 3/4 does not support D3D11.");
        return ffxQueryGetProviderVersion();
    }
    if (!UpscalingContext)
        return ffxQueryGetProviderVersion();
    ffxQueryGetProviderVersion Version = { 0 };
    Version.header.type = FFX_API_QUERY_DESC_TYPE_GET_PROVIDER_VERSION;
    ffx::ReturnCode retCode = ffx::Query(UpscalingContext, Version);
    return Version;
}

ffx::QueryDescUpscaleGetGPUMemoryUsage sFSR::GetUpscaleGetGPUMemoryUsage()
{
    if (GPU::GetGIType() == EGITypes::D3D11)
    {
        Engine::WriteToConsole("FSR 3/4 does not support D3D11.");
        return ffx::QueryDescUpscaleGetGPUMemoryUsage();
    }
    if (!UpscalingContext)
        return ffx::QueryDescUpscaleGetGPUMemoryUsage();
    FfxApiEffectMemoryUsage gpuMemoryUsageUpscaler;
    ffx::QueryDescUpscaleGetGPUMemoryUsage upscalerGetGPUMemoryUsage{};
    upscalerGetGPUMemoryUsage.gpuMemoryUsageUpscaler = &gpuMemoryUsageUpscaler;

    ffx::Query(UpscalingContext, upscalerGetGPUMemoryUsage);
    return upscalerGetGPUMemoryUsage;
}

bool sFSR::SetJitter(FVector2& Vec)
{
    if (!UpscalingContext)
    {
        Vec = FVector2::Zero();
        return false;
    }

    // Increment jitter index for frame
    ++JitterFrame;

    ffx::ReturnCode                     retCode;
    int32_t                             jitterPhaseCount;
    ffx::QueryDescUpscaleGetJitterPhaseCount getJitterPhaseDesc{};
    getJitterPhaseDesc.displayWidth = RenderDimension.DisplayWidth;
    getJitterPhaseDesc.renderWidth = RenderDimension.RenderWidth;
    getJitterPhaseDesc.pOutPhaseCount = &jitterPhaseCount;

    retCode = ffx::Query(UpscalingContext, getJitterPhaseDesc);
    Engine::Assert(AssertLevel::ASSERT_CRITICAL, retCode == ffx::ReturnCode::Ok, L"ffxQuery(FSR_GETJITTERPHASECOUNT) returned %d", retCode);

    ffx::QueryDescUpscaleGetJitterOffset getJitterOffsetDesc{};
    getJitterOffsetDesc.index = JitterFrame;
    getJitterOffsetDesc.phaseCount = jitterPhaseCount;
    getJitterOffsetDesc.pOutX = &JitterX;
    getJitterOffsetDesc.pOutY = &JitterY;

    retCode = ffx::Query(UpscalingContext, getJitterOffsetDesc);

    Engine::Assert(AssertLevel::ASSERT_CRITICAL, retCode == ffx::ReturnCode::Ok, L"ffxQuery(FSR_GETJITTEROFFSET) returned %d", retCode);

    Vec = FVector2(2.f * JitterX / RenderDimension.RenderWidth, -2.f * JitterY / RenderDimension.RenderHeight);
    return true;
}

void sFSR::Render(IRenderTarget* Color, IRenderTarget* MotionVectors, IRenderTarget* Reactive, IDepthTarget* Depth, ICamera* pCamera)
{
    if (bUpscalingActive && UpscalingContext)
    {
        if (pCamera)
        {
            if (!pCamera->IsJitterEnabled())
            {
                pCamera->SetEnableJitter(true);
                pCamera->SetJitterCallBack(std::bind(&sFSR::SetJitter, this, std::placeholders::_1));
            }
        }

        GraphicsCommandContext->BeginRecordCommandList(ECommandContextBeginState::ApiRender);
        GraphicsCommandContext->BeginProfile(ERenderPass::FSR, true);

        Color->AsResource(EResourceState::NonPixelShaderResource, GraphicsCommandContext.get());
        MotionVectors->AsResource(EResourceState::NonPixelShaderResource, GraphicsCommandContext.get());
        //if (Depth)
        //    Depth->AsResource(EResourceState::ShaderResource, GraphicsCommandContext.get());
        //Output->AsResource(EResourceState::UAV, GraphicsCommandContext.get());

        ffx::DispatchDescUpscale DispatchUpscale{};
        DispatchUpscale.commandList = GraphicsCommandContext->GetInternalCommandContext();
        DispatchUpscale.color = FSRContext->GetResource(Color, FFX_API_RESOURCE_STATE_COMPUTE_READ);
        DispatchUpscale.depth = FSRContext->GetDepthResource(Depth, FFX_API_RESOURCE_STATE_COMPUTE_READ);
        DispatchUpscale.motionVectors = FSRContext->GetResource(MotionVectors, FFX_API_RESOURCE_STATE_COMPUTE_READ);
        if (bGenerateReactiveMask)
        {
            GeneratedReactiveMask->AsResource(EResourceState::NonPixelShaderResource, GraphicsCommandContext.get());
            DispatchUpscale.reactive = FSRContext->GetResource(GeneratedReactiveMask.get(), FFX_API_RESOURCE_STATE_COMPUTE_READ);
        }
        else
        {
            //Reactive->AsResource(EResourceState::NonPixelShaderResource, GraphicsCommandContext.get());
            //DispatchUpscale.reactive = ffxApiGetResourceDX12((ID3D12Resource*)Reactive->GetNativeTexture(), FFX_API_RESOURCE_STATE_COMPUTE_READ);
        }
        DispatchUpscale.exposure = FSRContext->GetResource(nullptr, FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
        DispatchUpscale.transparencyAndComposition = FSRContext->GetResource(nullptr, FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
        DispatchUpscale.output = FSRContext->GetResource(Output.get(), FFX_API_RESOURCE_STATE_UNORDERED_ACCESS);
        // exposure, reactive, transparencyAndComposition left empty: auto exposure,
        // and FSR substitutes cleared 1x1 masks

        DispatchUpscale.enableSharpening = true;
        DispatchUpscale.sharpness = Sharpness; // 0.0 to 1.0
        DispatchUpscale.jitterOffset.x = -JitterX;
        DispatchUpscale.jitterOffset.y = -JitterY;
        DispatchUpscale.motionVectorScale.x = RenderDimension.fRenderWidth();
        DispatchUpscale.motionVectorScale.y = RenderDimension.fRenderHeight();
        DispatchUpscale.renderSize = { RenderDimension.RenderWidth,  RenderDimension.RenderHeight };
        DispatchUpscale.upscaleSize = { RenderDimension.UpscaleWidth, RenderDimension.UpscaleHeight };
        DispatchUpscale.frameTimeDelta = DT * 1000.0f;   // MILLISECONDSf
        DispatchUpscale.cameraNear = pCamera->GetNearClip(); // FLT_MAX
        DispatchUpscale.cameraFar = pCamera->GetFarClip();        // GetFarClip // GetNearClip
        DispatchUpscale.cameraFovAngleVertical = pCamera->GetFOV();
        DispatchUpscale.preExposure = 1.0f;
        DispatchUpscale.reset = bReset;
        DispatchUpscale.flags = 0;
#if FSR_DEBUG && FSR_Upscale_DEBUG
        DispatchUpscale.flags |= FFX_UPSCALE_FLAG_DRAW_DEBUG_VIEW;
#endif
        DispatchUpscale.flags |= ColorSpace == FSRColorSpace::sRGBColorSpace ? FFX_UPSCALE_FLAG_NON_LINEAR_COLOR_SRGB : 0;
        DispatchUpscale.flags |= ColorSpace == FSRColorSpace::PQColorSpace ? FFX_UPSCALE_FLAG_NON_LINEAR_COLOR_PQ : 0;
        bReset = false;

        auto Ret = ffx::Dispatch(UpscalingContext, DispatchUpscale);

        //Output->AsResource(EResourceState::ShaderResource, GraphicsCommandContext.get());

        //Color->AsRenderTarget(GraphicsCommandContext.get());
        //MotionVectors->AsRenderTarget(GraphicsCommandContext.get());
        //Depth->AsDepthTarget(GraphicsCommandContext.get());

        //if (!bFrameInterpolationActive)
        {
            GraphicsCommandContext->EndProfile();

            GraphicsCommandContext->FinishRecordCommandList();
            GraphicsCommandContext->ExecuteCommandList(ECommandContextExecuteType::Deferred, 6);
        }
    }

    {
        if (FrameGenContext != nullptr)
        {
            /*if (!bUpscalingActive)
            {
                GraphicsCommandContext->BeginRecordCommandList(ECommandContextBeginState::ApiRender);
                GraphicsCommandContext->BeginProfile(ERenderPass::FSR_FrameGen, true);
            }*/
            FrameGenCommandContext->BeginRecordCommandList(ECommandContextBeginState::ApiRender);
            FrameGenCommandContext->BeginProfile(ERenderPass::FSR_FrameGen, true);

            if (UIRenderMode == UICompositionMode::PreUiBackbuffer)
                HudlessTextures.Get()->AsResource(EResourceState::NonPixelShaderResource, FrameGenCommandContext.get());

            ffx::DispatchDescFrameGenerationPrepareV2 dispatchFgPrep{};
            ffx::ReturnCode retCode;

            dispatchFgPrep.commandList = FrameGenCommandContext->GetInternalCommandContext();
            dispatchFgPrep.depth = FSRContext->GetDepthResource(Depth, FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
            dispatchFgPrep.motionVectors = FSRContext->GetResource(MotionVectors, FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
            dispatchFgPrep.flags = 0;

            dispatchFgPrep.jitterOffset.x = -JitterX;
            dispatchFgPrep.jitterOffset.y = -JitterY;
            dispatchFgPrep.motionVectorScale.x = RenderDimension.fRenderWidth();
            dispatchFgPrep.motionVectorScale.y = RenderDimension.fRenderHeight();

            // Cauldron keeps time in seconds, but FSR expects milliseconds
            dispatchFgPrep.frameTimeDelta = static_cast<float>(DT * 1000.f);

            dispatchFgPrep.renderSize.width = RenderDimension.RenderWidth;
            dispatchFgPrep.renderSize.height = RenderDimension.RenderHeight;
            dispatchFgPrep.cameraFovAngleVertical = pCamera->GetFOV();

            dispatchFgPrep.cameraFar = pCamera->GetFarClip();
            dispatchFgPrep.cameraNear = pCamera->GetNearClip();
            dispatchFgPrep.viewSpaceToMetersFactor = 0.f;
            dispatchFgPrep.frameID = FrameID;
            dispatchFgPrep.reset = bResetFrameInterpolation;

            // Update frame generation config
            FfxApiResource hudLessResource = FSRContext->GetResource(HudlessTextures.Get(), FFX_API_RESOURCE_STATE_COMPUTE_READ);

            FrameGenerationConfig.frameGenerationEnabled = bFrameInterpolationActive;
            FrameGenerationConfig.flags = 0u;
#if FSR_DEBUG && FSR_FrameGen_DEBUG
            FrameGenerationConfig.flags |= bDrawFrameGenerationDebugTearLines ? FFX_FRAMEGENERATION_FLAG_DRAW_DEBUG_TEAR_LINES : 0;
            FrameGenerationConfig.flags |= bDrawFrameGenerationDebugResetIndicators ? FFX_FRAMEGENERATION_FLAG_DRAW_DEBUG_RESET_INDICATORS : 0;
            FrameGenerationConfig.flags |= bDrawFrameGenerationDebugPacingLines ? FFX_FRAMEGENERATION_FLAG_DRAW_DEBUG_PACING_LINES : 0;
            FrameGenerationConfig.flags |= bDrawFrameGenerationDebugView ? FFX_FRAMEGENERATION_FLAG_DRAW_DEBUG_VIEW : 0;
#endif
            if (!SwapChainContext)
                FrameGenerationConfig.flags |= FFX_FRAMEGENERATION_FLAG_NO_SWAPCHAIN_CONTEXT_NOTIFY;
            dispatchFgPrep.flags = FrameGenerationConfig.flags;  // TODO: maybe these should be distinct flags?
            FrameGenerationConfig.HUDLessColor = (UIRenderMode == UICompositionMode::PreUiBackbuffer) ? hudLessResource : FfxApiResource({});
            FrameGenerationConfig.allowAsyncWorkloads = bEnableAsyncCompute;
            // assume symmetric letterbox
            FrameGenerationConfig.generationRect.left = (RenderDimension.DisplayWidth - RenderDimension.UpscaleWidth) / 2;
            FrameGenerationConfig.generationRect.top = (RenderDimension.DisplayHeight - RenderDimension.UpscaleHeight) / 2;
            FrameGenerationConfig.generationRect.width = RenderDimension.UpscaleWidth;
            FrameGenerationConfig.generationRect.height = RenderDimension.UpscaleHeight;
            if (bUseCallback)
            {
                FrameGenerationConfig.frameGenerationCallback = [](ffxDispatchDescFrameGeneration* params, void* pUserCtx) -> ffxReturnCode_t
                {
                    return ffxDispatch(reinterpret_cast<ffxContext*>(pUserCtx), &params->header);
                };
                FrameGenerationConfig.frameGenerationCallbackUserContext = &FrameGenContext;
            }
            else
            {
                FrameGenerationConfig.frameGenerationCallback = nullptr;
                FrameGenerationConfig.frameGenerationCallbackUserContext = nullptr;
            }
            FrameGenerationConfig.onlyPresentGenerated = bPresentInterpolatedOnly;
            FrameGenerationConfig.frameID = FrameID;

            void* ffxSwapChain;
            ffxSwapChain = GPU::GetInternalSwapChain();
            FrameGenerationConfig.swapChain = ffxSwapChain;

            if (bUseDistortionField)
            {
                // ????
                /*ffx::ConfigureDescFrameGenerationRegisterDistortionFieldResource dfConfig{};
                dfConfig.distortionField = SDKWrapper::ffxGetResourceApi(m_pDistortionField[m_curUiTextureIndex]->GetResource(), FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
                retCode = ffx::Configure(FrameGenContext, FrameGenerationConfig, dfConfig);*/
            }
            else
            {
                retCode = ffx::Configure(FrameGenContext, FrameGenerationConfig);
            }
            Engine::Assert(AssertLevel::ASSERT_CRITICAL, !!retCode, L"Configuring FSR FG failed: %d", (uint32_t)retCode);

            //only dispatch FG Prepare if frame interpolation is enabled
            if (bFrameInterpolationActive)
            {
                //GPUScopedProfileCapture sampleMarker(pCmdList, L"FFX API FSR Frame Generation Prepare");
                // Acquire lock before dispatch FG Prepare
                std::lock_guard<std::mutex> Lock(FGContextMutex);
                FVector vectorInfo = pCamera->GetPosition();
                memcpy(dispatchFgPrep.cameraPosition, &vectorInfo, 3 * sizeof(float));
                vectorInfo = pCamera->GetUp();
                memcpy(dispatchFgPrep.cameraUp, &vectorInfo, 3 * sizeof(float));
                vectorInfo = pCamera->GetRight();
                memcpy(dispatchFgPrep.cameraRight, &vectorInfo, 3 * sizeof(float));
                vectorInfo = Negate(pCamera->GetDirection());
                memcpy(dispatchFgPrep.cameraForward, &vectorInfo, 3 * sizeof(float));

                retCode = ffx::Dispatch(FrameGenContext, dispatchFgPrep);
                Engine::Assert(AssertLevel::ASSERT_CRITICAL, !!retCode, L"Dispatching FSR FG (upscaling data) failed: %d", (uint32_t)retCode);
            }

            if (SwapChainContext)
            {
                FfxApiResource uiColor = (UIRenderMode == UICompositionMode::UiTexture) ? FfxApiResource({}) /*SDKWrapper::ffxGetResourceApi(m_pUiTexture[m_curUiTextureIndex]->GetResource(), FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ)*/ : FfxApiResource({});

                /*ffx::ConfigureDescFrameGenerationSwapChainRegisterUiResourceDX12 uiConfig{};
                uiConfig.uiResource = uiColor;
                uiConfig.flags = bDoublebufferInSwapchain ? FFX_FRAMEGENERATION_UI_COMPOSITION_FLAG_ENABLE_INTERNAL_UI_DOUBLE_BUFFERING : 0;
                ffx::Configure(SwapChainContext, uiConfig);*/

                FSRContext->ConfigureSwapChain(SwapChainContext, uiColor, bDoublebufferInSwapchain);
            }

            // Dispatch frame generation, if not using FG callback
            if (bFrameInterpolationActive && !bUseCallback)
            {
                ffx::DispatchDescFrameGeneration dispatchFg{};

                dispatchFg.numGeneratedFrames = 1;

                // assume symmetric letterbox
                dispatchFg.generationRect.left = (RenderDimension.DisplayWidth - RenderDimension.UpscaleWidth) / 2;
                dispatchFg.generationRect.top = (RenderDimension.DisplayHeight - RenderDimension.UpscaleHeight) / 2;
                dispatchFg.generationRect.width = RenderDimension.UpscaleWidth;
                dispatchFg.generationRect.height = RenderDimension.UpscaleHeight;
                if (SwapChainContext)
                {
                    FfxApiResource backbuffer = FSRContext->GetCurrentBackBuffer(FFX_API_RESOURCE_STATE_PRESENT);
                    dispatchFg.presentColor = backbuffer;

                    //ffx::QueryDescFrameGenerationSwapChainInterpolationCommandListDX12 queryCmdList{};
                    //queryCmdList.pOutCommandList = &dispatchFg.commandList;
                    //retCode = ffx::Query(SwapChainContext, queryCmdList);
                    retCode = FSRContext->QuerySwapChainCommandList(SwapChainContext, &dispatchFg.commandList);
                    Engine::Assert(AssertLevel::ASSERT_CRITICAL, retCode == ffx::ReturnCode::Ok,
                        L"ffx::Query(SwapChainContext,InterpolationCommandList) returned %d", (uint32_t)retCode);
                    //ffx::QueryDescFrameGenerationSwapChainInterpolationTextureDX12 queryFiTexture{};
                    //queryFiTexture.pOutTexture = &dispatchFg.outputs[0];
                    //retCode = ffx::Query(SwapChainContext, queryFiTexture);
                    retCode = FSRContext->QuerySwapChainInterpolationTexture(SwapChainContext, &dispatchFg.outputs[0]);
                    Engine::Assert(AssertLevel::ASSERT_CRITICAL, retCode == ffx::ReturnCode::Ok,
                        L"ffx::Query(SwapChainContext,InterpolationTexture) returned %d", (uint32_t)retCode);
                }
                else
                {
                    FfxApiResource backbuffer = FSRContext->GetCurrentBackBuffer(FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
                    dispatchFg.presentColor = backbuffer;

                    dispatchFg.commandList = FrameGenCommandContext->GetInternalCommandContext();
                    dispatchFg.outputs[0] = FSRContext->GetResource(InterpolationOutput.get(), FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
                }

                dispatchFg.frameID = FrameID;
                dispatchFg.reset = bResetFrameInterpolation;

                dispatchFg.backbufferTransferFunction = GetFfxApiBackbufferTransferFunction(GPU::GetDisplayMode());

                const HDRMetadata& hdrMetaData = GPU::GetHDRMetaData();
                dispatchFg.minMaxLuminance[0] = hdrMetaData.MinLuminance;
                dispatchFg.minMaxLuminance[1] = hdrMetaData.MaxLuminance;
                {
                    // Acquire lock if app dispatch frame generation directly
                    std::lock_guard<std::mutex> Lock(FGContextMutex);
                    retCode = ffx::Dispatch(FrameGenContext, dispatchFg);
                    Engine::Assert(AssertLevel::ASSERT_CRITICAL, !!retCode, L"Dispatching Frame Generation failed: %d", (uint32_t)retCode);
                }
            }

            FrameGenCommandContext->EndProfile();
            FrameGenCommandContext->FinishRecordCommandList();
            FrameGenCommandContext->ExecuteCommandList(ECommandContextExecuteType::Deferred, 7);
        }
    }

    FrameID += uint64_t(1 + bSimulatePresentSkip);
    bSimulatePresentSkip = false;
    bResetFrameInterpolation = false;

    {
        auto Result = GraphicsCommandContext->GetProfileResult();
        if (Result.IsValid())
            Engine::GetActiveCanvas()->AddRendererProfileResult(Result);
    }
    {
        auto Result = FrameGenCommandContext->GetProfileResult();
        if (Result.IsValid())
            Engine::GetActiveCanvas()->AddRendererProfileResult(Result);
    }
    //CopyToFrameBuffer(Color);
}

void sFSR::GenerateReactiveMask(IRenderTarget* Color)
{
    //if (GPU::GetGIType() == EGITypes::Vulkan || GPU::GetGIType() == EGITypes::D3D11)
    //{
    //    Engine::WriteToConsole("FSR 3/4 does not support Vulkan or D3D11.");
    //    return;
    //}
    //CMD->BeginRecordCommandList(ECommandContextBeginState::ApiRender);

    //GeneratedReactiveMask->AsResource(EResourceState::UAV, CMD.get());

    //ffx::DispatchDescUpscaleGenerateReactiveMask dispatchDesc{};
    //dispatchDesc.commandList = (ID3D12GraphicsCommandList8*)CMD->GetInternalCommandContext();
    //dispatchDesc.colorOpaqueOnly = ffxApiGetResourceDX12((ID3D12Resource*)Color->GetNativeTexture(), FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
    //dispatchDesc.colorPreUpscale = ffxApiGetResourceDX12((ID3D12Resource*)Output->GetNativeTexture(), FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
    //dispatchDesc.outReactive = ffxApiGetResourceDX12((ID3D12Resource*)GeneratedReactiveMask->GetNativeTexture(), FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);

    //dispatchDesc.renderSize.width = RenderDimension.RenderWidth;
    //dispatchDesc.renderSize.height = RenderDimension.RenderHeight;

    //// The following are all hard-coded in the original FSR2 sample. Should these be exposed?
    //dispatchDesc.scale = 1.f;
    //dispatchDesc.cutoffThreshold = 0.2f;
    //dispatchDesc.binaryValue = 0.9f;
    //dispatchDesc.flags = FFX_UPSCALE_AUTOREACTIVEFLAGS_APPLY_TONEMAP |
    //    FFX_UPSCALE_AUTOREACTIVEFLAGS_APPLY_THRESHOLD |
    //    FFX_UPSCALE_AUTOREACTIVEFLAGS_USE_COMPONENTS_MAX;

    //ffx::ReturnCode retCode = ffx::Dispatch(UpscalingContext, dispatchDesc);
    //Engine::Assert(AssertLevel::ASSERT_ERROR, retCode == ffx::ReturnCode::Ok, L"ffxDispatch(FSR_GENERATEREACTIVEMASK) failed with %d", (uint32_t)retCode);

    //CMD->FinishRecordCommandList();
    //CMD->ExecuteCommandList(/*ECommandContextExecuteType::Deferred, 5*/);
}

ffxReturnCode_t sFSR::UiCompositionCallback(ffxCallbackDescFrameGenerationPresent* Params)
{
    Engine::WriteToConsole("sFSR::UiCompositionCallback CALLED!");
    return ffxReturnCode_t();
}

void sFSR::CopyToFrameBuffer(IRenderTarget* Target)
{
    //GraphicsCommandContext->BeginRecordCommandList();
    GraphicsCommandContext->CopyRenderTarget(Target, Output.get());
    GraphicsCommandContext->FinishRecordCommandList();
    GraphicsCommandContext->ExecuteCommandList(ECommandContextExecuteType::Deferred, 4);
}

void sFSR::CopyHudlessTexture(IRenderTarget* HudlessTexture)
{
    FSRCopyCommandContext->BeginRecordCommandList();
    FSRCopyCommandContext->CopyRenderTarget(HudlessTextures.Get(), HudlessTexture);
    FSRCopyCommandContext->FinishRecordCommandList();
    FSRCopyCommandContext->ExecuteCommandList(ECommandContextExecuteType::Deferred, 31);
    HudlessTextures.SignalFence();
}

void sFSR::SetRenderSize(std::size_t InWidth, std::size_t InHeight)
{
    if (GPU::GetGIType() == EGITypes::D3D11)
    {
        return;
    }
    RenderDimension.DisplayWidth = (std::uint32_t)InWidth;
    RenderDimension.DisplayHeight = (std::uint32_t)InHeight;
    RenderDimension.RenderWidth = (std::uint32_t)InWidth;
    RenderDimension.RenderHeight = (std::uint32_t)InHeight;
    RenderDimension.UpscaleWidth = (std::uint32_t)InWidth;
    RenderDimension.UpscaleHeight = (std::uint32_t)InHeight;

    Output = nullptr;
    GeneratedReactiveMask = nullptr;

    Output = IRenderTarget::Create("FSR Output", sFrameBuffer(GPU::GetBackBufferFormat(), EFrameBufferAttachmentType::RT_SRV_UAV), sFBODesc(sFBODesc::sFBODimension((std::uint32_t)InWidth, (std::uint32_t)InHeight)));
    if (bGenerateReactiveMask)
        GeneratedReactiveMask = IRenderTarget::Create("FSR GeneratedReactiveMask", sFrameBuffer(GPU::GetBackBufferFormat(), EFrameBufferAttachmentType::RT_SRV_UAV), sFBODesc(sFBODesc::sFBODimension((std::uint32_t)InWidth, (std::uint32_t)InHeight)));

    HudlessTextures.First = IRenderTarget::Create("HudlessTextures_First", sFrameBuffer(GPU::GetBackBufferFormat(), EFrameBufferAttachmentType::RT_SRV_UAV), sFBODesc(sFBODesc::sFBODimension((std::uint32_t)InWidth, (std::uint32_t)InHeight)));
    HudlessTextures.Second = IRenderTarget::Create("HudlessTextures_Second", sFrameBuffer(GPU::GetBackBufferFormat(), EFrameBufferAttachmentType::RT_SRV_UAV), sFBODesc(sFBODesc::sFBODimension((std::uint32_t)InWidth, (std::uint32_t)InHeight)));

    InterpolationOutput = IRenderTarget::Create("FSR Interpolation Output", sFrameBuffer(GPU::GetBackBufferFormat(), EFrameBufferAttachmentType::RT_SRV_UAV), sFBODesc(sFBODesc::sFBODimension((std::uint32_t)InWidth, (std::uint32_t)InHeight)));
    JitterFrame = 0;

    if (bUpscalingActive)
    {
        SetUpscaleEnabled(false);
        SetUpscaleEnabled(true);
    }
    if (bFrameInterpolationActive)
    {
        SetFrameGenerationEnabled(false);
        SetFrameGenerationEnabled(true);
    }
}

sRenderDimension sFSR::UpdateResolution(uint32_t displayWidth, uint32_t displayHeight)
{
    return { static_cast<uint32_t>((float)displayWidth / UpscaleRatio * LetterboxRatio),
            static_cast<uint32_t>((float)displayHeight / UpscaleRatio * LetterboxRatio),
            static_cast<uint32_t>((float)displayWidth * LetterboxRatio),
            static_cast<uint32_t>((float)displayHeight * LetterboxRatio),
            displayWidth, displayHeight };
}

void sFSR::OnInputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar)
{
    if (GPU::GetGIType() == EGITypes::D3D11)
    {
        return;
    }
    if (KeyboardChar.KeyCode == 40 && KeyboardChar.bIsPressed /*&& KeyboardChar.bIsChar*/)
    {
        if (UpscaleMode == ERendererUpscaleMode::UltraPerformance)
            return;
        
        GPU::WaitForGPU();
        UpscaleMode = (ERendererUpscaleMode)((std::uint32_t)UpscaleMode + 1);

        bReset = true;
        UpdatePreset(nullptr);
        UpdateUpscaleRatio();
    }
    if (KeyboardChar.KeyCode == 38 && KeyboardChar.bIsPressed/* && KeyboardChar.bIsChar*/)
    {
        if (UpscaleMode == ERendererUpscaleMode::NativeAA)
            return;

        GPU::WaitForGPU();
        UpscaleMode = (ERendererUpscaleMode)((std::uint32_t)UpscaleMode - 1);

        bReset = true;
        UpdatePreset(nullptr);
        UpdateUpscaleRatio();
    }
}

void sFSR::SetUpscaleConstantBuffer(uint64_t key, float value)
{
    ffx::ConfigureDescUpscaleKeyValue m_upscalerKeyValueConfig{};
    m_upscalerKeyValueConfig.key = key;
    m_upscalerKeyValueConfig.ptr = &value;
    ffx::Configure(UpscalingContext, m_upscalerKeyValueConfig);
}

void sFSR::SetGlobalDebugCheckerMode(FSRDebugCheckerMode mode, bool recreate)
{
    ffx::ConfigureDescGlobalDebug GlobalDebugConfig{};
    if (mode == FSRDebugCheckerMode::Disabled)
    {
        GlobalDebugConfig.fpMessage = nullptr;
        GlobalDebugConfig.debugLevel = FFX_API_CONFIGURE_GLOBALDEBUG_LEVEL_WARNINGS;
    }
    else if (mode == FSRDebugCheckerMode::EnabledNoMessageCallbackSilence)
    {
        GlobalDebugConfig.fpMessage = nullptr;
        GlobalDebugConfig.debugLevel = FFX_API_CONFIGURE_GLOBALDEBUG_LEVEL_SILENCE;
    }
    else if (mode == FSRDebugCheckerMode::EnabledNoMessageCallbackErrors)
    {
        GlobalDebugConfig.fpMessage = nullptr;
        GlobalDebugConfig.debugLevel = FFX_API_CONFIGURE_GLOBALDEBUG_LEVEL_ERRORS;
    }
    else if (mode == FSRDebugCheckerMode::EnabledNoMessageCallbackWarnings)
    {
        GlobalDebugConfig.fpMessage = nullptr;
        GlobalDebugConfig.debugLevel = FFX_API_CONFIGURE_GLOBALDEBUG_LEVEL_WARNINGS;
    }

    else if (mode == FSRDebugCheckerMode::EnabledWithMessageCallbackSilence)
    {
        GlobalDebugConfig.fpMessage = &sFSR::FfxMsgCallback;
        GlobalDebugConfig.debugLevel = FFX_API_CONFIGURE_GLOBALDEBUG_LEVEL_SILENCE;
    }
    else if (mode == FSRDebugCheckerMode::EnabledWithMessageCallbackErrors)
    {
        GlobalDebugConfig.fpMessage = &sFSR::FfxMsgCallback;
        GlobalDebugConfig.debugLevel = FFX_API_CONFIGURE_GLOBALDEBUG_LEVEL_ERRORS;
    }
    else if (mode == FSRDebugCheckerMode::EnabledWithMessageCallbackWarnings)
    {
        GlobalDebugConfig.fpMessage = &sFSR::FfxMsgCallback;
        GlobalDebugConfig.debugLevel = FFX_API_CONFIGURE_GLOBALDEBUG_LEVEL_WARNINGS;
    }

    if (UpscalingContext)
    {
        if (recreate)
        {
            // Ask main loop to re-initialize.
            //bNeedReInit = true;
        }

        GlobalDebugConfig.effectId = FFX_API_EFFECT_ID_UPSCALE;
        ffx::ReturnCode retCode = ffx::Configure(UpscalingContext, GlobalDebugConfig);
        Engine::Assert(AssertLevel::ASSERT_ERROR, retCode == ffx::ReturnCode::Ok, L"Couldn't configure global debug config on the ffxapi upscaling context: %d", (uint32_t)retCode);
    }
    if (FrameGenContext)
    {
        if (recreate)
        {
            // Ask main loop to re-initialize.
            //bNeedReInit = true;
        }

        GlobalDebugConfig.effectId = FFX_API_EFFECT_ID_FRAMEGENERATION;
        ffx::ReturnCode retCode = ffx::Configure(FrameGenContext, GlobalDebugConfig);
        Engine::Assert(AssertLevel::ASSERT_ERROR, retCode == ffx::ReturnCode::Ok, L"Couldn't configure global debug config on the ffxapi frame generation context: %d", (uint32_t)retCode);
    }
}

void sFSR::UpdateUpscaleRatio()
{
    if (GPU::GetGIType() == EGITypes::D3D11)
    {
        Engine::WriteToConsole("FSR 3/4 does not support D3D11.");
        return;
    }
    if (bFrameInterpolationActive)
    {
        void* ffxSwapChain = GPU::GetInternalSwapChain();

        FrameGenerationConfig.frameGenerationEnabled = false;
        FrameGenerationConfig.swapChain = ffxSwapChain;
        FrameGenerationConfig.presentCallback = nullptr;
        FrameGenerationConfig.HUDLessColor = FfxApiResource({});
        auto retCode = ffx::Configure(FrameGenContext, FrameGenerationConfig);
        Engine::Assert(AssertLevel::ASSERT_CRITICAL, !!retCode, L"Configuring FSR FG before destroy failed: %d", (uint32_t)retCode);

        SetFrameGenerationEnabled(false);
        SetFrameGenerationEnabled(true);
    }
    if (bUpscalingActive)
    {
        SetUpscaleEnabled(false);
        SetUpscaleEnabled(true);
    }
    //UpdatePreset(nullptr);
}

void sFSR::UpdatePreset(const int32_t* pOldPreset)
{
    if (GPU::GetGIType() == EGITypes::D3D11)
    {
        Engine::WriteToConsole("FSR 3/4 does not support D3D11.");
        return;
    }
    if ((pOldPreset && *pOldPreset == static_cast<int32_t>(ERendererUpscaleMode::DynamicResolution)) || UpscaleMode == ERendererUpscaleMode::DynamicResolution)
    {
        //bNeedReInit = true;
    }

    switch (UpscaleMode)
    {
    case ERendererUpscaleMode::NativeAA:
    case ERendererUpscaleMode::Quality:
    case ERendererUpscaleMode::Balanced:
    case ERendererUpscaleMode::Performance:
    case ERendererUpscaleMode::UltraPerformance:
    {
        // Query the upscale ratio from the provider instead of using hard-coded values
        float queryRatio = 0.0f;
        ffx::ReturnCode retCode;

        ffx::QueryDescUpscaleGetUpscaleRatioFromQualityMode ratioDesc{};
        ratioDesc.qualityMode = static_cast<uint32_t>(UpscaleMode);
        ratioDesc.pOutUpscaleRatio = &queryRatio;

        if (FsrVersionIndex < FsrVersionIds.size() && bOverrideFSRUpscaleVersion)
        {
            ffx::CreateContextDescOverrideVersion versionOverride{};
            versionOverride.versionId = FsrVersionIds[FsrVersionIndex];
            {
                retCode = FSRContext->QueryUpscaleMode(ratioDesc, versionOverride);
                Engine::Assert(AssertLevel::ASSERT_CRITICAL, retCode == ffx::ReturnCode::Ok, 
                    L"ffxQuery(nullptr,UpscaleGetUpscaleRatioFromQualityMode,%S,backend) returned %d", FsrVersionNames[FsrVersionIndex].c_str(), (uint32_t)retCode);
            }
        }
        else
        {
            {
                retCode = FSRContext->QueryUpscaleMode(ratioDesc);
                Engine::Assert(AssertLevel::ASSERT_CRITICAL, retCode == ffx::ReturnCode::Ok, L"ffxQuery(nullptr,UpscaleGetUpscaleRatioFromQualityMode,backend) returned %d", (uint32_t)retCode);
            }
        }

        if (retCode == ffx::ReturnCode::Ok && queryRatio > 0.0f)
        {
            UpscaleRatio = queryRatio;
        }
        break;
    }
	case ERendererUpscaleMode::DynamicResolution:
    case ERendererUpscaleMode::Custom:
        break;
    //case ERendererUpscaleMode::Disabled:
    //    if (OnUpdatePreset)
    //        OnUpdatePreset(sScreenDimension(0, 0), -1.0f);
    //    return;
    default:
        // Leave the upscale ratio at whatever it was
        break;
    }

    // Update whether we can update the custom scale slider
    CustomUpscaleRatioEnabled = (UpscaleMode == ERendererUpscaleMode::Custom || UpscaleMode == ERendererUpscaleMode::DynamicResolution);

    // Update mip bias
    float oldValue = MipBias;
    if (UpscaleMode != ERendererUpscaleMode::Custom && UpscaleMode != ERendererUpscaleMode::DynamicResolution)
        MipBias = cMipBias[static_cast<uint32_t>(UpscaleMode)];
    else
        MipBias = MipBias = CalculateMipBias(UpscaleRatio);

    RenderDimension = UpdateResolution(RenderDimension.DisplayWidth, RenderDimension.DisplayHeight);

    sScreenDimension GeometryPassViewport = sScreenDimension();
    if (bUpscalingActive)
    {
        GeometryPassViewport = RenderDimension.GetRenderDimension();
    }
    if (OnUpdatePreset)
        OnUpdatePreset(GeometryPassViewport, MipBias);
}

float sFSR::GetMipBias() const
{
    return MipBias;
}

float sFSR::GetUpscaleBaseRenderRatio() const
{
    switch (UpscaleMode)
    {
    case ERendererUpscaleMode::NativeAA:
        return 1.0f;
    case ERendererUpscaleMode::Quality:
        return 1.5f;
    case ERendererUpscaleMode::Balanced:
        return 1.7f;
    case ERendererUpscaleMode::Performance:
        return 2.0f;
    case ERendererUpscaleMode::UltraPerformance:
        return 3.0f;
    case ERendererUpscaleMode::Custom:
        return CustomUpscaleRatio;
    }

    return -1.0f;
}

void sFSR::FfxMsgCallback(uint32_t type, const wchar_t* message)
{
    if (type == FFX_API_MESSAGE_TYPE_ERROR)
    {
        Engine::WriteToConsole(std::wstring(L"FSR_API_DEBUG_ERROR: ") + message);
    }
    else if (type == FFX_API_MESSAGE_TYPE_WARNING)
    {
        Engine::WriteToConsole(std::wstring(L"FSR_API_DEBUG_WARNING: ") + message);
    }
}
