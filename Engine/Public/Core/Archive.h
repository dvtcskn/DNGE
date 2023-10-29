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

#include <vector>
#include "Math/CoreMath.h"
#include <string>
#include "Engine/ClassBody.h"

class sArchive
{
	sBaseClassBody(sClassConstructor, sArchive)
public:
	sArchive(std::optional<std::string> FileName = std::nullopt);
	~sArchive();
	void ResetPos();
	void Close();
	void OpenFile(std::string FileName);
	bool SaveToFile(const std::string& FileName);
	std::string GetFileName();

	template<typename T>
	inline sArchive& operator<<(T data)
	{
		Serialize((int8_t)data);
		return *this;
	}
	inline sArchive& operator<<(bool data)
	{
		Serialize((uint32_t)(data ? 1 : 0));
		return *this;
	}
	inline sArchive& operator<<(char data)
	{
		Serialize((int8_t)data);
		return *this;
	}
	inline sArchive& operator<<(unsigned char data)
	{
		Serialize((uint8_t)data);
		return *this;
	}
	inline sArchive& operator<<(int data)
	{
		Serialize((int64_t)data);
		return *this;
	}
	inline sArchive& operator<<(unsigned int data)
	{
		Serialize((uint64_t)data);
		return *this;
	}
	inline sArchive& operator<<(long data)
	{
		Serialize((int64_t)data);
		return *this;
	}
	inline sArchive& operator<<(unsigned long data)
	{
		Serialize((uint64_t)data);
		return *this;
	}
	inline sArchive& operator<<(long long data)
	{
		Serialize((int64_t)data);
		return *this;
	}
	inline sArchive& operator<<(unsigned long long data)
	{
		Serialize((uint64_t)data);
		return *this;
	}
	inline sArchive& operator<<(float data)
	{
		Serialize(data);
		return *this;
	}
	inline sArchive& operator<<(double data)
	{
		Serialize(data);
		return *this;
	}
	inline sArchive& operator<<(const FVector& data)
	{
		Serialize(data);
		return *this;
	}
	/*inline sArchive& operator<<(const DirectX::FVector2& data)
	{
		Serialize(data);
		return *this;
	}
	inline sArchive& operator<<(const DirectX::FVector3& data)
	{
		Serialize(data);
		return *this;
	}
	inline sArchive& operator<<(const DirectX::FVector4& data)
	{
		Serialize(data);
		return *this;
	}
	inline sArchive& operator<<(const DirectX::XMFLOAT2& data)
	{
		Serialize(data);
		return *this;
	}
	inline sArchive& operator<<(const DirectX::XMFLOAT3& data)
	{
		Serialize(data);
		return *this;
	}
	inline sArchive& operator<<(const DirectX::XMFLOAT4& data)
	{
		Serialize(data);
		return *this;
	}*/
	/*inline sArchive& operator<<(const XMFLOAT3X3& data)
	{
		Serialize(data);
		return *this;
	}
	inline sArchive& operator<<(const XMFLOAT4X3& data)
	{
		Serialize(data);
		return *this;
	}*/
	/*inline sArchive& operator<<(const DirectX::FMatrix& data)
	{
		Serialize(data);
		return *this;
	}*/
	//inline sArchive& operator<<(const XMUINT2& data)
	//{
	//	Serialize(data);
	//	return *this;
	//}
	//inline sArchive& operator<<(const XMUINT3& data)
	//{
	//	Serialize(data);
	//	return *this;
	//}
	//inline sArchive& operator<<(const XMUINT4& data)
	//{
	//	Serialize(data);
	//	return *this;
	//}
	inline sArchive& operator<<(const std::string& STR)
	{
		for (const auto& pchar : STR)
			(*this) << pchar;

		std::size_t found = STR.find('\0');
		if (found == std::string::npos)
			(*this) << '\0';

		return *this;
	}

	template<typename T>
	inline sArchive& operator >> (T& data)
	{
		Deserialize(data);
		return *this;
	}
	inline sArchive& operator >> (bool& data)
	{
		uint32_t temp;
		Deserialize(temp);
		data = (temp == 1);
		return *this;
	}
	inline sArchive& operator >> (char& data)
	{
		int8_t temp;
		Deserialize(temp);
		data = (char)temp;
		return *this;
	}
	inline sArchive& operator >> (unsigned char& data)
	{
		uint8_t temp;
		Deserialize(temp);
		data = (unsigned char)temp;
		return *this;
	}
	inline sArchive& operator >> (int& data)
	{
		int64_t temp;
		Deserialize(temp);
		data = (int)temp;
		return *this;
	}
	inline sArchive& operator >> (unsigned int& data)
	{
		uint64_t temp;
		Deserialize(temp);
		data = (unsigned int)temp;
		return *this;
	}
	inline sArchive& operator >> (long& data)
	{
		int64_t temp;
		Deserialize(temp);
		data = (long)temp;
		return *this;
	}
	inline sArchive& operator >> (unsigned long& data)
	{
		uint64_t temp;
		Deserialize(temp);
		data = (unsigned long)temp;
		return *this;
	}
	inline sArchive& operator >> (long long& data)
	{
		int64_t temp;
		Deserialize(temp);
		data = (long long)temp;
		return *this;
	}
	inline sArchive& operator >> (unsigned long long& data)
	{
		uint64_t temp;
		Deserialize(temp);
		data = (unsigned long long)temp;
		return *this;
	}
	inline sArchive& operator >> (float& data)
	{
		Deserialize(data);
		return *this;
	}
	inline sArchive& operator >> (double& data)
	{
		Deserialize(data);
		return *this;
	}
	inline sArchive& operator >> (FVector& data)
	{
		Deserialize(data);
		return *this;
	}
	/*inline sArchive& operator >> (DirectX::FVector2& data)
	{
		Deserialize(data);
		return *this;
	}
	inline sArchive& operator >> (DirectX::FVector3& data)
	{
		Deserialize(data);
		return *this;
	}
	inline sArchive& operator >> (DirectX::FVector4& data)
	{
		Deserialize(data);
		return *this;
	}
	inline sArchive& operator >> (DirectX::XMFLOAT2& data)
	{
		Deserialize(data);
		return *this;
	}
	inline sArchive& operator >> (DirectX::XMFLOAT3& data)
	{
		Deserialize(data);
		return *this;
	}
	inline sArchive& operator >> (DirectX::XMFLOAT4& data)
	{
		Deserialize(data);
		return *this;
	}*/
	/*inline sArchive& operator >> (XMFLOAT3X3& data)
	{
		Deserialize(data);
		return *this;
	}
	inline sArchive& operator >> (XMFLOAT4X3& data)
	{
		Deserialize(data);
		return *this;
	}*/
	/*inline sArchive& operator >> (DirectX::FMatrix& data)
	{
		Deserialize(data);
		return *this;
	}*/
	/*inline sArchive& operator >> (XMUINT2& data)
	{
		Deserialize(data);
		return *this;
	}
	inline sArchive& operator >> (XMUINT3& data)
	{
		Deserialize(data);
		return *this;
	}
	inline sArchive& operator >> (XMUINT4& data)
	{
		Deserialize(data);
		return *this;
	}*/
	inline sArchive& operator >> (std::string& STR)
	{
		for (std::size_t i = pos; i < Data.size(); i++)
		{
			char pChar = 0;
			(*this) >> pChar;

			if (pChar == 0 || pChar == '\0' || pChar == std::string::npos)
				break;

			STR.push_back(pChar);
		}

		return *this;
	}

private:
	template<typename T>
	inline void Serialize(const T& data)
	{
		const std::size_t reqSize = pos + sizeof(data);
		if (reqSize > Data.size())
			GrowData(reqSize * 2);

		memcpy((void*)(Data.data() + pos), (&data), (std::size_t)(sizeof(data)));
		pos = reqSize;
	}

	void GrowData(std::size_t reqSize)
	{
		Data.resize(reqSize);
	}

	template<typename T>
	inline void Deserialize(T& data)
	{
		memcpy((void*)&data, (void*)(Data.data() + pos), (std::size_t)(sizeof(data)));
		pos += (size_t)(sizeof(data));
	}

private:
	std::size_t pos;
	std::vector<std::uint8_t> Data;
	std::string FileName;
};
