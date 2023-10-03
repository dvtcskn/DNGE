/* ---------------------------------------------------------------------------------------
* MIT License
*
* Copyright (c) 2022 Davut Coþkun.
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

#include "pch.h"
#include "cbScaleBox.h"
#include "cbCanvas.h"

namespace cbgui
{
	void cbScaleBox::cbScaleBoxSlot::ReplaceContent(const cbWidget::SharedPtr& pContent)
	{
		if (!pContent)
			return;

		if (Content)
			Content = nullptr;
		Content = pContent;
		Content->AttachToSlot(this);
		OnContentInsertedOrReplaced();
	}

	cbScaleBox::cbScaleBox()
		: Super()
		, Transform(cbTransform(cbDimension(100.0f, 40.0f)))
		, Scaler(cbMargin(10))
		, Slot(nullptr)
		, bIsActive(false)
		, bStayActiveAfterFocusLost(false)
		, fOnFocus(nullptr)
		, fLostOnFocus(nullptr)
	{}

	cbScaleBox::~cbScaleBox()
	{
		Slot = nullptr;
		fOnFocus = nullptr;
		fLostOnFocus = nullptr;
	}

	void cbScaleBox::SetXY(std::optional<float> X, std::optional<float> Y, bool Force)
	{
		if (X.has_value() && Y.has_value())
		{
			if ((HasOwner() || IsAlignedToCanvas()))
			{
				if (Force)
				{
					if (GetVerticalAlignment() != eVerticalAlignment::Align_NONE && GetHorizontalAlignment() != eHorizontalAlignment::Align_NONE)
						return;
				}
				else
				{
					return;
				}
			}
			if (Transform.SetLocation(cbVector(*X, *Y)))
			{
				UpdateSlotAlignments();
				NotifyCanvas_WidgetUpdated();
			}
		}
		else if (X.has_value())
		{
			if ((HasOwner() || IsAlignedToCanvas()))
			{
				if (Force)
				{
					if (GetHorizontalAlignment() != eHorizontalAlignment::Align_NONE)
						return;
				}
				else
				{
					return;
				}
			}
			if (Transform.SetPositionX(*X))
			{
				UpdateSlotHorizontalAlignment();
				NotifyCanvas_WidgetUpdated();
			}
		}
		else if (Y.has_value())
		{
			if ((HasOwner() || IsAlignedToCanvas()))
			{
				if (Force)
				{
					if (GetVerticalAlignment() != eVerticalAlignment::Align_NONE)
						return;
				}
				else
				{
					return;
				}
			}
			if (Transform.SetPositionY(*Y))
			{
				UpdateSlotVerticalAlignment();
				NotifyCanvas_WidgetUpdated();
			}
		}
	}

	void cbScaleBox::SetWidthHeight(std::optional<float> Width, std::optional<float> Height)
	{
		if (Width.has_value() && Height.has_value())
		{
			if (Transform.SetDimension(cbDimension(*Width, *Height)))
			{
				if (HasOwner())
					DimensionUpdated();
				else if (IsAlignedToCanvas())
					AlignToCanvas();
				else
				{
					UpdateSlotAlignments();
					NotifyCanvas_WidgetUpdated();
				}
			}
		}
		else if (Width.has_value())
		{
			if (Transform.SetWidth(*Width))
			{
				if (HasOwner())
					DimensionUpdated();
				else if (IsAlignedToCanvas())
					AlignToCanvas();
				else
				{
					UpdateSlotHorizontalAlignment();
					NotifyCanvas_WidgetUpdated();
				}
			}
		}
		else if (Height.has_value())
		{
			if (Transform.SetHeight(*Height))
			{
				if (HasOwner())
					DimensionUpdated();
				else if (IsAlignedToCanvas())
					AlignToCanvas();
				else
				{
					UpdateSlotVerticalAlignment();
					NotifyCanvas_WidgetUpdated();
				}
			}
		}
	}

	void cbScaleBox::SetRotation(const float Rotation)
	{
		if (Transform.Rotate2D(Rotation))
		{
			if (Slot)
				Slot->UpdateRotation();
			NotifyCanvas_WidgetUpdated();
		}
	}

	void cbScaleBox::SetPadding(const cbMargin& Padding)
	{
		if (Transform.SetPadding(Padding))
		{
			if (HasOwner())
				DimensionUpdated();
			else if (IsAlignedToCanvas())
				AlignToCanvas();
		}
	}

	cbBounds cbScaleBox::GetCulledBounds() const
	{
		return bIsActive ? Super::GetCulledBounds() + Scaler : Super::GetCulledBounds();
	}

	bool cbScaleBox::IsInside(const cbVector& Location) const
	{
		if (bIsActive)
		{
			const float Rotation = GetRotation();
			if (Rotation == 0.0f)
				return cbgui::IsInside(cbBounds(GetDimension() + Scaler, Transform.GetCenter()), Location);
			return cbgui::IsInside(cbBounds(GetDimension() + Scaler, Transform.GetCenter()), Location, Rotation, GetOrigin());
		}
		return Super::IsInside(Location);
	}

	void cbScaleBox::SetScale(const cbMargin& scale)
	{
		Scaler = scale;

		UpdateSlotAlignments();
	}

	void cbScaleBox::SetScale(const std::int32_t& scale)
	{
		Scaler = cbMargin(scale);

		UpdateSlotAlignments();
	}

	void cbScaleBox::Scale(std::optional<bool> Force)
	{
		if (Force.has_value())
		{
			bIsActive = Force.value();
		}
		else
		{
			bIsActive = !bIsActive;
		}
		UpdateSlotAlignments();

		if (bIsActive)
		{
			Slot->SetZOrderMode(eZOrderMode::LastInTheHierarchy);
		}
		else
		{
			if (Slot->GetZOrderMode() == eZOrderMode::LastInTheHierarchy)
				Slot->SetZOrderMode(eZOrderMode::InOrder);
		}
	}

	cbDimension cbScaleBox::GetSlotDimension() const
	{
		return bIsActive ? cbDimension((GetWidth() + Scaler.GetWidth()), (GetHeight() + Scaler.GetHeight())) : GetDimension();
	}

	void cbScaleBox::HideContent(bool value)
	{
		if (!Slot)
			return;

		Slot->Hidden(value);
	}

	void cbScaleBox::UpdateStatus()
	{
		if (!Slot)
			return;

		SetFocus(false);
		Slot->UpdateStatus();
	}

	void cbScaleBox::OnFocus()
	{
		if (!bIsActive)
		{
			Scale(true);

			if (fOnFocus)
				fOnFocus(this);
		}
	}

	void cbScaleBox::OnLostFocus()
	{
		if (bIsActive && !bStayActiveAfterFocusLost)
		{
			Scale(false);

			if (fLostOnFocus)
				fLostOnFocus(this);
		}
	}

	std::vector<cbWidgetObj*> cbScaleBox::GetAllChildren() const
	{
		if (Slot)
			return std::vector<cbWidgetObj*>{ Slot.get() };
		return std::vector<cbWidgetObj*>{ };
	}

	std::vector<cbGeometryVertexData> cbScaleBox::GetVertexData(const bool LineGeometry) const
	{
		if (LineGeometry)
		{
			cbBounds Bounds(GetDimension());
			std::vector<cbVector4> Data;
			Data.push_back(cbVector4(Bounds.GetCorner(0), 0.0f, 1.0f));
			Data.push_back(cbVector4(Bounds.GetCorner(1), 0.0f, 1.0f));
			Data.push_back(cbVector4(Bounds.GetCorner(2), 0.0f, 1.0f));
			Data.push_back(cbVector4(Bounds.GetCorner(3), 0.0f, 1.0f));

			std::vector<cbVector> TC;
			cbBounds Rect(cbDimension(1.0f, 1.0f), cbVector(0.5f, 0.5f));
			const std::array<unsigned int, 4> Edges = { 0,1,2,3 };
			for (std::size_t i = 0; i < 4; i++)
				TC.push_back(Rect.GetCorner(Edges[i]));

			return cbGeometryFactory::GetAlignedVertexData(Data, TC,
				cbColor::White(),
				GetLocation(), GetRotation(), IsRotated() ? GetOrigin() : cbVector::Zero());
		}
		return std::vector<cbGeometryVertexData>();
	};

	std::vector<std::uint32_t> cbScaleBox::GetIndexData(const bool LineGeometry) const
	{
		if (LineGeometry)
			return std::vector<std::uint32_t>{ 0, 1, 1, 2, 2, 3, 3, 0 };
		return std::vector<std::uint32_t>();
	};

	cbGeometryDrawData cbScaleBox::GetGeometryDrawData(const bool LineGeometry) const
	{
		if (LineGeometry)
			return cbGeometryDrawData("Line", 0, 4, 8, 8);
		return cbGeometryDrawData("NONE", 0, 0, 0, 0);
	};

	void cbScaleBox::OnSlotVisibilityChanged(cbSlot* Slot)
	{
		ResetInput();

		if (IsItWrapped())
			Wrap();
		else
			UpdateSlotAlignments();
	}

	void cbScaleBox::OnSlotDimensionUpdated(cbSlot* Slot)
	{
		if (IsItWrapped())
			Wrap();
		else
			UpdateSlotAlignments();
	}

	void cbScaleBox::RemoveContent()
	{
		if (!Slot)
			return;

		ResetInput();

		if (cbICanvas* Canvas = GetCanvas())
			Canvas->SlotRemoved(this, Slot.get());
		Slot = nullptr;
	}

	bool cbScaleBox::OnRemoveSlot(cbSlot* pSlot)
	{
		if (!Slot)
			return false;

		ResetInput();

		if (cbICanvas* Canvas = GetCanvas())
			Canvas->SlotRemoved(this, Slot.get());
		Slot = nullptr;

		return true;
	}

	void cbScaleBox::UpdateSlotVerticalAlignment()
	{
		if (Slot)
			Slot->UpdateVerticalAlignment();
	}

	void cbScaleBox::UpdateSlotHorizontalAlignment()
	{
		if (Slot)
			Slot->UpdateHorizontalAlignment();
	}

	void cbScaleBox::SetContent(const cbWidget::SharedPtr& Content)
	{
		if (!Content)
			return;

		if (!Slot)
		{
			SetSlot(cbScaleBoxSlot::Create(this, Content));
		}
		else
		{
			cbWidget::SharedPtr Old = Slot->GetSharedContent();
			Slot->ReplaceContent(Content);

			ResetInput();

			if (cbICanvas* Canvas = GetCanvas())
				Canvas->SlotContentReplaced(Slot.get(), Old.get(), Content.get());
		}
	}

	void cbScaleBox::SetSlot(const cbScaleBoxSlot::SharedPtr& pSlot)
	{
		if (!pSlot)
			return;

		Slot = nullptr;
		Slot = pSlot;
		Slot->Inserted();

		if (cbICanvas* Canvas = GetCanvas())
			Canvas->NewSlotAdded(this, Slot.get());

		UpdateSlotAlignments();
		if (IsItWrapped())
			Wrap();

		Slot->UpdateRotation();
		Slot->UpdateStatus();
	}

	void cbScaleBox::UpdateVerticalAlignment(const bool ForceAlign)
	{
		if (HasOwner())
		{
			const float OwnerHeight = GetOwner()->GetHeight();
			const std::optional<const eVerticalAlignment> CustomAlignment = GetNonAlignedHeight() > OwnerHeight ? std::optional<eVerticalAlignment>(eVerticalAlignment::Align_Fill) : std::nullopt;

			if (ForceAlign && Transform.IsHeightAligned())
				Transform.ResetHeightAlignment();

			if (Transform.Align(CustomAlignment.has_value() ? CustomAlignment.value() : GetVerticalAlignment(), GetOwner()->GetBounds(), GetVerticalAnchor()) || ForceAlign)
			{
				UpdateSlotVerticalAlignment();
				NotifyCanvas_WidgetUpdated();
			}
		}
		else if (IsAlignedToCanvas())
		{
			const auto Canvas = GetCanvas();
			const float OwnerHeight = Canvas->GetScreenDimension().GetHeight();
			const std::optional<const eVerticalAlignment> CustomAlignment = GetNonAlignedHeight() > OwnerHeight ? std::optional<eVerticalAlignment>(eVerticalAlignment::Align_Fill) : std::nullopt;

			if (ForceAlign && Transform.IsHeightAligned())
				Transform.ResetHeightAlignment();

			if (Transform.Align(CustomAlignment.has_value() ? CustomAlignment.value() : GetVerticalAlignment(), Canvas->GetScreenBounds(), GetVerticalAnchor()) || ForceAlign)
			{
				UpdateSlotVerticalAlignment();
				NotifyCanvas_WidgetUpdated();
			}
			else if (GetVerticalAlignment() == eVerticalAlignment::Align_NONE || ForceAlign)
			{
				const auto CanvasOffset = cbgui::GetAnchorPointsFromRect(GetCanvasAnchor(), Canvas->GetScreenBounds());
				Transform.SetPositionOffsetY(CanvasOffset.Y);
				UpdateSlotVerticalAlignment();
				NotifyCanvas_WidgetUpdated();
			}
		}
		else
		{
			if (Transform.IsHeightAligned())
			{
				Transform.ResetHeightAlignment();
				UpdateSlotVerticalAlignment();
				NotifyCanvas_WidgetUpdated();
			}
			else
			{
				UpdateSlotVerticalAlignment();
			}
		}
	}

	void cbScaleBox::UpdateHorizontalAlignment(const bool ForceAlign)
	{
		if (HasOwner())
		{
			const float OwnerWidth = GetOwner()->GetWidth();
			const std::optional<const eHorizontalAlignment> CustomAlignment = GetNonAlignedWidth() > OwnerWidth ? std::optional<eHorizontalAlignment>(eHorizontalAlignment::Align_Fill) : std::nullopt;

			if (ForceAlign && Transform.IsWidthAligned())
				Transform.ResetWidthAlignment();

			if (Transform.Align(CustomAlignment.has_value() ? CustomAlignment.value() : GetHorizontalAlignment(), GetOwner()->GetBounds(), GetHorizontalAnchor()) || ForceAlign)
			{
				UpdateSlotHorizontalAlignment();
				NotifyCanvas_WidgetUpdated();
			}
		}
		else if (IsAlignedToCanvas())
		{
			const auto Canvas = GetCanvas();
			const float OwnerWidth = Canvas->GetScreenDimension().GetWidth();
			const std::optional<const eHorizontalAlignment> CustomAlignment = GetNonAlignedWidth() > OwnerWidth ? std::optional<eHorizontalAlignment>(eHorizontalAlignment::Align_Fill) : std::nullopt;

			if (ForceAlign && Transform.IsWidthAligned())
				Transform.ResetWidthAlignment();

			if (Transform.Align(CustomAlignment.has_value() ? CustomAlignment.value() : GetHorizontalAlignment(), Canvas->GetScreenBounds(), GetHorizontalAnchor()) || ForceAlign)
			{
				UpdateSlotHorizontalAlignment();
				NotifyCanvas_WidgetUpdated();
			}			
			else if (GetHorizontalAlignment() == eHorizontalAlignment::Align_NONE || ForceAlign)
			{
				const auto CanvasOffset = cbgui::GetAnchorPointsFromRect(GetCanvasAnchor(), Canvas->GetScreenBounds());
				Transform.SetPositionOffsetX(CanvasOffset.X);
				UpdateSlotHorizontalAlignment();
				NotifyCanvas_WidgetUpdated();
			}
		}
		else
		{
			if (Transform.IsWidthAligned())
			{
				Transform.ResetWidthAlignment();
				UpdateSlotHorizontalAlignment();
				NotifyCanvas_WidgetUpdated();
			}
			else
			{
				UpdateSlotHorizontalAlignment();
			}
		}
	}

	void cbScaleBox::UpdateRotation()
	{
		if (HasOwner())
			Transform.SetRollOffset(GetOwner()->GetRotation());
		else if (IsAlignedToCanvas())
			Transform.SetRollOffset(GetCanvas()->GetScreenRotation());
		else
			Transform.SetRollOffset(0.0f);

		if (Slot)
			Slot->UpdateRotation();
		NotifyCanvas_WidgetUpdated();
	}

	bool cbScaleBox::WrapVertical()
	{
		if (!IsItWrapped())
			return false;

		float Height = 0.0;
		{
			const auto& Content = Slot->GetContent();
			const float SlotHeight = Content->GetNonAlignedHeight();
			const cbMargin& Padding = Content->GetPadding();
			if ((SlotHeight + (Padding.Top + Padding.Bottom)) > Height)
				Height += SlotHeight + (Padding.Top + Padding.Bottom);
		}

		return Transform.CompressHeight(Height);
	}

	bool cbScaleBox::WrapHorizontal()
	{
		if (!IsItWrapped())
			return false;

		float Width = 0.0f;
		{
			const auto& Content = Slot->GetContent();
			const float SlotWidth = Content->GetNonAlignedWidth();
			const cbMargin& Padding = Content->GetPadding();
			if ((SlotWidth + (Padding.Left + Padding.Right)) > Width)
				Width += SlotWidth + (Padding.Left + Padding.Right);
		}

		return Transform.CompressWidth(Width);
	}

	bool cbScaleBox::UnWrapVertical()
	{
		if (Transform.IsHeightCompressed())
		{
			Transform.ResetHeightCompressed();
			return true;
		}
		return false;
	}

	bool cbScaleBox::UnWrapHorizontal()
	{
		if (Transform.IsWidthCompressed())
		{
			Transform.ResetWidthCompressed();
			return true;
		}
		return false;
	}

	void cbScaleBox::OnAttach()
	{
		WrapVertical();
		WrapHorizontal();
		NotifyCanvas_WidgetUpdated();
		UpdateSlotAlignments();
	}

	void cbScaleBox::OnRemoveFromParent()
	{
		if (Transform.IsWidthAligned() || Transform.IsHeightAligned())
		{
			Transform.ResetWidthAlignment();
			Transform.ResetHeightAlignment();
			NotifyCanvas_WidgetUpdated();
		}

		UnWrap();
	}
}
