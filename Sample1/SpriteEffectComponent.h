#pragma once

#include "SpriteComponent.h"
#include "SpriteSheetComponent.h"

class sSpriteEffectComponent : public sPrimitiveComponent
{
	sClassBody(sClassConstructor, sSpriteEffectComponent, sPrimitiveComponent)
private:
	struct sSpriteEffect
	{
		sSpriteComponent::SharedPtr Sprite = nullptr;
		std::uint32_t FrameTime = 0;

		sSpriteEffect() = default;
		~sSpriteEffect()
		{
			Sprite = nullptr;
		}
	};

	struct sSpriteSheetEffect
	{
		sSpriteSheetComponent::SharedPtr Sprite = nullptr;
		std::uint32_t FrameTime = 0;

		sSpriteSheetEffect() = default;
		~sSpriteSheetEffect()
		{
			Sprite = nullptr;
		}
	};

public:
	sSpriteEffectComponent(const std::string Name);
	virtual ~sSpriteEffectComponent();

	virtual void OnBeginPlay() override;
	virtual void OnFixedUpdate(const double DeltaTime) override;

	bool IsFlipped() const;
	void Flip(bool value);

	void SetSprite(sSprite* Sprite, std::uint32_t FrameTime, FVector2 Offset = FVector2::Zero());
	void SetSpriteSheet(sSpriteSheet* Sprite, std::optional<std::uint32_t> FrameTime = std::nullopt, FVector2 Offset = FVector2::Zero());

	virtual void Serialize(Archive& archive) override;

private:
	virtual void OnUpdateTransform() override final;

	virtual void OnAttachToComponent() override final;
	virtual void OnDetachFromComponent() override final;

private:
	std::vector<sSpriteEffect*> Sprites;
	std::vector<sSpriteSheetEffect*> SpriteSheets;
	bool bFlip;
};
