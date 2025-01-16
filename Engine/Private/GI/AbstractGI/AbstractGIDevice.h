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

#include "Engine/ClassBody.h"
#include "Engine/AbstractEngine.h"

class IAbstractGIDevice
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IAbstractGIDevice)
public:
	virtual void InitWindow(void* HWND, std::uint32_t Width, std::uint32_t Height, bool Fullscreen) = 0;

	virtual void* GetInternalDevice() = 0;
	virtual void Present(IRenderTarget* pRT) = 0;

	virtual void BeginFrame() = 0;

	virtual void ResizeWindow(std::size_t Width, std::size_t Height) = 0;
	virtual void FullScreen(const bool value) = 0;
	virtual bool IsFullScreen() const = 0;
	virtual void Vsync(const bool value) = 0;
	virtual bool IsVsyncEnabled() const = 0;
	virtual void VsyncInterval(const std::uint32_t value) = 0;
	virtual std::uint32_t GetVsyncInterval() const = 0;

	virtual EGITypes GetGIType() const = 0;
	virtual sGPUInfo GetGPUInfo() const = 0;

	virtual sScreenDimension GetBackBufferDimension() const = 0;
	virtual EFormat GetBackBufferFormat() const = 0;
	virtual sViewport GetViewport() const = 0;

	virtual IShader* CompileShader(const sShaderAttachment& Attachment, bool Spirv = false) = 0;
	virtual IShader* CompileShader(std::wstring InSrcFile, std::string InFunctionName, eShaderType InProfile, bool Spirv = false, std::vector<sShaderDefines> InDefines = std::vector<sShaderDefines>()) = 0;
	virtual IShader* CompileShader(const void* InCode, std::size_t Size, std::string InFunctionName, eShaderType InProfile, bool Spirv = false, std::vector<sShaderDefines> InDefines = std::vector<sShaderDefines>()) = 0;

	virtual std::vector<sDisplayMode> GetAllSupportedResolutions() const = 0;

	virtual IGraphicsCommandContext::SharedPtr CreateGraphicsCommandContext() = 0;
	virtual IGraphicsCommandContext::UniquePtr CreateUniqueGraphicsCommandContext() = 0;

	virtual IComputeCommandContext::SharedPtr CreateComputeCommandContext() = 0;
	virtual IComputeCommandContext::UniquePtr CreateUniqueComputeCommandContext() = 0;

	virtual ICopyCommandContext::SharedPtr CreateCopyCommandContext() = 0;
	virtual ICopyCommandContext::UniquePtr CreateUniqueCopyCommandContext() = 0;

	virtual IConstantBuffer::SharedPtr CreateConstantBuffer(std::string InName, const BufferLayout& InDesc, std::uint32_t InRootParameterIndex) = 0;
	virtual IConstantBuffer::UniquePtr CreateUniqueConstantBuffer(std::string InName, const BufferLayout& InDesc, std::uint32_t InRootParameterIndex) = 0;

	virtual IVertexBuffer::SharedPtr CreateVertexBuffer(std::string InName, const BufferLayout& InDesc, BufferSubresource* InSubresource = nullptr) = 0;
	virtual IVertexBuffer::UniquePtr CreateUniqueVertexBuffer(std::string InName, const BufferLayout& InDesc, BufferSubresource* InSubresource = nullptr) = 0;

	virtual IIndexBuffer::SharedPtr CreateIndexBuffer(std::string InName, const BufferLayout& InDesc, BufferSubresource* InSubresource = nullptr) = 0;
	virtual IIndexBuffer::UniquePtr CreateUniqueIndexBuffer(std::string InName, const BufferLayout& InDesc, BufferSubresource* InSubresource = nullptr) = 0;

	virtual IFrameBuffer::SharedPtr CreateFrameBuffer(const std::string InName, const sFrameBufferAttachmentInfo& InAttachments) = 0;
	virtual IFrameBuffer::UniquePtr CreateUniqueFrameBuffer(const std::string InName, const sFrameBufferAttachmentInfo& InAttachments) = 0;

	virtual IRenderTarget::SharedPtr CreateRenderTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc) = 0;
	virtual IRenderTarget::UniquePtr CreateUniqueRenderTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc) = 0;
	virtual IDepthTarget::SharedPtr CreateDepthTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc) = 0;
	virtual IDepthTarget::UniquePtr CreateUniqueDepthTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc) = 0;
	virtual IUnorderedAccessTarget::SharedPtr CreateUnorderedAccessTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc, bool InEnableSRV) = 0;
	virtual IUnorderedAccessTarget::UniquePtr CreateUniqueUnorderedAccessTarget(const std::string InName, const EFormat Format, const sFBODesc& Desc, bool InEnableSRV) = 0;

	virtual IPipeline::SharedPtr CreatePipeline(const std::string& InName, const sPipelineDesc& InDesc) = 0;
	virtual IPipeline::UniquePtr CreateUniquePipeline(const std::string& InName, const sPipelineDesc& InDesc) = 0;

	virtual IComputePipeline::SharedPtr CreateComputePipeline(const std::string& InName, const sComputePipelineDesc& InDesc) = 0;
	virtual IComputePipeline::UniquePtr CreateUniqueComputePipeline(const std::string& InName, const sComputePipelineDesc& InDesc) = 0;

	virtual ITexture2D::SharedPtr CreateTexture2D(const std::wstring FilePath, const std::string InName, std::uint32_t DefaultRootParameterIndex = 0) = 0;
	virtual ITexture2D::UniquePtr CreateUniqueTexture2D(const std::wstring FilePath, const std::string InName, std::uint32_t DefaultRootParameterIndex = 0) = 0;
	virtual ITexture2D::SharedPtr CreateTexture2D(const std::string InName, void* InBuffer, const std::size_t InSize, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex = 0) = 0;
	virtual ITexture2D::UniquePtr CreateUniqueTexture2D(const std::string InName, void* InBuffer, const std::size_t InSize, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex = 0) = 0;

	virtual ITexture2D::SharedPtr CreateEmptyTexture2D(const std::string InName, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex = 0) = 0;
	virtual ITexture2D::UniquePtr CreateUniqueEmptyTexture2D(const std::string InName, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex = 0) = 0;

	//virtual ITiledTexture::SharedPtr CreateTiledTexture(const std::string InName, const std::uint32_t InTileX, const std::uint32_t InTileY, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex = 0) = 0;
	//virtual ITiledTexture::UniquePtr CreateUniqueTiledTexture(const std::string InName, const std::uint32_t InTileX, const std::uint32_t InTileY, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex = 0) = 0;
};
