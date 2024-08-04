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

#include "Engine/IMetaWorld.h"
#include "Engine/AbstractEngine.h"
#include "CanvasRenderer.h"
#include "PostProcessRenderer.h"
#include "AbstractGI/ToneMapping.h"
#include "LineRenderer.h"
#include "ParticleRenderer.h"

class sRenderer final
{
	sBaseClassBody(sClassConstructor, sRenderer);
public:
	sRenderer(std::size_t Width, std::size_t Height);
	~sRenderer();

	void BeginPlay();
	void Tick(const double DeltaTime);

	void Render();

	void OnResizeWindow(std::size_t Width, std::size_t Height);

	void SetMetaWorld(IMetaWorld* MetaWorld);
	void RemoveWorld();
	IMetaWorld* GetMetaWorld() const;

	inline IRenderTarget* GetFinalRenderTarget() const { return FinalRenderTarget; }
	inline sScreenDimension GetScreenDimension() const { return ScreenDimension; }

	void AddViewportInstance(sViewportInstance* ViewportInstance, std::optional<std::size_t> Priority = std::nullopt);
	void RemoveViewportInstance(sViewportInstance* ViewportInstance);
	void RemoveViewportInstance(std::size_t Index);
	void SetViewportInstancePriority(sViewportInstance* ViewportInstance, std::size_t Priority);

	void SetTonemapper(int Val);
	int GetTonemapperIndex() const;
	inline void EnableTonemapper() { bIsTonmapperEnabled = true; }
	inline void DisableTonemapper() { bIsTonmapperEnabled = false; }

	void AddPostProcess(const EPostProcessRenderOrder Order, const std::shared_ptr<sPostProcess>& PostProcess);
	void RemovePostProcess(const EPostProcessRenderOrder Order, const int Val);

	void DrawLine(const FVector& Start, const FVector& End, const FColor& Color, std::optional<float> Time);
	void DrawBound(const FBoundingBox& Box, const FColor& Color, std::optional<float> Time);

	inline sScreenDimension GetInternalBaseRenderResolution() const { return InternalBaseRenderResolution; }
	void SetInternalBaseRenderResolution(std::size_t Width, std::size_t Height);

	void SetGBufferClearMode(EGBufferClear Mode);
	EGBufferClear GetGBufferClearMode() { return GBufferClearMode; }

	void OnInputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar);

private:
	IMetaWorld* World;

	class sGBuffer;
	std::unique_ptr<sGBuffer> GBuffer;

	IGraphicsCommandContext::SharedPtr GraphicsCommandContext;
	IRenderTarget* FinalRenderTarget;
	std::vector<sViewportInstance*> ViewportInstances;
	sScreenDimension ScreenDimension;
	sScreenDimension InternalBaseRenderResolution;
	sCanvasRenderer::SharedPtr CanvasRenderer;
	sPostProcessRenderer::UniquePtr PostProcessRenderer;
	sToneMapping::UniquePtr ToneMapping;
	sLineRenderer::SharedPtr LineRenderer;
	EGBufferClear GBufferClearMode;
	ParticleRenderer::SharedPtr pParticleRenderer;

	bool bIsTonmapperEnabled;

	std::map<EPostProcessRenderOrder, std::vector<std::shared_ptr<sPostProcess>>> PostProcess;
};
