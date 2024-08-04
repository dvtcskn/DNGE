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

#include <d3d11_1.h>
#include <wrl\client.h>
using namespace Microsoft::WRL;

#include "D3D11Device.h"
#include "Engine/AbstractEngine.h"
#include "D3D11Pipeline.h"
#include "D3D11Buffer.h"
#include "D3D11Texture.h"

class D3D11FrameBuffer;

class D3D11CommandBuffer final : public IGraphicsCommandContext
{
	sClassBody(sClassConstructor, D3D11CommandBuffer, IGraphicsCommandContext)
public:
	D3D11CommandBuffer(D3D11Device* InDevice);
	virtual ~D3D11CommandBuffer();

	FORCEINLINE ID3D11DeviceContext1* Get() const { return DeferredCTX.Get(); }

	virtual void BeginRecordCommandList(const ERenderPass RenderPass = ERenderPass::eNONE) override final;
	virtual void FinishRecordCommandList() override final;
	virtual void ExecuteCommandList() override final;
	virtual void ClearState() override final;

	virtual void* GetInternalCommandContext() override final { return DeferredCTX.Get(); }

	virtual void SetViewport(const sViewport& Viewport) override final;

	virtual void SetScissorRect(std::uint32_t X, std::uint32_t Y, std::uint32_t Z, std::uint32_t W) override final;
	virtual void SetStencilRef(std::uint32_t Ref) override final;
	virtual std::uint32_t GetStencilRef() const override final { return StencilRef; }

	virtual void ClearFrameBuffer(IFrameBuffer* pFB) override final;
	virtual void ClearRenderTarget(IRenderTarget* pFB, IDepthTarget* DepthTarget = nullptr) override final;
	virtual void ClearRenderTargets(std::vector<IRenderTarget*> pRTs, IDepthTarget* DepthTarget = nullptr) override final;
	virtual void ClearDepthTarget(IDepthTarget* DepthTarget) override final;
	virtual void SetFrameBuffer(IFrameBuffer* pFB, std::optional<std::size_t> FBOIndex = std::nullopt) override final;
	virtual void SetRenderTarget(IRenderTarget* pRT, IDepthTarget* DepthTarget = nullptr) override final;
	virtual void SetRenderTargets(std::vector<IRenderTarget*> pRTs, IDepthTarget* DepthTarget = nullptr) override final;
	virtual void SetFrameBufferAsResource(IFrameBuffer* pFB, std::uint32_t RootParameterIndex) override final;
	virtual void SetFrameBufferAsResource(IFrameBuffer* pFB, std::uint32_t FBOIndex, std::uint32_t RootParameterIndex) override final;
	virtual void SetRenderTargetAsResource(IRenderTarget* pRT, std::uint32_t RootParameterIndex) override final;
	virtual void SetRenderTargetsAsResource(std::vector<IRenderTarget*> RTs, std::uint32_t RootParameterIndex) override final;
	virtual void CopyFrameBuffer(IFrameBuffer* Dest, std::size_t DestFBOIndex, IFrameBuffer* Source, std::uint32_t SourceFBOIndex) override final;
	virtual void CopyFrameBufferDepth(IFrameBuffer* Dest, IFrameBuffer* Source) override final;
	virtual void CopyRenderTarget(IRenderTarget* Dest, IRenderTarget* Source) override final;
	virtual void CopyDepthBuffer(IDepthTarget* Dest, IDepthTarget* Source) override final;

	virtual void SetUnorderedAccessBufferAsResource(IUnorderedAccessBuffer* pUAV, std::uint32_t RootParameterIndex) override final;
	virtual void SetUnorderedAccessBuffersAsResource(std::vector<IUnorderedAccessBuffer*> UAVs, std::uint32_t RootParameterIndex) override final;

	virtual void SetPipeline(IPipeline* Pipeline) override final;

	virtual void SetVertexBuffer(IVertexBuffer* VB, std::uint32_t Slot = 0) override final;
	virtual void SetIndexBuffer(IIndexBuffer* IB) override final;
	virtual void SetConstantBuffer(IConstantBuffer* CB, std::optional<std::uint32_t> RootParameterIndex = std::nullopt) override final;

	virtual void SetTexture2D(ITexture2D* Texture2D, std::optional<std::uint32_t> RootParameterIndex = std::nullopt) override final;

	virtual void UpdateBufferSubresource(IVertexBuffer* Buffer, BufferSubresource* Subresource) override final;
	virtual void UpdateBufferSubresource(IVertexBuffer* Buffer, std::size_t Location, std::size_t Size, const void* pSrcData) override final;
	virtual void UpdateBufferSubresource(IIndexBuffer* Buffer, BufferSubresource* Subresource) override final;
	virtual void UpdateBufferSubresource(IIndexBuffer* Buffer, std::size_t Location, std::size_t Size, const void* pSrcData) override final;

	virtual void Draw(std::uint32_t VertexCount, std::uint32_t VertexStartOffset = 0) override final;
	virtual void DrawInstanced(std::uint32_t VertexCountPerInstance, std::uint32_t InstanceCount, std::uint32_t StartVertexLocation, std::uint32_t StartInstanceLocation) override final;
	virtual void DrawIndexedInstanced(std::uint32_t IndexCountPerInstance, std::uint32_t InstanceCount, std::uint32_t StartIndexLocation, std::int32_t BaseVertexLocation, std::uint32_t StartInstanceLocation) override final;
	virtual void DrawIndexedInstanced(const sObjectDrawParameters& Params) override final;

	virtual void ExecuteIndirect(IIndirectBuffer* IndirectBuffer) override final;

	void ClearCMDStates();

private:
	D3D11Device* Owner;
	ComPtr<ID3D11CommandList> CommandList;
	ComPtr<ID3D11DeviceContext1> DeferredCTX;

	D3D11Pipeline* ActivePipeline;

	bool bIsSingleThreaded;
	std::uint32_t StencilRef;
};

class D3D11CopyCommandBuffer : public ICopyCommandContext
{
	sClassBody(sClassConstructor, D3D11CopyCommandBuffer, ICopyCommandContext)
public:
	D3D11CopyCommandBuffer(D3D11Device* InDevice);
	virtual ~D3D11CopyCommandBuffer();

	FORCEINLINE ID3D11DeviceContext1* Get() const { return DeferredCTX.Get(); }

	virtual void BeginRecordCommandList() override final;
	virtual void FinishRecordCommandList() override final;
	virtual void ExecuteCommandList() override final;
	virtual void ClearState() override final;

	virtual void* GetInternalCommandContext() override final { return DeferredCTX.Get(); }

	virtual void CopyFrameBuffer(IFrameBuffer* Dest, std::size_t DestFBOIndex, IFrameBuffer* Source, std::uint32_t SourceFBOIndex) override final;
	virtual void CopyFrameBufferDepth(IFrameBuffer* Dest, IFrameBuffer* Source) override final;

	virtual void UpdateBufferSubresource(IVertexBuffer* Buffer, BufferSubresource* Subresource) override final;
	virtual void UpdateBufferSubresource(IVertexBuffer* Buffer, std::size_t Location, std::size_t Size, const void* pSrcData) override final;
	virtual void UpdateBufferSubresource(IIndexBuffer* Buffer, BufferSubresource* Subresource) override final;
	virtual void UpdateBufferSubresource(IIndexBuffer* Buffer, std::size_t Location, std::size_t Size, const void* pSrcData) override final;

private:
	D3D11Device* Owner;
	ComPtr<ID3D11CommandList> CommandList;
	ComPtr<ID3D11DeviceContext1> DeferredCTX;

	bool bIsSingleThreaded;
};
