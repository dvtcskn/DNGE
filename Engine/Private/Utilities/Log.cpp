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

#include "pch.h"
#include "Utilities/Log.h"
#include "Utilities/FileManager.h"

Log::Log()
{
	opened = false;
}

std::wstring Log::CreateLogFile() const
{
	time_t t = std::time(nullptr);
	struct tm buf;
	errno_t m_tm = localtime_s(&buf, &t);

	std::ostringstream oss;
	oss << std::put_time(&buf, "%d-%m-%Y_%H-%M-%S");
	auto str = oss.str();

	std::string wsTmp(str.begin(), str.end());

	return FileManager::StringToWstring(FileManager::GetLogsFolder() + "Log_" + wsTmp + ".txt");
}

bool Log::Open() const
{
	if(!opened)
	{
		std::wstring filename = CreateLogFile();
		Logging.open(filename.c_str(), std::ofstream::out | std::ofstream::app);
		opened = Logging.is_open();
		if(!opened)
			return false;
		WriteLog(L"Log file opened.");
		return true;
	}
	return false;
}

bool Log::WriteLog(const wchar_t* Text) const
{
	if(opened) {
		Logging << Text;
		Logging << "\n";
		//Logging << L"---------------------------------";
		//Logging << "\n";
		Logging.flush();
		return true;
	}
	return false;
}


bool Log::WriteLog(std::wstring& Text) const
{
	if (opened) {
		Logging << Text;
		Logging << "\n";
		//Logging << L"---------------------------------";
		//Logging << "\n";
		Logging.flush();
		return true;
	}
	return false;
}

bool Log::WriteLog(const std::string & Text) const
{
	if (opened) {
		Logging << Text.c_str();
		Logging << "\n";
		//Logging << L"---------------------------------";
		//Logging << "\n";
		Logging.flush();
		return true;
	}
	return false;
}

bool Log::WriteLog(const std::string& Text, const char& Char) const
{
	if (opened) {
		Logging << Text.c_str();
		Logging << Char;
		Logging << "\n";
		//Logging << L"---------------------------------";
		//Logging << "\n";
		Logging.flush();
		return true;
	}
	return false;
}

bool Log::WriteLog(const char& Text) const
{
	if (opened) {
		Logging << Text;
		Logging << "\n";
		//Logging << L"---------------------------------";
		//Logging << "\n";
		Logging.flush();
		return true;
	}
	return false;
}

bool Log::WriteLog(const char * Text) const
{
	if (opened) {
		Logging << Text;
		Logging << "\n";
		Logging.flush();
		return true;
	}
	return false;
}

bool Log::WriteLog(const wchar_t * Location, const wchar_t * Text) const
{
	if (opened) {
		Logging << Location << " : " << Text;
		Logging << "\n";
		//Logging << L"---------------------------------";
		//Logging << "\n";
		Logging.flush();
		return true;
	}
	return false;
}

bool Log::WriteLog(std::wstring & Location, std::wstring & Text) const
{
	if (opened) {
		Logging << Location << " : " << Text;
		Logging << "\n";
		//Logging << L"---------------------------------";
		//Logging << "\n";
		Logging.flush();
		return true;
	}
	return false;
}

bool Log::WriteLog(std::string & Location, std::string & Text) const
{
	if (opened) {
		Logging << Location.c_str() << " : " << Text.c_str();
		Logging << "\n";
		//Logging << L"---------------------------------";
		//Logging << "\n";
		Logging.flush();
		return true;
	}
	return false;
}

bool Log::WriteLog(const char & Location, const char & Text) const
{
	if (opened) {
		Logging << Location << " : " << Text;
		Logging << "\n";
		//Logging << L"---------------------------------";
		//Logging << "\n";
		Logging.flush();
		return true;
	}
	return false;
}

bool Log::WriteLog(const wchar_t * Text, const int num) const
{
	if (opened) {
		Logging << Text << " : " << num;
		Logging << "\n";
		//Logging << L"---------------------------------";
		//Logging << "\n";
		Logging.flush();
		return true;
	}
	return false;
}

bool Log::WriteLog(const wchar_t * Text, const float num) const
{
	if (opened) {
		Logging << Text << " : " << num;
		Logging << "\n";
		//Logging << L"---------------------------------";
		//Logging << "\n";
		Logging.flush();
		return true;
	}
	return false;
}

bool Log::WriteLog(const wchar_t * Text, const double num) const
{
	if (opened) {
		Logging << Text << " : " << num;
		Logging << "\n";
		//Logging << L"---------------------------------";
		//Logging << "\n";
		Logging.flush();
		return true;
	}
	return false;
}

bool Log::WriteLog(std::wstring & Text, int & num) const
{
	if (opened) {
		Logging << Text << " : " << num;
		Logging << "\n";
		//Logging << L"---------------------------------";
		//Logging << "\n";
		Logging.flush();
		return true;
	}
	return false;
}

bool Log::WriteLog(std::string & Text, int & num) const
{
	if (opened) {
		Logging << Text.c_str() << " : " << num;
		Logging << "\n";
		//Logging << L"---------------------------------";
		//Logging << "\n";
		Logging.flush();
		return true;
	}
	return false;
}

bool Log::WriteLog(const char & Text, int & num) const
{
	if (opened) {
		Logging << Text << " : " << num;
		Logging << "\n";
		//Logging << L"---------------------------------";
		//Logging << "\n";
		Logging.flush();
		return true;
	}
	return false;
}

bool Log::Close() const
{
	WriteLog(L"Log file closed.");
	Logging.close();
	return true;
}