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
		, IP("")
		, Port(0)
	{}
	std::string Name;

	bool Fullscreen;
	sScreenDimension Dimensions;

	std::string NetworkRole;
	std::string IP;
	std::uint16_t Port;
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
		/*
		bütün satýrlarý std::vector<std::string> containerýna koy
		sonra her satýrý for loop ile araþtýr
		[] -> her zaman baþlýk
		baþlýk bulununca baþlýk tipini çöz
		sonra alt satýra in 
		alt satýrý for loop ile harf harf çözümle
		hangi deðiþken olduðunu bul sonra "=" bul ve gameCFG.deðiþken set et
		*/

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
			if (line.find("Role") != -1)
				sin >> gameCFG.NetworkRole;
			if (line.find("IP") != -1)
				sin >> gameCFG.IP;
			if (line.find("Port") != -1)
				sin >> gameCFG.Port;
		}
	}

private:
	GameConfig gameCFG;

	void CreateGameConfig()
	{
		FileManager::fsCreateFile("..//Config//", "GameConfig.ini");

		std::wofstream ConfigFile;

		ConfigFile.open("..//Config//GameConfig.ini", std::ofstream::out | std::ofstream::app);

		ConfigFile << "\n";
		ConfigFile << "[Renderer]";
		ConfigFile << "\n";
		ConfigFile << "WindowWidth = 1366";
		ConfigFile << "\n";
		ConfigFile << "WindowHeight = 768";
		ConfigFile << "\n";
		ConfigFile << "Fullscreen = 0";
		ConfigFile << "\n";
		ConfigFile << "\n";
		ConfigFile << "[Network]";
		ConfigFile << "\n";
		ConfigFile << "# None, Host or Client";
		ConfigFile << "\n";
		ConfigFile << "Role = Host";
		ConfigFile << "\n";
		ConfigFile << "# default IP 127.0.0.1 (Local)";
		ConfigFile << "\n";
		ConfigFile << "IP = 127.0.0.1";
		ConfigFile << "\n";
		ConfigFile << "# default Port 27020";
		ConfigFile << "\n";
		ConfigFile << "Port = 27020";
		ConfigFile << "\n";

		ConfigFile.close();
	}
};
