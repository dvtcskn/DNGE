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

#include "dx12.h"
#include <Engine/ClassBody.h>
#include <atomic>
#include <mutex>
#include <wrl/client.h>

//class D3D12Fence
//{
//	sBaseClassBody(sClassConstructor, D3D12Fence)
//public:
//	D3D12Fence(ID3D12Device* Device);
//	~D3D12Fence();
//
//    std::uint64_t Signal(ID3D12CommandQueue* queue);
//    bool Signaled(std::uint64_t fenceValue);
//    bool IsComplete(std::uint64_t fenceValue);
//
//    // This member is useful for tracking how ahead the CPU is from the GPU
//    //
//    // If the fence is used once per frame, calling this function with  
//    // WaitForFence(3) will make sure the CPU is no more than 3 frames ahead
//    //
//    void CpuWaitForFence(std::uint64_t fenceValue);
//    void CpuWait();
//    void GpuWaitForFence(ID3D12CommandQueue* pCommandQueue);
//
//    inline std::uint64_t GetFenceCounter() const { return FenceCounter; }
//    inline std::uint64_t GetCompletedValue() const { return Fence->GetCompletedValue(); }
//
//private:
//    ID3D12Fence* Fence;
//    HANDLE FenceHandle;
//
//    std::mutex FenceMutex;
//    std::uint64_t FenceCounter;
//    std::uint64_t LastSignaled;
//    std::uint64_t LastCompleted;
//};

class D3D12Fence
{
	sBaseClassBody(sClassConstructor, D3D12Fence)
public:
	D3D12Fence(ID3D12Device* Device);
	~D3D12Fence();

	std::uint64_t Signal(ID3D12CommandQueue* queue);
	bool Signaled(std::uint64_t fenceValue) const;
	bool IsComplete(std::uint64_t fenceValue) const;

	void CpuWaitForFence(std::uint64_t fenceValue);
	void CpuWait();

	// Prefer the overload with an explicit, already-signaled value.
	void GpuWaitForFence(
		ID3D12CommandQueue* queue,
		std::uint64_t fenceValue);
	void GpuWaitForFence(ID3D12CommandQueue* queue);

	std::uint64_t GetFenceCounter() const noexcept
	{
		return NextFenceValue.load(std::memory_order_relaxed);
	}

	std::uint64_t GetCompletedValue() const;

private:
	Microsoft::WRL::ComPtr<ID3D12Fence> Fence;
	HANDLE FenceHandle = nullptr;

	// Serializes fence-value assignment/signals; the wait mutex protects the shared event.
	std::mutex SignalMutex;
	std::mutex WaitMutex;

	std::atomic<std::uint64_t> NextFenceValue{ 1 };
	std::atomic<std::uint64_t> LastSignaled{ 0 };
};
