
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

void sSpriteEffectComponent::Serialize(Archive& archive)
{
}
