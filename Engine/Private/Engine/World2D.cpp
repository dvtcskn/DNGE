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
#include "Engine/World2D.h"
#include "Engine/Box2DRigidBody.h"
#include <array>
#include "Gameplay/PhysicalComponent.h"

#define DOWNSCALE PhysicalWorldScale
#define UPSCALE 1.0f / PhysicalWorldScale

std::vector<IRigidBody*> RigidBodys;

void sWorld2D::DestructionListener::SayGoodbye(b2Fixture* fixture)
{
	B2_NOT_USED(fixture);
	//void* UserData = (void*)fixture->GetBody()->GetUserData().pointer;
}

void sWorld2D::DestructionListener::SayGoodbye(b2Joint* joint)
{
	World2D->JointDestroyed(joint);
}

class AABBQueryCallback : public b2QueryCallback
{
public:
	std::vector<b2Body*> foundBodies;

	virtual bool ReportFixture(b2Fixture* fixture) override final
	{
		foundBodies.push_back(fixture->GetBody());
		return true;//keep going to find all fixtures in the query area
	}
};

class RTQueryCallback : public b2QueryCallback
{
public:
	RTQueryCallback(const b2Vec2& point)
		: Body(nullptr)
		, m_point(point)
	{}

	sPhysicalComponent* Body;
	b2Vec2 m_point;

	virtual bool ReportFixture(b2Fixture* fixture) override final
	{
		bool inside = fixture->TestPoint(m_point);
		if (inside)
		{
			Body = static_cast<IRigidBody*>((void*)fixture->GetBody()->GetUserData().pointer)->GetOwner();
			return false;
		}
		return true;//keep going to find all fixtures in the query area
	}
};

// This callback finds the closest hit. Polygon 0 is filtered.
class RayCastClosestCallback : public b2RayCastCallback
{
public:
	RayCastClosestCallback()
		: Body(nullptr)
		, m_point(b2Vec2(0.0f, 0.0f))
		, m_normal(b2Vec2(0.0f, 0.0f))
		, m_hit(false)
	{}

	virtual float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override final
	{
		uintptr_t index = fixture->GetBody()->GetUserData().pointer;
		if (index == 1)
		{
			// By returning -1, we instruct the calling code to ignore this fixture and
			// continue the ray-cast to the next fixture.
			return -1.0f;
		}

		m_hit = true;
		m_point = point;
		m_normal = normal;
		Body = static_cast<IRigidBody*>((void*)fixture->GetBody()->GetUserData().pointer)->GetOwner();

		// By returning the current fraction, we instruct the calling code to clip the ray and
		// continue the ray-cast to the next fixture. WARNING: do not assume that fixtures
		// are reported in order. However, by clipping, we can always get the closest fixture.
		//return fraction;

		// At this point we have a hit, so we know the ray is obstructed.
		// By returning 0, we instruct the calling code to terminate the ray-cast.
		return 0.0f;

		// By returning 1, we instruct the caller to continue without clipping the ray.
		//return 1.0f;
	}

	bool m_hit;
	b2Vec2 m_point;
	b2Vec2 m_normal;
	sPhysicalComponent* Body;
};

sWorld2D::sWorld2D(const float InPhysicalWorldScale)
	: Super()
	, m_velocityIterations(8)
	, m_positionIterations(12)
	//, InternalTick(1.0 / 60.0)
	, InternalTick(std::nullopt)
	, PhysicalWorldScale(InPhysicalWorldScale)
{
	b2Vec2 gravity;
	//gravity.Set(0.0f, -9.8f);
	gravity.Set(0.0f, 9.8f);
	//gravity.Set(0.0f, 100.0f);
	m_world = new b2World(gravity);
	m_pointCount = 0;
	
	m_destructionListener.World2D = this;
	m_world->SetDestructionListener(&m_destructionListener);
	m_world->SetContactListener(this);

	m_world->SetAllowSleeping(false);

	m_world->SetContinuousPhysics(true);
	m_world->SetSubStepping(true);
}

sWorld2D::~sWorld2D()
{
	DeferredDestroy();

	// By deleting the world, we delete the bomb, mouse joint, etc.
	delete m_world;
	m_world = NULL;

	RigidBodys.clear();
}

void sWorld2D::Tick(const double InDeltaTime)
{
	m_pointCount = 0;

	if (InternalTick.has_value())
		m_world->Step((float)*InternalTick, m_velocityIterations, m_positionIterations);
	else
		m_world->Step((float)InDeltaTime, m_velocityIterations, m_positionIterations);

	DeferredDestroy();

	UpdateBodyPhysics();
}

void sWorld2D::BeginContact(b2Contact* contact)
{
	if (std::find(DeferredDestroyBodyList.begin(), DeferredDestroyBodyList.end(), contact->GetFixtureA()->GetBody()) != DeferredDestroyBodyList.end())
		return;
	if (std::find(DeferredDestroyBodyList.begin(), DeferredDestroyBodyList.end(), contact->GetFixtureB()->GetBody()) != DeferredDestroyBodyList.end())
		return;

	BeginCollision(static_cast<IRigidBody*>((void*)contact->GetFixtureA()->GetBody()->GetUserData().pointer),
		static_cast<IRigidBody*>((void*)contact->GetFixtureB()->GetBody()->GetUserData().pointer));
}

void sWorld2D::EndContact(b2Contact* contact)
{
	if (std::find(DeferredDestroyBodyList.begin(), DeferredDestroyBodyList.end(), contact->GetFixtureA()->GetBody()) != DeferredDestroyBodyList.end())
		return;
	if (std::find(DeferredDestroyBodyList.begin(), DeferredDestroyBodyList.end(), contact->GetFixtureB()->GetBody()) != DeferredDestroyBodyList.end())
		return;

	EndCollision(static_cast<IRigidBody*>((void*)contact->GetFixtureA()->GetBody()->GetUserData().pointer),
		static_cast<IRigidBody*>((void*)contact->GetFixtureB()->GetBody()->GetUserData().pointer));
}

void sWorld2D::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
{
	const b2Manifold* manifold = contact->GetManifold();

	if (manifold->pointCount == 0)
	{
		return;
	}

	b2Fixture* fixtureA = contact->GetFixtureA();
	b2Fixture* fixtureB = contact->GetFixtureB();

	b2PointState state1[b2_maxManifoldPoints], state2[b2_maxManifoldPoints];
	b2GetPointStates(state1, state2, oldManifold, manifold);

	b2WorldManifold worldManifold;
	contact->GetWorldManifold(&worldManifold);

	for (int32 i = 0; i < manifold->pointCount && m_pointCount < k_maxContactPoints; ++i)
	{
		ContactPoint* cp = m_points + m_pointCount;
		cp->fixtureA = fixtureA;
		cp->fixtureB = fixtureB;
		cp->position = worldManifold.points[i];
		cp->normal = worldManifold.normal;
		cp->state = state2[i];
		cp->normalImpulse = manifold->points[i].normalImpulse;
		cp->tangentImpulse = manifold->points[i].tangentImpulse;
		cp->separation = worldManifold.separations[i];
		++m_pointCount;
	}
}

void sWorld2D::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
{
	B2_NOT_USED(contact); 
	B2_NOT_USED(impulse);
}

FVector sWorld2D::GetGravity() const 
{
	auto Gravity = m_world->GetGravity();
	return FVector(Gravity.x, Gravity.y, 0.0f);
}

void sWorld2D::SetGravity(const FVector& Gravity)
{
	m_world->SetGravity(b2Vec2(Gravity.X, Gravity.Y));
}

void sWorld2D::SetWorldOrigin(const FVector& newOrigin)
{
	m_world->ShiftOrigin(b2Vec2(newOrigin.X, newOrigin.Y));
}

std::size_t sWorld2D::GetBodyCount() const
{
	return m_world->GetBodyCount();
}

void sWorld2D::DestroyBody(b2Body* Body)
{
	if (std::find(DeferredDestroyBodyList.begin(), DeferredDestroyBodyList.end(), Body) != DeferredDestroyBodyList.end())
		return;
	DeferredDestroyBodyList.push_back(Body);
}

void sWorld2D::DeferredDestroy()
{
	if (DeferredDestroyBodyList.size() > 0)
	{
		for (auto& Body : DeferredDestroyBodyList)
			m_world->DestroyBody(Body);
		DeferredDestroyBodyList.clear();
	}
}

sPhysicalComponent* sWorld2D::GetPhysicalBody(std::size_t Index) const
{
	auto BodyCount = m_world->GetBodyCount();
	auto Bodys = m_world->GetBodyList();

	b2Body* Body = Bodys;
	for (std::size_t i = 0; i < BodyCount; i++)
	{
		if (i > Index)
			break;

		if (Index == i)
		{
			void* ptr = (void*)Body->GetUserData().pointer;
			if (ptr)
			{
				return static_cast<IRigidBody*>(ptr)->GetOwner();
			}
		}

		Body = Body->GetNext();
	}
	return nullptr;
}

IRigidBody* sWorld2D::GetBody(std::size_t Index) const
{
	auto BodyCount = m_world->GetBodyCount();
	auto Bodys = m_world->GetBodyList();

	b2Body* Body = Bodys;
	for (std::size_t i = 0; i < BodyCount; i++)
	{
		if (i > Index)
			break;

		if (Index == i)
		{
			void* ptr = (void*)Body->GetUserData().pointer;
			if (ptr)
			{
				return static_cast<IRigidBody*>(ptr);
			}
		}

		Body = Body->GetNext();
	}
	return nullptr;
}

sPhysicalComponent* sWorld2D::LineTraceToViewPort(const FVector& InOrigin, const FVector& InDirection) const
{
	/*RayCastClosestCallback callback;
	m_world->RayCast(&callback, b2Vec2(InOrigin.X, InOrigin.Y), b2Vec2(InDirection.X, InDirection.Y));

	if (callback.m_hit)
		return callback.Body;
	return nullptr;*/

	FBoundingBox Bounds = FBoundingBox(FBoxDimension(1.0f, 1.0f, 1.0f), InOrigin);
	b2AABB aabb;
	aabb.lowerBound = b2Vec2(Bounds.Min.X * DOWNSCALE, Bounds.Min.Y * DOWNSCALE);
	aabb.upperBound = b2Vec2(Bounds.Max.X * DOWNSCALE, Bounds.Max.Y * DOWNSCALE);

	RTQueryCallback callback(b2Vec2(InOrigin.X * DOWNSCALE, InOrigin.Y * DOWNSCALE));
	m_world->QueryAABB(&callback, aabb);
	return callback.Body;
}

std::vector<sPhysicalComponent*> sWorld2D::QueryAABB(const FBoundingBox& Bounds) const
{
	b2AABB aabb;
	aabb.lowerBound = b2Vec2(Bounds.Min.X * DOWNSCALE, Bounds.Min.Y * DOWNSCALE);
	aabb.upperBound = b2Vec2(Bounds.Max.X * DOWNSCALE, Bounds.Max.Y * DOWNSCALE);

	AABBQueryCallback callback;
	m_world->QueryAABB(&callback, aabb);
	std::vector<sPhysicalComponent*> Objs;
	for (auto& Body : callback.foundBodies)
	{
		if (std::find(DeferredDestroyBodyList.begin(), DeferredDestroyBodyList.end(), Body) != DeferredDestroyBodyList.end())
			continue;

		if (Body->GetUserData().pointer)
			Objs.push_back(static_cast<IRigidBody*>((void*)Body->GetUserData().pointer)->GetOwner());
	}
	return Objs;
}

void sWorld2D::SetPhysicsInternalTick(std::optional<double> Tick)
{
	InternalTick = Tick;
}

IRigidBody::SharedPtr sWorld2D::Create2DBoxBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FBounds2D& Bounds)
{
	auto BoxCenter = Bounds.GetCenter() * DOWNSCALE;
	b2BodyDef Def;
	switch (Desc.RigidBodyType)
	{
	case ERigidBodyType::Static:
		Def.type = b2BodyType::b2_staticBody;
		break;
	case ERigidBodyType::Kinematic:
		Def.type = b2BodyType::b2_kinematicBody;
		break;
	case ERigidBodyType::Dynamic:
		Def.type = b2BodyType::b2_dynamicBody;
		break;
	}
	Def.position = b2Vec2(BoxCenter.X, BoxCenter.Y);
	Def.linearDamping = 0.0f;
	Def.angularDamping = 1.0f;
	b2Body* Body = m_world->CreateBody(&Def);
	b2FixtureDef Fix;
	Fix.density = 0.005f;
	Fix.friction = Desc.Friction;
	Fix.restitution = Desc.Restitution;
	Fix.restitutionThreshold = 0.0f;
	Fix.userData.pointer = (uintptr_t)Desc.UserPointer;
	b2PolygonShape Box;
	Box.SetAsBox((Bounds.GetWidth() * DOWNSCALE) / 2.0f, (Bounds.GetHeight() * DOWNSCALE) / 2.0f, b2Vec2(BoxCenter.X, BoxCenter.Y), 0.0f);
	b2MassData Data;
	Data.mass = Desc.Mass;
	Data.center.x = BoxCenter.X;
	Data.center.y = BoxCenter.Y;
	Box.ComputeMass(&Data, Fix.density);
	Fix.shape = &Box;
	b2Fixture* Fixture = Body->CreateFixture(&Fix);
	auto RigidBody2D = sBox2DRigidBody::Create(Owner, this, Body, ERigidBodyShape::Box2D, Desc);
	//RigidBodys.push_back(RigidBody2D.get());
	return RigidBody2D;
}
IRigidBody::SharedPtr sWorld2D::Create2DPolygonBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, const std::array<FVector2, 8>& points)
{
	b2BodyDef Def;
	switch (Desc.RigidBodyType)
	{
	case ERigidBodyType::Static:
		Def.type = b2BodyType::b2_staticBody;
		break;
	case ERigidBodyType::Kinematic:
		Def.type = b2BodyType::b2_kinematicBody;
		break;
	case ERigidBodyType::Dynamic:
		Def.type = b2BodyType::b2_dynamicBody;
		break;
	}
	Def.position = b2Vec2(Origin.X * DOWNSCALE, Origin.Y * DOWNSCALE);
	Def.linearDamping = 0.0f;
	Def.angularDamping = 1.0f;
	b2Body* Body = m_world->CreateBody(&Def);
	b2FixtureDef Fix;
	Fix.density = 1.0f;
	Fix.friction = Desc.Friction;
	Fix.restitution = Desc.Restitution;
	Fix.restitutionThreshold = 0.0f;
	Fix.userData.pointer = (uintptr_t)Desc.UserPointer;
	b2PolygonShape Polygon;
	std::array<b2Vec2, 8> Vertices;
	Vertices[0] = b2Vec2(points[0].X * DOWNSCALE, points[0].Y * DOWNSCALE);
	Vertices[1] = b2Vec2(points[1].X * DOWNSCALE, points[1].Y * DOWNSCALE);
	Vertices[2] = b2Vec2(points[2].X * DOWNSCALE, points[2].Y * DOWNSCALE);
	Vertices[3] = b2Vec2(points[3].X * DOWNSCALE, points[3].Y * DOWNSCALE);
	Vertices[4] = b2Vec2(points[4].X * DOWNSCALE, points[4].Y * DOWNSCALE);
	Vertices[5] = b2Vec2(points[5].X * DOWNSCALE, points[5].Y * DOWNSCALE);
	Vertices[6] = b2Vec2(points[6].X * DOWNSCALE, points[6].Y * DOWNSCALE);
	Vertices[7] = b2Vec2(points[7].X * DOWNSCALE, points[7].Y * DOWNSCALE);
	Polygon.Set(Vertices.data(), 8);
	Polygon.m_centroid = b2Vec2(Origin.X * DOWNSCALE, Origin.Y * DOWNSCALE);
	b2MassData Data;
	Data.mass = Desc.Mass;
	Data.center.x = Origin.X * DOWNSCALE;
	Data.center.y = Origin.Y * DOWNSCALE;
	Polygon.ComputeMass(&Data, Fix.density);
	Fix.shape = &Polygon;
	b2Fixture* Fixture = Body->CreateFixture(&Fix);
	auto RigidBody2D = sBox2DRigidBody::Create(Owner, this, Body, ERigidBodyShape::Circle2D, Desc);
	//RigidBodys.push_back(RigidBody2D.get());
	return RigidBody2D;
}
IRigidBody::SharedPtr sWorld2D::Create2DCircleBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, float InRadius)
{
	b2BodyDef Def;
	switch (Desc.RigidBodyType)
	{
	case ERigidBodyType::Static:
		Def.type = b2BodyType::b2_staticBody;
		break;
	case ERigidBodyType::Kinematic:
		Def.type = b2BodyType::b2_kinematicBody;
		break;
	case ERigidBodyType::Dynamic:
		Def.type = b2BodyType::b2_dynamicBody;
		break;
	}
	Def.position = b2Vec2(Origin.X * DOWNSCALE, Origin.Y * DOWNSCALE);
	b2Body* Body = m_world->CreateBody(&Def);
	b2FixtureDef Fix;
	Fix.density = 1.0f;
	Fix.friction = Desc.Friction;
	Fix.restitution = Desc.Restitution;
	Fix.restitutionThreshold = 0.0f;
	Fix.userData.pointer = (uintptr_t)Desc.UserPointer;
	b2CircleShape Circle;
	Circle.m_radius = InRadius * DOWNSCALE;
	Circle.m_p = b2Vec2(Origin.X * DOWNSCALE, Origin.Y * DOWNSCALE);
	b2MassData Data;
	Data.mass = Desc.Mass;
	Data.center.x = Origin.X * DOWNSCALE;
	Data.center.y = Origin.Y * DOWNSCALE;
	Circle.ComputeMass(&Data, Fix.density);
	Fix.shape = &Circle;
	b2Fixture* Fixture = Body->CreateFixture(&Fix);
	auto RigidBody2D = sBox2DRigidBody::Create(Owner, this, Body, ERigidBodyShape::Circle2D, Desc);
	//RigidBodys.push_back(RigidBody2D.get());
	return RigidBody2D;
}
IRigidBody::SharedPtr sWorld2D::Create2DEdgeBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, const std::array<FVector2, 4>& points, bool OneSided)
{
	b2BodyDef Def;
	switch (Desc.RigidBodyType)
	{
	case ERigidBodyType::Static:
		Def.type = b2BodyType::b2_staticBody;
		break;
	case ERigidBodyType::Kinematic:
		Def.type = b2BodyType::b2_kinematicBody;
		break;
	case ERigidBodyType::Dynamic:
		Def.type = b2BodyType::b2_dynamicBody;
		break;
	}
	Def.position = b2Vec2(Origin.X * DOWNSCALE, Origin.Y * DOWNSCALE);
	b2Body* Body = m_world->CreateBody(&Def);
	b2FixtureDef Fix;
	Fix.density = 1.0f;
	Fix.friction = Desc.Friction;
	Fix.restitution = Desc.Restitution;
	Fix.restitutionThreshold = 0.0f;
	Fix.userData.pointer = (uintptr_t)Desc.UserPointer;
	b2EdgeShape Edge;
	Edge.m_oneSided = OneSided;
	if (OneSided)
		Edge.SetOneSided(b2Vec2(points[0].X * DOWNSCALE, points[0].Y * DOWNSCALE), b2Vec2(points[1].X * DOWNSCALE, points[1].Y * DOWNSCALE), b2Vec2(points[2].X * DOWNSCALE, points[2].Y * DOWNSCALE), b2Vec2(points[3].X * DOWNSCALE, points[3].Y * DOWNSCALE));
	else
		Edge.SetTwoSided(b2Vec2(points[0].X * DOWNSCALE, points[0].Y * DOWNSCALE), b2Vec2(points[1].X * DOWNSCALE, points[1].Y * DOWNSCALE));
	b2MassData Data;
	Data.mass = Desc.Mass;
	Data.center.x = Origin.X * DOWNSCALE;
	Data.center.y = Origin.Y * DOWNSCALE;
	Edge.ComputeMass(&Data, Fix.density);
	Fix.shape = &Edge;
	b2Fixture* Fixture = Body->CreateFixture(&Fix);
	auto RigidBody2D = sBox2DRigidBody::Create(Owner, this, Body, ERigidBodyShape::Circle2D, Desc);
	//RigidBodys.push_back(RigidBody2D.get());
	return RigidBody2D;
}
IRigidBody::SharedPtr sWorld2D::Create2DChainBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, const std::vector<FVector2>& vertices)
{
	std::vector<b2Vec2> b2Vertices;
	for (const auto& vert : vertices)
		b2Vertices.push_back(b2Vec2(vert.X * DOWNSCALE, vert.Y * DOWNSCALE));

	b2BodyDef Def;
	switch (Desc.RigidBodyType)
	{
	case ERigidBodyType::Static:
		Def.type = b2BodyType::b2_staticBody;
		break;
	case ERigidBodyType::Kinematic:
		Def.type = b2BodyType::b2_kinematicBody;
		break;
	case ERigidBodyType::Dynamic:
		Def.type = b2BodyType::b2_dynamicBody;
		break;
	}
	Def.position = b2Vec2(Origin.X * DOWNSCALE, Origin.Y * DOWNSCALE);
	b2Body* Body = m_world->CreateBody(&Def);
	b2FixtureDef Fix;
	Fix.density = 1.0f;
	Fix.friction = Desc.Friction;
	Fix.restitution = Desc.Restitution;
	Fix.restitutionThreshold = 0.0f;
	Fix.userData.pointer = (uintptr_t)Desc.UserPointer;
	b2ChainShape Chain;
	Chain.CreateLoop(b2Vertices.data(), (std::int32_t)b2Vertices.size());
	b2MassData Data;
	Data.mass = Desc.Mass;
	Data.center.x = Origin.X * DOWNSCALE;
	Data.center.y = Origin.Y * DOWNSCALE;
	Chain.ComputeMass(&Data, Fix.density);
	Fix.shape = &Chain;
	b2Fixture* Fixture = Body->CreateFixture(&Fix);
	auto RigidBody2D = sBox2DRigidBody::Create(Owner, this, Body, ERigidBodyShape::Circle2D, Desc);
	//RigidBodys.push_back(RigidBody2D.get());
	return RigidBody2D;
}
IRigidBody::SharedPtr sWorld2D::Create2DChainBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, const std::vector<FVector2>& vertices, const FVector2& prevVertex, const FVector2& nextVertex)
{
	std::vector<b2Vec2> b2Vertices;
	for (const auto& vert : vertices)
		b2Vertices.push_back(b2Vec2(vert.X * DOWNSCALE, vert.Y * DOWNSCALE));

	b2BodyDef Def;
	switch (Desc.RigidBodyType)
	{
	case ERigidBodyType::Static:
		Def.type = b2BodyType::b2_staticBody;
		break;
	case ERigidBodyType::Kinematic:
		Def.type = b2BodyType::b2_kinematicBody;
		break;
	case ERigidBodyType::Dynamic:
		Def.type = b2BodyType::b2_dynamicBody;
		break;
	}
	Def.position = b2Vec2(Origin.X * DOWNSCALE, Origin.Y * DOWNSCALE);
	b2Body* Body = m_world->CreateBody(&Def);
	b2FixtureDef Fix;
	Fix.density = 1.0f;
	Fix.friction = Desc.Friction;
	Fix.restitution = Desc.Restitution;
	Fix.restitutionThreshold = 0.0f;
	Fix.userData.pointer = (uintptr_t)Desc.UserPointer;
	b2ChainShape Chain;
	Chain.CreateChain(b2Vertices.data(), (std::int32_t)b2Vertices.size(), b2Vec2(prevVertex.X * DOWNSCALE, prevVertex.Y * DOWNSCALE), b2Vec2(nextVertex.X * DOWNSCALE, nextVertex.Y * DOWNSCALE));
	b2MassData Data;
	Data.mass = Desc.Mass;
	Data.center.x = Origin.X * DOWNSCALE;
	Data.center.y = Origin.Y * DOWNSCALE;
	Chain.ComputeMass(&Data, Fix.density);
	Fix.shape = &Chain;
	b2Fixture* Fixture = Body->CreateFixture(&Fix);
	auto RigidBody2D = sBox2DRigidBody::Create(Owner, this, Body, ERigidBodyShape::Circle2D, Desc);
	//RigidBodys.push_back(RigidBody2D.get());
	return RigidBody2D;
}

IRigidBody::SharedPtr sWorld2D::CreateBoxBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, FVector InHalf)
{
	return nullptr;
}
IRigidBody::SharedPtr sWorld2D::CreateSphereBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, float InRadius)
{
	return nullptr;
}
IRigidBody::SharedPtr sWorld2D::CreateCapsuleBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, float InRadius, float InHeight)
{
	return nullptr;
}
IRigidBody::SharedPtr sWorld2D::CreateCylinderBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, float InRadius, float InHeight)
{
	return nullptr;
}
IRigidBody::SharedPtr sWorld2D::CreateConeBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, float InRadius, float InHeight)
{
	return nullptr;
}

IRigidBody::SharedPtr sWorld2D::CreateMultiBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, FVector InInertia, float InMass)
{
	return nullptr;
}
IRigidBody::SharedPtr sWorld2D::CreateConvexHullBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, const float* points, int numPoints, int stride)
{
	return nullptr;
}
IRigidBody::SharedPtr sWorld2D::CreateTriangleMesh(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, const FVector* points, int numPoints, const std::uint32_t* indices, int numIndices)
{
	return nullptr;
}
