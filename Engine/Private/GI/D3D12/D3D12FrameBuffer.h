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
#include <optional>
#include "dx12.h"
#include "D3D12DescriptorHeapManager.h"
#include "Engine/AbstractEngine.h"

#include <wrl\client.h>
using namespace Microsoft::WRL;

class D3D12RenderTarget final : public IRenderTarget
{
	sClassBody(sClassConstructor, D3D12RenderTarget, IRenderTarget)
public:
	D3D12RenderTarget(D3D12Device* InOwner, const std::string InName, const EFormat Format, const sFBODesc& Desc, bool InIsSRVAllowed = true, bool InIsUnorderedAccessAllowed = false);
	virtual ~D3D12RenderTarget();

	virtual void SetDefaultRootParameterIndex(std::uint32_t RootParameterIndex) override final {}
	virtual std::uint32_t GetDefaultRootParameterIndex() const override final { return 0; }

	virtual void* GetNativeTexture() const override final { return Texture.Get(); }
	ID3D12Resource* GetD3D12Texture() const { return Texture.Get(); }
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const { return Texture->GetGPUVirtualAddress(); }

	inline const D3D12DescriptorHandle* GetRTV() const { return &RTV; }
	inline const D3D12DescriptorHandle* GetSRV() const { return &SRV; }
	inline const D3D12DescriptorHandle* GetUAV() const { return &UAV; }

	inline D3D12_CPU_DESCRIPTOR_HANDLE GetRTVCPU() const { return RTV.GetCPU(); }
	inline D3D12_GPU_DESCRIPTOR_HANDLE GetRTVGPU() const { return RTV.GetGPU(); }
	inline D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPU() const { return SRV.GetCPU(); }
	inline D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPU() const { return SRV.GetGPU();	}
	inline D3D12_CPU_DESCRIPTOR_HANDLE GetUAVCPU() const { return UAV.GetCPU(); }
	inline D3D12_GPU_DESCRIPTOR_HANDLE GetUAVGPU() const { return UAV.GetGPU(); }

	EFormat GetFormat() const { return Format; }

	virtual bool IsSRV_Allowed() const override final { return bIsSRVSupported; }
	virtual bool IsUAV_Allowed() const override final { return bIsUAVSupported; }

	D3D12_RESOURCE_STATES CurrentState;

private:
	std::string Name;
	EFormat Format;

	ComPtr<ID3D12Resource> Texture;
	D3D12DescriptorHandle RTV;
	D3D12DescriptorHandle SRV;
	D3D12DescriptorHandle UAV;

	bool bIsSRVSupported;
	bool bIsUAVSupported;
};

class D3D12DepthTarget final : public IDepthTarget
{
	sClassBody(sClassConstructor, D3D12DepthTarget, IDepthTarget)
public:
	D3D12DepthTarget(D3D12Device* InOwner, const std::string InName, const EFormat Format, const sFBODesc& Desc);
	virtual ~D3D12DepthTarget();

	virtual void SetDefaultRootParameterIndex(std::uint32_t RootParameterIndex) override final {}
	virtual std::uint32_t GetDefaultRootParameterIndex() const override final { return 0; }

	virtual void* GetNativeTexture() const override final { return Texture.Get(); }
	ID3D12Resource* GetD3D12Texture() const { return Texture.Get(); }
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const { return Texture->GetGPUVirtualAddress(); }

	inline const D3D12DescriptorHandle* GetDSV() const { return &DSV; }
	inline const D3D12DescriptorHandle* GetSRV() const { return &SRV; }

	inline D3D12_CPU_DESCRIPTOR_HANDLE GetDSVCPU() const { return DSV.GetCPU(); }
	inline D3D12_GPU_DESCRIPTOR_HANDLE GetDSVGPU() const { return DSV.GetGPU(); }
	inline D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPU() const { return SRV.GetCPU(); }
	inline D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPU() const { return SRV.GetGPU(); }

	bool IsSRVSupported() const { return bIsSRVSupported; }
	virtual bool IsSRV_Allowed() const override final { return bIsSRVSupported; }
	virtual bool IsUAV_Allowed() const override final { return false; }

	EFormat GetFormat() const { return Format; }

	D3D12_RESOURCE_STATES CurrentState;

private:
	std::string Name;
	EFormat Format;

	ComPtr<ID3D12Resource> Texture;
	D3D12DescriptorHandle DSV;
	D3D12DescriptorHandle SRV;

	bool bIsSRVSupported;
};

class D3D12UnorderedAccessTarget final : public IUnorderedAccessTarget
{
	sClassBody(sClassConstructor, D3D12UnorderedAccessTarget, IUnorderedAccessTarget)
public:
	D3D12UnorderedAccessTarget(D3D12Device* InOwner, const std::string InName, const EFormat Format, const sFBODesc& Desc, bool InEnableSRV = true);
	virtual ~D3D12UnorderedAccessTarget();

	virtual void SetDefaultRootParameterIndex(std::uint32_t RootParameterIndex) override final {}
	virtual std::uint32_t GetDefaultRootParameterIndex() const override final { return 0; }

	virtual void* GetNativeTexture() const override final { return Texture.Get(); }
	ID3D12Resource* GetD3D12Texture() const { return Texture.Get(); }
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const { return Texture->GetGPUVirtualAddress(); }

	inline const D3D12DescriptorHandle* GetUAV() const { return &UAV; }
	inline const D3D12DescriptorHandle* GetSRV() const { return &SRV; }

	inline D3D12_CPU_DESCRIPTOR_HANDLE GetUAVCPU() const { return UAV.GetCPU(); }
	inline D3D12_GPU_DESCRIPTOR_HANDLE GetUAVGPU() const { return UAV.GetGPU(); }
	inline D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPU() const { return SRV.GetCPU(); }
	inline D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPU() const { return SRV.GetGPU(); }

	bool IsSRVSupported() const { return bIsSRVSupported; }
	virtual bool IsSRV_Allowed() const override final { return bIsSRVSupported; }

	EFormat GetFormat() const { return Format; }

	D3D12_RESOURCE_STATES CurrentState;

private:
	std::string Name;
	EFormat Format;

	ComPtr<ID3D12Resource> Texture;
	D3D12DescriptorHandle UAV;
	D3D12DescriptorHandle SRV;

	bool bIsSRVSupported;
};

class D3D12FrameBuffer final : public IFrameBuffer
{
	sClassBody(sClassConstructor, D3D12FrameBuffer, IFrameBuffer)
private:
	D3D12Device* Owner;
	std::string Name;

	sFrameBufferAttachmentInfo AttachmentInfo;

public:
	std::vector<D3D12RenderTarget::SharedPtr> RenderTargets;
	std::vector<D3D12UnorderedAccessTarget::SharedPtr> UnorderedAccessTargets;
	D3D12DepthTarget::SharedPtr DepthTarget;

public:
	D3D12FrameBuffer(D3D12Device* InOwner, std::string InName, const sFrameBufferAttachmentInfo& InAttachments);

	virtual ~D3D12FrameBuffer()
	{
		Owner = nullptr;

		for (auto& RT : RenderTargets)
			RT = nullptr;
		RenderTargets.clear();

		for (auto& UAV : UnorderedAccessTargets)
			UAV = nullptr;
		UnorderedAccessTargets.clear();

		DepthTarget = nullptr;
	}

	virtual std::string GetName() const override final
	{
		return Name;
	}

	virtual sFrameBufferAttachmentInfo GetAttachmentInfo() const override final
	{
		return AttachmentInfo;
	}

	virtual std::size_t GetAttachmentCount() const override final
	{
		return AttachmentInfo.GetAttachmentCount();
	}
	virtual std::size_t GetRenderTargetAttachmentCount() const override final
	{
		return AttachmentInfo.GetRenderTargetAttachmentCount();
	}
	D3D12RenderTarget* PrimaryFrameBuffer() const
	{
		return RenderTargets.at(AttachmentInfo.PrimaryFB).get();
	}

	virtual std::vector<IRenderTarget*> GetRenderTargets() const override final
	{
		std::vector<IRenderTarget*> Result;
		Result.reserve(RenderTargets.size());
		std::transform(RenderTargets.cbegin(), RenderTargets.cend(), std::back_inserter(Result), [](auto& ptr) { return ptr.get(); });
		return Result;
	}
	virtual IRenderTarget* GetRenderTarget(std::size_t Index) const override final { return RenderTargets.at(Index).get(); }
	virtual void AttachRenderTarget(const IRenderTarget::SharedPtr& RenderTarget, std::optional<std::size_t> Index = std::nullopt) override final;

	virtual std::vector<IUnorderedAccessTarget*> GetUnorderedAccessTargets() const override final
	{
		std::vector<IUnorderedAccessTarget*> Result;
		Result.reserve(UnorderedAccessTargets.size());
		std::transform(UnorderedAccessTargets.cbegin(), UnorderedAccessTargets.cend(), std::back_inserter(Result), [](auto& ptr) { return ptr.get(); });
		return Result;
	}
	virtual IUnorderedAccessTarget* GetUnorderedAccessTarget(std::size_t Index) const override final { return UnorderedAccessTargets.at(Index).get(); }
	virtual void AttachUnorderedAccessTarget(const IUnorderedAccessTarget::SharedPtr& UnorderedAccessTarget, std::optional<std::size_t> Index = std::nullopt) override final;

	virtual IDepthTarget* GetDepthTarget() const override final { return DepthTarget.get(); }
	virtual void SetDepthTarget(const IDepthTarget::SharedPtr& DepthTarget) override final;
};
