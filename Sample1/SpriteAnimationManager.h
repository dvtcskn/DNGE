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

#include <DirectXMath.h>
#include "AnimationStates.h"
#include <map>
#include <vector>
#include <memory>
#include <optional>
#include <Engine/ClassBody.h>
#include <Gameplay/PrimitiveComponent.h>
#include "Sprite.h"
#include "SpriteComponent.h"
#include "SpriteSheetComponent.h"

struct sSpriteAnimationGraph
{
	std::map<EAnimationState, sSpriteSheet*> States;
};

struct sAnimFunc
{
	std::size_t Frame;
	EAnimationState State;
	std::function<void()> fOnFrame;

	sAnimFunc() = default;
	sAnimFunc(std::size_t InFrame, EAnimationState InState, std::function<void()> InfOnFrame)
		: Frame(InFrame)
		, State(InState)
		, fOnFrame(InfOnFrame)
	{}
	~sAnimFunc()
	{
		fOnFrame = nullptr;
	}
};

class sSpriteAnimationManager : public sPrimitiveComponent
{
	sClassBody(sClassConstructor, sSpriteAnimationManager, sPrimitiveComponent)
public:
	sSpriteAnimationManager(std::string Name = "SpriteAnimationManager");
	virtual ~sSpriteAnimationManager();

	virtual void OnFixedUpdate(const double DeltaTime) override final;

	EAnimationState GetAnimationState() const { return AnimationState; }
	void SetAnimationState(const EAnimationState State);

	void SetGraph(const sSpriteAnimationGraph& Graph);

	void AnimationStarted();
	void AnimationEnded();
	void AnimationFrameUpdated(std::size_t Frame);

	bool IsFlipped() const;

	void BindFunctionOnAnimationFrame(std::string Name, std::size_t Frame, EAnimationState State, std::function<void()> fOnFrame);

	std::map<std::string, sAnimFunc> States;

	void BindFunction_AnimationStarted(std::function<void()> pf);
	void BindFunction_AnimationEnded(std::function<void()> pf);
	void BindFunction_FrameUpdated(std::function<void(std::size_t)> pf);

private:
	EAnimationState AnimationState;
	sSpriteAnimationGraph Graph;

	sSpriteSheetComponent* SpriteSheetComponent;

	std::function<void()> fAnimationStarted;
	std::function<void()> fAnimationEnd;
	std::function<void(std::size_t)> fFrameUpdated;
};
