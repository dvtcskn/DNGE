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

#include <Gameplay/PlayerController.h>
#include "GPlayerCharacter.h"
#include "GCanvas.h"

class sGameInstance;
class GPlayerController : public sPlayerController
{
	sClassBody(sClassConstructor, GPlayerController, sPlayerController)
public:
	GPlayerController(sPlayer* InOwner);

	virtual ~GPlayerController();

	virtual void OnBeginPlay() override final;
	virtual void OnTick(const double DeltaTime) override final;

	virtual void Save() override final;
	virtual void Load() override final;

	virtual void InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar) override final;

	void OnCharacterDead();

	virtual void Replicate(bool bReplicate);

private:
	//virtual void OnNewPlayerAdded() {}
	//virtual void OnPlayerRemoved() {}

	virtual void OnPossess(sActor* Actor);
	virtual void OnUnPossess();

	void GameOverFadeOut();
	void StartScreenFadeOut();

	virtual void OnPlayerNetworkRoleChanged();

	virtual void OnSplitScreenEnabled() override;
	virtual void OnSplitScreenDisabled() override;

	virtual void OnLevelReset() override;
	void OnLevelReset_Client();

private:
	cbgui::GCanvas::SharedPtr Canvas;
};

class GProxyController : public sProxyController
{
	sClassBody(sClassConstructor, GProxyController, sProxyController)
public:
	GProxyController(sPlayerProxyBase* Proxy = nullptr);
	virtual ~GProxyController();

	virtual void BeginPlay();

	void OnLevelReset_Client();
};
