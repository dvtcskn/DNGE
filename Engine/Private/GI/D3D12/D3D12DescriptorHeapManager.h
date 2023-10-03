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
#pragma once

#include <assert.h>
#include <set>
#include <vector>
#include <algorithm>
#include <functional>
#include "d3dx12.h"

class D3D12Device;

struct D3D12DescriptorHandle
{
public:
    D3D12DescriptorHandle(/*D3D12DescriptorHeap* Owner,*/ D3D12_DESCRIPTOR_HEAP_TYPE Type/*, const uint32_t inDescriptorSize = 1*/);
    virtual ~D3D12DescriptorHandle();

    inline void Reset(uint32_t InHeapIndex, D3D12_CPU_DESCRIPTOR_HANDLE inCPUDescriptor, D3D12_GPU_DESCRIPTOR_HANDLE inGPUDescriptor, const std::function<void(D3D12DescriptorHandle*)>& InfDeallocate)
    {
        HeapIndex = InHeapIndex;
        CPUDescriptor = inCPUDescriptor;
        GPUDescriptor = inGPUDescriptor;
        fDeallocate = InfDeallocate;
    }

    inline uint32_t GetHeapIndex() const { return HeapIndex; }
    //inline uint32_t GetSize() const { return DescriptorSize; }
    inline D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType() const { return Type; }

    inline D3D12_CPU_DESCRIPTOR_HANDLE GetCPU(/*const uint32_t i = 0*/) const
    {
        return CPUDescriptor;
        //return CD3DX12_CPU_DESCRIPTOR_HANDLE(CPUDescriptor, i * IncrementSize);
    }

    inline D3D12_GPU_DESCRIPTOR_HANDLE GetGPU(/*const uint32_t i = 0*/) const
    {
        return GPUDescriptor;
        //return CD3DX12_GPU_DESCRIPTOR_HANDLE(GPUDescriptor, i * IncrementSize);
    }

    inline bool IsCPUOnly() const { return GPUDescriptor == D3D12_GPU_DESCRIPTOR_HANDLE(~0u); }

    void Release()
    {
        if (fDeallocate)
            fDeallocate(this);

        fDeallocate = nullptr;

        CPUDescriptor = D3D12_CPU_DESCRIPTOR_HANDLE(~0u);
        GPUDescriptor = D3D12_GPU_DESCRIPTOR_HANDLE(~0u);
        HeapIndex = ~0u;
    }

private:
    uint32_t HeapIndex;
    D3D12_DESCRIPTOR_HEAP_TYPE Type;
    //uint32_t DescriptorSize;

    CD3DX12_CPU_DESCRIPTOR_HANDLE CPUDescriptor;
    CD3DX12_GPU_DESCRIPTOR_HANDLE GPUDescriptor;

    std::function<void(D3D12DescriptorHandle*)> fDeallocate;

    //D3D12DescriptorHeap* Owner;
};

class D3D12DescriptorHeapManager
{
private:
    struct D3D12DescriptorHeap
    {
    public:
        D3D12DescriptorHeap(D3D12DescriptorHeapManager* Owner, ID3D12Device* pDevice, const D3D12_DESCRIPTOR_HEAP_TYPE Type, const uint32_t Count);
        ~D3D12DescriptorHeap();

        inline bool AllocateDescriptor(D3D12DescriptorHandle* DescriptorHandle)
        {
            if (TotalAllocatedDescriptorCount >= TotalDescriptorSize)
                /*GrowHeap();*/ return false;

            if (FreedDescriptorIndices.size() > 0)
            {
                uint32_t HeapIndex = FreedDescriptorIndices.back();
                FreedDescriptorIndices.pop_back();

                DescriptorHandle->Reset(HeapIndex, CD3DX12_CPU_DESCRIPTOR_HANDLE(Heap->GetCPUDescriptorHandleForHeapStart(), HeapIndex * IncrementSize),
                    Heap->GetDesc().Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE ? CD3DX12_GPU_DESCRIPTOR_HANDLE(Heap->GetGPUDescriptorHandleForHeapStart(), HeapIndex * IncrementSize)
                                                                                      : D3D12_GPU_DESCRIPTOR_HANDLE(~0u), std::bind(&D3D12DescriptorHeap::Free, this, std::placeholders::_1));
            }
            else
            {
                DescriptorHandle->Reset(TotalAllocatedDescriptorCount, CD3DX12_CPU_DESCRIPTOR_HANDLE(Heap->GetCPUDescriptorHandleForHeapStart(), TotalAllocatedDescriptorCount * IncrementSize),
                    Heap->GetDesc().Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE ? CD3DX12_GPU_DESCRIPTOR_HANDLE(Heap->GetGPUDescriptorHandleForHeapStart(), TotalAllocatedDescriptorCount * IncrementSize)
                                                                                      : D3D12_GPU_DESCRIPTOR_HANDLE(~0u), std::bind(&D3D12DescriptorHeap::Free, this, std::placeholders::_1));

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
        inline bool GrowHeap()
        {
            /*
            * to do ?
            * Save all DescriptorHandles.
            * Create new bigger heap
            * Then reset all DescriptorHandless again. 
            * ???
            */

            return false;
        }

    private:
        uint32_t IncrementSize;
        uint32_t TotalDescriptorSize;

        uint32_t TotalAllocatedDescriptorCount;
        std::vector<uint32_t> FreedDescriptorIndices;

        ID3D12DescriptorHeap* Heap;
        D3D12_DESCRIPTOR_HEAP_TYPE HeapType;

        D3D12DescriptorHeapManager* Owner;
    };

public:
    D3D12DescriptorHeapManager(D3D12Device* InOwner);
    ~D3D12DescriptorHeapManager();

    inline void AllocateDescriptor(D3D12DescriptorHandle* DescriptorHandle)
    {
        switch (DescriptorHandle->GetHeapType())
        {           
            case D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV: RTV_Heap->AllocateDescriptor(DescriptorHandle); break;                 //CPU
            case D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_DSV: DSV_Heap->AllocateDescriptor(DescriptorHandle); break;                 //CPU
            case D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV: CBV_SRV_UAV_Heap->AllocateDescriptor(DescriptorHandle); break; //CPU GPU
            case D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER: Sampler_Heap->AllocateDescriptor(DescriptorHandle); break;         //CPU GPU
        }
    }

    inline void DeallocateDescriptor(D3D12DescriptorHandle* DescriptorHandle)
    {
        switch (DescriptorHandle->GetHeapType())
        {
            case D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV: RTV_Heap->Free(DescriptorHandle); break;                 //CPU
            case D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_DSV: DSV_Heap->Free(DescriptorHandle); break;                 //CPU
            case D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV: CBV_SRV_UAV_Heap->Free(DescriptorHandle); break; //CPU GPU
            case D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER: Sampler_Heap->Free(DescriptorHandle); break;         //CPU GPU
        }
    }

    void SetHeaps(ID3D12GraphicsCommandList* cmd);

private:
    D3D12Device* Owner;
    /*
    * Make vectors and make another heap for each pass ?
    */
    std::unique_ptr<D3D12DescriptorHeap> RTV_Heap;
    std::unique_ptr<D3D12DescriptorHeap> DSV_Heap;
    std::unique_ptr<D3D12DescriptorHeap> CBV_SRV_UAV_Heap;
    std::unique_ptr<D3D12DescriptorHeap> Sampler_Heap;
};
