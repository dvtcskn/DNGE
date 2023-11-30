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

#include <d3d12.h>
#include <wrl\client.h>
using namespace Microsoft::WRL;

#include "D3D12Device.h"
#include "D3D12FrameBuffer.h"
#include "Engine/AbstractEngine.h"

#include "D3D12Fence.h"

class D3D12ComputeCommandContext : public IComputeCommandContext
{
	sClassBody(sClassConstructor, D3D12ComputeCommandContext, IComputeCommandContext)
public:
	D3D12ComputeCommandContext(D3D12Device* InDevice);
	virtual ~D3D12ComputeCommandContext();

	FORCEINLINE ID3D12GraphicsCommandList* Get() const { return CommandList.Get(); }
	FORCEINLINE ID3D12Device* GetDevice() const { return Owner->GetDevice(); }

	virtual void* GetInternalCommandContext() override final { return CommandList.Get(); }

	virtual void BeginRecordCommandList() override final;
	virtual void FinishRecordCommandList() override final;
	virtual void ExecuteCommandList() override final;

	void Open();
	void Close();
	bool IsClosed() const { return bIsClosed; }

	void ResourceBarrier(UINT NumBarriers, const D3D12_RESOURCE_BARRIER* pBarriers);

	void WaitForGPU();

	virtual void SetFrameBuffer(IFrameBuffer* pFB, std::optional<std::size_t> FBOIndex) override final;
	virtual void SetPipeline(IComputePipeline* Pipeline) override final;
	virtual void SetConstantBuffer(IConstantBuffer* CB, std::optional<std::uint32_t> RootParameterIndex = std::nullopt) override final;

	virtual void SetRenderTargetAsResource(IRenderTarget* pRT, std::uint32_t RootParameterIndex) override final;
	virtual void SetRenderTargetsAsResource(std::vector<IRenderTarget*> RTs, std::uint32_t RootParameterIndex) override final;
	virtual void SetRenderTargetAsUAV(IRenderTarget* pRT, std::uint32_t RootParameterIndex) override final;
	virtual void SetRenderTargetsAsUAV(std::vector<IRenderTarget*> RTs, std::uint32_t RootParameterIndex) override final;
	virtual void SetUnorderedAccessTarget(IUnorderedAccessTarget* pST, std::uint32_t RootParameterIndex) override final;
	virtual void SetUnorderedAccessTargets(std::vector<IUnorderedAccessTarget*> pSTs, std::uint32_t RootParameterIndex) override final;
	virtual void SetUnorderedAccessTargetAsSRV(IUnorderedAccessTarget* pST, std::uint32_t RootParameterIndex) override final;
	virtual void SetUnorderedAccessTargetsAsSRV(std::vector<IUnorderedAccessTarget*> pSTs, std::uint32_t RootParameterIndex) override final;

	virtual void SetUnorderedAccessBuffer(IUnorderedAccessBuffer* pUAV, std::uint32_t RootParameterIndex) override final;
	virtual void SetUnorderedAccessBuffers(std::vector<IUnorderedAccessBuffer*> UAVs, std::uint32_t RootParameterIndex) override final;
	virtual void SetUnorderedAccessBufferAsResource(IUnorderedAccessBuffer* pUAV, std::uint32_t RootParameterIndex) override final;
	virtual void SetUnorderedAccessBuffersAsResource(std::vector<IUnorderedAccessBuffer*> UAVs, std::uint32_t RootParameterIndex) override final;

	virtual void SetDepthTargetAsResource(IDepthTarget* pDT, std::uint32_t RootParameterIndex) override final;
	virtual void SetDepthTargetsAsResource(std::vector<IDepthTarget*> DTs, std::uint32_t RootParameterIndex) override final;

	void SetConstants(std::uint32_t RootEntry, std::uint32_t X, std::uint32_t Y, std::uint32_t Z, std::uint32_t W);

	virtual void Dispatch(std::uint32_t ThreadGroupCountX, std::uint32_t ThreadGroupCountY, std::uint32_t ThreadGroupCountZ) override final;
	virtual void ExecuteIndirect(IIndirectBuffer* IndirectBuffer) override final;

	virtual void ClearState() override final;

private:
	D3D12Device* Owner;
	ComPtr<ID3D12GraphicsCommandList> CommandList;
	ID3D12CommandAllocator* CommandAllocator;

	bool bIsClosed;
	bool bWaitForCompletion;
};
