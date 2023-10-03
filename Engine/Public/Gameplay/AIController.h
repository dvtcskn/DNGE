#pragma once

#include "PlayerState.h"
#include "Character.h"
#include "Controller.h"
#include "Engine/ClassBody.h"

class sGameInstance;
class sAIController : public sController, public std::enable_shared_from_this<sAIController>
{
	sClassBody(sClassConstructor, sAIController, sController)
public:
	sAIController(sGameInstance* InOwner);
	virtual ~sAIController();

	void BeginPlay();
	void Tick(const double DeltaTime);
	void FixedUpdate(const double DeltaTime);

	sActor* GetPossessedActor(std::size_t index) const;
	template<typename T>
	inline T* GetPossessedActor(std::size_t index) const { return static_cast<T*>(PossessedActors.at(index)); }
	bool HasPossessedActor(sActor* Actor) const;
	virtual void Possess(sActor* Actor) override;
	virtual void UnPossess(sActor* Actor) override;

	inline std::size_t PossessedActorCount() const { return PossessedActors.size(); }
	inline const std::vector<sActor*>& GetPossessedActors() const { return PossessedActors; }

	inline sGameInstance* GetOwner() const { return Owner; }

private:
	virtual void OnPossess(sActor* Actor) {}
	virtual void OnUnPossess() {}

	virtual void OnBeginPlay() {}
	virtual void OnTick(const double DeltaTime) {}
	virtual void OnFixedUpdate(const double DeltaTime) {}

private:
	sGameInstance* Owner;
	std::vector<sActor*> PossessedActors;
};
