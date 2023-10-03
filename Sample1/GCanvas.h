/* ---------------------------------------------------------------------------------------
* MIT License
*
* Copyright (c) 2022 Davut Coþkun.
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

#include "CanvasBase.h"
#include "GPlayerCharacter.h"

namespace cbgui
{
	class GCanvas : public cbgui::sCanvasBase
	{
		cbClassBody(cbClassConstructor, GCanvas, cbgui::sCanvasBase)
	public:
		GCanvas(IMetaWorld* pMetaWorld, GPlayerCharacter* InCharacter);
		virtual ~GCanvas();
		virtual void SetMaterial(WidgetHierarchy* pWP) override final;
		virtual void Tick(const double DeltaTime);

		virtual void InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar) override final;

		virtual void ResizeWindow(std::size_t Width, std::size_t Height) override final;

		bool IsMenuActive() const;
		void StartScreenFadeOut();

		void GameOver();
		void GameOverFadeOut();
		bool IsGameOverScreenActive() const;

		std::function<void()> fOnMenuScreenFadeOut;
		std::function<void()> fOnGameOverScreenFadeOut;

	private:
		virtual void Add(const std::shared_ptr<cbWidget>& Object) override;
		virtual void RemoveFromCanvas(cbWidget* Object) override;

	private:
		class cbStartScreen;
		std::shared_ptr<cbStartScreen> StartScreen;

		class cbGameOverScreen;
		std::shared_ptr<cbGameOverScreen> GameOverScreen;

		class cbPlayerInterface;
		std::shared_ptr<cbPlayerInterface> PlayerInterface;

		cbgui::cbImage::SharedPtr BGImage;

		cbWidget* Focus;

		GPlayerCharacter* Character;

		UIMaterialStyle::SharedPtr DefaultGUIGradientColorMatStyle;
		UIMaterialStyle::SharedPtr AppleUIMatStyle;
		UIMaterialStyle::SharedPtr CherrieUIMatStyle;

		IConstantBuffer::SharedPtr GradientConstantBuffer;
		unsigned int GradientIndex;
	};
}
