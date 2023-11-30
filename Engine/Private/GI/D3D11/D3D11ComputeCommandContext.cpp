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
	if (!CB)
		return;

	if (ActivePipeline)
	{
		const auto& D3D11CB = static_cast<D3D11ConstantBuffer*>(CB);
		const auto& RootParameterIndex = InRootParameterIndex.has_value() ? *InRootParameterIndex : D3D11CB->GetDefaultRootParameterIndex();

		if (RootParameterIndex >= ActivePipeline->GetDescriptorSetLayoutBindingSize())
			return;

		const auto& Binding = ActivePipeline->GetDescriptorSetLayoutBinding(RootParameterIndex);

#if _DEBUG
		if (Binding.GetDescriptorType() != EDescriptorType::eUniformBuffer)
			throw std::runtime_error("Wrong Root Parameter Index.");
#endif

		D3D11CB->ApplyConstantBuffer(DeferredCTX.Get(), Binding.Location, Binding.ShaderType);
	}
	else
	{
		throw std::runtime_error("Pipeline is not set.");
	}
}

void D3D11ComputeCommandContext::SetRenderTargetAsResource(IRenderTarget* pRT, std::uint32_t InRootParameterIndex)
{
	if (!pRT)
		return;

	if (ActivePipeline)
	{
		const auto& RootParameterIndex = InRootParameterIndex;

		if (RootParameterIndex >= ActivePipeline->GetDescriptorSetLayoutBindingSize())
			return;

		const auto& Binding = ActivePipeline->GetDescriptorSetLayoutBinding(RootParameterIndex);

#if _DEBUG
		if (Binding.GetDescriptorType() != EDescriptorType::eTexture)
			throw std::runtime_error("Wrong Root Parameter Index.");
#endif
		const int i = Binding.Location;

		D3D11RenderTarget* RT = static_cast<D3D11RenderTarget*>(pRT);

		if (const auto& SRV = RT->GetD3D11SRV())
		{
			DeferredCTX->CSSetShaderResources(i, 1, &SRV);
		}
	}
	else
	{
		throw std::runtime_error("Pipeline is not set.");
	}
}

void D3D11ComputeCommandContext::SetRenderTargetsAsResource(std::vector<IRenderTarget*> RTs, std::uint32_t InRootParameterIndex)
{
	if (RTs.size() == 0)
		return;

	if (ActivePipeline)
	{
		const auto& RootParameterIndex = InRootParameterIndex;

		if (RootParameterIndex >= ActivePipeline->GetDescriptorSetLayoutBindingSize())
			return;

		const auto& Binding = ActivePipeline->GetDescriptorSetLayoutBinding(RootParameterIndex);

#if _DEBUG
		if (Binding.GetDescriptorType() != EDescriptorType::eTexture)
			throw std::runtime_error("Wrong Root Parameter Index.");
#endif

		int i = Binding.Location;
		for (IRenderTarget* pRT : RTs)
		{
			D3D11RenderTarget* RT = static_cast<D3D11RenderTarget*>(pRT);
			if (const auto& SRV = RT->GetD3D11SRV())
			{
				DeferredCTX->CSSetShaderResources(i, 1, &SRV);
			}
			i++;
		}
	}
	else
	{
		throw std::runtime_error("Pipeline is not set.");
	}
}

void D3D11ComputeCommandContext::SetUnorderedAccessTarget(IUnorderedAccessTarget* pST, std::uint32_t InRootParameterIndex)
{
	if (!pST)
		return;

	if (ActivePipeline)
	{
		const auto& RootParameterIndex = InRootParameterIndex;

		if (RootParameterIndex >= ActivePipeline->GetDescriptorSetLayoutBindingSize())
			return;

		const auto& Binding = ActivePipeline->GetDescriptorSetLayoutBinding(RootParameterIndex);

#if _DEBUG
		if (Binding.GetDescriptorType() != EDescriptorType::eUAV)
			throw std::runtime_error("Wrong Root Parameter Index.");
#endif
		const int i = Binding.Location;

		D3D11UnorderedAccessTarget* RT = static_cast<D3D11UnorderedAccessTarget*>(pST);
		UINT Count = 1;

		if (const auto& UAV = RT->GetD3D11UAV())
		{
			DeferredCTX->CSSetUnorderedAccessViews(i, 1, &UAV, &Count);
		}
	}
	else
	{
		throw std::runtime_error("Pipeline is not set.");
	}
}

void D3D11ComputeCommandContext::SetUnorderedAccessTargets(std::vector<IUnorderedAccessTarget*> pSTs, std::uint32_t InRootParameterIndex)
{
	if (pSTs.size() == 0)
		return;

	if (ActivePipeline)
	{
		const auto& RootParameterIndex = InRootParameterIndex;

		if (RootParameterIndex >= ActivePipeline->GetDescriptorSetLayoutBindingSize())
			return;

		const auto& Binding = ActivePipeline->GetDescriptorSetLayoutBinding(RootParameterIndex);

#if _DEBUG
		if (Binding.GetDescriptorType() != EDescriptorType::eUAV)
			throw std::runtime_error("Wrong Root Parameter Index.");
#endif

		int i = Binding.Location;
		for (IUnorderedAccessTarget* pRT : pSTs)
		{
			D3D11UnorderedAccessTarget* RT = static_cast<D3D11UnorderedAccessTarget*>(pRT);
			UINT Count = 1;

			if (const auto& UAV = RT->GetD3D11UAV())
			{
				DeferredCTX->CSSetUnorderedAccessViews(i, 1, &UAV, &Count);
			}
			i++;
		}
	}
	else
	{
		throw std::runtime_error("Pipeline is not set.");
	}
}

void D3D11ComputeCommandContext::SetRenderTargetAsUAV(IRenderTarget* pRT, std::uint32_t InRootParameterIndex)
{
	if (!pRT)
		return;

	if (ActivePipeline)
	{
		const auto& RootParameterIndex = InRootParameterIndex;

		if (RootParameterIndex >= ActivePipeline->GetDescriptorSetLayoutBindingSize())
			return;

		const auto& Binding = ActivePipeline->GetDescriptorSetLayoutBinding(RootParameterIndex);

#if _DEBUG
		if (Binding.GetDescriptorType() != EDescriptorType::eTexture)
			throw std::runtime_error("Wrong Root Parameter Index.");
#endif
		const int i = Binding.Location;

		D3D11RenderTarget* RT = static_cast<D3D11RenderTarget*>(pRT);
		UINT Count = 1;

		if (!RT->IsUAV_Allowed())
		{
			throw std::runtime_error("UAV is not allowed.");
			return;
		}

		if (const auto& UAV = RT->GetD3D11UAV())
		{
			DeferredCTX->CSSetUnorderedAccessViews(i, 1, &UAV, &Count);
		}
	}
	else
	{
		throw std::runtime_error("Pipeline is not set.");
	}
}

void D3D11ComputeCommandContext::SetRenderTargetsAsUAV(std::vector<IRenderTarget*> RTs, std::uint32_t InRootParameterIndex)
{
	if (RTs.size() == 0)
		return;

	if (ActivePipeline)
	{
		const auto& RootParameterIndex = InRootParameterIndex;

		if (RootParameterIndex >= ActivePipeline->GetDescriptorSetLayoutBindingSize())
			return;

		const auto& Binding = ActivePipeline->GetDescriptorSetLayoutBinding(RootParameterIndex);

#if _DEBUG
		if (Binding.GetDescriptorType() != EDescriptorType::eTexture)
			throw std::runtime_error("Wrong Root Parameter Index.");
#endif

		int i = Binding.Location;
		for (IRenderTarget* pRT : RTs)
		{
			D3D11RenderTarget* RT = static_cast<D3D11RenderTarget*>(pRT);
			UINT Count = 1;

			if (!RT->IsUAV_Allowed())
			{
				//throw std::runtime_error("UAV is not allowed.");
				continue;
			}

			if (const auto& UAV = RT->GetD3D11UAV())
			{
				DeferredCTX->CSSetUnorderedAccessViews(i, 1, &UAV, &Count);
			}
			i++;
		}
	}
	else
	{
		throw std::runtime_error("Pipeline is not set.");
	}
}

void D3D11ComputeCommandContext::SetUnorderedAccessTargetAsSRV(IUnorderedAccessTarget* pST, std::uint32_t InRootParameterIndex)
{
	if (!pST)
		return;

	if (ActivePipeline)
	{
		const auto& RootParameterIndex = InRootParameterIndex;

		if (RootParameterIndex >= ActivePipeline->GetDescriptorSetLayoutBindingSize())
			return;

		const auto& Binding = ActivePipeline->GetDescriptorSetLayoutBinding(RootParameterIndex);

#if _DEBUG
		if (Binding.GetDescriptorType() != EDescriptorType::eTexture)
			throw std::runtime_error("Wrong Root Parameter Index.");
#endif
		const int i = Binding.Location;

		D3D11UnorderedAccessTarget* RT = static_cast<D3D11UnorderedAccessTarget*>(pST);

		if (const auto& SRV = RT->GetD3D11SRV())
		{
			DeferredCTX->CSSetShaderResources(i, 1, &SRV);
		}
	}
	else
	{
		throw std::runtime_error("Pipeline is not set.");
	}
}

void D3D11ComputeCommandContext::SetUnorderedAccessTargetsAsSRV(std::vector<IUnorderedAccessTarget*> pSTs, std::uint32_t InRootParameterIndex)
{
	if (pSTs.size() == 0)
		return;

	if (ActivePipeline)
	{
		const auto& RootParameterIndex = InRootParameterIndex;

		if (RootParameterIndex >= ActivePipeline->GetDescriptorSetLayoutBindingSize())
			return;

		const auto& Binding = ActivePipeline->GetDescriptorSetLayoutBinding(RootParameterIndex);

#if _DEBUG
		if (Binding.GetDescriptorType() != EDescriptorType::eTexture)
			throw std::runtime_error("Wrong Root Parameter Index.");
#endif

		int i = Binding.Location;
		for (IUnorderedAccessTarget* pRT : pSTs)
		{
			D3D11UnorderedAccessTarget* RT = static_cast<D3D11UnorderedAccessTarget*>(pRT);
			if (const auto& SRV = RT->GetD3D11SRV())
			{
				DeferredCTX->CSSetShaderResources(i, 1, &SRV);
			}
			i++;
		}
	}
	else
	{
		throw std::runtime_error("Pipeline is not set.");
	}
}

void D3D11ComputeCommandContext::SetUnorderedAccessBuffer(IUnorderedAccessBuffer* pUAV, std::uint32_t RootParameterIndex)
{
}

void D3D11ComputeCommandContext::SetUnorderedAccessBuffers(std::vector<IUnorderedAccessBuffer*> UAVs, std::uint32_t RootParameterIndex)
{
}

void D3D11ComputeCommandContext::SetUnorderedAccessBufferAsResource(IUnorderedAccessBuffer* pUAV, std::uint32_t RootParameterIndex)
{
}

void D3D11ComputeCommandContext::SetUnorderedAccessBuffersAsResource(std::vector<IUnorderedAccessBuffer*> UAVs, std::uint32_t RootParameterIndex)
{
}

void D3D11ComputeCommandContext::SetDepthTargetAsResource(IDepthTarget* pDT, std::uint32_t InRootParameterIndex)
{
	if (!pDT)
		return;

	if (ActivePipeline)
	{
		const auto& RootParameterIndex = InRootParameterIndex;

		if (RootParameterIndex >= ActivePipeline->GetDescriptorSetLayoutBindingSize())
			return;

		const auto& Binding = ActivePipeline->GetDescriptorSetLayoutBinding(RootParameterIndex);

#if _DEBUG
		if (Binding.GetDescriptorType() != EDescriptorType::eTexture)
			throw std::runtime_error("Wrong Root Parameter Index.");
#endif
		const int i = Binding.Location;

		D3D11DepthTarget* RT = static_cast<D3D11DepthTarget*>(pDT);

		if (const auto& SRV = RT->GetD3D11SRV())
		{
			DeferredCTX->CSSetShaderResources(i, 1, &SRV);
		}
	}
	else
	{
		throw std::runtime_error("Pipeline is not set.");
	}
}

void D3D11ComputeCommandContext::SetDepthTargetsAsResource(std::vector<IDepthTarget*> DTs, std::uint32_t InRootParameterIndex)
{
	if (DTs.size() == 0)
		return;

	if (ActivePipeline)
	{
		const auto& RootParameterIndex = InRootParameterIndex;

		if (RootParameterIndex >= ActivePipeline->GetDescriptorSetLayoutBindingSize())
			return;

		const auto& Binding = ActivePipeline->GetDescriptorSetLayoutBinding(RootParameterIndex);

#if _DEBUG
		if (Binding.GetDescriptorType() != EDescriptorType::eTexture)
			throw std::runtime_error("Wrong Root Parameter Index.");
#endif

		int i = Binding.Location;
		for (IDepthTarget* pRT : DTs)
		{
			D3D11DepthTarget* RT = static_cast<D3D11DepthTarget*>(pRT);
			if (const auto& SRV = RT->GetD3D11SRV())
			{
				DeferredCTX->CSSetShaderResources(i, 1, &SRV);
			}
			i++;
		}
	}
	else
	{
		throw std::runtime_error("Pipeline is not set.");
	}
}

void D3D11ComputeCommandContext::ClearCMDStates()
{
	//
	// Unbind shaders
	//
	DeferredCTX->CSSetShader(NULL, NULL, 0);

	ID3D11ShaderResourceView* pSRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = { 0 };
	DeferredCTX->CSSetShaderResources(0, (std::uint32_t)std::size(pSRVs), pSRVs);

	ID3D11UnorderedAccessView* pUAVs[D3D11_PS_CS_UAV_REGISTER_COUNT] = { 0 };
	std::uint32_t pUAVInitialCounts[D3D11_PS_CS_UAV_REGISTER_COUNT] = { 0 };
	DeferredCTX->CSSetUnorderedAccessViews(0, (std::uint32_t)std::size(pUAVs), pUAVs, pUAVInitialCounts);

	ID3D11Buffer* pCBs[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT] = { 0 };
	DeferredCTX->CSSetConstantBuffers(0, (std::uint32_t)std::size(pCBs), pCBs);

	ID3D11SamplerState* pSs[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT] = { 0 };
	DeferredCTX->CSSetSamplers(NULL, (std::uint32_t)std::size(pSs), pSs);
}
