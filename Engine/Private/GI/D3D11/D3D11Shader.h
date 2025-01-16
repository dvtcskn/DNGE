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

#include <d3d11_1.h>
#include <d3d11shader.h>
#include "D3D11Device.h"
#include <vector>
#include "Engine/AbstractEngine.h"

#include "D3D11ShaderStates.h"

#include <wrl\client.h>
using namespace Microsoft::WRL;

#include <d3dcompiler.h>
#pragma comment( lib, "d3dcompiler.lib")

class D3D11CompiledShader : public IShader
{
	sClassBody(sClassConstructor, D3D11CompiledShader, IShader)
public:
	D3D11CompiledShader(D3D11Device* InDevice, std::string InName, std::wstring InPath, eShaderType Type, ComPtr<ID3DBlob> InByteCode)
		: Owner(InDevice)
		, Name(InName)
		, Path(InPath)
		, ShaderType(Type)
		, ByteCode(InByteCode)
		, Shader(nullptr)
		, bIsCompiled(false)
	{
	}

	virtual ~D3D11CompiledShader()
	{
		Shader = nullptr;
		ByteCode = nullptr;
	}

	void CreateD3D11Shader()
	{
		if (bIsCompiled)
			return;

		switch (ShaderType)
		{
		case eShaderType::Vertex:
		{
			ID3D11VertexShader* VShader;
			Owner->GetDevice()->CreateVertexShader(ByteCode->GetBufferPointer(), ByteCode->GetBufferSize(), NULL, &VShader);
			Shader = VShader;
			bIsCompiled = true;
			break;
		}
		case eShaderType::HULL:
		{
			ID3D11HullShader* HShader;
			Owner->GetDevice()->CreateHullShader(ByteCode->GetBufferPointer(), ByteCode->GetBufferSize(), NULL, &HShader);
			Shader = HShader;
			bIsCompiled = true;
			break;
		}
		case eShaderType::Domain:
		{
			ID3D11DomainShader* DShader;
			Owner->GetDevice()->CreateDomainShader(ByteCode->GetBufferPointer(), ByteCode->GetBufferSize(), NULL, &DShader);
			Shader = DShader;
			bIsCompiled = true;
			break;
		}
		case eShaderType::Pixel:
		{
			ID3D11PixelShader* PShader;
			Owner->GetDevice()->CreatePixelShader(ByteCode->GetBufferPointer(), ByteCode->GetBufferSize(), NULL, &PShader);
			Shader = PShader;
			bIsCompiled = true;
			break;
		}
		case eShaderType::Geometry:
		{
			ID3D11GeometryShader* GShader;
			Owner->GetDevice()->CreateGeometryShader(ByteCode->GetBufferPointer(), ByteCode->GetBufferSize(), NULL, &GShader);
			Shader = GShader;
			bIsCompiled = true;
			break;
		}
		case eShaderType::Compute:
		{
			ID3D11ComputeShader* CShader;
			Owner->GetDevice()->CreateComputeShader(ByteCode->GetBufferPointer(), ByteCode->GetBufferSize(), NULL, &CShader);
			Shader = CShader;
			bIsCompiled = true;
			break;
		}
		}
	}

	virtual std::string GetName() const override final { return Name; }
	virtual std::wstring GetPath() const override final { return Path; }
	virtual eShaderType Type() const override final { return ShaderType; }
	virtual void* GetByteCode() const override final { return ByteCode->GetBufferPointer(); }
	virtual std::uint32_t GetByteCodeSize() const override final { return ByteCode->GetBufferSize(); }

	FORCEINLINE ID3DBlob* GetD3DByteCode() { return ByteCode.Get(); }

	ComPtr<ID3D11DeviceChild> GetShader() const { return Shader; }

	bool IsCompiled() const { return bIsCompiled; }

private:
	D3D11Device* Owner;
	std::string Name;
	std::wstring Path;
	eShaderType ShaderType;
	ComPtr<ID3DBlob> ByteCode;
	ComPtr<ID3D11DeviceChild> Shader;
	bool bIsCompiled;
};

class D3D11ShaderCompiler final
{
	sBaseClassBody(sClassConstructor, D3D11ShaderCompiler)
private:
	D3D11Device* Owner;

public:
	D3D11ShaderCompiler(D3D11Device* InDevice);
	IShader::SharedPtr Compile(const sShaderAttachment& Attachment);
	IShader::SharedPtr Compile(std::wstring InSrcFile, std::string InFunctionName, eShaderType InProfile, std::vector<sShaderDefines> InDefines = std::vector<sShaderDefines>());
	IShader::SharedPtr Compile(const void* InCode, std::size_t Size, std::string InFunctionName, eShaderType InProfile, std::vector<sShaderDefines> InDefines = std::vector<sShaderDefines>());

	virtual ~D3D11ShaderCompiler() 
	{
		Owner  = nullptr;
	}

private:
	IShader::SharedPtr CompileShaderFromFile(std::wstring InSrcFile, std::string InFunctionName, eShaderType InProfile, std::vector<sShaderDefines> InDefines = std::vector<sShaderDefines>());
	IShader::SharedPtr CompileShader(const void* InCode, std::size_t Size, std::wstring InSrcFile, std::string InFunctionName, eShaderType InProfile, std::vector<sShaderDefines> InDefines = std::vector<sShaderDefines>());
};
