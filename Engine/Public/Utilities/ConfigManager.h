#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "Engine/ClassBody.h"
#include "Core/Math/CoreMath.h"
#include "FileManager.h"
#include "..//Engine/AbstractEngine.h"

struct GameConfig 
{
	GameConfig()
		: Name("GameConfig.ini")
		, Fullscreen(false)
		, Dimensions(sScreenDimension())
	{}
	std::string Name;

	bool Fullscreen;
	sScreenDimension Dimensions;
};

class ConfigManager
{
	sBaseClassBody(sClassNoDefaults, ConfigManager)
private:
	ConfigManager()
	{
		if (!FileManager::FileExists("..//Config//GameConfig.ini"))
		{
			CreateGameConfig();
		}
		ReadGameConfig();
	};

	ConfigManager(const ConfigManager& Other) = delete;
	ConfigManager& operator=(const ConfigManager&) = delete;

public:
	~ConfigManager()
	{
		Release();
	}

	void Release()
	{
	}

	static ConfigManager& Get()
	{
		static ConfigManager instance;
		return instance;
	}

public:
	GameConfig GetGameConfig() const { return gameCFG; };

	void ReadGameConfig()
	{
		std::ifstream fin("..//Config//" + gameCFG.Name);
		std::string line;
		while (getline(fin, line)) 
		{
			std::istringstream sin(line.substr(line.find("=") + 1));
			if (line.find("WindowWidth") != -1)
				sin >> gameCFG.Dimensions.Width;
			if (line.find("WindowHeight") != -1)
				sin >> gameCFG.Dimensions.Height;
			if (line.find("Fullscreen") != -1)
				sin >> gameCFG.Fullscreen;
		}
	}

private:
	GameConfig gameCFG;

	void CreateGameConfig()
	{
		FileManager::fsCreateFile("..//Config//", "GameConfig.ini");

		std::wofstream ConfigFile;

		ConfigFile.open("..//Config//GameConfig.ini", std::ofstream::out | std::ofstream::app);

		ConfigFile << "[GameName]";
		ConfigFile << "\n";
		ConfigFile << "GameName = EditorTest";
		ConfigFile << "\n";
		ConfigFile << "\n";
		ConfigFile << "[Renderer]";
		ConfigFile << "\n";
		ConfigFile << "WindowWidth = 1366";
		ConfigFile << "\n";
		ConfigFile << "WindowHeight = 768";
		ConfigFile << "\n";
		ConfigFile << "Fullscreen = 0";
		ConfigFile << "\n";

		ConfigFile.close();
	}
};
