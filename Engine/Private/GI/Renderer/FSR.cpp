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

#if FSR_D3D12
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_6.h>
#include "GI/D3D12/D3D12Device.h"
#include "GI/D3D12/D3D12Viewport.h"
#pragma comment(lib, "amd_fidelityfx_loader_dx12.lib")
#elif FSR_VULKAN
//#pragma comment(lib, "amd_fidelityfx_vk.lib")
#endif

#ifndef FSR_DEBUG
#define FSR_DEBUG 0
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
    }
}

sFSR::sFSR(std::size_t Width, std::size_t Height)
	: Super()
    , RenderDimension(sScreenDimension(Width, Height), sScreenDimension(Width, Height), sScreenDimension(Width, Height))
	, GraphicsCommandContext(IGraphicsCommandContext::Create())
    , UpscaleMode(ERendererUpscaleMode::NativeAA)
    , bGenerateReactiveMask(false)
    , CMD(IGraphicsCommandContext::Create())
{
    Output = IRenderTarget::Create("FSR Output", sFrameBuffer(GPU::GetBackBufferFormat(), EFrameBufferAttachmentType::RT_SRV_UAV), sFBODesc(sFBODesc::sFBODimension((std::uint32_t)Width, (std::uint32_t)Height)));
    if (bGenerateReactiveMask)
        GeneratedReactiveMask = IRenderTarget::Create("FSR GeneratedReactiveMask", sFrameBuffer(GPU::GetBackBufferFormat(), EFrameBufferAttachmentType::RT_SRV_UAV), sFBODesc(sFBODesc::sFBODimension((std::uint32_t)Width, (std::uint32_t)Height)));

    {
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
    }

    ffx::CreateContextDescOverrideVersion versionOverride{};
    ffx::ReturnCode retCode;

    // Chain ffxCreateBackendDX12Desc to provide device for null-context queries to reach external driver provider
    struct ffxCreateBackendDX12Desc backendDX12Desc = {};
    backendDX12Desc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12;
#if FSR_D3D12
    backendDX12Desc.device = (ID3D12Device*)GPU::GetInternalDevice();
#elif FSR_VULKAN
    backendDesc.device = GPU::GetInternalDevice();
#endif

    for (size_t index = 0; index < FsrVersionIds.size(); index++)
    {
        versionOverride.versionId = FsrVersionIds[index];
        ffx::QueryDescUpscaleGetResourceRequirements upscaleResourceRequirementsDesc{};
        {
            retCode = ffx::Query(upscaleResourceRequirementsDesc, versionOverride, backendDX12Desc);
            Engine::Assert(AssertLevel::ASSERT_CRITICAL, retCode == ffx::ReturnCode::Ok,
                L"ffxQuery(nullptr,GetResourceRequirements,%S,backend) returned %d", FsrVersionNames[index], (uint32_t)retCode);
        }
        Engine::Print(L"Upscaler %S: required_resources: %llu optional_resources: %llu",
            FsrVersionNames[index], upscaleResourceRequirementsDesc.required_resources, upscaleResourceRequirementsDesc.optional_resources);
    }
}

sFSR::~sFSR()
{
	GraphicsCommandContext = nullptr;

    Output = nullptr;
    GeneratedReactiveMask = nullptr;

    // Destroy the FSR
    SetUpscaleEnabled(false);
    SetFrameGenerationEnabled(false);
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
    if (bEnabled)
    {
        bUpscalingActive = true;
        ffx::CreateBackendDX12Desc backendDesc = {};
        backendDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12;
#if FSR_D3D12
        backendDesc.device = (ID3D12Device*)GPU::GetInternalDevice();
#elif FSR_VULKAN
        backendDesc.device = GPU::GetInternalDevice();
#endif
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

#if FSR_DEBUG
        createFsr.flags |= FFX_UPSCALE_ENABLE_DEBUG_CHECKING;
        createFsr.flags |= FFX_UPSCALE_ENABLE_DEBUG_VISUALIZATION;
#endif
        //Before creating any of FSR contexts, query VRAM size
        struct FfxApiEffectMemoryUsage gpuMemoryUsageUpscaler = {0};
        ffxQueryDescUpscaleGetGPUMemoryUsageV2 upscalerGetGPUMemoryUsageV2 = { 0 };
        upscalerGetGPUMemoryUsageV2.header.type = FFX_API_QUERY_DESC_TYPE_UPSCALE_GPU_MEMORY_USAGE_V2;
        upscalerGetGPUMemoryUsageV2.device = GPU::GetInternalDevice();
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
        if (FsrVersionIndex < FsrVersionIds.size() && bOverrideVersion)
        {
            versionOverride.versionId = FsrVersionIds[FsrVersionIndex];
            upscalerGetGPUMemoryUsageV2.header.pNext = &versionOverride.header;
            ffxReturnCode_t retCode_t = ffxQuery(nullptr, &upscalerGetGPUMemoryUsageV2.header);
            Engine::Assert(AssertLevel::ASSERT_WARNING, retCode_t == FFX_API_RETURN_OK,
                L"ffxQuery(nullptr,UpscaleGetGPUMemoryUsageV2, %S) returned %d", FsrVersionNames[FsrVersionIndex], retCode_t);
            Engine::Print(L"Upscaler version %S Query GPUMemoryUsageV2 VRAM totalUsageInBytes %f MB aliasableUsageInBytes %f MB", FsrVersionNames[FsrVersionIndex], gpuMemoryUsageUpscaler.totalUsageInBytes / 1048576.f, gpuMemoryUsageUpscaler.aliasableUsageInBytes / 1048576.f);

            versionOverride.header.pNext = &backendDesc.header;
            renderResDesc.header.pNext = &versionOverride.header;
            // Query render resolution from display resolution via driver override path (null-context)
            retCode_t = ffxQuery(nullptr, &renderResDesc.header);
            Engine::Assert(AssertLevel::ASSERT_WARNING, retCode_t == FFX_API_RETURN_OK,
                L"ffxQuery(nullptr,QueryDescUpscaleGetRenderResolutionFromQualityMode, %S) returned %d", FsrVersionNames[FsrVersionIndex], retCode_t);
            Engine::Print(L"Upscaler version %S QueryDescUpscaleGetRenderResolutionFromQualityMode qualityMode %d renderResolution %d x %d", FsrVersionNames[FsrVersionIndex], static_cast<uint32_t>(UpscaleMode), renderResolution[0], renderResolution[1]);

            ffx::ReturnCode retCode = ffx::CreateContext(UpscalingContext, nullptr, createFsr, backendDesc, headerVersion, versionOverride);
        }
        else
        {
            ffx::ReturnCode retCode = ffx::Query(upscalerGetGPUMemoryUsageV2);
            Engine::Assert(AssertLevel::ASSERT_WARNING, retCode == ffx::ReturnCode::Ok,
                L"ffxQuery(nullptr,UpscaleGetGPUMemoryUsageV2) returned %d", (uint32_t)retCode);
            Engine::Print(L"Default Upscaler Query GPUMemoryUsageV2 totalUsageInBytes %f MB aliasableUsageInBytes %f MB", gpuMemoryUsageUpscaler.totalUsageInBytes / 1048576.f, gpuMemoryUsageUpscaler.aliasableUsageInBytes / 1048576.f);

            ffxReturnCode_t retCode_t = ffxQuery(nullptr, &renderResDesc.header);
            Engine::Assert(AssertLevel::ASSERT_WARNING, retCode_t == FFX_API_RETURN_OK,
                L"ffxQuery(nullptr,QueryDescUpscaleGetRenderResolutionFromQualityMode, %S) returned %d", FsrVersionNames[FsrVersionIndex], retCode_t);
            Engine::Print(L"Default Upscaler QueryDescUpscaleGetRenderResolutionFromQualityMode qualityMode %d renderResolution %d x %d", static_cast<uint32_t>(UpscaleMode), renderResolution[0], renderResolution[1]);

            ffx::ReturnCode UpscalingRetCode = ffx::CreateContext(UpscalingContext, nullptr, createFsr, backendDesc, headerVersion);
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
    Sharpness = NewSharpness;
}

void sFSR::SetUpscaleMode(ERendererUpscaleMode Mode)
{
    UpscaleMode = Mode;
    UpdatePreset(nullptr);
}

void sFSR::SetFrameGenerationEnabled(bool bEnabled)
{
    /*{
        ffx::ReturnCode retCode;
        FfxApiEffectMemoryUsage gpuMemoryUsageSwapChainV2;
        ffxQueryFrameGenerationSwapChainGetGPUMemoryUsageDX12V2 swapChainGetGPUMemoryUsageV2{};
        D3D12Device* Device = (D3D12Device*)GPU::GetDevice();
        D3D12Viewport* Viewport = (D3D12Viewport*)Device->GetD3D12Viewport();
        swapChainGetGPUMemoryUsageV2.device = GPU::GetInternalDevice();
        IDXGISwapChain4* dxgiSwapchain = Viewport->GetSwapChainPtr();
        DXGI_SWAP_CHAIN_DESC1 desc1;
        dxgiSwapchain->GetDesc1(&desc1);
        FfxApiDimensions2D swapChainDescDisplaySize;
        swapChainDescDisplaySize.width = desc1.Width;
        swapChainDescDisplaySize.height = desc1.Height;
        swapChainGetGPUMemoryUsageV2.displaySize = swapChainDescDisplaySize;
        swapChainGetGPUMemoryUsageV2.backBufferFormat = ffxApiGetSurfaceFormatDX12(desc1.Format);
        swapChainGetGPUMemoryUsageV2.backBufferCount = desc1.BufferCount;

        FfxApiResource uiColor = SDKWrapper::ffxGetResourceApi(m_pUiTexture[0]->GetResource(), FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
        FfxApiDimensions2D uiResourceSize;
        uiResourceSize.width = (s_uiRenderMode == UICompositionMode::UiTexture) ? uiColor.description.width : 0;
        uiResourceSize.height = (s_uiRenderMode == UICompositionMode::UiTexture) ? uiColor.description.height : 0;
        swapChainGetGPUMemoryUsageV2.uiResourceSize = uiResourceSize;
        swapChainGetGPUMemoryUsageV2.uiResourceFormat = (s_uiRenderMode == UICompositionMode::UiTexture) ? uiColor.description.format : FFX_API_SURFACE_FORMAT_UNKNOWN;
        swapChainGetGPUMemoryUsageV2.flags = m_DoublebufferInSwapchain ? FFX_FRAMEGENERATION_UI_COMPOSITION_FLAG_ENABLE_INTERNAL_UI_DOUBLE_BUFFERING : 0;
        swapChainGetGPUMemoryUsageV2.gpuMemoryUsageFrameGenerationSwapchain = &gpuMemoryUsageSwapChainV2;

        retCode = ffx::Query(swapChainGetGPUMemoryUsageV2);
        Engine::Assert(AssertLevel::ASSERT_WARNING, retCode == ffx::ReturnCode::Ok,
            L"ffxQuery(nullptr,SwapChainGetGPUMemoryUsageDX12V2) returned %d", (uint32_t)retCode);
        Engine::Print(L"Swapchain Query GPUMemoryUsageV2 VRAM totalUsageInBytes %f MB aliasableUsageInBytes %f MB", gpuMemoryUsageSwapChainV2.totalUsageInBytes / 1048576.f, gpuMemoryUsageSwapChainV2.aliasableUsageInBytes / 1048576.f);
    }*/

    if (bEnabled)
    {
        bFrameInterpolationActive = true;
        EnableFrameInterpolationSwapchain(true);
        ffx::CreateBackendDX12Desc backendDesc = {};
        backendDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12;
#if FSR_D3D12
        backendDesc.device = (ID3D12Device*)GPU::GetInternalDevice();
#elif FSR_VULKAN
        backendDesc.device = GPU::GetInternalDevice();
#endif
        ffx::CreateContextDescFrameGeneration createFg{};
        createFg.displaySize = { (std::uint32_t)RenderDimension.DisplayWidth, (std::uint32_t)RenderDimension.DisplayWidth };
        createFg.maxRenderSize = { (std::uint32_t)RenderDimension.DisplayWidth, (std::uint32_t)RenderDimension.DisplayWidth };
        if (bInvertedDepth)
            createFg.flags |= FFX_FRAMEGENERATION_ENABLE_DEPTH_INVERTED | FFX_FRAMEGENERATION_ENABLE_DEPTH_INFINITE;
        createFg.flags |= FFX_FRAMEGENERATION_ENABLE_HIGH_DYNAMIC_RANGE;

        if (bEnableAsyncCompute)
        {
            createFg.flags |= FFX_FRAMEGENERATION_ENABLE_ASYNC_WORKLOAD_SUPPORT;
        }

#if FSR_DEBUG
        createFg.flags |= FFX_FRAMEGENERATION_ENABLE_DEBUG_CHECKING;
#endif

        D3D12Device* Device = (D3D12Device*)GPU::GetDevice();
        D3D12Viewport* Viewport = (D3D12Viewport*)Device->GetD3D12Viewport();

        createFg.backBufferFormat = GetFfxSurfaceFormat(Viewport->GetBackBufferFormat());
        FrameGenerationConfig.flags = 0u;
#if FSR_DEBUG
        //FrameGenerationConfig.flags |= m_DrawFrameGenerationDebugTearLines ? FFX_FRAMEGENERATION_FLAG_DRAW_DEBUG_TEAR_LINES : 0;
        //FrameGenerationConfig.flags |= m_DrawFrameGenerationDebugResetIndicators ? FFX_FRAMEGENERATION_FLAG_DRAW_DEBUG_RESET_INDICATORS : 0;
        //FrameGenerationConfig.flags |= m_DrawFrameGenerationDebugPacingLines ? FFX_FRAMEGENERATION_FLAG_DRAW_DEBUG_PACING_LINES : 0;
        //FrameGenerationConfig.flags |= m_DrawFrameGenerationDebugView ? FFX_FRAMEGENERATION_FLAG_DRAW_DEBUG_VIEW : 0;
#endif
        FrameGenerationConfig.flags |= !SwapChainContext ? FFX_FRAMEGENERATION_FLAG_NO_SWAPCHAIN_CONTEXT_NOTIFY : 0;

        //Before creating FG context, query VRAM size
        FfxApiEffectMemoryUsage gpuMemoryUsageFrameGenerationV2;
        ffx::QueryDescFrameGenerationGetGPUMemoryUsageV2 frameGenGetGPUMemoryUsageV2{};
        frameGenGetGPUMemoryUsageV2.device = GPU::GetInternalDevice();
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
            /*ffx::CreateContextDescFrameGenerationHudless createFgHudless{};
            createFgHudless.hudlessBackBufferFormat = SDKWrapper::GetFfxSurfaceFormat(m_pHudLessTexture[0]->GetResource()->GetTextureResource()->GetFormat());
            frameGenGetGPUMemoryUsageV2.backBufferFormat = createFgHudless.hudlessBackBufferFormat;

            if (FgVersionIndex < FgVersionIds.size() && bOverrideVersion)
            {
                struct ffxOverrideVersion versionOverride = { 0 };
                versionOverride.header.type = FFX_API_DESC_TYPE_OVERRIDE_VERSION;
                versionOverride.versionId = FgVersionIds[FgVersionIndex];
                retCode = ffx::Query(frameGenGetGPUMemoryUsageV2, versionOverride);
                Engine::Assert(AssertLevel::ASSERT_WARNING, retCode == ffx::ReturnCode::Ok, L"ffx::Query(FrameGenerationGetGPUMemoryUsageV2, %S) returned %d", FsrVersionNames[FsrVersionIndex], (uint32_t)retCode);
                retCode = ffx::CreateContext(FrameGenContext, nullptr, createFg, backendDesc, headerVersion, createFgHudless, versionOverride);
            }
            else
            {
                retCode = ffx::Query(frameGenGetGPUMemoryUsageV2);
                Engine::Assert(AssertLevel::ASSERT_WARNING, retCode == ffx::ReturnCode::Ok, L"ffx::Query(FrameGenerationGetGPUMemoryUsageV2) returned %d", (uint32_t)retCode);
                retCode = ffx::CreateContext(FrameGenContext, nullptr, createFg, backendDesc, headerVersion, createFgHudless);
            }*/
        }
        else
        {
            frameGenGetGPUMemoryUsageV2.hudlessBackBufferFormat = FFX_API_SURFACE_FORMAT_UNKNOWN;

            if (FgVersionIndex < FgVersionIds.size() && bOverrideVersion)
            {
                struct ffxOverrideVersion versionOverride = { 0 };
                versionOverride.header.type = FFX_API_DESC_TYPE_OVERRIDE_VERSION;
                versionOverride.versionId = FgVersionIds[FgVersionIndex];
                retCode = ffx::Query(frameGenGetGPUMemoryUsageV2, versionOverride);
                Engine::Assert(AssertLevel::ASSERT_WARNING, retCode == ffx::ReturnCode::Ok, L"ffx::Query(FrameGenerationGetGPUMemoryUsageV2, %S) returned %d", FsrVersionNames[FsrVersionIndex], (uint32_t)retCode);
                retCode = ffx::CreateContext(FrameGenContext, nullptr, createFg, backendDesc, headerVersion, versionOverride);
            }
            else
            {
                retCode = ffx::Query(frameGenGetGPUMemoryUsageV2);
                Engine::Assert(AssertLevel::ASSERT_WARNING, retCode == ffx::ReturnCode::Ok, L"ffx::Query(FrameGenerationGetGPUMemoryUsageV2) returned %d", (uint32_t)retCode);
                retCode = ffx::CreateContext(FrameGenContext, nullptr, createFg, backendDesc, headerVersion);
            }

        }

        Engine::Assert(AssertLevel::ASSERT_CRITICAL, retCode == ffx::ReturnCode::Ok, L"Couldn't create the ffxapi framegen context: %d", (uint32_t)retCode);

        Engine::Print(L"FrameGenerationGetGPUMemoryUsageV2 VRAM totalUsageInBytes %f MB aliasableUsageInBytes %f MB", gpuMemoryUsageFrameGenerationV2.totalUsageInBytes / 1048576.f, gpuMemoryUsageFrameGenerationV2.aliasableUsageInBytes / 1048576.f);

        void* ffxSwapChain = GPU::GetInternalSwapChain();

        // Configure frame generation
        //FfxApiResource hudLessResource = SDKWrapper::ffxGetResourceApi(m_pHudLessTexture[m_curUiTextureIndex]->GetResource(), FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);

        FrameGenerationConfig.frameGenerationEnabled = false;
        FrameGenerationConfig.frameGenerationCallback = [](ffxDispatchDescFrameGeneration* params, void* pUserCtx) -> ffxReturnCode_t
            {
                return ffxDispatch(reinterpret_cast<ffxContext*>(pUserCtx), &params->header);
            };
        FrameGenerationConfig.frameGenerationCallbackUserContext = &FrameGenContext;
        if (UIRenderMode == UICompositionMode::UiCallback)
        {
            FrameGenerationConfig.presentCallback = nullptr;
            FrameGenerationConfig.presentCallbackUserContext = nullptr;
            //FrameGenerationConfig.presentCallback = [](ffxCallbackDescFrameGenerationPresent* params, void* self) -> auto { return reinterpret_cast<FSRRenderModule*>(self)->UiCompositionCallback(params); };
            //FrameGenerationConfig.presentCallbackUserContext = this;
        }
        else
        {
            FrameGenerationConfig.presentCallback = nullptr;
            FrameGenerationConfig.presentCallbackUserContext = nullptr;
        }
        FrameGenerationConfig.swapChain = ffxSwapChain;
        FrameGenerationConfig.HUDLessColor = /*(UIRenderMode == UICompositionMode::PreUiBackbuffer) ? hudLessResource : */FfxApiResource({});

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

void sFSR::EnableFrameInterpolationSwapchain(bool enabled)
{
    if (enabled)
    {
        bEnableFrameInterpolationSwapchain = true;
#if FSR_D3D12
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

#elif FSR_VULKAN
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

        ffx::ReturnCode retCode = ffx::CreateContext(m_SwapChainContext, nullptr, createSwapChainDesc);

        ffx::QueryDescSwapchainReplacementFunctionsVK replacementFunctions{};
        ffx::Query(m_SwapChainContext, replacementFunctions);
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
            m_SwapChainContext,
            &frameInterpolationInfo);

        // Set frameinterpolation swapchain to engine
        cauldron::GetFramework()->GetSwapChain()->GetImpl()->SetVKSwapChain(currentSwapchain, true);

        // we need to re initialize HDR info since swapchain was re created
        cauldron::GetSwapChain()->SetHDRMetadataAndColorspace();
#endif  // defined(FFX_API_DX12)
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
    ffxQueryGetProviderVersion Version = { 0 };
    Version.header.type = FFX_API_QUERY_DESC_TYPE_GET_PROVIDER_VERSION;
    ffx::ReturnCode retCode = ffx::Query(UpscalingContext, Version);
    return Version;
}

ffx::QueryDescUpscaleGetGPUMemoryUsage sFSR::GetUpscaleGetGPUMemoryUsage()
{
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
    if (pCamera)
    {
        if (!pCamera->IsJitterEnabled())
        {
            pCamera->SetEnableJitter(true);
            pCamera->SetJitterCallBack(std::bind(&sFSR::SetJitter, this, std::placeholders::_1));
        }
    }

    GraphicsCommandContext->BeginRecordCommandList(ECommandContextBeginState::ApiRender);

    Color->AsResource(EResourceState::NonPixelShaderResource, GraphicsCommandContext.get());
    MotionVectors->AsResource(EResourceState::NonPixelShaderResource, GraphicsCommandContext.get());
    //Depth->AsResource(EResourceState::NonPixelShaderResource, GraphicsCommandContext.get());
    Output->AsResource(EResourceState::UAV, GraphicsCommandContext.get());

    ffx::DispatchDescUpscale DispatchUpscale{};
    DispatchUpscale.commandList = (ID3D12GraphicsCommandList8*)GraphicsCommandContext->GetInternalCommandContext();
    DispatchUpscale.color = ffxApiGetResourceDX12((ID3D12Resource*)Color->GetNativeTexture(), FFX_API_RESOURCE_STATE_COMPUTE_READ);
    DispatchUpscale.depth = ffxApiGetResourceDX12((ID3D12Resource*)Depth->GetNativeTexture(), FFX_API_RESOURCE_STATE_COMPUTE_READ);
    DispatchUpscale.motionVectors = ffxApiGetResourceDX12((ID3D12Resource*)MotionVectors->GetNativeTexture(), FFX_API_RESOURCE_STATE_COMPUTE_READ);
    if (bGenerateReactiveMask)
    {
        GeneratedReactiveMask->AsResource(EResourceState::NonPixelShaderResource, GraphicsCommandContext.get());
        DispatchUpscale.reactive = ffxApiGetResourceDX12((ID3D12Resource*)GeneratedReactiveMask->GetNativeTexture(), FFX_API_RESOURCE_STATE_COMPUTE_READ);
    }
    else
    {
        //Reactive->AsResource(EResourceState::NonPixelShaderResource, GraphicsCommandContext.get());
        //DispatchUpscale.reactive = ffxApiGetResourceDX12((ID3D12Resource*)Reactive->GetNativeTexture(), FFX_API_RESOURCE_STATE_COMPUTE_READ);
    }
    DispatchUpscale.exposure = ffxApiGetResourceDX12(nullptr, FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
    DispatchUpscale.transparencyAndComposition = ffxApiGetResourceDX12(nullptr, FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
    DispatchUpscale.output = ffxApiGetResourceDX12((ID3D12Resource*)Output->GetNativeTexture(), FFX_API_RESOURCE_STATE_UNORDERED_ACCESS);
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
#if FSR_DEBUG
    DispatchUpscale.flags |= FFX_UPSCALE_FLAG_DRAW_DEBUG_VIEW;
#endif
    DispatchUpscale.flags |= ColorSpace == FSRColorSpace::sRGBColorSpace ? FFX_UPSCALE_FLAG_NON_LINEAR_COLOR_SRGB : 0;
    DispatchUpscale.flags |= ColorSpace == FSRColorSpace::PQColorSpace ? FFX_UPSCALE_FLAG_NON_LINEAR_COLOR_PQ : 0;
    bReset = false;

    auto Ret = ffx::Dispatch(UpscalingContext, DispatchUpscale);

    //Color->AsRenderTarget(GraphicsCommandContext.get());
    //MotionVectors->AsRenderTarget(GraphicsCommandContext.get());
    //Depth->AsDepthTarget(GraphicsCommandContext.get());

    GraphicsCommandContext->FinishRecordCommandList();
    GraphicsCommandContext->ExecuteCommandList(ECommandContextExecuteType::Deferred, 6);

    //CopyToFrameBuffer(Color);
}

void sFSR::GenerateReactiveMask(IRenderTarget* Color)
{
    CMD->BeginRecordCommandList(ECommandContextBeginState::ApiRender);

    GeneratedReactiveMask->AsResource(EResourceState::UAV, CMD.get());

    ffx::DispatchDescUpscaleGenerateReactiveMask dispatchDesc{};
    dispatchDesc.commandList = (ID3D12GraphicsCommandList8*)CMD->GetInternalCommandContext();
    dispatchDesc.colorOpaqueOnly = ffxApiGetResourceDX12((ID3D12Resource*)Color->GetNativeTexture(), FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
    dispatchDesc.colorPreUpscale = ffxApiGetResourceDX12((ID3D12Resource*)Output->GetNativeTexture(), FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);
    dispatchDesc.outReactive = ffxApiGetResourceDX12((ID3D12Resource*)GeneratedReactiveMask->GetNativeTexture(), FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ);

    dispatchDesc.renderSize.width = RenderDimension.RenderWidth;
    dispatchDesc.renderSize.height = RenderDimension.RenderHeight;

    // The following are all hard-coded in the original FSR2 sample. Should these be exposed?
    dispatchDesc.scale = 1.f;
    dispatchDesc.cutoffThreshold = 0.2f;
    dispatchDesc.binaryValue = 0.9f;
    dispatchDesc.flags = FFX_UPSCALE_AUTOREACTIVEFLAGS_APPLY_TONEMAP |
        FFX_UPSCALE_AUTOREACTIVEFLAGS_APPLY_THRESHOLD |
        FFX_UPSCALE_AUTOREACTIVEFLAGS_USE_COMPONENTS_MAX;

    ffx::ReturnCode retCode = ffx::Dispatch(UpscalingContext, dispatchDesc);
    Engine::Assert(AssertLevel::ASSERT_ERROR, retCode == ffx::ReturnCode::Ok, L"ffxDispatch(FSR_GENERATEREACTIVEMASK) failed with %d", (uint32_t)retCode);

    CMD->FinishRecordCommandList();
    CMD->ExecuteCommandList(/*ECommandContextExecuteType::Deferred, 5*/);
}

void sFSR::CopyToFrameBuffer(IRenderTarget* Target)
{
    //GraphicsCommandContext->BeginRecordCommandList();
    GraphicsCommandContext->CopyRenderTarget(Target, Output.get());
    GraphicsCommandContext->FinishRecordCommandList();
    GraphicsCommandContext->ExecuteCommandList(ECommandContextExecuteType::Deferred, 4);
}

void sFSR::SetRenderSize(std::size_t InWidth, std::size_t InHeight)
{
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
    ffx::ConfigureDescGlobalDebug1 GlobalDebugConfig{};
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
            bNeedReInit = true;
        }

        ffx::ReturnCode retCode = ffx::Configure(UpscalingContext, GlobalDebugConfig);
        Engine::Assert(AssertLevel::ASSERT_ERROR, retCode == ffx::ReturnCode::Ok, L"Couldn't configure global debug config on the ffxapi upscaling context: %d", (uint32_t)retCode);
    }
    if (FrameGenContext)
    {
        if (recreate)
        {
            // Ask main loop to re-initialize.
            bNeedReInit = true;
        }

        ffx::ReturnCode retCode = ffx::Configure(FrameGenContext, GlobalDebugConfig);
        Engine::Assert(AssertLevel::ASSERT_ERROR, retCode == ffx::ReturnCode::Ok, L"Couldn't configure global debug config on the ffxapi frame generation context: %d", (uint32_t)retCode);
    }
}

void sFSR::UpdateUpscaleRatio()
{
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
    if ((pOldPreset && *pOldPreset == static_cast<int32_t>(ERendererUpscaleMode::DynamicResolution)) || UpscaleMode == ERendererUpscaleMode::DynamicResolution)
    {
        bNeedReInit = true;
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

#if FSR_D3D12
        // Chain ffxCreateBackendDX12Desc to provide device for null-context query to reach external driver provider
        struct ffxCreateBackendDX12Desc backendDX12Desc = {};
        backendDX12Desc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12;
        backendDX12Desc.device = (ID3D12Device*)GPU::GetInternalDevice();
#elif FSR_VULKAN
#endif

        ffx::QueryDescUpscaleGetUpscaleRatioFromQualityMode ratioDesc{};
        ratioDesc.qualityMode = static_cast<uint32_t>(UpscaleMode);
        ratioDesc.pOutUpscaleRatio = &queryRatio;

        if (FsrVersionIndex < FsrVersionIds.size() && bOverrideVersion)
        {
            ffx::CreateContextDescOverrideVersion versionOverride{};
            versionOverride.versionId = FsrVersionIds[FsrVersionIndex];
            {
                retCode = ffx::Query(ratioDesc, versionOverride, backendDX12Desc);
                Engine::Assert(AssertLevel::ASSERT_CRITICAL, retCode == ffx::ReturnCode::Ok, 
                    L"ffxQuery(nullptr,UpscaleGetUpscaleRatioFromQualityMode,%S,backend) returned %d", FsrVersionNames[FsrVersionIndex], (uint32_t)retCode);
            }
        }
        else
        {
            {
                retCode = ffx::Query(ratioDesc, backendDX12Desc);
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

    if (OnUpdatePreset)
        OnUpdatePreset(RenderDimension.GetRenderDimension(), MipBias);
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
