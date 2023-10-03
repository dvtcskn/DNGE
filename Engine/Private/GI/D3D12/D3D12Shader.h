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

#include <string>
#include <d3d12shader.h>
#include "D3D12Device.h"
#include "Engine/AbstractEngine.h"

#include <d3dcompiler.h>
#pragma comment( lib, "d3dcompiler.lib")

class D3D12Shader final
{
	sBaseClassBody(sClassConstructor, D3D12Shader)
private:
	eShaderType ShaderType;
	ComPtr<ID3DBlob> ByteCode;
	std::string FunctionName;

public:
	D3D12Shader(const sShaderAttachment& Attachment);
	D3D12Shader(std::wstring InSrcFile, std::string InFunctionName, eShaderType InProfile, std::vector<sShaderDefines> InDefines = std::vector<sShaderDefines>());
	D3D12Shader(const void* InCode, std::size_t Size, std::string InFunctionName, eShaderType InProfile, std::vector<sShaderDefines> InDefines = std::vector<sShaderDefines>());

	virtual ~D3D12Shader();

	ComPtr<ID3DBlob> GetBlob() const { return ByteCode; }
	void* GetByteCode() const { return ByteCode->GetBufferPointer(); }
	std::uint32_t GetByteCodeSize() const { return (std::uint32_t)ByteCode->GetBufferSize(); }

private:
	void CompileShaderFromFile(std::wstring InSrcFile, std::vector<sShaderDefines> InDefines = std::vector<sShaderDefines>());
	void CompileShader(const void* InCode, std::size_t Size, std::vector<sShaderDefines> InDefines = std::vector<sShaderDefines>());
};
