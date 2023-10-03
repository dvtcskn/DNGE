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

#include <vector>
#include "Engine/ClassBody.h"
#include "Engine/AbstractEngine.h"

class VulkanRasterizer final
{
	sBaseClassBody(sClassConstructor, VulkanRasterizer)
public:
	VulkanRasterizer(sRasterizerAttributeDesc InDesc)
		: RasterizerDesc(InDesc)
	{
	}

	virtual	~VulkanRasterizer() 
	{
	}

private:
	sRasterizerAttributeDesc RasterizerDesc;
};

class VulkanSamplerState
{
	sBaseClassBody(sClassConstructor, VulkanSamplerState)
public:
	VulkanSamplerState(sSamplerAttributeDesc InDesc)
		: SamplerDesc(InDesc)
	{
	}

	virtual ~VulkanSamplerState()
	{
	}

private:
	sSamplerAttributeDesc SamplerDesc;
};

class VulkanDepthStencilState
{
	sBaseClassBody(sClassConstructor, VulkanDepthStencilState)
public:
	VulkanDepthStencilState(sDepthStencilAttributeDesc InDesc)
		: DepthStencilStateDesc(InDesc)
	{
	}

	virtual ~VulkanDepthStencilState()
	{
	}

private:
	sDepthStencilAttributeDesc DepthStencilStateDesc;
};

class VulkanBlendState
{
	sBaseClassBody(sClassConstructor, VulkanBlendState)
public:
	VulkanBlendState(sBlendAttributeDesc InDesc)
		: BlendStateDesc(InDesc)
	{
	}

	virtual ~VulkanBlendState()
	{
	}

private:
	sBlendAttributeDesc BlendStateDesc;
};

class VulkanVertexAttribute
{
public:
	VulkanVertexAttribute(std::vector<sVertexAttributeDesc> InDesc, void* InShaderCode = 0);

	virtual ~VulkanVertexAttribute()
	{
	}

private:
	std::vector<sVertexAttributeDesc> VertexAttributeDescData;
};
