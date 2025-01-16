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

#include <vector>
#include "Engine/AbstractEngine.h"
#include "VulkanDevice.h"

class VulkanCompiledShader : public IShader
{
	sClassBody(sClassConstructor, VulkanCompiledShader, IShader)
public:
	VulkanCompiledShader(std::string InName, std::wstring InPath, eShaderType Type)
		: Name(InName)
		, Path(InPath)
		, ShaderType(Type)
	{
	}

	virtual ~VulkanCompiledShader()
	{
		//vkDestroyShaderModule(Owner->Get(), shader_module, nullptr);
	}

	virtual std::string GetName() const override final { return Name; }
	virtual std::wstring GetPath() const override final { return Path; }
	virtual eShaderType Type() const override final { return ShaderType; }
	virtual void* GetByteCode() const override final { return nullptr; }
	virtual std::uint32_t GetByteCodeSize() const override final { return 0; }

private:
	std::string Name;
	std::wstring Path;
	eShaderType ShaderType;
};

class VulkanShaderCompiler final
{
	sBaseClassBody(sClassConstructor, VulkanShaderCompiler)
private:
	VulkanDevice* Device;
	eShaderType ShaderType;
	std::string FunctionName;

public:
	VulkanShaderCompiler(VulkanDevice* InDevice)
		: Device(InDevice)
	{}

	VulkanCompiledShader::SharedPtr Compile(const sShaderAttachment& Attachment) 
	{
		return nullptr;
	}
	VulkanCompiledShader::SharedPtr Compile(std::wstring InSrcFile, std::string InFunctionName, eShaderType InProfile, std::vector<sShaderDefines> InDefines = std::vector<sShaderDefines>()) 
	{
		return nullptr;
	}
	VulkanCompiledShader::SharedPtr Compile(const void* InCode, std::size_t Size, std::string InFunctionName, eShaderType InProfile, std::vector<sShaderDefines> InDefines = std::vector<sShaderDefines>()) 
	{
		return nullptr;
	}

	virtual ~VulkanShaderCompiler() 
	{
		Device = nullptr;
	}

	std::vector<VkShaderModule> shader_modules;
	VkPipelineShaderStageCreateInfo shader_stage;

private:
	VulkanCompiledShader::SharedPtr CompileShaderFromFile(std::wstring InSrcFile, std::vector<sShaderDefines> InDefines = std::vector<sShaderDefines>())
	{
		return nullptr;
	}
	VulkanCompiledShader::SharedPtr CompileShader(const void* InCode, std::size_t Size, std::vector<sShaderDefines> InDefines = std::vector<sShaderDefines>())
	{
		return nullptr;
	}
};
