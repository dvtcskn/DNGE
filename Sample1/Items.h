/* ---------------------------------------------------------------------------------------
* MIT License
*
* Copyright (c) 2023 Davut Co�kun.
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

#include <Gameplay/Actor.h>
#include <Gameplay/BoxCollision2DComponent.h>
#include <Gameplay/Controller.h>
#include "SpriteAnimationManager.h"
#include "SpriteComponent.h"
#include "SpriteSheetComponent.h"

class GItem : public sActor
{
	sClassBody(sClassConstructor, GItem, sActor)
public:
	GItem(std::string Name, std::string Fruit);
	virtual ~GItem();

	virtual void OnBeginPlay() override;
	virtual void OnFixedUpdate(const double DeltaTime) override;

	std::string GetFruitType() const { return Type; }

	virtual std::string GetClassNetworkAddress() const override { return "Level::" + GetName(); }

	virtual void Replicate(bool bReplicate) override;

	void CheckIfExistOnServer();

private:
	void AnimationStarted();
	void AnimationEnded();
	void AnimFrameUpdated(std::size_t Frame);

	void OnDestroyed();
	void OnCollected();
	void OnCollected_Client();

	virtual void OnTransformUpdated() override;

	void CheckIfExistOnServer_Client(bool val);

private:
	sSpriteSheetComponent* SpriteSheetComponent;
	FBounds2D Bound;
	bool bDestroyed;
	std::string Type;
};
