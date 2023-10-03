#pragma once

#include "CanvasBase.h"

namespace cbgui
{
	class sCanvas : public cbgui::sCanvasBase
	{
		cbClassBody(cbClassConstructor, sCanvas, cbgui::sCanvasBase)
	public:
		sCanvas(MetaWorld* pMetaWorld);
		virtual ~sCanvas();
		virtual void SetMaterial(WidgetHierarchy* pWP) override final;
		virtual void Tick(const double DeltaTime) override;
		virtual void FixedUpdate(const double DeltaTime) override;

		virtual void InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar) override final;

		virtual void ResizeWindow(std::size_t Width, std::size_t Height) override final;

	private:
		virtual void Add(const std::shared_ptr<cbWidget>& Object) override;
		virtual void RemoveFromCanvas(cbWidget* Object) override;

	private:
		cbWidget* Focus;
	};
}
