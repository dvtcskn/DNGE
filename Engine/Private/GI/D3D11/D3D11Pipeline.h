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
#pragma once

#include <map>
#include "Engine/ClassBody.h"
#include "Engine/AbstractEngine.h"

#include "D3D11Device.h"
#include "D3D11Shader.h"
#include "D3D11ShaderStates.h"

class D3D11Pipeline final : public IPipeline
{
	sClassBody(sClassConstructor, D3D11Pipeline, IPipeline)
public:
	D3D11Pipeline(D3D11Device* InDevice, const std::string& InName, const sPipelineDesc& InDesc);

	virtual ~D3D11Pipeline()
	{
		Owner = nullptr;

		BlendState = nullptr;
		DepthStencilState = nullptr;
		RasterizerState = nullptr;

		for (auto& Sampler : SamplerStates)
			Sampler.second = nullptr;
		SamplerStates.clear();

		for (auto& Shader : Shaders)
			Shader.second = nullptr;
		Shaders.clear();

		VertexAttribute = nullptr;
		InputLayout = nullptr;

		DescriptorSetLayout.clear();
		ShaderAttachments.clear();
	}

	void ApplyPipeline(ID3D11DeviceContext1* Context, std::uint32_t StencilRef = 0) const;
	virtual sPipelineDesc GetPipelineDesc() const override final { return Desc; }

	virtual void Recompile() override final;

	std::size_t GetDescriptorSetLayoutBindingSize() const { return DescriptorSetLayout.size(); }
	std::vector<sDescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings() const { return DescriptorSetLayout; }
	sDescriptorSetLayoutBinding GetDescriptorSetLayoutBinding(std::size_t index) const { return DescriptorSetLayout.at(index); }
	std::optional<sDescriptorSetLayoutBinding> GetDescriptorSetConstantBufferLayoutBinding(std::size_t index) const;
	std::optional<sDescriptorSetLayoutBinding> GetDescriptorSetTextureLayoutBinding(std::size_t index) const;

	void SetStencilRef(ID3D11DeviceContext1* Context, std::uint32_t Ref);

private:
	D3D11Device* Owner;

	std::string Name;
	sPipelineDesc Desc;

	D3D_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;

	ComPtr<ID3D11BlendState1> BlendState;
	ComPtr<ID3D11DepthStencilState> DepthStencilState;
	ComPtr<ID3D11RasterizerState1> RasterizerState;
	std::map<std::uint32_t, ComPtr<ID3D11SamplerState>> SamplerStates;
	ComPtr<ID3D11InputLayout> InputLayout;
	std::unique_ptr<D3D11VertexAttribute> VertexAttribute;

	std::vector<sDescriptorSetLayoutBinding> DescriptorSetLayout;
	std::vector<sShaderAttachment> ShaderAttachments;
	std::map<eShaderType, ComPtr<ID3D11DeviceChild>> Shaders;
};

class D3D11ComputePipeline : public IComputePipeline
{
	sClassBody(sClassConstructor, D3D11ComputePipeline, IComputePipeline)
public:
	D3D11ComputePipeline(D3D11Device* InDevice, const std::string& InName, const sComputePipelineDesc& InDesc);

	virtual ~D3D11ComputePipeline()
	{
		Owner = nullptr;

		for (auto& Sampler : SamplerStates)
			Sampler.second = nullptr;
		SamplerStates.clear();

		Shader = nullptr;
	}

public:
	virtual sComputePipelineDesc GetPipelineDesc() const override final { return Desc; }
	virtual void Recompile() override final;

	void ApplyPipeline(ID3D11DeviceContext1* Context) const;

	std::size_t GetDescriptorSetLayoutBindingSize() const { return Desc.DescriptorSetLayout.size(); }
	std::vector<sDescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings() const { return Desc.DescriptorSetLayout; }
	sDescriptorSetLayoutBinding GetDescriptorSetLayoutBinding(std::size_t index) const { return Desc.DescriptorSetLayout.at(index); }

private:
	D3D11Device* Owner;

	std::string Name;
	sComputePipelineDesc Desc;

	std::map<std::uint32_t, ComPtr<ID3D11SamplerState>> SamplerStates;

	ComPtr<ID3D11DeviceChild> Shader;
	sShaderAttachment ShaderAttachment;
};
