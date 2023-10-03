
#include "pch.h"
#include "DefaultLevel.h"
#include <Core/Math/CoreMath.h>
#include <algorithm>
#include <Core/MeshPrimitives.h>
#include <Gameplay/BoxCollision2DComponent.h>
#include <Gameplay/CircleCollision2DComponent.h>
#include <AbstractGI/MaterialManager.h>
#include <Utilities/tinyxml2.h>
#include <sstream>
#include "TrapActor.h"
#include "Items.h"

struct TileCollisionLayer
{
	int X = 0;
	int Y = 0;

	int Width = 0;
	int Height = 0;

	TileCollisionLayer(tinyxml2::XMLElement* textApproachElement)
	{
		X = std::stoi(textApproachElement->Attribute("x"));
		Y = std::stoi(textApproachElement->Attribute("y"));
		Width = std::stoi(textApproachElement->Attribute("width"));
		Height = std::stoi(textApproachElement->Attribute("height"));
	}
};

sDefaultLevel::sDefaultLevel(IWorld* pWorld, std::string InName)
	: Super()
	, World(pWorld)
	, Name(InName)
{
	Layers.push_back(std::make_unique<LevelLayer>());
	Layers.push_back(std::make_unique<LevelLayer>());
	Layers.push_back(std::make_unique<LevelLayer>());
	Layers.push_back(std::make_unique<LevelLayer>());
	Layers.push_back(std::make_unique<LevelLayer>());

	auto ScreenDimension = GPU::GetInternalBaseRenderResolution();

	PlayerSpawnLocations.push_back(sPlayerSpawn(0, FVector(75.0f, 250.0f, 0.0f), 3));

	{
		sMesh::SharedPtr Mesh = sMesh::Create("BackgroundMesh", EBasicMeshType::ePlane/*, FBoxDimension(1366, 768, 1)*/);
		Mesh->SetMaterial(sMaterialManager::Get().GetMaterialInstance("BackgoundMat", "BackgoundMatInstance"));
		Mesh->SetMeshTransform(FVector(ScreenDimension.Width / 2.0f, ScreenDimension.Height / 2.0f, 0.0f), FVector(640.0f, 360.0f, 1.0f), FVector4::Zero());
		AddMesh(Mesh, 0);
	}

	tinyxml2::XMLDocument doc;
	doc.LoadFile("..//Content//Pixel Adventure 1.tmx");
	tinyxml2::XMLElement* textApproachElement = doc.FirstChildElement("map")->FirstChildElement("layer")->FirstChildElement("data");
	const char* Text = textApproachElement->GetText();

	std::vector<int> Tiles;
	{
		std::istringstream ss(Text);
		int value;
		char comma;
		while (ss >> value)
		{
			Tiles.push_back(value);
			ss >> comma; // Read and discard the comma separator
		}

		for (int i = 0; i < 40; i++)
			Tiles.push_back(24);
	}

	std::vector<TileCollisionLayer> BlockCollisionLayer;
	std::vector<TileCollisionLayer> MoveableCollisionLayer;
	std::vector<TileCollisionLayer> ItemCollisionLayer;

	tinyxml2::XMLElement* Block_Objectgroup = doc.FirstChildElement("map")->FirstChildElement("objectgroup");
	{
		std::size_t ElementCounter = 0;
		for (auto child = Block_Objectgroup->FirstChildElement("object"); child; child = child->NextSiblingElement("object"))
			BlockCollisionLayer.push_back(child);
	}

	tinyxml2::XMLElement* Moveable_Objectgroup = Block_Objectgroup->NextSiblingElement("objectgroup");
	{
		std::size_t ElementCounter = 0;
		for (auto child = Moveable_Objectgroup->FirstChildElement("object"); child; child = child->NextSiblingElement("object"))
			MoveableCollisionLayer.push_back(child);

	}

	tinyxml2::XMLElement* Item_Objectgroup = Moveable_Objectgroup->NextSiblingElement("objectgroup");
	{
		std::size_t ElementCounter = 0;
		for (auto child = Item_Objectgroup->FirstChildElement("object"); child; child = child->NextSiblingElement("object"))
			ItemCollisionLayer.push_back(child);

	}

	{
		sActor::SharedPtr Terrain = sActor::Create("Terrain");
		sPrimitiveComponent::SharedPtr TerrainMeshParent = sPrimitiveComponent::Create("TerrainMeshParentComponent");
		Terrain->SetRootComponent(TerrainMeshParent);
		Terrain->SetLocation(FVector(0.0f, 0.0f, 0.0f));
		Terrain->SetRollPitchYaw(FAngles(0.0f));
		Terrain->AddToLevel(this, 2);
		//Terrain->SetEnabled(false);

		std::size_t X = 0;
		std::size_t Y = 0;
		for (const auto Tile : Tiles)
		{
			if (Tile == 0)
			{
				X += 16;
				if (X >= 640)
				{
					X = 0;
					Y += 16;
				}
				continue;
			}

			sMeshComponent::SharedPtr Mesh = sMeshComponent::Create("TerrainMesh_" + std::to_string(X) + "x" + std::to_string(Y), EBasicMeshType::ePlane);
			Mesh->SetMaterial(sMaterialManager::Get().GetMaterialInstance("DefaultTexturedMaterial", Tile - 1));
			Mesh->AttachToComponent(TerrainMeshParent.get());

			Mesh->SetRelativeScale(FVector(16.0f, 16.0f, 1.0f));
			Mesh->SetRelativeLocation(FVector(X + 8.0f, Y + 8.0f, 0.0f));
			
			/* Tile Based Collision */
			/*if (Tile != 24 && Tile != 192 && Tile != 214 && Tile != 236 && Tile != 189 && Tile != 190 && Tile != 191)
			{
				sRigidBodyDesc Desc;
				Desc.Friction = 1.0f;
				Desc.Mass = 1.0f;
				Desc.Restitution = 0.0f;
				Desc.RigidBodyType = ERigidBodyType::Static;

				sBoxCollision2DComponent::SharedPtr Terrain_BoxActorCollision = sBoxCollision2DComponent::Create("Terrain_BoxActorCollision", Desc, FDimension2D(16.0f, 16.0f));
				Terrain_BoxActorCollision->AttachToComponent(Mesh.get());
				if (Tile == 90 || Tile == 91 || Tile == 7 || Tile == 8 || Tile == 9 || Tile == 13 || Tile == 14 || Tile == 15
					|| Tile == 35 || Tile == 123 || Tile == 124 || Tile == 125 || Tile == 106 || Tile == 108
					|| Tile == 216 || Tile == 217 || Tile == 218 || Tile == 194 || Tile == 195 || Tile == 196)
					Terrain_BoxActorCollision->SetPrimaryTag("Jumpable_Ground");
				else
					Terrain_BoxActorCollision->SetPrimaryTag("Block");
			}*/

			X += 16;
			if (X >= 640)
			{
				X = 0;
				Y += 16;
			}
		}

		/* Simple Collision Level */
		sRigidBodyDesc Desc;
		Desc.Friction = 0.0f;
		Desc.Mass = 0.0f;
		Desc.Restitution = 0.0f;
		Desc.RigidBodyType = ERigidBodyType::Static;

		for (const auto& Layer : BlockCollisionLayer)
		{
			sBoxCollision2DComponent::SharedPtr Terrain_BoxActorCollision = sBoxCollision2DComponent::Create("Terrain_BoxActorCollision", Desc, FDimension2D((float)Layer.Width, (float)Layer.Height));
			Terrain_BoxActorCollision->AttachToComponent(TerrainMeshParent.get());
			Terrain_BoxActorCollision->AddTag("Block");
			Terrain_BoxActorCollision->SetRelativeLocation(FVector(Layer.X + Layer.Width / 2.0f, Layer.Y + Layer.Height / 2.0f, 0.0f));
		}

		for (const auto& Layer : MoveableCollisionLayer)
		{
			sBoxCollision2DComponent::SharedPtr Terrain_BoxActorCollision = sBoxCollision2DComponent::Create("Terrain_BoxActorCollision", Desc, FDimension2D((float)Layer.Width, (float)Layer.Height));
			Terrain_BoxActorCollision->AttachToComponent(TerrainMeshParent.get());
			Terrain_BoxActorCollision->AddTag("Jumpable_Ground");
			Terrain_BoxActorCollision->SetRelativeLocation(FVector(Layer.X + Layer.Width / 2.0f, Layer.Y + Layer.Height / 2.0f, 0.0f));
		}
	}

	Tiles.clear();
	BlockCollisionLayer.clear();
	MoveableCollisionLayer.clear();

	{
		GSawTrapActor::SharedPtr Saw = GSawTrapActor::Create("Saw1");
		Saw->AddToLevel(this, 4);
		Saw->SetRange(FVector2(440, 200), FVector2(440, 296));
	}
	{
		GSawTrapActor::SharedPtr Saw = GSawTrapActor::Create("Saw2");
		Saw->AddToLevel(this, 4);
		Saw->SetRange(FVector2(536, 296), FVector2(584, 296));
		Saw->SetSpeed(3.0f);
	}
	{
		GSawTrapActor::SharedPtr Saw = GSawTrapActor::Create("Saw3");
		Saw->AddToLevel(this, 1);
		Saw->SetRange(FVector2(160, 304), FVector2(288, 304));
	}
	{
		GSawTrapActor::SharedPtr Saw = GSawTrapActor::Create("Saw4");
		Saw->AddToLevel(this, 1);
		Saw->SetRange(FVector2(64, 160), FVector2(256, 160));
		Saw->SetSpeed(4.0f);
	}
	{
		GRockHeadActor::SharedPtr RockHead = GRockHeadActor::Create("RockHead");
		RockHead->AddToLevel(this, 1);
		RockHead->SetRange(FVector2(400, 224), FVector2(400, 96));
	}

	for (std::size_t i = 0; i < ItemCollisionLayer.size(); i++)
	{
		const auto& Item = ItemCollisionLayer.at(i);
		GItem::SharedPtr Apple = GItem::Create(i%2 ? "Cherries" : "Apple");
		Apple->AddToLevel(this, 4);
		Apple->SetLocation(FVector(Item.X - Item.Width / 2.0f, Item.Y + Item.Height / 2.0f, 0.0f));
	}

	ItemCollisionLayer.clear();
}

sDefaultLevel::~sDefaultLevel()
{
	for (auto& Layer : Layers)
	{
		Layer = nullptr;
	}
	Layers.clear();
	World = nullptr;
}

std::string sDefaultLevel::GetName() const
{
	return Name;
}

void sDefaultLevel::BeginPlay()
{
	for (const auto& Layer : Layers)
		Layer->BeginPlay();
}

void sDefaultLevel::Tick(const double DeltaTime)
{
	for (const auto& Layer : Layers)
		Layer->Tick(DeltaTime);
}

void sDefaultLevel::FixedUpdate(const double DeltaTime)
{
	for (const auto& Layer : Layers)
		Layer->FixedUpdate(DeltaTime);

	auto ScreenDimensi�n = GPU::GetInternalBaseRenderResolution();
	std::vector<sActor*> ActorsToRemove;
	std::size_t Count = ActorCount(2);
	for (std::size_t i = 0; i < Count; i++)
	{
		sActor* Actor = GetActor(i, 2);
		FVector Loc = Actor->GetLocation();

		if (Loc.X < -500.0f || Loc.Y < -500.0f ||
			Loc.X > ScreenDimensi�n.Width + 500.0f || Loc.Y > ScreenDimensi�n.Height + 500.0f)
		{
			ActorsToRemove.push_back(Actor);
		}
	}

	for (auto& Actor : ActorsToRemove)
		RemoveActor(Actor, Actor->GetLayerIndex());
	ActorsToRemove.clear();
}

void sDefaultLevel::InitLevel()
{
}

FBoundingBox sDefaultLevel::GetLevelBounds() const
{
	return FBoundingBox(FVector(0.0f, 0.0f, 0.0f), FVector(640.0f, 360.0f, 0.0f));
}

void sDefaultLevel::Serialize()
{
}

void sDefaultLevel::AddMesh(const std::shared_ptr<sMesh>& Mesh, std::size_t LayerIndex)
{
	if (Layers.size() <= LayerIndex)
		return;

	if (Layers[LayerIndex])
		Layers[LayerIndex]->AddMesh(Mesh);
}

void sDefaultLevel::RemoveMesh(sMesh* Mesh, std::size_t LayerIndex)
{
	if (Layers.size() <= LayerIndex)
		return;

	if (Layers[LayerIndex])
		Layers[LayerIndex]->RemoveMesh(Mesh);
}

void sDefaultLevel::AddActor(const std::shared_ptr<sActor>& Actor, std::size_t LayerIndex)
{
	if (Layers.size() <= LayerIndex)
		return;

	if (Layers[LayerIndex])
		Layers[LayerIndex]->AddActor(Actor);
}

void sDefaultLevel::RemoveActor(sActor* Actor, std::size_t LayerIndex, bool bDeferredRemove)
{
	if (Layers.size() <= LayerIndex)
		return;

	if (Layers[LayerIndex])
		Layers[LayerIndex]->RemoveActor(Actor, bDeferredRemove);
}

void sDefaultLevel::SpawnPlayerActor(const std::shared_ptr<sActor>& Actor, std::size_t PlayerIndex)
{
	for (const auto& Spawn : PlayerSpawnLocations)
	{
		if (Spawn.PlayerIndex == PlayerIndex)
		{
			Actor->SetLocation(Spawn.Location);
			Actor->AddToLevel(this, Spawn.LayerIndex);
			return;
		}
	}

	Actor->AddToLevel(this, 2);
}

size_t sDefaultLevel::LayerCount() const
{
	return Layers.size();
}

size_t sDefaultLevel::MeshCount(std::size_t LayerIndex) const
{
	if (Layers.size() <= LayerIndex)
		return 0;

	return Layers[LayerIndex]->MeshCount();
}

std::vector<std::shared_ptr<sMesh>> sDefaultLevel::GetAllMeshes(std::size_t LayerIndex) const
{
	if (Layers.size() <= LayerIndex)
		return std::vector<std::shared_ptr<sMesh>>();

	return Layers[LayerIndex]->Meshes;
}

sMesh* sDefaultLevel::GetMesh(const std::size_t Index, std::size_t LayerIndex) const
{
	if (Layers.size() <= LayerIndex)
		return nullptr;

	return Layers[LayerIndex]->GetMesh(Index);
}

size_t sDefaultLevel::ActorCount(std::size_t LayerIndex) const
{
	if (Layers.size() <= LayerIndex)
		return 0;

	return Layers[LayerIndex]->ActorCount();
}

std::vector<std::shared_ptr<sActor>> sDefaultLevel::GetAllActors(std::size_t LayerIndex) const
{
	if (Layers.size() <= LayerIndex)
		return std::vector<std::shared_ptr<sActor>>();

	return Layers[LayerIndex]->Actors;
}

sActor* sDefaultLevel::GetActor(const std::size_t Index, std::size_t LayerIndex) const
{
	if (Layers.size() <= LayerIndex)
		return nullptr;

	return Layers[LayerIndex]->GetActor(Index);
}

sActor* sDefaultLevel::GetPlayerFocusedActor(std::size_t Index) const
{
	return World->GetPlayerFocusedActor(Index);
}

void sDefaultLevel::OnResizeWindow(const std::size_t Width, const std::size_t Height)
{

}

void sDefaultLevel::Reset()
{
	Layers[1]->Release();
	Layers[3]->Release();
	Layers[4]->Release();

	{
		GSawTrapActor::SharedPtr Saw = GSawTrapActor::Create("Saw1");
		Saw->AddToLevel(this, 4);
		Saw->SetRange(FVector2(440, 200), FVector2(440, 296));
	}
	{
		GSawTrapActor::SharedPtr Saw = GSawTrapActor::Create("Saw2");
		Saw->AddToLevel(this, 4);
		Saw->SetRange(FVector2(536, 296), FVector2(584, 296));
		Saw->SetSpeed(3.0f);
	}
	{
		GSawTrapActor::SharedPtr Saw = GSawTrapActor::Create("Saw3");
		Saw->AddToLevel(this, 1);
		Saw->SetRange(FVector2(160, 304), FVector2(288, 304));
	}
	{
		GSawTrapActor::SharedPtr Saw = GSawTrapActor::Create("Saw4");
		Saw->AddToLevel(this, 1);
		Saw->SetRange(FVector2(64, 160), FVector2(256, 160));
		Saw->SetSpeed(4.0f);
	}
	{
		GRockHeadActor::SharedPtr RockHead = GRockHeadActor::Create("RockHead");
		RockHead->AddToLevel(this, 1);
		RockHead->SetRange(FVector2(400, 224), FVector2(400, 96));
	}

	std::vector<TileCollisionLayer> ItemCollisionLayer;

	tinyxml2::XMLDocument doc;
	doc.LoadFile("..//Content//Pixel Adventure 1.tmx");
	tinyxml2::XMLElement* Block_Objectgroup = doc.FirstChildElement("map")->FirstChildElement("objectgroup");
	tinyxml2::XMLElement* Moveable_Objectgroup = Block_Objectgroup->NextSiblingElement("objectgroup");
	tinyxml2::XMLElement* Item_Objectgroup = Moveable_Objectgroup->NextSiblingElement("objectgroup");
	{
		std::size_t ElementCounter = 0;
		for (auto child = Item_Objectgroup->FirstChildElement("object"); child; child = child->NextSiblingElement("object"))
			ItemCollisionLayer.push_back(child);

	}
	for (std::size_t i = 0; i < ItemCollisionLayer.size(); i++)
	{
		const auto& Item = ItemCollisionLayer.at(i);
		GItem::SharedPtr Apple = GItem::Create(i % 2 ? "Cherries" : "Apple");
		Apple->AddToLevel(this, 4);
		Apple->SetLocation(FVector(Item.X - Item.Width / 2.0f, Item.Y + Item.Height / 2.0f, 0.0f));
	}

	ItemCollisionLayer.clear();
}

void sDefaultLevel::InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar)
{
	for (const auto& Layer : Layers)
		Layer->InputProcess(MouseInput, KeyboardChar);
}
