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

	virtual void Serialize(sArchive& archive) override;

private:
	virtual void OnUpdateTransform() override final;

	virtual void OnAttachToComponent() override final;
	virtual void OnDetachFromComponent() override final;

private:
	std::vector<sSpriteEffect*> Sprites;
	std::vector<sSpriteSheetEffect*> SpriteSheets;
	bool bFlip;
};
