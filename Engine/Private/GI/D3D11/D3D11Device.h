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

#include <memory>
#include <d3d11.h>
#include <d3d11_1.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
#include <string>
#include "GI/AbstractGI/AbstractGIDevice.h"

using namespace Microsoft::WRL;

class D3D11Viewport;

class D3D11Device final : public IAbstractGIDevice
{
	sClassBody(sClassConstructor, D3D11Device, IAbstractGIDevice)
public:
	D3D11Device(std::optional<short> InGPUIndex = std::nullopt);
	virtual ~D3D11Device();
	virtual void InitWindow(void* HWND, std::uint32_t Width, std::uint32_t Height, bool Fullscreen) override final;
	virtual void Present(IRenderTarget* pRT) override final;
	bool GetDeviceIdentification(std::wstring& InVendorID, std::wstring& InDeviceID);
	IDXGIAdapter1* GetAdapter(std::optional<short> Index = std::nullopt);

	virtual void* GetInternalDevice() override final { return Direct3DDevice.Get(); }

	virtual void ResizeWindow(std::size_t Width, std::size_t Height) override final;
	virtual void FullScreen(const bool value) override final;
	virtual void Vsync(const bool value) override final;
	virtual void VsyncInterval(const std::uint32_t value) override final;

	virtual bool IsFullScreen() const override final;
	virtual bool IsVsyncEnabled() const override final;
	virtual std::uint32_t GetVsyncInterval() const override final;

	virtual std::vector<sDisplayMode> GetAllSupportedResolutions() const override final;

	virtual EGITypes GetGIType() const override final { return EGITypes::eD3D11; }
	virtual sGPUInfo GetGPUInfo() const override final { return sGPUInfo(); }

	virtual sScreenDimension GetBackBufferDimension() const override final;
	virtual EFormat GetBackBufferFormat() const override final;
	virtual sViewport GetViewport() const override final;

	virtual IGraphicsCommandContext::SharedPtr CreateGraphicsCommandContext() const override final;
	virtual IGraphicsCommandContext::UniquePtr CreateUniqueGraphicsCommandContext() const override final;

	virtual IComputeCommandContext::SharedPtr CreateComputeCommandContext() const override final;
	virtual IComputeCommandContext::UniquePtr CreateUniqueComputeCommandContext() const override final;

	virtual ICopyCommandContext::SharedPtr CreateCopyCommandContext() const override final;
	virtual ICopyCommandContext::UniquePtr CreateUniqueCopyCommandContext() const override final;

	virtual IConstantBuffer::SharedPtr CreateConstantBuffer(std::string InName, const sBufferDesc& InDesc, std::uint32_t InRootParameterIndex) const override final;
	virtual IConstantBuffer::UniquePtr CreateUniqueConstantBuffer(std::string InName, const sBufferDesc& InDesc, std::uint32_t InRootParameterIndex) const override final;

	virtual IVertexBuffer::SharedPtr CreateVertexBuffer(std::string InName, const sBufferDesc& InDesc, sBufferSubresource* InSubresource = nullptr) const override final;
	virtual IVertexBuffer::UniquePtr CreateUniqueVertexBuffer(std::string InName, const sBufferDesc& InDesc, sBufferSubresource* InSubresource = nullptr) const override final;

	virtual IIndexBuffer::SharedPtr CreateIndexBuffer(std::string InName, const sBufferDesc& InDesc, sBufferSubresource* InSubresource = nullptr) const override final;
	virtual IIndexBuffer::UniquePtr CreateUniqueIndexBuffer(std::string InName, const sBufferDesc& InDesc, sBufferSubresource* InSubresource = nullptr) const override final;

	virtual IFrameBuffer::SharedPtr CreateFrameBuffer(const std::string InName, const sFrameBufferAttachmentInfo& InAttachments) const override final;
	virtual IFrameBuffer::UniquePtr CreateUniqueFrameBuffer(const std::string InName, const sFrameBufferAttachmentInfo& InAttachments) const override final;

	virtual IRenderTarget::SharedPtr CreateRenderTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc) override final;
	virtual IRenderTarget::UniquePtr CreateUniqueRenderTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc) override final;
	virtual IDepthTarget::SharedPtr CreateDepthTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc) override final;
	virtual IDepthTarget::UniquePtr CreateUniqueDepthTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc) override final;
	virtual IUnorderedAccessTarget::SharedPtr CreateUnorderedAccessTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc, bool InEnableSRV) override final;
	virtual IUnorderedAccessTarget::UniquePtr CreateUniqueUnorderedAccessTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc, bool InEnableSRV) override final;

	virtual IPipeline::SharedPtr CreatePipeline(const std::string& InName, const sPipelineDesc& InDesc) const override final;
	virtual IPipeline::UniquePtr CreateUniquePipeline(const std::string& InName, const sPipelineDesc& InDesc) const override final;

	virtual IComputePipeline::SharedPtr CreateComputePipeline(const std::string& InName, const sComputePipelineDesc& InDesc) const override final;
	virtual IComputePipeline::UniquePtr CreateUniqueComputePipeline(const std::string& InName, const sComputePipelineDesc& InDesc) const override final;

	virtual ITexture2D::SharedPtr CreateTexture2D(const std::wstring FilePath, const std::string InName, std::uint32_t DefaultRootParameterIndex = 0) const override final;
	virtual ITexture2D::UniquePtr CreateUniqueTexture2D(const std::wstring FilePath, const std::string InName, std::uint32_t DefaultRootParameterIndex = 0) const override final;
	virtual ITexture2D::SharedPtr CreateTexture2D(const std::string InName, void* InBuffer, const std::size_t InSize, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex = 0) const override final;
	virtual ITexture2D::UniquePtr CreateUniqueTexture2D(const std::string InName, void* InBuffer, const std::size_t InSize, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex = 0) const override final;

	virtual ITexture2D::SharedPtr CreateEmptyTexture2D(const std::string InName, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex = 0) const override final;
	virtual ITexture2D::UniquePtr CreateUniqueEmptyTexture2D(const std::string InName, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex = 0) const override final;

	//virtual ITiledTexture::SharedPtr CreateTiledTexture(const std::string InName, const std::uint32_t InTileX, const std::uint32_t InTileY, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex = 0) const override final;
	//virtual ITiledTexture::UniquePtr CreateUniqueTiledTexture(const std::string InName, const std::uint32_t InTileX, const std::uint32_t InTileY, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex = 0) const override final;

private:
	std::optional<short> GPUIndex;
	ComPtr<ID3D11Device1> Direct3DDevice;
	ComPtr<ID3D11DeviceContext1> Direct3DDeviceIMContext;
	ComPtr<IDXGIFactory4> DXGIFactory;
	std::unique_ptr<D3D11Viewport> Viewport;
	std::uint32_t VendorId;

public:
	FORCEINLINE ID3D11Device1* Get() const
	{
		return Direct3DDevice.Get();
	}
	FORCEINLINE ID3D11Device1* GetDevice() const
	{
		return Direct3DDevice.Get();
	}
	FORCEINLINE ID3D11DeviceContext1* GetDeviceIMContext() const
	{
		return Direct3DDeviceIMContext.Get();
	}

	FORCEINLINE bool IsNvDeviceID() const
	{
		return VendorId == 0x10DE;
	}

	FORCEINLINE bool IsAMDDeviceID() const
	{
		return VendorId == 0x1002;
	}

	FORCEINLINE bool IsIntelDeviceID() const
	{
		return VendorId == 0x8086;
	}

	FORCEINLINE bool IsSoftwareDevice() const
	{
		return VendorId == 0x1414;
	}

	FORCEINLINE ComPtr<IDXGIFactory4> GetFactory() const
	{
		return DXGIFactory;
	}

	D3D11Viewport* GetViewportContext() const;
};
