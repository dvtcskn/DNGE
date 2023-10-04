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
    D3D12UploadBuffer(ID3D12Device* Device, std::string Name, std::uint32_t Size);
    virtual ~D3D12UploadBuffer();

    inline ID3D12Resource* GetBuffer() const { return Buffer.Get(); }
    inline ID3D12Resource* Get() const { return Buffer.Get(); }
    inline D3D12_GPU_VIRTUAL_ADDRESS GetGPU() const { return Buffer->GetGPUVirtualAddress(); }

    void UpdateSubresource(ID3D12Resource* InResource, D3D12_RESOURCE_STATES State, sBufferSubresource* Subresource, D3D12CommandBuffer* InCMDBuffer = nullptr);
    void Map(const void* Ptr = nullptr);
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

    ComPtr<ID3D12Resource> m_pResource;

    std::string Name;
    D3D12DescriptorHandle ViewHeap;
    sBufferDesc BufferDesc;
    D3D12Device* Owner;
    std::uint32_t RootParameterIndex;

    D3D12UploadBuffer::UniquePtr UploadBuffer;

public:
    D3D12ConstantBuffer(D3D12Device* InOwner, std::string Name, const sBufferDesc& InDesc, std::uint32_t RootParameterIndex);
    virtual ~D3D12ConstantBuffer();

    FORCEINLINE virtual std::string GetName() const final override { return Name; };

    std::size_t GetSize() const;

    virtual void SetDefaultRootParameterIndex(std::uint32_t inRootParameterIndex) override final { RootParameterIndex = inRootParameterIndex; }
    virtual std::uint32_t GetDefaultRootParameterIndex() const override final { return RootParameterIndex; }
    void ApplyConstantBuffer(ID3D12GraphicsCommandList* CommandList);
    void ApplyConstantBuffer(ID3D12GraphicsCommandList* CommandList, std::uint32_t InRootParameterIndex);

    virtual void Map(const void* Ptr, IGraphicsCommandContext* InCMDBuffer = nullptr) override final;
    void Unmap();

    inline D3D12DescriptorHandle GetHeapHandle() const { return ViewHeap; }
    inline D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const { return  UploadBuffer->GetGPU(); }
};

class D3D12VertexBuffer : public IVertexBuffer
{
    sClassBody(sClassConstructor, D3D12VertexBuffer, IVertexBuffer)

    std::string Name;

    ComPtr<ID3D12Resource> m_pResource;

    sBufferDesc BufferDesc;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    D3D12Device* Owner;

    D3D12UploadBuffer::UniquePtr UploadBuffer;

public:
    D3D12VertexBuffer(D3D12Device* InOwner, std::string Name, const sBufferDesc& InDesc, sBufferSubresource* Subresource);
    virtual ~D3D12VertexBuffer();

    FORCEINLINE virtual std::string GetName() const final override { return Name; };

    virtual std::size_t GetSize() const override final;
    virtual bool IsMapable() const final override { return false; }

    ID3D12Resource* GetBuffer() const { return m_pResource.Get(); }
    D3D12UploadBuffer* GetUploadBuffer() const { return UploadBuffer.get(); }

    void ApplyBuffer(ID3D12GraphicsCommandList* CommandList);
    void ResizeBuffer(std::size_t Size, sBufferSubresource* InSubresource = nullptr);
    virtual void UpdateSubresource(sBufferSubresource* Subresource, IGraphicsCommandContext* InCMDBuffer = nullptr) override final;

    inline D3D12_VERTEX_BUFFER_VIEW GetBufferView() const { return m_vertexBufferView; }

    D3D12_RESOURCE_STATES CurrentState;
};

class D3D12IndexBuffer : public IIndexBuffer
{
    sClassBody(sClassConstructor, D3D12IndexBuffer, IIndexBuffer)

    std::string Name;

    ComPtr<ID3D12Resource> m_pResource;

    sBufferDesc BufferDesc;
    D3D12_INDEX_BUFFER_VIEW IBView;
    D3D12Device* Owner;

    D3D12UploadBuffer::UniquePtr UploadBuffer;

public:
    D3D12IndexBuffer(D3D12Device* InOwner, std::string Name, const sBufferDesc& InDesc, sBufferSubresource* Subresource);
    virtual ~D3D12IndexBuffer();

    FORCEINLINE virtual std::string GetName() const final override { return Name; };

    virtual std::size_t GetSize() const override final;
    virtual bool IsMapable() const final override { return false; }

    ID3D12Resource* GetBuffer() const { return m_pResource.Get(); }
    D3D12UploadBuffer* GetUploadBuffer() const { return UploadBuffer.get(); }

    void ApplyBuffer(ID3D12GraphicsCommandList* CommandList);
    void ResizeBuffer(std::size_t Size, sBufferSubresource* InSubresource = nullptr);
    virtual void UpdateSubresource(sBufferSubresource* Subresource, IGraphicsCommandContext* InCMDBuffer = nullptr) override final;

    inline D3D12_INDEX_BUFFER_VIEW GetBufferView() const { return IBView; }

    D3D12_RESOURCE_STATES CurrentState;
};
