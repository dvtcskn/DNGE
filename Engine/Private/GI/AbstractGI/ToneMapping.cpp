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

#include "pch.h"
#include "AbstractGI/ToneMapping.h"
#include "Utilities/FileManager.h"

struct TonemappingPerFrameResources : public IBindlessSceneDescriptor
{
    std::uint32_t HDRTexture = std::uint32_t(-1);
    std::uint32_t Sampler = std::uint32_t(-1);
    std::uint32_t Attributes = std::uint32_t(-1);
};

struct sTonemappingSceneDescriptor : public IBindlessSceneContainer
{
    TonemappingPerFrameResources Descriptor;

    sTonemappingSceneDescriptor(TonemappingPerFrameResources NewDescriptor)
        : Descriptor(NewDescriptor)
    {}

    virtual const IBindlessSceneDescriptor* GetDescriptor() const override final
    {
        return &Descriptor;
    }

    virtual std::vector<std::uint32_t> GetAllBindlessIndices() const override final
    {
        std::vector<std::uint32_t> BindlessIndices;
        BindlessIndices.push_back(Descriptor.HDRTexture);
        BindlessIndices.push_back(Descriptor.Sampler);
        BindlessIndices.push_back(Descriptor.Attributes);
        return BindlessIndices;
    }

    virtual std::vector<std::uint32_t> GetAllTextureBindlessIndices() const override final
    {
        std::vector<std::uint32_t> BindlessIndices;
        BindlessIndices.push_back(Descriptor.HDRTexture);
        return BindlessIndices;
    }

    virtual std::uint32_t GetTextureSize() const override final
    {
        return 1;
    }

    virtual std::vector<std::uint32_t> GetAllSamplerBindlessIndices() const override final
    {
        std::vector<std::uint32_t> BindlessIndices;
        BindlessIndices.push_back(Descriptor.Sampler);
        return BindlessIndices;
    }

    virtual std::uint32_t GetSamplerSize() const override final
    {
        return 1;
    }

    virtual std::vector<std::uint32_t> GetAllConstantBufferBindlessIndices() const override final
    {
        std::vector<std::uint32_t> BindlessIndices;
        BindlessIndices.push_back(Descriptor.Attributes);
        return BindlessIndices;
    }

    virtual std::uint32_t GetConstantBuffereSize() const override final
    {
        return 1;
    }
};

sToneMapping::sToneMapping(std::size_t Width, std::size_t Height)
    : TonemapperIndex(4)
{
    PostProcessFB = IRenderTarget::Create("sToneMapping", GPU::GetBackBufferFormat(), sFBODesc(sFBODesc::sFBODimension((std::uint32_t)Width, (std::uint32_t)Height)));

    const sShaderAttachment PostProcessShader = sShaderAttachment(FileManager::GetShaderFolderW() + L"Tonemapping.hlsl", "mainPS", eShaderType::Pixel);
    std::vector<sShaderBinding> DescriptorSetLayout;
    DescriptorSetLayout.push_back(sShaderBinding(EDescriptorType::e32BitConstant, eShaderType::Pixel, 0, 3));

    sSamplerAttributeDesc SamplerDesc;
    SamplerDesc.Filter = ESamplerFilter::Point;
    SamplerDesc.AddressU = ESamplerAddressMode::Clamp;
    SamplerDesc.AddressV = ESamplerAddressMode::Clamp;
    SamplerDesc.AddressW = ESamplerAddressMode::Clamp;
    SamplerDesc.SamplerComparisonFunction = ECompareFunction::Always;
    SamplerDesc.BorderColor = FColor::Transparent();
    SamplerDesc.MinMipLevel = 0.0f;
    SamplerDesc.MaxMipLevel = FLT_MAX;
    SamplerDesc.MipBias = 0;
    SamplerDesc.MaxAnisotropy = 1;
    Sampler = ISamplerState::Create("sToneMapping_Sampler", SamplerDesc);

    const sDepthStencilAttributeDesc DepthStencil = sDepthStencilAttributeDesc(ECompareFunction::LessEqual, false);
    const sBlendAttributeDesc Blend = sBlendAttributeDesc();

    SetPipeline(PostProcessShader, DescriptorSetLayout, DepthStencil, Blend);

    {
        BufferLayout BufferDesc;
        BufferDesc.Size = sizeof(sToneMapping::sToneMappingConstants);
        ToneMappingCB = IConstantBuffer::Create("ToneMappingCB", BufferDesc, 0);

        sToneMappingConstants ToneMappingConstants;
        ToneMappingConstants.exposure = 1.0f;
        ToneMappingConstants.gamma2 = 0;
        ToneMappingConstants.toneMapper = TonemapperIndex;
        ToneMappingCB->Map(&ToneMappingConstants);
    }
}

sToneMapping::~sToneMapping()
{
    PostProcessFB = nullptr;
    ToneMappingCB = nullptr;
    Sampler = nullptr;
}

void sToneMapping::SetFrameBufferSize(const std::size_t InWidth, const std::size_t InHeight)
{
    PostProcessFB = IRenderTarget::Create("sToneMapping", GPU::GetBackBufferFormat(), sFBODesc(sFBODesc::sFBODimension((std::uint32_t)InWidth, (std::uint32_t)InHeight)));
}

void sToneMapping::SetPostProcessResources(IGraphicsCommandContext* Context, IRenderTarget* BackBuffer)
{
    if (!Context || !BackBuffer)
        return;

    if (GPU::IsBindlessRendererEnabled())
    {
        TonemappingPerFrameResources PerFrameResources;
        PerFrameResources.HDRTexture = BackBuffer->GetSRVBindlessIndex();
        PerFrameResources.Sampler = Sampler->GetBindlessIndex();
        PerFrameResources.Attributes = ToneMappingCB->GetBindlessIndex();

        //Context->Set32BitConstants(0, &PerFrameResources, 3, 0);
        sTonemappingSceneDescriptor Descriptor(PerFrameResources);
        Context->SetBindlessDescriptor(0, &Descriptor);
    }
    else
    {
        Context->SetConstantBuffer(ToneMappingCB.get());
       
        if (BackBuffer)
            Context->SetRenderTargetAsResource(BackBuffer, 0);
    }
}

void sToneMapping::SetTonemapper(std::uint32_t Val)
{
    TonemapperIndex = Val;
    sToneMappingConstants ToneMappingConstants;
    ToneMappingConstants.exposure = 1.0f;
    ToneMappingConstants.gamma2 = 0;
    ToneMappingConstants.toneMapper = TonemapperIndex;
    ToneMappingCB->Map(&ToneMappingConstants);
}

int sToneMapping::GetTonemapperIndex() const
{
    return TonemapperIndex;
}
