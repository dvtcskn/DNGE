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
#include "Utilities/FileManager.h"
#include <filesystem>
#include <regex>
#include <fstream>
#include <sstream>
#include <Windows.h>
#include <cderr.h>
#include <shellapi.h>
#include <wrl/wrappers/corewrappers.h>

using namespace std::filesystem;

namespace FileManager
{
	std::wstring GetContentDirectory()
	{
		return L"..//Content//";
	}

	std::wstring GetAssetDirectory()
	{
		return GetContentDirectory() + L"Assets//";
	}

	std::wstring GetMeshAssetDirectory()
	{
		return GetAssetDirectory() + L"Meshes//";
	}

	std::wstring GetMaterialAssetDirectory()
	{
		return GetAssetDirectory() + L"Materials//";
	}

	std::wstring GetMapAssetDirectory()
	{
		return GetAssetDirectory() + L"Levels//";
	}

	std::wstring GetLogsFolder()
	{
		return GetContentDirectory() + L"Logs//";
	}

	std::string Normalize(const std::string& filePath)
	{
		std::string output = std::string(filePath.begin(), filePath.end());
		NormalizeInline(output);
		return output;
	}

	void NormalizeInline(std::string& filePath)
	{
		for (char& c : filePath)
		{
			if (c == '\\')
			{
				c = '/';
			}
		}
		if (filePath.find("./") == 0)
		{
			filePath = std::string(filePath.begin() + 2, filePath.end());
		}
	}

	bool ResolveRelativePaths(std::string& path)
	{
		for (;;)
		{
			size_t index = path.rfind("../");
			if (index == std::string::npos)
				break;
			size_t idx0 = path.rfind('/', index);
			if (idx0 == std::string::npos)
				return false;
			idx0 = path.rfind('/', idx0 - 1);
			if (idx0 != std::string::npos)
				path = path.substr(0, idx0 + 1) + path.substr(index + 3);
		}
		return true;
	}

	bool FileExists(const std::wstring& file)
	{
		// Exist ?
		DWORD dwAttrib = GetFileAttributes(file.c_str());
		return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
	}

	bool FileIsNewer(const std::wstring& file1, const std::wstring& file2)
	{
		HANDLE handle1 = INVALID_HANDLE_VALUE;
		HANDLE handle2 = INVALID_HANDLE_VALUE;

		handle1 = CreateFile(file1.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		handle2 = CreateFile(file2.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

		FILETIME fileTime1;
		FILETIME fileTime2;

		GetFileTime(handle1, nullptr, nullptr, &fileTime1);
		GetFileTime(handle2, nullptr, nullptr, &fileTime2);

		CloseHandle(handle1);
		CloseHandle(handle2);

		if (fileTime1.dwHighDateTime > fileTime2.dwHighDateTime)
		{
			return true;
		}
		else if (fileTime1.dwHighDateTime > fileTime2.dwHighDateTime) {
			return (fileTime1.dwLowDateTime > fileTime2.dwLowDateTime);
		}

		return false;
	}

	bool fsCreateFile(const std::string& InPath, const std::string InName)
	{
		std::ofstream myfile;
		myfile.open(InPath + InName);
		myfile.close();

		return true;
	}

	bool CreateDirectory_(const std::string& path)
	{
		try
		{
			return create_directories(path);
		}
		catch (filesystem_error& e)
		{
			UNREFERENCED_PARAMETER(e);
			return true;
		}
	}

	bool DeleteDirectory(const std::string& directory)
	{
		try
		{
			return remove_all(directory);
		}
		catch (filesystem_error& e)
		{
			UNREFERENCED_PARAMETER(e);
			return true;
		}
	}

	bool DirectoryExists(const std::string& directory)
	{
		try
		{
			return exists(directory);
		}
		catch (filesystem_error& e)
		{
			UNREFERENCED_PARAMETER(e);
			return true;
		}
	}

	bool IsDirectory(const std::string& directory)
	{
		try
		{
			return is_directory(directory);
		}
		catch (filesystem_error& e)
		{
			UNREFERENCED_PARAMETER(e);
			return false;
		}
	}

	void OpenDirectoryWindow(const std::string& directory)
	{
		ShellExecute(nullptr, nullptr, StringToWstring(directory).c_str(), nullptr, nullptr, SW_SHOW);
	}

	bool OpenFile(OPENFILENAMEW& FILE)
	{
		if (GetOpenFileName(&FILE))
		{
			return true;
		}
		else
		{
			// All this stuff below is to tell you exactly how you messed up above. 
			// Once you've got that fixed, you can often (not always!) reduce it to a 'user cancelled' assumption.
			switch (CommDlgExtendedError())
			{
			case CDERR_DIALOGFAILURE: std::cout << "CDERR_DIALOGFAILURE\n";   break;
			case CDERR_FINDRESFAILURE: std::cout << "CDERR_FINDRESFAILURE\n";  break;
			case CDERR_INITIALIZATION: std::cout << "CDERR_INITIALIZATION\n";  break;
			case CDERR_LOADRESFAILURE: std::cout << "CDERR_LOADRESFAILURE\n";  break;
			case CDERR_LOADSTRFAILURE: std::cout << "CDERR_LOADSTRFAILURE\n";  break;
			case CDERR_LOCKRESFAILURE: std::cout << "CDERR_LOCKRESFAILURE\n";  break;
			case CDERR_MEMALLOCFAILURE: std::cout << "CDERR_MEMALLOCFAILURE\n"; break;
			case CDERR_MEMLOCKFAILURE: std::cout << "CDERR_MEMLOCKFAILURE\n";  break;
			case CDERR_NOHINSTANCE: std::cout << "CDERR_NOHINSTANCE\n";     break;
			case CDERR_NOHOOK: std::cout << "CDERR_NOHOOK\n";          break;
			case CDERR_NOTEMPLATE: std::cout << "CDERR_NOTEMPLATE\n";      break;
			case CDERR_STRUCTSIZE: std::cout << "CDERR_STRUCTSIZE\n";      break;
			case FNERR_BUFFERTOOSMALL: std::cout << "FNERR_BUFFERTOOSMALL\n";  break;
			case FNERR_INVALIDFILENAME: std::cout << "FNERR_INVALIDFILENAME\n"; break;
			case FNERR_SUBCLASSFAILURE: std::cout << "FNERR_SUBCLASSFAILURE\n"; break;
			default: std::cout << "You cancelled.\n";
			}
		}
		return false;
	}

	bool FileExists(const std::string& file_path)
	{
		try
		{
			return exists(file_path);
		}
		catch (filesystem_error& e)
		{
			UNREFERENCED_PARAMETER(e);
			return true;
		}
	}

	bool DeleteFile_(const std::string& file_path)
	{
		// If this is a directory path, return
		if (is_directory(file_path))
			return false;

		try
		{
			return remove(file_path.c_str()) == 0;
		}
		catch (filesystem_error& e)
		{
			UNREFERENCED_PARAMETER(e);
			return true;
		}
	}

	bool CopyFileFromTo(const std::string& source, const std::string& destination)
	{
		if (source == destination)
			return true;

		// In case the destination path doesn't exist, create it
		if (!DirectoryExists(GetDirectoryFromFilePath(destination)))
		{
			CreateDirectory_(GetDirectoryFromFilePath(destination));
		}

		try
		{
			return copy_file(source, destination, copy_options::overwrite_existing);
		}
		catch (filesystem_error& e)
		{
			UNREFERENCED_PARAMETER(e);
			return true;
		}
	}

	std::string GetFileNameFromFilePath(const std::string& path)
	{
		auto lastindex = path.find_last_of("\\/");
		auto fileName = path.substr(lastindex + 1, path.length());

		return fileName;
	}

	std::string GetFileNameNoExtensionFromFilePath(const std::string& filepath)
	{
		auto fileName = GetFileNameFromFilePath(filepath);
		auto lastindex = fileName.find_last_of('.');
		auto fileNameNoExt = fileName.substr(0, lastindex);

		return fileNameNoExt;
	}

	std::string GetDirectoryFromFilePath(const std::string& filePath)
	{
		auto lastindex = filePath.find_last_of("\\/");
		auto directory = filePath.substr(0, lastindex + 1);

		return directory;
	}

	std::string GetFilePathWithoutExtension(const std::string& filePath)
	{
		auto directory = GetDirectoryFromFilePath(filePath);
		auto fileNameNoExt = GetFileNameNoExtensionFromFilePath(filePath);

		return directory + fileNameNoExt;
	}

	std::string GetExtensionFromFilePath(const std::string& filePath)
	{
		if (filePath.empty() || filePath == "")
			return "";

		auto lastindex = filePath.find_last_of('.');
		if (std::string::npos != lastindex)
		{
			// extension with dot included
			return filePath.substr(lastindex, filePath.length());
		}

		return "";
	}

	std::vector<std::string> GetDirectoriesInDirectory(const std::string& directory)
	{
		std::vector<std::string> subDirs;
		directory_iterator end_itr; // default construction yields past-the-end
		for (directory_iterator itr(directory); itr != end_itr; ++itr)
		{
			if (!is_directory(itr->status()))
				continue;

			subDirs.emplace_back(itr->path().generic_string());
		}

		return subDirs;
	}

	std::vector<std::string> GetFilesInDirectory(const std::string& directory)
	{
		std::vector<std::string> filePaths;
		directory_iterator end_itr; // default construction yields past-the-end
		for (directory_iterator itr(directory); itr != end_itr; ++itr)
		{
			if (!is_regular_file(itr->status()))
				continue;

			filePaths.emplace_back(itr->path().generic_string());
		}

		return filePaths;
	}

	std::vector<std::string> GetSupportedFilesInDirectory(const std::string& directory)
	{
		std::vector<std::string> filesInDirectory = GetFilesInDirectory(directory);
		std::vector<std::string> imagesInDirectory = GetSupportedImageFilesFromPaths(filesInDirectory); // get all the images
		std::vector<std::string> modelsInDirectory = GetSupportedModelFilesFromPaths(filesInDirectory); // get all the models
		std::vector<std::string> supportedFiles;

		// get supported images
		for (const auto& imageInDirectory : imagesInDirectory)
		{
			supportedFiles.emplace_back(imageInDirectory);
		}

		// get supported models
		for (const auto& modelInDirectory : modelsInDirectory)
		{
			supportedFiles.emplace_back(modelInDirectory);
		}

		return supportedFiles;
	}

	std::vector<std::string> GetSupportedImageFilesFromPaths(const std::vector<std::string>& paths)
	{
		std::vector<std::string> imageFiles;
		for (const auto& path : paths)
		{
			if (!IsTextureFile(path))
				continue;

			imageFiles.emplace_back(path);
		}

		return imageFiles;
	}

	std::vector<std::string> GetSupportedAudioFilesFromPaths(const std::vector<std::string>& paths)
	{
		std::vector<std::string> audioFiles;
		for (const auto& path : paths)
		{
			if (!IsAudioFile(path))
				continue;

			audioFiles.emplace_back(path);
		}

		return audioFiles;
	}

	std::vector<std::string> GetSupportedModelFilesFromPaths(const std::vector<std::string>& paths)
	{
		std::vector<std::string> images;
		for (const auto& path : paths)
		{
			if (!IsMeshFile(path))
				continue;

			images.emplace_back(path);
		}

		return images;
	}

	std::vector<std::string> GetSupportedModelFilesInDirectory(const std::string& directory)
	{
		return GetSupportedModelFilesFromPaths(GetFilesInDirectory(directory));
	}

	std::vector<std::string> GetSupportedMapFilesInDirectory(const std::string& directory)
	{
		std::vector<std::string> sceneFiles;

		auto files = GetFilesInDirectory(directory);
		for (const auto& file : files)
		{
			if (!IsMapFile(file))
				continue;

			sceneFiles.emplace_back(file);
		}

		return sceneFiles;
	}

	// Returns a file path which is relative to the engine's executable
	std::string GetRelativeFilePath(const std::string& absoluteFilePath)
	{
		// create absolute paths
		path p = absolute(absoluteFilePath);
		path r = absolute(GetWorkingDirectory());

		// if root paths are different, return absolute path
		if (p.root_path() != r.root_path())
			return p.generic_string();

		// initialize relative path
		path result;

		// find out where the two paths diverge
		path::const_iterator itr_path = p.begin();
		path::const_iterator itr_relative_to = r.begin();
		while (*itr_path == *itr_relative_to && itr_path != p.end() && itr_relative_to != r.end())
		{
			++itr_path;
			++itr_relative_to;
		}

		// add "../" for each remaining token in relative_to
		if (itr_relative_to != r.end())
		{
			++itr_relative_to;
			while (itr_relative_to != r.end())
			{
				result /= "..";
				++itr_relative_to;
			}
		}

		// add remaining path
		while (itr_path != p.end())
		{
			result /= *itr_path;
			++itr_path;
		}

		return result.generic_string();
	}

	// Returns a file path which is where the engine's executable is located
	std::string GetWorkingDirectory()
	{
		return current_path().generic_string() + "/";
	}

	std::string GetParentDirectory(const std::string& directory)
	{
		auto found = directory.find_last_of("/\\");
		auto result = directory;

		// If no slash was found, return provided std::string
		if (found == std::string::npos)
			return directory;

		// If the slash was find at the last position, remove it and try again
		if (found == directory.length() - 1)
		{
			result = result.substr(0, found - 1);
			return GetParentDirectory(result);
		}

		// Return parent directory including a slash at the end
		return result.substr(0, found) + "/";
	}

	bool IsMaterialFile(const std::string& filePath)
	{
		return false;
	}

	bool IsMeshFile(const std::string& filePath)
	{
		return false;
	}

	bool IsAudioFile(const std::string& filePath)
	{
		return false;
	}

	bool IsMapFile(const std::string& filePath)
	{
		return false;
	}

	bool IsTextureFile(const std::string& filePath)
	{
		return false;
	}

	bool IsShaderFile(const std::string& filePath)
	{
		return false;
	}

	std::string GetStringAfterExpression(const std::string& str, const std::string& expression)
	{
		// ("The quick brown fox", "brown") -> "brown fox"
		auto position = str.find(expression);
		auto remaining = position != std::string::npos ? str.substr(position + expression.length()) : str;

		return remaining;
	}

	std::string GetStringBetweenExpressions(const std::string& str, const std::string& firstExpression, const std::string& secondExpression)
	{
		// ("The quick brown fox", "The ", " brown") -> "quick"

		std::regex base_regex(firstExpression + "(.*)" + secondExpression);

		std::smatch base_match;
		if (regex_search(str, base_match, base_regex))
		{
			// The first sub_match is the whole std::string; the next
			// sub_match is the first parenthesized expression.
			if (base_match.size() == 2)
			{
				return base_match[1].str();
			}
		}

		return str;
	}

	std::string ConvertToUppercase(const std::string& lower)
	{
		std::locale loc;
		std::string upper;
		for (const auto& character : lower)
		{
			upper += std::toupper(character, loc);
		}

		return upper;
	}

	std::string ReplaceExpression(const std::string& str, const std::string& from, const std::string& to)
	{
		return regex_replace(str, std::regex(from), to);
	}

	std::wstring StringToWstring(const std::string& str)
	{
		const auto slength = static_cast<int>(str.length()) + 1;
		const auto len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, nullptr, 0);
		const auto buf = new wchar_t[len];
		MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, buf, len);
		std::wstring result(buf);
		delete[] buf;
		return result;
	}

	std::string ResolveIncludeDirectives(const std::string& source, const std::string& directory)
	{
		std::string directive_exp = "#include \"";

		// Early exit if there is no include directive
		if (source.find(directive_exp) == std::string::npos)
			return source;

		auto load_include_directive = [&](const std::string& include_directive)
		{
			std::string file_name = GetStringBetweenExpressions(include_directive, directive_exp, "\"");
			std::ifstream t(directory + file_name);
			return std::string((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
		};

		// Scan for include directives
		std::vector<std::string> include_directives;
		std::istringstream stream(source); std::string line;
		while (std::getline(stream, line))
		{
			if (line.find(directive_exp) != std::string::npos)
				include_directives.emplace_back(line);
		}

		// Replace include directives with loaded file sources
		std::string result = source;
		for (const auto& include_directive : include_directives)
		{
			auto source = load_include_directive(include_directive);
			result = regex_replace(result, std::regex(include_directive), source, std::regex_constants::format_first_only);
		}

		// If there are still more include directives, resolve them too
		if (source.find(directive_exp) != std::string::npos)
		{
			result = ResolveIncludeDirectives(result, directory);
		}

		// At this point, everything should be resolved
		return result;
	}

	HRESULT ReadDataFromFile(LPCWSTR filename, byte* data, UINT* size)
	{
		using namespace Microsoft::WRL;

#if WINVER >= _WIN32_WINNT_WIN8
		CREATEFILE2_EXTENDED_PARAMETERS extendedParams = {};
		extendedParams.dwSize = sizeof(CREATEFILE2_EXTENDED_PARAMETERS);
		extendedParams.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
		extendedParams.dwFileFlags = FILE_FLAG_SEQUENTIAL_SCAN;
		extendedParams.dwSecurityQosFlags = SECURITY_ANONYMOUS;
		extendedParams.lpSecurityAttributes = nullptr;
		extendedParams.hTemplateFile = nullptr;

		Wrappers::FileHandle file(CreateFile2(filename, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, &extendedParams));
#else
		Wrappers::FileHandle file(CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | SECURITY_SQOS_PRESENT | SECURITY_ANONYMOUS, nullptr));
#endif
		if (file.Get() == INVALID_HANDLE_VALUE)
		{
			throw std::exception();
		}

		FILE_STANDARD_INFO fileInfo = {};
		if (!GetFileInformationByHandleEx(file.Get(), FileStandardInfo, &fileInfo, sizeof(fileInfo)))
		{
			throw std::exception();
		}

		if (fileInfo.EndOfFile.HighPart != 0)
		{
			throw std::exception();
		}

		data = reinterpret_cast<byte*>(malloc(fileInfo.EndOfFile.LowPart));
		*size = fileInfo.EndOfFile.LowPart;

		if (!ReadFile(file.Get(), data, fileInfo.EndOfFile.LowPart, nullptr, nullptr))
		{
			throw std::exception();
		}

		return S_OK;
	}

	HRESULT ReadDataFromDDSFile(LPCWSTR filename, byte* data, UINT* offset, UINT* size)
	{
		if (FAILED(ReadDataFromFile(filename, data, size)))
		{
			return E_FAIL;
		}

		// DDS files always start with the same magic number.
		static const UINT DDS_MAGIC = 0x20534444;
		UINT magicNumber = *reinterpret_cast<const UINT*>(*data);
		if (magicNumber != DDS_MAGIC)
		{
			return E_FAIL;
		}

		struct DDS_PIXELFORMAT
		{
			UINT size;
			UINT flags;
			UINT fourCC;
			UINT rgbBitCount;
			UINT rBitMask;
			UINT gBitMask;
			UINT bBitMask;
			UINT aBitMask;
		};

		struct DDS_HEADER
		{
			UINT size;
			UINT flags;
			UINT height;
			UINT width;
			UINT pitchOrLinearSize;
			UINT depth;
			UINT mipMapCount;
			UINT reserved1[11];
			DDS_PIXELFORMAT ddsPixelFormat;
			UINT caps;
			UINT caps2;
			UINT caps3;
			UINT caps4;
			UINT reserved2;
		};

		auto ddsHeader = reinterpret_cast<const DDS_HEADER*>(*data + sizeof(UINT));
		if (ddsHeader->size != sizeof(DDS_HEADER) || ddsHeader->ddsPixelFormat.size != sizeof(DDS_PIXELFORMAT))
		{
			return E_FAIL;
		}

		const ptrdiff_t ddsDataOffset = sizeof(UINT) + sizeof(DDS_HEADER);
		*offset = ddsDataOffset;
		*size = *size - ddsDataOffset;

		return S_OK;
	}

	void GetAssetsPath(_Out_writes_(pathSize) WCHAR* path, UINT pathSize)
	{
		if (path == nullptr)
		{
			throw std::exception();
		}

		DWORD size = GetModuleFileName(nullptr, path, pathSize);
		if (size == 0 || size == pathSize)
		{
			// Method failed or path was truncated.
			throw std::exception();
		}

		WCHAR* lastSlash = wcsrchr(path, L'\\');
		if (lastSlash)
		{
			*(lastSlash + 1) = L'\0';
		}
	}

	std::string WideStringToString(const std::wstring& s)
	{
		int len;
		int slength = (int)s.length() + 1;
		len = WideCharToMultiByte(0, 0, s.c_str(), slength, 0, 0, 0, 0);
		std::string r(len, '\0');
		WideCharToMultiByte(0, 0, s.c_str(), slength, &r[0], len, 0, 0);
		return r;
	}

	std::u16string StringtoU16(const std::string& str)
	{
		std::u16string wstr = u"";
		char16_t c16str[3] = u"\0";
		mbstate_t mbs;
		for (const auto& it : str)
		{
			//set shift state to the initial state
			memset(&mbs, 0, sizeof(mbs));
			memmove(c16str, u"\0\0\0", 3);
			std::mbrtoc16(c16str, &it, 3, &mbs);
			wstr.append(std::u16string(c16str));
		}
		return wstr;
	}

	std::string U16toString(const std::u16string& wstr)
	{
		std::string str = "";
		char cstr[3] = "\0";
		mbstate_t mbs;
		for (const auto& it : wstr)
		{
			//set shift state to the initial state
			memset(&mbs, 0, sizeof(mbs));
			memmove(cstr, "\0\0\0", 3);
			std::c16rtomb(cstr, it, &mbs);
			str.append(std::string(cstr));
		}
		return str;
	}
}
