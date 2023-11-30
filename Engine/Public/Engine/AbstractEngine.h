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

#include <memory>
#include <iostream>
#include <vector>
#include <string>
#include <optional>
#include <mutex>
#include "Core/Math/CoreMath.h"
#include "Engine/ClassBody.h"
#include "AbstractEngineUtilities.h"
#include "Core/Archive.h"

class IFrameBuffer;
class IGraphicsCommandContext;

enum class ERenderPass
{
	eNONE,
	eGBuffer,
	ePostProcess,
	eUI,
};

enum class EGITypes
{
	eD3D11,
	eD3D12,
	eVulkan,
	/* Unsuported */
	//eOpenGL46,
};

enum class EFormat
{
	UNKNOWN,
	R8_UINT,
	R8_UNORM,
	R8_SNORM,
	RG8_UINT,
	RG8_UNORM,
	R16_UINT,
	R16_UNORM,
	R16_FLOAT,
	R16_Typeless,
	RGBA8_UNORM,
	BGRA8_UNORM,
	BGRA8_TYPELESS,
	BGRA8_UNORM_SRGB,
	SRGBA8_UNORM,
	R10G10B10A2_UNORM,
	R11G11B10_FLOAT,
	RG16_UINT,
	RG16_FLOAT,
	R32_UINT,
	R32_FLOAT,
	R32_Typeless,
	RGBA8_UINT,
	RGBA16_FLOAT,
	RGBA16_UINT,
	RGBA16_UNORM,
	RGBA16_SNORM,
	RG32_UINT,
	RG32_FLOAT,
	RGB32_UINT,
	RGB32_FLOAT,
	RGBA32_UINT,
	RGBA32_FLOAT,
	R24G8_Typeless,
	R32G8X24_Typeless, 
	D16_UNORM,
	D32_FLOAT,
	D24_UNORM_S8_UINT,
	D32_FLOAT_S8X24_UINT,
	BC1_UNORM,
	BC1_UNORM_SRGB,
	BC2_UNORM,
	BC2_UNORM_SRGB,
	BC3_UNORM,
	BC3_UNORM_SRGB,
};

namespace
{
	inline bool IsValidDepthFormat(const EFormat Format)
	{
		return Format == EFormat::R16_Typeless || Format == EFormat::R32_Typeless || Format == EFormat::R24G8_Typeless || Format == EFormat::R32G8X24_Typeless;
	};

	inline bool IsDepthSRVSupported(const EFormat Format)
	{
		return Format == EFormat::R16_Typeless || Format == EFormat::R32_Typeless;
	};
}

enum class eShaderType : std::uint8_t
{
	Vertex,
	Pixel,
	Geometry,
	Compute,
	HULL,
	Domain,
	Mesh,
	Amplification,
};

struct sScreenDimension
{
	std::size_t Width = 0;
	std::size_t Height = 0;
	sScreenDimension() = default;
	sScreenDimension(std::size_t InWidth, std::size_t InHeight)
		: Width(InWidth)
		, Height(InHeight)
	{}

#if _MSVC_LANG >= 202002L
	constexpr auto operator<=>(const sScreenDimension&) const = default;
#endif
};

#if _MSVC_LANG < 202002L
FORCEINLINE bool constexpr operator ==(const sScreenDimension& value1, const sScreenDimension& value2)
{
	return (value1.Width == value2.Width && value1.Height == value2.Height);
};

FORCEINLINE bool constexpr operator !=(const sScreenDimension& value1, const sScreenDimension& value2)
{
	return !((value1.Width == value2.Width) && (value1.Height == value2.Height));
};
#endif

struct sViewport
{
	std::uint32_t Width = 0;
	std::uint32_t Height = 0;
	std::uint32_t TopLeftX = 0;
	std::uint32_t TopLeftY = 0;
	float MinDepth = 0.0f;
	float MaxDepth = 1.0f;

	sViewport() = default;
	sViewport(const std::uint32_t InWidth, const std::uint32_t InHeight, const std::uint32_t InTopLeftX = 0.0f, 
			  const std::uint32_t InTopLeftY = 0.0f, const float InMinDepth = 0.0f, const float InMaxDepth = 1.0f)
		: Width(InWidth)
		, Height(InHeight)
		, TopLeftX(InTopLeftX)
		, TopLeftY(InTopLeftY)
		, MinDepth(InMinDepth)
		, MaxDepth(InMaxDepth)
	{}
	sViewport(const sScreenDimension& dimension )
		: TopLeftX(0)
		, TopLeftY(0)
		, Width((std::uint32_t)dimension.Width)
		, Height((std::uint32_t)dimension.Height)
		, MinDepth(0.0f)
		, MaxDepth(1.0f)
	{}
};

class sCamera;
class ICanvas;

struct sViewportInstance
{
	sBaseClassBody(sClassConstructor, sViewportInstance)

	sViewportInstance() = default;
	inline ~sViewportInstance();

	bool bIsEnabled = true;
	std::optional<sViewport> Viewport = std::nullopt;
	std::shared_ptr<sCamera> pCamera = nullptr;
	std::vector<ICanvas*> Canvases;
};

struct sDisplayMode
{
	struct sRefreshRate
	{
		std::uint32_t Numerator = 0;
		std::uint32_t Denominator = 0;
	};

	std::uint32_t Width = 0;
	std::uint32_t Height = 0;
	sRefreshRate RefreshRate;
};

struct sVertexBufferEntry
{
	FVector position;
	FVector normal;
	FVector2 texCoord;
	FColor Color;
	FVector tangent;
	FVector binormal;
	std::uint32_t ArrayIndex;

	sVertexBufferEntry()
		: position(FVector::Zero())
		, normal(FVector::Zero())
		, texCoord(FVector2::Zero())
		, Color(FColor::White())
		, tangent(FVector::Zero())
		, binormal(FVector::Zero())
		, ArrayIndex(NULL)
	{}
};

struct sVertexAttributeDesc
{
	std::string name;
	EFormat format;
	uint32_t bufferIndex;
	uint32_t offset;
	bool isInstanced;
};

struct sObjectDrawParameters
{
	std::uint32_t IndexCountPerInstance;
	std::uint32_t InstanceCount;
	std::uint32_t StartIndexLocation;
	std::int32_t BaseVertexLocation;
	std::uint32_t StartInstanceLocation;

	explicit sObjectDrawParameters()
		: IndexCountPerInstance(NULL)
		, InstanceCount(1)
		, StartIndexLocation(NULL)
		, BaseVertexLocation(NULL)
		, StartInstanceLocation(NULL)
	{}
};

struct sMeshData
{
	std::vector<std::uint32_t> Indices;
	std::vector<sVertexBufferEntry> Vertices;
	sObjectDrawParameters DrawParameters;
};

struct sBufferDesc
{
	std::uint64_t Size;
	std::uint64_t Stride;

	explicit sBufferDesc()
		: Size(NULL)
		, Stride(NULL)
	{}

	sBufferDesc(std::uint64_t InBufferSize, std::uint64_t InStride = NULL)
		: Size(InBufferSize)
		, Stride(InStride)
	{}

	~sBufferDesc()
	{
		Size = NULL;
		Stride = NULL;
	}
};

struct sBufferSubresource
{
	void* pSysMem;
	std::size_t Size;
	std::size_t Location;

	constexpr sBufferSubresource(void* InSysMem = nullptr, std::size_t InSize = NULL, std::size_t InLocation = NULL)
		: pSysMem(InSysMem)
		, Size(InSize)
		, Location(InLocation)
	{}
};

_declspec(align(256)) struct sMeshConstantBufferAttributes
{
	FMatrix modelMatrix;
	FMatrix PrevModelMatrix;

	sMeshConstantBufferAttributes()
		: modelMatrix(FMatrix::Identity())
		, PrevModelMatrix(FMatrix::Identity())
	{}
};

static_assert((sizeof(sMeshConstantBufferAttributes) % 16) == 0, "CB size not padded correctly");

class IConstantBuffer
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IConstantBuffer)
public:
	static IConstantBuffer::SharedPtr Create(std::string InName, const sBufferDesc& InDesc, std::uint32_t InRootParameterIndex);
	static IConstantBuffer::UniquePtr CreateUnique(std::string InName, const sBufferDesc& InDesc, std::uint32_t InRootParameterIndex);

public:
	virtual std::string GetName() const = 0;
	virtual void SetDefaultRootParameterIndex(std::uint32_t RootParameterIndex) = 0;
	virtual std::uint32_t GetDefaultRootParameterIndex() const = 0;
	virtual void Map(const void* Ptr, IGraphicsCommandContext* InCMDBuffer = nullptr) = 0;
};

class IVertexBuffer
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IVertexBuffer)
public:
	static IVertexBuffer::SharedPtr Create(std::string InName, const sBufferDesc& InDesc, sBufferSubresource* InSubresource = nullptr);
	static IVertexBuffer::UniquePtr CreateUnique(std::string InName, const sBufferDesc& InDesc, sBufferSubresource* InSubresource = nullptr);

public:
	virtual std::string GetName() const = 0;
	virtual std::size_t GetSize() const = 0;
	virtual bool IsMapable() const = 0;
	virtual void UpdateSubresource(sBufferSubresource* Subresource, IGraphicsCommandContext* InCMDBuffer = nullptr) = 0;
};

class IIndexBuffer
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IIndexBuffer)
public:
	static IIndexBuffer::SharedPtr Create(std::string InName, const sBufferDesc& InDesc, sBufferSubresource* InSubresource = nullptr);
	static IIndexBuffer::UniquePtr CreateUnique(std::string InName, const sBufferDesc& InDesc, sBufferSubresource* InSubresource = nullptr);

public:
	virtual std::string GetName() const = 0;
	virtual std::size_t GetSize() const = 0;
	virtual bool IsMapable() const = 0;
	virtual void UpdateSubresource(sBufferSubresource* Subresource, IGraphicsCommandContext* InCMDBuffer = nullptr) = 0;
};

class IUnorderedAccessBuffer
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IUnorderedAccessBuffer)
public:
	static IUnorderedAccessBuffer::SharedPtr Create(std::string InName, const sBufferDesc& InDesc, bool bSRVAllowed = true);
	static IUnorderedAccessBuffer::UniquePtr CreateUnique(std::string InName, const sBufferDesc& InDesc, bool bSRVAllowed = true);

public:
	virtual std::string GetName() const = 0;

	virtual bool IsSRV_Allowed() const = 0;
	virtual std::size_t GetSize() const = 0;

	virtual bool IsMapable() const = 0;
	virtual void Map(const void* Ptr, IGraphicsCommandContext* InCMDBuffer = nullptr) = 0;
};

class IIndirectBuffer
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IIndirectBuffer)
public:
	static IIndirectBuffer::SharedPtr Create(std::string InName);
	static IIndirectBuffer::UniquePtr CreateUnique(std::string InName);

public:

};

struct sFBODesc
{
	struct sFBODimension
	{
		std::uint32_t X;
		std::uint32_t Y;
		sFBODimension(std::uint32_t x = NULL, std::uint32_t y = NULL)
			: X(x)
			, Y(y)
		{}
	};
	struct sMSLevel
	{
		std::uint32_t Count;
		std::uint32_t Quality;
		sMSLevel()
			: Count(1)
			, Quality(NULL)
		{}
	};
	struct sArraySize
	{
		std::uint32_t Size;
		bool bIsArray;
		sArraySize()
			: Size(1)
			, bIsArray(false)
		{}
	};

	sFBODimension Dimensions;
	//sArraySize Array;
	sMSLevel MSLevel;

	sFBODesc() = default;
	sFBODesc(sFBODimension InDimensions)
		: Dimensions(InDimensions)
	{}
};

enum class eFrameBufferAttachmentType
{
	eRT,
	eRT_SRV,
	eRT_UAV,
	eRT_SRV_UAV,
	eDepth,
	eDepth_SRV,
	eDepth_UAV,
	eDepth_SRV_UAV,
	eUAV,
	eUAV_SRV
};

struct sFrameBufferAttachmentInfo
{
	struct sFrameBuffer
	{
		EFormat Format;
		eFrameBufferAttachmentType AttachmentType;
		sFrameBuffer(const EFormat& InFormat, const eFrameBufferAttachmentType InAttachmentType = eFrameBufferAttachmentType::eRT_SRV)
			: Format(InFormat)
			, AttachmentType(InAttachmentType)
		{}
	};

	std::vector<sFrameBuffer> FrameBuffer;
	EFormat DepthFormat;
	sFBODesc Desc;
	std::size_t PrimaryFB;

	sFrameBufferAttachmentInfo()
		: DepthFormat(EFormat::UNKNOWN)
		, Desc(sFBODesc())
		, PrimaryFB(0)
	{}

	std::size_t GetAttachmentCount() const { return FrameBuffer.size() + (DepthFormat != EFormat::UNKNOWN ? 1 : 0); }
	std::size_t GetRenderTargetAttachmentCount() const { return FrameBuffer.size(); }
	void AddFrameBuffer(const EFormat Format, const eFrameBufferAttachmentType InAttachmentType = eFrameBufferAttachmentType::eRT_SRV)
	{
		FrameBuffer.push_back(sFrameBuffer(Format, InAttachmentType));
	}
};

class IUnorderedAccessTarget;
class IRenderTarget
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IRenderTarget)
public:
	static IRenderTarget::SharedPtr Create(const std::string InName, const EFormat Format, const sFBODesc& Desc);
	static IRenderTarget::UniquePtr CreateUnique(const std::string InName, const EFormat Format, const sFBODesc& Desc);

public:
	virtual bool IsSRV_Allowed() const = 0;
	virtual bool IsUAV_Allowed() const = 0;

	virtual void* GetNativeTexture() const = 0;

	virtual void SetDefaultRootParameterIndex(std::uint32_t RootParameterIndex) = 0;
	virtual std::uint32_t GetDefaultRootParameterIndex() const = 0;
};

class IDepthTarget
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IDepthTarget)
public:
	static IDepthTarget::SharedPtr Create(const std::string InName, const EFormat Format, const sFBODesc& Desc);
	static IDepthTarget::UniquePtr CreateUnique(const std::string InName, const EFormat Format, const sFBODesc& Desc);

public:
	virtual bool IsSRV_Allowed() const = 0;
	virtual bool IsUAV_Allowed() const = 0;

	virtual void* GetNativeTexture() const = 0;

	virtual void SetDefaultRootParameterIndex(std::uint32_t RootParameterIndex) = 0;
	virtual std::uint32_t GetDefaultRootParameterIndex() const = 0;
};

class IUnorderedAccessTarget
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IUnorderedAccessTarget)
public:
	static IUnorderedAccessTarget::SharedPtr Create(const std::string InName, const EFormat Format, const sFBODesc& Desc, bool InEnableSRV = true);
	static IUnorderedAccessTarget::UniquePtr CreateUnique(const std::string InName, const EFormat Format, const sFBODesc& Desc, bool InEnableSRV = true);

public:
	virtual bool IsSRV_Allowed() const = 0;

	virtual void* GetNativeTexture() const = 0;

	virtual void SetDefaultRootParameterIndex(std::uint32_t RootParameterIndex) = 0;
	virtual std::uint32_t GetDefaultRootParameterIndex() const = 0;
};

class IFrameBuffer
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IFrameBuffer)
public:
	static IFrameBuffer::SharedPtr Create(const std::string InName, const sFrameBufferAttachmentInfo& InAttachments);
	static IFrameBuffer::UniquePtr CreateUnique(const std::string InName, const sFrameBufferAttachmentInfo& InAttachments);

public:
	virtual std::string GetName() const = 0;
	virtual sFrameBufferAttachmentInfo GetAttachmentInfo() const = 0;

	virtual std::vector<IRenderTarget*> GetRenderTargets() const = 0;
	virtual IRenderTarget* GetRenderTarget(std::size_t Index) const = 0;
	virtual void AttachRenderTarget(const IRenderTarget::SharedPtr& RenderTarget, std::optional<std::size_t> Index = std::nullopt) = 0;

	virtual std::vector<IUnorderedAccessTarget*> GetUnorderedAccessTargets() const = 0;
	virtual IUnorderedAccessTarget* GetUnorderedAccessTarget(std::size_t Index) const = 0;
	virtual void AttachUnorderedAccessTarget(const IUnorderedAccessTarget::SharedPtr& UnorderedAccessTarget, std::optional<std::size_t> Index = std::nullopt) = 0;

	virtual IDepthTarget* GetDepthTarget() const = 0;
	virtual void SetDepthTarget(const IDepthTarget::SharedPtr& DepthTarget) = 0;

	virtual std::size_t GetAttachmentCount() const = 0;
	virtual std::size_t GetRenderTargetAttachmentCount() const = 0;
};

enum class ERasterizerCullMode
{
	eNone,
	eCW,
	eCCW,
};

enum class ERasterizerFillMode
{
	ePoint,
	eWireframe,
	eSolid,
};

struct sRasterizerAttributeDesc
{
	ERasterizerFillMode FillMode;
	ERasterizerCullMode CullMode;
	float DepthBias;
	float DepthBiasClamp;
	float SlopeScaleDepthBias;
	bool DepthClipEnable;
	bool bAllowMSAA;
	bool bEnableLineAA;

	bool FrontCounterClockwise;

	sRasterizerAttributeDesc(ERasterizerCullMode InMode = ERasterizerCullMode::eCW)
		: FillMode(ERasterizerFillMode::eSolid)
		, CullMode(InMode)
		, DepthBias(0)
		, DepthBiasClamp(0.0f)
		, DepthClipEnable(true)
		, SlopeScaleDepthBias(0.0f)
		, bAllowMSAA(false)
		, bEnableLineAA(false)
		, FrontCounterClockwise(false)
	{}

	sRasterizerAttributeDesc(ERasterizerFillMode InMode)
		: FillMode(InMode)
		, CullMode(InMode == ERasterizerFillMode::eWireframe ? ERasterizerCullMode::eNone : ERasterizerCullMode::eCW)
		, DepthBias(0)
		, DepthBiasClamp(0.0f)
		, DepthClipEnable(true)
		, SlopeScaleDepthBias(0.0f)
		, bAllowMSAA(false)
		, bEnableLineAA(false)
		, FrontCounterClockwise(false)
	{}
};

enum class ECompareFunction
{
	eLess,
	eLessEqual,
	eGreater,
	eGreaterEqual,
	eEqual,
	eNotEqual,
	eNever,
	eAlways,
};

enum class ESamplerFilter
{
	ePoint,
	eBilinear,
	eTrilinear,
	eAnisotropicPoint,
	eAnisotropicLinear,
};

enum class ESamplerAddressMode
{
	eWrap,
	eClamp,
	eMirror,
	eBorder,
};

enum class ESamplerStateMode
{
	ePointWrap,
	ePointClamp,
	ePointBorder,
	eLinearWrap,
	eLinearClamp,
	eAnisotropicWrap,
	eAnisotropicClamp,
	eAnisotropicLinear,
};

struct sSamplerAttributeDesc
{
	sSamplerAttributeDesc(ESamplerStateMode InMode = ESamplerStateMode::ePointWrap)
		: Filter(ESamplerFilter::ePoint)
		, AddressU(ESamplerAddressMode::eClamp)
		, AddressV(ESamplerAddressMode::eClamp)
		, AddressW(ESamplerAddressMode::eClamp)
		, MipBias(0)
		, MinMipLevel(0)
		, MaxMipLevel(FLT_MAX)
		, MaxAnisotropy(1)
		, BorderColor(FColor::White())
		, bUINTBorderColor(false)
		, SamplerComparisonFunction(ECompareFunction::eNever)
	{
		switch (InMode)
		{
		case ESamplerStateMode::ePointWrap:
			Filter = ESamplerFilter::ePoint;
			AddressU = ESamplerAddressMode::eWrap;
			AddressV = ESamplerAddressMode::eWrap;
			AddressW = ESamplerAddressMode::eWrap;
			break;
		case ESamplerStateMode::ePointClamp:
			Filter = ESamplerFilter::ePoint;
			AddressU = ESamplerAddressMode::eClamp;
			AddressV = ESamplerAddressMode::eClamp;
			AddressW = ESamplerAddressMode::eClamp;
			break;
		case ESamplerStateMode::ePointBorder:
			Filter = ESamplerFilter::ePoint;
			AddressU = ESamplerAddressMode::eBorder;
			AddressV = ESamplerAddressMode::eBorder;
			AddressW = ESamplerAddressMode::eBorder;
			break;
		case ESamplerStateMode::eLinearWrap:
			Filter = ESamplerFilter::eBilinear;
			AddressU = ESamplerAddressMode::eWrap;
			AddressV = ESamplerAddressMode::eWrap;
			AddressW = ESamplerAddressMode::eWrap;
			break;
		case ESamplerStateMode::eLinearClamp:
			Filter = ESamplerFilter::eBilinear;
			AddressU = ESamplerAddressMode::eClamp;
			AddressV = ESamplerAddressMode::eClamp;
			AddressW = ESamplerAddressMode::eClamp;
			break;
		case ESamplerStateMode::eAnisotropicWrap:
			Filter = ESamplerFilter::eAnisotropicPoint;
			AddressU = ESamplerAddressMode::eWrap;
			AddressV = ESamplerAddressMode::eWrap;
			AddressW = ESamplerAddressMode::eWrap;
			break;
		case ESamplerStateMode::eAnisotropicClamp:
			Filter = ESamplerFilter::eAnisotropicPoint;
			AddressU = ESamplerAddressMode::eClamp;
			AddressV = ESamplerAddressMode::eClamp;
			AddressW = ESamplerAddressMode::eClamp;
			break;
		case ESamplerStateMode::eAnisotropicLinear:
			Filter = ESamplerFilter::eAnisotropicLinear;
			AddressU = ESamplerAddressMode::eWrap;
			AddressV = ESamplerAddressMode::eWrap;
			AddressW = ESamplerAddressMode::eWrap;
			SamplerComparisonFunction = ECompareFunction::eLess;
			break;
		}
	}

	void SetToAddressToWrap()
	{
		AddressU = ESamplerAddressMode::eWrap;
		AddressV = ESamplerAddressMode::eWrap;
		AddressW = ESamplerAddressMode::eWrap;
	}

	void SetToAddressToClamp()
	{
		AddressU = ESamplerAddressMode::eClamp;
		AddressV = ESamplerAddressMode::eClamp;
		AddressW = ESamplerAddressMode::eClamp;
	}

	ESamplerFilter Filter;
	ESamplerAddressMode AddressU;
	ESamplerAddressMode AddressV;
	ESamplerAddressMode AddressW;
	int MipBias;
	float MinMipLevel;
	float MaxMipLevel;
	int MaxAnisotropy;
	FColor BorderColor;
	bool bUINTBorderColor;
	ECompareFunction SamplerComparisonFunction;
};

enum class EStencilOp
{
	eKeep,
	eZero,
	eReplace,
	eSaturatedIncrement,
	eSaturatedDecrement,
	eInvert,
	eIncrement,
	eDecrement,
};

struct sDepthStencilAttributeDesc
{
	bool bEnableDepthWrite;
	bool bDepthWriteMask;
	ECompareFunction DepthTest;
	bool bStencilEnable;

	bool bEnableFrontFaceStencil;
	ECompareFunction FrontFaceStencilTest;
	EStencilOp FrontFaceStencilFailStencilOp;
	EStencilOp FrontFaceDepthFailStencilOp;
	EStencilOp FrontFacePassStencilOp;
	bool bEnableBackFaceStencil;
	ECompareFunction BackFaceStencilTest;
	EStencilOp BackFaceStencilFailStencilOp;
	EStencilOp BackFaceDepthFailStencilOp;
	EStencilOp BackFacePassStencilOp;
	unsigned __int8 StencilReadMask;
	unsigned __int8 StencilWriteMask;

	sDepthStencilAttributeDesc(bool DepthWrite = true, bool StencilEnable = false)
		: bEnableDepthWrite(DepthWrite)
		, bStencilEnable(StencilEnable)
		, bDepthWriteMask(true)
		, DepthTest(ECompareFunction::eLessEqual)
		, bEnableFrontFaceStencil(false)
		, FrontFaceStencilTest(ECompareFunction::eAlways)
		, FrontFaceStencilFailStencilOp(EStencilOp::eKeep)
		, FrontFaceDepthFailStencilOp(EStencilOp::eKeep)
		, FrontFacePassStencilOp(EStencilOp::eKeep)
		, bEnableBackFaceStencil(false)
		, BackFaceStencilTest(ECompareFunction::eAlways)
		, BackFaceStencilFailStencilOp(EStencilOp::eKeep)
		, BackFaceDepthFailStencilOp(EStencilOp::eKeep)
		, BackFacePassStencilOp(EStencilOp::eKeep)
		, StencilReadMask(0xFF)
		, StencilWriteMask(0xFF)
	{}
};

enum class EBlendFactor
{
	eZero,
	eOne,
	eSourceColor,
	eInverseSourceColor,
	eSourceAlpha,
	eInverseSourceAlpha,
	eDestAlpha,
	eInverseDestAlpha,
	eDestColor,
	eInverseDestColor,
	eBlendFactor,
	eInverseBlendFactor,
};

enum class EColorWriteMask
{
	eNONE = 0,
	eRED = 0x01,
	eGREEN = 0x02,
	eBLUE = 0x04,
	eALPHA = 0x08,

	eRGB = eRED | eGREEN | eBLUE,
	eRGBA = eRED | eGREEN | eBLUE | eALPHA,
	eRG = eRED | eGREEN,
	eBA = eBLUE | eALPHA,
};

enum class EBlendOperation
{
	eAdd,
	eSubtract,
	eMin,
	eMax,
	eReverseSubtract,
};

enum class EBlendStateMode
{
	eOpaque,
	eAlphaBlend,
	eAdditive,
	eNonPremultiplied,
};

struct sBlendAttributeDesc
{
public:
	enum { MAX_BLEND_COUNT = 8 };

	struct BlendTarget
	{
		bool bBlendEnable;
		EBlendOperation ColorBlendOp;
		EBlendFactor ColorSrcBlend;
		EBlendFactor ColorDestBlend;
		EBlendOperation AlphaBlendOp;
		EBlendFactor AlphaSrcBlend;
		EBlendFactor AlphaDestBlend;
		EColorWriteMask ColorWriteMask;

		BlendTarget(
			bool InBlendEnable = false,
			EBlendOperation InColorBlendOp = EBlendOperation::eAdd,
			EBlendFactor InColorSrcBlend = EBlendFactor::eOne,
			EBlendFactor InColorDestBlend = EBlendFactor::eZero,
			EBlendOperation InAlphaBlendOp = EBlendOperation::eAdd,
			EBlendFactor InAlphaSrcBlend = EBlendFactor::eOne,
			EBlendFactor InAlphaDestBlend = EBlendFactor::eZero,
			EColorWriteMask InColorWriteMask = EColorWriteMask::eRGBA
		)
			: bBlendEnable(InBlendEnable)
			, ColorBlendOp(InColorBlendOp)
			, ColorSrcBlend(InColorSrcBlend)
			, ColorDestBlend(InColorDestBlend)
			, AlphaBlendOp(InAlphaBlendOp)
			, AlphaSrcBlend(InAlphaSrcBlend)
			, AlphaDestBlend(InAlphaDestBlend)
			, ColorWriteMask(InColorWriteMask)
		{}

		BlendTarget(EBlendStateMode InMode)
			: bBlendEnable(true)
			, ColorBlendOp(EBlendOperation::eAdd)
			, ColorSrcBlend(EBlendFactor::eOne)
			, ColorDestBlend(EBlendFactor::eZero)
			, AlphaBlendOp(EBlendOperation::eAdd)
			, AlphaSrcBlend(EBlendFactor::eOne)
			, AlphaDestBlend(EBlendFactor::eZero)
			, ColorWriteMask(EColorWriteMask::eRGBA)
		{
			switch (InMode)
			{
			case EBlendStateMode::eOpaque:
				ColorSrcBlend = EBlendFactor::eOne;
				ColorDestBlend = EBlendFactor::eZero;
				break;
			case EBlendStateMode::eAlphaBlend:
				ColorSrcBlend = EBlendFactor::eOne;
				ColorDestBlend = EBlendFactor::eInverseSourceAlpha;
				break;
			case EBlendStateMode::eAdditive:
				ColorSrcBlend = EBlendFactor::eSourceAlpha;
				ColorDestBlend = EBlendFactor::eOne;
				break;
			case EBlendStateMode::eNonPremultiplied:
				ColorSrcBlend = EBlendFactor::eSourceAlpha;
				ColorDestBlend = EBlendFactor::eInverseSourceAlpha;
				AlphaDestBlend = EBlendFactor::eSourceAlpha;
				break;
			}
		}
	};

	sBlendAttributeDesc()
		: bUseIndependentRenderTargetBlendStates(false)
		, balphaToCoverage(false)
	{}

	sBlendAttributeDesc(EBlendStateMode InMode)
		: bUseIndependentRenderTargetBlendStates(false)
		, balphaToCoverage(false)
	{
		for (auto& RT : RenderTargets)
		{
			RT = BlendTarget(InMode);
		}
	}

	sBlendAttributeDesc(const BlendTarget& InRenderTargetBlendState)
		: bUseIndependentRenderTargetBlendStates(false)
		, balphaToCoverage(false)
	{
		RenderTargets[0] = InRenderTargetBlendState;
	}

	std::array<BlendTarget, MAX_BLEND_COUNT> RenderTargets;
	bool bUseIndependentRenderTargetBlendStates = false;
	bool balphaToCoverage = false;
};

enum class EPrimitiveType : std::uint8_t
{
	ePOINT_LIST = 0x0,
	eTRIANGLE_LIST = 0x1,
	eTRIANGLE_STRIP = 0x2,
	eLINE_LIST = 0x4,
};

enum class EDescriptorType : std::uint8_t
{
	eUniformBuffer,
	eTexture,
	eSampler,
	eUAV,
	//eConstant,
};

struct sDescriptorSetLayoutBinding
{
private:
	EDescriptorType DescriptorType;
	std::optional<sSamplerAttributeDesc> SamplerDesc;

public:
	std::uint32_t Location;
	eShaderType ShaderType;
	std::uint32_t Size;
	std::uint32_t RegisterSpace;

	sDescriptorSetLayoutBinding(EDescriptorType InType, eShaderType InShaderType, std::uint32_t InLocation, std::uint32_t InSize = 1, std::uint32_t InRegisterSpace = 0)
		: Location(InLocation)
		, DescriptorType(InType)
		, ShaderType(InShaderType)
		, Size(InSize)
		, RegisterSpace(InRegisterSpace)
		, SamplerDesc(std::nullopt)
	{}
	sDescriptorSetLayoutBinding(sSamplerAttributeDesc Sampler, eShaderType InShaderType, std::uint32_t InLocation, std::uint32_t InSize = 1, std::uint32_t InRegisterSpace = 0)
		: Location(InLocation)
		, DescriptorType(EDescriptorType::eSampler)
		, ShaderType(InShaderType)
		, Size(InSize)
		, RegisterSpace(InRegisterSpace)
		, SamplerDesc(Sampler)
	{}

	EDescriptorType GetDescriptorType() const { return DescriptorType; }
	std::optional<sSamplerAttributeDesc> GetSamplerDesc() const { return SamplerDesc; }
};

struct sShaderDefines
{
	std::string Name;
	std::string Definition;

	sShaderDefines() = default;
	sShaderDefines(std::string InName, std::string InDefinition)
		: Name(InName)
		, Definition(InDefinition)
	{}
};

enum class EShaderAttachmentType
{
	eFile,
	eMemory,
};

struct sShaderAttachment
{
private:
	std::wstring Location;
	void* ByteCode;
	std::size_t Size;
	EShaderAttachmentType ShaderAttachmentType;

public:
	std::string FunctionName;
	eShaderType Type;
	std::vector<sShaderDefines> ShaderDefines;

	sShaderAttachment(std::wstring InLocation, std::string inEntryName, eShaderType InType, const std::vector<sShaderDefines> InShaderDefines = std::vector<sShaderDefines>())
		: Location(InLocation)
		, ByteCode(nullptr)
		, Size(0)
		, FunctionName(inEntryName)
		, Type(InType)
		, ShaderAttachmentType(EShaderAttachmentType::eFile)
		, ShaderDefines(InShaderDefines)
	{}
	sShaderAttachment(void* pByteCode, std::size_t inSize, std::string inEntryName, eShaderType InType, const std::vector<sShaderDefines> InShaderDefines = std::vector<sShaderDefines>())
		: Location(L"")
		, ByteCode(pByteCode)
		, Size(inSize)
		, FunctionName(inEntryName)
		, Type(InType)
		, ShaderAttachmentType(EShaderAttachmentType::eMemory)
		, ShaderDefines(InShaderDefines)
	{}

	~sShaderAttachment()
	{
		ShaderDefines.clear();
	}

	EShaderAttachmentType GetShaderAttachmentType() const { return ShaderAttachmentType; }
	std::wstring GetLocation() const { return Location; }
	void* GetByteCode() const { return ByteCode; }
	std::size_t GetByteCodeSize() const { return Size; }
};

struct ShaderByteCode
{
	void* ByteCode;
	std::uint32_t ByteCodeSize;
};

class IShader
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IShader)
public:
	static IShader::SharedPtr Create(const sShaderAttachment& Attachment);
	static IShader::SharedPtr Create(std::wstring InSrcFile, std::string InFunctionName, eShaderType InProfile, std::vector<sShaderDefines> InDefines = std::vector<sShaderDefines>());
	static IShader::SharedPtr Create(const void* InCode, std::size_t Size, std::string InFunctionName, eShaderType InProfile, std::vector<sShaderDefines> InDefines = std::vector<sShaderDefines>());

public:
	virtual void* GetByteCode() const = 0;
	virtual std::uint32_t GetByteCodeSize() const = 0;
};

struct sPipelineDesc
{
	sBaseClassBody(sClassNoDefaults, sPipelineDesc)
public:
	sPipelineDesc() = default;
	~sPipelineDesc()
	{
		DescriptorSetLayout.clear();
		ShaderAttachments.clear();
		VertexLayout.clear();
	}
	sRasterizerAttributeDesc RasterizerAttribute;
	sDepthStencilAttributeDesc DepthStencilAttribute;
	sBlendAttributeDesc BlendAttribute;
	EPrimitiveType PrimitiveTopologyType;

	std::vector<sDescriptorSetLayoutBinding> DescriptorSetLayout;
	std::vector<sShaderAttachment> ShaderAttachments;
	std::vector<sVertexAttributeDesc> VertexLayout;

	std::uint32_t NumRenderTargets;
	std::array<EFormat, 8> RTVFormats;
	EFormat DSVFormat;
};

class IVertexAttribute
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IVertexAttribute)
public:
	virtual void Release() = 0;
	virtual std::vector<sVertexAttributeDesc> GetVertexAttributeDesc() const = 0;
};

class IRootSignature
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IRootSignature)
public:
	virtual void Release() = 0;

	virtual std::vector<sDescriptorSetLayoutBinding> GetDescriptorSetLayout() const = 0;
};

class IPipeline
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IPipeline)
public:
	static IPipeline::SharedPtr Create(const std::string& InName, const sPipelineDesc& InDesc);
	static IPipeline::UniquePtr CreateUnique(const std::string& InName, const sPipelineDesc& InDesc);

public:
	virtual sPipelineDesc GetPipelineDesc() const = 0;
	virtual void Recompile() = 0;
};

struct sComputePipelineDesc
{
	sBaseClassBody(sClassNoDefaults, sComputePipelineDesc)
public:
	sComputePipelineDesc(const sShaderAttachment& InShaderAttachment, const std::vector<sDescriptorSetLayoutBinding>& InDescriptorSetLayout)
		: ShaderAttachment(InShaderAttachment)
		, DescriptorSetLayout(InDescriptorSetLayout)
	{}
	~sComputePipelineDesc()
	{
		DescriptorSetLayout.clear();
	}
	std::vector<sDescriptorSetLayoutBinding> DescriptorSetLayout;
	sShaderAttachment ShaderAttachment;
};

class IComputePipeline
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IComputePipeline)
public:
	static IComputePipeline::SharedPtr Create(const std::string& InName, const sComputePipelineDesc& InDesc);
	static IComputePipeline::UniquePtr CreateUnique(const std::string& InName, const sComputePipelineDesc& InDesc);

public:
	virtual sComputePipelineDesc GetPipelineDesc() const = 0;
	virtual void Recompile() = 0;
};

struct sTextureDesc
{
	struct sSize
	{
		std::uint32_t X;
		std::uint32_t Y;
		sSize()
			: X(NULL)
			, Y(NULL)
		{}
	};

	sSize Dimensions;
	std::uint32_t MipLevels;
	std::uint32_t ArraySize;
	EFormat Format;

	sTextureDesc()
		: MipLevels(NULL)
		, ArraySize(1)
		, Format(EFormat::UNKNOWN)
	{}
};

class ITexture2D
{
	sBaseClassBody(sClassDefaultProtectedConstructor, ITexture2D)
public:
	static ITexture2D::SharedPtr Create(const std::wstring FilePath, const std::string InName, std::uint32_t DefaultRootParameterIndex = 0);
	static ITexture2D::UniquePtr CreateUnique(const std::wstring FilePath, const std::string InName, std::uint32_t DefaultRootParameterIndex = 0);
	static ITexture2D::SharedPtr Create(const std::string InName, void* InBuffer, const std::size_t InSize, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex = 0);
	static ITexture2D::UniquePtr CreateUnique(const std::string InName, void* InBuffer, const std::size_t InSize, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex = 0);
	static ITexture2D::SharedPtr CreateEmpty(const std::string InName, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex = 0);
	static ITexture2D::UniquePtr CreateUniqueEmpty(const std::string InName, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex = 0);

public:
	virtual std::wstring GetPath() const = 0;
	virtual std::string GetName() const = 0;

	virtual sTextureDesc GetDesc() const = 0;

	virtual void SetDefaultRootParameterIndex(std::uint32_t RootParameterIndex) = 0;
	virtual std::uint32_t GetDefaultRootParameterIndex() const = 0;

	virtual void UpdateTexture(ITexture2D* SourceTexture, std::size_t SourceArrayIndex, std::size_t ArrayIndex, const std::optional<IntVector2> Dest = std::nullopt, const std::optional<FBounds2D> TargetBounds = std::nullopt) = 0;
	virtual void UpdateTexture(const std::wstring FilePath, std::size_t ArrayIndex, const std::optional<IntVector2> Dest = std::nullopt, const std::optional<FBounds2D> TargetBounds = std::nullopt) = 0;
	virtual void UpdateTexture(const void* pSrcData, const std::size_t InSize, const FDimension2D& Dimension, std::size_t ArrayIndex, const std::optional<IntVector2> Dest = std::nullopt, const std::optional<FBounds2D> TargetBounds = std::nullopt) = 0;

	virtual void UpdateTexture(const void* pSrcData, std::size_t RowPitch, std::size_t MinX, std::size_t MinY, std::size_t MaxX, std::size_t MaxY, IGraphicsCommandContext* InCommandBuffer = nullptr) = 0;

	virtual void SaveToFile(std::wstring InPath) const = 0;
};

typedef ITexture2D ITiledTexture;

class IGraphicsCommandContext
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IGraphicsCommandContext)
public:
	static IGraphicsCommandContext::SharedPtr Create();
	static IGraphicsCommandContext::UniquePtr CreateUnique();

public:
	virtual void BeginRecordCommandList(const ERenderPass RenderPass = ERenderPass::eNONE) = 0;
	virtual void FinishRecordCommandList() = 0;
	virtual void ExecuteCommandList() = 0;
	virtual void ClearState() = 0;

	virtual void* GetInternalCommandContext() = 0;

	virtual void SetViewport(const sViewport& Viewport) = 0;

	virtual void SetScissorRect(std::uint32_t X, std::uint32_t Y, std::uint32_t Z, std::uint32_t W) = 0;
	virtual void SetStencilRef(std::uint32_t Ref) = 0;
	virtual std::uint32_t GetStencilRef() const = 0;

	virtual void ClearFrameBuffer(IFrameBuffer* pFB) = 0;
	virtual void ClearRenderTarget(IRenderTarget* pRT, IDepthTarget* DepthTarget = nullptr) = 0;
	virtual void ClearRenderTargets(std::vector<IRenderTarget*> pRTs, IDepthTarget* DepthTarget = nullptr) = 0;
	virtual void ClearDepthTarget(IDepthTarget* DepthTarget) = 0;
	virtual void SetFrameBuffer(IFrameBuffer* pFB, std::optional<std::size_t> FBOIndex = std::nullopt) = 0;
	virtual void SetRenderTarget(IRenderTarget* pRT, IDepthTarget* DepthTarget = nullptr) = 0;
	virtual void SetRenderTargets(std::vector<IRenderTarget*> pRTs, IDepthTarget* DepthTarget = nullptr) = 0;
	virtual void SetFrameBufferAsResource(IFrameBuffer* pFB, std::uint32_t RootParameterIndex) = 0;
	virtual void SetFrameBufferAsResource(IFrameBuffer* pFB, std::uint32_t FBOIndex, std::uint32_t RootParameterIndex) = 0;
	virtual void SetRenderTargetAsResource(IRenderTarget* pRT, std::uint32_t RootParameterIndex) = 0;
	virtual void SetRenderTargetsAsResource(std::vector<IRenderTarget*> RTs, std::uint32_t RootParameterIndex) = 0;
	virtual void SetUnorderedAccessBufferAsResource(IUnorderedAccessBuffer* pUAV, std::uint32_t RootParameterIndex) = 0;
	virtual void SetUnorderedAccessBuffersAsResource(std::vector<IUnorderedAccessBuffer*> UAVs, std::uint32_t RootParameterIndex) = 0;
	virtual void CopyFrameBuffer(IFrameBuffer* Dest, std::size_t DestFBOIndex, IFrameBuffer* Source, std::uint32_t SourceFBOIndex) = 0;
	virtual void CopyFrameBufferDepth(IFrameBuffer* Dest, IFrameBuffer* Source) = 0;
	virtual void CopyRenderTarget(IRenderTarget* Dest, IRenderTarget* Source) = 0;
	virtual void CopyDepthBuffer(IDepthTarget* Dest, IDepthTarget* Source) = 0;

	virtual void SetPipeline(IPipeline* Pipeline) = 0;

	virtual void SetVertexBuffer(IVertexBuffer* VB) = 0;
	virtual void SetIndexBuffer(IIndexBuffer* IB) = 0;
	virtual void SetConstantBuffer(IConstantBuffer* CB, std::optional<std::uint32_t> RootParameterIndex = std::nullopt) = 0;

	virtual void SetTexture2D(ITexture2D* Texture2D, std::optional<std::uint32_t> RootParameterIndex = std::nullopt) = 0;

	virtual void UpdateBufferSubresource(IVertexBuffer* Buffer, sBufferSubresource* Subresource) = 0;
	virtual void UpdateBufferSubresource(IVertexBuffer* Buffer, std::size_t Location, std::size_t Size, const void* pSrcData) = 0;
	virtual void UpdateBufferSubresource(IIndexBuffer* Buffer, sBufferSubresource* Subresource) = 0;
	virtual void UpdateBufferSubresource(IIndexBuffer* Buffer, std::size_t Location, std::size_t Size, const void* pSrcData) = 0;

	virtual void Draw(std::uint32_t VertexCount, std::uint32_t VertexStartOffset = 0) = 0;
	virtual void DrawInstanced(std::uint32_t VertexCountPerInstance, std::uint32_t InstanceCount, std::uint32_t StartVertexLocation, std::uint32_t StartInstanceLocation) = 0;
	virtual void DrawIndexedInstanced(std::uint32_t IndexCountPerInstance, std::uint32_t InstanceCount, std::uint32_t StartIndexLocation, std::int32_t BaseVertexLocation, std::uint32_t StartInstanceLocation) = 0;
	virtual void DrawIndexedInstanced(const sObjectDrawParameters& Params) = 0;

	virtual void ExecuteIndirect(IIndirectBuffer* IndirectBuffer) = 0;
};

class IComputeCommandContext
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IComputeCommandContext)
public:
	static IComputeCommandContext::SharedPtr Create();
	static IComputeCommandContext::UniquePtr CreateUnique();

public:
	virtual void BeginRecordCommandList() = 0;
	virtual void FinishRecordCommandList() = 0;
	virtual void ExecuteCommandList() = 0;
	virtual void ClearState() = 0;

	virtual void* GetInternalCommandContext() = 0;

	virtual void SetFrameBuffer(IFrameBuffer* pFB, std::optional<std::size_t> FBOIndex) = 0;
	virtual void SetRenderTargetAsResource(IRenderTarget* pRT, std::uint32_t RootParameterIndex) = 0;
	virtual void SetRenderTargetsAsResource(std::vector<IRenderTarget*> RTs, std::uint32_t RootParameterIndex) = 0;
	virtual void SetDepthTargetAsResource(IDepthTarget* pDT, std::uint32_t RootParameterIndex) = 0;
	virtual void SetDepthTargetsAsResource(std::vector<IDepthTarget*> DTs, std::uint32_t RootParameterIndex) = 0;
	virtual void SetUnorderedAccessTarget(IUnorderedAccessTarget* pST, std::uint32_t RootParameterIndex) = 0;
	virtual void SetUnorderedAccessTargets(std::vector<IUnorderedAccessTarget*> pSTs, std::uint32_t RootParameterIndex) = 0;
	virtual void SetRenderTargetAsUAV(IRenderTarget* pRT, std::uint32_t RootParameterIndex) = 0;
	virtual void SetRenderTargetsAsUAV(std::vector<IRenderTarget*> RTs, std::uint32_t RootParameterIndex) = 0;
	virtual void SetUnorderedAccessTargetAsSRV(IUnorderedAccessTarget* pST, std::uint32_t RootParameterIndex) = 0;
	virtual void SetUnorderedAccessTargetsAsSRV(std::vector<IUnorderedAccessTarget*> pSTs, std::uint32_t RootParameterIndex) = 0;
	virtual void SetUnorderedAccessBuffer(IUnorderedAccessBuffer* pUAV, std::uint32_t RootParameterIndex) = 0;
	virtual void SetUnorderedAccessBuffers(std::vector<IUnorderedAccessBuffer*> UAVs, std::uint32_t RootParameterIndex) = 0;
	virtual void SetUnorderedAccessBufferAsResource(IUnorderedAccessBuffer* pUAV, std::uint32_t RootParameterIndex) = 0;
	virtual void SetUnorderedAccessBuffersAsResource(std::vector<IUnorderedAccessBuffer*> UAVs, std::uint32_t RootParameterIndex) = 0;

	virtual void SetPipeline(IComputePipeline* Pipeline) = 0;
	virtual void SetConstantBuffer(IConstantBuffer* CB, std::optional<std::uint32_t> RootParameterIndex = std::nullopt) = 0;

	virtual void Dispatch(std::uint32_t ThreadGroupCountX, std::uint32_t ThreadGroupCountY, std::uint32_t ThreadGroupCountZ) = 0;
	virtual void ExecuteIndirect(IIndirectBuffer* IndirectBuffer) = 0;
};

class ICopyCommandContext
{
	sBaseClassBody(sClassDefaultProtectedConstructor, ICopyCommandContext)
public:
	static ICopyCommandContext::SharedPtr Create();
	static ICopyCommandContext::UniquePtr CreateUnique();

public:
	virtual void BeginRecordCommandList() = 0;
	virtual void FinishRecordCommandList() = 0;
	virtual void ExecuteCommandList() = 0;
	virtual void ClearState() = 0;

	virtual void* GetInternalCommandContext() = 0;

	virtual void CopyFrameBuffer(IFrameBuffer* Dest, std::size_t DestFBOIndex, IFrameBuffer* Source, std::uint32_t SourceFBOIndex) = 0;
	virtual void CopyFrameBufferDepth(IFrameBuffer* Dest, IFrameBuffer* Source) = 0;

	virtual void UpdateBufferSubresource(IVertexBuffer* Buffer, sBufferSubresource* Subresource) = 0;
	virtual void UpdateBufferSubresource(IVertexBuffer* Buffer, std::size_t Location, std::size_t Size, const void* pSrcData) = 0;
	virtual void UpdateBufferSubresource(IIndexBuffer* Buffer, sBufferSubresource* Subresource) = 0;
	virtual void UpdateBufferSubresource(IIndexBuffer* Buffer, std::size_t Location, std::size_t Size, const void* pSrcData) = 0;
};

struct sDateTime
{
	std::int32_t Year = 0;
	std::int32_t Month = 0;
	std::int32_t Day = 0;
	std::int32_t DayOfWeek = 0;
	std::int32_t Hour = 0;
	std::int32_t Minute = 0;
	std::int32_t Second = 0;
	std::int32_t Millisecond = 0;

	constexpr sDateTime() = default;
	constexpr sDateTime(std::int32_t InYear, std::int32_t InMonth, std::int32_t InDay, int32_t InDayOfWeek,
		std::int32_t InHour = 0, std::int32_t InMinute = 0, std::int32_t InSecond = 0, std::int32_t InMillisecond = 0)
		: Year(InYear)
		, Month(InMonth)
		, Day(InDay)
		, DayOfWeek(InDayOfWeek)
		, Hour(InHour)
		, Minute(InMinute)
		, Second(InSecond)
		, Millisecond(InMillisecond)
	{}

	friend void operator<<(sArchive& Archive, const sDateTime& data)
	{
		Archive << data.Year;
		Archive << data.Month;
		Archive << data.Day;
		Archive << data.DayOfWeek;
		Archive << data.Hour;
		Archive << data.Minute;
		Archive << data.Second;
		Archive << data.Millisecond;
	}

	friend void operator>>(const sArchive& Archive, sDateTime& data)
	{
		Archive >> data.Year;
		Archive >> data.Month;
		Archive >> data.Day;
		Archive >> data.DayOfWeek;
		Archive >> data.Hour;
		Archive >> data.Minute;
		Archive >> data.Second;
		Archive >> data.Millisecond;
	}

	constexpr std::uint64_t GetCurrentDayTimeInSeconds() const
	{
		return Second + (Minute * 60) + (Hour * 3600);
	}

	constexpr std::uint64_t GetTotalSeconds() const
	{
		return Second + (Minute * 60) + (Hour * 3600) + (Day * 86400) + (Month * 604800) + (Year * 31556926);
	}

	constexpr std::uint64_t GetCurrentDayTimeInMillisecond() const
	{
		return GetCurrentDayTimeInSeconds() * 1000 + Millisecond;
		//return Millisecond + (Second * 1000) + (Minute * 60000) + (Hour * 3600000);
	}

	constexpr std::uint64_t GetTotalMillisecond() const
	{
		return GetTotalSeconds() * 1000 + Millisecond;
		//return Millisecond + (Second * 1000) + (Minute * 60000) + (Hour * 3600000) + (Day * 86400000) + (Month * 2629746000) + (Year * 31556952000);
	}

	constexpr double GetCurrentDayTimeAsDouble() const
	{
		return Second + (Minute * 60.0) + (Hour * 3600.0) + (Millisecond / 1000.0);
	}

	FORCEINLINE constexpr std::string ToString() const
	{
		return std::string("Year : " + std::to_string(Year) + " | Month: " + std::to_string(Month) + " | Day : " + std::to_string(Day) + " | DayOfWeek : " + std::to_string(DayOfWeek) + " | Hour : " + std::to_string(Hour) + " | Minute : " + std::to_string(Minute) + " | Second : " + std::to_string(Second) + " | Millisecond : " + std::to_string(Millisecond));
	};

#if _MSVC_LANG >= 202002L
	constexpr auto operator<=>(const sDateTime&) const = default;
#endif
};

FORCEINLINE constexpr sDateTime operator +(const sDateTime& value1, const sDateTime& value2)
{
	return sDateTime(value1.Year + value2.Year, value1.Month + value2.Month, value1.Day + value2.Day, value1.DayOfWeek + value2.DayOfWeek, value1.Hour + value2.Hour, value1.Minute + value2.Minute, value1.Second + value2.Second, value1.Millisecond + value2.Millisecond);
};

FORCEINLINE constexpr sDateTime operator -(const sDateTime& value1, const sDateTime& value2)
{
	return sDateTime(value1.Year - value2.Year, value1.Month - value2.Month, value1.Day - value2.Day, value1.DayOfWeek - value2.DayOfWeek, value1.Hour - value2.Hour, value1.Minute - value2.Minute, value1.Second - value2.Second, value1.Millisecond - value2.Millisecond);
};

struct sGPUInfo
{
	sBaseClassBody(sClassConstructor, sGPUInfo)

	std::string GPUName;
	std::size_t VendorId;
	std::size_t DeviceId;
	std::size_t SubSysId;
	std::size_t Revision;

	EGITypes SupportedAPI;
	std::size_t SupportedFeatureLevel;
	std::string SupportedFeatureLevelString;

	std::size_t SharedSystemMemory;
	std::size_t DedicatedVideoMemory;
	std::size_t DedicatedSystemMemory;
	std::size_t MaxVideoMemory;

	__forceinline constexpr sGPUInfo()
		: GPUName("")
		, VendorId(0)
		, DeviceId(0)
		, SubSysId(0)
		, Revision(0)
		, SupportedAPI(EGITypes::eD3D11)
		, SupportedFeatureLevel(0)
		, SupportedFeatureLevelString("")
		, SharedSystemMemory(0)
		, DedicatedVideoMemory(0)
		, DedicatedSystemMemory(0)
		, MaxVideoMemory(0)
	{}

	~sGPUInfo() = default;

	FORCEINLINE constexpr bool IsNvDeviceID() const
	{
		return VendorId == 0x10DE;
	}

	FORCEINLINE constexpr bool IsAMDDeviceID() const  // 0x1022 ?
	{
		return VendorId == 0x1002;
	}

	FORCEINLINE constexpr bool IsIntelDeviceID() const // 0x163C, 0x8087 ?
	{
		return VendorId == 0x8086;
	}

	FORCEINLINE constexpr bool IsSoftwareDevice() const
	{
		return VendorId == 0x1414;
	}

	FORCEINLINE constexpr std::string VendorIDToString() const
	{
		return IsNvDeviceID() ? "Nvidia" : IsAMDDeviceID() ? "AMD" : IsIntelDeviceID() ? "Intel" : IsSoftwareDevice() ? "Software" : "Unknown";
	}

	FORCEINLINE constexpr std::string SupportedAPIToString() const
	{
		return SupportedAPI == EGITypes::eD3D11 ? "D3D11" : SupportedAPI == EGITypes::eD3D12 ? "D3D12" : SupportedAPI == EGITypes::eVulkan ? "Vulkan" : "Unknown";
	}

	FORCEINLINE constexpr std::string ToString() const
	{
		return std::string(GPUName + "\n" + std::to_string(VendorId) + " : " + VendorIDToString() + "\n" + "DeviceID : " + std::to_string(DeviceId) + "\n" + "SubSysId : " + std::to_string(SubSysId) + "\n" + "Revision : " + std::to_string(Revision) + "\n" + "SupportedAPI : " + SupportedAPIToString() + "\n" + "SupportedFeatureLevel : " + std::to_string(SupportedFeatureLevel) + " " + SupportedFeatureLevelString + "\n" + "SharedSystemMemory : " + std::to_string(SharedSystemMemory) + "\n" + "DedicatedVideoMemory : " + std::to_string(DedicatedVideoMemory) + "\n" + "DedicatedSystemMemory : " + std::to_string(DedicatedSystemMemory) + "\n" + "MaxVideoMemory : " + std::to_string(MaxVideoMemory));
	}

	FORCEINLINE void WriteToConsole() const
	{
		std::cout << ToString() << std::endl;
	}
};

enum class EPhysicsEngine
{
	eNone,
	//eBulletPhysics,
	eBox2D,
	//ePhysX,
};

enum class EPostProcessRenderOrder
{
	BeforeTonemap,
	AfterTonemap,
	BeforeUI,
	AfterUI,
};

enum class EGBufferClear
{
	Driver,
	Disabled,
	//Sky,
};

enum class ESplitScreenType
{
	Grid,
	Horizontal,
};

enum class eNetworkRole : std::uint8_t
{
	None,
	Host,
	SimulatedProxy,
	NetProxy,
	Client,
};

enum class eRPCType
{
	Server,
	Client,
	ServerAndClient
};

/*
* WIP
*/
template<typename T>
class ReplicatedVariable
{
protected:
	ReplicatedVariable(const std::string& InName, bool bReliable, bool IsReqTimeStamp)
		: Name(InName)
		, bIsReliable(bReliable)
		, ReqTimeStamp(IsReqTimeStamp)
	{}

public:
	virtual ~ReplicatedVariable()
	{
		Name = "";
	}

	inline bool IsReqTimeStamp() const { return ReqTimeStamp; }
	inline std::string GetName() const { return Name; }
	inline bool IsReliable() const { return bIsReliable; }

	void Sync()
	{

	}

	T Variable;

private:
	std::string Name;
	bool bIsReliable;
	bool ReqTimeStamp;
};

class RemoteProcedureCallBase
{
protected:
	RemoteProcedureCallBase(eRPCType InType, const std::string& InName, bool bReliable, bool IsReqTimeStamp)
		: Name(InName)
		, Type(InType)
		, bIsReliable(bReliable)
		, ReqTimeStamp(IsReqTimeStamp)
	{}

public:
	virtual ~RemoteProcedureCallBase()
	{
		Name = "";
	}

	inline bool IsReqTimeStamp() const { return ReqTimeStamp; }
	inline std::string GetName() const { return Name; }
	inline eRPCType GetType() const { return Type; }
	inline bool IsReliable() const { return bIsReliable; }
	//inline virtual std::vector<eParamType> GetParamTypes() const = 0;
	inline virtual bool SetParams(const sArchive& sArchive) = 0;

	virtual void Call() = 0;
	virtual void Call(const sArchive& pArchive) = 0;

private:
	std::string Name;
	eRPCType Type;
	bool bIsReliable;
	bool ReqTimeStamp;
};

template<typename... Args>
class RemoteProcedureCall : public RemoteProcedureCallBase
{
public:
	RemoteProcedureCall(eRPCType InType, const std::string& InName, bool bReliable, bool IsReqTimeStamp, const std::function<void(Args...)>& pfn)
		: RemoteProcedureCallBase(InType, InName, bReliable, IsReqTimeStamp)
		, function(pfn)
		, ParamCount(0)
	{
		for_each_tuple(Params, [&](const auto& x) {
			//ParamTypes.push_back(GetParamType(x));
			ParamCount++;
		});
	}

	virtual ~RemoteProcedureCall()
	{
		function = nullptr;
		Params = std::tuple<Args...>();
		//ParamTypes.clear();
	}

	virtual void Call() override
	{
		std::lock_guard<std::mutex> lock(mutex);
		std::apply([&](auto...xs) { function(std::forward<decltype(xs)>(xs)...); }, Params);
		Params = std::tuple<Args...>();
	}

	virtual void Call(const sArchive& pArchive) override
	{
		std::lock_guard<std::mutex> lock(mutex);

		if (ParamCount > 0)
		{
			pArchive.ResetPos();
			for_each_tuple(Params, [&](auto& x) {
				pArchive >> x;
				});
		}

		std::apply([&](auto...xs) { function(std::forward<decltype(xs)>(xs)...); }, Params);
		Params = std::tuple<Args...>();
	}

	//inline virtual std::vector<eParamType> GetParamTypes() const override { return ParamTypes; }

	inline virtual void SetParamsAsTuple(std::tuple<Args...>& t)
	{
		std::lock_guard<std::mutex> lock(mutex);
		Params = t;
	}

	inline virtual bool SetParams(const sArchive& pArchive)
	{
		std::lock_guard<std::mutex> lock(mutex);
		if (ParamCount == 0)
			return false;

		//if (ParamTypes.size() == 0)
		//	return false;

		pArchive.ResetPos();
		for_each_tuple(Params, [&](auto& x) {
			pArchive >> x;
			});
		return true;
	}

	/*template<ParamType p>
	constexpr decltype(auto) GetParam() noexcept
	{
		if constexpr (p == ParamType::eInt) return P2;
		else if constexpr (p == ParamType::eFloat) return P3;
		else if constexpr (p == ParamType::eDouble) return P4;
		else if constexpr (p == ParamType::eFVector) return P1;
	}*/

private:
	std::mutex mutex;
	std::function<void(Args...)> function;
	//std::vector<eParamType> ParamTypes;
	int ParamCount;
	std::tuple<Args...> Params;
};

class sPostProcess;
class sPhysicalComponent;

namespace GPU
{
	sGPUInfo GetGPUInfo();
	EGITypes GetGIType();
	sViewport GetViewport();
	sScreenDimension GetBackBufferDimension();
	EFormat GetBackBufferFormat();
	EFormat GetDefaultDepthFormat();

	void SetGBufferClearMode(EGBufferClear Mode);
	EGBufferClear GetGBufferClearMode();

	void SetTonemapper(const int Val);
	int GetTonemapperIndex();

	void AddPostProcess(const EPostProcessRenderOrder Order, const std::shared_ptr<sPostProcess>& PostProcess);
	void RemovePostProcess(const EPostProcessRenderOrder Order, const int Val);

	void* GetInternalDevice();

	void DrawLine(const FVector& Start, const FVector& End, std::optional<float> Time);
	void DrawBound(const FBoundingBox& Box, std::optional<float> Time);
	void DrawLine(const FVector& Start, const FVector& End, const FColor& Color, std::optional<float> Time);
	void DrawBound(const FBoundingBox& Box, const FColor& Color, std::optional<float> Time);

	sScreenDimension GetInternalBaseRenderResolution();
	void SetInternalBaseRenderResolution(std::size_t Width, std::size_t Height);

	void AddViewportInstance(sViewportInstance* ViewportInstance, std::optional<std::size_t> Priority = std::nullopt);
	void RemoveViewportInstance(sViewportInstance* ViewportInstance);
	void RemoveViewportInstance(std::size_t Index);
	/*
	* Priority == 0 : On Top
	*/
	void SetViewportInstancePriority(sViewportInstance* ViewportInstance, std::size_t Priority);
}

sViewportInstance::~sViewportInstance()
{
	bIsEnabled = false;
	GPU::RemoveViewportInstance(this);
	pCamera = nullptr;
	Canvases.clear();
}

namespace Physics
{
	EPhysicsEngine GetActivePhysicsEngineType();

	bool IsPhysicsPaused();
	void PausePhysics(bool value);

	void SetPhysicsInternalTick(std::optional<double> Tick);

	void SetGravity(const FVector& Gravity);
	FVector GetGravity();
	void SetWorldOrigin(const FVector& newOrigin);

	sPhysicalComponent* LineTraceToViewPort(const FVector& InOrigin, const FVector& InDirection);
	template<typename T>
	T* LineTraceToViewPort(const FVector& InOrigin, const FVector& InDirection)
	{
		return dynamic_cast<T*>(LineTraceToViewPort(InOrigin, InDirection));
	}
	std::vector<sPhysicalComponent*> QueryAABB(const FBoundingBox& Bounds);

	float GetPhysicalWorldScale();
}

/*
* To do
* 3D Audio
* Effects
* Spatial
*/
namespace Audio
{
	void AddToPlayList(std::string Name, std::string path, bool loop = false, bool RunOnce = false);
	void Play(std::string Name, std::string path, bool loop, bool PlayAsOverlap);
	void Stop(bool immediate = true);
	void Next();
	void Resume();
	void Pause();
	void Remove(std::size_t index, bool IsOverlapSound = false);
	void Remove(std::string Name, bool IsOverlapSound = false);
	void DestroyAllVoice(bool IsOverlapSoundOnly = false);
	/*
	* Enable/Disable
	*/
	void SetPlayListState(bool State);

	bool IsLooped();

	float GetVolume();
	void SetVolume(float volume);

	std::size_t GetPlayListCount(bool IsOverlapSoundOnly = false);
	std::size_t GetCurrentAudioIndex();
	std::size_t GetNextAudioIndex();

	void BindFunctionOnVoiceStart(std::function<void(std::string)> fOnVoiceStart);
	void BindFunctionOnVoiceStop(std::function<void(std::string)> fOnVoiceStop);
}

class sGameInstance;
class sPlayer;
namespace Network
{
	bool CreateSession(std::string Name, sGameInstance* Instance, std::string Level, std::size_t PlayerCount = 8, std::uint16_t Port = 27020);
	void DestroySession();
	bool IsServerRunning();
	void SetServerName(std::string Name);
	bool ServerChangeLevel(std::string Level);
	void SetServerMaximumMessagePerTick(std::size_t Size);
	std::size_t GetServerMaximumMessagePerTick();
	std::string GetServerLevel();
	std::size_t GetPlayerSize();
	std::uint64_t GetLatency();

	bool IsHost();
	bool IsClient();

	bool Connect(sGameInstance* Instance);
	bool Connect(sGameInstance* Instance, std::string ip = "127.0.0.1", std::uint16_t Port = 27020);
	bool Disconnect();
	bool IsConnected();
	void SetClientMaximumMessagePerTick(std::size_t Size);
	std::size_t GetClientMaximumMessagePerTick();

	void CallRPC(std::string Address, std::string ClassName, std::string Name, std::optional<bool> reliable = std::nullopt);
	void CallRPC(std::string Address, std::string ClassName, std::string Name, const sArchive& Params, std::optional<bool> reliable = std::nullopt);

	template <typename... Args>
	bool CallRPCEx(std::string Address, std::string ClassName, std::string Name, bool reliable, Args&&... args)
	{
		return CallRPC(Address, ClassName, Name, sArchive(args...), reliable);
	}

	void RegisterRPC(std::string Address, std::string ClassName, RemoteProcedureCallBase* RPC);
#ifndef RegisterRPCfn
#define RegisterRPCfn(Add,c,s,x,y,t,f,...) Network::RegisterRPC(Add,c, new RemoteProcedureCall<__VA_ARGS__>(x, s, y, t, f))
#endif
	void UnregisterRPC(std::string Address);
	void UnregisterRPC(std::string Address, std::string ClassName);
	void UnregisterRPC(std::string Address, std::string ClassName, const std::string& rpcName);
}

class sInputController;
namespace Engine
{
	void WriteToConsole(const std::string& STR);

	void LocalUTCTimeNow(std::int32_t& Year, int32_t& Month, int32_t& DayOfWeek, int32_t& Day, int32_t& Hour, int32_t& Min, int32_t& Sec, int32_t& MSec);
	void UTCTimeNow(std::int32_t& Year, std::int32_t& Month, std::int32_t& DayOfWeek, std::int32_t& Day, std::int32_t& Hour, std::int32_t& Min, std::int32_t& Sec, std::int32_t& MSec);

	sDateTime GetLocalUTCTimeNow();
	sDateTime GetUTCTimeNow();
	sDateTime GetAppStartTime();
	sDateTime GetExecutionTime();
	std::uint64_t GetExecutionTimeInMS();
	std::uint64_t GetExecutionTimeInSecond();

	sInputController* GetInputController();

	void QueueJob(const std::function<void()>& job);
	std::size_t AvailableThreadCount();

	bool IsInputPaused();
	void PauseInput(bool value);
	bool IsTickPaused();
	void PauseTick(bool value);

	sScreenDimension GetScreenDimension();
}
