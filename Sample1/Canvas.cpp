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
		RendererProfile_VB = cbVerticalBox::Create();
		RendererProfile_VB->SetVerticalAlignment(eVerticalAlignment::Align_Fill);
		RendererProfile_VB->SetHorizontalAlignment(eHorizontalAlignment::Align_Fill);
		RendererProfile_VB->AddToCanvas(this);
		RendererProfile_VB->SetAlignToCanvas(true);
		RendererProfile_VB->SetPadding(cbgui::cbMargin(10, 10, 10, 10));
	}

	sCanvas::~sCanvas()
	{
		for (auto& Elements : ProfileElements)
			Elements.second.Release();
		ProfileElements.clear();

		RendererProfile_VB = nullptr;

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

	std::string RenderPassToString(ERenderPass RenderPass)
	{
		switch (RenderPass)
		{
		case ERenderPass::NONE:			return "NONE";
		case ERenderPass::GBuffer:		return "GBuffer";
		case ERenderPass::Line:			return "Line";
		case ERenderPass::Particle:		return "Particle";
		case ERenderPass::FSR:			return "FSR";
		case ERenderPass::Tonemapper:	return "Tonemapper";
		case ERenderPass::PostProcess:	return "PostProcess";
		case ERenderPass::UI:			return "UI";
		case ERenderPass::Custom1:		return "Custom1";
		case ERenderPass::Custom2:		return "Custom2";
		case ERenderPass::Custom3:		return "Custom3";
		case ERenderPass::Custom4:		return "Custom4";
		case ERenderPass::Custom5:		return "Custom5";
		case ERenderPass::Custom6:		return "Custom6";
		case ERenderPass::Custom7:		return "Custom7";
		case ERenderPass::Custom8:		return "Custom8";
		case ERenderPass::Custom9:		return "Custom9";
		case ERenderPass::Custom10:		return "Custom10";
		}

		return "";
	}

	void sCanvas::AddRendererProfileResult(const ProfileResult& Result)
	{
		if (ProfileElements.contains(Result.RenderPass))
		{
			if (Result.IsValid())
			{
				if (ProfileElements[Result.RenderPass].Main->GetVisibilityState() != cbgui::eVisibility::Visible)
					ProfileElements[Result.RenderPass].Main->SetVisibilityState(cbgui::eVisibility::Visible);
				ProfileElements[Result.RenderPass].GPUValueString->SetString(std::to_string(Result.GPU));

				if (Result.CPU == 0.0)
				{
					ProfileElements[Result.RenderPass].CPU_ProfileElement->SetVisibilityState(cbgui::eVisibility::Collapsed);
				}
				else
				{
					if (ProfileElements[Result.RenderPass].CPU_ProfileElement->GetVisibilityState() != cbgui::eVisibility::Visible)
						ProfileElements[Result.RenderPass].CPU_ProfileElement->SetVisibilityState(cbgui::eVisibility::Visible);
					ProfileElements[Result.RenderPass].CPUValueString->SetString(std::to_string(Result.CPU));
				}
			}
			else
			{
				ProfileElements[Result.RenderPass].Main->SetVisibilityState(cbgui::eVisibility::Collapsed);
			}
		}
		else
		{
			cbHorizontalBox::SharedPtr GPU_ProfileElement = nullptr;
			cbString::SharedPtr GPUValueString = nullptr;
			cbHorizontalBox::SharedPtr CPU_ProfileElement = nullptr;
			cbString::SharedPtr CPUValueString = nullptr;
			cbHorizontalBox::SharedPtr ProfileElement = cbHorizontalBox::Create();
			ProfileElement->SetVerticalAlignment(eVerticalAlignment::Align_Top);
			ProfileElement->SetHorizontalAlignment(eHorizontalAlignment::Align_Left);
			{
				cbString::SharedPtr String = cbString::Create(RenderPassToString(Result.RenderPass) + " : ");
				String->SetVerticalAlignment(eVerticalAlignment::Align_Top);
				String->SetHorizontalAlignment(eHorizontalAlignment::Align_Left);
				String->SetFontSize(18);
				String->SetVertexColorStyle(cbgui::cbVertexColorStyle(cbColor::Red()));
				ProfileElement->Insert(String);
			}
			{
				GPU_ProfileElement = cbHorizontalBox::Create();
				GPU_ProfileElement->SetVerticalAlignment(eVerticalAlignment::Align_Top);
				GPU_ProfileElement->SetHorizontalAlignment(eHorizontalAlignment::Align_Left);
				ProfileElement->Insert(GPU_ProfileElement);
				{
					cbString::SharedPtr String = cbString::Create("GPU : ");
					String->SetVerticalAlignment(eVerticalAlignment::Align_Top);
					String->SetHorizontalAlignment(eHorizontalAlignment::Align_Left);
					String->SetFontSize(18);
					String->SetVertexColorStyle(cbgui::cbVertexColorStyle(cbColor::Red()));
					GPU_ProfileElement->Insert(String);
				}
				GPUValueString = cbString::Create(std::to_string(Result.GPU)/*"GPU_Value"*/);
				GPUValueString->SetVerticalAlignment(eVerticalAlignment::Align_Top);
				GPUValueString->SetHorizontalAlignment(eHorizontalAlignment::Align_Left);
				GPUValueString->SetFontSize(18);
				GPUValueString->SetVertexColorStyle(cbgui::cbVertexColorStyle(cbColor::Red()));
				GPU_ProfileElement->Insert(GPUValueString);
			}
			{
				CPU_ProfileElement = cbHorizontalBox::Create();
				CPU_ProfileElement->SetVerticalAlignment(eVerticalAlignment::Align_Top);
				CPU_ProfileElement->SetHorizontalAlignment(eHorizontalAlignment::Align_Left);
				CPU_ProfileElement->SetPadding(cbgui::cbMargin(10, 0, 0, 0));
				ProfileElement->Insert(CPU_ProfileElement);
				{
					cbString::SharedPtr String = cbString::Create(" | CPU : ");
					String->SetVerticalAlignment(eVerticalAlignment::Align_Top);
					String->SetHorizontalAlignment(eHorizontalAlignment::Align_Left);
					String->SetFontSize(18);
					String->SetVertexColorStyle(cbgui::cbVertexColorStyle(cbColor::Red()));
					CPU_ProfileElement->Insert(String);
				}
				CPUValueString = cbString::Create(std::to_string(Result.CPU)/*"CPU_Value"*/);
				CPUValueString->SetVerticalAlignment(eVerticalAlignment::Align_Top);
				CPUValueString->SetHorizontalAlignment(eHorizontalAlignment::Align_Left);
				CPUValueString->SetFontSize(18);
				CPUValueString->SetVertexColorStyle(cbgui::cbVertexColorStyle(cbColor::Red()));
				CPU_ProfileElement->Insert(CPUValueString);
			}

			RendererProfile_VB->Insert(ProfileElement);
			ProfileElements.insert({ Result.RenderPass, ProfileContainer(ProfileElement, GPU_ProfileElement, GPUValueString, CPU_ProfileElement, CPUValueString) });
		}
	}

	void sCanvas::RemoveRendererProfileResult(const ERenderPass RenderPass)
	{

	}
}
