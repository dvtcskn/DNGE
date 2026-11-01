/* ---------------------------------------------------------------------------------------
* MIT License
*
* Copyright (c) 2023 Davut Coşkun.
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
#include "Engine/AbstractEngine.h"
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
		, RendererAPI("")
		, DeviceType("")
		, GPUIndex(0)
	{}
	std::string Name;

	bool Fullscreen;
	sScreenDimension Dimensions;

	std::string RendererAPI;
	std::string DeviceType;
	std::uint32_t GPUIndex;

	std::string NetworkRole;
	std::string IP;
	std::uint16_t Port;

	EGITypes GetRendererAPI() const
	{
		std::string TempRendererAPI = RendererAPI;
		std::transform(TempRendererAPI.begin(), TempRendererAPI.end(), TempRendererAPI.begin(), [](unsigned char c) {
			return std::tolower(c);
			});

		if (TempRendererAPI == "d3d11")
			return EGITypes::D3D11;
		else if (TempRendererAPI == "d3d12")
			return EGITypes::D3D12;
		else if (TempRendererAPI == "vulkan")
			return EGITypes::Vulkan;

		return EGITypes::D3D12;
	}

	EGPUDeviceType GetDeviceType() const
	{
		std::string TempDeviceType = DeviceType;
		std::transform(TempDeviceType.begin(), TempDeviceType.end(), TempDeviceType.begin(), [](unsigned char c) {
			return std::tolower(c);
			});

		if (TempDeviceType == "hardware")
			return EGPUDeviceType::Hardware;
		else if (TempDeviceType == "software")
			return EGPUDeviceType::Software;

		return EGPUDeviceType::Hardware;
	}
};

class ConfigManager
{
	sBaseClassBody(sClassNoDefaults, ConfigManager)
private:
	ConfigManager()
		: ConfigPath("..//Config//")
	{
		/*if (!FileManager::FileExists("..//Config//GameConfig.ini"))
		{
			CreateGameConfig();
		}*/
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

	std::string ConfigPath;

	bool IsGameConfigCreated()
	{
		return FileManager::FileExists(ConfigPath + "GameConfig.ini");
	}

	bool ReadGameConfig()
	{
		std::ifstream fin(ConfigPath + gameCFG.Name);
		if (fin.is_open())
		{
			std::string line;
			while (getline(fin, line))
			{
				std::istringstream sin(line.substr(line.find("=") + 1));
				if (line.find("API") != -1)
					sin >> gameCFG.RendererAPI;
				if (line.find("DeviceType") != -1)
					sin >> gameCFG.DeviceType;
				if (line.find("GPUIndex") != -1)
					sin >> gameCFG.GPUIndex;
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
			return true;
		}
		return false;
	}

	bool CreateGameConfig()
	{
		if (FileManager::fsCreateFile(ConfigPath, "GameConfig.ini"))
		{
			std::wofstream ConfigFile;

			ConfigFile.open(ConfigPath + "GameConfig.ini", std::ofstream::out | std::ofstream::app);

			ConfigFile << "\n";
			ConfigFile << "[Renderer]";
			ConfigFile << "\n";
			ConfigFile << "API = D3D12";
			ConfigFile << "\n";
			ConfigFile << "DeviceType = Hardware";
			ConfigFile << "\n";
			ConfigFile << "GPUIndex = 0";
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
			ConfigFile << "Role = None";
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

			return true;
		}
		return false;
	}

private:
	GameConfig gameCFG;
};
