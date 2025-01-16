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
		case ERasterizerFillMode::eSolid:
			desc.FillMode = D3D11_FILL_SOLID;
			break;
		case  ERasterizerFillMode::eWireframe:
			desc.FillMode = D3D11_FILL_WIREFRAME;
			break;
		default:
			break;
		}

		switch (InDesc.CullMode)
		{
		case ERasterizerCullMode::eCCW:
			desc.CullMode = D3D11_CULL_BACK;
			break;
		case ERasterizerCullMode::eCW:
			desc.CullMode = D3D11_CULL_FRONT;
			break;
		case ERasterizerCullMode::eNone:
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

		const bool bComparisonEnabled = false;// InDesc.SamplerComparisonFunction != ECompareFunction::eNever;

		auto Compare = [&](ECompareFunction Mode) -> D3D11_COMPARISON_FUNC
		{
			switch (Mode)
			{
			case ECompareFunction::eLess: return D3D11_COMPARISON_LESS;
			case ECompareFunction::eLessEqual: return D3D11_COMPARISON_LESS_EQUAL;
			case ECompareFunction::eGreater: return D3D11_COMPARISON_GREATER;
			case ECompareFunction::eGreaterEqual: return D3D11_COMPARISON_GREATER_EQUAL;
			case ECompareFunction::eEqual: return D3D11_COMPARISON_EQUAL;;
			case ECompareFunction::eNotEqual: return D3D11_COMPARISON_NOT_EQUAL;
			case ECompareFunction::eNever: return D3D11_COMPARISON_NEVER;
			case ECompareFunction::eAlways: return D3D11_COMPARISON_ALWAYS;
			default: return D3D11_COMPARISON_FUNC();
			}
		};

		switch (InDesc.Filter)
		{
		case ESamplerFilter::ePoint:
			desc.Filter = bComparisonEnabled ? D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT : D3D11_FILTER_MIN_MAG_MIP_POINT;
			break;
		case ESamplerFilter::eBilinear:
			desc.Filter = bComparisonEnabled ? D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT : D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			break;
		case ESamplerFilter::eTrilinear:
			desc.Filter = bComparisonEnabled ? D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			break;
		case ESamplerFilter::eAnisotropicPoint:
		case ESamplerFilter::eAnisotropicLinear:
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
			case ESamplerAddressMode::eWrap: return D3D11_TEXTURE_ADDRESS_WRAP;
			case ESamplerAddressMode::eClamp: return D3D11_TEXTURE_ADDRESS_CLAMP;
			case ESamplerAddressMode::eMirror: return D3D11_TEXTURE_ADDRESS_MIRROR;
			case ESamplerAddressMode::eMirrorOnce: return D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
			case ESamplerAddressMode::eBorder: return D3D11_TEXTURE_ADDRESS_BORDER;
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

private:
	D3D11Device* Owner;
	sSamplerAttributeDesc SamplerDesc;
	ComPtr<ID3D11SamplerState> pSamplerState;
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
			case ECompareFunction::eLess: return D3D11_COMPARISON_LESS;
			case ECompareFunction::eLessEqual: return D3D11_COMPARISON_LESS_EQUAL;
			case ECompareFunction::eGreater: return D3D11_COMPARISON_GREATER;
			case ECompareFunction::eGreaterEqual: return D3D11_COMPARISON_GREATER_EQUAL;
			case ECompareFunction::eEqual: return D3D11_COMPARISON_EQUAL;
			case ECompareFunction::eNotEqual: return D3D11_COMPARISON_NOT_EQUAL;
			case ECompareFunction::eNever: return D3D11_COMPARISON_NEVER;
			case ECompareFunction::eAlways:	return D3D11_COMPARISON_ALWAYS;
			default: return D3D11_COMPARISON_FUNC();
			}
		};

		auto Stencil = [&](EStencilOp Mode) -> D3D11_STENCIL_OP
		{
			switch (Mode)
			{
			case EStencilOp::eKeep:	return D3D11_STENCIL_OP_KEEP;
			case EStencilOp::eZero:	return D3D11_STENCIL_OP_ZERO;
			case EStencilOp::eReplace: return D3D11_STENCIL_OP_REPLACE;
			case EStencilOp::eSaturatedIncrement: return D3D11_STENCIL_OP_INCR_SAT;
			case EStencilOp::eSaturatedDecrement: return D3D11_STENCIL_OP_DECR_SAT;
			case EStencilOp::eInvert: return D3D11_STENCIL_OP_INVERT;
			case EStencilOp::eIncrement: return D3D11_STENCIL_OP_INCR;
			case EStencilOp::eDecrement: return D3D11_STENCIL_OP_DECR;
			default: return D3D11_STENCIL_OP();
			}
		};

		CD3D11_DEPTH_STENCIL_DESC desc(D3D11_DEFAULT);
		desc.DepthFunc = Compare(InDesc.DepthTest);
		desc.DepthEnable = InDesc.DepthTest != ECompareFunction::eAlways || InDesc.bEnableDepthWrite;
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
			case EBlendOperation::eAdd:
				return D3D11_BLEND_OP_ADD;
				break;
			case EBlendOperation::eSubtract:
				return D3D11_BLEND_OP_SUBTRACT;
				break;
			case EBlendOperation::eMin:
				return D3D11_BLEND_OP_MIN;
				break;
			case EBlendOperation::eMax:
				return D3D11_BLEND_OP_MAX;
				break;
			case EBlendOperation::eReverseSubtract:
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
			case EBlendFactor::eZero:
				return D3D11_BLEND_ZERO;
				break;
			case EBlendFactor::eOne:
				return D3D11_BLEND_ONE;
				break;
			case EBlendFactor::eSourceColor:
				return D3D11_BLEND_SRC_COLOR;
				break;
			case EBlendFactor::eInverseSourceColor:
				return D3D11_BLEND_INV_SRC_COLOR;
				break;
			case EBlendFactor::eSourceAlpha:
				return D3D11_BLEND_SRC_ALPHA;
				break;
			case EBlendFactor::eInverseSourceAlpha:
				return D3D11_BLEND_INV_SRC_ALPHA;
				break;
			case EBlendFactor::eDestAlpha:
				return D3D11_BLEND_DEST_ALPHA;
				break;
			case EBlendFactor::eInverseDestAlpha:
				return D3D11_BLEND_INV_DEST_ALPHA;
				break;
			case EBlendFactor::eDestColor:
				return D3D11_BLEND_DEST_COLOR;
				break;
			case EBlendFactor::eInverseDestColor:
				return D3D11_BLEND_INV_DEST_COLOR;
				break;
			case EBlendFactor::eBlendFactor:
				return D3D11_BLEND_BLEND_FACTOR;
				break;
			case EBlendFactor::eInverseBlendFactor:
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
			case EColorWriteMask::eNONE:
				return 0;
				break;
			case EColorWriteMask::eRED:
				return D3D11_COLOR_WRITE_ENABLE_RED;
				break;
			case EColorWriteMask::eGREEN:
				return D3D11_COLOR_WRITE_ENABLE_GREEN;
				break;
			case EColorWriteMask::eBLUE:
				return D3D11_COLOR_WRITE_ENABLE_BLUE;
				break;
			case EColorWriteMask::eALPHA:
				return D3D11_COLOR_WRITE_ENABLE_ALPHA;
				break;
			case EColorWriteMask::eRGB:
				return D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE;
				break;
			case EColorWriteMask::eRGBA:
				return D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE | D3D11_COLOR_WRITE_ENABLE_ALPHA;
				break;
			case EColorWriteMask::eRG:
				return D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN;
				break;
			case EColorWriteMask::eBA:
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
