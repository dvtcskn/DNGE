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
#include <string>
#include <array>
#include <stdexcept>
#include <optional>
#include <algorithm>
#include <cmath>
#include <random>
#include <limits>

#include <xmmintrin.h>
#include <emmintrin.h>

#ifndef Enable_DirectX_Math
#define Enable_DirectX_Math 1
#endif

#if Enable_DirectX_Math
#include <DirectXMath.h>
#include <DirectXColors.h>
#endif

#include "Engine/ClassBody.h"

#ifndef FORCEINLINE
#define FORCEINLINE __forceinline
#endif

//namespace /*CoreMath*/
//{
class FQuaternion;
class FRotationMatrix;
class FAngles;
class FVector4A;
class FBounds2D;

static constexpr float fPI			= 3.141592653f;
static constexpr double dPI			= 3.14159265358979323;
static constexpr double dPI_OVER_4  = 0.78539816339744830;

template<typename T>
FORCEINLINE constexpr T Square(const T A)
{
	return A * A;
}

template<typename T>
FORCEINLINE T ClampAxis(T pAngle)
{
	T Angle = std::fmod(pAngle, (T)360.0);

	if (Angle < (T)0.0)
		Angle += (T)360.0;

	return Angle;
}

template<typename T>
FORCEINLINE T NormalizeAxis(T Angle)
{
	Angle = ClampAxis(Angle);

	if (Angle > (T)180.0)
		Angle -= (T)360.0;

	return Angle;
}

template <typename T> __forceinline constexpr T AlignUpWithMask(T value, size_t mask)
{
	return (T)(((size_t)value + mask) & ~mask);
}

template <typename T> __forceinline constexpr T AlignDownWithMask(T value, size_t mask)
{
	return (T)((size_t)value & ~mask);
}

template <typename T> __forceinline constexpr T AlignUp(T value, size_t alignment)
{
	return AlignUpWithMask(value, alignment - 1);
}

template <typename T> __forceinline constexpr T AlignDown(T value, size_t alignment)
{
	return AlignDownWithMask(value, alignment - 1);
}

template <typename T> __forceinline constexpr bool IsAligned(T value, size_t alignment)
{
	return 0 == ((size_t)value & (alignment - 1));
}

template <typename T> __forceinline constexpr T DivideByMultiple(T value, size_t alignment)
{
	return (T)((value + alignment - 1) / alignment);
}

template <typename T> __forceinline constexpr bool IsPowerOfTwo(T value)
{
	return 0 == (value & (value - 1));
}

template <typename T> __forceinline constexpr bool IsDivisible(T value, T divisor)
{
	return (value / divisor) * divisor == value;
}

__forceinline uint8_t Log2(uint64_t value)
{
	unsigned long mssb; // most significant set bit
	unsigned long lssb; // least significant set bit

	// If perfect power of two (only one set bit), return index of bit.  Otherwise round up
	// fractional log by adding 1 to most signicant set bit's index.
	if (_BitScanReverse64(&mssb, value) > 0 && _BitScanForward64(&lssb, value) > 0)
		return uint8_t(mssb + (mssb == lssb ? 0 : 1));
	else
		return 0;
}

template <typename T> __forceinline constexpr T AlignPowerOfTwo(T value)
{
	return value == 0 ? 0 : 1 << Log2(value);
}

template<typename Type>
FORCEINLINE constexpr Type Clamp(Type x, Type min, Type max)
{
	return x > min ? x < max ? x : max : min;
}

template<typename T>
FORCEINLINE constexpr T SmoothStep(T edge0, T edge1, T x)
{
	// Scale, and clamp x to 0..1 range
	x = Clamp((x - edge0) / (edge1 - edge0), (T)0.0, (T)1.0);

	return x * x * ((T)3.0 - (T)2.0 * x);
}

FORCEINLINE constexpr float RadiansToDegrees(const float& Radians)
{
	return Radians * (180.f / 3.1415926535897932f);
};

FORCEINLINE constexpr float DegreesToRadians(const float& Degrees)
{
	return Degrees * (3.1415926535897932f / 180.f);
};

template<typename T, typename M>
FORCEINLINE constexpr T Lerp(T a, T b, M f)
{
	return a + f * (b - a);
}

template<typename T>
class TVector2
{
	typedef TVector2<T> Class;
	sStaticClassBody(Class)
public:
	static constexpr TVector2 Zero()
	{
		return TVector2(static_cast<T>(0.0f), static_cast<T>(0.0f));
	}

	static constexpr TVector2 One()
	{
		return TVector2(static_cast<T>(1.0f), static_cast<T>(1.0f));
	}

	static constexpr TVector2 UnitX()
	{
		return TVector2(static_cast<T>(1.0f), static_cast<T>(0.0f));
	}

	static constexpr TVector2 UnitY()
	{
		return TVector2(static_cast<T>(0.0f), static_cast<T>(1.0f));
	}

public:
	T X;
	T Y;

public:
	FORCEINLINE constexpr TVector2() noexcept
		: X(static_cast<T>(0.0f))
		, Y(static_cast<T>(0.0f))
	{}

	FORCEINLINE constexpr TVector2(const TVector2<T>& Other) noexcept
		: X(static_cast<T>(Other.X))
		, Y(static_cast<T>(Other.Y))
	{}

	FORCEINLINE constexpr TVector2(T val) noexcept
		: X(val)
		, Y(val)
	{}

	FORCEINLINE constexpr TVector2(T x, T y) noexcept
		: X(x)
		, Y(y)
	{}

	FORCEINLINE constexpr TVector2(const T* v)
		: X(v[0])
		, Y(v[1])
	{}

	T& operator [] (std::uint32_t i)
	{
		switch (i)
		{
		case 0: return X;
		case 1: return Y;
		}
		return Y;
	}

	const T& operator [] (std::uint32_t i) const
	{
		switch (i)
		{
		case 0: return X;
		case 1: return Y;
		}
		return Y;
	}

#if Enable_DirectX_Math
	FORCEINLINE constexpr TVector2(const DirectX::XMFLOAT2& value)
		: X(static_cast<T>(value.x))
		, Y(static_cast<T>(value.y))
	{}

	FORCEINLINE constexpr TVector2(const DirectX::XMFLOAT2A& value)
		: X(static_cast<T>(value.x))
		, Y(static_cast<T>(value.y))
	{}

	FORCEINLINE constexpr TVector2(const DirectX::XMVECTOR& value)
		: X(static_cast<T>(value.m128_f32[0]))
		, Y(static_cast<T>(value.m128_f32[1]))
	{}
#endif
	~TVector2() = default;

	FORCEINLINE constexpr float Length() const
	{
		return (float)std::sqrt((X * X) + (Y * Y));
	};

	FORCEINLINE constexpr float LengthSquared() const
	{
		return (X * X) + (Y * Y);
	};

	FORCEINLINE constexpr void Normalize()
	{
		float val = 1.0f / (float)std::sqrt((X * X) + (Y * Y));
		X *= val;
		Y *= val;
	};

	FORCEINLINE constexpr float Distance(const TVector2& value)
	{
		float v1 = X - value.X, v2 = Y - value.Y;
		return (float)std::sqrt((v1 * v1) + (v2 * v2));
	};

	FORCEINLINE constexpr float DistanceSquared(const TVector2& value)
	{
		float v1 = X - value.X, v2 = Y - value.Y;
		return (v1 * v1) + (v2 * v2);
	};

	FORCEINLINE constexpr float Dot(const TVector2& value)
	{
		return (X * value.X) + (Y * value.Y);
	};

	FORCEINLINE constexpr float Magnitude()
	{
		return std::sqrt(X * X + Y * Y);
	}

public:
	FORCEINLINE constexpr std::string ToString() const
	{
		return std::string("{ X: " + std::to_string(X) + " Y: " + std::to_string(Y) + " };");
	};

	FORCEINLINE constexpr bool Equals(const TVector2& other) const
	{
		return (X == other.X && Y == other.Y);
	};

#if Enable_DirectX_Math
	FORCEINLINE constexpr operator DirectX::XMFLOAT2() const
	{
		return DirectX::XMFLOAT2(X, Y);
	}
	FORCEINLINE constexpr operator DirectX::XMFLOAT2()
	{
		return DirectX::XMFLOAT2(X, Y);
	}

	FORCEINLINE constexpr operator DirectX::XMFLOAT2A() const
	{
		return DirectX::XMFLOAT2A(X, Y);
	}
	FORCEINLINE constexpr operator DirectX::XMFLOAT2A()
	{
		return DirectX::XMFLOAT2A(X, Y);
	}

	FORCEINLINE constexpr operator DirectX::XMVECTOR() const
	{
		return DirectX::XMVectorSet(X, Y, 0.0f, 0.0f);
	}
	FORCEINLINE constexpr operator DirectX::XMVECTOR()
	{
		return DirectX::XMVectorSet(X, Y, 0.0f, 0.0f);
	}

	template<typename T1>
	FORCEINLINE constexpr operator TVector2<T1>()
	{
		return TVector2<T1>((T1)X, (T1)Y);
	}
#endif
	FORCEINLINE constexpr void operator +=(const TVector2<T>& value)
	{
		X += value.X;
		Y += value.Y;
	}

	FORCEINLINE constexpr void operator -=(const TVector2<T>& value)
	{
		X -= value.X;
		Y -= value.Y;
	}

	FORCEINLINE constexpr void operator *=(const TVector2<T>& value)
	{
		X *= value.X;
		Y *= value.Y;
	}

	FORCEINLINE constexpr void operator *=(const float& scaleFactor)
	{
		X *= scaleFactor;
		Y *= scaleFactor;
	}

	FORCEINLINE constexpr void operator /=(const TVector2<T>& value)
	{
		X /= value.X;
		Y /= value.Y;
	}

	FORCEINLINE constexpr void operator /=(const float& divider)
	{
		X /= divider;
		Y /= divider;
	}

#if _MSVC_LANG >= 202002L
	constexpr auto operator<=>(const TVector2&) const = default;
#endif
};

#if _MSVC_LANG < 202002L
template<typename T>
FORCEINLINE bool constexpr operator ==(const TVector2<T>& value1, const TVector2<T>& value2)
{
	return (value1.X == value2.X && value1.Y == value2.Y);
};

template<typename T>
FORCEINLINE bool constexpr operator !=(const TVector2<T>& value1, const TVector2<T>& value2)
{
	return !((value1.X == value2.X) && (value1.Y == value2.Y));
};
#endif

template<typename T>
FORCEINLINE constexpr TVector2<T> operator +(const TVector2<T>& value1, const TVector2<T>& value2)
{
	return TVector2<T>(value1.X + value2.X, value1.Y + value2.Y);
};

template<typename T>
FORCEINLINE constexpr TVector2<T> operator -(const TVector2<T>& value1, const TVector2<T>& value2)
{
	return TVector2<T>(value1.X - value2.X, value1.Y - value2.Y);
};

template<typename T>
FORCEINLINE constexpr TVector2<T> operator *(const TVector2<T>& value1, const TVector2<T>& value2)
{
	return TVector2<T>(value1.X * value2.X, value1.Y * value2.Y);
};

template<typename T>
FORCEINLINE constexpr TVector2<T> operator *(const TVector2<T>& value, const float& scaleFactor)
{
	return TVector2<T>(value.X * scaleFactor, value.Y * scaleFactor);
};

template<typename T>
FORCEINLINE constexpr TVector2<T> operator *(const float& scaleFactor, const TVector2<T>& value)
{
	return TVector2<T>(value.X * scaleFactor, value.Y * scaleFactor);
};

template<typename T>
FORCEINLINE constexpr TVector2<T> operator /(const TVector2<T>& value1, const TVector2<T>& value2)
{
	return TVector2<T>(value1.X / value2.X, value1.Y / value2.Y);
};

template<typename T>
FORCEINLINE constexpr TVector2<T> operator /(const TVector2<T>& value1, const float& divider)
{
	return TVector2<T>(value1.X * (1 / divider), value1.Y * (1 / divider));
};

#if Enable_DirectX_Math
template<typename T>
FORCEINLINE constexpr TVector2<T> operator +(const TVector2<T>& value1, const DirectX::XMFLOAT2& value2)
{
	return TVector2<T>(value1.X + value2.x, value1.Y + value2.y);
};
template<typename T>
FORCEINLINE constexpr TVector2<T> operator +(const DirectX::XMFLOAT2& value1, const TVector2<T>& value2)
{
	return TVector2<T>(value1.x + value2.X, value1.y + value2.Y);
};

template<typename T>
FORCEINLINE constexpr TVector2<T> operator +(const TVector2<T>& value1, const DirectX::XMVECTOR& value2)
{
	return DirectX::XMVectorAdd(DirectX::XMVectorSet(value1.X, value1.Y, 0.0f, 0.0f), value2);
};
template<typename T>
FORCEINLINE constexpr TVector2<T> operator +(const DirectX::XMVECTOR& value1, const TVector2<T>& value2)
{
	return DirectX::XMVectorAdd(value1, DirectX::XMVectorSet(value2.X, value2.Y, 0.0f, 0.0f));
};

template<typename T>
FORCEINLINE constexpr TVector2<T> operator -(const TVector2<T>& value1, const DirectX::XMFLOAT2& value2)
{
	return TVector2<T>(value1.X - value2.x, value1.Y - value2.y);
};
template<typename T>
FORCEINLINE constexpr TVector2<T> operator -(const DirectX::XMFLOAT2& value1, const TVector2<T>& value2)
{
	return TVector2<T>(value1.x - value2.X, value1.y - value2.Y);
};

template<typename T>
FORCEINLINE constexpr TVector2<T> operator -(const TVector2<T>& value1, const DirectX::XMVECTOR& value2)
{
	return DirectX::XMVectorSubtract(DirectX::XMVectorSet(value1.X, value1.Y, 0.0f, 0.0f), value2);
};
template<typename T>
FORCEINLINE constexpr TVector2<T> operator -(const DirectX::XMVECTOR& value1, const TVector2<T>& value2)
{
	return DirectX::XMVectorSubtract(value1, DirectX::XMVectorSet(value2.X, value2.Y, 0.0f, 0.0f));
};

template<typename T>
FORCEINLINE constexpr TVector2<T> operator *(const TVector2<T>& value1, const DirectX::XMFLOAT2& value2)
{
	return TVector2<T>(value1.X * value2.x, value1.Y * value2.y);
};
template<typename T>
FORCEINLINE constexpr TVector2<T> operator *(const DirectX::XMFLOAT2& value1, const TVector2<T>& value2)
{
	return TVector2<T>(value1.x * value2.X, value1.y * value2.Y);
};
template<typename T>
FORCEINLINE constexpr TVector2<T> operator *(const DirectX::XMFLOAT2& value1, const float& scaleFactor)
{
	return TVector2<T>(value1.x * scaleFactor, value1.y * scaleFactor);
};
template<typename T>
FORCEINLINE constexpr TVector2<T> operator *(const float& scaleFactor, const DirectX::XMFLOAT2& value1)
{
	return TVector2<T>(scaleFactor * value1.x, scaleFactor * value1.y);
};

template<typename T>
FORCEINLINE constexpr TVector2<T> operator *(const TVector2<T>& value1, const DirectX::XMVECTOR& value2)
{
	return DirectX::XMVectorMultiply(DirectX::XMVectorSet(value1.X, value1.Y, 0.0f, 0.0f), value2);
};
template<typename T>
FORCEINLINE constexpr TVector2<T> operator *(const DirectX::XMVECTOR& value1, const TVector2<T>& value2)
{
	return DirectX::XMVectorMultiply(value1, DirectX::XMVectorSet(value2.X, value2.Y, 0.0f, 0.0f));
};
template<typename T>
FORCEINLINE constexpr TVector2<T> operator *(const DirectX::XMVECTOR& value1, const float& scaleFactor)
{
	return DirectX::XMVectorMultiply(value1, DirectX::XMVectorSet(scaleFactor, scaleFactor, scaleFactor, scaleFactor));
};
template<typename T>
FORCEINLINE constexpr TVector2<T> operator *(const float& scaleFactor, const DirectX::XMVECTOR& value1)
{
	return DirectX::XMVectorMultiply(DirectX::XMVectorSet(scaleFactor, scaleFactor, scaleFactor, scaleFactor), value1);
};

template<typename T>
FORCEINLINE constexpr TVector2<T> operator /(const TVector2<T>& value1, const DirectX::XMFLOAT2& value2)
{
	return TVector2<T>(value1.X / value2.x, value1.Y / value2.y);
};
template<typename T>
FORCEINLINE constexpr TVector2<T> operator /(const DirectX::XMFLOAT2& value1, const TVector2<T>& value2)
{
	return TVector2<T>(value1.x / value2.X, value1.y / value2.Y);
};
template<typename T>
FORCEINLINE constexpr TVector2<T> operator /(const DirectX::XMFLOAT2& value1, const float& divider)
{
	return TVector2<T>(value1.x / divider, value1.y / divider);
};

template<typename T>
FORCEINLINE constexpr TVector2<T> operator /(const TVector2<T>& value1, const DirectX::XMVECTOR& value2)
{
	return DirectX::XMVectorDivide(DirectX::XMVectorSet(value1.X, value1.Y, 0.0f, 0.0f), value2);
};
template<typename T>
FORCEINLINE constexpr TVector2<T> operator /(const DirectX::XMVECTOR& value1, const TVector2<T>& value2)
{
	return DirectX::XMVectorDivide(value1, DirectX::XMVectorSet(value2.X, value2.Y, 0.0f, 0.0f));
};
template<typename T>
FORCEINLINE constexpr TVector2<T> operator /(const DirectX::XMVECTOR& value1, const float& divider)
{
	return DirectX::XMVectorDivide(value1, DirectX::XMVectorSet(divider, divider, divider, divider));
};
#endif

typedef TVector2<float> FVector2;
typedef FVector2 FVector2f;
typedef TVector2<double> FVector2D;
typedef TVector2<std::int32_t> IntVector2;

template<typename T>
class TVector3
{
	typedef TVector3<T> Class;
	sStaticClassBody(Class)
public:
	static constexpr TVector3 Zero()
	{
		return TVector3(static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f));
	};

	static constexpr TVector3 One()
	{
		return TVector3(static_cast<T>(1.0f), static_cast<T>(1.0f), static_cast<T>(1.0f));
	};

	static constexpr TVector3 UnitX()
	{
		return TVector3(static_cast<T>(1.0f), static_cast<T>(0.0f), static_cast<T>(0.0f));
	};

	static constexpr TVector3 UnitY()
	{
		return TVector3(static_cast<T>(0.0f), static_cast<T>(1.0f), static_cast<T>(0.0f));
	};

	static constexpr TVector3 UnitZ()
	{
		return TVector3(static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(1.0f));
	};

public:
	T X;
	T Y;
	T Z;

public:
	FORCEINLINE constexpr TVector3() noexcept
		: X(0.0f)
		, Y(0.0f)
		, Z(0.0f)
	{}

	FORCEINLINE constexpr TVector3(const TVector3& Other) noexcept
		: X(Other.X)
		, Y(Other.Y)
		, Z(Other.Z)
	{}

	FORCEINLINE constexpr TVector3(T value) noexcept
		: X(value)
		, Y(value)
		, Z(value)
	{}

	explicit FORCEINLINE constexpr TVector3(const TVector2<T>& V) noexcept
		: X(V.X)
		, Y(V.Y)
		, Z(0.0f)
	{}

	FORCEINLINE constexpr TVector3(T x, T y, T z) noexcept
		: X(x)
		, Y(y)
		, Z(z)
	{}

	FORCEINLINE constexpr TVector3(const T* v)
		: X(v[0])
		, Y(v[1])
		, Z(v[2])
	{}

#if Enable_DirectX_Math
	FORCEINLINE constexpr TVector3(const DirectX::XMFLOAT3& value)
		: X(static_cast<T>(value.x))
		, Y(static_cast<T>(value.y))
		, Z(static_cast<T>(value.z))
	{}

	FORCEINLINE constexpr TVector3(const DirectX::XMFLOAT3A& value)
		: X(static_cast<T>(value.x))
		, Y(static_cast<T>(value.y))
		, Z(static_cast<T>(value.z))
	{}

	FORCEINLINE constexpr TVector3(const DirectX::XMVECTOR& value)
		: X(static_cast<T>(value.m128_f32[0]))
		, Y(static_cast<T>(value.m128_f32[1]))
		, Z(static_cast<T>(value.m128_f32[2]))
	{}
#endif

	~TVector3() = default;

	T& operator [] (std::uint32_t i) 
	{
		switch (i)
		{
		case 0: return X;
		case 1: return Y;
		case 2: return Z;
		}
		return Z;
	} 

	const T& operator [] (std::uint32_t i) const
	{
		switch (i)
		{
		case 0: return X;
		case 1: return Y;
		case 2: return Z;
		}
		return Z;
	} 

	FORCEINLINE constexpr bool IsNearlyZero(float Tolerance) const
	{
		return std::abs(X) <= Tolerance
			&& std::abs(Y) <= Tolerance
			&& std::abs(Z) <= Tolerance;
	}

	FORCEINLINE constexpr void Inverse()
	{
		X = -X;
		Y = -Y;
		Z = -Z;
	}
	FORCEINLINE constexpr TVector3<T> GetInverse() const
	{
		TVector3<T> V(X, Y, Z);
		V.Inverse();
		return V;
	}

	FORCEINLINE constexpr float Length() const
	{
		return (float)std::sqrt((X * X) + (Y * Y) + (Z * Z));
	};

	FORCEINLINE constexpr float LengthSquared() const
	{
		return (X * X) + (Y * Y) + (Z * Z);
	};

	FORCEINLINE constexpr void Normalize()
	{
		float val = 1.0f / (float)std::sqrt((X * X) + (Y * Y) + (Z * Z));
		X *= val;
		Y *= val;
		Z *= val;
	};

	FORCEINLINE constexpr float Distance(const TVector3<T>& value) const
	{
		float v1 = X - value.X, v2 = Y - value.Y, v3 = Z - value.Z;
		return (float)std::sqrt((v1 * v1) + (v2 * v2) + (v3 * v3));
	};

	FORCEINLINE constexpr float DistanceSquared(const TVector3<T>& value) const
	{
		float v1 = X - value.X, v2 = Y - value.Y, v3 = Z - value.Z;;
		return (v1 * v1) + (v2 * v2) + (v3 * v3);
	};

	FORCEINLINE constexpr float Dot(const TVector3<T>& value) const
	{
		return (X * value.X) + (Y * value.Y) + (Z * value.Z);
	};

	FORCEINLINE constexpr TVector3<T> CrossProduct(const TVector3<T>& V) const
	{
		return TVector3<T>
			(
				Y * V.Z - Z * V.Y,
				Z * V.X - X * V.Z,
				X * V.Y - Y * V.X
				);
	}

	FORCEINLINE constexpr TVector3<T> Negate()
	{
		X = -X;
		Y = -Y;
		Z = -Z;
		return *this;
	};

public:
	FORCEINLINE constexpr std::string ToString() const
	{
		return std::string("{ X: " + std::to_string(X) + " Y: " + std::to_string(Y) + " Z: " + std::to_string(Z) + " };");
	};

	FORCEINLINE constexpr bool Equals(const TVector3& other) const
	{
		return (X == other.X && Y == other.Y && Z == other.Z);
	};

#if Enable_DirectX_Math
	FORCEINLINE constexpr operator DirectX::XMFLOAT3() const
	{
		return DirectX::XMFLOAT3(X, Y, Z);
	}
	FORCEINLINE constexpr operator DirectX::XMFLOAT3()
	{
		return DirectX::XMFLOAT3(X, Y, Z);
	}

	FORCEINLINE constexpr operator DirectX::XMFLOAT3A() const
	{
		return DirectX::XMFLOAT3A(X, Y, Z);
	}
	FORCEINLINE constexpr operator DirectX::XMFLOAT3A()
	{
		return DirectX::XMFLOAT3A(X, Y, Z);
	}

	FORCEINLINE constexpr operator DirectX::XMVECTOR() const
	{
		return DirectX::XMVectorSet(X, Y, Z, 0.0f);
	}

	FORCEINLINE constexpr operator DirectX::XMVECTOR()
	{
		return DirectX::XMVectorSet(X, Y, Z, 0.0f);
	}

	template<typename T1>
	FORCEINLINE constexpr operator TVector3<T1>()
	{
		return TVector3<T1>((T1)X, (T1)Y, (T1)Z);
	}
#endif
	FORCEINLINE constexpr void operator +=(const TVector3& value)
	{
		X += value.X;
		Y += value.Y;
		Z += value.Z;
	}

	FORCEINLINE constexpr void operator -=(const TVector3& value)
	{
		X -= value.X;
		Y -= value.Y;
		Z -= value.Z;
	}

	FORCEINLINE constexpr void operator *=(const TVector3& value)
	{
		X *= value.X;
		Y *= value.Y;
		Z *= value.Z;
	}

	FORCEINLINE constexpr void operator *=(const float& scaleFactor)
	{
		X *= scaleFactor;
		Y *= scaleFactor;
		Z *= scaleFactor;
	}

	FORCEINLINE constexpr void operator /=(const TVector3& value)
	{
		X /= value.X;
		Y /= value.Y;
		Z /= value.Z;
	}

	FORCEINLINE constexpr void operator /=(const float& divider)
	{
		X /= divider;
		Y /= divider;
		Z /= divider;
	}

#if _MSVC_LANG >= 202002L
	constexpr auto operator<=>(const TVector3&) const = default;
#endif
};

#if _MSVC_LANG < 202002L
template<typename T>
FORCEINLINE constexpr bool operator ==(const TVector3<T>& value1, const TVector3<T>& value2)
{
	return (value1.X == value2.X
		&& value1.Y == value2.Y
		&& value1.Z == value2.Z);
};

template<typename T>
FORCEINLINE constexpr bool operator !=(const TVector3<T>& value1, const TVector3<T>& value2)
{
	return !((value1.X == value2.X) && (value1.Y == value2.Y) && (value1.Z == value2.Z));
};
#endif

template<typename T>
FORCEINLINE constexpr TVector3<T> operator +(const TVector3<T>& value1, const TVector3<T>& value2)
{
	return TVector3<T>(value1.X + value2.X, value1.Y + value2.Y, value1.Z + value2.Z);
};

template<typename T>
FORCEINLINE constexpr TVector3<T> operator -(const TVector3<T>& value1, const TVector3<T>& value2)
{
	return TVector3<T>(value1.X - value2.X, value1.Y - value2.Y, value1.Z - value2.Z);
};

template<typename T>
FORCEINLINE constexpr TVector3<T> operator *(const TVector3<T>& value1, const TVector3<T>& value2)
{
	return TVector3<T>(value1.X * value2.X, value1.Y * value2.Y, value1.Z * value2.Z);
};

template<typename T>
FORCEINLINE constexpr TVector3<T> operator *(const TVector3<T>& value, const float& scaleFactor)
{
	return TVector3<T>(value.X * scaleFactor, value.Y * scaleFactor, value.Z * scaleFactor);
};

template<typename T>
FORCEINLINE constexpr TVector3<T> operator *(const float& scaleFactor, const TVector3<T>& value)
{
	return TVector3<T>(value.X * scaleFactor, value.Y * scaleFactor, value.Z * scaleFactor);
};

template<typename T>
FORCEINLINE constexpr TVector3<T> operator /(const TVector3<T>& value1, const TVector3<T>& value2)
{
	return TVector3<T>(value1.X / value2.X, value1.Y / value2.Y, value1.Z / value2.Z);
};

template<typename T>
FORCEINLINE constexpr TVector3<T> operator /(const TVector3<T>& value1, const float& divider)
{
	return TVector3<T>(value1.X * (1 / divider), value1.Y * (1 / divider), value1.Z * (1 / divider));
};

#if Enable_DirectX_Math
template<typename T>
FORCEINLINE constexpr TVector3<T> operator +(const TVector3<T>& value1, const DirectX::XMFLOAT3& value2)
{
	return TVector3<T>(value1.X + value2.x, value1.Y + value2.y, value1.Z + value2.z);
};
template<typename T>
FORCEINLINE constexpr TVector3<T> operator +(const DirectX::XMFLOAT3& value1, const TVector3<T>& value2)
{
	return TVector3<T>(value1.x + value2.X, value1.y + value2.Y, value1.z + value2.Z);
};

template<typename T>
FORCEINLINE constexpr TVector3<T> operator +(const TVector3<T>& value1, const DirectX::XMVECTOR& value2)
{
	return DirectX::XMVectorAdd(DirectX::XMVectorSet(value1.X, value1.Y, value1.Z, 0.0f), value2);
};
template<typename T>
FORCEINLINE constexpr TVector3<T> operator +(const DirectX::XMVECTOR& value1, const TVector3<T>& value2)
{
	return DirectX::XMVectorAdd(value1, DirectX::XMVectorSet(value2.X, value2.Y, value2.Z, 0.0f));
};

template<typename T>
FORCEINLINE constexpr TVector3<T> operator -(const TVector3<T>& value1, const DirectX::XMFLOAT3& value2)
{
	return TVector3<T>(value1.X - value2.x, value1.Y - value2.y, value1.Z - value2.z);
};
template<typename T>
FORCEINLINE constexpr TVector3<T> operator -(const DirectX::XMFLOAT3& value1, const TVector3<T>& value2)
{
	return TVector3<T>(value1.x - value2.X, value1.y - value2.Y, value1.z - value2.Z);
};

template<typename T>
FORCEINLINE constexpr TVector3<T> operator -(const TVector3<T>& value1, const DirectX::XMVECTOR& value2)
{
	return DirectX::XMVectorSubtract(DirectX::XMVectorSet(value1.X, value1.Y, value1.Z, 0.0f), value2);
};
template<typename T>
FORCEINLINE constexpr TVector3<T> operator -(const DirectX::XMVECTOR& value1, const TVector3<T>& value2)
{
	return DirectX::XMVectorSubtract(value1, DirectX::XMVectorSet(value2.X, value2.Y, value2.Z, 0.0f));
};

template<typename T>
FORCEINLINE constexpr TVector3<T> operator *(const TVector3<T>& value1, const DirectX::XMFLOAT3& value2)
{
	return TVector3<T>(value1.X * value2.x, value1.Y * value2.y, value1.Z * value2.z);
};
template<typename T>
FORCEINLINE constexpr TVector3<T> operator *(const DirectX::XMFLOAT3& value1, const TVector3<T>& value2)
{
	return TVector3<T>(value1.x * value2.X, value1.y * value2.Y, value1.z * value2.Z);
};
template<typename T>
FORCEINLINE constexpr TVector3<T> operator *(const DirectX::XMFLOAT3& value1, const float& scaleFactor)
{
	return TVector3<T>(value1.x * scaleFactor, value1.y * scaleFactor, value1.z * scaleFactor);
};
template<typename T>
FORCEINLINE constexpr TVector3<T> operator *(const float& scaleFactor, const DirectX::XMFLOAT3& value1)
{
	return TVector3<T>(scaleFactor * value1.x, scaleFactor * value1.y, scaleFactor * value1.z);
};

template<typename T>
FORCEINLINE constexpr TVector3<T> operator *(const TVector3<T>& value1, const DirectX::XMVECTOR& value2)
{
	return DirectX::XMVectorMultiply(DirectX::XMVectorSet(value1.X, value1.Y, value1.Z, 0.0f), value2);
};
template<typename T>
FORCEINLINE constexpr TVector3<T> operator *(const DirectX::XMVECTOR& value1, const TVector3<T>& value2)
{
	return DirectX::XMVectorMultiply(value1, DirectX::XMVectorSet(value2.X, value2.Y, value2.Z, 0.0f));
};
template<typename T>
FORCEINLINE constexpr TVector3<T> operator *(const DirectX::XMVECTOR& value1, const float& scaleFactor)
{
	return DirectX::XMVectorMultiply(value1, DirectX::XMVectorSet(scaleFactor, scaleFactor, scaleFactor, scaleFactor));
};
template<typename T>
FORCEINLINE constexpr TVector3<T> operator *(const float& scaleFactor, const DirectX::XMVECTOR& value1)
{
	return DirectX::XMVectorMultiply(DirectX::XMVectorSet(scaleFactor, scaleFactor, scaleFactor, scaleFactor), value1);
};

template<typename T>
FORCEINLINE constexpr TVector3<T> operator /(const TVector3<T>& value1, const DirectX::XMFLOAT3& value2)
{
	return TVector3<T>(value1.X / value2.x, value1.Y / value2.y, value1.Z / value2.z);
};
template<typename T>
FORCEINLINE constexpr TVector3<T> operator /(const DirectX::XMFLOAT3& value1, const TVector3<T>& value2)
{
	return TVector3<T>(value1.x / value2.X, value1.y / value2.Y, value1.z / value2.Z);
};
template<typename T>
FORCEINLINE constexpr TVector3<T> operator /(const DirectX::XMFLOAT3& value1, const float& divider)
{
	return TVector3<T>(value1.x / divider, value1.y / divider, value1.z / divider);
};

template<typename T>
FORCEINLINE constexpr TVector3<T> operator /(const TVector3<T>& value1, const DirectX::XMVECTOR& value2)
{
	return DirectX::XMVectorDivide(DirectX::XMVectorSet(value1.X, value1.Y, value1.Z, 0.0f), value2);
};
template<typename T>
FORCEINLINE constexpr TVector3<T> operator /(const DirectX::XMVECTOR& value1, const TVector3<T>& value2)
{
	return DirectX::XMVectorDivide(value1, DirectX::XMVectorSet(value2.X, value2.Y, value2.Z, 0.0f));
};
template<typename T>
FORCEINLINE constexpr TVector3<T> operator /(const DirectX::XMVECTOR& value1, const float& divider)
{
	return DirectX::XMVectorDivide(value1, DirectX::XMVectorSet(divider, divider, divider, divider));
};
#endif

typedef TVector3<float> FVector;
typedef FVector FVector3f;
typedef TVector3<double> FVector3D;
typedef TVector3<std::int32_t> cbIntVector3;

template<typename T>
class TVector4
{
	typedef TVector4 Class;
	sStaticClassBody(Class)
public:
	static constexpr TVector4 Zero()
	{
		return TVector4(static_cast<T>(0.0), static_cast<T>(0.0), static_cast<T>(0.0), static_cast<T>(0.0));
	}

	static constexpr TVector4 One()
	{
		return TVector4(static_cast<T>(1.0), static_cast<T>(1.0), static_cast<T>(1.0), static_cast<T>(1.0));
	}

	static constexpr TVector4 UnitX()
	{
		return TVector4(static_cast<T>(1.0), static_cast<T>(0.0), static_cast<T>(0.0), static_cast<T>(0.0));
	}

	static constexpr TVector4 UnitY()
	{
		return TVector4(static_cast<T>(0.0), static_cast<T>(1.0), static_cast<T>(0.0), static_cast<T>(0.0));
	}

	static constexpr TVector4 UnitZ()
	{
		return TVector4(static_cast<T>(0.0), static_cast<T>(0.0), static_cast<T>(1.0), static_cast<T>(0.0));
	}

	static constexpr TVector4 UnitW()
	{
		return TVector4(static_cast<T>(0.0), static_cast<T>(0.0), static_cast<T>(0.0), static_cast<T>(1.0));
	}

public:
	T X;
	T Y;
	T Z;
	T W;

public:
	FORCEINLINE constexpr TVector4() noexcept
		: X(static_cast<T>(0.0))
		, Y(static_cast<T>(0.0))
		, Z(static_cast<T>(0.0))
		, W(static_cast<T>(0.0))
	{}

	FORCEINLINE constexpr TVector4(const TVector4& Other) noexcept
		: X(Other.X)
		, Y(Other.Y)
		, Z(Other.Z)
		, W(Other.W)
	{}

	FORCEINLINE constexpr TVector4(T value) noexcept
		: X(value)
		, Y(value)
		, Z(value)
		, W(value)
	{}

	FORCEINLINE constexpr TVector4(const TVector3<T>& value) noexcept
		: X(value.X)
		, Y(value.Y)
		, Z(value.Z)
		, W(1.0f)
	{}

	explicit FORCEINLINE constexpr TVector4(const TVector2<T>& XY, const TVector2<T>& ZW) noexcept
		: X(XY.X)
		, Y(XY.Y)
		, Z(ZW.X)
		, W(ZW.Y)
	{}

	explicit FORCEINLINE constexpr TVector4(T x, T y, T z, T w = 1.0f) noexcept
		: X(x)
		, Y(y)
		, Z(z)
		, W(w)
	{}

	FORCEINLINE constexpr TVector4(const T* v)
		: X(v[0])
		, Y(v[1])
		, Z(v[2])
		, W(v[3])
	{}

#if Enable_DirectX_Math
	FORCEINLINE constexpr TVector4(const DirectX::XMFLOAT4& value)
		: X(value.x)
		, Y(value.y)
		, Z(value.z)
		, W(value.w)
	{}

	FORCEINLINE constexpr TVector4(const DirectX::XMFLOAT4A& value)
		: X(value.x)
		, Y(value.y)
		, Z(value.z)
		, W(value.w)
	{}

	FORCEINLINE constexpr TVector4(const DirectX::XMVECTOR& value)
		: X(value.m128_f32[0])
		, Y(value.m128_f32[1])
		, Z(value.m128_f32[2])
		, W(value.m128_f32[3])
	{}
#endif

	~TVector4() = default;

	constexpr T& operator[](const std::size_t& idx)
	{
		return (&X)[idx];
	}

	const constexpr T& operator[](const std::size_t& idx) const
	{
		return (&X)[idx];
	}

	FORCEINLINE float Length() const
	{
		return (float)std::sqrt((X * X) + (Y * Y) + (Z * Z) + (W * W));
	};

	FORCEINLINE constexpr float LengthSquared() const
	{
		return (X * X) + (Y * Y) + (Z * Z) + (W * W);
	};

	FORCEINLINE void Normalize()
	{
		float val = 1.0f / (float)std::sqrt((X * X) + (Y * Y) + (Z * Z) + (W * W));
		X *= val;
		Y *= val;
		Z *= val;
		W *= val;
	};

	FORCEINLINE float Distance(const TVector4& value)
	{
		float v1 = X - value.X, v2 = Y - value.Y, v3 = Z - value.Z, v4 = W - value.W;
		return (float)std::sqrt((v1 * v1) + (v2 * v2) + (v3 * v3) + (v4 * v4));
	};

	FORCEINLINE constexpr float DistanceSquared(const TVector4& value)
	{
		float v1 = X - value.X, v2 = Y - value.Y, v3 = Z - value.Z, v4 = W - value.W;
		return (v1 * v1) + (v2 * v2) + (v3 * v3) + (v4 * v4);
	};

	FORCEINLINE constexpr float Dot(const TVector4& value)
	{
		return (X * value.X) + (Y * value.Y) + (Z * value.Z) + (W * value.W);
	};

	FORCEINLINE constexpr TVector4 CrossProduct(const TVector4& V) const
	{
		return TVector4(
			Y * V.Z - Z * V.Y,
			Z * V.X - X * V.Z,
			X * V.Y - Y * V.X,
			0.0f
		);
	};

public:
	FORCEINLINE constexpr std::string ToString() const
	{
		return std::string("{ X: " + std::to_string(X) + " Y: " + std::to_string(Y) + " Z: " + std::to_string(Z) + " W: " + std::to_string(W) + " };");
	};

	FORCEINLINE constexpr bool Equals(const TVector4& other) const
	{
		return (X == other.X && Y == other.Y && Z == other.Z && W == other.W);
	};

	FORCEINLINE constexpr operator FVector4A() const;
	FORCEINLINE constexpr operator FVector4A();

#if Enable_DirectX_Math
	FORCEINLINE constexpr operator DirectX::XMFLOAT4() const
	{
		return DirectX::XMFLOAT4(X, Y, Z, W);
	}
	FORCEINLINE constexpr operator DirectX::XMFLOAT4()
	{
		return DirectX::XMFLOAT4(X, Y, Z, W);
	}

	FORCEINLINE constexpr operator DirectX::XMFLOAT4A() const
	{
		return DirectX::XMFLOAT4A(X, Y, Z, W);
	}
	FORCEINLINE constexpr operator DirectX::XMFLOAT4A()
	{
		return DirectX::XMFLOAT4A(X, Y, Z, W);
	}

	FORCEINLINE operator DirectX::XMVECTOR() const
	{
		return DirectX::XMVectorSet(X, Y, Z, W);
	}
	FORCEINLINE operator DirectX::XMVECTOR()
	{
		return DirectX::XMVectorSet(X, Y, Z, W);
	}
#endif
	FORCEINLINE constexpr void operator +=(const TVector4& value)
	{
		X += value.X;
		Y += value.Y;
		Z += value.Z;
		W += value.W;
	}

	FORCEINLINE constexpr void operator -=(const TVector4& value)
	{
		X -= value.X;
		Y -= value.Y;
		Z -= value.Z;
		W += value.W;
	}

	FORCEINLINE constexpr void operator *=(const TVector4& value)
	{
		X *= value.X;
		Y *= value.Y;
		Z *= value.Z;
		W += value.W;
	}

	FORCEINLINE constexpr void operator *=(const float& scaleFactor)
	{
		X *= scaleFactor;
		Y *= scaleFactor;
		Z *= scaleFactor;
		W += scaleFactor;
	}

	FORCEINLINE constexpr void operator /=(const TVector4& value)
	{
		X /= value.X;
		Y /= value.Y;
		Z /= value.Z;
		W /= value.W;
	}

	FORCEINLINE constexpr void operator /=(const float& divider)
	{
		X /= divider;
		Y /= divider;
		Z /= divider;
		W /= divider;
	}

#if _MSVC_LANG >= 202002L
	constexpr auto operator<=>(const TVector4&) const = default;
#endif
};

#if _MSVC_LANG < 202002L
FORCEINLINE constexpr bool operator ==(const TVector4& value1, const TVector4& value2)
{
	return (value1.X == value2.X
		&& value1.Y == value2.Y
		&& value1.Z == value2.Z
		&& value1.W == value2.W);
};

FORCEINLINE constexpr bool operator !=(const TVector4& value1, const TVector4& value2)
{
	return !((value1.X == value2.X) && (value1.Y == value2.Y) && (value1.Z == value2.Z) && (value1.W == value2.W));
};
#endif

template<typename T>
FORCEINLINE constexpr TVector4<T> operator +(const TVector4<T>& value1, const TVector4<T>& value2)
{
	return TVector4<T>(value1.X + value2.X, value1.Y + value2.Y, value1.Z + value2.Z, value1.W + value2.W);
};

template<typename T>
FORCEINLINE constexpr TVector4<T> operator +(const TVector4<T>& value1, const FVector& value2)
{
	return TVector4<T>(value1.X + value2.X, value1.Y + value2.Y, value1.Z + value2.Z, value1.W);
};

template<typename T>
FORCEINLINE constexpr TVector4<T> operator +(const TVector4<T>& value1, const FVector2& value2)
{
	return TVector4<T>(value1.X + value2.X, value1.Y + value2.Y, value1.Z, value1.W);
};

template<typename T>
FORCEINLINE constexpr TVector4<T> operator +(const FVector2& value1, const TVector4<T>& value2)
{
	return TVector4<T>(value1.X + value2.X, value1.Y + value2.Y, value2.Z, value2.W);
};

template<typename T>
FORCEINLINE constexpr TVector4<T> operator -(const TVector4<T>& value1, const TVector4<T>& value2)
{
	return TVector4<T>(value1.X - value2.X, value1.Y - value2.Y, value1.Z - value2.Z, value1.W - value2.W);
};

template<typename T>
FORCEINLINE constexpr TVector4<T> operator -(const TVector4<T>& value1, const FVector& value2)
{
	return TVector4<T>(value1.X - value2.X, value1.Y - value2.Y, value1.Z - value2.Z, value1.W);
};

template<typename T>
FORCEINLINE constexpr TVector4<T> operator -(const FVector& value1, const TVector4<T>& value2)
{
	return TVector4<T>(value1.X - value2.X, value1.Y - value2.Y, value1.Z - value2.Z, value2.W);
};

template<typename T>
FORCEINLINE constexpr TVector4<T> operator *(const TVector4<T>& value1, const TVector4<T>& value2)
{
	return TVector4<T>(value1.X * value2.X, value1.Y * value2.Y, value1.Z * value2.Z, value1.W * value2.W);
};

template<typename T>
FORCEINLINE constexpr TVector4<T> operator *(const TVector4<T>& value, const float& scaleFactor)
{
	return TVector4<T>(value.X * scaleFactor, value.Y * scaleFactor, value.Z * scaleFactor, value.W * scaleFactor);
};

template<typename T>
FORCEINLINE constexpr TVector4<T> operator *(const float& scaleFactor, const TVector4<T>& value)
{
	return TVector4<T>(value.X * scaleFactor, value.Y * scaleFactor, value.Z * scaleFactor, value.W * scaleFactor);
};

template<typename T>
FORCEINLINE constexpr TVector4<T> operator /(const TVector4<T>& value1, const TVector4<T>& value2)
{
	return TVector4<T>(value1.X / value2.X, value1.Y / value2.Y, value1.Z / value2.Z, value1.W / value2.W);
};

template<typename T>
FORCEINLINE constexpr TVector4<T> operator /(const TVector4<T>& value, const float& divider)
{
	return TVector4<T>(value.X * (1 / divider), value.Y * (1 / divider), value.Z * (1 / divider), value.W * (1 / divider));
};

#if Enable_DirectX_Math
template<typename T>
FORCEINLINE constexpr TVector4<T> operator +(const TVector4<T>& value1, const DirectX::XMFLOAT4& value2)
{
	return TVector4<T>(value1.X + value2.x, value1.Y + value2.y, value1.Z + value2.z, value1.W + value2.w);
};
template<typename T>
FORCEINLINE constexpr TVector4<T> operator +(const DirectX::XMFLOAT4& value1, const TVector4<T>& value2)
{
	return TVector4<T>(value1.x + value2.X, value1.y + value2.Y, value1.z + value2.Z, value1.w + value2.W);
};

template<typename T>
FORCEINLINE constexpr TVector4<T> operator +(const TVector4<T>& value1, const DirectX::XMVECTOR& value2)
{
	return DirectX::XMVectorAdd(DirectX::XMVectorSet(value1.X, value1.Y, value1.Z, value1.W), value2);
};
template<typename T>
FORCEINLINE constexpr TVector4<T> operator +(const DirectX::XMVECTOR& value1, const TVector4<T>& value2)
{
	return DirectX::XMVectorAdd(value1, DirectX::XMVectorSet(value2.X, value2.Y, value2.Z, value2.W));
};

template<typename T>
FORCEINLINE constexpr TVector4<T> operator -(const TVector4<T>& value1, const DirectX::XMFLOAT4& value2)
{
	return TVector4<T>(value1.X - value2.x, value1.Y - value2.y, value1.Z - value2.z, value1.W - value2.w);
};
template<typename T>
FORCEINLINE constexpr TVector4<T> operator -(const DirectX::XMFLOAT4& value1, const TVector4<T>& value2)
{
	return TVector4<T>(value1.x - value2.X, value1.y - value2.Y, value1.z - value2.Z, value1.w - value2.W);
};

template<typename T>
FORCEINLINE constexpr TVector4<T> operator -(const TVector4<T>& value1, const DirectX::XMVECTOR& value2)
{
	return DirectX::XMVectorSubtract(DirectX::XMVectorSet(value1.X, value1.Y, value1.Z, value1.W), value2);
};
template<typename T>
FORCEINLINE constexpr TVector4<T> operator -(const DirectX::XMVECTOR& value1, const TVector4<T>& value2)
{
	return DirectX::XMVectorSubtract(value1, DirectX::XMVectorSet(value2.X, value2.Y, value2.Z, value2.W));
};

template<typename T>
FORCEINLINE constexpr TVector4<T> operator *(const TVector4<T>& value1, const DirectX::XMFLOAT4& value2)
{
	return TVector4<T>(value1.X * value2.x, value1.Y * value2.y, value1.Z * value2.z, value1.W * value2.w);
};
template<typename T>
FORCEINLINE constexpr TVector4<T> operator *(const DirectX::XMFLOAT4& value1, const TVector4<T>& value2)
{
	return TVector4<T>(value1.x * value2.X, value1.y * value2.Y, value1.z * value2.Z, value1.w * value2.W);
};
template<typename T>
FORCEINLINE constexpr TVector4<T> operator *(const DirectX::XMFLOAT4& value1, const float& scaleFactor)
{
	return TVector4<T>(value1.x * scaleFactor, value1.y * scaleFactor, value1.z * scaleFactor, value1.w * scaleFactor);
};
template<typename T>
FORCEINLINE constexpr TVector4<T> operator *(const float& scaleFactor, const DirectX::XMFLOAT4& value1)
{
	return TVector4<T>(scaleFactor * value1.x, scaleFactor * value1.y, scaleFactor * value1.z, scaleFactor * value1.w);
};

template<typename T>
FORCEINLINE constexpr TVector4<T> operator *(const TVector4<T>& value1, const DirectX::XMVECTOR& value2)
{
	return DirectX::XMVectorMultiply(DirectX::XMVectorSet(value1.X, value1.Y, value1.Z, value1.W), value2);
};
template<typename T>
FORCEINLINE constexpr TVector4<T> operator *(const DirectX::XMVECTOR& value1, const TVector4<T>& value2)
{
	return DirectX::XMVectorMultiply(value1, DirectX::XMVectorSet(value2.X, value2.Y, value2.Z, value2.W));
};
template<typename T>
FORCEINLINE constexpr TVector4<T> operator *(const DirectX::XMVECTOR& value1, const float& scaleFactor)
{
	return DirectX::XMVectorMultiply(value1, DirectX::XMVectorSet(scaleFactor, scaleFactor, scaleFactor, scaleFactor));
};
template<typename T>
FORCEINLINE constexpr TVector4<T> operator *(const float& scaleFactor, const DirectX::XMVECTOR& value1)
{
	return DirectX::XMVectorMultiply(DirectX::XMVectorSet(scaleFactor, scaleFactor, scaleFactor, scaleFactor), value1);
};

template<typename T>
FORCEINLINE constexpr TVector4<T> operator /(const TVector4<T>& value1, const DirectX::XMFLOAT4& value2)
{
	return TVector4<T>(value1.X / value2.x, value1.Y / value2.y, value1.Z / value2.z, value1.W / value2.w);
};
template<typename T>
FORCEINLINE constexpr TVector4<T> operator /(const DirectX::XMFLOAT4& value1, const TVector4<T>& value2)
{
	return TVector4<T>(value1.x / value2.X, value1.y / value2.Y, value1.z / value2.Z, value1.w / value2.W);
};
template<typename T>
FORCEINLINE constexpr TVector4<T> operator /(const DirectX::XMFLOAT4& value1, const float& divider)
{
	return TVector4<T>(value1.x / divider, value1.y / divider, value1.z / divider, value1.w / divider);
};

template<typename T>
FORCEINLINE constexpr TVector4<T> operator /(const TVector4<T>& value1, const DirectX::XMVECTOR& value2)
{
	return DirectX::XMVectorDivide(DirectX::XMVectorSet(value1.X, value1.Y, value1.Z, value1.W), value2);
};
template<typename T>
FORCEINLINE constexpr TVector4<T> operator /(const DirectX::XMVECTOR& value1, const TVector4<T>& value2)
{
	return DirectX::XMVectorDivide(value1, DirectX::XMVectorSet(value2.X, value2.Y, value2.Z, value2.W));
};
template<typename T>
FORCEINLINE constexpr TVector4<T> operator /(const DirectX::XMVECTOR& value1, const float& divider)
{
	return DirectX::XMVectorDivide(value1, DirectX::XMVectorSet(divider, divider, divider, divider));
};
#endif

//typedef TVector4<float> FVector4f;
typedef TVector4<double> FVector4D;
typedef TVector4<std::int32_t> cbIntVector4;

__declspec(align(16)) class FVector4A
{
	typedef FVector4A Class;
	sStaticClassBody(Class)
public:
	static constexpr FVector4A Zero()
	{
		return FVector4A(0.0f, 0.0f, 0.0f, 0.0f);
	}

	static constexpr FVector4A One()
	{
		return FVector4A(1.0f, 1.0f, 1.0f, 1.0f);
	}

	static constexpr FVector4A UnitX()
	{
		return FVector4A(1.0f, 0.0f, 0.0f, 0.0f);
	}

	static constexpr FVector4A UnitY()
	{
		return FVector4A(0.0f, 1.0f, 0.0f, 0.0f);
	}

	static constexpr FVector4A UnitZ()
	{
		return FVector4A(0.0f, 0.0f, 1.0f, 0.0f);
	}

	static constexpr FVector4A UnitW()
	{
		return FVector4A(0.0f, 0.0f, 0.0f, 1.0f);
	}

public:
	float X;
	float Y;
	float Z;
	float W;

public:
	FORCEINLINE constexpr FVector4A() noexcept
		: X(0.0f)
		, Y(0.0f)
		, Z(0.0f)
		, W(0.0f)
	{}

	FORCEINLINE constexpr FVector4A(const FVector4A& Other) noexcept
		: X(Other.X)
		, Y(Other.Y)
		, Z(Other.Z)
		, W(Other.W)
	{}

	FORCEINLINE constexpr FVector4A(const TVector4<float>& Other) noexcept
		: X(Other.X)
		, Y(Other.Y)
		, Z(Other.Z)
		, W(Other.W)
	{}

	FORCEINLINE constexpr FVector4A(float value) noexcept
		: X(value)
		, Y(value)
		, Z(value)
		, W(value)
	{}

	FORCEINLINE constexpr FVector4A(const FVector& value) noexcept
		: X(value.X)
		, Y(value.Y)
		, Z(value.Z)
		, W(1.0f)
	{}

	explicit FORCEINLINE constexpr FVector4A(const FVector2& XY, const FVector2& ZW) noexcept
		: X(XY.X)
		, Y(XY.Y)
		, Z(ZW.X)
		, W(ZW.Y)
	{}

	explicit FORCEINLINE constexpr FVector4A(float x, float y, float z, float w = 1.0f) noexcept
		: X(x)
		, Y(y)
		, Z(z)
		, W(w)
	{}

#if Enable_DirectX_Math
	FORCEINLINE constexpr FVector4A(const DirectX::XMFLOAT4& value)
		: X(value.x)
		, Y(value.y)
		, Z(value.z)
		, W(value.w)
	{}

	FORCEINLINE constexpr FVector4A(const DirectX::XMFLOAT4A& value)
		: X(value.x)
		, Y(value.y)
		, Z(value.z)
		, W(value.w)
	{}

	FORCEINLINE constexpr FVector4A(const DirectX::XMVECTOR& value)
		: X(value.m128_f32[0])
		, Y(value.m128_f32[1])
		, Z(value.m128_f32[2])
		, W(value.m128_f32[3])
	{}
#endif

	~FVector4A() = default;

	constexpr float& operator[](const std::size_t& idx)
	{
		return (&X)[idx];
	}

	const constexpr float& operator[](const std::size_t& idx) const
	{
		return (&X)[idx];
	}

	FORCEINLINE constexpr FVector4A GetInverse() const
	{
		return FVector4A(-X, -Y, -Z, -W);
	}

public:
	FORCEINLINE constexpr std::string ToString() const
	{
		return std::string("{ X: " + std::to_string(X) + " Y: " + std::to_string(Y) + " Z: " + std::to_string(Z) + " W: " + std::to_string(W) + " };");
	};

	FORCEINLINE constexpr bool Equals(const FVector4A& other) const
	{
		return (X == other.X && Y == other.Y && Z == other.Z && W == other.W);
	};

#if Enable_DirectX_Math
	FORCEINLINE constexpr operator DirectX::XMFLOAT4() const
	{
		return DirectX::XMFLOAT4(X, Y, Z, W);
	}
	FORCEINLINE constexpr operator DirectX::XMFLOAT4()
	{
		return DirectX::XMFLOAT4(X, Y, Z, W);
	}

	FORCEINLINE constexpr operator DirectX::XMFLOAT4A() const
	{
		return DirectX::XMFLOAT4A(X, Y, Z, W);
	}
	FORCEINLINE constexpr operator DirectX::XMFLOAT4A()
	{
		return DirectX::XMFLOAT4A(X, Y, Z, W);
	}

	FORCEINLINE operator DirectX::XMVECTOR() const
	{
		return DirectX::XMVectorSet(X, Y, Z, W);
	}
	FORCEINLINE operator DirectX::XMVECTOR()
	{
		return DirectX::XMVectorSet(X, Y, Z, W);
	}
#endif

#if _MSVC_LANG >= 202002L
	constexpr auto operator<=>(const FVector4A&) const = default;
#endif
};

template<typename T>
FORCEINLINE constexpr TVector4<T>::operator FVector4A() const
{
	return TVector4<T>(X, Y, Z, W);
}
template<typename T>
FORCEINLINE constexpr TVector4<T>::operator FVector4A()
{
	return TVector4<T>(X, Y, Z, W);
}

__declspec(align(16)) class FVector4
{
	typedef FVector4 Class;
	sStaticClassBody(Class)
public:
	static constexpr FVector4 Zero()
	{
		return FVector4(0.0f, 0.0f, 0.0f, 0.0f);
	}

	static constexpr FVector4 One()
	{
		return FVector4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	static constexpr FVector4 UnitX()
	{
		return FVector4(1.0f, 0.0f, 0.0f, 0.0f);
	}

	static constexpr FVector4 UnitY()
	{
		return FVector4(0.0f, 1.0f, 0.0f, 0.0f);
	}

	static constexpr FVector4 UnitZ()
	{
		return FVector4(0.0f, 0.0f, 1.0f, 0.0f);
	}

	static constexpr FVector4 UnitW()
	{
		return FVector4(0.0f, 0.0f, 0.0f, 1.0f);
	}

public:
	float X;
	float Y;
	float Z;
	float W;

public:
	FORCEINLINE constexpr FVector4() noexcept
		: X(0.0f)
		, Y(0.0f)
		, Z(0.0f)
		, W(0.0f)
	{}

	FORCEINLINE constexpr FVector4(const FVector4& Other) noexcept
		: X(Other.X)
		, Y(Other.Y)
		, Z(Other.Z)
		, W(Other.W)
	{}

	FORCEINLINE constexpr FVector4(float value) noexcept
		: X(value)
		, Y(value)
		, Z(value)
		, W(value)
	{}

	FORCEINLINE constexpr FVector4(const FVector& value) noexcept
		: X(value.X)
		, Y(value.Y)
		, Z(value.Z)
		, W(1.0f)
	{}

	explicit FORCEINLINE constexpr FVector4(const FVector2& XY, const FVector2& ZW) noexcept
		: X(XY.X)
		, Y(XY.Y)
		, Z(ZW.X)
		, W(ZW.Y)
	{}

	explicit FORCEINLINE constexpr FVector4(float x, float y, float z, float w = 1.0f) noexcept
		: X(x)
		, Y(y)
		, Z(z)
		, W(w)
	{}

	FORCEINLINE constexpr FVector4(const float* v)
		: X(v[0])
		, Y(v[1])
		, Z(v[2])
		, W(v[3])
	{}

#if Enable_DirectX_Math
	FORCEINLINE constexpr FVector4(const DirectX::XMFLOAT4& value)
		: X(value.x)
		, Y(value.y)
		, Z(value.z)
		, W(value.w)
	{}

	FORCEINLINE constexpr FVector4(const DirectX::XMFLOAT4A& value)
		: X(value.x)
		, Y(value.y)
		, Z(value.z)
		, W(value.w)
	{}

	FORCEINLINE constexpr FVector4(const DirectX::XMVECTOR& value)
		: X(value.m128_f32[0])
		, Y(value.m128_f32[1])
		, Z(value.m128_f32[2])
		, W(value.m128_f32[3])
	{}
#endif

	~FVector4() = default;

	constexpr float& operator[](const std::size_t& idx)
	{
		return (&X)[idx];
	}

	const constexpr float& operator[](const std::size_t& idx) const
	{
		return (&X)[idx];
	}

	FORCEINLINE float Length() const
	{
		return (float)std::sqrt((X * X) + (Y * Y) + (Z * Z) + (W * W));
	};

	FORCEINLINE constexpr float LengthSquared() const
	{
		return (X * X) + (Y * Y) + (Z * Z) + (W * W);
	};

	FORCEINLINE void Normalize()
	{
		float val = 1.0f / (float)std::sqrt((X * X) + (Y * Y) + (Z * Z) + (W * W));
		X *= val;
		Y *= val;
		Z *= val;
		W *= val;
	};

	FORCEINLINE float Distance(const FVector4& value)
	{
		float v1 = X - value.X, v2 = Y - value.Y, v3 = Z - value.Z, v4 = W - value.W;
		return (float)std::sqrt((v1 * v1) + (v2 * v2) + (v3 * v3) + (v4 * v4));
	};

	FORCEINLINE constexpr float DistanceSquared(const FVector4& value)
	{
		float v1 = X - value.X, v2 = Y - value.Y, v3 = Z - value.Z, v4 = W - value.W;
		return (v1 * v1) + (v2 * v2) + (v3 * v3) + (v4 * v4);
	};

	FORCEINLINE constexpr float Dot(const FVector4& value)
	{
		return (X * value.X) + (Y * value.Y) + (Z * value.Z) + (W * value.W);
	};

	FORCEINLINE constexpr FVector4 CrossProduct(const FVector4& V) const
	{
		return FVector4(
			Y * V.Z - Z * V.Y,
			Z * V.X - X * V.Z,
			X * V.Y - Y * V.X,
			0.0f
		);
	};

public:
	FORCEINLINE constexpr std::string ToString() const
	{
		return std::string("{ X: " + std::to_string(X) + " Y: " + std::to_string(Y) + " Z: " + std::to_string(Z) + " W: " + std::to_string(W) + " };");
	};

	FORCEINLINE constexpr bool Equals(const FVector4& other) const
	{
		return (X == other.X && Y == other.Y && Z == other.Z && W == other.W);
	};

#if Enable_DirectX_Math
	FORCEINLINE constexpr operator DirectX::XMFLOAT4() const
	{
		return DirectX::XMFLOAT4(X, Y, Z, W);
	}
	FORCEINLINE constexpr operator DirectX::XMFLOAT4()
	{
		return DirectX::XMFLOAT4(X, Y, Z, W);
	}

	FORCEINLINE constexpr operator DirectX::XMFLOAT4A() const
	{
		return DirectX::XMFLOAT4A(X, Y, Z, W);
	}
	FORCEINLINE constexpr operator DirectX::XMFLOAT4A()
	{
		return DirectX::XMFLOAT4A(X, Y, Z, W);
	}

	FORCEINLINE operator DirectX::XMVECTOR() const
	{
		return DirectX::XMVectorSet(X, Y, Z, W);
	}
	FORCEINLINE operator DirectX::XMVECTOR()
	{
		return DirectX::XMVectorSet(X, Y, Z, W);
	}
#endif
	FORCEINLINE constexpr void operator +=(const FVector4& value)
	{
		X += value.X;
		Y += value.Y;
		Z += value.Z;
		W += value.W;
	}

	FORCEINLINE constexpr void operator -=(const FVector4& value)
	{
		X -= value.X;
		Y -= value.Y;
		Z -= value.Z;
		W += value.W;
	}

	FORCEINLINE constexpr void operator *=(const FVector4& value)
	{
		X *= value.X;
		Y *= value.Y;
		Z *= value.Z;
		W += value.W;
	}

	FORCEINLINE constexpr void operator *=(const float& scaleFactor)
	{
		X *= scaleFactor;
		Y *= scaleFactor;
		Z *= scaleFactor;
		W += scaleFactor;
	}

	FORCEINLINE constexpr void operator /=(const FVector4& value)
	{
		X /= value.X;
		Y /= value.Y;
		Z /= value.Z;
		W /= value.W;
	}

	FORCEINLINE constexpr void operator /=(const float& divider)
	{
		X /= divider;
		Y /= divider;
		Z /= divider;
		W /= divider;
	}

#if _MSVC_LANG >= 202002L
	constexpr auto operator<=>(const FVector4&) const = default;
#endif
};

#if _MSVC_LANG < 202002L
FORCEINLINE constexpr bool operator ==(const FVector4& value1, const FVector4& value2)
{
	return (value1.X == value2.X
		&& value1.Y == value2.Y
		&& value1.Z == value2.Z
		&& value1.W == value2.W);
};

FORCEINLINE constexpr bool operator !=(const FVector4& value1, const FVector4& value2)
{
	return !((value1.X == value2.X) && (value1.Y == value2.Y) && (value1.Z == value2.Z) && (value1.W == value2.W));
};
#endif

FORCEINLINE constexpr FVector4 operator +(const FVector4& value1, const FVector4& value2)
{
	return FVector4(value1.X + value2.X, value1.Y + value2.Y, value1.Z + value2.Z, value1.W + value2.W);
};

FORCEINLINE constexpr FVector4 operator +(const FVector4& value1, const FVector& value2)
{
	return FVector4(value1.X + value2.X, value1.Y + value2.Y, value1.Z + value2.Z, value1.W);
};

FORCEINLINE constexpr FVector4 operator +(const FVector4& value1, const FVector2& value2)
{
	return FVector4(value1.X + value2.X, value1.Y + value2.Y, value1.Z, value1.W);
};

FORCEINLINE constexpr FVector4 operator +(const FVector2& value1, const FVector4& value2)
{
	return FVector4(value1.X + value2.X, value1.Y + value2.Y, value2.Z, value2.W);
};

FORCEINLINE constexpr FVector4 operator -(const FVector4& value1, const FVector4& value2)
{
	return FVector4(value1.X - value2.X, value1.Y - value2.Y, value1.Z - value2.Z, value1.W - value2.W);
};

FORCEINLINE constexpr FVector4 operator -(const FVector4& value1, const FVector& value2)
{
	return FVector4(value1.X - value2.X, value1.Y - value2.Y, value1.Z - value2.Z, value1.W);
};

FORCEINLINE constexpr FVector4 operator -(const FVector& value1, const FVector4& value2)
{
	return FVector4(value1.X - value2.X, value1.Y - value2.Y, value1.Z - value2.Z, value2.W);
};

FORCEINLINE constexpr FVector4 operator *(const FVector4& value1, const FVector4& value2)
{
	return FVector4(value1.X * value2.X, value1.Y * value2.Y, value1.Z * value2.Z, value1.W * value2.W);
};

FORCEINLINE constexpr FVector4 operator *(const FVector4& value, const float& scaleFactor)
{
	return FVector4(value.X * scaleFactor, value.Y * scaleFactor, value.Z * scaleFactor, value.W * scaleFactor);
};

FORCEINLINE constexpr FVector4 operator *(const float& scaleFactor, const FVector4& value)
{
	return FVector4(value.X * scaleFactor, value.Y * scaleFactor, value.Z * scaleFactor, value.W * scaleFactor);
};

FORCEINLINE constexpr FVector4 operator /(const FVector4& value1, const FVector4& value2)
{
	return FVector4(value1.X / value2.X, value1.Y / value2.Y, value1.Z / value2.Z, value1.W / value2.W);
};

FORCEINLINE constexpr FVector4 operator /(const FVector4& value, const float& divider)
{
	return FVector4(value.X * (1 / divider), value.Y * (1 / divider), value.Z * (1 / divider), value.W * (1 / divider));
};

#if Enable_DirectX_Math
FORCEINLINE constexpr FVector4 operator +(const FVector4& value1, const DirectX::XMFLOAT4& value2)
{
	return FVector4(value1.X + value2.x, value1.Y + value2.y, value1.Z + value2.z, value1.W + value2.w);
};
FORCEINLINE constexpr FVector4 operator +(const DirectX::XMFLOAT4& value1, const FVector4& value2)
{
	return FVector4(value1.x + value2.X, value1.y + value2.Y, value1.z + value2.Z, value1.w + value2.W);
};

FORCEINLINE constexpr FVector4 operator +(const FVector4& value1, const DirectX::XMVECTOR& value2)
{
	return DirectX::XMVectorAdd(DirectX::XMVectorSet(value1.X, value1.Y, value1.Z, value1.W), value2);
};
FORCEINLINE constexpr FVector4 operator +(const DirectX::XMVECTOR& value1, const FVector4& value2)
{
	return DirectX::XMVectorAdd(value1, DirectX::XMVectorSet(value2.X, value2.Y, value2.Z, value2.W));
};

FORCEINLINE constexpr FVector4 operator -(const FVector4& value1, const DirectX::XMFLOAT4& value2)
{
	return FVector4(value1.X - value2.x, value1.Y - value2.y, value1.Z - value2.z, value1.W - value2.w);
};
FORCEINLINE constexpr FVector4 operator -(const DirectX::XMFLOAT4& value1, const FVector4& value2)
{
	return FVector4(value1.x - value2.X, value1.y - value2.Y, value1.z - value2.Z, value1.w - value2.W);
};

FORCEINLINE constexpr FVector4 operator -(const FVector4& value1, const DirectX::XMVECTOR& value2)
{
	return DirectX::XMVectorSubtract(DirectX::XMVectorSet(value1.X, value1.Y, value1.Z, value1.W), value2);
};
FORCEINLINE constexpr FVector4 operator -(const DirectX::XMVECTOR& value1, const FVector4& value2)
{
	return DirectX::XMVectorSubtract(value1, DirectX::XMVectorSet(value2.X, value2.Y, value2.Z, value2.W));
};

FORCEINLINE constexpr FVector4 operator *(const FVector4& value1, const DirectX::XMFLOAT4& value2)
{
	return FVector4(value1.X * value2.x, value1.Y * value2.y, value1.Z * value2.z, value1.W * value2.w);
};
FORCEINLINE constexpr FVector4 operator *(const DirectX::XMFLOAT4& value1, const FVector4& value2)
{
	return FVector4(value1.x * value2.X, value1.y * value2.Y, value1.z * value2.Z, value1.w * value2.W);
};
FORCEINLINE constexpr FVector4 operator *(const DirectX::XMFLOAT4& value1, const float& scaleFactor)
{
	return FVector4(value1.x * scaleFactor, value1.y * scaleFactor, value1.z * scaleFactor, value1.w * scaleFactor);
};
FORCEINLINE constexpr FVector4 operator *(const float& scaleFactor, const DirectX::XMFLOAT4& value1)
{
	return FVector4(scaleFactor * value1.x, scaleFactor * value1.y, scaleFactor * value1.z, scaleFactor * value1.w);
};

FORCEINLINE constexpr FVector4 operator *(const FVector4& value1, const DirectX::XMVECTOR& value2)
{
	return DirectX::XMVectorMultiply(DirectX::XMVectorSet(value1.X, value1.Y, value1.Z, value1.W), value2);
};
FORCEINLINE constexpr FVector4 operator *(const DirectX::XMVECTOR& value1, const FVector4& value2)
{
	return DirectX::XMVectorMultiply(value1, DirectX::XMVectorSet(value2.X, value2.Y, value2.Z, value2.W));
};
FORCEINLINE constexpr FVector4 operator *(const DirectX::XMVECTOR& value1, const float& scaleFactor)
{
	return DirectX::XMVectorMultiply(value1, DirectX::XMVectorSet(scaleFactor, scaleFactor, scaleFactor, scaleFactor));
};
FORCEINLINE constexpr FVector4 operator *(const float& scaleFactor, const DirectX::XMVECTOR& value1)
{
	return DirectX::XMVectorMultiply(DirectX::XMVectorSet(scaleFactor, scaleFactor, scaleFactor, scaleFactor), value1);
};

FORCEINLINE constexpr FVector4 operator /(const FVector4& value1, const DirectX::XMFLOAT4& value2)
{
	return FVector4(value1.X / value2.x, value1.Y / value2.y, value1.Z / value2.z, value1.W / value2.w);
};
FORCEINLINE constexpr FVector4 operator /(const DirectX::XMFLOAT4& value1, const FVector4& value2)
{
	return FVector4(value1.x / value2.X, value1.y / value2.Y, value1.z / value2.Z, value1.w / value2.W);
};
FORCEINLINE constexpr FVector4 operator /(const DirectX::XMFLOAT4& value1, const float& divider)
{
	return FVector4(value1.x / divider, value1.y / divider, value1.z / divider, value1.w / divider);
};

FORCEINLINE constexpr FVector4 operator /(const FVector4& value1, const DirectX::XMVECTOR& value2)
{
	return DirectX::XMVectorDivide(DirectX::XMVectorSet(value1.X, value1.Y, value1.Z, value1.W), value2);
};
FORCEINLINE constexpr FVector4 operator /(const DirectX::XMVECTOR& value1, const FVector4& value2)
{
	return DirectX::XMVectorDivide(value1, DirectX::XMVectorSet(value2.X, value2.Y, value2.Z, value2.W));
};
FORCEINLINE constexpr FVector4 operator /(const DirectX::XMVECTOR& value1, const float& divider)
{
	return DirectX::XMVectorDivide(value1, DirectX::XMVectorSet(divider, divider, divider, divider));
};
#endif

__declspec(align(16)) class FColor
{
	typedef FColor Class;
	sStaticClassBody(Class)
public:
	static constexpr FColor Transparent()
	{
		return FColor::Zero();
	};

	static constexpr FColor Zero()
	{
		return FColor(0.0f, 0.0f, 0.0f, 0.0f);
	};

	static constexpr FColor Black()
	{
		return FColor(0.0f, 0.0f, 0.0f, 1.0f);
	};

	static constexpr FColor White()
	{
		return FColor(1.0f, 1.0f, 1.0f, 1.0f);
	};

	static constexpr FColor Red()
	{
		return FColor(1.0f, 0.0f, 0.0f, 1.0f);
	};

	static constexpr FColor Green()
	{
		return FColor(0.0f, 1.0f, 0.0f, 1.0f);
	};

	static constexpr FColor Blue()
	{
		return FColor(0.0f, 0.0f, 1.0f, 1.0f);
	};

	static constexpr FColor Grey()
	{
		return FColor(0.4f, 0.4f, 0.4f, 1.0f);
	};

	static FColor Random()
	{
		std::random_device rd; // obtain a random number from hardware
		std::mt19937 gen(rd()); // seed the generator
		std::uniform_real_distribution<> dist(0.0f, 1.0f); // define the range

		FColor RGB;
		RGB.R = static_cast<float>(dist(gen));
		RGB.G = static_cast<float>(dist(gen));
		RGB.B = static_cast<float>(dist(gen));

		return RGB;
	};

	/*
	* https://stackoverflow.com/questions/53879537/increase-the-intensity-of-texture-in-shader-code-opengl
	*/
	static FVector RGBtoHSV(FVector RGB)
	{
		const double Epsilon = 1e-10;
		FVector4  P = (RGB.Y < RGB.Z) ? FVector4(RGB.Z, RGB.Y, -1.0f, 2.0f / 3.0f) : FVector4(RGB.Y, RGB.Z, 0.0f, -1.0f / 3.0f);
		FVector4  Q = (RGB.X < P.X) ? FVector4(P.X, P.Y, P.W, RGB.X) : FVector4(RGB.X, P.Y, P.Z, P.X);
		float C = Q.X - std::min(Q.W, Q.Y);
		float H = abs((Q.W - Q.Y) / (6.0f * C + static_cast<float>(Epsilon)) + Q.Z);
		FVector  HCV = FVector(H, C, Q.X);
		float S = HCV.Y / (HCV.Z + static_cast<float>(Epsilon));
		return FVector(HCV.X, S, HCV.Z);
	}

	/*
	* https://stackoverflow.com/questions/53879537/increase-the-intensity-of-texture-in-shader-code-opengl
	*/
	static FVector HSVtoRGB(FVector HSV)
	{
		auto Clamp = [](const FVector& value1, const float min, const float max) -> FVector
		{
			auto lClamp = [](float value, float min, float max) -> float
			{
				value = (value > max) ? max : value;

				value = (value < min) ? min : value;

				return value;
			};

			return FVector(
				lClamp(value1.X, min, max),
				lClamp(value1.Y, min, max),
				lClamp(value1.Z, min, max)
			);
		};

		float H = HSV.X;
		float R = abs(H * 6.0f - 3.0f) - 1.0f;
		float G = 2.0f - abs(H * 6.0f - 2.0f);
		float B = 2.0f - abs(H * 6.0f - 4.0f);
		FVector RGB = Clamp(FVector(R, G, B), 0.0f, 1.0f);
		FVector Temp = FVector(RGB.X - 1.0f, RGB.Y - 1.0f, RGB.Z - 1.0f) * HSV.Y;
		Temp.X += 1.0f;
		Temp.Y += 1.0f;
		Temp.Z += 1.0f;

		return  Temp * HSV.Z;
	}

public:
	float R;
	float G;
	float B;
	float A;

public:
	FORCEINLINE constexpr FColor()
		: R(1.0f)
		, G(1.0f)
		, B(1.0f)
		, A(1.0f)
	{};

	FORCEINLINE constexpr FColor(const float r, const float g, const float b, const float& a = 1.0f)
		: R(r)
		, G(g)
		, B(b)
		, A(a)
	{};

	FORCEINLINE constexpr FColor(const std::int32_t r, const std::int32_t g, const std::int32_t b, const std::int32_t a = 255)
		: R(static_cast<float>(r) / 255.0f)
		, G(static_cast<float>(g) / 255.0f)
		, B(static_cast<float>(b) / 255.0f)
		, A(static_cast<float>(a) / 255.0f)
	{};

	FORCEINLINE constexpr FColor(const FColor& color, const float& alpha)
		: R(color.R)
		, G(color.G)
		, B(color.B)
		, A(alpha)
	{}

	FORCEINLINE constexpr FColor(const float& value)
		: R(value)
		, G(value)
		, B(value)
		, A(1.0f)
	{};

#if Enable_DirectX_Math
	FORCEINLINE constexpr FColor(const DirectX::XMVECTORF32& value)
		: R(value.f[0])
		, G(value.f[1])
		, B(value.f[2])
		, A(value.f[3])
	{}

	FORCEINLINE constexpr FColor(const DirectX::XMVECTOR& value)
		: R(value.m128_f32[0])
		, G(value.m128_f32[1])
		, B(value.m128_f32[2])
		, A(value.m128_f32[3])
	{}
#endif

	~FColor() = default;

	inline static constexpr FColor FromNonPremultiplied(const int& r, const int& g, const int& b, const int& a)
	{
		return FColor(
			(static_cast<float>(r) * static_cast<float>(a) / 255.0f),
			(static_cast<float>(g) * static_cast<float>(a) / 255.0f),
			(static_cast<float>(b) * static_cast<float>(a) / 255.0f),
			static_cast<float>(a)
		);
	}

	FORCEINLINE	FVector GetHSV() const
	{
		return RGBtoHSV(FVector(R, G, B));
	}

	FORCEINLINE void SetFromHSV(FVector HSV)
	{
		const FVector RGB = HSVtoRGB(HSV);

		R = RGB.X;
		G = RGB.Y;
		B = RGB.Z;
	}

	FORCEINLINE void SetHue(float Hue)
	{
		FVector HSV = GetHSV();
		HSV.X = Hue;
		const FVector RGB = HSVtoRGB(HSV);

		R = RGB.X;
		G = RGB.Y;
		B = RGB.Z;
	}
	FORCEINLINE void SetSaturation(float Saturation)
	{
		FVector HSV = GetHSV();
		HSV.Y = Saturation;
		const FVector RGB = HSVtoRGB(HSV);

		R = RGB.X;
		G = RGB.Y;
		B = RGB.Z;
	}
	FORCEINLINE void SetValue(float Value)
	{
		FVector HSV = GetHSV();
		HSV.Z = Value;
		const FVector RGB = HSVtoRGB(HSV);

		R = RGB.X;
		G = RGB.Y;
		B = RGB.Z;
	}

	FORCEINLINE constexpr FColor GetNegate()
	{
		return { 1.0f - R, 1.0f - G, 1.0f - B, A };
	};

	FORCEINLINE constexpr void Negate()
	{
		R = 1.0f - R;
		G = 1.0f - G;
		B = 1.0f - B;
	};

	FORCEINLINE constexpr void Saturate()
	{
		auto Clamp = [](FColor x, FColor min, FColor max) -> FColor
		{
			return x > min ? x < max ? x : max : min;
		};

		FColor Color = Clamp(*this, FColor::Zero(), FColor::White());
		R = Color.R;
		G = Color.G;
		B = Color.B;
	};

	FORCEINLINE constexpr FColor GetSaturate()
	{
		auto Clamp = [](FColor x, FColor min, FColor max) -> FColor
		{
			return x > min ? x < max ? x : max : min;
		};

		return Clamp(*this, FColor::Zero(), FColor::White());
	};

	FORCEINLINE constexpr void AdjustSaturation(float sat)
	{
		const FVector4 gvLuminance = FVector4(0.2125f, 0.7154f, 0.0721f, 0.0f);
		float fLuminance = (R * gvLuminance.X) + (G * gvLuminance.Y) + (B * gvLuminance.Z);
		FColor Color = *this;
		R = ((Color.R - fLuminance) * sat) + fLuminance;
		G = ((Color.G - fLuminance) * sat) + fLuminance;
		B = ((Color.B - fLuminance) * sat) + fLuminance;
		A = Color.A;
	};

	FORCEINLINE constexpr void AdjustContrast(float contrast)
	{
		FColor Color = *this;

		R = ((Color.R - 0.5f) * contrast) + 0.5f;
		G = ((Color.G - 0.5f) * contrast) + 0.5f;
		B = ((Color.B - 0.5f) * contrast) + 0.5f;
		A = Color.A;
	};

	FORCEINLINE constexpr float GetLuminance() const
	{
		return R * 0.3f + G * 0.59f + B * 0.11f;
	}

	FORCEINLINE constexpr std::string ToString() const
	{
		return std::string("{ R: " + std::to_string(R) + " G: " + std::to_string(G) + " B: " + std::to_string(B) + " Alpha: " + std::to_string(A) + " };");
	};

	FORCEINLINE FVector GetFVector() const
	{
		return FVector(R,G,B);
	}

#if Enable_DirectX_Math
	FORCEINLINE constexpr operator DirectX::XMVECTORF32() const
	{
		return { { { R, G, B, A } } };
	}

	FORCEINLINE constexpr operator DirectX::XMVECTORF32()
	{
		return { { { R, G, B, A } } };
	}

	FORCEINLINE operator DirectX::XMVECTOR() const
	{
		return DirectX::XMVectorSet(R, G, B, A);
	}

	FORCEINLINE operator DirectX::XMVECTOR()
	{
		return DirectX::XMVectorSet(R, G, B, A);
	}
#endif

#if _MSVC_LANG >= 202002L
	constexpr auto operator<=>(const FColor&) const = default;
#endif

#if _MSVC_LANG < 202002L
	inline constexpr bool operator ==(const FColor& b)
	{
		return (A == b.A &&
			R == b.R &&
			G == b.G &&
			B == b.B);
	}

	inline constexpr bool operator !=(const FColor& b)
	{
		return !(A == b.A)
			&& (R == b.R)
			&& (G == b.G)
			&& (B == b.B);
	}
#endif

	inline constexpr FColor operator *(const float& scale)
	{
		return FColor(
			(R * scale),
			(G * scale),
			(B * scale),
			(A * scale)
		);
	}
};

FORCEINLINE constexpr FColor operator *(const FColor& value, const float& scaleFactor)
{
	return FColor(value.R * scaleFactor, value.G * scaleFactor, value.B * scaleFactor, value.A/* * scaleFactor*/);
};

FORCEINLINE constexpr FColor operator *(const float& scaleFactor, const FColor& value)
{
	return FColor(value.R * scaleFactor, value.G * scaleFactor, value.B * scaleFactor, value.A/* * scaleFactor*/);
};

template <typename T>
struct TMatrix3x3
{
	union
	{
		std::array<TVector3<T>, 3> r;
		struct
		{
			T m00, m01, m02;
			T m10, m11, m12;
			T m20, m21, m22;
		};
		T m[3][3];
	};

	TMatrix3x3() 
	{}

	TMatrix3x3(const TMatrix3x3<T>& Other)
		: r(Other.r)
	{}

	TMatrix3x3(const T* v)
	{
		for (int i = 0; i < 3; ++i)
			for (int ii = 0; ii < 3; ++ii)
				m[i][ii] = v[i * 3 + ii];
	}

	constexpr TMatrix3x3(T a)
		: m00(a), m01(a), m02(a)
		, m10(a), m11(a), m12(a)
		, m20(a), m21(a), m22(a) 
	{}

	constexpr TMatrix3x3(T _m00, T _m01, T _m02, T _m10, T _m11, T _m12, T _m20, T _m21, T _m22)
		: m00(_m00), m01(_m01), m02(_m02)
		, m10(_m10), m11(_m11), m12(_m12)
		, m20(_m20), m21(_m21), m22(_m22) 
	{}

	constexpr TMatrix3x3(const TVector3<T>& _row0, const TVector3<T>& _row1, const TVector3<T>& _row2)
		: m00(_row0.X), m01(_row0.Y), m02(_row0.Z)
		, m10(_row1.X), m11(_row1.Y), m12(_row1.Z)
		, m20(_row2.X), m21(_row2.Y), m22(_row2.Z) 
	{}

	/*constexpr TMatrix3x3(const TMatrix<T, 3, 4>& m)
		: m00(m.m00), m01(m.m01), m02(m.m02)
		, m10(m.m10), m11(m.m11), m12(m.m12)
		, m20(m.m20), m21(m.m21), m22(m.m22) 
	{}

	constexpr TMatrix3x3(const matrix<T, 4, 4>& m)
		: m00(m.m00), m01(m.m01), m02(m.m02)
		, m10(m.m10), m11(m.m11), m12(m.m12)
		, m20(m.m20), m21(m.m21), m22(m.m22) 
	{}*/

	/*explicit constexpr TMatrix3x3(const TMatrix3x3<T>& Other)
	{
		for (int i = 0; i < 3; ++i)
			for (int ii = 0; ii < 3; ++ii)
				m[i][ii] = (T)(Other.m[i][ii]);
	}*/

	template<typename U>
	explicit constexpr TMatrix3x3(const TMatrix3x3<U>& Other)
	{
		for (int i = 0; i < 3; ++i)
			for (int ii = 0; ii < 3; ++ii)
				m[i][ii] = (T)(Other.m[i][ii]);
	}

	constexpr static TMatrix3x3 from_cols(const TVector3<T>& col0, const TVector3<T>& col1, const TVector3<T>& col2)
	{
		return TMatrix3x3(
			col0.X, col1.X, col2.X,
			col0.Y, col1.Y, col2.Y,
			col0.Z, col1.Z, col2.Z);
	}

	constexpr static TMatrix3x3 Diagonal(T diag)
	{
		return TMatrix3x3(
			diag, T(0), T(0),
			T(0), diag, T(0),
			T(0), T(0), diag);
	}

	constexpr static TMatrix3x3 Diagonal(TVector3<T> v)
	{
		return TMatrix3x3(
			v.X, T(0), T(0),
			T(0), v.Y, T(0),
			T(0), T(0), v.Z);
	}

	constexpr static TMatrix3x3 Identity()
	{
		return Diagonal(T(1));
	}

	constexpr static TMatrix3x3 Zero()
	{
		return TMatrix3x3(static_cast<T>(0));
	}

	TVector3<T> col(int j) const
	{
		const float m_data[9] = { m00, m01, m02, m10, m11, m12, m20, m21, m22 };
		TVector3<T> v; 
		for (int i = 0; i < 3; i++)
		{
			if (i == 0)
				v.X = m_data[i * 3 + j];
			else if (i == 1)
				v.Y = m_data[i * 3 + j];
			else if (i == 2)
				v.Z = m_data[i * 3 + j];
		}
		return v;
	}

	/* Subscript operators - built-in subscripts are ambiguous without these */
	TVector3<T>& operator [] (int i)
	{
		return reinterpret_cast<TVector3<T>&>(r[i * 3]);
	}
	const TVector3<T>& operator [] (int i) const
	{
		return reinterpret_cast<const TVector3<T>&>(r[i * 3]);
	}

	template <typename T>
	TMatrix3x3<T> operator * (const TMatrix3x3<T>& b) const
	{
		TMatrix3x3<T> result = TMatrix3x3<T>::Zero();
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j)
				for (int k = 0; k < 3; ++k)
					result.m[i][j] += m[i][k] * b.m[k][j];
		return result;
	}
};

__declspec(align(64)) class FMatrix
{
	typedef FMatrix Class;
	sStaticClassBody(Class)
public:
	FORCEINLINE static constexpr FMatrix Identity()
	{
		return FMatrix(1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
	}

	FORCEINLINE static constexpr FMatrix MatrixScaling(float ScaleX, float ScaleY, float ScaleZ) noexcept
	{
		FMatrix M;
		M.m[0][0] = ScaleX;
		M.m[0][1] = 0.0f;
		M.m[0][2] = 0.0f;
		M.m[0][3] = 0.0f;

		M.m[1][0] = 0.0f;
		M.m[1][1] = ScaleY;
		M.m[1][2] = 0.0f;
		M.m[1][3] = 0.0f;

		M.m[2][0] = 0.0f;
		M.m[2][1] = 0.0f;
		M.m[2][2] = ScaleZ;
		M.m[2][3] = 0.0f;

		M.m[3][0] = 0.0f;
		M.m[3][1] = 0.0f;
		M.m[3][2] = 0.0f;
		M.m[3][3] = 1.0f;
		return M;
	}

	FORCEINLINE static constexpr FMatrix MatrixTranslation(float OffsetX, float OffsetY, float OffsetZ) noexcept
	{
		FMatrix M;
		M.m[0][0] = 1.0f;
		M.m[0][1] = 0.0f;
		M.m[0][2] = 0.0f;
		M.m[0][3] = 0.0f;

		M.m[1][0] = 0.0f;
		M.m[1][1] = 1.0f;
		M.m[1][2] = 0.0f;
		M.m[1][3] = 0.0f;

		M.m[2][0] = 0.0f;
		M.m[2][1] = 0.0f;
		M.m[2][2] = 1.0f;
		M.m[2][3] = 0.0f;

		M.m[3][0] = OffsetX;
		M.m[3][1] = OffsetY;
		M.m[3][2] = OffsetZ;
		M.m[3][3] = 1.0f;
		return M;
	}

	FORCEINLINE static constexpr FMatrix Transpose(const FMatrix& matrix)
	{
		FMatrix ret;

		ret._11 = matrix._11;
		ret._12 = matrix._21;
		ret._13 = matrix._31;
		ret._14 = matrix._41;

		ret._21 = matrix._12;
		ret._22 = matrix._22;
		ret._23 = matrix._32;
		ret._24 = matrix._42;

		ret._31 = matrix._13;
		ret._32 = matrix._23;
		ret._33 = matrix._33;
		ret._34 = matrix._43;

		ret._41 = matrix._14;
		ret._42 = matrix._24;
		ret._43 = matrix._34;
		ret._44 = matrix._44;

		return ret;
	}

	FORCEINLINE constexpr FVector Backward()
	{
		return FVector(_31, _32, _33);
	}

	FORCEINLINE constexpr FVector Down()
	{
		return FVector(-_21, -_22, -_23);
	}

	FORCEINLINE constexpr FVector Forward()
	{
		return FVector(-_31, -_32, -_33);
	}

	FORCEINLINE constexpr FVector Left()
	{
		return FVector(-_11, -_12, -_13);
	}

	FORCEINLINE constexpr FVector Right()
	{
		return FVector(_11, _12, _13);
	}

	FORCEINLINE constexpr FVector Translation()
	{
		return FVector(_41, _42, _43);
	}

	FORCEINLINE constexpr FVector Up()
	{
		return FVector(_21, _22, _23);
	}

public:
	FORCEINLINE constexpr FMatrix() noexcept
		: _11(0.0f)
		, _12(0.0f)
		, _13(0.0f)
		, _14(0.0f)
		, _21(0.0f)
		, _22(0.0f)
		, _23(0.0f)
		, _24(0.0f)
		, _31(0.0f)
		, _32(0.0f)
		, _33(0.0f)
		, _34(0.0f)
		, _41(0.0f)
		, _42(0.0f)
		, _43(0.0f)
		, _44(0.0f)
	{}

	FORCEINLINE constexpr FMatrix(const float& f11, const float& f12, const float& f13, const float& f14,
		const float& f21, const float& f22, const float& f23, const float& f24,
		const float& f31, const float& f32, const float& f33, const float& f34,
		const float& f41, const float& f42, const float& f43, const float& f44) noexcept
		: _11(f11)
		, _12(f12)
		, _13(f13)
		, _14(f14)
		, _21(f21)
		, _22(f22)
		, _23(f23)
		, _24(f24)
		, _31(f31)
		, _32(f32)
		, _33(f33)
		, _34(f34)
		, _41(f41)
		, _42(f42)
		, _43(f43)
		, _44(f44)
	{}

	FORCEINLINE constexpr FMatrix(const FMatrix& Other) noexcept
		: _11(Other._11)
		, _12(Other._12)
		, _13(Other._13)
		, _14(Other._14)
		, _21(Other._21)
		, _22(Other._22)
		, _23(Other._23)
		, _24(Other._24)
		, _31(Other._31)
		, _32(Other._32)
		, _33(Other._33)
		, _34(Other._34)
		, _41(Other._41)
		, _42(Other._42)
		, _43(Other._43)
		, _44(Other._44)
	{}
	FORCEINLINE constexpr FMatrix(const FVector4& Vec0, const FVector4& Vec1, const FVector4& Vec2, const FVector4& Vec3) noexcept
	{
		r[0] = Vec0;
		r[1] = Vec1;
		r[2] = Vec2;
		r[3] = Vec3;
	}

	FORCEINLINE constexpr FMatrix(const FVector& Vec0, const FVector& Vec1, const FVector& Vec2, const FVector& Vec3)
	{
		r[0] = FVector4(Vec0.X, Vec0.Y, Vec0.Z, 0.0f);
		r[1] = FVector4(Vec1.X, Vec1.Y, Vec1.Z, 0.0f);
		r[2] = FVector4(Vec2.X, Vec2.Y, Vec2.Z, 0.0f);
		r[3] = FVector4(Vec3.X, Vec3.Y, Vec3.Z, 0.0f);
	}

	FORCEINLINE constexpr FMatrix(const FRotationMatrix& RotMat, const FVector& W);

#if Enable_DirectX_Math
	FORCEINLINE constexpr FMatrix(const DirectX::XMFLOAT3X3& Matrix)
		: _11(Matrix._11)
		, _12(Matrix._12)
		, _13(Matrix._13)
		, _14(0.0f)
		, _21(Matrix._21)
		, _22(Matrix._22)
		, _23(Matrix._23)
		, _24(0.0f)
		, _31(Matrix._31)
		, _32(Matrix._32)
		, _33(Matrix._33)
		, _34(0.0f)
		, _41(0.0f)
		, _42(0.0f)
		, _43(0.0f)
		, _44(0.0f)
	{}
	FORCEINLINE constexpr FMatrix(const DirectX::XMFLOAT3X4& Matrix)
		: _11(Matrix._11)
		, _12(Matrix._12)
		, _13(Matrix._13)
		, _14(Matrix._14)
		, _21(Matrix._21)
		, _22(Matrix._22)
		, _23(Matrix._23)
		, _24(Matrix._24)
		, _31(Matrix._31)
		, _32(Matrix._32)
		, _33(Matrix._33)
		, _34(Matrix._34)
		, _41(0.0f)
		, _42(0.0f)
		, _43(0.0f)
		, _44(0.0f)
	{}
	FORCEINLINE constexpr FMatrix(const DirectX::XMFLOAT3X4A& Matrix)
		: _11(Matrix._11)
		, _12(Matrix._12)
		, _13(Matrix._13)
		, _14(Matrix._14)
		, _21(Matrix._21)
		, _22(Matrix._22)
		, _23(Matrix._23)
		, _24(Matrix._24)
		, _31(Matrix._31)
		, _32(Matrix._32)
		, _33(Matrix._33)
		, _34(Matrix._34)
		, _41(0.0f)
		, _42(0.0f)
		, _43(0.0f)
		, _44(0.0f)
	{}
	FORCEINLINE constexpr FMatrix(const DirectX::XMFLOAT4X3& Matrix)
		: _11(Matrix._11)
		, _12(Matrix._12)
		, _13(Matrix._13)
		, _14(0.0f)
		, _21(Matrix._21)
		, _22(Matrix._22)
		, _23(Matrix._23)
		, _24(0.0f)
		, _31(Matrix._31)
		, _32(Matrix._32)
		, _33(Matrix._33)
		, _34(0.0f)
		, _41(Matrix._41)
		, _42(Matrix._42)
		, _43(Matrix._43)
		, _44(0.0f)
	{}
	FORCEINLINE constexpr FMatrix(const DirectX::XMFLOAT4X3A& Matrix)
		: _11(Matrix._11)
		, _12(Matrix._12)
		, _13(Matrix._13)
		, _14(0.0f)
		, _21(Matrix._21)
		, _22(Matrix._22)
		, _23(Matrix._23)
		, _24(0.0f)
		, _31(Matrix._31)
		, _32(Matrix._32)
		, _33(Matrix._33)
		, _34(0.0f)
		, _41(Matrix._41)
		, _42(Matrix._42)
		, _43(Matrix._43)
		, _44(0.0f)
	{}
	FORCEINLINE constexpr FMatrix(const DirectX::XMFLOAT4X4& Matrix)
		: _11(Matrix._11)
		, _12(Matrix._12)
		, _13(Matrix._13)
		, _14(Matrix._14)
		, _21(Matrix._21)
		, _22(Matrix._22)
		, _23(Matrix._23)
		, _24(Matrix._24)
		, _31(Matrix._31)
		, _32(Matrix._32)
		, _33(Matrix._33)
		, _34(Matrix._34)
		, _41(Matrix._41)
		, _42(Matrix._42)
		, _43(Matrix._43)
		, _44(Matrix._44)
	{}
	FORCEINLINE constexpr FMatrix(const DirectX::XMFLOAT4X4A& Matrix)
		: _11(Matrix._11)
		, _12(Matrix._12)
		, _13(Matrix._13)
		, _14(Matrix._14)
		, _21(Matrix._21)
		, _22(Matrix._22)
		, _23(Matrix._23)
		, _24(Matrix._24)
		, _31(Matrix._31)
		, _32(Matrix._32)
		, _33(Matrix._33)
		, _34(Matrix._34)
		, _41(Matrix._41)
		, _42(Matrix._42)
		, _43(Matrix._43)
		, _44(Matrix._44)
	{}
	FORCEINLINE constexpr FMatrix(const DirectX::XMMATRIX& Matrix)
	{
		r[0] = Matrix.r[0];
		r[1] = Matrix.r[1];
		r[2] = Matrix.r[2];
		r[3] = Matrix.r[3];
	}
#endif
	~FMatrix() = default;

	union
	{
		std::array<FVector4, 4> r;
		struct
		{
			float _11, _12, _13, _14;
			float _21, _22, _23, _24;
			float _31, _32, _33, _34;
			float _41, _42, _43, _44;
		};
		float m[4][4];
	};

	inline constexpr FVector GetOrigin() const
	{
		return FVector(m[3][0], m[3][1], m[3][2]);
	}

	// casting operators
	constexpr operator float* ()
	{
		return (float*)&_11;
	}

	constexpr operator const float* () const
	{
		return (const float*)&_11;
	}

#if Enable_DirectX_Math
	FORCEINLINE operator DirectX::XMFLOAT3X3() const
	{
		return DirectX::XMFLOAT3X3(_11, _12, _13,
			_21, _22, _23,
			_31, _32, _33);
	}
	FORCEINLINE operator DirectX::XMFLOAT3X3()
	{
		return DirectX::XMFLOAT3X3(_11, _12, _13,
			_21, _22, _23,
			_31, _32, _33);
	}
	FORCEINLINE operator DirectX::XMFLOAT4X4() const
	{
		return DirectX::XMFLOAT4X4(_11, _12, _13, _14,
			_21, _22, _23, _24,
			_31, _32, _33, _34,
			_41, _42, _43, _44);
	}
	FORCEINLINE operator DirectX::XMFLOAT4X4()
	{
		return DirectX::XMFLOAT4X4(_11, _12, _13, _14,
			_21, _22, _23, _24,
			_31, _32, _33, _34,
			_41, _42, _43, _44);
	}

	FORCEINLINE operator DirectX::XMMATRIX() const
	{
		return DirectX::XMMATRIX(_11, _12, _13, _14,
			_21, _22, _23, _24,
			_31, _32, _33, _34,
			_41, _42, _43, _44);
	}

	FORCEINLINE operator DirectX::XMMATRIX()
	{
		return DirectX::XMMATRIX(r[0], r[1], r[2], r[3]);
	}
#endif

#if Enable_DirectX_Math
	static FORCEINLINE FMatrix MakeXRotation(const float angle)
	{
		return DirectX::XMMatrixRotationX(angle);
	}

	static FORCEINLINE FMatrix MakeYRotation(const float angle)
	{
		return DirectX::XMMatrixRotationY(angle);
	}

	static FORCEINLINE FMatrix MakeZRotation(const float angle)
	{
		return DirectX::XMMatrixRotationZ(angle);
	}

	static FORCEINLINE FMatrix MakeScale(const float scale)
	{
		return DirectX::XMMatrixScaling(scale, scale, scale);
	}

	static FORCEINLINE FMatrix MakeScale(const float sx, const float sy, const float sz)
	{
		return DirectX::XMMatrixScaling(sx, sy, sz);
	}

	static FORCEINLINE FMatrix MakeScale(const FVector& scale)
	{
		return DirectX::XMMatrixScalingFromVector(scale);
	}
#endif


#if Enable_DirectX_Math
	FORCEINLINE FVector operator* (const FVector& vec) const
	{
		return FVector(DirectX::XMVector3TransformNormal(vec, *this));
	}
#endif
	// assignment operators
	/*inline FMatrix& operator = (const FMatrix& Matrix)
	{
		r[0] = Matrix.r[0];
		r[1] = Matrix.r[1];
		r[2] = Matrix.r[2];
		r[3] = Matrix.r[3];

		return *this;
	}*/
	inline FMatrix& operator *= (const FMatrix& Matrix)
	{
		const FMatrix& Mat = DirectX::XMMatrixMultiply(*this, Matrix);

		r[0] = Mat.r[0];
		r[1] = Mat.r[1];
		r[2] = Mat.r[2];
		r[3] = Mat.r[3];

		return *this;
	}
	inline constexpr FMatrix& operator += (const FMatrix& mat)
	{
		_11 += mat._11; _12 += mat._12; _13 += mat._13; _14 += mat._14;
		_21 += mat._21; _22 += mat._22; _23 += mat._23; _24 += mat._24;
		_31 += mat._31; _32 += mat._32; _33 += mat._33; _34 += mat._34;
		_41 += mat._41; _42 += mat._42; _43 += mat._43; _44 += mat._44;
		return *this;
	}

	inline constexpr FMatrix& operator -= (const FMatrix& mat)
	{
		_11 -= mat._11; _12 -= mat._12; _13 -= mat._13; _14 -= mat._14;
		_21 -= mat._21; _22 -= mat._22; _23 -= mat._23; _24 -= mat._24;
		_31 -= mat._31; _32 -= mat._32; _33 -= mat._33; _34 -= mat._34;
		_41 -= mat._41; _42 -= mat._42; _43 -= mat._43; _44 -= mat._44;
		return *this;
	}
	inline constexpr FMatrix& operator *= (const float& f)
	{
		_11 *= f; _12 *= f; _13 *= f; _14 *= f;
		_21 *= f; _22 *= f; _23 *= f; _24 *= f;
		_31 *= f; _32 *= f; _33 *= f; _34 *= f;
		_41 *= f; _42 *= f; _43 *= f; _44 *= f;
		return *this;
	}
	inline constexpr FMatrix& operator /= (const float& f)
	{
		float fInv = 1.0f / f;
		_11 *= fInv; _12 *= fInv; _13 *= fInv; _14 *= fInv;
		_21 *= fInv; _22 *= fInv; _23 *= fInv; _24 *= fInv;
		_31 *= fInv; _32 *= fInv; _33 *= fInv; _34 *= fInv;
		_41 *= fInv; _42 *= fInv; _43 *= fInv; _44 *= fInv;
		return *this;
	}

	inline FMatrix operator - () const
	{
		return FMatrix(-_11, -_12, -_13, -_14,
			-_21, -_22, -_23, -_24,
			-_31, -_32, -_33, -_34,
			-_41, -_42, -_43, -_44);
	}

	// binary operators
	/*inline constexpr FMatrix operator * (const FMatrix& Mat)
	{
		return DirectX::XMMatrixMultiply(*this, Mat);
	}*/

	inline constexpr FMatrix operator + (const FMatrix& mat) const
	{
		return FMatrix(_11 + mat._11, _12 + mat._12, _13 + mat._13, _14 + mat._14,
			_21 + mat._21, _22 + mat._22, _23 + mat._23, _24 + mat._24,
			_31 + mat._31, _32 + mat._32, _33 + mat._33, _34 + mat._34,
			_41 + mat._41, _42 + mat._42, _43 + mat._43, _44 + mat._44);
	}
	inline constexpr FMatrix operator - (const FMatrix& mat) const
	{
		return FMatrix(_11 - mat._11, _12 - mat._12, _13 - mat._13, _14 - mat._14,
			_21 - mat._21, _22 - mat._22, _23 - mat._23, _24 - mat._24,
			_31 - mat._31, _32 - mat._32, _33 - mat._33, _34 - mat._34,
			_41 - mat._41, _42 - mat._42, _43 - mat._43, _44 - mat._44);
	}
	inline constexpr FMatrix operator * (const float& f) const
	{
		return FMatrix(_11 * f, _12 * f, _13 * f, _14 * f,
			_21 * f, _22 * f, _23 * f, _24 * f,
			_31 * f, _32 * f, _33 * f, _34 * f,
			_41 * f, _42 * f, _43 * f, _44 * f);
	}
	inline constexpr FMatrix operator / (const float& f) const
	{
		float fInv = 1.0f / f;
		return FMatrix(_11 * fInv, _12 * fInv, _13 * fInv, _14 * fInv,
			_21 * fInv, _22 * fInv, _23 * fInv, _24 * fInv,
			_31 * fInv, _32 * fInv, _33 * fInv, _34 * fInv,
			_41 * fInv, _42 * fInv, _43 * fInv, _44 * fInv);
	}

#if _MSVC_LANG >= 202002L
	auto operator<=>(const FMatrix&) const = default;
#endif

#if _MSVC_LANG < 202002L
	inline constexpr bool operator == (const FMatrix& mat) const
	{
		return 0 == memcmp(this, &mat, sizeof(FMatrix));
	}
	inline constexpr bool operator != (const FMatrix& mat) const
	{
		return 0 != memcmp(this, &mat, sizeof(FMatrix));
	}
#endif
};

inline constexpr FMatrix operator * (const float& f, const FMatrix& mat)
{
	return FMatrix(f * mat._11, f * mat._12, f * mat._13, f * mat._14,
		f * mat._21, f * mat._22, f * mat._23, f * mat._24,
		f * mat._31, f * mat._32, f * mat._33, f * mat._34,
		f * mat._41, f * mat._42, f * mat._43, f * mat._44);
}

#if Enable_DirectX_Math
/*FORCEINLINE constexpr FMatrix operator *(const FMatrix& value1, const DirectX::XMMATRIX& value2)
{
	return DirectX::XMMatrixMultiply(value1, value2);
}
FORCEINLINE constexpr FMatrix operator *(const DirectX::XMMATRIX& value1, const FMatrix& value2)
{
	return DirectX::XMMatrixMultiply(value1, value2);
}*/
FORCEINLINE constexpr FMatrix operator *(const FMatrix& value1, const FMatrix& value2)
{
	return DirectX::XMMatrixMultiply(value1, value2);
}
#endif

class FRotationMatrix
{
	typedef FRotationMatrix Class;
	sStaticClassBody(Class)
public:
	FORCEINLINE static constexpr FRotationMatrix Identity()
	{
		return FRotationMatrix(1.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 1.0f);
	}

	FORCEINLINE static FRotationMatrix CreateRotationX(const float radians)
	{
#if Enable_DirectX_Math
		return FRotationMatrix(DirectX::XMMatrixRotationX(radians));
#else
		FRotationMatrix result = FRotationMatrix::Identity();

		const float val1 = (float)std::cos(radians);
		const float val2 = (float)std::sin(radians);

		result._22 = val1;
		result._23 = val2;
		result._32 = -val2;
		result._33 = val1;

		return result;
#endif
	}

	FORCEINLINE static FRotationMatrix CreateRotationY(const float radians)
	{
#if Enable_DirectX_Math
		return FRotationMatrix(DirectX::XMMatrixRotationY(radians));
#else
		FRotationMatrix result = FRotationMatrix::Identity();

		const float val1 = (float)std::cos(radians);
		const float val2 = (float)std::sin(radians);

		result._11 = val1;
		result._13 = -val2;
		result._31 = val2;
		result._33 = val1;

		return result;
#endif
	}

	FORCEINLINE static FRotationMatrix CreateRotationZ(const float radians)
	{
#if Enable_DirectX_Math
		return FRotationMatrix(DirectX::XMMatrixRotationZ(radians));
#else
		FRotationMatrix result = FRotationMatrix::Identity();

		const float val1 = (float)std::cos(radians);
		const float val2 = (float)std::sin(radians);

		result._11 = val1;
		result._12 = val2;
		result._21 = -val2;
		result._22 = val1;

		return result;
#endif
	}

	FORCEINLINE static FRotationMatrix MakeScale(const float scale)
	{
		return FRotationMatrix(DirectX::XMMatrixScaling(scale, scale, scale));
	}

	FORCEINLINE static FRotationMatrix MakeScale(const float sx, const float sy, const float sz)
	{
		return FRotationMatrix(DirectX::XMMatrixScaling(sx, sy, sz));
	}

	FORCEINLINE static FRotationMatrix MakeScale(const FVector& scale)
	{
		return FRotationMatrix(DirectX::XMMatrixScalingFromVector(scale));
	}

public:
	FORCEINLINE constexpr FRotationMatrix() noexcept
		: _11(0.0f)
		, _12(0.0f)
		, _13(0.0f)
		, _21(0.0f)
		, _22(0.0f)
		, _23(0.0f)
		, _31(0.0f)
		, _32(0.0f)
		, _33(0.0f)
	{}

	FORCEINLINE constexpr FRotationMatrix(const FRotationMatrix& Other) noexcept
		: _11(Other._11)
		, _12(Other._12)
		, _13(Other._13)
		, _21(Other._21)
		, _22(Other._22)
		, _23(Other._23)
		, _31(Other._31)
		, _32(Other._32)
		, _33(Other._33)
	{}

	FORCEINLINE constexpr FRotationMatrix(const FMatrix& Other) noexcept
		: _11(Other._11)
		, _12(Other._12)
		, _13(Other._13)
		, _21(Other._21)
		, _22(Other._22)
		, _23(Other._23)
		, _31(Other._31)
		, _32(Other._32)
		, _33(Other._33)
	{}

#if Enable_DirectX_Math
	FORCEINLINE constexpr FRotationMatrix(const DirectX::XMMATRIX& Matrix) noexcept
	{
		r[0] = FVector(Matrix.r[0].m128_f32[0], Matrix.r[0].m128_f32[1], Matrix.r[0].m128_f32[2]);
		r[1] = FVector(Matrix.r[1].m128_f32[0], Matrix.r[1].m128_f32[1], Matrix.r[1].m128_f32[2]);
		r[2] = FVector(Matrix.r[2].m128_f32[0], Matrix.r[2].m128_f32[1], Matrix.r[2].m128_f32[2]);
	}
#endif

	FORCEINLINE constexpr FRotationMatrix(const float& f11, const float& f12, const float& f13,
		const float& f21, const float& f22, const float& f23,
		const float& f31, const float& f32, const float& f33) noexcept
		: _11(f11)
		, _12(f12)
		, _13(f13)
		, _21(f21)
		, _22(f22)
		, _23(f23)
		, _31(f31)
		, _32(f32)
		, _33(f33)
	{}

	FORCEINLINE constexpr FRotationMatrix(const FVector& Vec0, const FVector& Vec1, const FVector& Vec2)
	{
		r[0] = Vec0;
		r[1] = Vec1;
		r[2] = Vec2;
	}

	FORCEINLINE FRotationMatrix(const float Roll, const float Pitch, const float Yaw)
	{
		const float SP = std::sin(DirectX::XMConvertToRadians(Pitch));
		const float CP = std::cos(DirectX::XMConvertToRadians(Pitch));
		const float SY = std::sin(DirectX::XMConvertToRadians(Yaw));
		const float CY = std::cos(DirectX::XMConvertToRadians(Yaw));
		const float SR = std::sin(DirectX::XMConvertToRadians(Roll));
		const float CR = std::cos(DirectX::XMConvertToRadians(Roll));

		r[0].X = CP * CY;
		r[0].Y = CP * SY;
		r[0].Z = SP;
		//r[0].W = 0.f;

		r[1].X = SR * SP * CY - CR * SY;
		r[1].Y = SR * SP * SY + CR * CY;
		r[1].Z = -SR * CP;
		//r[1].W = 0.f;

		r[2].X = -(CR * SP * CY + SR * SY);
		r[2].Y = CY * SR - CR * SP * SY;
		r[2].Z = CR * CP;
		//r[2].W = 0.f;

		//r[3].X = 0.0f;
		//r[3].Y = 0.0f;
		//r[3].Z = 0.0f;
		//r[3].W = 1.f;
	}

	FORCEINLINE constexpr FRotationMatrix(const FQuaternion& q);

	~FRotationMatrix() = default;

	union
	{
		std::array<FVector, 3> r;
		struct
		{
			float _11, _12, _13;
			float _21, _22, _23;
			float _31, _32, _33;
		};
		float m[3][3];
	};

	FORCEINLINE FAngles GetAngles() const;

	FORCEINLINE float Roll() const
	{
		return atan2f(r[0].Y, r[1].Y);
	}

#if Enable_DirectX_Math
	FORCEINLINE operator DirectX::XMFLOAT3X3() const
	{
		return DirectX::XMFLOAT3X3(_11, _12, _13,
			_21, _22, _23,
			_31, _32, _33);
	}
	FORCEINLINE operator DirectX::XMFLOAT3X3()
	{
		return DirectX::XMFLOAT3X3(_11, _12, _13,
			_21, _22, _23,
			_31, _32, _33);
	}

	FORCEINLINE operator DirectX::XMMATRIX() const
	{
		return DirectX::XMMATRIX(_11, _12, _13, 0.0f,
			_21, _22, _23, 0.0f,
			_31, _32, _33, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f);
	}

	FORCEINLINE operator DirectX::XMMATRIX()
	{
		return DirectX::XMMATRIX(FVector4(r[0].X, r[0].Y, r[0].Z, 0.0f), FVector4(r[1].X, r[1].Y, r[1].Z, 0.0f),
			FVector4(r[2].X, r[2].Y, r[2].Z, 0.0f), FVector4::Zero());
	}

	FORCEINLINE operator FMatrix() const
	{
		return FMatrix(_11, _12, _13, 0.0f,
			_21, _22, _23, 0.0f,
			_31, _32, _33, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f);
	}

	FORCEINLINE operator FMatrix()
	{
		return FMatrix(FVector4(r[0].X, r[0].Y, r[0].Z, 0.0f), FVector4(r[1].X, r[1].Y, r[1].Z, 0.0f),
			FVector4(r[2].X, r[2].Y, r[2].Z, 0.0f), FVector4::Zero());
	}
#endif

	FORCEINLINE FRotationMatrix operator* (const float scl) const
	{
		return FRotationMatrix(scl * r[0], scl * r[1], scl * r[2]);
	}

	FORCEINLINE FVector operator* (const FVector& vec) const
	{
		return FVector(DirectX::XMVector3TransformNormal(vec, *this));
	}

	FORCEINLINE FRotationMatrix operator* (const FRotationMatrix& mat) const
	{
		return FRotationMatrix(*this * mat.r[0], *this * mat.r[1], *this * mat.r[2]);
	}
};

FORCEINLINE constexpr FMatrix::FMatrix(const FRotationMatrix& RotMat, const FVector& W)
{
	r[0] = FVector4(RotMat.r[0].X, RotMat.r[0].Y, RotMat.r[0].Z, 0.0f);
	r[1] = FVector4(RotMat.r[1].X, RotMat.r[1].Y, RotMat.r[1].Z, 0.0f);
	r[2] = FVector4(RotMat.r[2].X, RotMat.r[2].Y, RotMat.r[2].Z, 0.0f);
	r[3] = FVector4(W);
}

class FAngles
{
	typedef FAngles Class;
	sStaticClassBody(Class)
public:
	FORCEINLINE constexpr FAngles()
		: Pitch(0.0f)
		, Yaw(0.0f)
		, Roll(0.0f)
	{}
	FORCEINLINE constexpr FAngles(const FAngles& Other)
		: Pitch(Other.Pitch)
		, Yaw(Other.Yaw)
		, Roll(Other.Roll)
	{}
	FORCEINLINE constexpr FAngles(const float pitch, const float yaw, const float roll)
		: Pitch(pitch)
		, Yaw(yaw)
		, Roll(roll)
	{}
	FORCEINLINE constexpr FAngles(const float roll)
		: Pitch(0.0f)
		, Yaw(0.0f)
		, Roll(roll)
	{}

	~FAngles() = default;

	union
	{
		FVector Angles;
		struct
		{
			float Pitch;
			float Yaw;
			float Roll;

		};
	};

	FORCEINLINE constexpr FVector GetRadians() const
	{
		return FVector(Pitch * (fPI / 180.0f), Yaw * (fPI / 180), Roll * (fPI / 180));
	}

	FORCEINLINE constexpr operator FVector() const
	{
		return Angles;
	}
	FORCEINLINE constexpr operator FVector()
	{
		return Angles;
	}

	FORCEINLINE operator FQuaternion() const;
	FORCEINLINE operator FQuaternion();

	FORCEINLINE constexpr std::string ToString() const
	{
		return std::string("{ Pitch: " + std::to_string(Pitch) + " Yaw: " + std::to_string(Yaw) + " Roll: " + std::to_string(Roll) + " };");
	};
};

FORCEINLINE FAngles FRotationMatrix::GetAngles() const
{
	FAngles	Angle = FAngles(
		std::atan2(r[0].Z, std::sqrt(Square(r[0].X) + Square(r[0].Y))) * 180.f / fPI,
		std::atan2(r[0].Y, r[0].X) * 180.f / fPI,
		0
	);

	FRotationMatrix RotationMatrix = FRotationMatrix(Angle.Roll, Angle.Pitch, Angle.Yaw);

	Angle.Roll = std::atan2(r[2].X * RotationMatrix.r[1].X + r[2].Y * RotationMatrix.r[1].Y + r[2].Z * RotationMatrix.r[1].Z,
		r[1].X * RotationMatrix.r[1].X + r[1].Y * RotationMatrix.r[1].Y + r[1].Z * RotationMatrix.r[1].Z) * 180.f / fPI;

	return Angle;
};

class FRotatedVector
{
	typedef FRotatedVector Class;
	sStaticClassBody(Class)
private:
	float Angle;
	FVector Axis;

public:
	FORCEINLINE static FRotatedVector MakeXRotation(const float angle)
	{
		return FRotatedVector(angle, FVector::UnitX());
	}

	FORCEINLINE static FRotatedVector MakeYRotation(const float angle)
	{
		return FRotatedVector(angle, FVector::UnitY());
	}

	FORCEINLINE static FRotatedVector MakeZRotation(const float angle)
	{
		return FRotatedVector(angle, FVector::UnitZ());
	}

public:
	FORCEINLINE constexpr FRotatedVector()
		: Angle(0.0f)
		, Axis(FVector::Zero())
	{}
	FORCEINLINE constexpr FRotatedVector(const FRotatedVector& Other)
		: Angle(Other.Angle)
		, Axis(Other.Axis)
	{}
	FORCEINLINE constexpr FRotatedVector(const float& InAngle, const FVector& InAxis)
		: Angle(InAngle)
		, Axis(InAxis)
	{}

	~FRotatedVector() = default;

	FORCEINLINE constexpr float GetAngle() const { return Angle; }
	FORCEINLINE constexpr FVector GetAxis() const { return Axis; }

	FORCEINLINE constexpr FRotationMatrix ToRotationMatrix() const
	{
		return DirectX::XMMatrixRotationAxis(Axis, Angle);
	}
	FORCEINLINE constexpr FVector4 ToQuaternion() const
	{
		return DirectX::XMQuaternionRotationAxis(Axis, Angle);
	}

	FORCEINLINE constexpr operator FRotationMatrix() const
	{
		return DirectX::XMMatrixRotationAxis(Axis, Angle);
	}
	FORCEINLINE constexpr operator FRotationMatrix()
	{
		return DirectX::XMMatrixRotationAxis(Axis, Angle);
	}

	FORCEINLINE constexpr operator FQuaternion() const;
	FORCEINLINE constexpr operator FQuaternion();
};

class FQuaternion
{
	typedef FQuaternion Class;
	sStaticClassBody(Class)
public:
	FORCEINLINE static constexpr FQuaternion Identity()
	{
		return FQuaternion(DirectX::XMQuaternionIdentity());
	}

	FORCEINLINE static FQuaternion MakeXRotation(const float angle)
	{
		return FQuaternion(FRotatedVector(angle, FVector::UnitX()));
	}

	FORCEINLINE static FQuaternion MakeYRotation(const float angle)
	{
		return FQuaternion(FRotatedVector(angle, FVector::UnitY()));
	}

	FORCEINLINE static FQuaternion MakeZRotation(const float angle)
	{
		return FQuaternion(FRotatedVector(angle, FVector::UnitZ()));
	}

	FORCEINLINE static FVector4 QuaternionRotationRollPitchYaw(const float Pitch, const float Yaw, const float Roll)
	{
		//return DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(Pitch), DirectX::XMConvertToRadians(Yaw), DirectX::XMConvertToRadians(Roll));

		const float SP = std::sin(std::fmod(Pitch, 360.0f) * ((fPI / (180.f)) / 2.0f));
		const float CP = std::cos(std::fmod(Pitch, 360.0f) * ((fPI / (180.f)) / 2.0f));
		const float SY = std::sin(std::fmod(Yaw, 360.0f) * ((fPI / (180.f)) / 2.0f));
		const float CY = std::cos(std::fmod(Yaw, 360.0f) * ((fPI / (180.f)) / 2.0f));
		const float SR = std::sin(std::fmod(Roll, 360.0f) * ((fPI / (180.f)) / 2.0f));
		const float CR = std::cos(std::fmod(Roll, 360.0f) * ((fPI / (180.f)) / 2.0f));

		FVector4 Quat;
		Quat.X = CR * SP * SY - SR * CP * CY;
		Quat.Y = -CR * SP * CY - SR * CP * SY;
		Quat.Z = CR * CP * SY - SR * SP * CY;
		Quat.W = CR * CP * CY + SR * SP * SY;

		return Quat;
	}

public:
	FORCEINLINE constexpr FQuaternion()
		: m_vec(DirectX::XMQuaternionIdentity())
	{}
	FORCEINLINE constexpr FQuaternion(const FQuaternion& Other)
		: m_vec(Other.m_vec)
	{}
	FORCEINLINE constexpr FQuaternion(const FRotatedVector& RotatedVector)
		: m_vec(DirectX::XMQuaternionRotationAxis(RotatedVector.GetAxis(), RotatedVector.GetAngle()))
	{}
	FORCEINLINE FQuaternion(const FAngles& Angle)
		: m_vec(QuaternionRotationRollPitchYaw(Angle.Pitch, Angle.Yaw, Angle.Roll))
	{}
	FORCEINLINE constexpr FQuaternion(const FMatrix& matrix)
		: m_vec(DirectX::XMQuaternionRotationMatrix(matrix))
	{}
	FORCEINLINE constexpr FQuaternion(const FRotationMatrix& matrix)
		: m_vec(DirectX::XMQuaternionRotationMatrix(matrix))
	{}
	FORCEINLINE constexpr FQuaternion(const float InX, const float InY, const float InZ, const float InW)
		: m_vec(FVector4(InX, InY, InZ, InW))
	{}
	FORCEINLINE constexpr FQuaternion(const FVector4& vec)
		: m_vec(vec)
	{}
	FORCEINLINE constexpr FQuaternion(const float* v)
		: m_vec(FVector4(v[0], v[1], v[2], v[3]))
	{}


	~FQuaternion() = default;

	union
	{
		FVector4 m_vec;
		struct
		{
			float X, Y, Z, W;
		};
	};

	FAngles GetAngles() const
	{
		return ToRotationMatrix().GetAngles();
	}

	FORCEINLINE constexpr std::string ToString() const
	{
		return "FQuaternion::" + m_vec.ToString();
	};

	FORCEINLINE constexpr operator FVector4() const
	{
		return m_vec;
	}
	FORCEINLINE constexpr operator FVector4()
	{
		return m_vec;
	}

#if Enable_DirectX_Math
	FORCEINLINE constexpr operator DirectX::XMFLOAT4() const
	{
		return DirectX::XMFLOAT4(X, Y, Z, W);
	}
	FORCEINLINE constexpr operator DirectX::XMFLOAT4()
	{
		return DirectX::XMFLOAT4(X, Y, Z, W);
	}

	FORCEINLINE constexpr operator DirectX::XMFLOAT4A() const
	{
		return DirectX::XMFLOAT4A(X, Y, Z, W);
	}
	FORCEINLINE constexpr operator DirectX::XMFLOAT4A()
	{
		return DirectX::XMFLOAT4A(X, Y, Z, W);
	}

	FORCEINLINE operator DirectX::XMVECTOR() const
	{
		return DirectX::XMVectorSet(X, Y, Z, W);
	}
	FORCEINLINE operator DirectX::XMVECTOR()
	{
		return DirectX::XMVectorSet(X, Y, Z, W);
	}
#endif

	FORCEINLINE constexpr FQuaternion Conjugate() const
	{
		return FQuaternion(DirectX::XMQuaternionConjugate(m_vec));
	}
	FORCEINLINE constexpr FQuaternion GetNegate() const
	{
		return FQuaternion(DirectX::XMVectorNegate(m_vec));
	}

	FORCEINLINE constexpr FRotationMatrix ToRotationMatrix() const
	{
		return DirectX::XMMatrixRotationQuaternion(m_vec);
	}

	// assignment operators
	FORCEINLINE constexpr FQuaternion& operator += (const FQuaternion& q)
	{
		X += q.X;
		Y += q.Y;
		Z += q.Z;
		W += q.W;
		return *this;
	}

	FORCEINLINE FQuaternion& operator -= (const FQuaternion& q)
	{
		X -= q.X;
		Y -= q.Y;
		Z -= q.Z;
		W -= q.W;
		return *this;
	}

	FORCEINLINE FQuaternion& operator *= (const FQuaternion& q)
	{
		const FQuaternion& Quat = FQuaternion(DirectX::XMQuaternionMultiply(q, m_vec));
		m_vec = Quat.m_vec;
		return *this;
	}

	FORCEINLINE FQuaternion& operator *= (float f)
	{
		X *= f;
		Y *= f;
		Z *= f;
		W *= f;
		return *this;
	}

	FORCEINLINE FQuaternion& operator /= (float f)
	{
		float fInv = 1.0f / f;
		X *= fInv;
		Y *= fInv;
		Z *= fInv;
		W *= fInv;
		return *this;
	}

	// unary operators
	FORCEINLINE constexpr FQuaternion operator -() const
	{
		return FQuaternion(-X, -Y, -Z, -W);
	}

	// binary operators
	FORCEINLINE constexpr FQuaternion operator + (const FQuaternion& q) const
	{
		return FQuaternion(X + q.X, Y + q.Y, Z + q.Z, W + q.W);
	}

	FORCEINLINE constexpr FQuaternion operator - (const FQuaternion& q) const
	{
		return FQuaternion(X - q.X, Y - q.Y, Z - q.Z, W - q.W);
	}

	FORCEINLINE constexpr FQuaternion operator * (float f) const
	{
		return FQuaternion(X * f, Y * f, Z * f, W * f);
	}

	FORCEINLINE constexpr FQuaternion operator / (float f) const
	{
		const float fInv = 1.0f / f;
		return FQuaternion(X * fInv, Y * fInv, Z * fInv, W * fInv);
	}

#if _MSVC_LANG >= 202002L
	//auto operator<=>(const FQuaternion&) const = default;
#endif
};

//#if _MSVC_LANG < 202002L
FORCEINLINE bool constexpr operator ==(const FQuaternion& value1, const FQuaternion& value2)
{
	return (value1.X == value2.X && value1.Y == value2.Y && value1.Z == value2.Z && value1.W == value2.W);
};

FORCEINLINE bool constexpr operator !=(const FQuaternion& value1, const FQuaternion& value2)
{
	return !((value1.X == value2.X) && (value1.Y == value2.Y) && (value1.Z == value2.Z) && (value1.W == value2.W));
};
//#endif

FORCEINLINE constexpr FRotationMatrix::FRotationMatrix(const FQuaternion& q)
{
	const FRotationMatrix& RotationMatrix = FRotationMatrix(DirectX::XMMatrixRotationQuaternion(q));
	r[0] = RotationMatrix.r[0];
	r[1] = RotationMatrix.r[1];
	r[2] = RotationMatrix.r[2];
}

FORCEINLINE constexpr FQuaternion Normalize(const FQuaternion& q)
{
	return FQuaternion(DirectX::XMQuaternionNormalize(q));
}

FORCEINLINE constexpr FQuaternion Slerp(const FQuaternion& a, const FQuaternion& b, const float t)
{
	return Normalize(FQuaternion(DirectX::XMQuaternionSlerp(a, b, t)));
}

FORCEINLINE constexpr FQuaternion Lerp(const FQuaternion& a, const FQuaternion& b, const float t)
{
	return Normalize(FQuaternion(DirectX::XMVectorLerp(a, b, t)));
}

FORCEINLINE constexpr FQuaternion operator * (const float f, const FQuaternion& q)
{
	return FQuaternion(f * q.X, f * q.Y, f * q.Z, f * q.W);
}

FORCEINLINE constexpr FQuaternion operator* (const FQuaternion& var1, const FQuaternion& var2)
{
	return FQuaternion(DirectX::XMQuaternionMultiply(var1, var2));
}

FORCEINLINE constexpr FVector operator* (const FVector& var1, const FQuaternion& var2)
{
	return FVector(DirectX::XMVector3Rotate(var1, var2));
}
FORCEINLINE constexpr FVector operator* (const FQuaternion& var1, const FVector& var2)
{
	return FVector(DirectX::XMVector3Rotate(var2, var1));
}

FORCEINLINE FAngles::operator FQuaternion() const
{
	return FQuaternion(*this);
}

FORCEINLINE FAngles::operator FQuaternion()
{
	return FQuaternion(*this);
}

FORCEINLINE constexpr FRotatedVector::operator FQuaternion() const
{
	return FQuaternion(*this);
}

FORCEINLINE constexpr FRotatedVector::operator FQuaternion()
{
	return FQuaternion(*this);
}

template<typename T>
struct TAffine3
{
	TMatrix3x3<T>	m_linear;
	TVector3<T>		m_translation;

	TAffine3()
	{}

	template<typename U>
	explicit constexpr TAffine3(const TAffine3<U>& a)
		: m_linear(a.m_linear)
		, m_translation(TVector3<T>((T)a.m_translation.X, (T)a.m_translation.Y, (T)a.m_translation.Z))
	{}

	constexpr TAffine3(T m00, T m01, T m02, T m10, T m11, T m12, T m20, T m21, T m22, T t0, T t1, T t2)
		: m_linear(m00, m01, m02, m10, m11, m12, m20, m21, m22)
		, m_translation(t0, t1, t2) 
	{}

	constexpr TAffine3(const TVector3<T>& row0, const TVector3<T>& row1, const TVector3<T>& row2, const TVector3<T>& translation)
		: m_linear(row0, row1, row2)
		, m_translation(translation) 
	{}

	constexpr TAffine3(const TMatrix3x3<T>& linear, const TVector3<T>& translation)
		: m_linear(linear)
		, m_translation(translation) 
	{}

	static constexpr TAffine3 from_cols(const TVector3<T>& col0, const TVector3<T>& col1, const TVector3<T>& col2, const TVector3<T>& translation)
	{
		return TAffine3(TMatrix3x3<T>::from_cols(col0, col1, col2), translation);
	}

	static constexpr TAffine3 identity()
	{
		return TAffine3(TMatrix3x3<T>::Identity(), TVector3<T>::Zero());
	}

	[[nodiscard]] TVector3<T> transformVector(const TVector3<T>& v) const
	{
		TVector3<T> result;
		result.X = v.X * m_linear.r[0].X + v.Y * m_linear.r[1].X + v.Z * m_linear.r[2].X;
		result.Y = v.X * m_linear.r[0].Y + v.Y * m_linear.r[1].Y + v.Z * m_linear.r[2].Y;
		result.Z = v.X * m_linear.r[0].Z + v.Y * m_linear.r[1].Z + v.Z * m_linear.r[2].Z;
		return result;
	}

	[[nodiscard]] TVector3<T> transformPoint(const TVector3<T>& v) const
	{
		TVector3<T> result;
		result.X = v.X * m_linear.r[0].X + v.Y * m_linear.r[1].X + v.Z * m_linear.r[2].X + m_translation.X;
		result.Y = v.X * m_linear.r[0].Y + v.Y * m_linear.r[1].Y + v.Z * m_linear.r[2].Y + m_translation.Y;
		result.Z = v.X * m_linear.r[0].Z + v.Y * m_linear.r[1].Z + v.Z * m_linear.r[2].Z + m_translation.Z;
		return result;
	}
};

template <typename T>
TVector3<T> operator * (const TMatrix3x3<T>& a, const TVector3<T>& b)
{
	TVector3<T> result;
	result.X = a.r[0].X * b.X + a.r[0].Y * b.Y + a.r[0].Z * b.Z;
	result.Y = a.r[1].X * b.X + a.r[1].Y * b.Y + a.r[1].Z * b.Z;
	result.Z = a.r[2].X * b.X + a.r[2].Y * b.Y + a.r[2].Z * b.Z;
	return result;
}

template <typename T>
TVector3<T> operator * (const TVector3<T>& a, const TMatrix3x3<T>& b)
{
	TVector3<T> result;
	result.X = a.X * b.r[0].X + a.Y * b.r[1].X + a.Z * b.r[2].X;
	result.Y = a.X * b.r[0].Y + a.Y * b.r[1].Y + a.Z * b.r[2].Y;
	result.Z = a.X * b.r[0].Z + a.Y * b.r[1].Z + a.Z * b.r[2].Z;
	return result;
}

template <typename T>
TAffine3<T> operator * (const TAffine3<T>& a, const TAffine3<T>& b)
{
	TAffine3<T> result =
	{
		a.m_linear * b.m_linear,
		a.m_translation * b.m_linear + b.m_translation
	};
	return result;
}

typedef TAffine3<float> Affine3;
typedef TAffine3<double> DAffine;
typedef TAffine3<int> IAffine;

class FBoundingSphere
{
	typedef FBoundingSphere Class;
	sStaticClassBody(Class)
public:
	constexpr FBoundingSphere(FVector center = FVector(), float radius = 10.0f)
		: Center(Center)
		, Radius(radius)
	{
	}
	constexpr FBoundingSphere(FVector4 sphere)
		: Center(Center)
		, Radius(sphere.W)
	{}

	constexpr FVector GetCenter(void) const
	{
		return Center;
	}

	constexpr float GetRadius(void) const
	{
		return Radius;
	}

private:
	FVector Center;
	float Radius;
};

struct FBoxDimension
{
private:
	typedef FBoxDimension Class;
	sStaticClassBody(Class)
public:
	float Width;
	float Height;
	float Depth;

	inline constexpr FBoxDimension()
		: Width(0.0f)
		, Height(0.0f)
		, Depth(0.0f)
	{}
	inline constexpr FBoxDimension(float width, float height, float depth)
		: Width(width)
		, Height(height)
		, Depth(depth)
	{}
	template<typename T>
	inline constexpr FBoxDimension(const TVector3<T> Dimension)
		: Width(static_cast<float>(Dimension.X))
		, Height(static_cast<float>(Dimension.Y))
		, Depth(static_cast<float>(Dimension.Z))
	{}
	inline constexpr FBoxDimension(const FBoxDimension& Other)
		: Width(Other.Width)
		, Height(Other.Height)
		, Depth(Other.Depth)
	{}

	~FBoxDimension() = default;

	inline constexpr void SetWidth(const float& width) { Width = width; };
	inline constexpr void SetHeight(const float& height) { Height = height; };
	inline constexpr void SetDepth(const float& depth) { Depth = depth; };

	inline constexpr float GetWidth() const { return Width; };
	inline constexpr float GetHeight() const { return Height; };
	inline constexpr float GetDepth() const { return Depth; };

	inline constexpr bool IsEqual(const FBoxDimension& Other) const
	{
		return (GetWidth() == Other.GetWidth()) && (GetHeight() == Other.GetHeight()) && (GetDepth() == Other.GetDepth());
	}

	inline constexpr std::string ToString() const
	{
		return std::string("{ Width: " + std::to_string(Width) + " Height: " + std::to_string(Height) + " Depth: " + std::to_string(Depth) + " };");
	};

	auto operator<=>(const FBoxDimension&) const = default;
};

class FBoundingBox
{
	typedef FBoundingBox Class;
	sStaticClassBody(Class)
public:
	FORCEINLINE static constexpr FBoundingBox Zero()
	{
		return FBoundingBox(FVector::Zero(), FVector::Zero());
	};

public:
	FVector Min;
	FVector Max;

public:
	FORCEINLINE constexpr FBoundingBox() noexcept
		: Min(FVector::Zero())
		, Max(FVector::Zero())
	{}

	FORCEINLINE constexpr FBoundingBox(const FBounds2D& Bound);

	FORCEINLINE constexpr FBoundingBox(const FVector& InMin, const FVector& InMax)
		: Min(InMin)
		, Max(InMax)
	{}

	template<typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
	FORCEINLINE constexpr FBoundingBox(const FBoxDimension& InDimension, const TVector3<T>& Location = TVector3<T>::Zero())
		: Min(FVector(static_cast<float>(TVector3<T>::Zero().X - (InDimension.GetWidth() / 2.0f)), static_cast<float>(TVector3<T>::Zero().Y - (InDimension.GetHeight() / 2.0f)), static_cast<float>(TVector3<T>::Zero().Z - (InDimension.GetDepth() / 2.0f))))
		, Max(FVector(static_cast<float>(TVector3<T>::Zero().X + (InDimension.GetWidth() / 2.0f)), static_cast<float>(TVector3<T>::Zero().Y + (InDimension.GetHeight() / 2.0f)), static_cast<float>(TVector3<T>::Zero().Z - (InDimension.GetDepth() / 2.0f))))
	{
		SetPosition(FVector(static_cast<float>(Location.X), static_cast<float>(Location.Y), static_cast<float>(Location.Z)));
	}

	FORCEINLINE constexpr FBoundingBox(const FBoxDimension& InDimension, const FVector& Location = FVector::Zero())
		: Min(FVector(FVector::Zero().X - (InDimension.GetWidth() / 2.0f), FVector::Zero().Y - (InDimension.GetHeight() / 2.0f), FVector::Zero().Z - (InDimension.GetDepth() / 2.0f)))
		, Max(FVector(FVector::Zero().X + (InDimension.GetWidth() / 2.0f), FVector::Zero().Y + (InDimension.GetHeight() / 2.0f), FVector::Zero().Z - (InDimension.GetDepth() / 2.0f)))
	{
		SetPosition(Location);
	}

	~FBoundingBox() = default;

	FORCEINLINE constexpr void SetPosition(const FVector& Position)
	{
		const FVector Offset = Position - GetCenter();
		Min = Min + Offset;
		Max = Max + Offset;
	}

	FORCEINLINE constexpr bool IsValid() const
	{
		return (Min.X < Max.X&& Min.Y < Max.Y&& Min.Z < Max.Z);
	};

	FORCEINLINE constexpr bool Equals(const FBoundingBox& other) const
	{
		return (Min.Equals(other.Min) && Max.Equals(other.Max));
	};

	FORCEINLINE constexpr float GetTop() const { return Min.Y; };
	FORCEINLINE constexpr float GetLeft() const { return Min.X; };
	FORCEINLINE constexpr float GetMinDepth() const { return Min.Z; };
	FORCEINLINE constexpr float GetRight() const { return Max.X; };
	FORCEINLINE constexpr float GetBottom() const { return Max.Y; };
	FORCEINLINE constexpr float GetMaxDepth() const { return Max.Z; };

	FORCEINLINE constexpr void SetTop(const float& Value) { Min.Y = Value; };
	FORCEINLINE constexpr void SetLeft(const float& Value) { Min.X = Value; };
	FORCEINLINE constexpr void SetMinDepth(const float& Value) { Min.Z = Value; };
	FORCEINLINE constexpr void SetRight(const float& Value) { Max.X = Value; };
	FORCEINLINE constexpr void SetBottom(const float& Value) { Max.Y = Value; };
	FORCEINLINE constexpr void SetMaxDepth(const float& Value) { Max.Z = Value; };
	FORCEINLINE constexpr void SetDepth(const float& Value) { Max.Z = Min.Z + Value; };

	FORCEINLINE constexpr FVector GetCenter() const { return ((Min + Max) * 0.5f); };
	FORCEINLINE constexpr FVector GetExtent() const { return (0.5f * (Max - Min)); };
	FORCEINLINE constexpr float GetVolume() const { return ((Max.X - Min.X) * (Max.Y - Min.Y) * (Max.Z - Min.Z)); }
	FORCEINLINE constexpr FBoxDimension GetDimension() const { return FBoxDimension(GetWidth(), GetHeight(), GetDepth()); }
	FORCEINLINE constexpr float GetHeight() const { return Max.Y - Min.Y; }
	FORCEINLINE constexpr float GetWidth() const { return Max.X - Min.X; }
	FORCEINLINE constexpr float GetDepth() const { return Max.Z - Min.Z; }

	FORCEINLINE constexpr void SetExtents(const FVector& InExtent) { FVector Center = GetCenter(); Min = (Center - InExtent); Max = (Center + InExtent); };
	FORCEINLINE constexpr void SetExtents(const FVector& InExtent, const FVector& InOrigin) { FVector Center = InOrigin; Min = (Center - InExtent); Max = (Center + InExtent); };
	FORCEINLINE constexpr void SetMinExtend(const FVector& InExtent) { FVector Center = GetCenter(); Min = (Center - InExtent); };
	FORCEINLINE constexpr void SetMinExtend(const FVector& InExtent, const FVector& InOrigin) { FVector Center = InOrigin; Min = (Center - InExtent); };
	FORCEINLINE constexpr void SetMaxExtend(const FVector& InExtent) { FVector Center = GetCenter(); Max = (Center + InExtent); };
	FORCEINLINE constexpr void SetMaxExtend(const FVector& InExtent, const FVector& InOrigin) { FVector Center = InOrigin; Max = (Center + InExtent); };

	FORCEINLINE constexpr void SetHeight(const float& InHeight)
	{
		FVector Center = GetCenter();
		Min.Y = (Center.Y - (static_cast<float>(InHeight) / 2.0f));
		Max.Y = (Center.Y + (static_cast<float>(InHeight) / 2.0f));
	};

	FORCEINLINE constexpr void SetHeight(const float& InHeight, const FVector& InOrigin)
	{
		const FVector Center = InOrigin;
		Min.Y = (Center.Y - (static_cast<float>(InHeight) / 2.0f));
		Max.Y = (Center.Y + (static_cast<float>(InHeight) / 2.0f));
	};
	FORCEINLINE constexpr void SetWidth(const float& InWidth)
	{
		const FVector Center = GetCenter();
		Min.X = (Center.X - (static_cast<float>(InWidth) / 2.0f));
		Max.X = (Center.X + (static_cast<float>(InWidth) / 2.0f));
	};

	FORCEINLINE constexpr void SetWidth(const float& InWidth, const FVector& InOrigin)
	{
		const FVector Center = InOrigin;
		Min.X = (Center.X - (static_cast<float>(InWidth) / 2.0f));
		Max.X = (Center.X + (static_cast<float>(InWidth) / 2.0f));
	};

	FORCEINLINE constexpr void SetDimension(const FBoxDimension& InDimension)
	{
		const FVector Center = GetCenter();
		Min.X = (Center.X - (static_cast<float>(InDimension.GetWidth()) / 2.0f));
		Min.Y = (Center.Y - (static_cast<float>(InDimension.GetHeight()) / 2.0f));
		Min.Z = (Center.Z - (static_cast<float>(InDimension.GetDepth()) / 2.0f));
		Max.X = (Center.X + (static_cast<float>(InDimension.GetWidth()) / 2.0f));
		Max.Y = (Center.Y + (static_cast<float>(InDimension.GetHeight()) / 2.0f));
		Max.Z = (Center.Z + (static_cast<float>(InDimension.GetDepth()) / 2.0f));
	};

	FORCEINLINE constexpr void Reset(const FBoxDimension& InDimension, const FVector& InOrigin)
	{
		const FVector Center = InOrigin;
		Min.X = (Center.X - (static_cast<float>(InDimension.GetWidth()) / 2.0f));
		Min.Y = (Center.Y - (static_cast<float>(InDimension.GetHeight()) / 2.0f));
		Min.Z = (Center.Z - (static_cast<float>(InDimension.GetDepth()) / 2.0f));
		Max.X = (Center.X + (static_cast<float>(InDimension.GetWidth()) / 2.0f));
		Max.Y = (Center.Y + (static_cast<float>(InDimension.GetHeight()) / 2.0f));
		Max.Z = (Center.Z + (static_cast<float>(InDimension.GetDepth()) / 2.0f));
	};

	FORCEINLINE constexpr bool Contains(const FVector& value) const
	{
		return ((((Min.X <= value.X) && (value.X < (Min.X + GetWidth()))) && (Min.Y <= value.Y)) && (value.Y < (Min.Y + GetHeight())));
	}

	FORCEINLINE constexpr bool IsInside(const FVector& value) const
	{
		return ((value.X > Min.X) && (value.X < Max.X) && (value.Y > Min.Y) && (value.Y < Max.Y) && (value.Z > Min.Z) && (value.Z < Max.Z));
	}

	FORCEINLINE constexpr bool IsInsideOrOn(const FVector& value) const
	{
		return ((value.X >= Min.X) && (value.X <= Max.X) && (value.Y >= Min.Y) && (value.Y <= Max.Y) && (value.Z >= Min.Z) && (value.Z <= Max.Z));
	}

	FORCEINLINE constexpr bool IsInside(const FBoundingBox& Other) const
	{
		return (IsInside(Other.Min) && IsInside(Other.Max));
	}

	FORCEINLINE constexpr bool IsInsideXY(const FVector& value) const
	{
		return ((value.X > Min.X) && (value.X < Max.X) && (value.Y > Min.Y) && (value.Y < Max.Y));
	}

	FORCEINLINE constexpr bool IsInsideXY(const FBoundingBox& Other) const
	{
		return (IsInsideXY(Other.Min) && IsInsideXY(Other.Max));
	}

	FORCEINLINE constexpr bool Intersect(const FBoundingBox& Other) const
	{
		if ((Min.X > Other.Max.X) || (Other.Min.X > Max.X))
		{
			return false;
		}

		if ((Min.Y > Other.Max.Y) || (Other.Min.Y > Max.Y))
		{
			return false;
		}

		if ((Min.Z > Other.Max.Z) || (Other.Min.Z > Max.Z))
		{
			return false;
		}

		return true;
	}

	FORCEINLINE constexpr bool IntersectXY(const FBoundingBox& Other) const
	{
		if ((Min.X > Other.Max.X) || (Other.Min.X > Max.X))
		{
			return false;
		}

		if ((Min.Y > Other.Max.Y) || (Other.Min.Y > Max.Y))
		{
			return false;
		}

		return true;
	}

	FORCEINLINE constexpr bool IntersectX(const FBoundingBox& Other) const
	{
		if ((Min.X > Other.Max.X) || (Other.Min.X > Max.X))
		{
			return false;
		}

		return true;
	}

	FORCEINLINE constexpr bool IntersectY(const FBoundingBox& Other) const
	{
		if ((Min.Y > Other.Max.Y) || (Other.Min.Y > Max.Y))
		{
			return false;
		}

		return true;
	}

	FORCEINLINE constexpr std::string ToString() const
	{
		return std::string("{ Min: " + Min.ToString() + " Max: " + Max.ToString() + " };");
	};

	FORCEINLINE constexpr FBoundingBox operator | (const FVector& v) const
	{
		return FBoundingBox(std::min(Min, v), std::max(Max, v));
	}

	FORCEINLINE FBoundingBox operator |= (const FVector& v)
	{
		*this = *this | v;
		return *this;
	}

	constexpr FBoundingBox operator | (const FBoundingBox& other) const
	{
		return FBoundingBox(std::min(Min, other.Min), std::max(Max, other.Max));
	}

	FBoundingBox operator |= (const FBoundingBox& other)
	{
		*this = *this | other;
		return *this;
	}

	FBoundingBox operator * (const Affine3& transform) const
	{
		// fast method to apply an affine transform to an AABB
		FBoundingBox result;
		result.Min = transform.m_translation;
		result.Max = transform.m_translation;
		const FVector* row = &transform.m_linear.r[0];
		for (int i = 0; i < 3; i++)
		{
			FVector e = (&Min.X)[i] * *row;
			FVector f = (&Max.X)[i] * *row;
			result.Min += std::min(e, f);
			result.Max += std::max(e, f);
			++row;
		}
		return result;
	}

	auto operator<=>(const FBoundingBox&) const = default;
};

FORCEINLINE constexpr FBoundingBox operator +(const FBoundingBox& Rect, const FVector& V)
{
	return FBoundingBox(Rect.Min + V, Rect.Max + V);
}

FORCEINLINE constexpr FBoundingBox operator -(const FBoundingBox& Rect, const FVector& V)
{
	return FBoundingBox(Rect.Min - V, Rect.Max - V);
}

FORCEINLINE constexpr FVector Normalize(const FVector& v)
{
	return FVector(DirectX::XMVector3Normalize(v));
}

template<typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
struct TDimension2D
{
private:
	typedef TDimension2D<T> Class;
	sStaticClassBody(Class)
public:
	T Width;
	T Height;

	FORCEINLINE constexpr TDimension2D() noexcept
		: Width(0.0f)
		, Height(0.0f)
	{}
	FORCEINLINE constexpr TDimension2D(const T InWidth, const T InHeight) noexcept
		: Width(InWidth)
		, Height(InHeight)
	{}
	FORCEINLINE constexpr TDimension2D(const TVector2<T> FDimension2D) noexcept
		: Width(FDimension2D.X)
		, Height(FDimension2D.Y)
	{}
	FORCEINLINE constexpr TDimension2D(const TDimension2D<T>& Other) noexcept
		: Width(Other.Width)
		, Height(Other.Height)
	{}

	FORCEINLINE ~TDimension2D() = default;

	inline constexpr void SetWidth(const T width) { Width = width; }
	inline constexpr void SetHeight(const T height) { Height = height; }

	inline constexpr T GetWidth() const { return Width; }
	inline constexpr T GetHeight() const { return Height; }

	inline constexpr bool IsEqual(const TDimension2D<T>& Other) const
	{
		return (GetWidth()) == (Other.GetWidth()) && (GetHeight()) == (Other.GetHeight());
	}

	constexpr std::string ToString() const
	{
		return std::string("{ Width: " + std::to_string(Width) + " Height: " + std::to_string(Height) + " };");
	}

#if _MSVC_LANG >= 202002L
	auto operator<=>(const TDimension2D&) const = default;
#endif
};

typedef TDimension2D<float> FDimension2D;

template<typename T>
FORCEINLINE constexpr TDimension2D<T> operator +(const TDimension2D<T>& value1, const TVector2<T>& value2)
{
	return TDimension2D<T>(value1.Width + value2.X, value1.Height + value2.Y);
}

template<typename T>
FORCEINLINE constexpr TDimension2D<T> operator +(const TDimension2D<T>& value1, const TDimension2D<T>& value2)
{
	return TDimension2D<T>(value1.Width + value2.Width, value1.Height + value2.Height);
}

template<typename T>
FORCEINLINE constexpr TDimension2D<T> operator -(const TDimension2D<T>& value1, const TVector2<T>& value2)
{
	return TDimension2D<T>(value1.Width - value2.X, value1.Height - value2.Y);
}

template<typename T>
FORCEINLINE constexpr TDimension2D<T> operator -(const TDimension2D<T>& value1, const TDimension2D<T>& value2)
{
	return TDimension2D<T>(value1.Width - value2.Width, value1.Height - value2.Height);
}

class FBounds2D
{
	typedef FBounds2D Class;
	sStaticClassBody(Class)
public:
	static constexpr FBounds2D Zero()
	{
		return FBounds2D(FVector2::Zero(), FVector2::Zero());
	}

public:
	FVector2 Min;
	FVector2 Max;

public:
	FORCEINLINE constexpr FBounds2D() noexcept
		: Min(FVector2::Zero())
		, Max(FVector2::Zero())
	{}

	FORCEINLINE constexpr FBounds2D(const FBoundingBox& BoundingBox) noexcept
		: Min(FVector2(BoundingBox.Min.X, BoundingBox.Min.Y))
		, Max(FVector2(BoundingBox.Max.X, BoundingBox.Max.Y))
	{}

	FORCEINLINE constexpr FBounds2D(const FBounds2D& Other) noexcept
		: Min(Other.Min)
		, Max(Other.Max)
	{}

	FORCEINLINE constexpr FBounds2D(const FVector2& InMin, const FVector2& InMax) noexcept
		: Min(InMin)
		, Max(InMax)
	{}

	template<typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
	FORCEINLINE constexpr FBounds2D(const TDimension2D<T>& InDimension, const TVector2<T>& Location = TVector2<T>::Zero())
		: Min(FVector2(static_cast<float>(TVector2<T>::Zero().X - (InDimension.GetWidth() / 2.0f)), static_cast<float>(TVector2<T>::Zero().Y - (InDimension.GetHeight() / 2.0f))))
		, Max(FVector2(static_cast<float>(TVector2<T>::Zero().X + (InDimension.GetWidth() / 2.0f)), static_cast<float>(TVector2<T>::Zero().Y + (InDimension.GetHeight() / 2.0f))))
	{
		SetPosition(FVector2(static_cast<float>(Location.X), static_cast<float>(Location.Y)));
	}

	FORCEINLINE constexpr FBounds2D(const FDimension2D& InDimension, const FVector2& Location = FVector2::Zero())
		: Min(FVector2(FVector2::Zero().X - (InDimension.GetWidth() / 2.0f), FVector2::Zero().Y - (InDimension.GetHeight() / 2.0f)))
		, Max(FVector2(FVector2::Zero().X + (InDimension.GetWidth() / 2.0f), FVector2::Zero().Y + (InDimension.GetHeight() / 2.0f)))
	{
		SetPosition(Location);
	}

	FORCEINLINE ~FBounds2D() = default;

	FORCEINLINE constexpr void SetPosition(const FVector2& Position)
	{
		const FVector2 Offset = Position - GetCenter();
		Min = Min + Offset;
		Max = Max + Offset;
	}

	FORCEINLINE constexpr bool IsValid() const
	{
		return (Min.X < Max.X&& Min.Y < Max.Y);
	}

	FORCEINLINE constexpr bool Equals(const FBounds2D& other) const
	{
		return (Min.Equals(other.Min) && Max.Equals(other.Max));
	}

	FORCEINLINE constexpr FVector2 GetCorner(const unsigned int& index) const
	{
		switch (index)
		{
		case 0: return Min;
		case 1: return FVector2(Max.X, Min.Y);
		case 2: return Max;
		case 3: return FVector2(Min.X, Max.Y);
		}
		return FVector2(0.f, 0.f);
	}

	FORCEINLINE constexpr float GetTop() const { return Min.Y; }
	FORCEINLINE constexpr float GetLeft() const { return Min.X; }
	FORCEINLINE constexpr float GetRight() const { return Max.X; }
	FORCEINLINE constexpr float GetBottom() const { return Max.Y; }

	FORCEINLINE constexpr void SetTop(const float Value) { Min.Y = Value; }
	FORCEINLINE constexpr void SetLeft(const float Value) { Min.X = Value; }
	FORCEINLINE constexpr void SetRight(const float Value) { Max.X = Value; }
	FORCEINLINE constexpr void SetBottom(const float Value) { Max.Y = Value; }

	FORCEINLINE constexpr FVector2 GetCenter() const { return ((Min + Max) * 0.5f); }
	FORCEINLINE constexpr FVector2 GetExtent() const { return (0.5f * (Max - Min)); }
	FORCEINLINE constexpr void SetExtents(const FVector2& InExtent) { FVector2 Center = GetCenter(); Min = (Center - InExtent); Max = (Center + InExtent); }
	FORCEINLINE constexpr void SetExtents(const FVector2& InExtent, const FVector2& InOrigin) { FVector2 Center = InOrigin; Min = (Center - InExtent); Max = (Center + InExtent); }
	FORCEINLINE constexpr void SetMinExtend(const FVector2& InExtent) { FVector2 Center = GetCenter(); Min = (Center - InExtent); }
	FORCEINLINE constexpr void SetMinExtend(const FVector2& InExtent, const FVector2& InOrigin) { FVector2 Center = InOrigin; Min = (Center - InExtent); }
	FORCEINLINE constexpr void SetMaxExtend(const FVector2& InExtent) { FVector2 Center = GetCenter(); Max = (Center + InExtent); }
	FORCEINLINE constexpr void SetMaxExtend(const FVector2& InExtent, const FVector2& InOrigin) { FVector2 Center = InOrigin; Max = (Center + InExtent); }
	FORCEINLINE constexpr float GetHeight() const { return Max.Y - Min.Y; }
	FORCEINLINE constexpr float GetWidth() const { return Max.X - Min.X; }
	FORCEINLINE constexpr FDimension2D GetDimension() const { return FDimension2D(GetWidth(), GetHeight()); }

	FORCEINLINE constexpr void SetHeight(const float InHeight)
	{
		FVector2 Center = GetCenter();
		Min.Y = (Center.Y - (static_cast<float>(InHeight) / 2.0f));
		Max.Y = (Center.Y + (static_cast<float>(InHeight) / 2.0f));
	}

	FORCEINLINE constexpr void SetHeight(const float InHeight, const FVector2& InOrigin)
	{
		const FVector2 Center = InOrigin;
		Min.Y = (Center.Y - (static_cast<float>(InHeight) / 2.0f));
		Max.Y = (Center.Y + (static_cast<float>(InHeight) / 2.0f));
	}

	FORCEINLINE constexpr void SetWidth(const float InWidth)
	{
		const FVector2 Center = GetCenter();
		Min.X = (Center.X - (static_cast<float>(InWidth) / 2.0f));
		Max.X = (Center.X + (static_cast<float>(InWidth) / 2.0f));
	}

	FORCEINLINE constexpr void SetWidth(const float InWidth, const FVector2& InOrigin)
	{
		const FVector2 Center = InOrigin;
		Min.X = (Center.X - (static_cast<float>(InWidth) / 2.0f));
		Max.X = (Center.X + (static_cast<float>(InWidth) / 2.0f));
	}

	FORCEINLINE constexpr void SetDimension(const FDimension2D& InDimension)
	{
		const FVector2 Center = GetCenter();
		Min.X = (Center.X - (static_cast<float>(InDimension.GetWidth()) / 2.0f));
		Min.Y = (Center.Y - (static_cast<float>(InDimension.GetHeight()) / 2.0f));
		Max.X = (Center.X + (static_cast<float>(InDimension.GetWidth()) / 2.0f));
		Max.Y = (Center.Y + (static_cast<float>(InDimension.GetHeight()) / 2.0f));
	}

	FORCEINLINE constexpr void Reset(const FDimension2D& InDimension, const FVector2& InOrigin)
	{
		const FVector2 Center = InOrigin;
		Min.X = (Center.X - (static_cast<float>(InDimension.GetWidth()) / 2.0f));
		Min.Y = (Center.Y - (static_cast<float>(InDimension.GetHeight()) / 2.0f));
		Max.X = (Center.X + (static_cast<float>(InDimension.GetWidth()) / 2.0f));
		Max.Y = (Center.Y + (static_cast<float>(InDimension.GetHeight()) / 2.0f));
	}

	FORCEINLINE constexpr void Expand(const FBounds2D& Rect)
	{
		if (Rect.Min.X < Min.X)
		{
			Min.X = Rect.Min.X;
		}
		if (Rect.Min.Y < Min.Y)
		{
			Min.Y = Rect.Min.Y;
		}
		if (Rect.Max.X > Max.X)
		{
			Max.X = Rect.Max.X;
		}
		if (Rect.Max.Y > Max.Y)
		{
			Max.Y = Rect.Max.Y;
		}
	}

	FORCEINLINE constexpr FBounds2D Crop(const FBounds2D& FilterBound)
	{
		if (Min.X < FilterBound.Min.X)
		{
			Min.X = FilterBound.Min.X;
		}
		if (Min.Y < FilterBound.Min.Y)
		{
			Min.Y = FilterBound.Min.Y;
		}
		if (Max.X > FilterBound.Max.X)
		{
			Max.X = FilterBound.Max.X;
		}
		if (Max.Y > FilterBound.Max.Y)
		{
			Max.Y = FilterBound.Max.Y;
		}

		return *this;
	}

	FORCEINLINE constexpr bool IsInside(const FVector2& value) const
	{
		return ((value.X >= Min.X) && (value.X <= Max.X) && (value.Y >= Min.Y) && (value.Y <= Max.Y));
	}

	FORCEINLINE constexpr bool IsInside(const FBounds2D& Other) const
	{
		return (IsInside(Other.Min) && IsInside(Other.Max));
	}

	FORCEINLINE constexpr bool Intersect(const FVector2& InLocation) const
	{
		if ((Min.X > InLocation.X) || (InLocation.X > Max.X))
		{
			return false;
		}

		if ((Min.Y > InLocation.Y) || (InLocation.Y > Max.Y))
		{
			return false;
		}

		return true;
	}

	FORCEINLINE constexpr bool Intersect(const FBounds2D& Other) const
	{
		if ((Min.X > Other.Max.X) || (Other.Min.X > Max.X))
		{
			return false;
		}

		if ((Min.Y > Other.Max.Y) || (Other.Min.Y > Max.Y))
		{
			return false;
		}

		return true;
	}

	constexpr std::string ToString() const
	{
		return std::string("{ Min: " + Min.ToString() + " Max: " + Max.ToString() + " };");
	}

#if _MSVC_LANG >= 202002L
	auto operator<=>(const FBounds2D&) const = default;
#endif
};

FORCEINLINE constexpr FBounds2D operator +(const FBounds2D& Rect, const FVector2& V)
{
	return FBounds2D(Rect.Min + V, Rect.Max + V);
}

FORCEINLINE constexpr FBounds2D operator -(const FBounds2D& Rect, const FVector2& V)
{
	return FBounds2D(Rect.Min - V, Rect.Max - V);
}

FORCEINLINE constexpr FBoundingBox::FBoundingBox(const FBounds2D& Bound)
	: Min(FVector(Bound.Min.X, Bound.Min.Y, 0.0f))
	, Max(FVector(Bound.Max.X, Bound.Max.Y, 0.0f))
{}

typedef TVector2<std::int16_t> Int16Vec;

class IntBounds2D
{
	typedef IntBounds2D Class;
	sStaticClassBody(Class)
public:
	static constexpr IntBounds2D Zero()
	{
		return IntBounds2D(Int16Vec::Zero(), Int16Vec::Zero());
	}

public:
	Int16Vec Min;
	Int16Vec Max;

public:
	FORCEINLINE constexpr IntBounds2D() noexcept
		: Min(Int16Vec::Zero())
		, Max(Int16Vec::Zero())
	{}

	FORCEINLINE constexpr IntBounds2D(const IntBounds2D& Other) noexcept
		: Min(Other.Min)
		, Max(Other.Max)
	{}

	FORCEINLINE constexpr IntBounds2D(const FBounds2D& Bounds) noexcept
		: Min(Int16Vec(static_cast<std::int16_t>(std::floor(Bounds.Min.X)), static_cast<std::int16_t>(std::floor(Bounds.Min.Y))))
		, Max(Int16Vec(static_cast<std::int16_t>(std::ceil(Bounds.Max.X)), static_cast<std::int16_t>(std::ceil(Bounds.Max.Y))))
	{}

	FORCEINLINE constexpr IntBounds2D(const Int16Vec& InMin, const Int16Vec& InMax) noexcept
		: Min(InMin)
		, Max(InMax)
	{}

	FORCEINLINE ~IntBounds2D() = default;

	FORCEINLINE constexpr bool IsValid() const
	{
		return (Min.X < Max.X&& Min.Y < Max.Y);
	}

	FORCEINLINE constexpr bool Equals(const IntBounds2D& other) const
	{
		return (Min.Equals(other.Min) && Max.Equals(other.Max));
	}

	FORCEINLINE constexpr std::int16_t GetTop() const { return Min.Y; }
	FORCEINLINE constexpr std::int16_t GetLeft() const { return Min.X; }
	FORCEINLINE constexpr std::int16_t GetRight() const { return Max.X; }
	FORCEINLINE constexpr std::int16_t GetBottom() const { return Max.Y; }

	FORCEINLINE constexpr FVector2 GetCenter() const { return FVector2((Min.X + Max.X) * 0.5f, (Min.Y + Max.Y) * 0.5f); }
	FORCEINLINE constexpr float GetHeight() const { return static_cast<float>(Max.Y) - static_cast<float>(Min.Y); }
	FORCEINLINE constexpr float GetWidth() const { return static_cast<float>(Max.X) - static_cast<float>(Min.X); }
	FORCEINLINE constexpr FDimension2D GetDimension() const { return FDimension2D(GetWidth(), GetHeight()); }

	FORCEINLINE constexpr const IntBounds2D& Crop(const IntBounds2D& FilterBound) noexcept
	{
		if (Min.X < FilterBound.Min.X)
		{
			Min.X = FilterBound.Min.X;
		}
		if (Min.Y < FilterBound.Min.Y)
		{
			Min.Y = FilterBound.Min.Y;
		}
		if (Max.X > FilterBound.Max.X)
		{
			Max.X = FilterBound.Max.X;
		}
		if (Max.Y > FilterBound.Max.Y)
		{
			Max.Y = FilterBound.Max.Y;
		}

		return *this;
	}

	constexpr std::string ToString() const
	{
		return std::string("{ Min: " + Min.ToString() + " Max: " + Max.ToString() + " };");
	}

#if _MSVC_LANG >= 202002L
	auto operator<=>(const IntBounds2D&) const = default;
#endif
};

FORCEINLINE constexpr TVector2<float> Negate(const TVector2<float>& V)
{
	return TVector2<float>(-V.X, -V.Y);
};

FORCEINLINE constexpr TVector3<float> Negate(const TVector3<float>& V)
{
	return TVector3<float>(-V.X, -V.Y, -V.Z);
};

FORCEINLINE constexpr TVector4<float> Negate(const TVector4<float>& V)
{
	return TVector4<float>(-V.X, -V.Y, -V.Z, -V.W);
}

FORCEINLINE constexpr FVector4 Negate(const FVector4& Vec)
{
	FVector4 Temp;
	Temp.X = -Vec.X;
	Temp.Y = -Vec.Y;
	Temp.Z = -Vec.Z;
	Temp.W = -Vec.W;
	return Temp;
};

template<typename T>
FORCEINLINE constexpr T DistSquared(const TVector2<T>& V1, const TVector2<T>& V2)
{
	return Square(V2.X - V1.X) + Square(V2.Y - V1.Y);
}

template<typename T>
FORCEINLINE constexpr T DistSquared(const TVector3<T>& V1, const TVector3<T>& V2)
{
	return Square(V2.X - V1.X) + Square(V2.Y - V1.Y) + Square(V2.Z - V1.Z);
}

template<typename T>
FORCEINLINE constexpr T Distance(const TVector2<T>& V1, const TVector2<T>& V2)
{
	return std::sqrt(DistSquared(V1, V2));
}

template<typename T>
FORCEINLINE constexpr T Distance(const TVector3<T>& V1, const TVector3<T>& V2)
{
	return std::sqrt(DistSquared(V1, V2));
}

template<typename T>
FORCEINLINE constexpr float Magnitude(const TVector2<T>& V)
{
	return std::sqrt(V.X * V.X + V.Y * V.Y);
};

template<typename T>
FORCEINLINE constexpr float Magnitude(const TVector3<T>& V)
{
	return std::sqrt(V.X * V.X + V.Y * V.Y + V.Z * V.Z);
};

FORCEINLINE float Magnitude(const FVector4& V)
{
	return std::sqrt(V.X * V.X + V.Y * V.Y + V.Z * V.Z + V.W * V.W);
};

template<typename T>
FORCEINLINE constexpr float Dot(const TVector2<T>& v1, const TVector2<T>& v2)
{
	return (v1.X * v2.X) + (v1.Y * v2.Y);
}

template<typename T>
FORCEINLINE constexpr float Dot(const TVector3<T>& v1, const TVector3<T>& v2)
{
	return (v1.X * v2.X) + (v1.Y * v2.Y) + (v1.Z * v2.Z);
}

FORCEINLINE constexpr float Dot(const FVector4& v1, const FVector4& v2)
{
	return (v1.X * v2.X) + (v1.Y * v2.Y) + (v1.Z * v2.Z) + (v1.W * v2.W);
}

FORCEINLINE constexpr FVector Cross(FVector v1, FVector v2)
{
	return v1.CrossProduct(v2);
}

template<typename T>
FORCEINLINE constexpr float GetDeterminant(const TVector2<T>& v1, const TVector2<T>& v2)
{
	return v1.X * v2.Y - v1.Y * v2.X;
}

template<typename T>
FORCEINLINE constexpr float GetAngleBetweenTwoVectors(const TVector2<T>& v1, const TVector2<T>& v2)
{
	return (std::atan2(GetDeterminant(v1, v2), Dot(v1, v2))) * 180 / 3.14159265359f; // PI
};

template<typename T>
FORCEINLINE constexpr float GetAngleBetweenTwoVectors(const TVector3<T>& v1, const TVector3<T>& v2)
{
	return (std::acos(Dot(v1, v2) / sqrt(v1.Length() * v2.Length()))) * 180 / 3.14159265359f; // PI
};

FORCEINLINE FMatrix Invert(const FMatrix& mat)
{
	return FMatrix(DirectX::XMMatrixInverse(nullptr, mat));
}

FORCEINLINE float RecipSqrt(const float s)
{
	return (DirectX::XMVectorReciprocalSqrt(FVector4(s, s, s, s))).m128_f32[0];
}

template<typename T>
FORCEINLINE constexpr TVector2<T> GetInverse(const TVector2<T>& V)
{
	return TVector2<T>(-V.X, -V.Y);
}

template<typename T>
FORCEINLINE constexpr TVector3<T> GetInverse(const TVector3<T>& V)
{
	return TVector3<T>(-V.X, -V.Y, -V.Z);
}

template<typename T>
FORCEINLINE constexpr bool IsInside(const FBoundingBox& AABB, const TVector3<T>& Point)
{
	return AABB.IsInside(Point);
}

FORCEINLINE constexpr bool Intersect(const FBoundingBox& AABB, const FBoundingBox& Other)
{
	return AABB.Intersect(Other);
}

FORCEINLINE constexpr float SizeSquared(FVector Current)
{
	return Current.X * Current.X + Current.Y * Current.Y + Current.Z * Current.Z;
}

FORCEINLINE float Size(FVector2 Current)
{
	return sqrtf(Current.X * Current.X + Current.Y * Current.Y);
}

FORCEINLINE float Size(FVector Current)
{
	return sqrtf(Current.X * Current.X + Current.Y * Current.Y + Current.Z * Current.Z);
}

FORCEINLINE constexpr FMatrix ToMatrixWithScale(const FVector& Translation, const FVector& Scale3D, const FVector4& Rotation)
{
	FMatrix OutMatrix;

	OutMatrix.r[3][0] = Translation.X;
	OutMatrix.r[3][1] = Translation.Y;
	OutMatrix.r[3][2] = Translation.Z;

	const float x2 = Rotation.X + Rotation.X;
	const float y2 = Rotation.Y + Rotation.Y;
	const float z2 = Rotation.Z + Rotation.Z;
	{
		const float xx2 = Rotation.X * x2;
		const float yy2 = Rotation.Y * y2;
		const float zz2 = Rotation.Z * z2;

		OutMatrix.r[0][0] = (1.0f - (yy2 + zz2)) * Scale3D.X;
		OutMatrix.r[1][1] = (1.0f - (xx2 + zz2)) * Scale3D.Y;
		OutMatrix.r[2][2] = (1.0f - (xx2 + yy2)) * Scale3D.Z;
	}
	{
		const float yz2 = Rotation.Y * z2;
		const float wx2 = Rotation.W * x2;

		OutMatrix.r[2][1] = (yz2 - wx2) * Scale3D.Z;
		OutMatrix.r[1][2] = (yz2 + wx2) * Scale3D.Y;
	}
	{
		const float xy2 = Rotation.X * y2;
		const float wz2 = Rotation.W * z2;

		OutMatrix.r[1][0] = (xy2 - wz2) * Scale3D.Y;
		OutMatrix.r[0][1] = (xy2 + wz2) * Scale3D.X;
	}
	{
		const float xz2 = Rotation.X * z2;
		const float wy2 = Rotation.W * y2;

		OutMatrix.r[2][0] = (xz2 + wy2) * Scale3D.Z;
		OutMatrix.r[0][2] = (xz2 - wy2) * Scale3D.X;
	}

	OutMatrix.r[0][3] = 0.0f;
	OutMatrix.r[1][3] = 0.0f;
	OutMatrix.r[2][3] = 0.0f;
	OutMatrix.r[3][3] = 1.0f;

	return OutMatrix;
}

FORCEINLINE static constexpr FMatrix GetOrthographicTransform(const int width, const int height)
{
	int Dx = 0;
	int Dy = 0;
	int x = width;
	int y = height;

	float L = static_cast<float>(Dx);
	float R = static_cast<float>(Dx + x);
	float T = static_cast<float>(Dy);
	float B = static_cast<float>(Dy + y);

	FMatrix TempWidgetMatrix;
	TempWidgetMatrix.r[0][0] = 2.0f / (R - L);
	TempWidgetMatrix.r[0][1] = 0.0f;
	TempWidgetMatrix.r[0][2] = 0.0f;
	TempWidgetMatrix.r[0][3] = 0.0f;

	TempWidgetMatrix.r[1][0] = 0.0f;
	TempWidgetMatrix.r[1][1] = 2.0f / (T - B);
	TempWidgetMatrix.r[1][2] = 0.0f;
	TempWidgetMatrix.r[1][3] = 0.0f;

	TempWidgetMatrix.r[2][0] = 0.0f;
	TempWidgetMatrix.r[2][1] = 0.0f;
	TempWidgetMatrix.r[2][2] = 0.5f;
	TempWidgetMatrix.r[2][3] = 0.0f;

	TempWidgetMatrix.r[3][0] = (R + L) / (L - R);
	TempWidgetMatrix.r[3][1] = (T + B) / (B - T);
	TempWidgetMatrix.r[3][2] = 0.5f;
	TempWidgetMatrix.r[3][3] = 1.00000000f;

	return TempWidgetMatrix;
}

FORCEINLINE std::uint32_t vector2ToSnorm8(const FVector2& v)
{
	float scale = 127.0f / sqrtf(v.X * v.X + v.Y * v.Y);
	int x = int(v.X * scale);
	int y = int(v.Y * scale);
	return (x & 0xff) | ((y & 0xff) << 8);
}

FORCEINLINE std::uint32_t vector3ToSnorm8(const FVector& v)
{
	float scale = 127.0f / sqrtf(v.X * v.X + v.Y * v.Y + v.Z * v.Z);
	int x = int(v.X * scale);
	int y = int(v.Y * scale);
	int z = int(v.Z * scale);
	return (x & 0xff) | ((y & 0xff) << 8) | ((z & 0xff) << 16);
}

FORCEINLINE std::uint32_t vector4ToSnorm8(const FVector4& v)
{
	float scale = 127.0f / sqrtf(v.X * v.X + v.Y * v.Y + v.Z * v.Z);
	int x = int(v.X * scale);
	int y = int(v.Y * scale);
	int z = int(v.Z * scale);
	int w = int(v.W * scale);
	return (x & 0xff) | ((y & 0xff) << 8) | ((z & 0xff) << 16) | ((w & 0xff) << 24);
}

FORCEINLINE FVector2 snorm8ToVector2(std::uint32_t v)
{

	float x = static_cast<signed char>(v & 0xff);
	float y = static_cast<signed char>((v >> 8) & 0xff);
	return std::max(FVector2(x, y) / 127.0f, FVector2(-1.f));
}

FORCEINLINE FVector snorm8ToVector3(std::uint32_t v)
{
	float x = static_cast<signed char>(v & 0xff);
	float y = static_cast<signed char>((v >> 8) & 0xff);
	float z = static_cast<signed char>((v >> 16) & 0xff);
	return std::max(FVector(x, y, z) / 127.0f, FVector(-1.f));
}

FORCEINLINE FVector4 snorm8ToVector4(std::uint32_t v)
{
	float x = static_cast<signed char>(v & 0xff);
	float y = static_cast<signed char>((v >> 8) & 0xff);
	float z = static_cast<signed char>((v >> 16) & 0xff);
	float w = static_cast<signed char>((v >> 24) & 0xff);
	return std::max(FVector4(x, y, z, w) / 127.0f, FVector4(-1.f));
}

//}
