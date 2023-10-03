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

