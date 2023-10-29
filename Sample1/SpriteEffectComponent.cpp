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
#include "SpriteEffectComponent.h"

sSpriteEffectComponent::sSpriteEffectComponent(const std::string Name)
	: Super(Name)
	, bFlip(false)
{
}

sSpriteEffectComponent::~sSpriteEffectComponent()
{
	for (auto& Sprite : Sprites)
	{
		delete Sprite;
		Sprite = nullptr;
	}
	Sprites.clear();

	for (auto& Sprite : SpriteSheets)
	{
		delete Sprite;
		Sprite = nullptr;
	}
	SpriteSheets.clear();
}

void sSpriteEffectComponent::OnBeginPlay()
{
}

void sSpriteEffectComponent::OnFixedUpdate(const double DeltaTime)
{
	{
		std::vector<sSpriteEffect*>::iterator it = Sprites.begin();
		while (it != Sprites.end())
		{
			(*it)->Sprite->Tick(DeltaTime);
			if ((*it)->FrameTime > 0)
				(*it)->FrameTime--;

			if ((*it)->FrameTime == 0)
			{
				(*it)->Sprite->DetachFromComponent();
				(*it)->Sprite = nullptr;
				sSpriteEffect* Effect = (*it);
				it = Sprites.erase(it);
				delete Effect;
				Effect = nullptr;
				break;
			}
			else {
				it++;
			}
		}
	}

	{
		std::vector<sSpriteSheetEffect*>::iterator it = SpriteSheets.begin();
		while (it != SpriteSheets.end())
		{
			(*it)->Sprite->Tick(DeltaTime);
			if ((*it)->FrameTime > 0)
				(*it)->FrameTime--;

			if ((*it)->FrameTime == 0)
			{
				(*it)->Sprite->DetachFromComponent();
				(*it)->Sprite = nullptr;
				sSpriteSheetEffect* Effect = (*it);
				it = SpriteSheets.erase(it);
				delete Effect;
				Effect = nullptr;
				break;
			}
			else {
				it++;
			}
		}
	}
}

bool sSpriteEffectComponent::IsFlipped() const
{
	return bFlip;
}

void sSpriteEffectComponent::Flip(bool value)
{
	if (bFlip == value)
		return;
	bFlip = value;

	if (bFlip)
	{
		SetRelativeRotation(FQuaternion(FAngles(180.0f, 0.0f, 0.0f)));
	}
	else
	{
		SetRelativeRotation(FQuaternion(FAngles(0.0f, 0.0f, 0.0f)));
	}
}

void sSpriteEffectComponent::SetSprite(sSprite* Sprite, std::uint32_t FrameTime, FVector2 Offset)
{
	sSpriteEffect* Effect = new sSpriteEffect();
	Effect->FrameTime = FrameTime;

	Effect->Sprite = sSpriteComponent::Create(Sprite);
	Effect->Sprite->Offset.X = Offset.X;
	Effect->Sprite->Offset.Y = Offset.Y;
	Effect->Sprite->Offset.Z = 0.0f;
	Effect->Sprite->AttachToComponent(this);

	Sprites.push_back(Effect);
}

void sSpriteEffectComponent::SetSpriteSheet(sSpriteSheet* Sprite, std::optional<std::uint32_t> FrameTime, FVector2 Offset)
{
	sSpriteSheetEffect* Effect = new sSpriteSheetEffect();
	Effect->FrameTime = FrameTime.has_value() ? *FrameTime : (std::uint32_t)Sprite->GetNumFrames() * (std::uint32_t)Sprite->FPS;

	Effect->Sprite = sSpriteSheetComponent::Create(Sprite);
	Effect->Sprite->Offset.X = Offset.X;
	Effect->Sprite->Offset.Y = Offset.Y;
	Effect->Sprite->Offset.Z = 0.0f;
	Effect->Sprite->AttachToComponent(this);

	SpriteSheets.push_back(Effect);
}

void sSpriteEffectComponent::OnUpdateTransform()
{
}

void sSpriteEffectComponent::OnAttachToComponent()
{
}

void sSpriteEffectComponent::OnDetachFromComponent()
{
}

void sSpriteEffectComponent::Serialize(sArchive& archive)
{
}
