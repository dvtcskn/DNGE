#pragma once

#include "Engine/ClassBody.h"
#include "Engine/AbstractEngine.h"

class sPostProcess
{
	sBaseClassBody(sClassNoDefaults, sPostProcess)
protected:
	sPostProcess()
		: Pipeline(nullptr)
	{}
	sPostProcess(const sShaderAttachment& PostProcessShader, const std::vector<sDescriptorSetLayoutBinding>& DescriptorSetLayout, 
		const sDepthStencilAttributeDesc& DepthStencil = sDepthStencilAttributeDesc(false),
		const sBlendAttributeDesc& Blend = sBlendAttributeDesc(EBlendStateMode::eOpaque));
	virtual ~sPostProcess();

	void SetPipeline(const sShaderAttachment& PostProcessShader, const std::vector<sDescriptorSetLayoutBinding>& DescriptorSetLayout,
		const sDepthStencilAttributeDesc& DepthStencil = sDepthStencilAttributeDesc(false),
		const sBlendAttributeDesc& Blend = sBlendAttributeDesc(EBlendStateMode::eOpaque));

public:
	virtual bool HasFrameBuffer() const { return false; }
	virtual IRenderTarget* GetFrameBuffer() const { return nullptr; }
	virtual void SetFrameBufferSize(const std::size_t Width, const std::size_t Height) {}

	virtual bool UseBackBufferAsResource() const { return false; }
	virtual std::size_t GetBackBufferResourceRootParameterIndex() const { return 0; }

	virtual void SetPostProcessResources(IGraphicsCommandContext* Context);

	IPipeline* GetPipeline() const { return Pipeline.get(); }

private:
	IPipeline::UniquePtr Pipeline;
};
