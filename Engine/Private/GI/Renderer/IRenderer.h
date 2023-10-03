#pragma once

#include "Engine/ClassBody.h"
#include "Engine/AbstractEngine.h"
#include "Utilities/Input.h"

class sRenderer;

class IRenderer
{
	sBaseClassBody(sClassNoDefaults, IRenderer);
protected:
	IRenderer()
	{}

public:
	virtual ~IRenderer()
	{
	}

	virtual void BeginPlay() = 0;
	virtual void Tick(const double DeltaTime) = 0;

	//virtual void Render(IFrameBuffer* pFB) = 0;

	virtual void SetRenderSize(std::size_t Width, std::size_t Height) = 0;
	virtual void OnInputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar) = 0;
};
