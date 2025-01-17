/* ---------------------------------------------------------------------------------------
* MIT License
*
* Copyright (c) 2023 Davut Co�kun.
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

#include "pch.h"
#include "GI/D3DShared/D3DShared.h"
#include "D3D12Pipeline.h"
#include "D3D12Viewport.h"
//#include "GI/Shared/Shader.h"

D3D12Pipeline::D3D12Pipeline(D3D12Device* InOwner, const std::string& InName, const sPipelineDesc& InDesc)
    : Owner(InOwner)
    , Name(InName)
    , Desc(InDesc)
    , Compiled(false)
{
    ZeroMemory(&PSODesc, sizeof(PSODesc));
    PSODesc.NodeMask = 1;
    PSODesc.SampleMask = 0xFFFFFFFFu;
    PSODesc.SampleDesc.Count = 1;

    Compile();
}

D3D12Pipeline::~D3D12Pipeline()
{
    Owner = nullptr;

    RootSignature = nullptr;

    ShaderAttachments.clear();
    VertexAttribute = nullptr;

    PSO = nullptr;
}

bool D3D12Pipeline::Compile(IFrameBuffer* FrameBuffer)
{
    if (Compiled)
        return false;

    if (!FrameBuffer)
        return false;

    D3D12FrameBuffer* FB = Cast<D3D12FrameBuffer>(FrameBuffer);

    if (FB->DepthTarget)
        PSODesc.DSVFormat = GetDXGIDepthViewFormat(ConvertFormat_Format_To_DXGI(FB->DepthTarget->GetFormat()));
    else
        PSODesc.DSVFormat = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;

    for (UINT i = 0; i < 8; i++)
        PSODesc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;

    const auto& RTs = FB->RenderTargets;
    for (std::size_t i = 0; i < RTs.size(); i++)
        PSODesc.RTVFormats[i] = ConvertFormat_Format_To_DXGI(RTs[i]->GetFormat());

    PSODesc.NumRenderTargets = RTs.size();
  
    CompilePipeline();

    return true;
}

bool D3D12Pipeline::Compile(IRenderTarget* InRT, IDepthTarget* Depth)
{
    if (Compiled)
        return false;

    D3D12RenderTarget* RT = Cast<D3D12RenderTarget>(InRT);

    if (Depth)
        PSODesc.DSVFormat = GetDXGIDepthViewFormat(ConvertFormat_Format_To_DXGI(Cast<D3D12DepthTarget>(Depth)->GetFormat())); //GetDXGIDepthViewFormat(ConvertFormat_Format_To_DXGI(Desc.DSVFormat));
    else
        PSODesc.DSVFormat = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;

    for (UINT i = 0; i < 8; i++)
        PSODesc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;

    PSODesc.RTVFormats[0] = ConvertFormat_Format_To_DXGI(RT->GetFormat());

    PSODesc.NumRenderTargets = 1;

    CompilePipeline();

    return true;
}

bool D3D12Pipeline::Compile(std::vector<IRenderTarget*> RTs, IDepthTarget* Depth)
{
    if (Compiled)
        return false;

    if (Depth)
        PSODesc.DSVFormat = GetDXGIDepthViewFormat(ConvertFormat_Format_To_DXGI(Cast<D3D12DepthTarget>(Depth)->GetFormat())); //GetDXGIDepthViewFormat(ConvertFormat_Format_To_DXGI(Desc.DSVFormat));
    else
        PSODesc.DSVFormat = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;

    for (UINT i = 0; i < 8; i++)
        PSODesc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;

    for (std::size_t i = 0; i < RTs.size(); i++)
        PSODesc.RTVFormats[i] = ConvertFormat_Format_To_DXGI(Cast<D3D12RenderTarget>(RTs[i])->GetFormat());

    PSODesc.NumRenderTargets = RTs.size();

    CompilePipeline();

    return true;
}

void D3D12Pipeline::ApplyPipeline(ID3D12GraphicsCommandList* CommandList) const
{
    CommandList->IASetPrimitiveTopology(PrimitiveTopologyType);
    CommandList->SetGraphicsRootSignature(RootSignature->Get());
    CommandList->SetPipelineState(PSO.Get());
}

void D3D12Pipeline::CompilePipeline()
{
    ShaderAttachments = Desc.ShaderAttachments;

    //std::vector<DXCShaderCompiler::SharedPtr> Blobs;

    /*for (auto& Attachment : ShaderAttachments)
    {
        if (!sShaderManager::Get().IsShaderExist(Attachment.FunctionName))
            DXCShaderCompiler::SharedPtr pShader = DXCShaderCompiler::Create(Attachment);
    }

    for (auto& Attachment : ShaderAttachments)
    {
        CompiledShader* pCShader = Cast<CompiledShader>(sShaderManager::Get().GetShader(Attachment.FunctionName));*/

    for (const auto& Attachment : ShaderAttachments)
    {
        auto pShader = Owner->CompileShader(Attachment);
        //Blobs.push_back(pShader);

        switch (Attachment.Type)
        {
        case eShaderType::Vertex:
        {
            PSODesc.VS.pShaderBytecode = pShader->GetByteCode();
            PSODesc.VS.BytecodeLength = pShader->GetByteCodeSize();
            break;
        }
        case eShaderType::Pixel:
        {
            PSODesc.PS.pShaderBytecode = pShader->GetByteCode();
            PSODesc.PS.BytecodeLength = pShader->GetByteCodeSize();
            break;
        }
        case eShaderType::Geometry:
        {
            PSODesc.GS.pShaderBytecode = pShader->GetByteCode();
            PSODesc.GS.BytecodeLength = pShader->GetByteCodeSize();
            break;
        }
        case eShaderType::HULL:
        {
            PSODesc.HS.pShaderBytecode = pShader->GetByteCode();
            PSODesc.HS.BytecodeLength = pShader->GetByteCodeSize();
            break;
        }
        case eShaderType::Domain:
        {
            PSODesc.DS.pShaderBytecode = pShader->GetByteCode();
            PSODesc.DS.BytecodeLength = pShader->GetByteCodeSize();
            break;
        }
        case eShaderType::Compute:
        {
            break;
        }
        case eShaderType::Amplification:
        {
            break;
        }
        case eShaderType::Mesh:
        {
            break;
        }
        }
    }

    auto PrimTopoType = [&](EPrimitiveType Type) -> D3D12_PRIMITIVE_TOPOLOGY_TYPE
        {
            switch (Type)
            {
            case EPrimitiveType::ePOINT_LIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
            case EPrimitiveType::eTRIANGLE_LIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            case EPrimitiveType::eTRIANGLE_STRIP: return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            case EPrimitiveType::eLINE_LIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
                /*case EPrimitiveType::ePATCH_1_CONTROL_POINT: return D3D12_PRIMITIVE_TOPOLOGY_TYPE:: D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST;
                case EPrimitiveType::ePATCH_3_CONTROL_POINT: return D3D12_PRIMITIVE_TOPOLOGY_TYPE:: D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST;*/
            }
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        };

    auto PrimTopology = [&](EPrimitiveType Type) -> D3D12_PRIMITIVE_TOPOLOGY
        {
            switch (Type)
            {
            case EPrimitiveType::ePOINT_LIST: return D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
            case EPrimitiveType::eTRIANGLE_LIST: return D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            case EPrimitiveType::eTRIANGLE_STRIP: return D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            case EPrimitiveType::eLINE_LIST: return D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_LINELIST;
                /*case EPrimitiveType::ePATCH_1_CONTROL_POINT: return D3D12_PRIMITIVE_TOPOLOGY_TYPE:: D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST;
                case EPrimitiveType::ePATCH_3_CONTROL_POINT: return D3D12_PRIMITIVE_TOPOLOGY_TYPE:: D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST;*/
            }
            return D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        };

    PrimitiveTopologyType = PrimTopology(Desc.PrimitiveTopologyType);

    RootSignature = std::make_shared<D3D12RootSignature>(Owner->GetDevice(), Desc.DescriptorSetLayout);
    PSODesc.pRootSignature = RootSignature->Get();

    VertexAttribute = std::make_shared<D3D12VertexAttribute>(Desc.VertexLayout);

    auto InputLayout = VertexAttribute->Get();
    PSODesc.InputLayout.NumElements = (UINT)VertexAttribute->GetSize();
    PSODesc.InputLayout.pInputElementDescs = InputLayout.data();

    PSODesc.NodeMask = 0; auto BlendState = D3D12BlendState(Desc.BlendAttribute);
    PSODesc.BlendState = BlendState.Get();
    auto RasterState = D3D12Rasterizer(Desc.RasterizerAttribute/*, !Owner->IsSoftwareDevice() && PrimitiveTopologyType == D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST*/);
    PSODesc.RasterizerState = RasterState.Get();
    PSODesc.SampleMask = 0xFFFFFFFFu;
    PSODesc.SampleDesc.Count = 1;
    PSODesc.SampleDesc.Quality = 0;
    auto DSState = D3D12DepthStencilState(Desc.DepthStencilAttribute);
    PSODesc.DepthStencilState = DSState.Get();
    PSODesc.PrimitiveTopologyType = PrimTopoType(Desc.PrimitiveTopologyType);

    //PSODesc.StreamOutput;
    //PSODesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE::D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    //PSODesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    auto Device = Owner->GetDevice();
    ThrowIfFailed(Device->CreateGraphicsPipelineState(&PSODesc, IID_PPV_ARGS(&PSO)));

    //for (auto& Blob : Blobs)
    //    Blob = nullptr;
    //Blobs.clear();

    PSO->SetName(L"");

    Compiled = true;
}

bool D3D12Pipeline::Recompile()
{
    if (!Compiled)
        return false;

    std::vector<ComPtr<ID3DBlob>> Blobs;

    for (auto& Attachment : ShaderAttachments)
    {
        auto pShader = Owner->CompileShader(Attachment);
        //Blobs.push_back(Shader.GetBlob());

        switch (Attachment.Type)
        {
        case eShaderType::Vertex:
        {
            PSODesc.VS.pShaderBytecode = pShader->GetByteCode();
            PSODesc.VS.BytecodeLength = pShader->GetByteCodeSize();
            break;
        }
        case eShaderType::Pixel:
        {
            PSODesc.PS.pShaderBytecode = pShader->GetByteCode();
            PSODesc.PS.BytecodeLength = pShader->GetByteCodeSize();
            break;
        }
        case eShaderType::Geometry:
        {
            PSODesc.GS.pShaderBytecode = pShader->GetByteCode();
            PSODesc.GS.BytecodeLength = pShader->GetByteCodeSize();
            break;
        }
        case eShaderType::HULL:
        {
            PSODesc.HS.pShaderBytecode = pShader->GetByteCode();
            PSODesc.HS.BytecodeLength = pShader->GetByteCodeSize();
            break;
        }
        case eShaderType::Domain:
        {
            PSODesc.DS.pShaderBytecode = pShader->GetByteCode();
            PSODesc.DS.BytecodeLength = pShader->GetByteCodeSize();
            break;
        }
        case eShaderType::Compute:
        {
            break;
        }
        case eShaderType::Amplification:
        {
            break;
        }
        case eShaderType::Mesh:
        {
            break;
        }
        }
    }

    PSODesc.pRootSignature = RootSignature->Get();

    auto InputLayout = VertexAttribute->Get();
    PSODesc.InputLayout.NumElements = (UINT)VertexAttribute->GetSize();
    PSODesc.InputLayout.pInputElementDescs = InputLayout.data();

    auto Device = Owner->GetDevice();
    ThrowIfFailed(Device->CreateGraphicsPipelineState(&PSODesc, IID_PPV_ARGS(&PSO)));

    for (auto& Blob : Blobs)
        Blob = nullptr;
    Blobs.clear();

    return true;
}

D3D12ComputePipeline::D3D12ComputePipeline(D3D12Device* InOwner, const std::string& InName, const sComputePipelineDesc& InDesc)
    : Owner(InOwner)
    , Name(InName)
    , Desc(InDesc)
    , ShaderAttachment(InDesc.ShaderAttachment)
{
    ZeroMemory(&ComputePipelineDesc, sizeof(ComputePipelineDesc));
    
    auto pShader = Owner->CompileShader(ShaderAttachment);
    ComputePipelineDesc.CS.pShaderBytecode = pShader->GetByteCode();
    ComputePipelineDesc.CS.BytecodeLength = pShader->GetByteCodeSize();

    RootSignature = std::make_shared<D3D12RootSignature>(Owner->GetDevice(), InDesc.DescriptorSetLayout);
    ComputePipelineDesc.pRootSignature = RootSignature->Get();

    auto Device = Owner->GetDevice();
    ThrowIfFailed(Device->CreateComputePipelineState(&ComputePipelineDesc, IID_PPV_ARGS(&ComputePipeline)));
}

D3D12ComputePipeline::~D3D12ComputePipeline()
{
    Owner = nullptr;
    ComputePipeline = nullptr;
    RootSignature = nullptr;
}

void D3D12ComputePipeline::ApplyPipeline(ID3D12GraphicsCommandList* Context) const
{
    Context->SetComputeRootSignature(RootSignature->Get());
    Context->SetPipelineState(ComputePipeline.Get());
}

bool D3D12ComputePipeline::Recompile()
{
    auto pShader = Owner->CompileShader(ShaderAttachment);
    ComputePipelineDesc.CS.pShaderBytecode = pShader->GetByteCode();
    ComputePipelineDesc.CS.BytecodeLength = pShader->GetByteCodeSize();

    RootSignature = std::make_shared<D3D12RootSignature>(Owner->GetDevice(), Desc.DescriptorSetLayout);
    ComputePipelineDesc.pRootSignature = RootSignature->Get();

    auto Device = Owner->GetDevice();
    ThrowIfFailed(Device->CreateComputePipelineState(&ComputePipelineDesc, IID_PPV_ARGS(&ComputePipeline)));

    return true;
}
