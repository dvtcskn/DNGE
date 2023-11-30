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

#include <vector>
#include "Core/Transform.h"
#include "Core/Archive.h"
#include "AbstractGI/Material.h"
#include "Core/Math/CoreMath.h"
#include "Engine/IRigidBody.h"
#include "AbstractGI/RendererResource.h"
#include "ILevel.h"

class sStaticMesh : public sMesh, public std::enable_shared_from_this<sStaticMesh>
{
	sClassBody(sClassConstructor, sStaticMesh, sMesh)
public:
	sStaticMesh(const std::string& InName, const std::string& InPath, const sMeshData& InData);
	virtual ~sStaticMesh();

	virtual void BeginPlay();
	virtual void Tick(const double DeltaTime);

	void SetTransform(Transform InTransform);
	inline FVector GetLocation()const { return mTransform.GetLocation(); }
	inline FVector GetScale()const { return mTransform.GetScale(); }
	inline FVector4 GetRotation()const { return mTransform.GetRotation(); }
	void SetLocation(FVector InLocation);
	void SetRollPitchYaw(FVector4 RPY);
	void SetScale(FVector InScale);
	void SetScale(float InScale);

	void AddToLevel(ILevel* pLevel, std::size_t LayerIndex = 0);
	ILevel* GetOwnedLevel() const { return Level; };

protected:
	void UpdateTransform();

private:
	Transform mTransform;
	ILevel* Level;

	sMeshConstantBufferAttributes ObjectConstants;
};

