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

#include "pch.h"
#include <stdexcept>
#include <iostream>
#include "D3D11Shader.h"

D3D11ShaderCompiler::D3D11ShaderCompiler(D3D11Device* InDevice)
	: Owner(InDevice)
{
}

IShader::SharedPtr D3D11ShaderCompiler::Compile(const sShaderAttachment& Attachment)
{
	if (!Attachment.IsCodeValid())
		return CompileShaderFromFile(Attachment.GetLocation(), Attachment.FunctionName, Attachment.Type, Attachment.ShaderDefines);
	
	return CompileShader(Attachment.GetByteCode(), Attachment.GetByteCodeSize(), L"",  Attachment.FunctionName, Attachment.Type, Attachment.ShaderDefines);
}

IShader::SharedPtr D3D11ShaderCompiler::Compile(std::wstring InSrcFile, std::string InFunctionName, eShaderType InProfile, std::vector<sShaderDefines> InDefines)
{
	return CompileShaderFromFile(InSrcFile, InFunctionName, InProfile, InDefines);
}

IShader::SharedPtr D3D11ShaderCompiler::Compile(const void* InCode, std::size_t Size, std::string InFunctionName, eShaderType InProfile, std::vector<sShaderDefines> InDefines)
{
	return CompileShader(InCode, Size, L"", InFunctionName, InProfile, InDefines);
}

IShader::SharedPtr D3D11ShaderCompiler::CompileShaderFromFile(std::wstring InSrcFile, std::string InFunctionName, eShaderType InProfile, std::vector<sShaderDefines> InDefines)
{
	std::uint32_t shaderFlags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;

	ComPtr<ID3DBlob> ByteCode = nullptr;

#if _DEBUG
	shaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	shaderFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_PARTIAL_PRECISION;
#endif

	auto Type = [&](eShaderType InType) -> LPCSTR
	{
		switch (InType)
		{
			case eShaderType::Vertex:   return "vs_5_0";
			case eShaderType::HULL:     return "hs_5_0";
			case eShaderType::Domain:   return "ds_5_0";
			case eShaderType::Pixel:	return "ps_5_0";
			case eShaderType::Geometry: return "gs_5_0";
			case eShaderType::Compute:  return "cs_5_0";
		}
		return " ";
	};

	D3D_SHADER_MACRO* MacroDefines = nullptr;
	if (InDefines.size() > 0)
	{
		MacroDefines = new D3D_SHADER_MACRO[InDefines.size() + 1];

		for (size_t i = 0; i < InDefines.size(); ++i)
			MacroDefines[i] = { InDefines[i].Name.data(), InDefines[i].Definition.data() };

		MacroDefines[InDefines.size()] = { nullptr, nullptr };
	}

	ID3DBlob *errors = 0;
	HRESULT hr = D3DCompileFromFile(InSrcFile.c_str(), MacroDefines, D3D_COMPILE_STANDARD_FILE_INCLUDE, InFunctionName.c_str(), Type(InProfile), shaderFlags, 0, &ByteCode, &errors);

	if (MacroDefines)
		delete MacroDefines;

	MacroDefines = nullptr;

	if (FAILED(hr)) 
	{
		std::string Error = "";
		if (errors)
			Error = (static_cast<char*>(errors->GetBufferPointer()));
		std::cout << Error << std::endl;

		throw std::runtime_error("Error compiling shader");
	}

	return D3D11CompiledShader::Create(Owner, InFunctionName, InSrcFile, InProfile, ByteCode);
}

IShader::SharedPtr D3D11ShaderCompiler::CompileShader(const void* InCode, std::size_t Size, std::wstring InSrcFile, std::string InFunctionName, eShaderType InProfile, std::vector<sShaderDefines> InDefines)
{
	std::uint32_t shaderFlags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;
	
	ComPtr<ID3DBlob> ByteCode = nullptr;

#if _DEBUG
	shaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	shaderFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_PARTIAL_PRECISION;
#endif

	auto Type = [&](eShaderType InType) -> LPCSTR
	{
		switch (InType)
		{
			case eShaderType::Vertex:   return "vs_5_0";
			case eShaderType::HULL:     return "hs_5_0";
			case eShaderType::Domain:   return "ds_5_0";
			case eShaderType::Pixel:	return "ps_5_0";
			case eShaderType::Geometry: return "gs_5_0";
			case eShaderType::Compute:  return "cs_5_0";
		}
		return " ";
	};

	D3D_SHADER_MACRO* MacroDefines = nullptr;
	if (InDefines.size() > 0)
	{
		MacroDefines = new D3D_SHADER_MACRO[InDefines.size() + 1];

		for (size_t i = 0; i < InDefines.size(); ++i)
			MacroDefines[i] = { InDefines[i].Name.data(), InDefines[i].Definition.data() };

		MacroDefines[InDefines.size()] = { nullptr, nullptr };
	}

	ID3DBlob *errors = 0;
	HRESULT hr = D3DCompile(InCode, Size, NULL, MacroDefines, D3D_COMPILE_STANDARD_FILE_INCLUDE, InFunctionName.c_str(), Type(InProfile), shaderFlags, 0, &ByteCode, &errors);

	if (MacroDefines)
		delete MacroDefines;

	MacroDefines = nullptr;

	if (FAILED(hr))
	{
		std::string Error = "";
		if (errors)
			Error = (static_cast<char*>(errors->GetBufferPointer()));

		std::cout << Error << std::endl;

		throw std::runtime_error("Error compiling shader");
	}

	return D3D11CompiledShader::Create(Owner, InFunctionName, InSrcFile, InProfile, ByteCode);
}
