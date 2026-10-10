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

#include "CanvasBase.h"
#include "cbString.h"

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

		virtual void AddRendererProfileResult(const ProfileResult& Result) override final;
		virtual void RemoveRendererProfileResult(const ERenderPass RenderPass) override final;

	private:
		virtual void Add(const std::shared_ptr<cbWidget>& Object) override;
		virtual void RemoveFromCanvas(cbWidget* Object) override;

	private:
		cbWidget* Focus;

	private:
		cbVerticalBox::SharedPtr RendererProfile_VB;
		struct ProfileContainer
		{
			cbHorizontalBox::SharedPtr Main;
			cbHorizontalBox::SharedPtr GPU_ProfileElement;
			cbString::SharedPtr GPUValueString;
			cbHorizontalBox::SharedPtr CPU_ProfileElement;
			cbString::SharedPtr CPUValueString;
			ProfileContainer() = default;
			ProfileContainer(cbHorizontalBox::SharedPtr NewMain, cbHorizontalBox::SharedPtr NewGPU_ProfileElement, cbString::SharedPtr NewGPUResult, cbHorizontalBox::SharedPtr NewCPUValueString, cbString::SharedPtr NewCPUResult)
				: Main(NewMain)
				, GPU_ProfileElement(NewGPU_ProfileElement)
				, GPUValueString(NewGPUResult)
				, CPU_ProfileElement(NewCPUValueString)
				, CPUValueString(NewCPUResult)
			{}

			~ProfileContainer()
			{
				Release();
			}

			void Release()
			{
				Main = nullptr;
				GPU_ProfileElement = nullptr;
				GPUValueString = nullptr;
				CPU_ProfileElement = nullptr;
				CPUValueString = nullptr;
			}
		};
		std::map<ERenderPass, ProfileContainer> ProfileElements;
	};
}
