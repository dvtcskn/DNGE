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

#include <assert.h>
#include <set>
#include <vector>
#include <algorithm>
#include <functional>
#include "dx12.h"

class D3D12Device;
class D3D12DescriptorHeap;
class D3D12DescriptorHeapManager;

struct D3D12DescriptorHandle
{
public:
    D3D12DescriptorHandle(/*D3D12DescriptorHeap* Owner,*/ D3D12_DESCRIPTOR_HEAP_TYPE Type/*, const uint32_t inDescriptorSize = 1*/);
    virtual ~D3D12DescriptorHandle();

    inline void Reset(std::weak_ptr<D3D12DescriptorHeap> NewOwner, uint32_t InHeapIndex, D3D12_CPU_DESCRIPTOR_HANDLE inCPUDescriptor, D3D12_GPU_DESCRIPTOR_HANDLE inGPUDescriptor)
    {
        Owner = NewOwner;
        HeapIndex = InHeapIndex;
        CPUDescriptor = inCPUDescriptor;
        GPUDescriptor = inGPUDescriptor;
    }

    inline uint32_t GetHeapIndex() const { return HeapIndex; }
    //inline uint32_t GetSize() const { return DescriptorSize; }
    inline D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType() const { return Type; }

    inline D3D12_CPU_DESCRIPTOR_HANDLE GetCPU() const
    {
        return CPUDescriptor;
    }

    inline D3D12_GPU_DESCRIPTOR_HANDLE GetGPU() const
    {
        return GPUDescriptor;
    }

    inline bool IsCPUOnly() const { return GPUDescriptor == D3D12_GPU_DESCRIPTOR_HANDLE(~0u); }

    void Release();

private:
    uint32_t HeapIndex;
    D3D12_DESCRIPTOR_HEAP_TYPE Type;
    //uint32_t DescriptorSize;

    CD3DX12_CPU_DESCRIPTOR_HANDLE CPUDescriptor;
    CD3DX12_GPU_DESCRIPTOR_HANDLE GPUDescriptor;

    std::weak_ptr<D3D12DescriptorHeap> Owner;

    //D3D12DescriptorHeap* Owner;
};

class D3D12DescriptorHeap : public std::enable_shared_from_this<D3D12DescriptorHeap>
{
public:
    D3D12DescriptorHeap(D3D12DescriptorHeapManager* NewOwner, ID3D12Device* pDevice, const D3D12_DESCRIPTOR_HEAP_TYPE Type, const uint32_t Count);
    ~D3D12DescriptorHeap();

    inline bool AllocateDescriptor(D3D12DescriptorHandle* DescriptorHandle)
    {
        if (FreedDescriptorIndices.size() > 0)
        {
            uint32_t HeapIndex = FreedDescriptorIndices.back();
            FreedDescriptorIndices.pop_back();

            DescriptorHandle->Reset(weak_from_this(), HeapIndex, CD3DX12_CPU_DESCRIPTOR_HANDLE(Heap->GetCPUDescriptorHandleForHeapStart(), HeapIndex * IncrementSize),
                Heap->GetDesc().Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE ? CD3DX12_GPU_DESCRIPTOR_HANDLE(Heap->GetGPUDescriptorHandleForHeapStart(), HeapIndex * IncrementSize)
                : D3D12_GPU_DESCRIPTOR_HANDLE(~0u));
        }
        else
        {
            if (TotalAllocatedDescriptorCount >= TotalDescriptorSize)
                return false;

            DescriptorHandle->Reset(weak_from_this(), TotalAllocatedDescriptorCount, CD3DX12_CPU_DESCRIPTOR_HANDLE(Heap->GetCPUDescriptorHandleForHeapStart(), TotalAllocatedDescriptorCount * IncrementSize),
                Heap->GetDesc().Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE ? CD3DX12_GPU_DESCRIPTOR_HANDLE(Heap->GetGPUDescriptorHandleForHeapStart(), TotalAllocatedDescriptorCount * IncrementSize)
                : D3D12_GPU_DESCRIPTOR_HANDLE(~0u));

            TotalAllocatedDescriptorCount++;
        }

        return true;
    }

    inline void Free(D3D12DescriptorHandle* Handle)
    {
        if (std::find(FreedDescriptorIndices.begin(), FreedDescriptorIndices.end(), Handle->GetHeapIndex()) != FreedDescriptorIndices.end())
            return;

        FreedDescriptorIndices.push_back(Handle->GetHeapIndex());
        std::sort(FreedDescriptorIndices.begin(), FreedDescriptorIndices.end());
        //FreedDescriptorIndices.erase(std::unique(FreedDescriptorIndices.begin(), FreedDescriptorIndices.end()), FreedDescriptorIndices.end());
    }

    inline ID3D12DescriptorHeap* GetHeap() const { return Heap; }

private:
    uint32_t IncrementSize;
    uint32_t TotalDescriptorSize;

    uint32_t TotalAllocatedDescriptorCount;
    std::vector<uint32_t> FreedDescriptorIndices;

    ID3D12DescriptorHeap* Heap;
    D3D12_DESCRIPTOR_HEAP_TYPE HeapType;

    D3D12DescriptorHeapManager* Owner;
};

class D3D12DescriptorHeapManager
{
public:
    D3D12DescriptorHeapManager(D3D12Device* InOwner);
    ~D3D12DescriptorHeapManager();

    inline void AllocateDescriptor(D3D12DescriptorHandle* DescriptorHandle)
    {
        switch (DescriptorHandle->GetHeapType())
        {
        case D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
            RTV_Heap->AllocateDescriptor(DescriptorHandle);         //CPU
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
            DSV_Heap->AllocateDescriptor(DescriptorHandle);         //CPU
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
            CBV_SRV_UAV_Heap->AllocateDescriptor(DescriptorHandle); //CPU GPU
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
            Sampler_Heap->AllocateDescriptor(DescriptorHandle);     //CPU GPU
            break;
        }
    }

    inline void DeallocateDescriptor(D3D12DescriptorHandle* DescriptorHandle)
    {
        switch (DescriptorHandle->GetHeapType())
        {
        case D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
            RTV_Heap->Free(DescriptorHandle);           //CPU
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
            DSV_Heap->Free(DescriptorHandle);           //CPU
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
            CBV_SRV_UAV_Heap->Free(DescriptorHandle);   //CPU GPU
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
            Sampler_Heap->Free(DescriptorHandle);       //CPU GPU
            break;
        }
    }

    ID3D12DescriptorHeap* GetHeap() const { return CBV_SRV_UAV_Heap->GetHeap(); }

    void SetHeaps(ID3D12GraphicsCommandList* cmd);

private:
    D3D12Device* Owner;
    std::unique_ptr<D3D12DescriptorHeap> RTV_Heap;
    std::unique_ptr<D3D12DescriptorHeap> DSV_Heap;
    std::unique_ptr<D3D12DescriptorHeap> CBV_SRV_UAV_Heap;
    std::unique_ptr<D3D12DescriptorHeap> Sampler_Heap;
};
