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
#include "D3D11Buffer.h"
#include "D3D11CommandBuffer.h"

D3D11Buffer::D3D11Buffer(D3D11Device* InDevice, const sBufferDesc& InDesc, sBufferSubresource* InSubresource, ResourceTypeFlags InType, bool InDynamic)
	: Owner(InDevice)
	, Buffer(nullptr)
{
	auto BindFlag = [&](ResourceTypeFlags InAType) -> std::uint32_t
	{
		switch (InAType)
		{
		case D3D11Buffer::ResourceTypeFlags::eDEFAULT:
			return NULL;
			break;
		case D3D11Buffer::ResourceTypeFlags::eVERTEX_BUFFER:
			return D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
			break;
		case D3D11Buffer::ResourceTypeFlags::eINDEX_BUFFER:
			return D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
			break;
		case D3D11Buffer::ResourceTypeFlags::eCONSTANT_BUFFER:
			return D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
			break;
		case D3D11Buffer::ResourceTypeFlags::eUnorderedAccess_BUFFER:
			return /*D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS |*/ D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
			break;
		}
		return NULL;
	};

	BufferDesc = InDesc;

	D3D11_SUBRESOURCE_DATA SUBRESOURCE = { 0 };
	if (InSubresource)
		SUBRESOURCE.pSysMem = InSubresource->pSysMem;

	D3D11_BUFFER_DESC DESC = { 0 };
	DESC.ByteWidth = static_cast<std::uint32_t>(BufferDesc.Size);
	DESC.StructureByteStride = static_cast<std::uint32_t>(BufferDesc.Stride);
	DESC.BindFlags = BindFlag(InType);
	DESC.Usage = InDynamic ? D3D11_USAGE::D3D11_USAGE_DYNAMIC : D3D11_USAGE::D3D11_USAGE_DEFAULT;
	DESC.CPUAccessFlags = InDynamic ? D3D11_CPU_ACCESS_WRITE : NULL;

	if (InType == ResourceTypeFlags::eCONSTANT_BUFFER || InType == ResourceTypeFlags::eUnorderedAccess_BUFFER)
	{
		DESC.Usage = D3D11_USAGE_DYNAMIC;
		DESC.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		if (InType == ResourceTypeFlags::eUnorderedAccess_BUFFER)
		{
			DESC.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		}
	}

	ID3D11Device1* d3dDevice = Owner->GetDevice();
	d3dDevice->CreateBuffer(&DESC, InSubresource ? &SUBRESOURCE : NULL, &Buffer);
}

bool D3D11Buffer::IsMapable() const
{
	D3D11_BUFFER_DESC DESC = { 0 };
	Buffer->GetDesc(&DESC);
	return DESC.Usage == D3D11_USAGE::D3D11_USAGE_DYNAMIC && DESC.CPUAccessFlags == D3D11_CPU_ACCESS_WRITE;
}

void D3D11Buffer::ResizeBuffer(std::size_t Size, sBufferSubresource* InSubresource)
{
	Buffer = nullptr;

	BufferDesc.Size = Size;

	D3D11_SUBRESOURCE_DATA SUBRESOURCE = { 0 };
	if (InSubresource)
		SUBRESOURCE.pSysMem = InSubresource->pSysMem;

	D3D11_BUFFER_DESC DESC = { 0 };
	Buffer->GetDesc(&DESC);
	DESC.ByteWidth = static_cast<std::uint32_t>(BufferDesc.Size);

	ID3D11Device1* d3dDevice = Owner->GetDevice();
	d3dDevice->CreateBuffer(&DESC, InSubresource ? &SUBRESOURCE : NULL, &Buffer);
}

void D3D11ConstantBuffer::Map(const void* Ptr, IGraphicsCommandContext* InCMDBuffer)
{
	ID3D11DeviceContext1* CTX = InCMDBuffer ? static_cast<D3D11CommandBuffer*>(InCMDBuffer)->Get() : GetOwner()->GetDeviceIMContext();

	HRESULT HR;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	HR = CTX->Map(Buffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, Ptr, BufferDesc.Size);
	CTX->Unmap(Buffer.Get(), 0);
}

void D3D11ConstantBuffer::ApplyConstantBuffer(ID3D11DeviceContext1* CMD, std::uint32_t InSlot, eShaderType InType)
{
	switch (InType)
	{
	case eShaderType::Vertex:
		CMD->VSSetConstantBuffers(InSlot, 1, Buffer.GetAddressOf());
		break;
	case eShaderType::Pixel:
		CMD->PSSetConstantBuffers(InSlot, 1, Buffer.GetAddressOf());
		break;
	case eShaderType::Geometry:
		CMD->GSSetConstantBuffers(InSlot, 1, Buffer.GetAddressOf());
		break;
	case eShaderType::Compute:
		CMD->CSSetConstantBuffers(InSlot, 1, Buffer.GetAddressOf());
		break;
	case eShaderType::HULL:
		CMD->HSSetConstantBuffers(InSlot, 1, Buffer.GetAddressOf());
		break;
	case eShaderType::Domain:
		CMD->DSSetConstantBuffers(InSlot, 1, Buffer.GetAddressOf());
		break;
	}
}

void D3D11VertexBuffer::UpdateSubresource(sBufferSubresource* Subresource, IGraphicsCommandContext* InCMDBuffer)
{
	UpdateSubresource(Subresource, InCMDBuffer ? static_cast<D3D11CommandBuffer*>(InCMDBuffer)->Get() : GetOwner()->GetDeviceIMContext());
}

void D3D11VertexBuffer::UpdateSubresource(sBufferSubresource* Subresource, ID3D11DeviceContext1* Device)
{
	D3D11_BOX box{};
	box.left = (std::uint32_t)Subresource->Location;
	box.right = BufferDesc.Size < Subresource->Size ? (std::uint32_t)(Subresource->Location + BufferDesc.Size)
		: (std::uint32_t)(Subresource->Location + Subresource->Size);
	box.top = 0;
	box.bottom = 1;
	box.front = 0;
	box.back = 1;

	if (box.left >= box.right)
		return;

	Device->UpdateSubresource1(Buffer.Get(), 0, &box, Subresource->pSysMem, 0, 0, 0);
}

void D3D11VertexBuffer::ApplyBuffer(IGraphicsCommandContext* InCMDBuffer)
{
	if (InCMDBuffer)
	{
		const auto& CTX = static_cast<D3D11CommandBuffer*>(InCMDBuffer)->Get();
		std::uint32_t offset = NULL;
		std::uint32_t Stride = static_cast<std::uint32_t>(BufferDesc.Stride);
		CTX->IASetVertexBuffers(0, 1, Buffer.GetAddressOf(), &Stride, &offset);
	}
	else
	{
		std::uint32_t offset = NULL;
		std::uint32_t Stride = static_cast<std::uint32_t>(BufferDesc.Stride);
		GetOwner()->GetDeviceIMContext()->IASetVertexBuffers(0, 1, Buffer.GetAddressOf(), &Stride, &offset);
	}
}

void D3D11IndexBuffer::UpdateSubresource(sBufferSubresource* Subresource, IGraphicsCommandContext* InCMDBuffer)
{
	UpdateSubresource(Subresource, InCMDBuffer ? static_cast<D3D11CommandBuffer*>(InCMDBuffer)->Get() : GetOwner()->GetDeviceIMContext());
}

void D3D11IndexBuffer::UpdateSubresource(sBufferSubresource* Subresource, ID3D11DeviceContext1* Device)
{
	D3D11_BOX box{};
	box.left = (std::uint32_t)Subresource->Location;
	box.right = BufferDesc.Size < Subresource->Size ? (std::uint32_t)(Subresource->Location + BufferDesc.Size)
		: (std::uint32_t)(Subresource->Location + Subresource->Size);
	box.top = 0;
	box.bottom = 1;
	box.front = 0;
	box.back = 1;

	if (box.left >= box.right)
		return;

	Device->UpdateSubresource1(Buffer.Get(), 0, &box, Subresource->pSysMem, 0, 0, 0);
}

void D3D11IndexBuffer::ApplyBuffer(IGraphicsCommandContext* InCMDBuffer)
{
	if (InCMDBuffer)
	{
		const auto& CTX = static_cast<D3D11CommandBuffer*>(InCMDBuffer)->Get();
		std::uint32_t offset = NULL;
		CTX->IASetIndexBuffer(Buffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, offset);
	}
	else
	{
		std::uint32_t offset = NULL;
		GetOwner()->GetDeviceIMContext()->IASetIndexBuffer(Buffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, offset);
	}
}
