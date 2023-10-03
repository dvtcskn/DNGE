
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

	std::wstring wsTmp(str.begin(), str.end());

	return FileManager::GetLogsFolder() + L"Log_" + wsTmp + L".txt";
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