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

#include <memory>
#include <iostream>
#include <vector>
#include <string>
#include <optional>
#include <mutex>
#include "Core/Math/CoreMath.h"
#include "Engine/ClassBody.h"

//enum eParamType
//{
//	eNoParam,
//	eVoid = eNoParam,
//	eInt,
//	eUInt,
//	eFloat,
//	eDouble,
//	eFVector,
//	eString,
//};

namespace
{
	/*constexpr std::string ParamTypeToString(eParamType Type)
	{
		switch (Type)
		{
			case eVoid: return "eVoid";
			case eInt: return "eInt";
			case eUInt: return "eUInt";
			case eFloat: return "eFloat";
			case eDouble: return "eDouble";
			case eFVector: return "eFVector";
			case eString: return "eString";
		}
		return "eVoid";
	}

	constexpr eParamType StringToParamType(std::string Type)
	{
		if (Type == "eVoid")
			return eParamType::eVoid;
		else if (Type == "eInt")
			return eParamType::eInt;
		else if (Type == "eUInt")
			return eParamType::eUInt;
		else if (Type == "eFloat")
			return eParamType::eFloat;
		else if (Type == "eDouble")
			return eParamType::eDouble;
		else if (Type == "eFVector")
			return eParamType::eFVector;
		else if (Type == "eString")
			return eParamType::eString;

		return eParamType::eVoid;
	}

	template<typename p>
	constexpr decltype(auto) GetParamType(p P) noexcept
	{
		if constexpr (std::is_same_v<decltype(P), void>) return eParamType::eVoid;
		else if constexpr (std::is_same_v<decltype(P), int>)  return eParamType::eInt;
		else if constexpr (std::is_same_v<decltype(P), std::uint32_t>)  return eParamType::eUInt;
		else if constexpr (std::is_same_v<decltype(P), float>)  return eParamType::eFloat;
		else if constexpr (std::is_same_v<decltype(P), double>)  return eParamType::eDouble;
		else if constexpr (std::is_same_v<decltype(P), FVector>)  return eParamType::eFVector;
		else if constexpr (std::is_same_v<decltype(P), std::string>)  return eParamType::eString;
	}*/

	/*
	* https://www.cppstories.com/2022/tuple-iteration-apply/
	*/
	template <typename TupleT, typename Fn>
	void for_each_tuple(TupleT&& tp, Fn&& fn)
	{
		std::apply
		(
			[&fn](auto&& ...args)
			{
				(fn(std::forward<decltype(args)>(args)), ...);
			}, std::forward<TupleT>(tp)
				);
	}
};
