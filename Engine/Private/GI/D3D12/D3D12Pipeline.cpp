/* ---------------------------------------------------------------------------------------
* MIT License
*
* Copyright (c) 2022 Davut Coþkun.
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

D3D12Pipeline::D3D12Pipeline(D3D12Device* InOwner, const std::string& InName, const sPipelineDesc& InDesc)
    : Owner(InOwner)
    , Name(InName)
    , Desc(InDesc)
{
    ZeroMemory(&PSODesc, sizeof(PSODesc));
    PSODesc.NodeMask = 1;
    PSODesc.SampleMask = 0xFFFFFFFFu;
    PSODesc.SampleDesc.Count = 1;
    PSODesc.InputLayout.NumElements = 0;

    ShaderAttachments = InDesc.ShaderAttachments;

    std::vector<ComPtr<ID3DBlob>> Blobs;

    for (auto& Attachment : ShaderAttachments)
    {
        D3D12Shader Shader(Attachment);

        Blobs.push_back(Shader.GetBlob());
        switch (Attachment.Type)
        {
        case eShaderType::Vertex:
        {
            PSODesc.VS.pShaderBytecode = Shader.GetByteCode();
            PSODesc.VS.BytecodeLength = Shader.GetByteCodeSize();
            break;
        }
        case eShaderType::Pixel:
        {
            PSODesc.PS.pShaderBytecode = Shader.GetByteCode();
            PSODesc.PS.BytecodeLength = Shader.GetByteCodeSize();
            break;
        }
        case eShaderType::Geometry:
        {
            PSODesc.GS.pShaderBytecode = Shader.GetByteCode();
            PSODesc.GS.BytecodeLength = Shader.GetByteCodeSize();
            break;
        }
        case eShaderType::HULL:
        {
            PSODesc.HS.pShaderBytecode = Shader.GetByteCode();
            PSODesc.HS.BytecodeLength = Shader.GetByteCodeSize();
            break;
        }
        case eShaderType::Domain:
        {
            PSODesc.DS.pShaderBytecode = Shader.GetByteCode();
            PSODesc.DS.BytecodeLength = Shader.GetByteCodeSize();
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

    PrimitiveTopologyType = PrimTopology(InDesc.PrimitiveTopologyType);

    RootSignature = std::make_shared<D3D12RootSignature>(Owner->GetDevice(), InDesc.DescriptorSetLayout);
    PSODesc.pRootSignature = RootSignature->Get();

    VertexAttribute = std::make_shared<D3D12VertexAttribute>(InDesc.VertexLayout);

    auto InputLayout = VertexAttribute->Get();
    PSODesc.InputLayout.NumElements = (UINT)VertexAttribute->GetSize();
    PSODesc.InputLayout.pInputElementDescs = InputLayout.data();

    PSODesc.NodeMask = 0; auto BlendState = D3D12BlendState(InDesc.BlendAttribute);
    PSODesc.BlendState = BlendState.Get();
    auto RasterState = D3D12Rasterizer(InDesc.RasterizerAttribute);
    PSODesc.RasterizerState = RasterState.Get();
    PSODesc.SampleMask = 0xFFFFFFFFu;
    PSODesc.SampleDesc.Count = 1;
    PSODesc.SampleDesc.Quality = 0;
    auto DSState = D3D12DepthStencilState(InDesc.DepthStencilAttribute);
    PSODesc.DepthStencilState = DSState.Get();
    PSODesc.PrimitiveTopologyType = PrimTopoType(InDesc.PrimitiveTopologyType);

    //PSODesc.StreamOutput;
    //PSODesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE::D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    //PSODesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    PSODesc.DSVFormat = GetDepthViewFormat(ConvertFormat_Format_To_DXGI(InDesc.DSVFormat));

    for (UINT i = 0; i < 8; i++)
        PSODesc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
    for (UINT i = 0; i < InDesc.NumRenderTargets; i++)
        PSODesc.RTVFormats[i] = ConvertFormat_Format_To_DXGI(InDesc.RTVFormats[i]);

    PSODesc.NumRenderTargets = InDesc.NumRenderTargets;

    auto Device = Owner->GetDevice();
    ThrowIfFailed(Device->CreateGraphicsPipelineState(&PSODesc, IID_PPV_ARGS(&PSO)));

    for (auto& Blob : Blobs)
        Blob = nullptr;
    Blobs.clear();

    PSO->SetName(L"");
}

D3D12Pipeline::~D3D12Pipeline()
{
    Owner = nullptr;

    RootSignature = nullptr;

    ShaderAttachments.clear();
    VertexAttribute = nullptr;

    PSO = nullptr;
}

void D3D12Pipeline::ApplyPipeline(ID3D12GraphicsCommandList* CommandList) const
{
    CommandList->IASetPrimitiveTopology(PrimitiveTopologyType);
    CommandList->SetGraphicsRootSignature(RootSignature->Get());
    CommandList->SetPipelineState(PSO.Get());
}

void D3D12Pipeline::Recompile()
{
    std::vector<ComPtr<ID3DBlob>> Blobs;

    for (auto& Attachment : ShaderAttachments)
    {
        D3D12Shader Shader(Attachment);

        Blobs.push_back(Shader.GetBlob());
        switch (Attachment.Type)
        {
        case eShaderType::Vertex:
        {
            PSODesc.VS.pShaderBytecode = Shader.GetByteCode();
            PSODesc.VS.BytecodeLength = Shader.GetByteCodeSize();
            break;
        }
        case eShaderType::Pixel:
        {
            PSODesc.PS.pShaderBytecode = Shader.GetByteCode();
            PSODesc.PS.BytecodeLength = Shader.GetByteCodeSize();
            break;
        }
        case eShaderType::Geometry:
        {
            PSODesc.GS.pShaderBytecode = Shader.GetByteCode();
            PSODesc.GS.BytecodeLength = Shader.GetByteCodeSize();
            break;
        }
        case eShaderType::HULL:
        {
            PSODesc.HS.pShaderBytecode = Shader.GetByteCode();
            PSODesc.HS.BytecodeLength = Shader.GetByteCodeSize();
            break;
        }
        case eShaderType::Domain:
        {
            PSODesc.DS.pShaderBytecode = Shader.GetByteCode();
            PSODesc.DS.BytecodeLength = Shader.GetByteCodeSize();
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
}

D3D12ComputePipeline::D3D12ComputePipeline(D3D12Device* InOwner, const std::string& InName, const sComputePipelineDesc& InDesc)
    : Owner(InOwner)
    , Name(InName)
    , Desc(InDesc)
    , ShaderAttachment(InDesc.ShaderAttachment)
{
    ZeroMemory(&ComputePipelineDesc, sizeof(ComputePipelineDesc));

    D3D12Shader Shader(ShaderAttachment);
    ComputePipelineDesc.CS.pShaderBytecode = Shader.GetByteCode();
    ComputePipelineDesc.CS.BytecodeLength = Shader.GetByteCodeSize();

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

void D3D12ComputePipeline::Recompile()
{
    D3D12Shader Shader(ShaderAttachment);
    ComputePipelineDesc.CS.pShaderBytecode = Shader.GetByteCode();
    ComputePipelineDesc.CS.BytecodeLength = Shader.GetByteCodeSize();

    RootSignature = std::make_shared<D3D12RootSignature>(Owner->GetDevice(), Desc.DescriptorSetLayout);
    ComputePipelineDesc.pRootSignature = RootSignature->Get();

    auto Device = Owner->GetDevice();
    ThrowIfFailed(Device->CreateComputePipelineState(&ComputePipelineDesc, IID_PPV_ARGS(&ComputePipeline)));
}
