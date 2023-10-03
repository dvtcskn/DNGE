#pragma once

#include "Engine/IMetaWorld.h"
#include "Engine/AbstractEngine.h"
#include "CanvasRenderer.h"
#include "PostProcessRenderer.h"
#include "AbstractGI/ToneMapping.h"
#include "LineRenderer.h"

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
	IGraphicsCommandContext::SharedPtr GraphicsCommandContext;
	IRenderTarget* FinalRenderTarget;
	EGBufferClear GBufferClearMode;

	std::vector<sViewportInstance*> ViewportInstances;

	sScreenDimension ScreenDimension;
	sScreenDimension InternalBaseRenderResolution;

	class sGBuffer;
	std::unique_ptr<sGBuffer> GBuffer;

	sLineRenderer::SharedPtr LineRenderer;

	sCanvasRenderer::SharedPtr CanvasRenderer;

	sPostProcessRenderer::UniquePtr PostProcessRenderer;
	sToneMapping::UniquePtr ToneMapping;
	bool bIsTonmapperEnabled;

	std::map<EPostProcessRenderOrder, std::vector<std::shared_ptr<sPostProcess>>> PostProcess;
};
