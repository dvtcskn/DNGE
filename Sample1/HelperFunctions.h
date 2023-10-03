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
