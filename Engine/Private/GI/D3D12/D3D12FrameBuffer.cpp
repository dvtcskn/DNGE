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
#include "D3D12FrameBuffer.h"
#include "GI/D3DShared/D3DShared.h"
#include "D3D12Device.h"
#include "Utilities/FileManager.h"

D3D12RenderTarget::D3D12RenderTarget(D3D12Device* InOwner, const std::string InName, const EFormat InFormat, const sFBODesc& Desc)
	: Super()
	, Name(InName)
	, Format(InFormat)
	, RTV(D3D12DescriptorHandle(D3D12_DESCRIPTOR_HEAP_TYPE_RTV))
	, SRV(D3D12DescriptorHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV))
	, CurrentState(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET)
{
	const DXGI_FORMAT DXGIFormat = ConvertFormat_Format_To_DXGI(Format);
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGIFormat, Desc.Dimensions.X, Desc.Dimensions.Y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

	const std::uint32_t mipLevel = -1;
	const std::uint32_t arraySize = -1;
	const std::uint32_t firstArraySlice = -1;

	{
		D3D12_CLEAR_VALUE clearValue = {};
		D3D12_RESOURCE_STATES states = D3D12_RESOURCE_STATE_COMMON;

		clearValue.Format = DXGIFormat;
		clearValue.Color[0] = 0.0f;
		clearValue.Color[1] = 0.0f;
		clearValue.Color[2] = 0.0f;
		clearValue.Color[3] = 0.0f;

		const FLOAT clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		if (clearColor)
		{
			clearValue.Color[0] = clearColor[0];
			clearValue.Color[1] = clearColor[1];
			clearValue.Color[2] = clearColor[2];
			clearValue.Color[3] = clearColor[3];
		}

		states |= D3D12_RESOURCE_STATE_RENDER_TARGET;
		
		auto HeapDesc = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		HRESULT hr = InOwner->Get()->CreateCommittedResource(
			&HeapDesc,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			states,
			&clearValue,
			IID_PPV_ARGS(&Texture));

		assert(hr == S_OK);

#if _DEBUG
		Texture->SetName(FileManager::StringToWstring(InName).c_str());
#endif
	}

	{
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};

		rtvDesc.Format = DXGIFormat;

		if (resourceDesc.DepthOrArraySize == 1)
		{
			if (resourceDesc.SampleDesc.Count == 1)
			{
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
				rtvDesc.Texture2D.MipSlice = (mipLevel == -1) ? 0 : mipLevel;
			}
			else
			{
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
			}
		}
		else
		{
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			rtvDesc.Texture2DArray.ArraySize = arraySize;
			rtvDesc.Texture2DArray.FirstArraySlice = firstArraySlice;
			rtvDesc.Texture2DArray.MipSlice = (mipLevel == -1) ? 0 : mipLevel;
		}

		{
			InOwner->AllocateDescriptor(&RTV);
			InOwner->Get()->CreateRenderTargetView(Texture.Get(), &rtvDesc, RTV.GetCPU());
		}
	}

	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

		{
			srvDesc.Format = resourceDesc.Format;

			if (resourceDesc.SampleDesc.Count == 1)
			{
				if (resourceDesc.DepthOrArraySize == 1)
				{
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

					srvDesc.Texture2D.MostDetailedMip = (mipLevel == -1) ? 0 : mipLevel;
					srvDesc.Texture2D.MipLevels = 1;
				}
				else
				{
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;

					srvDesc.Texture2DArray.MostDetailedMip = (mipLevel == -1) ? 0 : mipLevel;
					srvDesc.Texture2DArray.MipLevels = 1;

					srvDesc.Texture2DArray.FirstArraySlice = (firstArraySlice == -1) ? 0 : firstArraySlice;
					srvDesc.Texture2DArray.ArraySize = (arraySize == -1) ? resourceDesc.DepthOrArraySize : arraySize;
				}
			}
			else
			{
				if (resourceDesc.DepthOrArraySize == 1)
				{
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
				}
				else
				{
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
					srvDesc.Texture2DMSArray.FirstArraySlice = (firstArraySlice == -1) ? 0 : firstArraySlice;
					srvDesc.Texture2DMSArray.ArraySize = (arraySize == -1) ? resourceDesc.DepthOrArraySize : arraySize;
				}
			}
		}

		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		InOwner->AllocateDescriptor(&SRV);
		InOwner->Get()->CreateShaderResourceView(Texture.Get(), & srvDesc, SRV.GetCPU());
	}
}

D3D12RenderTarget::~D3D12RenderTarget()
{
	Texture = nullptr;
}

D3D12DepthTarget::D3D12DepthTarget(D3D12Device* InOwner, const std::string InName, const EFormat InFormat, const sFBODesc& Desc)
	: Super()
	, Name(InName)
	, Format(InFormat)
	, DSV(D3D12DescriptorHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV))
	, SRV(D3D12DescriptorHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV))
	, CurrentState(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE)
{
	bIsSRVSupported = IsDepthSRVSupported(Format);
	const DXGI_FORMAT DXGIFormat = ConvertFormat_Format_To_DXGI(Format);
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGIFormat, Desc.Dimensions.X, Desc.Dimensions.Y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
	//resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

	const std::uint32_t mipLevel = -1;
	const std::uint32_t arraySize = -1;
	const std::uint32_t firstArraySlice = -1;

	{
		D3D12_CLEAR_VALUE clearValue = {};
		D3D12_RESOURCE_STATES states = D3D12_RESOURCE_STATE_COMMON;

		if (IsValidDepthFormat(DXGIFormat))
		{
			clearValue.Format = GetDepthViewFormat(DXGIFormat);
			clearValue.DepthStencil.Depth = 0.0f;
			clearValue.DepthStencil.Stencil = 0;

			if (resourceDesc.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE)
				states |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
			states |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
		}
		else
		{
			throw std::runtime_error("Invalid Depth Format");
		}

		auto HeapDesc = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		HRESULT hr = InOwner->Get()->CreateCommittedResource(
			&HeapDesc,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			states,
			&clearValue,
			IID_PPV_ARGS(&Texture));

		assert(hr == S_OK);

#if _DEBUG
		Texture->SetName(FileManager::StringToWstring(InName).c_str());
#endif
	}

	{
		D3D12_DEPTH_STENCIL_VIEW_DESC DSViewDesc = {};
		DSViewDesc.Format = GetDepthViewFormat(DXGIFormat);
		if (resourceDesc.SampleDesc.Count == 1)
		{
			if (resourceDesc.DepthOrArraySize == 1)
			{
				DSViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
				DSViewDesc.Texture2D.MipSlice = 0;
			}
			else
			{
				DSViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
				DSViewDesc.Texture2DArray.MipSlice = 0;
				DSViewDesc.Texture2DArray.FirstArraySlice = firstArraySlice;
				DSViewDesc.Texture2DArray.ArraySize = arraySize;
			}
		}
		else
		{
			DSViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
		}

		InOwner->AllocateDescriptor(&DSV);
		InOwner->Get()->CreateDepthStencilView(Texture.Get(), &DSViewDesc, DSV.GetCPU());
	}

	{
		if (IsValidDepthFormat(resourceDesc.Format))
		{
			if (IsDepthSRVSupported(resourceDesc.Format))
			{
				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.Format = GetDepthViewFormat(resourceDesc.Format);

				if (resourceDesc.SampleDesc.Count == 1)
				{
					if (resourceDesc.DepthOrArraySize == 1)
					{
						srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

						srvDesc.Texture2D.MostDetailedMip = (mipLevel == -1) ? 0 : mipLevel;
						srvDesc.Texture2D.MipLevels = 1;
					}
					else
					{
						srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;

						srvDesc.Texture2DArray.MostDetailedMip = (mipLevel == -1) ? 0 : mipLevel;
						srvDesc.Texture2DArray.MipLevels = 1;

						srvDesc.Texture2DArray.FirstArraySlice = (firstArraySlice == -1) ? 0 : firstArraySlice;
						srvDesc.Texture2DArray.ArraySize = (arraySize == -1) ? resourceDesc.DepthOrArraySize : arraySize;
					}
				}
				else
				{
					if (resourceDesc.DepthOrArraySize == 1)
					{
						srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
					}
					else
					{
						srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
						srvDesc.Texture2DMSArray.FirstArraySlice = (firstArraySlice == -1) ? 0 : firstArraySlice;
						srvDesc.Texture2DMSArray.ArraySize = (arraySize == -1) ? resourceDesc.DepthOrArraySize : arraySize;
					}
				}

				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

				InOwner->AllocateDescriptor(&SRV);
				InOwner->Get()->CreateShaderResourceView(Texture.Get(), &srvDesc, SRV.GetCPU());
			}
		}
		else
		{
			throw std::runtime_error("Invalid Depth Format");
		}
	}
}

D3D12DepthTarget::~D3D12DepthTarget()
{
	Texture = nullptr;
}

D3D12UnorderedAccessTarget::D3D12UnorderedAccessTarget(D3D12Device* InOwner, const std::string InName, const EFormat InFormat, const sFBODesc& Desc, bool InEnableSRV)
	: Super()
	, Name(InName)
	, Format(InFormat)
	, UAV(D3D12DescriptorHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV))
	, SRV(D3D12DescriptorHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV))
	, CurrentState(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
	, bIsSRVSupported(InEnableSRV)
{
	const DXGI_FORMAT DXGIFormat = ConvertFormat_Format_To_DXGI(Format);
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGIFormat, Desc.Dimensions.X, Desc.Dimensions.Y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	const std::uint32_t mipLevel = -1;
	const std::uint32_t arraySize = -1;
	const std::uint32_t firstArraySlice = -1;

	{
		D3D12_RESOURCE_STATES states = D3D12_RESOURCE_STATE_COMMON;

		if (!bIsSRVSupported)
			states |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

		//states |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

		auto HeapDesc = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		HRESULT hr = InOwner->Get()->CreateCommittedResource(
			&HeapDesc,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			states,
			0,
			IID_PPV_ARGS(&Texture));

		assert(hr == S_OK);

#if _DEBUG
		Texture->SetName(FileManager::StringToWstring(InName).c_str());
#endif
	}

	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = resourceDesc.Format;

		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = (mipLevel == -1) ? 0 : mipLevel;

		InOwner->AllocateDescriptor(&UAV);
		InOwner->Get()->CreateUnorderedAccessView(Texture.Get(), NULL, & uavDesc, UAV.GetCPU());
	}

	if (bIsSRVSupported)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

		srvDesc.Format = resourceDesc.Format;

		if (resourceDesc.SampleDesc.Count == 1)
		{
			if (resourceDesc.DepthOrArraySize == 1)
			{
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

				srvDesc.Texture2D.MostDetailedMip = (mipLevel == -1) ? 0 : mipLevel;
				srvDesc.Texture2D.MipLevels = 1;
			}
			else
			{
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;

				srvDesc.Texture2DArray.MostDetailedMip = (mipLevel == -1) ? 0 : mipLevel;
				srvDesc.Texture2DArray.MipLevels = 1;

				srvDesc.Texture2DArray.FirstArraySlice = (firstArraySlice == -1) ? 0 : firstArraySlice;
				srvDesc.Texture2DArray.ArraySize = (arraySize == -1) ? resourceDesc.DepthOrArraySize : arraySize;
			}
		}
		else
		{
			if (resourceDesc.DepthOrArraySize == 1)
			{
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
			}
			else
			{
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
				srvDesc.Texture2DMSArray.FirstArraySlice = (firstArraySlice == -1) ? 0 : firstArraySlice;
				srvDesc.Texture2DMSArray.ArraySize = (arraySize == -1) ? resourceDesc.DepthOrArraySize : arraySize;
			}
		}

		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		InOwner->AllocateDescriptor(&SRV);
		InOwner->Get()->CreateShaderResourceView(Texture.Get(), &srvDesc, SRV.GetCPU());
	}
}

D3D12UnorderedAccessTarget::~D3D12UnorderedAccessTarget()
{
	Texture = nullptr;
}

D3D12FrameBuffer::D3D12FrameBuffer(D3D12Device* InOwner, std::string InName, const sFrameBufferAttachmentInfo& InAttachments)
	: Owner(InOwner)
	, AttachmentInfo(InAttachments)
	, Name(InName)
{
	const auto& FDesc = AttachmentInfo.Desc;

	for (std::size_t i = 0; i < AttachmentInfo.FrameBuffer.size(); i++)
	{
		const auto& FB = AttachmentInfo.FrameBuffer[i];

		if (FB.bIsUAV)
		{
			UnorderedAccessTargets.push_back(D3D12UnorderedAccessTarget::Create(Owner, Name + "_UAV_" + std::to_string(UnorderedAccessTargets.size()), FB.Format, FDesc, FB.bIsShaderResource));
		}
		else
		{
			RenderTargets.push_back(D3D12RenderTarget::Create(Owner, Name + "_RenderTarget_" + std::to_string(RenderTargets.size()), FB.Format, FDesc));
		}
	}

	if (IsValidDepthFormat(AttachmentInfo.DepthFormat))
	{
		DepthTarget = D3D12DepthTarget::Create(Owner, Name + "_DepthTarget", AttachmentInfo.DepthFormat, FDesc);
	}
}

void D3D12FrameBuffer::AttachRenderTarget(const IRenderTarget::SharedPtr& RenderTarget, std::optional<std::size_t> Index)
{
	if (auto RT = std::dynamic_pointer_cast<D3D12RenderTarget>(RenderTarget))
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

void D3D12FrameBuffer::AttachUnorderedAccessTarget(const IUnorderedAccessTarget::SharedPtr& UnorderedAccessTarget, std::optional<std::size_t> Index)
{
	if (auto ST = std::dynamic_pointer_cast<D3D12UnorderedAccessTarget>(UnorderedAccessTarget))
	{
		if (Index.has_value())
			UnorderedAccessTargets.insert(UnorderedAccessTargets.begin() + Index.value(), ST);
		else
			UnorderedAccessTargets.push_back(ST);

		if (Index.has_value())
			AttachmentInfo.FrameBuffer.insert(AttachmentInfo.FrameBuffer.begin() + Index.value(), sFrameBufferAttachmentInfo::sFrameBuffer(ST->GetFormat(), ST->IsSRVSupported(), true));
		else
			AttachmentInfo.AddFrameBuffer(ST->GetFormat(), ST->IsSRVSupported(), true);
	}
}

void D3D12FrameBuffer::SetDepthTarget(const IDepthTarget::SharedPtr& InDepthTarget)
{
	if (auto DT = std::dynamic_pointer_cast<D3D12DepthTarget>(InDepthTarget))
	{
		DepthTarget = nullptr;
		DepthTarget = DT;
		AttachmentInfo.DepthFormat = DepthTarget->GetFormat();
	}
}
