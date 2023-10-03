#pragma once

#include <memory>
#include "Engine/ClassBody.h"

class sActor;
class sPlayer;
class sController
{
	sBaseClassBody(sClassDefaultProtectedConstructor, sController)
public:
	virtual void Possess(sActor* Actor) = 0;
	virtual void UnPossess(sActor* Actor) = 0;
};
