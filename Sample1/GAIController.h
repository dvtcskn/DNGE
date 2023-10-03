#pragma once

#include <Gameplay/AIController.h>

class GAIController : public sAIController, public std::enable_shared_from_this<GAIController>
{
	sClassBody(sClassConstructor, GAIController, sAIController)
public:
	GAIController(sGameInstance* InOwner);
	virtual ~GAIController();

	virtual void OnBeginPlay() override;
	virtual void OnTick(const double DeltaTime) override;
	virtual void OnFixedUpdate(const double DeltaTime) override;

	virtual void OnPossess(sActor* Actor) override;
	virtual void OnUnPossess() override;
};
