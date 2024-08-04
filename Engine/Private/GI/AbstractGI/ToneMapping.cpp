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

sToneMapping::sToneMapping(std::size_t Width, std::size_t Height)
    : TonemapperIndex(4)
{
    {
        /*sFrameBufferAttachmentInfo AttachmentInfo;
        AttachmentInfo.Desc.Dimensions.X = (std::uint32_t)Width;
        AttachmentInfo.Desc.Dimensions.Y = (std::uint32_t)Height;
        AttachmentInfo.AddFrameBuffer(GPU::GetBackBufferFormat());
        AttachmentInfo.DepthFormat = GPU::GetDefaultDepthFormat();
        PostProcessFB = IFrameBuffer::Create("PostProcessFBO", AttachmentInfo);*/
        PostProcessFB = IRenderTarget::Create("sToneMapping", GPU::GetBackBufferFormat(), sFBODesc(sFBODesc::sFBODimension((std::uint32_t)Width, (std::uint32_t)Height)));
    }

    const sShaderAttachment PostProcessShader = sShaderAttachment(L"..//Content\\Shaders\\Tonemapping.hlsl", "mainPS", eShaderType::Pixel);
    std::vector<sDescriptorSetLayoutBinding> DescriptorSetLayout;
    {
        DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eUniformBuffer, eShaderType::Pixel, 0));
        DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(EDescriptorType::eTexture, eShaderType::Pixel, 0));
        sSamplerAttributeDesc SamplerDesc;
        SamplerDesc.Filter = ESamplerFilter::ePoint;
        SamplerDesc.AddressU = ESamplerAddressMode::eClamp;
        SamplerDesc.AddressV = ESamplerAddressMode::eClamp;
        SamplerDesc.AddressW = ESamplerAddressMode::eClamp;
        SamplerDesc.SamplerComparisonFunction = ECompareFunction::eAlways;
        SamplerDesc.BorderColor = FColor::Transparent();
        SamplerDesc.MinMipLevel = 0.0f;
        SamplerDesc.MaxMipLevel = FLT_MAX;
        SamplerDesc.MipBias = 0;
        SamplerDesc.MaxAnisotropy = 1;
        DescriptorSetLayout.push_back(sDescriptorSetLayoutBinding(SamplerDesc, eShaderType::Pixel, 0));
    }
    sDepthStencilAttributeDesc DepthStencil = sDepthStencilAttributeDesc(false);
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
}

void sToneMapping::SetFrameBufferSize(const std::size_t InWidth, const std::size_t InHeight)
{
    /*sFrameBufferAttachmentInfo AttachmentInfo;
    AttachmentInfo.Desc.Dimensions.X = (std::uint32_t)InWidth;
    AttachmentInfo.Desc.Dimensions.Y = (std::uint32_t)InHeight;
    AttachmentInfo.AddFrameBuffer(GPU::GetBackBufferFormat());
    AttachmentInfo.DepthFormat = GPU::GetDefaultDepthFormat();
    PostProcessFB = IFrameBuffer::Create("PostProcessFBO", AttachmentInfo);*/
    PostProcessFB = IRenderTarget::Create("sToneMapping", GPU::GetBackBufferFormat(), sFBODesc(sFBODesc::sFBODimension((std::uint32_t)InWidth, (std::uint32_t)InHeight)));
}

void sToneMapping::SetPostProcessResources(IGraphicsCommandContext* Context)
{
    Context->SetConstantBuffer(ToneMappingCB.get());
}

void sToneMapping::SetTonemapper(int Val)
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
