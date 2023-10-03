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
#include "D3D12Texture.h"
#include "WICTextureLoader12.h"
#include "DDSTextureLoader12.h"
#include "D3D12Device.h"
#include "Utilities/FileManager.h"
#include <iostream>
#include "D3D12ComputeCommandContext.h"

static std::uint32_t BytesPerPixel(DXGI_FORMAT Format)
{
    return (std::uint32_t)DXGI_BitsPerPixel(Format) / 8;
}

D3D12Texture::D3D12Texture(D3D12Device* InOwner, const std::wstring FilePath, const std::string InName, std::uint32_t InRootParameterIndex)
    : Owner(InOwner)
    , Name(InName)
    , SRV(D3D12DescriptorHandle(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV))
    , RootParameterIndex(InRootParameterIndex)
    , Path(FilePath)
    , CurrentState(D3D12_RESOURCE_STATE_COMMON)
{
    auto WideStringToString = [](const std::wstring & s) -> std::string
    {
        int len;
        int slength = (int)s.length() + 1;
        len = WideCharToMultiByte(0, 0, s.c_str(), slength, 0, 0, 0, 0);
        std::string r(len, '\0');
        WideCharToMultiByte(0, 0, s.c_str(), slength, &r[0], len, 0, 0);
        return r;
    };

    std::unique_ptr<uint8_t[]> decodedData;

    auto Device = Owner->GetDevice();
    auto IMCommandList = Owner->GetIMCommandList();

    std::string str = WideStringToString(FilePath);
    std::string Ext = std::string(str.end() - 4, str.end());

    bool DDS = false;
    if (Ext.find("DDS") != std::string::npos || Ext.find("dds") != std::string::npos)
        DDS = true;

    if (DDS)
    {
        std::vector<D3D12_SUBRESOURCE_DATA> subresources;
        ThrowIfFailed(
            DirectX::LoadDDSTextureFromFile(Device, FilePath.c_str(), Texture.ReleaseAndGetAddressOf(),
                decodedData, subresources));

        const UINT64 uploadBufferSize = GetRequiredIntermediateSize(Texture.Get(), 0,
            static_cast<UINT>(subresources.size()));

        // Create the GPU upload buffer.
        CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);

        auto desc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

        ThrowIfFailed(
            Device->CreateCommittedResource(
                &heapProps,
                D3D12_HEAP_FLAG_NONE,
                &desc,
                D3D12_RESOURCE_STATE_COMMON, // D3D12_RESOURCE_STATE_GENERIC_READ
                nullptr,
                IID_PPV_ARGS(uploadRes.GetAddressOf())));
;
        IMCommandList->BeginRecordCommandList();

        UpdateSubresources(IMCommandList->Get(), Texture.Get(), uploadRes.Get(),
            0, 0, static_cast<UINT>(subresources.size()), subresources.data());

        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(Texture.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST, CurrentState);
        IMCommandList->ResourceBarrier(1, &barrier);

        IMCommandList->FinishRecordCommandList();
        IMCommandList->ExecuteCommandList();
    }
    else
    {
        D3D12_SUBRESOURCE_DATA subresource;
        ThrowIfFailed(
            DirectX::LoadWICTextureFromFile(Device, FilePath.c_str(), Texture.ReleaseAndGetAddressOf(),
                decodedData, subresource));

        const UINT64 uploadBufferSize = GetRequiredIntermediateSize(Texture.Get(), 0, 1);

        CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);

        auto desc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

        // Create the GPU upload buffer.
        ThrowIfFailed(
            Device->CreateCommittedResource(
                &heapProps,
                D3D12_HEAP_FLAG_NONE,
                &desc,
                D3D12_RESOURCE_STATE_COMMON, // D3D12_RESOURCE_STATE_GENERIC_READ
                nullptr,
                IID_PPV_ARGS(uploadRes.GetAddressOf())));

        IMCommandList->BeginRecordCommandList();

        UpdateSubresources(IMCommandList->Get(), Texture.Get(), uploadRes.Get(),
            0, 0, 1, &subresource);

        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(Texture.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST, CurrentState);
        IMCommandList->ResourceBarrier(1, &barrier);

        IMCommandList->FinishRecordCommandList();
        IMCommandList->ExecuteCommandList();
    }

    Owner->AllocateDescriptor(&SRV);
    CreateShaderResourceView(Device, Texture.Get(), SRV.GetCPU(), false);

#if _DEBUG
    Texture->SetName(FileManager::StringToWstring(Name).c_str());
    uploadRes->SetName(FileManager::StringToWstring(Name + "_UploadBuffer").c_str());
#endif

    const auto texdesc = Texture->GetDesc();
    Desc.ArraySize = texdesc.DepthOrArraySize;
    Desc.Dimensions.X = (std::uint32_t)texdesc.Width;
    Desc.Dimensions.Y = (std::uint32_t)texdesc.Height;
    Desc.Format = ConvertFormat_DXGI_To_Format(texdesc.Format);
    Desc.MipLevels = texdesc.MipLevels;
}

D3D12Texture::D3D12Texture(D3D12Device* InOwner, const std::string InName, void* InData, const std::size_t InSize, const sTextureDesc& InDesc, std::uint32_t InRootParameterIndex)
    : Owner(InOwner)
    , Name(InName)
    , Desc(InDesc)
    , SRV(D3D12DescriptorHandle(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV))
    , RootParameterIndex(InRootParameterIndex)
    , Path(L"")
    , CurrentState(D3D12_RESOURCE_STATE_COMMON)
{
    auto Device = Owner->GetDevice();
    auto IMCommandList = Owner->GetIMCommandList();

    D3D12_SUBRESOURCE_DATA subresource;
    subresource.pData = InData;
    subresource.RowPitch = InDesc.Dimensions.X * BytesPerPixel(ConvertFormat_Format_To_DXGI(InDesc.Format));
    subresource.SlicePitch = subresource.RowPitch * InDesc.Dimensions.Y;

    // Describe and create a Texture2D.
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = InDesc.MipLevels;
    textureDesc.Format = ConvertFormat_Format_To_DXGI(InDesc.Format);
    textureDesc.Width = InDesc.Dimensions.X;
    textureDesc.Height = InDesc.Dimensions.Y;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    textureDesc.DepthOrArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    auto TextureHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_CUSTOM);
    TextureHeap.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
    TextureHeap.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;

    ThrowIfFailed(Device->CreateCommittedResource(
        &TextureHeap,
        D3D12_HEAP_FLAG_NONE,
        &textureDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&Texture)));

    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(Texture.Get(), 0, 1);

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);

    auto desc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

    // Create the GPU upload buffer.
    ThrowIfFailed(
        Device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_COMMON, // D3D12_RESOURCE_STATE_GENERIC_READ
            nullptr,
            IID_PPV_ARGS(uploadRes.GetAddressOf())));

    IMCommandList->BeginRecordCommandList();

    UpdateSubresources(IMCommandList->Get(), Texture.Get(), uploadRes.Get(),
        0, 0, 1, &subresource);

    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(Texture.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST, CurrentState);
    IMCommandList->ResourceBarrier(1, &barrier);

    IMCommandList->FinishRecordCommandList();
    IMCommandList->ExecuteCommandList();

    Owner->AllocateDescriptor(&SRV);
    CreateShaderResourceView(Device, Texture.Get(), SRV.GetCPU(), false);

#if _DEBUG
    Texture->SetName(FileManager::StringToWstring(Name).c_str());
    uploadRes->SetName(FileManager::StringToWstring(Name + "_UploadBuffer").c_str());
#endif

    const auto texdesc = Texture->GetDesc();
    Desc.ArraySize = texdesc.DepthOrArraySize;
    Desc.Dimensions.X = (std::uint32_t)texdesc.Width;
    Desc.Dimensions.Y = (std::uint32_t)texdesc.Height;
    Desc.Format = ConvertFormat_DXGI_To_Format(texdesc.Format);
    Desc.MipLevels = texdesc.MipLevels;
}

D3D12Texture::D3D12Texture(D3D12Device* InOwner, const std::string InName, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
    : Owner(InOwner)
    , Name(InName)
    , Desc(InDesc)
    , SRV(D3D12DescriptorHandle(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV))
    , RootParameterIndex(DefaultRootParameterIndex)
    , Path(L"")
    , CurrentState(D3D12_RESOURCE_STATE_COMMON)
{
    auto Device = Owner->GetDevice();
    auto IMCommandList = Owner->GetIMCommandList();

    // Describe and create a Texture2D.
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = InDesc.MipLevels;
    textureDesc.Format = ConvertFormat_Format_To_DXGI(InDesc.Format);
    textureDesc.Width = InDesc.Dimensions.X;
    textureDesc.Height = InDesc.Dimensions.Y;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    textureDesc.DepthOrArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    auto TextureHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_CUSTOM);
    TextureHeap.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
    TextureHeap.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;

    ThrowIfFailed(Device->CreateCommittedResource(
        &TextureHeap,
        D3D12_HEAP_FLAG_NONE,
        &textureDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&Texture)));

    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(Texture.Get(), 0, 1);

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);

    auto desc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

    // Create the GPU upload buffer.
    ThrowIfFailed(
        Device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_COMMON, // D3D12_RESOURCE_STATE_GENERIC_READ
            nullptr,
            IID_PPV_ARGS(uploadRes.GetAddressOf())));

    Owner->AllocateDescriptor(&SRV);
    CreateShaderResourceView(Device, Texture.Get(), SRV.GetCPU(), false);

#if _DEBUG
    Texture->SetName(FileManager::StringToWstring(Name).c_str());
    uploadRes->SetName(FileManager::StringToWstring(Name + "_UploadBuffer").c_str());
#endif

    const auto texdesc = Texture->GetDesc();
    Desc.ArraySize = texdesc.DepthOrArraySize;
    Desc.Dimensions.X = (std::uint32_t)texdesc.Width;
    Desc.Dimensions.Y = (std::uint32_t)texdesc.Height;
    Desc.Format = ConvertFormat_DXGI_To_Format(texdesc.Format);
    Desc.MipLevels = texdesc.MipLevels;
}

D3D12Texture::~D3D12Texture()
{
    Owner = nullptr;
    Texture = nullptr;
    uploadRes = nullptr;
}

void D3D12Texture::ApplyTexture(ID3D12GraphicsCommandList* CommandList)
{
    if (CurrentState != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
    {
        D3D12_RESOURCE_BARRIER Barriers[1];
        Barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(Texture.Get(), CurrentState, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        CommandList->ResourceBarrier(ARRAYSIZE(Barriers), Barriers);
        CurrentState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    }

    if (Texture)
        CommandList->SetGraphicsRootDescriptorTable(RootParameterIndex, SRV.GetGPU());
}

void D3D12Texture::ApplyTexture(ID3D12GraphicsCommandList* CommandList, std::uint32_t inRootParameterIndex)
{
    if (CurrentState != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
    {
        D3D12_RESOURCE_BARRIER Barriers[1];
        Barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(Texture.Get(), CurrentState, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        CommandList->ResourceBarrier(ARRAYSIZE(Barriers), Barriers);
        CurrentState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    }

    if (Texture)
        CommandList->SetGraphicsRootDescriptorTable(inRootParameterIndex, SRV.GetGPU());
}

void D3D12Texture::UpdateTexture(ITexture2D* SourceTexture, std::size_t SourceArrayIndex, std::size_t ArrayIndex, const std::optional<IntVector2> Dest, const std::optional<FBounds2D> TargetBounds)
{
    auto IMCommandList = Owner->GetIMCommandList();

    D3D12Texture* RawSrcTexture = static_cast<D3D12Texture*>(SourceTexture);
    ID3D12Resource* RawTexture = static_cast<D3D12Texture*>(SourceTexture)->Texture.Get();

    D3D12_RESOURCE_DESC RawTextureDesc = RawTexture->GetDesc();
    sTextureDesc SpriteDesc;

    SpriteDesc.Dimensions.X = (std::uint32_t)RawTextureDesc.Width;
    SpriteDesc.Dimensions.Y = (std::uint32_t)RawTextureDesc.Height;
    SpriteDesc.Format = (Desc.Format);
    SpriteDesc.MipLevels = RawTextureDesc.MipLevels;

    D3D12_BOX sourceRegion;
    sourceRegion.left = TargetBounds.has_value() ? (UINT)TargetBounds->Min.X : 0;
    sourceRegion.right = TargetBounds.has_value() ? (UINT)TargetBounds->Max.X : (UINT)SpriteDesc.Dimensions.X;
    sourceRegion.top = TargetBounds.has_value() ? (UINT)TargetBounds->Min.Y : 0;
    sourceRegion.bottom = TargetBounds.has_value() ? (UINT)TargetBounds->Max.Y : (UINT)SpriteDesc.Dimensions.Y;
    sourceRegion.front = 0;
    sourceRegion.back = 1;

    D3D12_BOX DSTRegion;
    DSTRegion.left = 0;
    DSTRegion.right = (UINT)Desc.Dimensions.X;
    DSTRegion.top = 0;
    DSTRegion.bottom = (UINT)Desc.Dimensions.Y;
    DSTRegion.front = 0;
    DSTRegion.back = 1;

    CD3DX12_TEXTURE_COPY_LOCATION SRCfp(RawTexture, D3D12CalcSubresource(0, (UINT)SourceArrayIndex, 0, (UINT)SpriteDesc.MipLevels, RawTextureDesc.DepthOrArraySize));

    IMCommandList->BeginRecordCommandList();

    std::vector<CD3DX12_RESOURCE_BARRIER> preCopyBarriers;
    preCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(Texture.Get(), CurrentState, D3D12_RESOURCE_STATE_COPY_DEST));
    preCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(RawTexture, RawSrcTexture->CurrentState, D3D12_RESOURCE_STATE_COPY_SOURCE));
    IMCommandList->ResourceBarrier((UINT)preCopyBarriers.size(), preCopyBarriers.data());

    //CurrentState = D3D12_RESOURCE_STATE_COPY_DEST;
    //RawSrcTexture->CurrentState = D3D12_RESOURCE_STATE_COPY_SOURCE;

    for (size_t i = 0; i < SpriteDesc.MipLevels; i++)
    {
        if (i >= Desc.MipLevels)
            break;

        CD3DX12_TEXTURE_COPY_LOCATION DSTfp(Texture.Get(), D3D12CalcSubresource((UINT)i, (UINT)SourceArrayIndex, 0, (UINT)SpriteDesc.MipLevels, RawTextureDesc.DepthOrArraySize));

        IMCommandList->Get()->CopyTextureRegion(&DSTfp, Dest.has_value() ? (UINT)Dest->X : 0, Dest.has_value() ? (UINT)Dest->Y : 0, 0, &SRCfp, &sourceRegion);

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

    std::vector<CD3DX12_RESOURCE_BARRIER> postCopyBarriers;
    postCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(Texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, CurrentState));
    postCopyBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(RawTexture, D3D12_RESOURCE_STATE_COPY_SOURCE, RawSrcTexture->CurrentState));
    IMCommandList->ResourceBarrier((UINT)postCopyBarriers.size(), postCopyBarriers.data());

    //CurrentState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    //RawSrcTexture->CurrentState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

    //HRESULT hr = DirectX::SaveWICTextureToFile(Direct3DDeviceIMContext, TiledTextureRes.Get(),
    //	GUID_ContainerFormatJpeg, (L"..//Content//Terrain (16x16).png//" + FileManager::StringToWstring("stagingTexture" + std::to_string(TiledTextureSize)) + L".jpg").c_str());

    RawTexture = nullptr;

    IMCommandList->FinishRecordCommandList();
    IMCommandList->ExecuteCommandList();
}

void D3D12Texture::UpdateTexture(const std::wstring FilePath, std::size_t ArrayIndex, const std::optional<IntVector2> Dest, const std::optional<FBounds2D> TargetBounds)
{
    std::cout << "D3D12Texture::UpdateTexture_Unimplemented(FilePath, ArrayIndex, Dest, TargetBounds)" << std::endl;

    /*
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
    */
}

void D3D12Texture::UpdateTexture(const void* pSrcData, const std::size_t InSize, const FDimension2D& Dimension, std::size_t ArrayIndex, const std::optional<IntVector2>  Dest, const std::optional<FBounds2D> TargetBounds)
{
    std::cout << "D3D12Texture::UpdateTexture_Unimplemented(pSrcData, InSize, Dimension, ArrayIndex, Dest, TargetBounds)" << std::endl;

    /*
    ComPtr<ID3D11Texture2D> RawTexture;

	D3D11_TEXTURE2D_DESC sTextureDesc;
	ZeroMemory(&sTextureDesc, sizeof(sTextureDesc));
	sTextureDesc.Width = Dimension.Width;
	sTextureDesc.Height = Dimension.Height;
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
    */
}

void D3D12Texture::UpdateTexture(const void* pSrcData, std::size_t RowPitch, std::size_t MinX, std::size_t MinY, std::size_t MaxX, std::size_t MaxY, IGraphicsCommandContext* InCommandBuffer)
{
    //auto IMCommandList = Owner->GetIMCommandList()->Get();

    //Owner->GetIMCommandList()->BeginRecordCommandList();

    D3D12_BOX box{};
    box.left = (std::uint32_t)MinX;
    box.right = (std::uint32_t)MaxX;
    box.top = (std::uint32_t)MinY;
    box.bottom = (std::uint32_t)MaxY;
    box.front = 0;
    box.back = 1;

   /* bool bBarrier = false;
    if (CurrentState != D3D12_RESOURCE_STATE_COMMON)
    {
        bBarrier = true;
        D3D12_RESOURCE_BARRIER preCopyBarriers = CD3DX12_RESOURCE_BARRIER::Transition(Texture.Get(), CurrentState, D3D12_RESOURCE_STATE_COMMON);
        IMCommandList->ResourceBarrier(1, &preCopyBarriers);
    }*/

    CD3DX12_RANGE readRange(0, 0);
    Texture->Map(0, &readRange, nullptr);
    Texture->WriteToSubresource(0, &box, pSrcData, (std::uint32_t)RowPitch * BytesPerPixel(ConvertFormat_Format_To_DXGI(Desc.Format)), 0);
    Texture->Unmap(0, nullptr);

   /* if (bBarrier)
    {
        D3D12_RESOURCE_BARRIER postCopyBarriers = CD3DX12_RESOURCE_BARRIER::Transition(Texture.Get(), D3D12_RESOURCE_STATE_COMMON, CurrentState);
        IMCommandList->ResourceBarrier(1, &postCopyBarriers);
    }

    Owner->GetIMCommandList()->FinishRecordCommandList();
    Owner->GetIMCommandList()->ExecuteCommandList();*/
}

void D3D12Texture::CreateShaderResourceView(ID3D12Device* device, ID3D12Resource* tex, D3D12_CPU_DESCRIPTOR_HANDLE srvDescriptor, bool isCubeMap)
{
    const auto desc = tex->GetDesc();

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = desc.Format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    switch (desc.Dimension)
    {
    case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
        if (desc.DepthOrArraySize > 1)
        {
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
            srvDesc.Texture1DArray.MipLevels = (!desc.MipLevels) ? UINT(-1) : desc.MipLevels;
            srvDesc.Texture1DArray.ArraySize = static_cast<UINT>(desc.DepthOrArraySize);
        }
        else
        {
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
            srvDesc.Texture1D.MipLevels = (!desc.MipLevels) ? UINT(-1) : desc.MipLevels;
        }
        break;

    case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
        if (isCubeMap)
        {
            if (desc.DepthOrArraySize > 6)
            {
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
                srvDesc.TextureCubeArray.MipLevels = (!desc.MipLevels) ? UINT(-1) : desc.MipLevels;
                srvDesc.TextureCubeArray.NumCubes = static_cast<UINT>(desc.DepthOrArraySize / 6);
            }
            else
            {
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                srvDesc.TextureCube.MipLevels = (!desc.MipLevels) ? UINT(-1) : desc.MipLevels;
            }
        }
        else if (desc.DepthOrArraySize > 1)
        {
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
            srvDesc.Texture2DArray.MipLevels = (!desc.MipLevels) ? UINT(-1) : desc.MipLevels;
            srvDesc.Texture2DArray.ArraySize = static_cast<UINT>(desc.DepthOrArraySize);
        }
        else
        {
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = (!desc.MipLevels) ? UINT(-1) : desc.MipLevels;
        }
        break;

    case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
        srvDesc.Texture3D.MipLevels = (!desc.MipLevels) ? UINT(-1) : desc.MipLevels;
        break;

    case D3D12_RESOURCE_DIMENSION_BUFFER:
    case D3D12_RESOURCE_DIMENSION_UNKNOWN:
    default:
        throw std::invalid_argument("resource dimension not supported");
    }

    device->CreateShaderResourceView(tex, &srvDesc, srvDescriptor);
}
