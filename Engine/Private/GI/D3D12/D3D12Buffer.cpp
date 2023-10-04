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
#include "D3D12Buffer.h"
#include "D3D12CommandBuffer.h"
#include "D3D12Viewport.h"
#include "Utilities/FileManager.h"
#include "GI/D3DShared/D3DShared.h"

D3D12UploadBuffer::D3D12UploadBuffer(ID3D12Device* Device, std::string InName, std::uint32_t InSize)
    : Name(InName)
    , Size(InSize)
    , pData(nullptr)
    , bIsMapped(false)
{
    auto HeapDesc = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto Desc = CD3DX12_RESOURCE_DESC::Buffer(Size);

    ThrowIfFailed(Device->CreateCommittedResource(
        &HeapDesc,
        D3D12_HEAP_FLAG_NONE,
        &Desc,
        D3D12_RESOURCE_STATE_COMMON, // D3D12_RESOURCE_STATE_GENERIC_READ
        nullptr,
        IID_PPV_ARGS(&Buffer)));

    ZeroMemory(&pData, sizeof(pData));

#if _DEBUG
    Buffer->SetName(FileManager::StringToWstring(Name).c_str());
#endif
}

D3D12UploadBuffer::~D3D12UploadBuffer()
{
    Unmap();
    Buffer = nullptr;
}

void D3D12UploadBuffer::UpdateSubresource(ID3D12Resource* InResource, D3D12_RESOURCE_STATES State, sBufferSubresource* Subresource, D3D12CommandBuffer* InCMDBuffer)
{
    static_cast<D3D12CommandBuffer*>(InCMDBuffer)->UpdateSubresource(InResource, State, this, Subresource);
}

void D3D12UploadBuffer::Map(const void* Ptr)
{
    if (!bIsMapped)
    {
        CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
        ThrowIfFailed(Buffer->Map(0, &readRange, &pData));
        bIsMapped = true;
        if (Ptr)
            memcpy(pData, Ptr, Size);
    }
    else
    {
        if (Ptr)
            memcpy(pData, Ptr, Size);
    }
}

void D3D12UploadBuffer::Unmap()
{
    if (bIsMapped)
    {
        bIsMapped = false;
        Buffer->Unmap(0, nullptr);
    }
}

D3D12ConstantBuffer::D3D12ConstantBuffer(D3D12Device* InOwner, std::string InName, const sBufferDesc& InDesc, std::uint32_t InRootParameterIndex)
    : Super()
    , Name(InName)
    , BufferDesc(InDesc)
    , Owner(InOwner)
    , ViewHeap(D3D12DescriptorHandle(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV))
    , RootParameterIndex(InRootParameterIndex)
{

    ID3D12Device* m_device = Owner->GetDevice();

    Owner->AllocateDescriptor(&ViewHeap);

    const UINT constantBufferSize = (BufferDesc.Size + 255) & ~255;    // CB size is required to be 256-byte aligned.

    UploadBuffer = D3D12UploadBuffer::CreateUnique(Owner->Get(), Name + "_UploadBuffer", constantBufferSize/*BufferDesc.Size*/);
    UploadBuffer->Map();

    // Describe and create a constant buffer view.
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = UploadBuffer->GetGPU();
    cbvDesc.SizeInBytes = constantBufferSize;
    m_device->CreateConstantBufferView(&cbvDesc, ViewHeap.GetCPU());
}

D3D12ConstantBuffer::~D3D12ConstantBuffer()
{
    Unmap();
    Owner = nullptr;
    UploadBuffer = nullptr;
}

std::size_t D3D12ConstantBuffer::GetSize() const
{
    return BufferDesc.Size;
}

void D3D12ConstantBuffer::ApplyConstantBuffer(ID3D12GraphicsCommandList* CommandList)
{
    CommandList->SetGraphicsRootDescriptorTable(RootParameterIndex, ViewHeap.GetGPU());
}

void D3D12ConstantBuffer::ApplyConstantBuffer(ID3D12GraphicsCommandList* CommandList, std::uint32_t InRootParameterIndex)
{
    CommandList->SetGraphicsRootDescriptorTable(InRootParameterIndex, ViewHeap.GetGPU());
}

void D3D12ConstantBuffer::Map(const void* Data, IGraphicsCommandContext* InCMDBuffer)
{
    UploadBuffer->Map(Data);
}

void D3D12ConstantBuffer::Unmap()
{
    UploadBuffer->Unmap();
}

D3D12VertexBuffer::D3D12VertexBuffer(D3D12Device* InOwner, std::string InName, const sBufferDesc& InDesc, sBufferSubresource* Subresource)
    : BufferDesc(InDesc)
    , Owner(InOwner)
    , Name(InName)
    , CurrentState(D3D12_RESOURCE_STATE_COMMON) // D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
{
    ID3D12Device* m_device = Owner->GetDevice();
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
    auto desc = CD3DX12_RESOURCE_DESC::Buffer(BufferDesc.Size);
    HRESULT hr = m_device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&m_pResource));

    UploadBuffer = D3D12UploadBuffer::CreateUnique(Owner->Get(), Name + "_UploadBuffer", (std::uint32_t)BufferDesc.Size);
    UploadBuffer->Map();

#if _DEBUG
    m_pResource->SetName(FileManager::StringToWstring(Name).c_str());
#endif

    // Initialize the vertex buffer view.
    m_vertexBufferView.BufferLocation = m_pResource->GetGPUVirtualAddress();
    m_vertexBufferView.StrideInBytes = (UINT)BufferDesc.Stride;
    m_vertexBufferView.SizeInBytes = (UINT)BufferDesc.Size;

    UpdateSubresource(Subresource);
}

std::size_t D3D12VertexBuffer::GetSize() const
{
    return (std::size_t)BufferDesc.Size;
}

D3D12VertexBuffer::~D3D12VertexBuffer()
{
    m_pResource = nullptr;
    Owner = nullptr;
    UploadBuffer = nullptr;
}

void D3D12VertexBuffer::ApplyBuffer(ID3D12GraphicsCommandList* CommandList)
{
    if (CurrentState != D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
    {
        D3D12_RESOURCE_BARRIER Barriers[1];
        Barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(m_pResource.Get(), CurrentState, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
        CommandList->ResourceBarrier(ARRAYSIZE(Barriers), Barriers);
        CurrentState = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    }

    CommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
}

void D3D12VertexBuffer::ResizeBuffer(std::size_t Size, sBufferSubresource* InSubresource)
{
    BufferDesc.Size = Size;

    UploadBuffer->Unmap();
    UploadBuffer = nullptr;

    ID3D12Device* m_device = Owner->GetDevice();
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
    auto desc = CD3DX12_RESOURCE_DESC::Buffer(BufferDesc.Size);
    HRESULT hr = m_device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&m_pResource));

    UploadBuffer = D3D12UploadBuffer::CreateUnique(Owner->Get(), Name + "_UploadBuffer", (std::uint32_t)BufferDesc.Size);
    UploadBuffer->Map();

    m_vertexBufferView.SizeInBytes = (UINT)BufferDesc.Size;

#if _DEBUG
    m_pResource->SetName(FileManager::StringToWstring(Name).c_str());
#endif

    UpdateSubresource(InSubresource);
}

void D3D12VertexBuffer::UpdateSubresource(sBufferSubresource* Subresource, IGraphicsCommandContext* InCMDBuffer)
{
    if (!Subresource)
        return;

    if (InCMDBuffer)
    {
        static_cast<D3D12CommandBuffer*>(InCMDBuffer)->UpdateSubresource(m_pResource.Get(), CurrentState, UploadBuffer.get(), Subresource);
    }
    else
    {
        Owner->GetIMCommandList()->BeginRecordCommandList();
        Owner->GetIMCommandList()->UpdateSubresource(m_pResource.Get(), CurrentState, UploadBuffer.get(), Subresource);
        Owner->GetIMCommandList()->FinishRecordCommandList();
        Owner->GetIMCommandList()->ExecuteCommandList();
    }
}

D3D12IndexBuffer::D3D12IndexBuffer(D3D12Device* InOwner, std::string InName, const sBufferDesc& InDesc, sBufferSubresource* Subresource)
    : BufferDesc(InDesc)
    , Owner(InOwner)
    , Name(InName)
    , CurrentState(D3D12_RESOURCE_STATE_COMMON) // D3D12_RESOURCE_STATE_INDEX_BUFFER
{
    ID3D12Device* m_device = Owner->GetDevice();
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
    auto desc = CD3DX12_RESOURCE_DESC::Buffer(BufferDesc.Size);
    ThrowIfFailed(m_device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&m_pResource)));


    UploadBuffer = D3D12UploadBuffer::CreateUnique(Owner->Get(), Name + "_UploadBuffer", (std::uint32_t)BufferDesc.Size);
    UploadBuffer->Map();

#if _DEBUG
    m_pResource->SetName(FileManager::StringToWstring(Name).c_str());
#endif

    // Initialize the vertex buffer view.
    IBView.BufferLocation = m_pResource->GetGPUVirtualAddress();
    IBView.Format = DXGI_FORMAT_R32_UINT;
    IBView.SizeInBytes = (UINT)BufferDesc.Size;

    UpdateSubresource(Subresource);
}

D3D12IndexBuffer::~D3D12IndexBuffer()
{
    m_pResource = nullptr;
    UploadBuffer = nullptr;
    Owner = nullptr;;
}

std::size_t D3D12IndexBuffer::GetSize() const
{
    return (std::size_t)BufferDesc.Size;
}

void D3D12IndexBuffer::ApplyBuffer(ID3D12GraphicsCommandList* CommandList)
{
    if (CurrentState != D3D12_RESOURCE_STATE_INDEX_BUFFER)
    {
        D3D12_RESOURCE_BARRIER Barriers[1];
        Barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(m_pResource.Get(), CurrentState, D3D12_RESOURCE_STATE_INDEX_BUFFER);
        CommandList->ResourceBarrier(ARRAYSIZE(Barriers), Barriers);
        CurrentState = D3D12_RESOURCE_STATE_INDEX_BUFFER;
    }

    CommandList->IASetIndexBuffer(&IBView);
}

void D3D12IndexBuffer::ResizeBuffer(std::size_t Size, sBufferSubresource* InSubresource)
{
    BufferDesc.Size = Size;

    UploadBuffer->Unmap();
    UploadBuffer = nullptr;

    ID3D12Device* m_device = Owner->GetDevice();
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
    auto desc = CD3DX12_RESOURCE_DESC::Buffer(BufferDesc.Size);
    ThrowIfFailed(m_device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&m_pResource)));


    UploadBuffer = D3D12UploadBuffer::CreateUnique(Owner->Get(), Name + "_UploadBuffer", (std::uint32_t)BufferDesc.Size);
    UploadBuffer->Map();

#if _DEBUG
    m_pResource->SetName(FileManager::StringToWstring(Name).c_str());
#endif

    IBView.SizeInBytes = (UINT)BufferDesc.Size;

    UpdateSubresource(InSubresource);
}

void D3D12IndexBuffer::UpdateSubresource(sBufferSubresource* Subresource, IGraphicsCommandContext* InCMDBuffer)
{
    if (InCMDBuffer)
    {
        static_cast<D3D12CommandBuffer*>(InCMDBuffer)->UpdateSubresource(m_pResource.Get(), CurrentState, UploadBuffer.get(), Subresource);
    }
    else
    {
        Owner->GetIMCommandList()->BeginRecordCommandList();
        Owner->GetIMCommandList()->UpdateSubresource(m_pResource.Get(), CurrentState, UploadBuffer.get(), Subresource);
        Owner->GetIMCommandList()->FinishRecordCommandList();
        Owner->GetIMCommandList()->ExecuteCommandList();
    }
}
