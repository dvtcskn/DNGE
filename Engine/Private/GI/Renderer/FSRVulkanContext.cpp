

#include "pch.h"
#include "FSRVulkanContext.h"

#include "GI/Vulkan/VulkanDevice.h"
#include "GI/Vulkan/VulkanViewport.h"
#include "GI/Vulkan/VulkanBuffer.h"
#include "GI/Vulkan/VulkanFrameBuffer.h"

#include <vk/ffx_api_vk.hpp>
#include <vk/ffx_api_framegeneration_vk.hpp>
#include <ffx_vk.h>
#pragma comment(lib, "amd_fidelityfx_loader_vk.lib")

class VulkanFSRContext::InternalFSRContext
{
public:
    InternalFSRContext()
    {
        VulkanDevice* Device = (VulkanDevice*)GPU::GetDevice();
        vkDeviceContext.vkDevice = Device->Get();
        vkDeviceContext.vkPhysicalDevice = Device->GetPhysicalDevice();
        vkDeviceContext.vkDeviceProcAddr = vkGetDeviceProcAddr;
    }

    ~InternalFSRContext()
    {}

    VkDeviceContext vkDeviceContext = {};
    VkDeviceContext* GetDeviceContext()
    {
        return &vkDeviceContext;
    }
};

VulkanFSRContext::VulkanFSRContext()
    : Super()
{
    InternalContext = new InternalFSRContext();
}

VulkanFSRContext::~VulkanFSRContext()
{
    delete InternalContext;
    InternalContext = nullptr;
}

void* VulkanFSRContext::GetDevice() const
{
    return reinterpret_cast<void*>(&InternalContext->vkDeviceContext);
}

const wchar_t* VulkanFSRContext::GetVersionQueryName(ffxStructType_t createDescType) const
{
    switch (createDescType)
    {
    case FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE:
        return L"Upscale";
    case FFX_API_CREATE_CONTEXT_DESC_TYPE_FRAMEGENERATION:
        return L"FrameGeneration";
    case FFX_API_CREATE_CONTEXT_DESC_TYPE_FRAMEGENERATIONSWAPCHAIN_VERSION_VK:
        return L"FrameGenerationSwapchainVK";
    default:
        return L"Unknown";
    }
}

void VulkanFSRContext::QueryVersionNames(ffxStructType_t createDescType, std::vector<uint64_t>& versionIds, std::vector<std::string>& versionNames) const
{    
    ffxQueryDescGetVersions versionQuery{};
    versionQuery.header.type = FFX_API_QUERY_DESC_TYPE_GET_VERSIONS;
    versionQuery.createDescType = createDescType;
    versionQuery.device = nullptr; //GetDevice();

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

FfxApiResource VulkanFSRContext::GetResource(IRenderTarget* RT, FfxApiResourceState State)
{
    if (RT == nullptr)
    {
        return ffxApiGetResourceVK(nullptr, FfxApiResourceDescription(), State);
    }
    ///*else if (cauldronResource->GetImpl()->IsBuffer())
    //{
    //    VkBuffer buffer = cauldronResource->GetImpl()->GetBuffer();
    //    return ffxApiGetResourceVK((void*)buffer, ffxApiGetBufferResourceDescriptionVK(buffer, cauldronResource->GetImpl()->GetBufferCreateInfo(), 0), State);
    //}*/

    VulkanRenderTarget* RenderTarget = static_cast<VulkanRenderTarget*>(RT);
    VkImage image = RenderTarget->Image;
    return ffxApiGetResourceVK((void*)image, ffxApiGetImageResourceDescriptionVK(image, RenderTarget->imageInfo, 0), State);
}

FfxApiResource VulkanFSRContext::GetDepthResource(IDepthTarget* Depth, FfxApiResourceState State)
{
    if (Depth == nullptr)
    {
        return ffxApiGetResourceVK(nullptr, FfxApiResourceDescription(), State);
    }
    ///*else if (cauldronResource->GetImpl()->IsBuffer())
    //{
    //    VkBuffer buffer = cauldronResource->GetImpl()->GetBuffer();
    //    return ffxApiGetResourceVK((void*)buffer, ffxApiGetBufferResourceDescriptionVK(buffer, cauldronResource->GetImpl()->GetBufferCreateInfo(), 0), State);
    //}*/

    VulkanDepthTarget* DepthTarget = static_cast<VulkanDepthTarget*>(Depth);
    VkImage image = DepthTarget->Image;
    return ffxApiGetResourceVK((void*)image, ffxApiGetImageResourceDescriptionVK(image, DepthTarget->image_create_info, 0), State);
}

FfxApiResource VulkanFSRContext::GetCurrentBackBuffer(FfxApiResourceState State)
{
    VulkanDevice* Device = (VulkanDevice*)GPU::GetDevice();
    VulkanViewport* Viewport = (VulkanViewport*)Device->GetVulkanViewport();

    VkImage image = Viewport->GetCurrentBackBufferImage();
    VkImageCreateInfo CreateInfo = Viewport->GetCurrentBackBufferImageCreateInfo();

    return ffxApiGetResourceVK((void*)image, ffxApiGetImageResourceDescriptionVK(image, CreateInfo, 0), State);
}

ffx::ReturnCode VulkanFSRContext::Query(ffx::QueryDescUpscaleGetResourceRequirements& ResourceRequirements, ffx::CreateContextDescOverrideVersion OverrideVersion)
{
    VulkanDevice* Device = (VulkanDevice*)GPU::GetDevice();
    ffx::CreateBackendVKDesc backendDesc{};
    backendDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_VK;
    backendDesc.deviceContext.vkDevice = Device->Get();
    backendDesc.deviceContext.vkPhysicalDevice = Device->GetPhysicalDevice();
    backendDesc.deviceContext.vkDeviceProcAddr = vkGetDeviceProcAddr;

    return ffx::Query(ResourceRequirements, OverrideVersion, backendDesc);
}

ffx::ReturnCode VulkanFSRContext::QueryUpscaleMode(ffx::QueryDescUpscaleGetUpscaleRatioFromQualityMode UpscaleRatioFromQualityMode, std::optional<ffx::CreateContextDescOverrideVersion> OverrideVersion)
{
    VulkanDevice* Device = (VulkanDevice*)GPU::GetDevice();
    ffx::CreateBackendVKDesc backendDesc{};
    backendDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_VK;
    backendDesc.deviceContext.vkDevice = Device->Get();
    backendDesc.deviceContext.vkPhysicalDevice = Device->GetPhysicalDevice();
    backendDesc.deviceContext.vkDeviceProcAddr = vkGetDeviceProcAddr;

    if (OverrideVersion.has_value())
        return ffx::Query(UpscaleRatioFromQualityMode, OverrideVersion.value(), backendDesc);
    return ffx::Query(UpscaleRatioFromQualityMode, backendDesc);
}

ffx::ReturnCode VulkanFSRContext::CreateUpscalingContext(ffx::Context& UpscalingContext, ffx::CreateContextDescUpscale Desc, ffxCreateContextDescUpscaleVersion Version, std::optional<ffxOverrideVersion> OverrideVersion)
{
    VulkanDevice* Device = (VulkanDevice*)GPU::GetDevice();
    ffx::CreateBackendVKDesc backendDesc{};
    backendDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_VK;
    backendDesc.deviceContext.vkDevice = Device->Get();
    backendDesc.deviceContext.vkPhysicalDevice = Device->GetPhysicalDevice();
    backendDesc.deviceContext.vkDeviceProcAddr = vkGetDeviceProcAddr;

    if (OverrideVersion.has_value())
    {
        OverrideVersion->header.pNext = &backendDesc.header;
        return ffx::CreateContext(UpscalingContext, nullptr, Desc, backendDesc, Version, OverrideVersion.value());
    }
    return ffx::CreateContext(UpscalingContext, nullptr, Desc, backendDesc, Version);
}

ffx::ReturnCode VulkanFSRContext::CreateFrameGenContext(ffx::Context& UpscalingContext, ffx::CreateContextDescFrameGeneration Desc, ffx::CreateContextDescFrameGenerationVersion Version, std::optional<ffxOverrideVersion> OverrideVersion)
{
    VulkanDevice* Device = (VulkanDevice*)GPU::GetDevice();
    ffx::CreateBackendVKDesc backendDesc{};
    backendDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_VK;
    backendDesc.deviceContext.vkDevice = Device->Get();
    backendDesc.deviceContext.vkPhysicalDevice = Device->GetPhysicalDevice();
    backendDesc.deviceContext.vkDeviceProcAddr = vkGetDeviceProcAddr;

    if (OverrideVersion.has_value())
        return ffx::CreateContext(UpscalingContext, nullptr, Desc, backendDesc, Version, OverrideVersion.value());
    return ffx::CreateContext(UpscalingContext, nullptr, Desc, backendDesc, Version);
}

ffx::ReturnCode VulkanFSRContext::CreateHudlessFrameGenContext(ffx::Context& UpscalingContext, ffx::CreateContextDescFrameGeneration Desc, ffx::CreateContextDescFrameGenerationHudless FgHudless, ffx::CreateContextDescFrameGenerationVersion Version, std::optional<ffxOverrideVersion> OverrideVersion)
{
    VulkanDevice* Device = (VulkanDevice*)GPU::GetDevice();
    ffx::CreateBackendVKDesc backendDesc{};
    backendDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_VK;
    backendDesc.deviceContext.vkDevice = Device->Get();
    backendDesc.deviceContext.vkPhysicalDevice = Device->GetPhysicalDevice();
    backendDesc.deviceContext.vkDeviceProcAddr = vkGetDeviceProcAddr;

    if (OverrideVersion.has_value())
        return ffx::CreateContext(UpscalingContext, nullptr, Desc, backendDesc, Version, FgHudless, OverrideVersion.value());
    return ffx::CreateContext(UpscalingContext, nullptr, Desc, backendDesc, Version, FgHudless);
}

ffx::ReturnCode VulkanFSRContext::GetSwapChainGPUMemoryUsage(FfxApiEffectMemoryUsage* MemoryUsage, FfxApiDimensions2D Dimension, uint32_t Format, uint32_t Flags)
{
    VulkanDevice* Device = (VulkanDevice*)GPU::GetDevice();
    VulkanViewport* Viewport = (VulkanViewport*)Device->GetVulkanViewport();

    VkSwapchainCreateInfoKHR swapChainInfo = Viewport->GetSwapChainCreateInfo();
    ffx::QueryFrameGenerationSwapChainGetGPUMemoryUsageVKV2 swapChainGetGPUMemoryUsageV2{};
    swapChainGetGPUMemoryUsageV2.device = Device->Get();
    swapChainGetGPUMemoryUsageV2.physicalDevice = Device->GetPhysicalDevice();
    swapChainGetGPUMemoryUsageV2.displaySize = { swapChainInfo.imageExtent.width, swapChainInfo.imageExtent.height };
    swapChainGetGPUMemoryUsageV2.backBufferFormat = ffxApiGetSurfaceFormatVK(swapChainInfo.imageFormat);
    swapChainGetGPUMemoryUsageV2.backBufferCount = swapChainInfo.minImageCount;
    swapChainGetGPUMemoryUsageV2.composeOnGameQueue = 0;

    swapChainGetGPUMemoryUsageV2.uiResourceSize = Dimension;
    swapChainGetGPUMemoryUsageV2.uiResourceFormat = Format;
    swapChainGetGPUMemoryUsageV2.flags = Flags;
    swapChainGetGPUMemoryUsageV2.gpuMemoryUsageFrameGenerationSwapchain = MemoryUsage;

    return ffx::Query(swapChainGetGPUMemoryUsageV2);
}

void VulkanFSRContext::CreateSwapChainContext(ffx::Context& SwapChainContext, bool bGetLatencyWaitableObject, HANDLE LatencyWaitableObj)
{
    VulkanDevice* Device = (VulkanDevice*)GPU::GetDevice();
    VulkanViewport* Viewport = (VulkanViewport*)Device->GetVulkanViewport();
    
    // Create frameinterpolation swapchain
    VkSwapchainKHR currentSwapchain = Viewport->GetSwapchain();

    ffx::CreateContextDescFrameGenerationSwapChainVK createSwapChainDesc{};
    createSwapChainDesc.physicalDevice = Device->GetPhysicalDevice();
    createSwapChainDesc.device = Device->Get();

    createSwapChainDesc.swapchain = &currentSwapchain;
    createSwapChainDesc.createInfo = Viewport->GetSwapChainCreateInfo();
    createSwapChainDesc.allocator = nullptr;
    createSwapChainDesc.gameQueue.queue = Device->GetQueue();
    createSwapChainDesc.gameQueue.familyIndex = Device->GraphicsQueueIndex();
    createSwapChainDesc.gameQueue.submitFunc = nullptr;  // this queue is only used in vkQueuePresentKHR, hence doesn't need a callback

    createSwapChainDesc.asyncComputeQueue.queue = Device->GetComputeQueue();
    createSwapChainDesc.asyncComputeQueue.familyIndex = Device->GetComputeQueueIndex();
    createSwapChainDesc.asyncComputeQueue.submitFunc = nullptr;

    createSwapChainDesc.presentQueue.queue = Device->GetPresentQueue();
    createSwapChainDesc.presentQueue.familyIndex = Device->GetPresentQueueIndex();
    createSwapChainDesc.presentQueue.submitFunc = nullptr;

    createSwapChainDesc.imageAcquireQueue.queue = Device->GetImageAcquireQueue();
    createSwapChainDesc.imageAcquireQueue.familyIndex = Device->GetImageAcquireQueueIndex();
    createSwapChainDesc.imageAcquireQueue.submitFunc = nullptr;

    // make sure swapchain is not holding a ref to real swapchain
    Viewport->SetSwapChain(VK_NULL_HANDLE);

    // Deal with this when it's time to
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
    //cauldron::GetDevice()->GetImpl()->SetSwapchainContext(SwapChainContext, &frameInterpolationInfo);

    // Set frameinterpolation swapchain to engine
    Viewport->SetSwapChain(currentSwapchain/*, true*/);

    // we need to re initialize HDR info since swapchain was re created
    Viewport->SetHDRMetadataAndColorspace();
}

ffx::ReturnCode VulkanFSRContext::GetSwapChainGetGPUMemoryUsage(ffx::Context SwapChainContext, FfxApiEffectMemoryUsage& gpuMemoryUsageFrameGenerationSwapchain)
{
    ffx::QueryFrameGenerationSwapChainGetGPUMemoryUsageVK frameGenSwapchainGetGPUMemoryUsage{};
    frameGenSwapchainGetGPUMemoryUsage.gpuMemoryUsageFrameGenerationSwapchain = &gpuMemoryUsageFrameGenerationSwapchain;
    return ffx::Query(SwapChainContext, frameGenSwapchainGetGPUMemoryUsage);
}

void VulkanFSRContext::ConfigureSwapChain(ffx::Context& SwapChainContext, FfxApiResource uiColor, bool bDoublebufferInSwapchain)
{
    ffx::ConfigureDescFrameGenerationSwapChainRegisterUiResourceVK uiConfig{};
    uiConfig.uiResource = uiColor;
    uiConfig.flags = bDoublebufferInSwapchain ? FFX_FRAMEGENERATION_UI_COMPOSITION_FLAG_ENABLE_INTERNAL_UI_DOUBLE_BUFFERING : 0;
    ffx::Configure(SwapChainContext, uiConfig);
}

ffx::ReturnCode VulkanFSRContext::QuerySwapChainCommandList(ffx::Context& SwapChainContext, void** CommandList)
{
    ffx::QueryDescFrameGenerationSwapChainInterpolationCommandListVK queryCmdList{};
    queryCmdList.pOutCommandList = CommandList;
    return ffx::Query(SwapChainContext, queryCmdList);
}

ffx::ReturnCode VulkanFSRContext::QuerySwapChainInterpolationTexture(ffx::Context& SwapChainContext, FfxApiResource* Texture)
{
    ffx::QueryDescFrameGenerationSwapChainInterpolationTextureVK queryFiTexture{};
    queryFiTexture.pOutTexture = Texture;
    return ffx::Query(SwapChainContext, queryFiTexture);
}
