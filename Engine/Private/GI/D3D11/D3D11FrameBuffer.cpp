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
#include "D3D11FrameBuffer.h"
#include "D3D11CommandBuffer.h"

/*
Depth Formats
DXGI_FORMAT_R16_TYPELESS
DXGI_FORMAT_R16_UINT
DXGI_FORMAT_D16_UNORM

DXGI_FORMAT_R32_TYPELESS
DXGI_FORMAT_R32_UINT
DXGI_FORMAT_D32_FLOAT

// NO SRV Only
DXGI_FORMAT_R24G8_TYPELESS
DXGI_FORMAT_D24_UNORM_S8_UINT

DXGI_FORMAT_R32G8X24_TYPELESS
DXGI_FORMAT_D32_FLOAT_S8X24_UINT
*/

/*struct FBO_Depth
{
	ComPtr<ID3D11Texture2D> Texture;
	ComPtr<ID3D11ShaderResourceView> ShaderResource;
	ComPtr<ID3D11DepthStencilView> DepthResource;


	FBO_Depth()
		: Texture(nullptr)
		, ShaderResource(nullptr)
		, DepthResource(nullptr)
	{}

	~FBO_Depth()
	{
		Texture = nullptr;
		ShaderResource = nullptr;
		DepthResource = nullptr;
	}
};

std::vector<FBO_Depth*> GetSupportedDepths(ID3D11Device* Device, const sFBODesc& FDesc, bool WithSRV)
{
	std::vector<FBO_Depth*> Depths;
	const auto Formats = GetAllDXGIFormats();

	for (const auto& Format : Formats)
	{
		FBO_Depth* DBuffer = new FBO_Depth;
		ComPtr<ID3D11Texture2D> Texture;
		D3D11_TEXTURE2D_DESC sTextureDesc;
		ZeroMemory(&sTextureDesc, sizeof(sTextureDesc));
		sTextureDesc.Width = FDesc.Dimensions.X;
		sTextureDesc.Height = FDesc.Dimensions.Y;
		sTextureDesc.MipLevels = 1;
		sTextureDesc.ArraySize = 1;
		sTextureDesc.Format = Format;
		sTextureDesc.SampleDesc.Count = FDesc.MSLevel.Count;
		sTextureDesc.SampleDesc.Quality = FDesc.MSLevel.Quality;
		sTextureDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
		sTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		sTextureDesc.CPUAccessFlags = NULL;
		sTextureDesc.MiscFlags = NULL;

		Device->CreateTexture2D(&sTextureDesc, NULL, Texture.GetAddressOf());
		DBuffer->Texture = Texture;
		//DBuffer->TextureFormat = Format;
		if (!Texture)
		{
			delete DBuffer;
			DBuffer = nullptr;
			continue;
		}

		for (const auto& DSVFormat : Formats)
		{
			CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilDesc(
				D3D11_DSV_DIMENSION_TEXTURE2D,
				DSVFormat,
				0,          // Mips
				1, 1        // Array
			);

			ComPtr<ID3D11DepthStencilView> depthStencilView;
			Device->CreateDepthStencilView(Texture.Get(), &depthStencilDesc, depthStencilView.GetAddressOf());
			DBuffer->DepthResource = depthStencilView;
			//DBuffer->DepthFormat = DSVFormat;

			if (depthStencilView)
				break;
		}

		if (WithSRV)
		{
			for (const auto& SRVFormat : Formats)
			{
				CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(
					D3D_SRV_DIMENSION_TEXTURE2D,
					SRVFormat,
					0, 1,  // Mips
					0, sTextureDesc.ArraySize   // Array
				);

				ComPtr<ID3D11ShaderResourceView> ShaderResource;
				Device->CreateShaderResourceView(Texture.Get(), &srvDesc, ShaderResource.GetAddressOf());
				DBuffer->ShaderResource = ShaderResource;
				//DBuffer->SRVFormat = SRVFormat;

				if (ShaderResource)
					break;
			}
		}

		if (DBuffer->Texture && DBuffer->DepthResource && (WithSRV && DBuffer->ShaderResource))
		{
			Depths.push_back(DBuffer);
		}
		else
		{
			delete DBuffer;
			DBuffer = nullptr;
		}
	}
	return Depths;
};*/

D3D11RenderTarget::D3D11RenderTarget(D3D11Device* InDevice, const std::string InName, const EFormat InFormat, const sFBODesc& Desc)
	: Super()
	, Name(InName)
	, Format(InFormat)
	, Texture(nullptr)
	, RenderTarget(nullptr)
	, ShaderResource(nullptr)
{
	const DXGI_FORMAT DXGIFormat = (ConvertFormat_Format_To_DXGI(Format));

	D3D11_TEXTURE2D_DESC sTextureDesc;
	ZeroMemory(&sTextureDesc, sizeof(sTextureDesc));
	sTextureDesc.Width = Desc.Dimensions.X;
	sTextureDesc.Height = Desc.Dimensions.Y;
	sTextureDesc.MipLevels = 1;
	sTextureDesc.ArraySize = 1;
	sTextureDesc.Format = DXGIFormat;
	sTextureDesc.SampleDesc.Count = Desc.MSLevel.Count;
	sTextureDesc.SampleDesc.Quality = Desc.MSLevel.Quality;
	sTextureDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	sTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
	sTextureDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
	//sTextureDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
	sTextureDesc.CPUAccessFlags = NULL;
	sTextureDesc.MiscFlags = NULL;

	InDevice->Get()->CreateTexture2D(&sTextureDesc, NULL, Texture.GetAddressOf());

	{
		CD3D11_RENDER_TARGET_VIEW_DESC rtvElementDesc(
			D3D11_RTV_DIMENSION_TEXTURE2D,
			DXGIFormat,
			0,          // Mips
			0, 1        // Array
		);

		InDevice->Get()->CreateRenderTargetView(Texture.Get(), &rtvElementDesc, RenderTarget.GetAddressOf());
	}

	{
		CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(
			D3D_SRV_DIMENSION_TEXTURE2D,
			DXGIFormat,
			0, sTextureDesc.MipLevels,  // Mips
			0, sTextureDesc.ArraySize   // Array
		);

		InDevice->Get()->CreateShaderResourceView(Texture.Get(), &srvDesc, ShaderResource.GetAddressOf());
	}
}

D3D11RenderTarget::~D3D11RenderTarget()
{
	Texture = nullptr;
	RenderTarget = nullptr;
	ShaderResource = nullptr;
}

D3D11DepthTarget::D3D11DepthTarget(D3D11Device* InDevice, const std::string InName, const EFormat InFormat, const sFBODesc& Desc)
	: Super()
	, Name(InName)
	, Format(InFormat)
	, Texture(nullptr)
	, DepthResource(nullptr)
	, ShaderResource(nullptr)
{
	bIsSRVSupported = IsDepthSRVSupported(Format);

	D3D11_TEXTURE2D_DESC sTextureDesc;
	ZeroMemory(&sTextureDesc, sizeof(sTextureDesc));
	sTextureDesc.Width = Desc.Dimensions.X;
	sTextureDesc.Height = Desc.Dimensions.Y;
	sTextureDesc.MipLevels = 1;
	sTextureDesc.ArraySize = 1;
	sTextureDesc.Format = ConvertFormat_Format_To_DXGI(Format);
	sTextureDesc.SampleDesc.Count = Desc.MSLevel.Count;
	sTextureDesc.SampleDesc.Quality = Desc.MSLevel.Quality;
	sTextureDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	sTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	if (bIsSRVSupported)
		sTextureDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
	sTextureDesc.CPUAccessFlags = NULL;
	sTextureDesc.MiscFlags = NULL;

	ThrowIfFailed(InDevice->Get()->CreateTexture2D(&sTextureDesc, NULL, Texture.GetAddressOf()));

	{
		CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilDesc(
			D3D11_DSV_DIMENSION_TEXTURE2D,
			GetDepthViewFormat(sTextureDesc.Format),
			0,          // Mips
			0, 1        // Array
		);

		ThrowIfFailed(InDevice->Get()->CreateDepthStencilView(Texture.Get(), &depthStencilDesc, DepthResource.GetAddressOf()));
	}

	if (bIsSRVSupported)
	{
		CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(
			D3D_SRV_DIMENSION_TEXTURE2D,
			GetDepthSRVFormat(sTextureDesc.Format),
			0, 1,  // Mips
			0, sTextureDesc.ArraySize   // Array
		);

		ThrowIfFailed(InDevice->Get()->CreateShaderResourceView(Texture.Get(), &srvDesc, ShaderResource.GetAddressOf()));
	}
}

D3D11DepthTarget::~D3D11DepthTarget()
{
	Texture = nullptr;
	DepthResource = nullptr;
	ShaderResource = nullptr;
}

D3D11UnorderedAccessTarget::D3D11UnorderedAccessTarget(D3D11Device* InDevice, const std::string InName, const EFormat InFormat, const sFBODesc& Desc, bool InEnableSRV)
	: Super()
	, Name(InName)
	, Format(InFormat)
	, Texture(nullptr)
	, UnorderedAccessView(nullptr)
	, ShaderResource(nullptr)
	, bIsSRVSupported(InEnableSRV)
{
	const DXGI_FORMAT DXGIFormat = (ConvertFormat_Format_To_DXGI(Format));

	D3D11_TEXTURE2D_DESC sTextureDesc;
	ZeroMemory(&sTextureDesc, sizeof(sTextureDesc));
	sTextureDesc.Width = Desc.Dimensions.X;
	sTextureDesc.Height = Desc.Dimensions.Y;
	sTextureDesc.MipLevels = 1;
	sTextureDesc.ArraySize = 1;
	sTextureDesc.Format = DXGIFormat;
	sTextureDesc.SampleDesc.Count = Desc.MSLevel.Count;
	sTextureDesc.SampleDesc.Quality = Desc.MSLevel.Quality;
	sTextureDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	sTextureDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	if (bIsSRVSupported)
		sTextureDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
	sTextureDesc.CPUAccessFlags = NULL;
	sTextureDesc.MiscFlags = NULL;

	InDevice->Get()->CreateTexture2D(&sTextureDesc, NULL, Texture.GetAddressOf());

	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc;
		UAVDesc.Format = DXGIFormat;
		UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		UAVDesc.Texture2D.MipSlice = 0;

		InDevice->Get()->CreateUnorderedAccessView(Texture.Get(), &UAVDesc, UnorderedAccessView.GetAddressOf());
	}

	if (bIsSRVSupported)
	{
		CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(
			D3D_SRV_DIMENSION_TEXTURE2D,
			DXGIFormat,
			0, sTextureDesc.MipLevels,  // Mips
			0, sTextureDesc.ArraySize   // Array
		);

		InDevice->Get()->CreateShaderResourceView(Texture.Get(), &srvDesc, ShaderResource.GetAddressOf());
	}
}

D3D11UnorderedAccessTarget::~D3D11UnorderedAccessTarget()
{
	Texture = nullptr;
	UnorderedAccessView = nullptr;
	ShaderResource = nullptr;
}

D3D11FrameBuffer::D3D11FrameBuffer(D3D11Device* InDevice, const std::string InName, const sFrameBufferAttachmentInfo& InAttachments)
	: Owner(InDevice)
	, AttachmentInfo(InAttachments)
	, Name(InName)
{
	const auto& FDesc = AttachmentInfo.Desc;

	for (std::size_t i = 0; i < AttachmentInfo.FrameBuffer.size(); i++)
	{
		const auto& FB = AttachmentInfo.FrameBuffer[i];

		if (FB.bIsUAV)
		{
			UAVs.push_back(D3D11UnorderedAccessTarget::Create(InDevice, Name + "_UAV_" + std::to_string(UAVs.size()), FB.Format, FDesc, FB.bIsShaderResource));
		}
		else
		{
			RenderTargets.push_back(D3D11RenderTarget::Create(InDevice, Name + "_RenderTarget_" + std::to_string(RenderTargets.size()), FB.Format, FDesc));
		}
	}
	
	if (IsValidDepthFormat(AttachmentInfo.DepthFormat))
	{
		DepthTarget = D3D11DepthTarget::Create(InDevice, Name + "_DepthTarget", AttachmentInfo.DepthFormat, FDesc);
	}
}

void D3D11FrameBuffer::AttachRenderTarget(const IRenderTarget::SharedPtr& RenderTarget, std::optional<std::size_t> Index)
{
	if (auto RT = std::dynamic_pointer_cast<D3D11RenderTarget>(RenderTarget))
	{
		if (Index.has_value())
			RenderTargets.insert(RenderTargets.begin() + Index.value(), RT);
		else
			RenderTargets.push_back(RT);

		if (Index.has_value())
			AttachmentInfo.FrameBuffer.insert(AttachmentInfo.FrameBuffer.begin() + Index.value(), sFrameBufferAttachmentInfo::sFrameBuffer(RT->GetFormat(), true, false));
		else
			AttachmentInfo.AddFrameBuffer(RT->GetFormat(), true, false);
	}
}

void D3D11FrameBuffer::AttachUnorderedAccessTarget(const IUnorderedAccessTarget::SharedPtr& UnorderedAccessTarget, std::optional<std::size_t> Index)
{
	if (auto ST = std::dynamic_pointer_cast<D3D11UnorderedAccessTarget>(UnorderedAccessTarget))
	{
		if (Index.has_value())
			UAVs.insert(UAVs.begin() + Index.value(), ST);
		else
			UAVs.push_back(ST);

		if (Index.has_value())
			AttachmentInfo.FrameBuffer.insert(AttachmentInfo.FrameBuffer.begin() + Index.value(), sFrameBufferAttachmentInfo::sFrameBuffer(ST->GetFormat(), ST->IsSRVSupported(), true));
		else
			AttachmentInfo.AddFrameBuffer(ST->GetFormat(), ST->IsSRVSupported(), true);
	}
}

void D3D11FrameBuffer::SetDepthTarget(const IDepthTarget::SharedPtr& InDepthTarget)
{
	if (auto DT = std::dynamic_pointer_cast<D3D11DepthTarget>(InDepthTarget))
	{
		DepthTarget =  nullptr;
		DepthTarget = DT;
		AttachmentInfo.DepthFormat = DepthTarget->GetFormat();
	}
}

void D3D11FrameBuffer::ApplyFrameBuffer(IGraphicsCommandContext* InCMDBuffer, std::optional<std::size_t> FBOIndex)
{
	ID3D11DeviceContext1* CMD = InCMDBuffer ? static_cast<D3D11CommandBuffer*>(InCMDBuffer)->Get() : Owner->GetDeviceIMContext();
	
	std::vector<ID3D11RenderTargetView*> RenderTargetElements;
	if (FBOIndex.has_value())
	{
		RenderTargetElements.push_back(RenderTargets.at(*FBOIndex)->GetD3D11RTV());
	}
	else
	{
		for (auto& RT : RenderTargets)
		{
			RenderTargetElements.push_back(RT->GetD3D11RTV());
		}
	}

	CMD->OMSetRenderTargets(static_cast<std::uint32_t>(RenderTargetElements.size()), RenderTargetElements.data(), DepthTarget ? DepthTarget->GetD3D11DSV() : nullptr);

	CMD = nullptr;
}

void D3D11FrameBuffer::ApplyFrameBuffer(ID3D11DeviceContext1* CMD, std::optional<std::size_t> FBOIndex)
{
	std::vector<ID3D11RenderTargetView*> RenderTargetElements;
	if (FBOIndex.has_value())
	{
		RenderTargetElements.push_back(RenderTargets.at(*FBOIndex)->GetD3D11RTV());
	}
	else
	{
		for (auto& RT : RenderTargets)
		{
			RenderTargetElements.push_back(RT->GetD3D11RTV());
		}
	}

	CMD->OMSetRenderTargets(static_cast<std::uint32_t>(RenderTargetElements.size()), RenderTargetElements.data(), DepthTarget ? DepthTarget->GetD3D11DSV() : nullptr);
}

void D3D11FrameBuffer::ApplyFrameBufferAsResource(ID3D11DeviceContext1* CMD, std::uint32_t StartSlot, eShaderType InType)
{
	int i = StartSlot;
	for (const auto& RT : RenderTargets)
	{
		if (const auto& SRV = RT->GetD3D11SRV())
		{
			switch (InType)
			{
			case eShaderType::Vertex:
				CMD->VSSetShaderResources(i, 1, &SRV);
				break;
			case eShaderType::Pixel:
				CMD->PSSetShaderResources(i, 1, &SRV);
				break;
			case eShaderType::Geometry:
				CMD->GSSetShaderResources(i, 1, &SRV);
				break;
			case eShaderType::Compute:
				CMD->CSSetShaderResources(i, 1, &SRV);
				break;
			case eShaderType::HULL:
				CMD->HSSetShaderResources(i, 1, &SRV);
				break;
			case eShaderType::Domain:
				CMD->DSSetShaderResources(i, 1, &SRV);
				break;
			}
		}
		i++;
	}

	CMD = nullptr;
}

void D3D11FrameBuffer::ApplyFrameBufferAsResource(ID3D11DeviceContext1* CMD, std::uint32_t FBIndex, std::uint32_t StartSlot, eShaderType InType)
{
	const int i = StartSlot;

	if (FBIndex >= RenderTargets.size())
		return;

	auto& RT = RenderTargets.at(FBIndex);

	if (const auto& SRV = RT->GetD3D11SRV())
	{
		switch (InType)
		{
		case eShaderType::Vertex:
			CMD->VSSetShaderResources(i, 1, &SRV);
			break;
		case eShaderType::Pixel:
			CMD->PSSetShaderResources(i, 1, &SRV);
			break;
		case eShaderType::Geometry:
			CMD->GSSetShaderResources(i, 1, &SRV);
			break;
		case eShaderType::Compute:
			CMD->CSSetShaderResources(i, 1, &SRV);
			break;
		case eShaderType::HULL:
			CMD->HSSetShaderResources(i, 1, &SRV);
			break;
		case eShaderType::Domain:
			CMD->DSSetShaderResources(i, 1, &SRV);
			break;
		}
	}

	CMD = nullptr;
}

void D3D11FrameBuffer::ClearRTs(IGraphicsCommandContext* InCMDBuffer)
{
	ID3D11DeviceContext1* CMD = InCMDBuffer ? static_cast<D3D11CommandBuffer*>(InCMDBuffer)->Get() : Owner->GetDeviceIMContext();
	for (auto& RT : RenderTargets)
	{
		float zeros[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		CMD->ClearRenderTargetView(RT->GetD3D11RTV(), zeros);
	}

	if (DepthTarget)
	{
		std::uint32_t ClearFlags = 0;
		ClearFlags |= D3D11_CLEAR_DEPTH;
		ClearFlags |= D3D11_CLEAR_STENCIL;
		CMD->ClearDepthStencilView(DepthTarget->GetD3D11DSV(), ClearFlags, 0.0f, (UINT8)0);
	}

	CMD = nullptr;
}
