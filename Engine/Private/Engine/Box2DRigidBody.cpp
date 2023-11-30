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
#include "Engine/Box2DRigidBody.h"
#include "Engine/World2D.h"

#define DOWNSCALE World->GetPhysicalWorldScale()
#define UPSCALE 1.0f / World->GetPhysicalWorldScale()

sBox2DRigidBody::sBox2DRigidBody(sPhysicalComponent* InOwner, IPhysicalWorld* InWorld, b2Body* InBody, const ERigidBodyShape InShapeType, const sRigidBodyDesc& InDesc)
	: Super()
	, Owner(InOwner)
	, World((sWorld2D*)InWorld)
	, Body(InBody)
	, Fixture(Body->GetFixtureList())
	, Desc(InDesc)
	, ShapeType(InShapeType)
{
	SetUserPointer(this);
}

sBox2DRigidBody::~sBox2DRigidBody()
{
	World->DestroyBody(Body);
	World = nullptr;
	Owner = nullptr;
	Body = nullptr;
	Fixture = nullptr;
}

bool sBox2DRigidBody::HasOwner() const
{
	return Owner != nullptr;
}

sPhysicalComponent* sBox2DRigidBody::GetOwner() const
{
	return Owner;
}

void sBox2DRigidBody::OnUpdatePhysics()
{

}

void sBox2DRigidBody::SetCollisionChannel(ECollisionChannel Type)
{
	b2Filter Filter;
	Filter.categoryBits = (std::uint16_t)Type;
	if (Type == ECollisionChannel::None)
		Filter.maskBits = 0;
	else if (Type == ECollisionChannel::Default)
		Filter.maskBits = 0xFFFF;
	else
		Filter.maskBits = (std::uint16_t)Type;

	Fixture->SetFilterData(Filter);
}

void sBox2DRigidBody::SetCollisionChannel(ECollisionChannel Type, std::uint16_t CollideTo)
{
	b2Filter Filter;
	Filter.categoryBits = (std::uint16_t)Type;
	Filter.maskBits = CollideTo;

	Fixture->SetFilterData(Filter);
}

ECollisionChannel sBox2DRigidBody::GetCollisionChannel() const
{
	auto& Filter = Fixture->GetFilterData();
	return ECollisionChannel(Filter.categoryBits);
}

ERigidBodyShape sBox2DRigidBody::GetShape() const
{
	return ShapeType;
}

sRigidBodyDesc sBox2DRigidBody::GetDesc() const
{
	return Desc;
}

void sBox2DRigidBody::SetTransform(const FVector& position, const FQuaternion& orientation) const
{
	Body->SetTransform(b2Vec2(position.X * DOWNSCALE, position.Y * DOWNSCALE), b2Atan2(orientation.Z, orientation.W) * 2.0f);
}

FVector sBox2DRigidBody::GetLocation() const
{
	auto P = Body->GetPosition();
	return FVector(P.x * UPSCALE, P.y * UPSCALE, 0.0f);
}

FQuaternion sBox2DRigidBody::GetRotation() const
{
	return FQuaternion(FAngles(0.0f, 0.0f, RadiansToDegrees(Body->GetTransform().q.GetAngle())));
}

void sBox2DRigidBody::GetAabb(FVector& InMýn, FVector& InMax) const
{
	if (!IsEnabled())
		return;

	auto F = Body->GetFixtureList();
	auto AABB = F->GetAABB(0);

	InMýn.X = AABB.lowerBound.x * UPSCALE;
	InMýn.Y = AABB.lowerBound.y * UPSCALE;

	InMax.X = AABB.upperBound.x * UPSCALE;
	InMax.Y = AABB.upperBound.y * UPSCALE;
}

FVector sBox2DRigidBody::GetInertia() const
{
	return Body->GetInertia();
}

FMatrix sBox2DRigidBody::GetTransformMatrix() const
{
	auto P = Body->GetTransform().p;
	auto R = Body->GetTransform().q;
	return ToMatrixWithScale(FVector(P.x * UPSCALE, P.y * UPSCALE, 0.0f), FVector(UPSCALE, UPSCALE, UPSCALE), FQuaternion(FAngles(R.GetAngle())));
}

void sBox2DRigidBody::SetUserPointer(void* InActor)
{
	b2BodyUserData& Data = Body->GetUserData();
	Data.pointer = (uintptr_t)InActor;
}

void* sBox2DRigidBody::GetUserPointer() const
{
	return (void*)Body->GetUserData().pointer;
}

float sBox2DRigidBody::GetMass() const
{
	return Body->GetMass();
}

void sBox2DRigidBody::SetMass(float InMass)
{
	b2MassData Data;
	Data.mass = InMass;
	Body->SetMassData(&Data);
	Body->ResetMassData();
}

float sBox2DRigidBody::GetFriction() const 
{
	return Fixture->GetFriction();
}

void sBox2DRigidBody::SetFriction(float InFriction) 
{
	Fixture->SetFriction(InFriction);
}

float sBox2DRigidBody::GetRestitution() const
{
	return Fixture->GetRestitution();
}

void sBox2DRigidBody::SetRestitution(float InRestitution) 
{
	Fixture->SetRestitution(InRestitution);
}

float sBox2DRigidBody::GetRestitutionThreshold() const
{
	return Fixture->GetRestitutionThreshold();
}

void sBox2DRigidBody::SetRestitutionThreshold(float InRestitutionThreshold) 
{
	Fixture->SetRestitutionThreshold(InRestitutionThreshold);
}

float sBox2DRigidBody::GetDensity() const 
{
	return Fixture->GetDensity();
}

void sBox2DRigidBody::SetDensity(float InDensity) 
{
	Fixture->SetDensity(InDensity);
	Body->ResetMassData();
}

void sBox2DRigidBody::SetLinearVelocity(const FVector& v)
{
	Body->SetLinearVelocity(b2Vec2(v.X, v.Y));
}

FVector sBox2DRigidBody::GetLinearVelocity() const
{
	auto V = Body->GetLinearVelocity();
	return FVector(V.x, V.y, 0.0f);
}

void sBox2DRigidBody::SetLinearDamping(const float v)
{
	Body->SetLinearDamping(v);
}

float sBox2DRigidBody::GetLinearDamping() const
{
	return Body->GetLinearDamping();
}

void sBox2DRigidBody::SetAngularVelocity(float omega)
{
	Body->SetAngularVelocity(omega);
}

float sBox2DRigidBody::GetAngularVelocity() const
{
	return Body->GetAngularVelocity();
}

void sBox2DRigidBody::SetAngularDamping(const float v)
{
	Body->SetAngularDamping(v);
}

float sBox2DRigidBody::GetAngularDamping() const
{
	return Body->GetAngularDamping();
}

void sBox2DRigidBody::ApplyForce(const FVector& force, const FVector& point, bool wake)
{
	Body->ApplyForce(b2Vec2(force.X, force.Y), b2Vec2(point.X, point.Y), wake);
}

void sBox2DRigidBody::ApplyForceToCenter(const FVector& force, bool wake)
{
	Body->ApplyForceToCenter(b2Vec2(force.X, force.Y), wake);
}

void sBox2DRigidBody::ApplyTorque(float torque, bool wake)
{
	Body->ApplyTorque(torque, wake);
}

void sBox2DRigidBody::ApplyLinearImpulse(const FVector& impulse, const FVector& point, bool wake)
{
	Body->ApplyLinearImpulse(b2Vec2(impulse.X, impulse.Y), b2Vec2(point.X, point.Y), wake);
}

void sBox2DRigidBody::ApplyLinearImpulseToCenter(const FVector& impulse, bool wake)
{
	Body->ApplyLinearImpulseToCenter(b2Vec2(impulse.X, impulse.Y), wake);
}

void sBox2DRigidBody::ApplyAngularImpulse(float impulse, bool wake)
{
	Body->ApplyAngularImpulse(impulse, wake);
}

FVector sBox2DRigidBody::GetLinearVelocityFromWorldPoint(const FVector& worldPoint) const
{
	auto V = Body->GetLinearVelocityFromWorldPoint(b2Vec2(worldPoint.X, worldPoint.Y));
	return FVector(V.x, V.y, 0.0f);
}

FVector sBox2DRigidBody::GetLinearVelocityFromLocalPoint(const FVector& localPoint) const
{
	auto V = Body->GetLinearVelocityFromLocalPoint(b2Vec2(localPoint.X, localPoint.Y));
	return FVector(V.x, V.y, 0.0f);
}

FVector sBox2DRigidBody::GetWorldCenter() const
{
	auto V = Body->GetWorldCenter();
	return FVector(V.x, V.y, 0.0f);
}

void sBox2DRigidBody::SetGravityScale(float scale)
{
	Body->SetGravityScale(scale);
}

void sBox2DRigidBody::SetGravityScale(const FVector& scale)
{
	Body->SetGravityScale(scale.Y);
}

FVector sBox2DRigidBody::GetGravityScale() const
{
	return FVector(0.0f, Body->GetGravityScale(), 0.0f);
}

void sBox2DRigidBody::SetType(ERigidBodyType type)
{
	switch (Desc.RigidBodyType)
	{
	case ERigidBodyType::Static:
		Body->SetType(b2BodyType::b2_staticBody);
		break;
	case ERigidBodyType::Kinematic:
		Body->SetType(b2BodyType::b2_kinematicBody);
		break;
	case ERigidBodyType::Dynamic:
		Body->SetType(b2BodyType::b2_dynamicBody);
		break;
	}
}

void sBox2DRigidBody::SetBullet(bool flag)
{
	Body->SetBullet(flag);
}

bool sBox2DRigidBody::IsBullet() const
{
	return Body->IsBullet();
}

void sBox2DRigidBody::SetAwake(bool flag)
{
	Body->SetAwake(flag);
}

bool sBox2DRigidBody::IsAwake() const
{
	return Body->IsAwake();
}

void sBox2DRigidBody::SetEnabled(bool flag)
{
	Body->SetAwake(flag);
	//Body->SetEnabled(flag);
}

bool sBox2DRigidBody::IsEnabled() const
{
	return Body->IsAwake();
	//return Body->IsEnabled();
}

void sBox2DRigidBody::SetFixedRotation(bool flag)
{
	Body->SetFixedRotation(flag);
}

bool sBox2DRigidBody::IsFixedRotation() const
{
	return Body->IsFixedRotation();
}
