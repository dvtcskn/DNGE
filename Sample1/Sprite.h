#pragma once

#include <Gameplay/PhysicalComponent.h>
#include <vector>
#include <memory>
#include "Core/Math/CoreMath.h"
#include "AbstractGI/RendererResource.h"

class sSprite
{
	sBaseClassBody(sClassConstructor, sSprite)
public:
	sSprite(const std::string& Name, const std::wstring& TextureAtlasPath, const FBounds2D& SpriteBound, sMaterial* DefaultMaterialName);
	sSprite(const std::string& Name, const std::wstring& TextureAtlasPath, const FBounds2D& SpriteBound, const std::string& DefaultMaterialName = "DefaultActorAtlastMat");
	sSprite(const std::string& Name, ITexture2D* TextureAtlas, const FBounds2D& SpriteBound, sMaterial* DefaultMaterialName);
	sSprite(const std::string& Name, ITexture2D* TextureAtlas, const FBounds2D& SpriteBound, const std::string& DefaultMaterialName = "DefaultActorAtlastMat");
	sSprite(const std::string& Name, const FBounds2D& SpriteBound, sMaterial::sMaterialInstance* AtlasMaterialInstance);

public:
	virtual ~sSprite();

	std::string GetName() const { return Name; };

	IVertexBuffer* GetVertexBuffer() const { return VertexBuffer.get(); };
	IIndexBuffer* GetIndexBuffer() const { return IndexBuffer.get(); };

	sObjectDrawParameters GetDrawParameters() const { return ObjectDrawParameters; };

	sMaterial::sMaterialInstance* GetMaterialInstance() const { return MaterialInstance; }

	const FBounds2D& GetBound() const { return SpriteBound; }

	//bool IsFlipped() const;
	//void Flip(bool value);

private:
	std::string Name;
	FBounds2D SpriteBound;

	sObjectDrawParameters ObjectDrawParameters;
	IVertexBuffer::SharedPtr VertexBuffer;
	IIndexBuffer::SharedPtr IndexBuffer;

	sMaterial::sMaterialInstance* MaterialInstance;

	//__declspec(align(256)) struct sFlipConstantBuffer
	//{
	//	//std::uint32_t Index = 0;
	//	std::uint32_t Flip = 0;
	//	//float FlipMaxX = 0;
	//	//float FlipMinX = 0;
	//};
	//sFlipConstantBuffer AnimationConstantBuffer;
	//IConstantBuffer::SharedPtr AnimationCB;
};

struct sSpriteSheetKeyFrame
{
public:
	sSprite::SharedPtr Sprite;
	std::size_t FrameCount;

	sSpriteSheetKeyFrame(sSprite::SharedPtr InSprite = nullptr, std::size_t InFrameCount = 1)
		: Sprite(InSprite)
		, FrameCount(InFrameCount)
	{}

	~sSpriteSheetKeyFrame()
	{
		Sprite = nullptr;
	}
};

class sSpriteSheet
{
	sBaseClassBody(sClassConstructor, sSpriteSheet)
public:
	sSpriteSheet(const std::string& Name, bool IsAnimationInLoop = true);
	sSpriteSheet(const std::string& Name, bool IsAnimationInLoop, const std::vector<sSpriteSheetKeyFrame*>& Sprites);
	virtual ~sSpriteSheet();

	std::string Name;
	std::vector<sSpriteSheetKeyFrame*> KeyFrames;
	std::size_t FPS;
	bool bIsAnimationInLoop;
	float WidthOffset;

	void AddSprite(const sSprite::SharedPtr& Sprite, std::size_t FrameCount, std::optional<std::size_t> index = std::nullopt);

	std::string GetName() const { return Name; };

	IVertexBuffer* GetVertexBuffer(std::size_t SpriteIndex) const;
	IIndexBuffer* GetIndexBuffer(std::size_t SpriteIndex) const;

	sObjectDrawParameters GetDrawParameters(std::size_t SpriteIndex) const;

	sMaterial::sMaterialInstance* GetMaterialInstance(std::size_t SpriteIndex) const;

	const FBounds2D& GetSpriteBound(std::size_t SpriteIndex) const;

	sSprite* GetSprite(std::size_t index) const;
	std::size_t GetKeyFrameSize() const { return KeyFrames.size(); }
	sSpriteSheetKeyFrame* GetKeyFrame(std::size_t index) const;
	std::size_t GetKeyFrameIndexAtTime(float Time) const;
	std::size_t GetKeyFrameIndexAtFrame(std::size_t Frame) const;
	sSprite* GetSpriteAtTime(float Time) const;
	sSpriteSheetKeyFrame* GetKeyFrameAtTime(float Time) const;
	sSprite* GetSpriteAtIndex(std::size_t index) const;
	sSprite* GetSpriteAtFrame(std::size_t Frame) const;
	sSpriteSheetKeyFrame* GetKeyFrameAtFrame(std::size_t Frame) const;

	std::size_t GetSpriteFrameCount(std::size_t index) const;

	std::size_t GetNumFrames() const;
	float GetTotalDurationInSeconds() const;
};
