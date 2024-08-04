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

#include "D3D11Device.h"
#include <string>
#include "Engine/ClassBody.h"
#include "Engine/AbstractEngine.h"
#include "GI/D3DShared/D3DShared.h"

/*
* Create
* Base Image Texture2D
* Base Empty Texture2D
* Base Empty Texture3D
*/

class D3D11Texture final : public ITexture2D
{
	sClassBody(sClassConstructor, D3D11Texture, ITexture2D)
public:
	D3D11Texture(D3D11Device* InDevice, const std::wstring FilePath, const std::string InName = std::string(), std::uint32_t RootParameterIndex = 0);
	D3D11Texture(D3D11Device* InDevice, const std::string InName, void* InBuffer, const std::size_t InSize, const sTextureDesc& InDesc, std::uint32_t RootParameterIndex = 0);
	D3D11Texture(D3D11Device* InDevice, const std::string InName, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex = 0);

	virtual ~D3D11Texture()
	{
		Owner = nullptr;
		pTexture = nullptr;
		pTextureSRV = nullptr;
	}

	virtual std::string GetName() const override final { return Name; }
	virtual std::wstring GetPath() const override final { return Path; }

	virtual sTextureDesc GetDesc() const override final { return Desc; }

	virtual void SetDefaultRootParameterIndex(std::uint32_t inRootParameterIndex) override final { RootParameterIndex = inRootParameterIndex; }
	virtual std::uint32_t GetDefaultRootParameterIndex() const override final { return RootParameterIndex; }

	virtual void UpdateTexture(ITexture2D* SourceTexture, std::size_t SourceArrayIndex, std::size_t ArrayIndex, const std::optional<IntVector2> Dest = std::nullopt, const std::optional<FBounds2D> TargetBounds = std::nullopt) override final;
	virtual void UpdateTexture(const std::wstring FilePath, std::size_t ArrayIndex, const std::optional<IntVector2> Dest = std::nullopt, const std::optional<FBounds2D> TargetBounds = std::nullopt) override final;
	virtual void UpdateTexture(const void* pSrcData, const std::size_t InSize, const FDimension2D& Dimension, std::size_t ArrayIndex, const std::optional<IntVector2> Dest = std::nullopt, const std::optional<FBounds2D> TargetBounds = std::nullopt) override final;

	virtual void UpdateTexture(const void* pSrcData, std::size_t RowPitch, std::size_t MinX, std::size_t MinY, std::size_t MaxX, std::size_t MaxY, IGraphicsCommandContext* InCommandBuffer = nullptr) override final;
	void ApplyTexture(ID3D11DeviceContext1* CMD, std::uint32_t InSlot, eShaderType InType = eShaderType::Pixel);

	virtual void SaveToFile(std::wstring InPath) const override final;

	ID3D11Texture2D* GetTexture() const { return pTexture.Get(); }
	ID3D11ShaderResourceView* GetTextureSRV() const { return pTextureSRV.Get(); }

private:
	D3D11Device* Owner;

	std::string Name;
	std::wstring Path;
	sTextureDesc Desc;

	ComPtr<ID3D11Texture2D> pTexture;
	ComPtr<ID3D11ShaderResourceView> pTextureSRV;

	std::uint32_t RootParameterIndex;
};
