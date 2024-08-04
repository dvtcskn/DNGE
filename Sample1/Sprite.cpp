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
#include "Sprite.h"
#include <Core/MeshPrimitives.h>
#include <AbstractGI/MaterialManager.h>
#include <Gameplay/BoxCollision2DComponent.h>
#include "AssetManager.h"

sSprite::sSprite(const std::string& InName, const std::wstring& TextureAtlasPath, const FBounds2D& InSpriteBound, sMaterial* Mat)
	: Name(InName)
	, SpriteBound(InSpriteBound)
	, MaterialInstance(nullptr)
{
	//AnimationCB = IConstantBuffer::Create(Name + "_FlipConstantBuffer", BufferLayout(sizeof(sFlipConstantBuffer), 0), 3);
	//{
	//	//AnimationConstantBuffer.Index = 0;
	//	AnimationConstantBuffer.Flip = 0;
	//	AnimationCB->Map(&AnimationConstantBuffer);
	//}

	MaterialInstance = Mat->CreateInstance(Name + "_Instance").get();
	auto AtlasTexture = ITexture2D::Create(TextureAtlasPath, "DefaultActorAtlas");
	sTextureDesc Desc;
	Desc.Dimensions.X = (std::uint32_t)InSpriteBound.GetWidth();
	Desc.Dimensions.Y = (std::uint32_t)InSpriteBound.GetHeight();
	Desc.MipLevels = 1;
	Desc.Format = AtlasTexture->GetDesc().Format;
	auto Texture = ITexture2D::CreateEmpty(InName, Desc, 2);
	Texture->UpdateTexture(AtlasTexture.get(), 0, 0, IntVector2(0, 0), InSpriteBound);
	MaterialInstance->AddTexture(Texture);
	//MaterialInstance->BindConstantBuffer(AnimationCB);

	{
		const auto Plane = MeshPrimitives::Create2DPlaneVerticesFromDimension(InSpriteBound.GetDimension());
		auto PlaneTC = MeshPrimitives::GeneratePlaneTextureCoordinate(0.0f);

		/*const unsigned int mip = 0;
		int32_t mipWidth = std::max(1U, (unsigned int)(4096 >> mip));
		uint32_t mipHeight = std::max(1U, (unsigned int)(4096 >> mip));

		const float invWidth = 1.0f / mipWidth;
		const float invHeight = 1.0f / mipHeight;

		const auto Bounds = FBounds2D(FVector2((InSpriteBound.Min.X) * invWidth, (InSpriteBound.Min.Y) * invHeight),
			FVector2((InSpriteBound.Max.X) * invWidth, (InSpriteBound.Max.Y) * invHeight));

		FVector2 P1 = Bounds.GetCorner(0);
		FVector2 P2 = Bounds.GetCorner(1);
		FVector2 P3 = Bounds.GetCorner(2);
		FVector2 P4 = Bounds.GetCorner(3);

		std::array<FVector2, 4> PlaneTC = { P1, P2, P3, P4 };*/

		std::vector<sVertexLayout> Vertices;
		for (std::size_t i = 0; i < Plane.size(); i++)
		{
			auto& Verts = Plane.at(i);
			auto& TC = PlaneTC.at(i);
			sVertexLayout VBE;
			VBE.position = FVector(Verts.X, Verts.Y, 0.0f);
			VBE.texCoord = TC;
			VBE.Color = FColor::Green();
			Vertices.push_back(VBE);
		}
		std::vector<std::uint32_t> Indices = MeshPrimitives::GeneratePlaneIndices(1);
		ObjectDrawParameters.IndexCountPerInstance = (uint32_t)Indices.size();

		{
			BufferSubresource Subresource = BufferSubresource(Vertices.data(), Vertices.size() * sizeof(sVertexLayout));
			VertexBuffer = (IVertexBuffer::Create(Name, BufferLayout(Vertices.size() * sizeof(sVertexLayout), sizeof(sVertexLayout)), &Subresource));
		}
		{
			BufferSubresource Subresource = BufferSubresource(Indices.data(), Indices.size() * sizeof(std::uint32_t));
			IndexBuffer = (IIndexBuffer::Create(Name, BufferLayout(Indices.size() * sizeof(std::uint32_t), sizeof(std::uint32_t)), &Subresource));
		}
	}
}

sSprite::sSprite(const std::string& InName, const std::wstring& TextureAtlasPath, const FBounds2D& InSpriteBound, const std::string& DefaultMaterialName)
	: sSprite(InName, TextureAtlasPath, InSpriteBound, sMaterialManager::Get().GetMaterial(DefaultMaterialName))
{}

sSprite::sSprite(const std::string& InName, ITexture2D* TextureAtlas, const FBounds2D& InSpriteBound, sMaterial* Mat)
{
	MaterialInstance = Mat->CreateInstance(Name + "_Instance").get();
	sTextureDesc Desc;
	Desc.Dimensions.X = (std::uint32_t)InSpriteBound.GetWidth();
	Desc.Dimensions.Y = (std::uint32_t)InSpriteBound.GetHeight();
	Desc.Format = TextureAtlas->GetDesc().Format;
	Desc.MipLevels = 1;
	auto Texture = ITexture2D::CreateEmpty(InName, Desc, 2);
	Texture->UpdateTexture(TextureAtlas, 0, 0, IntVector2(0, 0), InSpriteBound);
	MaterialInstance->AddTexture(Texture);

	{
		const auto Plane = MeshPrimitives::Create2DPlaneVerticesFromDimension(InSpriteBound.GetDimension());
		auto PlaneTC = MeshPrimitives::GeneratePlaneTextureCoordinate(0.0f);

		/*const unsigned int mip = 0;
		int32_t mipWidth = std::max(1U, (unsigned int)(4096 >> mip));
		uint32_t mipHeight = std::max(1U, (unsigned int)(4096 >> mip));

		const float invWidth = 1.0f / mipWidth;
		const float invHeight = 1.0f / mipHeight;

		const auto Bounds = FBounds2D(FVector2((InSpriteBound.Min.X) * invWidth, (InSpriteBound.Min.Y) * invHeight),
			FVector2((InSpriteBound.Max.X) * invWidth, (InSpriteBound.Max.Y) * invHeight));

		FVector2 P1 = Bounds.GetCorner(0);
		FVector2 P2 = Bounds.GetCorner(1);
		FVector2 P3 = Bounds.GetCorner(2);
		FVector2 P4 = Bounds.GetCorner(3);

		std::array<FVector2, 4> PlaneTC = { P1, P2, P3, P4 };*/

		std::vector<sVertexLayout> Vertices;
		for (std::size_t i = 0; i < Plane.size(); i++)
		{
			auto& Verts = Plane.at(i);
			auto& TC = PlaneTC.at(i);
			sVertexLayout VBE;
			VBE.position = FVector(Verts.X, Verts.Y, 0.0f);
			VBE.texCoord = TC;
			VBE.Color = FColor::Green();
			Vertices.push_back(VBE);
		}
		std::vector<std::uint32_t> Indices = MeshPrimitives::GeneratePlaneIndices(1);
		ObjectDrawParameters.IndexCountPerInstance = (uint32_t)Indices.size();

		{
			BufferSubresource Subresource = BufferSubresource(Vertices.data(), Vertices.size() * sizeof(sVertexLayout));
			VertexBuffer = (IVertexBuffer::Create(Name, BufferLayout(Vertices.size() * sizeof(sVertexLayout), sizeof(sVertexLayout)), &Subresource));
		}
		{
			BufferSubresource Subresource = BufferSubresource(Indices.data(), Indices.size() * sizeof(std::uint32_t));
			IndexBuffer = (IIndexBuffer::Create(Name, BufferLayout(Indices.size() * sizeof(std::uint32_t), sizeof(std::uint32_t)), &Subresource));
		}
	}
}

sSprite::sSprite(const std::string& InName, ITexture2D* TextureAtlas, const FBounds2D& InSpriteBound, const std::string& DefaultMaterialName)
	: sSprite(InName, TextureAtlas, InSpriteBound, sMaterialManager::Get().GetMaterial(DefaultMaterialName))
{}

sSprite::sSprite(const std::string& InName, const FBounds2D& InSpriteBound, sMaterial::sMaterialInstance* AtlasMaterialInstance)
	: Name(InName)
	, SpriteBound(InSpriteBound)
	, MaterialInstance(AtlasMaterialInstance)
{
	{
		const auto Plane = MeshPrimitives::Create2DPlaneVerticesFromDimension(InSpriteBound.GetDimension());

		const unsigned int mip = 0;
		int32_t mipWidth = std::max(1U, (unsigned int)(4096 >> mip));
		uint32_t mipHeight = std::max(1U, (unsigned int)(4096 >> mip));

		const float invWidth = 1.0f / mipWidth;
		const float invHeight = 1.0f / mipHeight;

		const auto Bounds = FBounds2D(FVector2((InSpriteBound.Min.X) * invWidth, (InSpriteBound.Min.Y) * invHeight),
			FVector2((InSpriteBound.Max.X) * invWidth, (InSpriteBound.Max.Y) * invHeight));

		FVector2 P1 = Bounds.GetCorner(0);
		FVector2 P2 = Bounds.GetCorner(1);
		FVector2 P3 = Bounds.GetCorner(2);
		FVector2 P4 = Bounds.GetCorner(3);

		std::array<FVector2, 4> PlaneTC = { P1, P2, P3, P4 };

		std::vector<sVertexLayout> Vertices;
		for (std::size_t i = 0; i < Plane.size(); i++)
		{
			auto& Verts = Plane.at(i);
			auto& TC = PlaneTC.at(i);
			sVertexLayout VBE;
			VBE.position = FVector(Verts.X, Verts.Y, 0.0f);
			VBE.texCoord = TC;
			VBE.Color = FColor::Green();
			Vertices.push_back(VBE);
		}
		std::vector<std::uint32_t> Indices = MeshPrimitives::GeneratePlaneIndices(1);
		ObjectDrawParameters.IndexCountPerInstance = (uint32_t)Indices.size();

		{
			BufferSubresource Subresource = BufferSubresource(Vertices.data(), Vertices.size() * sizeof(sVertexLayout));
			VertexBuffer = (IVertexBuffer::Create(Name, BufferLayout(Vertices.size() * sizeof(sVertexLayout), sizeof(sVertexLayout)), &Subresource));
		}
		{
			BufferSubresource Subresource = BufferSubresource(Indices.data(), Indices.size() * sizeof(std::uint32_t));
			IndexBuffer = (IIndexBuffer::Create(Name, BufferLayout(Indices.size() * sizeof(std::uint32_t), sizeof(std::uint32_t)), &Subresource));
		}
	}
}

sSprite::~sSprite()
{
	//AnimationCB = nullptr;
	VertexBuffer = nullptr;
	IndexBuffer = nullptr;
	//auto Mat = MaterialInstance->GetParent();
	//Mat->DestroyInstance(MaterialInstance->GetName());
	MaterialInstance = nullptr;
}

//void sSprite::Flip(bool value)
//{
//	if (AnimationConstantBuffer.Flip == value)
//		return;
//
//	AnimationConstantBuffer.Flip = value;
//	AnimationCB->Map(&AnimationConstantBuffer);
//}
//
//bool sSprite::IsFlipped() const
//{
//	return AnimationConstantBuffer.Flip;
//}

sSpriteSheet::sSpriteSheet(const std::string& InName, bool IsAnimationInLoop)
	: Name(InName)
	, bIsAnimationInLoop(IsAnimationInLoop)
	, WidthOffset(0.0f)
	, FPS(10)
{}

sSpriteSheet::sSpriteSheet(const std::string& InName, bool IsAnimationInLoop, const std::vector<sSpriteSheetKeyFrame*>& InSprites)
	: Name(InName)
	, bIsAnimationInLoop(IsAnimationInLoop)
	, KeyFrames(InSprites)
	, WidthOffset(0.0f)
	, FPS(10)
{}

sSpriteSheet::~sSpriteSheet()
{
	for (auto& KeyFrame : KeyFrames)
	{
		delete KeyFrame;
		KeyFrame = nullptr;
	}
	KeyFrames.clear();
}

void sSpriteSheet::AddSprite(const sSprite::SharedPtr& Sprite, std::size_t FrameCount, std::optional<std::size_t> index)
{
	AssetManager::Get().StoreSprite(Sprite);

	if (index.has_value())
	{
		if (*index >= KeyFrames.size())
		{
			KeyFrames.push_back(new sSpriteSheetKeyFrame(Sprite, FrameCount));
		}
		else
		{
			KeyFrames.insert(KeyFrames.begin() + *index, new sSpriteSheetKeyFrame(Sprite, FrameCount));
		}
	}
	else
	{
		KeyFrames.push_back(new sSpriteSheetKeyFrame(Sprite, FrameCount));
	}
}

IVertexBuffer* sSpriteSheet::GetVertexBuffer(std::size_t SpriteIndex) const
{
	return KeyFrames.at(SpriteIndex)->Sprite->GetVertexBuffer();
}

IIndexBuffer* sSpriteSheet::GetIndexBuffer(std::size_t SpriteIndex) const
{
	return KeyFrames.at(SpriteIndex)->Sprite->GetIndexBuffer();
}

sObjectDrawParameters sSpriteSheet::GetDrawParameters(std::size_t SpriteIndex) const
{
	return KeyFrames.at(SpriteIndex)->Sprite->GetDrawParameters();
}

sMaterial::sMaterialInstance* sSpriteSheet::GetMaterialInstance(std::size_t SpriteIndex) const
{
	return KeyFrames.at(SpriteIndex)->Sprite->GetMaterialInstance();
}

const FBounds2D& sSpriteSheet::GetSpriteBound(std::size_t SpriteIndex) const
{
	return KeyFrames.at(SpriteIndex)->Sprite->GetBound();
}

sSprite* sSpriteSheet::GetSprite(std::size_t index) const
{
	return KeyFrames.at(index)->Sprite.get();
}

sSpriteSheetKeyFrame* sSpriteSheet::GetKeyFrame(std::size_t index) const
{
	return KeyFrames.at(index);
}

std::size_t sSpriteSheet::GetKeyFrameIndexAtTime(float Time) const
{
	float ElapsedTime = 0.0f;

	for (std::size_t Index = 0; Index < KeyFrames.size(); Index++)
	{
		ElapsedTime += (float)KeyFrames[Index]->FrameCount / (float)FPS;

		if (Time <= ElapsedTime)
			return Index;
	}

	return KeyFrames.size() - 1;
}

sSprite* sSpriteSheet::GetSpriteAtTime(float Time) const
{
	return KeyFrames[GetKeyFrameIndexAtTime(Time)]->Sprite.get();
}

sSpriteSheetKeyFrame* sSpriteSheet::GetKeyFrameAtTime(float Time) const
{
	return KeyFrames[GetKeyFrameIndexAtTime(Time)];
}

std::size_t sSpriteSheet::GetKeyFrameIndexAtFrame(std::size_t Frame) const
{
	std::size_t Frames = 0;

	for (std::size_t Index = 0; Index < KeyFrames.size(); Index++)
	{
		Frames += KeyFrames[Index]->FrameCount;

		if (Frame < Frames)
			return Index;
	}

	return KeyFrames.size() - 1;
}

sSprite* sSpriteSheet::GetSpriteAtIndex(std::size_t index) const
{
	return KeyFrames.at(index)->Sprite.get();
}

sSprite* sSpriteSheet::GetSpriteAtFrame(std::size_t Frame) const
{
	std::size_t FrameCount = 0;

	for (std::size_t Index = 0; Index < KeyFrames.size(); Index++)
	{
		FrameCount += KeyFrames[Index]->FrameCount;

		if (Frame < FrameCount)
			return KeyFrames[Index]->Sprite.get();
	}

	return KeyFrames[KeyFrames.size() - 1]->Sprite.get();
}

sSpriteSheetKeyFrame* sSpriteSheet::GetKeyFrameAtFrame(std::size_t Frame) const
{
	std::size_t FrameCount = 0;

	for (std::size_t Index = 0; Index < KeyFrames.size(); Index++)
	{
		FrameCount += KeyFrames[Index]->FrameCount;

		if (Frame < FrameCount)
			return KeyFrames.at(Index);
	}

	return KeyFrames[KeyFrames.size() - 1];
}

std::size_t sSpriteSheet::GetSpriteFrameCount(std::size_t index) const
{
	return KeyFrames.at(index)->FrameCount;
}

std::size_t sSpriteSheet::GetNumFrames() const
{
	std::size_t Frames = 0;
	for (std::size_t i = 0; i < KeyFrames.size(); i++)
		Frames += KeyFrames[i]->FrameCount;

	return Frames;
}

float sSpriteSheet::GetTotalDurationInSeconds() const
{
	if (FPS != 0.0f)
		return (float)GetNumFrames() / (float)FPS;

	return 0.0f;
}
