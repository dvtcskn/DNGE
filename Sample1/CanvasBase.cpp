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
#include "CanvasBase.h"
#include "cbString.h"
#include <Engine/AbstractEngine.h>
#include <AbstractGI/MaterialManager.h>

#define CanvasConsole(x) std::cout << x << std::endl;

namespace cbgui
{
	sCanvasBase::sCanvasBase(IMetaWorld* pMetaWorld)
		: Super(pMetaWorld)
		, Transform(cbTransform())
		, PrevVertexOffset(0)
		, PrevIndexOffset(0)
		, FocusedObj(nullptr)
	{
		const auto& Screen = GPU::GetBackBufferDimension();
		Transform.SetDimension(cbDimension((float)Screen.Width, (float)Screen.Height));

		const auto& Dimension = Transform.GetDimension();
		Transform.SetLocation(cbVector(Dimension.Width / 2.0f, Dimension.Height / 2.0f));

		{
			sBufferDesc Desc;
			Desc.Size = 3000000;// (std::uint32_t)(sizeof(cbGeometryVertexData));
			Desc.Stride = sizeof(cbgui::cbGeometryVertexData);
			VertexBuffer = IVertexBuffer::Create("sCanvasBaseVB", Desc, nullptr);
		}
		{
			sBufferDesc Desc;
			Desc.Size = 300000;// (std::uint32_t)(sizeof(int));
			Desc.Stride = 0;
			IndexBuffer = IIndexBuffer::Create("sCanvasBaseIB", Desc, nullptr);
		}

		{
			sMaterial::sMaterialInstance* FontInstance = sMaterialManager::Get().GetMaterialInstance("Default_Font_Mat", "Default_Font_Mat_Instance");
			DefaultGUIFontMatStyle = UIFontMaterialStyle::Create("Font", "DejaVu Sans", UIMaterial::Create(FontInstance));
			FontInstance = nullptr;
		}

		{
			sMaterial::sMaterialInstance* FlatColorInstance = sMaterialManager::Get().GetMaterialInstance("Default_GUI_Mat", "Default_GUI_MatInstance");
			DefaultGUIFlatColorMatStyle = UIMaterialStyle::Create("Image", UIMaterial::Create(FlatColorInstance));
			FlatColorInstance = nullptr;
		}
	}

	sCanvasBase::~sCanvasBase()
	{
		DefaultGUIFontMatStyle = nullptr;

		DefaultGUIFlatColorMatStyle = nullptr;

		for (auto& UI : Widgets)
		{
			UI = nullptr;
		}
		Widgets.clear();

		for (auto& Widget : Hierarchy)
		{
			delete Widget;
			Widget = nullptr;
		}
		Hierarchy.clear();

		WidgetsList.clear();

		VertexBuffer = nullptr;
		IndexBuffer = nullptr;
	}

	void sCanvasBase::BeginPlay()
	{
		for (auto& Obj : Widgets)
			Obj->BeginPlay();
	}

	void sCanvasBase::Tick(const double DeltaTime)
	{
		for (auto& Obj : Widgets)
			Obj->Tick((float)DeltaTime);
	}

	void sCanvasBase::FixedUpdate(const double DeltaTime)
	{

	}

	void sCanvasBase::ResizeWindow(std::size_t Width, std::size_t Height)
	{
		Transform = cbgui::cbTransform(cbgui::cbDimension((float)Width, (float)Height));
		Transform.SetLocation(cbgui::cbVector(Width / 2.0f, Height / 2.0f));

		for (const auto& Widget : Widgets)
			Widget->UpdateAlignments(true);
	}

	void sCanvasBase::InputProcess(const GMouseInput& InMouseInput, const GKeyboardChar& KeyboardChar)
	{
		if (!KeyboardChar.bIsIdle)
		{
			if (KeyboardChar.bIsPressed && (KeyboardChar.bIsChar || KeyboardChar.KeyCode < 32))
			{
				for (const auto& Obj : Widgets)
					Obj->OnKeyDown(KeyboardChar.KeyCode);
			}
			else
			{
				for (const auto& Obj : Widgets)
					Obj->OnKeyUp(KeyboardChar.KeyCode);
			}
		}

		if (InMouseInput.State == EMouseState::eMoving)
		{
			cbMouseInput MouseInput;
			MouseInput.State = cbMouseState::Moving;
			MouseInput.MouseLocation = cbVector(InMouseInput.MouseLocation.X, InMouseInput.MouseLocation.Y);

			if (FocusedObj)
			{
				if (FocusedObj->IsInside(MouseInput.MouseLocation))
				{
					FocusedObj->OnMouseMove(MouseInput);
					//CanvasConsole("OnMouseMove");
					return;
				}
				else
				{
					if (FocusedObj->OnMouseLeave(MouseInput))
					{
						//CanvasConsole("OnMouseMove/OnMouseLeave");
						FocusedObj = nullptr;
					}
					else
					{
						FocusedObj->OnMouseMove(MouseInput);
						//CanvasConsole("OnMouseMove");
						return;
					}
				}
			}

			cbWidget* Focus = nullptr;

			for (auto& Obj : Widgets)
			{
				if (Obj->IsInside(MouseInput.MouseLocation))
				{
					if (Obj->IsFocusable())
						Focus = Obj.get();
				}
				else if (Obj->IsFocused())
				{
					Obj->OnMouseLeave(MouseInput);
				}
			}

			if (Focus)
			{
				if (FocusedObj && FocusedObj != Focus)
				{
					FocusedObj->OnMouseLeave(MouseInput);
					FocusedObj = nullptr;
				}

				if (!Focus->OnMouseEnter(MouseInput))
					Focus = nullptr;

				FocusedObj = Focus;
			}

		}
		else if (InMouseInput.State == EMouseState::eScroll)
		{
			if (FocusedObj)
			{
				cbMouseInput MouseInput;
				MouseInput.MouseLocation = cbVector(InMouseInput.MouseLocation.X, InMouseInput.MouseLocation.Y);

				if (FocusedObj->IsInside(MouseInput.MouseLocation))
					FocusedObj->OnMouseWheel(InMouseInput.WheelDelta, MouseInput);
				return;
			}
		}

		if (InMouseInput.Buttons.at("Left") == EMouseButtonState::ePressed)
		{
			cbMouseInput MouseInput;
			MouseInput.Buttons["Left"] = cbMouseButtonState::Pressed;
			MouseInput.MouseLocation = cbVector(InMouseInput.MouseLocation.X, InMouseInput.MouseLocation.Y);

			if (FocusedObj)
			{
				if (!FocusedObj->OnMouseButtonDown(MouseInput))
					FocusedObj = nullptr;
			}
			else
			{
				return;
			}
		}
		else if (InMouseInput.Buttons.at("Left") == EMouseButtonState::eReleased)
		{
			cbMouseInput MouseInput;
			MouseInput.Buttons["Left"] = cbMouseButtonState::Released;
			MouseInput.MouseLocation = cbVector(InMouseInput.MouseLocation.X, InMouseInput.MouseLocation.Y);

			if (FocusedObj)
			{
				if (FocusedObj->OnMouseButtonUp(MouseInput))
				{
					if (FocusedObj->IsFocused())
					{
						if (!FocusedObj->IsInside(MouseInput.MouseLocation))
						{
							if (FocusedObj->OnMouseLeave(MouseInput))
								FocusedObj = nullptr;
						}
						else
						{
							return;
						}
					}
					else
					{
						FocusedObj = nullptr;
					}
				}
			}
		}
	}

	void sCanvasBase::ReuploadGeometry()
	{
		std::function<void(WidgetHierarchy*)> it;
		it = [&](WidgetHierarchy* Widget)
		{
			if (Widget->Widget->HasGeometry())
			{
				Widget->bVertexDirty = true;
				Widget->bIndexDirty = true;
			}

			for (const auto& Widget : Widget->Nodes)
				it(Widget);
		};

		for (const auto& Widget : Hierarchy)
			it(Widget);
	}

	void sCanvasBase::SetScreenDimension(const cbDimension& pScreenDimension)
	{
		Transform.SetDimension(pScreenDimension);

		for (const auto& Obj : Widgets)
		{
			if (Obj->IsAlignedToCanvas())
				Obj->UpdateAlignments();
		}
	}

	void sCanvasBase::ZOrderChanged(cbWidgetObj* Object, const std::int32_t ZOrder)
	{
		if (Object->HasOwner())
		{
			std::function<WidgetHierarchy* (WidgetHierarchy*, const cbWidgetObj*)> GetParentHierarchy;
			GetParentHierarchy = [&](WidgetHierarchy* pWidgetHierarchy, const cbWidgetObj* pParent) -> WidgetHierarchy*
			{
				for (auto& WidgetHierarchy : pWidgetHierarchy->Nodes)
				{
					if (WidgetHierarchy->Widget == pParent)
					{
						return WidgetHierarchy;
					}
					else if (auto WP = GetParentHierarchy(WidgetHierarchy, pParent))
					{
						return WP;
					}
				}
				return nullptr;
			};

			WidgetHierarchy* pWH = nullptr;
			for (auto& pWidgetHierarchy : Hierarchy)
			{
				if (pWidgetHierarchy->Widget == Object->GetOwner()->GetOwner())
				{
					pWH = pWidgetHierarchy;
					break;
				}
				else if (auto WP = GetParentHierarchy(pWidgetHierarchy, Object->GetOwner()->GetOwner()))
				{
					pWH = WP;
					break;
				}
			}

			if (pWH)
			{
				std::sort(pWH->Nodes.begin(), pWH->Nodes.end(), [](const WidgetHierarchy* Widged1, const WidgetHierarchy* Widged2)
					{
						return (Widged1->Widget->GetZOrder() < Widged2->Widget->GetZOrder());
					});
			}
		}
		else
		{
			SortWidgetsByZOrder();
		}
	}

	/*
	* Set material by class ID, geometry component name, or geometry type.
	*/
	void sCanvasBase::SetMaterial(WidgetHierarchy* pWH)
	{
		if (pWH->Widget->HasGeometry())
		{
			auto DrawData = pWH->Widget->GetGeometryDrawData();
			if (DrawData.GeometryType == "Button")
				pWH->MaterialStyle = DefaultGUIFlatColorMatStyle.get();
			else if (DrawData.GeometryType == "CheckBox")
				pWH->MaterialStyle = DefaultGUIFlatColorMatStyle.get();
			else if (DrawData.GeometryType == "Plane")
				pWH->MaterialStyle = DefaultGUIFlatColorMatStyle.get();
			else if (DrawData.GeometryType == "Border")
				pWH->MaterialStyle = DefaultGUIFlatColorMatStyle.get();
			else if (DrawData.GeometryType == "Text")
				pWH->MaterialStyle = DefaultGUIFontMatStyle.get();
			else if (DrawData.GeometryType == "Line")
				pWH->MaterialStyle = DefaultGUIFlatColorMatStyle.get();
			else
				DebugBreak();
		}
	}

	void sCanvasBase::RealignWidgets()
	{
		for (const auto& Obj : Widgets)
			Obj->UpdateAlignments(true);
	}

	void sCanvasBase::SortWidgetsByZOrder()
	{
		std::sort(Widgets.begin(), Widgets.end(), [](const cbWidget::SharedPtr& Widged1, const cbWidget::SharedPtr& Widged2)
			{
				return (Widged1->GetZOrder() < Widged2->GetZOrder());
			});
		std::sort(Hierarchy.begin(), Hierarchy.end(), [](WidgetHierarchy* Widged1, WidgetHierarchy* Widged2)
			{
				return (Widged1->Widget->GetZOrder() < Widged2->Widget->GetZOrder());
			});
	}

	void sCanvasBase::Add(cbWidget* Widget)
	{
	}

	void sCanvasBase::Add(const std::shared_ptr<cbWidget>& Widget)
	{
		if (IsWidgetExist(Widget.get()))
			return;

		Widgets.push_back(Widget);

		std::function<void(WidgetHierarchy*, cbWidgetObj*)> SetHierarchy;
		SetHierarchy = [&](WidgetHierarchy* pWidgetHierarchy, cbWidgetObj* pWidget)
		{
			if (pWidget->HasAnyChildren())
			{
				const auto& Children = pWidget->GetAllChildren();
				for (const auto& Child : Children)
				{
					if (!Child)
						continue;

					WidgetHierarchy* pHierarchy = new WidgetHierarchy;
					pHierarchy->Widget = Child;
					SetVertexOffset(pHierarchy);
					SetIndexOffset(pHierarchy);

					SetMaterial(pHierarchy);

					WidgetsList.insert({ Child, pHierarchy });
					SetHierarchy(pHierarchy, Child);
					pWidgetHierarchy->Nodes.push_back(pHierarchy);
				}
			}
		};

		WidgetHierarchy* mWidgetHierarchy = new WidgetHierarchy;
		mWidgetHierarchy->Widget = Widget.get();
		SetVertexOffset(mWidgetHierarchy);
		SetIndexOffset(mWidgetHierarchy);

		SetMaterial(mWidgetHierarchy);

		WidgetsList.insert({ Widget.get(), mWidgetHierarchy });
		SetHierarchy(mWidgetHierarchy, Widget.get());

		Hierarchy.push_back(mWidgetHierarchy);

		SortWidgetsByZOrder();
	}

	bool sCanvasBase::IsWidgetExist(cbWidget* Widget) const
	{
		for (const auto& pWidget : Widgets)
		{
			if (pWidget.get() == Widget)
				return true;
		}
		return false;
	}

	void sCanvasBase::WidgetUpdated(cbWidgetObj* Object)
	{
		const auto& It = WidgetsList.find(Object);
		if (It != WidgetsList.end())
			It->second->bVertexDirty = true;
	}

	void sCanvasBase::RemoveFromCanvas(cbWidget* Object)
	{
		cbWidget::SharedPtr Widget = nullptr;
		std::vector<cbWidget::SharedPtr>::iterator it = Widgets.begin();
		while (it != Widgets.end())
		{
			if ((*it).get() == Object)
			{
				Widget = (*it);
				it = Widgets.erase(it);
				break;
			}
			else {
				it++;
			}
		}

		if (!Widget)
			return;

		{
			WidgetHierarchy* pHierarchy = nullptr;
			std::vector<WidgetHierarchy*>::iterator it = Hierarchy.begin();
			while (it != Hierarchy.end())
			{
				if ((*it)->Widget == Widget.get())
				{
					pHierarchy = (*it);
					it = Hierarchy.erase(it);
					break;
				}
				else {
					it++;
				}
			}

			if (pHierarchy)
			{
				delete pHierarchy;
				pHierarchy = nullptr;
			}
		}

		{
			std::function<void(const cbWidgetObj*)> Remove;
			Remove = [&](const cbWidgetObj* pWidget)
			{
				if (pWidget->HasAnyChildren())
				{
					const auto& Children = pWidget->GetAllChildren();
					for (const auto& Child : Children)
					{
						if (!Child)
							continue;

						WidgetsList.erase(Child);
						Remove(Child);
					}
				}
			};
			Remove(Widget.get());
			WidgetsList.erase(Widget.get());
		}

		Widget = nullptr;

		ReorderWidgets();
	}

	void sCanvasBase::NewSlotAdded(cbSlottedBox* Parent, cbSlot* Slot)
	{
		std::function<void(WidgetHierarchy*, const cbWidgetObj*)> SetHierarchy;
		SetHierarchy = [&](WidgetHierarchy* pWidgetHierarchy, const cbWidgetObj* pWidget)
		{
			if (pWidget->HasAnyChildren())
			{
				const auto& Children = pWidget->GetAllChildren();
				for (const auto& Child : Children)
				{
					if (!Child)
						continue;

					WidgetHierarchy* pHierarchy = new WidgetHierarchy;
					pHierarchy->Widget = Child;
					SetVertexOffset(pHierarchy);
					SetIndexOffset(pHierarchy);

					SetMaterial(pHierarchy);

					SetHierarchy(pHierarchy, Child);
					WidgetsList.insert({ Child, pHierarchy });
					pWidgetHierarchy->Nodes.push_back(pHierarchy);
				}
			}
		};

		std::function<WidgetHierarchy* (WidgetHierarchy*, const cbWidgetObj*)> GetParentHierarchy;
		GetParentHierarchy = [&](WidgetHierarchy* pWidgetHierarchy, const cbWidgetObj* pParent) -> WidgetHierarchy*
		{
			for (auto& WidgetHierarchy : pWidgetHierarchy->Nodes)
			{
				if (WidgetHierarchy->Widget == pParent)
				{
					return WidgetHierarchy;
				}
				else if (auto WP = GetParentHierarchy(WidgetHierarchy, pParent))
				{
					return WP;
				}
			}
			return nullptr;
		};

		WidgetHierarchy* pWH = nullptr;
		for (auto& pWidgetHierarchy : Hierarchy)
		{
			if (pWidgetHierarchy->Widget == Parent)
			{
				pWH = pWidgetHierarchy;
				break;
			}
			else if (auto WP = GetParentHierarchy(pWidgetHierarchy, Parent))
			{
				pWH = WP;
				break;
			}
		}

		if (pWH)
		{
			WidgetHierarchy* mWidgetHierarchy = new WidgetHierarchy;
			mWidgetHierarchy->Widget = Slot;
			SetVertexOffset(mWidgetHierarchy);
			SetIndexOffset(mWidgetHierarchy);

			SetMaterial(mWidgetHierarchy);

			SetHierarchy(mWidgetHierarchy, Slot);

			pWH->Nodes.push_back(mWidgetHierarchy);
			WidgetsList.insert({ Slot, mWidgetHierarchy });
		}
	}

	void sCanvasBase::NewSlotContentAdded(cbSlot* Parent, cbWidget* Content)
	{
		std::function<void(WidgetHierarchy*, const cbWidgetObj*)> SetHierarchy;
		SetHierarchy = [&](WidgetHierarchy* pWidgetHierarchy, const cbWidgetObj* pWidget)
		{
			if (pWidget->HasAnyChildren())
			{
				const auto& Children = pWidget->GetAllChildren();
				for (const auto& Child : Children)
				{
					if (!Child)
						continue;

					WidgetHierarchy* pHierarchy = new WidgetHierarchy;
					pHierarchy->Widget = Child;
					SetVertexOffset(pHierarchy);
					SetIndexOffset(pHierarchy);

					SetMaterial(pHierarchy);

					SetHierarchy(pHierarchy, Child);
					WidgetsList.insert({ Child, pHierarchy });
					pWidgetHierarchy->Nodes.push_back(pHierarchy);
				}
			}
		};

		std::function<WidgetHierarchy* (WidgetHierarchy*, const cbWidgetObj*)> GetParentHierarchy;
		GetParentHierarchy = [&](WidgetHierarchy* pWidgetHierarchy, const cbWidgetObj* pParent) -> WidgetHierarchy*
		{
			for (auto& WidgetHierarchy : pWidgetHierarchy->Nodes)
			{
				if (WidgetHierarchy->Widget == pParent)
				{
					return WidgetHierarchy;
				}
				else if (auto WP = GetParentHierarchy(WidgetHierarchy, pParent))
				{
					return WP;
				}
			}
			return nullptr;
		};

		WidgetHierarchy* pWH = nullptr;
		for (auto& pWidgetHierarchy : Hierarchy)
		{
			if (pWidgetHierarchy->Widget == Parent)
			{
				pWH = pWidgetHierarchy;
				break;
			}
			else if (auto WP = GetParentHierarchy(pWidgetHierarchy, Parent))
			{
				pWH = WP;
				break;
			}
		}

		if (pWH)
		{
			WidgetHierarchy* mWidgetHierarchy = new WidgetHierarchy;
			mWidgetHierarchy->Widget = Content;
			SetVertexOffset(mWidgetHierarchy);
			SetIndexOffset(mWidgetHierarchy);

			SetMaterial(mWidgetHierarchy);

			SetHierarchy(mWidgetHierarchy, Content);

			pWH->Nodes.push_back(mWidgetHierarchy);
			WidgetsList.insert({ Content, mWidgetHierarchy });
		}
	}

	void sCanvasBase::SlotRemoved(cbSlottedBox* Parent, cbSlot* Slot)
	{
		{
			std::function<WidgetHierarchy* (WidgetHierarchy*, const cbWidgetObj*)> GetParentHierarchy;
			GetParentHierarchy = [&](WidgetHierarchy* pWidgetHierarchy, const cbWidgetObj* pParent) -> WidgetHierarchy*
			{
				for (auto& WidgetHierarchy : pWidgetHierarchy->Nodes)
				{
					if (WidgetHierarchy->Widget == pParent)
					{
						return WidgetHierarchy;
					}
					else if (auto WP = GetParentHierarchy(WidgetHierarchy, pParent))
					{
						return WP;
					}
				}
				return nullptr;
			};

			WidgetHierarchy* pWH = nullptr;
			for (auto& pWidgetHierarchy : Hierarchy)
			{
				if (pWidgetHierarchy->Widget == Parent)
				{
					pWH = pWidgetHierarchy;
					break;
				}
				else if (auto WP = GetParentHierarchy(pWidgetHierarchy, Parent))
				{
					pWH = WP;
					break;
				}
			}

			if (pWH)
			{
				WidgetHierarchy* pHierarchy = nullptr;
				std::vector<WidgetHierarchy*>::iterator it = pWH->Nodes.begin();
				while (it != pWH->Nodes.end())
				{
					if ((*it)->Widget == Slot)
					{
						pHierarchy = (*it);
						it = pWH->Nodes.erase(it);
						break;
					}
					else {
						it++;
					}
				}

				{
					std::function<void(const cbWidgetObj*)> Remove;
					Remove = [&](const cbWidgetObj* pWidget)
					{
						if (pWidget->HasAnyChildren())
						{
							const auto& Children = pWidget->GetAllChildren();
							for (const auto& Child : Children)
							{
								if (!Child)
									continue;

								WidgetsList.erase(Child);
								Remove(Child);
							}
						}
					};
					Remove(pHierarchy->Widget);
					WidgetsList.erase(pHierarchy->Widget);
				}
				if (pHierarchy)
				{
					delete pHierarchy;
					pHierarchy = nullptr;
				}
			}
		}

		ReorderWidgets();
	}

	void sCanvasBase::SlotContentReplaced(cbSlot* Parent, cbWidget* Old, cbWidget* New)
	{
		{
			std::function<WidgetHierarchy* (WidgetHierarchy*, const cbWidgetObj*)> GetParentHierarchy;
			GetParentHierarchy = [&](WidgetHierarchy* pWidgetHierarchy, const cbWidgetObj* pParent) -> WidgetHierarchy*
			{
				for (auto& WidgetHierarchy : pWidgetHierarchy->Nodes)
				{
					if (WidgetHierarchy->Widget == pParent)
					{
						return WidgetHierarchy;
					}
					else if (auto WP = GetParentHierarchy(WidgetHierarchy, pParent))
					{
						return WP;
					}
				}
				return nullptr;
			};

			WidgetHierarchy* pWH = nullptr;
			for (auto& pWidgetHierarchy : Hierarchy)
			{
				if (pWidgetHierarchy->Widget == Parent)
				{
					pWH = pWidgetHierarchy;
					break;
				}
				else if (auto WP = GetParentHierarchy(pWidgetHierarchy, Parent))
				{
					pWH = WP;
					break;
				}
			}

			if (pWH)
			{
				std::function<void(WidgetHierarchy*, const cbWidgetObj*)> SetHierarchy;
				SetHierarchy = [&](WidgetHierarchy* pWidgetHierarchy, const cbWidgetObj* pWidget)
				{
					if (pWidget->HasAnyChildren())
					{
						const auto& Children = pWidget->GetAllChildren();
						for (const auto& Child : Children)
						{
							if (!Child)
								continue;

							WidgetHierarchy* pHierarchy = new WidgetHierarchy;
							pHierarchy->Widget = Child;
							SetVertexOffset(pHierarchy);
							SetIndexOffset(pHierarchy);

							SetMaterial(pHierarchy);

							SetHierarchy(pHierarchy, Child);
							pWidgetHierarchy->Nodes.push_back(pHierarchy);
							WidgetsList.insert({ Child, pHierarchy });
						}
					}
				};

				WidgetHierarchy* pHierarchy = nullptr;
				std::vector<WidgetHierarchy*>::iterator it = pWH->Nodes.begin();
				while (it != pWH->Nodes.end())
				{
					if ((*it)->Widget == Old)
					{
						pHierarchy = (*it);
						it = pWH->Nodes.erase(it);
						break;
					}
					else {
						it++;
					}
				}

				if (pHierarchy->Widget)
					WidgetsList.erase(pHierarchy->Widget);

				if (pHierarchy)
				{
					delete pHierarchy;
					pHierarchy = nullptr;
				}

				WidgetHierarchy* mWidgetHierarchy = new WidgetHierarchy;
				mWidgetHierarchy->Widget = New;
				SetVertexOffset(mWidgetHierarchy);
				SetIndexOffset(mWidgetHierarchy);

				SetMaterial(mWidgetHierarchy);

				SetHierarchy(mWidgetHierarchy, New);

				pWH->Nodes.push_back(mWidgetHierarchy);
				WidgetsList.insert({ New, mWidgetHierarchy });
			}
		}

		ReorderWidgets();
	}

	void sCanvasBase::VerticesSizeChanged(cbWidgetObj* Object, const std::size_t NewSize)
	{
		ReorderWidgets();
	}

	std::vector<cbWidget*> sCanvasBase::GetOverlappingWidgets(const cbBounds& Bounds) const
	{
		std::vector<cbWidget*> UIObjects;
		for (const auto& Obj : Widgets)
		{
			if (Obj->Intersect(Bounds))
			{
				UIObjects.push_back(Obj.get());
			}
		}

		return UIObjects;
	}

	void sCanvasBase::ReorderWidgets()
	{
		std::size_t VertexOrder = 0;
		std::size_t IndexOrder = 0;

		std::function<void(WidgetHierarchy*, std::size_t&, std::size_t&)> SetHierarchy;
		SetHierarchy = [&](WidgetHierarchy* pWidgetHierarchy, std::size_t& VertexOrder, std::size_t& IndexOrder)
		{
			for (auto& Child : pWidgetHierarchy->Nodes)
			{
				Child->DrawParams.VertexOffset = VertexOrder;
				Child->DrawParams.IndexOffset = IndexOrder;

				Child->bVertexDirty = true;
				Child->bIndexDirty = true;

				if (Child->Widget->HasGeometry())
				{
					auto DrawData = Child->Widget->GetGeometryDrawData();
					VertexOrder += DrawData.VertexCount;
				}
				else
					VertexOrder += 4;
				if (Child->Widget->HasGeometry())
				{
					auto DrawData = Child->Widget->GetGeometryDrawData();
					IndexOrder += DrawData.IndexCount;
				}
				else
					IndexOrder += 8;

				SetHierarchy(Child, VertexOrder, IndexOrder);
			}
		};

		for (auto& pHierarchy : Hierarchy)
		{
			pHierarchy->DrawParams.VertexOffset = VertexOrder;
			pHierarchy->DrawParams.IndexOffset = IndexOrder;
			pHierarchy->bVertexDirty = true;
			pHierarchy->bIndexDirty = true;
			if (pHierarchy->Widget->HasGeometry())
			{
				auto DrawData = pHierarchy->Widget->GetGeometryDrawData();
				VertexOrder += DrawData.VertexCount;
			}
			else
				VertexOrder += 4;
			if (pHierarchy->Widget->HasGeometry())
			{
				auto DrawData = pHierarchy->Widget->GetGeometryDrawData();
				IndexOrder += DrawData.IndexCount;
			}
			else
				IndexOrder += 8;
			SetHierarchy(pHierarchy, VertexOrder, IndexOrder);
		}

		PrevVertexOffset = VertexOrder;
		PrevIndexOffset = IndexOrder;
	}

	void sCanvasBase::SetVertexOffset(WidgetHierarchy* pWH)
	{
		std::size_t PrevVertexSize = 0;
		std::size_t CurrentVertexOffset = PrevVertexOffset;

		if (pWH->Widget->HasGeometry())
		{
			PrevVertexSize = pWH->Widget->GetGeometryDrawData().VertexCount;
			pWH->DrawParams.VertexSize = PrevVertexSize;
			pWH->DrawParams.VertexOffset = CurrentVertexOffset;
		}
		else
		{
			PrevVertexSize = 4;
			pWH->DrawParams.VertexSize = PrevVertexSize;
			pWH->DrawParams.VertexOffset = CurrentVertexOffset;
		}
		PrevVertexOffset = CurrentVertexOffset + PrevVertexSize;
	}

	void sCanvasBase::SetIndexOffset(WidgetHierarchy* pWH)
	{
		std::size_t PrevIndexSize = 0;
		std::size_t CurrentIndexOffset = PrevIndexOffset;

		if (pWH->Widget->HasGeometry())
		{
			PrevIndexSize = pWH->Widget->GetGeometryDrawData().IndexCount;
			pWH->DrawParams.IndexSize = PrevIndexSize;
			pWH->DrawParams.IndexOffset = CurrentIndexOffset;
		}
		else
		{
			PrevIndexSize = 8;
			pWH->DrawParams.IndexSize = PrevIndexSize;
			pWH->DrawParams.IndexOffset = CurrentIndexOffset;
		}
		PrevIndexOffset = CurrentIndexOffset + PrevIndexSize;
	}
}
