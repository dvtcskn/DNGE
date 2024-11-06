#pragma once

#include <assimp/scene.h>

#include "Engine/AbstractEngine.h"
#include "Core/Math/CoreMath.h"
#include <map>

enum class EModelType 
{
	FBX = 0x0,
	OBJ = 0x1,
	GLTF = 0x2,
};

enum class EImportType
{
	Single = 0x0,
	Instanced = 0x1,
};

class ModelImporter 
{
public:
	struct ModelImportAttributes
	{
		struct Material
		{
			std::string MaterialName = "";
			std::map<std::size_t, std::string> TexturePaths;

			FColor diffuse;
			FColor specular;
			FColor ambient;
			FColor emission;
			float shininess;
			float shininess_strength;
			int two_sided;
		};

		std::string	ScenePath = "";
		std::string	SceneName = "";

		std::string MeshName = "";
		std::string MaterialName = "";

		FBoundingBox MeshBounds;

		Material Mat;

		std::uint32_t MaterialIndex;
		sMeshData Data;
	};
public:
	ModelImporter() = default;
	~ModelImporter();

	/*
	* TO DO :
	* array veya tek olmak üzere ayrý ayrý import ettiðimiz fonksiyonlar yap
	*/

	std::vector<ModelImporter::ModelImportAttributes> ImportOBJ(const char* Path, const char* FileName, EImportType InImportType = EImportType::Single,
		bool MakeLeftHanded = true, std::uint32_t flags = 0);
	std::vector<ModelImporter::ModelImportAttributes> ImportGLTF(const char* Path, const char* FileName, EImportType InImportType = EImportType::Single,
		bool MakeLeftHanded = true, std::uint32_t flags = 0);
	std::vector<ModelImporter::ModelImportAttributes> ImportFBX(const char* Path, const char* FileName, EImportType InImportType = EImportType::Single,
		bool MakeLeftHanded = true, std::uint32_t flags = 0);
	
private:
	std::vector<ModelImportAttributes> InitMesh(EImportType InImportType = EImportType::Single, EModelType InModelType = EModelType::OBJ);

	std::vector<std::string> ReadMTL(const char* strfileName);
	void UpdateBounds();
	void Release();

	aiScene* pScene;

	std::vector<FBoundingBox> MeshBounds;
	FBoundingBox SceneBounds;

	std::string	m_ScenePath;
	std::string	m_SceneName;
};
