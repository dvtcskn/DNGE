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

#include <functional>
#include <memory>
#include <map>
#include <vector>
#include <string>
#include <array>
#include "Engine/ClassBody.h"
#include "Utilities/Input.h"
#include "Gamepad.h"

enum eModifiers : char8_t
{
	LeftCTRL	= 0xA2,
	RightCTRL	= 0xA3,
	LeftShift	= 0xA0,
	RightShift	= 0xA1,
	LeftAlt		= 0xA4,
	RightAlt	= 0xA5,
	CapsLock	= 0x14,
};

enum eMouseButtons : char8_t
{
	LeftButton		= 0x01,
	RightButton		= 0x02,
	MiddleButton	= 0x04,
	X1Button		= 0x05,
	X2Button		= 0x06,
};

struct sKMButtonInputDesc
{
	int Key = 0;
	bool bHoldable = false;
	bool bIsEnabled = true;

	std::map<eModifiers, bool> Modifiers;

	std::function<void(int)> fOnPressed = nullptr;
	std::function<void(int)> fOnHeld = nullptr;
	std::function<void(int)> fOnReleased = nullptr;

	inline bool IsMouseInput() const
	{
		return Key == eMouseButtons::LeftButton || Key == eMouseButtons::RightButton ||
			Key == eMouseButtons::MiddleButton || Key == eMouseButtons::X1Button ||
			Key == eMouseButtons::X2Button;
	}

	inline bool IsKeyboardInput() const
	{
		return !IsMouseInput();
	}

	sKMButtonInputDesc() = default;
	~sKMButtonInputDesc()
	{
		fOnPressed = nullptr;
		fOnHeld = nullptr;
		fOnReleased = nullptr;
		Modifiers.clear();
	}
};

enum eGamepadButtons : char8_t
{
	A				 = 0x01,
	B				 = 0x02,
	X				 = 0x03,
	Y				 = 0x04,
	LeftStick		 = 0x05,
	RightStick		 = 0x06,
	LeftShoulder	 = 0x07,
	RightShoulder	 = 0x08,
	Back			 = 0x09,
	View			 = Back,
	Start			 = 0x0A,
	Menu			 = Start,
	Up				 = 0x0B,
	Down			 = 0x0C,
	Right			 = 0x0D,
	Left			 = 0x0E,
	RightTrigger	 = 0x0F,
	LeftTrigger		 = 0x10,
	LeftThumbSticks  = 0x11,
	RightThumbSticks = 0x12,
	Unknown			 = 0x13,
};

enum eGamepadPlayer : char8_t
{
	Player1 = 0x00,
	Player2 = 0x01,
	Player3 = 0x02,
	Player4 = 0x03,
	Player5 = 0x04,
	Player6 = 0x05,
	Player7 = 0x06,
	Player8 = 0x07,
};

struct sGamepadButtonInputDesc
{
	friend class sInputController;
	struct GamepadButton
	{
		friend class sInputController;
		bool bIsEnabled = true;
		bool bHoldable = false;
		bool bUpState = false;

		std::function<void(eGamepadButtons, eGamepadPlayer)> fOnPressed = nullptr;
		std::function<void(eGamepadButtons, eGamepadPlayer)> fOnHeld = nullptr;
		std::function<void(eGamepadButtons, eGamepadPlayer)> fOnReleased = nullptr;
		std::function<void(eGamepadButtons, eGamepadPlayer)> fOnUp = nullptr;

		GamepadButton() = default;
		~GamepadButton() = default;
	};

	struct ThumbSticks
	{
		friend class sInputController;
		bool bIsEnabled = true;

		std::function<void(float, eGamepadButtons, eGamepadPlayer)> X = nullptr;
		std::function<void(float, eGamepadButtons, eGamepadPlayer)> Y = nullptr;
		std::function<void(FVector2, eGamepadButtons, eGamepadPlayer)> fOnXYAxis = nullptr;

		ThumbSticks() = default;
		~ThumbSticks() = default;
	};

	struct Triggers
	{
		friend class sInputController;
		bool bIsEnabled = true;
		bool bHoldable = false;
		bool bUpState = false;

		std::function<void(float, eGamepadButtons, eGamepadPlayer)> fOnPressed = nullptr;
		std::function<void(float, eGamepadButtons, eGamepadPlayer)> fOnHeld = nullptr;
		std::function<void(float, eGamepadButtons, eGamepadPlayer)> fOnReleased = nullptr;
		std::function<void(float, eGamepadButtons, eGamepadPlayer)> fOnUp = nullptr;

		Triggers() = default;
		~Triggers() = default;
	};

	sGamepadButtonInputDesc() = default;
	~sGamepadButtonInputDesc() = default;

	bool bIsEnabled = true;

	GamepadButton a;
	GamepadButton b;
	GamepadButton x;
	GamepadButton y;
	GamepadButton leftStick;
	GamepadButton rightStick;
	GamepadButton leftShoulder;
	GamepadButton rightShoulder;

	/*view | back*/
	GamepadButton view;
	/*menu | start*/
	GamepadButton menu;

	GamepadButton up;
	GamepadButton down;
	GamepadButton right;
	GamepadButton left;

	ThumbSticks LeftThumbStick;
	ThumbSticks RightThumbStick;

	Triggers LeftTrigger;
	Triggers RightTrigger;

	std::function<void(eGamepadPlayer)> OnConnected = nullptr;
	std::function<void(eGamepadPlayer)> OnDisconnected = nullptr;

private:
	DirectX::GamePad::ButtonStateTracker StateTracker;
	bool ConnectionStatus = false;
};

class sInputController : public std::enable_shared_from_this<sInputController>
{
	sBaseClassBody(sClassConstructor, sInputController)
private:
	enum class InputState
	{
		PRESSED,
		RELEASED,
	};

	struct sKMButtonInput
	{
		sKMButtonInput(sKMButtonInputDesc InDesc)
			: Desc(InDesc)
		{}
		~sKMButtonInput() = default;
		sKMButtonInputDesc Desc;
		InputState State = InputState::RELEASED;
	};

	struct sMouseAxisInput
	{
		bool bIsEnabled = true;

		std::function<void(float, FVector2)> fOnXYAxis = nullptr;
		std::function<void(float, float)> fOnXAxis = nullptr;
		std::function<void(float, float)> fOnYAxis = nullptr;
		std::function<void(float)> fOnWheel = nullptr;

		sMouseAxisInput() = default;
		~sMouseAxisInput()
		{
			fOnXYAxis = nullptr;
			fOnXAxis = nullptr;
			fOnYAxis = nullptr;
			fOnWheel = nullptr;
		}
	};

public:
	sInputController(void* InHWND);
	virtual ~sInputController();

	void BeginPlay();
	void Tick(const double DeltaTime);
	void FixedUpdate(const double DeltaTime);

	void Update();

	void BindInput(std::string Name, const sKMButtonInputDesc Desc);
	void RemoveInput(std::string Name);
	void BindInputToMouseXAxis(std::function<void(float, float)> fAxis);
	void BindInputToMouseYAxis(std::function<void(float, float)> fAxis);
	void BindInputToMouseXYAxis(std::function<void(float, FVector2)> fAxis);
	void BindInputToMouseWheel(std::function<void(float)> fAxis);

	inline void EnableKeyboard() { bIsKeyboardEnabled = true; }
	inline void DisableKeyboard() { bIsKeyboardEnabled = false; }
	inline void EnableMouse() { bIsMouseEnabled = true; }
	inline void DisableMouse() { bIsMouseEnabled = false; }
	inline void EnableGamepad() { bIsGamepadEnabled = true; }
	inline void DisableGamepad() { bIsGamepadEnabled = false; }
	inline void EnableMouseAndKeyboardUpdateOnTick() { bUpdateOnTick = true; }
	inline void DisableMouseAndKeyboardUpdateOnTick() { bUpdateOnTick = false; }

	inline FVector2 GetMouseLocation() const { return MouseLocation; }
	inline float GetMouseWheelDelta() const { return WheelDelta; }

	bool SetKeyState(std::string name, bool State);
	bool SetKeyHoldableState(std::string name, bool bHoldAble);
	void SetMouseAxisState(bool State);

	bool SetGamePadInputMap(eGamepadPlayer Player, sGamepadButtonInputDesc Input);
	bool RemoveGamePadInputMap(eGamepadPlayer Player);
	bool SetGamepadVibration(int player, float leftMotor, float rightMotor, float leftTrigger, float rightTrigger) noexcept;
	bool SetGamePadState(eGamepadPlayer Player, bool State);

	bool SetGamepadButtonState(eGamepadPlayer Player, eGamepadButtons Button, bool State);
	bool SetGamepadButtonHoldableState(eGamepadPlayer Player, eGamepadButtons Button, bool bHoldAble);
	bool SetGamepadButtonUpState(eGamepadPlayer Player, eGamepadButtons Button, bool bUpState);

	void InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar);

private:
	void* pHWND;
	std::map<std::string, sKMButtonInput> InputMap;
	sMouseAxisInput MouseAxisInput;
	FVector2 MouseLocation;
	float WheelDelta;

	std::map<eGamepadPlayer, sGamepadButtonInputDesc> GamePadInputMap;
	std::unique_ptr<DirectX::GamePad> pGamePad;

	bool bIsKeyboardEnabled;
	bool bIsMouseEnabled;
	bool bIsGamepadEnabled;
	bool bUpdateOnTick;
};
