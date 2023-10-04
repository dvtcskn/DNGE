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

class D3D11Buffer
{
protected:
	enum class ResourceTypeFlags : std::uint8_t
	{
		eDEFAULT,
		eVERTEX_BUFFER,
		eINDEX_BUFFER,
		eCONSTANT_BUFFER,
		eUnorderedAccess_BUFFER,
	};

private:
	D3D11Device* Owner;

protected:
	ComPtr<ID3D11Buffer> Buffer;
	sBufferDesc BufferDesc;

protected:
	D3D11Buffer(D3D11Device* InDevice, const sBufferDesc& InDesc, sBufferSubresource* InSubresource = NULL, ResourceTypeFlags InType = ResourceTypeFlags::eDEFAULT, bool InDynamic = false);

public:
	using SharedPtr = std::shared_ptr<D3D11Buffer>;
	
	virtual ~D3D11Buffer() {
		Release();
	}

	void Release()
	{
		Buffer = nullptr;
		Owner = nullptr;
	}

	D3D11Device* GetOwner() const { return Owner; }

	bool IsMapable() const;

	FORCEINLINE ComPtr<ID3D11Buffer> GetBuffer() { return Buffer; }
	FORCEINLINE sBufferDesc GetBufferDesc() { return BufferDesc; }

	virtual void ResizeBuffer(std::size_t Size, sBufferSubresource* Subresource = nullptr);
};

class D3D11ConstantBuffer final : public D3D11Buffer, public IConstantBuffer
{
	sClassBody(sClassConstructor, D3D11ConstantBuffer, IConstantBuffer)
private:
	std::uint32_t RootParameterIndex;
	std::string Name;

public:
	D3D11ConstantBuffer(D3D11Device* InDevice, std::string InName, const sBufferDesc& InDesc, std::uint32_t InRootParameterIndex)
		: D3D11Buffer(InDevice, InDesc, NULL, ResourceTypeFlags::eCONSTANT_BUFFER)
		, RootParameterIndex(InRootParameterIndex)
		, Name(InName)
	{}

	virtual ~D3D11ConstantBuffer() = default;

	FORCEINLINE virtual std::string GetName() const final override { return Name; };

	virtual void SetDefaultRootParameterIndex(std::uint32_t inRootParameterIndex) override final { RootParameterIndex = inRootParameterIndex; }
	virtual std::uint32_t GetDefaultRootParameterIndex() const override final { return RootParameterIndex; }
	void ApplyConstantBuffer(ID3D11DeviceContext1* CMD, std::uint32_t InSlot, eShaderType InType = eShaderType::Pixel);

	virtual void Map(const void* Ptr, IGraphicsCommandContext* InCMDBuffer = nullptr) final override;
};

class D3D11VertexBuffer final : public D3D11Buffer, public IVertexBuffer
{
	sClassBody(sClassConstructor, D3D11VertexBuffer, IVertexBuffer)
private:
	std::string Name;

public:
	D3D11VertexBuffer(D3D11Device* InDevice, std::string InName, const sBufferDesc& InDesc, sBufferSubresource* InSubresource = nullptr)
		: D3D11Buffer(InDevice, InDesc, InSubresource, ResourceTypeFlags::eVERTEX_BUFFER)
		, Name(InName)
	{}

	virtual ~D3D11VertexBuffer() = default;

	FORCEINLINE virtual std::string GetName() const final override { return Name; };

	virtual std::size_t GetSize() const final override { return BufferDesc.Size; }
	virtual bool IsMapable() const final override { return D3D11Buffer::IsMapable(); }
	virtual void UpdateSubresource(sBufferSubresource* Subresource, IGraphicsCommandContext* InCMDBuffer = nullptr) final override;
	void UpdateSubresource(sBufferSubresource* Subresource, ID3D11DeviceContext1* Device = nullptr);
	void ApplyBuffer(IGraphicsCommandContext* InCMDBuffer = nullptr);

	void ResizeBuffer(std::size_t Size, sBufferSubresource* Subresource = nullptr)
	{
		D3D11Buffer::ResizeBuffer(Size, Subresource);
	}
};

class D3D11IndexBuffer final : public D3D11Buffer, public IIndexBuffer
{
	sClassBody(sClassConstructor, D3D11IndexBuffer, IIndexBuffer)
private:
	std::string Name;

public:
	D3D11IndexBuffer(D3D11Device* InDevice, std::string InName, const sBufferDesc& InDesc, sBufferSubresource* InSubresource = nullptr)
		: D3D11Buffer(InDevice, InDesc, InSubresource, ResourceTypeFlags::eINDEX_BUFFER)
		, Name(InName)
	{}

	virtual ~D3D11IndexBuffer() = default;

	FORCEINLINE virtual std::string GetName() const final override { return Name; };

	virtual std::size_t GetSize() const final override { return BufferDesc.Size; }
	virtual bool IsMapable() const final override { return D3D11Buffer::IsMapable(); }
	virtual void UpdateSubresource(sBufferSubresource* Subresource, IGraphicsCommandContext* InCMDBuffer = nullptr) final override;
	void UpdateSubresource(sBufferSubresource* Subresource, ID3D11DeviceContext1* Device = nullptr);
	void ApplyBuffer(IGraphicsCommandContext* InCMDBuffer = nullptr);

	void ResizeBuffer(std::size_t Size, sBufferSubresource* Subresource = nullptr)
	{
		D3D11Buffer::ResizeBuffer(Size, Subresource);
	}
};
