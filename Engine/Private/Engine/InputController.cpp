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
#include "Engine/InputController.h"
#include <iostream>
#include <Windows.h>

FVector2 WinGetMouseLocation(const HWND& hWnd)
{
	//HWND hWnd = GetActiveWindow();
	//if (hWnd == 0)
	//	return FVector2::Zero();
	POINT point;
	GetCursorPos(&point);
	ScreenToClient(hWnd, &point);

	return FVector2(static_cast<float>(point.x), static_cast<float>(point.y));
};

sInputController::sInputController(void* InHWND)
	: pHWND(InHWND)
	, bIsKeyboardEnabled(true)
	, bIsMouseEnabled(true)
	, bIsGamepadEnabled(true)
	, MouseLocation(FVector2::Zero())
	, WheelDelta(0.0f)
	, pGamePad(std::make_unique<DirectX::GamePad>())
	, bUpdateOnTick(true)
{}

sInputController::~sInputController()
{
	pGamePad = nullptr;
	InputMap.clear();
}

void sInputController::BeginPlay()
{
}

void sInputController::Tick(const double DeltaTime)
{
}

void sInputController::FixedUpdate(const double DeltaTime)
{
	if (pHWND != GetForegroundWindow())
		return;
	Update();
}

void sInputController::Update()
{
	if (bIsGamepadEnabled)
	{
		for (auto& InputMap : GamePadInputMap)
		{
			if (InputMap.first >= DirectX::GamePad::MAX_PLAYER_COUNT)
				break;

			auto state = pGamePad->GetState(InputMap.first);

			if (state.IsConnected())
			{
				if (!InputMap.second.ConnectionStatus)
				{
					InputMap.second.ConnectionStatus = true;
					if (InputMap.second.OnConnected)
						InputMap.second.OnConnected(InputMap.first);
				}

				if (!InputMap.second.bIsEnabled)
					continue;

				InputMap.second.StateTracker.Update(state);

				if (InputMap.second.menu.bIsEnabled)
				{
					if (InputMap.second.StateTracker.menu == DirectX::GamePad::ButtonStateTracker::ButtonState::PRESSED)
					{
						if (InputMap.second.menu.fOnPressed)
							InputMap.second.menu.fOnPressed(eGamepadButtons::Menu, InputMap.first);
					}
					else if (InputMap.second.StateTracker.menu == DirectX::GamePad::ButtonStateTracker::ButtonState::HELD)
					{
						if (InputMap.second.menu.fOnHeld && InputMap.second.menu.bHoldable)
							InputMap.second.menu.fOnHeld(eGamepadButtons::Menu, InputMap.first);
					}
					else if (InputMap.second.StateTracker.menu == DirectX::GamePad::ButtonStateTracker::ButtonState::RELEASED)
					{
						if (InputMap.second.menu.fOnReleased)
							InputMap.second.menu.fOnReleased(eGamepadButtons::Menu, InputMap.first);
					}
					else if (InputMap.second.StateTracker.menu == DirectX::GamePad::ButtonStateTracker::ButtonState::UP)
					{
						if (InputMap.second.menu.fOnUp && InputMap.second.menu.bUpState)
							InputMap.second.menu.fOnUp(eGamepadButtons::Menu, InputMap.first);
					}
				}

				if (InputMap.second.view.bIsEnabled)
				{
					if (InputMap.second.StateTracker.view == DirectX::GamePad::ButtonStateTracker::ButtonState::PRESSED)
					{
						if (InputMap.second.view.fOnPressed)
							InputMap.second.view.fOnPressed(eGamepadButtons::View, InputMap.first);
					}
					else if (InputMap.second.StateTracker.view == DirectX::GamePad::ButtonStateTracker::ButtonState::HELD)
					{
						if (InputMap.second.view.fOnHeld && InputMap.second.view.bHoldable)
							InputMap.second.view.fOnHeld(eGamepadButtons::View, InputMap.first);
					}
					else if (InputMap.second.StateTracker.view == DirectX::GamePad::ButtonStateTracker::ButtonState::RELEASED)
					{
						if (InputMap.second.view.fOnReleased)
							InputMap.second.view.fOnReleased(eGamepadButtons::View, InputMap.first);
					}
					else if (InputMap.second.StateTracker.view == DirectX::GamePad::ButtonStateTracker::ButtonState::UP)
					{
						if (InputMap.second.view.fOnUp && InputMap.second.view.bUpState)
							InputMap.second.view.fOnUp(eGamepadButtons::View, InputMap.first);
					}
				}

				if (InputMap.second.leftStick.bIsEnabled)
				{
					if (InputMap.second.StateTracker.leftStick == DirectX::GamePad::ButtonStateTracker::ButtonState::PRESSED)
					{
						if (InputMap.second.leftStick.fOnPressed)
							InputMap.second.leftStick.fOnPressed(eGamepadButtons::LeftStick, InputMap.first);
					}
					else if (InputMap.second.StateTracker.leftStick == DirectX::GamePad::ButtonStateTracker::ButtonState::HELD)
					{
						if (InputMap.second.leftStick.fOnHeld && InputMap.second.leftStick.bHoldable)
							InputMap.second.leftStick.fOnHeld(eGamepadButtons::LeftStick, InputMap.first);
					}
					else if (InputMap.second.StateTracker.leftStick == DirectX::GamePad::ButtonStateTracker::ButtonState::RELEASED)
					{
						if (InputMap.second.leftStick.fOnReleased)
							InputMap.second.leftStick.fOnReleased(eGamepadButtons::LeftStick, InputMap.first);
					}
					else if (InputMap.second.StateTracker.leftStick == DirectX::GamePad::ButtonStateTracker::ButtonState::UP)
					{
						if (InputMap.second.leftStick.fOnUp && InputMap.second.leftStick.bUpState)
							InputMap.second.leftStick.fOnUp(eGamepadButtons::LeftStick, InputMap.first);
					}
				}

				if (InputMap.second.rightStick.bIsEnabled)
				{
					if (InputMap.second.StateTracker.rightStick == DirectX::GamePad::ButtonStateTracker::ButtonState::PRESSED)
					{
						if (InputMap.second.rightStick.fOnPressed)
							InputMap.second.rightStick.fOnPressed(eGamepadButtons::RightStick, InputMap.first);
					}
					else if (InputMap.second.StateTracker.rightStick == DirectX::GamePad::ButtonStateTracker::ButtonState::HELD)
					{
						if (InputMap.second.rightStick.fOnHeld && InputMap.second.rightStick.bHoldable)
							InputMap.second.rightStick.fOnHeld(eGamepadButtons::RightStick, InputMap.first);
					}
					else if (InputMap.second.StateTracker.rightStick == DirectX::GamePad::ButtonStateTracker::ButtonState::RELEASED)
					{
						if (InputMap.second.rightStick.fOnReleased)
							InputMap.second.rightStick.fOnReleased(eGamepadButtons::RightStick, InputMap.first);
					}
					else if (InputMap.second.StateTracker.rightStick == DirectX::GamePad::ButtonStateTracker::ButtonState::UP)
					{
						if (InputMap.second.rightStick.fOnUp && InputMap.second.rightStick.bUpState)
							InputMap.second.rightStick.fOnUp(eGamepadButtons::RightStick, InputMap.first);
					}
				}

				if (InputMap.second.LeftThumbStick.bIsEnabled)
				{
					// These values are normalized to -1 to 1
					float posx = state.thumbSticks.leftX;
					float posy = state.thumbSticks.leftY;

					if (InputMap.second.LeftThumbStick.X)
						InputMap.second.LeftThumbStick.X(posx, eGamepadButtons::LeftThumbSticks, InputMap.first);
					if (InputMap.second.LeftThumbStick.Y)
						InputMap.second.LeftThumbStick.Y(posy, eGamepadButtons::LeftThumbSticks, InputMap.first);
					if (InputMap.second.LeftThumbStick.fOnXYAxis)
						InputMap.second.LeftThumbStick.fOnXYAxis(FVector2(posx, posy), eGamepadButtons::LeftThumbSticks, InputMap.first);
				}

				if (InputMap.second.RightThumbStick.bIsEnabled)
				{
					// These values are normalized to -1 to 1
					float posx = state.thumbSticks.rightX;
					float posy = state.thumbSticks.rightY;

					if (InputMap.second.RightThumbStick.X)
						InputMap.second.RightThumbStick.X(posx, eGamepadButtons::RightThumbSticks, InputMap.first);
					if (InputMap.second.RightThumbStick.Y)
						InputMap.second.RightThumbStick.Y(posy, eGamepadButtons::RightThumbSticks, InputMap.first);
					if (InputMap.second.RightThumbStick.fOnXYAxis)
						InputMap.second.RightThumbStick.fOnXYAxis(FVector2(posx, posy), eGamepadButtons::RightThumbSticks, InputMap.first);
				}

				if (InputMap.second.up.bIsEnabled)
				{
					if (InputMap.second.StateTracker.dpadUp == DirectX::GamePad::ButtonStateTracker::ButtonState::PRESSED)
					{
						if (InputMap.second.up.fOnPressed)
							InputMap.second.up.fOnPressed(eGamepadButtons::Up, InputMap.first);
					}
					else if (InputMap.second.StateTracker.dpadUp == DirectX::GamePad::ButtonStateTracker::ButtonState::HELD)
					{
						if (InputMap.second.up.fOnHeld && InputMap.second.up.bHoldable)
							InputMap.second.up.fOnHeld(eGamepadButtons::Up, InputMap.first);
					}
					else if (InputMap.second.StateTracker.dpadUp == DirectX::GamePad::ButtonStateTracker::ButtonState::RELEASED)
					{
						if (InputMap.second.up.fOnReleased)
							InputMap.second.up.fOnReleased(eGamepadButtons::Up, InputMap.first);
					}
					else if (InputMap.second.StateTracker.dpadUp == DirectX::GamePad::ButtonStateTracker::ButtonState::UP)
					{
						if (InputMap.second.up.fOnUp && InputMap.second.up.bUpState)
							InputMap.second.up.fOnUp(eGamepadButtons::Up, InputMap.first);
					}
				}

				if (InputMap.second.down.bIsEnabled)
				{
					if (InputMap.second.StateTracker.dpadDown == DirectX::GamePad::ButtonStateTracker::ButtonState::PRESSED)
					{
						if (InputMap.second.down.fOnPressed)
							InputMap.second.down.fOnPressed(eGamepadButtons::Down, InputMap.first);
					}
					else if (InputMap.second.StateTracker.dpadDown == DirectX::GamePad::ButtonStateTracker::ButtonState::HELD)
					{
						if (InputMap.second.down.fOnHeld && InputMap.second.down.bHoldable)
							InputMap.second.down.fOnHeld(eGamepadButtons::Down, InputMap.first);
					}
					else if (InputMap.second.StateTracker.dpadDown == DirectX::GamePad::ButtonStateTracker::ButtonState::RELEASED)
					{
						if (InputMap.second.down.fOnReleased)
							InputMap.second.down.fOnReleased(eGamepadButtons::Down, InputMap.first);
					}
					else if (InputMap.second.StateTracker.dpadDown == DirectX::GamePad::ButtonStateTracker::ButtonState::UP)
					{
						if (InputMap.second.down.fOnUp && InputMap.second.down.bUpState)
							InputMap.second.down.fOnUp(eGamepadButtons::Down, InputMap.first);
					}
				}

				if (InputMap.second.left.bIsEnabled)
				{
					if (InputMap.second.StateTracker.dpadLeft == DirectX::GamePad::ButtonStateTracker::ButtonState::PRESSED)
					{
						if (InputMap.second.left.fOnPressed)
							InputMap.second.left.fOnPressed(eGamepadButtons::Left, InputMap.first);
					}
					else if (InputMap.second.StateTracker.dpadLeft == DirectX::GamePad::ButtonStateTracker::ButtonState::HELD)
					{
						if (InputMap.second.left.fOnHeld && InputMap.second.left.bHoldable)
							InputMap.second.left.fOnHeld(eGamepadButtons::Left, InputMap.first);
					}
					else if (InputMap.second.StateTracker.dpadLeft == DirectX::GamePad::ButtonStateTracker::ButtonState::RELEASED)
					{
						if (InputMap.second.left.fOnReleased)
							InputMap.second.left.fOnReleased(eGamepadButtons::Left, InputMap.first);
					}
					else if (InputMap.second.StateTracker.dpadLeft == DirectX::GamePad::ButtonStateTracker::ButtonState::UP)
					{
						if (InputMap.second.left.fOnUp && InputMap.second.left.bUpState)
							InputMap.second.left.fOnUp(eGamepadButtons::Left, InputMap.first);
					}
				}

				if (InputMap.second.right.bIsEnabled)
				{
					if (InputMap.second.StateTracker.dpadRight == DirectX::GamePad::ButtonStateTracker::ButtonState::PRESSED)
					{
						if (InputMap.second.right.fOnPressed)
							InputMap.second.right.fOnPressed(eGamepadButtons::Right, InputMap.first);
					}
					else if (InputMap.second.StateTracker.dpadRight == DirectX::GamePad::ButtonStateTracker::ButtonState::HELD)
					{
						if (InputMap.second.right.fOnHeld && InputMap.second.right.bHoldable)
							InputMap.second.right.fOnHeld(eGamepadButtons::Right, InputMap.first);
					}
					else if (InputMap.second.StateTracker.dpadRight == DirectX::GamePad::ButtonStateTracker::ButtonState::RELEASED)
					{
						if (InputMap.second.right.fOnReleased)
							InputMap.second.right.fOnReleased(eGamepadButtons::Right, InputMap.first);
					}
					else if (InputMap.second.StateTracker.dpadRight == DirectX::GamePad::ButtonStateTracker::ButtonState::UP)
					{
						if (InputMap.second.right.fOnUp && InputMap.second.right.bUpState)
							InputMap.second.right.fOnUp(eGamepadButtons::Right, InputMap.first);
					}
				}

				if (InputMap.second.a.bIsEnabled)
				{
					if (InputMap.second.StateTracker.a == DirectX::GamePad::ButtonStateTracker::ButtonState::PRESSED)
					{
						if (InputMap.second.a.fOnPressed)
							InputMap.second.a.fOnPressed(eGamepadButtons::A, InputMap.first);
					}
					else if (InputMap.second.StateTracker.a == DirectX::GamePad::ButtonStateTracker::ButtonState::HELD)
					{
						if (InputMap.second.a.fOnHeld && InputMap.second.a.bHoldable)
							InputMap.second.a.fOnHeld(eGamepadButtons::A, InputMap.first);
					}
					else if (InputMap.second.StateTracker.a == DirectX::GamePad::ButtonStateTracker::ButtonState::RELEASED)
					{
						if (InputMap.second.a.fOnReleased)
							InputMap.second.a.fOnReleased(eGamepadButtons::A, InputMap.first);
					}
					else if (InputMap.second.StateTracker.a == DirectX::GamePad::ButtonStateTracker::ButtonState::UP)
					{
						if (InputMap.second.a.fOnUp && InputMap.second.a.bUpState)
							InputMap.second.a.fOnUp(eGamepadButtons::A, InputMap.first);
					}
				}

				if (InputMap.second.b.bIsEnabled)
				{
					if (InputMap.second.StateTracker.b == DirectX::GamePad::ButtonStateTracker::ButtonState::PRESSED)
					{
						if (InputMap.second.b.fOnPressed)
							InputMap.second.b.fOnPressed(eGamepadButtons::B, InputMap.first);
					}
					else if (InputMap.second.StateTracker.b == DirectX::GamePad::ButtonStateTracker::ButtonState::HELD)
					{
						if (InputMap.second.b.fOnHeld && InputMap.second.b.bHoldable)
							InputMap.second.b.fOnHeld(eGamepadButtons::B, InputMap.first);
					}
					else if (InputMap.second.StateTracker.b == DirectX::GamePad::ButtonStateTracker::ButtonState::RELEASED)
					{
						if (InputMap.second.b.fOnReleased)
							InputMap.second.b.fOnReleased(eGamepadButtons::B, InputMap.first);
					}
					else if (InputMap.second.StateTracker.b == DirectX::GamePad::ButtonStateTracker::ButtonState::UP)
					{
						if (InputMap.second.b.fOnUp && InputMap.second.b.bUpState)
							InputMap.second.b.fOnUp(eGamepadButtons::B, InputMap.first);
					}
				}

				if (InputMap.second.y.bIsEnabled)
				{
					if (InputMap.second.StateTracker.y == DirectX::GamePad::ButtonStateTracker::ButtonState::PRESSED)
					{
						if (InputMap.second.y.fOnPressed)
							InputMap.second.y.fOnPressed(eGamepadButtons::Y, InputMap.first);
					}
					else if (InputMap.second.StateTracker.y == DirectX::GamePad::ButtonStateTracker::ButtonState::HELD)
					{
						if (InputMap.second.y.fOnHeld && InputMap.second.y.bHoldable)
							InputMap.second.y.fOnHeld(eGamepadButtons::Y, InputMap.first);
					}
					else if (InputMap.second.StateTracker.y == DirectX::GamePad::ButtonStateTracker::ButtonState::RELEASED)
					{
						if (InputMap.second.y.fOnReleased)
							InputMap.second.y.fOnReleased(eGamepadButtons::Y, InputMap.first);
					}
					else if (InputMap.second.StateTracker.y == DirectX::GamePad::ButtonStateTracker::ButtonState::UP)
					{
						if (InputMap.second.y.fOnUp && InputMap.second.y.bUpState)
							InputMap.second.y.fOnUp(eGamepadButtons::Y, InputMap.first);
					}
				}

				if (InputMap.second.x.bIsEnabled)
				{
					if (InputMap.second.StateTracker.x == DirectX::GamePad::ButtonStateTracker::ButtonState::PRESSED)
					{
						if (InputMap.second.x.fOnPressed)
							InputMap.second.x.fOnPressed(eGamepadButtons::X, InputMap.first);
					}
					else if (InputMap.second.StateTracker.x == DirectX::GamePad::ButtonStateTracker::ButtonState::HELD)
					{
						if (InputMap.second.x.fOnHeld && InputMap.second.x.bHoldable)
							InputMap.second.x.fOnHeld(eGamepadButtons::X, InputMap.first);
					}
					else if (InputMap.second.StateTracker.x == DirectX::GamePad::ButtonStateTracker::ButtonState::RELEASED)
					{
						if (InputMap.second.x.fOnReleased)
							InputMap.second.x.fOnReleased(eGamepadButtons::X, InputMap.first);
					}
					else if (InputMap.second.StateTracker.x == DirectX::GamePad::ButtonStateTracker::ButtonState::UP)
					{
						if (InputMap.second.x.fOnUp && InputMap.second.x.bUpState)
							InputMap.second.x.fOnUp(eGamepadButtons::X, InputMap.first);
					}
				}

				if (InputMap.second.leftShoulder.bIsEnabled)
				{
					if (InputMap.second.StateTracker.leftShoulder == DirectX::GamePad::ButtonStateTracker::ButtonState::PRESSED)
					{
						if (InputMap.second.leftShoulder.fOnPressed)
							InputMap.second.leftShoulder.fOnPressed(eGamepadButtons::LeftShoulder, InputMap.first);
					}
					else if (InputMap.second.StateTracker.leftShoulder == DirectX::GamePad::ButtonStateTracker::ButtonState::HELD)
					{
						if (InputMap.second.leftShoulder.fOnHeld && InputMap.second.leftShoulder.bHoldable)
							InputMap.second.leftShoulder.fOnHeld(eGamepadButtons::LeftShoulder, InputMap.first);
					}
					else if (InputMap.second.StateTracker.leftShoulder == DirectX::GamePad::ButtonStateTracker::ButtonState::RELEASED)
					{
						if (InputMap.second.leftShoulder.fOnReleased)
							InputMap.second.leftShoulder.fOnReleased(eGamepadButtons::LeftShoulder, InputMap.first);
					}
					else if (InputMap.second.StateTracker.leftShoulder == DirectX::GamePad::ButtonStateTracker::ButtonState::UP)
					{
						if (InputMap.second.leftShoulder.fOnUp && InputMap.second.leftShoulder.bUpState)
							InputMap.second.leftShoulder.fOnUp(eGamepadButtons::LeftShoulder, InputMap.first);
					}
				}

				if (InputMap.second.rightShoulder.bIsEnabled)
				{
					if (InputMap.second.StateTracker.rightShoulder == DirectX::GamePad::ButtonStateTracker::ButtonState::PRESSED)
					{
						if (InputMap.second.rightShoulder.fOnPressed)
							InputMap.second.rightShoulder.fOnPressed(eGamepadButtons::RightShoulder, InputMap.first);
					}
					else if (InputMap.second.StateTracker.rightShoulder == DirectX::GamePad::ButtonStateTracker::ButtonState::HELD)
					{
						if (InputMap.second.rightShoulder.fOnHeld && InputMap.second.rightShoulder.bHoldable)
							InputMap.second.rightShoulder.fOnHeld(eGamepadButtons::RightShoulder, InputMap.first);
					}
					else if (InputMap.second.StateTracker.rightShoulder == DirectX::GamePad::ButtonStateTracker::ButtonState::RELEASED)
					{
						if (InputMap.second.rightShoulder.fOnReleased)
							InputMap.second.rightShoulder.fOnReleased(eGamepadButtons::RightShoulder, InputMap.first);
					}
					else if (InputMap.second.StateTracker.rightShoulder == DirectX::GamePad::ButtonStateTracker::ButtonState::UP)
					{
						if (InputMap.second.rightShoulder.fOnUp && InputMap.second.rightShoulder.bUpState)
							InputMap.second.rightShoulder.fOnUp(eGamepadButtons::RightShoulder, InputMap.first);
					}
				}

				if (InputMap.second.RightTrigger.bIsEnabled)
				{
					// state.triggers.right This value is normalized 0 -> 1

					if (InputMap.second.StateTracker.rightTrigger == DirectX::GamePad::ButtonStateTracker::ButtonState::PRESSED)
					{
						if (InputMap.second.RightTrigger.fOnPressed)
							InputMap.second.RightTrigger.fOnPressed(state.triggers.right, eGamepadButtons::RightTrigger, InputMap.first);
					}
					else if (InputMap.second.StateTracker.rightTrigger == DirectX::GamePad::ButtonStateTracker::ButtonState::HELD)
					{
						if (InputMap.second.RightTrigger.fOnHeld && InputMap.second.RightTrigger.bHoldable)
							InputMap.second.RightTrigger.fOnHeld(state.triggers.right, eGamepadButtons::RightTrigger, InputMap.first);
					}
					else if (InputMap.second.StateTracker.rightTrigger == DirectX::GamePad::ButtonStateTracker::ButtonState::RELEASED)
					{
						if (InputMap.second.RightTrigger.fOnReleased)
							InputMap.second.RightTrigger.fOnReleased(state.triggers.right, eGamepadButtons::RightTrigger, InputMap.first);
					}
					else if (InputMap.second.StateTracker.rightTrigger == DirectX::GamePad::ButtonStateTracker::ButtonState::UP)
					{
						if (InputMap.second.RightTrigger.fOnUp && InputMap.second.RightTrigger.bUpState)
							InputMap.second.RightTrigger.fOnUp(state.triggers.right, eGamepadButtons::RightTrigger, InputMap.first);
					}
				}

				if (InputMap.second.LeftTrigger.bIsEnabled)
				{
					// state.triggers.left This value is normalized 0 -> 1

					if (InputMap.second.StateTracker.leftTrigger == DirectX::GamePad::ButtonStateTracker::ButtonState::PRESSED)
					{
						if (InputMap.second.LeftTrigger.fOnPressed)
							InputMap.second.LeftTrigger.fOnPressed(state.triggers.left, eGamepadButtons::LeftTrigger, InputMap.first);
					}
					else if (InputMap.second.StateTracker.leftTrigger == DirectX::GamePad::ButtonStateTracker::ButtonState::HELD)
					{
						if (InputMap.second.LeftTrigger.fOnHeld && InputMap.second.LeftTrigger.bHoldable)
							InputMap.second.LeftTrigger.fOnHeld(state.triggers.left, eGamepadButtons::LeftTrigger, InputMap.first);
					}
					else if (InputMap.second.StateTracker.leftTrigger == DirectX::GamePad::ButtonStateTracker::ButtonState::RELEASED)
					{
						if (InputMap.second.LeftTrigger.fOnReleased)
							InputMap.second.LeftTrigger.fOnReleased(state.triggers.left, eGamepadButtons::LeftTrigger, InputMap.first);
					}
					else if (InputMap.second.StateTracker.leftTrigger == DirectX::GamePad::ButtonStateTracker::ButtonState::UP)
					{
						if (InputMap.second.LeftTrigger.fOnUp && InputMap.second.LeftTrigger.bUpState)
							InputMap.second.LeftTrigger.fOnUp(state.triggers.left, eGamepadButtons::LeftTrigger, InputMap.first);
					}
				}
			}
			else
			{
				if (InputMap.second.ConnectionStatus)
				{
					InputMap.second.ConnectionStatus = false;
					if (InputMap.second.OnDisconnected)
						InputMap.second.OnDisconnected(InputMap.first);
				}
			}
		}
	}

	if (bUpdateOnTick)
	{
		if (bIsMouseEnabled)
		{
			FVector2 CurrentMouseLocation = WinGetMouseLocation((HWND)pHWND);
			if (MouseAxisInput.fOnXAxis)
			{
				MouseAxisInput.fOnXAxis(MouseLocation.X == CurrentMouseLocation.X ? 0.0f : MouseLocation.X > CurrentMouseLocation.X ? 1.0f : -1.0f, CurrentMouseLocation.X);
			}
			if (MouseAxisInput.fOnYAxis)
			{
				MouseAxisInput.fOnYAxis(MouseLocation.Y == CurrentMouseLocation.Y ? 0.0f : MouseLocation.Y > CurrentMouseLocation.Y ? 1.0f : -1.0f, CurrentMouseLocation.Y);
			}
			if (MouseAxisInput.fOnXYAxis)
			{
				MouseAxisInput.fOnXYAxis(MouseLocation == CurrentMouseLocation ? 0.0f : MouseLocation > CurrentMouseLocation ? 1.0f : -1.0f, CurrentMouseLocation);
			}
			MouseLocation = CurrentMouseLocation;

			//if (MouseAxisInput.fOnWheel)
			{
				//MouseAxisInput.fOnWheel(WheelDelta);
			}
		}

		if (bIsKeyboardEnabled)
		{
			const bool bIsLControlPressed = (GetAsyncKeyState(VK_LCONTROL) & 0x8000);
			const bool bIsRControlPressed = (GetAsyncKeyState(VK_RCONTROL) & 0x8000);
			const bool bIsLShiftPressed = (GetAsyncKeyState(VK_LSHIFT) & 0x8000);
			const bool bIsRShiftPressed = (GetAsyncKeyState(VK_RSHIFT) & 0x8000);
			const bool bIsLAltPressed = (GetAsyncKeyState(VK_LMENU) & 0x8000);
			const bool bIsRAltPressed = (GetAsyncKeyState(VK_RMENU) & 0x8000);
			const bool bIsCapitalPressed = (GetAsyncKeyState(VK_CAPITAL) & 0x8000);

			for (auto& Input : InputMap)
			{
				sKMButtonInput& KeyboardInput = Input.second;
				sKMButtonInputDesc& InputDesc = KeyboardInput.Desc;
				if (InputDesc.bIsEnabled)
				{
					if (InputDesc.IsMouseInput() && !bIsMouseEnabled)
						continue;

					if (InputDesc.Modifiers[eModifiers::LeftShift] && !bIsLShiftPressed || InputDesc.Modifiers[eModifiers::RightShift] && !bIsRShiftPressed ||
						InputDesc.Modifiers[eModifiers::LeftCTRL] && !bIsLControlPressed || InputDesc.Modifiers[eModifiers::RightCTRL] && !bIsRControlPressed ||
						InputDesc.Modifiers[eModifiers::LeftAlt] && !bIsLAltPressed || InputDesc.Modifiers[eModifiers::RightAlt] && !bIsRAltPressed ||
						InputDesc.Modifiers[eModifiers::CapsLock] && !bIsCapitalPressed)
					{
						if (KeyboardInput.State == InputState::PRESSED)
						{
							KeyboardInput.State = InputState::RELEASED;
							InputDesc.fOnReleased(InputDesc.Key);
						}
						continue;
					}

					const bool bIsPressed = (GetAsyncKeyState(InputDesc.Key) & 0x8000);

					if (bIsPressed && KeyboardInput.State != InputState::PRESSED)
					{
						KeyboardInput.State = InputState::PRESSED;
						if (InputDesc.fOnPressed)
							InputDesc.fOnPressed(InputDesc.Key);
					}
					else if (!bIsPressed && KeyboardInput.State == InputState::PRESSED)
					{
						KeyboardInput.State = InputState::RELEASED;
						if (InputDesc.fOnReleased)
							InputDesc.fOnReleased(InputDesc.Key);
					}
					else if (bIsPressed)
					{
						if (InputDesc.bHoldable)
						{
							if (InputDesc.fOnHeld)
								InputDesc.fOnHeld(InputDesc.Key);
							else if (InputDesc.fOnPressed)
								InputDesc.fOnPressed(InputDesc.Key);
						}
					}
				}
			}
		}
	}
}

void sInputController::BindInput(std::string Name, const sKMButtonInputDesc Desc)
{
	InputMap.insert(std::make_pair(Name, sKMButtonInput(Desc)));
}

void sInputController::RemoveInput(std::string Name)
{
	InputMap.erase(Name);
}

void sInputController::BindInputToMouseXAxis(std::function<void(float, float)> fAxis)
{
	MouseAxisInput.fOnXAxis = fAxis;
}

void sInputController::BindInputToMouseYAxis(std::function<void(float, float)> fAxis)
{
	MouseAxisInput.fOnYAxis = fAxis;
}

void sInputController::BindInputToMouseXYAxis(std::function<void(float, FVector2)> fAxis)
{
	MouseAxisInput.fOnXYAxis = fAxis;
}

void sInputController::BindInputToMouseWheel(std::function<void(float)> fAxis)
{
	MouseAxisInput.fOnWheel = fAxis;
}

bool sInputController::SetKeyState(std::string name, bool State)
{
	if (InputMap.find(name) != InputMap.end())
		return false;
	InputMap.at(name).Desc.bIsEnabled = State;
	return true;
}

bool sInputController::SetKeyHoldableState(std::string name, bool bHoldAble)
{
	if (InputMap.find(name) != InputMap.end())
		return false;
	InputMap.at(name).Desc.bHoldable = bHoldAble;
	return true;
}

void sInputController::SetMouseAxisState(bool State)
{
	MouseAxisInput.bIsEnabled = State;
}

bool sInputController::SetGamePadInputMap(eGamepadPlayer Player, sGamepadButtonInputDesc Input)
{
	if (GamePadInputMap.find(Player) != GamePadInputMap.end())
		return false;
	GamePadInputMap.insert(std::make_pair(Player, Input));
	return true;
}

bool sInputController::RemoveGamePadInputMap(eGamepadPlayer Player)
{
	if (GamePadInputMap.find(Player) != GamePadInputMap.end())
		return false;
	GamePadInputMap.erase(Player);
	return true;
}

bool sInputController::SetGamepadVibration(int player, float leftMotor, float rightMotor, float leftTrigger, float rightTrigger) noexcept
{
	return pGamePad->SetVibration(player, leftMotor, rightMotor, leftTrigger, rightTrigger);
}

bool sInputController::SetGamePadState(eGamepadPlayer Player, bool State)
{
	if (GamePadInputMap.find(Player) != GamePadInputMap.end())
		return false;
	GamePadInputMap.at(Player).bIsEnabled = State;
	return true;
}

bool sInputController::SetGamepadButtonState(eGamepadPlayer Player, eGamepadButtons Button, bool State)
{
	if (GamePadInputMap.find(Player) != GamePadInputMap.end())
		return false;

	switch (Button)
	{
		case A:	GamePadInputMap.at(Player).a.bIsEnabled = State; return true;  
		case B:	GamePadInputMap.at(Player).b.bIsEnabled = State; return true;  
		case X:	GamePadInputMap.at(Player).x.bIsEnabled = State; return true;  
		case Y:	GamePadInputMap.at(Player).y.bIsEnabled = State; return true;  
		case LeftStick:	GamePadInputMap.at(Player).leftStick.bIsEnabled = State; return true;  
		case RightStick: GamePadInputMap.at(Player).rightStick.bIsEnabled = State; return true;  
		case LeftShoulder: GamePadInputMap.at(Player).leftShoulder.bIsEnabled = State; return true;  
		case RightShoulder: GamePadInputMap.at(Player).rightShoulder.bIsEnabled = State; return true;  
		case View: GamePadInputMap.at(Player).view.bIsEnabled = State; return true;  
		case Menu: GamePadInputMap.at(Player).menu.bIsEnabled = State; return true;  
		case Up: GamePadInputMap.at(Player).up.bIsEnabled = State; return true;  
		case Down: GamePadInputMap.at(Player).down.bIsEnabled = State; return true;  
		case Right: GamePadInputMap.at(Player).right.bIsEnabled = State; return true;  
		case Left: GamePadInputMap.at(Player).left.bIsEnabled = State; return true;  
		case LeftThumbSticks: GamePadInputMap.at(Player).LeftThumbStick.bIsEnabled = State; return true;  
		case RightThumbSticks: GamePadInputMap.at(Player).RightThumbStick.bIsEnabled = State; return true;  
		case LeftTrigger: GamePadInputMap.at(Player).LeftTrigger.bIsEnabled = State; return true;  
		case RightTrigger: GamePadInputMap.at(Player).RightTrigger.bIsEnabled = State; return true;  
	}
	return false;
}

bool sInputController::SetGamepadButtonHoldableState(eGamepadPlayer Player, eGamepadButtons Button, bool bHoldAble)
{
	if (GamePadInputMap.find(Player) != GamePadInputMap.end())
		return false;

	switch (Button)
	{
		case A:	GamePadInputMap.at(Player).a.bHoldable = bHoldAble; return true;  
		case B:	GamePadInputMap.at(Player).b.bHoldable = bHoldAble; return true;  
		case X:	GamePadInputMap.at(Player).x.bHoldable = bHoldAble; return true;  
		case Y:	GamePadInputMap.at(Player).y.bHoldable = bHoldAble; return true;  
		case LeftStick:	GamePadInputMap.at(Player).leftStick.bHoldable = bHoldAble; return true;  
		case RightStick: GamePadInputMap.at(Player).rightStick.bHoldable = bHoldAble; return true;  
		case LeftShoulder: GamePadInputMap.at(Player).leftShoulder.bHoldable = bHoldAble; return true;  
		case RightShoulder: GamePadInputMap.at(Player).rightShoulder.bHoldable = bHoldAble; return true;  
		case View: GamePadInputMap.at(Player).view.bHoldable = bHoldAble; return true;  
		case Menu: GamePadInputMap.at(Player).menu.bHoldable = bHoldAble; return true;  
		case Up: GamePadInputMap.at(Player).up.bHoldable = bHoldAble; return true;  
		case Down: GamePadInputMap.at(Player).down.bHoldable = bHoldAble; return true;  
		case Right: GamePadInputMap.at(Player).right.bHoldable = bHoldAble; return true;  
		case Left: GamePadInputMap.at(Player).left.bHoldable = bHoldAble; return true;  
		case LeftTrigger: GamePadInputMap.at(Player).LeftTrigger.bHoldable = bHoldAble; return true;  
		case RightTrigger: GamePadInputMap.at(Player).RightTrigger.bHoldable = bHoldAble; return true;  
	}
	return false;
}

bool sInputController::SetGamepadButtonUpState(eGamepadPlayer Player, eGamepadButtons Button, bool bUpState)
{
	if (GamePadInputMap.find(Player) != GamePadInputMap.end())
		return false;

	switch (Button)
	{
		case A:	GamePadInputMap.at(Player).a.bUpState = bUpState; return true;
		case B:	GamePadInputMap.at(Player).b.bUpState = bUpState; return true;
		case X:	GamePadInputMap.at(Player).x.bUpState = bUpState; return true;
		case Y:	GamePadInputMap.at(Player).y.bUpState = bUpState; return true;
		case LeftStick:	GamePadInputMap.at(Player).leftStick.bUpState = bUpState; return true;
		case RightStick: GamePadInputMap.at(Player).rightStick.bUpState = bUpState; return true;
		case LeftShoulder: GamePadInputMap.at(Player).leftShoulder.bUpState = bUpState; return true;
		case RightShoulder: GamePadInputMap.at(Player).rightShoulder.bUpState = bUpState; return true;
		case View: GamePadInputMap.at(Player).view.bUpState = bUpState; return true;
		case Menu: GamePadInputMap.at(Player).menu.bUpState = bUpState; return true;
		case Up: GamePadInputMap.at(Player).up.bUpState = bUpState; return true;
		case Down: GamePadInputMap.at(Player).down.bUpState = bUpState; return true;
		case Right: GamePadInputMap.at(Player).right.bUpState = bUpState; return true;
		case Left: GamePadInputMap.at(Player).left.bUpState = bUpState; return true;
		case LeftTrigger: GamePadInputMap.at(Player).LeftTrigger.bUpState = bUpState; return true;
		case RightTrigger: GamePadInputMap.at(Player).RightTrigger.bUpState = bUpState; return true;
	}
	return false;
}

void sInputController::InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar)
{
	WheelDelta = (float)MouseInput.WheelDelta;

	if (MouseAxisInput.fOnWheel)
	{
		MouseAxisInput.fOnWheel(WheelDelta);
	}

	if (!bUpdateOnTick)
	{
		if (bIsMouseEnabled)
		{
			FVector2 CurrentMouseLocation = MouseInput.MouseLocation;
			if (MouseAxisInput.fOnXAxis)
			{
				MouseAxisInput.fOnXAxis(MouseLocation.X == CurrentMouseLocation.X ? 0.0f : MouseLocation.X > CurrentMouseLocation.X ? 1.0f : -1.0f, CurrentMouseLocation.X);
			}
			if (MouseAxisInput.fOnYAxis)
			{
				MouseAxisInput.fOnYAxis(MouseLocation.Y == CurrentMouseLocation.Y ? 0.0f : MouseLocation.Y > CurrentMouseLocation.Y ? 1.0f : -1.0f, CurrentMouseLocation.Y);
			}
			if (MouseAxisInput.fOnXYAxis)
			{
				MouseAxisInput.fOnXYAxis(MouseLocation == CurrentMouseLocation ? 0.0f : MouseLocation > CurrentMouseLocation ? 1.0f : -1.0f, CurrentMouseLocation);
			}
			MouseLocation = CurrentMouseLocation;

			//if (MouseAxisInput.fOnWheel)
			{
				//MouseAxisInput.fOnWheel(WheelDelta);
			}
		}

		if (bIsKeyboardEnabled)
		{
			const bool bIsLControlPressed = (GetAsyncKeyState(VK_LCONTROL) & 0x8000);
			const bool bIsRControlPressed = (GetAsyncKeyState(VK_RCONTROL) & 0x8000);
			const bool bIsLShiftPressed = (GetAsyncKeyState(VK_LSHIFT) & 0x8000);
			const bool bIsRShiftPressed = (GetAsyncKeyState(VK_RSHIFT) & 0x8000);
			const bool bIsLAltPressed = (GetAsyncKeyState(VK_LMENU) & 0x8000);
			const bool bIsRAltPressed = (GetAsyncKeyState(VK_RMENU) & 0x8000);
			const bool bIsCapitalPressed = (GetAsyncKeyState(VK_CAPITAL) & 0x8000);

			int iKeyPressed = (int)KeyboardChar.KeyCode;
			int iKeyPressed2 = towlower(iKeyPressed);
			int iKeyPressed3 = towupper(iKeyPressed);

			for (auto& Input : InputMap)
			{
				sKMButtonInput& KeyboardInput = Input.second;
				sKMButtonInputDesc& InputDesc = KeyboardInput.Desc;
				if (InputDesc.bIsEnabled)
				{
					if (KeyboardChar.KeyCode == 0)
						continue;

					if (towlower(iKeyPressed) == InputDesc.Key || towupper(iKeyPressed) == InputDesc.Key)
					{
						if (InputDesc.IsMouseInput() && !bIsMouseEnabled)
							continue;

						if (InputDesc.Modifiers[eModifiers::LeftShift] && !bIsLShiftPressed || InputDesc.Modifiers[eModifiers::RightShift] && !bIsRShiftPressed ||
							InputDesc.Modifiers[eModifiers::LeftCTRL] && !bIsLControlPressed || InputDesc.Modifiers[eModifiers::RightCTRL] && !bIsRControlPressed ||
							InputDesc.Modifiers[eModifiers::LeftAlt] && !bIsLAltPressed || InputDesc.Modifiers[eModifiers::RightAlt] && !bIsRAltPressed ||
							InputDesc.Modifiers[eModifiers::CapsLock] && !bIsCapitalPressed)
						{
							if (KeyboardInput.State == InputState::PRESSED)
							{
								KeyboardInput.State = InputState::RELEASED;
								InputDesc.fOnReleased(InputDesc.Key);
							}
							continue;
						}

						bool bIsPressed = InputDesc.IsMouseInput() ? false : KeyboardChar.bIsPressed;

						if (InputDesc.IsMouseInput())
						{
							if (InputDesc.Key == eMouseButtons::LeftButton)
								bIsPressed = MouseInput.Buttons.at("Left") == EMouseButtonState::ePressed;
							else if (InputDesc.Key == eMouseButtons::RightButton)
								bIsPressed = MouseInput.Buttons.at("Right") == EMouseButtonState::ePressed;
							else if (InputDesc.Key == eMouseButtons::MiddleButton)
								bIsPressed = MouseInput.Buttons.at("Middle") == EMouseButtonState::ePressed;
							else if (InputDesc.Key == eMouseButtons::X1Button)
								bIsPressed = MouseInput.Buttons.at("XB1") == EMouseButtonState::ePressed;
							else if (InputDesc.Key == eMouseButtons::X2Button)
								bIsPressed = MouseInput.Buttons.at("XB2") == EMouseButtonState::ePressed;
						}

						if (bIsPressed && KeyboardInput.State != InputState::PRESSED)
						{
							KeyboardInput.State = InputState::PRESSED;
							InputDesc.fOnPressed(InputDesc.Key);
						}
						else if (!bIsPressed && KeyboardInput.State == InputState::PRESSED)
						{
							KeyboardInput.State = InputState::RELEASED;
							InputDesc.fOnReleased(InputDesc.Key);
						}
						else if (bIsPressed)
						{
							if (InputDesc.bHoldable && InputDesc.fOnHeld)
								InputDesc.fOnHeld(InputDesc.Key);
						}

						break;
					}
					else
					{
						//std::cout << "Warning : " << "---Do not send modifier keys!!!---" << std::endl;
					}
				}
			}
		}
	}
}
