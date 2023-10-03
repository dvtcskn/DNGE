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
