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

#include "AbstractGI/Material.h"
#include "AbstractGI/UIMaterialStyle.h"
#include "Gameplay/ICanvas.h"
#include "Utilities/Input.h"
#include "Gameplay/CameraManager.h"
#include <mutex>
#include <ffx_upscale.hpp>
#include <ffx_api.hpp>
#include <ffx_api_types.h>
#include <ffx_denoiser.hpp>
#include <ffx_framegeneration.hpp>
#include <ffx_framegeneration_api_types.h>
#include <ffx_radiancecache.hpp>

enum UICompositionMode : std::uint32_t
{
    No_UI_Handling = 0,
    UiTexture = 1,
    UiCallback = 2,
    PreUiBackbuffer = 3, // HudLess
};

enum class FSRColorSpace
{
    DefaultLinearColorSpace = 0,
    NonLinearColorSpace,
    sRGBColorSpace,
    PQColorSpace
};

class IFSRContext
{
    sBaseClassBody(sClassDefaultProtectedConstructor, IFSRContext)
public:
    virtual void* GetDevice() const = 0;
    virtual const wchar_t* GetVersionQueryName(ffxStructType_t createDescType) const = 0;
    virtual void QueryVersionNames(ffxStructType_t createDescType, std::vector<uint64_t>& versionIds, std::vector<std::string>& versionNames) const = 0;
    virtual FfxApiResource GetResource(IRenderTarget* RT, FfxApiResourceState State) = 0;
    virtual FfxApiResource GetDepthResource(IDepthTarget* Depth, FfxApiResourceState State) = 0;
    virtual FfxApiResource GetCurrentBackBuffer(FfxApiResourceState State) = 0;
    virtual ffx::ReturnCode Query(ffx::QueryDescUpscaleGetResourceRequirements& ResourceRequirements, ffx::CreateContextDescOverrideVersion OverrideVersion) = 0;
    virtual ffx::ReturnCode QueryUpscaleMode(ffx::QueryDescUpscaleGetUpscaleRatioFromQualityMode UpscaleRatioFromQualityMode, std::optional<ffx::CreateContextDescOverrideVersion> OverrideVersion = std::nullopt) = 0;
    virtual ffx::ReturnCode CreateUpscalingContext(ffx::Context& UpscalingContext, ffx::CreateContextDescUpscale Desc, ffxCreateContextDescUpscaleVersion Version, std::optional<ffxOverrideVersion> OverrideVersion = std::nullopt) = 0;
    virtual ffx::ReturnCode CreateFrameGenContext(ffx::Context& UpscalingContext, ffx::CreateContextDescFrameGeneration Desc, ffx::CreateContextDescFrameGenerationVersion Version, std::optional<ffxOverrideVersion> OverrideVersion = std::nullopt) = 0;
    virtual ffx::ReturnCode CreateHudlessFrameGenContext(ffx::Context& UpscalingContext, ffx::CreateContextDescFrameGeneration Desc, ffx::CreateContextDescFrameGenerationHudless FgHudless, ffx::CreateContextDescFrameGenerationVersion Version, std::optional<ffxOverrideVersion> OverrideVersion = std::nullopt) = 0;
    virtual ffx::ReturnCode GetSwapChainGPUMemoryUsage(FfxApiEffectMemoryUsage* MemoryUsage, FfxApiDimensions2D Dimension, uint32_t Format, uint32_t Flags) = 0;

    virtual void CreateSwapChainContext(ffx::Context& SwapChainContext, bool bGetLatencyWaitableObject = false, HANDLE LatencyWaitableObj = 0) = 0;
    virtual ffx::ReturnCode GetSwapChainGetGPUMemoryUsage(ffx::Context SwapChainContext, FfxApiEffectMemoryUsage& gpuMemoryUsageFrameGenerationSwapchain) = 0;
    virtual void ConfigureSwapChain(ffx::Context& SwapChainContext, FfxApiResource uiColor, bool bDoublebufferInSwapchain) = 0;
    virtual ffx::ReturnCode QuerySwapChainCommandList(ffx::Context& SwapChainContext, void** CommandList) = 0;
    virtual ffx::ReturnCode QuerySwapChainInterpolationTexture(ffx::Context& SwapChainContext, FfxApiResource* Texture) = 0;
};

class sFSR : public IRenderPass
{
	sClassBody(sClassConstructor, sFSR, IRenderPass)
public:
    sFSR(std::size_t Width, std::size_t Height);
    virtual ~sFSR();

    virtual void BeginPlay() override final;
    virtual void Tick(const double DeltaTime) override final;

    void OnEnabled(bool bEnableUpscale, bool bEnableFrameGen);
    void OnDisabled();

	void SetUpscaleEnabled(bool bEnabled);
	void SetUpscaleSharpness(float Sharpness);
	void SetUpscaleMode(ERendererUpscaleMode Mode);
    inline ERendererUpscaleMode GetUpscaleMode() const { return UpscaleMode; }
	void SetFrameGenerationEnabled(bool bEnabled);
    void SetAsyncComputeEnabled(bool bEnabled);

	ffxQueryGetProviderVersion GetVersion() /*const*/;
	ffx::QueryDescUpscaleGetGPUMemoryUsage GetUpscaleGetGPUMemoryUsage() /*const*/;

    void Render(IRenderTarget* Color, IRenderTarget* MotionVectors, IRenderTarget* Reactive, IDepthTarget* Depth, ICamera* pCamera);
    void GenerateReactiveMask(IRenderTarget* Color);
    void CopyToFrameBuffer(IRenderTarget* Target);
    inline IRenderTarget* GetOutputRenderTarget() const
    {
        return Output.get();
    }
    inline IRenderTarget* GetHudlessTextures() const
    {
        return HudlessTextures.Get();
    }

    virtual void SetRenderSize(std::size_t Width, std::size_t Height) override final;
    virtual void OnInputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar) override final;

    void UpdatePreset(const int32_t* pOldPreset);

    bool SetJitter(FVector2& Vec);

    float GetMipBias() const;

    std::function<void(sScreenDimension, float)> OnUpdatePreset;

    void CopyHudlessTexture(IRenderTarget* HudlessTexture);

    UICompositionMode GetUICompositionMode() const
    {
        return UIRenderMode;
    }

    bool IsUpscalingActive() const
    {
        return bUpscalingActive;
    }

    bool IsFrameGenActive() const
    {
        return bFrameInterpolationActive;
    }

private:
    ffxReturnCode_t UiCompositionCallback(ffxCallbackDescFrameGenerationPresent* Params);
    void SetUpscaleConstantBuffer(uint64_t key, float value);
    float GetUpscaleBaseRenderRatio() const;

    static void FfxMsgCallback(uint32_t type, const wchar_t* message);

private:
    IFSRContext::UniquePtr FSRContext;
    IGraphicsCommandContext::SharedPtr GraphicsCommandContext;
    IGraphicsCommandContext::SharedPtr FrameGenCommandContext;
    IGraphicsCommandContext::SharedPtr FSRCopyCommandContext;
    sRenderDimension RenderDimension;
    IRenderTarget::SharedPtr Output;
    IRenderTarget::SharedPtr GeneratedReactiveMask;
    IRenderTarget::SharedPtr InterpolationOutput;
    bool bGenerateReactiveMask = false;
    std::mutex Mutex;

    TDoubleBuffer<IRenderTarget, true> HudlessTextures;

private:
    // FSR
    sRenderDimension UpdateResolution(uint32_t displayWidth, uint32_t displayHeight);

    const float cMipBias[static_cast<uint32_t>(ERendererUpscaleMode::Custom)] = {
        std::log2f(1.f / 1.0f) - 1.f + std::numeric_limits<float>::epsilon(),
        std::log2f(1.f / 1.5f) - 1.f + std::numeric_limits<float>::epsilon(),
        std::log2f(1.f / 1.7f) - 1.f + std::numeric_limits<float>::epsilon(),
        std::log2f(1.f / 2.0f) - 1.f + std::numeric_limits<float>::epsilon(),
        std::log2f(1.f / 3.0f) - 1.f + std::numeric_limits<float>::epsilon()
    };

    enum FSRDebugCheckerMode
    {
        Disabled = 0,
        EnabledNoMessageCallbackSilence,
        EnabledNoMessageCallbackErrors,
        EnabledNoMessageCallbackWarnings,
        EnabledWithMessageCallbackSilence,
        EnabledWithMessageCallbackErrors,
        EnabledWithMessageCallbackWarnings,
    };
    void SetGlobalDebugCheckerMode(FSRDebugCheckerMode mode, bool recreate);
    enum class FSRMaskMode
    {
        Disabled = 0,
        Manual,
        Auto
    };

    uint32_t JitterFrame = 0;
    uint64_t FrameID = 0;
    //FSRMaskMode MaskMode = FSRMaskMode::Manual;
    FSRDebugCheckerMode GlobalDebugCheckerMode = FSRDebugCheckerMode::Disabled;
    ERendererUpscaleMode UpscaleMode = ERendererUpscaleMode::Balanced;
    std::uint64_t CurrentUpscaleContextVersionId = 0;
    const char* CurrentUpscaleContextVersionName = nullptr;
    Version currentFgContextVersion = {};
    bool bEnableAsyncCompute = true;
    bool bInvertedDepth = true;
    float MipBias = cMipBias[static_cast<uint32_t>(ERendererUpscaleMode::Quality)];
    FSRColorSpace ColorSpace = FSRColorSpace::DefaultLinearColorSpace;

private:
    // FSR Upscale
    bool bUpscalingActive = false;
    ffx::Context UpscalingContext = nullptr;
    bool bReset = true;
    float DT = 0.0f;
    float JitterX = 0.0f;
    float JitterY = 0.0f;
    float Sharpness = 0.8f;
    float UpscaleRatio = 2.f;
    float LetterboxRatio = 1.f;
    float CustomUpscaleRatio = 2.f;
    bool CustomUpscaleRatioEnabled = false;
    bool bOverrideFSRUpscaleVersion = false;
    std::uint32_t FsrVersionIndex = 0;
    std::vector<uint64_t> FsrVersionIds;
    std::vector<std::string> FsrVersionNames;

    void UpdateUpscaleRatio();

private:
    // FSR FrameGen
    bool bFrameInterpolationActive = false;
    UICompositionMode UIRenderMode;
    ffx::Context FrameGenContext = nullptr;
    ffx::Context SwapChainContext = nullptr;
    ffx::ConfigureDescFrameGeneration FrameGenerationConfig;
    bool bSimulatePresentSkip = false;
    bool bResetFrameInterpolation = true;
    std::mutex FGContextMutex;

    bool bOverrideFSRFrameGenVersion = false;
    int32_t FgVersionIndex = 0;
    std::vector<uint64_t> FgVersionIds;
    std::vector<std::string> FgVersionNames;

    bool bEnableFrameInterpolationSwapchain = false;
    void EnableFrameInterpolationSwapchain(bool enabled);

    bool   bGetLatencyWaitableObject = false;
    HANDLE LatencyWaitableObj = 0;

    bool bUseCallback = false; // ??
    bool bUseDistortionField = false; // ??
    bool bDoublebufferInSwapchain = true;
    bool bPresentInterpolatedOnly = false;

    bool bDrawFrameGenerationDebugTearLines = true;
    bool bDrawFrameGenerationDebugResetIndicators = true;
    bool bDrawFrameGenerationDebugPacingLines = true;
    bool bDrawFrameGenerationDebugView = true;






    //////////////////////////
    /*
    * New variables
    */
    float           m_LetterboxRatioX = 1.0f;
    float           m_LetterboxRatioY = 1.0f;
    float           m_GenerationRectRatioX = 1.0f;
    float           m_GenerationRectRatioY = 1.0f;
    bool  m_AnimateGenerationRect = false;
    float m_GenRectAnimationT = 0.0f;   // current animation parameter [0..1]
    bool  m_GenRectAnimationForward = true;    // true = animating toward upscalerOutputRect
};
