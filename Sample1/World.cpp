
#include "pch.h"
#include "World.h"
#include "MetaWorld.h"

sWorld::sWorld(MetaWorld* pOwner, std::string InName)
	: Owner(pOwner)
	, Name(InName)
	, ActiveLevelIndex(0)
	, bPauseActiveLevel(false)
{
	AddLevel(sDefaultLevel::Create(this));
}

sWorld::~sWorld()
{
	for (auto& Level : Levels)
		Level = nullptr;
	Levels.clear();
	Owner = nullptr;
}

std::string sWorld::GetName() const
{
	return Name;
}

void sWorld::InitWorld()
{
	for (const auto& Level : Levels)
		Level->InitLevel();
}

void sWorld::BeginPlay()
{
	for (const auto& Level : Levels)
		Level->BeginPlay();
}

void sWorld::Tick(const double DeltaTime)
{
	//for (const auto& Level : Levels)
	//	Level->Tick(DeltaTime);
	auto ActiveLevel = GetActiveLevel();
	if (ActiveLevel && !bPauseActiveLevel)
		ActiveLevel->Tick(DeltaTime);
}

void sWorld::FixedUpdate(const double DeltaTime)
{
	auto ActiveLevel = GetActiveLevel();
	if (ActiveLevel && !bPauseActiveLevel)
		ActiveLevel->FixedUpdate(DeltaTime);
}

void sWorld::Serialize()
{
}

IMetaWorld* sWorld::GetMetaWorld() const
{
	return Owner;
}

void sWorld::AddLevel(const ILevel::SharedPtr& Level)
{
	if (std::find(Levels.begin(), Levels.end(), Level) == Levels.end())
		Levels.push_back(Level);
}

void sWorld::SetActiveLevel(const std::size_t index)
{
	if (index < LevelCount())
		ActiveLevelIndex = index;
}

void sWorld::SetActiveLevel(const std::string& Name)
{
	for (std::size_t i = 0; i < Levels.size(); i++)
	{
		if (Levels[i]->GetName() == Name)
		{
			ActiveLevelIndex = i;
			break;
		}
	}
}

std::vector<ILevel*> sWorld::GetAllLevels() const
{
	std::vector<ILevel*> Result;
	Result.reserve(Levels.size());
	std::transform(Levels.cbegin(), Levels.cend(), std::back_inserter(Result), [](auto& ptr) { return ptr.get(); });
	return Result;
}

ILevel* sWorld::GetLevel(const std::size_t Index) const
{
	if (Index < LevelCount())
		return Levels.at(Index).get();
	return nullptr;
}

ILevel* sWorld::GetLevel(const std::string& Name) const
{
	for (const auto& Level : Levels)
	{
		if (Level->GetName() == Name)
			return Level.get();
	}
	return nullptr;
}

ILevel* sWorld::GetActiveLevel() const
{
	if (ActiveLevelIndex < LevelCount())
		return Levels.at(ActiveLevelIndex).get();
	return nullptr;
}

void sWorld::RemoveLevel(const std::size_t index)
{
	if (index < LevelCount())
		Levels.erase(Levels.begin() + index);
}

void sWorld::RemoveLevel(const std::string& Name)
{
	Levels.erase(std::find_if(Levels.begin(), Levels.end(), [Name](const ILevel::SharedPtr& Level)
		{
			return Name == Level->GetName();
		}));
}

sGameInstance* sWorld::GetGameInstance() const
{
	return Owner->GetGameInstance();
}

sActor* sWorld::GetPlayerFocusedActor(std::size_t Index) const
{
	return Owner->GetGameInstance()->GetPlayerFocusedActor(Index);
}

void sWorld::PauseActiveLevel(bool Pause)
{
	bPauseActiveLevel = Pause;
}

void sWorld::OnResizeWindow(const std::size_t Width, const std::size_t Height)
{
	auto ActiveLevel = GetActiveLevel();
	if (ActiveLevel)
		ActiveLevel->OnResizeWindow(Width, Height);
}

void sWorld::InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar)
{
	auto ActiveLevel = GetActiveLevel();
	if (ActiveLevel && !bPauseActiveLevel)
		ActiveLevel->InputProcess(MouseInput, KeyboardChar);
}
