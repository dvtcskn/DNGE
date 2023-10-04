/* ---------------------------------------------------------------------------------------
* MIT License
*
* Copyright (c) 2023 Davut Co�kun.
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

#include <vector>
#include "Math/CoreMath.h"
#include <string>
#include "Engine/ClassBody.h"

class Archive
{
	sBaseClassBody(sClassConstructor, Archive)
public:
	Archive();
	Archive(const std::string& fileName, bool readMode = true);
	~Archive();
	bool IsReadMode() { return readMode; }
	void SetReadModeAndResetPos(bool isReadMode);
	bool IsOpen();
	void Close();
	bool SaveFile(const std::string& fileName);
	std::string GetSourceDirectory();
	std::string GetSourceFileName();

	// It could be templated but we have to be extremely careful of different datasizes on different platforms
	// because serialized data should be interchangeable!
	// So providing exact copy operations for exact types enforces platform agnosticism

	// Write operations
	inline Archive& operator<<(bool data)
	{
		_write((uint32_t)(data ? 1 : 0));
		return *this;
	}
	inline Archive& operator<<(char data)
	{
		_write((int8_t)data);
		return *this;
	}
	inline Archive& operator<<(unsigned char data)
	{
		_write((uint8_t)data);
		return *this;
	}
	inline Archive& operator<<(int data)
	{
		_write((int64_t)data);
		return *this;
	}
	inline Archive& operator<<(unsigned int data)
	{
		_write((uint64_t)data);
		return *this;
	}
	inline Archive& operator<<(long data)
	{
		_write((int64_t)data);
		return *this;
	}
	inline Archive& operator<<(unsigned long data)
	{
		_write((uint64_t)data);
		return *this;
	}
	inline Archive& operator<<(long long data)
	{
		_write((int64_t)data);
		return *this;
	}
	inline Archive& operator<<(unsigned long long data)
	{
		_write((uint64_t)data);
		return *this;
	}
	inline Archive& operator<<(float data)
	{
		_write(data);
		return *this;
	}
	inline Archive& operator<<(double data)
	{
		_write(data);
		return *this;
	}
	/*inline Archive& operator<<(const DirectX::FVector2& data)
	{
		_write(data);
		return *this;
	}
	inline Archive& operator<<(const DirectX::FVector3& data)
	{
		_write(data);
		return *this;
	}
	inline Archive& operator<<(const DirectX::FVector4& data)
	{
		_write(data);
		return *this;
	}
	inline Archive& operator<<(const DirectX::XMFLOAT2& data)
	{
		_write(data);
		return *this;
	}
	inline Archive& operator<<(const DirectX::XMFLOAT3& data)
	{
		_write(data);
		return *this;
	}
	inline Archive& operator<<(const DirectX::XMFLOAT4& data)
	{
		_write(data);
		return *this;
	}*/
	/*inline Archive& operator<<(const XMFLOAT3X3& data)
	{
		_write(data);
		return *this;
	}
	inline Archive& operator<<(const XMFLOAT4X3& data)
	{
		_write(data);
		return *this;
	}*/
	/*inline Archive& operator<<(const DirectX::FMatrix& data)
	{
		_write(data);
		return *this;
	}*/
	//inline Archive& operator<<(const XMUINT2& data)
	//{
	//	_write(data);
	//	return *this;
	//}
	//inline Archive& operator<<(const XMUINT3& data)
	//{
	//	_write(data);
	//	return *this;
	//}
	//inline Archive& operator<<(const XMUINT4& data)
	//{
	//	_write(data);
	//	return *this;
	//}
	inline Archive& operator<<(const std::string& data)
	{
		uint64_t len = (uint64_t)(data.length() + 1); // +1 for the null-terminator
		_write(len);
		_write(*data.c_str(), len);
		return *this;
	}
	inline Archive& operator<<(const std::wstring& data)
	{
		uint64_t len = (uint64_t)(data.length() + 1); // +1 for the null-terminator
		_write(len);
		_write(*data.c_str(), len);
		return *this;
	}
	template<typename T>
	inline Archive& operator<<(const std::vector<T>& data)
	{
		// Here we will use the << operator so that non-specified types will have compile error!
		(*this) << data.size();
		for (const T& x : data)
		{
			(*this) << x;
		}
		return *this;
	}

	// Read operations
	inline Archive& operator >> (bool& data)
	{
		uint32_t temp;
		_read(temp);
		data = (temp == 1);
		return *this;
	}
	inline Archive& operator >> (char& data)
	{
		int8_t temp;
		_read(temp);
		data = (char)temp;
		return *this;
	}
	inline Archive& operator >> (unsigned char& data)
	{
		uint8_t temp;
		_read(temp);
		data = (unsigned char)temp;
		return *this;
	}
	inline Archive& operator >> (int& data)
	{
		int64_t temp;
		_read(temp);
		data = (int)temp;
		return *this;
	}
	inline Archive& operator >> (unsigned int& data)
	{
		uint64_t temp;
		_read(temp);
		data = (unsigned int)temp;
		return *this;
	}
	inline Archive& operator >> (long& data)
	{
		int64_t temp;
		_read(temp);
		data = (long)temp;
		return *this;
	}
	inline Archive& operator >> (unsigned long& data)
	{
		uint64_t temp;
		_read(temp);
		data = (unsigned long)temp;
		return *this;
	}
	inline Archive& operator >> (long long& data)
	{
		int64_t temp;
		_read(temp);
		data = (long long)temp;
		return *this;
	}
	inline Archive& operator >> (unsigned long long& data)
	{
		uint64_t temp;
		_read(temp);
		data = (unsigned long long)temp;
		return *this;
	}
	inline Archive& operator >> (float& data)
	{
		_read(data);
		return *this;
	}
	inline Archive& operator >> (double& data)
	{
		_read(data);
		return *this;
	}
	/*inline Archive& operator >> (DirectX::FVector2& data)
	{
		_read(data);
		return *this;
	}
	inline Archive& operator >> (DirectX::FVector3& data)
	{
		_read(data);
		return *this;
	}
	inline Archive& operator >> (DirectX::FVector4& data)
	{
		_read(data);
		return *this;
	}
	inline Archive& operator >> (DirectX::XMFLOAT2& data)
	{
		_read(data);
		return *this;
	}
	inline Archive& operator >> (DirectX::XMFLOAT3& data)
	{
		_read(data);
		return *this;
	}
	inline Archive& operator >> (DirectX::XMFLOAT4& data)
	{
		_read(data);
		return *this;
	}*/
	/*inline Archive& operator >> (XMFLOAT3X3& data)
	{
		_read(data);
		return *this;
	}
	inline Archive& operator >> (XMFLOAT4X3& data)
	{
		_read(data);
		return *this;
	}*/
	/*inline Archive& operator >> (DirectX::FMatrix& data)
	{
		_read(data);
		return *this;
	}*/
	/*inline Archive& operator >> (XMUINT2& data)
	{
		_read(data);
		return *this;
	}
	inline Archive& operator >> (XMUINT3& data)
	{
		_read(data);
		return *this;
	}
	inline Archive& operator >> (XMUINT4& data)
	{
		_read(data);
		return *this;
	}*/
	inline Archive& operator >> (std::string& data)
	{
		uint64_t len;
		_read(len);
		char* str = new char[(size_t)len];
		memset(str, '\0', (size_t)(sizeof(char) * len));
		_read(*str, len);
		data = std::string(str);
		delete[] str;
		return *this;
	}
	inline Archive& operator >> (std::wstring& data)
	{
		uint64_t len;
		_read(len);
		wchar_t* str = new wchar_t[(size_t)len];
		memset(str, '\0', (size_t)(sizeof(wchar_t) * len));
		_read(*str, len);
		data = std::wstring(str);
		delete[] str;
		return *this;
	}
	template<typename T>
	inline Archive& operator >> (std::vector<T>& data)
	{
		// Here we will use the >> operator so that non-specified types will have compile error!
		size_t count;
		(*this) >> count;
		data.resize(count);
		for (size_t i = 0; i < count; ++i)
		{
			(*this) >> data[i];
		}
		return *this;
	}

private:
	// This should not be exposed to avoid misaligning data by mistake
	// Any specific type serialization should be implemented by hand
	// But these can be used as helper functions inside this class

	// Write data using memory operations
	template<typename T>
	inline void _write(const T& data, uint64_t count = 1)
	{
		size_t _size = (size_t)(sizeof(data) * count);
		size_t _right = pos + _size;
		if (_right > dataSize)
		{
			char* NEWDATA = new char[_right * 2];
			memcpy(NEWDATA, DATA, dataSize);
			dataSize = _right * 2;
			delete (DATA);
			DATA = NEWDATA;
		}
		memcpy(reinterpret_cast<void*>((uint64_t)DATA + (uint64_t)pos), &data, _size);
		pos = _right;
	}

	// Read data using memory operations
	template<typename T>
	inline void _read(T& data, uint64_t count = 1)
	{
		memcpy(&data, reinterpret_cast<void*>((uint64_t)DATA + (uint64_t)pos), (size_t)(sizeof(data) * count));
		pos += (size_t)(sizeof(data) * count);
	}

private:
	std::uint64_t version;
	bool readMode;
	size_t pos;
	char* DATA;
	size_t dataSize;

	std::string fileName; // save to this file on closing if not empty
};
