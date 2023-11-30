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
#include "Gameplay/StaticMesh.h"

sStaticMesh::sStaticMesh(const std::string& InName, const std::string& InPath, const sMeshData& InData)
	: sMesh(InName, InPath, InData)
	, Level(nullptr)
{
	ObjectConstants.PrevModelMatrix = FMatrix::Identity();
	ObjectConstants.modelMatrix = FMatrix::Identity();
}

sStaticMesh::~sStaticMesh()
{
	Level = nullptr;
}

void sStaticMesh::BeginPlay()
{
}

void sStaticMesh::Tick(const double DeltaTime)
{
}

void sStaticMesh::AddToLevel(ILevel* pLevel, std::size_t LayerIndex)
{
	Level = pLevel;
	Level->AddMesh(shared_from_this(), LayerIndex);
}

void sStaticMesh::SetTransform(Transform InTransform)
{
	mTransform = InTransform;
	UpdateTransform();
}

void sStaticMesh::SetLocation(FVector InLocation)
{
	mTransform.SetLocation(InLocation);
	UpdateTransform();
}

void sStaticMesh::SetRollPitchYaw(FVector4 RPY)
{
	mTransform.SetRotation(RPY);
	UpdateTransform();
}

void sStaticMesh::SetScale(FVector InScale)
{
	mTransform.SetScale(InScale);
	UpdateTransform();
}

void sStaticMesh::SetScale(float InScale)
{
	mTransform.SetScale(InScale);
	UpdateTransform();
}

void sStaticMesh::UpdateTransform()
{
	ObjectConstants.PrevModelMatrix = ObjectConstants.modelMatrix;
	ObjectConstants.modelMatrix = ToMatrixWithScale(mTransform.GetLocation(), mTransform.GetScale(), mTransform.GetRotation());

	SetMeshTransform(ObjectConstants);
}

//void sDynamicMesh::ResetRigidBody()
//{
//	if (Shape == ERigidBodyShape::Box3D)
//	{
//		auto Vertices = GetVertices();
//
//		auto xExtremes = std::minmax_element(Vertices.begin(), Vertices.end(),
//			[](const sVertexBufferEntry& lhs, const sVertexBufferEntry& rhs) {
//				return lhs.position.X < rhs.position.X;
//			});
//
//		auto yExtremes = std::minmax_element(Vertices.begin(), Vertices.end(),
//			[](const sVertexBufferEntry& lhs, const sVertexBufferEntry& rhs) {
//				return lhs.position.Y < rhs.position.Y;
//			});
//
//		auto zExtremes = std::minmax_element(Vertices.begin(), Vertices.end(),
//			[](const sVertexBufferEntry& lhs, const sVertexBufferEntry& rhs) {
//				return lhs.position.Z < rhs.position.Z;
//			});
//
//		FVector upperLeft(xExtremes.first->position.X, yExtremes.first->position.Y, zExtremes.first->position.Z);
//		FVector lowerRight(xExtremes.second->position.X, yExtremes.second->position.Y, zExtremes.first->position.Z);
//
//		FBoundingBox Bounds(upperLeft, lowerRight);
//		auto Dimension = Bounds.GetDimension();
//
//		sRigidBodyDesc Desc;
//		Desc.UserPointer = this;
//		Desc.Friction = 1.0f;
//		Desc.Mass = 1.0f;
//		Desc.Restitution = 0.0f;
//		Desc.RigidBodyType = BodyType;
//
//		Body = nullptr;
//		//mTransform.SetLocation(Bounds.GetCenter());
//		//Body = IRigidBody::CreateBoxBody(Desc, FVector::Zero() /*mTransform.GetTranslation()*/, FVector(Dimension.Width / 2.0f, Dimension.Height / 2.0f, Dimension.Depth / 2.0f));
//		Body->SetTransform(GetLocation(), GetRotation());
//	}
//	else if (Shape == ERigidBodyShape::Box2D)
//	{
//		auto Vertices = GetVertices();
//
//		auto xExtremes = std::minmax_element(Vertices.begin(), Vertices.end(),
//			[](const sVertexBufferEntry& lhs, const sVertexBufferEntry& rhs) {
//				return lhs.position.X < rhs.position.X;
//			});
//
//		auto yExtremes = std::minmax_element(Vertices.begin(), Vertices.end(),
//			[](const sVertexBufferEntry& lhs, const sVertexBufferEntry& rhs) {
//				return lhs.position.Y < rhs.position.Y;
//			});
//
//		FVector2 upperLeft(xExtremes.first->position.X, yExtremes.first->position.Y);
//		FVector2 lowerRight(xExtremes.second->position.X, yExtremes.second->position.Y);
//
//		FBounds2D Bounds(upperLeft, lowerRight);
//
//		sRigidBodyDesc Desc;
//		Desc.UserPointer = this;
//		Desc.Friction = 1.0f;
//		Desc.Mass = 1.0f;
//		Desc.Restitution = 0.0f;
//		Desc.RigidBodyType = BodyType;
//
//		Body = nullptr;
//		//Body = IRigidBody::Create2DBoxBody(Desc, Bounds);
//		Body->SetTransform(GetLocation(), GetRotation());
//	}
//}
