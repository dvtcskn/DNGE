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
	void Draw(IRenderTarget* pFB, IVertexBuffer* VertexBuffer, IIndexBuffer* IndexBuffer, ICanvas::WidgetHierarchy* Node, const cbgui::cbIntBounds& ScissorsRect, const sViewport& VP);
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
