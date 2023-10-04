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

#include <Gameplay/PhysicalComponent.h>
#include <Gameplay/MeshComponent.h>
#include <vector>
#include <memory>
#include "Core/Math/CoreMath.h"
#include "AbstractGI/RendererResource.h"
#include "Sprite.h"
#include "AnimationStates.h"

class sSpriteComponent : public IMeshComponent
{
	sClassBody(sClassConstructor, sSpriteComponent, IMeshComponent)
private:
	class sSpriteComponentMeshContainer : public IMesh
	{
		sClassBody(sClassConstructor, sSpriteComponentMeshContainer, IMesh)
	public:
		sSpriteComponentMeshContainer(sSpriteComponent* pOwner)
			: Super()
			, Owner(pOwner)
		{}

		virtual ~sSpriteComponentMeshContainer()
		{
			Owner = nullptr;
		}

		virtual std::string GetName() const override final { return Owner->GetName(); }

		virtual IConstantBuffer* GetMeshConstantBuffer() const override final { return Owner->GetMeshConstantBuffer(); }
		virtual IVertexBuffer* GetVertexBuffer() const override final { return Owner->Sprite->GetVertexBuffer(); }
		virtual IIndexBuffer* GetIndexBuffer() const override final { return Owner->Sprite->GetIndexBuffer(); };

		virtual sObjectDrawParameters GetDrawParameters() const override final { return Owner->Sprite->GetDrawParameters(); }

		virtual std::vector<sMaterial::sMaterialInstance*> GetMaterialInstances() const override final { return std::vector<sMaterial::sMaterialInstance*>{Owner->Sprite->GetMaterialInstance()}; }
		virtual std::int32_t GetNumMaterials() const override final { return 1; }
		virtual sMaterial::sMaterialInstance* GetMaterialInstance(/*std::int32_t Index = 0*/) const override final { return Owner->Sprite->GetMaterialInstance(); }
		virtual std::string GetMaterialName(/*std::int32_t Index = 0*/) const override final { return Owner->Sprite->GetMaterialInstance()->GetName(); }

		virtual void Serialize(Archive& archive) override {}

	private:
		sSpriteComponent* Owner;
	};

public:
	sSpriteComponent(const std::string Name);
	sSpriteComponent(sSprite* Sprite);
	virtual ~sSpriteComponent();

	virtual void OnBeginPlay() override;
	virtual void OnTick(const double DeltaTime) override;

	IConstantBuffer* GetMeshConstantBuffer() const { return MeshConstantBuffer.get(); };

	virtual IMesh* GetMesh() const override final { return MeshContainer.get(); }
	virtual std::string GetMeshName() const override final { return MeshContainer->GetName(); }

	void SetSprite(sSprite* pSprite);

	bool IsFlipped() const;
	void Flip(bool value);

	virtual void Serialize(Archive& archive) override;

	FVector Offset;

private:
	virtual void OnUpdateTransform() override final;

	virtual void OnAttachToComponent() override final;
	virtual void OnDetachFromComponent() override final;

	void UpdateOffset();

private:
	sSprite* Sprite;
	sSpriteComponentMeshContainer::UniquePtr MeshContainer;

	IConstantBuffer::SharedPtr MeshConstantBuffer;
	sMeshConstantBufferAttributes ObjectConstants;

	bool bFlip;
};
