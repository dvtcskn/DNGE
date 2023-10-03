#pragma once

#include "IRenderer.h"
#include "AbstractGI/Material.h"
#include "AbstractGI/UIMaterialStyle.h"
#include "Gameplay/ICanvas.h"
#include "Utilities/Input.h"

class sCanvasRenderer final : public IRenderer
{
	sClassBody(sClassConstructor, sCanvasRenderer, IRenderer);
public:
	sCanvasRenderer(std::size_t Width, std::size_t Height);
	virtual ~sCanvasRenderer();

	virtual void BeginPlay() override final;
	virtual void Tick(const double DeltaTime) override final;

	void Render(ICanvas* Canvases, IRenderTarget* pFB, std::optional<sViewport> Viewport);
	void Render(const std::vector<ICanvas*>& Canvases, IRenderTarget* pFB, std::optional<sViewport> Viewport);
	void Draw(IVertexBuffer* VertexBuffer, IIndexBuffer* IndexBuffer, ICanvas::WidgetHierarchy* Node, const cbgui::cbIntBounds& ScissorsRect, const sViewport& VP);
	bool DepthPass(cbgui::cbWidgetObj* Widget, const sViewport& VP);

	virtual void SetRenderSize(std::size_t Width, std::size_t Height) override final;

	virtual void OnInputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar) override final;

private:
	//IFrameBuffer::SharedPtr CanvasFBO;
	sScreenDimension ScreenDimension;

	sMaterial* LastMaterial;

	IGraphicsCommandContext::SharedPtr CMD;
	IDepthTarget::UniquePtr Depth;

	bool bEnableStencilClipping;
	bool bShowLines;

	IConstantBuffer::SharedPtr WidgetConstantBuffer;

	sMaterial::SharedPtr DefaultUILineMaterial;
	UIMaterialStyle::SharedPtr DefaultLineColorMatStyle;

	IVertexBuffer::UniquePtr DepthVertexBuffer;

	IPipeline::UniquePtr DepthMainPipeline;
	IPipeline::UniquePtr DepthPipeline;
};
