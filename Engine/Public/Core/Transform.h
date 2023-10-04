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

#include "Math/CoreMath.h"

class Transform
{
public:

	Transform()
		: Rotation(FVector4::Zero())
		, Location(0.f, 0.f, 0.f)
		, Scale3D(1.f, 1.f, 1.f)
	{}

	explicit Transform(const FVector& InLocation)
		: Rotation(FVector4::Zero())
		, Location(InLocation)
		, Scale3D(1.f, 1.f, 1.f)
	{
	}

	Transform(const FVector& InLocation, const FVector4& InRotation, const FVector& InScale = FVector(1.0f, 1.0f, 1.0f))
		: Rotation(InRotation)
		, Location(InLocation)
		, Scale3D(InScale)
	{
	}

	~Transform() = default;

	FORCEINLINE FVector GetLocation() const
	{
		return Location;
	}

	FORCEINLINE void SetLocation(const FVector Origin)
	{
		Location = Origin;
	}

	FORCEINLINE FVector GetScale() const
	{
		return Scale3D;
	}

	FORCEINLINE void SetScale(const FVector& InScale)
	{
		Scale3D = InScale;
	}

	FORCEINLINE FVector4 GetRotation() const
	{
		return Rotation;
	}

	FORCEINLINE void SetRotation(const FVector4& InRotation)
	{
		Rotation = InRotation;
	}

private:
	FVector4 Rotation;
	FVector	Location;
	FVector	Scale3D;
};
