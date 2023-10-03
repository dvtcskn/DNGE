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

#include "D3D11Device.h"
#include "D3D11CommandBuffer.h"
#include <memory>
#include <vector>
#include "Engine/AbstractEngine.h"
#include "GI/D3DShared/D3DShared.h"

class D3D11RenderTarget final : public IRenderTarget
{
	sClassBody(sClassConstructor, D3D11RenderTarget, IRenderTarget)
public:
	D3D11RenderTarget(D3D11Device* InDevice, const std::string InName, const EFormat Format, const sFBODesc& Desc);
	virtual ~D3D11RenderTarget();

	virtual void* GetNativeTexture() const override final { return Texture.Get(); }

	virtual void SetDefaultRootParameterIndex(std::uint32_t RootParameterIndex) override final {}
	virtual std::uint32_t GetDefaultRootParameterIndex() const override final { return 0; }

	ID3D11Texture2D* GetD3D11Texture() const { return Texture.Get(); }
	ID3D11RenderTargetView* GetD3D11RTV() const { return RenderTarget.Get(); }
	ID3D11ShaderResourceView* GetD3D11SRV() const { return ShaderResource.Get(); }

	virtual bool IsSRV_Allowed() const override final { return true; }
	virtual bool IsUAV_Allowed() const override final { return false; }

	EFormat GetFormat() const { return Format; }

private:
	std::string Name;
	EFormat Format;

	ComPtr<ID3D11Texture2D> Texture;
	ComPtr<ID3D11RenderTargetView> RenderTarget;
	ComPtr<ID3D11ShaderResourceView> ShaderResource;
};

class D3D11DepthTarget final : public IDepthTarget
{
	sClassBody(sClassConstructor, D3D11DepthTarget, IDepthTarget)
public:
	D3D11DepthTarget(D3D11Device* InDevice, const std::string InName, const EFormat Format, const sFBODesc& Desc);
	virtual ~D3D11DepthTarget();

	virtual void* GetNativeTexture() const override final { return Texture.Get(); }

	virtual void SetDefaultRootParameterIndex(std::uint32_t RootParameterIndex) override final {}
	virtual std::uint32_t GetDefaultRootParameterIndex() const override final { return 0; }

	ID3D11Texture2D* GetD3D11Texture() const { return Texture.Get(); }
	ID3D11DepthStencilView* GetD3D11DSV() const { return DepthResource.Get(); }
	ID3D11ShaderResourceView* GetD3D11SRV() const { return ShaderResource.Get(); }

	bool IsSRVSupported() const { return bIsSRVSupported; }
	virtual bool IsSRV_Allowed() const override final { return bIsSRVSupported; }
	virtual bool IsUAV_Allowed() const override final { return false; }

	EFormat GetFormat() const { return Format; }

private:
	std::string Name;
	EFormat Format;

	ComPtr<ID3D11Texture2D> Texture;
	ComPtr<ID3D11DepthStencilView> DepthResource;
	ComPtr<ID3D11ShaderResourceView> ShaderResource;

	bool bIsSRVSupported;
};

class D3D11UnorderedAccessTarget final : public IUnorderedAccessTarget
{
	sClassBody(sClassConstructor, D3D11UnorderedAccessTarget, IUnorderedAccessTarget)
public:
	D3D11UnorderedAccessTarget(D3D11Device* InDevice, const std::string InName, const EFormat Format, const sFBODesc& Desc, bool InEnableSRV = true);
	virtual ~D3D11UnorderedAccessTarget();

	virtual void* GetNativeTexture() const override final { return Texture.Get(); }

	virtual void SetDefaultRootParameterIndex(std::uint32_t RootParameterIndex) override final {}
	virtual std::uint32_t GetDefaultRootParameterIndex() const override final { return 0; }

	ID3D11Texture2D* GetD3D11Texture() const { return Texture.Get(); }
	ID3D11UnorderedAccessView* GetD3D11UAV() const { return UnorderedAccessView.Get(); }
	ID3D11ShaderResourceView* GetD3D11SRV() const { return ShaderResource.Get(); }

	bool IsSRVSupported() const { return bIsSRVSupported; }
	virtual bool IsSRV_Allowed() const override final { return bIsSRVSupported; }

	EFormat GetFormat() const { return Format; }

private:
	std::string Name;
	EFormat Format;

	ComPtr<ID3D11Texture2D> Texture;
	ComPtr<ID3D11UnorderedAccessView> UnorderedAccessView;
	ComPtr<ID3D11ShaderResourceView> ShaderResource;

	bool bIsSRVSupported;
};

class D3D11FrameBuffer final : public IFrameBuffer
{
	sClassBody(sClassConstructor, D3D11FrameBuffer, IFrameBuffer)
private:
	D3D11Device* Owner;

	sFrameBufferAttachmentInfo AttachmentInfo;
	std::string Name;

public:
	std::vector<D3D11RenderTarget::SharedPtr> RenderTargets;
	std::vector<D3D11UnorderedAccessTarget::SharedPtr> UAVs;
	D3D11DepthTarget::SharedPtr DepthTarget;

public:
	D3D11FrameBuffer(D3D11Device* InDevice, const std::string InName, const sFrameBufferAttachmentInfo& InAttachments);

	virtual ~D3D11FrameBuffer()
	{
		Owner = nullptr;

		for (auto& RT : RenderTargets)
			RT = nullptr;
		RenderTargets.clear();

		for (auto& UAV : UAVs)
			UAV = nullptr;
		UAVs.clear();

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
	ID3D11Texture2D* GetPrimaryTexture() const
	{
		return RenderTargets.at(AttachmentInfo.PrimaryFB)->GetD3D11Texture();
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
		Result.reserve(UAVs.size());
		std::transform(UAVs.cbegin(), UAVs.cend(), std::back_inserter(Result), [](auto& ptr) { return ptr.get(); });
		return Result;
	}
	virtual IUnorderedAccessTarget* GetUnorderedAccessTarget(std::size_t Index) const override final { return UAVs.at(Index).get(); }
	virtual void AttachUnorderedAccessTarget(const IUnorderedAccessTarget::SharedPtr& UnorderedAccessTarget, std::optional<std::size_t> Index = std::nullopt) override final;

	virtual IDepthTarget* GetDepthTarget() const override final { return DepthTarget.get(); }
	virtual void SetDepthTarget(const IDepthTarget::SharedPtr& DepthTarget) override final;

	void ApplyFrameBuffer(IGraphicsCommandContext* InCMDBuffer = nullptr, std::optional<std::size_t> FBOIndex = std::nullopt);
	void ApplyFrameBuffer(ID3D11DeviceContext1* InCMDBuffer = nullptr, std::optional<std::size_t> FBOIndex = std::nullopt);
	void ApplyFrameBufferAsResource(ID3D11DeviceContext1* CMD, std::uint32_t InSlot, eShaderType InType);
	void ApplyFrameBufferAsResource(ID3D11DeviceContext1* CMD, std::uint32_t FBIndex, std::uint32_t InSlot, eShaderType InType);

	void ClearRTs(IGraphicsCommandContext* InCMDBuffer = nullptr);
};
