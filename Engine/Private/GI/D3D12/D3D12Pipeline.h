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

#include <array>
#include <vector>
#include <map>
#include "Engine/ClassBody.h"
#include "Engine/AbstractEngine.h"

#include "D3D12Device.h"
#include "D3D12Shader.h"
#include "D3D12ShaderStates.h"
#include "D3D12CommandBuffer.h"

class D3D12Pipeline final : public IPipeline
{
    sClassBody(sClassConstructor, D3D12Pipeline, IPipeline)
public:
	D3D12Pipeline(D3D12Device* InOwner, const std::string& InName, const sPipelineDesc& InDesc);
	virtual ~D3D12Pipeline();

    void ApplyPipeline(ID3D12GraphicsCommandList* CommandList) const;
    virtual sPipelineDesc GetPipelineDesc() const  override final { return Desc; }

    virtual bool IsCompiled() const override final
    {
        return Compiled;
    }

    virtual bool Compile(IFrameBuffer* FrameBuffer = nullptr) override final;
    virtual bool Compile(IRenderTarget* RT, IDepthTarget* Depth = nullptr) override final;
    virtual bool Compile(std::vector<IRenderTarget*> RTs, IDepthTarget* Depth = nullptr) override final;

    virtual bool Recompile() override final;

    ID3D12PipelineState* GetPSO() const { return PSO.Get(); };

private:
    void CompilePipeline();

private:
    D3D12Device* Owner;

    std::string Name;
    sPipelineDesc Desc;
    bool Compiled;

    D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;

    std::vector<sShaderAttachment> ShaderAttachments;
    std::shared_ptr<D3D12VertexAttribute> VertexAttribute;
    std::shared_ptr<D3D12RootSignature> RootSignature;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC PSODesc;
    ComPtr<ID3D12PipelineState> PSO;
};

class D3D12ComputePipeline final : public IComputePipeline
{
    sClassBody(sClassConstructor, D3D12ComputePipeline, IComputePipeline)
public:
    D3D12ComputePipeline(D3D12Device* InOwner, const std::string& InName, const sComputePipelineDesc& InDesc);
    virtual ~D3D12ComputePipeline();

    virtual sComputePipelineDesc GetPipelineDesc() const override final { return Desc; }

    virtual bool Recompile() override final;

    void ApplyPipeline(ID3D12GraphicsCommandList* CommandList) const;

private:
    D3D12Device* Owner;

    std::string Name;
    sComputePipelineDesc Desc;

    std::shared_ptr<D3D12RootSignature> RootSignature;
    sShaderAttachment ShaderAttachment;
    ComPtr<ID3D12PipelineState> ComputePipeline;
    D3D12_COMPUTE_PIPELINE_STATE_DESC ComputePipelineDesc;
};
