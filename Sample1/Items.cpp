
#include "pch.h"
#include "Items.h"
#include "AssetManager.h"
#include <Gameplay/CircleCollision2DComponent.h>
#include "GPlayerCharacter.h"
#include "HelperFunctions.h"

GItem::GItem(std::string Fruit)
	: Super(Fruit)
	, bDestroyed(false)
{
	auto Sprite = AssetManager::Get().GetSpriteSheet(Fruit);
	auto Dim = Sprite->GetSpriteBound(0).GetDimension();

	auto SpriteComponent = sSpriteSheetComponent::Create("DefaultSpriteSheetComponent");
	SetRootComponent(SpriteComponent);
	SpriteSheetComponent = SpriteComponent.get();
	SpriteComponent->SetSpriteSheet(Sprite);
	SpriteComponent->Play();

	SpriteComponent->BindFunction_AnimationStarted(std::bind(&GItem::AnimationStarted, this));
	SpriteComponent->BindFunction_AnimationEnded(std::bind(&GItem::AnimationEnded, this));
	SpriteComponent->BindFunction_FrameUpdated(std::bind(&GItem::AnimFrameUpdated, this, std::placeholders::_1));

	Bound = FBounds2D(FDimension2D(Dim.Width/2, Dim.Height/2), FVector2::Zero());
}

GItem::~GItem()
{
	SpriteSheetComponent = nullptr;
}

void GItem::OnBeginPlay()
{
}

void GItem::OnFixedUpdate(const double DeltaTime)
{
	if (bDestroyed)
		return;

	auto Result = Physics::QueryAABB(Bound);
	for (const auto& Res : Result)
	{
		if (!Res)
			continue;

		if (Res->HasTag("PlayerCharacter"))
		{
			Res->GetOwner<GPlayerCharacter>()->ReceiveItem(this);

			bDestroyed = true;
			auto Sprite = AssetManager::Get().GetSpriteSheet("Collected");
			SpriteSheetComponent->SetSpriteSheet(Sprite);
			SpriteSheetComponent->Play();

			break;
		}
	}
}

void GItem::AnimationStarted()
{

}

void GItem::AnimationEnded()
{
	if (bDestroyed)
		OnDestroyed();
}

void GItem::AnimFrameUpdated(std::size_t Frame)
{

}

void GItem::OnDestroyed()
{
	SetEnabled(false);
	Hide(true);
	RemoveFromLevel();
}

void GItem::OnTransformUpdated()
{
	auto Loc = GetLocation();
	Bound.SetPosition(FVector2(Loc.X, Loc.Y));
}
