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

#include <string>
#include <cbCanvas.h>
#include "Engine/IMetaWorld.h"
#include "AbstractGI/UIMaterialStyle.h"

class ICanvas : public cbgui::cbICanvas
{
	cbBaseClassBody(cbClassNoDefaults, ICanvas)
public:
	struct GeometryDrawParams
	{
		std::size_t VertexOffset;
		std::size_t IndexOffset;
		std::size_t VertexSize;
		std::size_t IndexSize;

		GeometryDrawParams()
			: VertexOffset(0)
			, IndexOffset(0)
			, VertexSize(0)
			, IndexSize(0)
		{}

		~GeometryDrawParams() = default;
	};

	struct WidgetHierarchy
	{
		std::vector<WidgetHierarchy*> Nodes;

		cbgui::cbWidgetObj* Widget;
		bool bVertexDirty = true;
		bool bIndexDirty = true;
		IUIMaterialStyle* MaterialStyle;
		GeometryDrawParams DrawParams;

		WidgetHierarchy()
			: Widget(nullptr)
			, bVertexDirty(true)
			, bIndexDirty(true)
			, MaterialStyle(nullptr)
		{}

		~WidgetHierarchy()
		{
			Widget = nullptr;
			MaterialStyle = nullptr;
			for (auto& Node : Nodes)
			{
				delete Node;
				Node = nullptr;
			}
			Nodes.clear();
		}
	};

protected:
	IMetaWorld* pMetaWorld;

protected:
	ICanvas(IMetaWorld* InMetaWorld)
		: pMetaWorld(InMetaWorld)
	{}
public:
	virtual ~ICanvas()
	{
		pMetaWorld = nullptr;
	}

	virtual void BeginPlay() = 0;
	virtual void Tick(const double DeltaTime) = 0;

	IMetaWorld* GetMetaWorld() const { return pMetaWorld; };
	template<typename T>
	T* GetMetaWorld() const { return static_cast<T*>(pMetaWorld); };

	virtual void InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar) = 0;

	virtual std::vector<WidgetHierarchy*> GetWidgetHierarchy() const = 0;

	virtual void ResizeWindow(std::size_t Width, std::size_t Height) = 0;

	virtual void RealignWidgets() = 0;

	virtual IVertexBuffer* GetVertexBuffer() const = 0;
	virtual IIndexBuffer* GetIndexBuffer() const = 0;
};
