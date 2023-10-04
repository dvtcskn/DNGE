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

#include <string>
#include <vector>
#include <d3d11.h>
#include <d3d12.h>
#include <assert.h>
#include <wrl/client.h>
#include "Engine/ClassBody.h"
#include "Engine/AbstractEngine.h"
#include "D3D11Shader.h"

using namespace Microsoft::WRL;

class VulkanConstantBuffer final : public IConstantBuffer
{
	sClassBody(sClassConstructor, VulkanConstantBuffer, IConstantBuffer)
private:
	std::uint32_t RootParameterIndex;
	std::string Name;

public:
	VulkanConstantBuffer(std::string InName, const sBufferDesc& InDesc, std::uint32_t InRootParameterIndex)
		: RootParameterIndex(InRootParameterIndex)
		, Name(InName)
	{}

	virtual ~VulkanConstantBuffer() = default;

	FORCEINLINE virtual std::string GetName() const final override { return Name; };

	virtual void SetDefaultRootParameterIndex(std::uint32_t inRootParameterIndex) override final { RootParameterIndex = inRootParameterIndex; }
	virtual std::uint32_t GetDefaultRootParameterIndex() const override final { return RootParameterIndex; }

	virtual void Map(const void* Ptr, IGraphicsCommandContext* InCMDBuffer = nullptr) final override;
};

class VulkanVertexBuffer final : public IVertexBuffer
{
	sClassBody(sClassConstructor, VulkanVertexBuffer, IVertexBuffer)
private:
	std::string Name;

public:
	VulkanVertexBuffer(std::string InName, const sBufferDesc& InDesc, sBufferSubresource* InSubresource = nullptr)
		: Name(InName)
	{}

	virtual ~VulkanVertexBuffer() = default;

	FORCEINLINE virtual std::string GetName() const final override { return Name; };

	virtual std::size_t GetSize() const final override { return 0; }
	virtual bool IsMapable() const final override { return false; }
	virtual void UpdateSubresource(sBufferSubresource* Subresource, IGraphicsCommandContext* InCMDBuffer = nullptr) final override;
};

class VulkanIndexBuffer final : public IIndexBuffer
{
	sClassBody(sClassConstructor, VulkanIndexBuffer, IIndexBuffer)
private:
	std::string Name;

public:
	VulkanIndexBuffer(std::string InName, const sBufferDesc& InDesc, sBufferSubresource* InSubresource = nullptr)
		: Name(InName)
	{}

	virtual ~VulkanIndexBuffer() = default;

	FORCEINLINE virtual std::string GetName() const final override { return Name; };

	virtual std::size_t GetSize() const final override { return 0; }
	virtual bool IsMapable() const final override { return false; }
	virtual void UpdateSubresource(sBufferSubresource* Subresource, IGraphicsCommandContext* InCMDBuffer = nullptr) final override;
};
