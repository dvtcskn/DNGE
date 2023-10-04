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
