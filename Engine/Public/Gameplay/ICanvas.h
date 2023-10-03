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

	virtual void InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar) = 0;

	virtual std::vector<WidgetHierarchy*> GetWidgetHierarchy() const = 0;

	virtual void ResizeWindow(std::size_t Width, std::size_t Height) = 0;

	virtual void RealignWidgets() = 0;

	virtual IVertexBuffer* GetVertexBuffer() const = 0;
	virtual IIndexBuffer* GetIndexBuffer() const = 0;
};
