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
#include "D3D12CommandBuffer.h"
#include "D3D12Buffer.h"
#include "D3D12Shader.h"
#include "D3D12Pipeline.h"
#include "D3D12Viewport.h"
#include "D3D12Texture.h"
#include "GI/D3DShared/D3DShared.h"

D3D12CommandBuffer::D3D12CommandBuffer(D3D12Device* InOwner)
	: Owner(InOwner)
	, StencilRef(0)
	, bIsClosed(false)
	, bWaitForCompletion(false)
	, bIsRenderPassEnabled(false)
	, bIsRenderPassActive(false)
{
	auto Device = Owner->GetDevice();
	CommandAllocator = Owner->RequestCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT);
	ThrowIfFailed(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocator, nullptr, IID_PPV_ARGS(&CommandList)));
	CommandList->Close();
	bIsClosed = true;
#if _DEBUG
	CommandList->SetName(L"D3D12CommandBuffer");
#endif
}

D3D12CommandBuffer::~D3D12CommandBuffer()
{
	RT_ToTransition.clear();
	Depth_ToTransition.clear();

	CommandList = nullptr;
	CommandAllocator = nullptr;
	Owner = nullptr;
}

void D3D12CommandBuffer::BeginRecordCommandList(const ERenderPass RenderPass)
{
	RT_ToTransition.clear();
	Depth_ToTransition.clear();

	Open();

	if (RenderPass != ERenderPass::eNONE)
	{
		Owner->SetHeaps(CommandList.Get());
		if (bIsRenderPassEnabled && RenderPass != ERenderPass::eUI)
			bIsRenderPassActive = true;
	}
	else
	{
		//Owner->CPUWait();
	}
}

void D3D12CommandBuffer::Open()
{
	if (!CommandAllocator)
		CommandAllocator = Owner->RequestCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT);

	ThrowIfFailed(CommandList->Reset(CommandAllocator, nullptr));
	bIsClosed = false;
	bWaitForCompletion = false;
}

void D3D12CommandBuffer::FinishRecordCommandList()
{
	std::vector<D3D12_RESOURCE_BARRIER> Barriers;
	for (const auto& State : RT_ToTransition)
	{
		for (const auto& RT : State.second)
		{
			if (RT->CurrentState != State.first)
			{
				Barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(RT->GetD3D12Texture(), RT->CurrentState, State.first));
				RT->CurrentState = State.first;
			}
		}
	}

	for (const auto& State : Depth_ToTransition)
	{
		for (const auto& Depth : State.second)
		{
			if (Depth->CurrentState != State.first)
			{
				Barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(Depth->GetD3D12Texture(), Depth->CurrentState, State.first));
				Depth->CurrentState = State.first;
			}
		}
	}

	if (Barriers.size() > 0)
		CommandList->ResourceBarrier(Barriers.size(), Barriers.data());
	Barriers.clear();
	RT_ToTransition.clear();
	Depth_ToTransition.clear();

	if (bIsRenderPassActive)
		CommandList->EndRenderPass();

	Close();
}

void D3D12CommandBuffer::Close()
{
	CommandList->Close();
	bIsClosed = true;
}

void D3D12CommandBuffer::ExecuteCommandList()
{
	Owner->ExecuteDirectCommandLists(CommandList.Get()/*, bWaitForCompletion*/);

	Owner->DiscardCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocator);
	CommandAllocator = nullptr;
	bWaitForCompletion = false;
	bIsRenderPassActive = false;
}

void D3D12CommandBuffer::ResourceBarrier(UINT NumBarriers, const D3D12_RESOURCE_BARRIER* pBarriers)
{
	CommandList->ResourceBarrier(NumBarriers, pBarriers);
}

void D3D12CommandBuffer::TransitionTo(D3D12RenderTarget* RT, D3D12_RESOURCE_STATES State)
{
	if (State == D3D12_RESOURCE_STATE_COMMON)
	{
		RT_ToTransition[State].push_back(RT);
		return;
	}

	if (RT->CurrentState != State)
	{
		D3D12_RESOURCE_BARRIER Barriers[1];
		Barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(RT->GetD3D12Texture(), RT->CurrentState, State);
		CommandList->ResourceBarrier(ARRAYSIZE(Barriers), Barriers);
		RT->CurrentState = State;
	}
}

void D3D12CommandBuffer::TransitionTo(D3D12DepthTarget* Depth, D3D12_RESOURCE_STATES State)
{
	if (State == D3D12_RESOURCE_STATE_COMMON)
	{
		Depth_ToTransition[State].push_back(Depth);
		return;
	}

	if (Depth->CurrentState != State)
	{
		D3D12_RESOURCE_BARRIER Barriers[1];
		Barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(Depth->GetD3D12Texture(), Depth->CurrentState, State);
		CommandList->ResourceBarrier(ARRAYSIZE(Barriers), Barriers);
		Depth->CurrentState = State;
	}
}

void D3D12CommandBuffer::CopyResource(ID3D12Resource* pDstResource, D3D12_RESOURCE_STATES DestState, ID3D12Resource* pSrcResource, D3D12_RESOURCE_STATES SrcState)
{
	/*CD3DX12_TEXTURE_BARRIER Barrier[2];
	Barrier[0].AccessBefore = D3D12_BARRIER_ACCESS_RENDER_TARGET;
	Barrier[0].AccessAfter = D3D12_BARRIER_ACCESS_COPY_DEST;
	Barrier[0].SyncBefore = D3D12_BARRIER_SYNC_RENDER_TARGET;
	Barrier[0].SyncAfter = D3D12_BARRIER_SYNC_COPY;
	Barrier[0].LayoutBefore = D3D12_BARRIER_LAYOUT_RENDER_TARGET;
	Barrier[0].LayoutAfter = D3D12_BARRIER_LAYOUT_COPY_DEST;
	Barrier[0].Flags = D3D12_TEXTURE_BARRIER_FLAG_NONE;
	Barrier[0].pResource = RTV[FrameIndex].renderTarget.Get();
	Barrier[0].Subresources = CD3DX12_BARRIER_SUBRESOURCE_RANGE(0xffffffff);

	Barrier[1].AccessBefore = D3D12_BARRIER_ACCESS_RENDER_TARGET;
	Barrier[1].AccessAfter = D3D12_BARRIER_ACCESS_COPY_SOURCE;
	Barrier[1].SyncBefore = D3D12_BARRIER_SYNC_RENDER_TARGET;
	Barrier[1].SyncAfter = D3D12_BARRIER_SYNC_COPY;
	Barrier[1].LayoutBefore = D3D12_BARRIER_LAYOUT_RENDER_TARGET;
	Barrier[1].LayoutAfter = D3D12_BARRIER_LAYOUT_COPY_SOURCE;
	Barrier[1].Flags = D3D12_TEXTURE_BARRIER_FLAG_NONE;
	Barrier[1].pResource = FrameBufferTexture;
	Barrier[1].Subresources = CD3DX12_BARRIER_SUBRESOURCE_RANGE(0xffffffff);*/

	//CD3DX12_BARRIER_GROUP BarrierGroup(2, Barrier);
	//CMDList->Get()->Barrier(1, &BarrierGroup);

	{
		std::vector<D3D12_RESOURCE_BARRIER> preCopyBarriers;
		preCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(pSrcResource, SrcState, D3D12_RESOURCE_STATE_COPY_SOURCE));
		preCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(pDstResource, DestState, D3D12_RESOURCE_STATE_COPY_DEST));
		CommandList->ResourceBarrier((UINT)preCopyBarriers.size(), preCopyBarriers.data());
	}

	CommandList->CopyResource(pDstResource, pSrcResource);

	{
		std::vector<D3D12_RESOURCE_BARRIER> postCopyBarriers;
		postCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(pSrcResource, D3D12_RESOURCE_STATE_COPY_SOURCE, SrcState));
		postCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(pDstResource, D3D12_RESOURCE_STATE_COPY_DEST, DestState));
		CommandList->ResourceBarrier((UINT)postCopyBarriers.size(), postCopyBarriers.data());
	}
}

void D3D12CommandBuffer::WaitForGPU()
{
	Owner->GPUSignal();
	Owner->CPUWait();
}

void D3D12CommandBuffer::SetViewport(const sViewport& Viewport)
{
	D3D12_VIEWPORT viewport = { (float)Viewport.TopLeftX, (float)Viewport.TopLeftY, (float)Viewport.Width,  (float)Viewport.Height, Viewport.MinDepth, Viewport.MaxDepth };
	CommandList->RSSetViewports(1, &viewport);
}

void D3D12CommandBuffer::SetScissorRect(std::uint32_t X, std::uint32_t Y, std::uint32_t Z, std::uint32_t W)
{
	D3D12_RECT Rect;
	Rect.top = X;
	Rect.left = Y;
	Rect.bottom = Z;
	Rect.right = W;
	CommandList->RSSetScissorRects(1, &Rect);
}

void D3D12CommandBuffer::ClearFrameBuffer(IFrameBuffer* pFB)
{
	if (pFB)
	{
		D3D12FrameBuffer* FrameBuffer = static_cast<D3D12FrameBuffer*>(pFB);

		const auto& FrameBuffers = FrameBuffer->RenderTargets;
		for (const auto& FBuffer : FrameBuffers)
		{
			/*if (FBuffer->CurrentState != D3D12_RESOURCE_STATE_RENDER_TARGET)
			{
				D3D12_RESOURCE_BARRIER Barriers[1];
				Barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(FBuffer->GetD3D12Texture(), FBuffer->CurrentState, D3D12_RESOURCE_STATE_RENDER_TARGET);
				CommandList->ResourceBarrier(ARRAYSIZE(Barriers), Barriers);
				FBuffer->CurrentState = D3D12_RESOURCE_STATE_RENDER_TARGET;
			}*/

			TransitionTo(FBuffer.get(), D3D12_RESOURCE_STATE_RENDER_TARGET);

			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(FBuffer->GetRTV()->GetCPU(), 0, Owner->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
			const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
			CommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

			TransitionTo(FBuffer.get(), D3D12_RESOURCE_STATE_COMMON);
		}
		const auto& DepthBuffer = FrameBuffer->DepthTarget;
		if (DepthBuffer)
		{
			TransitionTo(DepthBuffer.get(), D3D12_RESOURCE_STATE_DEPTH_WRITE);

			CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(DepthBuffer->GetDSV()->GetCPU(), 0, Owner->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV));
			const FLOAT clearColor = 0.0f;
			CommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, clearColor, 0, 0, nullptr);

			TransitionTo(DepthBuffer.get(), D3D12_RESOURCE_STATE_COMMON);
		}
	}
}

void D3D12CommandBuffer::ClearRenderTarget(IRenderTarget* pFB, IDepthTarget* DepthTarget)
{
	if (pFB)
	{
		D3D12RenderTarget* FBuffer = static_cast<D3D12RenderTarget*>(pFB);
		{
			/*if (FBuffer->CurrentState != D3D12_RESOURCE_STATE_RENDER_TARGET)
			{
				D3D12_RESOURCE_BARRIER Barriers[1];
				Barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(FBuffer->GetD3D12Texture(), FBuffer->CurrentState, D3D12_RESOURCE_STATE_RENDER_TARGET);
				CommandList->ResourceBarrier(ARRAYSIZE(Barriers), Barriers);
				FBuffer->CurrentState = D3D12_RESOURCE_STATE_RENDER_TARGET;
			}*/

			TransitionTo(FBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);

			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(FBuffer->GetRTV()->GetCPU(), 0, Owner->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
			const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
			CommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

			TransitionTo(FBuffer, D3D12_RESOURCE_STATE_COMMON);
		}
	}
	if (DepthTarget)
	{
		TransitionTo((D3D12DepthTarget*)DepthTarget, D3D12_RESOURCE_STATE_DEPTH_WRITE);

		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(static_cast<D3D12DepthTarget*>(DepthTarget)->GetDSVCPU(), 0, Owner->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV));
		const FLOAT clearColor = 0.0f;
		CommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, clearColor, 0, 0, nullptr);

		TransitionTo((D3D12DepthTarget*)DepthTarget, D3D12_RESOURCE_STATE_COMMON);
	}
}

void D3D12CommandBuffer::ClearRenderTargets(std::vector<IRenderTarget*> pRTs, IDepthTarget* DepthTarget)
{
	if (pRTs.size() > 0)
	{
		for (const auto& RT : pRTs)
		{
			D3D12RenderTarget* FBuffer = static_cast<D3D12RenderTarget*>(RT);
			/*if (FBuffer->CurrentState != D3D12_RESOURCE_STATE_RENDER_TARGET)
			{
				D3D12_RESOURCE_BARRIER Barriers[1];
				Barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(FBuffer->GetD3D12Texture(), FBuffer->CurrentState, D3D12_RESOURCE_STATE_RENDER_TARGET);
				CommandList->ResourceBarrier(ARRAYSIZE(Barriers), Barriers);
				FBuffer->CurrentState = D3D12_RESOURCE_STATE_RENDER_TARGET;
			}*/

			TransitionTo(FBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);

			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(FBuffer->GetRTV()->GetCPU(), 0, Owner->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
			const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
			CommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

			TransitionTo(FBuffer, D3D12_RESOURCE_STATE_COMMON);
		}
	}
	if (DepthTarget)
	{
		TransitionTo((D3D12DepthTarget*)DepthTarget, D3D12_RESOURCE_STATE_DEPTH_WRITE);

		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(static_cast<D3D12DepthTarget*>(DepthTarget)->GetDSVCPU(), 0, Owner->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV));
		const FLOAT clearColor = 0.0f;
		CommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, clearColor, 0, 0, nullptr);

		TransitionTo((D3D12DepthTarget*)DepthTarget, D3D12_RESOURCE_STATE_COMMON);
	}
}

void D3D12CommandBuffer::ClearDepthTarget(IDepthTarget* DepthTarget)
{
	if (DepthTarget)
	{
		TransitionTo((D3D12DepthTarget*)DepthTarget, D3D12_RESOURCE_STATE_DEPTH_WRITE);

		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(static_cast<D3D12DepthTarget*>(DepthTarget)->GetDSVCPU(), 0, Owner->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV));
		const FLOAT clearColor = 0.0f;
		CommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, clearColor, 0, 0, nullptr);

		TransitionTo((D3D12DepthTarget*)DepthTarget, D3D12_RESOURCE_STATE_COMMON);
	}
}

void D3D12CommandBuffer::SetFrameBuffer(IFrameBuffer* pFB, std::optional<std::size_t> FBOIndex)
{
	if (pFB)
	{
		D3D12FrameBuffer* FrameBuffer = static_cast<D3D12FrameBuffer*>(pFB);

		const auto& FrameBuffers = FrameBuffer->RenderTargets;
		std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> RTs;
		if (FBOIndex.has_value())
		{
			const auto& FBuffer = FrameBuffers[*FBOIndex];

			/*if (FBuffer->CurrentState != D3D12_RESOURCE_STATE_RENDER_TARGET)
			{
				D3D12_RESOURCE_BARRIER Barriers[1];
				Barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(FBuffer->GetD3D12Texture(), FBuffer->CurrentState, D3D12_RESOURCE_STATE_RENDER_TARGET);
				CommandList->ResourceBarrier(ARRAYSIZE(Barriers), Barriers);
				FBuffer->CurrentState = D3D12_RESOURCE_STATE_RENDER_TARGET;
			}*/

			TransitionTo(FBuffer.get(), D3D12_RESOURCE_STATE_RENDER_TARGET);

			RTs.push_back(CD3DX12_CPU_DESCRIPTOR_HANDLE(FBuffer->GetRTV()->GetCPU(), 0, Owner->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)));
		}
		else
		{
			for (const auto& FBuffer : FrameBuffers)
			{
				/*if (FBuffer->CurrentState != D3D12_RESOURCE_STATE_RENDER_TARGET)
				{
					D3D12_RESOURCE_BARRIER Barriers[1];
					Barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(FBuffer->GetD3D12Texture(), FBuffer->CurrentState, D3D12_RESOURCE_STATE_RENDER_TARGET);
					CommandList->ResourceBarrier(ARRAYSIZE(Barriers), Barriers);
					FBuffer->CurrentState = D3D12_RESOURCE_STATE_RENDER_TARGET;
				}*/

				TransitionTo(FBuffer.get(), D3D12_RESOURCE_STATE_RENDER_TARGET);

				RTs.push_back(CD3DX12_CPU_DESCRIPTOR_HANDLE(FBuffer->GetRTV()->GetCPU(), 0, Owner->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)));
			}
		}

		if (bIsRenderPassEnabled && bIsRenderPassActive)
		{
			D3D12_RENDER_PASS_BEGINNING_ACCESS renderPassBeginningAccessClear{ D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE, { } };
			D3D12_RENDER_PASS_ENDING_ACCESS renderPassEndingAccessPreserve{ D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE, {} };
			std::vector<D3D12_RENDER_PASS_RENDER_TARGET_DESC> renderPassRenderTargetDesc;
			for (auto RT : RTs)
				renderPassRenderTargetDesc.push_back(D3D12_RENDER_PASS_RENDER_TARGET_DESC(RT, renderPassBeginningAccessClear, renderPassEndingAccessPreserve));

			D3D12_RENDER_PASS_BEGINNING_ACCESS renderPassBeginningAccessNoAccess{ D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS, {} };
			D3D12_RENDER_PASS_ENDING_ACCESS renderPassEndingAccessNoAccess{ D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS, {} };
			
			const auto& DepthBuffer = FrameBuffer->DepthTarget;
			if (DepthBuffer)
			{
				D3D12_RENDER_PASS_DEPTH_STENCIL_DESC renderPassDepthStencilDesc{ DepthBuffer->GetDSVCPU(), renderPassBeginningAccessNoAccess, renderPassBeginningAccessNoAccess, renderPassEndingAccessNoAccess, renderPassEndingAccessNoAccess };
				CommandList->BeginRenderPass(1, renderPassRenderTargetDesc.data(), &renderPassDepthStencilDesc, D3D12_RENDER_PASS_FLAG_NONE);
			}
			else
			{
				CommandList->BeginRenderPass(1, renderPassRenderTargetDesc.data(), nullptr, D3D12_RENDER_PASS_FLAG_NONE);
			}
		}
		else
		{
			const auto& DepthBuffer = FrameBuffer->DepthTarget;
			if (DepthBuffer)
			{
				TransitionTo(DepthBuffer.get(), D3D12_RESOURCE_STATE_DEPTH_WRITE);

				CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(DepthBuffer->GetDSVCPU());
				CommandList->OMSetRenderTargets((UINT)RTs.size(), &RTs[0], FALSE, &dsvHandle);

				TransitionTo(DepthBuffer.get(), D3D12_RESOURCE_STATE_COMMON);
			}
			else
			{
				CommandList->OMSetRenderTargets((UINT)RTs.size(), &RTs[0], FALSE, nullptr);
			}

			if (FBOIndex.has_value())
			{
				const auto& FBuffer = FrameBuffers[*FBOIndex];
				TransitionTo(FBuffer.get(), D3D12_RESOURCE_STATE_COMMON);
			}
			else
			{
				for (const auto& FBuffer : FrameBuffers)
				{
					TransitionTo(FBuffer.get(), D3D12_RESOURCE_STATE_COMMON);
				}
			}
		}
	}
}

void D3D12CommandBuffer::SetRenderTarget(IRenderTarget* pRT, IDepthTarget* DepthTarget)
{
	if (pRT)
	{
		D3D12RenderTarget* FBuffer = static_cast<D3D12RenderTarget*>(pRT);
		/*if (FBuffer->CurrentState != D3D12_RESOURCE_STATE_RENDER_TARGET)
		{
			D3D12_RESOURCE_BARRIER Barriers[1];
			Barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(FBuffer->GetD3D12Texture(), FBuffer->CurrentState, D3D12_RESOURCE_STATE_RENDER_TARGET);
			CommandList->ResourceBarrier(ARRAYSIZE(Barriers), Barriers);
			FBuffer->CurrentState = D3D12_RESOURCE_STATE_RENDER_TARGET;
		}*/

		TransitionTo(FBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);

		CD3DX12_CPU_DESCRIPTOR_HANDLE RT(CD3DX12_CPU_DESCRIPTOR_HANDLE(FBuffer->GetRTVCPU(), 0, Owner->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)));

		if (bIsRenderPassEnabled && bIsRenderPassActive)
		{
			D3D12_RENDER_PASS_BEGINNING_ACCESS renderPassBeginningAccessClear{ D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE, { } };
			D3D12_RENDER_PASS_ENDING_ACCESS renderPassEndingAccessPreserve{ D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE, {} };
			std::vector<D3D12_RENDER_PASS_RENDER_TARGET_DESC> renderPassRenderTargetDesc;
			renderPassRenderTargetDesc.push_back(D3D12_RENDER_PASS_RENDER_TARGET_DESC(RT, renderPassBeginningAccessClear, renderPassEndingAccessPreserve));

			D3D12_RENDER_PASS_BEGINNING_ACCESS renderPassBeginningAccessNoAccess{ D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS, {} };
			D3D12_RENDER_PASS_ENDING_ACCESS renderPassEndingAccessNoAccess{ D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS, {} };

			if (DepthTarget)
			{
				CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(static_cast<D3D12DepthTarget*>(DepthTarget)->GetDSVCPU());
				D3D12_RENDER_PASS_DEPTH_STENCIL_DESC renderPassDepthStencilDesc{ dsvHandle, renderPassBeginningAccessNoAccess, renderPassBeginningAccessNoAccess, renderPassEndingAccessNoAccess, renderPassEndingAccessNoAccess };
				CommandList->BeginRenderPass(1, renderPassRenderTargetDesc.data(), &renderPassDepthStencilDesc, D3D12_RENDER_PASS_FLAG_NONE);
			}
			else
			{
				CommandList->BeginRenderPass(1, renderPassRenderTargetDesc.data(), nullptr, D3D12_RENDER_PASS_FLAG_NONE);
			}
		}
		else
		{
			if (DepthTarget)
			{
				TransitionTo((D3D12DepthTarget*)DepthTarget, D3D12_RESOURCE_STATE_DEPTH_WRITE);

				CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(static_cast<D3D12DepthTarget*>(DepthTarget)->GetDSVCPU());
				CommandList->OMSetRenderTargets(1, &RT, FALSE, &dsvHandle);

				TransitionTo((D3D12DepthTarget*)DepthTarget, D3D12_RESOURCE_STATE_COMMON);
			}
			else
			{
				CommandList->OMSetRenderTargets(1, &RT, FALSE, nullptr);
			}

			TransitionTo(FBuffer, D3D12_RESOURCE_STATE_COMMON);
		}
	}
}

void D3D12CommandBuffer::SetRenderTargets(std::vector<IRenderTarget*> pRTs, IDepthTarget* DepthTarget)
{
	if (pRTs.size() > 0)
	{
		std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> RTs;

		for (const auto& RT : pRTs)
		{
			D3D12RenderTarget* FBuffer = static_cast<D3D12RenderTarget*>(RT);
			/*if (FBuffer->CurrentState != D3D12_RESOURCE_STATE_RENDER_TARGET)
			{
				D3D12_RESOURCE_BARRIER Barriers[1];
				Barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(FBuffer->GetD3D12Texture(), FBuffer->CurrentState, D3D12_RESOURCE_STATE_RENDER_TARGET);
				CommandList->ResourceBarrier(ARRAYSIZE(Barriers), Barriers);
				FBuffer->CurrentState = D3D12_RESOURCE_STATE_RENDER_TARGET;
			}*/

			TransitionTo(FBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);

			RTs.push_back(CD3DX12_CPU_DESCRIPTOR_HANDLE(FBuffer->GetRTV()->GetCPU(), 0, Owner->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)));
		}

		if (bIsRenderPassEnabled && bIsRenderPassActive)
		{
			D3D12_RENDER_PASS_BEGINNING_ACCESS renderPassBeginningAccessClear{ D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE, { } };
			D3D12_RENDER_PASS_ENDING_ACCESS renderPassEndingAccessPreserve{ D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE, {} };
			std::vector<D3D12_RENDER_PASS_RENDER_TARGET_DESC> renderPassRenderTargetDesc;
			for (auto RT : RTs)
				renderPassRenderTargetDesc.push_back(D3D12_RENDER_PASS_RENDER_TARGET_DESC(RT, renderPassBeginningAccessClear, renderPassEndingAccessPreserve));

			D3D12_RENDER_PASS_BEGINNING_ACCESS renderPassBeginningAccessNoAccess{ D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS, {} };
			D3D12_RENDER_PASS_ENDING_ACCESS renderPassEndingAccessNoAccess{ D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS, {} };

			if (DepthTarget)
			{
				CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(static_cast<D3D12DepthTarget*>(DepthTarget)->GetDSVCPU());
				D3D12_RENDER_PASS_DEPTH_STENCIL_DESC renderPassDepthStencilDesc{ dsvHandle, renderPassBeginningAccessNoAccess, renderPassBeginningAccessNoAccess, renderPassEndingAccessNoAccess, renderPassEndingAccessNoAccess };
				CommandList->BeginRenderPass(1, renderPassRenderTargetDesc.data(), &renderPassDepthStencilDesc, D3D12_RENDER_PASS_FLAG_NONE);
			}
			else
			{
				CommandList->BeginRenderPass(1, renderPassRenderTargetDesc.data(), nullptr, D3D12_RENDER_PASS_FLAG_NONE);
			}
		}
		else
		{
			if (DepthTarget)
			{
				TransitionTo((D3D12DepthTarget*)DepthTarget, D3D12_RESOURCE_STATE_DEPTH_WRITE);

				CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(static_cast<D3D12DepthTarget*>(DepthTarget)->GetDSVCPU());
				CommandList->OMSetRenderTargets((UINT)RTs.size(), &RTs[0], FALSE, &dsvHandle);

				TransitionTo((D3D12DepthTarget*)DepthTarget, D3D12_RESOURCE_STATE_COMMON);
			}
			else
			{
				CommandList->OMSetRenderTargets((UINT)RTs.size(), &RTs[0], FALSE, nullptr);
			}

			for (const auto& RT : pRTs)
			{
				D3D12RenderTarget* FBuffer = static_cast<D3D12RenderTarget*>(RT);
				TransitionTo(FBuffer, D3D12_RESOURCE_STATE_COMMON);
			}
		}
	}
}

void D3D12CommandBuffer::SetFrameBufferAsResource(IFrameBuffer* pFB, std::uint32_t RootParameterIndex)
{
	if (pFB)
	{
		D3D12FrameBuffer* FrameBuffer = static_cast<D3D12FrameBuffer*>(pFB);

		const auto& FrameBuffers = FrameBuffer->RenderTargets;

		int i = RootParameterIndex;
		for (const auto& FBuffer : FrameBuffers)
		{
			/*if (FBuffer->CurrentState != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
			{
				D3D12_RESOURCE_BARRIER Barriers[1];
				Barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(FBuffer->GetD3D12Texture(), FBuffer->CurrentState, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				CommandList->ResourceBarrier(ARRAYSIZE(Barriers), Barriers);
				FBuffer->CurrentState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
			}*/

			TransitionTo(FBuffer.get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

			CommandList->SetGraphicsRootDescriptorTable(i, FBuffer->GetSRV()->GetGPU());

			TransitionTo(FBuffer.get(), D3D12_RESOURCE_STATE_COMMON);

			i++;
		}
	}
}

void D3D12CommandBuffer::SetFrameBufferAsResource(IFrameBuffer* pFB, std::uint32_t FBOIndex, std::uint32_t RootParameterIndex)
{
	if (pFB)
	{
		D3D12FrameBuffer* FrameBuffer = static_cast<D3D12FrameBuffer*>(pFB);

		const auto& FrameBuffers = FrameBuffer->RenderTargets;
		if (FBOIndex >= FrameBuffers.size())
			return;
		const auto& FBuffer = FrameBuffers.at(FBOIndex);

		/*if (FBuffer->CurrentState != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
		{
			D3D12_RESOURCE_BARRIER Barriers[1];
			Barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(FBuffer->GetD3D12Texture(), FBuffer->CurrentState, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			CommandList->ResourceBarrier(ARRAYSIZE(Barriers), Barriers);
			FBuffer->CurrentState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		}*/

		TransitionTo(FBuffer.get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		CommandList->SetGraphicsRootDescriptorTable(RootParameterIndex, FBuffer->GetSRV()->GetGPU());

		TransitionTo(FBuffer.get(), D3D12_RESOURCE_STATE_COMMON);
	}
}

void D3D12CommandBuffer::SetRenderTargetAsResource(IRenderTarget* pRT, std::uint32_t RootParameterIndex)
{
	if (pRT)
	{
		D3D12RenderTarget* FBuffer = static_cast<D3D12RenderTarget*>(pRT);

		/*if (FBuffer->CurrentState != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
		{
			D3D12_RESOURCE_BARRIER Barriers[1];
			Barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(FBuffer->GetD3D12Texture(), FBuffer->CurrentState, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			CommandList->ResourceBarrier(ARRAYSIZE(Barriers), Barriers);
			FBuffer->CurrentState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		}*/

		TransitionTo(FBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		CommandList->SetGraphicsRootDescriptorTable(RootParameterIndex, FBuffer->GetSRV()->GetGPU());

		TransitionTo(FBuffer, D3D12_RESOURCE_STATE_COMMON);
	}
}

void D3D12CommandBuffer::SetRenderTargetsAsResource(std::vector<IRenderTarget*> RTs, std::uint32_t RootParameterIndex)
{
	if (RTs.size() > 0)
	{
		int i = RootParameterIndex;
		for (IRenderTarget* pRT : RTs)
		{
			D3D12RenderTarget* FBuffer = static_cast<D3D12RenderTarget*>(pRT);
			if (FBuffer->CurrentState != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
			{
				D3D12_RESOURCE_BARRIER Barriers[1];
				Barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(FBuffer->GetD3D12Texture(), FBuffer->CurrentState, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				CommandList->ResourceBarrier(ARRAYSIZE(Barriers), Barriers);
				FBuffer->CurrentState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
			}

			TransitionTo(FBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

			CommandList->SetGraphicsRootDescriptorTable(i, FBuffer->GetSRV()->GetGPU());

			TransitionTo(FBuffer, D3D12_RESOURCE_STATE_COMMON);

			i++;
		}
	}
}

void D3D12CommandBuffer::CopyFrameBuffer(IFrameBuffer* Dest, std::size_t DestFBOIndex, IFrameBuffer* Source, std::uint32_t SourceFBOIndex)
{
	if (!Dest || !Source)
		return;

	D3D12FrameBuffer* DestFB = static_cast<D3D12FrameBuffer*>(Dest);
	D3D12FrameBuffer* SourceFB = static_cast<D3D12FrameBuffer*>(Source);

	if (DestFBOIndex >= DestFB->RenderTargets.size() || SourceFBOIndex >= SourceFB->RenderTargets.size())
		return;

	auto DestFBO = DestFB->RenderTargets.at(DestFBOIndex);
	auto SourceFBO = SourceFB->RenderTargets.at(SourceFBOIndex);

	D3D12_RESOURCE_STATES DestBeforeState = DestFBO->CurrentState;
	D3D12_RESOURCE_STATES SourceBeforeState = SourceFBO->CurrentState;
	{
		std::vector<D3D12_RESOURCE_BARRIER> preCopyBarriers;
		if (DestFBO->CurrentState != D3D12_RESOURCE_STATE_COPY_DEST)
		{
			preCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(DestFBO->GetD3D12Texture(), DestFBO->CurrentState, D3D12_RESOURCE_STATE_COPY_DEST));
			DestFBO->CurrentState = D3D12_RESOURCE_STATE_COPY_DEST;
		}
		if (SourceFBO->CurrentState != D3D12_RESOURCE_STATE_COPY_SOURCE)
		{
			preCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(SourceFBO->GetD3D12Texture(), SourceFBO->CurrentState, D3D12_RESOURCE_STATE_COPY_SOURCE));
			SourceFBO->CurrentState = D3D12_RESOURCE_STATE_COPY_SOURCE;
		}
		CommandList->ResourceBarrier((UINT)preCopyBarriers.size(), preCopyBarriers.data());
	}

	CommandList->CopyResource(DestFBO->GetD3D12Texture(), SourceFBO->GetD3D12Texture());

	{
		std::vector<D3D12_RESOURCE_BARRIER> postCopyBarriers;
		if (DestFBO->CurrentState != DestBeforeState)
		{
			postCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(DestFBO->GetD3D12Texture(), DestFBO->CurrentState, DestBeforeState));
			DestFBO->CurrentState = DestBeforeState;
		}
		if (SourceFBO->CurrentState != SourceBeforeState)
		{
			postCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(SourceFBO->GetD3D12Texture(), SourceFBO->CurrentState, SourceBeforeState));
			SourceFBO->CurrentState = SourceBeforeState;
		}
		CommandList->ResourceBarrier((UINT)postCopyBarriers.size(), postCopyBarriers.data());
	}

	bWaitForCompletion = true;
}

void D3D12CommandBuffer::CopyFrameBufferDepth(IFrameBuffer* Dest, IFrameBuffer* Source)
{
	if (!Dest || !Source)
		return;

	D3D12FrameBuffer* DestFB = static_cast<D3D12FrameBuffer*>(Dest);
	D3D12FrameBuffer* SourceFB = static_cast<D3D12FrameBuffer*>(Source);

	if (!DestFB->DepthTarget || !SourceFB->DepthTarget)
		return;

	auto DestFBO = DestFB->DepthTarget;
	auto SourceFBO = SourceFB->DepthTarget;

	D3D12_RESOURCE_STATES DestBeforeState = DestFBO->CurrentState;
	D3D12_RESOURCE_STATES SourceBeforeState = SourceFBO->CurrentState;
	{
		std::vector<D3D12_RESOURCE_BARRIER> preCopyBarriers;
		if (DestFBO->CurrentState != D3D12_RESOURCE_STATE_COPY_DEST)
		{
			preCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(DestFBO->GetD3D12Texture(), DestFBO->CurrentState, D3D12_RESOURCE_STATE_COPY_DEST));
			DestFBO->CurrentState = D3D12_RESOURCE_STATE_COPY_DEST;
		}
		if (SourceFBO->CurrentState != D3D12_RESOURCE_STATE_COPY_SOURCE)
		{
			preCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(SourceFBO->GetD3D12Texture(), SourceFBO->CurrentState, D3D12_RESOURCE_STATE_COPY_SOURCE));
			SourceFBO->CurrentState = D3D12_RESOURCE_STATE_COPY_SOURCE;
		}
		CommandList->ResourceBarrier((UINT)preCopyBarriers.size(), preCopyBarriers.data());
	}

	CommandList->CopyResource(DestFBO->GetD3D12Texture(), SourceFBO->GetD3D12Texture());

	{
		std::vector<D3D12_RESOURCE_BARRIER> postCopyBarriers;
		if (DestFBO->CurrentState != DestBeforeState)
		{
			postCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(DestFBO->GetD3D12Texture(), DestFBO->CurrentState, DestBeforeState));
			DestFBO->CurrentState = DestBeforeState;
		}
		if (SourceFBO->CurrentState != SourceBeforeState)
		{
			postCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(SourceFBO->GetD3D12Texture(), SourceFBO->CurrentState, SourceBeforeState));
			SourceFBO->CurrentState = SourceBeforeState;
		}
		CommandList->ResourceBarrier((UINT)postCopyBarriers.size(), postCopyBarriers.data());
	}

	bWaitForCompletion = true;
}

void D3D12CommandBuffer::CopyRenderTarget(IRenderTarget* Dest, IRenderTarget* Source)
{
	if (!Dest || !Source)
		return;

	D3D12RenderTarget* DestFBO = static_cast<D3D12RenderTarget*>(Dest);
	D3D12RenderTarget* SourceFBO = static_cast<D3D12RenderTarget*>(Source);

	D3D12_RESOURCE_STATES DestBeforeState = DestFBO->CurrentState;
	D3D12_RESOURCE_STATES SourceBeforeState = SourceFBO->CurrentState;
	{
		std::vector<D3D12_RESOURCE_BARRIER> preCopyBarriers;
		if (DestFBO->CurrentState != D3D12_RESOURCE_STATE_COPY_DEST)
		{
			preCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(DestFBO->GetD3D12Texture(), DestFBO->CurrentState, D3D12_RESOURCE_STATE_COPY_DEST));
			DestFBO->CurrentState = D3D12_RESOURCE_STATE_COPY_DEST;
		}
		if (SourceFBO->CurrentState != D3D12_RESOURCE_STATE_COPY_SOURCE)
		{
			preCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(SourceFBO->GetD3D12Texture(), SourceFBO->CurrentState, D3D12_RESOURCE_STATE_COPY_SOURCE));
			SourceFBO->CurrentState = D3D12_RESOURCE_STATE_COPY_SOURCE;
		}
		CommandList->ResourceBarrier((UINT)preCopyBarriers.size(), preCopyBarriers.data());
	}

	CommandList->CopyResource(DestFBO->GetD3D12Texture(), SourceFBO->GetD3D12Texture());

	{
		std::vector<D3D12_RESOURCE_BARRIER> postCopyBarriers;
		if (DestFBO->CurrentState != DestBeforeState)
		{
			postCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(DestFBO->GetD3D12Texture(), DestFBO->CurrentState, DestBeforeState));
			DestFBO->CurrentState = DestBeforeState;
		}
		if (SourceFBO->CurrentState != SourceBeforeState)
		{
			postCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(SourceFBO->GetD3D12Texture(), SourceFBO->CurrentState, SourceBeforeState));
			SourceFBO->CurrentState = SourceBeforeState;
		}
		CommandList->ResourceBarrier((UINT)postCopyBarriers.size(), postCopyBarriers.data());
	}

	bWaitForCompletion = true;
}

void D3D12CommandBuffer::CopyDepthBuffer(IDepthTarget* Dest, IDepthTarget* Source)
{
	if (!Dest || !Source)
		return;

	D3D12DepthTarget* DestFBO = static_cast<D3D12DepthTarget*>(Dest);
	D3D12DepthTarget* SourceFBO = static_cast<D3D12DepthTarget*>(Source);

	D3D12_RESOURCE_STATES DestBeforeState = DestFBO->CurrentState;
	D3D12_RESOURCE_STATES SourceBeforeState = SourceFBO->CurrentState;
	{
		std::vector<D3D12_RESOURCE_BARRIER> preCopyBarriers;
		if (DestFBO->CurrentState != D3D12_RESOURCE_STATE_COPY_DEST)
		{
			preCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(DestFBO->GetD3D12Texture(), DestFBO->CurrentState, D3D12_RESOURCE_STATE_COPY_DEST));
			DestFBO->CurrentState = D3D12_RESOURCE_STATE_COPY_DEST;
		}
		if (SourceFBO->CurrentState != D3D12_RESOURCE_STATE_COPY_SOURCE)
		{
			preCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(SourceFBO->GetD3D12Texture(), SourceFBO->CurrentState, D3D12_RESOURCE_STATE_COPY_SOURCE));
			SourceFBO->CurrentState = D3D12_RESOURCE_STATE_COPY_SOURCE;
		}
		CommandList->ResourceBarrier((UINT)preCopyBarriers.size(), preCopyBarriers.data());
	}

	CommandList->CopyResource(DestFBO->GetD3D12Texture(), SourceFBO->GetD3D12Texture());

	{
		std::vector<D3D12_RESOURCE_BARRIER> postCopyBarriers;
		if (DestFBO->CurrentState != DestBeforeState)
		{
			postCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(DestFBO->GetD3D12Texture(), DestFBO->CurrentState, DestBeforeState));
			DestFBO->CurrentState = DestBeforeState;
		}
		if (SourceFBO->CurrentState != SourceBeforeState)
		{
			postCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(SourceFBO->GetD3D12Texture(), SourceFBO->CurrentState, SourceBeforeState));
			SourceFBO->CurrentState = SourceBeforeState;
		}
		CommandList->ResourceBarrier((UINT)postCopyBarriers.size(), postCopyBarriers.data());
	}

	bWaitForCompletion = true;
}

void D3D12CommandBuffer::SetUnorderedAccessBufferAsResource(IUnorderedAccessBuffer* pUAV, std::uint32_t RootParameterIndex)
{
}

void D3D12CommandBuffer::SetUnorderedAccessBuffersAsResource(std::vector<IUnorderedAccessBuffer*> UAVs, std::uint32_t RootParameterIndex)
{
}

void D3D12CommandBuffer::UpdateSubresource(ID3D12Resource* Buffer, D3D12_RESOURCE_STATES State, D3D12UploadBuffer* UploadBuffer, BufferSubresource* Subresource)
{
	if (!Buffer || !UploadBuffer || !Subresource)
		return;

	memcpy((BYTE*)UploadBuffer->GetData() + Subresource->Location, Subresource->pSysMem, Subresource->Size);

	D3D12_RESOURCE_BARRIER preCopyBarriers = CD3DX12_RESOURCE_BARRIER::Transition(Buffer, State, D3D12_RESOURCE_STATE_COPY_DEST);
	CommandList->ResourceBarrier(1, &preCopyBarriers);

	CommandList->CopyBufferRegion(Buffer, (UINT64)Subresource->Location, UploadBuffer->Get(), (UINT64)Subresource->Location, (UINT64)Subresource->Size);

	D3D12_RESOURCE_BARRIER postCopyBarriers = CD3DX12_RESOURCE_BARRIER::Transition(Buffer, D3D12_RESOURCE_STATE_COPY_DEST, State);
	CommandList->ResourceBarrier(1, &postCopyBarriers);
}

void D3D12CommandBuffer::SetPipeline(IPipeline* Pipeline)
{
	if (Pipeline)
	{
		static_cast<D3D12Pipeline*>(Pipeline)->ApplyPipeline(CommandList.Get());
	}
}

void D3D12CommandBuffer::SetVertexBuffer(IVertexBuffer* VB, std::uint32_t Slot)
{
	if (VB)
	{
		static_cast<D3D12VertexBuffer*>(VB)->ApplyBuffer(CommandList.Get(), Slot);
	}
}

void D3D12CommandBuffer::SetIndexBuffer(IIndexBuffer* IB)
{
	if (IB)
	{
		static_cast<D3D12IndexBuffer*>(IB)->ApplyBuffer(CommandList.Get());
	}
}

void D3D12CommandBuffer::SetConstantBuffer(IConstantBuffer* CB, std::optional<std::uint32_t> RootParameterIndex)
{
	if (CB)
	{
		if (RootParameterIndex.has_value())
			static_cast<D3D12ConstantBuffer*>(CB)->ApplyConstantBuffer(CommandList.Get(), *RootParameterIndex);
		else
			static_cast<D3D12ConstantBuffer*>(CB)->ApplyConstantBuffer(CommandList.Get());
	}
}

void D3D12CommandBuffer::SetTexture2D(ITexture2D* Texture2D, std::optional<std::uint32_t> RootParameterIndex)
{
	if (Texture2D)
	{
		if (RootParameterIndex.has_value())
			static_cast<D3D12Texture*>(Texture2D)->ApplyTexture(CommandList.Get(), *RootParameterIndex);
		else
			static_cast<D3D12Texture*>(Texture2D)->ApplyTexture(CommandList.Get());
	}
}

void D3D12CommandBuffer::UpdateBufferSubresource(IVertexBuffer* Buffer, BufferSubresource* Subresource)
{
	if (!Subresource || !Buffer)
		return;

	UpdateBufferSubresource(Buffer, Subresource->Location, Subresource->Size, Subresource->pSysMem);
}

void D3D12CommandBuffer::UpdateBufferSubresource(IVertexBuffer* Buffer, std::size_t Location, std::size_t Size, const void* pSrcData)
{
	if (!Buffer || !pSrcData)
		return;

	BufferSubresource Subresource;
	Subresource.Location = Location;
	Subresource.pSysMem = const_cast<void*>(pSrcData);
	Subresource.Size = Size;

	static_cast<D3D12VertexBuffer*>(Buffer)->UpdateSubresource(&Subresource, this);
}

void D3D12CommandBuffer::UpdateBufferSubresource(IIndexBuffer* Buffer, BufferSubresource* Subresource)
{
	if (!Subresource || !Buffer)
		return;

	UpdateBufferSubresource(Buffer, Subresource->Location, Subresource->Size, Subresource->pSysMem);
}

void D3D12CommandBuffer::UpdateBufferSubresource(IIndexBuffer* Buffer, std::size_t Location, std::size_t Size, const void* pSrcData)
{
	if (!Buffer || !pSrcData)
		return;

	BufferSubresource Subresource;
	Subresource.Location = Location;
	Subresource.pSysMem = const_cast<void*>(pSrcData);
	Subresource.Size = Size;

	static_cast<D3D12IndexBuffer*>(Buffer)->UpdateSubresource(&Subresource, this);
}

void D3D12CommandBuffer::Draw(std::uint32_t VertexCount, std::uint32_t VertexStartOffset)
{
	CommandList->DrawInstanced(VertexCount, 1, VertexStartOffset, 0);
}

void D3D12CommandBuffer::DrawInstanced(std::uint32_t VertexCountPerInstance, std::uint32_t InstanceCount, std::uint32_t StartVertexLocation, std::uint32_t StartInstanceLocation)
{
	CommandList->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

void D3D12CommandBuffer::DrawIndexedInstanced(std::uint32_t IndexCountPerInstance, std::uint32_t InstanceCount, std::uint32_t StartIndexLocation, std::int32_t BaseVertexLocation, std::uint32_t StartInstanceLocation)
{
	CommandList->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

void D3D12CommandBuffer::DrawIndexedInstanced(const sObjectDrawParameters& DrawParameters)
{
	DrawIndexedInstanced(
		DrawParameters.IndexCountPerInstance,
		DrawParameters.InstanceCount,
		DrawParameters.StartIndexLocation,
		DrawParameters.BaseVertexLocation,
		DrawParameters.StartInstanceLocation);
}

void D3D12CommandBuffer::ExecuteIndirect(IIndirectBuffer* IndirectBuffer)
{
}

void D3D12CommandBuffer::ClearState()
{
	StencilRef = 0;
	CommandList->ClearState(nullptr);
	CommandList->Reset(CommandAllocator, nullptr);
	CommandList->Close();

	bIsClosed = true;
	bWaitForCompletion = false;
}

D3D12CopyCommandBuffer::D3D12CopyCommandBuffer(D3D12Device* InDevice)
	: Owner(InDevice)
	, bIsClosed(false)
	, bWaitForCompletion(false)
{
	auto Device = Owner->GetDevice();
	CommandAllocator = Owner->RequestCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY);
	ThrowIfFailed(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, CommandAllocator, nullptr, IID_PPV_ARGS(&CommandList)));
	CommandList->Close();
	bIsClosed = true;
#if _DEBUG
	CommandList->SetName(L"D3D12CommandBuffer");
#endif
}

D3D12CopyCommandBuffer::~D3D12CopyCommandBuffer()
{
	CommandList = nullptr;
	CommandAllocator = nullptr;
	Owner = nullptr;
}

void D3D12CopyCommandBuffer::BeginRecordCommandList()
{
	WaitForGPU();
	Open();
}

void D3D12CopyCommandBuffer::FinishRecordCommandList()
{
	Close();
}

void D3D12CopyCommandBuffer::ExecuteCommandList()
{
	Owner->ExecuteCopyCommandLists(CommandList.Get(), bWaitForCompletion);

	Owner->DiscardCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, CommandAllocator);
	CommandAllocator = nullptr;
	bWaitForCompletion = false;
}

void D3D12CopyCommandBuffer::Open()
{
	if (!CommandAllocator)
		CommandAllocator = Owner->RequestCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY);

	ThrowIfFailed(CommandList->Reset(CommandAllocator, nullptr));
	bIsClosed = false;
	bWaitForCompletion = false;
}

void D3D12CopyCommandBuffer::Close()
{
	CommandList->Close();
	bIsClosed = true;
}

void D3D12CopyCommandBuffer::ResourceBarrier(UINT NumBarriers, const D3D12_RESOURCE_BARRIER* pBarriers)
{
	CommandList->ResourceBarrier(NumBarriers, pBarriers);
}

void D3D12CopyCommandBuffer::WaitForGPU()
{
	Owner->GPUSignal();
	Owner->CPUWait();
}

void D3D12CopyCommandBuffer::CopyResource(ID3D12Resource* pDstResource, D3D12_RESOURCE_STATES DestState, ID3D12Resource* pSrcResource, D3D12_RESOURCE_STATES SrcState)
{
	/*CD3DX12_TEXTURE_BARRIER Barrier[2];
	Barrier[0].AccessBefore = D3D12_BARRIER_ACCESS_RENDER_TARGET;
	Barrier[0].AccessAfter = D3D12_BARRIER_ACCESS_COPY_DEST;
	Barrier[0].SyncBefore = D3D12_BARRIER_SYNC_RENDER_TARGET;
	Barrier[0].SyncAfter = D3D12_BARRIER_SYNC_COPY;
	Barrier[0].LayoutBefore = D3D12_BARRIER_LAYOUT_RENDER_TARGET;
	Barrier[0].LayoutAfter = D3D12_BARRIER_LAYOUT_COPY_DEST;
	Barrier[0].Flags = D3D12_TEXTURE_BARRIER_FLAG_NONE;
	Barrier[0].pResource = RTV[FrameIndex].renderTarget.Get();
	Barrier[0].Subresources = CD3DX12_BARRIER_SUBRESOURCE_RANGE(0xffffffff);

	Barrier[1].AccessBefore = D3D12_BARRIER_ACCESS_RENDER_TARGET;
	Barrier[1].AccessAfter = D3D12_BARRIER_ACCESS_COPY_SOURCE;
	Barrier[1].SyncBefore = D3D12_BARRIER_SYNC_RENDER_TARGET;
	Barrier[1].SyncAfter = D3D12_BARRIER_SYNC_COPY;
	Barrier[1].LayoutBefore = D3D12_BARRIER_LAYOUT_RENDER_TARGET;
	Barrier[1].LayoutAfter = D3D12_BARRIER_LAYOUT_COPY_SOURCE;
	Barrier[1].Flags = D3D12_TEXTURE_BARRIER_FLAG_NONE;
	Barrier[1].pResource = FrameBufferTexture;
	Barrier[1].Subresources = CD3DX12_BARRIER_SUBRESOURCE_RANGE(0xffffffff);*/

	//CD3DX12_BARRIER_GROUP BarrierGroup(2, Barrier);
	//CMDList->Get()->Barrier(1, &BarrierGroup);

	/*{
		std::vector<D3D12_RESOURCE_BARRIER> preCopyBarriers;
		preCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(pSrcResource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE));
		preCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(pDstResource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
		CommandList->ResourceBarrier((UINT)preCopyBarriers.size(), preCopyBarriers.data());
	}*/

	CommandList->CopyResource(pDstResource, pSrcResource);

	/*{
		std::vector<D3D12_RESOURCE_BARRIER> postCopyBarriers;
		postCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(pSrcResource, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON));
		postCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(pDstResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON));
		CommandList->ResourceBarrier((UINT)postCopyBarriers.size(), postCopyBarriers.data());
	}*/
}

void D3D12CopyCommandBuffer::CopyFrameBuffer(IFrameBuffer* Dest, std::size_t DestFBOIndex, IFrameBuffer* Source, std::uint32_t SourceFBOIndex)
{
	if (!Dest || !Source)
		return;

	D3D12FrameBuffer* DestFB = static_cast<D3D12FrameBuffer*>(Dest);
	D3D12FrameBuffer* SourceFB = static_cast<D3D12FrameBuffer*>(Source);

	if (DestFBOIndex >= DestFB->RenderTargets.size() || SourceFBOIndex >= SourceFB->RenderTargets.size())
		return;

	auto DestFBO = DestFB->RenderTargets.at(DestFBOIndex);
	auto SourceFBO = SourceFB->RenderTargets.at(SourceFBOIndex);

	D3D12_RESOURCE_STATES DestBeforeState = DestFBO->CurrentState;
	D3D12_RESOURCE_STATES SourceBeforeState = SourceFBO->CurrentState;
	{
		/*{
			auto CMD = Owner->GetIMCommandList();
			CMD->BeginRecordCommandList();
			std::vector<D3D12_RESOURCE_BARRIER> preCopyBarriers;
			if (DestFBO->CurrentState != D3D12_RESOURCE_STATE_COMMON)
			{
				preCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(DestFBO->GetD3D12Texture(), DestFBO->CurrentState, D3D12_RESOURCE_STATE_COMMON));
				DestFBO->CurrentState = D3D12_RESOURCE_STATE_COMMON;
			}
			if (SourceFBO->CurrentState != D3D12_RESOURCE_STATE_COMMON)
			{
				preCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(SourceFBO->GetD3D12Texture(), SourceFBO->CurrentState, D3D12_RESOURCE_STATE_COMMON));
				SourceFBO->CurrentState = D3D12_RESOURCE_STATE_COMMON;
			}
			CMD->Get()->ResourceBarrier((UINT)preCopyBarriers.size(), preCopyBarriers.data());
			CMD->FinishRecordCommandList();
			CMD->ExecuteCommandList();
			CMD->WaitForGPU();
		}*/
		
		std::vector<D3D12_RESOURCE_BARRIER> preCopyBarriers;
		if (DestFBO->CurrentState != D3D12_RESOURCE_STATE_COPY_DEST)
		{
			preCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(DestFBO->GetD3D12Texture(), DestFBO->CurrentState, D3D12_RESOURCE_STATE_COPY_DEST));
			DestFBO->CurrentState = D3D12_RESOURCE_STATE_COPY_DEST;
		}
		if (SourceFBO->CurrentState != D3D12_RESOURCE_STATE_COPY_SOURCE)
		{
			preCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(SourceFBO->GetD3D12Texture(), SourceFBO->CurrentState, D3D12_RESOURCE_STATE_COPY_SOURCE));
			SourceFBO->CurrentState = D3D12_RESOURCE_STATE_COPY_SOURCE;
		}
		CommandList->ResourceBarrier((UINT)preCopyBarriers.size(), preCopyBarriers.data());
	}

	CommandList->CopyResource(DestFBO->GetD3D12Texture(), SourceFBO->GetD3D12Texture());

	{
		std::vector<D3D12_RESOURCE_BARRIER> postCopyBarriers;
		if (DestFBO->CurrentState != DestBeforeState)
		{
			postCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(DestFBO->GetD3D12Texture(), DestFBO->CurrentState, D3D12_RESOURCE_STATE_COMMON));
			DestFBO->CurrentState = D3D12_RESOURCE_STATE_COMMON;
		}
		if (SourceFBO->CurrentState != SourceBeforeState)
		{
			postCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(SourceFBO->GetD3D12Texture(), SourceFBO->CurrentState, D3D12_RESOURCE_STATE_COMMON));
			SourceFBO->CurrentState = D3D12_RESOURCE_STATE_COMMON;
		}
		CommandList->ResourceBarrier((UINT)postCopyBarriers.size(), postCopyBarriers.data());
	}
	bWaitForCompletion = true;
}

void D3D12CopyCommandBuffer::CopyFrameBufferDepth(IFrameBuffer* Dest, IFrameBuffer* Source)
{
	if (!Dest || !Source)
		return;

	D3D12FrameBuffer* DestFB = static_cast<D3D12FrameBuffer*>(Dest);
	D3D12FrameBuffer* SourceFB = static_cast<D3D12FrameBuffer*>(Source);

	if (!DestFB->DepthTarget || !SourceFB->DepthTarget)
		return;

	auto DestFBO = DestFB->DepthTarget;
	auto SourceFBO = SourceFB->DepthTarget;

	D3D12_RESOURCE_STATES DestBeforeState = DestFBO->CurrentState;
	D3D12_RESOURCE_STATES SourceBeforeState = SourceFBO->CurrentState;
	{
		std::vector<D3D12_RESOURCE_BARRIER> preCopyBarriers;
		if (DestFBO->CurrentState != D3D12_RESOURCE_STATE_COPY_DEST)
		{
			preCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(DestFBO->GetD3D12Texture(), DestFBO->CurrentState, D3D12_RESOURCE_STATE_COPY_DEST));
			DestFBO->CurrentState = D3D12_RESOURCE_STATE_COPY_DEST;
		}
		if (SourceFBO->CurrentState != D3D12_RESOURCE_STATE_COPY_SOURCE)
		{
			preCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(SourceFBO->GetD3D12Texture(), SourceFBO->CurrentState, D3D12_RESOURCE_STATE_COPY_SOURCE));
			SourceFBO->CurrentState = D3D12_RESOURCE_STATE_COPY_SOURCE;
		}
		CommandList->ResourceBarrier((UINT)preCopyBarriers.size(), preCopyBarriers.data());
	}

	CommandList->CopyResource(DestFBO->GetD3D12Texture(), SourceFBO->GetD3D12Texture());

	{
		std::vector<D3D12_RESOURCE_BARRIER> postCopyBarriers;
		if (DestFBO->CurrentState != DestBeforeState)
		{
			postCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(DestFBO->GetD3D12Texture(), DestFBO->CurrentState, D3D12_RESOURCE_STATE_COMMON));
			DestFBO->CurrentState = DestBeforeState;
		}
		if (SourceFBO->CurrentState != SourceBeforeState)
		{
			postCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(SourceFBO->GetD3D12Texture(), SourceFBO->CurrentState, D3D12_RESOURCE_STATE_COMMON));
			SourceFBO->CurrentState = SourceBeforeState;
		}
		CommandList->ResourceBarrier((UINT)postCopyBarriers.size(), postCopyBarriers.data());
	}
	bWaitForCompletion = true;
}

void D3D12CopyCommandBuffer::UpdateSubresource(ID3D12Resource* Buffer, D3D12_RESOURCE_STATES State, D3D12UploadBuffer* UploadBuffer, BufferSubresource* Subresource)
{
	if (!Buffer || !UploadBuffer || !Subresource)
		return;

	memcpy((BYTE*)UploadBuffer->GetData() + Subresource->Location, Subresource->pSysMem, Subresource->Size);

	D3D12_RESOURCE_BARRIER preCopyBarriers = CD3DX12_RESOURCE_BARRIER::Transition(Buffer, State, D3D12_RESOURCE_STATE_COPY_DEST);
	CommandList->ResourceBarrier(1, &preCopyBarriers);

	CommandList->CopyBufferRegion(Buffer, (UINT64)Subresource->Location, UploadBuffer->Get(), (UINT64)Subresource->Location, (UINT64)Subresource->Size);

	D3D12_RESOURCE_BARRIER postCopyBarriers = CD3DX12_RESOURCE_BARRIER::Transition(Buffer, D3D12_RESOURCE_STATE_COMMON, State);
	CommandList->ResourceBarrier(1, &postCopyBarriers);
}

void D3D12CopyCommandBuffer::UpdateBufferSubresource(IVertexBuffer* Buffer, BufferSubresource* Subresource)
{
	if (!Subresource || !Buffer)
		return;

	UpdateBufferSubresource(Buffer, Subresource->Location, Subresource->Size, Subresource->pSysMem);
}

void D3D12CopyCommandBuffer::UpdateBufferSubresource(IVertexBuffer* Buffer, std::size_t Location, std::size_t Size, const void* pSrcData)
{
	if (!Buffer || !pSrcData)
		return;

	BufferSubresource Subresource;
	Subresource.Location = Location;
	Subresource.pSysMem = const_cast<void*>(pSrcData);
	Subresource.Size = Size;

	D3D12VertexBuffer* pd3dBuffer = static_cast<D3D12VertexBuffer*>(Buffer);
	UpdateSubresource(pd3dBuffer->GetBuffer(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pd3dBuffer->GetUploadBuffer(), &Subresource);
}

void D3D12CopyCommandBuffer::UpdateBufferSubresource(IIndexBuffer* Buffer, BufferSubresource* Subresource)
{
	if (!Subresource || !Buffer)
		return;

	UpdateBufferSubresource(Buffer, Subresource->Location, Subresource->Size, Subresource->pSysMem);
}

void D3D12CopyCommandBuffer::UpdateBufferSubresource(IIndexBuffer* Buffer, std::size_t Location, std::size_t Size, const void* pSrcData)
{
	if (!Buffer || !pSrcData)
		return;

	BufferSubresource Subresource;
	Subresource.Location = Location;
	Subresource.pSysMem = const_cast<void*>(pSrcData);
	Subresource.Size = Size;

	D3D12IndexBuffer* pd3dBuffer = static_cast<D3D12IndexBuffer*>(Buffer);
	UpdateSubresource(pd3dBuffer->GetBuffer(), D3D12_RESOURCE_STATE_INDEX_BUFFER, pd3dBuffer->GetUploadBuffer(), &Subresource);
}

void D3D12CopyCommandBuffer::ClearState()
{
	CommandList->ClearState(nullptr);
	CommandList->Reset(CommandAllocator, nullptr);
	CommandList->Close();

	bIsClosed = true;
	bWaitForCompletion = false;
}
