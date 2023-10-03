
#include "pch.h"
#include "D3D12ComputeCommandContext.h"
#include "D3D12Buffer.h"
#include "D3D12Shader.h"
#include "D3D12Pipeline.h"
#include "D3D12Viewport.h"
#include "D3D12Texture.h"
#include "GI/D3DShared/D3DShared.h"

D3D12ComputeCommandContext::D3D12ComputeCommandContext(D3D12Device* InDevice)
	: Owner(InDevice)
	, bIsClosed(false)
	, bWaitForCompletion(false)
{
	auto Device = Owner->GetDevice();
	CommandAllocator = Owner->RequestCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE);
	ThrowIfFailed(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, CommandAllocator, nullptr, IID_PPV_ARGS(&CommandList)));
	CommandList->Close();
	bIsClosed = true;
#if _DEBUG
	CommandList->SetName(L"D3D12ComputeCommandContext");
#endif
}

D3D12ComputeCommandContext::~D3D12ComputeCommandContext()
{
	CommandList = nullptr;
	CommandAllocator = nullptr;
	Owner = nullptr;
}

void D3D12ComputeCommandContext::BeginRecordCommandList()
{
	Open();

	{
		//WaitForGPU();
	}
}

void D3D12ComputeCommandContext::FinishRecordCommandList()
{
	Close();
}

void D3D12ComputeCommandContext::ExecuteCommandList()
{
	Owner->ExecuteComputeCommandLists(CommandList.Get()/*, bWaitForCompletion*/);

	Owner->DiscardCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, CommandAllocator);
	CommandAllocator = nullptr;
	bWaitForCompletion = false;
}

void D3D12ComputeCommandContext::Open()
{
	if (!CommandAllocator)
		CommandAllocator = Owner->RequestCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE);

	ThrowIfFailed(CommandList->Reset(CommandAllocator, nullptr));
	bIsClosed = false;
	bWaitForCompletion = false;
}

void D3D12ComputeCommandContext::Close()
{
	CommandList->Close();
	bIsClosed = true;
}

void D3D12ComputeCommandContext::ResourceBarrier(UINT NumBarriers, const D3D12_RESOURCE_BARRIER* pBarriers)
{
	CommandList->ResourceBarrier(NumBarriers, pBarriers);
}

void D3D12ComputeCommandContext::WaitForGPU()
{
	Owner->GPUSignal();
	Owner->CPUWait();
}

void D3D12ComputeCommandContext::SetFrameBuffer(IFrameBuffer* pFB, std::optional<std::size_t> FBOIndex)
{
	//CommandList->SetComputeRootUnorderedAccessView(0, static_cast<D3D12UnorderedAccessTarget*>(pFB->GetUnorderedAccessTarget(0))->GetGPUVirtualAddress());
	//CommandList->SetComputeRootShaderResourceView(0, static_cast<D3D12RenderTarget*>(pFB->GetRenderTarget(0))->GetGPUVirtualAddress());
}

void D3D12ComputeCommandContext::SetPipeline(IComputePipeline* Pipeline)
{
	if (Pipeline)
	{
		static_cast<D3D12ComputePipeline*>(Pipeline)->ApplyPipeline(CommandList.Get());
	}
}

void D3D12ComputeCommandContext::SetConstantBuffer(IConstantBuffer* CB, std::optional<std::uint32_t> RootParameterIndex)
{
	if (CB)
	{
		auto GPU = static_cast<D3D12ConstantBuffer*>(CB)->GetGPUVirtualAddress();
		if (RootParameterIndex.has_value())
			CommandList->SetComputeRootConstantBufferView(*RootParameterIndex, GPU);
		else
			CommandList->SetComputeRootConstantBufferView(CB->GetDefaultRootParameterIndex(), GPU);
	}
}

void D3D12ComputeCommandContext::SetRenderTargetAsResource(IRenderTarget* pRT, std::uint32_t RootParameterIndex)
{

}

void D3D12ComputeCommandContext::SetRenderTargetsAsResource(std::vector<IRenderTarget*> RTs, std::uint32_t RootParameterIndex)
{

}

void D3D12ComputeCommandContext::SetRenderTargetAsUAV(IRenderTarget* pRT, std::uint32_t RootParameterIndex)
{

}

void D3D12ComputeCommandContext::SetRenderTargetsAsUAV(std::vector<IRenderTarget*> RTs, std::uint32_t RootParameterIndex)
{

}

void D3D12ComputeCommandContext::SetUnorderedAccessTarget(IUnorderedAccessTarget* pST, std::uint32_t RootParameterIndex)
{

}

void D3D12ComputeCommandContext::SetUnorderedAccessTargets(std::vector<IUnorderedAccessTarget*> pSTs, std::uint32_t RootParameterIndex)
{

}

void D3D12ComputeCommandContext::SetUnorderedAccessTargetAsSRV(IUnorderedAccessTarget* pST, std::uint32_t RootParameterIndex)
{

}

void D3D12ComputeCommandContext::SetUnorderedAccessTargetsAsSRV(std::vector<IUnorderedAccessTarget*> pSTs, std::uint32_t RootParameterIndex)
{

}

void D3D12ComputeCommandContext::SetDepthTargetAsResource(IDepthTarget* pDT, std::uint32_t RootParameterIndex)
{

}

void D3D12ComputeCommandContext::SetDepthTargetsAsResource(std::vector<IDepthTarget*> DTs, std::uint32_t RootParameterIndex)
{

}

void D3D12ComputeCommandContext::SetConstants(std::uint32_t RootEntry, std::uint32_t X, std::uint32_t Y, std::uint32_t Z, std::uint32_t W)
{
	CommandList->SetComputeRoot32BitConstant(RootEntry, X, 0);
	CommandList->SetComputeRoot32BitConstant(RootEntry, Y, 1);
	CommandList->SetComputeRoot32BitConstant(RootEntry, Z, 2);
	CommandList->SetComputeRoot32BitConstant(RootEntry, W, 3);
}

void D3D12ComputeCommandContext::Dispatch(std::uint32_t ThreadGroupCountX, std::uint32_t ThreadGroupCountY, std::uint32_t ThreadGroupCountZ)
{
	CommandList->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
}

void D3D12ComputeCommandContext::ExecuteIndirect(IIndirectBuffer* IndirectBuffer)
{
}

void D3D12ComputeCommandContext::ClearState()
{
	CommandList->ClearState(nullptr);
	CommandList->Reset(CommandAllocator, nullptr);
	CommandList->Close();

	bIsClosed = true;
	bWaitForCompletion = false;
}
