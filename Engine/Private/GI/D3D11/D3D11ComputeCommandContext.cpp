/* ---------------------------------------------------------------------------------------
* MIT License
*
* Copyright (c) 2022 Davut Coþkun.
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
#include "D3D11ComputeCommandContext.h"
#include "D3D11Buffer.h"
#include "D3D11Pipeline.h"
#include "D3D11FrameBuffer.h"
#include "D3D11Shader.h"
#include "D3D11Viewport.h"
#include "D3D11Texture.h"

D3D11ComputeCommandContext::D3D11ComputeCommandContext(D3D11Device* InDevice)
	: CommandList(nullptr)
	, DeferredCTX(nullptr)
	, Owner(InDevice)
	, StencilRef(0)
	, bIsSingleThreaded(true)
	, ActivePipeline(nullptr)
{
	bIsSingleThreaded = !Owner->IsNvDeviceID();

	if (!bIsSingleThreaded)
	{
		ComPtr<ID3D11DeviceContext> lDirect3DDeviceIMContext;
		Owner->GetDevice()->CreateDeferredContext(0, lDirect3DDeviceIMContext.GetAddressOf());
		lDirect3DDeviceIMContext->QueryInterface(__uuidof (ID3D11DeviceContext1), (void**)&DeferredCTX);
		lDirect3DDeviceIMContext = nullptr;
	}
	else
	{
		DeferredCTX = Owner->GetDeviceIMContext();
	}
}

D3D11ComputeCommandContext::~D3D11ComputeCommandContext()
{
	ClearCMDStates();
	DeferredCTX->ClearState();
	DeferredCTX->Flush();
	DeferredCTX = nullptr;

	CommandList = nullptr;
	Owner = nullptr;

	ActivePipeline = nullptr;
}

void D3D11ComputeCommandContext::BeginRecordCommandList()
{
	ClearCMDStates();
	ClearState();
}

void D3D11ComputeCommandContext::FinishRecordCommandList()
{
	if (!bIsSingleThreaded)
	{
		DeferredCTX->FinishCommandList(
#if _DEBUG
			false,
#else
			true,
#endif
			CommandList.GetAddressOf());
	}
}

void D3D11ComputeCommandContext::ExecuteCommandList()
{
	if (!bIsSingleThreaded)
	{
		if (!CommandList)
			return;
		Owner->GetDeviceIMContext()->ExecuteCommandList(CommandList.Get(),
#if _DEBUG
			false
#else
			true
#endif
		);
		CommandList = nullptr;
	}
	ActivePipeline = nullptr;
}

void D3D11ComputeCommandContext::ClearState()
{
	DeferredCTX->ClearState();
	StencilRef = 0;
}

void D3D11ComputeCommandContext::ExecuteIndirect(IIndirectBuffer* IndirectBuffer)
{
}

void D3D11ComputeCommandContext::Dispatch(std::uint32_t ThreadGroupCountX, std::uint32_t ThreadGroupCountY, std::uint32_t ThreadGroupCountZ)
{
	DeferredCTX->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
}

void D3D11ComputeCommandContext::SetFrameBuffer(IFrameBuffer* pFB, std::optional<std::size_t> FBOIndex)
{
	static_cast<D3D11FrameBuffer*>(pFB)->ApplyFrameBuffer(DeferredCTX.Get(), FBOIndex);
}

void D3D11ComputeCommandContext::SetPipeline(IComputePipeline* Pipeline)
{
	ActivePipeline = Pipeline ? static_cast<D3D11ComputePipeline*>(Pipeline) : nullptr;
	if (ActivePipeline)
		ActivePipeline->ApplyPipeline(DeferredCTX.Get());
}

void D3D11ComputeCommandContext::SetConstantBuffer(IConstantBuffer* CB, std::optional<std::uint32_t> InRootParameterIndex)
{
}

void D3D11ComputeCommandContext::SetRenderTargetAsResource(IRenderTarget* pRT, std::uint32_t InRootParameterIndex)
{
}

void D3D11ComputeCommandContext::SetRenderTargetsAsResource(std::vector<IRenderTarget*> RTs, std::uint32_t InRootParameterIndex)
{
}

void D3D11ComputeCommandContext::SetUnorderedAccessTarget(IUnorderedAccessTarget* pST, std::uint32_t InRootParameterIndex)
{
}

void D3D11ComputeCommandContext::SetUnorderedAccessTargets(std::vector<IUnorderedAccessTarget*> pSTs, std::uint32_t InRootParameterIndex)
{
}

void D3D11ComputeCommandContext::SetRenderTargetAsUAV(IRenderTarget* pRT, std::uint32_t InRootParameterIndex)
{
}

void D3D11ComputeCommandContext::SetRenderTargetsAsUAV(std::vector<IRenderTarget*> RTs, std::uint32_t InRootParameterIndex)
{
}

void D3D11ComputeCommandContext::SetUnorderedAccessTargetAsSRV(IUnorderedAccessTarget* pST, std::uint32_t InRootParameterIndex)
{
}

void D3D11ComputeCommandContext::SetUnorderedAccessTargetsAsSRV(std::vector<IUnorderedAccessTarget*> pSTs, std::uint32_t InRootParameterIndex)
{
}

void D3D11ComputeCommandContext::SetDepthTargetAsResource(IDepthTarget* pDT, std::uint32_t InRootParameterIndex)
{
}

void D3D11ComputeCommandContext::SetDepthTargetsAsResource(std::vector<IDepthTarget*> DTs, std::uint32_t InRootParameterIndex)
{
}

void D3D11ComputeCommandContext::ClearCMDStates()
{
}
