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

#include "Core/Math/CoreMath.h"
#include "Gameplay/CameraManager.h"
#include "Gameplay/ParticleSystem.h"

class ParticleRenderer : public IRenderPass
{
    sClassBody(sClassConstructor, ParticleRenderer, IRenderPass)
public:
    ParticleRenderer(std::size_t Width, std::size_t Height, IGraphicsCommandContext::SharedPtr CMD = nullptr);
    virtual ~ParticleRenderer();

	virtual void BeginPlay() override final;
	virtual void Tick(const double DeltaTime) override final;

	void Render(const ILevel* Level, std::uint32_t CameraBindlessIndex, IRenderTarget* pRT, std::optional<sViewport> Viewport);

	virtual void SetRenderSize(std::size_t Width, std::size_t Height) override final;
	virtual void OnInputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar) override final;

	sIndirectLayoutBindingDesc GetIndirectLayoutBindingDesc() const
	{
		return IndirectLayoutBindingDesc;
	}

private:
	sScreenDimension ScreenDimension;

	sIndirectLayoutBindingDesc IndirectLayoutBindingDesc;
	IGraphicsCommandContext::SharedPtr GraphicsCommandContext;
	sMaterial::SharedPtr DefaultParticle_EngineMat;
	sMaterialInstance::SharedPtr DefaultParticle_MatInstance;
	IDepthTarget::SharedPtr Depth;
	IIndirectBuffer::SharedPtr IndirectBuffer;
	bool bEnableExecuteIndirect;
};
