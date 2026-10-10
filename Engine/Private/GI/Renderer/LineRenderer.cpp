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
#include "LineRenderer.h"
#include "Utilities/FileManager.h"

struct LineSceneDescriptor : public IBindlessSceneDescriptor
{
	std::uint32_t CameraConstantBuffer = std::uint32_t(-1);
};

struct sLineSceneDescriptor : public IBindlessSceneContainer
{
	LineSceneDescriptor Descriptor;

	sLineSceneDescriptor(LineSceneDescriptor NewDescriptor)
		: Descriptor(NewDescriptor)
	{}

	virtual const IBindlessSceneDescriptor* GetDescriptor() const override final
	{
		return &Descriptor;
	}

	virtual std::vector<std::uint32_t> GetAllBindlessIndices() const override final
	{
		std::vector<std::uint32_t> BindlessIndices;
		BindlessIndices.push_back(Descriptor.CameraConstantBuffer);
		return BindlessIndices;
	}

	virtual std::vector<std::uint32_t> GetAllConstantBufferBindlessIndices() const override final
	{
		std::vector<std::uint32_t> BindlessIndices;
		BindlessIndices.push_back(Descriptor.CameraConstantBuffer);
		return BindlessIndices;
	}

	virtual std::uint32_t GetConstantBuffereSize() const override final
	{
		return 1;
	}
};

sLineRenderer::sLineRenderer(std::size_t Width, std::size_t Height)
	: Super()
	, GraphicsCommandContext(IGraphicsCommandContext::Create())
	, ScreenDimension(sScreenDimension(Width, Height))
{
	{
		BufferLayout Desc;
		Desc.Size = 4000000;
		Desc.Stride = sizeof(sLineVertexBufferEntry);
		VertexBuffer = IVertexBuffer::Create("LineRendererVB", Desc, nullptr);
	}

	sPipelineDesc pPipelineDesc(ERenderPass::Line);
	pPipelineDesc.BlendAttribute = sBlendAttributeDesc();
	pPipelineDesc.DepthStencilAttribute = sDepthStencilAttributeDesc(ECompareFunction::LessEqual, false);
	//pPipelineDesc.DepthStencilAttribute.DepthTest = ECompareFunction::Always;
	pPipelineDesc.PrimitiveTopologyType = EPrimitiveType::LINE_LIST;
	pPipelineDesc.RasterizerAttribute = sRasterizerAttributeDesc();
	pPipelineDesc.RasterizerAttribute.bEnableLineAA = true;
	pPipelineDesc.RasterizerAttribute.CullMode = ERasterizerCullMode::None;

	pPipelineDesc.VertexLayout = sVertexAttributeDesc::GetDefaultLineVertexLayout();

	pPipelineDesc.Bindings.push_back(sShaderBinding(EDescriptorType::e32BitConstant, eShaderType::Vertex, 0, 1));

	std::vector<sShaderAttachment> ShaderAttachments;
	pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(FileManager::GetShaderFolderW() + L"GBufferVS.hlsl", "GeometryVSLine", eShaderType::Vertex));
	pPipelineDesc.ShaderAttachments.push_back(sShaderAttachment(FileManager::GetShaderFolderW() + L"GBufferPS.hlsl", "LineGeometryFlatPS", eShaderType::Pixel));

	DefaultEngineMat = sMaterial::Create("Line", EMaterialBlendMode::Opaque, pPipelineDesc);

	{
		DefaultMatInstance = DefaultEngineMat->CreateInstance("LineInstance");
	}
}

sLineRenderer::~sLineRenderer()
{
	DefaultEngineMat = nullptr;
	DefaultMatInstance = nullptr;
	GraphicsCommandContext = nullptr;

	VertexBuffer = nullptr;
}

void sLineRenderer::BeginPlay()
{
}

void sLineRenderer::Tick(const double DeltaTime)
{
	bool bReorder = false;
	std::vector<sLines::LinesVertexData>::iterator it = Lines.VertexData.begin();
	while (it != Lines.VertexData.end())
	{
		if ((*it).Time.has_value())
		{
			(*it).Time = (*it).Time.value() - static_cast<float>(DeltaTime);
			if ((*it).Time.value() <= 0.0f)
			{
				auto LineType = (*it).LineType;
				it = Lines.VertexData.erase(it);

				bReorder = true;

				if (LineType == sLines::ELineType::Line)
				{
					if (Lines.LineCount > 0)
						Lines.LineCount--;
				}
				else if (LineType == sLines::ELineType::eBound)
				{
					if (Lines.BoundCount > 0)
						Lines.BoundCount--;
				}
			}
			else
			{
				it++;
			}
		}
		else
		{
			it++;
		}
	}

	if (bReorder)
	{
		auto Data = Lines.GetVertices();
		BufferSubresource Subresource;
		Subresource.pSysMem = Data.data();
		Subresource.Size = ((Lines.BoundCount * 8) + (Lines.LineCount * 2)) * sizeof(sLineVertexBufferEntry);
		Subresource.Location = 0;

		VertexBuffer->UpdateSubresource(&Subresource);
	}
}

void sLineRenderer::DrawLine(const FVector& Start, const FVector& End, const FColor& Color, std::optional<float> Time)
{
	sLines::LinesVertexData Data;
	std::vector<sLineVertexBufferEntry> VertexAttributes;
	sLineVertexBufferEntry sVertexAttribute;
	sVertexAttribute.position = Start;
	sVertexAttribute.Color = Color;
	VertexAttributes.push_back(sVertexAttribute);
	sVertexAttribute.position = End;
	sVertexAttribute.Color = Color;
	VertexAttributes.push_back(sVertexAttribute);

	Data.Vertices = VertexAttributes;
	Data.Time = Time;
	Data.Location = ((Lines.BoundCount * 8) + (Lines.LineCount * 2)) * sizeof(sLineVertexBufferEntry);
	Data.LineType = sLines::ELineType::Line;

	BufferSubresource Subresource;
	Subresource.pSysMem = VertexAttributes.data();
	Subresource.Size = VertexAttributes.size() * sizeof(sLineVertexBufferEntry);
	Subresource.Location = Data.Location;

	VertexBuffer->UpdateSubresource(&Subresource);

	Lines.LineCount++;

	Lines.VertexData.push_back(Data);
}

void sLineRenderer::DrawBound(const FBoundingBox& Box, const FColor& Color, std::optional<float> Time)
{
	sLines::LinesVertexData Data;
	std::vector<sLineVertexBufferEntry> VertexAttributes;
	sLineVertexBufferEntry sVertexAttribute;
	sVertexAttribute.position = (FVector(Box.Min.X, Box.Min.Y, 0.0f));
	sVertexAttribute.Color = Color;
	VertexAttributes.push_back(sVertexAttribute);
	sVertexAttribute.position = (FVector(Box.Max.X, Box.Min.Y, 0.0f));
	sVertexAttribute.Color = Color;
	VertexAttributes.push_back(sVertexAttribute);
	sVertexAttribute.position = (FVector(Box.Max.X, Box.Max.Y, 0.0f));
	sVertexAttribute.Color = Color;
	VertexAttributes.push_back(sVertexAttribute);
	sVertexAttribute.position = (FVector(Box.Min.X, Box.Max.Y, 0.0f));
	sVertexAttribute.Color = Color;
	VertexAttributes.push_back(sVertexAttribute);

	sVertexAttribute.position = (FVector(Box.Min.X, Box.Min.Y, 0.0f));
	sVertexAttribute.Color = Color;
	VertexAttributes.push_back(sVertexAttribute);
	sVertexAttribute.position = (FVector(Box.Min.X, Box.Max.Y, 0.0f));
	sVertexAttribute.Color = Color;
	VertexAttributes.push_back(sVertexAttribute);
	sVertexAttribute.position = (FVector(Box.Max.X, Box.Max.Y, 0.0f));
	sVertexAttribute.Color = Color;
	VertexAttributes.push_back(sVertexAttribute);
	sVertexAttribute.position = (FVector(Box.Max.X, Box.Min.Y, 0.0f));
	sVertexAttribute.Color = Color;
	VertexAttributes.push_back(sVertexAttribute);

	Data.Vertices = VertexAttributes;
	Data.Time = Time;
	Data.Location = ((Lines.BoundCount * 8) + (Lines.LineCount * 2)) * sizeof(sLineVertexBufferEntry);
	Data.LineType = sLines::ELineType::eBound;

	BufferSubresource Subresource;
	Subresource.pSysMem = VertexAttributes.data();
	Subresource.Size = VertexAttributes.size() * sizeof(sLineVertexBufferEntry);
	Subresource.Location = Data.Location;

	VertexBuffer->UpdateSubresource(&Subresource);

	Lines.BoundCount++;

	Lines.VertexData.push_back(Data);
}

void sLineRenderer::Render(IRenderTarget* BackBuffer, std::uint32_t CameraBindlessIndex, std::optional<sViewport> Viewport)
{
	if (Lines.GetDrawCount() == 0)
		return;

	GraphicsCommandContext->BeginRecordCommandList(ECommandContextBeginState::Render);

	GraphicsCommandContext->SetRenderTarget(BackBuffer);

	if (Viewport.has_value())
		GraphicsCommandContext->SetViewport(*Viewport);
	else
		GraphicsCommandContext->SetViewport(sViewport(ScreenDimension));

	GraphicsCommandContext->SetScissorRect(0, 0, (std::uint32_t)ScreenDimension.Height, (std::uint32_t)ScreenDimension.Width);

	if (!DefaultEngineMat->IsCompiled())
		DefaultEngineMat->Compile(BackBuffer);

	DefaultEngineMat->ApplyMaterial(GraphicsCommandContext.get());
	DefaultMatInstance->ApplyMaterialInstance(GraphicsCommandContext.get());

	GraphicsCommandContext->SetVertexBuffer(VertexBuffer.get());
	//GraphicsCommandContext->SetIndexBuffer(IndexBuffer.get());

	//GraphicsCommandContext->SetConstantBuffer(CameraCB);
	LineSceneDescriptor SceneDescriptor;
	SceneDescriptor.CameraConstantBuffer = CameraBindlessIndex;
	//GraphicsCommandContext->Set32BitConstants(0, &SceneDescriptor, 1, 0);
	sLineSceneDescriptor Descriptor(SceneDescriptor);
	GraphicsCommandContext->SetBindlessDescriptor(0, &Descriptor);

	//GraphicsCommandContext->DrawIndexedInstanced(LineDrawParams);
	GraphicsCommandContext->Draw((std::uint32_t)Lines.GetDrawCount());

	GraphicsCommandContext->FinishRecordCommandList();
	GraphicsCommandContext->ExecuteCommandList(ECommandContextExecuteType::Deferred, 1);
}

void sLineRenderer::Render(IGraphicsCommandContext* CMD, bool Exec, IRenderTarget* BackBuffer, std::uint32_t CameraBindlessIndex, std::optional<sViewport> Viewport)
{
	if (Lines.GetDrawCount() == 0)
		return;

	CMD->BeginRecordCommandList(ECommandContextBeginState::Render);

	CMD->SetRenderTarget(BackBuffer);

	if (Viewport.has_value())
		CMD->SetViewport(*Viewport);
	else
		CMD->SetViewport(sViewport(ScreenDimension));

	CMD->SetScissorRect(0, 0, (std::uint32_t)ScreenDimension.Height, (std::uint32_t)ScreenDimension.Width);

	if (!DefaultEngineMat->IsCompiled())
		DefaultEngineMat->Compile(BackBuffer);

	DefaultEngineMat->ApplyMaterial(CMD);
	DefaultMatInstance->ApplyMaterialInstance(CMD);

	CMD->SetVertexBuffer(VertexBuffer.get());
	//GraphicsCommandContext->SetIndexBuffer(IndexBuffer.get());

	LineSceneDescriptor SceneDescriptor;
	SceneDescriptor.CameraConstantBuffer = CameraBindlessIndex;
	//GraphicsCommandContext->Set32BitConstants(0, &SceneDescriptor, 1, 0);
	sLineSceneDescriptor Descriptor(SceneDescriptor);
	CMD->SetBindlessDescriptor(0, &Descriptor);

	//GraphicsCommandContext->DrawIndexedInstanced(LineDrawParams);
	CMD->Draw((std::uint32_t)Lines.GetDrawCount());

	CMD->FinishRecordCommandList();
	if (Exec)
		CMD->ExecuteCommandList();
}

void sLineRenderer::SetRenderSize(std::size_t InWidth, std::size_t InHeight)
{
	ScreenDimension.Width = InWidth;
	ScreenDimension.Height = InHeight;
}

void sLineRenderer::OnInputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar)
{

}
