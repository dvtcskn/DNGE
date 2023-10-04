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

Archive::Archive()
{
	readMode = false;
	pos = 0;

	version = 1;
	dataSize = 128; // this will grow if necessary anyway...
	DATA = new char[dataSize];
	(*this) << version;
}

Archive::Archive(const std::string& InfileName, bool InreadMode)
	: readMode(InreadMode)
	, fileName(InfileName)
	, version(1)
	, DATA(nullptr)
	, dataSize(128)
	, pos(0)
{
	if (!fileName.empty())
	{
		if (readMode)
		{
			std::ifstream file(fileName, std::ios::binary | std::ios::ate);
			if (file.is_open())
			{
				dataSize = (size_t)file.tellg();
				file.seekg(0, file.beg);
				DATA = new char[(size_t)dataSize];
				file.read(DATA, dataSize);
				file.close();
				(*this) >> version;
				if (version < 1.0f)
				{
					std::stringstream ss("");
					ss << "The archive version (" << version << ") is no longer supported!";
					Close();
				}
				if (version > 1.0f)
				{
					std::stringstream ss("");

					ss << "The archive version (" << version << ") is higher than the program's (" << 1.0f << ")!";
					Close();
				}
			}
		}
		else
		{

			readMode = false;
			pos = 0;

			version = 1;
			dataSize = 128; // this will grow if necessary anyway...
			DATA = new char[dataSize];
			(*this) << version;
		}
	}
}

Archive::~Archive()
{
	Close();
}

void Archive::SetReadModeAndResetPos(bool isReadMode)
{
	readMode = isReadMode;
	pos = 0;

	if (readMode)
	{
		(*this) >> version;
	}
	else
	{
		(*this) << version;
	}
}

bool Archive::IsOpen()
{
	// when it is open, DATA is not null because it contains the version number at least!
	return DATA != nullptr;
}

void Archive::Close()
{
	if (!readMode && !fileName.empty())
	{
		SaveFile(fileName);
	}
	delete (DATA);
	(DATA) = nullptr;
}

bool Archive::SaveFile(const std::string& fileName)
{
	if (pos <= 0)
	{
		return false;
	}

	std::ofstream file(fileName, std::ios::binary | std::ios::trunc);
	if (file.is_open())
	{
		file.write(DATA, (std::streamsize)pos);
		file.close();
		return true;
	}

	return false;
}

std::string Archive::GetSourceDirectory()
{
	return "..//Content";
}

std::string Archive::GetSourceFileName()
{
	return fileName;
}
