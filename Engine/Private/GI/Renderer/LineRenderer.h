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

#include "IRenderer.h"
#include "AbstractGI/Material.h"
#include "AbstractGI/UIMaterialStyle.h"
#include "Gameplay/ICanvas.h"
#include "Utilities/Input.h"
#include "Gameplay/CameraManager.h"

class sLineRenderer : public IRenderer
{
	sClassBody(sClassConstructor, sLineRenderer, IRenderer)
private:
	struct sLineVertexBufferEntry
	{
		FVector position;
		FColor Color;

		sLineVertexBufferEntry()
			: position(FVector::Zero())
			, Color(FColor::White())
		{}
	};

	struct sLines
	{
		enum class ELineType
		{
			eLine,
			eBound,
		};
		struct LinesVertexData
		{
			std::vector<sLineVertexBufferEntry> Vertices;
			std::optional<float> Time = std::nullopt;
			std::size_t Location = 0;
			ELineType LineType;
		};
		std::vector<LinesVertexData> VertexData;
		std::size_t LineCount = 0;
		std::size_t BoundCount = 0;

		constexpr std::size_t GetDrawCount() { return (BoundCount * 8) + (LineCount * 2); }
		constexpr std::vector<sLineVertexBufferEntry> GetVertices()
		{
			std::vector<sLineVertexBufferEntry> Vertices;
			for (const auto& Data : VertexData)
				Vertices.insert(Vertices.end(), Data.Vertices.begin(), Data.Vertices.end());

			return Vertices;
		}
	};

public:
	sLineRenderer(std::size_t Width, std::size_t Height);
	virtual ~sLineRenderer();

	virtual void BeginPlay() override final;
	virtual void Tick(const double DeltaTime) override final;

	void Render(IRenderTarget* BackBuffer, IConstantBuffer* CameraCB, ICamera* pCamera, std::optional<sViewport> Viewport);
	void Render(IGraphicsCommandContext* CMD, bool Exec, IRenderTarget* BackBuffer, IConstantBuffer* CameraCB, ICamera* pCamera, std::optional<sViewport> Viewport);

	virtual void SetRenderSize(std::size_t Width, std::size_t Height) override final;
	virtual void OnInputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar) override final;
	
	void DrawLine(const FVector& Start, const FVector& End, const FColor& Color, std::optional<float> Time);
	void DrawBound(const FBoundingBox& Box, const FColor& Color, std::optional<float> Time);

private:
	sScreenDimension ScreenDimension;

	IGraphicsCommandContext::SharedPtr GraphicsCommandContext;
	sMaterial::SharedPtr DefaultEngineMat;
	sMaterial::sMaterialInstance::SharedPtr DefaultMatInstance;

	IVertexBuffer::SharedPtr VertexBuffer;

	sLines Lines;
};
