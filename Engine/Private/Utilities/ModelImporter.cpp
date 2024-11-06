
#include "pch.h"
#include "Utilities/ModelImporter.h"

#include <assimp/material.h>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>

#include <fstream>
#include <sstream>
#include <map>

#pragma comment (lib, "assimp-vc143-mt.lib")

void color4_to_float4(const C_STRUCT aiColor4D* c, FColor f)
{
	f.R = c->r;
	f.G = c->g;
	f.B = c->b;
	f.A = c->a;
}

ModelImporter::~ModelImporter()
{
	Release();
}

void ModelImporter::Release()
{
	aiReleaseImport(pScene);
	pScene = NULL;
	m_ScenePath = "";
	m_SceneName = "";
}

std::vector<ModelImporter::ModelImportAttributes> ModelImporter::ImportOBJ(const char * Path, const char * FileName, EImportType InImportType, bool MakeLeftHanded, std::uint32_t flags)
{
	flags |= aiProcess_Triangulate;
	flags |= aiProcess_CalcTangentSpace;
	flags |= aiProcess_FindDegenerates;
	flags |= aiProcess_FindInstances;
	flags |= aiProcess_FindInvalidData;
	flags |= aiProcess_GenNormals;
	flags |= aiProcess_GenSmoothNormals;
	flags |= aiProcess_GenUVCoords;
	flags |= aiProcess_ImproveCacheLocality;
	//flags |= aiProcess_OptimizeGraph;
	flags |= aiProcess_OptimizeMeshes;

	if (MakeLeftHanded)
	{
		//flags |= aiProcess_ConvertToLeftHanded;
		flags |= aiProcess_MakeLeftHanded;
		flags |= aiProcess_FlipUVs;
		flags |= aiProcess_FlipWindingOrder;
	}

	Release();

	pScene = (aiScene*)aiImportFileEx(Path, flags, 0);

	if (!pScene)
	{
		char buf[1024];
		sprintf_s(buf, "unable to load scene file `%s`: %s\n", Path, aiGetErrorString());
		Engine::WriteToConsole(buf);
		return std::vector<ModelImporter::ModelImportAttributes>();
	}

	m_ScenePath = Path;
	m_SceneName = FileName;

	std::uint32_t facesNum = NULL;
	std::uint32_t verticesNum = NULL;

	for (std::uint32_t i = 0; i < pScene->mNumMeshes; ++i)
	{
		facesNum += pScene->mMeshes[i]->mNumFaces;
		verticesNum += pScene->mMeshes[i]->mNumVertices;
	}

	std::vector<std::string> MTLData;
	MTLData = ReadMTL(Path);

	UpdateBounds();
	return InitMesh(InImportType, EModelType::OBJ);
}

std::vector<ModelImporter::ModelImportAttributes> ModelImporter::ImportGLTF(const char* Path, const char* FileName, EImportType InImportType, bool MakeLeftHanded, std::uint32_t flags)
{

	flags |= aiProcess_Triangulate;
	flags |= aiProcess_CalcTangentSpace;
	flags |= aiProcess_FindDegenerates;
	flags |= aiProcess_FindInstances;
	flags |= aiProcess_FindInvalidData;
	flags |= aiProcess_GenNormals;
	flags |= aiProcess_GenSmoothNormals;
	flags |= aiProcess_GenUVCoords;
	flags |= aiProcess_ImproveCacheLocality;
	//flags |= aiProcess_OptimizeGraph;
	flags |= aiProcess_OptimizeMeshes;

	if (MakeLeftHanded)
	{
		//flags |= aiProcess_ConvertToLeftHanded;
		flags |= aiProcess_MakeLeftHanded;
		flags |= aiProcess_FlipUVs;
		flags |= aiProcess_FlipWindingOrder;
	}

	Release();

	pScene = (aiScene*)aiImportFileEx(Path, flags, 0);

	if (!pScene)
	{
		char buf[1024];
		sprintf_s(buf, "unable to load scene file `%s`: %s\n", Path, aiGetErrorString());
		Engine::WriteToConsole(buf);
		return std::vector<ModelImporter::ModelImportAttributes>();
	}
	
	m_ScenePath = Path;
	m_SceneName = FileName;

	std::uint32_t facesNum = NULL;
	std::uint32_t verticesNum = NULL;

	for (std::uint32_t i = 0; i < pScene->mNumMeshes; ++i)
	{
		facesNum += pScene->mMeshes[i]->mNumFaces;
		verticesNum += pScene->mMeshes[i]->mNumVertices;
	}

	UpdateBounds(); 
	std::vector<ModelImporter::ModelImportAttributes> Meshes = InitMesh(InImportType, EModelType::OBJ);

	std::vector<ModelImportAttributes::Material> Mats;
	if (pScene->HasMaterials())
	{
		for (std::size_t i = 0; i < pScene->mNumMaterials; i++)
		{
			auto Mat = pScene->mMaterials[i];
			if (Mat)
			{
				ModelImportAttributes::Material Material;
				Material.MaterialName = std::string(Mat->GetName().C_Str());
				//aiString name;
				//Mat->Get(AI_MATKEY_NAME, name);

				if (Material.MaterialName.empty())
				{
					Material.MaterialName = m_SceneName + "_Materials_" + std::to_string(i);
				}

				std::vector<std::string> V_Paths;
				for (std::size_t Type = 0; Type < aiTextureType_UNKNOWN; Type++)
				{
					aiString str;
					Mat->Get(AI_MATKEY_TEXTURE((aiTextureType)Type, 0), str);
					std::string Path = std::string(str.C_Str());
					//auto Count = Mat->GetTextureCount((aiTextureType)Type);
					if (str.length > 0 && std::find(V_Paths.begin(), V_Paths.end(), Path) == V_Paths.end())
					{
						V_Paths.push_back(Path);
						Material.TexturePaths.insert({ (aiTextureType)Type, Path });
					}
				}

				int ret1, ret2;
				unsigned int max;
				C_STRUCT aiColor4D diffuse;
				C_STRUCT aiColor4D specular;
				C_STRUCT aiColor4D ambient;
				C_STRUCT aiColor4D emission;

				if (AI_SUCCESS == aiGetMaterialColor(Mat, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
					color4_to_float4(&diffuse, Material.diffuse);

				if (AI_SUCCESS == aiGetMaterialColor(Mat, AI_MATKEY_COLOR_SPECULAR, &specular))
					color4_to_float4(&specular, Material.specular);

				if (AI_SUCCESS == aiGetMaterialColor(Mat, AI_MATKEY_COLOR_AMBIENT, &ambient))
					color4_to_float4(&ambient, Material.ambient);

				if (AI_SUCCESS == aiGetMaterialColor(Mat, AI_MATKEY_COLOR_EMISSIVE, &emission))
					color4_to_float4(&emission, Material.emission);

				max = 1;
				ret1 = aiGetMaterialFloatArray(Mat, AI_MATKEY_SHININESS, &Material.shininess, &max);
				if (ret1 == AI_SUCCESS) {
					max = 1;
					ret2 = aiGetMaterialFloatArray(Mat, AI_MATKEY_SHININESS_STRENGTH, &Material.shininess_strength, &max);
				}
				max = 1;
				if ((AI_SUCCESS == aiGetMaterialIntegerArray(Mat, AI_MATKEY_TWOSIDED, &Material.two_sided, &max)) && Material.two_sided)
				{

				}
				
				Mats.push_back(Material);
			}
		}
	}

	for (auto& Mesh : Meshes)
	{
		Mesh.Mat = Mats[Mesh.MaterialIndex];
	}

	return Meshes;
}

std::vector<ModelImporter::ModelImportAttributes> ModelImporter::ImportFBX(const char * Path, const char * FileName, EImportType InImportType, bool MakeLeftHanded, std::uint32_t flags)
{
	flags |= aiProcess_Triangulate;
	flags |= aiProcess_CalcTangentSpace;
	flags |= aiProcess_FindDegenerates;
	flags |= aiProcess_FindInstances;
	flags |= aiProcess_FindInvalidData;
	flags |= aiProcess_GenNormals;
	flags |= aiProcess_GenSmoothNormals;
	flags |= aiProcess_GenUVCoords;
	flags |= aiProcess_ImproveCacheLocality;
	//flags |= aiProcess_OptimizeGraph;
	flags |= aiProcess_OptimizeMeshes;

	if (MakeLeftHanded)
	{
		//flags |= aiProcess_ConvertToLeftHanded;
		flags |= aiProcess_MakeLeftHanded;
		flags |= aiProcess_FlipUVs;
		flags |= aiProcess_FlipWindingOrder;
	}

	Release();

	pScene = (aiScene*)aiImportFileEx(Path, flags, 0);

	if (!pScene)
	{
		char buf[1024];
		sprintf_s(buf, "unable to load scene file `%s`: %s\n", Path, aiGetErrorString());
		Engine::WriteToConsole(buf);
		return std::vector<ModelImporter::ModelImportAttributes>();
	}

	m_ScenePath = Path;
	m_SceneName = FileName;

	std::uint32_t facesNum = NULL;
	std::uint32_t verticesNum = NULL;

	for (std::uint32_t i = 0; i < pScene->mNumMeshes; ++i)
	{
		facesNum += pScene->mMeshes[i]->mNumFaces;
		verticesNum += pScene->mMeshes[i]->mNumVertices;
	}

	std::vector<std::string> MTLData;

	UpdateBounds();

	return InitMesh(InImportType, EModelType::FBX);
}

std::vector<std::string> ModelImporter::ReadMTL(const char * strfileName)
{
	std::string FileName = strfileName;
	FileName.erase(FileName.end() - 3, FileName.end());
	FileName.append("mtl");

	std::vector<std::string> MTLData;

	std::ifstream file(FileName);
	std::string temp;
	while (std::getline(file, temp))
	{
		if (temp.find("newmtl") == 0) {
			temp.erase(temp.begin(), temp.begin() + 7);
			MTLData.push_back(temp);
		}
	}

	return MTLData;
}

void ModelImporter::UpdateBounds()
{
	assert(pScene != NULL);

	const float maxFloat = 3.402823466e+38F;
	DirectX::XMFLOAT3 _minBoundary(maxFloat, maxFloat, maxFloat);
	DirectX::XMFLOAT3 _maxBoundary(-maxFloat, -maxFloat, -maxFloat);

	if (pScene->mNumMeshes)
	{
		MeshBounds.resize(pScene->mNumMeshes);

		SceneBounds.Min = _minBoundary;
		SceneBounds.Max = _maxBoundary;

		for (std::size_t i = 0; i < pScene->mNumMeshes; ++i)
		{
			DirectX::XMFLOAT3 minBoundary = _minBoundary;
			DirectX::XMFLOAT3 maxBoundary = _maxBoundary;

			for (std::size_t v = 0; v < pScene->mMeshes[i]->mNumVertices; ++v)
			{
				minBoundary.x = __min(minBoundary.x, pScene->mMeshes[i]->mVertices[v].x);
				minBoundary.y = __min(minBoundary.y, pScene->mMeshes[i]->mVertices[v].y);
				minBoundary.z = __min(minBoundary.z, pScene->mMeshes[i]->mVertices[v].z);

				maxBoundary.x = __max(maxBoundary.x, pScene->mMeshes[i]->mVertices[v].x);
				maxBoundary.y = __max(maxBoundary.y, pScene->mMeshes[i]->mVertices[v].y);
				maxBoundary.z = __max(maxBoundary.z, pScene->mMeshes[i]->mVertices[v].z);
			}

			MeshBounds[i].Min = minBoundary;
			MeshBounds[i].Max = maxBoundary;

			SceneBounds.Min.X = __min(SceneBounds.Min.X, minBoundary.x);
			SceneBounds.Min.Y = __min(SceneBounds.Min.Y, minBoundary.y);
			SceneBounds.Min.Z = __min(SceneBounds.Min.Z, minBoundary.z);

			SceneBounds.Max.X = __max(SceneBounds.Max.X, maxBoundary.x);
			SceneBounds.Max.Y = __max(SceneBounds.Max.Y, maxBoundary.y);
			SceneBounds.Max.Z = __max(SceneBounds.Max.Z, maxBoundary.z);
		}
	}
}

std::vector<ModelImporter::ModelImportAttributes> ModelImporter::InitMesh(EImportType InImportType, EModelType InModelType)
{
	if (!pScene)
		return std::vector<ModelImportAttributes>();

	std::vector<ModelImportAttributes> MAs;

	if (pScene->mNumMeshes)
	{
		std::vector<std::pair<std::uint32_t, std::uint32_t>> meshMaterials(pScene->mNumMeshes);

		for (std::uint32_t sceneMesh = 0; sceneMesh < pScene->mNumMeshes; ++sceneMesh)
		{
			meshMaterials[sceneMesh] = std::pair<std::uint32_t, std::uint32_t>(sceneMesh, pScene->mMeshes[sceneMesh]->mMaterialIndex);
		}

		std::sort(meshMaterials.begin(), meshMaterials.end(), [](std::pair<std::uint32_t, std::uint32_t> a, std::pair<std::uint32_t, std::uint32_t> b) { return a.second < b.second; });
		
		// Copy data into buffer images
		for (std::uint32_t meshID = 0; meshID < pScene->mNumMeshes; ++meshID)
		{
			std::uint32_t sceneMesh = meshMaterials[meshID].first;
			std::uint32_t indexOffset = 0;
			std::uint32_t vertexOffset = 0;

			std::vector<std::uint32_t> TotalIndices(static_cast<size_t>(pScene->mMeshes[sceneMesh]->mNumFaces) * static_cast<size_t>(3));
			std::vector<sVertexLayout> TotalVertices(pScene->mMeshes[sceneMesh]->mNumVertices);

			const aiMesh* pMesh = pScene->mMeshes[sceneMesh];
			std::uint32_t numVertices = pMesh->mNumVertices;

			// Indices
			for (std::uint32_t f = 0; f < pMesh->mNumFaces; ++f)
			{
				memcpy(&TotalIndices[static_cast<size_t>(indexOffset) + static_cast<size_t>(f) * static_cast<size_t>(3)], pMesh->mFaces[f].mIndices, sizeof(int) * 3);
				//memcpy(&TotalIndices[ f > 4 ? indexOffset + (f - 2) * 3 : indexOffset + f * 3 ], pMesh->mFaces[ f > 4 ? f - 2 : f ].mIndices, sizeof(int) * 3);
			}

			for (std::uint32_t v = 0; v < numVertices; v++)
			{
				sVertexLayout& vertex = TotalVertices[static_cast<size_t>(v) + static_cast<size_t>(vertexOffset)];

				vertex.position = (DirectX::XMFLOAT3&)pMesh->mVertices[v];

				if (pMesh->HasNormals())
				{
					vertex.normal = (DirectX::XMFLOAT3&)pMesh->mNormals[v];
				}

				if (pMesh->HasTangentsAndBitangents())
				{
					vertex.tangent = (DirectX::XMFLOAT3&)pMesh->mTangents[v];
					vertex.binormal = (DirectX::XMFLOAT3&)pMesh->mBitangents[v];
				}

				if (pMesh->HasTextureCoords(0))
				{
					vertex.texCoord = (DirectX::XMFLOAT2&)aiVector2D(pMesh->mTextureCoords[0][v].x, pMesh->mTextureCoords[0][v].y);
				}
				vertex.ArrayIndex = 0;
			}

			ModelImportAttributes ptr;
			ptr.Data.Indices = TotalIndices;
			ptr.Data.Vertices = TotalVertices;

			ptr.MeshName = std::string(pScene->mMeshes[sceneMesh]->mName.C_Str()) + "_" + std::to_string(meshID);
			//if (InModelType == EModelType::OBJ)
			//{
			//	ptr.MaterialName = InMTLData.size() > 0 ? InMTLData[pScene->mMeshes[sceneMesh]->mMaterialIndex-1] : pScene->mMeshes[sceneMesh]->mName.C_Str();
			//	//ptr->MaterialName = InMTLData.size() > 0 ? InMTLData[meshMaterials[meshID].second - 1] : pScene->mMeshes[sceneMesh]->mName.C_Str();
			//}
			//else {
			//	ptr.MaterialName = pScene->mMeshes[sceneMesh]->mName.C_Str();
			//}

			ptr.MaterialName = pScene->mMeshes[sceneMesh]->mName.C_Str();
			ptr.MaterialIndex = pScene->mMeshes[sceneMesh]->mMaterialIndex;

			ptr.SceneName = m_SceneName;
			ptr.ScenePath = m_ScenePath;

			ptr.MeshBounds = MeshBounds[meshID];
			
			ptr.Data.DrawParameters.IndexCountPerInstance = pScene->mMeshes[sceneMesh]->mNumFaces * 3;
			ptr.Data.DrawParameters.InstanceCount = 1;

			ptr.Data.DrawParameters.StartIndexLocation = 0;
			ptr.Data.DrawParameters.BaseVertexLocation = 0;
			ptr.Data.DrawParameters.StartInstanceLocation = 0;

			if (InImportType == EImportType::Instanced)
			{
				ptr.Data.DrawParameters.StartIndexLocation = indexOffset;
				ptr.Data.DrawParameters.BaseVertexLocation = vertexOffset;
				ptr.Data.DrawParameters.StartInstanceLocation = meshID;
			}

			MAs.push_back(ptr);
		}
	}

	return MAs;
}
