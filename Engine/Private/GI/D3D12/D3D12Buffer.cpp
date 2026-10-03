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

D3D12UploadBuffer::D3D12UploadBuffer(D3D12Device* InOwner, std::string InName, std::uint32_t InSize)
    : Name(InName)
    , Size(InSize)
    , pData(nullptr)
    , bIsMapped(false)
{
    ID3D12Device14* Device = InOwner->GetDevice();

    auto HeapDesc = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto Desc = CD3DX12_RESOURCE_DESC::Buffer(Size);

    const bool IsEnhancedBarriersSupported = InOwner->IsEnhancedBarriersSupported();
    if (IsEnhancedBarriersSupported)
    {
        auto Desc1 = CD3DX12_RESOURCE_DESC1(Desc);
        ThrowIfFailed(Device->CreateCommittedResource3(
            &HeapDesc,
            D3D12_HEAP_FLAG_NONE,
            &Desc1,
            D3D12_BARRIER_LAYOUT_UNDEFINED,
            nullptr,
            nullptr,
            0,
            nullptr,
            IID_PPV_ARGS(&Buffer)));
    }
    else
    {
        ThrowIfFailed(Device->CreateCommittedResource(
            &HeapDesc,
            D3D12_HEAP_FLAG_NONE,
            &Desc,
            D3D12_RESOURCE_STATE_COMMON, // D3D12_RESOURCE_STATE_GENERIC_READ
            nullptr,
            IID_PPV_ARGS(&Buffer)));
    }

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

void D3D12UploadBuffer::UpdateSubresource(ID3D12Resource* InResource, D3D12_RESOURCE_STATES State, BufferSubresource* Subresource, D3D12CommandBuffer* InCMDBuffer)
{
    static_cast<D3D12CommandBuffer*>(InCMDBuffer)->CopyBufferRegion(InResource, State, Buffer.Get(), pData, Subresource);
}

void D3D12UploadBuffer::Map(const void* Ptr)
{
    if (!bIsMapped)
    {
        CD3DX12_RANGE readRange(0, 0);
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

void D3D12UploadBuffer::Map(const void* Ptr, std::size_t Location, std::uint32_t Stride, IGraphicsCommandContext* InCMDBuffer)
{
    if (!bIsMapped)
    {
        CD3DX12_RANGE readRange(0, 0);
        ThrowIfFailed(Buffer->Map(0, &readRange, &pData));
        bIsMapped = true;
        if (Ptr)
            memcpy((BYTE*)pData + (Location * Stride), Ptr, Stride);
    }
    else
    {
        if (Ptr)
            memcpy((BYTE*)pData + (Location * Stride), Ptr, Stride);
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

D3D12ConstantBuffer::D3D12ConstantBuffer(D3D12Device* InOwner, std::string InName, const BufferLayout& InDesc, std::uint32_t InRootParameterIndex)
    : Super()
    , Name(InName)
    , BufferDesc(InDesc)
    , Owner(InOwner)
    , ViewHeap(D3D12DescriptorHandle(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV))
    , RootParameterIndex(InRootParameterIndex)
{
    ID3D12Device* Device = Owner->GetDevice();

    Owner->AllocateDescriptor(&ViewHeap);

    const UINT constantBufferSize = (BufferDesc.Size + 255) & ~255;    // CB size is required to be 256-byte aligned.

    UploadBuffer = D3D12UploadBuffer::CreateUnique(Owner, Name + "_UploadBuffer", constantBufferSize/*BufferDesc.Size*/);
    UploadBuffer->Map();

    // Describe and create a constant buffer view.
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = UploadBuffer->GetGPU();
    cbvDesc.SizeInBytes = constantBufferSize;
    Device->CreateConstantBufferView(&cbvDesc, ViewHeap.GetCPU());
}

D3D12ConstantBuffer::~D3D12ConstantBuffer()
{
    Unmap();
    Owner = nullptr;
    UploadBuffer = nullptr;
}

std::uint32_t D3D12ConstantBuffer::GetBindlessIndex() const
{
	return ViewHeap.GetHeapIndex();
}

std::size_t D3D12ConstantBuffer::GetSize() const
{
    return BufferDesc.Size;
}

void D3D12ConstantBuffer::ApplyConstantBuffer(ID3D12GraphicsCommandList7* CommandList)
{
    //CommandList->SetGraphicsRootDescriptorTable(RootParameterIndex, ViewHeap.GetGPU());
    CommandList->SetGraphicsRootConstantBufferView(RootParameterIndex, UploadBuffer->GetGPU());
}

void D3D12ConstantBuffer::ApplyConstantBuffer(ID3D12GraphicsCommandList7* CommandList, std::uint32_t InRootParameterIndex)
{
    //CommandList->SetGraphicsRootDescriptorTable(InRootParameterIndex, ViewHeap.GetGPU());
    CommandList->SetGraphicsRootConstantBufferView(RootParameterIndex, UploadBuffer->GetGPU());
}

void D3D12ConstantBuffer::Map(const void* Data, IGraphicsCommandContext* InCMDBuffer)
{
    UploadBuffer->Map(Data);
}

void D3D12ConstantBuffer::Unmap()
{
    UploadBuffer->Unmap();
}

D3D12VertexBuffer::D3D12VertexBuffer(D3D12Device* InOwner, std::string InName, const BufferLayout& InDesc, BufferSubresource* Subresource)
    : BufferDesc(InDesc)
    , Owner(InOwner)
    , Name(InName)
    , CurrentState(D3D12_RESOURCE_STATE_COMMON) // D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
    , CurrentBarrierLayout(D3D12_BARRIER_LAYOUT_UNDEFINED)
    , CurrentSyncState(D3D12_BARRIER_SYNC_NONE)
    , CurrentAccessState(D3D12_BARRIER_ACCESS_NO_ACCESS)
{
    ID3D12Device14* Device = Owner->GetDevice();

    CD3DX12_HEAP_PROPERTIES HeapDesc(D3D12_HEAP_TYPE_DEFAULT);
    auto Desc = CD3DX12_RESOURCE_DESC::Buffer(BufferDesc.Size);

    const bool IsEnhancedBarriersSupported = Owner->IsEnhancedBarriersSupported();
    if (IsEnhancedBarriersSupported)
    {
        auto Desc1 = CD3DX12_RESOURCE_DESC1(Desc);
        ThrowIfFailed(Device->CreateCommittedResource3(
            &HeapDesc,
            D3D12_HEAP_FLAG_NONE,
            &Desc1,
            D3D12_BARRIER_LAYOUT_UNDEFINED,
            nullptr,
            nullptr,
            0,
            nullptr,
            IID_PPV_ARGS(&VertexBuffer)));
    }
    else
    {
        HRESULT hr = Device->CreateCommittedResource(
            &HeapDesc,
            D3D12_HEAP_FLAG_NONE,
            &Desc,
            CurrentState,
            nullptr,
            IID_PPV_ARGS(&VertexBuffer));
    }

    UploadBuffer = D3D12UploadBuffer::CreateUnique(Owner, Name + "_UploadBuffer", (std::uint32_t)BufferDesc.Size);
    UploadBuffer->Map();

#if _DEBUG
    VertexBuffer->SetName(FileManager::StringToWstring(Name).c_str());
#endif

    // Initialize the vertex buffer view.
    VertexBufferView.BufferLocation = VertexBuffer->GetGPUVirtualAddress();
    VertexBufferView.StrideInBytes = (UINT)BufferDesc.Stride;
    VertexBufferView.SizeInBytes = (UINT)BufferDesc.Size;

    UpdateSubresource(Subresource);
}

std::size_t D3D12VertexBuffer::GetSize() const
{
    return (std::size_t)BufferDesc.Size;
}

D3D12VertexBuffer::~D3D12VertexBuffer()
{
    VertexBuffer = nullptr;
    Owner = nullptr;
    UploadBuffer = nullptr;
}

void D3D12VertexBuffer::ApplyBuffer(ID3D12GraphicsCommandList7* CommandList, std::uint32_t Slot)
{
    const bool IsEnhancedBarriersSupported = Owner->IsEnhancedBarriersSupported();
    if (IsEnhancedBarriersSupported)
    {
        if (CurrentSyncState != D3D12_BARRIER_SYNC_VERTEX_SHADING && CurrentAccessState != D3D12_BARRIER_ACCESS_VERTEX_BUFFER)
        {
            D3D12_BUFFER_BARRIER VertexBufBarriers[] =
            {
                CD3DX12_BUFFER_BARRIER(
                    D3D12_BARRIER_SYNC_COPY,            // SyncBefore
                    CurrentSyncState,                          // SyncAfter        // D3D12_BARRIER_SYNC_VERTEX_SHADING
                    D3D12_BARRIER_ACCESS_COPY_DEST,     // AccessBefore
                    CurrentAccessState,                        // AccessAfter      // D3D12_BARRIER_ACCESS_VERTEX_BUFFER
                    VertexBuffer.Get()
                )
            };
            D3D12_BARRIER_GROUP VertexBufBarrierGroups[] = { CD3DX12_BARRIER_GROUP(_countof(VertexBufBarriers), VertexBufBarriers) };
            CurrentSyncState = D3D12_BARRIER_SYNC_VERTEX_SHADING;
            CurrentAccessState = D3D12_BARRIER_ACCESS_VERTEX_BUFFER;
            CommandList->Barrier(_countof(VertexBufBarrierGroups), VertexBufBarrierGroups);
        }
    }
    else
    {
        if (CurrentState != D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
        {
            D3D12_RESOURCE_BARRIER Barriers[1];
            Barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(VertexBuffer.Get(), CurrentState, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
            CommandList->ResourceBarrier(ARRAYSIZE(Barriers), Barriers);
            CurrentState = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        }
    }

    CommandList->IASetVertexBuffers(Slot, 1, &VertexBufferView);
}

void D3D12VertexBuffer::ResizeBuffer(std::size_t Size, BufferSubresource* InSubresource)
{
    BufferDesc.Size = Size;

    UploadBuffer->Unmap();
    UploadBuffer = nullptr;

    ID3D12Device14* Device = Owner->GetDevice();
    CD3DX12_HEAP_PROPERTIES HeapDesc(D3D12_HEAP_TYPE_DEFAULT);
    auto Desc = CD3DX12_RESOURCE_DESC::Buffer(BufferDesc.Size);

    const bool IsEnhancedBarriersSupported = Owner->IsEnhancedBarriersSupported();
    if (IsEnhancedBarriersSupported)
    {
        auto Desc1 = CD3DX12_RESOURCE_DESC1(Desc);
        ThrowIfFailed(Device->CreateCommittedResource3(
            &HeapDesc,
            D3D12_HEAP_FLAG_NONE,
            &Desc1,
            D3D12_BARRIER_LAYOUT_UNDEFINED,
            nullptr,
            nullptr,
            0,
            nullptr,
            IID_PPV_ARGS(&VertexBuffer)));
    }
    else
    {
        CurrentState = D3D12_RESOURCE_STATE_COMMON;
        HRESULT hr = Device->CreateCommittedResource(
            &HeapDesc,
            D3D12_HEAP_FLAG_NONE,
            &Desc,
            CurrentState,
            nullptr,
            IID_PPV_ARGS(&VertexBuffer));
    }

    UploadBuffer = D3D12UploadBuffer::CreateUnique(Owner, Name + "_UploadBuffer", (std::uint32_t)BufferDesc.Size);
    UploadBuffer->Map();

    VertexBufferView.SizeInBytes = (UINT)BufferDesc.Size;

#if _DEBUG
    VertexBuffer->SetName(FileManager::StringToWstring(Name).c_str());
#endif

    UpdateSubresource(InSubresource);
}

void D3D12VertexBuffer::UpdateSubresource(BufferSubresource* Subresource, IGraphicsCommandContext* InCMDBuffer)
{
    if (!Subresource)
        return;

    if (InCMDBuffer)
    {
        static_cast<D3D12CommandBuffer*>(InCMDBuffer)->CopyBufferRegion(VertexBuffer.Get(), CurrentState, UploadBuffer->Get(), UploadBuffer->GetData(), Subresource);
    }
    else
    {
        Owner->GetIMCommandList()->BeginRecordCommandList();
        Owner->GetIMCommandList()->CopyBufferRegion(VertexBuffer.Get(), CurrentState, UploadBuffer->Get(), UploadBuffer->GetData(), Subresource);
        Owner->GetIMCommandList()->FinishRecordCommandList();
        Owner->GetIMCommandList()->ExecuteCommandList();
    }
}

D3D12IndexBuffer::D3D12IndexBuffer(D3D12Device* InOwner, std::string InName, const BufferLayout& InDesc, BufferSubresource* Subresource)
    : BufferDesc(InDesc)
    , Owner(InOwner)
    , Name(InName)
    , CurrentState(D3D12_RESOURCE_STATE_COMMON) // D3D12_RESOURCE_STATE_INDEX_BUFFER
    , CurrentBarrierLayout(D3D12_BARRIER_LAYOUT_UNDEFINED)
    , CurrentSyncState(D3D12_BARRIER_SYNC_NONE)
    , CurrentAccessState(D3D12_BARRIER_ACCESS_NO_ACCESS)
{
    ID3D12Device14* Device = Owner->GetDevice();

    CD3DX12_HEAP_PROPERTIES HeapDesc(D3D12_HEAP_TYPE_DEFAULT);
    auto Desc = CD3DX12_RESOURCE_DESC::Buffer(BufferDesc.Size);

    const bool IsEnhancedBarriersSupported = Owner->IsEnhancedBarriersSupported();
    if (IsEnhancedBarriersSupported)
    {
        auto Desc1 = CD3DX12_RESOURCE_DESC1(Desc);
        ThrowIfFailed(Device->CreateCommittedResource3(
            &HeapDesc,
            D3D12_HEAP_FLAG_NONE,
            &Desc1,
            D3D12_BARRIER_LAYOUT_UNDEFINED,
            nullptr,
            nullptr,
            0,
            nullptr,
            IID_PPV_ARGS(&IndexBuffer)));
    }
    else
    {
        ThrowIfFailed(Device->CreateCommittedResource(
            &HeapDesc,
            D3D12_HEAP_FLAG_NONE,
            &Desc,
            CurrentState,
            nullptr,
            IID_PPV_ARGS(&IndexBuffer)));
    }

    UploadBuffer = D3D12UploadBuffer::CreateUnique(Owner, Name + "_UploadBuffer", (std::uint32_t)BufferDesc.Size);
    UploadBuffer->Map();

#if _DEBUG
    IndexBuffer->SetName(FileManager::StringToWstring(Name).c_str());
#endif

    // Initialize the vertex buffer view.
    IndexBufferView.BufferLocation = IndexBuffer->GetGPUVirtualAddress();
    IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
    IndexBufferView.SizeInBytes = (UINT)BufferDesc.Size;

    UpdateSubresource(Subresource);
}

D3D12IndexBuffer::~D3D12IndexBuffer()
{
    IndexBuffer = nullptr;
    UploadBuffer = nullptr;
    Owner = nullptr;
}

std::size_t D3D12IndexBuffer::GetSize() const
{
    return (std::size_t)BufferDesc.Size;
}

void D3D12IndexBuffer::ApplyBuffer(ID3D12GraphicsCommandList7* CommandList)
{
    const bool IsEnhancedBarriersSupported = Owner->IsEnhancedBarriersSupported();
    if (IsEnhancedBarriersSupported)
    {
        if (CurrentSyncState != D3D12_BARRIER_SYNC_INDEX_INPUT && CurrentAccessState != D3D12_BARRIER_ACCESS_INDEX_BUFFER)
        {
            D3D12_BUFFER_BARRIER BufBarriers[] =
            {
                CD3DX12_BUFFER_BARRIER(
                    CurrentSyncState,                               // SyncBefore        // D3D12_BARRIER_SYNC_COPY
                    D3D12_BARRIER_SYNC_INDEX_INPUT,          // SyncAfter
                    CurrentAccessState,                             // AccessBefore      // D3D12_BARRIER_ACCESS_COPY_DEST
                    D3D12_BARRIER_ACCESS_INDEX_BUFFER,       // AccessAfter
                    IndexBuffer.Get()
                )
            };
            D3D12_BARRIER_GROUP BufBarrierGroups[] = { CD3DX12_BARRIER_GROUP(_countof(BufBarriers), BufBarriers) };
            CurrentSyncState = D3D12_BARRIER_SYNC_INDEX_INPUT;
            CurrentAccessState = D3D12_BARRIER_ACCESS_INDEX_BUFFER;
            CommandList->Barrier(_countof(BufBarrierGroups), BufBarrierGroups);
        }
    }
    else
    {
        if (CurrentState != D3D12_RESOURCE_STATE_INDEX_BUFFER)
        {
            D3D12_RESOURCE_BARRIER Barriers[1];
            Barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(IndexBuffer.Get(), CurrentState, D3D12_RESOURCE_STATE_INDEX_BUFFER);
            CommandList->ResourceBarrier(ARRAYSIZE(Barriers), Barriers);
            CurrentState = D3D12_RESOURCE_STATE_INDEX_BUFFER;
        }
    }

    CommandList->IASetIndexBuffer(&IndexBufferView);
}

void D3D12IndexBuffer::ResizeBuffer(std::size_t Size, BufferSubresource* InSubresource)
{
    BufferDesc.Size = Size;

    UploadBuffer->Unmap();
    UploadBuffer = nullptr;

    ID3D12Device14* Device = Owner->GetDevice();
    CD3DX12_HEAP_PROPERTIES HeapDesc(D3D12_HEAP_TYPE_DEFAULT);
    auto Desc = CD3DX12_RESOURCE_DESC::Buffer(BufferDesc.Size);

    const bool IsEnhancedBarriersSupported = Owner->IsEnhancedBarriersSupported();
    if (IsEnhancedBarriersSupported)
    {
        auto Desc1 = CD3DX12_RESOURCE_DESC1(Desc);
        ThrowIfFailed(Device->CreateCommittedResource3(
            &HeapDesc,
            D3D12_HEAP_FLAG_NONE,
            &Desc1,
            D3D12_BARRIER_LAYOUT_UNDEFINED,
            nullptr,
            nullptr,
            0,
            nullptr,
            IID_PPV_ARGS(&IndexBuffer)));
    }
    else
    {
        CurrentState = D3D12_RESOURCE_STATE_COMMON;
        ThrowIfFailed(Device->CreateCommittedResource(
            &HeapDesc,
            D3D12_HEAP_FLAG_NONE,
            &Desc,
            CurrentState,
            nullptr,
            IID_PPV_ARGS(&IndexBuffer)));
    }

    UploadBuffer = D3D12UploadBuffer::CreateUnique(Owner, Name + "_UploadBuffer", (std::uint32_t)BufferDesc.Size);
    UploadBuffer->Map();

#if _DEBUG
    IndexBuffer->SetName(FileManager::StringToWstring(Name).c_str());
#endif

    IndexBufferView.SizeInBytes = (UINT)BufferDesc.Size;

    UpdateSubresource(InSubresource);
}

void D3D12IndexBuffer::UpdateSubresource(BufferSubresource* Subresource, IGraphicsCommandContext* InCMDBuffer)
{
    if (InCMDBuffer)
    {
        static_cast<D3D12CommandBuffer*>(InCMDBuffer)->CopyBufferRegion(IndexBuffer.Get(), CurrentState, UploadBuffer->Get(), UploadBuffer->GetData(), Subresource);
    }
    else
    {
        Owner->GetIMCommandList()->BeginRecordCommandList();
        Owner->GetIMCommandList()->CopyBufferRegion(IndexBuffer.Get(), CurrentState, UploadBuffer->Get(), UploadBuffer->GetData(), Subresource);
        Owner->GetIMCommandList()->FinishRecordCommandList();
        Owner->GetIMCommandList()->ExecuteCommandList();
    }
}

D3D12IndirectBuffer::D3D12IndirectBuffer(D3D12Device* InDevice, std::string InName, BufferLayout NewLayout)
    : Super()
    , Name(InName)
    , Layout(NewLayout)
    , Offset(0)
    , CommandSize(0)
    , Buffer(nullptr)
    , pData(nullptr)
    , bIsMapped(false)
{
    ID3D12Device14* Device = InDevice->GetDevice();

    auto HeapDesc = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto Desc = CD3DX12_RESOURCE_DESC::Buffer(NewLayout.Stride * NewLayout.Size);

    const bool IsEnhancedBarriersSupported = InDevice->IsEnhancedBarriersSupported();
    if (IsEnhancedBarriersSupported)
    {
        auto Desc1 = CD3DX12_RESOURCE_DESC1(Desc);
        ThrowIfFailed(Device->CreateCommittedResource3(
            &HeapDesc,
            D3D12_HEAP_FLAG_NONE,
            &Desc1,
            D3D12_BARRIER_LAYOUT_UNDEFINED,
            nullptr,
            nullptr,
            0,
            nullptr,
            IID_PPV_ARGS(&Buffer)));
    }
    else
    {
        ThrowIfFailed(Device->CreateCommittedResource(
            &HeapDesc,
            D3D12_HEAP_FLAG_NONE,
            &Desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&Buffer)));
    }

    ZeroMemory(&pData, sizeof(pData));

#if _DEBUG
    Buffer->SetName(FileManager::StringToWstring("D3D12IndirectBuffer::" + Name).c_str());
#endif
}

D3D12IndirectBuffer::~D3D12IndirectBuffer()
{
    if (bIsMapped)
    {
        bIsMapped = false;
        Buffer->Unmap(0, nullptr);
    }
    Buffer = nullptr;
}

void D3D12IndirectBuffer::SetArgument(void* Argument, std::size_t NewCommandSize, std::size_t NewOffset)
{
    Offset = NewOffset;
    CommandSize = NewCommandSize;
    if (!bIsMapped)
    {
        CD3DX12_RANGE readRange(0, 0);
        ThrowIfFailed(Buffer->Map(0, &readRange, &pData));
        bIsMapped = true;
        if (Argument)
            memcpy((BYTE*)pData + Offset, Argument, Layout.Stride * CommandSize);
    }
    else
    {
        if (Argument)
            memcpy((BYTE*)pData + Offset, Argument, Layout.Stride * CommandSize);
    }
}

D3D12ByteAddressBuffer::D3D12ByteAddressBuffer(D3D12Device* InDevice, std::string InName, std::uint64_t NewSize, bool bReadWriteAllowed)
    : Super()
    , Name(InName)
    , Owner(InDevice)
    , Size(NewSize)
    , ViewHeap(D3D12DescriptorHandle(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV))
    , bIsReadWriteAllowed(bReadWriteAllowed)
    , pData(nullptr)
    , bIsMapped(false)
{
    ID3D12Device14* Device = Owner->GetDevice();
    Owner->AllocateDescriptor(&ViewHeap);

    auto HeapDesc = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD); // D3D12_HEAP_TYPE_DEFAULT
    auto Desc = CD3DX12_RESOURCE_DESC::Buffer(Size);
    if (bIsReadWriteAllowed)
        Desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    const bool IsEnhancedBarriersSupported = Owner->IsEnhancedBarriersSupported();
    if (IsEnhancedBarriersSupported)
    {
        auto Desc1 = CD3DX12_RESOURCE_DESC1(Desc);
        ThrowIfFailed(Device->CreateCommittedResource3(
            &HeapDesc,
            D3D12_HEAP_FLAG_NONE,
            &Desc1,
            D3D12_BARRIER_LAYOUT_UNDEFINED,
            nullptr,
            nullptr,
            0,
            nullptr,
            IID_PPV_ARGS(&Buffer)));
    }
    else
    {
        Device->CreateCommittedResource(
            &HeapDesc,
            D3D12_HEAP_FLAG_NONE,
            &Desc,
            D3D12_RESOURCE_STATE_COMMON/*D3D12_RESOURCE_STATE_COPY_DEST*/,
            nullptr,
            IID_PPV_ARGS(&Buffer));
    }

    if (bIsReadWriteAllowed)
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.FirstElement = 0;
        uavDesc.Buffer.NumElements = static_cast<UINT>(Size / 4);
        uavDesc.Buffer.StructureByteStride = 0;
        uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;

        Device->CreateUnorderedAccessView(Buffer.Get(), nullptr, &uavDesc, ViewHeap.GetCPU());
    }
    else
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Buffer.FirstElement = 0;
        srvDesc.Buffer.NumElements = static_cast<UINT>(Size / 4); // count of 32-bit words, not bytes
        srvDesc.Buffer.StructureByteStride = 0;   // must be 0 for raw — this isn't a structured buffer
        srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;

        Device->CreateShaderResourceView(Buffer.Get(), &srvDesc, ViewHeap.GetCPU());
    }
}

D3D12ByteAddressBuffer::~D3D12ByteAddressBuffer()
{
    Unmap();
    Buffer = nullptr;
    Owner = nullptr;
}

void D3D12ByteAddressBuffer::Map(const void* Ptr, std::size_t Location, std::uint32_t Stride, IGraphicsCommandContext* InCMDBuffer)
{
    if (!bIsMapped)
    {
        Buffer->Map(0, nullptr, &pData);
        bIsMapped = true;
        if (Ptr)
            memcpy((BYTE*)pData + Location, Ptr, Stride);
    }
    else
    {
        if (Ptr)
            memcpy((BYTE*)pData + Location, Ptr, Stride);
    }
}

void D3D12ByteAddressBuffer::Unmap()
{
    if (bIsMapped)
    {
        bIsMapped = false;
        Buffer->Unmap(0, nullptr);
    }
}

D3D12StructuredBuffer::D3D12StructuredBuffer(D3D12Device* InOwner, std::string InName, const BufferLayout& InDesc, bool bSRVAllowed)
    : Super()
    , Name(InName)
    , Owner(InOwner)
    , Desc(InDesc)
    , ViewHeap(D3D12DescriptorHandle(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV))
    , bIsSRVAllowed(bSRVAllowed)
    , pData(nullptr)
    , bIsMapped(false)
{
    ID3D12Device14* Device = Owner->GetDevice();
    Owner->AllocateDescriptor(&ViewHeap);

    const UINT MaterialCapacity = (UINT)Desc.Size;
    const UINT Stride = (UINT)Desc.Stride;
    const UINT64 BufferSize = MaterialCapacity * Stride;

    auto HeapDesc = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto Desc = CD3DX12_RESOURCE_DESC::Buffer(BufferSize);

    const bool IsEnhancedBarriersSupported = Owner->IsEnhancedBarriersSupported();
    if (IsEnhancedBarriersSupported)
    {
        auto Desc1 = CD3DX12_RESOURCE_DESC1(Desc);
        ThrowIfFailed(Device->CreateCommittedResource3(
            &HeapDesc,
            D3D12_HEAP_FLAG_NONE,
            &Desc1,
            D3D12_BARRIER_LAYOUT_UNDEFINED,
            nullptr,
            nullptr,
            0,
            nullptr,
            IID_PPV_ARGS(&Buffer)));
    }
    else
    {
        Device->CreateCommittedResource(
            &HeapDesc,
            D3D12_HEAP_FLAG_NONE,
            &Desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&Buffer));
    }

    ZeroMemory(&pData, sizeof(pData));

    Buffer->Map(0, nullptr, &pData);
    bIsMapped = true;

    if (bSRVAllowed)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Buffer.FirstElement = 0;
        srvDesc.Buffer.NumElements = MaterialCapacity;
        srvDesc.Buffer.StructureByteStride = Stride;
        srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

        Device->CreateShaderResourceView(Buffer.Get(), &srvDesc, ViewHeap.GetCPU());
    }
}

D3D12StructuredBuffer::~D3D12StructuredBuffer()
{
    Unmap();
    Buffer = nullptr;
    Owner = nullptr;
}

void D3D12StructuredBuffer::Map(const void* Ptr, std::size_t Location, IGraphicsCommandContext* InCMDBuffer)
{
    if (!bIsMapped)
    {
        Buffer->Map(0, nullptr, &pData);
        bIsMapped = true;
        if (Ptr)
            memcpy((BYTE*)pData + (Location * Desc.Stride), Ptr, Desc.Stride);
    }
    else
    {
        if (Ptr)
            memcpy((BYTE*)pData + (Location * Desc.Stride), Ptr, Desc.Stride);
    }
}

void D3D12StructuredBuffer::Unmap()
{
    if (bIsMapped)
    {
        bIsMapped = false;
        Buffer->Unmap(0, nullptr);
    }
}
