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
#include "D3D12DescriptorHeapManager.h"
#include "GI/D3DShared/D3DShared.h"
#include "D3D12Device.h"

D3D12DescriptorHandle::D3D12DescriptorHandle(/*D3D12DescriptorHeap* pOwner,*/ D3D12_DESCRIPTOR_HEAP_TYPE Type/*, const uint32_t inDescriptorSize*/)
    : Type(Type)
    //, DescriptorSize(inDescriptorSize)
    , CPUDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE(~0u))
    , GPUDescriptor(D3D12_GPU_DESCRIPTOR_HANDLE(~0u))
    , fDeallocate(nullptr)
    //, Owner(pOwner)
{}

D3D12DescriptorHandle::~D3D12DescriptorHandle()
{
    if (fDeallocate)
        fDeallocate(this);

    fDeallocate = nullptr;

    CPUDescriptor = D3D12_CPU_DESCRIPTOR_HANDLE(~0u);
    GPUDescriptor = D3D12_GPU_DESCRIPTOR_HANDLE(~0u);
    HeapIndex = ~0u;
}

D3D12DescriptorHeapManager::D3D12DescriptorHeapManager(D3D12Device* InOwner)
    : Owner(InOwner)
{
    const auto Device = Owner->GetDevice();
    RTV_Heap = std::make_unique<D3D12DescriptorHeap>(this, Device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 128);
    DSV_Heap = std::make_unique<D3D12DescriptorHeap>(this, Device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 64);
    CBV_SRV_UAV_Heap = std::make_unique<D3D12DescriptorHeap>(this, Device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 16384);
    Sampler_Heap = std::make_unique<D3D12DescriptorHeap>(this, Device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 2048);
}

D3D12DescriptorHeapManager::~D3D12DescriptorHeapManager()
{
    RTV_Heap = nullptr;
    CBV_SRV_UAV_Heap = nullptr;
    Sampler_Heap = nullptr;
    DSV_Heap = nullptr;
    Owner = nullptr;
}

void D3D12DescriptorHeapManager::SetHeaps(ID3D12GraphicsCommandList* cmd)
{
    ID3D12DescriptorHeap* ppHeaps[] = { CBV_SRV_UAV_Heap->GetHeap(), Sampler_Heap->GetHeap()};
    cmd->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
}

D3D12DescriptorHeapManager::D3D12DescriptorHeap::D3D12DescriptorHeap(D3D12DescriptorHeapManager* pOwner, ID3D12Device* pDevice, const D3D12_DESCRIPTOR_HEAP_TYPE Type, const uint32_t Count)
    : Owner(pOwner)
    //, TotalFreedDescriptorSize(0)
    , IncrementSize(pDevice->GetDescriptorHandleIncrementSize(Type))
    , TotalAllocatedDescriptorCount(0)
    , TotalDescriptorSize(Count)
    , Heap(nullptr)
    , HeapType(Type)
{
    D3D12_DESCRIPTOR_HEAP_DESC descHeap;
    descHeap.NumDescriptors = TotalDescriptorSize;
    descHeap.Type = HeapType;
    descHeap.NodeMask = 0;
    descHeap.Flags = HeapType == D3D12_DESCRIPTOR_HEAP_TYPE_RTV || HeapType == D3D12_DESCRIPTOR_HEAP_TYPE_DSV ? D3D12_DESCRIPTOR_HEAP_FLAG_NONE : D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(pDevice->CreateDescriptorHeap(&descHeap, IID_PPV_ARGS(&Heap)));
#if _DEBUG
    Heap->SetName(L"D3D12DescriptorHeap");
#endif
}

D3D12DescriptorHeapManager::D3D12DescriptorHeap::~D3D12DescriptorHeap()
{
    Heap->Release();
    Heap = nullptr;
    Owner = nullptr;

    FreedDescriptorIndices.clear();
}
