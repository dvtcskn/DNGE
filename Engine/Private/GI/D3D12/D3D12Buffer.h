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

#include "D3D12Device.h"
#include "D3D12DescriptorHeapManager.h"
#include "Engine/AbstractEngine.h"

class D3D12UploadBuffer
{
    sBaseClassBody(sClassConstructor, D3D12UploadBuffer)
public:
    D3D12UploadBuffer(D3D12Device* InOwner, std::string Name, std::uint32_t Size);
    virtual ~D3D12UploadBuffer();

    inline ID3D12Resource* GetBuffer() const { return Buffer.Get(); }
    inline ID3D12Resource* Get() const { return Buffer.Get(); }
    inline D3D12_GPU_VIRTUAL_ADDRESS GetGPU() const { return Buffer->GetGPUVirtualAddress(); }

    void UpdateSubresource(ID3D12Resource* InResource, D3D12_RESOURCE_STATES State, BufferSubresource* Subresource, D3D12CommandBuffer* InCMDBuffer = nullptr);
    void Map(const void* Ptr = nullptr);
    void Map(const void* Ptr, std::size_t Location, std::uint32_t Stride, IGraphicsCommandContext* InCMDBuffer = nullptr);
    void Unmap();

    bool IsMapped() const { return bIsMapped; }
    void* GetData() const { return pData; }

private:
    ComPtr<ID3D12Resource> Buffer;
    std::uint32_t Size;
    std::string Name;
    void* pData;
    bool bIsMapped;
};

class D3D12ConstantBuffer : public IConstantBuffer
{
    sClassBody(sClassConstructor, D3D12ConstantBuffer, IConstantBuffer)

    //ComPtr<ID3D12Resource> m_pResource;

    std::string Name;
    D3D12DescriptorHandle ViewHeap;
    BufferLayout BufferDesc;
    D3D12Device* Owner;
    std::uint32_t RootParameterIndex;

    D3D12UploadBuffer::UniquePtr UploadBuffer;

public:
    D3D12ConstantBuffer(D3D12Device* InOwner, std::string Name, const BufferLayout& InDesc, std::uint32_t RootParameterIndex);
    virtual ~D3D12ConstantBuffer();

    FORCEINLINE virtual std::string GetName() const override final { return Name; };
    virtual std::uint32_t GetBindlessIndex() const override final;
        
    std::size_t GetSize() const;

    virtual void SetDefaultRootParameterIndex(std::uint32_t inRootParameterIndex) override final { RootParameterIndex = inRootParameterIndex; }
    virtual std::uint32_t GetDefaultRootParameterIndex() const override final { return RootParameterIndex; }
    void ApplyConstantBuffer(ID3D12GraphicsCommandList7* CommandList);
    void ApplyConstantBuffer(ID3D12GraphicsCommandList7* CommandList, std::uint32_t InRootParameterIndex);

    virtual void Map(const void* Ptr, IGraphicsCommandContext* InCMDBuffer = nullptr) override final;
    void Unmap();

    inline D3D12DescriptorHandle GetHeapHandle() const { return ViewHeap; }
    inline D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const { return  UploadBuffer->GetGPU(); }

    virtual ResourceSharedHandle* GetSharedHandle() const override final { return nullptr; }
    virtual bool CopyFrom(IConstantBuffer* ConstantBuffer) override final { return false; }
};

class D3D12VertexBuffer : public IVertexBuffer
{
    sClassBody(sClassConstructor, D3D12VertexBuffer, IVertexBuffer)

    std::string Name;
    BufferLayout BufferDesc;
    ComPtr<ID3D12Resource> VertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
    D3D12Device* Owner;

    D3D12UploadBuffer::UniquePtr UploadBuffer;

public:
    D3D12VertexBuffer(D3D12Device* InOwner, std::string Name, const BufferLayout& InDesc, BufferSubresource* Subresource);
    virtual ~D3D12VertexBuffer();

    FORCEINLINE virtual std::string GetName() const final override { return Name; };

    virtual std::size_t GetSize() const override final;
    virtual bool IsMapable() const final override { return false; }

    ID3D12Resource* GetBuffer() const { return VertexBuffer.Get(); }
    D3D12UploadBuffer* GetUploadBuffer() const { return UploadBuffer.get(); }

    void ApplyBuffer(ID3D12GraphicsCommandList7* CommandList, std::uint32_t Slot = 0);
    void ResizeBuffer(std::size_t Size, BufferSubresource* InSubresource = nullptr);
    virtual void UpdateSubresource(BufferSubresource* Subresource, IGraphicsCommandContext* InCMDBuffer = nullptr) override final;

    inline D3D12_VERTEX_BUFFER_VIEW GetBufferView() const { return VertexBufferView; }

    virtual ResourceSharedHandle* GetSharedHandle() const override final { return nullptr; }
    virtual bool CopyFrom(IVertexBuffer* VertexBuffer) override final { return false; }

    D3D12_RESOURCE_STATES CurrentState;
    // Enhanced Barrier States
    D3D12_BARRIER_ACCESS CurrentAccessState;
    D3D12_BARRIER_SYNC CurrentSyncState;
    D3D12_BARRIER_LAYOUT CurrentBarrierLayout;
};

class D3D12IndexBuffer : public IIndexBuffer
{
    sClassBody(sClassConstructor, D3D12IndexBuffer, IIndexBuffer)

    std::string Name;

    BufferLayout BufferDesc;
    ComPtr<ID3D12Resource> IndexBuffer;
    D3D12_INDEX_BUFFER_VIEW IndexBufferView;
    D3D12Device* Owner;

    D3D12UploadBuffer::UniquePtr UploadBuffer;

public:
    D3D12IndexBuffer(D3D12Device* InOwner, std::string Name, const BufferLayout& InDesc, BufferSubresource* Subresource);
    virtual ~D3D12IndexBuffer();

    FORCEINLINE virtual std::string GetName() const final override { return Name; };

    virtual std::size_t GetSize() const override final;
    virtual bool IsMapable() const final override { return false; }

    ID3D12Resource* GetBuffer() const { return IndexBuffer.Get(); }
    D3D12UploadBuffer* GetUploadBuffer() const { return UploadBuffer.get(); }

    void ApplyBuffer(ID3D12GraphicsCommandList7* CommandList);
    void ResizeBuffer(std::size_t Size, BufferSubresource* InSubresource = nullptr);
    virtual void UpdateSubresource(BufferSubresource* Subresource, IGraphicsCommandContext* InCMDBuffer = nullptr) override final;

    inline D3D12_INDEX_BUFFER_VIEW GetBufferView() const { return IndexBufferView; }

    virtual ResourceSharedHandle* GetSharedHandle() const override final { return nullptr; }
    virtual bool CopyFrom(IIndexBuffer* IndexBuffer) override final { return false; }

    D3D12_RESOURCE_STATES CurrentState;
    // Enhanced Barrier States
    D3D12_BARRIER_LAYOUT CurrentBarrierLayout;
    D3D12_BARRIER_SYNC CurrentSyncState;
    D3D12_BARRIER_ACCESS CurrentAccessState;
};

class D3D12IndirectBuffer final : public IIndirectBuffer
{
    sClassBody(sClassConstructor, D3D12IndirectBuffer, IIndirectBuffer)
private:
    std::string Name; 
    BufferLayout Layout;
    std::uint64_t Offset;
    std::uint64_t CommandSize;

    ComPtr<ID3D12Resource> Buffer;
    void* pData;
    bool bIsMapped;

public:
    D3D12IndirectBuffer(D3D12Device* InDevice, std::string InName, BufferLayout NewLayout);
    virtual ~D3D12IndirectBuffer();

    ID3D12Resource* GetBuffer() const { return Buffer.Get(); }

    virtual std::string GetName() const override final { return Name; };
    virtual std::uint64_t GetSize() const override final { return Layout.Size * Layout.Stride; };
    virtual std::uint64_t GetTotalCommandSize() const override final { return Layout.Size; };
    virtual std::uint64_t GetCurrentCommandSize() const override final { return CommandSize; };
    virtual std::uint64_t GetStride() const override final { return Layout.Stride; };
    virtual std::uint64_t GetOffset() const override final { return Offset; };

    virtual void SetArgument(void* InArgument, std::size_t NewCommandSize = 1, std::size_t NewOffset = 0) override final;

    virtual ResourceSharedHandle* GetSharedHandle() const override final { return nullptr; }
    virtual bool CopyFrom(IIndirectBuffer* IndirectBuffer) override final { return false; }
};

class D3D12ByteAddressBuffer final : public IByteAddressBuffer
{
    sClassBody(sClassConstructor, D3D12ByteAddressBuffer, IByteAddressBuffer)
private:
    std::string Name;
    D3D12Device* Owner;
    std::uint64_t Size;
    ComPtr<ID3D12Resource> Buffer;
    D3D12DescriptorHandle ViewHeap;
    void* pData;
    bool bIsMapped;
    bool bIsReadWriteAllowed;

public:
    D3D12ByteAddressBuffer(D3D12Device* InDevice, std::string InName, std::uint64_t Size, bool bReadWriteAllowed);
    virtual ~D3D12ByteAddressBuffer();

    FORCEINLINE virtual std::string GetName() const override final { return Name;};
    virtual std::uint32_t GetBindlessIndex() const override final { return ViewHeap.GetHeapIndex(); }

    virtual bool IsReadWriteAllowed() const { return bIsReadWriteAllowed; }

    virtual std::uint64_t GetSize() const override final { return Size; }
    virtual bool IsMapable() const override final { return true; }
    virtual void Map(const void* Ptr, std::size_t Location, std::uint32_t Stride, IGraphicsCommandContext* InCMDBuffer = nullptr) override final;
    void Unmap();

    virtual ResourceSharedHandle* GetSharedHandle() const override final { return nullptr; }

    virtual bool CopyFrom(IByteAddressBuffer* UnorderedAccessBuffer) override final { return false; }
};

class D3D12StructuredBuffer final : public IStructuredBuffer
{
    sClassBody(sClassConstructor, D3D12StructuredBuffer, IStructuredBuffer)
private:
    std::string Name;
    D3D12Device* Owner;
    BufferLayout Desc;
    ComPtr<ID3D12Resource> Buffer;
    D3D12DescriptorHandle ViewHeap;
    void* pData;
    bool bIsMapped;
    bool bIsSRVAllowed;

public:
    D3D12StructuredBuffer(D3D12Device* InDevice, std::string InName, const BufferLayout& InDesc, bool bSRVAllowed = true);

    virtual ~D3D12StructuredBuffer();

    FORCEINLINE virtual std::string GetName() const override final { return Name; };
	virtual std::uint32_t GetBindlessIndex() const override final {	return ViewHeap.GetHeapIndex(); }

    virtual bool IsSRV_Allowed() const { return bIsSRVAllowed; }

    virtual std::size_t GetSize() const override final { return Desc.Size; }
    virtual bool IsMapable() const override final { return true; }
    virtual void Map(const void* Ptr, std::size_t Location, IGraphicsCommandContext* InCMDBuffer = nullptr) override final;
    void Unmap();

    virtual ResourceSharedHandle* GetSharedHandle() const override final { return nullptr; }

    virtual bool CopyFrom(IStructuredBuffer* UnorderedAccessBuffer) override final { return false; }
};
