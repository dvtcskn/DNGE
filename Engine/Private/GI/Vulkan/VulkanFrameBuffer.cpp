/* ---------------------------------------------------------------------------------------
* MIT License
*
* Copyright (c) 2022 Davut Coþkun.
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
#include "VulkanFrameBuffer.h"

/*
Depth Formats
DXGI_FORMAT_R16_TYPELESS
DXGI_FORMAT_R16_UINT
DXGI_FORMAT_D16_UNORM

DXGI_FORMAT_R32_TYPELESS
DXGI_FORMAT_R32_UINT
DXGI_FORMAT_D32_FLOAT

// NO SRV Only
DXGI_FORMAT_R24G8_TYPELESS
DXGI_FORMAT_D24_UNORM_S8_UINT

DXGI_FORMAT_R32G8X24_TYPELESS
DXGI_FORMAT_D32_FLOAT_S8X24_UINT
*/

VulkanFrameBuffer::VulkanFrameBuffer(const std::string InName, const sFrameBufferAttachmentInfo& InAttachments)
{
}

