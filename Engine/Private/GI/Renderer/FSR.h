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

#ifndef FSR_D3D12
#define FSR_D3D12 1
#endif
#ifndef FSR_VULKAN
#define FSR_VULKAN 0
#endif

#if FSR_D3D12
#include <dx12/ffx_api_dx12.hpp>
#include <dx12/ffx_api_framegeneration_dx12.hpp>
#include <ffx_dx12.h>
#elif FSR_VULKAN
//#include <ffx_api/vk/ffx_api_vk.hpp>
#endif

enum UICompositionMode : std::uint32_t
{
    No_UI_Handling = 0,
    UiTexture = 1,
    UiCallback = 2,
    PreUiBackbuffer = 3,
};

enum class FSRColorSpace
{
    DefaultLinearColorSpace = 0,
    NonLinearColorSpace,
    sRGBColorSpace,
    PQColorSpace
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

	ffxQueryGetProviderVersion GetVersion() /*const*/;
	ffx::QueryDescUpscaleGetGPUMemoryUsage GetUpscaleGetGPUMemoryUsage() /*const*/;

    void Render(IRenderTarget* Color, IRenderTarget* MotionVectors, IRenderTarget* Reactive, IDepthTarget* Depth, ICamera* pCamera);
    void GenerateReactiveMask(IRenderTarget* Color);
    void CopyToFrameBuffer(IRenderTarget* Target);
    inline IRenderTarget* GetOutputRenderTarget() const
    {
        return Output.get();
    }

    virtual void SetRenderSize(std::size_t Width, std::size_t Height) override final;
    virtual void OnInputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar) override final;

    void UpdatePreset(const int32_t* pOldPreset);

    bool SetJitter(FVector2& Vec);

    float GetMipBias() const;

    std::function<void(sScreenDimension, float)> OnUpdatePreset;

private:
    void SetUpscaleConstantBuffer(uint64_t key, float value);
    float GetUpscaleBaseRenderRatio() const;

    static void FfxMsgCallback(uint32_t type, const wchar_t* message);

    /**
     * @brief   Returns whether or not FSR requires sample-side re-initialization.
     */
    inline bool NeedsReInit() const
    {
        return bNeedReInit;
    }

    /**
     * @brief   Clears FSR re-initialization flag.
     */
    inline void ClearReInit()
    {
        bNeedReInit = false;
    }

private:
    IGraphicsCommandContext::SharedPtr GraphicsCommandContext;
    IGraphicsCommandContext::SharedPtr CMD;
    sRenderDimension RenderDimension;
    IRenderTarget::SharedPtr Output;
    IRenderTarget::SharedPtr GeneratedReactiveMask;
    bool bGenerateReactiveMask = false;
    std::mutex Mutex;

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

    bool bNeedReInit = false;
    uint32_t JitterFrame = 0;
    uint64_t FrameID = 0;
    //FSRMaskMode MaskMode = FSRMaskMode::Manual;
    FSRDebugCheckerMode GlobalDebugCheckerMode = FSRDebugCheckerMode::Disabled;
    ERendererUpscaleMode UpscaleMode = ERendererUpscaleMode::Balanced;
    std::uint64_t CurrentUpscaleContextVersionId = 0;
    const char* CurrentUpscaleContextVersionName = nullptr;
    std::vector<uint64_t> FsrVersionIds;
    std::uint32_t FsrVersionIndex = 0;
    std::vector<const char*> FsrVersionNames;
    bool bOverrideVersion = false;
    std::vector<uint64_t> FgVersionIds;
    int32_t FgVersionIndex = 0;
    std::vector<const char*> FgVersionNames;
    Version currentFgContextVersion = {};
    bool bEnableAsyncCompute = false;
    bool bInvertedDepth = true;
    float MipBias = cMipBias[static_cast<uint32_t>(ERendererUpscaleMode::Quality)];
    FSRColorSpace ColorSpace = FSRColorSpace::DefaultLinearColorSpace;

private:
    // FSR Upscale
    bool bUpscalingActive = true;
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

    void UpdateUpscaleRatio();

private:
    // FSR FrameGen
    bool bFrameInterpolationActive = false;
    UICompositionMode UIRenderMode = UICompositionMode::No_UI_Handling;
    ffx::Context FrameGenContext = nullptr;
    ffx::Context SwapChainContext = nullptr;
    ffx::ConfigureDescFrameGeneration FrameGenerationConfig;

    bool bEnableFrameInterpolationSwapchain = false;
    void EnableFrameInterpolationSwapchain(bool enabled);

    bool   bGetLatencyWaitableObject = false;
    HANDLE LatencyWaitableObj = 0;
};
