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

#include "Engine/ClassBody.h"
#include "Engine/AbstractEngine.h"

class sPostProcess
{
	sBaseClassBody(sClassNoDefaults, sPostProcess)
protected:
	sPostProcess()
		: Pipeline(nullptr)
	{}
	sPostProcess(const sShaderAttachment& PostProcessShader, const std::vector<sShaderBinding>& DescriptorSetLayout, 
		const sDepthStencilAttributeDesc& DepthStencil = sDepthStencilAttributeDesc(ECompareFunction::LessEqual, false),
		const sBlendAttributeDesc& Blend = sBlendAttributeDesc(EBlendStateMode::Opaque));
	virtual ~sPostProcess();

	void SetPipeline(const sShaderAttachment& PostProcessShader, const std::vector<sShaderBinding>& DescriptorSetLayout,
		const sDepthStencilAttributeDesc& DepthStencil = sDepthStencilAttributeDesc(ECompareFunction::LessEqual, false),
		const sBlendAttributeDesc& Blend = sBlendAttributeDesc(EBlendStateMode::Opaque));

public:
	virtual bool HasFrameBuffer() const { return false; }
	virtual IRenderTarget* GetFrameBuffer() const { return nullptr; }
	virtual void SetFrameBufferSize(const std::size_t Width, const std::size_t Height) {}

	virtual void SetPostProcessResources(IGraphicsCommandContext* Context, IRenderTarget* BackBuffer = nullptr);

	inline bool IsCompiled() const { return Pipeline->IsCompiled(); }
	inline bool Compile(IFrameBuffer* FrameBuffer = nullptr) { return Pipeline->Compile(FrameBuffer); }
	inline bool Compile(IRenderTarget* RT, IDepthTarget* Depth = nullptr) { return Pipeline->Compile(RT, Depth); }
	inline bool Compile(std::vector<IRenderTarget*> RTs, IDepthTarget* Depth = nullptr) { return Pipeline->Compile(RTs, Depth); }
	inline bool Recompile() { return Pipeline->Recompile(); }

	inline IPipeline* GetPipeline() const { return Pipeline.get(); }

private:
	IPipeline::UniquePtr Pipeline;
};
