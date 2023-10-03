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
