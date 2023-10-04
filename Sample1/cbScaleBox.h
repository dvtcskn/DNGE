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

#include "cbSlottedBox.h"
#include "cbGeometry.h"

namespace cbgui
{
	class cbScaleBox : public cbSlottedBox
	{
		cbClassBody(cbClassConstructor, cbScaleBox, cbSlottedBox);
	protected:
		class cbScaleBoxSlot : public cbSlot
		{
			cbClassBody(cbClassConstructor, cbScaleBoxSlot, cbSlot)
			friend cbScaleBox;
		private:
			bool bIsInserted;
			std::shared_ptr<cbWidget> Content;
			eZOrderMode ZOrderMode;

			void Inserted() { bIsInserted = true; OnInserted(); }
			virtual void OnInserted() { }

		public:
			cbScaleBoxSlot(cbScaleBox* pOwner, const cbWidget::SharedPtr& pContent = nullptr)
				: Super(pOwner)
				, bIsInserted(false)
				, Content(pContent)
				, ZOrderMode(eZOrderMode::InOrder)
			{
				if (HasContent())
					Content->AttachToSlot(this);
			}

			virtual ~cbScaleBoxSlot()
			{
				Content = nullptr;
			}

		public:
			virtual bool IsInserted() const override final { return HasOwner() && bIsInserted; }

			void ReplaceContent(const cbWidget::SharedPtr& pContent);

			virtual bool HasContent() const override { return Content != nullptr; }
			virtual cbWidget::SharedPtr GetSharedContent() const override { return Content; }
			virtual cbWidget* GetContent() const override { return Content.get(); }

			virtual eZOrderMode GetZOrderMode() const override final
			{
				return ZOrderMode;
			}
			inline void SetZOrderMode(const eZOrderMode Mode)
			{
				ZOrderMode = Mode;
			}

			virtual bool HasAnyChildren() const { return Content != nullptr; }
			virtual std::vector<cbWidgetObj*> GetAllChildren() const { return std::vector<cbWidgetObj*>{ Content.get() }; }

			virtual cbBounds GetBounds() const override { return cbBounds(GetOwner<cbScaleBox>()->GetSlotDimension(), GetOwner<cbScaleBox>()->GetLocation()); }
			virtual cbVector GetLocation() const override { return GetOwner<cbScaleBox>()->GetLocation(); }
			virtual cbDimension GetDimension() const override { return GetOwner<cbScaleBox>()->GetSlotDimension(); }
			virtual float GetWidth() const override { return GetDimension().GetWidth(); }
			virtual float GetHeight() const override { return GetDimension().GetHeight(); }

			virtual void UpdateVerticalAlignment() override { if (HasContent()) Content->UpdateVerticalAlignment(); }
			virtual void UpdateHorizontalAlignment() override { if (HasContent()) Content->UpdateHorizontalAlignment(); }
			virtual void UpdateRotation() override { if (HasContent()) Content->UpdateRotation(); }
		};

	public:
		cbScaleBox();
	public:
		virtual ~cbScaleBox();

	public:
		virtual cbVector GetLocation() const override final { return Transform.GetCenter(); }
		virtual float GetRotation() const override final { return Transform.GetRotation(); }
		virtual cbBounds GetBounds() const override final { return Transform.GetBounds(); }
		virtual cbMargin GetPadding() const override final { return Transform.GetPadding(); }
		virtual cbDimension GetDimension() const override final { return Transform.GetDimension(); }
		virtual float GetWidth() const override final { return Transform.GetWidth(); }
		virtual float GetHeight() const override final { return Transform.GetHeight(); }

		virtual float GetNonAlignedWidth() const override final { return Transform.GetNonAlignedWidth(); }
		virtual float GetNonAlignedHeight() const override final { return Transform.GetNonAlignedHeight(); }

		virtual bool IsInside(const cbVector& Location) const override;

		virtual void SetXY(std::optional<float> X, std::optional<float> Y, bool Force = false);
		virtual void SetWidthHeight(std::optional<float> Width, std::optional<float> Height);
		virtual void SetPadding(const cbMargin& Padding) override final;
		virtual void SetRotation(const float Rotation) override final;

		virtual cbBounds GetCulledBounds() const override;

		virtual void UpdateVerticalAlignment(const bool ForceAlign = false) override final;
		virtual void UpdateHorizontalAlignment(const bool ForceAlign = false) override final;
		virtual void UpdateRotation() override final;
		virtual void UpdateStatus() override final;

		inline void BindFunctionTo_OnFocus(std::function<void(cbScaleBox*)> Function) { fOnFocus = Function; }
		inline void UnBindFunctionTo_OnFocus() { fOnFocus = nullptr; }
		inline void BindFunctionTo_LostOnFocus(std::function<void(cbScaleBox*)> Function) { fLostOnFocus = Function; }
		inline void UnBindFunctionTo_LostOnFocus() { fLostOnFocus = nullptr; }

		virtual void OnFocus() override;
		virtual void OnLostFocus() override;

		void SetStayActiveAfterFocusLost(bool value) { bStayActiveAfterFocusLost = value; }
		bool StayActiveAfterFocusLost() const { return bStayActiveAfterFocusLost; }

	private:
		virtual void UpdateSlotVerticalAlignment() override final;
		virtual void UpdateSlotHorizontalAlignment() override final;

		virtual std::size_t GetSlotIndex(const cbSlot* Slot) const override final { return 0; };

	public:
		inline cbMargin GetScale() const { return Scaler; }
		void Scale(std::optional<bool> Force = std::nullopt);

		void SetScale(const cbMargin& Scale);
		void SetScale(const std::int32_t& Scale);

		virtual std::vector<cbSlot*> GetSlots() const override { return std::vector<cbSlot*>{ Slot.get() }; }
		virtual cbSlot* GetSlot(const std::size_t Index = 0) const override final { return Index == 0 ? Slot.get() : nullptr; }
		virtual std::size_t GetSlotSize(const bool ExcludeHidden = false) const override final
		{
			if (Slot)
			{
				if (ExcludeHidden && Slot->IsHidden())
					return 0;
				return 1;
			}
			return 0;
		}

		void SetContent(const cbWidget::SharedPtr& Content);
	protected:
		void SetSlot(const cbScaleBoxSlot::SharedPtr& Slot);
	public:
		cbWidget* GetContent() const { return Slot ? Slot->GetContent() : nullptr; }

		void HideContent(bool value);
		void RemoveContent();

		virtual std::vector<cbWidgetObj*> GetAllChildren() const override;
		virtual bool HasAnyChildren() const { return Slot != nullptr; }

		virtual bool HasAnyComponents() const override final { return false; }
		virtual std::vector<cbComponent*> GetAllComponents() const override final { return std::vector<cbComponent*>(); }

		virtual bool HasGeometry() const override final { return false; }
		virtual std::vector<cbGeometryVertexData> GetVertexData(const bool LineGeometry = false) const override final;
		virtual std::vector<std::uint32_t> GetIndexData(const bool LineGeometry = false) const override final;
		virtual cbGeometryDrawData GetGeometryDrawData(const bool LineGeometry = false) const override final;

	protected:
		virtual cbDimension GetSlotDimension() const;

	private:
		virtual void OnAttach() override final;
		virtual void OnRemoveFromParent() override final;

		virtual bool WrapVertical() override final;
		virtual bool WrapHorizontal() override final;
		virtual bool UnWrapVertical() override final;
		virtual bool UnWrapHorizontal() override final;

		virtual void OnSlotVisibilityChanged(cbSlot* Slot);
		virtual void OnSlotDimensionUpdated(cbSlot* Slot);

		virtual bool OnRemoveSlot(cbSlot* Slot) override;

	private:
		cbTransform Transform;
		cbMargin Scaler;
		cbScaleBoxSlot::SharedPtr Slot;
		bool bIsActive;
		bool bStayActiveAfterFocusLost;

		std::function<void(cbScaleBox*)> fOnFocus;
		std::function<void(cbScaleBox*)> fLostOnFocus;
	};
}
