
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
