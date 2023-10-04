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

#include "PostProcess.h"
#include "Core/Math/CoreMath.h"
#include "Gameplay/CameraManager.h"

class sToneMapping : public sPostProcess
{
    sClassBody(sClassConstructor, sToneMapping, sPostProcess)
public:
    __declspec(align(256)) struct sToneMappingConstants
    {
        float exposure; 
        int toneMapper;
        int gamma2;
    };

public:
    sToneMapping(std::size_t Width, std::size_t Height);
	virtual ~sToneMapping();

    virtual bool HasFrameBuffer() const override final { return true; }
    virtual IRenderTarget* GetFrameBuffer() const override final { return PostProcessFB.get(); }
    virtual void SetFrameBufferSize(const std::size_t Width, const std::size_t Height);

    virtual bool UseBackBufferAsResource() const override final { return true; }
    virtual std::size_t GetBackBufferResourceRootParameterIndex() const override final { return 1; }

    virtual void SetPostProcessResources(IGraphicsCommandContext* Context) override final;

    void SetTonemapper(int Val);
    int GetTonemapperIndex() const;

private:
    IRenderTarget::SharedPtr PostProcessFB;
    IConstantBuffer::SharedPtr ToneMappingCB;
    int TonemapperIndex;
};
