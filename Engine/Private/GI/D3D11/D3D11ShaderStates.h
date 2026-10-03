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

#include <vector>
#include "D3D11Device.h"
#include "Engine/ClassBody.h"
#include "Engine/AbstractEngine.h"

class D3D11Rasterizer final
{
	sBaseClassBody(sClassConstructor, D3D11Rasterizer)
public:
	D3D11Rasterizer(D3D11Device* InOwner, sRasterizerAttributeDesc InDesc)
		: RasterizerDesc(InDesc)
		, Owner(InOwner)
	{
		CD3D11_RASTERIZER_DESC1 desc(D3D11_DEFAULT);

		switch (InDesc.FillMode)
		{
		case ERasterizerFillMode::Solid:
			desc.FillMode = D3D11_FILL_SOLID;
			break;
		case  ERasterizerFillMode::Wireframe:
			desc.FillMode = D3D11_FILL_WIREFRAME;
			break;
		default:
			break;
		}

		switch (InDesc.CullMode)
		{
		case ERasterizerCullMode::CCW:
			desc.CullMode = D3D11_CULL_BACK;
			break;
		case ERasterizerCullMode::CW:
			desc.CullMode = D3D11_CULL_FRONT;
			break;
		case ERasterizerCullMode::None:
			desc.CullMode = D3D11_CULL_NONE;
			break;
		}

		desc.FrontCounterClockwise = InDesc.FrontCounterClockwise ? TRUE : FALSE;
		desc.DepthBias = static_cast<INT>(InDesc.DepthBias);
		desc.DepthBiasClamp = InDesc.DepthBiasClamp;
		desc.SlopeScaledDepthBias = InDesc.SlopeScaleDepthBias;
		desc.DepthClipEnable = InDesc.DepthClipEnable ? TRUE : FALSE;
		desc.ScissorEnable = TRUE;
		desc.MultisampleEnable = InDesc.bAllowMSAA ? TRUE : FALSE;
		desc.AntialiasedLineEnable = InDesc.bEnableLineAA ? TRUE : FALSE;

		Owner->GetDevice()->CreateRasterizerState1(&desc, pRasterizerState.GetAddressOf());

#ifdef _DEBUG
		{
			std::string Name = "pRasterizerState";
			pRasterizerState->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(Name) - 1, Name.c_str());
		}
#endif
	}

	virtual	~D3D11Rasterizer() {
		Release();
	}

	void Release()
	{
		Owner = nullptr;
		pRasterizerState = nullptr;
	}

	FORCEINLINE ComPtr<ID3D11RasterizerState1> Get() const { return pRasterizerState; }

private:
	D3D11Device* Owner;
	sRasterizerAttributeDesc RasterizerDesc;
	ComPtr<ID3D11RasterizerState1> pRasterizerState;
};

class D3D11SamplerState
{
	sBaseClassBody(sClassConstructor, D3D11SamplerState)
public:
	D3D11SamplerState(D3D11Device* InOwner, sSamplerAttributeDesc InDesc)
		: SamplerDesc(InDesc)
		, Owner(InOwner)
	{
		CD3D11_SAMPLER_DESC desc(D3D11_DEFAULT);

		const bool bComparisonEnabled = false;// InDesc.SamplerComparisonFunction != ECompareFunction::Never;

		auto Compare = [&](ECompareFunction Mode) -> D3D11_COMPARISON_FUNC
		{
			switch (Mode)
			{
			case ECompareFunction::Less: return D3D11_COMPARISON_LESS;
			case ECompareFunction::LessEqual: return D3D11_COMPARISON_LESS_EQUAL;
			case ECompareFunction::Greater: return D3D11_COMPARISON_GREATER;
			case ECompareFunction::GreaterEqual: return D3D11_COMPARISON_GREATER_EQUAL;
			case ECompareFunction::Equal: return D3D11_COMPARISON_EQUAL;;
			case ECompareFunction::NotEqual: return D3D11_COMPARISON_NOT_EQUAL;
			case ECompareFunction::Never: return D3D11_COMPARISON_NEVER;
			case ECompareFunction::Always: return D3D11_COMPARISON_ALWAYS;
			default: return D3D11_COMPARISON_FUNC();
			}
		};

		switch (InDesc.Filter)
		{
		case ESamplerFilter::Point:
			desc.Filter = bComparisonEnabled ? D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT : D3D11_FILTER_MIN_MAG_MIP_POINT;
			break;
		case ESamplerFilter::Bilinear:
			desc.Filter = bComparisonEnabled ? D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT : D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			break;
		case ESamplerFilter::Trilinear:
			desc.Filter = bComparisonEnabled ? D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			break;
		case ESamplerFilter::AnisotropicPoint:
		case ESamplerFilter::AnisotropicLinear:
			if (InDesc.MaxAnisotropy == 1)
			{
				desc.Filter = bComparisonEnabled ? D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			}
			else
			{
				// D3D11 doesn't allow using point filtering for mip filter when using anisotropic filtering
				desc.Filter = bComparisonEnabled ? D3D11_FILTER_COMPARISON_ANISOTROPIC : D3D11_FILTER_ANISOTROPIC;
			}
			break;
		}

		auto Mode = [&](ESamplerAddressMode Mode) -> D3D11_TEXTURE_ADDRESS_MODE
		{
			switch (Mode)
			{
			case ESamplerAddressMode::Wrap: return D3D11_TEXTURE_ADDRESS_WRAP;
			case ESamplerAddressMode::Clamp: return D3D11_TEXTURE_ADDRESS_CLAMP;
			case ESamplerAddressMode::Mirror: return D3D11_TEXTURE_ADDRESS_MIRROR;
			case ESamplerAddressMode::MirrorOnce: return D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
			case ESamplerAddressMode::Border: return D3D11_TEXTURE_ADDRESS_BORDER;
			default: return D3D11_TEXTURE_ADDRESS_MODE();
			}
		};

		desc.AddressU = Mode(InDesc.AddressU);
		desc.AddressV = Mode(InDesc.AddressV);
		desc.AddressW = Mode(InDesc.AddressW);
		desc.MaxAnisotropy = InDesc.MaxAnisotropy;
		desc.ComparisonFunc = Compare(InDesc.SamplerComparisonFunction);
		desc.BorderColor[0] = static_cast<FLOAT>(InDesc.BorderColor.R);
		desc.BorderColor[1] = static_cast<FLOAT>(InDesc.BorderColor.G);
		desc.BorderColor[2] = static_cast<FLOAT>(InDesc.BorderColor.B);
		desc.BorderColor[3] = static_cast<FLOAT>(InDesc.BorderColor.A);
		desc.MinLOD = static_cast<FLOAT>(InDesc.MinMipLevel);
		desc.MaxLOD = static_cast<FLOAT>(InDesc.MaxMipLevel);
		desc.MipLODBias = static_cast<FLOAT>(InDesc.MipBias);

		Owner->GetDevice()->CreateSamplerState(&desc, pSamplerState.GetAddressOf());
#ifdef _DEBUG
		{
			std::string Name = "pSamplerState";
			pSamplerState->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(Name) - 1, Name.c_str());
		}
#endif
	}

	virtual ~D3D11SamplerState() {
		Release();
	}

	void Release()
	{
		Owner = nullptr;
		pSamplerState = nullptr;
	}

	FORCEINLINE ComPtr<ID3D11SamplerState> Get() const { return pSamplerState; }
	virtual sSamplerAttributeDesc GetSamplerDesc() const { return SamplerDesc; }

private:
	D3D11Device* Owner;
	sSamplerAttributeDesc SamplerDesc;
	ComPtr<ID3D11SamplerState> pSamplerState;
};

class D3DX11SamplerState : public ISamplerState
{
	sClassBody(sClassConstructor, D3DX11SamplerState, ISamplerState)
public:
	D3DX11SamplerState(D3D11Device* InOwner, std::string InName, sSamplerAttributeDesc InDesc)
		: Super()
		, Owner(InOwner)
		, Name(InName)
		, SamplerState(D3D11SamplerState(InOwner, InDesc))
	{

	}

	virtual ~D3DX11SamplerState()
	{
		Owner = nullptr;
	}

	virtual std::string GetName() const override { return Name; }
	virtual sSamplerAttributeDesc GetSamplerDesc() const override { return SamplerState.GetSamplerDesc(); }
	virtual std::uint32_t GetBindlessIndex() const override { return std::uint32_t(-1); }
	D3D11SamplerState Get() const {	return SamplerState; }

private:
	D3D11Device* Owner;
	std::string Name;
	D3D11SamplerState SamplerState;
};

class D3D11DepthStencilState
{
	sBaseClassBody(sClassConstructor, D3D11DepthStencilState)
public:
	D3D11DepthStencilState(D3D11Device* InOwner, sDepthStencilAttributeDesc InDesc)
		: DepthStencilStateDesc(InDesc)
		, Owner(InOwner)
	{
		auto Compare = [&](ECompareFunction Mode) -> D3D11_COMPARISON_FUNC
		{
			switch (Mode)
			{
			case ECompareFunction::Less: return D3D11_COMPARISON_LESS;
			case ECompareFunction::LessEqual: return D3D11_COMPARISON_LESS_EQUAL;
			case ECompareFunction::Greater: return D3D11_COMPARISON_GREATER;
			case ECompareFunction::GreaterEqual: return D3D11_COMPARISON_GREATER_EQUAL;
			case ECompareFunction::Equal: return D3D11_COMPARISON_EQUAL;
			case ECompareFunction::NotEqual: return D3D11_COMPARISON_NOT_EQUAL;
			case ECompareFunction::Never: return D3D11_COMPARISON_NEVER;
			case ECompareFunction::Always:	return D3D11_COMPARISON_ALWAYS;
			default: return D3D11_COMPARISON_FUNC();
			}
		};

		auto Stencil = [&](EStencilOp Mode) -> D3D11_STENCIL_OP
		{
			switch (Mode)
			{
			case EStencilOp::Keep:	return D3D11_STENCIL_OP_KEEP;
			case EStencilOp::Zero:	return D3D11_STENCIL_OP_ZERO;
			case EStencilOp::Replace: return D3D11_STENCIL_OP_REPLACE;
			case EStencilOp::SaturatedIncrement: return D3D11_STENCIL_OP_INCR_SAT;
			case EStencilOp::SaturatedDecrement: return D3D11_STENCIL_OP_DECR_SAT;
			case EStencilOp::Invert: return D3D11_STENCIL_OP_INVERT;
			case EStencilOp::Increment: return D3D11_STENCIL_OP_INCR;
			case EStencilOp::Decrement: return D3D11_STENCIL_OP_DECR;
			default: return D3D11_STENCIL_OP();
			}
		};

		CD3D11_DEPTH_STENCIL_DESC desc(D3D11_DEFAULT);
		desc.DepthFunc = Compare(InDesc.DepthTest);
		desc.DepthEnable = InDesc.DepthTest != ECompareFunction::Always || InDesc.bEnableDepthWrite;
		desc.StencilEnable = InDesc.bStencilEnable;

		desc.DepthWriteMask = InDesc.bDepthWriteMask ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;

		desc.StencilEnable = InDesc.bEnableFrontFaceStencil || InDesc.bEnableBackFaceStencil;
		desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK; //Initializer.StencilReadMask;
		desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK; //Initializer.StencilWriteMask;

		desc.FrontFace.StencilFunc = Compare(InDesc.FrontFaceStencilTest);
		desc.FrontFace.StencilDepthFailOp = Stencil(InDesc.FrontFaceDepthFailStencilOp);
		desc.FrontFace.StencilFailOp = Stencil(InDesc.FrontFaceStencilFailStencilOp);
		desc.FrontFace.StencilPassOp = Stencil(InDesc.FrontFacePassStencilOp);

		desc.BackFace.StencilFunc = Compare(InDesc.BackFaceStencilTest);
		desc.BackFace.StencilDepthFailOp = Stencil(InDesc.BackFaceDepthFailStencilOp);
		desc.BackFace.StencilFailOp = Stencil(InDesc.BackFaceStencilFailStencilOp);
		desc.BackFace.StencilPassOp = Stencil(InDesc.BackFacePassStencilOp);

		Owner->GetDevice()->CreateDepthStencilState(&desc, pDepthStencilState.GetAddressOf());
#ifdef _DEBUG
		{
			std::string Name = "pDepthStencilState";
			pDepthStencilState->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(Name) - 1, Name.c_str());
		}
#endif
	}

	virtual ~D3D11DepthStencilState() {
		Release();
	}

	void Release()
	{
		Owner = nullptr;
		pDepthStencilState = nullptr;
	}

	FORCEINLINE ComPtr<ID3D11DepthStencilState> Get() const { return pDepthStencilState; }

private:
	D3D11Device* Owner;
	sDepthStencilAttributeDesc DepthStencilStateDesc;
	ComPtr<ID3D11DepthStencilState> pDepthStencilState;
};

class D3D11BlendState
{
	sBaseClassBody(sClassConstructor, D3D11BlendState)
public:
	D3D11BlendState(D3D11Device* InOwner, sBlendAttributeDesc InDesc)
		: BlendStateDesc(InDesc)
		, Owner(InOwner)
	{
		auto BlendOP = [&](EBlendOperation var) -> D3D11_BLEND_OP
		{
			switch (var)
			{
			case EBlendOperation::Add:
				return D3D11_BLEND_OP_ADD;
				break;
			case EBlendOperation::Subtract:
				return D3D11_BLEND_OP_SUBTRACT;
				break;
			case EBlendOperation::Min:
				return D3D11_BLEND_OP_MIN;
				break;
			case EBlendOperation::Max:
				return D3D11_BLEND_OP_MAX;
				break;
			case EBlendOperation::ReverseSubtract:
				return D3D11_BLEND_OP_REV_SUBTRACT;
				break;
			default:
				return D3D11_BLEND_OP_ADD;
				break;
			}
		};

		auto BlendFactor = [&](EBlendFactor var) -> D3D11_BLEND
		{
			switch (var)
			{
			case EBlendFactor::Zero:
				return D3D11_BLEND_ZERO;
				break;
			case EBlendFactor::One:
				return D3D11_BLEND_ONE;
				break;
			case EBlendFactor::SourceColor:
				return D3D11_BLEND_SRC_COLOR;
				break;
			case EBlendFactor::InverseSourceColor:
				return D3D11_BLEND_INV_SRC_COLOR;
				break;
			case EBlendFactor::SourceAlpha:
				return D3D11_BLEND_SRC_ALPHA;
				break;
			case EBlendFactor::InverseSourceAlpha:
				return D3D11_BLEND_INV_SRC_ALPHA;
				break;
			case EBlendFactor::DestAlpha:
				return D3D11_BLEND_DEST_ALPHA;
				break;
			case EBlendFactor::InverseDestAlpha:
				return D3D11_BLEND_INV_DEST_ALPHA;
				break;
			case EBlendFactor::DestColor:
				return D3D11_BLEND_DEST_COLOR;
				break;
			case EBlendFactor::InverseDestColor:
				return D3D11_BLEND_INV_DEST_COLOR;
				break;
			case EBlendFactor::BlendFactor:
				return D3D11_BLEND_BLEND_FACTOR;
				break;
			case EBlendFactor::InverseBlendFactor:
				return D3D11_BLEND_INV_BLEND_FACTOR;
				break;
			default:
				return D3D11_BLEND_ZERO;
				break;
			}
		};

		auto ColorWriteMask = [&](EColorWriteMask var) -> UINT8
		{
			switch (var)
			{
			case EColorWriteMask::NONE:
				return 0;
				break;
			case EColorWriteMask::RED:
				return D3D11_COLOR_WRITE_ENABLE_RED;
				break;
			case EColorWriteMask::GREEN:
				return D3D11_COLOR_WRITE_ENABLE_GREEN;
				break;
			case EColorWriteMask::BLUE:
				return D3D11_COLOR_WRITE_ENABLE_BLUE;
				break;
			case EColorWriteMask::ALPHA:
				return D3D11_COLOR_WRITE_ENABLE_ALPHA;
				break;
			case EColorWriteMask::RGB:
				return D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE;
				break;
			case EColorWriteMask::RGBA:
				return D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE | D3D11_COLOR_WRITE_ENABLE_ALPHA;
				break;
			case EColorWriteMask::RG:
				return D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN;
				break;
			case EColorWriteMask::BA:
				return D3D11_COLOR_WRITE_ENABLE_BLUE | D3D11_COLOR_WRITE_ENABLE_ALPHA;
				break;
			default:
				return 0;
				break;
			}
		};

		CD3D11_BLEND_DESC1 desc(D3D11_DEFAULT);

		// Additive blending
		desc.AlphaToCoverageEnable = BlendStateDesc.balphaToCoverage ? TRUE : FALSE;
		desc.IndependentBlendEnable = BlendStateDesc.bUseIndependentRenderTargetBlendStates ? TRUE : FALSE;
		//desc.IndependentBlendEnable = false;// TRUE;

		std::uint32_t i = 0;
		for (const auto& var : BlendStateDesc.RenderTargets)
		{
			if (!var.bBlendEnable)
				continue;

			desc.RenderTarget[i].LogicOpEnable = false;
			desc.RenderTarget[i].LogicOp = D3D11_LOGIC_OP_NOOP;

			desc.RenderTarget[i].BlendEnable = var.bBlendEnable;
			desc.RenderTarget[i].SrcBlend = BlendFactor(var.ColorSrcBlend);
			desc.RenderTarget[i].DestBlend = BlendFactor(var.ColorDestBlend);
			desc.RenderTarget[i].BlendOp = BlendOP(var.ColorBlendOp);

			desc.RenderTarget[i].SrcBlendAlpha = BlendFactor(var.AlphaSrcBlend);
			desc.RenderTarget[i].DestBlendAlpha = BlendFactor(var.AlphaDestBlend);
			desc.RenderTarget[i].BlendOpAlpha = BlendOP(var.AlphaBlendOp);
			desc.RenderTarget[i].RenderTargetWriteMask = ColorWriteMask(var.ColorWriteMask);
			i++;
		}

		Owner->GetDevice()->CreateBlendState1(&desc, pBlendState.GetAddressOf());
#ifdef _DEBUG
		{
			std::string Name = "pBlendState";
			pBlendState->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(Name) - 1, Name.c_str());
		}
#endif
	}

	virtual ~D3D11BlendState() {
		Release();
	}

	void Release()
	{
		Owner = nullptr;
		pBlendState = nullptr;
	}

	FORCEINLINE ComPtr<ID3D11BlendState1> Get() const { return pBlendState; }

private:
	D3D11Device* Owner;
	sBlendAttributeDesc BlendStateDesc;
	ComPtr<ID3D11BlendState1> pBlendState;
};

class D3D11VertexAttribute
{
	sBaseClassBody(sClassConstructor, D3D11VertexAttribute)
public:
	D3D11VertexAttribute(D3D11Device* InDevice, std::vector<sVertexAttributeDesc> InDesc, void* InShaderCode = 0);

	virtual ~D3D11VertexAttribute()
	{
		Release();
	}

	void Release()
	{
		Owner = nullptr;
		pVertexAttribute = nullptr;
		VertexAttributeDescData.clear();
	}

	FORCEINLINE ComPtr<ID3D11InputLayout> Get() const { return pVertexAttribute; }
	std::vector<sVertexAttributeDesc> GetVertexAttributeDesc() const { return VertexAttributeDescData; }

private:
	D3D11Device* Owner;
	std::vector<sVertexAttributeDesc> VertexAttributeDescData;
	ComPtr<ID3D11InputLayout> pVertexAttribute;
};
