#pragma once

#include <chrono>
#include <iostream>

struct Timer
{
	std::chrono::time_point<std::chrono::steady_clock> Start, End;
	std::chrono::duration<float> Duration;

	std::string FunctionName;

	Timer()
		: FunctionName("")
		, Duration(0.0f)
	{
		Start = std::chrono::high_resolution_clock::now();
	}
	Timer(const std::string& Str)
		: FunctionName(Str)
		, Duration(0.0f)
	{
		Start = std::chrono::high_resolution_clock::now();
	}

	~Timer()
	{
		End = std::chrono::high_resolution_clock::now();
		Duration = End - Start;

		float ms = Duration.count() * 1000.0f;
		std::cout << "(" << FunctionName << ") " << "Timer duration : " << ms << "ms" << std::endl;
	}
};

#define TimerProfiler Timer timer(__FUNCTION__);
