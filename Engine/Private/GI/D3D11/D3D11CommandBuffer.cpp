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
#include "D3D11CommandBuffer.h"
#include "D3D11Buffer.h"
#include "D3D11Pipeline.h"
#include "D3D11FrameBuffer.h"
#include "D3D11Shader.h"
#include "D3D11Viewport.h"
#include "D3D11Texture.h"

D3D11CommandBuffer::D3D11CommandBuffer(D3D11Device* InDevice)
	: CommandList(nullptr)
	, DeferredCTX(nullptr)
	, Owner(InDevice)
	, StencilRef(0)
	, bIsSingleThreaded(false)
	, ActivePipeline(nullptr)
{
	bIsSingleThreaded = !Owner->IsNvDeviceID();

	if (!bIsSingleThreaded)
	{
		ComPtr<ID3D11DeviceContext> lDirect3DDeviceIMContext;
		Owner->GetDevice()->CreateDeferredContext(0, lDirect3DDeviceIMContext.GetAddressOf());
		lDirect3DDeviceIMContext->QueryInterface(__uuidof (ID3D11DeviceContext1), (void **)&DeferredCTX);
		lDirect3DDeviceIMContext = nullptr;
	} 
	else
	{
		DeferredCTX = Owner->GetDeviceIMContext();
	}
}

D3D11CommandBuffer::~D3D11CommandBuffer()
{
	ClearCMDStates();
	DeferredCTX->ClearState();
	DeferredCTX->Flush();
	DeferredCTX = nullptr;

	CommandList = nullptr;
	Owner = nullptr;

	ActivePipeline = nullptr;
}

void D3D11CommandBuffer::BeginRecordCommandList(const ERenderPass RenderPass)
{
	ClearCMDStates();
	ClearState(); 
}

void D3D11CommandBuffer::FinishRecordCommandList()
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

void D3D11CommandBuffer::ExecuteCommandList()
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

void D3D11CommandBuffer::ClearState()
{
	DeferredCTX->ClearState();
	StencilRef = 0;
}

void D3D11CommandBuffer::Draw(std::uint32_t VertexCount, std::uint32_t VertexStartOffset)
{
#if _DEBUG
	if (!ActivePipeline)
	{
		throw std::runtime_error("Pipeline is not set.");
		return;
	}
#endif

	DeferredCTX->Draw(VertexCount, VertexStartOffset);
}

void D3D11CommandBuffer::DrawInstanced(std::uint32_t VertexCountPerInstance, std::uint32_t InstanceCount, std::uint32_t StartVertexLocation, std::uint32_t StartInstanceLocation)
{
#if _DEBUG
	if (!ActivePipeline)
	{
		throw std::runtime_error("Pipeline is not set.");
		return;
	}
#endif

	DeferredCTX->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

void D3D11CommandBuffer::DrawIndexedInstanced(std::uint32_t IndexCountPerInstance, std::uint32_t InstanceCount, std::uint32_t StartIndexLocation, std::int32_t BaseVertexLocation, std::uint32_t StartInstanceLocation)
{
#if _DEBUG
	if (!ActivePipeline)
	{
		throw std::runtime_error("Pipeline is not set.");
		return;
	}
#endif

	DeferredCTX->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);

	ID3D11ShaderResourceView* pSRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = { 0 };
	DeferredCTX->PSSetShaderResources(0, (std::uint32_t)std::size(pSRVs), pSRVs);

	ID3D11Buffer* pCBs[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT] = { 0 };
	DeferredCTX->PSSetConstantBuffers(0, (std::uint32_t)std::size(pCBs), pCBs);
}

void D3D11CommandBuffer::DrawIndexedInstanced(const sObjectDrawParameters& DrawParameters)
{
#if _DEBUG
	if (!ActivePipeline)
	{
		throw std::runtime_error("Pipeline is not set.");
		return;
	}
#endif

	DrawIndexedInstanced(
		DrawParameters.IndexCountPerInstance,
		DrawParameters.InstanceCount,
		DrawParameters.StartIndexLocation,
		DrawParameters.BaseVertexLocation,
		DrawParameters.StartInstanceLocation);
}

void D3D11CommandBuffer::ExecuteIndirect(IIndirectBuffer* IndirectBuffer)
{
}

void D3D11CommandBuffer::SetViewport(const sViewport& Viewport)
{
	D3D11_VIEWPORT VP;
	VP.Width = (float)Viewport.Width;
	VP.Height = (float)Viewport.Height;
	VP.MinDepth = (float)Viewport.MinDepth;
	VP.MaxDepth = (float)Viewport.MaxDepth;
	VP.TopLeftX = (float)Viewport.TopLeftX;
	VP.TopLeftY = (float)Viewport.TopLeftY;
	DeferredCTX->RSSetViewports(1, &VP);
}

void D3D11CommandBuffer::SetScissorRect(std::uint32_t X, std::uint32_t Y, std::uint32_t Z, std::uint32_t W)
{
	D3D11_RECT Rect;
	Rect.top = X;
	Rect.left = Y;
	Rect.bottom = Z;
	Rect.right = W;
	DeferredCTX->RSSetScissorRects(1, &Rect);
}

void D3D11CommandBuffer::SetStencilRef(std::uint32_t Ref)
{
	if (ActivePipeline)
	{
		StencilRef = Ref;
		ActivePipeline->SetStencilRef(DeferredCTX.Get(), StencilRef);
	}
	else
	{
		throw std::runtime_error("Pipeline is not set.");
	}
}

void D3D11CommandBuffer::ClearFrameBuffer(IFrameBuffer* pFB)
{
	static_cast<D3D11FrameBuffer*>(pFB)->ClearRTs(this);
}

void D3D11CommandBuffer::ClearRenderTarget(IRenderTarget* pRT, IDepthTarget* DepthTarget)
{
	{
		D3D11RenderTarget* RT = static_cast<D3D11RenderTarget*>(pRT);
		float zeros[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		DeferredCTX->ClearRenderTargetView(RT->GetD3D11RTV(), zeros);
	}

	if (DepthTarget)
	{
		D3D11DepthTarget* Depth = static_cast<D3D11DepthTarget*>(DepthTarget);
		std::uint32_t ClearFlags = 0;
		ClearFlags |= D3D11_CLEAR_DEPTH;
		ClearFlags |= D3D11_CLEAR_STENCIL;
		DeferredCTX->ClearDepthStencilView(Depth->GetD3D11DSV(), ClearFlags, 0.0f, (UINT8)0);
	}
}

void D3D11CommandBuffer::ClearRenderTargets(std::vector<IRenderTarget*> pRTs, IDepthTarget* DepthTarget)
{
	for (auto& pRT : pRTs)
	{
		D3D11RenderTarget* RT = static_cast<D3D11RenderTarget*>(pRT);
		float zeros[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		DeferredCTX->ClearRenderTargetView(RT->GetD3D11RTV(), zeros);
	}

	if (DepthTarget)
	{
		D3D11DepthTarget* Depth = static_cast<D3D11DepthTarget*>(DepthTarget);
		std::uint32_t ClearFlags = 0;
		ClearFlags |= D3D11_CLEAR_DEPTH;
		ClearFlags |= D3D11_CLEAR_STENCIL;
		DeferredCTX->ClearDepthStencilView(Depth->GetD3D11DSV(), ClearFlags, 0.0f, (UINT8)0);
	}
}

void D3D11CommandBuffer::ClearDepthTarget(IDepthTarget* DepthTarget)
{
	if (DepthTarget)
	{
		D3D11DepthTarget* Depth = static_cast<D3D11DepthTarget*>(DepthTarget);
		std::uint32_t ClearFlags = 0;
		ClearFlags |= D3D11_CLEAR_DEPTH;
		ClearFlags |= D3D11_CLEAR_STENCIL;
		DeferredCTX->ClearDepthStencilView(Depth->GetD3D11DSV(), ClearFlags, 0.0f, (UINT8)0);
	}
}

void D3D11CommandBuffer::SetFrameBuffer(IFrameBuffer* pFB, std::optional<std::size_t> FBOIndex)
{
	static_cast<D3D11FrameBuffer*>(pFB)->ApplyFrameBuffer(this, FBOIndex);
}

void D3D11CommandBuffer::SetRenderTarget(IRenderTarget* pRT, IDepthTarget* DepthTarget)
{
	ID3D11RenderTargetView* RTV = static_cast<D3D11RenderTarget*>(pRT)->GetD3D11RTV();
	if (DepthTarget)
	{
		ID3D11DepthStencilView* Depth = static_cast<D3D11DepthTarget*>(DepthTarget)->GetD3D11DSV();
		DeferredCTX->OMSetRenderTargets(1, &RTV, Depth);
	}
	else
	{
		DeferredCTX->OMSetRenderTargets(1, &RTV, nullptr);
	}
}

void D3D11CommandBuffer::SetRenderTargets(std::vector<IRenderTarget*> pRTs, IDepthTarget* DepthTarget)
{
	std::vector<ID3D11RenderTargetView*> RenderTargetElements;

	for (auto& RT : pRTs)
	{
		RenderTargetElements.push_back(static_cast<D3D11RenderTarget*>(RT)->GetD3D11RTV());
	}

	if (DepthTarget)
	{
		ID3D11DepthStencilView* Depth = static_cast<D3D11DepthTarget*>(DepthTarget)->GetD3D11DSV();
		DeferredCTX->OMSetRenderTargets(static_cast<std::uint32_t>(RenderTargetElements.size()), RenderTargetElements.data(), Depth);
	}
	else
	{
		DeferredCTX->OMSetRenderTargets(static_cast<std::uint32_t>(RenderTargetElements.size()), RenderTargetElements.data(), nullptr);
	}
}

void D3D11CommandBuffer::SetFrameBufferAsResource(IFrameBuffer* pFB, std::uint32_t InRootParameterIndex)
{
	if (!pFB)
		return;

	if (ActivePipeline)
	{
		const auto& D3D11FB = static_cast<D3D11FrameBuffer*>(pFB);
		const auto& RootParameterIndex = InRootParameterIndex;

		if (RootParameterIndex >= ActivePipeline->GetDescriptorSetLayoutBindingSize())
			return;

		const auto& Binding = ActivePipeline->GetDescriptorSetLayoutBinding(RootParameterIndex);

#if _DEBUG
		if (Binding.GetDescriptorType() != EDescriptorType::eTexture)
			throw std::runtime_error("Wrong Root Parameter Index.");
#endif

		D3D11FB->ApplyFrameBufferAsResource(DeferredCTX.Get(), Binding.Location, Binding.ShaderType);
	}
	else
	{
		throw std::runtime_error("Pipeline is not set.");
	}
}

void D3D11CommandBuffer::SetFrameBufferAsResource(IFrameBuffer* pFB, std::uint32_t FBOIndex, std::uint32_t InRootParameterIndex)
{
	if (!pFB)
		return;

	if (ActivePipeline)
	{
		const auto& D3D11FB = static_cast<D3D11FrameBuffer*>(pFB);
		const auto& RootParameterIndex = InRootParameterIndex;

		if (RootParameterIndex >= ActivePipeline->GetDescriptorSetLayoutBindingSize())
			return;

		const auto& Binding = ActivePipeline->GetDescriptorSetLayoutBinding(RootParameterIndex);

#if _DEBUG
		if (Binding.GetDescriptorType() != EDescriptorType::eTexture)
			throw std::runtime_error("Wrong Root Parameter Index.");
#endif

		D3D11FB->ApplyFrameBufferAsResource(DeferredCTX.Get(), FBOIndex, Binding.Location, Binding.ShaderType);
	}
	else
	{
		throw std::runtime_error("Pipeline is not set.");
	}
}

void D3D11CommandBuffer::SetRenderTargetAsResource(IRenderTarget* pRT, std::uint32_t InRootParameterIndex)
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
			switch (Binding.ShaderType)
			{
			case eShaderType::Vertex:
				DeferredCTX->VSSetShaderResources(i, 1, &SRV);
				break;
			case eShaderType::Pixel:
				DeferredCTX->PSSetShaderResources(i, 1, &SRV);
				break;
			case eShaderType::Geometry:
				DeferredCTX->GSSetShaderResources(i, 1, &SRV);
				break;
			case eShaderType::Compute:
				DeferredCTX->CSSetShaderResources(i, 1, &SRV);
				break;
			case eShaderType::HULL:
				DeferredCTX->HSSetShaderResources(i, 1, &SRV);
				break;
			case eShaderType::Domain:
				DeferredCTX->DSSetShaderResources(i, 1, &SRV);
				break;
			}
		}
	}
	else
	{
		throw std::runtime_error("Pipeline is not set.");
	}
}

void D3D11CommandBuffer::SetRenderTargetsAsResource(std::vector<IRenderTarget*> RTs, std::uint32_t InRootParameterIndex)
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
				switch (Binding.ShaderType)
				{
				case eShaderType::Vertex:
					DeferredCTX->VSSetShaderResources(i, 1, &SRV);
					break;
				case eShaderType::Pixel:
					DeferredCTX->PSSetShaderResources(i, 1, &SRV);
					break;
				case eShaderType::Geometry:
					DeferredCTX->GSSetShaderResources(i, 1, &SRV);
					break;
				case eShaderType::Compute:
					DeferredCTX->CSSetShaderResources(i, 1, &SRV);
					break;
				case eShaderType::HULL:
					DeferredCTX->HSSetShaderResources(i, 1, &SRV);
					break;
				case eShaderType::Domain:
					DeferredCTX->DSSetShaderResources(i, 1, &SRV);
					break;
				}
			}
			i++;
		}
	}
	else
	{
		throw std::runtime_error("Pipeline is not set.");
	}
}

void D3D11CommandBuffer::CopyFrameBuffer(IFrameBuffer* Dest, std::size_t DestFBOIndex, IFrameBuffer* Source, std::uint32_t SourceFBOIndex)
{
	if (!Dest || !Source)
		return;

	D3D11FrameBuffer* DestFB = static_cast<D3D11FrameBuffer*>(Dest);
	D3D11FrameBuffer* SourceFB = static_cast<D3D11FrameBuffer*>(Source);
	if (DestFBOIndex >= DestFB->RenderTargets.size() || SourceFBOIndex >= SourceFB->RenderTargets.size())
		return;
	DeferredCTX->CopyResource(DestFB->RenderTargets.at(DestFBOIndex)->GetD3D11Texture(), SourceFB->RenderTargets.at(SourceFBOIndex)->GetD3D11Texture());
}

void D3D11CommandBuffer::CopyFrameBufferDepth(IFrameBuffer* Dest, IFrameBuffer* Source)
{
	if (!Dest || !Source)
		return;

	D3D11FrameBuffer* DestFB = static_cast<D3D11FrameBuffer*>(Dest);
	D3D11FrameBuffer* SourceFB = static_cast<D3D11FrameBuffer*>(Source);
	if (!DestFB->DepthTarget || SourceFB->DepthTarget)
		return;
	DeferredCTX->CopyResource(DestFB->DepthTarget->GetD3D11Texture(), SourceFB->DepthTarget->GetD3D11Texture());
}

void D3D11CommandBuffer::CopyRenderTarget(IRenderTarget* Dest, IRenderTarget* Source)
{
	if (!Dest || !Source)
		return;

	D3D11RenderTarget* DestFB = static_cast<D3D11RenderTarget*>(Dest);
	D3D11RenderTarget* SourceFB = static_cast<D3D11RenderTarget*>(Source);

	DeferredCTX->CopyResource(DestFB->GetD3D11Texture(), SourceFB->GetD3D11Texture());
}

void D3D11CommandBuffer::CopyDepthBuffer(IDepthTarget* Dest, IDepthTarget* Source)
{
	if (!Dest || !Source)
		return;

	D3D11DepthTarget* DestFB = static_cast<D3D11DepthTarget*>(Dest);
	D3D11DepthTarget* SourceFB = static_cast<D3D11DepthTarget*>(Source);

	DeferredCTX->CopyResource(DestFB->GetD3D11Texture(), SourceFB->GetD3D11Texture());
}

void D3D11CommandBuffer::SetUnorderedAccessBufferAsResource(IUnorderedAccessBuffer* pUAV, std::uint32_t RootParameterIndex)
{
}

void D3D11CommandBuffer::SetUnorderedAccessBuffersAsResource(std::vector<IUnorderedAccessBuffer*> UAVs, std::uint32_t RootParameterIndex)
{
}

void D3D11CommandBuffer::SetPipeline(IPipeline* Pipeline)
{
	ActivePipeline = Pipeline ? static_cast<D3D11Pipeline*>(Pipeline) : nullptr;
	if (ActivePipeline)
		ActivePipeline->ApplyPipeline(DeferredCTX.Get(), StencilRef);
}

void D3D11CommandBuffer::SetConstantBuffer(IConstantBuffer* CB, std::optional<std::uint32_t> InRootParameterIndex)
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

void D3D11CommandBuffer::SetTexture2D(ITexture2D* Texture2D, std::optional<std::uint32_t> InRootParameterIndex)
{
	if (ActivePipeline)
	{
		const auto& D3D11Tex = static_cast<D3D11Texture*>(Texture2D);
		const auto& RootParameterIndex = InRootParameterIndex.has_value() ? *InRootParameterIndex : D3D11Tex->GetDefaultRootParameterIndex();

		if (RootParameterIndex >= ActivePipeline->GetDescriptorSetLayoutBindingSize())
			return;

		const auto& Binding = ActivePipeline->GetDescriptorSetLayoutBinding(RootParameterIndex);

#if _DEBUG
		if (Binding.GetDescriptorType() != EDescriptorType::eTexture)
			throw std::runtime_error("Wrong Root Parameter Index.");
#endif

		D3D11Tex->ApplyTexture(DeferredCTX.Get(), Binding.Location, Binding.ShaderType);
	}
	else
	{
		throw std::runtime_error("Pipeline is not set.");
	}
}

void D3D11CommandBuffer::SetVertexBuffer(IVertexBuffer* VBuffer)
{
	if (!VBuffer)
		return;

	static_cast<D3D11VertexBuffer*>(VBuffer)->ApplyBuffer(this);
}

void D3D11CommandBuffer::SetIndexBuffer(IIndexBuffer* IBuffer)
{
	if (!IBuffer)
		return;

	static_cast<D3D11IndexBuffer*>(IBuffer)->ApplyBuffer(this);
}

void D3D11CommandBuffer::UpdateBufferSubresource(IVertexBuffer* Buffer, sBufferSubresource* Subresource)
{
	if (!Subresource || !Buffer)
		return;

	UpdateBufferSubresource(Buffer, Subresource->Location, Subresource->Size, Subresource->pSysMem);
}

void D3D11CommandBuffer::UpdateBufferSubresource(IVertexBuffer* Buffer, std::size_t Location, std::size_t Size, const void* pSrcData)
{
	if (!Buffer)
		return;

	sBufferSubresource Subresource;
	Subresource.Location = Location;
	Subresource.Size = Size;
	Subresource.pSysMem = const_cast<void*>(pSrcData);
	static_cast<D3D11VertexBuffer*>(Buffer)->UpdateSubresource(&Subresource, this);
}

void D3D11CommandBuffer::UpdateBufferSubresource(IIndexBuffer* Buffer, sBufferSubresource* Subresource)
{
	if (!Subresource || !Buffer)
		return;

	UpdateBufferSubresource(Buffer, Subresource->Location, Subresource->Size, Subresource->pSysMem);
}

void D3D11CommandBuffer::UpdateBufferSubresource(IIndexBuffer* Buffer, std::size_t Location, std::size_t Size, const void* pSrcData)
{
	if (!Buffer)
		return;

	sBufferSubresource Subresource;
	Subresource.Location = Location;
	Subresource.Size = Size;
	Subresource.pSysMem = const_cast<void*>(pSrcData);
	static_cast<D3D11IndexBuffer*>(Buffer)->UpdateSubresource(&Subresource, this);
}

void D3D11CommandBuffer::ClearCMDStates()
{
	// DeferredCTX->ClearState();
	//
	// Unbind IB and VB
	//

	ID3D11Buffer* pVBs[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT] = { 0 };
	std::uint32_t countsAndOffsets[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT] = { 0 };
	DeferredCTX->IASetInputLayout(NULL);
	DeferredCTX->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	DeferredCTX->IASetVertexBuffers(0, (std::uint32_t)std::size(pVBs), pVBs, countsAndOffsets, countsAndOffsets);

	//
	// Unbind shaders
	//
	DeferredCTX->VSSetShader(NULL, NULL, 0);
	DeferredCTX->GSSetShader(NULL, NULL, 0);
	DeferredCTX->PSSetShader(NULL, NULL, 0);
	DeferredCTX->CSSetShader(NULL, NULL, 0);

	//
	// Unbind resources
	//
	ID3D11RenderTargetView *pRTVs[8] = { nullptr };
	DeferredCTX->OMSetRenderTargets((std::uint32_t)std::size(pRTVs), pRTVs, NULL);

	ID3D11ShaderResourceView* pSRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = { 0 };
	DeferredCTX->VSSetShaderResources(0, (std::uint32_t)std::size(pSRVs), pSRVs);
	DeferredCTX->GSSetShaderResources(0, (std::uint32_t)(std::uint32_t)std::size(pSRVs), pSRVs);
	DeferredCTX->PSSetShaderResources(0, (std::uint32_t)std::size(pSRVs), pSRVs);
	DeferredCTX->CSSetShaderResources(0, (std::uint32_t)std::size(pSRVs), pSRVs);

	ID3D11UnorderedAccessView* pUAVs[D3D11_PS_CS_UAV_REGISTER_COUNT] = { 0 };
	std::uint32_t pUAVInitialCounts[D3D11_PS_CS_UAV_REGISTER_COUNT] = { 0 };
	DeferredCTX->CSSetUnorderedAccessViews(0, (std::uint32_t)std::size(pUAVs), pUAVs, pUAVInitialCounts);

	ID3D11Buffer* pCBs[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT] = { 0 };
	DeferredCTX->VSSetConstantBuffers(0, (std::uint32_t)std::size(pCBs), pCBs);
	DeferredCTX->GSSetConstantBuffers(0, (std::uint32_t)std::size(pCBs), pCBs);
	DeferredCTX->PSSetConstantBuffers(0, (std::uint32_t)std::size(pCBs), pCBs);
	DeferredCTX->CSSetConstantBuffers(0, (std::uint32_t)std::size(pCBs), pCBs);

	ID3D11SamplerState* pSs[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT] = { 0 };
	DeferredCTX->VSSetSamplers(NULL, (std::uint32_t)std::size(pSs), pSs);
	DeferredCTX->PSSetSamplers(NULL, (std::uint32_t)std::size(pSs), pSs);
	DeferredCTX->DSSetSamplers(NULL, (std::uint32_t)std::size(pSs), pSs);
	DeferredCTX->HSSetSamplers(NULL, (std::uint32_t)std::size(pSs), pSs);
	DeferredCTX->CSSetSamplers(NULL, (std::uint32_t)std::size(pSs), pSs);

	DeferredCTX->RSSetState(NULL);
	DeferredCTX->OMSetDepthStencilState(nullptr, 0);
	DeferredCTX->OMSetBlendState(nullptr, 0, 0xFFFFFFFF);

	StencilRef = 0;
}

D3D11CopyCommandBuffer::D3D11CopyCommandBuffer(D3D11Device* InDevice) 
	: CommandList(nullptr)
	, DeferredCTX(nullptr)
	, Owner(InDevice)
	, bIsSingleThreaded(true)
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

D3D11CopyCommandBuffer::~D3D11CopyCommandBuffer()
{
	DeferredCTX->ClearState();
	DeferredCTX->Flush();
	DeferredCTX = nullptr;

	CommandList = nullptr;
	Owner = nullptr;
}

void D3D11CopyCommandBuffer::BeginRecordCommandList()
{
	ClearState();
}

void D3D11CopyCommandBuffer::FinishRecordCommandList()
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

void D3D11CopyCommandBuffer::ExecuteCommandList()
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
}

void D3D11CopyCommandBuffer::ClearState()
{
	DeferredCTX->ClearState();
}

void D3D11CopyCommandBuffer::CopyFrameBuffer(IFrameBuffer* Dest, std::size_t DestFBOIndex, IFrameBuffer* Source, std::uint32_t SourceFBOIndex)
{
	if (!Dest || !Source)
		return;

	D3D11FrameBuffer* DestFB = static_cast<D3D11FrameBuffer*>(Dest);
	D3D11FrameBuffer* SourceFB = static_cast<D3D11FrameBuffer*>(Source);
	if (DestFBOIndex >= DestFB->RenderTargets.size() || SourceFBOIndex >= SourceFB->RenderTargets.size())
		return;
	DeferredCTX->CopyResource(DestFB->RenderTargets.at(DestFBOIndex)->GetD3D11Texture(), SourceFB->RenderTargets.at(SourceFBOIndex)->GetD3D11Texture());
}

void D3D11CopyCommandBuffer::CopyFrameBufferDepth(IFrameBuffer* Dest, IFrameBuffer* Source)
{
	if (!Dest || !Source)
		return;

	D3D11FrameBuffer* DestFB = static_cast<D3D11FrameBuffer*>(Dest);
	D3D11FrameBuffer* SourceFB = static_cast<D3D11FrameBuffer*>(Source);
	if (!DestFB->DepthTarget || SourceFB->DepthTarget)
		return;
	DeferredCTX->CopyResource(DestFB->DepthTarget->GetD3D11Texture(), SourceFB->DepthTarget->GetD3D11Texture());
}

void D3D11CopyCommandBuffer::UpdateBufferSubresource(IVertexBuffer* Buffer, sBufferSubresource* Subresource)
{
	if (!Subresource || !Buffer)
		return;

	UpdateBufferSubresource(Buffer, Subresource->Location, Subresource->Size, Subresource->pSysMem);
}

void D3D11CopyCommandBuffer::UpdateBufferSubresource(IVertexBuffer* Buffer, std::size_t Location, std::size_t Size, const void* pSrcData)
{
	if (!Buffer)
		return;

	sBufferSubresource Subresource;
	Subresource.Location = Location;
	Subresource.Size = Size;
	Subresource.pSysMem = const_cast<void*>(pSrcData);
	static_cast<D3D11VertexBuffer*>(Buffer)->UpdateSubresource(&Subresource, DeferredCTX.Get());
}

void D3D11CopyCommandBuffer::UpdateBufferSubresource(IIndexBuffer* Buffer, sBufferSubresource* Subresource)
{
	if (!Subresource || !Buffer)
		return;

	UpdateBufferSubresource(Buffer, Subresource->Location, Subresource->Size, Subresource->pSysMem);
}

void D3D11CopyCommandBuffer::UpdateBufferSubresource(IIndexBuffer* Buffer, std::size_t Location, std::size_t Size, const void* pSrcData)
{
	if (!Buffer)
		return;

	sBufferSubresource Subresource;
	Subresource.Location = Location;
	Subresource.Size = Size;
	Subresource.pSysMem = const_cast<void*>(pSrcData);
	static_cast<D3D11IndexBuffer*>(Buffer)->UpdateSubresource(&Subresource, DeferredCTX.Get());
}
