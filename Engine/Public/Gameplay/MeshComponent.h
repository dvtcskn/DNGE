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

#include "PhysicalComponent.h"
#include <vector>
#include <memory>
#include "Core/Math/CoreMath.h"
#include "AbstractGI/RendererResource.h"

class IMeshComponent : public sPrimitiveComponent
{
	sClassBody(sClassNoDefaults, IMeshComponent, sPrimitiveComponent)
protected:
	IMeshComponent(std::string InName, sActor* pActor = nullptr)
		: Super(InName, pActor)
	{}
public:
	virtual ~IMeshComponent() = default;

	virtual IMesh* GetMesh() const = 0;
	virtual std::string GetMeshName() const = 0;
};

class sMeshComponent : public IMeshComponent
{
	sClassBody(sClassConstructor, sMeshComponent, IMeshComponent)
public:
	sMeshComponent(const std::string Name, const std::string Path, const sMeshData& InData);
	sMeshComponent(const std::string Name, const std::string Path, sActor* Owner, const sMeshData& InData);
	sMeshComponent(const std::string Name, EBasicMeshType MeshType, std::optional<FBoxDimension> Dimension = std::nullopt);
	sMeshComponent(const std::string Name, sActor* Owner, EBasicMeshType MeshType, std::optional<FBoxDimension> Dimension = std::nullopt);

	virtual ~sMeshComponent();

	virtual IMesh* GetMesh() const override final { return Mesh.get(); }
	virtual std::string GetMeshName() const override final { return Mesh->GetName(); }
	std::string GetPath() const { return Mesh->GetPath(); }

	void SetMaterial(/*std::int32_t Index,*/ sMaterial::sMaterialInstance* Material);

	virtual void Serialize(sArchive& archive) override;

private:
	virtual void OnUpdateTransform() override final;

private:
	sMesh::UniquePtr Mesh;
	sMeshConstantBufferAttributes ObjectConstants;
};
