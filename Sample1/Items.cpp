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

GItem::GItem(std::string Name, std::string Fruit)
	: Super(Name)
	, bDestroyed(false)
	, Type(Fruit)
{
	Replicate(true);
	auto Sprite = AssetManager::Get().GetSpriteSheet(Type);
	auto Dim = Sprite->GetSpriteBound(0).GetDimension();

	auto SpriteComponent = sSpriteSheetComponent::Create("DefaultSpriteSheetComponent");
	SetRootComponent(SpriteComponent);
	SpriteComponent->Replicate(true);
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
	if (Network::IsClient())
		return;

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

			OnCollected();

			break;
		}
	}
}

void GItem::Replicate(bool bReplicate)
{
	if (IsReplicated() == bReplicate)
		return;

	Super::Replicate(bReplicate);

	if (IsReplicated())
	{
		RegisterRPCfn(GetClassNetworkAddress(), GetName(), "OnCollected_Client", eRPCType::Client, true, false, std::bind(&GItem::OnCollected_Client, this));
		RegisterRPCfn(GetClassNetworkAddress(), GetName(), "CheckIfExistOnServer", eRPCType::Server, true, false, std::bind(&GItem::CheckIfExistOnServer, this));
		RegisterRPCfn(GetClassNetworkAddress(), GetName(), "CheckIfExistOnServer_Client", eRPCType::Client, true, false, std::bind(&GItem::CheckIfExistOnServer_Client, this, std::placeholders::_1), bool);
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
	bDestroyed = true;
	SetEnabled(false);
	Hide(true);
	//if (Network::IsHost() || Network::IsClient())
	//	return;
	//RemoveFromLevel();
}

void GItem::OnCollected()
{
	if (Network::IsHost())
	{
		Network::CallRPC(GetClassNetworkAddress(), GetName(), "OnCollected_Client", sArchive());
	}

	if (bDestroyed)
		return;

	bDestroyed = true;
	auto Sprite = AssetManager::Get().GetSpriteSheet("Collected");
	SpriteSheetComponent->SetSpriteSheet(Sprite);
	SpriteSheetComponent->Play();
}

void GItem::OnCollected_Client()
{
	if (Network::IsHost())
		return;

	if (bDestroyed)
		return;

	bDestroyed = true;
	auto Sprite = AssetManager::Get().GetSpriteSheet("Collected");
	SpriteSheetComponent->SetSpriteSheet(Sprite);
	SpriteSheetComponent->Play();
}

void GItem::OnTransformUpdated()
{
	auto Loc = GetLocation();
	Bound.SetPosition(FVector2(Loc.X, Loc.Y));
}

void GItem::CheckIfExistOnServer()
{
	if (Network::IsClient())
	{
		Network::CallRPC(GetClassNetworkAddress(), GetName(), "CheckIfExistOnServer", sArchive());
	}
	else if (Network::IsHost())
	{
		Network::CallRPC(GetClassNetworkAddress(), GetName(), "CheckIfExistOnServer_Client", sArchive(bDestroyed));
	}
}

void GItem::CheckIfExistOnServer_Client(bool val)
{
	if (val)
	{
		OnDestroyed();
	}
}
