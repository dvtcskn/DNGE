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
#include "VulkanShader.h"
#include "VulkanException.h"
#include "Utilities/FileManager.h"

#ifndef USE_SPIRV
#define USE_SPIRV 0
#endif // !USE_SPIRV

#if USE_SPIRV
#include <SPIRV/GlslangToSpv.h>
#include <glslang/Public/ResourceLimits.h>
#include <spirv_glsl.hpp>
#include <spirv_hlsl.hpp>
#include <spirv_common.hpp>
#include <spirv_cross.hpp>

#if _DEBUG
#pragma comment(lib, "MachineIndependentd.lib")
#pragma comment(lib, "GenericCodeGend.lib")
#pragma comment(lib, "OSDependentd.lib")
#pragma comment(lib, "OGLCompilerd.lib")
#pragma comment(lib, "glslangd.lib")
#pragma comment(lib, "glslang-default-resource-limitsd.lib")
#pragma comment(lib, "SPIRVd.lib")
#pragma comment(lib, "spirv-cross-cored.lib")
#pragma comment(lib, "spirv-cross-glsld.lib")
#else
#pragma comment(lib, "MachineIndependent.lib")
#pragma comment(lib, "GenericCodeGen.lib")
#pragma comment(lib, "OSDependent.lib")
#pragma comment(lib, "OGLCompiler.lib")
#pragma comment(lib, "glslang.lib")
#pragma comment(lib, "glslang-default-resource-limits.lib")
#pragma comment(lib, "SPIRV.lib")
#pragma comment(lib, "spirv-cross-core.lib")
#pragma comment(lib, "spirv-cross-glsl.lib")
#endif

VulkanShader::VulkanShader(VulkanDevice* InDevice, const sShaderAttachment& Attachment)
	: FunctionName(Attachment.FunctionName)
	, ShaderType(Attachment.Type)
	, Device(InDevice)
{
	if (!Attachment.IsCodeValid())
		CompileShaderFromFile(Attachment.GetLocation(), Attachment.ShaderDefines);
	else
		CompileShader(Attachment.GetByteCode(), Attachment.GetByteCodeSize(), Attachment.ShaderDefines);
}

VulkanShader::VulkanShader(VulkanDevice* InDevice, std::wstring InSrcFile, std::string InFunctionName, eShaderType InProfile, std::vector<sShaderDefines> InDefines)
	: FunctionName(InFunctionName)
	, ShaderType(InProfile)
	, Device(InDevice)
{
	CompileShaderFromFile(InSrcFile, InDefines);
}

VulkanShader::VulkanShader(VulkanDevice* InDevice, const void* InCode, std::size_t Size, std::string InFunctionName, eShaderType InProfile, std::vector<sShaderDefines> InDefines)
	: FunctionName(InFunctionName)
	, ShaderType(InProfile)
	, Device(InDevice)
{
	CompileShader(InCode, Size, InDefines);
}

VulkanShader::~VulkanShader()
{

}

void VulkanShader::CompileShaderFromFile(std::wstring InSrcFile, std::vector<sShaderDefines> InDefines)
{
	std::vector<uint32_t> spirv;
	std::string           info_log;

	// Compile HLSL to SPIR-V

	// Initialize glslang library
	glslang::InitializeProcess();

	auto messages = static_cast<EShMessages>(EShMsgReadHlsl | EShMsgDefault | EShMsgVulkanRules | EShMsgSpvRules);

	VkShaderStageFlagBits stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
	EShLanguage language{};
	switch (ShaderType)
	{
	case eShaderType::Vertex:
		language = EShLangVertex;
		stage = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
		break;
	case eShaderType::Domain:
		//language = EShLangVertex;
		//stage = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
		break;
	case eShaderType::Geometry:
		language = EShLangGeometry;
		stage = VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT;
		break;
	case eShaderType::HULL:
		//language = EShLangVertex;
		//stage = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
		break;
	case eShaderType::Pixel:
		language = EShLangFragment;
		stage = VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
		break;
	case eShaderType::Compute:
		language = EShLangCompute;
		stage = VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT;
		break;
	case eShaderType::Amplification:
		//language = EShLangVertex;
		//stage = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
		break;
	case eShaderType::Mesh:
		language = EShLangMesh;
		stage = VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT;
		break;
	}

	auto LoadedFile = FileManager::readFile(InSrcFile);
	std::string source;
	source.assign(LoadedFile->GetData<char>());

	if (InDefines.size() > 0)
	{
		for (size_t i = 0; i < InDefines.size(); ++i)
		{
			std::string Define = InDefines[i].Name + " = " + InDefines[i].Definition;
			source.insert(source.begin(), Define.begin(), source.end());
		}
	}

	const char* shader_source = reinterpret_cast<const char*>(source.data());

	glslang::TShader shader(language);
	shader.setStringsWithLengths(&shader_source, nullptr, 1);
	shader.setEnvInput(glslang::EShSourceHlsl, language, glslang::EShClientVulkan, 1);
	shader.setEntryPoint(FunctionName.c_str());
	shader.setSourceEntryPoint(FunctionName.c_str());
	shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
	shader.setEnvTarget(glslang::EshTargetSpv, glslang::EShTargetSpv_1_0);

	if (!shader.parse(GetDefaultResources(), 100, false, messages))
	{
		Engine::WriteToConsole("Failed to parse HLSL shader, Error: { " + std::string(shader.getInfoLog()) + "\n" + std::string(shader.getInfoDebugLog()) + " }");
		throw std::runtime_error("Failed to parse HLSL shader");
	}

	// Add shader to new program object
	glslang::TProgram program;
	program.addShader(&shader);

	// Link program
	if (!program.link(messages))
	{
		Engine::WriteToConsole("Failed to compile HLSL shader, Error: {" + std::string(program.getInfoLog()) + "\n" + std::string(program.getInfoDebugLog()) + " }");
		throw std::runtime_error("Failed to compile HLSL shader");
	}

	if (shader.getInfoLog())
	{
		info_log += std::string(shader.getInfoLog()) + "\n" + std::string(shader.getInfoDebugLog()) + "\n";
	}
	if (program.getInfoLog())
	{
		info_log += std::string(program.getInfoLog()) + "\n" + std::string(program.getInfoDebugLog());
	}

	// Translate to SPIRV
	glslang::TIntermediate* intermediate = program.getIntermediate(language);
	if (!intermediate)
	{
		Engine::WriteToConsole("Failed to get shared intermediate code.");
		throw std::runtime_error("Failed to compile HLSL shader");
	}

	spv::SpvBuildLogger logger;

	glslang::GlslangToSpv(*intermediate, spirv, &logger);

	info_log += logger.getAllMessages() + "\n";

	glslang::FinalizeProcess();

	// Create shader module from generated SPIR-V
	VkShaderModule           shader_module;
	VkShaderModuleCreateInfo module_create_info{};
	module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	module_create_info.codeSize = spirv.size() * sizeof(uint32_t);
	module_create_info.pCode = spirv.data();
	VK_CHECK(vkCreateShaderModule(Device->Get(), &module_create_info, nullptr, &shader_module));

	shader_stage = {};
	shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stage.stage = stage;
	shader_stage.module = shader_module;
	shader_stage.pName = "main";
	assert(shader_stage.module != VK_NULL_HANDLE);
	shader_modules.push_back(shader_stage.module);
}

void VulkanShader::CompileShader(const void* InCode, std::size_t Size, std::vector<sShaderDefines> InDefines)
{
	std::vector<uint32_t> spirv;
	std::string           info_log;

	// Compile HLSL to SPIR-V

	// Initialize glslang library
	glslang::InitializeProcess();

	auto messages = static_cast<EShMessages>(EShMsgReadHlsl | EShMsgDefault | EShMsgVulkanRules | EShMsgSpvRules);

	VkShaderStageFlagBits stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
	EShLanguage language{};
	switch (ShaderType)
	{
	case eShaderType::Vertex:
		language = EShLangVertex;
		stage = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
		break;
	case eShaderType::Domain:
		//language = EShLangVertex;
		//stage = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
		break;
	case eShaderType::Geometry:
		language = EShLangGeometry;
		stage = VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT;
		break;
	case eShaderType::HULL:
		//language = EShLangVertex;
		//stage = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
		break;
	case eShaderType::Pixel:
		language = EShLangFragment;
		stage = VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
		break;
	case eShaderType::Compute:
		language = EShLangCompute;
		stage = VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT;
		break;
	case eShaderType::Amplification:
		//language = EShLangVertex;
		//stage = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
		break;
	case eShaderType::Mesh:
		language = EShLangMesh;
		stage = VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT;
		break;
	}

	std::string source;
	source.assign(static_cast<const char*>(InCode));

	if (InDefines.size() > 0)
	{
		for (size_t i = 0; i < InDefines.size(); ++i)
		{
			std::string Define = InDefines[i].Name + " = " + InDefines[i].Definition;
			source.insert(source.begin(), Define.begin(), source.end());
		}
	}

	const char* shader_source = reinterpret_cast<const char*>(source.data());

	glslang::TShader shader(language);
	shader.setStringsWithLengths(&shader_source, nullptr, 1);
	shader.setEnvInput(glslang::EShSourceHlsl, language, glslang::EShClientVulkan, 1);
	shader.setEntryPoint(FunctionName.c_str());
	shader.setSourceEntryPoint(FunctionName.c_str());
	shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_0);
	shader.setEnvTarget(glslang::EshTargetSpv, glslang::EShTargetSpv_1_0);

	if (!shader.parse(GetDefaultResources(), 100, false, messages))
	{
		Engine::WriteToConsole("Failed to parse HLSL shader, Error: { " + std::string(shader.getInfoLog()) + "\n" + std::string(shader.getInfoDebugLog()) + " }");
		throw std::runtime_error("Failed to parse HLSL shader");
	}

	// Add shader to new program object
	glslang::TProgram program;
	program.addShader(&shader);

	// Link program
	if (!program.link(messages))
	{
		Engine::WriteToConsole("Failed to compile HLSL shader, Error: {" + std::string(program.getInfoLog()) + "\n" + std::string(program.getInfoDebugLog()) + " }");
		throw std::runtime_error("Failed to compile HLSL shader");
	}

	if (shader.getInfoLog())
	{
		info_log += std::string(shader.getInfoLog()) + "\n" + std::string(shader.getInfoDebugLog()) + "\n";
	}
	if (program.getInfoLog())
	{
		info_log += std::string(program.getInfoLog()) + "\n" + std::string(program.getInfoDebugLog());
	}

	// Translate to SPIRV
	glslang::TIntermediate* intermediate = program.getIntermediate(language);
	if (!intermediate)
	{
		Engine::WriteToConsole("Failed to get shared intermediate code.");
		throw std::runtime_error("Failed to compile HLSL shader");
	}

	spv::SpvBuildLogger logger;

	glslang::GlslangToSpv(*intermediate, spirv, &logger);

	info_log += logger.getAllMessages() + "\n";

	glslang::FinalizeProcess();

	// Create shader module from generated SPIR-V
	VkShaderModule           shader_module;
	VkShaderModuleCreateInfo module_create_info{};
	module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	module_create_info.codeSize = spirv.size() * sizeof(uint32_t);
	module_create_info.pCode = spirv.data();
	VK_CHECK(vkCreateShaderModule(Device->Get(), &module_create_info, nullptr, &shader_module));

	shader_stage = {};
	shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stage.stage = stage;
	shader_stage.module = shader_module;
	shader_stage.pName = "main";
	assert(shader_stage.module != VK_NULL_HANDLE);
	shader_modules.push_back(shader_stage.module);
}
#endif
