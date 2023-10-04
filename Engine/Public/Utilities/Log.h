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

#include <string>
#include <fstream>
#include "Engine/ClassBody.h"

class Log
{
	sBaseClassBody(sClassNoDefaults, Log)
private:
	Log();
	Log(const Log& Other) = delete;
	Log& operator=(const Log&) = delete;
	
	mutable std::wofstream Logging;
	mutable bool opened;

public:
	static Log& Get()
	{
		static Log instance;
		return instance;
	}

	std::wstring CreateLogFile() const;
	bool Open() const;
	bool Close() const;

	bool WriteLog(const wchar_t* Text) const;
	bool WriteLog(std::wstring& Text) const;
	bool WriteLog(const std::string& Text) const;
	bool WriteLog(const char& Text) const;
	bool WriteLog(const char* Text) const;

	bool WriteLog(const wchar_t* Location, const wchar_t* Text) const;
	bool WriteLog(std::wstring& Location, std::wstring& Text) const;
	bool WriteLog(std::string& Location, std::string& Text) const;
	bool WriteLog(const char& Location, const char& Text) const;

	bool WriteLog(const wchar_t* Text, const int num) const;
	bool WriteLog(const wchar_t* Text, const float num) const;
	bool WriteLog(const wchar_t* Text, const double num) const;
	bool WriteLog(std::wstring& Text, int& num) const;
	bool WriteLog(std::string& Text, int& num) const;
	bool WriteLog(const char& Text, int& num) const;

	bool WriteLog(const std::string& Text, const char& Char) const;
};
