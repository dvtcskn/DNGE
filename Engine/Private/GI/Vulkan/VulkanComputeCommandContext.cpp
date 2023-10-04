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
