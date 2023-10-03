#pragma once

#include <d3d12.h>
#include "D3DX12.h"
#include <Engine/ClassBody.h>
#include <mutex>

class D3D12Fence
{
	sBaseClassBody(sClassConstructor, D3D12Fence)
public:
	D3D12Fence(ID3D12Device* Device);
	~D3D12Fence();

    std::uint64_t Signal(ID3D12CommandQueue* queue);
    bool Signaled(std::uint64_t fenceValue);
    bool IsComplete(std::uint64_t fenceValue);

    // This member is useful for tracking how ahead the CPU is from the GPU
    //
    // If the fence is used once per frame, calling this function with  
    // WaitForFence(3) will make sure the CPU is no more than 3 frames ahead
    //
    void CpuWaitForFence(std::uint64_t fenceValue);
    void CpuWait();
    void GpuWaitForFence(ID3D12CommandQueue* pCommandQueue);

    inline std::uint64_t GetFenceCounter() const { return FenceCounter; }
    inline std::uint64_t GetCompletedValue() const { return Fence->GetCompletedValue(); }

private:
    ID3D12Fence* Fence;
    HANDLE FenceHandle;

    std::mutex FenceMutex;
    std::uint64_t FenceCounter;
    std::uint64_t LastSignaled;
    std::uint64_t LastCompleted;
};
