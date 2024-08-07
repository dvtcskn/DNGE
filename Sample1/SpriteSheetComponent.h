/* ---------------------------------------------------------------------------------------
* MIT License
*
* Copyright (c) 2023 Davut Co�kun.
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
#include "AbstractGI/Mesh.h"
#include "Sprite.h"
#include "AnimationStates.h"

class sSpriteEffectComponent;

class sSpriteSheetComponent : public IMeshComponent
{
	sClassBody(sClassConstructor, sSpriteSheetComponent, IMeshComponent)
private:
	class sSpriteSheetComponentMeshContainer : public IMesh
	{
		sClassBody(sClassConstructor, sSpriteSheetComponentMeshContainer, IMesh)
	public:
		sSpriteSheetComponentMeshContainer(sSpriteSheetComponent* pOwner)
			: Super()
			, Owner(pOwner)
		{}

		virtual ~sSpriteSheetComponentMeshContainer()
		{
			Owner = nullptr;
		}

		virtual std::string GetName() const override final { return Owner->GetName(); }

		virtual std::size_t GetSecondaryConstantBufferCount() const { return Owner->AnimationCB != nullptr ? 1 : 0; }
		virtual IConstantBuffer* GetSecondaryConstantBuffer(std::size_t Index) const { return Owner->AnimationCB.get(); }
		virtual std::vector<IConstantBuffer*> GetSecondaryConstantBuffers() const { return std::vector<IConstantBuffer*>{ Owner->AnimationCB.get() }; }

		virtual IConstantBuffer* GetMeshConstantBuffer() const override final { return Owner->GetMeshConstantBuffer(); }
		virtual IVertexBuffer* GetVertexBuffer() const override final { return Owner->CurrentKeyFrame->Sprite->GetVertexBuffer(); }
		virtual IIndexBuffer* GetIndexBuffer() const override final { return Owner->CurrentKeyFrame->Sprite->GetIndexBuffer(); };

		virtual bool HasInstanceBuffer() const override final { return false; };
		virtual IVertexBuffer* GetInstanceBuffer() const override final { return nullptr; };

		virtual sObjectDrawParameters GetDrawParameters() const override final { return Owner->CurrentKeyFrame->Sprite->GetDrawParameters(); }

		virtual std::vector<sMaterial::sMaterialInstance*> GetMaterialInstances() const override final { return std::vector<sMaterial::sMaterialInstance*>{Owner->CurrentKeyFrame->Sprite->GetMaterialInstance()}; }
		virtual std::int32_t GetNumMaterials() const override final { return 1; }
		virtual sMaterial::sMaterialInstance* GetMaterialInstance(/*std::int32_t Index = 0*/) const override final { return Owner->CurrentKeyFrame->Sprite->GetMaterialInstance(); }
		virtual std::string GetMaterialName(/*std::int32_t Index = 0*/) const override final { return Owner->CurrentKeyFrame->Sprite->GetMaterialInstance()->GetName(); }

		virtual void Serialize(sArchive& archive) override {}

	private:
		sSpriteSheetComponent* Owner;
	};

public:
	sSpriteSheetComponent(const std::string Name);
	sSpriteSheetComponent(sSpriteSheet* SpriteSheet);
	virtual ~sSpriteSheetComponent();

	virtual void OnBeginPlay() override;
	virtual void OnFixedUpdate(const double DeltaTime) override;

	IConstantBuffer* GetMeshConstantBuffer() const { return MeshConstantBuffer.get(); }

	virtual IMesh* GetMesh() const override final { return MeshContainer.get(); }
	virtual std::string GetMeshName() const override final { return MeshContainer->GetName(); }

	inline sSpriteSheet* GetCurrentSpriteSheet() const { return SpriteSheet; }
	sSprite* GetCurrentSprite() const;
	std::size_t GetCurrentSpriteIndex() const;

	bool IsAnimationEnded() const;
	bool IsAnimationRunning() const;

	void Run(const float DeltaTime);
	void Play();
	void Pause();
	void Stop();
	void Reset();
	void ForceEndAnimation();

	bool IsFlipped() const;
	void Flip(bool value);

	void SetSpriteSheet(sSpriteSheet* SpriteSheet);

	FVector Offset;

	virtual void Serialize(sArchive& archive) override;

	void BindFunction_AnimationStarted(std::function<void()> pf);
	void BindFunction_AnimationEnded(std::function<void()> pf);
	void BindFunction_FrameUpdated(std::function<void(std::size_t)> pf);

private:
	void UpdateFrame(std::optional<std::size_t> Frame = std::nullopt);

	virtual void OnUpdateTransform() override final;

	void OnFrameUpdated();
	void OnAnimationUpdated();

	virtual void OnAttachToComponent() override final;
	virtual void OnDetachFromComponent() override final;

	void OnAnimationStarted();
	void OnAnimationEnded();

	//void UpdateFrameAtTime(float DeltaTime);

private:
	sSpriteSheet* SpriteSheet;

	IConstantBuffer::SharedPtr MeshConstantBuffer;
	sMeshConstantBufferAttributes ObjectConstants;
	sSpriteSheetComponentMeshContainer::UniquePtr MeshContainer;

	bool bFlip;
	bool bIsAnimationActive;

	std::size_t FrameCount;
	std::size_t AnimSeqFrameCount;

	std::size_t CurrentSpriteIndex;
	sSpriteSheetKeyFrame* CurrentKeyFrame;

	sSpriteEffectComponent* SpriteEffectComponent;

	std::function<void()> fAnimationStarted;
	std::function<void()> fAnimationEnd;
	std::function<void(std::size_t)> fFrameUpdated;

	__declspec(align(256)) struct sAnimationFlip
	{
		//uint LayerIndex;
		std::uint32_t Flip;
		//float FlipMaxX = 0;
		//float FlipMinX = 0;
	};
	sAnimationFlip AnimationFlip;
	IConstantBuffer::SharedPtr AnimationCB;

	//float ElapsedTime;
	//float FrameTime;
};
