
#include "pch.h"
#include "ShaderCompiler.h"
#include "Utilities/FileManager.h"
#include <ranges>
#include <unordered_set>

// Include handler library to collect all included files for tracking
class FileTrackingIncludeHandler : public IDxcIncludeHandler
{
public:
    FileTrackingIncludeHandler(DXCShaderCompiler& parent) 
        : parent_(parent)
    {}

    HRESULT STDMETHODCALLTYPE LoadSource(_In_ LPCWSTR                             pFilename,
        _COM_Outptr_result_maybenull_ IDxcBlob** ppIncludeSource) override
    {
        //if (pFilename == nullptr) {
        //    return E_FAIL;
        //}
        //if (ppIncludeSource == nullptr) {
        //    return E_FAIL;
        //}

        //const auto shaderSourceFilePath = parent_.GetShaderSourceFilePath(pFilename);

        //IDxcBlobEncoding* includeSource;
        //const auto result = parent_.utils_->LoadFile(shaderSourceFilePath.wstring().c_str(), nullptr, &includeSource);

        //*ppIncludeSource = includeSource;

        //if (SUCCEEDED(result)) {
        //    // Update/insert last file write time for hot-reloading
        //    parent_.trackedFiles_[shaderSourceFilePath] = std::filesystem::last_write_time(shaderSourceFilePath);
        //}

        //return result;
        return E_FAIL;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override
    {
        return E_FAIL;
    }

    ULONG STDMETHODCALLTYPE AddRef(void) override
    {
        return 0;
    }

    ULONG STDMETHODCALLTYPE Release(void) override
    {
        return 0;
    }

private:
    DXCShaderCompiler& parent_;
};

DXCShaderCompiler::DXCShaderCompiler()
{
    ThrowIfFailed(::DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils)));
    ThrowIfFailed(::DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler)));
    ThrowIfFailed(pUtils->CreateDefaultIncludeHandler(&IncludeHandler));
}

DXCShaderCompiler::~DXCShaderCompiler()
{
    pUtils = nullptr;
    pCompiler = nullptr;
    IncludeHandler = nullptr;
}

IShader::SharedPtr DXCShaderCompiler::Compile(const sShaderAttachment& Attachment, bool Spirv)
{
    if (!Attachment.IsCodeValid())
    {
        // Load the shader source file to a blob.
        ComPtr<IDxcBlobEncoding> SourceBlob = nullptr;
        ThrowIfFailed(pUtils->LoadFile(Attachment.GetLocation().c_str(), nullptr, &SourceBlob));

        return CompileShader(SourceBlob->GetBufferPointer(), SourceBlob->GetBufferSize(), Attachment.GetLocation(), Attachment.FunctionName, Attachment.Type, Spirv, Attachment.ShaderDefines);
    }
    return CompileShader(Attachment.GetByteCode(), Attachment.GetByteCodeSize(), Attachment.GetLocation(), Attachment.FunctionName, Attachment.Type, Spirv, Attachment.ShaderDefines);
}

IShader::SharedPtr DXCShaderCompiler::Compile(std::wstring InSrcFile, std::string InFunctionName, eShaderType InProfile, bool Spirv, std::vector<sShaderDefines> InDefines)
{
    // Load the shader source file to a blob.
    ComPtr<IDxcBlobEncoding> SourceBlob = nullptr;
    ThrowIfFailed(pUtils->LoadFile(InSrcFile.c_str(), nullptr, &SourceBlob));

    return CompileShader(SourceBlob->GetBufferPointer(), SourceBlob->GetBufferSize(), InSrcFile, InFunctionName, InProfile, Spirv, InDefines);
}

IShader::SharedPtr DXCShaderCompiler::Compile(const void* InCode, std::size_t Size, std::string InFunctionName, eShaderType InProfile, bool Spirv, std::vector<sShaderDefines> InDefines)
{
    return CompileShader(InCode, Size, L"", InFunctionName, InProfile, Spirv, InDefines);
}

IShader::SharedPtr DXCShaderCompiler::CompileShader(const void* InCode, std::size_t Size, std::wstring Path, std::string InFunctionName, eShaderType InProfile, bool Spirv, std::vector<sShaderDefines> InDefines)
{
    ComPtr<ID3D12ShaderReflection> ShaderReflection = nullptr;
    ComPtr<IDxcBlob> Blob = nullptr;

    std::wstring FunctionName = FileManager::StringToWstring(InFunctionName);
    std::vector<LPCWSTR> Arguments;

    Arguments.push_back(L"-HV");
    Arguments.push_back(L"2021");

    // -E for the entry point (eg. 'main')
    Arguments.push_back(L"-E");
    Arguments.push_back(FunctionName.c_str());

    auto Type = [](eShaderType InType) -> LPCWSTR
    {
         switch (InType)
         {
            case eShaderType::Vertex:         return L"vs_6_6";
            case eShaderType::HULL:           return L"hs_6_6";
            case eShaderType::Domain:         return L"ds_6_6";
            case eShaderType::Pixel:	      return L"ps_6_6";
            case eShaderType::Geometry:       return L"gs_6_6";
            case eShaderType::Compute:        return L"cs_6_6";
            case eShaderType::Amplification:  return L"as_6_6";
            case eShaderType::Mesh:           return L"ms_6_6";
         }
         return L" ";
    };

    // -T for the target profile (eg. 'ps_6_6')
    Arguments.push_back(L"-T");
    Arguments.push_back(Type(InProfile));

    if (Spirv)
    {
        Arguments.push_back(L"-spirv");
        // Use scalar memory layout for Vulkan resources
        //Arguments.push_back(L"-fvk-use-scalar-layout");
        // Use strict OpenGL std140/std430 memory layout for Vulkan resources
        //Arguments.push_back(L"-fvk-use-gl-layout");
        // Reciprocate SV_Position.w after reading from stage input in PS to accommodate the difference between Vulkan and DirectX
        //Arguments.push_back(L"-fvk-use-dx-position-w");
        // Use DirectX memory layout for Vulkan resources
        Arguments.push_back(L"-fvk-use-dx-layout"); 

        // Specify the SPIR - V entry point name.Defaults to the HLSL entry point name.
        //Arguments.push_back(L"-fspv-entrypoint-name=");
        // Specify SPIR - V extension permitted to use
        //Arguments.push_back(L"-fspv-extension=");
        // Flatten arrays of resources so each array element takes one binding number
        //Arguments.push_back(L"-fspv-flatten-resource-arrays");
        // Print the SPIR - V module before each pass and after the last one.Useful for debugging SPIR - V legalization and optimization passes.
        //Arguments.push_back(L"-fspv-print-all");
        // Replaces loads of composite objects to reduce memory pressure for the loads
        //Arguments.push_back(L"-fspv-reduce-load-size");
        
        // Emit additional SPIR - V instructions to aid reflection
        //Arguments.push_back(L"-fspv-reflect");

        // Specify the target environment : vulkan1.0 (default), vulkan1.1, vulkan1.1spirv1.4, vulkan1.2, vulkan1.3, or universal1.5
        //Arguments.push_back(L"-fspv-target-env=");
        // Assume the legacy matrix order(row major) when accessing raw buffers(e.g., ByteAdddressBuffer)
        //Arguments.push_back(L"-fspv-use-legacy-buffer-matrix-order");
        // Apply fvk - *-shift to resources without an explicit register assignment.
        //Arguments.push_back(L"-fvk-auto-shift-bindings");
        // Specify Vulkan binding number shift for b - type register
        //Arguments.push_back(L"-fvk-b-shift");
        // Specify Vulkan binding number and set number for the $Globals cbuffer
        //Arguments.push_back(L"-fvk-bind-globals");
        // Specify Vulkan descriptor set and binding for a specific register
        //Arguments.push_back(L"-fvk-bind-register");
        // Negate SV_Position.y before writing to stage output in VS / DS / GS to accommodate Vulkan’s coordinate system
        //Arguments.push_back(L"-fvk-invert-y");
        // Specify Vulkan binding number shift for s - type register
        //Arguments.push_back(L"-fvk-s-shift");
        // Follow Vulkan spec to use gl_BaseInstance as the first vertex instance, which makes SV_InstanceID = gl_InstanceIndex - gl_BaseInstance(without this option, SV_InstanceID = gl_InstanceIndex)
        //Arguments.push_back(L"-fvk-support-nonzero-base-instance");
        // Specify Vulkan binding number shift for t - type register
        //Arguments.push_back(L"-fvk-t-shift");
        // Specify Vulkan binding number shift for u - type register
        //Arguments.push_back(L"-fvk-u-shift");
    }
    else
    {
        Arguments.push_back(L"-Qstrip_reflect");
    }

    std::wstring STR = L"-I " + FileManager::GetShaderFolderW();
    Arguments.push_back(STR.c_str());

    Arguments.push_back(DXC_ARG_PACK_MATRIX_ROW_MAJOR);
    Arguments.push_back(DXC_ARG_ALL_RESOURCES_BOUND);
    Arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS); //-WX

    // Collect optimization passes and list them
    //Arguments.push_back(L"-Odump");

    bool DEBUG_MODE = false;
#ifdef _DEBUG
    DEBUG_MODE = true;
#endif

    // Indicate that the shader should be in a debuggable state if in debug mode.
    // Else, set optimization level to 3.
    if (DEBUG_MODE)
    {
        //-fspv-debug= Specify whitelist of debug info category(file->source->line, tool, vulkan-with-source)
        if (Spirv)
            Arguments.push_back(L"-fspv-debug=vulkan-with-source");
        else
            Arguments.push_back(DXC_ARG_DEBUG);
    }
    else
    {
        Arguments.push_back(L"-Qstrip_debug");
        Arguments.push_back(DXC_ARG_OPTIMIZATION_LEVEL3);
    }

    // Enable backward compatibility mode
    //Arguments.push_back(L"-Gec");

    std::vector<std::wstring> Params;
    for (const sShaderDefines& define : InDefines)
    {
        Arguments.push_back(L"-D");
        std::string STR = define.Name + "=" + define.Definition;
        std::wstring WSTR = FileManager::StringToWstring(STR).c_str();
        Params.push_back(WSTR);
        Arguments.push_back(Params.back().c_str());
    }

    if (GPU::IsBindlessRendererEnabled())
    {
        Arguments.push_back(L"-D");
        std::string STR2 = std::string("BINDLESS") + std::string("=") + std::string("1");
        std::wstring WSTR2 = FileManager::StringToWstring(STR2).c_str();
        Params.push_back(WSTR2);
        Arguments.push_back(Params.back().c_str());
    }

    DxcBuffer sourceBuffer
    {
        .Ptr = InCode,
        .Size = Size,
        .Encoding = 0u,
    };

    // Compile the shader.
    Microsoft::WRL::ComPtr<IDxcResult> compiledShaderBuffer{};
    const HRESULT hr = pCompiler->Compile(&sourceBuffer,
        Arguments.data(),
        static_cast<uint32_t>(Arguments.size()),
        IncludeHandler.Get(),
        IID_PPV_ARGS(&compiledShaderBuffer));
    if (FAILED(hr))
    {
        throw (std::wstring(L"Failed to compile shader with path : ") + FunctionName.data());
    }

    // Get compilation errors (if any).
    ComPtr<IDxcBlobUtf8> errors{};
    ThrowIfFailed(compiledShaderBuffer->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr));
    if (errors && errors->GetStringLength() > 0)
    {
        const LPCSTR errorMessage = errors->GetStringPointer();
        Engine::WriteToConsole(errorMessage);
        throw (errorMessage);
    }

    ThrowIfFailed(compiledShaderBuffer->GetResult(Blob.GetAddressOf()));

    if (Spirv)
    {
        return DXCCompiledShader::Create(InFunctionName, Path, InProfile, Blob, ShaderReflection);
    }

    // Get shader reflection data.
    ComPtr<IDxcBlob> ReflectionBlob = nullptr;
    ThrowIfFailed(compiledShaderBuffer->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&ReflectionBlob), nullptr));

    const DxcBuffer reflectionBuffer
    {
        .Ptr = ReflectionBlob->GetBufferPointer(),
        .Size = ReflectionBlob->GetBufferSize(),
        .Encoding = 0,
    };

    ShaderReflection = nullptr;
    pUtils->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(&ShaderReflection));

    ComPtr<IDxcBlob> pHash;
    if (SUCCEEDED(compiledShaderBuffer->GetOutput(DXC_OUT_SHADER_HASH, IID_PPV_ARGS(pHash.GetAddressOf()), nullptr)))
    {
        DxcShaderHash* pHashBuf = (DxcShaderHash*)pHash->GetBufferPointer();
    }

    return DXCCompiledShader::Create(InFunctionName, Path, InProfile, Blob, ShaderReflection);
}
