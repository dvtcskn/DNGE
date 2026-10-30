#pragma once

#include "FSR.h"

class VulkanFSRContext : public IFSRContext
{
    sClassBody(sClassConstructor, VulkanFSRContext, IFSRContext)
public:
    VulkanFSRContext();
    virtual ~VulkanFSRContext();

    virtual void* GetDevice() const override final;
    virtual const wchar_t* GetVersionQueryName(ffxStructType_t createDescType) const override final;
    virtual void QueryVersionNames(ffxStructType_t createDescType, std::vector<uint64_t>& versionIds, std::vector<std::string>& versionNames) const override final;
    virtual FfxApiResource GetResource(IRenderTarget* RT, FfxApiResourceState State) override final;
    virtual FfxApiResource GetDepthResource(IDepthTarget* Depth, FfxApiResourceState State) override final;
    virtual FfxApiResource GetCurrentBackBuffer(FfxApiResourceState State) override final;
    virtual ffx::ReturnCode Query(ffx::QueryDescUpscaleGetResourceRequirements& ResourceRequirements, ffx::CreateContextDescOverrideVersion OverrideVersion) override final;
    virtual ffx::ReturnCode QueryUpscaleMode(ffx::QueryDescUpscaleGetUpscaleRatioFromQualityMode UpscaleRatioFromQualityMode, std::optional<ffx::CreateContextDescOverrideVersion> OverrideVersion = std::nullopt) override final;
    virtual ffx::ReturnCode CreateUpscalingContext(ffx::Context& UpscalingContext, ffx::CreateContextDescUpscale Desc, ffxCreateContextDescUpscaleVersion Version, std::optional<ffxOverrideVersion> OverrideVersion = std::nullopt) override final;
    virtual ffx::ReturnCode CreateFrameGenContext(ffx::Context& UpscalingContext, ffx::CreateContextDescFrameGeneration Desc, ffx::CreateContextDescFrameGenerationVersion Version, std::optional<ffxOverrideVersion> OverrideVersion = std::nullopt) override final;
    virtual ffx::ReturnCode CreateHudlessFrameGenContext(ffx::Context& UpscalingContext, ffx::CreateContextDescFrameGeneration Desc, ffx::CreateContextDescFrameGenerationHudless FgHudless, ffx::CreateContextDescFrameGenerationVersion Version, std::optional<ffxOverrideVersion> OverrideVersion = std::nullopt) override final;
    virtual ffx::ReturnCode GetSwapChainGPUMemoryUsage(FfxApiEffectMemoryUsage* MemoryUsage, FfxApiDimensions2D Dimension, uint32_t Format, uint32_t Flags) override final;

    virtual void CreateSwapChainContext(ffx::Context& SwapChainContext, bool bGetLatencyWaitableObject = false, HANDLE LatencyWaitableObj = 0) override final;
    virtual ffx::ReturnCode GetSwapChainGetGPUMemoryUsage(ffx::Context SwapChainContext, FfxApiEffectMemoryUsage& gpuMemoryUsageFrameGenerationSwapchain) override final;
    virtual void ConfigureSwapChain(ffx::Context& SwapChainContext, FfxApiResource uiColor, bool bDoublebufferInSwapchain) override final;
    virtual ffx::ReturnCode QuerySwapChainCommandList(ffx::Context& SwapChainContext, void** CommandList) override final;
    virtual ffx::ReturnCode QuerySwapChainInterpolationTexture(ffx::Context& SwapChainContext, FfxApiResource* Texture) override final;

private:
    class InternalFSRContext;
    InternalFSRContext* InternalContext;
};