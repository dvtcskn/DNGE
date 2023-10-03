
#include "pch.h"
#include "PostProcessRenderer.h"

sPostProcessRenderer::sPostProcessRenderer(std::size_t Width, std::size_t Height)
	: Super()
	, GraphicsCommandContext(IGraphicsCommandContext::Create())
	, ScreenDimension(sScreenDimension(Width, Height))
{
}

sPostProcessRenderer::~sPostProcessRenderer()
{
	GraphicsCommandContext = nullptr;
}

void sPostProcessRenderer::BeginPlay()
{

}

void sPostProcessRenderer::Tick(const double DeltaTime)
{

}

void sPostProcessRenderer::Render(sPostProcess* PostProcess, IRenderTarget* BackBuffer, std::optional<sViewport> Viewport)
{
	GraphicsCommandContext->BeginRecordCommandList(ERenderPass::ePostProcess);

	if (PostProcess->HasFrameBuffer())
		GraphicsCommandContext->ClearRenderTarget(PostProcess->GetFrameBuffer());

	if (PostProcess->HasFrameBuffer())
		GraphicsCommandContext->SetRenderTarget(PostProcess->GetFrameBuffer());
	else
		GraphicsCommandContext->SetRenderTarget(BackBuffer);

	if (Viewport.has_value())
		GraphicsCommandContext->SetViewport(*Viewport);
	else
		GraphicsCommandContext->SetViewport(sViewport(ScreenDimension));

	GraphicsCommandContext->SetScissorRect(0, 0, (std::uint32_t)ScreenDimension.Height, (std::uint32_t)ScreenDimension.Width);

	GraphicsCommandContext->SetPipeline(PostProcess->GetPipeline());
	PostProcess->SetPostProcessResources(GraphicsCommandContext.get());

	if (PostProcess->UseBackBufferAsResource())
		GraphicsCommandContext->SetRenderTargetAsResource(BackBuffer, (std::uint32_t)PostProcess->GetBackBufferResourceRootParameterIndex());

	GraphicsCommandContext->DrawInstanced(3, 1, 0, 0);

	GraphicsCommandContext->FinishRecordCommandList();
	GraphicsCommandContext->ExecuteCommandList();
}

void sPostProcessRenderer::CopyToFrameBuffer(sPostProcess* PostProcess, IRenderTarget* FrameBuffer)
{
	if (PostProcess->HasFrameBuffer())
	{
		GraphicsCommandContext->BeginRecordCommandList();
		GraphicsCommandContext->CopyRenderTarget(FrameBuffer, PostProcess->GetFrameBuffer());
		GraphicsCommandContext->FinishRecordCommandList();
		GraphicsCommandContext->ExecuteCommandList();
	}
}

void sPostProcessRenderer::Render(IGraphicsCommandContext* CMD, sPostProcess* PostProcess, IRenderTarget* BackBuffer, std::optional<sViewport> Viewport)
{
	CMD->BeginRecordCommandList(ERenderPass::ePostProcess);

	if (PostProcess->HasFrameBuffer())
		CMD->ClearRenderTarget(PostProcess->GetFrameBuffer());

	if (PostProcess->HasFrameBuffer())
		CMD->SetRenderTarget(PostProcess->GetFrameBuffer());
	else
		CMD->SetRenderTarget(BackBuffer);

	if (Viewport.has_value())
		CMD->SetViewport(*Viewport);
	else
		CMD->SetViewport(sViewport(ScreenDimension));

	CMD->SetScissorRect(0, 0, (std::uint32_t)ScreenDimension.Height, (std::uint32_t)ScreenDimension.Width);

	CMD->SetPipeline(PostProcess->GetPipeline());
	PostProcess->SetPostProcessResources(CMD);

	if (PostProcess->UseBackBufferAsResource())
		CMD->SetRenderTargetAsResource(BackBuffer, (std::uint32_t)PostProcess->GetBackBufferResourceRootParameterIndex());

	CMD->DrawInstanced(3, 1, 0, 0);

	CMD->FinishRecordCommandList();
	CMD->ExecuteCommandList();
}

void sPostProcessRenderer::CopyToFrameBuffer(IGraphicsCommandContext* CMD, sPostProcess* PostProcess, IRenderTarget* FrameBuffer)
{
	if (PostProcess->HasFrameBuffer())
	{
		CMD->BeginRecordCommandList();
		CMD->CopyRenderTarget(FrameBuffer, PostProcess->GetFrameBuffer());
		CMD->FinishRecordCommandList();
		CMD->ExecuteCommandList();
	}
}

void sPostProcessRenderer::SetRenderSize(std::size_t InWidth, std::size_t InHeight)
{
	ScreenDimension.Width = InWidth;
	ScreenDimension.Height = InHeight;
}

void sPostProcessRenderer::OnInputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar)
{

}
