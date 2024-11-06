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

#include <shlobj.h>
#include <ctime>
#include <iomanip>
#include <commdlg.h>
#include <sstream>
#include <vector>
#include <cuchar>
#include <iostream>
#include <filesystem>
#include <Windows.h>

#pragma comment(lib, "Shell32.lib")

struct UntypedData;

namespace FileManager
{
	void SetContentDirectory(const std::string Path);
	std::string GetContentDirectory();
	std::string GetAssetDirectory();
	std::string GetMeshAssetDirectory();
	std::string GetMaterialAssetDirectory();
	std::string GetMapAssetDirectory();
	std::string GetLogsFolder();
	std::string GetShaderFolder();
	std::string GetTextureFolder();

	std::wstring GetContentDirectoryW();
	std::wstring GetAssetDirectoryW();
	std::wstring GetMeshAssetDirectoryW();
	std::wstring GetMaterialAssetDirectoryW();
	std::wstring GetMapAssetDirectoryW();
	std::wstring GetLogsFolderW();
	std::wstring GetShaderFolderW();
	std::wstring GetTextureFolderW();

	bool FileExists(const std::wstring& file);
	bool FileIsNewer(const std::wstring& file1, const std::wstring& file2); // ?

	std::string Normalize(const std::string& filePath);
	void NormalizeInline(std::string& filePath);
	bool ResolveRelativePaths(std::string& path);

	bool fsCreateFile(const std::string& InPath, const std::string InName);
	bool CreateDirectory_(const std::string& path);
	bool DeleteDirectory(const std::string& directory);
	bool DirectoryExists(const std::string& directory);
	bool IsDirectory(const std::string& directory);
	void OpenDirectoryWindow(const std::string& directory);
	bool OpenFile(OPENFILENAMEW& FILE);

	bool FileExists(const std::string& filePath);
	bool DeleteFile_(const std::string& filePath);
	bool CopyFileFromTo(const std::string& source, const std::string& destination);

	std::string GetFileNameFromFilePath(const std::string& path);
	std::string GetFileNameNoExtensionFromFilePath(const std::string& filepath);
	std::string GetDirectoryFromFilePath(const std::string& filePath);
	std::string GetFilePathWithoutExtension(const std::string& filePath);
	std::string GetExtensionFromFilePath(const std::string& filePath);
	std::string GetRelativeFilePath(const std::string& absoluteFilePath);
	std::string GetWorkingDirectory();
	std::string GetParentDirectory(const std::string& directory);
	std::vector<std::string> GetDirectoriesInDirectory(const std::string& directory);
	std::vector<std::string> GetFilesInDirectory(const std::string& directory);

	std::vector<std::string> GetSupportedFilesInDirectory(const std::string& directory);
	std::vector<std::string> GetSupportedImageFilesFromPaths(const std::vector<std::string>& paths);
	std::vector<std::string> GetSupportedAudioFilesFromPaths(const std::vector<std::string>& paths);
	std::vector<std::string> GetSupportedModelFilesFromPaths(const std::vector<std::string>& paths);
	std::vector<std::string> GetSupportedModelFilesInDirectory(const std::string& directory);
	std::vector<std::string> GetSupportedMapFilesInDirectory(const std::string& directory);

	bool IsMaterialFile(const std::string& filePath);
	bool IsMeshFile(const std::string& filePath);
	bool IsAudioFile(const std::string& filePath);
	bool IsMapFile(const std::string& filePath);
	bool IsTextureFile(const std::string& filePath);
	bool IsShaderFile(const std::string& filePath);

	std::string GetStringAfterExpression(const std::string& str, const std::string& expression);
	std::string GetStringBetweenExpressions(const std::string& str, const std::string& firstExpression, const std::string& secondExpression);
	std::string ConvertToUppercase(const std::string& lower);
	std::string ReplaceExpression(const std::string& str, const std::string& from, const std::string& to);
	std::string ResolveIncludeDirectives(const std::string& source, const std::string& directory);

	HRESULT ReadDataFromFile(LPCWSTR filename, byte* data, UINT* size);
	HRESULT ReadDataFromDDSFile(LPCWSTR filename, byte* data, UINT* offset, UINT* size);
	void GetAssetsPath(_Out_writes_(pathSize) WCHAR* path, UINT pathSize);

	std::wstring StringToWstring(const std::string& str);
	std::string WideStringToString(const std::wstring& s);
	std::u16string StringtoU16(const std::string& str);
	std::string U16toString(const std::u16string& wstr);

	bool folderExists(const std::filesystem::path& name);
	bool fileExists(const std::filesystem::path& name);
	std::shared_ptr<UntypedData> readFile(const std::filesystem::path& path);
	bool writeFile(const std::filesystem::path& name, const void* data, size_t size);
	int enumerateFiles(const std::filesystem::path& path, const std::vector<std::string>& extensions);
	int enumerateDirectories(const std::filesystem::path& path);
};
