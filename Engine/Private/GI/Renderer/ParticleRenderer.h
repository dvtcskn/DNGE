#pragma once
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
#include "Core/Math/CoreMath.h"
#include "Gameplay/CameraManager.h"
#include "Gameplay/ParticleSystem.h"

class ParticleRenderer : public IRenderer
{
    sClassBody(sClassConstructor, ParticleRenderer, IRenderer)
public:
    ParticleRenderer(std::size_t Width, std::size_t Height, IGraphicsCommandContext::SharedPtr CMD = nullptr);
    virtual ~ParticleRenderer();

	virtual void BeginPlay() override final;
	virtual void Tick(const double DeltaTime) override final;

	void Render(const ILevel* Level, ICamera* pCamera, IRenderTarget* pRT, std::optional<sViewport> Viewport);

	virtual void SetRenderSize(std::size_t Width, std::size_t Height) override final;
	virtual void OnInputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar) override final;

	void UpdateCameraBuffer(ICamera* pCamera, IGraphicsCommandContext* CMD = nullptr);

private:
	sScreenDimension ScreenDimension;

	IGraphicsCommandContext::SharedPtr GraphicsCommandContext;
	sMaterial::SharedPtr DefaultParticle_EngineMat;
	sMaterial::sMaterialInstance::SharedPtr DefaultParticle_MatInstance;
	IConstantBuffer::SharedPtr CameraCB;
	IDepthTarget::SharedPtr Depth;

	__declspec(align(256)) struct sParticleCameraBuffer
	{
		FMatrix ViewProjMatrix;
		FMatrix PrevViewProjMatrix;
	};
	static_assert((sizeof(sParticleCameraBuffer) % 256) == 0, "Constant Buffer size must be 256-byte aligned");

	sParticleCameraBuffer CameraBuffer;
};
