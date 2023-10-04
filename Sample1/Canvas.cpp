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
