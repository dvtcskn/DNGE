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
