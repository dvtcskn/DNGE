#pragma once

#include "Actor.h"

class sCharacter : public sActor
{
	sClassBody(sClassConstructor, sCharacter, sActor)
public:
	sCharacter(std::string InName = "", sController* InController = nullptr);

	virtual ~sCharacter();

private:
	//UCameraComponent* CameraComponent;
};