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
#include "Core/Archive.h"

#include <fstream>
#include <sstream>

sArchive::sArchive()
	: FileName("")
	, pos(0)
{}

sArchive::~sArchive()
{
	Close();
}

void sArchive::Close()
{
	if (!FileName.empty())
	{
		SaveToFile(FileName);
	}
	Data.clear();
}

void sArchive::OpenFile(std::string FilePath)
{
	if (!FilePath.empty())
	{
		std::string str = FilePath;
		std::size_t found = str.find_last_of("//\\");
		FileName = std::string(str.begin() + found + 1, str.end());

		std::ifstream file(FilePath, std::ios::binary | std::ios::ate);
		if (file.is_open())
		{
			//file.seekg(0, std::ios::end);
			//uint64_t size = file.tellg();
			file.seekg(0, std::ios::beg);

			std::string line;
			while (std::getline(file, line))
			{
				Data.insert(Data.end(), line.begin(), line.end());
			}
		}
	}
}

bool sArchive::SaveToFile(const std::string& fileName)
{
	ResetPos();
	std::ofstream file(fileName, std::ios::binary | std::ios::trunc);
	if (file.is_open())
	{
		file.write((char*)Data.data(), (std::streamsize)pos);
		file.close();
		return true;
	}

	return false;
}

std::string sArchive::GetFileName()
{
	return FileName;
}
