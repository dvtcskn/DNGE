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

#include <map>
#include <vector>
#include <memory>
#include <string>
#include <functional>
#include "Utilities/FileManager.h"
#include "Engine/AbstractEngine.h"

#include <filesystem>
namespace fs = std::filesystem;

#include "DNGEAsset.h"

class ContentManager final
{
	sBaseClassBody(sClassNoDefaults, ContentManager)
private:
	ContentManager() = default;
	ContentManager(const ContentManager& Other) = delete;
	ContentManager& operator=(const ContentManager&) = delete;

private:
	std::map<std::string, MeshAsset> MeshAssets;
	std::map<std::string, MaterialAsset> MaterialAssets;
	std::map<std::string, MaterialInstanceAsset> MaterialInstanceAssets;
	std::map<std::string, MapAsset> MapAssets;

public:
	~ContentManager()
	{
		Release();
	}

	void Release()
	{
		MeshAssets.clear();
		MaterialAssets.clear();
		MaterialInstanceAssets.clear();
		MapAssets.clear();
	}

	static ContentManager& Get()
	{
		static ContentManager instance;
		return instance;
	}

public:
	MapAsset LoadMap(std::string InMapName)
	{
		//if (IsMapAssetExist(InMapName))
		//{
		//	return GetMapAsset(InMapName);
		//};

		//std::string path("..//Content//");
		//std::string MapExt(".DNGEMap");
		//for (auto& p : fs::recursive_directory_iterator(path))
		//{
		//	if (p.path().extension() == MapExt)
		//	{
		//		std::string Path = FileManager::WideStringToString(p.path().c_str());
		//		sArchive archive(Path);

		//		MapAsset Asset;
		//		archive >> Asset.Name;

		//		if (InMapName != Asset.Name)
		//			continue;

		//		archive >> Asset.Path;

		//		archive >> Asset.mMaterialData.MaterialSize;
		//		for (std::uint32_t i = 0; i < Asset.mMaterialData.MaterialSize; i++)
		//		{
		//			MapAsset::MapMaterialData::MaterialData Data;
		//			archive >> Data.Name;
		//			archive >> Data.Path;
		//			archive >> Data.InstanceSize;
		//			for (std::uint32_t ii = 0; ii < Data.InstanceSize; ii++)
		//			{
		//				MapAsset::MapMaterialData::MapMaterialInstanceData InstanceData;
		//				archive >> InstanceData.Name;
		//				archive >> InstanceData.Path;
		//				Data.Instances.push_back(InstanceData);
		//			}
		//			Asset.mMaterialData.Materials.push_back(Data);
		//		}

		//		archive >> Asset.mActorData.ActorSize;
		//		for (std::uint32_t i = 0; i < Asset.mActorData.ActorSize; i++)
		//		{
		//			MapAsset::MapActorData::ActorData Data;
		//			archive >> Data.Name;

		//			FVector4 ActorRotation;
		//			FVector ActorTranslation;
		//			FVector ActorScale3D;
		//			archive >> ActorRotation;
		//			archive >> ActorTranslation;
		//			archive >> ActorScale3D;
		//			Data.ActorTransform.SetRotation(ActorRotation);
		//			int TransformProperty = 0;
		//			archive >> TransformProperty;
		//			Data.ActorTransform.SetLocation(ActorTranslation);
		//			Data.ActorTransform.SetScale(ActorScale3D);

		//			archive >> Data.ComponentSize;
		//			for (std::uint32_t ii = 0; ii < Data.ComponentSize; ii++)
		//			{
		//				MapAsset::MapActorData::MapActorComponentData InstanceData;
		//				archive >> InstanceData.Name;

		//				FVector4 ComponentRotation;
		//				FVector ComponentTranslation;
		//				FVector ComponentScale3D;
		//				archive >> ComponentRotation;
		//				archive >> ComponentTranslation;
		//				archive >> ComponentScale3D;
		//				InstanceData.ComponentTransform.SetRotation(ComponentRotation);
		//				int ComponentTransformProperty = 0;
		//				archive >> ComponentTransformProperty;
		//				InstanceData.ComponentTransform.SetLocation(ComponentTranslation);
		//				InstanceData.ComponentTransform.SetScale(ComponentScale3D);

		//				//int Type = 0;
		//				//archive >> Type;
		//				////InstanceData.Type = static_cast<EComponentType>(Type);
		//				//int CreationType = 0;
		//				//archive >> CreationType;
		//				////InstanceData.CreationType = static_cast<EComponentCreationType>(CreationType);
		//				archive >> InstanceData.Path;
		//				Data.Components.push_back(InstanceData);
		//			}
		//			Asset.mActorData.Actors.push_back(Data);
		//		}

		//		Asset.mCameraAsset.Read(archive);
		//		MapAssets.insert({ Asset.Name, Asset });

		//		return Asset;
		//	}
		//};
		return MapAsset();
	};

	void LoadAllAssets()
	{
		std::string path("..//Content//");
		std::string ext(".DNGEAsset");
		std::string MapExt(".DNGEMap");
		//for (auto& p : fs::recursive_directory_iterator(path))
		//{
		//	if (p.path().extension() == ext)
		//	{
		//		std::string Path = FileManager::Get().WideStringToString(p.path().c_str());
		//		sArchive archive(Path);
		//		int Type;
		//		archive >> Type;

		//		switch (Type)
		//		{
		//		case 0:
		//		{
		//			MeshAsset Asset;
		//			Asset.Type = DNGEAssetType::Mesh;
		//			archive >> Asset.Name;
		//			archive >> Asset.Path;
		//			archive >> Asset.OffsetFromOrigin;
		//			archive >> Asset.AABB.Min;
		//			archive >> Asset.AABB.Max;
		//			archive >> Asset.DefaultMaterialName;
		//			archive >> Asset.DefaultMaterialPath;
		//			archive >> Asset.mMeshData.Indices;

		//			archive >> Asset.VerticesSize;

		//			for (std::uint32_t i = 0; i < Asset.VerticesSize; i++)
		//			{
		//				sVertexBufferEntry VBE;
		//				archive >> VBE.position;
		//				archive >> VBE.normal;
		//				archive >> VBE.texCoord;
		//				archive >> VBE.tangent;
		//				archive >> VBE.binormal;
		//				archive >> VBE.ArrayIndex;
		//				Asset.mMeshData.Vertices.push_back(VBE);
		//			}

		//			archive >> Asset.mMeshData.DrawParameters.IndexCountPerInstance;
		//			archive >> Asset.mMeshData.DrawParameters.InstanceCount;
		//			archive >> Asset.mMeshData.DrawParameters.StartIndexLocation;
		//			archive >> Asset.mMeshData.DrawParameters.BaseVertexLocation;
		//			archive >> Asset.mMeshData.DrawParameters.StartInstanceLocation;

		//			MeshAssets.insert({ Asset.Name, Asset });
		//		}
		//		break;
		//		case 1:
		//		{
		//			MaterialAsset Asset;

		//			Asset.Type = DNGEAssetType::Material;
		//			archive >> Asset.Name;
		//			archive >> Asset.Path;

		//			int MaterialType = 0;
		//			archive >> MaterialType;
		//			Asset.MaterialType = static_cast<EMaterialType>(MaterialType);
		//			int MaterialUsage = 0;
		//			archive >> MaterialUsage;
		//			Asset.MaterialUsage = static_cast<EMaterialUsage>(MaterialUsage);
		//			int BlendMode = 0;
		//			archive >> BlendMode;
		//			Asset.BlendMode = static_cast<EMaterialBlendMode>(BlendMode);

		//			archive >> Asset.mPipelineAsset.VertexLayoutSize;

		//			for (std::uint32_t i = 0; i < Asset.mPipelineAsset.VertexLayoutSize; i++)
		//			{
		//				sVertexAttributeDesc VBE;
		//				archive >> VBE.name;
		//				int format = 0;
		//				archive >> format;
		//				VBE.format = static_cast<EFormat>(format);
		//				archive >> VBE.bufferIndex;
		//				archive >> VBE.offset;
		//				archive >> VBE.isInstanced;
		//				Asset.mPipelineAsset.VertexLayout.push_back(VBE);
		//			}

		//			archive >> Asset.mPipelineAsset.DescriptorSetLayoutSize;

		//			for (std::uint32_t i = 0; i < Asset.mPipelineAsset.DescriptorSetLayoutSize; i++)
		//			{
		//				sDescriptorSetLayoutBinding DSL;
		//				unsigned char DescriptorType = 0;
		//				archive >> DescriptorType;
		//				DSL.DescriptorType = static_cast<EDescriptorType>(DescriptorType);
		//				unsigned char ShaderType = 0;
		//				archive >> ShaderType;
		//				DSL.ShaderType = static_cast<eShaderType>(ShaderType);
		//				archive >> DSL.Location;
		//				Asset.mPipelineAsset.DescriptorSetLayout.push_back(DSL);
		//			}

		//			archive >> Asset.mPipelineAsset.ShaderAttachmentsSize;

		//			for (std::uint32_t i = 0; i < Asset.mPipelineAsset.ShaderAttachmentsSize; i++)
		//			{
		//				sShaderAttachment SA;
		//				archive >> SA.Location;
		//				archive >> SA.FunctionName;
		//				unsigned char ShaderType = 0;
		//				archive >> ShaderType;
		//				SA.Type = static_cast<eShaderType>(ShaderType);
		//				Asset.mPipelineAsset.ShaderAttachments.push_back(SA);
		//			}

		//			archive >> Asset.mPipelineAsset.Name;
		//			archive >> Asset.mPipelineAsset.PipelineCBSize;

		//			/*std::map<eShaderType, PipelineDesc::ConstantBufferAttributes> ConstantBuffers;
		//			for (std::uint32_t i = 0; i < Asset.mPipelineAsset.PipelineCBSize; i++)
		//			{
		//				int slot = 0;
		//				std::string Name;
		//				eShaderType Type;

		//				unsigned char ShaderType = 0;
		//				archive >> ShaderType;
		//				Type = static_cast<eShaderType>(ShaderType);

		//				archive >> Name;
		//				archive >> slot;

		//				PipelineDesc::ConstantBufferAttributes CB(AResources::Get().GetConstantBuffer(Name), slot);
		//				ConstantBuffers.insert({ Type, CB });
		//				Asset.mPipelineAsset.pPipelineDesc.ConstantBuffers = ConstantBuffers;
		//			}*/

		//			{
		//				int FillMode = 0;
		//				archive >> FillMode;
		//				Asset.mPipelineAsset.pPipelineDesc.RasterizerAttribute.FillMode = static_cast<ERasterizerFillMode>(FillMode);
		//				int CullMode = 0;
		//				archive >> CullMode;
		//				Asset.mPipelineAsset.pPipelineDesc.RasterizerAttribute.CullMode = static_cast<ERasterizerCullMode>(CullMode);
		//				archive >> Asset.mPipelineAsset.pPipelineDesc.RasterizerAttribute.DepthBias;
		//				archive >> Asset.mPipelineAsset.pPipelineDesc.RasterizerAttribute.DepthBiasClamp;
		//				archive >> Asset.mPipelineAsset.pPipelineDesc.RasterizerAttribute.SlopeScaleDepthBias;
		//				archive >> Asset.mPipelineAsset.pPipelineDesc.RasterizerAttribute.DepthClipEnable;
		//				archive >> Asset.mPipelineAsset.pPipelineDesc.RasterizerAttribute.bAllowMSAA;
		//				archive >> Asset.mPipelineAsset.pPipelineDesc.RasterizerAttribute.bEnableLineAA;
		//				archive >> Asset.mPipelineAsset.pPipelineDesc.RasterizerAttribute.FrontCounterClockwise;
		//				int StateStatus = 0;
		//				archive >> StateStatus;
		//				Asset.mPipelineAsset.pPipelineDesc.RasterizerAttribute.StateStatus = static_cast<EStateStatus>(StateStatus);
		//			}
		//			// DepthStencilAttributeDesc
		//			{
		//				archive >> Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.bEnableDepthWrite;
		//				archive >> Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.bDepthWriteMask;
		//				int DepthTest = 0;
		//				archive >> DepthTest;
		//				Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.DepthTest = static_cast<ECompareFunction>(DepthTest);
		//				archive >> Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.bStencilEnable;

		//				archive >> Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.bEnableFrontFaceStencil;
		//				{
		//					int FrontFaceStencilTest = 0;
		//					archive >> FrontFaceStencilTest;
		//					Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.FrontFaceStencilTest = static_cast<ECompareFunction>(FrontFaceStencilTest);
		//					int FrontFaceStencilFailStencilOp = 0;
		//					archive >> FrontFaceStencilFailStencilOp;
		//					Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.FrontFaceStencilFailStencilOp = static_cast<EStencilOp>(FrontFaceStencilFailStencilOp);
		//					int FrontFaceDepthFailStencilOp = 0;
		//					archive >> FrontFaceDepthFailStencilOp;
		//					Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.FrontFaceDepthFailStencilOp = static_cast<EStencilOp>(FrontFaceDepthFailStencilOp);
		//					int FrontFacePassStencilOp = 0;
		//					archive >> FrontFacePassStencilOp;
		//					Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.FrontFacePassStencilOp = static_cast<EStencilOp>(FrontFacePassStencilOp);
		//				}
		//				archive >> Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.bEnableBackFaceStencil;
		//				{
		//					int BackFaceStencilTest = 0;
		//					archive >> BackFaceStencilTest;
		//					Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.BackFaceStencilTest = static_cast<ECompareFunction>(BackFaceStencilTest);
		//					int BackFaceStencilFailStencilOp = 0;
		//					archive >> BackFaceStencilFailStencilOp;
		//					Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.BackFaceStencilFailStencilOp = static_cast<EStencilOp>(BackFaceStencilFailStencilOp);
		//					int BackFaceDepthFailStencilOp = 0;
		//					archive >> BackFaceDepthFailStencilOp;
		//					Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.BackFaceDepthFailStencilOp = static_cast<EStencilOp>(BackFaceDepthFailStencilOp);
		//					int BackFacePassStencilOp = 0;
		//					archive >> BackFacePassStencilOp;
		//					Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.BackFacePassStencilOp = static_cast<EStencilOp>(BackFacePassStencilOp);
		//				}
		//				archive >> Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.StencilReadMask;
		//				archive >> Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.StencilWriteMask;

		//				int StateStatus = 0;
		//				archive >> StateStatus;
		//				Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.StateStatus = static_cast<EStateStatus>(StateStatus);
		//			}
		//			// BlendAttributeDesc
		//			{
		//				for (auto& Blend : Asset.mPipelineAsset.pPipelineDesc.BlendAttribute.RenderTargets)
		//				{
		//					archive >> Blend.bBlendEnable;
		//					int ColorBlendOp = 0;
		//					archive >> ColorBlendOp;
		//					Blend.ColorBlendOp = static_cast<EBlendOperation>(ColorBlendOp);
		//					int ColorSrcBlend = 0;
		//					archive >> ColorSrcBlend;
		//					Blend.ColorSrcBlend = static_cast<EBlendFactor>(ColorSrcBlend);
		//					int ColorDestBlend = 0;
		//					archive >> ColorDestBlend;
		//					Blend.ColorDestBlend = static_cast<EBlendFactor>(ColorDestBlend);
		//					int AlphaBlendOp = 0;
		//					archive >> AlphaBlendOp;
		//					Blend.AlphaBlendOp = static_cast<EBlendOperation>(AlphaBlendOp);
		//					int AlphaSrcBlend = 0;
		//					archive >> AlphaSrcBlend;
		//					Blend.AlphaSrcBlend = static_cast<EBlendFactor>(AlphaSrcBlend);
		//					int AlphaDestBlend = 0;
		//					archive >> AlphaDestBlend;
		//					Blend.AlphaDestBlend = static_cast<EBlendFactor>(AlphaDestBlend);
		//					int ColorWriteMask = 0;
		//					archive >> ColorWriteMask;
		//					Blend.ColorWriteMask = static_cast<EColorWriteMask>(ColorWriteMask);
		//					int StateStatus = 0;
		//					archive >> StateStatus;
		//					Blend.StateStatus = static_cast<EStateStatus>(StateStatus);
		//				}
		//			}
		//			// PrimitiveType
		//			{
		//				int StateStatus = 0;
		//				archive >> StateStatus;
		//				Asset.mPipelineAsset.pPipelineDesc.PrimitiveTopologyType = static_cast<PrimitiveType>(StateStatus);
		//			}

		//			MaterialAssets.insert({ Asset.Name, Asset });
		//		}
		//		break;
		//		case 2:
		//		{
		//			MaterialInstanceAsset Asset;
		//			Asset.Type = DNGEAssetType::MaterialInstance;

		//			archive >> Asset.Name;
		//			archive >> Asset.ParentName;
		//			archive >> Asset.Path;
		//			archive >> Asset.TextureSize;

		//			for (std::uint32_t i = 0; i < Asset.TextureSize; i++)
		//			{
		//				MaterialInstanceAsset::CTexture CTexture;
		//				archive >> CTexture.Slot;
		//				archive >> CTexture.Location;
		//				Asset.TextureLocations.push_back(CTexture);
		//			}

		//			MaterialInstanceAssets.insert({ Asset.Name, Asset });
		//		}
		//		break;
		//		default:
		//			std::cout << "UNK" << std::endl;
		//			break;
		//		}
		//	}
		//	else if (p.path().extension() == MapExt)
		//	{
		//		std::string Path = FileManager::WideStringToString(p.path().c_str());
		//		sArchive archive(Path);

		//		MapAsset Asset;
		//		archive >> Asset.Name;
		//		if (IsMapAssetExist(Asset.Name))
		//		{
		//			continue;
		//		}
		//		archive >> Asset.Path;

		//		archive >> Asset.mMaterialData.MaterialSize;
		//		for (std::uint32_t i = 0; i < Asset.mMaterialData.MaterialSize; i++)
		//		{
		//			MapAsset::MapMaterialData::MaterialData Data;
		//			archive >> Data.Name;
		//			archive >> Data.Path;
		//			archive >> Data.InstanceSize;
		//			for (std::uint32_t ii = 0; ii < Data.InstanceSize; ii++)
		//			{
		//				MapAsset::MapMaterialData::MapMaterialInstanceData InstanceData;
		//				archive >> InstanceData.Name;
		//				archive >> InstanceData.Path;
		//				Data.Instances.push_back(InstanceData);
		//			}
		//			Asset.mMaterialData.Materials.push_back(Data);
		//		}

		//		archive >> Asset.mActorData.ActorSize;
		//		for (std::uint32_t i = 0; i < Asset.mActorData.ActorSize; i++)
		//		{
		//			MapAsset::MapActorData::ActorData Data;
		//			archive >> Data.Name;

		//			FVector4 ActorRotation;
		//			FVector ActorTranslation;
		//			FVector ActorScale3D;
		//			archive >> ActorRotation;
		//			archive >> ActorTranslation;
		//			archive >> ActorScale3D;
		//			Data.ActorTransform.SetRotation(ActorRotation);
		//			int TransformProperty = 0;
		//			archive >> TransformProperty;
		//			Data.ActorTransform.SetLocation(ActorTranslation);
		//			Data.ActorTransform.SetScale(ActorScale3D);

		//			archive >> Data.ComponentSize;
		//			for (std::uint32_t ii = 0; ii < Data.ComponentSize; ii++)
		//			{
		//				MapAsset::MapActorData::MapActorComponentData InstanceData;
		//				archive >> InstanceData.Name;

		//				FVector4 ComponentRotation;
		//				FVector ComponentTranslation;
		//				FVector ComponentScale3D;
		//				archive >> ComponentRotation;
		//				archive >> ComponentTranslation;
		//				archive >> ComponentScale3D;
		//				InstanceData.ComponentTransform.SetRotation(ComponentRotation);
		//				int ComponentTransformProperty = 0;
		//				archive >> ComponentTransformProperty;
		//				InstanceData.ComponentTransform.SetLocation(ComponentTranslation);
		//				InstanceData.ComponentTransform.SetScale(ComponentScale3D);

		//				int Type = 0;
		//				//archive >> Type;
		//				//InstanceData.Type = static_cast<EComponentType>(Type);
		//				int CreationType = 0;
		//				//archive >> CreationType;
		//				//InstanceData.CreationType = static_cast<EComponentCreationType>(CreationType);
		//				archive >> InstanceData.Path;
		//				Data.Components.push_back(InstanceData);
		//			}
		//			Asset.mActorData.Actors.push_back(Data);
		//		}

		//		Asset.mCameraAsset.Read(archive);

		//		MapAssets.insert({ Asset.Name, Asset });
		//	}
		//}
	}

	MeshAsset LoadMeshAsset(std::string InName)
	{
		/*if (IsMeshAssetExist(InName))
		{
			return GetMeshAsset(InName);
		}

		std::string path("..//Content//");
		std::string ext(".DNGEAsset");
		for (auto& p : fs::recursive_directory_iterator(path))
		{
			if (p.path().extension() == ext)
			{
				std::string Path = FileManager::WideStringToString(p.path().c_str());
				sArchive archive(Path);
				int Type;
				archive >> Type;

				MeshAsset Asset;
				Asset.Type = DNGEAssetType::Mesh;
				archive >> Asset.Name;

				if (InName != Asset.Name)
					continue;

				archive >> Asset.Path;
				archive >> Asset.OffsetFromOrigin;
				archive >> Asset.AABB.Min;
				archive >> Asset.AABB.Max;
				archive >> Asset.DefaultMaterialName;
				archive >> Asset.DefaultMaterialPath;
				archive >> Asset.mMeshData.Indices;

				archive >> Asset.VerticesSize;

				for (std::uint32_t i = 0; i < Asset.VerticesSize; i++)
				{
					sVertexBufferEntry VBE;
					archive >> VBE.position;
					archive >> VBE.normal;
					archive >> VBE.texCoord;
					archive >> VBE.tangent;
					archive >> VBE.binormal;
					archive >> VBE.ArrayIndex;
					Asset.mMeshData.Vertices.push_back(VBE);
				}

				archive >> Asset.mMeshData.DrawParameters.IndexCountPerInstance;
				archive >> Asset.mMeshData.DrawParameters.InstanceCount;
				archive >> Asset.mMeshData.DrawParameters.StartIndexLocation;
				archive >> Asset.mMeshData.DrawParameters.BaseVertexLocation;
				archive >> Asset.mMeshData.DrawParameters.StartInstanceLocation;

				MeshAssets.insert({ Asset.Name, Asset });

				return Asset;
			}
		}*/
		return MeshAsset();
	}

	MaterialAsset LoadMaterialAsset(std::string InName)
	{
		//if (IsMaterialAssetExist(InName))
		//{
		//	return GetMaterialAsset(InName);
		//}

		//std::string path("..//Content//");
		//std::string ext(".DNGEAsset");
		//for (auto& p : fs::recursive_directory_iterator(path))
		//{
		//	if (p.path().extension() == ext)
		//	{
		//		std::string Path = FileManager::WideStringToString(p.path().c_str());
		//		sArchive archive(Path);
		//		int Type;
		//		archive >> Type;

		//		MaterialAsset Asset;

		//		Asset.Type = DNGEAssetType::Material;
		//		archive >> Asset.Name;

		//		if (InName != Asset.Name)
		//			continue;

		//		archive >> Asset.Path;

		//		int MaterialType = 0;
		//		archive >> MaterialType;
		//		Asset.MaterialType = static_cast<EMaterialType>(MaterialType);
		//		int MaterialUsage = 0;
		//		archive >> MaterialUsage;
		//		Asset.MaterialUsage = static_cast<EMaterialUsage>(MaterialUsage);
		//		int BlendMode = 0;
		//		archive >> BlendMode;
		//		Asset.BlendMode = static_cast<EMaterialBlendMode>(BlendMode);

		//		archive >> Asset.mPipelineAsset.VertexLayoutSize;

		//		for (std::uint32_t i = 0; i < Asset.mPipelineAsset.VertexLayoutSize; i++)
		//		{
		//			sVertexAttributeDesc VBE;
		//			archive >> VBE.name;
		//			int format = 0;
		//			archive >> format;
		//			VBE.format = static_cast<EFormat>(format);
		//			archive >> VBE.bufferIndex;
		//			archive >> VBE.offset;
		//			archive >> VBE.isInstanced;
		//			Asset.mPipelineAsset.VertexLayout.push_back(VBE);
		//		}

		//		archive >> Asset.mPipelineAsset.DescriptorSetLayoutSize;

		//		for (std::uint32_t i = 0; i < Asset.mPipelineAsset.DescriptorSetLayoutSize; i++)
		//		{
		//			sDescriptorSetLayoutBinding DSL;
		//			unsigned char DescriptorType = 0;
		//			archive >> DescriptorType;
		//			DSL.DescriptorType = static_cast<EDescriptorType>(DescriptorType);
		//			unsigned char ShaderType = 0;
		//			archive >> ShaderType;
		//			DSL.ShaderType = static_cast<eShaderType>(ShaderType);
		//			archive >> DSL.Location;
		//			Asset.mPipelineAsset.DescriptorSetLayout.push_back(DSL);
		//		}

		//		archive >> Asset.mPipelineAsset.ShaderAttachmentsSize;

		//		for (std::uint32_t i = 0; i < Asset.mPipelineAsset.ShaderAttachmentsSize; i++)
		//		{
		//			sShaderAttachment SA;
		//			archive >> SA.Location;
		//			archive >> SA.FunctionName;
		//			unsigned char ShaderType = 0;
		//			archive >> ShaderType;
		//			SA.Type = static_cast<eShaderType>(ShaderType);
		//			Asset.mPipelineAsset.ShaderAttachments.push_back(SA);
		//		}

		//		archive >> Asset.mPipelineAsset.Name;
		//		archive >> Asset.mPipelineAsset.PipelineCBSize;

		//		/*std::map<eShaderType, PipelineDesc::ConstantBufferAttributes> ConstantBuffers;
		//		for (std::uint32_t i = 0; i < Asset.mPipelineAsset.PipelineCBSize; i++)
		//		{
		//			int slot = 0;
		//			std::string Name;
		//			eShaderType Type;

		//			unsigned char ShaderType = 0;
		//			archive >> ShaderType;
		//			Type = static_cast<eShaderType>(ShaderType);

		//			archive >> Name;
		//			archive >> slot;

		//			PipelineDesc::ConstantBufferAttributes CB(AResources::Get().GetConstantBuffer(Name), slot);
		//			ConstantBuffers.insert({ Type, CB });
		//			Asset.mPipelineAsset.pPipelineDesc.ConstantBuffers = ConstantBuffers;
		//		}*/

		//		{
		//			int FillMode = 0;
		//			archive >> FillMode;
		//			Asset.mPipelineAsset.pPipelineDesc.RasterizerAttribute.FillMode = static_cast<ERasterizerFillMode>(FillMode);
		//			int CullMode = 0;
		//			archive >> CullMode;
		//			Asset.mPipelineAsset.pPipelineDesc.RasterizerAttribute.CullMode = static_cast<ERasterizerCullMode>(CullMode);
		//			archive >> Asset.mPipelineAsset.pPipelineDesc.RasterizerAttribute.DepthBias;
		//			archive >> Asset.mPipelineAsset.pPipelineDesc.RasterizerAttribute.DepthBiasClamp;
		//			archive >> Asset.mPipelineAsset.pPipelineDesc.RasterizerAttribute.SlopeScaleDepthBias;
		//			archive >> Asset.mPipelineAsset.pPipelineDesc.RasterizerAttribute.DepthClipEnable;
		//			archive >> Asset.mPipelineAsset.pPipelineDesc.RasterizerAttribute.bAllowMSAA;
		//			archive >> Asset.mPipelineAsset.pPipelineDesc.RasterizerAttribute.bEnableLineAA;
		//			archive >> Asset.mPipelineAsset.pPipelineDesc.RasterizerAttribute.FrontCounterClockwise;
		//			int StateStatus = 0;
		//			archive >> StateStatus;
		//			Asset.mPipelineAsset.pPipelineDesc.RasterizerAttribute.StateStatus = static_cast<EStateStatus>(StateStatus);
		//		}
		//		// DepthStencilAttributeDesc
		//		{
		//			archive >> Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.bEnableDepthWrite;
		//			archive >> Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.bDepthWriteMask;
		//			int DepthTest = 0;
		//			archive >> DepthTest;
		//			Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.DepthTest = static_cast<ECompareFunction>(DepthTest);
		//			archive >> Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.bStencilEnable;

		//			archive >> Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.bEnableFrontFaceStencil;
		//			{
		//				int FrontFaceStencilTest = 0;
		//				archive >> FrontFaceStencilTest;
		//				Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.FrontFaceStencilTest = static_cast<ECompareFunction>(FrontFaceStencilTest);
		//				int FrontFaceStencilFailStencilOp = 0;
		//				archive >> FrontFaceStencilFailStencilOp;
		//				Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.FrontFaceStencilFailStencilOp = static_cast<EStencilOp>(FrontFaceStencilFailStencilOp);
		//				int FrontFaceDepthFailStencilOp = 0;
		//				archive >> FrontFaceDepthFailStencilOp;
		//				Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.FrontFaceDepthFailStencilOp = static_cast<EStencilOp>(FrontFaceDepthFailStencilOp);
		//				int FrontFacePassStencilOp = 0;
		//				archive >> FrontFacePassStencilOp;
		//				Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.FrontFacePassStencilOp = static_cast<EStencilOp>(FrontFacePassStencilOp);
		//			}
		//			archive >> Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.bEnableBackFaceStencil;
		//			{
		//				int BackFaceStencilTest = 0;
		//				archive >> BackFaceStencilTest;
		//				Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.BackFaceStencilTest = static_cast<ECompareFunction>(BackFaceStencilTest);
		//				int BackFaceStencilFailStencilOp = 0;
		//				archive >> BackFaceStencilFailStencilOp;
		//				Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.BackFaceStencilFailStencilOp = static_cast<EStencilOp>(BackFaceStencilFailStencilOp);
		//				int BackFaceDepthFailStencilOp = 0;
		//				archive >> BackFaceDepthFailStencilOp;
		//				Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.BackFaceDepthFailStencilOp = static_cast<EStencilOp>(BackFaceDepthFailStencilOp);
		//				int BackFacePassStencilOp = 0;
		//				archive >> BackFacePassStencilOp;
		//				Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.BackFacePassStencilOp = static_cast<EStencilOp>(BackFacePassStencilOp);
		//			}
		//			archive >> Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.StencilReadMask;
		//			archive >> Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.StencilWriteMask;

		//			int StateStatus = 0;
		//			archive >> StateStatus;
		//			Asset.mPipelineAsset.pPipelineDesc.DepthStencilAttribute.StateStatus = static_cast<EStateStatus>(StateStatus);
		//		}
		//		// BlendAttributeDesc
		//		{
		//			for (auto& Blend : Asset.mPipelineAsset.pPipelineDesc.BlendAttribute.RenderTargets)
		//			{
		//				archive >> Blend.bBlendEnable;
		//				int ColorBlendOp = 0;
		//				archive >> ColorBlendOp;
		//				Blend.ColorBlendOp = static_cast<EBlendOperation>(ColorBlendOp);
		//				int ColorSrcBlend = 0;
		//				archive >> ColorSrcBlend;
		//				Blend.ColorSrcBlend = static_cast<EBlendFactor>(ColorSrcBlend);
		//				int ColorDestBlend = 0;
		//				archive >> ColorDestBlend;
		//				Blend.ColorDestBlend = static_cast<EBlendFactor>(ColorDestBlend);
		//				int AlphaBlendOp = 0;
		//				archive >> AlphaBlendOp;
		//				Blend.AlphaBlendOp = static_cast<EBlendOperation>(AlphaBlendOp);
		//				int AlphaSrcBlend = 0;
		//				archive >> AlphaSrcBlend;
		//				Blend.AlphaSrcBlend = static_cast<EBlendFactor>(AlphaSrcBlend);
		//				int AlphaDestBlend = 0;
		//				archive >> AlphaDestBlend;
		//				Blend.AlphaDestBlend = static_cast<EBlendFactor>(AlphaDestBlend);
		//				int ColorWriteMask = 0;
		//				archive >> ColorWriteMask;
		//				Blend.ColorWriteMask = static_cast<EColorWriteMask>(ColorWriteMask);
		//				int StateStatus = 0;
		//				archive >> StateStatus;
		//				Blend.StateStatus = static_cast<EStateStatus>(StateStatus);
		//			}
		//		}
		//		// PrimitiveType
		//		{
		//			int StateStatus = 0;
		//			archive >> StateStatus;
		//			Asset.mPipelineAsset.pPipelineDesc.PrimitiveTopologyType = static_cast<PrimitiveType>(StateStatus);
		//		}

		//		MaterialAssets.insert({ Asset.Name, Asset });

		//		return Asset;
		//	}
		//}
		return MaterialAsset();
	}

	MaterialInstanceAsset LoadMaterialInstanceAsset(std::string InName)
	{
		/*if (IsMaterialInstanceAssetExist(InName))
		{
			return GetMaterialInstanceAsset(InName);
		}

		std::string path("..//Content//");
		std::string ext(".DNGEAsset");
		for (auto& p : fs::recursive_directory_iterator(path))
		{
			if (p.path().extension() == ext)
			{
				std::string Path = FileManager::Get().WideStringToString(p.path().c_str());
				sArchive archive(Path);
				int Type;
				archive >> Type;

				MaterialInstanceAsset Asset;
				Asset.Type = DNGEAssetType::MaterialInstance;
				archive >> Asset.Name;

				if (InName != Asset.Name)
					continue;

				archive >> Asset.ParentName;
				archive >> Asset.Path;
				archive >> Asset.TextureSize;

				for (std::uint32_t i = 0; i < Asset.TextureSize; i++)
				{
					MaterialInstanceAsset::CTexture CTexture;
					archive >> CTexture.Slot;
					archive >> CTexture.Location;
					Asset.TextureLocations.push_back(CTexture);
				}

				MaterialInstanceAssets.insert({ Asset.Name, Asset });

				return Asset;
			}
		}*/
		return MaterialInstanceAsset();
	}

	std::map<std::string, MeshAsset> GetAllMeshAssets()
	{
		return MeshAssets;
	}

	MeshAsset& GetMeshAsset(std::string InName)
	{
		return MeshAssets[InName];
	}

	void AddMeshAsset(MeshAsset InAsset)
	{
		MeshAssets.insert({ InAsset.Name, InAsset });
	}

	bool IsMeshAssetExist(std::string InName)
	{
		if (MeshAssets.find(InName) == MeshAssets.end()) {
			return false;
		}
		else {
			return true;
		}
	}

	std::map<std::string, MaterialAsset> GetAllMaterialAssets()
	{
		return MaterialAssets;
	}

	MaterialAsset& GetMaterialAsset(std::string InName)
	{
		return MaterialAssets[InName];
	}

	void AddMaterialAsset(MaterialAsset InAsset)
	{
		MaterialAssets.insert({ InAsset.Name, InAsset });
	}

	bool IsMaterialAssetExist(std::string InName)
	{
		if (MaterialAssets.find(InName) == MaterialAssets.end()) {
			return false;
		}
		else {
			return true;
		}
	}

	std::map<std::string, MaterialInstanceAsset> GetAllMaterialInstanceAssets()
	{
		return MaterialInstanceAssets;
	}

	MaterialInstanceAsset& GetMaterialInstanceAsset(std::string InName)
	{
		return MaterialInstanceAssets[InName];
	}

	void AddMaterialInstanceAsset(MaterialInstanceAsset InAsset)
	{
		MaterialInstanceAssets.insert({ InAsset.Name, InAsset });
	}

	bool IsMaterialInstanceAssetExist(std::string InName)
	{
		if (MaterialInstanceAssets.find(InName) == MaterialInstanceAssets.end()) {
			return false;
		}
		else {
			return true;
		}
	}

	std::map<std::string, MapAsset> GetAllMapAssets()
	{
		return MapAssets;
	}

	MapAsset& GetMapAsset(std::string InName)
	{
		return MapAssets[InName];
	}

	void AddMapAsset(MapAsset InAsset)
	{
		MapAssets.insert({ InAsset.Name, InAsset });
	}

	bool IsMapAssetExist(std::string InName)
	{
		if (MapAssets.find(InName) == MapAssets.end()) {
			return false;
		}
		else {
			return true;
		}
	}
};
