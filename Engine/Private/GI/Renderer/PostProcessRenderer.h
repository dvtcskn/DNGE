#pragma once

#include "IRenderer.h"
#include "AbstractGI/Material.h"
#include "AbstractGI/UIMaterialStyle.h"
#include "Gameplay/ICanvas.h"
#include "Utilities/Input.h"
#include "Gameplay/CameraManager.h"
#include "AbstractGI/PostProcess.h"

class sPostProcessRenderer : public IRenderer
{
	sClassBody(sClassConstructor, sPostProcessRenderer, IRenderer)
public:
	sPostProcessRenderer(std::size_t Width, std::size_t Height);
	virtual ~sPostProcessRenderer();

	virtual void BeginPlay() override final;
	virtual void Tick(const double DeltaTime) override final;

	void Render(sPostProcess* PostProcess, IRenderTarget* BackBuffer, std::optional<sViewport> Viewport);
	void CopyToFrameBuffer(sPostProcess* PostProcess, IRenderTarget* FrameBuffer);

	void Render(IGraphicsCommandContext* CMD, sPostProcess* PostProcess, IRenderTarget* BackBuffer, std::optional<sViewport> Viewport);
	void CopyToFrameBuffer(IGraphicsCommandContext* CMD, sPostProcess* PostProcess, IRenderTarget* FrameBuffer);

	virtual void SetRenderSize(std::size_t Width, std::size_t Height) override final;
	virtual void OnInputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar) override final;

private:
	sScreenDimension ScreenDimension;
	IGraphicsCommandContext::SharedPtr GraphicsCommandContext;
};
