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
#include <assert.h>
#include "Engine/ClassBody.h"
#include "Engine/AbstractEngine.h"
#include "VulkanShaderStates.h"
#include "VulkanDevice.h"

class VulkanUploadBuffer final
{
	sBaseClassBody(sClassConstructor, VulkanUploadBuffer)
private:
	std::string Name;
	VulkanDevice* Owner;
	VkBuffer Buffer;
	VkDeviceMemory Memory;
	void* pData;
	std::uint32_t Size;
	bool bIsMapped;

public:
	VulkanUploadBuffer(VulkanDevice* Device, std::string InName, const std::uint32_t Size);

	virtual ~VulkanUploadBuffer();

	FORCEINLINE std::string GetName() const { return Name; };

	std::size_t GetSize() const { return Size; }
	void* GetData() const { return pData; }
	VkBuffer GetBuffer() const { return Buffer; }
	void Map(const void* Ptr = nullptr, IGraphicsCommandContext* InCMDBuffer = nullptr);
	void Unmap();
};

class VulkanConstantBuffer final : public IConstantBuffer
{
	sClassBody(sClassConstructor, VulkanConstantBuffer, IConstantBuffer)
private:
	std::uint32_t RootParameterIndex;
	std::string Name;
	VulkanDevice* Owner;
	VkBuffer Buffer;
	VkDeviceMemory Memory;
	BufferLayout Desc;
	void* pData;
	bool bIsMapped;

public:
	VulkanConstantBuffer(VulkanDevice* Device, std::string InName, const BufferLayout& InDesc, std::uint32_t InRootParameterIndex);

	virtual ~VulkanConstantBuffer();

	FORCEINLINE virtual std::string GetName() const override final { return Name; };

	VkBuffer GetBuffer() const { return Buffer; }
	void* GetData() const { return pData; }
	virtual void SetDefaultRootParameterIndex(std::uint32_t inRootParameterIndex) override final { RootParameterIndex = inRootParameterIndex; }
	virtual std::uint32_t GetDefaultRootParameterIndex() const override final { return RootParameterIndex; }

	virtual void Map(const void* Ptr, IGraphicsCommandContext* InCMDBuffer = nullptr) override final;
	void Unmap();

	VkDescriptorBufferInfo DescriptorInfo;
	std::uint32_t GetSize() const;
};

class VulkanVertexBuffer final : public IVertexBuffer
{
	sClassBody(sClassConstructor, VulkanVertexBuffer, IVertexBuffer)
private:
	std::string Name;
	VulkanDevice* Owner;
	VkBuffer Buffer;
	VkDeviceMemory Memory;
	BufferLayout Desc;
	VulkanUploadBuffer::UniquePtr UploadBuffer;

public:
	VulkanVertexBuffer(VulkanDevice* Device, std::string InName, const BufferLayout& InDesc, BufferSubresource* InSubresource = nullptr);

	virtual ~VulkanVertexBuffer();

	FORCEINLINE virtual std::string GetName() const final override { return Name; };

	VkBuffer GetBuffer() const { return Buffer; }
	virtual std::size_t GetSize() const override final { return Desc.Size; }
	std::size_t Stride() const { return Desc.Stride; }
	virtual bool IsMapable() const override final { return false; }
	virtual void UpdateSubresource(BufferSubresource* Subresource, IGraphicsCommandContext* InCMDBuffer = nullptr) override final;

	bool bIsMapped;
	void* pData;
	void Map(const void* Ptr = nullptr, IGraphicsCommandContext* InCMDBuffer = nullptr);
	void Unmap();
};

class VulkanIndexBuffer final : public IIndexBuffer
{
	sClassBody(sClassConstructor, VulkanIndexBuffer, IIndexBuffer)
private:
	std::string Name;
	VulkanDevice* Owner;
	VkBuffer Buffer;
	VkDeviceMemory Memory;
	BufferLayout Desc;
	VulkanUploadBuffer::UniquePtr UploadBuffer;

public:
	VulkanIndexBuffer(VulkanDevice* Device, std::string InName, const BufferLayout& InDesc, BufferSubresource* InSubresource = nullptr);

	virtual ~VulkanIndexBuffer();

	FORCEINLINE virtual std::string GetName() const override final { return Name; };

	VkBuffer GetBuffer() const { return Buffer; }
	virtual std::size_t GetSize() const override final { return Desc.Size; }
	virtual bool IsMapable() const override final { return false; }
	virtual void UpdateSubresource(BufferSubresource* Subresource, IGraphicsCommandContext* InCMDBuffer = nullptr) override final;

	bool bIsMapped;
	void* pData;
	void Map(const void* Ptr = nullptr, IGraphicsCommandContext* InCMDBuffer = nullptr);
	void Unmap();
};

class VulkanUnorderedAccessBuffer final : public IUnorderedAccessBuffer
{
	sClassBody(sClassConstructor, VulkanUnorderedAccessBuffer, IUnorderedAccessBuffer)
private:
	std::string Name;

public:
	VulkanUnorderedAccessBuffer(VulkanDevice* InDevice, std::string InName, const BufferLayout& InDesc, bool bSRVAllowed = true);

	virtual ~VulkanUnorderedAccessBuffer()
	{

	}

	FORCEINLINE virtual std::string GetName() const override final { return Name; };

	virtual bool IsSRV_Allowed() const { return false; }

	VkBuffer GetBuffer() const { return VK_NULL_HANDLE; }
	virtual std::size_t GetSize() const override final { return 0; }
	virtual bool IsMapable() const override final { return false; }
	virtual void Map(const void* Ptr, IGraphicsCommandContext* InCMDBuffer = nullptr) override final;
};
