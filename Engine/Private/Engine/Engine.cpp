
#include "pch.h"
#include "Engine/Engine.h"
#include "GI/D3D11/D3D11Device.h"
#include "GI/D3D12/D3D12Device.h"
#include "GI/Vulkan/VulkanDevice.h"
#include "GI/Renderer/Renderer.h"
#include "GI/AbstractGI/AbstractGIDevice.h"
#include "Engine/AbstractEngine.h"
#include "AbstractGI/PostProcess.h"
#include "Engine/IPhysicalWorld.h"
#include "Engine/World2D.h"
#include "AbstractGI/MaterialManager.h"
#include "AbstractGI/TextureManager.h"
#include "AbstractGI/ShaderManager.h"
#include "Engine/InputController.h"
#include "Engine/Audio.h"
#include "Core/ThreadPool.h"

namespace
{
	static std::unique_ptr<IAbstractGIDevice> Device = nullptr;
	static std::unique_ptr<sRenderer> Renderer = nullptr;
	static std::shared_ptr<IPhysicalWorld> PhysicalWorld = nullptr;
	static std::unique_ptr<sInputController> InputController = nullptr;
	static std::unique_ptr<XAudio> pAudio = nullptr;
	static ThreadPool mThreadPool;

	static bool bPauseInput = false;
	static bool bPausePhysics = false;
	static bool bPauseTick = false;
}

namespace GPU
{
	sGPUInfo GetGPUInfo()
	{
		return Device->GetGPUInfo();
	}

	EGITypes GetGIType()
	{
		return Device->GetGIType();
	}

	sViewport GetViewport()
	{
		return Device->GetViewport();
	}

	sScreenDimension GetBackBufferDimension()
	{
		return Device->GetBackBufferDimension();
	}

	EFormat GetBackBufferFormat()
	{
		return Device->GetBackBufferFormat();
	}

	EFormat GetDefaultDepthFormat()
	{
		return EFormat::R32G8X24_Typeless;
	}

	void SetGBufferClearMode(EGBufferClear Mode)
	{
		Renderer->SetGBufferClearMode(Mode);
	}

	EGBufferClear GetGBufferClearMode()
	{
		return Renderer->GetGBufferClearMode();
	}

	void* GetInternalDevice()
	{
		return Device->GetInternalDevice();
	}

	void SetTonemapper(const int Val)
	{
		Renderer->SetTonemapper(Val);
	}

	int GetTonemapperIndex()
	{
		return Renderer->GetTonemapperIndex();
	}

	void AddPostProcess(const EPostProcessRenderOrder Order, const std::shared_ptr<sPostProcess>& PostProcess)
	{
		Renderer->AddPostProcess(Order, PostProcess);
	}

	void RemovePostProcess(const EPostProcessRenderOrder Order, const int Val)
	{
		Renderer->RemovePostProcess(Order, Val);
	}

	void DrawLine(const FVector& Start, const FVector& End, std::optional<float> Time)
	{
		Renderer->DrawLine(Start, End, FColor::White(), Time);
	}

	void DrawBound(const FBoundingBox& Box, std::optional<float> Time)
	{
		Renderer->DrawBound(Box, FColor::White(), Time);
	}

	void DrawLine(const FVector& Start, const FVector& End, const FColor& Color, std::optional<float> Time)
	{
		Renderer->DrawLine(Start, End, Color, Time);
	}

	void DrawBound(const FBoundingBox& Box, const FColor& Color, std::optional<float> Time)
	{
		Renderer->DrawBound(Box, Color, Time);
	}

	sScreenDimension GetInternalBaseRenderResolution()
	{
		return Renderer->GetInternalBaseRenderResolution();
	}

	void SetInternalBaseRenderResolution(std::size_t Width, std::size_t Height)
	{
		Renderer->SetInternalBaseRenderResolution(Width, Height);
	}

	void AddViewportInstance(sViewportInstance* ViewportInstance, std::optional<std::size_t> Priority)
	{
		Renderer->AddViewportInstance(ViewportInstance, Priority);
	}

	void RemoveViewportInstance(sViewportInstance* ViewportInstance)
	{
		Renderer->RemoveViewportInstance(ViewportInstance);
	}

	void RemoveViewportInstance(std::size_t Index)
	{
		Renderer->RemoveViewportInstance(Index);
	}

	void GPU::SetViewportInstancePriority(sViewportInstance* ViewportInstance, std::size_t Priority)
	{
		Renderer->SetViewportInstancePriority(ViewportInstance, Priority);
	}
}

namespace Audio
{
	void AddToPlayList(std::string Name, std::string path, bool loop, bool RunOnce)
	{
		pAudio->AddToPlayList(Name, path, loop, RunOnce);
	}

	void Play(std::string Name, std::string path, bool loop, bool PlayAsOverlap)
	{
		pAudio->Play(Name, path, loop, PlayAsOverlap);
	}

	void Stop(bool immediate)
	{
		pAudio->Stop(immediate);
	}

	void Next()
	{
		pAudio->Next();
	}

	void Resume()
	{
		pAudio->Resume();
	}

	void Pause()
	{
		pAudio->Pause();
	}

	void Remove(std::size_t index)
	{
		pAudio->Remove(index);
	}

	void Remove(std::string Name)
	{
		pAudio->Remove(Name);
	}

	void DestroyAllVoice(bool IsOverlapSoundOnly)
	{
		pAudio->DestroyAllVoice(IsOverlapSoundOnly);
	}

	bool IsLooped()
	{
		return pAudio->IsLooped();
	}

	float GetVolume()
	{
		return pAudio->GetVolume();
	}

	void SetVolume(float volume)
	{
		pAudio->SetVolume(volume);
	}

	std::size_t GetPlayListCount(bool IsOverlapSoundOnly)
	{
		return pAudio->GetPlayListCount(IsOverlapSoundOnly);
	}

	std::size_t GetCurrentAudioIndex()
	{
		return pAudio->GetCurrentAudioIndex();
	}

	std::size_t GetNextAudioIndex()
	{
		return pAudio->GetCurrentAudioIndex();
	}

	void SetPlayListState(bool State)
	{
		return pAudio->SetPlayListState(State);
	}

	void BindFunctionOnVoiceStart(std::function<void(std::string)> fOnVoiceStart)
	{
		return pAudio->BindFunctionOnVoiceStart(fOnVoiceStart);
	}

	void BindFunctionOnVoiceStop(std::function<void(std::string)> fOnVoiceStop)
	{
		return pAudio->BindFunctionOnVoiceStop(fOnVoiceStop);
	}
}

namespace Engine
{
	void SystemTime(std::int32_t& Year, int32_t& Month, int32_t& DayOfWeek, int32_t& Day, int32_t& Hour, int32_t& Min, int32_t& Sec, int32_t& MSec)
	{
		SYSTEMTIME st;
		GetLocalTime(&st);

		Year = st.wYear;
		Month = st.wMonth;
		DayOfWeek = st.wDayOfWeek;
		Day = st.wDay;
		Hour = st.wHour;
		Min = st.wMinute;
		Sec = st.wSecond;
		MSec = st.wMilliseconds;
	}

	void UtcTime(std::int32_t& Year, std::int32_t& Month, std::int32_t& DayOfWeek, std::int32_t& Day, std::int32_t& Hour, std::int32_t& Min, std::int32_t& Sec, std::int32_t& MSec)
	{
		SYSTEMTIME st;
		GetSystemTime(&st);

		Year = st.wYear;
		Month = st.wMonth;
		DayOfWeek = st.wDayOfWeek;
		Day = st.wDay;
		Hour = st.wHour;
		Min = st.wMinute;
		Sec = st.wSecond;
		MSec = st.wMilliseconds;
	}

	sDateTime GetSystemTime()
	{
		sDateTime Time;
		SystemTime(Time.Year, Time.Month, Time.DayOfWeek, Time.Day, Time.Hour, Time.Minute, Time.Second, Time.Millisecond);
		return Time;
	}

	sDateTime UtcTime()
	{
		sDateTime Time;
		UtcTime(Time.Year, Time.Month, Time.DayOfWeek, Time.Day, Time.Hour, Time.Minute, Time.Second, Time.Millisecond);
		return Time;
	}

	void QueueJob(const std::function<void()>& job)
	{
		mThreadPool.QueueJob(job);
	}

	std::size_t AvailableThreadCount()
	{
		return mThreadPool.AvailableThreadCount();
	}

	sInputController* GetInputController()
	{
		return InputController.get();
	}

	bool IsInputPaused()
	{
		return bPauseInput;
	}
	void PauseInput(bool value)
	{
		bPauseInput = value;
	}
	bool IsTickPaused()
	{
		return bPauseTick;
	}
	void PauseTick(bool value)
	{
		bPauseTick = value;
	}

	sScreenDimension GetScreenDimension()
	{
		return Device->GetBackBufferDimension();
	}
}

namespace Physics
{
	EPhysicsEngine GetActivePhysicsEngineType()
	{
		return PhysicalWorld->GetPhysicsEngineType();
	}

	bool IsPhysicsPaused()
	{
		return bPausePhysics || bPauseTick;
	}
	void PausePhysics(bool value)
	{
		bPausePhysics = value;
	}

	void SetPhysicsInternalTick(std::optional<double> Tick)
	{
		return PhysicalWorld->SetPhysicsInternalTick(Tick);
	}

	void SetGravity(const FVector& Gravity)
	{
		return PhysicalWorld->SetGravity(Gravity);
	}

	FVector GetGravity()
	{
		return PhysicalWorld->GetGravity();
	}

	void SetWorldOrigin(const FVector& newOrigin)
	{
		return PhysicalWorld->SetWorldOrigin(newOrigin);
	}

	sPhysicalComponent* LineTraceToViewPort(const FVector& InOrigin, const FVector& InDirection)
	{
		return PhysicalWorld->LineTraceToViewPort(InOrigin, InDirection);
	}
	std::vector<sPhysicalComponent*> QueryAABB(const FBoundingBox& Bounds)
	{
		return PhysicalWorld->QueryAABB(Bounds);
	}

	float Physics::GetPhysicalWorldScale()
	{
		return PhysicalWorld->GetPhysicalWorldScale();
	}
}

IGraphicsCommandContext::SharedPtr IGraphicsCommandContext::Create()
{
	return Device->CreateGraphicsCommandContext();
}

IGraphicsCommandContext::UniquePtr IGraphicsCommandContext::CreateUnique()
{
	return Device->CreateUniqueGraphicsCommandContext();
}

IComputeCommandContext::SharedPtr IComputeCommandContext::Create()
{
	return Device->CreateComputeCommandContext();
}

IComputeCommandContext::UniquePtr IComputeCommandContext::CreateUnique()
{
	return Device->CreateUniqueComputeCommandContext();
}

ICopyCommandContext::SharedPtr ICopyCommandContext::Create()
{
	return Device->CreateCopyCommandContext();
}

ICopyCommandContext::UniquePtr ICopyCommandContext::CreateUnique()
{
	return Device->CreateUniqueCopyCommandContext();
}

IConstantBuffer::SharedPtr IConstantBuffer::Create(std::string InName, const sBufferDesc& InDesc, std::uint32_t InRootParameterIndex)
{
	return Device->CreateConstantBuffer(InName, InDesc, InRootParameterIndex);
}

IConstantBuffer::UniquePtr IConstantBuffer::CreateUnique(std::string InName, const sBufferDesc& InDesc, std::uint32_t InRootParameterIndex)
{
	return Device->CreateUniqueConstantBuffer(InName, InDesc, InRootParameterIndex);
}

IVertexBuffer::SharedPtr IVertexBuffer::Create(std::string InName, const sBufferDesc& InDesc, sBufferSubresource* InSubresource)
{
	return Device->CreateVertexBuffer(InName, InDesc, InSubresource);
}

IVertexBuffer::UniquePtr IVertexBuffer::CreateUnique(std::string InName, const sBufferDesc& InDesc, sBufferSubresource* InSubresource)
{
	return Device->CreateUniqueVertexBuffer(InName, InDesc, InSubresource);
}

IIndexBuffer::SharedPtr IIndexBuffer::Create(std::string InName, const sBufferDesc& InDesc, sBufferSubresource* InSubresource)
{
	return Device->CreateIndexBuffer(InName, InDesc, InSubresource);
}

IIndexBuffer::UniquePtr IIndexBuffer::CreateUnique(std::string InName, const sBufferDesc& InDesc, sBufferSubresource* InSubresource)
{
	return Device->CreateUniqueIndexBuffer(InName, InDesc, InSubresource);
}

IRenderTarget::SharedPtr IRenderTarget::Create(const std::string InName, const EFormat Format, const sFBODesc& Desc)
{
	return Device->CreateRenderTarget(InName, Format, Desc);
}

IRenderTarget::UniquePtr IRenderTarget::CreateUnique(const std::string InName, const EFormat Format, const sFBODesc& Desc)
{
	return Device->CreateUniqueRenderTarget(InName, Format, Desc);
}

IDepthTarget::SharedPtr IDepthTarget::Create(const std::string InName, const EFormat Format, const sFBODesc& Desc)
{
	return Device->CreateDepthTarget(InName, Format, Desc);
}

IDepthTarget::UniquePtr IDepthTarget::CreateUnique(const std::string InName, const EFormat Format, const sFBODesc& Desc)
{
	return Device->CreateUniqueDepthTarget(InName, Format, Desc);
}

IUnorderedAccessTarget::SharedPtr IUnorderedAccessTarget::Create(const std::string InName, const EFormat Format, const sFBODesc& Desc, bool InEnableSRV)
{
	return Device->CreateUnorderedAccessTarget(InName, Format, Desc, InEnableSRV);
}

IUnorderedAccessTarget::UniquePtr IUnorderedAccessTarget::CreateUnique(const std::string InName, const EFormat Format, const sFBODesc& Desc, bool InEnableSRV)
{
	return Device->CreateUniqueUnorderedAccessTarget(InName, Format, Desc, InEnableSRV);
}

IUnorderedAccessTarget::SharedPtr IUnorderedAccessTarget::CreateFromRenderTarget(IRenderTarget* RenderTarget, bool InEnableSRV)
{
	return nullptr; //Device->CreateUniqueUnorderedAccessTarget(RenderTarget, InEnableSRV);
}

IUnorderedAccessTarget::UniquePtr IUnorderedAccessTarget::CreateUniqueFromRenderTarget(IRenderTarget* RenderTarget, bool InEnableSRV)
{
	return nullptr; //Device->CreateUniqueUnorderedAccessTarget(RenderTarget, InEnableSRV);
}

IFrameBuffer::SharedPtr IFrameBuffer::Create(const std::string InName, const sFrameBufferAttachmentInfo& InAttachments)
{
	return Device->CreateFrameBuffer(InName, InAttachments);
}

IFrameBuffer::UniquePtr IFrameBuffer::CreateUnique(const std::string InName, const sFrameBufferAttachmentInfo& InAttachments)
{
	return Device->CreateUniqueFrameBuffer(InName, InAttachments);
}

IPipeline::SharedPtr IPipeline::Create(const std::string& InName, const sPipelineDesc& InDesc)
{
	return Device->CreatePipeline(InName, InDesc);
}

IPipeline::UniquePtr IPipeline::CreateUnique(const std::string& InName, const sPipelineDesc& InDesc)
{
	return Device->CreateUniquePipeline(InName, InDesc);
}

IShader::SharedPtr IShader::Create(const sShaderAttachment& Attachment)
{
	return nullptr;
}

IShader::SharedPtr IShader::Create(std::wstring InSrcFile, std::string InFunctionName, eShaderType InProfile, std::vector<sShaderDefines> InDefines)
{
	return nullptr;
}

IShader::SharedPtr IShader::Create(const void* InCode, std::size_t Size, std::string InFunctionName, eShaderType InProfile, std::vector<sShaderDefines> InDefines)
{
	return nullptr;
}

IComputePipeline::SharedPtr IComputePipeline::Create(const std::string& InName, const sComputePipelineDesc& InDesc)
{
	return Device->CreateComputePipeline(InName, InDesc);
}

IComputePipeline::UniquePtr IComputePipeline::CreateUnique(const std::string& InName, const sComputePipelineDesc& InDesc)
{
	return Device->CreateUniqueComputePipeline(InName, InDesc);
}

ITexture2D::SharedPtr ITexture2D::Create(const std::wstring FilePath, const std::string InName, std::uint32_t DefaultRootParameterIndex)
{
	if (!sTextureManager::Get().IsTextureExist(FilePath, InName))
	{
		auto Texture = Device->CreateTexture2D(FilePath, InName, DefaultRootParameterIndex);
		sTextureManager::Get().StoreTexture(Texture);
		return Texture;
	}
	return sTextureManager::Get().GetTextureAsShared(FilePath, InName);
}

ITexture2D::UniquePtr ITexture2D::CreateUnique(const std::wstring FilePath, const std::string InName, std::uint32_t DefaultRootParameterIndex)
{
	return Device->CreateUniqueTexture2D(FilePath, InName, DefaultRootParameterIndex);
}

ITexture2D::SharedPtr ITexture2D::Create(const std::string InName, void* InBuffer, const std::size_t InSize, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
{
	return Device->CreateTexture2D(InName, InBuffer, InSize, InDesc, DefaultRootParameterIndex);
}

ITexture2D::UniquePtr ITexture2D::CreateUnique(const std::string InName, void* InBuffer, const std::size_t InSize, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
{
	return Device->CreateUniqueTexture2D(InName, InBuffer, InSize, InDesc, DefaultRootParameterIndex);
}

ITexture2D::SharedPtr ITexture2D::CreateEmpty(const std::string InName, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
{
	return Device->CreateEmptyTexture2D(InName, InDesc, DefaultRootParameterIndex);
}

ITexture2D::UniquePtr ITexture2D::CreateUniqueEmpty(const std::string InName, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
{
	return Device->CreateUniqueEmptyTexture2D(InName, InDesc, DefaultRootParameterIndex);
}

//ITiledTexture::SharedPtr ITiledTexture::Create(const std::string InName, const std::uint32_t InTileX, const std::uint32_t InTileY, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
//{
//	return Device->CreateTiledTexture(InName, InTileX, InTileY, InDesc, DefaultRootParameterIndex);
//}
//
//ITiledTexture::UniquePtr ITiledTexture::CreateUnique(const std::string InName, const std::uint32_t InTileX, const std::uint32_t InTileY, const sTextureDesc& InDesc, std::uint32_t DefaultRootParameterIndex)
//{
//	return Device->CreateUniqueTiledTexture(InName, InTileX, InTileY, InDesc, DefaultRootParameterIndex);
//}

IRigidBody::SharedPtr IRigidBody::Create2DBoxBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FBounds2D& Bounds)
{
	return PhysicalWorld->Create2DBoxBody(Owner, Desc, Bounds);
}
IRigidBody::SharedPtr IRigidBody::Create2DPolygonBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, const std::array<FVector2, 8>& points)
{
	return PhysicalWorld->Create2DPolygonBody(Owner, Desc, Origin, points);
}
IRigidBody::SharedPtr IRigidBody::Create2DCircleBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, float InRadius)
{
	return PhysicalWorld->Create2DCircleBody(Owner, Desc, Origin, InRadius);
}
IRigidBody::SharedPtr IRigidBody::Create2DEdgeBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, const std::array<FVector2, 4>& points, bool OneSided)
{
	return PhysicalWorld->Create2DEdgeBody(Owner, Desc, Origin, points, OneSided);
}
IRigidBody::SharedPtr IRigidBody::Create2DChainBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, const std::vector<FVector2>& vertices)
{
	return PhysicalWorld->Create2DChainBody(Owner, Desc, Origin, vertices);
}
IRigidBody::SharedPtr IRigidBody::Create2DChainBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, const std::vector<FVector2>& vertices, const FVector2& prevVertex, const FVector2& nextVertex)
{
	return PhysicalWorld->Create2DChainBody(Owner, Desc, Origin, vertices, prevVertex, nextVertex);
}

IRigidBody::SharedPtr IRigidBody::CreateBoxBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, FVector InHalf)
{
	return PhysicalWorld->CreateBoxBody(Owner, Desc, Origin, InHalf);
}
IRigidBody::SharedPtr IRigidBody::CreateSphereBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, float InRadius)
{
	return PhysicalWorld->CreateSphereBody(Owner, Desc, Origin, InRadius);
}
IRigidBody::SharedPtr IRigidBody::CreateCapsuleBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, float InRadius, float InHeight)
{
	return PhysicalWorld->CreateCapsuleBody(Owner, Desc, Origin, InRadius, InHeight);
}
IRigidBody::SharedPtr IRigidBody::CreateCylinderBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, float InRadius, float InHeight)
{
	return PhysicalWorld->CreateCylinderBody(Owner, Desc, Origin, InRadius, InHeight);
}
IRigidBody::SharedPtr IRigidBody::CreateConeBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, float InRadius, float InHeight)
{
	return PhysicalWorld->CreateConeBody(Owner, Desc, Origin, InRadius, InHeight);
}

IRigidBody::SharedPtr IRigidBody::CreateMultiBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, FVector InInertia, float InMass)
{
	return PhysicalWorld->CreateMultiBody(Owner, Desc, Origin, InInertia, InMass);
}
IRigidBody::SharedPtr IRigidBody::CreateConvexHullBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, const float* points, int numPoints, int stride)
{
	return PhysicalWorld->CreateConvexHullBody(Owner, Desc, Origin, points, numPoints, stride);
}
IRigidBody::SharedPtr IRigidBody::CreateTriangleMesh(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, const FVector* points, int numPoints, const std::uint32_t* indices, int numIndices)
{
	return PhysicalWorld->CreateTriangleMesh(Owner, Desc, Origin, points, numPoints, indices, numIndices);
}

sEngine::sEngine(const EGITypes GIType, const IPhysicalWorld::SharedPtr& InPhysicalWorld, std::optional<short> GPUIndex)
	: MetaWorld(nullptr)
	, ScreenDimension(sScreenDimension())
{
	mThreadPool.Start();
	pAudio = std::make_unique<XAudio>();

	FixedStepTimer.SetFixedTimeStep(true);
	FixedStepTimer.SetTargetElapsedSeconds(1.0 / 60.0);

	switch (GIType)
	{
	case EGITypes::eD3D11:
		Device = D3D11Device::CreateUnique(GPUIndex);
		break;
	case EGITypes::eD3D12:
		Device = D3D12Device::CreateUnique(GPUIndex);
		break;
	case EGITypes::eVulkan:
		Device = VulkanDevice::CreateUnique(GPUIndex);
		break;
		//case EGITypes::eOpenGL46:
		//	break;
	default:
		break;
	}

	PhysicalWorld = InPhysicalWorld;
}

sEngine::~sEngine()
{
	InputController = nullptr;
	sMaterialManager::Get().Destroy();
	sTextureManager::Get().Destroy();
	sShaderManager::Get().Destroy();
	DestroyWorld();
	PhysicalWorld = nullptr;
	Renderer = nullptr;
	pAudio = nullptr;
	mThreadPool.Stop();
	Device = nullptr;
}

void sEngine::InitWindow(void* Handle, std::uint32_t InWidth, std::uint32_t InHeight, bool Fullscreen)
{
	ScreenDimension.Width = (std::size_t)InWidth;
	ScreenDimension.Height = (std::size_t)InHeight;

	InputController = sInputController::CreateUnique((HWND)Handle);
	Device->InitWindow(Handle, (std::uint32_t)ScreenDimension.Width, (std::uint32_t)ScreenDimension.Height, Fullscreen);
	Renderer = sRenderer::CreateUnique(ScreenDimension.Width, ScreenDimension.Height);
}

void sEngine::SetEngineFixedTargetElapsedSeconds(bool Enable, double DeltaTime)
{
	mStepTimer.SetFixedTimeStep(Enable);
	mStepTimer.SetTargetElapsedSeconds(DeltaTime);
}

void sEngine::SetFixedTargetElapsedSeconds(double DeltaTime)
{
	FixedStepTimer.SetTargetElapsedSeconds(DeltaTime);
}

void sEngine::EngineInternalTick()
{
	FixedStepTimer.Tick([&]()
		{
			PhysicsTick(FixedStepTimer.GetElapsedSeconds());
			FixedTick(FixedStepTimer.GetElapsedSeconds());
		});
	mStepTimer.Tick([&]()
		{
			Tick(mStepTimer.GetElapsedSeconds());
			Render();
			Present();
		});
}

void sEngine::BeginPlay()
{
	PhysicalWorld->BeginPlay();
	if (MetaWorld)
		MetaWorld->BeginPlay();
	if (InputController)
		InputController->BeginPlay();
	if (pAudio)
		pAudio->BeginPlay();

	Renderer->BeginPlay();
}

void sEngine::PhysicsTick(const double DeltaTime)
{
	if (!bPausePhysics)
		PhysicalWorld->Tick(DeltaTime);
}

void sEngine::FixedTick(const double DeltaTime)
{
	if (bPauseTick)
		return;

	if (MetaWorld)
		MetaWorld->FixedUpdate(DeltaTime);
	if (InputController)
		InputController->FixedUpdate(DeltaTime);
}

void sEngine::Tick(const double DeltaTime)
{
	if (bPauseTick)
		return;

	if (MetaWorld && !bPauseTick)
		MetaWorld->Tick(DeltaTime);
	if (InputController)
		InputController->Tick(DeltaTime);
	if (pAudio)
		pAudio->Tick(DeltaTime);

	Renderer->Tick(DeltaTime);
}

void sEngine::Render()
{
	Renderer->Render();
}

void sEngine::Present()
{
	Device->Present(Renderer->GetFinalRenderTarget());
}

bool sEngine::SetMetaWorld(const std::shared_ptr<IMetaWorld>& pMetaWorld)
{
	if (!pMetaWorld || !Renderer)
		return false;

	if (MetaWorld)
		DestroyWorld();

	MetaWorld = pMetaWorld;
	Renderer->SetMetaWorld(MetaWorld.get());

	return true;
}

void sEngine::DestroyWorld()
{
	MetaWorld = nullptr;
	Renderer->RemoveWorld();
}

IMetaWorld* sEngine::GetMetaWorld() const
{
	return MetaWorld.get();
}

void sEngine::FullScreen(const bool value)
{
	Device->FullScreen(value);
}

bool sEngine::IsFullScreen() const
{
	return Device->IsFullScreen();
}

bool sEngine::IsVsyncEnabled() const
{
	return Device->IsVsyncEnabled();
}

void sEngine::Vsync(const bool value)
{
	Device->Vsync(value);
}

void sEngine::VsyncInterval(const std::uint32_t value)
{
	Device->VsyncInterval(value);
}

std::uint32_t sEngine::GetVsyncInterval() const
{
	return Device->GetVsyncInterval();
}

sGPUInfo sEngine::GetGPUInfo() const
{
	return Device->GetGPUInfo();
}

std::vector<sDisplayMode> sEngine::GetAllSupportedResolutions() const
{
	return Device->GetAllSupportedResolutions();
}

sScreenDimension sEngine::GetScreenDimension() const
{
	return ScreenDimension;
}

void sEngine::ResizeWindow(std::size_t InWidth, std::size_t InHeight)
{
	ScreenDimension.Width = InWidth;
	ScreenDimension.Height = InHeight;

	Device->ResizeWindow(ScreenDimension.Width, ScreenDimension.Height);
	MetaWorld->OnResizeWindow(ScreenDimension.Width, ScreenDimension.Height);
	Renderer->OnResizeWindow(ScreenDimension.Width, ScreenDimension.Height);
}

sScreenDimension sEngine::GetInternalBaseRenderResolution() const
{
	return Renderer->GetInternalBaseRenderResolution();
}

void sEngine::SetInternalBaseRenderResolution(std::size_t Width, std::size_t Height)
{
	Renderer->SetInternalBaseRenderResolution(Width, Height);
}

void sEngine::SetGBufferClearMode(EGBufferClear Mode)
{
	return Renderer->SetGBufferClearMode(Mode);
}

EGBufferClear sEngine::GetGBufferClearMode() const
{
	return Renderer->GetGBufferClearMode();
}

void sEngine::InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar)
{
	if (bPauseInput)
		return;

	if (MetaWorld)
		MetaWorld->InputProcess(MouseInput, KeyboardChar);
	if (InputController)
		InputController->InputProcess(MouseInput, KeyboardChar);

	Renderer->OnInputProcess(MouseInput, KeyboardChar);
}
