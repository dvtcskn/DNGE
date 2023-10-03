
#include "pch.h"
#include "GPlayerController.h"
#include "GPlayerState.h"
#include <Engine/AbstractEngine.h>
#include <Utilities/Utilities.h>
#include <Gameplay/BoxCollision2DComponent.h>
#include <Engine/InputController.h>
#include <Gameplay/GameInstance.h>
#include "GPlayer.h"

GPlayerController::GPlayerController(sPlayer* InOwner)
	: Super(InOwner, GPlayerState::Create(this))
{
	auto pCameraManager = GetCameraManager();
	pCameraManager->DisableInputMovement();
	pCameraManager->DisableVelocity();
	auto Dim = GPU::GetInternalBaseRenderResolution();
	pCameraManager->SetOrthographic(Dim.Width, Dim.Height);
}

GPlayerController::~GPlayerController()
{
}

//void PressedTest(int Char)
//{
//	std::cout << "Pressed" << std::endl;
//}
//
//void HoldTest(int Char)
//{
//	std::cout << "Hold" << std::endl;
//}
//
//void ReleasedTest(int Char)
//{
//	std::cout << "Released" << std::endl;
//}
//
//void MouseXY(float Val, FVector2 MouseLoc)
//{
//	if (Val != 0.0f)
//	std::cout << "Val : " << Val << "; " << MouseLoc.ToString() << std::endl;
//}
//
//void OnGamepadButtonPressed(eGamepadButtons Button, eGamepadPlayer Player)
//{
//	switch (Button)
//	{
//		case A:	std::cout << "A : OnPressed" << std::endl;; break;
//		case B:	std::cout << "B : OnPressed" << std::endl; break;
//		case X:	std::cout << "X : OnPressed" << std::endl; break;
//		case Y: std::cout << "Y : OnPressed" << std::endl; break;
//		case LeftStick:	std::cout << "LeftStick : OnPressed" << std::endl; break;
//		case RightStick: std::cout << "RightStick : OnPressed" << std::endl; break;
//		case LeftShoulder: std::cout << "LeftShoulder : OnPressed" << std::endl; break;
//		case RightShoulder: std::cout << "RightShoulder : OnPressed" << std::endl; break;
//		case View: std::cout << "View : OnPressed" << std::endl; break;
//		case Menu: std::cout << "Menu : OnPressed" << std::endl; break;
//		case Up: std::cout << "Up : OnPressed" << std::endl; break;
//		case Down: std::cout << "Down : OnPressed" << std::endl; break;
//		case Right: std::cout << "Right : OnPressed" << std::endl; break;
//		case Left: std::cout << "Left : OnPressed" << std::endl; break;
//		case LeftTrigger: std::cout << "LeftTrigger : OnPressed" << std::endl; break;
//		case RightTrigger:std::cout << "RightTrigger : OnPressed" << std::endl; break;
//	}
//}
//
//void OnGamepadButtonHeld(eGamepadButtons Button, eGamepadPlayer Player)
//{
//	switch (Button)
//	{
//		case A:	std::cout << "A : OnHeld" << std::endl;; break;
//		case B:	std::cout << "B : OnHeld" << std::endl; break;
//		case X:	std::cout << "X : OnHeld" << std::endl; break;
//		case Y: std::cout << "Y : OnHeld" << std::endl; break;
//		case LeftStick:	std::cout << "LeftStick : OnHeld" << std::endl; break;
//		case RightStick: std::cout << "RightStick : OnHeld" << std::endl; break;
//		case LeftShoulder: std::cout << "LeftShoulder : OnHeld" << std::endl; break;
//		case RightShoulder: std::cout << "RightShoulder : OnHeld" << std::endl; break;
//		case View: std::cout << "View : OnHeld" << std::endl; break;
//		case Menu: std::cout << "Menu : OnHeld" << std::endl; break;
//		case Up: std::cout << "Up : OnHeld" << std::endl; break;
//		case Down: std::cout << "Down : OnHeld" << std::endl; break;
//		case Right: std::cout << "Right : OnHeld" << std::endl; break;
//		case Left: std::cout << "Left : OnHeld" << std::endl; break;
//		case LeftTrigger: std::cout << "LeftTrigger : OnHeld" << std::endl; break;
//		case RightTrigger:std::cout << "RightTrigger : OnHeld" << std::endl; break;
//	}
//}
//
//void OnGamepadButtonReleased(eGamepadButtons Button, eGamepadPlayer Player)
//{
//	switch (Button)
//	{
//		case A:	std::cout << "A : OnReleased" << std::endl;; break;
//		case B:	std::cout << "B : OnReleased" << std::endl; break;
//		case X:	std::cout << "X : OnReleased" << std::endl; break;
//		case Y: std::cout << "Y : OnReleased" << std::endl; break;
//		case LeftStick:	std::cout << "LeftStick : OnReleased" << std::endl; break;
//		case RightStick: std::cout << "RightStick : OnReleased" << std::endl; break;
//		case LeftShoulder: std::cout << "LeftShoulder : OnReleased" << std::endl; break;
//		case RightShoulder: std::cout << "RightShoulder : OnReleased" << std::endl; break;
//		case View: std::cout << "View : OnReleased" << std::endl; break;
//		case Menu: std::cout << "Menu : OnReleased" << std::endl; break;
//		case Up: std::cout << "Up : OnReleased" << std::endl; break;
//		case Down: std::cout << "Down : OnReleased" << std::endl; break;
//		case Right: std::cout << "Right : OnReleased" << std::endl; break;
//		case Left: std::cout << "Left : OnReleased" << std::endl; break;
//		case LeftTrigger: std::cout << "LeftTrigger : OnReleased" << std::endl; break;
//		case RightTrigger:std::cout << "RightTrigger : OnReleased" << std::endl; break;
//	}
//}
//
//void OnGamepadTriggerPressed(float val, eGamepadButtons Button, eGamepadPlayer Player)
//{
//	switch (Button)
//	{
//	case LeftTrigger: std::cout << "LeftTrigger : OnPressed | Val : " << val << std::endl; break;
//	case RightTrigger:std::cout << "RightTrigger : OnPressed | Val : " << val << std::endl; break;
//	}
//}
//
//void OnGamepadTriggerHeld(float val, eGamepadButtons Button, eGamepadPlayer Player)
//{
//	switch (Button)
//	{
//	case LeftTrigger: std::cout << "LeftTrigger : OnHeld | Val : " << val << std::endl; break;
//	case RightTrigger:std::cout << "RightTrigger : OnHeld | Val : " << val << std::endl; break;
//	}
//}
//
//void OnGamepadTriggerReleased(float val, eGamepadButtons Button, eGamepadPlayer Player)
//{
//	switch (Button)
//	{
//	case LeftTrigger: std::cout << "LeftTrigger : OnReleased | Val : " << val << std::endl; break;
//	case RightTrigger:std::cout << "RightTrigger : OnReleased | Val : " << val << std::endl; break;
//	}
//}
//
//void OnGamepadThumbStickX(float X, eGamepadButtons Button, eGamepadPlayer Player)
//{
//	switch (Button)
//	{
//	case eGamepadButtons::LeftThumbSticks: std::cout << "LeftThumbSticks X : " << X << std::endl; break;
//	case eGamepadButtons::RightThumbSticks: std::cout << "RightThumbSticks X : " << X << std::endl; break;
//	}
//}
//
//void OnGamepadThumbStickY(float X, eGamepadButtons Button, eGamepadPlayer Player)
//{
//	switch (Button)
//	{
//	case eGamepadButtons::LeftThumbSticks: std::cout << "LeftThumbSticks Y : " << X << std::endl; break;
//	case eGamepadButtons::RightThumbSticks: std::cout << "RightThumbSticks Y : " << X << std::endl; break;
//	}
//}
//
//void OnGamepadThumbStickXY(FVector2 XY, eGamepadButtons Button, eGamepadPlayer Player)
//{
//	switch (Button)
//	{
//	case eGamepadButtons::LeftThumbSticks: std::cout << "LeftThumbSticks : " << XY.ToString() << std::endl; break;
//	case eGamepadButtons::RightThumbSticks: std::cout << "RightThumbSticks : " << XY.ToString() << std::endl; break;
//	}
//}
//
//void OnGamePadConnected(eGamepadPlayer Player)
//{
//	std::cout << "OnGamePadConnected" << std::endl;
//}
//
//void OnGamePadDisconnected(eGamepadPlayer Player)
//{
//	std::cout << "OnGamePadDisconnected" << std::endl;
//}

void GPlayerController::OnBeginPlay()
{
	//auto Input = GetInputController();
	//sKMButtonInputDesc InputController;
	//InputController.Key = 'Q';
	//InputController.Modifiers[eModifiers::LeftShift] = true;
	////InputController.Modifiers[eModifiers::LeftCTRL] = true;
	//InputController.bHoldable = true;
	//InputController.fOnPressed = std::bind(&PressedTest, std::placeholders::_1);
	//InputController.fOnHeld = std::bind(&HoldTest, std::placeholders::_1);
	//InputController.fOnReleased = std::bind(&ReleasedTest, std::placeholders::_1);
	//Input->BindInput("Test", InputController);
	////Input->BindInputToMouseXYAxis(std::bind(&MouseXY, std::placeholders::_1, std::placeholders::_2));
	////Input->DisableMouseAndKeyboardUpdateOnTick();

	//sGamepadButtonInputDesc GamepadButtonInputDesc;

	////GamepadButtonInputDesc.LeftThumbStick.X = std::bind(&OnGamepadThumbStickX, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	////GamepadButtonInputDesc.LeftThumbStick.Y = std::bind(&OnGamepadThumbStickY, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	////GamepadButtonInputDesc.LeftThumbStick.fOnXYAxis = std::bind(&OnGamepadThumbStickXY, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

	////GamepadButtonInputDesc.RightThumbStick.X = std::bind(&OnGamepadThumbStickX, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	////GamepadButtonInputDesc.RightThumbStick.Y = std::bind(&OnGamepadThumbStickY, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	////GamepadButtonInputDesc.RightThumbStick.fOnXYAxis = std::bind(&OnGamepadThumbStickXY, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

	//GamepadButtonInputDesc.a.fOnPressed = std::bind(&OnGamepadButtonPressed, std::placeholders::_1, std::placeholders::_2);
	//GamepadButtonInputDesc.a.fOnHeld = std::bind(&OnGamepadButtonHeld, std::placeholders::_1, std::placeholders::_2);
	//GamepadButtonInputDesc.a.fOnReleased = std::bind(&OnGamepadButtonReleased, std::placeholders::_1, std::placeholders::_2);
	//GamepadButtonInputDesc.a.bHoldable = true;

	//GamepadButtonInputDesc.b.fOnPressed = std::bind(&OnGamepadButtonPressed, std::placeholders::_1, std::placeholders::_2);
	//GamepadButtonInputDesc.b.fOnHeld = std::bind(&OnGamepadButtonHeld, std::placeholders::_1, std::placeholders::_2);
	//GamepadButtonInputDesc.b.fOnReleased = std::bind(&OnGamepadButtonReleased, std::placeholders::_1, std::placeholders::_2);

	//GamepadButtonInputDesc.y.fOnPressed = std::bind(&OnGamepadButtonPressed, std::placeholders::_1, std::placeholders::_2);
	//GamepadButtonInputDesc.y.fOnHeld = std::bind(&OnGamepadButtonHeld, std::placeholders::_1, std::placeholders::_2);
	//GamepadButtonInputDesc.y.fOnReleased = std::bind(&OnGamepadButtonReleased, std::placeholders::_1, std::placeholders::_2);
	//GamepadButtonInputDesc.y.bHoldable = true;

	//GamepadButtonInputDesc.x.fOnPressed = std::bind(&OnGamepadButtonPressed, std::placeholders::_1, std::placeholders::_2);
	//GamepadButtonInputDesc.x.fOnHeld = std::bind(&OnGamepadButtonHeld, std::placeholders::_1, std::placeholders::_2);
	//GamepadButtonInputDesc.x.fOnReleased = std::bind(&OnGamepadButtonReleased, std::placeholders::_1, std::placeholders::_2);

	//GamepadButtonInputDesc.rightShoulder.fOnPressed = std::bind(&OnGamepadButtonPressed, std::placeholders::_1, std::placeholders::_2);
	//GamepadButtonInputDesc.rightShoulder.fOnHeld = std::bind(&OnGamepadButtonHeld, std::placeholders::_1, std::placeholders::_2);
	//GamepadButtonInputDesc.rightShoulder.fOnReleased = std::bind(&OnGamepadButtonReleased, std::placeholders::_1, std::placeholders::_2);

	//GamepadButtonInputDesc.RightTrigger.fOnPressed = std::bind(&OnGamepadTriggerPressed, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	//GamepadButtonInputDesc.RightTrigger.fOnHeld = std::bind(&OnGamepadTriggerHeld, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	//GamepadButtonInputDesc.RightTrigger.fOnReleased = std::bind(&OnGamepadTriggerReleased, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	//GamepadButtonInputDesc.RightTrigger.bHoldable = true;

	//GamepadButtonInputDesc.leftShoulder.fOnPressed = std::bind(&OnGamepadButtonPressed, std::placeholders::_1, std::placeholders::_2);
	//GamepadButtonInputDesc.leftShoulder.fOnHeld = std::bind(&OnGamepadButtonHeld, std::placeholders::_1, std::placeholders::_2);
	//GamepadButtonInputDesc.leftShoulder.fOnReleased = std::bind(&OnGamepadButtonReleased, std::placeholders::_1, std::placeholders::_2);

	//GamepadButtonInputDesc.LeftTrigger.fOnPressed = std::bind(&OnGamepadTriggerPressed, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	//GamepadButtonInputDesc.LeftTrigger.fOnHeld = std::bind(&OnGamepadTriggerHeld, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	//GamepadButtonInputDesc.LeftTrigger.fOnReleased = std::bind(&OnGamepadTriggerReleased, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	//GamepadButtonInputDesc.LeftTrigger.bHoldable = true;

	//GamepadButtonInputDesc.view.fOnPressed = std::bind(&OnGamepadButtonPressed, std::placeholders::_1, std::placeholders::_2);
	//GamepadButtonInputDesc.view.fOnHeld = std::bind(&OnGamepadButtonHeld, std::placeholders::_1, std::placeholders::_2);
	//GamepadButtonInputDesc.view.fOnReleased = std::bind(&OnGamepadButtonReleased, std::placeholders::_1, std::placeholders::_2);

	//GamepadButtonInputDesc.menu.fOnPressed = std::bind(&OnGamepadButtonPressed, std::placeholders::_1, std::placeholders::_2);
	//GamepadButtonInputDesc.menu.fOnHeld = std::bind(&OnGamepadButtonHeld, std::placeholders::_1, std::placeholders::_2);
	//GamepadButtonInputDesc.menu.fOnReleased = std::bind(&OnGamepadButtonReleased, std::placeholders::_1, std::placeholders::_2);

	//GamepadButtonInputDesc.leftStick.fOnPressed = std::bind(&OnGamepadButtonPressed, std::placeholders::_1, std::placeholders::_2);
	//GamepadButtonInputDesc.leftStick.fOnHeld = std::bind(&OnGamepadButtonHeld, std::placeholders::_1, std::placeholders::_2);
	//GamepadButtonInputDesc.leftStick.fOnReleased = std::bind(&OnGamepadButtonReleased, std::placeholders::_1, std::placeholders::_2);

	//GamepadButtonInputDesc.rightStick.fOnPressed = std::bind(&OnGamepadButtonPressed, std::placeholders::_1, std::placeholders::_2);
	//GamepadButtonInputDesc.rightStick.fOnHeld = std::bind(&OnGamepadButtonHeld, std::placeholders::_1, std::placeholders::_2);
	//GamepadButtonInputDesc.rightStick.fOnReleased = std::bind(&OnGamepadButtonReleased, std::placeholders::_1, std::placeholders::_2);

	//GamepadButtonInputDesc.up.fOnPressed = std::bind(&OnGamepadButtonPressed, std::placeholders::_1, std::placeholders::_2);
	//GamepadButtonInputDesc.up.fOnHeld = std::bind(&OnGamepadButtonHeld, std::placeholders::_1, std::placeholders::_2);
	//GamepadButtonInputDesc.up.fOnReleased = std::bind(&OnGamepadButtonReleased, std::placeholders::_1, std::placeholders::_2);

	//GamepadButtonInputDesc.down.fOnPressed = std::bind(&OnGamepadButtonPressed, std::placeholders::_1, std::placeholders::_2);
	//GamepadButtonInputDesc.down.fOnHeld = std::bind(&OnGamepadButtonHeld, std::placeholders::_1, std::placeholders::_2);
	//GamepadButtonInputDesc.down.fOnReleased = std::bind(&OnGamepadButtonReleased, std::placeholders::_1, std::placeholders::_2);

	//GamepadButtonInputDesc.left.fOnPressed = std::bind(&OnGamepadButtonPressed, std::placeholders::_1, std::placeholders::_2);
	//GamepadButtonInputDesc.left.fOnHeld = std::bind(&OnGamepadButtonHeld, std::placeholders::_1, std::placeholders::_2);
	//GamepadButtonInputDesc.left.fOnReleased = std::bind(&OnGamepadButtonReleased, std::placeholders::_1, std::placeholders::_2);

	//GamepadButtonInputDesc.right.fOnPressed = std::bind(&OnGamepadButtonPressed, std::placeholders::_1, std::placeholders::_2);
	//GamepadButtonInputDesc.right.fOnHeld = std::bind(&OnGamepadButtonHeld, std::placeholders::_1, std::placeholders::_2);
	//GamepadButtonInputDesc.right.fOnReleased = std::bind(&OnGamepadButtonReleased, std::placeholders::_1, std::placeholders::_2);

	//GamepadButtonInputDesc.OnConnected = std::bind(&OnGamePadConnected, std::placeholders::_1);
	//GamepadButtonInputDesc.OnDisconnected = std::bind(&OnGamePadDisconnected, std::placeholders::_1);

	//Input->SetGamePadInputMap(eGamepadPlayer::Player1, GamepadButtonInputDesc);
}

void GPlayerController::OnTick(const double DeltaTime)
{
}

void GPlayerController::Save()
{
	Super::Save();
}

void GPlayerController::Load()
{
	Super::Load();
}

void GPlayerController::OnPossess(sActor* Actor)
{
	auto pCameraManager = GetCameraManager();
	pCameraManager->DetachActorCamera();
	pCameraManager->DetachFromActor();
}

void GPlayerController::OnUnPossess()
{

}

void GPlayerController::InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar)
{
	Super::InputProcess(MouseInput, KeyboardChar);
}

void GPlayerController::OnCharacterDead()
{
	GetPlayer<GPlayer>()->OnCharacterDead();
}
