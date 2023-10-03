#pragma once

#include "FileManager.h"

namespace Utilities
{
	inline void WriteToConsole(const std::string& STR)
	{
		std::cout << STR << std::endl;
	}
	inline void WriteToConsole(const std::wstring& STR)
	{
		//std::cout << FileManager::WideStringToString(STR) << std::endl;
		std::wcout << STR.c_str() << std::endl;
	}
}
