
#include "Engine/ClassBody.h"
#include "Engine/AbstractEngine.h"

//#include <d3d12.h>
#include <d3d12shader.h> 
#include <dxcapi.h>
#include "Utilities/Exception.h"
#include "AbstractGI/ShaderManager.h"

#include <wrl/client.h>
using namespace Microsoft::WRL;

class DXCCompiledShader : public IShader
{
	sClassBody(sClassConstructor, DXCCompiledShader, IShader)
public:
	DXCCompiledShader(std::string InName, std::wstring InPath, eShaderType Type, ComPtr<IDxcBlob> InBlob, ComPtr<ID3D12ShaderReflection> InShaderReflection)
		: Name(InName)
		, Path(InPath)
		, ShaderType(Type)
		, Blob(InBlob)
		, ShaderReflection(InShaderReflection)
	{}

	virtual ~DXCCompiledShader()
	{
		ShaderReflection = nullptr;
		Blob = nullptr;
	}

	virtual std::string GetName() const override final { return Name; }
	virtual std::wstring GetPath() const override final { return Path; }
	virtual eShaderType Type() const override final { return ShaderType; }
	virtual void* GetByteCode() const override final { return Blob->GetBufferPointer(); }
	virtual std::uint32_t GetByteCodeSize() const override final { return Blob->GetBufferSize(); }

	inline ComPtr<ID3D12ShaderReflection> GetShaderReflection() const { return ShaderReflection; }

private:
	std::string Name; 
	std::wstring Path;
	eShaderType ShaderType;
	ComPtr<IDxcBlob> Blob;
	ComPtr<ID3D12ShaderReflection> ShaderReflection;
};

class DXCShaderCompiler
{
	sBaseClassBody(sClassConstructor, DXCShaderCompiler)
public:
	DXCShaderCompiler();
	virtual ~DXCShaderCompiler();

	IShader::SharedPtr Compile(const sShaderAttachment& Attachment, bool Spirv = false);
	IShader::SharedPtr Compile(std::wstring InSrcFile, std::string InFunctionName, eShaderType InProfile, bool Spirv = false, std::vector<sShaderDefines> InDefines = std::vector<sShaderDefines>());
	IShader::SharedPtr Compile(const void* InCode, std::size_t Size, std::string InFunctionName, eShaderType InProfile, bool Spirv = false, std::vector<sShaderDefines> InDefines = std::vector<sShaderDefines>());

private:
	IShader::SharedPtr CompileShader(const void* InCode, std::size_t Size, std::wstring Path, std::string InFunctionName, eShaderType InProfile, bool Spirv = false, std::vector<sShaderDefines> InDefines = std::vector<sShaderDefines>());

private:
	ComPtr<IDxcUtils> pUtils;
	ComPtr<IDxcCompiler3> pCompiler;
	ComPtr<IDxcIncludeHandler> IncludeHandler;
};
