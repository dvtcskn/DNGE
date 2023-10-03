
#include "pch.h"
#include "VulkanComputeCommandContext.h"

VulkanComputeCommandContext::VulkanComputeCommandContext()
{
}

VulkanComputeCommandContext::~VulkanComputeCommandContext()
{
}

void VulkanComputeCommandContext::BeginRecordCommandList()
{
}

void VulkanComputeCommandContext::FinishRecordCommandList()
{
}

void VulkanComputeCommandContext::ExecuteCommandList()
{
}

void VulkanComputeCommandContext::ClearState()
{
}

void* VulkanComputeCommandContext::GetInternalCommandContext()
{
	return nullptr;
}

void VulkanComputeCommandContext::SetFrameBuffer(IFrameBuffer* pFB, std::optional<std::size_t> FBOIndex)
{
}

void VulkanComputeCommandContext::SetPipeline(IComputePipeline* Pipeline)
{
}

void VulkanComputeCommandContext::SetConstantBuffer(IConstantBuffer* CB, std::optional<std::uint32_t> RootParameterIndex)
{
}

void VulkanComputeCommandContext::SetRenderTargetAsResource(IRenderTarget* pRT, std::uint32_t RootParameterIndex)
{
}

void VulkanComputeCommandContext::SetRenderTargetsAsResource(std::vector<IRenderTarget*> RTs, std::uint32_t RootParameterIndex)
{
}

void VulkanComputeCommandContext::SetUnorderedAccessTarget(IUnorderedAccessTarget* pST, std::uint32_t RootParameterIndex)
{
}

void VulkanComputeCommandContext::SetUnorderedAccessTargets(std::vector<IUnorderedAccessTarget*> pSTs, std::uint32_t RootParameterIndex)
{
}

void VulkanComputeCommandContext::SetRenderTargetAsUAV(IRenderTarget* pRT, std::uint32_t RootParameterIndex)
{
}

void VulkanComputeCommandContext::SetRenderTargetsAsUAV(std::vector<IRenderTarget*> RTs, std::uint32_t RootParameterIndex)
{
}

void VulkanComputeCommandContext::SetUnorderedAccessTargetAsSRV(IUnorderedAccessTarget* pST, std::uint32_t RootParameterIndex)
{
}

void VulkanComputeCommandContext::SetUnorderedAccessTargetsAsSRV(std::vector<IUnorderedAccessTarget*> pSTs, std::uint32_t RootParameterIndex)
{
}

void VulkanComputeCommandContext::SetDepthTargetAsResource(IDepthTarget* pDT, std::uint32_t RootParameterIndex)
{
}

void VulkanComputeCommandContext::SetDepthTargetsAsResource(std::vector<IDepthTarget*> DTs, std::uint32_t RootParameterIndex)
{
}

void VulkanComputeCommandContext::Dispatch(std::uint32_t ThreadGroupCountX, std::uint32_t ThreadGroupCountY, std::uint32_t ThreadGroupCountZ)
{
}

void VulkanComputeCommandContext::ExecuteIndirect(IIndirectBuffer* IndirectBuffer)
{
}
