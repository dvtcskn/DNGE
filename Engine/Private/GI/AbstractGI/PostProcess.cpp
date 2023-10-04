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
#include "AbstractGI/PostProcess.h"

namespace
{
	// Create the vertex shader
	static const std::string vertexShader =
		"static const float4 FullScreenVertsPos[3] = { float4(-1, 1, 1, 1), float4(3, 1, 1, 1), float4(-1, -3, 1, 1) };\
             static const float2 FullScreenVertsUVs[3] = { float2(0, 0), float2(2, 0), float2(0, 2) };\
            struct VERTEX_OUT\
            {\
                float2 vTexture : TEXCOORD;\
                float4 vPosition : SV_POSITION;\
            };\
            VERTEX_OUT mainVS(uint vertexId : SV_VertexID)\
            {\
                VERTEX_OUT Output;\
                Output.vPosition = FullScreenVertsPos[vertexId];\
                Output.vTexture = FullScreenVertsUVs[vertexId];\
                return Output;\
            }";
};

sPostProcess::sPostProcess(const sShaderAttachment& PostProcessShader, const std::vector<sDescriptorSetLayoutBinding>& DescriptorSetLayout,
	const sDepthStencilAttributeDesc& DepthStencil, const sBlendAttributeDesc& Blend)
	: Pipeline(nullptr)
{
	sPipelineDesc pPipelineDesc;
	pPipelineDesc.BlendAttribute = Blend;
	pPipelineDesc.DepthStencilAttribute = DepthStencil;
	pPipelineDesc.PrimitiveTopologyType = EPrimitiveType::eTRIANGLE_LIST;
	pPipelineDesc.RasterizerAttribute = sRasterizerAttributeDesc();
	pPipelineDesc.RasterizerAttribute.CullMode = ERasterizerCullMode::eNone;

	pPipelineDesc.NumRenderTargets = 1;
	pPipelineDesc.RTVFormats[0] = EFormat::BGRA8_UNORM;
	//pPipelineDesc.DSVFormat = EFormat::R32G8X24_Typeless;

	pPipelineDesc.DescriptorSetLayout = DescriptorSetLayout;

	pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment((void*)vertexShader.data(), vertexShader.length(), "mainVS", eShaderType::Vertex));
	//pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(L"..//Content\\Shaders\\PostProcess.hlsl", "FullScreenTriangleVS", eShaderType::Vertex));
	pPipelineDesc.ShaderAttachments.push_back(PostProcessShader);

	Pipeline = IPipeline::CreateUnique("PostProcess", pPipelineDesc);
}

sPostProcess::~sPostProcess()
{
	Pipeline = nullptr;
}

void sPostProcess::SetPipeline(const sShaderAttachment& PostProcessShader, const std::vector<sDescriptorSetLayoutBinding>& DescriptorSetLayout,
	const sDepthStencilAttributeDesc& DepthStencil, const sBlendAttributeDesc& Blend)
{
	Pipeline = nullptr;

	sPipelineDesc pPipelineDesc;
	pPipelineDesc.BlendAttribute = Blend;
	pPipelineDesc.DepthStencilAttribute = DepthStencil;
	pPipelineDesc.PrimitiveTopologyType = EPrimitiveType::eTRIANGLE_LIST;
	pPipelineDesc.RasterizerAttribute = sRasterizerAttributeDesc();
	pPipelineDesc.RasterizerAttribute.CullMode = ERasterizerCullMode::eNone;

	pPipelineDesc.NumRenderTargets = 1;
	pPipelineDesc.RTVFormats[0] = EFormat::BGRA8_UNORM;
	//pPipelineDesc.DSVFormat = EFormat::R32G8X24_Typeless;

	pPipelineDesc.DescriptorSetLayout = DescriptorSetLayout;

	pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment((void*)vertexShader.data(), vertexShader.length(), "mainVS", eShaderType::Vertex));
	//pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(L"..//Content\\Shaders\\PostProcess.hlsl", "FullScreenTriangleVS", eShaderType::Vertex));
	pPipelineDesc.ShaderAttachments.push_back(PostProcessShader);

	Pipeline = IPipeline::CreateUnique("PostProcess", pPipelineDesc);
}

void sPostProcess::SetPostProcessResources(IGraphicsCommandContext* Context)
{
	//Context->SetPipeline(PostProcess->GetPipeline());
}
