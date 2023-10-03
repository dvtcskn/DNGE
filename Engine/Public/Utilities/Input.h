#pragma once

#include <map>
#include "Core/Math/CoreMath.h"

struct GKeyboardChar
{
	bool bIsIdle = true;
	bool bIsPressed = false;
	bool bIsChar = false;
	std::uint64_t KeyCode = 0;
};

enum class EMouseButtonState
{
	eIdle,
	eReleased,
	ePressed,
	eDisabled,
};

enum class EMouseState
{
	eIdle,
	eMoving,
	eScroll,
	eDisabled,
};

struct GMouseInput
{
	std::map<std::string, EMouseButtonState> Buttons;
	EMouseState State;
	FVector2 MouseLocation;
	short WheelDelta;

	GMouseInput()
		: State(EMouseState::eIdle)
		, MouseLocation(FVector2::Zero())
		, WheelDelta(0)
	{
		Buttons.insert({ "Left", EMouseButtonState::eIdle });
		Buttons.insert({ "Right", EMouseButtonState::eIdle });
		Buttons.insert({ "Middle", EMouseButtonState::eIdle });
		Buttons.insert({ "XB1", EMouseButtonState::eDisabled });
		Buttons.insert({ "XB2", EMouseButtonState::eDisabled });
	}
};
