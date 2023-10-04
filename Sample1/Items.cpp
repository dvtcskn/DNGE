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
