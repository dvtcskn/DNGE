#pragma once

#include <memory>
#include <array>
#include <vector>
#include <string>
#include <iostream>

#include <windows.h>
#include <psapi.h>

namespace MemoryManager
{
	inline DWORDLONG GetTotalVirtualMem()
	{
		MEMORYSTATUSEX memInfo;
		memInfo.dwLength = sizeof(MEMORYSTATUSEX);
		GlobalMemoryStatusEx(&memInfo);
		return memInfo.ullTotalPageFile;
	};

	inline DWORDLONG GetVirtualMemUsed()
	{
		MEMORYSTATUSEX memInfo;
		memInfo.dwLength = sizeof(MEMORYSTATUSEX);
		GlobalMemoryStatusEx(&memInfo);
		DWORDLONG virtualMemUsed = memInfo.ullTotalPageFile - memInfo.ullAvailPageFile;
		return virtualMemUsed;
	};

	inline SIZE_T GetVirtualMemUsedByEngine()
	{
		PROCESS_MEMORY_COUNTERS_EX pmc;
		GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
		return pmc.PrivateUsage;
	};

	inline DWORDLONG GetTotalPhysicalMemory()
	{
		MEMORYSTATUSEX memInfo;
		memInfo.dwLength = sizeof(MEMORYSTATUSEX);
		GlobalMemoryStatusEx(&memInfo);
		return memInfo.ullTotalPhys;
	};

	inline DWORDLONG GetTotalPhysicalMemoryUsed()
	{
		MEMORYSTATUSEX memInfo;
		memInfo.dwLength = sizeof(MEMORYSTATUSEX);
		GlobalMemoryStatusEx(&memInfo);
		DWORDLONG physMemUsed = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
		return physMemUsed;
	};

	inline SIZE_T GetTotalPhysicalMemoryUsedByEngine()
	{
		PROCESS_MEMORY_COUNTERS_EX pmc;
		GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
		return pmc.WorkingSetSize;
	};
};
