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
#include "D3D12Device.h"
#include "Engine/AbstractEngine.h"

#include <d3d12shader.h>
#include <d3dcompiler.h>
#pragma comment( lib, "d3dcompiler.lib")

#include <wrl/client.h>
using namespace Microsoft::WRL;

class D3D12CompiledShader : public IShader
{
	sClassBody(sClassConstructor, D3D12CompiledShader, IShader)
public:
	D3D12CompiledShader(std::string InName, std::wstring InPath, eShaderType Type, ComPtr<ID3DBlob> InByteCode)
		: Name(InName)
		, Path(InPath)
		, ShaderType(Type)
		, ByteCode(InByteCode)
	{
	}

	virtual ~D3D12CompiledShader()
	{
		ByteCode = nullptr;
	}

	virtual std::string GetName() const override final { return Name; }
	virtual std::wstring GetPath() const override final { return Path; }
	virtual eShaderType Type() const override final { return ShaderType; }
	virtual void* GetByteCode() const override final { return ByteCode->GetBufferPointer(); }
	virtual std::uint32_t GetByteCodeSize() const override final { return ByteCode->GetBufferSize(); }

private:
	std::string Name;
	std::wstring Path;
	eShaderType ShaderType;
	ComPtr<ID3DBlob> ByteCode;
};

class D3D12ShaderCompiler final
{
	sBaseClassBody(sClassConstructor, D3D12ShaderCompiler)
public:
	D3D12ShaderCompiler();
	virtual ~D3D12ShaderCompiler();

	D3D12CompiledShader::SharedPtr Compile(const sShaderAttachment& Attachment);
	D3D12CompiledShader::SharedPtr Compile(std::wstring InSrcFile, std::string InFunctionName, eShaderType InProfile, std::vector<sShaderDefines> InDefines = std::vector<sShaderDefines>());
	D3D12CompiledShader::SharedPtr Compile(const void* InCode, std::size_t Size, std::string InFunctionName, eShaderType InProfile, std::vector<sShaderDefines> InDefines = std::vector<sShaderDefines>());

private:
	D3D12CompiledShader::SharedPtr CompileShaderFromFile(std::wstring InSrcFile, std::string InFunctionName, eShaderType InProfile, std::vector<sShaderDefines> InDefines = std::vector<sShaderDefines>());
	D3D12CompiledShader::SharedPtr CompileShader(const void* InCode, std::size_t Size, std::wstring InSrcFile, std::string InFunctionName, eShaderType InProfile, std::vector<sShaderDefines> InDefines = std::vector<sShaderDefines>());
};
