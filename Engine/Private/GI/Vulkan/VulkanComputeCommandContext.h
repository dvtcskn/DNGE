#pragma once

#include "Engine/AbstractEngine.h"

class VulkanComputeCommandContext : public IComputeCommandContext
{
	sClassBody(sClassConstructor, VulkanComputeCommandContext, IComputeCommandContext)
public:
	VulkanComputeCommandContext();
	virtual ~VulkanComputeCommandContext();

	virtual void BeginRecordCommandList() override final;
	virtual void FinishRecordCommandList() override final;
	virtual void ExecuteCommandList() override final;
	virtual void ClearState() override final;

	virtual void* GetInternalCommandContext() override final;

	virtual void SetFrameBuffer(IFrameBuffer* pFB, std::optional<std::size_t> FBOIndex) override final;
	virtual void SetPipeline(IComputePipeline* Pipeline) override final;
	virtual void SetConstantBuffer(IConstantBuffer* CB, std::optional<std::uint32_t> RootParameterIndex = std::nullopt) override final;

	virtual void SetRenderTargetAsResource(IRenderTarget* pRT, std::uint32_t RootParameterIndex) override final;
	virtual void SetRenderTargetsAsResource(std::vector<IRenderTarget*> RTs, std::uint32_t RootParameterIndex) override final;
	virtual void SetUnorderedAccessTarget(IUnorderedAccessTarget* pST, std::uint32_t RootParameterIndex) override final;
	virtual void SetUnorderedAccessTargets(std::vector<IUnorderedAccessTarget*> pSTs, std::uint32_t RootParameterIndex) override final;

	virtual void SetRenderTargetAsUAV(IRenderTarget* pRT, std::uint32_t RootParameterIndex) override final;
	virtual void SetRenderTargetsAsUAV(std::vector<IRenderTarget*> RTs, std::uint32_t RootParameterIndex) override final;
	virtual void SetUnorderedAccessTargetAsSRV(IUnorderedAccessTarget* pST, std::uint32_t RootParameterIndex) override final;
	virtual void SetUnorderedAccessTargetsAsSRV(std::vector<IUnorderedAccessTarget*> pSTs, std::uint32_t RootParameterIndex) override final;

	virtual void SetDepthTargetAsResource(IDepthTarget* pDT, std::uint32_t RootParameterIndex) override final;
	virtual void SetDepthTargetsAsResource(std::vector<IDepthTarget*> DTs, std::uint32_t RootParameterIndex) override final;

	virtual void Dispatch(std::uint32_t ThreadGroupCountX, std::uint32_t ThreadGroupCountY, std::uint32_t ThreadGroupCountZ) override final;
	virtual void ExecuteIndirect(IIndirectBuffer* IndirectBuffer) override final;
};
