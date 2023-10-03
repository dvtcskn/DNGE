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
#include "D3D11Texture.h"
#include "WICTextureLoader_Internal.h"
#include "DDSTextureLoader_Internal.h"
#include "D3D11CommandBuffer.h"
#include "wincodec.h"
#include "Utilities/FileManager.h"

#include "ScreenGrab11.h"

static std::uint32_t D3D11_BytesPerPixel(DXGI_FORMAT Format)
{
	return (std::uint32_t)DXGI_BitsPerPixel(Format) / 8;
}

D3D11Texture::D3D11Texture(D3D11Device* InDevice, const std::wstring FilePath, const std::string InName, std::uint32_t InRootParameterIndex)
	: Owner(InDevice)
	, Name(InName)
	, Path(FilePath)
	, RootParameterIndex(InRootParameterIndex)
{
    auto WideStringToString = [](const std::wstring& utf16) -> std::string
    {
        const int utf16Length = (int)utf16.length() + 1;
        const int len = WideCharToMultiByte(0, 0, utf16.data(), utf16Length, 0, 0, 0, 0);
        std::string STR(len, '\0');
        WideCharToMultiByte(0, 0, utf16.data(), utf16Length, STR.data(), len, 0, 0);
        return STR;
    };

    std::wstring ws(FilePath);
    std::string str = WideStringToString(ws);
    //std::size_t found = str.find_last_of("/\\");
    //Name = std::string(str.begin() + found, str.end());
    std::string Ext = std::string(str.end() - 4, str.end());

    {
        ComPtr<ID3D11Resource> tempRes;
        ComPtr<ID3D11ShaderResourceView> tempSRV;
        std::uint32_t BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        std::uint32_t MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
        size_t MS = 0;
        HRESULT HR;

        bool DDS = false;
        if (Ext.find("DDS") != std::string::npos || Ext.find("dds") != std::string::npos)
            DDS = true;

        auto CTX = Owner->GetDeviceIMContext();
        if (!DDS)
        {
            std::uint32_t LoadFlags = DirectX::WIC_LOADER_FLAGS_Internal::WIC_LOADER_DEFAULT;
            HR = DirectX::CreateWICTextureFromFileEx_Internal(Owner->GetDevice(), CTX, FilePath.c_str(), MS,
                D3D11_USAGE_DEFAULT, BindFlags, 0, MiscFlags, LoadFlags, tempRes.GetAddressOf(), tempSRV.GetAddressOf());
        }
        else {
            HR = DirectX::CreateDDSTextureFromFileEx_Internal(Owner->GetDevice(), CTX, FilePath.c_str(), MS,
                D3D11_USAGE_DEFAULT, BindFlags, 0, MiscFlags, false, tempRes.GetAddressOf(), tempSRV.GetAddressOf());
        }

        if (HR == S_OK)
        {
            tempRes->QueryInterface(__uuidof (ID3D11Texture2D), (void**)&pTexture);
            tempSRV->QueryInterface(__uuidof (ID3D11ShaderResourceView), (void**)&pTextureSRV);

            tempRes = nullptr;
            tempSRV = nullptr;

            D3D11_TEXTURE2D_DESC pDesc;
            pTexture->GetDesc(&pDesc);
            Desc.Dimensions.X = pDesc.Width;
            Desc.Dimensions.Y = pDesc.Height;
            Desc.MipLevels = pDesc.MipLevels;
			Desc.ArraySize = pDesc.ArraySize;
			Desc.Format = ConvertFormat_DXGI_To_Format(pDesc.Format);
        }
    }
}

D3D11Texture::D3D11Texture(D3D11Device* InDevice, const std::string InName, void* InBuffer, const std::size_t InSize, const sTextureDesc& InDesc, std::uint32_t InRootParameterIndex)
	: Owner(InDevice)
	, Desc(InDesc)
	, Name(InName)
	, Path(L"")
	, RootParameterIndex(InRootParameterIndex)
{
    D3D11_TEXTURE2D_DESC sTextureDesc;
    ZeroMemory(&sTextureDesc, sizeof(sTextureDesc));
    sTextureDesc.Width = Desc.Dimensions.X;
    sTextureDesc.Height = Desc.Dimensions.Y;
    sTextureDesc.MipLevels = Desc.MipLevels;
    sTextureDesc.ArraySize = Desc.ArraySize;
    sTextureDesc.Format = ConvertFormat_Format_To_DXGI(Desc.Format);
    sTextureDesc.SampleDesc.Count = 1;
    sTextureDesc.SampleDesc.Quality = 0;
    sTextureDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA Data;
    Data.pSysMem = InBuffer;
    Data.SysMemPitch = sTextureDesc.Width * D3D11_BytesPerPixel(ConvertFormat_Format_To_DXGI(Desc.Format));
    Data.SysMemSlicePitch = Data.SysMemPitch * sTextureDesc.Height;

    Owner->GetDevice()->CreateTexture2D(&sTextureDesc, &Data, pTexture.GetAddressOf());

    {
        pTexture->GetDesc(&sTextureDesc);
        CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(
            D3D_SRV_DIMENSION_TEXTURE2D,
            sTextureDesc.Format,
            0, sTextureDesc.MipLevels,  // Mips
            0, sTextureDesc.ArraySize   // Array
        );

        Owner->GetDevice()->CreateShaderResourceView(pTexture.Get(), &srvDesc, pTextureSRV.GetAddressOf());
    }
}

D3D11Texture::D3D11Texture(D3D11Device* InDevice, const std::string InName, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
	: Owner(InDevice)
	, Desc(InDesc)
	, Name(InName)
	, Path(L"")
	, RootParameterIndex(DefaultRootParameterIndex)
{
	ID3D11Device1* Direct3DDevice = Owner->GetDevice();
	ID3D11DeviceContext1* Direct3DDeviceIMContext = Owner->GetDeviceIMContext();

	auto ArraySize = InDesc.ArraySize;

	{
		D3D11_TEXTURE2D_DESC TextureDesc;
		ZeroMemory(&TextureDesc, sizeof(TextureDesc));
		TextureDesc.Width = Desc.Dimensions.X;
		TextureDesc.Height = Desc.Dimensions.Y;
		TextureDesc.MipLevels = Desc.MipLevels;
		TextureDesc.ArraySize = ArraySize;
		TextureDesc.Format = ConvertFormat_Format_To_DXGI(Desc.Format);
		TextureDesc.SampleDesc.Count = 1;
		TextureDesc.SampleDesc.Quality = 0;
		TextureDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

		Owner->GetDevice()->CreateTexture2D(&TextureDesc, nullptr, &pTexture);

		pTexture->GetDesc(&TextureDesc);
		Desc.MipLevels = TextureDesc.MipLevels;

		D3D11_SHADER_RESOURCE_VIEW_DESC TiledViewDesc;
		ZeroMemory(&TiledViewDesc, sizeof(TiledViewDesc));
		TiledViewDesc.Format = TextureDesc.Format;
		TiledViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		TiledViewDesc.Texture2DArray.MostDetailedMip = 0;
		TiledViewDesc.Texture2DArray.MipLevels = TextureDesc.MipLevels;
		TiledViewDesc.Texture2DArray.FirstArraySlice = 0;
		TiledViewDesc.Texture2DArray.ArraySize = (UINT)TextureDesc.ArraySize;

		Direct3DDevice->CreateShaderResourceView(pTexture.Get(), &TiledViewDesc, &pTextureSRV);
	}
}

void D3D11Texture::UpdateTexture(ITexture2D* SourceTexture, std::size_t SourceArrayIndex, std::size_t ArrayIndex, const std::optional<IntVector2> Dest, const std::optional<FBounds2D> TargetBounds)
{
	ID3D11Device1* Direct3DDevice = Owner->GetDevice();
	ID3D11DeviceContext1* Direct3DDeviceIMContext = Owner->GetDeviceIMContext();

	ID3D11Texture2D* RawTexture = static_cast<D3D11Texture*>(SourceTexture)->GetTexture();
	D3D11_TEXTURE2D_DESC RawTextureDesc;
	sTextureDesc SpriteDesc;

	RawTexture->GetDesc(&RawTextureDesc);

	SpriteDesc.Dimensions.X = RawTextureDesc.Width;
	SpriteDesc.Dimensions.Y = RawTextureDesc.Height;
	SpriteDesc.Format = (Desc.Format);
	SpriteDesc.MipLevels = RawTextureDesc.MipLevels;

	{
		D3D11_BOX sourceRegion;
		sourceRegion.left = TargetBounds.has_value() ? (UINT)TargetBounds->Min.X : 0;
		sourceRegion.right = TargetBounds.has_value() ? (UINT)TargetBounds->Max.X : (UINT)SpriteDesc.Dimensions.X;
		sourceRegion.top = TargetBounds.has_value() ? (UINT)TargetBounds->Min.Y : 0;
		sourceRegion.bottom = TargetBounds.has_value() ? (UINT)TargetBounds->Max.Y : (UINT)SpriteDesc.Dimensions.Y;
		sourceRegion.front = 0;
		sourceRegion.back = 1;

		D3D11_BOX DSTRegion;
		DSTRegion.left = 0;
		DSTRegion.right = (UINT)Desc.Dimensions.X;
		DSTRegion.top = 0;
		DSTRegion.bottom = (UINT)Desc.Dimensions.Y;
		DSTRegion.front = 0;
		DSTRegion.back = 1;

		for (size_t i = 0; i < SpriteDesc.MipLevels; i++)
		{
			if (i >= Desc.MipLevels)
				break;

			Direct3DDeviceIMContext->CopySubresourceRegion1(pTexture.Get(), D3D11CalcSubresource((UINT)i, (UINT)ArrayIndex, (UINT)Desc.MipLevels), Dest.has_value() ? (UINT)Dest->X : 0, Dest.has_value() ? (UINT)Dest->Y : 0, 0, RawTexture, D3D11CalcSubresource(0, (UINT)SourceArrayIndex, (UINT)SpriteDesc.MipLevels), &sourceRegion, D3D11_COPY_DISCARD);

			DSTRegion.left = DSTRegion.left / 2;
			DSTRegion.top = DSTRegion.top / 2;

			DSTRegion.right = DSTRegion.right / 2;
			DSTRegion.bottom = DSTRegion.bottom / 2;

			sourceRegion.left = sourceRegion.left / 2;
			sourceRegion.top = sourceRegion.top / 2;

			sourceRegion.right = sourceRegion.right / 2;
			sourceRegion.bottom = sourceRegion.bottom / 2;

			if ((sourceRegion.right - sourceRegion.left) > (DSTRegion.right - DSTRegion.left) || (sourceRegion.bottom - sourceRegion.top) > (DSTRegion.bottom - DSTRegion.top))
			{
				break;
			}
		}

		HRESULT hr = DirectX::SaveWICTextureToFile(Direct3DDeviceIMContext, pTexture.Get(),
			GUID_ContainerFormatJpeg, (L"..//Content//Output_Textures//" + FileManager::StringToWstring(Name) + L".jpg").c_str());

		RawTexture = nullptr;
	}
}

void D3D11Texture::UpdateTexture(const std::wstring FilePath, std::size_t ArrayIndex, const std::optional<IntVector2> Dest, const std::optional<FBounds2D> TargetBounds)
{
	ID3D11Device1* Direct3DDevice = Owner->GetDevice();
	ID3D11DeviceContext1* Direct3DDeviceIMContext = Owner->GetDeviceIMContext();

	std::wstring ws(FilePath);
	std::string str = FileManager::WideStringToString(ws);
	std::string Ext = std::string(str.end() - 4, str.end());

	ComPtr<ID3D11Texture2D> RawTexture;
	D3D11_TEXTURE2D_DESC RawTextureDesc;
	sTextureDesc SpriteDesc;

	{
		ComPtr<ID3D11Resource> tempRes;
		ComPtr<ID3D11ShaderResourceView> SRV;
		UINT BindFlags = D3D11_BIND_SHADER_RESOURCE;
		UINT MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
		size_t MS = 0;
		HRESULT HR;

		bool DDS = false;
		if (Ext.find("DDS") != std::string::npos || Ext.find("dds") != std::string::npos)
			DDS = true;

		if (!DDS)
		{
			UINT LoadFlags = DirectX::WIC_LOADER_FLAGS_Internal::WIC_LOADER_DEFAULT;
			HR = DirectX::CreateWICTextureFromFileEx_Internal(Direct3DDevice, Direct3DDeviceIMContext, FilePath.c_str(), MS, D3D11_USAGE_DEFAULT, BindFlags, 0, MiscFlags, LoadFlags, &tempRes, &SRV);
		}
		else
		{
			HR = DirectX::CreateDDSTextureFromFileEx_Internal(Direct3DDevice, Direct3DDeviceIMContext, FilePath.c_str(), MS, D3D11_USAGE_DEFAULT, BindFlags, D3D11_CPU_ACCESS_READ, MiscFlags, false, &tempRes, &SRV);
		}

		if (HR == S_OK) {
			tempRes->QueryInterface(__uuidof (ID3D11Texture2D), (void**)&RawTexture);
			RawTexture->GetDesc(&RawTextureDesc);

			SpriteDesc.Dimensions.X = RawTextureDesc.Width;
			SpriteDesc.Dimensions.Y = RawTextureDesc.Height;
			SpriteDesc.Format = (Desc.Format);
			SpriteDesc.MipLevels = RawTextureDesc.MipLevels;
		}
		tempRes = nullptr;
		SRV = nullptr;
	}

	{
		D3D11_BOX sourceRegion;
		sourceRegion.left = TargetBounds.has_value() ? (UINT)TargetBounds->Min.X : 0;
		sourceRegion.right = TargetBounds.has_value() ? (UINT)TargetBounds->Max.X : (UINT)SpriteDesc.Dimensions.X;
		sourceRegion.top = TargetBounds.has_value() ? (UINT)TargetBounds->Min.Y : 0;
		sourceRegion.bottom = TargetBounds.has_value() ? (UINT)TargetBounds->Max.Y : (UINT)SpriteDesc.Dimensions.Y;
		sourceRegion.front = 0;
		sourceRegion.back = 1;

		D3D11_BOX DSTRegion;
		DSTRegion.left = 0;
		DSTRegion.right = (UINT)Desc.Dimensions.X;
		DSTRegion.top = 0;
		DSTRegion.bottom = (UINT)Desc.Dimensions.Y;
		DSTRegion.front = 0;
		DSTRegion.back = 1;

		for (size_t i = 0; i < SpriteDesc.MipLevels; i++)
		{
			if (i >= Desc.MipLevels)
				break;

			Direct3DDeviceIMContext->CopySubresourceRegion1(pTexture.Get(), D3D11CalcSubresource((UINT)i, (UINT)ArrayIndex, (UINT)Desc.MipLevels), Dest.has_value() ? (UINT)Dest->X : 0, Dest.has_value() ? (UINT)Dest->Y : 0, 0, RawTexture.Get(), D3D11CalcSubresource(0, 0, (UINT)SpriteDesc.MipLevels), &sourceRegion, D3D11_COPY_DISCARD);

			DSTRegion.left = DSTRegion.left / 2;
			DSTRegion.top = DSTRegion.top / 2;

			DSTRegion.right = DSTRegion.right / 2;
			DSTRegion.bottom = DSTRegion.bottom / 2;

			sourceRegion.left = sourceRegion.left / 2;
			sourceRegion.top = sourceRegion.top / 2;

			sourceRegion.right = sourceRegion.right / 2;
			sourceRegion.bottom = sourceRegion.bottom / 2;

			if ((sourceRegion.right - sourceRegion.left) > (DSTRegion.right - DSTRegion.left) || (sourceRegion.bottom - sourceRegion.top) > (DSTRegion.bottom - DSTRegion.top))
			{
				break;
			}
		}

		//HRESULT hr = DirectX::SaveWICTextureToFile(Direct3DDeviceIMContext, TiledTextureRes.Get(),
		//	GUID_ContainerFormatJpeg, (L"..//Content//" + FileManager::StringToWstring("stagingTexture" + std::to_string(TiledTextureSize)) + L".jpg").c_str());

		RawTexture = nullptr;
	}
}

void D3D11Texture::UpdateTexture(const void* pSrcData, const std::size_t InSize, const FDimension2D& Dimension, std::size_t ArrayIndex, const std::optional<IntVector2> Dest, const std::optional<FBounds2D> TargetBounds)
{
	ComPtr<ID3D11Texture2D> RawTexture;

	D3D11_TEXTURE2D_DESC sTextureDesc;
	ZeroMemory(&sTextureDesc, sizeof(sTextureDesc));
	sTextureDesc.Width = (UINT)Dimension.Width;
	sTextureDesc.Height = (UINT)Dimension.Height;
	sTextureDesc.MipLevels = Desc.MipLevels;
	sTextureDesc.ArraySize = 1;
	sTextureDesc.Format = ConvertFormat_Format_To_DXGI(Desc.Format);
	sTextureDesc.SampleDesc.Count = 1;
	sTextureDesc.SampleDesc.Quality = 0;
	sTextureDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA Data;
	Data.pSysMem = pSrcData;
	Data.SysMemPitch = sTextureDesc.Width * D3D11_BytesPerPixel(ConvertFormat_Format_To_DXGI(Desc.Format));
	Data.SysMemSlicePitch = Data.SysMemPitch * sTextureDesc.Height;

	Owner->GetDevice()->CreateTexture2D(&sTextureDesc, &Data, RawTexture.GetAddressOf());
	{
		RawTexture->GetDesc(&sTextureDesc);

		ID3D11DeviceContext1* Direct3DDeviceIMContext = Owner->GetDeviceIMContext();

		std::size_t Slice = ArrayIndex;

		D3D11_BOX sourceRegion;
		sourceRegion.left = TargetBounds.has_value() ? (UINT)TargetBounds->Min.X : 0;
		sourceRegion.right = TargetBounds.has_value() ? (UINT)TargetBounds->Max.X : (UINT)sTextureDesc.Width;
		sourceRegion.top = TargetBounds.has_value() ? (UINT)TargetBounds->Min.Y : 0;
		sourceRegion.bottom = TargetBounds.has_value() ? (UINT)TargetBounds->Max.Y : (UINT)sTextureDesc.Height;
		sourceRegion.front = 0;
		sourceRegion.back = 1;

		D3D11_BOX DSTRegion;
		DSTRegion.left = 0;
		DSTRegion.right = (UINT)Desc.Dimensions.X;
		DSTRegion.top = 0;
		DSTRegion.bottom = (UINT)Desc.Dimensions.Y;
		DSTRegion.front = 0;
		DSTRegion.back = 1;

		for (size_t i = 0; i < sTextureDesc.MipLevels; i++)
		{
			if (i >= Desc.MipLevels)
				break;

			Direct3DDeviceIMContext->CopySubresourceRegion1(pTexture.Get(), D3D11CalcSubresource((UINT)i, (UINT)Slice, (UINT)Desc.MipLevels), Dest.has_value() ? (UINT)Dest->X : 0, Dest.has_value() ? (UINT)Dest->Y : 0, 0, RawTexture.Get(), D3D11CalcSubresource(0, 0, (UINT)sTextureDesc.MipLevels), &sourceRegion, D3D11_COPY_NO_OVERWRITE);

			DSTRegion.left = DSTRegion.left / 2;
			DSTRegion.top = DSTRegion.top / 2;

			DSTRegion.right = DSTRegion.right / 2;
			DSTRegion.bottom = DSTRegion.bottom / 2;

			sourceRegion.left = sourceRegion.left / 2;
			sourceRegion.top = sourceRegion.top / 2;

			sourceRegion.right = sourceRegion.right / 2;
			sourceRegion.bottom = sourceRegion.bottom / 2;

			if ((sourceRegion.right - sourceRegion.left) > (DSTRegion.right - DSTRegion.left) || (sourceRegion.bottom - sourceRegion.top) > (DSTRegion.bottom - DSTRegion.top))
			{
				break;
			}
		}

		//HRESULT hr = DirectX::SaveWICTextureToFile(Direct3DDeviceIMContext, TiledTextureRes.Get(),
		//	GUID_ContainerFormatJpeg, (L"..//Content//" + FileManager::StringToWstring("stagingTexture" + std::to_string(TiledTextureSize)) + L".jpg").c_str());

		RawTexture = nullptr;
	}
}

void D3D11Texture::UpdateTexture(const void* pSrcData, std::size_t RowPitch, std::size_t MinX, std::size_t MinY, std::size_t MaxX, std::size_t MaxY, IGraphicsCommandContext* InCommandBuffer)
{
	D3D11_BOX box{};
	box.left = (std::uint32_t)MinX;
	box.right = (std::uint32_t)MaxX;
	box.top = (std::uint32_t)MinY;
	box.bottom = (std::uint32_t)MaxY;
	box.front = 0;
	box.back = 1;

    ID3D11DeviceContext1* CMD = InCommandBuffer ? static_cast<D3D11CommandBuffer*>(InCommandBuffer)->Get() : Owner->GetDeviceIMContext();
    CMD->UpdateSubresource(pTexture.Get(), 0, &box, pSrcData, (std::uint32_t)RowPitch * D3D11_BytesPerPixel(ConvertFormat_Format_To_DXGI(Desc.Format)), 0);
    CMD = nullptr;
}

void D3D11Texture::ApplyTexture(ID3D11DeviceContext1* CMD, std::uint32_t Location, eShaderType InType)
{
	switch (InType)
	{
	case eShaderType::Vertex:
		CMD->VSSetShaderResources(Location, 1, pTextureSRV.GetAddressOf());
		break;
	case eShaderType::Pixel:
		CMD->PSSetShaderResources(Location, 1, pTextureSRV.GetAddressOf());
		break;
	case eShaderType::Geometry:
		CMD->GSSetShaderResources(Location, 1, pTextureSRV.GetAddressOf());
		break;
	case eShaderType::Compute:
		CMD->CSSetShaderResources(Location, 1, pTextureSRV.GetAddressOf());
		break;
	case eShaderType::HULL:
		CMD->HSSetShaderResources(Location, 1, pTextureSRV.GetAddressOf());
		break;
	case eShaderType::Domain:
		CMD->DSSetShaderResources(Location, 1, pTextureSRV.GetAddressOf());
		break;
	//case eShaderType::Amplification:
	//	break;
	//case eShaderType::Mesh:
	//	break;
	}
}

void D3D11Texture::SaveToFile(std::wstring InPath) const
{
	//HRESULT hr = DirectX::SaveWICTextureToFile(Owner->GetDeviceIMContext(), pTexture.Get(),
	//	GUID_ContainerFormatPng, (InPath + FileManager::StringToWstring(Name) + L".png").c_str());
	HRESULT hr = DirectX::SaveDDSTextureToFile(Owner->GetDeviceIMContext(), pTexture.Get(), (InPath + FileManager::StringToWstring(Name) + L".dds").c_str());
}
