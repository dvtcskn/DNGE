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
#include "Network.h"
#include "RemoteProcedureCall.h"
#include "Utilities/ConfigManager.h"

namespace
{
	static std::unique_ptr<IAbstractGIDevice> Device = nullptr;
	static std::unique_ptr<sRenderer> Renderer = nullptr;
	static std::shared_ptr<IPhysicalWorld> PhysicalWorld = nullptr;
	static std::unique_ptr<sInputController> InputController = nullptr;
	static std::unique_ptr<XAudio> pAudio = nullptr;
	static ThreadPool mThreadPool;
	static IServer::UniquePtr Server = nullptr;
	static IClient::UniquePtr Client = nullptr;
	static sDateTime AppStartTime = sDateTime();

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
		return EFormat::R32G8X24_Typeless; //EFormat::R32G8X24_Typeless; // EFormat::R32_Typeless
	}

	void SetGBufferClearMode(EGBufferClear Mode)
	{
		Renderer->SetGBufferClearMode(Mode);
	}

	EGBufferClear GetGBufferClearMode()
	{
		return Renderer->GetGBufferClearMode();
	}

	std::uint32_t GetGBufferTextureEntryPoint()
	{
		return 2;
	}

	std::uint32_t GetGBufferTextureSize()
	{
		return 1;
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

namespace Network
{
	bool CreateSession(std::string Name, sGameInstance* Instance, std::string Level, std::size_t PlayerCount, std::uint16_t Port)
	{
		if (!Instance)
			return false;
		if (Instance->GetPlayerCount() == 0)
			return false;

		if (Client)
		{
			if (Client->IsConnected())
				Client->Disconnect();
		}

		if (!Server)
		{
			Server = CreateServer();
		}
		else
		{
			if (Server->IsServerRunning())
			{
				//Server->DestroySession();
				return false;
			}
		}
		bool Result = Server->CreateSession(Name, Instance, Level, Port, PlayerCount);
		if (!Result)
			return false;

		if (!Client)
		{
			Client = CreateClient();
		}
		else
		{
			if (Client->IsConnected())
				Client->Disconnect();
		}
		Result = Client->Connect(Instance, "127.0.0.1", Port);
		if (!Result)
		{
			Server->DestroySession();
			return false;
		}
		return true;
	}

	void DestroySession()
	{
		if (!Server)
			return;
		Server->DestroySession();
	}

	bool IsServerRunning()
	{
		if (!Server)
			return false;
		return Server->IsServerRunning();
	}

	std::size_t GetPlayerSize()
	{
		if (!Server)
			return 0;
		return Server->GetPlayerSize();
	}

	bool IsHost()
	{
		if (!Server/* || !Client*/)
			return false;
		return Server->IsServerRunning()/* && Client->IsConnected()*/;
	}

	bool IsClient()
	{
		if (!Client)
			return false;
		if (Server)
			return !Server->IsServerRunning() && Client->IsConnected();
		return Client->IsConnected();
	}

	bool ServerChangeLevel(std::string Level)
	{
		if (!Server)
			return false;
		return Server->ChangeLevel(Level);
	}

	void SetServerName(std::string Name)
	{
		if (!Server)
			Server->SetServerName(Name);
	}

	void SetServerMaximumMessagePerTick(std::size_t Size)
	{
		if (!Server)
			return;
		return Server->SetMaximumMessagePerTick(Size);
	}

	std::size_t GetServerMaximumMessagePerTick()
	{
		if (!Server)
			return 0;
		return Server->GetMaximumMessagePerTick();
	}

	void SetClientMaximumMessagePerTick(std::size_t Size)
	{
		if (!Client)
			return;
		return Client->SetMaximumMessagePerTick(Size);
	}

	std::size_t GetClientMaximumMessagePerTick()
	{
		if (!Client)
			return 0;
		return Client->GetMaximumMessagePerTick();
	}

	std::string GetServerLevel()
	{
		if (!Server)
			return "";
		return Server->GetLevel();
	}

	bool Connect(sGameInstance* Instance)
	{
		if (Server)
		{
			if (Server->IsServerRunning())
				Server->DestroySession();
		}

		if (!Client)
		{
			Client = CreateClient();
		}
		else
		{
			if (Client->IsConnected())
				Client->Disconnect();
		}

		std::string ip = ConfigManager::Get().GetGameConfig().IP;
		std::uint16_t Port = ConfigManager::Get().GetGameConfig().Port;

		return Client->Connect(Instance, ip, Port);
	}

	bool Connect(sGameInstance* Instance, std::string ip, std::uint16_t Port)
	{
		if (Server)
		{
			if (Server->IsServerRunning())
				Server->DestroySession();
		}

		if (!Client)
		{
			Client = CreateClient();
		}
		else
		{
			if (Client->IsConnected())
				Client->Disconnect();
		}

		return Client->Connect(Instance, ip, Port);
	}

	bool Disconnect()
	{
		if (!Client)
			return false;
		return Client->Disconnect();
	}

	bool IsConnected()
	{
		if (!Client)
			return false;
		return Client->IsConnected();
	}

	void RegisterRPC(std::string Address, std::string ClassName, RemoteProcedureCallBase* RPC)
	{
		RemoteProcedureCallManager::Get().Register(Address, ClassName, RPC);
	}

	void UnregisterRPC(std::string Address)
	{
		RemoteProcedureCallManager::Get().Unregister(Address);
	}

	void UnregisterRPC(std::string Address, std::string ClassName)
	{
		RemoteProcedureCallManager::Get().Unregister(Address, ClassName);
	}

	void UnregisterRPC(std::string Address, std::string ClassName, const std::string& rpcName)
	{
		RemoteProcedureCallManager::Get().Unregister(Address, ClassName, rpcName);
	}
	
	void CallRPC(std::string Address, std::string ClassName, std::string Name, std::optional<bool> reliable)
	{
		CallRPC(Address, ClassName, Name, sArchive(), reliable);
	}
	
	void CallRPC(std::string Address, std::string ClassName, std::string Name, const sArchive& Params, std::optional<bool> reliable)
	{
		if (Server)
		{
			Server->CallRPC(Address, ClassName, Name, Params, reliable);
		}
		else if (Client)
		{
			Client->CallRPC(Address, ClassName, Name, Params, reliable);
		}
	}
	
	/*bool CallRPCFromClient(std::string Name, bool reliable)
	{
		if (!Client)
			return false;
		return Client->CallRPC("0::GNSClient", Name, reliable);
	}

	bool CallRPCFromClient(std::string Name, const sArchive& Params, bool reliable)
	{
		if (!Client)
			return false;
		return Client->CallRPC("0::GNSClient", Name, Params, reliable);
	}*/
	
	std::uint64_t GetLatency()
	{
		return Client ? Client->GetLatency() : 0;
	}
}

namespace Engine
{
	void WriteToConsole(const std::string& STR)
	{
		if (Server)
			std::cout << "Server : " << STR << std::endl;
		else if (Client)
			std::cout << "Client : " << STR << std::endl;
		else
			std::cout << STR << std::endl;
	}

	void LocalUTCTimeNow(std::int32_t& Year, int32_t& Month, int32_t& DayOfWeek, int32_t& Day, int32_t& Hour, int32_t& Min, int32_t& Sec, int32_t& MSec)
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

	void UTCTimeNow(std::int32_t& Year, std::int32_t& Month, std::int32_t& DayOfWeek, std::int32_t& Day, std::int32_t& Hour, std::int32_t& Min, std::int32_t& Sec, std::int32_t& MSec)
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

	sDateTime GetLocalUTCTimeNow()
	{
		sDateTime Time;
		LocalUTCTimeNow(Time.Year, Time.Month, Time.DayOfWeek, Time.Day, Time.Hour, Time.Minute, Time.Second, Time.Millisecond);
		return Time;
	}

	sDateTime GetUTCTimeNow()
	{
		sDateTime Time;
		UTCTimeNow(Time.Year, Time.Month, Time.DayOfWeek, Time.Day, Time.Hour, Time.Minute, Time.Second, Time.Millisecond);
		return Time;
	}

	sDateTime GetAppStartTime()
	{
		return AppStartTime;
	}

	sDateTime GetExecutionTime()
	{
		return Engine::GetUTCTimeNow() - AppStartTime;
	}

	std::uint64_t GetExecutionTimeInMS()
	{
		return GetExecutionTime().GetTotalMillisecond();
	}

	std::uint64_t GetExecutionTimeInSecond()
	{
		return GetExecutionTime().GetTotalSeconds();
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
		return PhysicalWorld ? PhysicalWorld->GetPhysicsEngineType() : EPhysicsEngine::eNone;
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
		if (PhysicalWorld)
			return PhysicalWorld->SetPhysicsInternalTick(Tick);
	}

	void SetGravity(const FVector& Gravity)
	{
		if (PhysicalWorld)
			return PhysicalWorld->SetGravity(Gravity);
	}

	FVector GetGravity()
	{
		return PhysicalWorld ? PhysicalWorld->GetGravity() : FVector::Zero();
	}

	void SetWorldOrigin(const FVector& newOrigin)
	{
		if (PhysicalWorld)
			return PhysicalWorld->SetWorldOrigin(newOrigin);
	}

	sPhysicalComponent* LineTraceToViewPort(const FVector& InOrigin, const FVector& InDirection)
	{
		return PhysicalWorld ? PhysicalWorld->LineTraceToViewPort(InOrigin, InDirection) : nullptr;
	}
	std::vector<sPhysicalComponent*> QueryAABB(const FBoundingBox& Bounds)
	{
		return PhysicalWorld ? PhysicalWorld->QueryAABB(Bounds) : std::vector<sPhysicalComponent*>();
	}

	float Physics::GetPhysicalWorldScale()
	{
		return PhysicalWorld ? PhysicalWorld->GetPhysicalWorldScale() : -1.0f;
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

IConstantBuffer::SharedPtr IConstantBuffer::Create(std::string InName, const BufferLayout& InDesc, std::uint32_t InRootParameterIndex)
{
	return Device->CreateConstantBuffer(InName, InDesc, InRootParameterIndex);
}

IConstantBuffer::UniquePtr IConstantBuffer::CreateUnique(std::string InName, const BufferLayout& InDesc, std::uint32_t InRootParameterIndex)
{
	return Device->CreateUniqueConstantBuffer(InName, InDesc, InRootParameterIndex);
}

IVertexBuffer::SharedPtr IVertexBuffer::Create(std::string InName, const BufferLayout& InDesc, BufferSubresource* InSubresource)
{
	return Device->CreateVertexBuffer(InName, InDesc, InSubresource);
}

IVertexBuffer::UniquePtr IVertexBuffer::CreateUnique(std::string InName, const BufferLayout& InDesc, BufferSubresource* InSubresource)
{
	return Device->CreateUniqueVertexBuffer(InName, InDesc, InSubresource);
}

IIndexBuffer::SharedPtr IIndexBuffer::Create(std::string InName, const BufferLayout& InDesc, BufferSubresource* InSubresource)
{
	return Device->CreateIndexBuffer(InName, InDesc, InSubresource);
}

IIndexBuffer::UniquePtr IIndexBuffer::CreateUnique(std::string InName, const BufferLayout& InDesc, BufferSubresource* InSubresource)
{
	return Device->CreateUniqueIndexBuffer(InName, InDesc, InSubresource);
}

IUnorderedAccessBuffer::SharedPtr IUnorderedAccessBuffer::Create(std::string InName, const BufferLayout& InDesc, bool bSRVAllowed)
{
	return IUnorderedAccessBuffer::SharedPtr();
}

IUnorderedAccessBuffer::UniquePtr IUnorderedAccessBuffer::CreateUnique(std::string InName, const BufferLayout& InDesc, bool bSRVAllowed)
{
	return IUnorderedAccessBuffer::UniquePtr();
}

IIndirectBuffer::SharedPtr IIndirectBuffer::Create(std::string InName)
{
	return IIndirectBuffer::UniquePtr();
}

IIndirectBuffer::UniquePtr IIndirectBuffer::CreateUnique(std::string InName)
{
	return IIndirectBuffer::UniquePtr();
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
	return PhysicalWorld ? PhysicalWorld->Create2DBoxBody(Owner, Desc, Bounds) : nullptr;
}
IRigidBody::SharedPtr IRigidBody::Create2DPolygonBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, const std::array<FVector2, 8>& points)
{
	return PhysicalWorld ? PhysicalWorld->Create2DPolygonBody(Owner, Desc, Origin, points) : nullptr;
}
IRigidBody::SharedPtr IRigidBody::Create2DCircleBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, float InRadius)
{
	return PhysicalWorld ? PhysicalWorld->Create2DCircleBody(Owner, Desc, Origin, InRadius) : nullptr;
}
IRigidBody::SharedPtr IRigidBody::Create2DEdgeBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, const std::array<FVector2, 4>& points, bool OneSided)
{
	return PhysicalWorld ? PhysicalWorld->Create2DEdgeBody(Owner, Desc, Origin, points, OneSided) : nullptr;
}
IRigidBody::SharedPtr IRigidBody::Create2DChainBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, const std::vector<FVector2>& vertices)
{
	return PhysicalWorld ? PhysicalWorld->Create2DChainBody(Owner, Desc, Origin, vertices) : nullptr;
}
IRigidBody::SharedPtr IRigidBody::Create2DChainBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector2& Origin, const std::vector<FVector2>& vertices, const FVector2& prevVertex, const FVector2& nextVertex)
{
	return PhysicalWorld ? PhysicalWorld->Create2DChainBody(Owner, Desc, Origin, vertices, prevVertex, nextVertex) : nullptr;
}

IRigidBody::SharedPtr IRigidBody::CreateBoxBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, FVector InHalf)
{
	return PhysicalWorld ? PhysicalWorld->CreateBoxBody(Owner, Desc, Origin, InHalf) : nullptr;
}
IRigidBody::SharedPtr IRigidBody::CreateSphereBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, float InRadius)
{
	return PhysicalWorld ? PhysicalWorld->CreateSphereBody(Owner, Desc, Origin, InRadius) : nullptr;
}
IRigidBody::SharedPtr IRigidBody::CreateCapsuleBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, float InRadius, float InHeight)
{
	return PhysicalWorld ? PhysicalWorld->CreateCapsuleBody(Owner, Desc, Origin, InRadius, InHeight) : nullptr;
}
IRigidBody::SharedPtr IRigidBody::CreateCylinderBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, float InRadius, float InHeight)
{
	return PhysicalWorld ? PhysicalWorld->CreateCylinderBody(Owner, Desc, Origin, InRadius, InHeight) : nullptr;
}
IRigidBody::SharedPtr IRigidBody::CreateConeBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, float InRadius, float InHeight)
{
	return PhysicalWorld ? PhysicalWorld->CreateConeBody(Owner, Desc, Origin, InRadius, InHeight) : nullptr;
}

IRigidBody::SharedPtr IRigidBody::CreateMultiBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, FVector InInertia, float InMass)
{
	return PhysicalWorld ? PhysicalWorld->CreateMultiBody(Owner, Desc, Origin, InInertia, InMass) : nullptr;
}
IRigidBody::SharedPtr IRigidBody::CreateConvexHullBody(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, const float* points, int numPoints, int stride)
{
	return PhysicalWorld ? PhysicalWorld->CreateConvexHullBody(Owner, Desc, Origin, points, numPoints, stride) : nullptr;
}
IRigidBody::SharedPtr IRigidBody::CreateTriangleMesh(sPhysicalComponent* Owner, const sRigidBodyDesc& Desc, const FVector& Origin, const FVector* points, int numPoints, const std::uint32_t* indices, int numIndices)
{
	return PhysicalWorld ? PhysicalWorld->CreateTriangleMesh(Owner, Desc, Origin, points, numPoints, indices, numIndices) : nullptr;
}

sEngine::sEngine(const EGITypes GIType, const IPhysicalWorld::SharedPtr& InPhysicalWorld, std::optional<short> GPUIndex)
	: MetaWorld(nullptr)
	, ScreenDimension(sScreenDimension())
{
	AppStartTime = Engine::GetUTCTimeNow();

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
	if (Server)
		Server->DestroySession();

	Client = nullptr;
	Server = nullptr;
	InputController = nullptr;
	sMaterialManager::Get().Destroy();
	sTextureManager::Get().Destroy();
	sShaderManager::Get().Destroy();
	DestroyWorld();
	PhysicalWorld = nullptr;
	Renderer = nullptr;
	pAudio = nullptr;
	mThreadPool.Stop();
	RemoteProcedureCallManager::Get().Destroy();
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
	if (PhysicalWorld)
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
	if (PhysicalWorld && !bPausePhysics)
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

	if (Server)
		Server->Tick(DeltaTime);
	if (Client)
		Client->Tick(DeltaTime);

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
