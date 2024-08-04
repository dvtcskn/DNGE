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
#include "VulkanCommandBuffer.h"

VulkanCommandBuffer::VulkanCommandBuffer()
	: StencilRef(0)
	, bIsSingleThreaded(true)
{
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
	ClearCMDStates();
}

void VulkanCommandBuffer::BeginRecordCommandList(const ERenderPass RenderPass)
{
	ClearCMDStates();
	ClearState(); 
}

void VulkanCommandBuffer::FinishRecordCommandList()
{
}

void VulkanCommandBuffer::ExecuteCommandList()
{
}

void VulkanCommandBuffer::ClearState()
{
	StencilRef = 0;
}

void VulkanCommandBuffer::Draw(std::uint32_t VertexCount, std::uint32_t VertexStartOffset)
{
}

void VulkanCommandBuffer::DrawInstanced(std::uint32_t VertexCountPerInstance, std::uint32_t InstanceCount, std::uint32_t StartVertexLocation, std::uint32_t StartInstanceLocation)
{
}

void VulkanCommandBuffer::DrawIndexedInstanced(std::uint32_t IndexCountPerInstance, std::uint32_t InstanceCount, std::uint32_t StartIndexLocation, std::int32_t BaseVertexLocation, std::uint32_t StartInstanceLocation)
{
}

void VulkanCommandBuffer::DrawIndexedInstanced(const sObjectDrawParameters& DrawParameters)
{
}

void VulkanCommandBuffer::ExecuteIndirect(IIndirectBuffer* IndirectBuffer)
{
}

void VulkanCommandBuffer::SetViewport(const sViewport& Viewport)
{
}

void VulkanCommandBuffer::SetScissorRect(std::uint32_t X, std::uint32_t Y, std::uint32_t Z, std::uint32_t W)
{
}

void VulkanCommandBuffer::SetStencilRef(std::uint32_t Ref)
{
}

void VulkanCommandBuffer::ClearFrameBuffer(IFrameBuffer* pFB)
{

}

void VulkanCommandBuffer::ClearRenderTarget(IRenderTarget* pFB, IDepthTarget* DepthTarget)
{
}

void VulkanCommandBuffer::ClearRenderTargets(std::vector<IRenderTarget*> pRTs, IDepthTarget* DepthTarget)
{
}

void VulkanCommandBuffer::ClearDepthTarget(IDepthTarget* DepthTarget)
{
}

void VulkanCommandBuffer::SetFrameBuffer(IFrameBuffer* pFB, std::optional<std::size_t> FBOIndex)
{

}

void VulkanCommandBuffer::SetRenderTarget(IRenderTarget* pRT, IDepthTarget* DepthTarget)
{
}

void VulkanCommandBuffer::SetRenderTargets(std::vector<IRenderTarget*> pRTs, IDepthTarget* DepthTarget)
{
}

void VulkanCommandBuffer::SetFrameBufferAsResource(IFrameBuffer* pFB, std::uint32_t RootParameterIndex)
{
}

void VulkanCommandBuffer::SetFrameBufferAsResource(IFrameBuffer* pFB, std::uint32_t FBOIndex, std::uint32_t RootParameterIndex)
{
}

void VulkanCommandBuffer::SetRenderTargetAsResource(IRenderTarget* pRT, std::uint32_t RootParameterIndex)
{
}

void VulkanCommandBuffer::SetRenderTargetsAsResource(std::vector<IRenderTarget*> RTs, std::uint32_t RootParameterIndex)
{
}

void VulkanCommandBuffer::CopyFrameBuffer(IFrameBuffer* Dest, std::size_t DestFBOIndex, IFrameBuffer* Source, std::uint32_t SourceFBOIndex)
{
}

void VulkanCommandBuffer::CopyFrameBufferDepth(IFrameBuffer* Dest, IFrameBuffer* Source)
{
}

void VulkanCommandBuffer::CopyRenderTarget(IRenderTarget* Dest, IRenderTarget* Source)
{
}

void VulkanCommandBuffer::CopyDepthBuffer(IDepthTarget* Dest, IDepthTarget* Source)
{
}

void VulkanCommandBuffer::SetUnorderedAccessBufferAsResource(IUnorderedAccessBuffer* pUAV, std::uint32_t RootParameterIndex)
{
}

void VulkanCommandBuffer::SetUnorderedAccessBuffersAsResource(std::vector<IUnorderedAccessBuffer*> UAVs, std::uint32_t RootParameterIndex)
{
}

void VulkanCommandBuffer::SetPipeline(IPipeline* Pipeline)
{

}

void VulkanCommandBuffer::SetConstantBuffer(IConstantBuffer* CB, std::optional<std::uint32_t> InRootParameterIndex)
{
}

void VulkanCommandBuffer::SetTexture2D(ITexture2D* Texture2D, std::optional<std::uint32_t> InRootParameterIndex)
{
}

void VulkanCommandBuffer::SetVertexBuffer(IVertexBuffer* VBuffer, std::uint32_t Slot)
{
}

void VulkanCommandBuffer::SetIndexBuffer(IIndexBuffer* IBuffer)
{
}

void VulkanCommandBuffer::UpdateBufferSubresource(IVertexBuffer* Buffer, BufferSubresource* Subresource)
{
}

void VulkanCommandBuffer::UpdateBufferSubresource(IVertexBuffer* Buffer, std::size_t Location, std::size_t Size, const void* pSrcData)
{
}

void VulkanCommandBuffer::UpdateBufferSubresource(IIndexBuffer* Buffer, BufferSubresource* Subresource)
{
}

void VulkanCommandBuffer::UpdateBufferSubresource(IIndexBuffer* Buffer, std::size_t Location, std::size_t Size, const void* pSrcData)
{
}

void VulkanCommandBuffer::ClearCMDStates()
{
	StencilRef = 0;
}
