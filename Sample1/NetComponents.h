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

#include <Engine/ClassBody.h>
#include <Engine/AbstractEngine.h>
#include <Gameplay/BoxCollision2DComponent.h>
#include <Gameplay/CircleCollision2DComponent.h>

class sNet_BoxCollision2DComponent : public sBoxCollision2DComponent
{
	sClassBody(sClassConstructor, sNet_BoxCollision2DComponent, sBoxCollision2DComponent)
public:
	sNet_BoxCollision2DComponent(std::string InName, const sRigidBodyDesc& Desc, const FDimension2D& Dimension, sActor* pActor = nullptr)
		: Super(InName, Desc, Dimension, pActor)
	{}
	virtual ~sNet_BoxCollision2DComponent()
	{}

	virtual void Replicate(bool bReplicate) override
	{
		if (IsReplicated() == bReplicate)
			return;

		Super::Replicate(bReplicate);

		if (IsReplicated())
		{
			RegisterRPCfn(GetClassNetworkAddress(), GetName(), "SetTransform_Client", eRPCType::Client, false, false, std::bind(&sNet_BoxCollision2DComponent::Net_SetTransform_Smoothing_Client, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3/*, std::placeholders::_4*/),/* sDateTime,*/ FVector, FVector4, FVector);
		}
		else
		{
			Network::UnregisterRPC(GetClassNetworkAddress(), GetName());
		}
	}

	/*
	* Time is half of ping in ms
	*/
	void Net_SetTransform_Smoothing_Client(/*const sDateTime& Time,*/ const FVector& InLocation, const FVector4& InRotation, const FVector InScale)
	{
		if (Network::IsHost())
			return;

		if (!IsReplicated())
			return;

		auto Location = GetRelativeLocation();

		if (Location != InLocation)
		{
			//auto UTCTimeNow = Engine::GetUTCTimeNow();
			//auto MS = UTCTimeNow.GetTotalMillisecond() - Time.GetTotalMillisecond();
			//Engine::WriteToConsole("Net_SetTransform_Predication_Client : " + std::to_string(MS));

			Location = Lerp(Location, InLocation, 0.5f);

			float Diff_X = std::abs(Location.X - InLocation.X);
			float Diff_Y = std::abs(Location.Y - InLocation.Y);

			float Tolerance_X = 15.0F;
			float Tolerance_Y = 15.0F;

			if (Diff_X > Tolerance_X || Diff_Y > Tolerance_Y)
			{
				Location = InLocation;
				SetTransform(Location, InRotation, InScale);
			}
			else
			{
				SetTransform(Location, InRotation, InScale);
			}
		}
	}
};

class sNet_CircleCollision2DComponent : public sCircleCollision2DComponent
{
	sClassBody(sClassConstructor, sNet_CircleCollision2DComponent, sCircleCollision2DComponent)
public:
	sNet_CircleCollision2DComponent(std::string InName, const sRigidBodyDesc& Desc, const FVector2& Origin, float InRadius, sActor* pActor = nullptr)
		: Super(InName, Desc, Origin, InRadius, pActor)
	{}
	virtual ~sNet_CircleCollision2DComponent()
	{}

	virtual void Replicate(bool bReplicate) override
	{
		if (IsReplicated() == bReplicate)
			return;

		Super::Replicate(bReplicate);

		if (IsReplicated())
		{
			RegisterRPCfn(GetClassNetworkAddress(), GetName(), "SetTransform_Client", eRPCType::Client, false, false, std::bind(&sNet_CircleCollision2DComponent::Net_SetTransform_Smoothing_Client, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3/*, std::placeholders::_4*/),/* sDateTime,*/ FVector, FVector4, FVector);
		}
		else
		{
			Network::UnregisterRPC(GetClassNetworkAddress(), GetName());
		}
	}

	/*
	* Time is half of ping in ms
	*/
	void Net_SetTransform_Smoothing_Client(/*const sDateTime& Time,*/ const FVector& InLocation, const FVector4& InRotation, const FVector InScale)
	{
		if (Network::IsHost())
			return;

		if (!IsReplicated())
			return;

		auto Location = GetRelativeLocation();

		if (Location != InLocation)
		{
			//auto UTCTimeNow = Engine::GetUTCTimeNow();
			//auto MS = UTCTimeNow.GetTotalMillisecond() - Time.GetTotalMillisecond();
			//Engine::WriteToConsole("Net_SetTransform_Predication_Client : " + std::to_string(MS));

			Location = Lerp(Location, InLocation, 0.5f);

			float Diff_X = std::abs(Location.X - InLocation.X);
			float Diff_Y = std::abs(Location.Y - InLocation.Y);

			float Tolerance_X = 15.0F;
			float Tolerance_Y = 15.0F;

			if (Diff_X > Tolerance_X || Diff_Y > Tolerance_Y)
			{
				Location = InLocation;
				SetTransform(Location, InRotation, InScale);
			}
			else
			{
				SetTransform(Location, InRotation, InScale);
			}
		}
	}
};
