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
#include <stdarg.h>
#include <map>
#include <mutex>
#include <type_traits>
#include <wrl/client.h>
#include "Core/Math/CoreMath.h"
#include "Engine/ClassBody.h"
#include "AbstractEngineUtilities.h"
#include "Core/Archive.h"

class IFrameBuffer;
class IGraphicsCommandContext;

enum class ERenderPass
{
	NONE,
	GBuffer,
	Line,
	Particle,
	PostProcess,
	UI,
};

enum class EGPUDeviceType
{
	Hardware,
	Software
};

enum class EGITypes
{
	Undefined,
	Default,
	D3D12 = Default,
	/*
	* WIP
	* When "ResourceDescriptorHeap" support is added, this will be supported.
	*/
	Vulkan,
	// Deprecated
	D3D11,
	/* Unsuported */
	//eOpenGL46,
};

enum class DisplayMode
{
	DISPLAYMODE_LDR,
	DISPLAYMODE_HDR10_2084,
	DISPLAYMODE_HDR10_SCRGB,
	DISPLAYMODE_FSHDR_2084,
	DISPLAYMODE_FSHDR_SCRGB
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
	//BGRA8_TYPELESS,
	BGRA8_UNORM_SRGB,
	SRGBA8_UNORM,
	RGB10A2_UNORM,
	R11G11B10_FLOAT,
	RG16_UINT,
	RG16_FLOAT,
	R32_UINT,
	R32_SINT,
	R32_FLOAT,
	R32_Typeless,
	RGBA8_UINT,
	RGBA16_FLOAT,
	RGBA16_UINT,
	RGBA16_UNORM,
	RGBA16_SNORM,
	RG32_UINT,
	RG32_SINT,
	RG32_FLOAT,
	RGB32_UINT,
	RGB32_SINT,
	RGB32_FLOAT,
	RGBA32_UINT,
	RGBA32_SINT,
	RGBA32_FLOAT,
	//R24G8_Typeless,
	//R24UX8_Typeless
	//R32G8X24_Typeless, 
	D16_UNORM,
	D32_FLOAT,
	D24_UNORM_S8_UINT,
	D32_FLOAT_S8X24_UINT,
	/*BC1_UNORM,
	BC1_UNORM_SRGB,
	BC2_UNORM,
	BC2_UNORM_SRGB,
	BC3_UNORM,
	BC3_UNORM_SRGB,*/
};

struct HDRMetadata
{
	float RedPrimary[2];                ///< HDR red primaries.
	float GreenPrimary[2];              ///< HDR green primaries.
	float BluePrimary[2];               ///< HDR blue primaries.
	float WhitePoint[2];                ///< HDR white points.
	float MinLuminance;                 ///< HDR minimum luminance value.
	float MaxLuminance;                 ///< HDR maximum luminance value.
	float MaxContentLightLevel;         ///< HDR maximum content light level.
	float MaxFrameAverageLightLevel;    ///< HDR maximum average light level.
};

namespace
{
	inline EFormat GetDisplayFormat(DisplayMode displayMode)
	{
		switch (displayMode)
		{
		case DisplayMode::DISPLAYMODE_LDR:
			return EFormat::RGBA8_UNORM;
		case DisplayMode::DISPLAYMODE_FSHDR_2084:
		case DisplayMode::DISPLAYMODE_HDR10_2084:
			return EFormat::RGB10A2_UNORM;
		case DisplayMode::DISPLAYMODE_FSHDR_SCRGB:
		case DisplayMode::DISPLAYMODE_HDR10_SCRGB:
			return EFormat::RGBA16_FLOAT;
		default:
			return EFormat::UNKNOWN;
		}
	}

	inline void hash_combine(std::size_t& seed, std::size_t h)
	{
		seed ^= h + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
	}

	inline std::uint32_t ComputeIntersectionArea(int ax1, int ay1, int ax2, int ay2, int bx1, int by1, int bx2, int by2)
	{
		return (std::max)(0, (std::min)(ax2, bx2) - (std::max)(ax1, bx1)) * (std::max)(0, (std::min)(ay2, by2) - (std::max)(ay1, by1));
	}

	inline bool IsValidDepthOnlyFormat(const EFormat Format)
	{
		return Format == EFormat::D32_FLOAT || Format == EFormat::D16_UNORM || Format == EFormat::R32_Typeless;
	};

	inline bool IsValidDepthStencilFormat(const EFormat Format)
	{
		return Format == EFormat::D24_UNORM_S8_UINT || Format == EFormat::D32_FLOAT_S8X24_UINT || Format == EFormat::D32_FLOAT || Format == EFormat::D16_UNORM;
	};

	inline bool IsValidDepthFormat(const EFormat Format)
	{
		return IsValidDepthOnlyFormat(Format) || IsValidDepthStencilFormat(Format);
	};

	inline bool IsDepthSRVSupported(const EFormat Format)
	{
		return Format == EFormat::R16_FLOAT || Format == EFormat::R32_FLOAT || Format == EFormat::R32_Typeless /*|| Format == EFormat::R24UX8_Typeless*/;
	};

	inline float CalculateMipBias(float upscalerRatio)
	{
		return std::log2f(1.f / upscalerRatio) - 1.f + std::numeric_limits<float>::epsilon();
	}

	std::uint32_t GetFormatSize(EFormat format)
	{
		std::uint32_t Size = 0;
		switch (format)
		{
		case EFormat::RGBA8_UINT: Size += sizeof(TVector4<std::uint8_t>); break;
		case EFormat::RGBA16_UINT: Size += sizeof(TVector4<std::uint16_t>); break;
		case EFormat::BGRA8_UNORM: Size += sizeof(TVector4<std::uint8_t>); break;
		case EFormat::BGRA8_UNORM_SRGB: Size += sizeof(TVector4<std::uint8_t>); break;
		case EFormat::R8_UINT: Size += sizeof(std::uint8_t); break;
		case EFormat::R8_UNORM: Size += sizeof(std::uint8_t); break;
		case EFormat::R8_SNORM: Size += sizeof(std::int8_t); break;
		case EFormat::RG8_UINT: Size += sizeof(TVector2<std::uint8_t>); break;
		case EFormat::RG8_UNORM: Size += sizeof(TVector2<std::uint8_t>); break;
		case EFormat::R16_UINT: Size += sizeof(std::uint16_t); break;
		case EFormat::R16_UNORM: Size += sizeof(std::uint16_t); break;
		case EFormat::R16_FLOAT: Size += sizeof(float); break;
		case EFormat::RGBA8_UNORM: Size += sizeof(TVector4<std::uint8_t>); break;
		case EFormat::SRGBA8_UNORM: Size += sizeof(TVector4<std::uint8_t>); break;
		case EFormat::RGB10A2_UNORM: Size += sizeof(TVector4<std::uint8_t>); break;
		case EFormat::R11G11B10_FLOAT: Size += sizeof(TVector3<float>); break;
		case EFormat::RG16_UINT: Size += sizeof(TVector2<std::uint16_t>); break;
		case EFormat::RG16_FLOAT: Size += sizeof(TVector2<float>); break;
		case EFormat::R32_UINT: Size += sizeof(std::uint32_t); break;
		case EFormat::R32_SINT: Size += sizeof(std::int32_t); break;
		case EFormat::R32_FLOAT: Size += sizeof(float); break;
		case EFormat::RGBA16_FLOAT:	Size += sizeof(TVector4<float>); break;
		case EFormat::RGBA16_UNORM:	Size += sizeof(TVector4<std::uint16_t>); break;
		case EFormat::RGBA16_SNORM:	Size += sizeof(TVector4<std::int16_t>); break;
		case EFormat::RG32_UINT: Size += sizeof(TVector2<std::uint32_t>); break;
		case EFormat::RG32_SINT: Size += sizeof(TVector2<std::int32_t>); break;
		case EFormat::RG32_FLOAT: Size += sizeof(TVector2<float>); break;
		case EFormat::RGB32_UINT: Size += sizeof(TVector3<std::uint32_t>); break;
		case EFormat::RGB32_SINT: Size += sizeof(TVector3<std::int32_t>); break;
		case EFormat::RGB32_FLOAT: Size += sizeof(TVector3<float>); break;
		case EFormat::RGBA32_UINT: Size += sizeof(TVector4<std::uint32_t>); break;
		case EFormat::RGBA32_SINT: Size += sizeof(TVector4<std::int32_t>); break;
		case EFormat::RGBA32_FLOAT: Size += sizeof(TVector4<float>); break;
		case EFormat::D16_UNORM: Size += sizeof(std::uint16_t); break;
		case EFormat::D32_FLOAT: Size += sizeof(std::uint32_t); break;
		case EFormat::D24_UNORM_S8_UINT: Size += sizeof(std::uint32_t); break;
		case EFormat::D32_FLOAT_S8X24_UINT: Size += sizeof(std::uint32_t); break;
		}
		return Size;
	}
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

	All,
};

class RefCountedObject 
{
public:
	RefCountedObject() 
		: RefCount(1)
	{}

	virtual ~RefCountedObject() = default;

	void AddRef() 
	{
		RefCount.fetch_add(1, std::memory_order_relaxed);
	}

	void Release() 
	{
		if (RefCount.fetch_sub(1, std::memory_order_acquire) == 1)
		{
			std::atomic_thread_fence(std::memory_order_release);
			delete this;
		}
	}

	int GetRefCounter() const
	{
		return RefCount.load(std::memory_order_relaxed);;
	}

private:
	std::atomic<int> RefCount;
};

template <typename T>
class RefCountPtr
{
public:
	typedef T InterfaceType;

protected:
	InterfaceType* ptr_;
	template<class U> friend class RefCountPtr;

	void InternalAddRef() const throw()
	{
		if (ptr_ != nullptr)
		{
			ptr_->AddRef();
		}
	}

	unsigned long InternalRelease() throw()
	{
		unsigned long ref = 0;
		T* temp = ptr_;

		if (temp != nullptr)
		{
			ptr_ = nullptr;
			ref = temp->Release();
		}

		return ref;
	}

public:
	RefCountPtr() throw() 
		: ptr_(nullptr)
	{}

	RefCountPtr(decltype(__nullptr)) throw() 
		: ptr_(nullptr)
	{}

	template<class U>
	RefCountPtr(_In_opt_ U* other) throw() 
		: ptr_(other)
	{
		InternalAddRef();
	}

	RefCountPtr(const RefCountPtr& other) throw() 
		: ptr_(other.ptr_)
	{
		InternalAddRef();
	}

	template<class U>
	RefCountPtr(const RefCountPtr<U>& other, typename std::enable_if<std::is_convertible<U*, T*>::value, void*>::type* = nullptr) noexcept
		: ptr_(other.ptr_)
	{
		InternalAddRef();
	}

	RefCountPtr(_Inout_ RefCountPtr&& other) throw() 
		: ptr_(nullptr)
	{
		if (this != reinterpret_cast<RefCountPtr*>(&reinterpret_cast<unsigned char&>(other)))
		{
			Swap(other);
		}
	}

	template<class U>
	RefCountPtr(RefCountPtr<U>&& other, typename std::enable_if<std::is_convertible<U*, T*>::value, void*>::type* = nullptr) noexcept
		: ptr_(other.ptr_)
	{
		other.ptr_ = nullptr;
	}

	~RefCountPtr() throw()
	{
		InternalRelease();
	}

	static RefCountPtr<T> Create(T* other)
	{
		RefCountPtr<T> Ptr;
		Ptr.Attach(other);
		return Ptr;
	}

	RefCountPtr& operator=(decltype(__nullptr)) throw()
	{
		InternalRelease();
		return *this;
	}

	RefCountPtr& operator=(_In_opt_ T* other) throw()
	{
		if (ptr_ != other)
		{
			RefCountPtr(other).Swap(*this);
		}
		return *this;
	}

	template <typename U>
	RefCountPtr& operator=(_In_opt_ U* other) throw()
	{
		RefCountPtr(other).Swap(*this);
		return *this;
	}

	RefCountPtr& operator=(const RefCountPtr& other) throw()
	{
		if (ptr_ != other.ptr_)
		{
			RefCountPtr(other).Swap(*this);
		}
		return *this;
	}

	template<class U>
	RefCountPtr& operator=(const RefCountPtr<U>& other) throw()
	{
		RefCountPtr(other).Swap(*this);
		return *this;
	}

	RefCountPtr& operator=(_Inout_ RefCountPtr&& other) throw()
	{
		RefCountPtr(static_cast<RefCountPtr&&>(other)).Swap(*this);
		return *this;
	}

	template<class U>
	RefCountPtr& operator=(_Inout_ RefCountPtr<U>&& other) throw()
	{
		RefCountPtr(static_cast<RefCountPtr<U>&&>(other)).Swap(*this);
		return *this;
	}
	void Swap(_Inout_ RefCountPtr&& r) throw()
	{
		T* tmp = ptr_;
		ptr_ = r.ptr_;
		r.ptr_ = tmp;
	}

	void Swap(_Inout_ RefCountPtr& r) throw()
	{
		T* tmp = ptr_;
		ptr_ = r.ptr_;
		r.ptr_ = tmp;
	}

	T* Get() const throw()
	{
		return ptr_;
	}

	InterfaceType* operator->() const throw()
	{
		return ptr_;
	}

	T* const* GetAddressOf() const throw()
	{
		return &ptr_;
	}

	T** GetAddressOf() throw()
	{
		return &ptr_;
	}

	T** operator&()
	{
		return &ptr_;
	}

	operator T* () const
	{
		return ptr_;
	}

	T** ReleaseAndGetAddressOf() throw()
	{
		InternalRelease();
		return &ptr_;
	}

	T* Detach() throw()
	{
		T* ptr = ptr_;
		ptr_ = nullptr;
		return ptr;
	}

	void Attach(_In_opt_ InterfaceType* other) throw()
	{
		if (ptr_ != nullptr)
		{
			auto ref = ptr_->Release();
			(void)ref;

			assert(ref != 0 || ptr_ != other);
		}

		ptr_ = other;
	}

	unsigned long Reset()
	{
		return InternalRelease();
	}

	void CopyTo(_Outptr_result_maybenull_ InterfaceType** ptr) const throw()
	{
		InternalAddRef();
		*ptr = ptr_;
	}
};    // RefCountPtr

struct IBuffer
{
	IBuffer() = default;
	virtual ~IBuffer() = default;
	[[nodiscard]] virtual const void* GetData() const = 0;
	[[nodiscard]] virtual size_t GetSize() const = 0;
};

struct UntypedData : public IBuffer
{
private:
	void* Data;
	size_t Size;

public:
	UntypedData(void* data, size_t size)
		: Data(data)
		, Size(size)
	{}

	virtual ~UntypedData()
	{
		if (Data)
		{
			free(Data);
			Data = nullptr;
		}

		Size = 0;
	}

	inline bool IsValid() const
	{
		return Data != nullptr || Size != 0;
	}

	[[nodiscard]] virtual const void* GetData() const override
	{
		return Data;
	}
	template<class T>
	[[nodiscard]] T* GetData() const
	{
		return static_cast<T*>(Data);
	}
	[[nodiscard]] virtual size_t GetSize() const override
	{
		return Size;
	}
};

struct GPUCreateInfo
{
	struct DeviceCreateInfo
	{
		EGITypes Type = EGITypes::Default;
		std::uint32_t GPUIndex = 0;
		EGPUDeviceType DeviceType;
	};
	DeviceCreateInfo PrimaryGPU;
	/*
	* WIP
	*/
	DeviceCreateInfo SecondaryGPU;
	void* pHWND = nullptr;
	std::uint32_t Width = 0;
	std::uint32_t Height = 0;
	bool Fullscreen = false;
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

	bool IsValid() const 
	{
		return Width != 0 && Width != std::size_t(-1) && Height != 0 && Height != std::size_t(-1); 
	}

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

struct sRenderDimension
{
	std::uint32_t RenderWidth = 0;
	std::uint32_t RenderHeight = 0;
	std::uint32_t UpscaleWidth = 0;
	std::uint32_t UpscaleHeight = 0;
	std::uint32_t DisplayWidth = 0;
	std::uint32_t DisplayHeight = 0;
	sRenderDimension() = default;
	sRenderDimension(sScreenDimension InRenderDimension, sScreenDimension InUpscaleDimension, sScreenDimension InDisplayDimension)
		: RenderWidth((std::uint32_t)InRenderDimension.Width)
		, RenderHeight((std::uint32_t)InRenderDimension.Height)
		, UpscaleWidth((std::uint32_t)InUpscaleDimension.Width)
		, UpscaleHeight((std::uint32_t)InUpscaleDimension.Height)
		, DisplayWidth((std::uint32_t)InDisplayDimension.Width)
		, DisplayHeight((std::uint32_t)InDisplayDimension.Height)
	{}
	sRenderDimension(std::uint32_t InRenderWidth, std::uint32_t InRenderHeight, std::uint32_t InUpscaleWidth, std::uint32_t InUpscaleHeight, std::uint32_t InDisplayWidth, std::uint32_t InDisplayHeight)
		: RenderWidth(InRenderWidth)
		, RenderHeight(InRenderHeight)
		, UpscaleWidth(InUpscaleWidth)
		, UpscaleHeight(InUpscaleHeight)
		, DisplayWidth(InDisplayWidth)
		, DisplayHeight(InDisplayHeight)
	{}

#if _MSVC_LANG >= 202002L
	constexpr auto operator<=>(const sRenderDimension&) const = default;
#endif

public:
	inline float fRenderWidth() const { return static_cast<float>(RenderWidth); }
	inline float fRenderHeight() const { return static_cast<float>(RenderHeight); }
	inline float fDisplayWidth() const { return static_cast<float>(DisplayWidth); }
	inline float fDisplayHeight() const { return static_cast<float>(DisplayHeight); }
	inline float GetRenderWidthScaleRatio() const { return fRenderWidth() / fDisplayWidth(); }
	inline float GetRenderHeightScaleRatio() const { return fRenderHeight() / fDisplayHeight(); }
	inline float GetRenderAspectRatio() const { return fRenderWidth() / fDisplayHeight(); }
	inline float GetDisplayWidthScaleRatio() const { return fDisplayWidth() / fRenderWidth(); }
	inline float GetDisplayHeightScaleRatio() const { return fDisplayHeight() / fRenderHeight(); }
	inline float GetDisplayAspectRatio() const { return fDisplayWidth() / fDisplayHeight(); }
	inline sScreenDimension GetDisplayDimension() const { return sScreenDimension(DisplayWidth, DisplayHeight); }
	inline sScreenDimension GetRenderDimension() const { return sScreenDimension(RenderWidth, RenderHeight);	}
	inline sScreenDimension GetUpscaleDimension() const { return sScreenDimension(UpscaleWidth, DisplayHeight); }

	bool IsValid() const
	{
		return RenderWidth != 0 && RenderWidth != std::uint32_t(-1) && RenderHeight != 0 && RenderHeight != std::uint32_t(-1)
			&& UpscaleWidth != 0 && UpscaleWidth != std::uint32_t(-1) && UpscaleHeight != 0 && UpscaleHeight != std::uint32_t(-1)
			&& DisplayWidth != 0 && DisplayWidth != std::uint32_t(-1) && DisplayHeight != 0 && DisplayHeight != std::uint32_t(-1);
	}
};

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
	~sViewportInstance();

	bool bIsEnabled = true;
	std::optional<sViewport> Viewport = std::nullopt;
	std::shared_ptr<sCamera> pCamera = nullptr;
	std::vector<ICanvas*> Canvases;
};

struct sDisplayDesc
{
	struct sRefreshRate
	{
		std::uint32_t Numerator = 0;
		std::uint32_t Denominator = 0;
	};

	std::string Name;
	std::uint32_t Width = 0;
	std::uint32_t Height = 0;
	sRefreshRate RefreshRate;
};

enum class EVertexLayoutType
{
	DefaultVertexLayout,
	Line,
	Particle,
	GUI,
};

struct sVertexLayout
{
	struct sVertexInstanceLayout
	{
		FVector position;
		FColor Color;
		//std::uint32_t ArrayIndex;

		sVertexInstanceLayout()
			: position(FVector::Zero())
			, Color(FColor::White())
			//, ArrayIndex(NULL)
		{}
		sVertexInstanceLayout(FVector InPosition, FColor InColor = FColor::White())
			: position(InPosition)
			, Color(InColor)
			//, ArrayIndex(NULL)
		{}

		constexpr static std::size_t SizeWithoutPadding = sizeof(FVector) + sizeof(FColor);
	};
	FVector position;
	FVector normal;
	FVector2 texCoord;
	FColor Color;
	FVector tangent;
	FVector binormal;
	std::uint32_t ArrayIndex;

	sVertexLayout()
		: position(FVector::Zero())
		, normal(FVector::Zero())
		, texCoord(FVector2::Zero())
		, Color(FColor::White())
		, tangent(FVector::Zero())
		, binormal(FVector::Zero())
		, ArrayIndex(NULL)
	{}
	sVertexLayout(FVector InPosition, FVector2 InTexCoord, FColor InColor = FColor::White())
		: position(InPosition)
		, normal(FVector::Zero())
		, texCoord(InTexCoord)
		, Color(InColor)
		, tangent(FVector::Zero())
		, binormal(FVector::Zero())
		, ArrayIndex(NULL)
	{}
	sVertexLayout(FVector InPosition, FVector InNormal, FVector2 InTexCoord, FColor InColor, 
		FVector InTangent, FVector InBinormal, std::uint32_t InArrayIndex)
		: position(InPosition)
		, normal(InNormal)
		, texCoord(InTexCoord)
		, Color(InColor)
		, tangent(InTangent)
		, binormal(InBinormal)
		, ArrayIndex(InArrayIndex)
	{}

	constexpr static std::size_t SizeWithoutPadding = sizeof(FVector) + sizeof(FVector) + sizeof(FVector2) + sizeof(FColor) + sizeof(FVector) + sizeof(FVector) + sizeof(std::uint32_t);
};

struct sParticleVertexLayout
{
	struct sParticleInstanceLayout
	{
		FVector position;
		//std::uint32_t padding = 0;
		FColor Color;

		sParticleInstanceLayout()
			: position(FVector::Zero())
			, Color(FColor::White())
		{}
		sParticleInstanceLayout(FVector InPosition, FColor InColor = FColor::White())
			: position(InPosition)
			, Color(InColor)
		{}

		constexpr static std::size_t SizeWithoutPadding = sizeof(FVector) + sizeof(FColor);
	};
	FVector position;
	FVector2 texCoord;
	FColor Color;

	sParticleVertexLayout()
		: position(FVector::Zero())
		, texCoord(FVector2::Zero())
		, Color(FColor::White())
	{}
	sParticleVertexLayout(FVector InPosition, FVector2 InTexCoord, FColor InColor = FColor::White())
		: position(InPosition)
		, texCoord(InTexCoord)
		, Color(InColor)
	{}

	constexpr static std::size_t SizeWithoutPadding = sizeof(FVector) + sizeof(FVector2) + sizeof(FColor);
};

struct sLineVertexBufferEntry
{
	struct sLineVertexBufferInstanceLayout
	{
		FVector position;
		FColor Color;

		sLineVertexBufferInstanceLayout()
			: position(FVector::Zero())
			, Color(FColor::White())
		{}
		sLineVertexBufferInstanceLayout(FVector InPosition, FColor InColor = FColor::White())
			: position(InPosition)
			, Color(InColor)
		{}

		constexpr static std::size_t SizeWithoutPadding = sizeof(FVector) + sizeof(FColor);
	};
	FVector position;
	FColor Color;

	sLineVertexBufferEntry()
		: position(FVector::Zero())
		, Color(FColor::White())
	{}
};

struct sVertexAttributeDesc
{
	std::string name = "";
	EFormat format = EFormat::UNKNOWN;
	//uint32_t Index;
	uint32_t InputSlot = 0;
	uint32_t offset = 0;
	bool isInstanced = false;
	std::size_t Stride = 0;

	static std::vector<sVertexAttributeDesc> GetDefaultMeshVertexLayout(bool bInstanced = false)
	{
		std::vector<sVertexAttributeDesc> VertexLayout;
		if (bInstanced)
		{
			VertexLayout =
			{
				{ "POSITION",		EFormat::RGB32_FLOAT,   0, offsetof(sVertexLayout, position),							false, sizeof(sVertexLayout) },
				{ "NORMAL",			EFormat::RGB32_FLOAT,   0, offsetof(sVertexLayout, normal),								false, sizeof(sVertexLayout) },
				{ "TEXCOORD",		EFormat::RG32_FLOAT,    0, offsetof(sVertexLayout, texCoord),							false, sizeof(sVertexLayout) },
				{ "COLOR",			EFormat::RGBA32_FLOAT,  0, offsetof(sVertexLayout, Color),								false, sizeof(sVertexLayout) },
				{ "TANGENT",		EFormat::RGB32_FLOAT,   0, offsetof(sVertexLayout, tangent),							false, sizeof(sVertexLayout) },
				{ "BINORMAL",		EFormat::RGB32_FLOAT,   0, offsetof(sVertexLayout, binormal),							false, sizeof(sVertexLayout) },
				{ "ARRAYINDEX",		EFormat::R32_UINT,	    0, offsetof(sVertexLayout, ArrayIndex),							false, sizeof(sVertexLayout) },
				{ "INSTANCEPOS",	EFormat::RGB32_FLOAT,	1, offsetof(sVertexLayout::sVertexInstanceLayout, position),    true, sizeof(sVertexLayout::sVertexInstanceLayout) },
				{ "INSTANCECOLOR",	EFormat::RGBA32_FLOAT,	1, offsetof(sVertexLayout::sVertexInstanceLayout, Color),		true, sizeof(sVertexLayout::sVertexInstanceLayout) },
			};
		}
		else
		{
			VertexLayout =
			{
				{ "POSITION",	EFormat::RGB32_FLOAT,   0, offsetof(sVertexLayout, position),    false, sizeof(sVertexLayout) },
				{ "NORMAL",		EFormat::RGB32_FLOAT,   0, offsetof(sVertexLayout, normal),      false, sizeof(sVertexLayout) },
				{ "TEXCOORD",	EFormat::RG32_FLOAT,    0, offsetof(sVertexLayout, texCoord),    false, sizeof(sVertexLayout) },
				{ "COLOR",		EFormat::RGBA32_FLOAT,  0, offsetof(sVertexLayout, Color),		 false, sizeof(sVertexLayout) },
				{ "TANGENT",	EFormat::RGB32_FLOAT,   0, offsetof(sVertexLayout, tangent),     false, sizeof(sVertexLayout) },
				{ "BINORMAL",	EFormat::RGB32_FLOAT,   0, offsetof(sVertexLayout, binormal),    false, sizeof(sVertexLayout) },
				{ "ARRAYINDEX",	EFormat::R32_UINT,	    0, offsetof(sVertexLayout, ArrayIndex),  false, sizeof(sVertexLayout) },
			};
		}

		return VertexLayout;
	}

	static std::vector<sVertexAttributeDesc> GetDefaultLineVertexLayout(bool bInstanced = false)
	{
		std::vector<sVertexAttributeDesc> VertexLayout;
		if (bInstanced)
		{
			VertexLayout =
			{
				{ "POSITION",		EFormat::RGB32_FLOAT,   0, offsetof(sLineVertexBufferEntry, position),									  false, sizeof(sLineVertexBufferEntry) },
				{ "COLOR",			EFormat::RGBA32_FLOAT,  0, offsetof(sLineVertexBufferEntry, Color),										  false, sizeof(sLineVertexBufferEntry) },
				{ "INSTANCEPOS",	EFormat::RGB32_FLOAT,	1, offsetof(sLineVertexBufferEntry::sLineVertexBufferInstanceLayout, position),	  true, sizeof(sLineVertexBufferEntry::sLineVertexBufferInstanceLayout) },
				{ "INSTANCECOLOR",	EFormat::RGBA32_FLOAT,	1, offsetof(sLineVertexBufferEntry::sLineVertexBufferInstanceLayout, Color),	  true, sizeof(sLineVertexBufferEntry::sLineVertexBufferInstanceLayout) },
			};
		}
		else
		{
			VertexLayout =
			{
				{ "POSITION",	EFormat::RGB32_FLOAT,   0, offsetof(sLineVertexBufferEntry, position),    false, sizeof(sLineVertexBufferEntry) },
				{ "COLOR",		EFormat::RGBA32_FLOAT,  0, offsetof(sLineVertexBufferEntry, Color),       false, sizeof(sLineVertexBufferEntry) },
			};
		}

		return VertexLayout;
	}

	static std::vector<sVertexAttributeDesc> GetDefaultParticleVertexLayout()
	{
		std::vector<sVertexAttributeDesc> VertexLayout =
		{
			{ "POSITION",		 EFormat::RGB32_FLOAT,   0, offsetof(sParticleVertexLayout, position),	false, sizeof(sParticleVertexLayout) },
			{ "TEXCOORD",		 EFormat::RG32_FLOAT,    0, offsetof(sParticleVertexLayout, texCoord),	false, sizeof(sParticleVertexLayout) },
			{ "COLOR",			 EFormat::RGBA32_FLOAT,  0, offsetof(sParticleVertexLayout, Color),		false, sizeof(sParticleVertexLayout) },
			{ "INSTANCEPOS",	 EFormat::RGB32_FLOAT,	 1, offsetof(sParticleVertexLayout::sParticleInstanceLayout, position),		true, sizeof(sParticleVertexLayout::sParticleInstanceLayout) },
			{ "INSTANCECOLOR",	 EFormat::RGBA32_FLOAT,	 1, offsetof(sParticleVertexLayout::sParticleInstanceLayout, Color),		true, sizeof(sParticleVertexLayout::sParticleInstanceLayout) },
		};
		return VertexLayout;
	}

	static std::vector<sVertexAttributeDesc> GetDefaultGUIVertexLayout(bool bInstanced = false);
};

enum class EDrawTypes
{
	Undefined,
	Draw,
	DrawInstanced,
	DrawIndexedInstanced,
	Indirect,
	Dispatch,
};

struct sObjectDrawParameters
{
	std::uint32_t IndexCountPerInstance;
	std::uint32_t InstanceCount;
	std::uint32_t StartIndexLocation;
	std::int32_t BaseVertexLocation;
	std::uint32_t StartInstanceLocation;

	explicit sObjectDrawParameters(std::uint32_t InIndexCountPerInstance = NULL)
		: IndexCountPerInstance(InIndexCountPerInstance)
		, InstanceCount(1)
		, StartIndexLocation(NULL)
		, BaseVertexLocation(NULL)
		, StartInstanceLocation(NULL)
	{}
};

struct sMeshData
{
	std::vector<std::uint32_t> Indices;
	std::vector<sVertexLayout> Vertices;
	std::vector<sVertexLayout::sVertexInstanceLayout> InstanceData;
	sObjectDrawParameters DrawParameters;
};

struct BufferLayout
{
	std::uint64_t Size;
	std::uint64_t Stride;

	explicit BufferLayout()
		: Size(NULL)
		, Stride(NULL)
	{}

	BufferLayout(std::uint64_t InBufferSize, std::uint64_t InStride = NULL)
		: Size(InBufferSize)
		, Stride(InStride)
	{}

	~BufferLayout()
	{
		Size = NULL;
		Stride = NULL;
	}
};

struct BufferSubresource
{
	void* pSysMem;
	std::size_t Size;
	std::size_t Location;

	constexpr BufferSubresource(void* InData = nullptr, std::size_t InSize = NULL, std::size_t InLocation = NULL)
		: pSysMem(InData)
		, Size(InSize)
		, Location(InLocation)
	{}
};

struct TextureSubresource
{
	void* pData;
	std::size_t RowPitch;
	std::size_t SlicePitch;

	constexpr TextureSubresource(void* InData = nullptr, std::size_t InRowPitch = NULL, std::size_t InSlicePitch = NULL)
		: pData(InData)
		, RowPitch(InRowPitch)
		, SlicePitch(InSlicePitch)
	{
	}
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

struct ResourceSharedHandle
{
	EGITypes GIType = EGITypes::Undefined;
	std::uint32_t DeviceIndex = -1;
	void* ResourceHandle = nullptr;
};

class IConstantBuffer
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IConstantBuffer)
public:
	static IConstantBuffer::SharedPtr Create(std::string InName, const BufferLayout& InDesc, std::uint32_t InRootParameterIndex, std::uint32_t GPUIndex = 0);
	static IConstantBuffer::UniquePtr CreateUnique(std::string InName, const BufferLayout& InDesc, std::uint32_t InRootParameterIndex, std::uint32_t GPUIndex = 0);

public:
	virtual std::string GetName() const = 0;
	virtual std::uint32_t GetBindlessIndex() const = 0;
	virtual void SetDefaultRootParameterIndex(std::uint32_t RootParameterIndex) = 0;
	virtual std::uint32_t GetDefaultRootParameterIndex() const = 0;
	virtual void Map(const void* Ptr, IGraphicsCommandContext* InCMDBuffer = nullptr) = 0;

	virtual ResourceSharedHandle* GetSharedHandle() const = 0;
	virtual bool CopyFrom(IConstantBuffer* ConstantBuffer) = 0;
};

class IVertexBuffer
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IVertexBuffer)
public:
	static IVertexBuffer::SharedPtr Create(std::string InName, const BufferLayout& InDesc, BufferSubresource* InSubresource = nullptr, std::uint32_t GPUIndex = 0);
	static IVertexBuffer::UniquePtr CreateUnique(std::string InName, const BufferLayout& InDesc, BufferSubresource* InSubresource = nullptr, std::uint32_t GPUIndex = 0);

public:
	virtual std::string GetName() const = 0;
	virtual std::size_t GetSize() const = 0;
	virtual bool IsMapable() const = 0;
	virtual void UpdateSubresource(BufferSubresource* Subresource, IGraphicsCommandContext* InCMDBuffer = nullptr) = 0;

	virtual ResourceSharedHandle* GetSharedHandle() const = 0;
	virtual bool CopyFrom(IVertexBuffer* VertexBuffer) = 0;
};

class IIndexBuffer
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IIndexBuffer)
public:
	static IIndexBuffer::SharedPtr Create(std::string InName, const BufferLayout& InDesc, BufferSubresource* InSubresource = nullptr, std::uint32_t GPUIndex = 0);
	static IIndexBuffer::UniquePtr CreateUnique(std::string InName, const BufferLayout& InDesc, BufferSubresource* InSubresource = nullptr, std::uint32_t GPUIndex = 0);

public:
	virtual std::string GetName() const = 0;
	virtual std::size_t GetSize() const = 0;
	virtual bool IsMapable() const = 0;
	virtual void UpdateSubresource(BufferSubresource* Subresource, IGraphicsCommandContext* InCMDBuffer = nullptr) = 0;

	virtual ResourceSharedHandle* GetSharedHandle() const = 0;
	virtual bool CopyFrom(IIndexBuffer* IndexBuffer) = 0;
};

class IByteAddressBuffer
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IByteAddressBuffer)
public:
	/*
	* size should be a multiple of 4
	*/
	static IByteAddressBuffer::SharedPtr Create(std::string InName, std::uint64_t Size, bool bReadWriteAllowed = false, std::uint32_t GPUIndex = 0);
	static IByteAddressBuffer::UniquePtr CreateUnique(std::string InName, std::uint64_t Size, bool bReadWriteAllowed = false, std::uint32_t GPUIndex = 0);

public:
	virtual std::string GetName() const = 0;
	virtual std::uint32_t GetBindlessIndex() const = 0;

	virtual bool IsReadWriteAllowed() const = 0;
	virtual std::uint64_t GetSize() const = 0;

	virtual bool IsMapable() const = 0;
	virtual void Map(const void* Ptr, std::size_t Location, std::uint32_t Stride, IGraphicsCommandContext* InCMDBuffer = nullptr) = 0;

	virtual ResourceSharedHandle* GetSharedHandle() const = 0;
	virtual bool CopyFrom(IByteAddressBuffer* UnorderedAccessBuffer) = 0;
};

class IStructuredBuffer
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IStructuredBuffer)
public:
	static IStructuredBuffer::SharedPtr Create(std::string InName, const BufferLayout& InDesc, bool bSRVAllowed = true, std::uint32_t GPUIndex = 0);
	static IStructuredBuffer::UniquePtr CreateUnique(std::string InName, const BufferLayout& InDesc, bool bSRVAllowed = true, std::uint32_t GPUIndex = 0);

public:
	virtual std::string GetName() const = 0;
	virtual std::uint32_t GetBindlessIndex() const = 0;

	virtual bool IsSRV_Allowed() const = 0;
	virtual std::size_t GetSize() const = 0;

	virtual bool IsMapable() const = 0;
	virtual void Map(const void* Ptr, std::size_t Location, IGraphicsCommandContext* InCMDBuffer = nullptr) = 0;

	virtual ResourceSharedHandle* GetSharedHandle() const = 0;
	virtual bool CopyFrom(IStructuredBuffer* UnorderedAccessBuffer) = 0;
};

class IIndirectBuffer
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IIndirectBuffer)
public:
	static IIndirectBuffer::SharedPtr Create(std::string InName, BufferLayout Layout, std::uint32_t GPUIndex = 0);
	static IIndirectBuffer::UniquePtr CreateUnique(std::string InName, BufferLayout Layout, std::uint32_t GPUIndex = 0);

public:
	virtual std::string GetName() const = 0;

	virtual std::uint64_t GetSize() const = 0;
	virtual std::uint64_t GetTotalCommandSize() const = 0;
	virtual std::uint64_t GetCurrentCommandSize() const = 0;
	virtual std::uint64_t GetStride() const = 0;
	virtual std::uint64_t GetOffset() const = 0;

	virtual void SetArgument(void* InArgument, std::size_t NewCommandSize = 1, std::uint64_t NewOffset = 0) = 0;

	virtual ResourceSharedHandle* GetSharedHandle() const = 0;
	virtual bool CopyFrom(IIndirectBuffer* IndirectBuffer) = 0;
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

enum class EFrameBufferAttachmentType
{
	RT,
	RT_SRV,
	RT_UAV,
	RT_SRV_UAV,
	Depth,
	Depth_SRV,
	Depth_UAV,
	Depth_SRV_UAV,
	UAV,
	UAV_SRV
};

struct sFrameBuffer
{
	EFormat Format;
	EFrameBufferAttachmentType AttachmentType;
	sFrameBuffer(const EFormat& InFormat, const EFrameBufferAttachmentType InAttachmentType = EFrameBufferAttachmentType::RT_SRV)
		: Format(InFormat)
		, AttachmentType(InAttachmentType)
	{}
};

struct sFrameBufferAttachmentInfo
{
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
	std::vector<sFrameBuffer> GetAttachments(bool WithDepth = false) const
	{
		std::vector<sFrameBuffer> Attachments = FrameBuffer;
		if (WithDepth && DepthFormat != EFormat::UNKNOWN)
			Attachments.push_back(sFrameBuffer(DepthFormat, EFrameBufferAttachmentType::Depth));
		return Attachments;
	}
	std::size_t GetRenderTargetAttachmentCount() const { return FrameBuffer.size(); }
	void AddFrameBuffer(const EFormat Format, const EFrameBufferAttachmentType InAttachmentType = EFrameBufferAttachmentType::RT_SRV)
	{
		FrameBuffer.push_back(sFrameBuffer(Format, InAttachmentType));
	}
};

enum class EResourceState
{
	Common,
	RenderTarget,
	Depth,
	ShaderResource,
	NonPixelShaderResource,
	UAV,
};

class IUnorderedAccessTarget;
class IRenderTarget
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IRenderTarget)
public:
	static IRenderTarget::SharedPtr Create(const std::string InName, const sFrameBuffer& Desc, const sFBODesc& FBODesc, std::uint32_t GPUIndex = 0);
	static IRenderTarget::UniquePtr CreateUnique(const std::string InName, const sFrameBuffer& Desc, const sFBODesc& FBODesc, std::uint32_t GPUIndex = 0);

public:
	virtual void AsResource(EResourceState ResourceState, IGraphicsCommandContext* GraphicsCommandContext) = 0;

	virtual bool IsSRV_Allowed() const = 0;
	virtual bool IsUAV_Allowed() const = 0;
	virtual std::uint32_t GetSRVBindlessIndex() const = 0;
	virtual std::uint32_t GetUAVBindlessIndex() const = 0;

	virtual void* GetNativeTexture() const = 0;

	virtual void SetDefaultRootParameterIndex(std::uint32_t RootParameterIndex) = 0;
	virtual std::uint32_t GetDefaultRootParameterIndex() const = 0;

	virtual ResourceSharedHandle* GetSharedHandle() const = 0;
	virtual bool CopyFrom(IRenderTarget* RenderTarget) = 0;
};

class IDepthTarget
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IDepthTarget)
public:
	static IDepthTarget::SharedPtr Create(const std::string InName, const EFormat Format, const sFBODesc& Desc, std::uint32_t GPUIndex = 0);
	static IDepthTarget::UniquePtr CreateUnique(const std::string InName, const EFormat Format, const sFBODesc& Desc, std::uint32_t GPUIndex = 0);

public:
	virtual void AsResource(EResourceState ResourceState, IGraphicsCommandContext* GraphicsCommandContext) = 0;

	virtual bool IsSRV_Allowed() const = 0;
	virtual bool IsUAV_Allowed() const = 0;

	virtual void* GetNativeTexture() const = 0;

	virtual void SetDefaultRootParameterIndex(std::uint32_t RootParameterIndex) = 0;
	virtual std::uint32_t GetDefaultRootParameterIndex() const = 0;

	virtual ResourceSharedHandle* GetSharedHandle() const = 0;
	virtual bool CopyFrom(IDepthTarget* DepthTarget) = 0;
};

class IUnorderedAccessTarget
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IUnorderedAccessTarget)
public:
	static IUnorderedAccessTarget::SharedPtr Create(const std::string InName, const EFormat Format, const sFBODesc& Desc, bool InEnableSRV = true, std::uint32_t GPUIndex = 0);
	static IUnorderedAccessTarget::UniquePtr CreateUnique(const std::string InName, const EFormat Format, const sFBODesc& Desc, bool InEnableSRV = true, std::uint32_t GPUIndex = 0);

public:
	//virtual void AsResource(EResourceState ResourceState, IGraphicsCommandContext* GraphicsCommandContext) = 0;

	virtual bool IsSRV_Allowed() const = 0;
	virtual std::uint32_t GetSRVBindlessIndex() const = 0;
	virtual std::uint32_t GetUAVBindlessIndex() const = 0;

	virtual void* GetNativeTexture() const = 0;

	virtual void SetDefaultRootParameterIndex(std::uint32_t RootParameterIndex) = 0;
	virtual std::uint32_t GetDefaultRootParameterIndex() const = 0;

	virtual ResourceSharedHandle* GetSharedHandle() const = 0;
	virtual bool CopyFrom(IUnorderedAccessTarget* UnorderedAccessTarget) = 0;
};

class IFrameBuffer
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IFrameBuffer)
public:
	static IFrameBuffer::SharedPtr Create(const std::string InName, const sFrameBufferAttachmentInfo& InAttachments, std::uint32_t GPUIndex = 0);
	static IFrameBuffer::UniquePtr CreateUnique(const std::string InName, const sFrameBufferAttachmentInfo& InAttachments, std::uint32_t GPUIndex = 0);

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

	virtual bool CopyFrom(IFrameBuffer* FrameBuffer) = 0;
};

enum class ERasterizerCullMode
{
	None,
	CW,
	CCW,
};

enum class ERasterizerFillMode
{
	Point,
	Wireframe,
	Solid,
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

	sRasterizerAttributeDesc(ERasterizerCullMode InMode = ERasterizerCullMode::CW)
		: FillMode(ERasterizerFillMode::Solid)
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
		, CullMode(InMode == ERasterizerFillMode::Wireframe ? ERasterizerCullMode::None : ERasterizerCullMode::CW)
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
	Less,
	LessEqual,
	Greater,
	GreaterEqual,
	Equal,
	NotEqual,
	Never,
	Always,
};

enum class ESamplerFilter
{
	Point,
	Bilinear,
	Trilinear,
	AnisotropicPoint,
	AnisotropicLinear,
};

enum class ESamplerAddressMode
{
	Wrap,
	Clamp,
	Mirror,
	MirrorOnce,
	Border,
};

enum class ESamplerStateMode
{
	PointWrap,
	PointClamp,
	PointBorder,
	LinearWrap,
	LinearClamp,
	AnisotropicWrap,
	AnisotropicClamp,
	AnisotropicLinear,
};

struct sSamplerAttributeDesc
{
	sSamplerAttributeDesc(ESamplerStateMode InMode = ESamplerStateMode::PointWrap)
		: Filter(ESamplerFilter::Point)
		, AddressU(ESamplerAddressMode::Clamp)
		, AddressV(ESamplerAddressMode::Clamp)
		, AddressW(ESamplerAddressMode::Clamp)
		, MipBias(0)
		, MinMipLevel(0)
		, MaxMipLevel(FLT_MAX)
		, MaxAnisotropy(1)
		, BorderColor(FColor::White())
		, bUINTBorderColor(false)
		, SamplerComparisonFunction(ECompareFunction::Never)
	{
		switch (InMode)
		{
		case ESamplerStateMode::PointWrap:
			Filter = ESamplerFilter::Point;
			AddressU = ESamplerAddressMode::Wrap;
			AddressV = ESamplerAddressMode::Wrap;
			AddressW = ESamplerAddressMode::Wrap;
			break;
		case ESamplerStateMode::PointClamp:
			Filter = ESamplerFilter::Point;
			AddressU = ESamplerAddressMode::Clamp;
			AddressV = ESamplerAddressMode::Clamp;
			AddressW = ESamplerAddressMode::Clamp;
			break;
		case ESamplerStateMode::PointBorder:
			Filter = ESamplerFilter::Point;
			AddressU = ESamplerAddressMode::Border;
			AddressV = ESamplerAddressMode::Border;
			AddressW = ESamplerAddressMode::Border;
			break;
		case ESamplerStateMode::LinearWrap:
			Filter = ESamplerFilter::Bilinear;
			AddressU = ESamplerAddressMode::Wrap;
			AddressV = ESamplerAddressMode::Wrap;
			AddressW = ESamplerAddressMode::Wrap;
			break;
		case ESamplerStateMode::LinearClamp:
			Filter = ESamplerFilter::Bilinear;
			AddressU = ESamplerAddressMode::Clamp;
			AddressV = ESamplerAddressMode::Clamp;
			AddressW = ESamplerAddressMode::Clamp;
			break;
		case ESamplerStateMode::AnisotropicWrap:
			Filter = ESamplerFilter::AnisotropicPoint;
			AddressU = ESamplerAddressMode::Wrap;
			AddressV = ESamplerAddressMode::Wrap;
			AddressW = ESamplerAddressMode::Wrap;
			break;
		case ESamplerStateMode::AnisotropicClamp:
			Filter = ESamplerFilter::AnisotropicPoint;
			AddressU = ESamplerAddressMode::Clamp;
			AddressV = ESamplerAddressMode::Clamp;
			AddressW = ESamplerAddressMode::Clamp;
			break;
		case ESamplerStateMode::AnisotropicLinear:
			Filter = ESamplerFilter::AnisotropicLinear;
			AddressU = ESamplerAddressMode::Wrap;
			AddressV = ESamplerAddressMode::Wrap;
			AddressW = ESamplerAddressMode::Wrap;
			SamplerComparisonFunction = ECompareFunction::Less;
			break;
		}
	}

	void SetToAddressToWrap()
	{
		AddressU = ESamplerAddressMode::Wrap;
		AddressV = ESamplerAddressMode::Wrap;
		AddressW = ESamplerAddressMode::Wrap;
	}

	void SetToAddressToClamp()
	{
		AddressU = ESamplerAddressMode::Clamp;
		AddressV = ESamplerAddressMode::Clamp;
		AddressW = ESamplerAddressMode::Clamp;
	}

	ESamplerFilter Filter;
	ESamplerAddressMode AddressU;
	ESamplerAddressMode AddressV;
	ESamplerAddressMode AddressW;
	std::uint32_t MipBias;
	float MinMipLevel;
	float MaxMipLevel;
	std::uint32_t MaxAnisotropy;
	FColor BorderColor;
	bool bUINTBorderColor;
	ECompareFunction SamplerComparisonFunction;

#if _MSVC_LANG >= 202002L
	constexpr auto operator<=>(const sSamplerAttributeDesc&) const = default;
#endif
};

#if _MSVC_LANG < 202002L
FORCEINLINE bool constexpr operator ==(const sSamplerAttributeDesc& value1, const sSamplerAttributeDesc& value2)
{
	return value1.Filter == value2.Filter && value1.AddressU == value2.AddressU && value1.AddressV == value2.AddressV &&
		   value1.AddressW == value2.AddressW && value1.MipBias == value2.MipBias && value1.MinMipLevel == value2.MinMipLevel &&
		   value1.MinMipLevel == value2.MinMipLevel && value1.MaxAnisotropy == value2.MaxAnisotropy && value1.BorderColor == value2.BorderColor &&
		   value1.bUINTBorderColor == value2.bUINTBorderColor && value1.SamplerComparisonFunction == value2.SamplerComparisonFunction;
};

FORCEINLINE bool constexpr operator !=(const sSamplerAttributeDesc& value1, const sSamplerAttributeDesc& value2)
{
	return value1.Filter != value2.Filter && value1.AddressU != value2.AddressU && value1.AddressV != value2.AddressV &&
		   value1.AddressW != value2.AddressW && value1.MipBias != value2.MipBias && value1.MinMipLevel != value2.MinMipLevel &&
		   value1.MinMipLevel != value2.MinMipLevel && value1.MaxAnisotropy != value2.MaxAnisotropy && value1.BorderColor != value2.BorderColor &&
		   value1.bUINTBorderColor != value2.bUINTBorderColor && value1.SamplerComparisonFunction != value2.SamplerComparisonFunction;
};
#endif

struct sSamplerAttributeDescHash
{
	std::size_t operator()(const sSamplerAttributeDesc& s) const
	{
		std::size_t seed = 0;
		hash_combine(seed, std::hash<ESamplerFilter>{}(s.Filter));
		hash_combine(seed, std::hash<ESamplerAddressMode>{}(s.AddressU));
		hash_combine(seed, std::hash<ESamplerAddressMode>{}(s.AddressV));
		hash_combine(seed, std::hash<ESamplerAddressMode>{}(s.AddressW));
		hash_combine(seed, std::hash<std::uint32_t>{}(s.MipBias));
		hash_combine(seed, std::hash<float>{}(s.MinMipLevel));
		hash_combine(seed, std::hash<float>{}(s.MaxMipLevel));
		hash_combine(seed, std::hash<std::uint32_t>{}(s.MaxAnisotropy));
		hash_combine(seed, std::hash<float>{}(s.BorderColor.R));
		hash_combine(seed, std::hash<float>{}(s.BorderColor.G));
		hash_combine(seed, std::hash<float>{}(s.BorderColor.B));
		hash_combine(seed, std::hash<float>{}(s.BorderColor.A));
		hash_combine(seed, std::hash<bool>{}(s.bUINTBorderColor));
		hash_combine(seed, std::hash<ECompareFunction>{}(s.SamplerComparisonFunction));
		return seed;
	}
};

class ISamplerState
{
	sBaseClassBody(sClassDefaultProtectedConstructor, ISamplerState)
public:
	static ISamplerState::SharedPtr Create(const std::string InName, const sSamplerAttributeDesc& InDesc, std::uint32_t GPUIndex = 0);
	static ISamplerState::UniquePtr CreateUnique(const std::string InName, const sSamplerAttributeDesc& InDesc, std::uint32_t GPUIndex = 0);

public:
	virtual std::string GetName() const = 0;
	virtual sSamplerAttributeDesc GetSamplerDesc() const = 0;
	virtual std::uint32_t GetBindlessIndex() const = 0;
};

enum class EStencilOp
{
	Keep,
	Zero,
	Replace,
	SaturatedIncrement,
	SaturatedDecrement,
	Invert,
	Increment,
	Decrement,
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

	sDepthStencilAttributeDesc(ECompareFunction DepthTestFunc = ECompareFunction::LessEqual, bool DepthWrite = true, bool StencilEnable = false)
		: bEnableDepthWrite(DepthWrite)
		, bStencilEnable(StencilEnable)
		, bDepthWriteMask(true)
		, DepthTest(DepthTestFunc)
		, bEnableFrontFaceStencil(false)
		, FrontFaceStencilTest(ECompareFunction::Always)
		, FrontFaceStencilFailStencilOp(EStencilOp::Keep)
		, FrontFaceDepthFailStencilOp(EStencilOp::Keep)
		, FrontFacePassStencilOp(EStencilOp::Keep)
		, bEnableBackFaceStencil(false)
		, BackFaceStencilTest(ECompareFunction::Always)
		, BackFaceStencilFailStencilOp(EStencilOp::Keep)
		, BackFaceDepthFailStencilOp(EStencilOp::Keep)
		, BackFacePassStencilOp(EStencilOp::Keep)
		, StencilReadMask(0xFF)
		, StencilWriteMask(0xFF)
	{}
};

enum class EBlendFactor
{
	Zero,
	One,
	SourceColor,
	InverseSourceColor,
	SourceAlpha,
	InverseSourceAlpha,
	DestAlpha,
	InverseDestAlpha,
	DestColor,
	InverseDestColor,
	BlendFactor,
	InverseBlendFactor,
};

enum class EColorWriteMask
{
	NONE = 0,
	RED = 0x01,
	GREEN = 0x02,
	BLUE = 0x04,
	ALPHA = 0x08,

	RGB = RED | GREEN | BLUE,
	RGBA = RED | GREEN | BLUE | ALPHA,
	RG = RED | GREEN,
	BA = BLUE | ALPHA,
};

enum class EBlendOperation
{
	Add,
	Subtract,
	Min,
	Max,
	ReverseSubtract,
};

enum class EBlendStateMode
{
	Opaque,
	AlphaBlend,
	Additive,
	NonPremultiplied,
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
			EBlendOperation InColorBlendOp = EBlendOperation::Add,
			EBlendFactor InColorSrcBlend = EBlendFactor::One,
			EBlendFactor InColorDestBlend = EBlendFactor::Zero,
			EBlendOperation InAlphaBlendOp = EBlendOperation::Add,
			EBlendFactor InAlphaSrcBlend = EBlendFactor::One,
			EBlendFactor InAlphaDestBlend = EBlendFactor::Zero,
			EColorWriteMask InColorWriteMask = EColorWriteMask::RGBA
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
			, ColorBlendOp(EBlendOperation::Add)
			, ColorSrcBlend(EBlendFactor::One)
			, ColorDestBlend(EBlendFactor::Zero)
			, AlphaBlendOp(EBlendOperation::Add)
			, AlphaSrcBlend(EBlendFactor::One)
			, AlphaDestBlend(EBlendFactor::Zero)
			, ColorWriteMask(EColorWriteMask::RGBA)
		{
			switch (InMode)
			{
			case EBlendStateMode::Opaque:
				ColorSrcBlend = EBlendFactor::One;
				ColorDestBlend = EBlendFactor::Zero;
				break;
			case EBlendStateMode::AlphaBlend:
				ColorSrcBlend = EBlendFactor::One;
				ColorDestBlend = EBlendFactor::InverseSourceAlpha;
				break;
			case EBlendStateMode::Additive:
				ColorSrcBlend = EBlendFactor::SourceAlpha;
				ColorDestBlend = EBlendFactor::One;
				break;
			case EBlendStateMode::NonPremultiplied:
				ColorSrcBlend = EBlendFactor::SourceAlpha;
				ColorDestBlend = EBlendFactor::InverseSourceAlpha;
				AlphaDestBlend = EBlendFactor::SourceAlpha;
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
	UNDEFINED,
	POINT_LIST,
	TRIANGLE_LIST,
	TRIANGLE_STRIP,
	LINE_LIST,
};

enum class EDescriptorType : std::uint8_t
{
	UniformBuffer,
	UAV,
	e32BitConstant,
	Texture,
	Sampler,
	StaticSampler,
};

struct sShaderBinding
{
public:
	EDescriptorType DescriptorType;

	std::uint32_t Location;
	eShaderType ShaderType;
	std::uint32_t Size;
	std::uint32_t RegisterSpace;

	sSamplerAttributeDesc SamplerDesc;

	sShaderBinding(EDescriptorType InType, eShaderType InShaderType, std::uint32_t InLocation, std::uint32_t InSize = 1, std::uint32_t InRegisterSpace = 0)
		: Location(InLocation)
		, DescriptorType(InType)
		, ShaderType(InShaderType)
		, Size(InSize)
		, RegisterSpace(InRegisterSpace)
		, SamplerDesc(sSamplerAttributeDesc())
	{}
	sShaderBinding(sSamplerAttributeDesc Sampler, eShaderType InShaderType, std::uint32_t InLocation, std::uint32_t InSize = 1, std::uint32_t InRegisterSpace = 0)
		: Location(InLocation)
		, DescriptorType(EDescriptorType::StaticSampler)
		, ShaderType(InShaderType)
		, Size(InSize)
		, RegisterSpace(InRegisterSpace)
		, SamplerDesc(Sampler)
	{}

	EDescriptorType GetDescriptorType() const { return DescriptorType; }

#if _MSVC_LANG >= 202002L
	//constexpr auto operator<=>(const sShaderBinding&) const = default;
#endif

	bool operator==(const sShaderBinding& rhs)
	{
		return DescriptorType == rhs.DescriptorType && SamplerDesc == rhs.SamplerDesc
			&& Location == rhs.Location && ShaderType == rhs.ShaderType && Size == rhs.Size && RegisterSpace == rhs.RegisterSpace;
	}
};

struct sDescriptorSetLayoutBindingHash
{
	std::size_t operator()(const sShaderBinding& s) const
	{
		std::size_t seed = 0;
		hash_combine(seed, std::hash<EDescriptorType>{}(s.DescriptorType));
		hash_combine(seed, std::hash<std::uint32_t>{}(s.Location));
		hash_combine(seed, std::hash<eShaderType>{}(s.ShaderType));
		hash_combine(seed, std::hash<std::uint32_t>{}(s.Size));
		hash_combine(seed, std::hash<std::uint32_t>{}(s.RegisterSpace));
		hash_combine(seed, sSamplerAttributeDescHash{}(s.SamplerDesc));
		return seed;
	}
};

struct sDescriptorSetLayoutBindingVectorHash
{
	std::size_t operator()(const std::vector<sShaderBinding>& v) const {
		std::size_t seed = v.size();
		for (const auto& s : v)
			hash_combine(seed, sDescriptorSetLayoutBindingHash{}(s));
		return seed;
	}
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

struct sShaderAttachment
{
public:
	std::wstring Location;
	void* ByteCode;
	std::size_t Size;

	std::string FunctionName;
	eShaderType Type;
	std::vector<sShaderDefines> ShaderDefines;

	sShaderAttachment(std::wstring InLocation, std::string inEntryName, eShaderType InType, const std::vector<sShaderDefines> InShaderDefines = std::vector<sShaderDefines>())
		: Location(InLocation)
		, ByteCode(nullptr)
		, Size(0)
		, FunctionName(inEntryName)
		, Type(InType)
		, ShaderDefines(InShaderDefines)
	{}
	sShaderAttachment(void* pByteCode, std::size_t inSize, std::string inEntryName, eShaderType InType, const std::vector<sShaderDefines> InShaderDefines = std::vector<sShaderDefines>())
		: Location(L"")
		, ByteCode(pByteCode)
		, Size(inSize)
		, FunctionName(inEntryName)
		, Type(InType)
		, ShaderDefines(InShaderDefines)
	{}

	~sShaderAttachment()
	{
		ShaderDefines.clear();
	}

	std::wstring GetLocation() const { return Location; }
	bool IsCodeValid() const { return ByteCode != nullptr; }
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
	static IShader::SharedPtr Create(const sShaderAttachment& Attachment, std::uint32_t GPUIndex = 0);
	static IShader::SharedPtr Create(std::wstring InSrcFile, std::string InFunctionName, eShaderType InProfile, std::vector<sShaderDefines> InDefines = std::vector<sShaderDefines>(), std::uint32_t GPUIndex = 0);
	static IShader::SharedPtr Create(const void* InCode, std::size_t Size, std::string InFunctionName, eShaderType InProfile, std::vector<sShaderDefines> InDefines = std::vector<sShaderDefines>(), std::uint32_t GPUIndex = 0);

public:
	virtual std::string GetName() const = 0;
	virtual std::wstring GetPath() const = 0;
	virtual eShaderType Type() const = 0;
	virtual void* GetByteCode() const = 0;
	virtual std::size_t GetByteCodeSize() const = 0;
};

struct sIndirectLayoutBindingDesc
{
	sBaseClassBody(sClassNoDefaults, sIndirectLayoutBindingDesc)
public:
	sIndirectLayoutBindingDesc(EDrawTypes InDrawType = EDrawTypes::Undefined, std::uint32_t InNumArguments = std::uint32_t(-1), std::uint32_t InStride = std::uint32_t(-1), std::uint32_t InDrawIndex = std::uint32_t(-1), std::vector<sShaderBinding> NewBindings = std::vector<sShaderBinding>())
		: DrawType(InDrawType)
		, DrawIndex(InDrawIndex)
		, NumArguments(InNumArguments)
		, Stride(InStride)
		, CustomBindings(NewBindings)
	{}

	EDrawTypes DrawType = EDrawTypes::Undefined;
	std::uint32_t DrawIndex = std::uint32_t(-1);
	std::uint32_t NumArguments = std::uint32_t(-1);
	std::uint32_t Stride = std::uint32_t(-1);

	std::vector<sShaderBinding> CustomBindings;
};

struct sPipelineDesc
{
	sBaseClassBody(sClassNoDefaults, sPipelineDesc)
public:
	sPipelineDesc(ERenderPass InRenderPass = ERenderPass::NONE)
		: RenderPass(InRenderPass)
		, RasterizerAttribute(sRasterizerAttributeDesc())
		, DepthStencilAttribute(sDepthStencilAttributeDesc())
		, BlendAttribute(sBlendAttributeDesc())
		, PrimitiveTopologyType(EPrimitiveType::TRIANGLE_LIST)
		, IndirectLayoutBindingDesc(sIndirectLayoutBindingDesc())
	{}

	sPipelineDesc(ERenderPass InRenderPass, EBlendStateMode BlendStateMode, ECompareFunction DepthCompareFunction, bool bStencilEnabled, EVertexLayoutType VertexLayoutType, 
		bool bIsInstanced, sIndirectLayoutBindingDesc NewIndirectLayoutBindingDesc)
		: RenderPass(InRenderPass)
		, RasterizerAttribute(sRasterizerAttributeDesc())
		, DepthStencilAttribute(sDepthStencilAttributeDesc())
		, BlendAttribute(sBlendAttributeDesc())
		, PrimitiveTopologyType(EPrimitiveType::TRIANGLE_LIST)
		, IndirectLayoutBindingDesc(sIndirectLayoutBindingDesc())
	{
		DepthStencilAttribute = sDepthStencilAttributeDesc(DepthCompareFunction, true, bStencilEnabled);
		BlendAttribute = sBlendAttributeDesc(BlendStateMode);
		PrimitiveTopologyType = EPrimitiveType::TRIANGLE_LIST;
		IndirectLayoutBindingDesc = NewIndirectLayoutBindingDesc;

		switch (VertexLayoutType)
		{
		case EVertexLayoutType::DefaultVertexLayout:
			VertexLayout = sVertexAttributeDesc::GetDefaultMeshVertexLayout(bIsInstanced);
			break;
		case EVertexLayoutType::GUI:
			VertexLayout = sVertexAttributeDesc::GetDefaultGUIVertexLayout(bIsInstanced);
			break;
		case EVertexLayoutType::Particle:
			VertexLayout = sVertexAttributeDesc::GetDefaultParticleVertexLayout();
			break;
		case EVertexLayoutType::Line:
			VertexLayout = sVertexAttributeDesc::GetDefaultLineVertexLayout(bIsInstanced);
			break;
		}
	}

	~sPipelineDesc()
	{
		Bindings.clear();
		ShaderAttachments.clear();
		VertexLayout.clear();
	}

	ERenderPass RenderPass;
	sRasterizerAttributeDesc RasterizerAttribute;
	sDepthStencilAttributeDesc DepthStencilAttribute;
	sBlendAttributeDesc BlendAttribute;
	EPrimitiveType PrimitiveTopologyType;
	sIndirectLayoutBindingDesc IndirectLayoutBindingDesc;

	std::vector<sShaderBinding> Bindings;
	std::vector<sShaderAttachment> ShaderAttachments;
	std::vector<sVertexAttributeDesc> VertexLayout;

	static sPipelineDesc CreateDefaultPipelineDesc(ERenderPass InRenderPass, sBlendAttributeDesc BlendState = sBlendAttributeDesc(), sRasterizerAttributeDesc RasterizerDesc = sRasterizerAttributeDesc(), sDepthStencilAttributeDesc DepthStencilDesc = sDepthStencilAttributeDesc(), std::vector<sVertexAttributeDesc> VertexLayoutDesc = sVertexAttributeDesc::GetDefaultMeshVertexLayout(false), 
	  EPrimitiveType InPrimitiveType = EPrimitiveType::TRIANGLE_LIST, sIndirectLayoutBindingDesc IndirectLayoutBindingDesc = sIndirectLayoutBindingDesc(), 
	  std::vector<sShaderBinding> InBindings = std::vector<sShaderBinding>(), std::vector<sShaderAttachment> InShaderAttachments = std::vector<sShaderAttachment>())
	{
		sPipelineDesc Desc(InRenderPass);
		Desc.RasterizerAttribute = RasterizerDesc;
		Desc.DepthStencilAttribute = DepthStencilDesc;
		Desc.BlendAttribute = BlendState;
		Desc.PrimitiveTopologyType = InPrimitiveType;
		Desc.VertexLayout = VertexLayoutDesc;
		Desc.IndirectLayoutBindingDesc = IndirectLayoutBindingDesc;
		Desc.Bindings = InBindings;
		Desc.ShaderAttachments = InShaderAttachments;

		return Desc;
	}

	static sPipelineDesc CreateDefaultPipelineDesc(ERenderPass InRenderPass, EBlendStateMode BlendStateMode = EBlendStateMode::Opaque, ECompareFunction DepthCompareFunction = ECompareFunction::LessEqual, bool bStencilEnabled = true, EVertexLayoutType VertexLayoutType = EVertexLayoutType::DefaultVertexLayout, bool bIsInstanced = false, sIndirectLayoutBindingDesc IndirectLayoutBindingDesc = sIndirectLayoutBindingDesc())
	{
		sPipelineDesc Desc(InRenderPass);
		Desc.DepthStencilAttribute = sDepthStencilAttributeDesc(DepthCompareFunction, true, bStencilEnabled);
		Desc.BlendAttribute = sBlendAttributeDesc(BlendStateMode);
		Desc.PrimitiveTopologyType = EPrimitiveType::TRIANGLE_LIST;
		Desc.IndirectLayoutBindingDesc = IndirectLayoutBindingDesc;

		switch (VertexLayoutType)
		{
			case EVertexLayoutType::DefaultVertexLayout:
				Desc.VertexLayout = sVertexAttributeDesc::GetDefaultMeshVertexLayout(bIsInstanced);
				break;
			case EVertexLayoutType::GUI:
				Desc.VertexLayout = sVertexAttributeDesc::GetDefaultGUIVertexLayout(bIsInstanced);
				break;
			case EVertexLayoutType::Particle:
				Desc.VertexLayout = sVertexAttributeDesc::GetDefaultParticleVertexLayout();
				break;
			case EVertexLayoutType::Line:
				Desc.VertexLayout = sVertexAttributeDesc::GetDefaultLineVertexLayout(bIsInstanced);
				break;
		}

		return Desc;
	}
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
	virtual std::vector<sShaderBinding> GetDescriptorSetLayout() const = 0;
	virtual bool IsIndirectCommandAvailable() const = 0;
};

class IPipeline
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IPipeline)
public:
	static IPipeline::SharedPtr Create(const std::string& InName, const sPipelineDesc& InDesc, std::uint32_t GPUIndex = 0);
	static IPipeline::UniquePtr CreateUnique(const std::string& InName, const sPipelineDesc& InDesc, std::uint32_t GPUIndex = 0);

public:
	virtual sPipelineDesc GetPipelineDesc() const = 0;
	virtual ERenderPass GetRenderPass() const = 0;
	virtual IRootSignature* GetRootSignature() const = 0;
	virtual bool IsIndirectCommandAvailable() const = 0;
	virtual bool IsCompiled() const = 0;
	virtual bool Compile(IFrameBuffer* FrameBuffer = nullptr) = 0;
	virtual bool Compile(IRenderTarget* RT, IDepthTarget* Depth = nullptr) = 0;
	virtual bool Compile(std::vector<IRenderTarget*> RTs, IDepthTarget* Depth = nullptr) = 0;
	virtual bool Recompile() = 0;
};

struct sComputePipelineDesc
{
	sBaseClassBody(sClassNoDefaults, sComputePipelineDesc)
public:
	sComputePipelineDesc(const sShaderAttachment& InShaderAttachment, const std::vector<sShaderBinding>& InBindings)
		: ShaderAttachment(InShaderAttachment)
		, Bindings(InBindings)
	{}
	~sComputePipelineDesc()
	{
		Bindings.clear();
	}
	std::vector<sShaderBinding> Bindings;
	sShaderAttachment ShaderAttachment;
};

class IComputePipeline
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IComputePipeline)
public:
	static IComputePipeline::SharedPtr Create(const std::string& InName, const sComputePipelineDesc& InDesc, std::uint32_t GPUIndex = 0);
	static IComputePipeline::UniquePtr CreateUnique(const std::string& InName, const sComputePipelineDesc& InDesc, std::uint32_t GPUIndex = 0);

public:
	virtual sComputePipelineDesc GetPipelineDesc() const = 0;
	virtual bool Recompile() = 0;
};

struct sTextureDesc
{
	struct sSize
	{
		std::uint32_t X;
		std::uint32_t Y;
		std::uint32_t Z;
		sSize()
			: X(NULL)
			, Y(NULL)
			, Z(1)
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
	static ITexture2D::SharedPtr Create(const std::wstring FilePath, const std::string InName, std::uint32_t DefaultRootParameterIndex = 0, std::uint32_t GPUIndex = 0);
	static ITexture2D::UniquePtr CreateUnique(const std::wstring FilePath, const std::string InName, std::uint32_t DefaultRootParameterIndex = 0, std::uint32_t GPUIndex = 0);
	static ITexture2D::SharedPtr Create(const std::string InName, void* InBuffer, const std::size_t InSize, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex = 0, std::uint32_t GPUIndex = 0);
	static ITexture2D::UniquePtr CreateUnique(const std::string InName, void* InBuffer, const std::size_t InSize, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex = 0, std::uint32_t GPUIndex = 0);
	static ITexture2D::SharedPtr CreateEmpty(const std::string InName, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex = 0, std::uint32_t GPUIndex = 0);
	static ITexture2D::UniquePtr CreateUniqueEmpty(const std::string InName, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex = 0, std::uint32_t GPUIndex = 0);

public:
	virtual std::wstring GetPath() const = 0;
	virtual std::string GetName() const = 0;
	virtual std::uint32_t GetBindlessIndex() const = 0;

	virtual sTextureDesc GetDesc() const = 0;

	virtual void SetDefaultRootParameterIndex(std::uint32_t RootParameterIndex) = 0;
	virtual std::uint32_t GetDefaultRootParameterIndex() const = 0;

	virtual void UpdateTexture(ITexture2D* SourceTexture, std::size_t SourceArrayIndex, std::size_t ArrayIndex, const std::optional<IntVector2> Dest = std::nullopt, const std::optional<FBounds2D> TargetBounds = std::nullopt) = 0;
	virtual void UpdateTexture(const std::wstring FilePath, std::size_t ArrayIndex, const std::optional<IntVector2> Dest = std::nullopt, const std::optional<FBounds2D> TargetBounds = std::nullopt) = 0;
	virtual void UpdateTexture(const void* pSrcData, const std::size_t InSize, const FDimension2D& Dimension, std::size_t ArrayIndex, const std::optional<IntVector2> Dest = std::nullopt, const std::optional<FBounds2D> TargetBounds = std::nullopt) = 0;

	virtual void UpdateTexture(const void* pSrcData, std::size_t RowPitch, std::size_t MinX, std::size_t MinY, std::size_t MaxX, std::size_t MaxY, IGraphicsCommandContext* InCommandBuffer = nullptr) = 0;

	virtual void SaveToFile(std::wstring InPath) const = 0;
	virtual ResourceSharedHandle* GetSharedHandle() const = 0;
	virtual bool CopyFrom(ITexture2D* Texture2D) = 0;
};

typedef ITexture2D ITiledTexture;

class sMaterial;
struct GMouseInput;
struct GKeyboardChar;

class IRenderPass
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IRenderPass);
public:
	virtual void BeginPlay() = 0;
	virtual void Tick(const double DeltaTime) = 0;

	//virtual void Render(IFrameBuffer* pFB) = 0;

	virtual void SetRenderSize(std::size_t Width, std::size_t Height) = 0;
	virtual void OnInputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar) = 0;
};

class IRenderer
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IRenderer)
public:
	virtual void RegisterMaterial(sMaterial* Material) = 0;
	virtual void CompileMaterial(sMaterial* Material, bool bRecompile = false) = 0;
	virtual void CompilePipeline(IPipeline* Pipeline, bool bRecompile = false) = 0;
	virtual sIndirectLayoutBindingDesc GetRenderPassIndirectLayoutBindingDesc(ERenderPass RenderPass) const = 0;
	virtual IFrameBuffer* GetFrameBuffer(ERenderPass RenderPass) const = 0;
};

struct IBindlessSceneDescriptor {};
struct IBindlessSceneContainer
{
	virtual const IBindlessSceneDescriptor* GetDescriptor() const
	{
		return nullptr;
	}

	virtual std::uint32_t Size() const
	{
		return GetMaterialInstanceSize() + GetConstantBuffereSize() + GetUAVSize() + GetTextureSize() + GetSamplerSize() + GetVertexBuffersSize() + GetByteAddressBufferSize();
	}

	virtual std::uint32_t GetOffset() const
	{
		return 0;
	}

	virtual std::vector<std::uint32_t> GetAllBindlessIndices() const
	{
		return std::vector<std::uint32_t>();
	}

	virtual std::vector<std::uint32_t> GetAllVertexBufferBindlessIndices() const
	{
		return std::vector<std::uint32_t>();
	}

	virtual std::uint32_t GetVertexBuffersSize() const
	{
		return 0;
	}

	virtual std::vector<std::uint32_t> GetAllMaterialInstanceBindlessIndices() const
	{
		return std::vector<std::uint32_t>();
	}

	virtual std::uint32_t GetMaterialInstanceSize() const
	{
		return 0;
	}

	virtual std::vector<std::uint32_t> GetAllConstantBufferBindlessIndices() const
	{
		return std::vector<std::uint32_t>();
	}

	virtual std::uint32_t GetConstantBuffereSize() const
	{
		return 0;
	}

	virtual std::vector<std::uint32_t> GetAllTextureBindlessIndices() const
	{
		return std::vector<std::uint32_t>();
	}

	virtual std::uint32_t GetTextureSize() const
	{
		return 0;
	}

	virtual std::vector<std::uint32_t> GetAllSamplerBindlessIndices() const
	{
		return std::vector<std::uint32_t>();
	}

	virtual std::uint32_t GetSamplerSize() const
	{
		return 0;
	}

	virtual std::vector<std::uint32_t> GetAllUAVBindlessIndices() const
	{
		return std::vector<std::uint32_t>();
	}

	virtual std::uint32_t GetUAVSize() const
	{
		return 0;
	}

	virtual std::vector<std::uint32_t> GetAllByteAddressBufferBindlessIndices() const
	{
		return std::vector<std::uint32_t>();
	}

	virtual std::uint32_t GetByteAddressBufferSize() const
	{
		return 0;
	}
};

enum class ECommandContextType
{
	Graphics,
	Compute,
	Copy,
};

enum class ECommandContextExecuteType
{
	Immediate,
	Deferred,
};

enum class ECommandContextState
{
	Waiting,
	Begin,
	End,
	WaitingForExecute,
};

enum class ECommandContextBeginResult
{
	Started,
	AlreadyStarted,
	Failed_WaitingForExecute,
	Restarted,
};

enum class ECommandContextBeginState
{
	Default,
	NonRender = Default,
	Render,
	ApiRender,
};

class ICommandContext
{
	sBaseClassBody(sClassDefaultProtectedConstructor, ICommandContext)
public:
	virtual void* GetInternalCommandContext() = 0;
	virtual ECommandContextType GetCommandContextType() const = 0;
	virtual ECommandContextState GetState() const = 0;
};

class IGraphicsCommandContext : public ICommandContext
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IGraphicsCommandContext)
public:
	static IGraphicsCommandContext::SharedPtr Create(std::uint32_t GPUIndex = 0);
	static IGraphicsCommandContext::UniquePtr CreateUnique(std::uint32_t GPUIndex = 0);

public:
	virtual ECommandContextType GetCommandContextType() const override final
	{
		return ECommandContextType::Graphics;
	}

	virtual ECommandContextBeginResult BeginRecordCommandList(const ECommandContextBeginState Begin = ECommandContextBeginState::Default) = 0;
	virtual bool FinishRecordCommandList() = 0;
	virtual bool ExecuteCommandList(ECommandContextExecuteType ExecuteType = ECommandContextExecuteType::Immediate, std::uint32_t Order = std::uint32_t(-1)) = 0;
	virtual void ClearState() = 0;

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
	virtual void SetUnorderedAccessBufferAsResource(IStructuredBuffer* pUAV, std::optional<std::uint32_t> RootParameterIndex = std::nullopt) = 0;
	virtual void SetUnorderedAccessBuffersAsResource(std::vector<IStructuredBuffer*> UAVs, std::optional<std::uint32_t> RootParameterIndex = std::nullopt) = 0;
	virtual void CopyFrameBuffer(IFrameBuffer* Dest, std::size_t DestFBOIndex, IFrameBuffer* Source, std::uint32_t SourceFBOIndex) = 0;
	virtual void CopyFrameBufferDepth(IFrameBuffer* Dest, IFrameBuffer* Source) = 0;
	virtual void CopyRenderTarget(IRenderTarget* Dest, IRenderTarget* Source) = 0;
	virtual void CopyDepthBuffer(IDepthTarget* Dest, IDepthTarget* Source) = 0;

	virtual void SetPipeline(IPipeline* Pipeline) = 0;

	virtual void SetVertexBuffer(IVertexBuffer* VB, std::uint32_t Slot = 0) = 0;
	virtual void SetIndexBuffer(IIndexBuffer* IB) = 0;
	virtual void SetConstantBuffer(IConstantBuffer* CB, std::optional<std::uint32_t> RootParameterIndex = std::nullopt) = 0;

	virtual void SetTexture2D(ITexture2D* Texture2D, std::optional<std::uint32_t> RootParameterIndex = std::nullopt) = 0;

	virtual void UpdateBufferSubresource(IVertexBuffer* Buffer, BufferSubresource* Subresource) = 0;
	virtual void UpdateBufferSubresource(IVertexBuffer* Buffer, std::size_t Location, std::size_t Size, const void* pSrcData) = 0;
	virtual void UpdateBufferSubresource(IIndexBuffer* Buffer, BufferSubresource* Subresource) = 0;
	virtual void UpdateBufferSubresource(IIndexBuffer* Buffer, std::size_t Location, std::size_t Size, const void* pSrcData) = 0;

	virtual void Set32BitConstant(std::uint32_t RootParameterIndex, std::uint32_t SrcData, std::uint32_t DestOffsetIn32BitValues = 0) = 0;
	virtual void Set32BitConstants(std::uint32_t RootParameterIndex, const void* pSrcData, std::uint32_t Num32BitValuesToSet = 1, std::uint32_t DestOffsetIn32BitValues = 0) = 0;

	virtual void SetBindlessDescriptor(std::uint32_t RootParameterIndex, IBindlessSceneContainer* Container) = 0;

	virtual void Draw(std::uint32_t VertexCount, std::uint32_t VertexStartOffset = 0) = 0;
	virtual void DrawInstanced(std::uint32_t VertexCountPerInstance, std::uint32_t InstanceCount, std::uint32_t StartVertexLocation, std::uint32_t StartInstanceLocation) = 0;
	virtual void DrawIndexedInstanced(std::uint32_t IndexCountPerInstance, std::uint32_t InstanceCount, std::uint32_t StartIndexLocation, std::int32_t BaseVertexLocation, std::uint32_t StartInstanceLocation) = 0;
	virtual void DrawIndexedInstanced(const sObjectDrawParameters& Params) = 0;

	virtual void ExecuteIndirect(IIndirectBuffer* IndirectBuffer) = 0;
};

class IComputeCommandContext : public ICommandContext
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IComputeCommandContext)
public:
	static IComputeCommandContext::SharedPtr Create(std::uint32_t GPUIndex = 0);
	static IComputeCommandContext::UniquePtr CreateUnique(std::uint32_t GPUIndex = 0);

public:
	virtual ECommandContextType GetCommandContextType() const override final
	{
		return ECommandContextType::Compute;
	}

	virtual void BeginRecordCommandList() = 0;
	virtual void FinishRecordCommandList() = 0;
	virtual void ExecuteCommandList() = 0;
	virtual void ClearState() = 0;

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
	virtual void SetUnorderedAccessBuffer(IStructuredBuffer* pUAV, std::uint32_t RootParameterIndex) = 0;
	virtual void SetUnorderedAccessBuffers(std::vector<IStructuredBuffer*> UAVs, std::uint32_t RootParameterIndex) = 0;
	virtual void SetUnorderedAccessBufferAsResource(IStructuredBuffer* pUAV, std::uint32_t RootParameterIndex) = 0;
	virtual void SetUnorderedAccessBuffersAsResource(std::vector<IStructuredBuffer*> UAVs, std::uint32_t RootParameterIndex) = 0;

	virtual void SetPipeline(IComputePipeline* Pipeline) = 0;
	virtual void SetConstantBuffer(IConstantBuffer* CB, std::optional<std::uint32_t> RootParameterIndex = std::nullopt) = 0;

	virtual void Dispatch(std::uint32_t ThreadGroupCountX, std::uint32_t ThreadGroupCountY, std::uint32_t ThreadGroupCountZ) = 0;
	virtual void ExecuteIndirect(IIndirectBuffer* IndirectBuffer) = 0;
};

class ICopyCommandContext : public ICommandContext
{
	sBaseClassBody(sClassDefaultProtectedConstructor, ICopyCommandContext)
public:
	static ICopyCommandContext::SharedPtr Create(std::uint32_t GPUIndex = 0);
	static ICopyCommandContext::UniquePtr CreateUnique(std::uint32_t GPUIndex = 0);

public:
	virtual ECommandContextType GetCommandContextType() const override final
	{
		return ECommandContextType::Copy;
	}

	virtual void BeginRecordCommandList() = 0;
	virtual void FinishRecordCommandList() = 0;
	virtual void ExecuteCommandList() = 0;
	virtual void ClearState() = 0;

	virtual void CopyFrameBuffer(IFrameBuffer* Dest, std::size_t DestFBOIndex, IFrameBuffer* Source, std::uint32_t SourceFBOIndex) = 0;
	virtual void CopyFrameBufferDepth(IFrameBuffer* Dest, IFrameBuffer* Source) = 0;

	virtual void UpdateBufferSubresource(IVertexBuffer* Buffer, BufferSubresource* Subresource) = 0;
	virtual void UpdateBufferSubresource(IVertexBuffer* Buffer, std::size_t Location, std::size_t Size, const void* pSrcData) = 0;
	virtual void UpdateBufferSubresource(IIndexBuffer* Buffer, BufferSubresource* Subresource) = 0;
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

struct Version
{
	uint32_t major;
	uint32_t minor;
	uint32_t patch;

	operator std::string() const
	{
		return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
	}

	Version& operator=(const std::string& str)
	{
		return from_string(str.c_str());
	}

	Version& operator=(const char* str)
	{
		return from_string(str);
	}

private:
	Version& from_string(const char* str)
	{
		sscanf_s(str, "%u.%u.%u", &major, &minor, &patch);
		return *this;
	}
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
		, SupportedAPI(EGITypes::D3D11)
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
		return SupportedAPI == EGITypes::D3D11 ? "D3D11" : SupportedAPI == EGITypes::D3D12 ? "D3D12" : SupportedAPI == EGITypes::Vulkan ? "Vulkan" : "Unknown";
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

enum class EParticleType
{
	CPU,
	GPU,
};

enum class EPhysicsEngine
{
	None,
	eBulletPhysics,
	Box2D,
	PhysX,
};

enum class EPostProcessRenderOrder
{
	BeforeTonemap,
	AfterTonemap,
	BeforeUI,
	AfterUI,
};

enum class ERendererClear
{
	Disabled,
	Driver,
	Sky,
};

enum class ERendererUpscaleMode : std::uint32_t
{
	NativeAA = 0,           // 1.0f
	Quality = 1,            // 1.5f
	Balanced = 2,           // 1.7f
	Performance = 3,        // 2.f
	UltraPerformance = 4,   // 3.f
	Custom = 5,             // 1.f - 3.f range
	DynamicResolution = 6,
};

enum class ERendererUpscalerType : std::uint32_t
{
	None,
	//TAA,
	FSR,
	//DLSS,
	//XESS,
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
class IAbstractGIDevice;

namespace GPU
{
	EGPUDeviceType GetDeviceType();
	IAbstractGIDevice* GetDevice();
	void* GetInternalDevice();
	void* GetInternalSwapChain();

	void WaitForGPU();
	void WaitForCPU();
	void WaitForCPUFence(std::uint64_t Value);
	std::uint64_t GPUFenceSignal();

	sGPUInfo GetGPUInfo();
	EGITypes GetGIType();
	sViewport GetViewport();
	sScreenDimension GetBackBufferDimension();
	EFormat GetBackBufferFormat();
	EFormat GetDefaultDepthFormat();

	void ResizeWindow(std::size_t Width, std::size_t Height);
	void FullScreen(const bool value);
	bool IsFullScreen();
	void Vsync(const bool value);
	bool IsVsyncEnabled();
	void VsyncInterval(const std::uint32_t value);
	std::uint32_t GetVsyncInterval();
	std::vector<sDisplayDesc> GetAllSupportedResolutions();
	DisplayMode GetDisplayMode();

	void RecreateSwapChain();
	std::uint32_t GetBackBufferSize();
	std::uint32_t GetCurrentBackBufferIndex();
	std::uint64_t GetFrameIndex();

	bool IsBindlessRendererSupported();
	bool IsBindlessRendererEnabled();

	IRenderer* GetRenderer();
	void RegisterMaterial(sMaterial* Material);
	void CompileMaterial(sMaterial* Material, bool bRecompile = false);
	void CompilePipeline(IPipeline* Pipeline, bool bRecompile = false);
	sIndirectLayoutBindingDesc GetRenderPassIndirectLayoutBindingDesc(ERenderPass RenderPass);
	IFrameBuffer* GetFrameBuffer(ERenderPass RenderPass);
	void SetRendererClearMode(ERendererClear Mode);
	ERendererClear GetRendererClearMode();
	void SetUpscalerType(ERendererUpscalerType UpscalerType);
	void SetUpscaleMode(ERendererUpscaleMode UpscaleMode);
	void SetFSRSharpness(float Sharpness);
	void SetEnableFrameGen(bool bEnable, std::uint32_t Multiplier = 2);

	void SetTonemapper(const int Val);
	int GetTonemapperIndex();

	void AddPostProcess(const EPostProcessRenderOrder Order, const std::shared_ptr<sPostProcess>& PostProcess);
	void RemovePostProcess(const EPostProcessRenderOrder Order, const int Val);

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

enum class AssertLevel
{
	ASSERT_WARNING = 0,
	ASSERT_ERROR,
	ASSERT_CRITICAL,
};

class sInputController;
namespace Engine
{
	void WriteToConsole(const std::string& STR);
	void WriteToConsole(const std::wstring& STR);
	void WriteToConsole(const wchar_t* STR);
	void WriteToConsole(const char*STR);
	void Print(const char* message, ...);
	void Print(const wchar_t* message, ...);
	void ShowDialogWindow(AssertLevel Level, const char* message, ...);
	void ShowDialogWindow(AssertLevel Level, const wchar_t* message, ...);

	void Assert(AssertLevel severity, bool condition, const char* message, ...);
	void Assert(AssertLevel severity, bool condition, const wchar_t* message, ...);

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

	template<typename T>
	inline T RandomValueInRange(T Min, T Max)
	{
		//return (double)rand() / RAND_MAX;

		std::random_device rd; // obtain a random number from hardware
		std::mt19937 gen(rd()); // seed the generator
		std::uniform_real_distribution<> dist(Min, Max); // define the range

		return static_cast<T>(dist(gen));
	}
}

#define ASSERT(condition)  Engine::Assert(AssertLevel::ASSERT_CRITICAL, condition, L"Assertion Failed %ls - line %d", __FILE__, __LINE__)

template <typename T>
inline constexpr bool IsComInterfaceV = std::is_base_of_v<IUnknown, T>;

// to do : Use Fence
template<typename T, bool bUseFence = false>
struct TDoubleBuffer
{
	TDoubleBuffer() = default;
	~TDoubleBuffer()
	{
		Release();
	}

	void Release()
	{
		if constexpr (IsComInterfaceV<T>)
		{
			First.Reset();
			Second.Reset();
		}
		else
		{
			First = nullptr;
			Second = nullptr;
		}
	}

	using Type = std::conditional_t<IsComInterfaceV<T>, Microsoft::WRL::ComPtr<T>, std::shared_ptr<T>>;

	Type First = nullptr;
	Type Second = nullptr;
	std::uint64_t FenceValue = 0;

	T* Get() const
	{
		/*if (bUseFence)
		{

		}
		else*/
		{
			//std::uint32_t FrameIndex = GPU::GetFrameIndex();
			//std::uint32_t BackBufferSize = GPU::GetBackBufferSize();
			//std::uint32_t Index = FrameIndex % BackBufferSize;

			std::uint32_t Index = GPU::GetCurrentBackBufferIndex();

			if constexpr (IsComInterfaceV<T>)
			{
				if (Index > 0)
					return First.Get();
				return Second.Get();
			}
			else
			{
				if (Index > 0)
					return First.get();
				return Second.get();
			}
		}
	}
};
