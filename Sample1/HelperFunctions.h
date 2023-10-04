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

//#include "GPlayerCharacter.h"

namespace
{
	bool IsOnGround(const FBoundingBox& Bound)
	{
		auto Dimension3D = Bound.GetDimension();
		auto Center = Bound.GetCenter();
		FBounds2D Bounds(FDimension2D(Dimension3D.Width, 2.0f), FVector2(Center.X, Bound.Max.Y));
		auto Result = Physics::QueryAABB(Bounds);

		for (const auto& Res : Result)
		{
			if (!Res)
				continue;

			if (Res->HasTag("Jumpable_Ground"))
			{
				return true;
			}
		}

		return false;
	}

	bool IsOnTrap(const FBoundingBox& Bound)
	{
		auto Dimension3D = Bound.GetDimension();
		auto Center = Bound.GetCenter();
		FBounds2D Bounds(FDimension2D(Dimension3D.Width, 2.0f), FVector2(Center.X, Bound.Max.Y));
		auto Result = Physics::QueryAABB(Bounds);

		for (const auto& Res : Result)
		{
			if (!Res)
				continue;

			if (Res->HasTag("Trap"))
			{
				return true;
			}
		}

		return false;
	}

	enum class eTagDirectionType
	{
		eUp,
		eLeft,
		eRight,
		eDown,
	};

	bool CheckTag(const FBoundingBox& Bound, eTagDirectionType Type, std::string Tag, bool JumpTest = false, std::optional<float> WidthHeight = std::nullopt)
	{
		auto Dimension3D = Bound.GetDimension();
		auto Center = Bound.GetCenter();
		std::vector<sPhysicalComponent*> Result;

		switch (Type)
		{
		case eTagDirectionType::eUp:
		{
			FBounds2D Bounds(FDimension2D(WidthHeight.has_value() ? *WidthHeight : 0.05f, 2.0f), FVector2(Center.X, Bound.Min.Y));
			Result = Physics::QueryAABB(Bounds);
		}
		break;
		case eTagDirectionType::eLeft:
		{
			FBounds2D Bounds(FDimension2D(WidthHeight.has_value() ? *WidthHeight : 2.0f, Dimension3D.Height), FVector2(Bound.Min.X, Center.Y));
			Result = Physics::QueryAABB(Bounds);
		}
		break;
		case eTagDirectionType::eRight:
		{
			FBounds2D Bounds(FDimension2D(WidthHeight.has_value() ? *WidthHeight : 2.0f, Dimension3D.Height), FVector2(Bound.Max.X, Center.Y));
			Result = Physics::QueryAABB(Bounds);
		}
		break;
		case eTagDirectionType::eDown:
		{
			FBounds2D Bounds(FDimension2D(WidthHeight.has_value() ? *WidthHeight : 2.0f, Dimension3D.Height), FVector2(Center.X, Bound.Max.Y));
			Result = Physics::QueryAABB(Bounds);
		}
		break;
		}

		for (const auto& Res : Result)
		{
			if (!Res)
				continue;

			if (JumpTest && Res->HasTag("PlayerCharacter"))
			{
				if (Res->GetOwner<GPlayerCharacter>()->GetIsJumping())
					return false;
				if (Res->HasTag(Tag))
				{
					return true;
				}
			}
			else if (Res->HasTag(Tag))
			{
				return true;
			}
		}

		return false;
	}
}
