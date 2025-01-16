/* ---------------------------------------------------------------------------------------
* MIT License
*
* Copyright (c) 2023 Davut Coşkun.
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

#include "D3D12Device.h"
#include <mutex>
#include <vector>
#include "Engine/ClassBody.h"
#include "D3D12Shader.h"
#include "Engine/AbstractEngine.h"
#include "GI/D3DShared/D3DShared.h"
#include "dx12.h"
#include <ranges>

class D3D12Rasterizer
{
	sBaseClassBody(sClassConstructor, D3D12Rasterizer)
public:
	D3D12Rasterizer(sRasterizerAttributeDesc InDesc, bool EnableConservativeRaster = false)
		: RasterizerDesc(InDesc)
	{
		RasterizerStateDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

		switch (InDesc.FillMode)
		{
		case ERasterizerFillMode::eSolid:
			RasterizerStateDesc.FillMode = D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
			break;
		case  ERasterizerFillMode::eWireframe:
			RasterizerStateDesc.FillMode = D3D12_FILL_MODE::D3D12_FILL_MODE_WIREFRAME;
			break;
		}

		switch (InDesc.CullMode)
		{
		case ERasterizerCullMode::eCCW:
			RasterizerStateDesc.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_BACK;
			break;
		case ERasterizerCullMode::eCW:
			RasterizerStateDesc.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_FRONT;
			break;
		case ERasterizerCullMode::eNone:
			RasterizerStateDesc.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
			break;
		}

		RasterizerStateDesc.FrontCounterClockwise = InDesc.FrontCounterClockwise ? TRUE : FALSE;
		RasterizerStateDesc.DepthBias = static_cast<INT>(InDesc.DepthBias);
		RasterizerStateDesc.DepthBiasClamp = InDesc.DepthBiasClamp;
		RasterizerStateDesc.SlopeScaledDepthBias = InDesc.SlopeScaleDepthBias;
		RasterizerStateDesc.DepthClipEnable = InDesc.DepthClipEnable ? TRUE : FALSE;
		RasterizerStateDesc.MultisampleEnable = InDesc.bAllowMSAA ? TRUE : FALSE;
		RasterizerStateDesc.AntialiasedLineEnable = InDesc.bEnableLineAA ? TRUE : FALSE;
		RasterizerStateDesc.ForcedSampleCount = 0;

		if (EnableConservativeRaster)
			RasterizerStateDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON;
		else
			RasterizerStateDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	}

	virtual	~D3D12Rasterizer() = default;

	FORCEINLINE D3D12_RASTERIZER_DESC Get() const { return RasterizerStateDesc; }

private:
	sRasterizerAttributeDesc RasterizerDesc;
	D3D12_RASTERIZER_DESC RasterizerStateDesc;
};

class D3D12StaticSamplerState
{
	sBaseClassBody(sClassConstructor, D3D12StaticSamplerState)
public:
	D3D12StaticSamplerState(sSamplerAttributeDesc InDesc, UINT ShaderRegister)
		: SamplerDesc(InDesc)
	{
		const bool bComparisonEnabled = false;// InDesc.SamplerComparisonFunction != ECompareFunction::eNever;

		switch (SamplerDesc.Filter)
		{
		case ESamplerFilter::eAnisotropicLinear:
		case ESamplerFilter::eAnisotropicPoint:
			if (SamplerDesc.MaxAnisotropy == 1)
			{
				D3D12SamplerDesc.Filter = bComparisonEnabled ? D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR : D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			}
			else
			{
				// D3D12  doesn't allow using point filtering for mip filter when using anisotropic filtering
				D3D12SamplerDesc.Filter = bComparisonEnabled ? D3D12_FILTER_COMPARISON_ANISOTROPIC : D3D12_FILTER_ANISOTROPIC;
			}

			break;
		case ESamplerFilter::eTrilinear:
			D3D12SamplerDesc.Filter = bComparisonEnabled ? D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR : D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			break;
		case ESamplerFilter::eBilinear:
			D3D12SamplerDesc.Filter = bComparisonEnabled ? D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT : D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			break;
		case ESamplerFilter::ePoint:
			D3D12SamplerDesc.Filter = bComparisonEnabled ? D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT : D3D12_FILTER_MIN_MAG_MIP_POINT;
			break;
		}

		auto WrapMpde = [&](ESamplerAddressMode Mode) -> D3D12_TEXTURE_ADDRESS_MODE
			{
				switch (Mode)
				{
				case ESamplerAddressMode::eBorder: return D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_BORDER;
				case ESamplerAddressMode::eClamp: return D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				case ESamplerAddressMode::eMirror: return D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
				case ESamplerAddressMode::eMirrorOnce: return D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
				case ESamplerAddressMode::eWrap: return D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				};
				return D3D12_TEXTURE_ADDRESS_MODE();
			};

		auto Compare = [&](ECompareFunction Mode) -> D3D12_COMPARISON_FUNC
			{
				switch (Mode)
				{
				case ECompareFunction::eLess: return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS;
				case ECompareFunction::eLessEqual: return D3D12_COMPARISON_FUNC_LESS_EQUAL;
				case ECompareFunction::eGreater: return D3D12_COMPARISON_FUNC_GREATER;
				case ECompareFunction::eGreaterEqual: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
				case ECompareFunction::eEqual: return D3D12_COMPARISON_FUNC_EQUAL;
				case ECompareFunction::eNotEqual: return D3D12_COMPARISON_FUNC_NOT_EQUAL;
				case ECompareFunction::eNever: return D3D12_COMPARISON_FUNC_NEVER;
				case ECompareFunction::eAlways: return D3D12_COMPARISON_FUNC_ALWAYS;
				}
				return D3D12_COMPARISON_FUNC();
			};

		D3D12SamplerDesc.AddressU = WrapMpde(SamplerDesc.AddressU);
		D3D12SamplerDesc.AddressV = WrapMpde(SamplerDesc.AddressV);
		D3D12SamplerDesc.AddressW = WrapMpde(SamplerDesc.AddressW);
		D3D12SamplerDesc.MipLODBias = (float)SamplerDesc.MipBias;
		D3D12SamplerDesc.MaxAnisotropy = SamplerDesc.MaxAnisotropy;
		D3D12SamplerDesc.ComparisonFunc = Compare(SamplerDesc.SamplerComparisonFunction);
		if (InDesc.BorderColor.A == 0)
			D3D12SamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		else if (InDesc.BorderColor == FColor::Black())
			D3D12SamplerDesc.BorderColor = SamplerDesc.bUINTBorderColor ? D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK_UINT : D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
		else /*if (InDesc.BorderColor == FColor::White())*/
			D3D12SamplerDesc.BorderColor = SamplerDesc.bUINTBorderColor ? D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE_UINT : D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		D3D12SamplerDesc.MinLOD = SamplerDesc.MinMipLevel;
		D3D12SamplerDesc.MaxLOD = SamplerDesc.MaxMipLevel;
		D3D12SamplerDesc.ShaderRegister = ShaderRegister;
	}

	virtual ~D3D12StaticSamplerState() = default;

	FORCEINLINE D3D12_STATIC_SAMPLER_DESC Get() const { return D3D12SamplerDesc; }

private:
	sSamplerAttributeDesc SamplerDesc;
	D3D12_STATIC_SAMPLER_DESC D3D12SamplerDesc;
};

class D3D12DepthStencilState
{
	sBaseClassBody(sClassConstructor, D3D12DepthStencilState)
public:
	D3D12DepthStencilState(sDepthStencilAttributeDesc InDesc)
		: DepthStencilStateDesc(InDesc)
	{
		auto Compare = [&](ECompareFunction Mode) -> D3D12_COMPARISON_FUNC
			{
				switch (Mode)
				{
				case ECompareFunction::eLess: return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS;
				case ECompareFunction::eLessEqual: return D3D12_COMPARISON_FUNC_LESS_EQUAL;
				case ECompareFunction::eGreater: return D3D12_COMPARISON_FUNC_GREATER;
				case ECompareFunction::eGreaterEqual: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
				case ECompareFunction::eEqual: return D3D12_COMPARISON_FUNC_EQUAL;
				case ECompareFunction::eNotEqual: return D3D12_COMPARISON_FUNC_NOT_EQUAL;
				case ECompareFunction::eNever: return D3D12_COMPARISON_FUNC_NEVER;
				case ECompareFunction::eAlways:	return D3D12_COMPARISON_FUNC_ALWAYS;
				}
				return D3D12_COMPARISON_FUNC();
			};

		auto Stencil = [&](EStencilOp Mode) -> D3D12_STENCIL_OP
			{
				switch (Mode)
				{
				case EStencilOp::eKeep: return D3D12_STENCIL_OP_KEEP;
				case EStencilOp::eZero: return D3D12_STENCIL_OP_ZERO;
				case EStencilOp::eReplace: return D3D12_STENCIL_OP_REPLACE;
				case EStencilOp::eSaturatedIncrement: return D3D12_STENCIL_OP_INCR_SAT;
				case EStencilOp::eSaturatedDecrement:	return D3D12_STENCIL_OP_DECR_SAT;
				case EStencilOp::eInvert:	return D3D12_STENCIL_OP_INVERT;
				case EStencilOp::eIncrement: return D3D12_STENCIL_OP_INCR;
				case EStencilOp::eDecrement: return D3D12_STENCIL_OP_DECR;
				}
				return D3D12_STENCIL_OP();
			};

		DepthStencilDesc = CD3DX12_DEPTH_STENCIL_DESC1(D3D12_DEFAULT);

		DepthStencilDesc.DepthFunc = Compare(InDesc.DepthTest);
		DepthStencilDesc.DepthEnable = /*InDesc.DepthTest != ECompareFunction::eAlways ||*/ InDesc.bEnableDepthWrite;
		DepthStencilDesc.StencilEnable = InDesc.bStencilEnable;

		DepthStencilDesc.DepthWriteMask = InDesc.bDepthWriteMask ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;

		DepthStencilDesc.StencilEnable = InDesc.bEnableFrontFaceStencil || InDesc.bEnableBackFaceStencil;
		DepthStencilDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK; //Initializer.StencilReadMask;
		DepthStencilDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK; //Initializer.StencilWriteMask;

		DepthStencilDesc.FrontFace.StencilFunc = Compare(InDesc.FrontFaceStencilTest);
		DepthStencilDesc.FrontFace.StencilDepthFailOp = Stencil(InDesc.FrontFaceDepthFailStencilOp);
		DepthStencilDesc.FrontFace.StencilFailOp = Stencil(InDesc.FrontFaceStencilFailStencilOp);
		DepthStencilDesc.FrontFace.StencilPassOp = Stencil(InDesc.FrontFacePassStencilOp);

		DepthStencilDesc.BackFace.StencilFunc = Compare(InDesc.BackFaceStencilTest);
		DepthStencilDesc.BackFace.StencilDepthFailOp = Stencil(InDesc.BackFaceDepthFailStencilOp);
		DepthStencilDesc.BackFace.StencilFailOp = Stencil(InDesc.BackFaceStencilFailStencilOp);
		DepthStencilDesc.BackFace.StencilPassOp = Stencil(InDesc.BackFacePassStencilOp);
	}

	virtual ~D3D12DepthStencilState() = default;

	FORCEINLINE D3D12_DEPTH_STENCIL_DESC Get() const { return DepthStencilDesc; }

private:
	sDepthStencilAttributeDesc DepthStencilStateDesc;
	D3D12_DEPTH_STENCIL_DESC DepthStencilDesc;
};

class D3D12BlendState
{
	sBaseClassBody(sClassConstructor, D3D12BlendState)
public:
	D3D12BlendState(sBlendAttributeDesc InDesc)
		: BlendStateDesc(InDesc)
	{
		auto BlendOP = [&](EBlendOperation var) -> D3D12_BLEND_OP
			{
				switch (var)
				{
				case EBlendOperation::eAdd: return D3D12_BLEND_OP_ADD;
				case EBlendOperation::eSubtract: return D3D12_BLEND_OP_SUBTRACT;
				case EBlendOperation::eMin: return D3D12_BLEND_OP_MIN;
				case EBlendOperation::eMax: return D3D12_BLEND_OP_MAX;
				case EBlendOperation::eReverseSubtract: return D3D12_BLEND_OP_REV_SUBTRACT;
				}
				return D3D12_BLEND_OP();
			};

		auto BlendFactor = [&](EBlendFactor var) -> D3D12_BLEND
			{
				switch (var)
				{
				case EBlendFactor::eZero: return D3D12_BLEND_ZERO;
				case EBlendFactor::eOne: return D3D12_BLEND_ONE;
				case EBlendFactor::eSourceColor: return D3D12_BLEND_SRC_COLOR;
				case EBlendFactor::eInverseSourceColor: return D3D12_BLEND_INV_SRC_COLOR;
				case EBlendFactor::eSourceAlpha: return D3D12_BLEND_SRC_ALPHA;
				case EBlendFactor::eInverseSourceAlpha:	return D3D12_BLEND_INV_SRC_ALPHA;
				case EBlendFactor::eDestAlpha: return D3D12_BLEND_DEST_ALPHA;
				case EBlendFactor::eInverseDestAlpha: return D3D12_BLEND_INV_DEST_ALPHA;
				case EBlendFactor::eDestColor: return D3D12_BLEND_DEST_COLOR;
				case EBlendFactor::eInverseDestColor: return D3D12_BLEND_INV_DEST_COLOR;
				case EBlendFactor::eBlendFactor: return D3D12_BLEND_BLEND_FACTOR;
				case EBlendFactor::eInverseBlendFactor: return D3D12_BLEND_INV_BLEND_FACTOR;
				}
				return D3D12_BLEND();
			};

		auto ColorWriteMask = [&](EColorWriteMask var) -> UINT8
			{
				switch (var)
				{
				case EColorWriteMask::eNONE: return 0;
				case EColorWriteMask::eRED: return D3D12_COLOR_WRITE_ENABLE_RED;
				case EColorWriteMask::eGREEN: return D3D12_COLOR_WRITE_ENABLE_GREEN;
				case EColorWriteMask::eBLUE:	return D3D12_COLOR_WRITE_ENABLE_BLUE;
				case EColorWriteMask::eALPHA: return D3D12_COLOR_WRITE_ENABLE_ALPHA;
				case EColorWriteMask::eRGB: return D3D12_COLOR_WRITE_ENABLE_RED | D3D12_COLOR_WRITE_ENABLE_GREEN | D3D12_COLOR_WRITE_ENABLE_BLUE;
				case EColorWriteMask::eRGBA:	return D3D12_COLOR_WRITE_ENABLE_RED | D3D12_COLOR_WRITE_ENABLE_GREEN | D3D12_COLOR_WRITE_ENABLE_BLUE | D3D12_COLOR_WRITE_ENABLE_ALPHA;
				case EColorWriteMask::eRG: return D3D12_COLOR_WRITE_ENABLE_RED | D3D12_COLOR_WRITE_ENABLE_GREEN;
				case EColorWriteMask::eBA: return D3D12_COLOR_WRITE_ENABLE_BLUE | D3D12_COLOR_WRITE_ENABLE_ALPHA;
				}
				return 0;
			};

		BlendDesc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		// Additive blending
		BlendDesc.AlphaToCoverageEnable = BlendStateDesc.balphaToCoverage ? TRUE : FALSE;
		BlendDesc.IndependentBlendEnable = BlendStateDesc.bUseIndependentRenderTargetBlendStates ? TRUE : FALSE;
		//desc.IndependentBlendEnable = false;// TRUE;

		std::uint32_t i = 0;
		for (const auto& RT : BlendStateDesc.RenderTargets)
		{
			if (!RT.bBlendEnable)
				continue;

			BlendDesc.RenderTarget[i].LogicOpEnable = false;
			BlendDesc.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_NOOP;

			BlendDesc.RenderTarget[i].BlendEnable = RT.bBlendEnable;
			BlendDesc.RenderTarget[i].SrcBlend = BlendFactor(RT.ColorSrcBlend);
			BlendDesc.RenderTarget[i].DestBlend = BlendFactor(RT.ColorDestBlend);
			BlendDesc.RenderTarget[i].BlendOp = BlendOP(RT.ColorBlendOp);

			BlendDesc.RenderTarget[i].SrcBlendAlpha = BlendFactor(RT.AlphaSrcBlend);
			BlendDesc.RenderTarget[i].DestBlendAlpha = BlendFactor(RT.AlphaDestBlend);
			BlendDesc.RenderTarget[i].BlendOpAlpha = BlendOP(RT.AlphaBlendOp);
			BlendDesc.RenderTarget[i].RenderTargetWriteMask = ColorWriteMask(RT.ColorWriteMask);
			i++;
		}
	}

	virtual ~D3D12BlendState() = default;

	D3D12_BLEND_DESC Get() const { return BlendDesc; };

private:
	sBlendAttributeDesc BlendStateDesc;
	D3D12_BLEND_DESC BlendDesc;
};

class D3D12VertexAttribute
{
public:
	D3D12VertexAttribute(std::vector<sVertexAttributeDesc> InDesc)
		: VertexAttributeDescData(InDesc)
	{
		InputLayout.resize(VertexAttributeDescData.size());
		for (std::uint32_t i = 0; i < VertexAttributeDescData.size(); i++)
		{
			InputLayout[i].SemanticName = VertexAttributeDescData[i].name.c_str();
			InputLayout[i].SemanticIndex = 0;
			InputLayout[i].Format = ConvertFormat_Format_To_DXGI(VertexAttributeDescData[i].format);
			InputLayout[i].InputSlot = VertexAttributeDescData[i].InputSlot;
			InputLayout[i].AlignedByteOffset = VertexAttributeDescData[i].offset;
			InputLayout[i].InputSlotClass = VertexAttributeDescData[i].isInstanced ?
				D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA : D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			InputLayout[i].InstanceDataStepRate = VertexAttributeDescData[i].isInstanced ? 1 : 0;
		}
	}

	D3D12VertexAttribute(ID3D12ShaderReflection* ShaderReflection)
		//: VertexAttributeDescData(InDesc)
	{
		D3D12_SHADER_DESC shaderDesc{};
		ShaderReflection->GetDesc(&shaderDesc);

		for (const uint32_t parameterIndex : std::views::iota(0u, shaderDesc.InputParameters))
		{
			D3D12_SIGNATURE_PARAMETER_DESC signatureParameterDesc{};
			ShaderReflection->GetInputParameterDesc(parameterIndex, &signatureParameterDesc);

			sVertexAttributeDesc VertexAttributeDesc;
			VertexAttributeDesc.name = signatureParameterDesc.SemanticName;
			VertexAttributeDesc.InputSlot = signatureParameterDesc.SemanticIndex;
			VertexAttributeDesc.offset = D3D12_APPEND_ALIGNED_ELEMENT;
			VertexAttributeDesc.isInstanced = false;

			if (signatureParameterDesc.Mask == 1)
			{
				if (signatureParameterDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
					VertexAttributeDesc.format = EFormat::R32_UINT; // DXGI_FORMAT_R32_UINT;
				else if (signatureParameterDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
					VertexAttributeDesc.format = EFormat::R32_SINT; //DXGI_FORMAT_R32_SINT;
				else if (signatureParameterDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
					VertexAttributeDesc.format = EFormat::R32_FLOAT; //DXGI_FORMAT_R32_FLOAT;
			}
			else if (signatureParameterDesc.Mask <= 3)
			{
				if (signatureParameterDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
					VertexAttributeDesc.format = EFormat::RG32_UINT; //DXGI_FORMAT_R32G32_UINT;
				else if (signatureParameterDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
					VertexAttributeDesc.format = EFormat::RG32_SINT; //DXGI_FORMAT_R32G32_SINT;
				else if (signatureParameterDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
					VertexAttributeDesc.format = EFormat::RG32_FLOAT; //DXGI_FORMAT_R32G32_FLOAT;
			}
			else if (signatureParameterDesc.Mask <= 7)
			{
				if (signatureParameterDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
					VertexAttributeDesc.format = EFormat::RGB32_UINT; //DXGI_FORMAT_R32G32B32_UINT;
				else if (signatureParameterDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
					VertexAttributeDesc.format = EFormat::RGB32_SINT; //DXGI_FORMAT_R32G32B32_SINT;
				else if (signatureParameterDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
					VertexAttributeDesc.format = EFormat::RGB32_FLOAT; //DXGI_FORMAT_R32G32B32_FLOAT;
			}
			else if (signatureParameterDesc.Mask <= 15)
			{
				if (signatureParameterDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
					VertexAttributeDesc.format = EFormat::RGBA32_UINT; //DXGI_FORMAT_R32G32B32A32_UINT;
				else if (signatureParameterDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
					VertexAttributeDesc.format = EFormat::RGBA32_SINT; //DXGI_FORMAT_R32G32B32A32_SINT;
				else if (signatureParameterDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
					VertexAttributeDesc.format = EFormat::RGBA32_UINT; //DXGI_FORMAT_R32G32B32A32_FLOAT;
			}

			VertexAttributeDescData.push_back(VertexAttributeDesc);
		}

		InputLayout.resize(VertexAttributeDescData.size());
		for (std::uint32_t i = 0; i < VertexAttributeDescData.size(); i++)
		{
			InputLayout[i].SemanticName = VertexAttributeDescData[i].name.c_str();
			InputLayout[i].SemanticIndex = 0;
			InputLayout[i].Format = ConvertFormat_Format_To_DXGI(VertexAttributeDescData[i].format);
			InputLayout[i].InputSlot = VertexAttributeDescData[i].InputSlot;
			InputLayout[i].AlignedByteOffset = VertexAttributeDescData[i].offset;
			InputLayout[i].InputSlotClass = VertexAttributeDescData[i].isInstanced ?
				D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA : D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			InputLayout[i].InstanceDataStepRate = VertexAttributeDescData[i].isInstanced ? 1 : 0;
		}
	}

	virtual ~D3D12VertexAttribute()
	{
		VertexAttributeDescData.clear();
		InputLayout.clear();
	}

	FORCEINLINE std::vector<D3D12_INPUT_ELEMENT_DESC> Get() const { return InputLayout; }
	FORCEINLINE std::size_t GetSize() const { return InputLayout.size(); }
	FORCEINLINE std::vector<sVertexAttributeDesc> GetVertexAttributeDesc() const { return VertexAttributeDescData; }

private:
	std::vector<sVertexAttributeDesc> VertexAttributeDescData;
	std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;
};

class D3D12RootSignature
{
public:
	D3D12RootSignature(ID3D12Device* Device, std::vector<sDescriptorSetLayoutBinding> Bindings)
	{
		auto GetVisibility = [](const eShaderType ShaderType) -> D3D12_SHADER_VISIBILITY
			{
				if (ShaderType == eShaderType::Vertex)
					return D3D12_SHADER_VISIBILITY_VERTEX;
				else if (ShaderType == eShaderType::Pixel)
					return D3D12_SHADER_VISIBILITY_PIXEL;
				else if (ShaderType == eShaderType::Geometry)
					return D3D12_SHADER_VISIBILITY_GEOMETRY;
				else if (ShaderType == eShaderType::HULL)
					return D3D12_SHADER_VISIBILITY_HULL;
				else if (ShaderType == eShaderType::Domain)
					return D3D12_SHADER_VISIBILITY_DOMAIN;
				else if (ShaderType == eShaderType::Mesh)
					return D3D12_SHADER_VISIBILITY_MESH;
				else if (ShaderType == eShaderType::Amplification)
					return D3D12_SHADER_VISIBILITY_AMPLIFICATION;

				return D3D12_SHADER_VISIBILITY_ALL;
			};

		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

		// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_2;

		if (FAILED(Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
			if (FAILED(Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
			{
				featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
			}
		}

		std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters;
		std::vector<CD3DX12_DESCRIPTOR_RANGE1> ranges;
		ranges.reserve(Bindings.size());

		auto BindlessRangeFlags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE | D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE;
		if (GPU::IsBindlessRendererEnabled())
		{
			// Work in progress...

			std::size_t iv = 0;
			std::size_t ip = 0;
			for (const auto& Binding : Bindings)
			{
				if (Binding.GetDescriptorType() == EDescriptorType::eUniformBuffer)
				{
					D3D12_ROOT_DESCRIPTOR_FLAGS FLAGS = D3D12_ROOT_DESCRIPTOR_FLAGS::D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE;
					rootParameters.push_back(CD3DX12_ROOT_PARAMETER1());

					if (Binding.ShaderType == eShaderType::Vertex)
					{
						rootParameters.back().InitAsConstantBufferView(iv/*Binding.Location*/, 0, FLAGS, GetVisibility(Binding.ShaderType));
						iv++;
					}
					else if (Binding.ShaderType == eShaderType::Pixel)
					{
						rootParameters.back().InitAsConstantBufferView(ip/*Binding.Location*/, 0, FLAGS, GetVisibility(Binding.ShaderType));
						ip++;
					}
				}
				else if (Binding.GetDescriptorType() == EDescriptorType::eSampler)
				{
					continue;
				}
				else if (Binding.GetDescriptorType() == EDescriptorType::eTexture)
				{
				}
				else if (Binding.GetDescriptorType() == EDescriptorType::eUAV)
				{
					ranges.push_back(CD3DX12_DESCRIPTOR_RANGE1());
					ranges.back().Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, Binding.Size, Binding.Location, 0/*Binding.RegisterSpace*/, BindlessRangeFlags/*, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC*/);
					rootParameters.push_back(CD3DX12_ROOT_PARAMETER1());
					rootParameters.back().InitAsDescriptorTable(Binding.Size, &ranges.back()/*, GetVisibility(Binding.ShaderType)*/);
				}
				else if (Binding.GetDescriptorType() == EDescriptorType::e32BitConstant)
				{
					rootParameters.push_back(CD3DX12_ROOT_PARAMETER1());
					rootParameters.back().InitAsConstants(Binding.Size, Binding.Location, 0/*Binding.RegisterSpace*//*, GetVisibility(Binding.ShaderType)*/);
				}
			}

			ranges.push_back(CD3DX12_DESCRIPTOR_RANGE1());
			ranges.back().Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, UINT_MAX, 0, 0, BindlessRangeFlags);
			rootParameters.push_back(CD3DX12_ROOT_PARAMETER1());
			rootParameters.back().InitAsDescriptorTable(1, &ranges.back(), GetVisibility(eShaderType::Pixel));
		}
		else
		{
			for (const auto& Binding : Bindings)
			{
				if (Binding.GetDescriptorType() == EDescriptorType::eUniformBuffer)
				{
					//ranges.push_back(CD3DX12_DESCRIPTOR_RANGE1());
					//ranges.back().Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, Binding.Size, Binding.Location, Binding.RegisterSpace, BindlessRangeFlags/*, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC*/);
					//rootParameters.push_back(CD3DX12_ROOT_PARAMETER1());
					//rootParameters.back().InitAsDescriptorTable(Binding.Size, &ranges.back(), GetVisibility(Binding.ShaderType));

					D3D12_ROOT_DESCRIPTOR_FLAGS FLAGS = D3D12_ROOT_DESCRIPTOR_FLAGS::D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE;

					rootParameters.push_back(CD3DX12_ROOT_PARAMETER1());
					rootParameters.back().InitAsConstantBufferView(Binding.Location, Binding.RegisterSpace, FLAGS/*D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC*/, GetVisibility(Binding.ShaderType));
				}
				else if (Binding.GetDescriptorType() == EDescriptorType::eSampler)
				{
					continue;
				}
				else if (Binding.GetDescriptorType() == EDescriptorType::eTexture)
				{
					ranges.push_back(CD3DX12_DESCRIPTOR_RANGE1());
					ranges.back().Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, Binding.Size, Binding.Location, Binding.RegisterSpace, BindlessRangeFlags/*, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC*/);
					rootParameters.push_back(CD3DX12_ROOT_PARAMETER1());
					rootParameters.back().InitAsDescriptorTable(Binding.Size, &ranges.back(), GetVisibility(Binding.ShaderType));
				}
				else if (Binding.GetDescriptorType() == EDescriptorType::eUAV)
				{
					ranges.push_back(CD3DX12_DESCRIPTOR_RANGE1());
					ranges.back().Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, Binding.Size, Binding.Location, Binding.RegisterSpace, BindlessRangeFlags/*, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC*/);
					rootParameters.push_back(CD3DX12_ROOT_PARAMETER1());
					rootParameters.back().InitAsDescriptorTable(Binding.Size, &ranges.back(), GetVisibility(Binding.ShaderType));
				}
				else if (Binding.GetDescriptorType() == EDescriptorType::e32BitConstant)
				{
					rootParameters.push_back(CD3DX12_ROOT_PARAMETER1());
					rootParameters.back().InitAsConstants(Binding.Size, Binding.Location, Binding.RegisterSpace, GetVisibility(Binding.ShaderType));
				}
			}
		}

		// Allow input layout and deny uneccessary access to certain pipeline stages.
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED
			| D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
			/*| D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS*/
			/*| D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS*/
			/*| D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS*/;

		std::vector<D3D12_STATIC_SAMPLER_DESC> StaticSamplers;

		for (std::size_t i = 0; i < Bindings.size(); i++)
		{
			auto& Binding = Bindings[i];

			if (Binding.GetDescriptorType() == EDescriptorType::eSampler)
			{
				auto Sampler = Binding.GetSamplerDesc();
				if (Sampler.has_value())
				{
					D3D12StaticSamplerState SamplerState(*Sampler, Binding.Location);
					D3D12_STATIC_SAMPLER_DESC sampler = SamplerState.Get();
					sampler.RegisterSpace = Binding.RegisterSpace;
					sampler.ShaderVisibility = GetVisibility(Binding.ShaderType);

					StaticSamplers.push_back(sampler);
				}
				else
				{
					CD3DX12_STATIC_SAMPLER_DESC sampler = CD3DX12_STATIC_SAMPLER_DESC(Binding.Location);
					sampler.RegisterSpace = Binding.RegisterSpace;
					sampler.ShaderVisibility = GetVisibility(Binding.ShaderType);

					StaticSamplers.push_back(sampler);
				}
			}
		}

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		if (StaticSamplers.size() > 0)
			rootSignatureDesc.Init_1_1((UINT)rootParameters.size(), &rootParameters[0], (UINT)StaticSamplers.size(), &StaticSamplers[0], rootSignatureFlags);
		else
			rootSignatureDesc.Init_1_1((UINT)rootParameters.size(), &rootParameters[0], 0, nullptr, rootSignatureFlags);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		std::string Error;
		HRESULT Serialize_HR = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error);
		if (error)
		{
			Error = (static_cast<char*>(error->GetBufferPointer()));
			Engine::WriteToConsole(Error);
		}
		HRESULT RootSignature_HR = Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&RootSignature));
		ThrowIfFailed(RootSignature_HR);

#if _DEBUG
		RootSignature->SetName(L"D3D12RootSignature");
#endif
	}

	D3D12RootSignature(ID3D12Device* Device, ID3D12ShaderReflection* ShaderReflection)
	{
		std::vector<D3D12_ROOT_PARAMETER1> rootParameters;
		std::vector<CD3DX12_DESCRIPTOR_RANGE1> descriptorRanges;

		D3D12_SHADER_DESC shaderDesc{};
		ShaderReflection->GetDesc(&shaderDesc);

		for (const uint32_t i : std::views::iota(0u, shaderDesc.BoundResources))
		{
			D3D12_SHADER_INPUT_BIND_DESC shaderInputBindDesc{};
			ThrowIfFailed(ShaderReflection->GetResourceBindingDesc(i, &shaderInputBindDesc));

			if (shaderInputBindDesc.Type == D3D_SIT_CBUFFER)
			{
				// root param index ?
				//rootParameterIndexMap[stringToWString(shaderInputBindDesc.Name)] = static_cast<uint32_t>(rootParameters.size());
				ID3D12ShaderReflectionConstantBuffer* shaderReflectionConstantBuffer = ShaderReflection->GetConstantBufferByIndex(i);
				D3D12_SHADER_BUFFER_DESC constantBufferDesc{};
				shaderReflectionConstantBuffer->GetDesc(&constantBufferDesc);

				const D3D12_ROOT_PARAMETER1 rootParameter
				{
					.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
					.Descriptor{
						.ShaderRegister = shaderInputBindDesc.BindPoint,
						.RegisterSpace = shaderInputBindDesc.Space,
						.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
					},
				};

				rootParameters.push_back(rootParameter);
			}
			else if (shaderInputBindDesc.Type == D3D_SIT_TEXTURE)
			{
				// root param index ?
				//rootParameterIndexMap[stringToWString(shaderInputBindDesc.Name)] = static_cast<uint32_t>(rootParameters.size());
				const CD3DX12_DESCRIPTOR_RANGE1 srvRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
					1u,
					shaderInputBindDesc.BindPoint,
					shaderInputBindDesc.Space,
					D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

				descriptorRanges.push_back(srvRange);

				const D3D12_ROOT_PARAMETER1 rootParameter
				{
					.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
					.DescriptorTable =
					{
						.NumDescriptorRanges = 1u,
						.pDescriptorRanges = &descriptorRanges.back(),
					},
					.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL,
				};

				rootParameters.push_back(rootParameter);
			}
			else if (shaderInputBindDesc.Type == D3D_SIT_SAMPLER)
			{

			}
			else if (shaderInputBindDesc.Type == D3D_SIT_STRUCTURED)
			{

			}
			else if (shaderInputBindDesc.Type == D3D_SIT_UAV_RWTYPED)
			{

			}
			else if (shaderInputBindDesc.Type == D3D_SIT_BYTEADDRESS)
			{

			}
		}

		const D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSignaureDesc =
		{
			.Version = D3D_ROOT_SIGNATURE_VERSION_1_1,
			.Desc_1_1 =
			{
				.NumParameters = static_cast<uint32_t>(rootParameters.size()),
				.pParameters = rootParameters.data(),
				.NumStaticSamplers = 0u,
				.pStaticSamplers = nullptr,
				.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT,
			},
		};

		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

		// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		if (FAILED(Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		std::string Error;
		HRESULT Serialize_HR = D3DX12SerializeVersionedRootSignature(&rootSignaureDesc, featureData.HighestVersion, &signature, &error);
		if (error)
		{
			Error = (static_cast<char*>(error->GetBufferPointer()));
			Engine::WriteToConsole(Error);
		}
		HRESULT RootSignature_HR = Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&RootSignature));
		ThrowIfFailed(RootSignature_HR);
	}

	virtual ~D3D12RootSignature()
	{
		RootSignature = nullptr;
	}

	ID3D12RootSignature* Get() const { return RootSignature.Get(); }

private:
	ComPtr<ID3D12RootSignature> RootSignature;
};
