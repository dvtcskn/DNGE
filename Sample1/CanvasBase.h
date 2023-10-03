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
#pragma once

#include <cbgui.h>
#include <unordered_map>
#include <Gameplay/ICanvas.h>
#include "MetaWorld.h"

class IVertexBuffer;
class IIndexBuffer;

namespace cbgui
{
	class sCanvasBase : public ICanvas
	{
		cbClassBody(cbClassConstructor, sCanvasBase, ICanvas)
	public:
		sCanvasBase(IMetaWorld* pMetaWorld);

	public:
		virtual ~sCanvasBase();

		virtual void BeginPlay() override;
		virtual void Tick(const double DeltaTime) override;
		virtual void FixedUpdate(const double DeltaTime);

		virtual void ResizeWindow(std::size_t Width, std::size_t Height) override;

		virtual void InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar) override;

		virtual std::string GetName() const override { return Name; }
		void SetName(const std::string& name) { Name = name; }

		virtual cbVector GetCenter() const override { return Transform.GetCenter(); }
		virtual cbBounds GetScreenBounds() const override { return Transform.GetBounds(); }
		virtual float GetScreenRotation() const override { return Transform.GetRotation(); }
		virtual cbDimension GetScreenDimension() const override { return Transform.GetDimension(); }
		void SetScreenDimension(const cbDimension& pScreenDimension);

		virtual void ZOrderModeUpdated(cbWidgetObj* Object) override {}
		virtual void ZOrderChanged(cbWidgetObj* Object, const std::int32_t ZOrder);

		virtual void Add(cbWidget* Object) override;
		virtual void Add(const std::shared_ptr<cbWidget>& Object) override;
		virtual bool IsWidgetExist(cbWidget* Widget) const override;
		virtual void RemoveFromCanvas(cbWidget* Object) override;

		virtual void WidgetUpdated(cbWidgetObj* Object) override;

		virtual void NewSlotAdded(cbSlottedBox* Parent, cbSlot* Slot) override;
		virtual void SlotRemoved(cbSlottedBox* Parent, cbSlot* Slot) override;
		virtual void SlotContentReplaced(cbSlot* Parent, cbWidget* Old, cbWidget* New) override;

		virtual void VisibilityChanged(cbWidgetObj* Object) override {}

		virtual void VerticesSizeChanged(cbWidgetObj* Object, const std::size_t NewSize) override;

		virtual void NewSlotContentAdded(cbSlot* Parent, cbWidget* Content);

		virtual std::vector<cbWidget*> GetOverlappingWidgets(const cbBounds& Bounds) const override;

		std::vector<cbWidget::SharedPtr> GetWidgets() const { return Widgets; }
		virtual std::vector<WidgetHierarchy*> GetWidgetHierarchy() const override { return Hierarchy; }

		/*
		* Set material by class ID, geometry component name, or geometry type.
		*/
		virtual void SetMaterial(WidgetHierarchy* pWP);

		virtual void RealignWidgets() override final;

		virtual IVertexBuffer* GetVertexBuffer() const { return VertexBuffer.get(); }
		virtual IIndexBuffer* GetIndexBuffer() const { return IndexBuffer.get(); }

	private:
		void SortWidgetsByZOrder();
		void ReorderWidgets();
		void ReuploadGeometry();

		void SetVertexOffset(WidgetHierarchy* pWP);
		void SetIndexOffset(WidgetHierarchy* pWP);

	private:
		cbgui::cbTransform Transform;

		cbWidget* FocusedObj;

		std::string Name;

		std::vector<cbWidget::SharedPtr> Widgets;
		std::vector<WidgetHierarchy*> Hierarchy;

		std::map<cbWidgetObj*, WidgetHierarchy*> WidgetsList;

		std::size_t PrevVertexOffset;
		std::size_t PrevIndexOffset;

		UIMaterialStyle::SharedPtr DefaultGUIFlatColorMatStyle;
		UIFontMaterialStyle::SharedPtr DefaultGUIFontMatStyle;

		IVertexBuffer::SharedPtr VertexBuffer;
		IIndexBuffer::SharedPtr IndexBuffer;
	};
}
