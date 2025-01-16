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
#include "D3D11Pipeline.h"
#include "D3D11Buffer.h"
#include "D3D11Viewport.h"
#include "D3D11CommandBuffer.h"
#include "D3D11Shader.h"
#include <stdexcept>

D3D11Pipeline::D3D11Pipeline(D3D11Device* InDevice, const std::string& InName, const sPipelineDesc& InDesc)
	: Owner(InDevice)
	, Name(InName)
	, Desc(InDesc)
	, PrimitiveTopologyType(D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
{
	auto PrimType = [&](EPrimitiveType Type) -> D3D_PRIMITIVE_TOPOLOGY
	{
		switch (Type)
		{
		case EPrimitiveType::ePOINT_LIST:
			return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
			break;
		case EPrimitiveType::eTRIANGLE_LIST:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			break;
		case EPrimitiveType::eTRIANGLE_STRIP:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
			break;
		case EPrimitiveType::eLINE_LIST:
			return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
			break;
		/*case EPrimitiveType::ePATCH_1_CONTROL_POINT:
			return D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST;
			break;
		case EPrimitiveType::ePATCH_3_CONTROL_POINT:
			return D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST;
			break;*/
		default:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			break;
		}
	};

	DescriptorSetLayout = InDesc.DescriptorSetLayout;
	ShaderAttachments = InDesc.ShaderAttachments;
	if (ShaderAttachments.size() == 0)
		throw std::runtime_error("No Shader attached.");

	for (const auto& Attachment : ShaderAttachments)
	{
		auto Shader = Cast<D3D11CompiledShader>(Owner->CompileShader(Attachment));

		if (InDesc.VertexLayout.size() > 0 && Attachment.Type == eShaderType::Vertex && !VertexAttribute)
		{
			VertexAttribute = D3D11VertexAttribute::CreateUnique(Owner, InDesc.VertexLayout, Shader->GetD3DByteCode());
			InputLayout = VertexAttribute->Get();
		}

		Shader->CreateD3D11Shader();
		Shaders.insert({ Attachment.Type, Shader->GetShader() });
	}

	PrimitiveTopologyType = PrimType(InDesc.PrimitiveTopologyType);

	for (const auto& Binding : DescriptorSetLayout)
	{
		if (Binding.GetDescriptorType() == EDescriptorType::eSampler)
		{
			auto SamplerDesc = Binding.GetSamplerDesc();
			D3D11SamplerState::SharedPtr SS = D3D11SamplerState::Create(Owner, SamplerDesc.has_value() ? *SamplerDesc : sSamplerAttributeDesc());
			SamplerStates.insert({ Binding.Location, SS->Get() });
		}
	}

	//if (InDesc.RasterizerAttribute.StateStatus == EStateStatus::eActive)
	{
		D3D11Rasterizer RS = D3D11Rasterizer(Owner, InDesc.RasterizerAttribute);
		RasterizerState = RS.Get();
	}

	//if (InDesc.DepthStencilAttribute.StateStatus == EStateStatus::EActive)
	{
		//InDesc.DepthStencilAttribute.DepthTest = ECompareFunction::CF_LessEqual;
		D3D11DepthStencilState DS = D3D11DepthStencilState(Owner, InDesc.DepthStencilAttribute);
		DepthStencilState = DS.Get();
	}
	D3D11BlendState BS = D3D11BlendState(Owner, InDesc.BlendAttribute);
	BlendState = BS.Get();
}

void D3D11Pipeline::ApplyPipeline(ID3D11DeviceContext1* Context, std::uint32_t StencilRef) const
{
	{
		ID3D11Buffer* pCBs[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT] = { 0 };

		if (Shaders.find(eShaderType::Vertex) != Shaders.end())
		{
			Context->VSSetShader(static_cast<ID3D11VertexShader*>(Shaders.at(eShaderType::Vertex).Get()), NULL, NULL);
		}
		else
		{
			Context->VSSetShader(NULL, NULL, 0);
		}

		if (Shaders.find(eShaderType::HULL) != Shaders.end())
		{
			Context->HSSetShader(static_cast<ID3D11HullShader*>(Shaders.at(eShaderType::HULL).Get()), NULL, NULL);
		}
		else
		{
			Context->HSSetShader(NULL, NULL, 0);
		}

		if (Shaders.find(eShaderType::Domain) != Shaders.end())
		{
			Context->DSSetShader(static_cast<ID3D11DomainShader*>(Shaders.at(eShaderType::Domain).Get()), NULL, NULL);
		}
		else
		{
			Context->DSSetShader(NULL, NULL, 0);
		}

		if (Shaders.find(eShaderType::Geometry) != Shaders.end())
		{
			Context->GSSetShader(static_cast<ID3D11GeometryShader*>(Shaders.at(eShaderType::Geometry).Get()), NULL, NULL);
		}
		else
		{
			Context->GSSetShader(NULL, NULL, 0);
		}

		if (Shaders.find(eShaderType::Pixel) != Shaders.end())
		{
			Context->PSSetShader(static_cast<ID3D11PixelShader*>(Shaders.at(eShaderType::Pixel).Get()), NULL, NULL);
		}
		else
		{
			Context->PSSetShader(NULL, NULL, 0);
		}

		if (Shaders.find(eShaderType::Compute) != Shaders.end())
		{
			Context->CSSetShader(static_cast<ID3D11ComputeShader*>(Shaders.at(eShaderType::Compute).Get()), NULL, NULL);
		}
		else
		{
			Context->CSSetShader(NULL, NULL, 0);
		}
	}

	if (InputLayout)
	{
		Context->IASetInputLayout(InputLayout.Get());
	}
	else
	{
		ID3D11Buffer* pVBs[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT] = { 0 };
		std::uint32_t countsAndOffsets[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT] = { 0 };
		Context->IASetInputLayout(nullptr);
		Context->IASetVertexBuffers(0, (std::uint32_t)std::size(pVBs), pVBs, countsAndOffsets, countsAndOffsets);
		Context->IASetIndexBuffer(nullptr, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
	}

	if (RasterizerState)
		Context->RSSetState(RasterizerState.Get());
	else
		Context->RSSetState(nullptr);

	if (DepthStencilState)
		Context->OMSetDepthStencilState(DepthStencilState.Get(), StencilRef);
	else
		Context->OMSetDepthStencilState(nullptr, 0);

	//D3D11_VIEWPORT pViewport(Owner->GetViewportContext()->GetViewport());
	//Context->RSSetViewports(1, &pViewport);
	
	//float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	Context->OMSetBlendState(BlendState.Get(), 0, 0xFFFFFFFF);

	for (auto& Sampler : SamplerStates)
		Context->PSSetSamplers(Sampler.first, 1, Sampler.second.GetAddressOf());

	Context->IASetPrimitiveTopology(PrimitiveTopologyType);

	Context = nullptr;
}

void D3D11Pipeline::SetStencilRef(ID3D11DeviceContext1* Context, std::uint32_t Ref)
{
	if (DepthStencilState)
		Context->OMSetDepthStencilState(DepthStencilState.Get(), Ref);
	else
		Context->OMSetDepthStencilState(nullptr, 0);
}

std::optional<sDescriptorSetLayoutBinding> D3D11Pipeline::GetDescriptorSetConstantBufferLayoutBinding(std::size_t index) const
{
	std::size_t i = 0;
	for (auto& DescriptorSet : DescriptorSetLayout)
	{
		if (DescriptorSet.GetDescriptorType() == EDescriptorType::eUniformBuffer)
		{
			if (index == i)
				return DescriptorSet;
			i++;
		}
	}

	return std::nullopt;
}

std::optional<sDescriptorSetLayoutBinding> D3D11Pipeline::GetDescriptorSetTextureLayoutBinding(std::size_t index) const
{
	std::size_t i = 0;
	for (auto& DescriptorSet : DescriptorSetLayout)
	{
		if (DescriptorSet.GetDescriptorType() == EDescriptorType::eTexture)
		{
			if (index == i)
				return DescriptorSet;
			i++;
		}
	}

	return std::nullopt;
}

bool D3D11Pipeline::Recompile()
{
	for (auto& Shader : Shaders)
	{
		Shader.second = nullptr;
	}
	Shaders.clear();

	for (auto& Attachment : ShaderAttachments)
	{
		auto Shader = Cast<D3D11CompiledShader>(Owner->CompileShader(Attachment));

		/*if (InVertexLayout.size() > 0 && Attachment.Type == EShaderType::Vertex && !VertexAttribute)
		{
			VertexAttribute = D3D11VertexAttribute::Create(Owner, InVertexLayout, Shader->GetByteCode().Get());
			InputLayout = VertexAttribute->Get();
		}*/

		Shader->CreateD3D11Shader();
		Shaders.insert({ Attachment.Type, Shader->GetShader() });
	}

	return true;
}

D3D11ComputePipeline::D3D11ComputePipeline(D3D11Device* InDevice, const std::string& InName, const sComputePipelineDesc& InDesc)
	: Owner(InDevice)
	, Name(InName)
	, Desc(InDesc)
	, ShaderAttachment(InDesc.ShaderAttachment)
{
	for (const auto& Binding : Desc.DescriptorSetLayout)
	{
		if (Binding.GetDescriptorType() == EDescriptorType::eSampler)
		{
			auto SamplerDesc = Binding.GetSamplerDesc();
			D3D11SamplerState::SharedPtr SS = D3D11SamplerState::Create(Owner, SamplerDesc.has_value() ? *SamplerDesc : sSamplerAttributeDesc());
			SamplerStates.insert({ Binding.Location, SS->Get() });
		}
	}

	if (!ShaderAttachment.IsCodeValid())
	{
		auto pShader = Cast<D3D11CompiledShader>(Owner->CompileShader(ShaderAttachment.GetLocation(), ShaderAttachment.FunctionName, eShaderType::Compute, false, ShaderAttachment.ShaderDefines));
		if (!pShader->IsCompiled())
			pShader->CreateD3D11Shader();
		Shader = pShader->GetShader();
	}
	else
	{
		auto pShader = Cast<D3D11CompiledShader>(Owner->CompileShader(ShaderAttachment.GetByteCode(), ShaderAttachment.GetByteCodeSize(), ShaderAttachment.FunctionName, eShaderType::Compute, false, ShaderAttachment.ShaderDefines));
		if (!pShader->IsCompiled())
			pShader->CreateD3D11Shader();
		Shader = pShader->GetShader();
	}
}

void D3D11ComputePipeline::ApplyPipeline(ID3D11DeviceContext1* Context) const
{
	Context->CSSetShader(static_cast<ID3D11ComputeShader*>(Shader.Get()), NULL, NULL);

	for (auto& Sampler : SamplerStates)
		Context->CSSetSamplers(Sampler.first, 1, Sampler.second.GetAddressOf());
}

bool D3D11ComputePipeline::Recompile()
{
	for (const auto& Binding : Desc.DescriptorSetLayout)
	{
		if (Binding.GetDescriptorType() == EDescriptorType::eSampler)
		{
			auto SamplerDesc = Binding.GetSamplerDesc();
			D3D11SamplerState::SharedPtr SS = D3D11SamplerState::Create(Owner, SamplerDesc.has_value() ? *SamplerDesc : sSamplerAttributeDesc());
			SamplerStates.insert({ Binding.Location, SS->Get() });
		}
	}

	if (!ShaderAttachment.IsCodeValid())
	{
		auto pShader = Cast<D3D11CompiledShader>(Owner->CompileShader(ShaderAttachment.GetLocation(), ShaderAttachment.FunctionName, eShaderType::Compute, false, ShaderAttachment.ShaderDefines));
		if (!pShader->IsCompiled())
			pShader->CreateD3D11Shader();
		Shader = pShader->GetShader();
	}
	else
	{
		auto pShader = Cast<D3D11CompiledShader>(Owner->CompileShader(ShaderAttachment.GetByteCode(), ShaderAttachment.GetByteCodeSize(), ShaderAttachment.FunctionName, eShaderType::Compute, false, ShaderAttachment.ShaderDefines));
		if (!pShader->IsCompiled())
			pShader->CreateD3D11Shader();
		Shader = pShader->GetShader();
	}

	return true;
}
