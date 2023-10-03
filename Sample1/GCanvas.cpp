/* ---------------------------------------------------------------------------------------
* MIT License
*
* Copyright (c) 2022 Davut Coşkun.
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
#include "GCanvas.h"
#include "cbScaleBox.h"
#include "cbString.h"
#include <cbgui.h>
#include <AbstractGI/MaterialManager.h>

using namespace cbgui;

class cbLevelData
{
private:
	cbLevelData()
	{
		LevelCompletion.push_back(14);
		LevelCompletion.push_back(2);
		LevelCompletion.push_back(23);
		LevelCompletion.push_back(50);
		LevelCompletion.push_back(15);
		LevelCompletion.push_back(100);
		LevelCompletion.push_back(75);
		LevelCompletion.push_back(90);
	}
	cbLevelData(const cbLevelData&) = delete;
	cbLevelData(cbLevelData&&) = delete;
	cbLevelData& operator=(const cbLevelData&) = delete;
	cbLevelData& operator=(cbLevelData&&) = delete;

public:
	~cbLevelData() = default;

	std::vector<int> LevelCompletion;

	static cbLevelData& Get()
	{
		static cbLevelData instance;
		return instance;
	}
};

class GCanvas::cbStartScreen : public cbgui::cbVerticalBox
{
	cbClassBody(cbClassConstructor, cbStartScreen, cbgui::cbVerticalBox)
private:
	cbString::SharedPtr Text;
	cbString::SharedPtr Header;
	cbgui::cbBorder::SharedPtr Border;
	cbgui::cbColor Color;
	int Modifier;
	bool bWait;
	float Time;
	bool bStartScreenFadeOut;

public:
	cbStartScreen(cbDimension Dimension)
		: cbVerticalBox()
		, Color(cbgui::cbColor::White())
		, Modifier(1)
		, bWait(false)
		, bStartScreenFadeOut(false)
		, Time(0.0f)
	{
		{
			cbgui::cbSizeBox::SharedPtr Empty = cbSizeBox::Create();
			Empty->SetHorizontalAlignment(eHorizontalAlignment::Align_Fill);
			Empty->SetVerticalAlignment(eVerticalAlignment::Align_Top);
			Empty->SetHeight((Dimension.GetHeight() / 3));
			Insert(Empty, eSlotAlignment::BoundToContent);
		}
		{
			Border = cbBorder::Create();
			Border->SetHorizontalAlignment(eHorizontalAlignment::Align_Center);
			Border->SetVerticalAlignment(eVerticalAlignment::Align_Top);
			Border->SetThickness(cbMargin(5));
			Insert(Border, eSlotAlignment::BoundToContent);
			cbTextDesc Desc;
			Desc.CharSize = 144 /*+ 24*/;
			Desc.FontType = cbgui::eFontType::Bold;
			Header = cbString::Create("SAMPLE 1", Desc);
			Header->Wrap();
			Header->SetDefaultSpaceWidth(3);
			Header->SetHorizontalAlignment(eHorizontalAlignment::Align_Center);
			Header->SetVerticalAlignment(eVerticalAlignment::Align_Center);
			Header->SetPadding(cbMargin(0, 0, 5, 0));
			//Header->SetLocationY(Dimension.GetHeight() / 2 - 56);
			//Header->SetLocationX(Dimension.GetWidth()/ 2);
			Border->SetContent(Header);
		}
		{
			cbgui::cbSizeBox::SharedPtr Empty = cbSizeBox::Create();
			Empty->SetHorizontalAlignment(eHorizontalAlignment::Align_Fill);
			Empty->SetVerticalAlignment(eVerticalAlignment::Align_Top);
			Empty->SetHeight((Dimension.GetHeight() / 6));
			Insert(Empty, eSlotAlignment::BoundToContent);
		}
		Text = cbString::Create("PRESS [Enter] TO PLAY");
		Text->SetDefaultSpaceWidth(3);
		Text->Wrap();
		Text->SetHorizontalAlignment(eHorizontalAlignment::Align_Center);
		Text->SetVerticalAlignment(eVerticalAlignment::Align_Top);
		//Text->SetLocationY(Dimension.GetHeight() - (Dimension.GetHeight() / 3));
		//Text->SetLocationX(Dimension.GetWidth()) / 2);
		Insert(Text, eSlotAlignment::BoundToContent);
	}

	virtual ~cbStartScreen()
	{
		Border = nullptr;
		Text = nullptr;
		Header = nullptr;
	}

	void Appear()
	{
		bStartScreenFadeOut = false;
		bWait = false;
		Color.A = 1.0f;
		Modifier = 1;
		Time = 0.0f;
		SetVisibilityState(eVisibility::Visible);
		Text->SetVertexColorStyle(Color);
		Header->SetVertexColorStyle(Color);
		Text->SetVisibilityState(eVisibility::Visible);
		Text->Enable();
	}

	bool IsStartScreenFadeOut() const { return bStartScreenFadeOut; }

	virtual void OnTick(float DeltaTime) override
	{
		if (!IsEnabled())
			return;

		if (!bWait)
		{
			Color.A -= DeltaTime * Modifier;
			if (bStartScreenFadeOut)
			{
				Border->SetVertexColorStyle(Color);
				Header->SetVertexColorStyle(Color);
			}
			else
				Text->SetVertexColorStyle(Color);
		}

		if (bWait)
			Time += DeltaTime;

		if (!bWait && Color.A <= 0.0f)
		{
			if (bStartScreenFadeOut)
			{
				SetVisibilityState(eVisibility::Hidden);
				Disable();
				GetCanvas<GCanvas>()->StartScreenFadeOut();
				return;
			}

			Color.A = 0.0f;
			Modifier = -2;

			bWait = false;
			Time = 0.0f;
		}
		else if (!bWait && Color.A >= 1.0f)
		{
			Color.A = 1.0f;
			Modifier = 1;

			bWait = true;
			Time = 0.0f;
		}

		if (Time >= 0.5f)
		{
			bWait = false;
			Time = 0.0f;
		}
	}

	void FadeOut()
	{
		bStartScreenFadeOut = true;

		{
			Color.A = 1.0f;

			Text->SetVisibilityState(eVisibility::Hidden);
			Text->Disable();
		}
	}
};

class GCanvas::cbGameOverScreen : public cbgui::cbVerticalBox
{
	cbClassBody(cbClassConstructor, cbGameOverScreen, cbgui::cbVerticalBox)
private:
	cbString::SharedPtr Text;
	cbString::SharedPtr Header;
	cbgui::cbColor Color;
	int Modifier;
	bool bWait;
	float Time;
	bool bFadeOut;

public:
	cbGameOverScreen(cbDimension Dimension)
		: cbVerticalBox()
		, Color(cbgui::cbColor::White())
		, Modifier(1)
		, bWait(false)
		, bFadeOut(false)
		, Time(0.0f)
	{
		{
			cbgui::cbSizeBox::SharedPtr Empty = cbSizeBox::Create();
			Empty->SetHorizontalAlignment(eHorizontalAlignment::Align_Fill);
			Empty->SetVerticalAlignment(eVerticalAlignment::Align_Top);
			Empty->SetHeight((Dimension.GetHeight() / 3));
			Insert(Empty, eSlotAlignment::BoundToContent);
		}
		{
			cbTextDesc Desc;
			Desc.CharSize = 144 /*+ 24*/;
			Desc.FontType = cbgui::eFontType::Bold;
			Header = cbString::Create("GAME OVER!", Desc);
			Header->Wrap();
			Header->SetDefaultSpaceWidth(3);
			Header->SetHorizontalAlignment(eHorizontalAlignment::Align_Center);
			Header->SetVerticalAlignment(eVerticalAlignment::Align_Center);
			Header->SetPadding(cbMargin(0, 0, 5, 0));
			Insert(Header, eSlotAlignment::BoundToContent);
		}
		{
			cbgui::cbSizeBox::SharedPtr Empty = cbSizeBox::Create();
			Empty->SetHorizontalAlignment(eHorizontalAlignment::Align_Fill);
			Empty->SetVerticalAlignment(eVerticalAlignment::Align_Top);
			Empty->SetHeight((Dimension.GetHeight() / 6));
			Insert(Empty, eSlotAlignment::BoundToContent);
		}
		Text = cbString::Create("TRY AGAIN!");
		Text->SetDefaultSpaceWidth(3);
		Text->Wrap();
		Text->SetHorizontalAlignment(eHorizontalAlignment::Align_Center);
		Text->SetVerticalAlignment(eVerticalAlignment::Align_Top);
		//Text->SetLocationY(Dimension.GetHeight() - (Dimension.GetHeight() / 3));
		//Text->SetLocationX(Dimension.GetWidth()) / 2);
		Insert(Text, eSlotAlignment::BoundToContent);
	}

	virtual ~cbGameOverScreen()
	{
		Text = nullptr;
		Header = nullptr;
	}

	void Appear()
	{
		bFadeOut = false;
		bWait = false;
		Color.A = 1.0f;
		Modifier = 1;
		Time = 0.0f;
		SetVisibilityState(eVisibility::Visible);
		Text->SetVertexColorStyle(Color);
		Header->SetVertexColorStyle(Color);
		Text->SetVisibilityState(eVisibility::Visible);
		Text->Enable();
	}

	bool IsFadeOut() const { return bFadeOut; }

	virtual void OnTick(float DeltaTime) override
	{
		if (!IsEnabled())
			return;

		if (!bWait)
		{
			Color.A -= DeltaTime * Modifier;
			if (bFadeOut)
			{
				Header->SetVertexColorStyle(Color);
			}
			else
				Text->SetVertexColorStyle(Color);
		}

		if (bWait)
			Time += DeltaTime;

		if (!bWait && Color.A <= 0.0f)
		{
			if (bFadeOut)
			{
				SetVisibilityState(eVisibility::Hidden);
				Disable();
				GetCanvas<GCanvas>()->GameOverFadeOut();
				return;
			}

			Color.A = 0.0f;
			Modifier = -2;

			bWait = false;
			Time = 0.0f;
		}
		else if (!bWait && Color.A >= 1.0f)
		{
			Color.A = 1.0f;
			Modifier = 1;

			bWait = true;
			Time = 0.0f;
		}

		if (Time >= 0.5f)
		{
			bWait = false;
			Time = 0.0f;
		}
	}

	void FadeOut()
	{
		bFadeOut = true;

		{
			Color.A = 1.0f;

			Text->SetVisibilityState(eVisibility::Hidden);
			Text->Disable();
		}
	}
};

class GCanvas::cbPlayerInterface : public cbgui::cbHorizontalBox
{
	cbClassBody(cbClassConstructor, cbPlayerInterface, cbgui::cbHorizontalBox)
private:
	cbString::SharedPtr HealthText;
	cbString::SharedPtr AppleText;
	cbString::SharedPtr CherrieText;
	GPlayerCharacter* Character;

	float Health;
	int Apples;
	int Cherries;

public:
	cbPlayerInterface(cbDimension Dimension, GPlayerCharacter* InCharacter)
		: Super()
		, Character(InCharacter)
		, Health(0.0f)
		, Apples(0)
		, Cherries(0)
	{
		{
			cbString::SharedPtr HealthIcon = cbString::Create(U"❤️");
			HealthIcon->SetDefaultSpaceWidth(3);
			HealthIcon->Wrap();
			HealthIcon->SetVerticalAlignment(eVerticalAlignment::Align_Top);
			HealthIcon->SetHorizontalAlignment(eHorizontalAlignment::Align_Left);
			HealthIcon->SetPadding(cbMargin(6, 20, 0, 0));
			HealthIcon->SetVertexColorStyle(cbVertexColorStyle(cbColor(156,27,77)));
			Insert(HealthIcon, eSlotAlignment::BoundToContent);
		}

		{
			HealthText = cbString::Create("100");
			HealthText->SetDefaultSpaceWidth(3);
			HealthText->Wrap();
			HealthText->SetVerticalAlignment(eVerticalAlignment::Align_Top);
			HealthText->SetHorizontalAlignment(eHorizontalAlignment::Align_Left);
			HealthText->SetPadding(cbMargin(6, 20, 0, 0));
			Insert(HealthText, eSlotAlignment::BoundToContent);
		}

		auto AppleImage = std::make_shared<cbgui::cbImage>();
		AppleImage->SetName("AppleImage");

		AppleImage->SetVerticalAlignment(eVerticalAlignment::Align_Top);
		AppleImage->SetHorizontalAlignment(eHorizontalAlignment::Align_Left);
		AppleImage->SetPadding(cbMargin(20, 22, 0, 0));
		Insert(AppleImage, eSlotAlignment::BoundToContent);

		{
			AppleText = cbString::Create("0");
			AppleText->SetDefaultSpaceWidth(3);
			AppleText->Wrap();
			AppleText->SetVerticalAlignment(eVerticalAlignment::Align_Top);
			AppleText->SetHorizontalAlignment(eHorizontalAlignment::Align_Left);
			AppleText->SetPadding(cbMargin(6, 20, 0, 0));
			Insert(AppleText, eSlotAlignment::BoundToContent);
		}

		auto CherrieImage = std::make_shared<cbgui::cbImage>();
		CherrieImage->SetName("CherrieImage");

		CherrieImage->SetVerticalAlignment(eVerticalAlignment::Align_Top);
		CherrieImage->SetHorizontalAlignment(eHorizontalAlignment::Align_Left);
		CherrieImage->SetPadding(cbMargin(20, 20, 0, 0));
		Insert(CherrieImage, eSlotAlignment::BoundToContent);

		{
			CherrieText = cbString::Create("0");
			CherrieText->SetDefaultSpaceWidth(3);
			CherrieText->Wrap();
			CherrieText->SetVerticalAlignment(eVerticalAlignment::Align_Top);
			CherrieText->SetHorizontalAlignment(eHorizontalAlignment::Align_Left);
			CherrieText->SetPadding(cbMargin(6, 20, 0, 0));
			Insert(CherrieText, eSlotAlignment::BoundToContent);
		}

	}

	virtual ~cbPlayerInterface()
	{
		HealthText = nullptr;
		AppleText = nullptr;
		CherrieText = nullptr;
		Character = nullptr;
	}

	virtual void OnTick(float DeltaTime) override
	{
		float CurrentHealth = Character->GetHealth();
		if (Health != CurrentHealth)
		{
			Health = CurrentHealth < 0.0f ? 0.0f : CurrentHealth;
			std::string str = std::to_string((std::uint32_t)Health);
			HealthText->SetText(std::u32string(str.begin(), str.end()));
		}

		int CurrentAppleSize = Character->AppleCount;
		if (Apples != CurrentAppleSize)
		{
			Apples = CurrentAppleSize;
			std::string str = std::to_string((std::uint32_t)Apples);
			AppleText->SetText(std::u32string(str.begin(), str.end()));
		}

		int CurrentCherrieSize = Character->CherrieCount;
		if (Cherries != CurrentCherrieSize)
		{
			Cherries = CurrentCherrieSize;
			std::string str = std::to_string((std::uint32_t)Cherries);
			CherrieText->SetText(std::u32string(str.begin(), str.end()));
		}
	}
};

namespace cbgui
{
	__declspec(align(256)) struct CBGradientIdx
	{
		unsigned int GradientIdx;
	};
	static_assert((sizeof(CBGradientIdx) % 256) == 0, "Constant Buffer size must be 256-byte aligned");

	GCanvas::GCanvas(IMetaWorld* pMetaWorld, GPlayerCharacter* InCharacter)
		: Super(pMetaWorld)
		, Character(InCharacter)
		, GradientIndex(5)
		, Focus(nullptr)
		, fOnMenuScreenFadeOut(nullptr)
		, fOnGameOverScreenFadeOut(nullptr)
	{
		{
			{
				sBufferDesc BufferDesc;
				BufferDesc.Size = sizeof(CBGradientIdx);
				GradientConstantBuffer = IConstantBuffer::Create("GradientConstantBuffer", BufferDesc, 1);

				CBGradientIdx GradientIdx;
				GradientIdx.GradientIdx = GradientIndex;
				GradientConstantBuffer->Map(&GradientIdx);
			}
			{
				sMaterial::sMaterialInstance* GradientColorInstance = sMaterialManager::Get().CreateMaterialInstance("Default_GUI_GradientMat", "Default_GUI_Gradient_MatInstance");
				GradientColorInstance->BindConstantBuffer(GradientConstantBuffer);
				DefaultGUIGradientColorMatStyle = UIMaterialStyle::Create("GradientImage", UIMaterial::Create(GradientColorInstance));
				GradientColorInstance = nullptr;
			}
			{
				sMaterial::sMaterialInstance* AppleUIMatInstance = sMaterialManager::Get().CreateMaterialInstance("Default_GUI_TexturedMaterial", "AppleMatInstance");
				AppleUIMatStyle = UIMaterialStyle::Create("AppleImage", UIMaterial::Create(AppleUIMatInstance));
				AppleUIMatInstance = nullptr;
			}
			{
				sMaterial::sMaterialInstance* CherrieUIMatInstance = sMaterialManager::Get().CreateMaterialInstance("Default_GUI_TexturedMaterial", "CherrieMatInstance");
				CherrieUIMatStyle = UIMaterialStyle::Create("CherrieImage", UIMaterial::Create(CherrieUIMatInstance));
				CherrieUIMatInstance = nullptr;
			}
		}

		/*{
			BGImage = std::make_shared<cbgui::cbImage>();
			BGImage->SetName("Start Screen Image");
			BGImage->AddToCanvas(this);

			BGImage->SetVerticalAlignment(eVerticalAlignment::Align_Fill);
			BGImage->SetHorizontalAlignment(eHorizontalAlignment::Align_Fill);
			BGImage->SetAlignToCanvas(true);
		}

		{
			StartScreen = cbStartScreen::Create(GetScreenDimension());
			StartScreen->AddToCanvas(this);
			StartScreen->Wrap();

			StartScreen->SetVerticalAlignment(eVerticalAlignment::Align_Fill);
			StartScreen->SetHorizontalAlignment(eHorizontalAlignment::Align_Fill);
			StartScreen->SetAlignToCanvas(true);
			Focus = StartScreen.get();
		}*/ 

		{
			GameOverScreen = cbGameOverScreen::Create(GetScreenDimension());
			GameOverScreen->AddToCanvas(this);
			GameOverScreen->Wrap();

			GameOverScreen->SetVerticalAlignment(eVerticalAlignment::Align_Fill);
			GameOverScreen->SetHorizontalAlignment(eHorizontalAlignment::Align_Fill);
			GameOverScreen->SetAlignToCanvas(true);
			Focus = GameOverScreen.get();

			GameOverScreen->SetVisibilityState(cbgui::eVisibility::Hidden);
			GameOverScreen->Disable();
		}

		{
			PlayerInterface = cbPlayerInterface::Create(GetScreenDimension(), Character);
			PlayerInterface->AddToCanvas(this);
			PlayerInterface->Wrap();

			PlayerInterface->SetVerticalAlignment(eVerticalAlignment::Align_Fill);
			PlayerInterface->SetHorizontalAlignment(eHorizontalAlignment::Align_Fill);
			PlayerInterface->SetAlignToCanvas(true);
		}
	}

	GCanvas::~GCanvas()
	{
		fOnMenuScreenFadeOut = nullptr;
		fOnGameOverScreenFadeOut = nullptr;

		StartScreen = nullptr;
		Focus = nullptr;

		DefaultGUIGradientColorMatStyle = nullptr;
		GradientConstantBuffer = nullptr;
		AppleUIMatStyle = nullptr;
		CherrieUIMatStyle = nullptr;

		BGImage = nullptr;

		GameOverScreen = nullptr;
	}

	void GCanvas::ResizeWindow(std::size_t Width, std::size_t Height)
	{
		Super::ResizeWindow(Width, Height);
		if (StartScreen)
			StartScreen->UpdateAlignments(true);
		if (GameOverScreen)
			GameOverScreen->UpdateAlignments(true);
	}

	void GCanvas::Add(const std::shared_ptr<cbWidget>& Object)
	{
		Super::Add(Object);
	}

	void GCanvas::RemoveFromCanvas(cbWidget* Object)
	{
		Super::RemoveFromCanvas(Object);
	}

	void GCanvas::SetMaterial(WidgetHierarchy* pWP)
	{
		Super::SetMaterial(pWP);

		if (pWP->Widget->GetName() == "Start Screen Image")
		{
			pWP->MaterialStyle = DefaultGUIGradientColorMatStyle.get();
		}
		else if (pWP->Widget->GetName() == "AppleImage")
		{
			pWP->MaterialStyle = AppleUIMatStyle.get();
		}
		else if (pWP->Widget->GetName() == "CherrieImage")
		{
			pWP->MaterialStyle = CherrieUIMatStyle.get();
		}
	}

	void GCanvas::Tick(const double DeltaTime)
	{
		Super::Tick(DeltaTime);
	}

	void GCanvas::InputProcess(const GMouseInput& InMouseInput, const GKeyboardChar& KeyboardChar)
	{
		if (!KeyboardChar.bIsIdle)
		{
			if (KeyboardChar.KeyCode == 39 && !KeyboardChar.bIsChar && KeyboardChar.bIsPressed)
			{
				GradientIndex++;
				if (GradientIndex > 15)
					GradientIndex = 0;
				CBGradientIdx GradientIdx;
				GradientIdx.GradientIdx = GradientIndex;
				GradientConstantBuffer->Map(&GradientIdx);
			}
			if (KeyboardChar.KeyCode == 37 && !KeyboardChar.bIsChar && KeyboardChar.bIsPressed)
			{
				if (GradientIndex != 0)
					GradientIndex--;
				else
					GradientIndex = 15;
				CBGradientIdx GradientIdx;
				GradientIdx.GradientIdx = GradientIndex;
				GradientConstantBuffer->Map(&GradientIdx);
			}

			if (KeyboardChar.bIsPressed)
			{
				if (IsMenuActive())
				{
					if (!StartScreen->IsStartScreenFadeOut())
						StartScreen->FadeOut();
				}
				if (IsGameOverScreenActive())
				{
					if (!GameOverScreen->IsFadeOut())
						GameOverScreen->FadeOut();
				}
			}
		}

		if (InMouseInput.State == EMouseState::eMoving)
		{
			cbMouseInput MouseInput;
			MouseInput.State = cbMouseState::Moving;
			MouseInput.MouseLocation = cbVector(InMouseInput.MouseLocation.X, InMouseInput.MouseLocation.Y);

			if (Focus)
			{
				if (!Focus->IsFocused())
					Focus->OnMouseEnter(MouseInput);
				else
					Focus->OnMouseMove(MouseInput);
			}
		}
		else if (InMouseInput.State == EMouseState::eScroll)
		{
			if (Focus)
			{
				if (Focus->IsEnabled())
				{
					cbMouseInput MouseInput;

					MouseInput.MouseLocation = cbVector(InMouseInput.MouseLocation.X, InMouseInput.MouseLocation.Y);

					if (Focus->IsInside(MouseInput.MouseLocation))
					{
						Focus->OnMouseWheel(InMouseInput.WheelDelta, MouseInput);
					}
				}
				return;
			}
		}
		
		if (InMouseInput.Buttons.at("Left") == EMouseButtonState::ePressed)
		{
			cbMouseInput MouseInput;
			MouseInput.Buttons["Left"] = cbMouseButtonState::Pressed;
			MouseInput.MouseLocation = cbVector(InMouseInput.MouseLocation.X, InMouseInput.MouseLocation.Y);

			if (Focus)
			{
				if (Focus->IsFocused())
				{
					Focus->OnMouseButtonDown(MouseInput);
				}
			}
			return;
		}
		else if (InMouseInput.Buttons.at("Left") == EMouseButtonState::eReleased)
		{
			cbMouseInput MouseInput;
			MouseInput.Buttons["Left"] = cbMouseButtonState::Released;
			MouseInput.MouseLocation = cbVector(InMouseInput.MouseLocation.X, InMouseInput.MouseLocation.Y);

			if (Focus)
			{
				if (Focus->IsFocused())
				{
					Focus->OnMouseButtonUp(MouseInput);
				}
			}
			return;
		}
	}

	void GCanvas::StartScreenFadeOut()
	{
		if (StartScreen)
		{
			StartScreen->Appear();
			StartScreen->SetVisibilityState(cbgui::eVisibility::Hidden);
			StartScreen->Disable();
			if (BGImage)
			{
				BGImage->SetVisibilityState(cbgui::eVisibility::Hidden);
				BGImage->Disable();
			}
		}

		if (fOnMenuScreenFadeOut)
			fOnMenuScreenFadeOut();
	}

	bool GCanvas::IsMenuActive() const
	{
		return StartScreen ? StartScreen->IsVisible() : false;
	}

	void GCanvas::GameOver()
	{
		if (GameOverScreen)
		{
			GameOverScreen->Appear();
			GameOverScreen->SetVisibilityState(cbgui::eVisibility::Visible);
			GameOverScreen->Enable();
		}
	}

	void GCanvas::GameOverFadeOut()
	{
		if (GameOverScreen)
		{
			GameOverScreen->SetVisibilityState(cbgui::eVisibility::Hidden);
			GameOverScreen->Disable();
		}

		if (fOnGameOverScreenFadeOut)
			fOnGameOverScreenFadeOut();
	}

	bool GCanvas::IsGameOverScreenActive() const
	{
		return GameOverScreen ? GameOverScreen->IsVisible() : false;
	}
}
