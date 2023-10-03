
#include "pch.h"
#include "Canvas.h"
#include "cbString.h"

namespace cbgui
{
	sCanvas::sCanvas(MetaWorld* pMetaWorld)
		: Super(pMetaWorld)
	{
		/*cbString::SharedPtr String = cbString::Create("TEST");
		String->SetVerticalAlignment(eVerticalAlignment::Align_Top);
		String->SetHorizontalAlignment(eHorizontalAlignment::Align_Left);
		String->AddToCanvas(this);
		String->SetAlignToCanvas(true);*/
	}

	sCanvas::~sCanvas()
	{
		Focus = nullptr;
	}

	void sCanvas::ResizeWindow(std::size_t Width, std::size_t Height)
	{
		Super::ResizeWindow(Width, Height);
	}

	void sCanvas::Add(const std::shared_ptr<cbWidget>& Object)
	{
		Super::Add(Object);
	}

	void sCanvas::RemoveFromCanvas(cbWidget* Object)
	{
		Super::RemoveFromCanvas(Object);
	}

	void sCanvas::SetMaterial(WidgetHierarchy* pWP)
	{
		Super::SetMaterial(pWP);
	}

	void sCanvas::Tick(const double DeltaTime)
	{
		Super::Tick(DeltaTime);
	}

	void sCanvas::FixedUpdate(const double DeltaTime)
	{
		Super::FixedUpdate(DeltaTime);
	}

	void sCanvas::InputProcess(const GMouseInput& InMouseInput, const GKeyboardChar& KeyboardChar)
	{
		Super::InputProcess(InMouseInput, KeyboardChar);
	}
}
