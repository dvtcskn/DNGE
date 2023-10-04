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
#include "D3D12Fence.h"
#include "GI/D3DShared/D3DShared.h"

D3D12Fence::D3D12Fence(ID3D12Device* Device)
	: Fence(nullptr)
	, FenceHandle(INVALID_HANDLE_VALUE)
	, FenceCounter(1)
	, LastSignaled(0)
	, LastCompleted(0)
{
    ThrowIfFailed(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));
	FenceHandle = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
	if (FenceHandle == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}
}

D3D12Fence::~D3D12Fence()
{
	Fence->Release();
	Fence = nullptr;
	CloseHandle(FenceHandle);
}

std::uint64_t D3D12Fence::Signal(ID3D12CommandQueue* queue)
{
	ThrowIfFailed(queue->Signal(Fence, FenceCounter));
	LastSignaled = FenceCounter;
	FenceCounter++;
	return LastSignaled;
}

bool D3D12Fence::Signaled(std::uint64_t fenceValue)
{
	return Fence->GetCompletedValue() >= fenceValue;
}

void D3D12Fence::CpuWaitForFence(std::uint64_t fenceValue)
{
	if (IsComplete(fenceValue))
		return;

	std::lock_guard<std::mutex> lockGuard(FenceMutex);

	Fence->SetEventOnCompletion(fenceValue, FenceHandle);
	WaitForSingleObject(FenceHandle, INFINITE);

	LastCompleted = Fence->GetCompletedValue();
}

void D3D12Fence::CpuWait()
{
	CpuWaitForFence(LastSignaled);
}

bool D3D12Fence::IsComplete(std::uint64_t fenceValue)
{
	if (fenceValue <= LastCompleted)
		return true;
	LastCompleted = std::max(LastCompleted, Fence->GetCompletedValue());
	return fenceValue <= LastCompleted;
}

void D3D12Fence::GpuWaitForFence(ID3D12CommandQueue* pCommandQueue)
{
	ThrowIfFailed(pCommandQueue->Wait(Fence, FenceCounter));
}
