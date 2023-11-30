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
	WaitForGPU();
	Open();

	{
		Owner->SetHeaps(CommandList.Get());
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
		/*auto GPU = static_cast<D3D12ConstantBuffer*>(CB)->GetGPUVirtualAddress();
		if (RootParameterIndex.has_value())
			CommandList->SetComputeRootConstantBufferView(*RootParameterIndex, GPU);
		else
			CommandList->SetComputeRootConstantBufferView(CB->GetDefaultRootParameterIndex(), GPU);*/

		if (RootParameterIndex.has_value())
			CommandList->SetComputeRootDescriptorTable(*RootParameterIndex, static_cast<D3D12ConstantBuffer*>(CB)->GetHeapHandle().GetGPU());
		else
			CommandList->SetComputeRootDescriptorTable(CB->GetDefaultRootParameterIndex(), static_cast<D3D12ConstantBuffer*>(CB)->GetHeapHandle().GetGPU());
	}
}

void D3D12ComputeCommandContext::SetRenderTargetAsResource(IRenderTarget* pRT, std::uint32_t RootParameterIndex)
{
	if (pRT)
	{
		D3D12RenderTarget* FBuffer = static_cast<D3D12RenderTarget*>(pRT);

		if (FBuffer->CurrentState != D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)
		{
			D3D12_RESOURCE_BARRIER Barriers[1];
			Barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(FBuffer->GetD3D12Texture(), FBuffer->CurrentState, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			CommandList->ResourceBarrier(ARRAYSIZE(Barriers), Barriers);
			FBuffer->CurrentState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		}

		CommandList->SetComputeRootDescriptorTable(RootParameterIndex, FBuffer->GetSRVGPU());
	}
}

void D3D12ComputeCommandContext::SetRenderTargetsAsResource(std::vector<IRenderTarget*> RTs, std::uint32_t RootParameterIndex)
{
	if (RTs.size() > 0)
	{
		int i = RootParameterIndex;
		for (IRenderTarget* pRT : RTs)
		{
			D3D12RenderTarget* FBuffer = static_cast<D3D12RenderTarget*>(pRT);
			if (FBuffer->CurrentState != D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)
			{
				D3D12_RESOURCE_BARRIER Barriers[1];
				Barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(FBuffer->GetD3D12Texture(), FBuffer->CurrentState, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
				CommandList->ResourceBarrier(ARRAYSIZE(Barriers), Barriers);
				FBuffer->CurrentState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
			}

			CommandList->SetComputeRootShaderResourceView(i, FBuffer->GetGPUVirtualAddress());

			i++;
		}
	}
}

void D3D12ComputeCommandContext::SetRenderTargetAsUAV(IRenderTarget* pRT, std::uint32_t RootParameterIndex)
{
	if (pRT)
	{
		D3D12RenderTarget* FBuffer = static_cast<D3D12RenderTarget*>(pRT);

		if (FBuffer->CurrentState != D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
		{
			D3D12_RESOURCE_BARRIER Barriers[1];
			Barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(FBuffer->GetD3D12Texture(), FBuffer->CurrentState, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			CommandList->ResourceBarrier(ARRAYSIZE(Barriers), Barriers);
			FBuffer->CurrentState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		}

		CommandList->SetComputeRootDescriptorTable(RootParameterIndex, FBuffer->GetUAVGPU());
	}
}

void D3D12ComputeCommandContext::SetRenderTargetsAsUAV(std::vector<IRenderTarget*> RTs, std::uint32_t RootParameterIndex)
{
	if (RTs.size() > 0)
	{
		int i = RootParameterIndex;
		for (IRenderTarget* pRT : RTs)
		{
			D3D12RenderTarget* FBuffer = static_cast<D3D12RenderTarget*>(pRT);
			if (FBuffer->CurrentState != D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
			{
				D3D12_RESOURCE_BARRIER Barriers[1];
				Barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(FBuffer->GetD3D12Texture(), FBuffer->CurrentState, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
				CommandList->ResourceBarrier(ARRAYSIZE(Barriers), Barriers);
				FBuffer->CurrentState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			}

			CommandList->SetComputeRootDescriptorTable(RootParameterIndex, FBuffer->GetUAVGPU());

			i++;
		}
	}
}

void D3D12ComputeCommandContext::SetUnorderedAccessTarget(IUnorderedAccessTarget* pST, std::uint32_t RootParameterIndex)
{
	if (pST)
	{
		D3D12UnorderedAccessTarget* FBuffer = static_cast<D3D12UnorderedAccessTarget*>(pST);

		if (FBuffer->CurrentState != D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
		{
			D3D12_RESOURCE_BARRIER Barriers[1];
			Barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(FBuffer->GetD3D12Texture(), FBuffer->CurrentState, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			CommandList->ResourceBarrier(ARRAYSIZE(Barriers), Barriers);
			FBuffer->CurrentState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		}

		CommandList->SetComputeRootDescriptorTable(RootParameterIndex, FBuffer->GetUAVGPU());
	}
}

void D3D12ComputeCommandContext::SetUnorderedAccessTargets(std::vector<IUnorderedAccessTarget*> pSTs, std::uint32_t RootParameterIndex)
{
	if (pSTs.size() > 0)
	{
		int i = RootParameterIndex;
		for (IUnorderedAccessTarget* pRT : pSTs)
		{
			D3D12UnorderedAccessTarget* FBuffer = static_cast<D3D12UnorderedAccessTarget*>(pRT);
			if (FBuffer->CurrentState != D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
			{
				D3D12_RESOURCE_BARRIER Barriers[1];
				Barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(FBuffer->GetD3D12Texture(), FBuffer->CurrentState, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
				CommandList->ResourceBarrier(ARRAYSIZE(Barriers), Barriers);
				FBuffer->CurrentState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			}

			CommandList->SetComputeRootDescriptorTable(RootParameterIndex, FBuffer->GetUAVGPU());

			i++;
		}
	}
}

void D3D12ComputeCommandContext::SetUnorderedAccessTargetAsSRV(IUnorderedAccessTarget* pST, std::uint32_t RootParameterIndex)
{
	if (pST)
	{
		D3D12UnorderedAccessTarget* FBuffer = static_cast<D3D12UnorderedAccessTarget*>(pST);

		if (FBuffer->CurrentState != D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)
		{
			D3D12_RESOURCE_BARRIER Barriers[1];
			Barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(FBuffer->GetD3D12Texture(), FBuffer->CurrentState, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			CommandList->ResourceBarrier(ARRAYSIZE(Barriers), Barriers);
			FBuffer->CurrentState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		}

		CommandList->SetComputeRootDescriptorTable(RootParameterIndex, FBuffer->GetSRVGPU());
	}
}

void D3D12ComputeCommandContext::SetUnorderedAccessTargetsAsSRV(std::vector<IUnorderedAccessTarget*> pSTs, std::uint32_t RootParameterIndex)
{
	if (pSTs.size() > 0)
	{
		int i = RootParameterIndex;
		for (IUnorderedAccessTarget* pRT : pSTs)
		{
			D3D12UnorderedAccessTarget* FBuffer = static_cast<D3D12UnorderedAccessTarget*>(pRT);
			if (FBuffer->CurrentState != D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)
			{
				D3D12_RESOURCE_BARRIER Barriers[1];
				Barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(FBuffer->GetD3D12Texture(), FBuffer->CurrentState, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
				CommandList->ResourceBarrier(ARRAYSIZE(Barriers), Barriers);
				FBuffer->CurrentState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
			}

			CommandList->SetComputeRootDescriptorTable(RootParameterIndex, FBuffer->GetSRVGPU());

			i++;
		}
	}
}

void D3D12ComputeCommandContext::SetUnorderedAccessBuffer(IUnorderedAccessBuffer* pUAV, std::uint32_t RootParameterIndex)
{
}

void D3D12ComputeCommandContext::SetUnorderedAccessBuffers(std::vector<IUnorderedAccessBuffer*> UAVs, std::uint32_t RootParameterIndex)
{
}

void D3D12ComputeCommandContext::SetUnorderedAccessBufferAsResource(IUnorderedAccessBuffer* pUAV, std::uint32_t RootParameterIndex)
{
}

void D3D12ComputeCommandContext::SetUnorderedAccessBuffersAsResource(std::vector<IUnorderedAccessBuffer*> UAVs, std::uint32_t RootParameterIndex)
{
}

void D3D12ComputeCommandContext::SetDepthTargetAsResource(IDepthTarget* pDT, std::uint32_t RootParameterIndex)
{
	if (pDT)
	{
		D3D12DepthTarget* FBuffer = static_cast<D3D12DepthTarget*>(pDT);

		if (FBuffer->CurrentState != D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)
		{
			D3D12_RESOURCE_BARRIER Barriers[1];
			Barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(FBuffer->GetD3D12Texture(), FBuffer->CurrentState, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			CommandList->ResourceBarrier(ARRAYSIZE(Barriers), Barriers);
			FBuffer->CurrentState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		}

		CommandList->SetComputeRootDescriptorTable(RootParameterIndex, FBuffer->GetSRVGPU());
	}
}

void D3D12ComputeCommandContext::SetDepthTargetsAsResource(std::vector<IDepthTarget*> DTs, std::uint32_t RootParameterIndex)
{
	if (DTs.size() > 0)
	{
		int i = RootParameterIndex;
		for (IDepthTarget* pRT : DTs)
		{
			D3D12DepthTarget* FBuffer = static_cast<D3D12DepthTarget*>(pRT);
			if (FBuffer->CurrentState != D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)
			{
				D3D12_RESOURCE_BARRIER Barriers[1];
				Barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(FBuffer->GetD3D12Texture(), FBuffer->CurrentState, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
				CommandList->ResourceBarrier(ARRAYSIZE(Barriers), Barriers);
				FBuffer->CurrentState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
			}

			CommandList->SetComputeRootDescriptorTable(RootParameterIndex, FBuffer->GetSRVGPU());

			i++;
		}
	}
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
