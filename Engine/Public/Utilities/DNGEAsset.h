#pragma once

#include <string>
#include <vector>
#include "Engine/AbstractEngine.h"
#include "Core/Math/CoreMath.h"
#include "Core/Camera.h"
#include "Core/Archive.h"
#include "AbstractGI/Material.h"
#include "Gameplay/Actor.h"
#include "Gameplay/PrimitiveComponent.h"

enum class DNGEAssetType
{
	Mesh = 0x0,
	Material = 0x1,
	MaterialInstance = 0x2,
};

struct DNGEAsset
{
	DNGEAssetType Type;

	DNGEAsset() = default;
	DNGEAsset(DNGEAssetType InType)
		: Type(InType)
	{}
};

struct CameraAsset
{
	CameraAsset()
		: reverseZ(true)
		, AspectRatio(0.0f)
		, Fov(0.0f)
		, NearZ(0.0f)
		, FarZ(0.0f)
		, ViewMatrix(FMatrix::Identity())
		, WorldMatrix(FMatrix::Identity())
		, sceneScaling(1.0f)
		, IsOrtho(false)
	{}

	CameraAsset(sCamera* InCamera)
		: reverseZ(InCamera->IsZReversed())
		, AspectRatio(InCamera->GetAspectRatio())
		, Fov(InCamera->GetFOV())
		, NearZ(InCamera->GetNearClip())
		, FarZ(InCamera->GetFarClip())
		, Eye(InCamera->GetEye())
		, Focus(InCamera->GetFocus())
		, ViewMatrix(InCamera->GetViewMatrix())
		, Position(InCamera->GetPosition())
		, Rotation(InCamera->GetRotation())
		, WorldMatrix(InCamera->GetWorldMatrix())
		, sceneScaling(InCamera->GetSceneScaling())
		, IsOrtho(InCamera->IsOrthographic())
	{}

	bool reverseZ;
	float AspectRatio;
	float Fov;
	float NearZ;
	float FarZ;
	FMatrix ViewMatrix;
	FVector Position;
	FQuaternion Rotation;

	FVector Eye;
	FVector Focus;

	FMatrix WorldMatrix;
	float sceneScaling;

	bool IsOrtho;

	void Write(Archive& archive) const
	{
		//archive << reverseZ;
		//archive << AspectRatio;
		//archive << Fov;
		//archive << NearZ;
		//archive << FarZ;
		////archive << Eye;
		////archive << At;
		////archive << Up;

		//FVector4 VM[4];
		//VM[0] = (DirectX::XMVECTOR)ViewMatrix.r[0];
		//VM[1] = (DirectX::XMVECTOR)ViewMatrix.r[1];
		//VM[2] = (DirectX::XMVECTOR)ViewMatrix.r[2];
		//VM[3] = (DirectX::XMVECTOR)ViewMatrix.r[3];
		//for (int i = 0; i < 4; i++)
		//{
		//	archive << VM[i];
		//}

		//archive << Position;
		//archive << Rotation;
		//FVector4 WM[4];
		//WM[0] = (DirectX::XMVECTOR)WorldMatrix.r[0];
		//WM[1] = (DirectX::XMVECTOR)WorldMatrix.r[1];
		//WM[2] = (DirectX::XMVECTOR)WorldMatrix.r[2];
		//WM[3] = (DirectX::XMVECTOR)WorldMatrix.r[3];
		//for (int i = 0; i < 4; i++)
		//{
		//	archive << WM[i];
		//}
		//archive << IsOrtho;
		//archive << sceneScaling;
	}

	void Write(std::string InPath) const
	{
		//Archive archive(InPath, false);
		//archive << reverseZ;
		//archive << AspectRatio;
		//archive << Fov;
		//archive << NearZ;
		//archive << FarZ;
		////archive << Eye;
		////archive << At;
		////archive << Up;

		//FVector4 VM[4];
		//VM[0] = (DirectX::XMVECTOR)ViewMatrix.r[0];
		//VM[1] = (DirectX::XMVECTOR)ViewMatrix.r[1];
		//VM[2] = (DirectX::XMVECTOR)ViewMatrix.r[2];
		//VM[3] = (DirectX::XMVECTOR)ViewMatrix.r[3];
		//for (int i = 0; i < 4; i++)
		//{
		//	archive << VM[i];
		//}

		//archive << Position;
		//archive << Rotation;
		//FVector4 WM[4];
		//WM[0] = (DirectX::XMVECTOR)WorldMatrix.r[0];
		//WM[1] = (DirectX::XMVECTOR)WorldMatrix.r[1];
		//WM[2] = (DirectX::XMVECTOR)WorldMatrix.r[2];
		//WM[3] = (DirectX::XMVECTOR)WorldMatrix.r[3];
		//for (int i = 0; i < 4; i++)
		//{
		//	archive << WM[i];
		//}
		//archive << IsOrtho;
		//archive << sceneScaling;
	}

	void Read(Archive& archive)
	{
		//archive >> reverseZ;
		//archive >> AspectRatio;
		//archive >> Fov;
		//archive >> NearZ;
		//archive >> FarZ;
		////archive >> Eye;
		////archive >> At;
		////archive >> Up;

		//FVector4 VM[4];
		//for (int i = 0; i < 4; i++)
		//{
		//	archive >> VM[i];
		//	ViewMatrix.r[0] = (VM[i].x);
		//	ViewMatrix.r[1] = (VM[i].y);
		//	ViewMatrix.r[2] = (VM[i].z);
		//	ViewMatrix.r[3] = (VM[i].w);
		//}

		//archive >> Position;
		//archive >> Rotation;
		//FVector4 WM[4];
		//for (int i = 0; i < 4; i++)
		//{
		//	archive >> WM[i];
		//	VWM[i].SetX(WM[i].x);
		//	WorldMatrix.r[0] = (VM[i].x);
		//	WorldMatrix.r[1] = (VM[i].y);
		//	WorldMatrix.r[2] = (VM[i].z);
		//	WorldMatrix.r[3] = (VM[i].w);
		//}
		//archive >> IsOrtho;
		//archive >> sceneScaling;
	}

	void Read(std::string InPath)
	{
		//Archive archive(InPath);
		//archive >> reverseZ;
		//archive >> AspectRatio;
		//archive >> Fov;
		//archive >> NearZ;
		//archive >> FarZ;
		////archive >> Eye;
		////archive >> At;
		////archive >> Up;

		//FVector4 VM[4];
		//for (int i = 0; i < 4; i++)
		//{
		//	archive >> VM[i];
		//	ViewMatrix.r[0] = (VM[i].x);
		//	ViewMatrix.r[1] = (VM[i].y);
		//	ViewMatrix.r[2] = (VM[i].z);
		//	ViewMatrix.r[3] = (VM[i].w);
		//}

		//archive >> Position;
		//archive >> Rotation;
		//DirectX::FVector4 WM[4];
		//Math::Vector4 VWM[4];
		//for (int i = 0; i < 4; i++)
		//{
		//	archive >> WM[i];
		//	WorldMatrix.r[0] = (VM[i].x);
		//	WorldMatrix.r[1] = (VM[i].y);
		//	WorldMatrix.r[2] = (VM[i].z);
		//	WorldMatrix.r[3] = (VM[i].w);
		//}
		//archive >> IsOrtho;
		//archive >> sceneScaling;
	}
};

struct MeshAsset : public DNGEAsset
{
	MeshAsset()
		: DNGEAsset(DNGEAssetType::Mesh)
		, VerticesSize(0)
	{};

	std::string Name;
	std::string Path;

	FVector OffsetFromOrigin;
	FBoundingBox AABB;

	std::string DefaultMaterialName;
	std::wstring DefaultMaterialPath;

	mutable std::uint32_t VerticesSize;
	sMeshData mMeshData;

	void Write() const
	{
		/*Archive archive(Path, false);
		archive << static_cast<std::underlying_type<DNGEAssetType>::type>(Type);

		archive << Name;
		archive << Path;

		archive << OffsetFromOrigin;
		archive << AABB.Min;
		archive << AABB.Max;

		archive << DefaultMaterialName;
		archive << DefaultMaterialPath;

		archive << mMeshData.Indices;

		VerticesSize = static_cast<std::uint32_t>(mMeshData.Vertices.size());
		archive << VerticesSize;

		for (auto& VBE : mMeshData.Vertices)
		{
			archive << VBE.position;
			archive << VBE.normal;
			archive << VBE.texCoord;
			archive << VBE.tangent;
			archive << VBE.binormal;
			archive << VBE.ArrayIndex;
		}

		archive << mMeshData.DrawParameters.IndexCountPerInstance;
		archive << mMeshData.DrawParameters.InstanceCount;
		archive << mMeshData.DrawParameters.StartIndexLocation;
		archive << mMeshData.DrawParameters.BaseVertexLocation;
		archive << mMeshData.DrawParameters.StartInstanceLocation;*/
	}

	void Read(std::string InPath, bool InSkipType = false)
	{
		/*Archive archive(InPath);
		Type = DNGEAssetType::Mesh;

		if (InSkipType)
		{
			int A = 0;
			archive >> A;
		}

		archive >> Name;
		archive >> Path;

		archive >> OffsetFromOrigin;
		archive >> AABB.Min;
		archive >> AABB.Max;

		archive >> DefaultMaterialName;
		archive >> DefaultMaterialPath;

		archive >> mMeshData.Indices;

		archive >> VerticesSize;

		for (std::uint32_t i = 0; i < VerticesSize; i++)
		{
			VertexBufferEntry VBE;
			archive >> VBE.position;
			archive >> VBE.normal;
			archive >> VBE.texCoord;
			archive >> VBE.tangent;
			archive >> VBE.binormal;
			archive >> VBE.ArrayIndex;
			mMeshData.Vertices.push_back(VBE);
		}

		archive >> mMeshData.DrawParameters.IndexCountPerInstance;
		archive >> mMeshData.DrawParameters.InstanceCount;
		archive >> mMeshData.DrawParameters.StartIndexLocation;
		archive >> mMeshData.DrawParameters.BaseVertexLocation;
		archive >> mMeshData.DrawParameters.StartInstanceLocation;*/
	}

};

struct MaterialAsset : public DNGEAsset
{
	struct PipelineAsset
	{
		PipelineAsset()
			: Name("")
			, PipelineCBSize(0)
			, VertexLayoutSize(0)
			, DescriptorSetLayoutSize(0)
			, ShaderAttachmentsSize(0)
		{}

		std::string Name;

		mutable std::uint32_t PipelineCBSize;
		sPipelineDesc pPipelineDesc;

		std::vector<sVertexAttributeDesc> VertexLayout;
		mutable std::uint32_t VertexLayoutSize;

		std::vector<sDescriptorSetLayoutBinding> DescriptorSetLayout;
		mutable std::uint32_t DescriptorSetLayoutSize;

		std::vector<sShaderAttachment> ShaderAttachments;
		mutable std::uint32_t ShaderAttachmentsSize;

		void Write(Archive& InArchive) const
		{
			//InArchive << Name;

			//// VertexAttributeDesc
			//{
			//	VertexLayoutSize = static_cast<std::uint32_t>(VertexLayout.size());
			//	InArchive << VertexLayoutSize;

			//	for (auto& VL : VertexLayout)
			//	{
			//		InArchive << VL.name;
			//		InArchive << VL.format;
			//		InArchive << VL.bufferIndex;
			//		InArchive << VL.offset;
			//		InArchive << VL.isInstanced;
			//	}
			//}
			//// DescriptorSetLayoutBinding
			//{
			//	DescriptorSetLayoutSize = static_cast<std::uint32_t>(DescriptorSetLayout.size());
			//	InArchive << DescriptorSetLayoutSize;

			//	for (auto& DSL : DescriptorSetLayout)
			//	{
			//		InArchive << static_cast<std::underlying_type<EDescriptorType>::type>(DSL.DescriptorType);
			//		InArchive << static_cast<std::underlying_type<eShaderType>::type>(DSL.ShaderType);
			//		InArchive << DSL.Location;
			//	}
			//}
			//// ShaderAttachment
			//{
			//	ShaderAttachmentsSize = static_cast<std::uint32_t>(ShaderAttachments.size());
			//	InArchive << ShaderAttachmentsSize;

			//	for (auto& SA : ShaderAttachments)
			//	{
			//		InArchive << SA.Location;
			//		InArchive << SA.FunctionName;
			//		InArchive << static_cast<std::underlying_type<eShaderType>::type>(SA.Type);
			//	}
			//}

			//// ConstantBufferAttributes
			//{
			//	/*PipelineCBSize = static_cast<std::uint32_t>(pPipelineDesc.ConstantBuffers.size());
			//	InArchive << PipelineCBSize;

			//	for (auto& CB : pPipelineDesc.ConstantBuffers)
			//	{
			//		InArchive << static_cast<std::underlying_type<eShaderType>::type>(CB.first);
			//		InArchive << CB.second.ConstantBuffer->GetName();
			//		InArchive << CB.second.slot;
			//	}*/
			//}
			//// Rasterizer
			//{
			//	InArchive << static_cast<std::underlying_type<ERasterizerFillMode>::type>(pPipelineDesc.RasterizerAttribute.FillMode);
			//	InArchive << static_cast<std::underlying_type<ERasterizerCullMode>::type>(pPipelineDesc.RasterizerAttribute.CullMode);
			//	InArchive << pPipelineDesc.RasterizerAttribute.DepthBias;
			//	InArchive << pPipelineDesc.RasterizerAttribute.DepthBiasClamp;
			//	InArchive << pPipelineDesc.RasterizerAttribute.SlopeScaleDepthBias;
			//	InArchive << pPipelineDesc.RasterizerAttribute.DepthClipEnable;
			//	InArchive << pPipelineDesc.RasterizerAttribute.bAllowMSAA;
			//	InArchive << pPipelineDesc.RasterizerAttribute.bEnableLineAA;
			//	InArchive << pPipelineDesc.RasterizerAttribute.FrontCounterClockwise;
			//	InArchive << static_cast<std::underlying_type<EStateStatus>::type>(pPipelineDesc.RasterizerAttribute.StateStatus); 
			//}
			//// DepthStencilAttributeDesc
			//{
			//	InArchive << pPipelineDesc.DepthStencilAttribute.bEnableDepthWrite;
			//	InArchive << pPipelineDesc.DepthStencilAttribute.bDepthWriteMask;
			//	InArchive << static_cast<std::underlying_type<ECompareFunction>::type>(pPipelineDesc.DepthStencilAttribute.DepthTest);
			//	InArchive << pPipelineDesc.DepthStencilAttribute.bStencilEnable;
			//	InArchive << pPipelineDesc.DepthStencilAttribute.bEnableFrontFaceStencil;
			//	InArchive << static_cast<std::underlying_type<ECompareFunction>::type>(pPipelineDesc.DepthStencilAttribute.FrontFaceStencilTest);
			//	InArchive << static_cast<std::underlying_type<EStencilOp>::type>(pPipelineDesc.DepthStencilAttribute.FrontFaceStencilFailStencilOp);
			//	InArchive << static_cast<std::underlying_type<EStencilOp>::type>(pPipelineDesc.DepthStencilAttribute.FrontFaceDepthFailStencilOp);
			//	InArchive << static_cast<std::underlying_type<EStencilOp>::type>(pPipelineDesc.DepthStencilAttribute.FrontFacePassStencilOp);
			//	InArchive << pPipelineDesc.DepthStencilAttribute.bEnableBackFaceStencil;
			//	InArchive << static_cast<std::underlying_type<ECompareFunction>::type>(pPipelineDesc.DepthStencilAttribute.BackFaceStencilTest);
			//	InArchive << static_cast<std::underlying_type<EStencilOp>::type>(pPipelineDesc.DepthStencilAttribute.BackFaceStencilFailStencilOp);
			//	InArchive << static_cast<std::underlying_type<EStencilOp>::type>(pPipelineDesc.DepthStencilAttribute.BackFaceDepthFailStencilOp);
			//	InArchive << static_cast<std::underlying_type<EStencilOp>::type>(pPipelineDesc.DepthStencilAttribute.BackFacePassStencilOp);
			//	InArchive << pPipelineDesc.DepthStencilAttribute.StencilReadMask;
			//	InArchive << pPipelineDesc.DepthStencilAttribute.StencilWriteMask;
			//	InArchive << static_cast<std::underlying_type<EStateStatus>::type>(pPipelineDesc.DepthStencilAttribute.StateStatus);
			//}
			//// BlendAttributeDesc
			//{
			//	for (auto& Blend : pPipelineDesc.BlendAttribute.RenderTargets)
			//	{
			//		InArchive << Blend.bBlendEnable;
			//		InArchive << static_cast<std::underlying_type<EBlendOperation>::type>(Blend.ColorBlendOp);
			//		InArchive << static_cast<std::underlying_type<EBlendFactor>::type>(Blend.ColorSrcBlend);
			//		InArchive << static_cast<std::underlying_type<EBlendFactor>::type>(Blend.ColorDestBlend);
			//		InArchive << static_cast<std::underlying_type<EBlendOperation>::type>(Blend.AlphaBlendOp);
			//		InArchive << static_cast<std::underlying_type<EBlendFactor>::type>(Blend.AlphaSrcBlend);
			//		InArchive << static_cast<std::underlying_type<EBlendFactor>::type>(Blend.AlphaDestBlend);
			//		InArchive << static_cast<std::underlying_type<EColorWriteMask>::type>(Blend.ColorWriteMask);
			//		InArchive << static_cast<std::underlying_type<EStateStatus>::type>(Blend.StateStatus);
			//	}
			//}
			//// PrimitiveType
			//{
			//	InArchive << static_cast<std::underlying_type<PrimitiveType>::type>(pPipelineDesc.PrimitiveTopologyType);
			//}
		}
	};

	MaterialAsset()
		: DNGEAsset(DNGEAssetType::Material)
		, MaterialType(EMaterialType::Actor)
		, MaterialUsage(EMaterialUsage::BeforPostProcess)
		, BlendMode(EMaterialBlendMode::Opaque)
	{};

	std::string Name;
	std::string Path;

	EMaterialType MaterialType;
	EMaterialUsage MaterialUsage;
	EMaterialBlendMode BlendMode;

	PipelineAsset mPipelineAsset;

	void Write() const
	{
		/*Archive archive(Path, false);
		archive << static_cast<std::underlying_type<DNGEAssetType>::type>(Type);

		archive << Name;
		archive << Path;

		archive << static_cast<std::underlying_type<EMaterialType>::type>(MaterialType);
		archive << static_cast<std::underlying_type<EMaterialUsage>::type>(MaterialUsage);
		archive << static_cast<std::underlying_type<EMaterialBlendMode>::type>(BlendMode);

		mPipelineAsset.Write(archive);*/
	}

	void Read(std::string InPath, bool InSkipType = false)
	{
		/*Archive archive(InPath);
		Type = DNGEAssetType::Mesh;

		if (InSkipType)
		{
			int A = 0;
			archive >> A;
		}

		archive >> Name;
		archive >> Path;*/
	}
};

struct MaterialInstanceAsset : public DNGEAsset
{
	struct CTexture
	{
		CTexture()
			: Slot(0)
		{}

		uint32_t Slot;
		std::wstring Location;
	};
	MaterialInstanceAsset()
		: DNGEAsset(DNGEAssetType::MaterialInstance)
		, TextureSize(0)
	{}

	std::string Name;
	std::string ParentName;
	std::string Path;

	size_t TextureSize;
	std::vector<CTexture> TextureLocations;

	void Write() const
	{
		/*Archive archive(Path, false);
		archive << static_cast<std::underlying_type<DNGEAssetType>::type>(Type);

		archive << Name;
		archive << ParentName;
		archive << Path;

		archive << TextureSize;

		for (auto& Texture : TextureLocations)
		{
			archive << Texture.Slot;
			archive << Texture.Location;
		}*/
	}
};

struct MapAsset
{
	struct MapMaterialData
	{
		struct MapMaterialInstanceData
		{
			MapMaterialInstanceData()
			{}
			std::string Name;
			std::string Path;
		};
		struct MaterialData
		{
			MaterialData()
				: InstanceSize(0)
			{}
			std::string Name;
			std::string Path;
			std::uint32_t InstanceSize;
			std::vector<MapMaterialInstanceData> Instances;
		};
		MapMaterialData()
			: MaterialSize(0)
		{}

		mutable std::uint32_t MaterialSize;
		std::vector<MaterialData> Materials;

		void Write(Archive& archive) const
		{
			/*MaterialSize = static_cast<std::uint32_t>(Materials.size());
			archive << MaterialSize;
			for (auto& pMaterial : Materials)
			{
				archive << pMaterial.Name;
				archive << pMaterial.Path;
				archive << static_cast<std::uint32_t>(pMaterial.Instances.size());
				for (auto& Instance : pMaterial.Instances)
				{
					archive << Instance.Name;
					archive << Instance.Path;
				}
			}*/
		}
	};

	struct MapActorData
	{
		struct MapActorComponentData
		{
			MapActorComponentData()
				//: CreationType(EComponentCreationType::Asset)
				//, Type(EComponentType::Mesh)
			{}
			std::string Name;
			Transform ComponentTransform;
			//EComponentType Type;
			//EComponentCreationType CreationType;
			std::string Path;
		};
		struct ActorData
		{
			ActorData()
				: ComponentSize(0)
			{}
			std::string Name;
			Transform ActorTransform;
			std::uint32_t ComponentSize;
			std::vector<MapActorComponentData> Components;
		};
		MapActorData()
			: ActorSize(0)
		{}

		mutable std::uint32_t ActorSize;
		std::vector<ActorData> Actors;

		void Write(Archive& archive) const
		{
			//ActorSize = static_cast<std::uint32_t>(Actors.size());
			//archive << ActorSize;
			//for (auto& pActor : Actors)
			//{
			//	archive << pActor.Name;

			//	// Transform
			//	FVector4 Rotation = pActor.ActorTransform.GetRotation();
			//	archive << Rotation;
			//	archive << pActor.ActorTransform.GetTranslation();
			//	archive << pActor.ActorTransform.GetScale3D();

			//	archive << static_cast<std::uint32_t>(pActor.Components.size());
			//	for (auto& Component : pActor.Components)
			//	{
			//		archive << Component.Name;

			//		// Transform
			//		FVector4 Rotation = Component.ComponentTransform.GetRotation();
			//		DirectX::XMStoreFloat4(&Rotation, Component.ComponentTransform.GetRotation());
			//		archive << Rotation;
			//		archive << Component.ComponentTransform.GetTranslation();
			//		archive << Component.ComponentTransform.GetScale3D();

			//		//archive << static_cast<std::underlying_type<EComponentType>::type>(Component.Type);
			//		//archive << static_cast<std::underlying_type<EComponentCreationType>::type>(Component.CreationType);
			//		archive << Component.Path;
			//	}
			//}
		}
	};

	MapAsset()
		: Name("")
		, Path("")
	{}

	std::string Name;
	std::string Path;

	MapMaterialData mMaterialData;
	MapActorData mActorData;

	CameraAsset mCameraAsset;

	void Write() const
	{
		/*Archive archive(Path, false);

		archive << Name;
		archive << Path;
		mMaterialData.Write(archive);
		mActorData.Write(archive);
		mCameraAsset.Write(archive);*/
	}
};
