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
#include "Engine/AbstractEngineUtilities.h"

class sArchive
{
	sBaseClassBody(sClassConstructor, sArchive)
public:
	template <typename... Args>
	sArchive(Args&&... args)
		: FileName("")
		, pos(0)
	{
		std::tuple<Args...> Params = std::tuple<Args...>(args...);
		for_each_tuple(Params, [&](const auto& x) {
			(*this) << x;
			});
	}
	sArchive(std::optional<std::string> FileName = std::nullopt);
	sArchive(const std::vector<std::uint8_t>& Data);
	sArchive(const std::string& Data);
	~sArchive();
	void ResetPos(std::size_t pos = 0) const;
	void Close();
	void OpenFile(std::string FileName);
	bool SaveToFile(const std::string& FileName);
	std::string GetFileName();
	void SetData(const std::vector<std::uint8_t>& Data);
	void SetData(const std::uint8_t* Data, std::size_t Size);
	void SetData(const std::string& Data);

	inline std::size_t GetPosition() const { return pos; }
	inline std::size_t GetSize() const { return Data.size(); }
	inline std::vector<std::uint8_t> GetData() const { return Data; }
	std::string GetDataAsString() const;

	template <typename T>
	inline sArchive& operator<<(const std::vector<T>& data)
	{
		std::size_t Size = data.size();
		(*this) << Size;
		for (std::size_t i = 0; i < Size; i++)
			(*this) << data.at(i);
		return *this;
	}
	inline sArchive& operator<<(const std::string& STR)
	{
		for (const auto& pchar : STR)
			(*this) << pchar;

		std::size_t found = STR.find('\0');
		if (found == std::string::npos)
			(*this) << '\0';

		return *this;
	}

	inline sArchive& operator<<(bool data)
	{
		Serialize((data ? 1 : 0));
		return *this;
	}
	inline sArchive& operator<<(char data)
	{
		Serialize((int8_t)data);
		return *this;
	}
	inline sArchive& operator<<(unsigned char data)
	{
		Serialize(data);
		return *this;
	}
	inline sArchive& operator<<(int data)
	{
		Serialize(data);
		return *this;
	}
	inline sArchive& operator<<(unsigned int data)
	{
		Serialize(data);
		return *this;
	}
	inline sArchive& operator<<(long data)
	{
		Serialize(data);
		return *this;
	}
	inline sArchive& operator<<(unsigned long data)
	{
		Serialize(data);
		return *this;
	}
	inline sArchive& operator<<(long long data)
	{
		Serialize(data);
		return *this;
	}
	inline sArchive& operator<<(unsigned long long data)
	{
		Serialize(data);
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
	inline sArchive& operator<<(const FVector2& data)
	{
		Serialize(data);
		return *this;
	}
	inline sArchive& operator<<(const FVector4& data)
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
	}
	inline sArchive& operator<<(const DirectX::XMFLOAT3X3& data)
	{
		Serialize(data);
		return *this;
	}
	inline sArchive& operator<<(const DirectX::XMFLOAT4X3& data)
	{
		Serialize(data);
		return *this;
	}
	inline sArchive& operator<<(const FMatrix& data)
	{
		Serialize(data);
		return *this;
	}

	template <typename T>
	inline sArchive& operator>>(std::vector<T>& data)
	{
		std::size_t Size = 0;
		(*this) >> Size;
		for (std::size_t i = 0; i < Size; i++)
		{
			T t;
			(*this) >> t;

			data.push_back(t);
		}
		return *this;
	}
	template <typename T>
	inline void operator>>(std::vector<T>& data) const
	{
		std::size_t Size = 0;
		(*this) >> Size;
		for (std::size_t i = 0; i < Size; i++)
		{
			T t;
			(*this) >> t;

			data.push_back(t);
		}
	}
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
	inline void operator >> (std::string& STR) const
	{
		for (std::size_t i = pos; i < Data.size(); i++)
		{
			char pChar = 0;
			(*this) >> pChar;

			if (pChar == 0 || pChar == '\0' || pChar == std::string::npos)
				break;

			STR.push_back(pChar);
		}
	}
	inline sArchive& operator >> (bool& data)
	{
		bool temp;
		Deserialize(temp);
		data = (temp == 1);
		return *this;
	}
	inline void operator >> (bool& data) const
	{
		bool temp;
		Deserialize(temp);
		data = (temp == 1);
	}
	inline sArchive& operator >> (char& data)
	{
		Deserialize(data);
		return *this;
	}
	inline void operator >> (char& data) const
	{
		Deserialize(data);
	}
	inline sArchive& operator >> (unsigned char& data)
	{
		Deserialize(data);
		return *this;
	}
	inline void operator >> (unsigned char& data) const
	{
		Deserialize(data);
	}
	inline sArchive& operator >> (int& data)
	{
		Deserialize(data);
		return *this;
	}
	inline void operator >> (int& data) const
	{
		Deserialize(data);
	}
	inline sArchive& operator >> (unsigned int& data)
	{
		Deserialize(data);
		return *this;
	}
	inline void operator >> (unsigned int& data) const
	{
		Deserialize(data);
	}
	inline sArchive& operator >> (long& data)
	{
		Deserialize(data);
		return *this;
	}
	inline void operator >> (long& data) const
	{
		Deserialize(data);
	}
	inline sArchive& operator >> (unsigned long& data)
	{
		Deserialize(data);
		return *this;
	}
	inline void operator >> (unsigned long& data) const
	{
		Deserialize(data);
	}
	inline sArchive& operator >> (long long& data)
	{
		Deserialize(data);
		return *this;
	}
	inline void operator >> (long long& data) const
	{
		Deserialize(data);
	}
	inline sArchive& operator >> (unsigned long long& data)
	{
		Deserialize(data);
		return *this;
	}
	inline void operator >> (unsigned long long& data) const
	{
		Deserialize(data);
	}
	inline sArchive& operator >> (float& data)
	{
		Deserialize(data);
		return *this;
	}
	inline void operator >> (float& data) const
	{
		Deserialize(data);
	}
	inline sArchive& operator >> (double& data)
	{
		Deserialize(data);
		return *this;
	}
	inline void operator >> (double& data) const
	{
		Deserialize(data);
	}
	inline sArchive& operator >> (FVector& data)
	{
		Deserialize(data);
		return *this;
	}
	inline void operator >> (FVector& data) const
	{
		Deserialize(data);
	}
	inline sArchive& operator >> (FVector2& data)
	{
		Deserialize(data);
		return *this;
	}
	inline void operator >> (FVector2& data) const
	{
		Deserialize(data);
	}
	inline sArchive& operator >> (FVector4& data)
	{
		Deserialize(data);
		return *this;
	}
	inline void operator >> (FVector4& data) const
	{
		Deserialize(data);
	}
	inline sArchive& operator >> (DirectX::XMFLOAT2& data)
	{
		Deserialize(data);
		return *this;
	}
	inline void operator >> (DirectX::XMFLOAT2& data) const
	{
		Deserialize(data);
	}
	inline sArchive& operator >> (DirectX::XMFLOAT3& data)
	{
		Deserialize(data);
		return *this;
	}
	inline void operator >> (DirectX::XMFLOAT3& data) const
	{
		Deserialize(data);
	}
	inline sArchive& operator >> (DirectX::XMFLOAT4& data)
	{
		Deserialize(data);
		return *this;
	}
	inline void operator >> (DirectX::XMFLOAT4& data) const
	{
		Deserialize(data);
	}
	inline sArchive& operator >> (DirectX::XMFLOAT3X3& data)
	{
		Deserialize(data);
		return *this;
	}
	inline void operator >> (DirectX::XMFLOAT3X3& data) const
	{
		Deserialize(data);
	}
	inline sArchive& operator >> (DirectX::XMFLOAT4X3& data)
	{
		Deserialize(data);
		return *this;
	}
	inline void operator >> (DirectX::XMFLOAT4X3& data) const
	{
		Deserialize(data);
	}
	inline sArchive& operator >> (FMatrix& data)
	{
		Deserialize(data);
		return *this;
	}
	inline void operator >> (FMatrix& data) const
	{
		Deserialize(data);
	}

private:
	template<typename T>
	inline void Serialize(const T& data)
	{
		const std::size_t dataSize = sizeof(data);
		const std::size_t reqSize = pos + dataSize;
		if (reqSize > Data.size())
		{
			ResizeData(Data.size() + dataSize * 2);
		}

		if (reqSize <= Data.size())
		{
			memcpy(&Data[pos], &data, dataSize);
			pos = reqSize;
		}
	}

	template<typename T>
	inline void Deserialize(T& data) const
	{
		const std::size_t dataSize = sizeof(data);
		if (pos + dataSize <= Data.size())
		{
			memcpy(&data, &Data[pos], dataSize);
			pos += dataSize;
		}
	}

public:
	inline void ResizeData(std::size_t reqSize, bool ResizeIfSmall = false)
	{
		if (ResizeIfSmall)
		{
			if (reqSize > Data.size())
				Data.resize(reqSize);
		}
		else
		{
			Data.resize(reqSize);
		}
	}

private:
	mutable std::size_t pos;
	std::vector<std::uint8_t> Data;
	std::string FileName;
};
