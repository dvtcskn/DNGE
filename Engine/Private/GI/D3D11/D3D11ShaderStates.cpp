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
#include "D3D11ShaderStates.h"
#include "GI/D3DShared/D3DShared.h"

D3D11VertexAttribute::D3D11VertexAttribute(D3D11Device* InDevice, std::vector<sVertexAttributeDesc> InDesc, void* InShaderCode)
	: VertexAttributeDescData(InDesc)
	, Owner(InDevice)
	, pVertexAttribute(NULL)
{
	ID3DBlob* bytecode = static_cast<ID3DBlob*>(InShaderCode);

	D3D11_INPUT_ELEMENT_DESC elementDesc[16];
	for (std::uint32_t i = 0; i < VertexAttributeDescData.size(); i++)
	{
		elementDesc[i].SemanticName = VertexAttributeDescData[i].name.c_str();
		elementDesc[i].SemanticIndex = 0;
		elementDesc[i].Format = ConvertFormat_Format_To_DXGI(VertexAttributeDescData[i].format);
		elementDesc[i].InputSlot = 0;// VertexAttributeDescData[i].bufferIndex;
		elementDesc[i].AlignedByteOffset = VertexAttributeDescData[i].offset;
		elementDesc[i].InputSlotClass = VertexAttributeDescData[i].isInstanced ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
		elementDesc[i].InstanceDataStepRate = VertexAttributeDescData[i].isInstanced ? 1 : 0;
	}

	Owner->GetDevice()->CreateInputLayout(elementDesc, static_cast<std::uint32_t>(VertexAttributeDescData.size()),
		bytecode->GetBufferPointer(), bytecode->GetBufferSize(), &pVertexAttribute);
}
