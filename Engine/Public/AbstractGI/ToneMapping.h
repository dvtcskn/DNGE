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
