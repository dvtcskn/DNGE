
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
