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

#include "Engine/ClassBody.h"
#include "Utilities/Input.h"
#include "Engine/IMetaWorld.h"
#include <mutex>
#include "Core/Archive.h"
#include <stdio.h>
#include "Gameplay/GameInstance.h"
#include "Engine/StepTimer.h"

#if Enable_ENET
#include <enet/enet.h>
#endif

/*
* Be ready for weird memory leaks during shutdown.
* Because it has little and incomplete documentation.
* However, it is implemented properly and works.
*/
#define Enable_GameNetworkingSockets 0
/*
* TCP Only, UDP WIP
*/
#define Enable_Winsock 1
/*
* WIP
*/
#define Enable_ENET 0

#if Enable_Winsock
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#endif

#if Enable_GameNetworkingSockets
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
#include <steam/steam_api.h>
#endif
#endif

enum class eNetworkPacketType : std::uint8_t
{
	Validation,
	String,
	RPC,
	DirectCall,
};

/*__declspec(align(256))*/ struct sPacket
{
	sBaseClassBody(sClassConstructor, sPacket)
public:
	sDateTime TimeStamp = sDateTime();
	std::string Address = "";
	eNetworkPacketType Type = eNetworkPacketType::RPC;
	std::string ClassName = "";
	std::string FunctionName = "";
	std::string Data = "";

	sPacket() = default;
	sPacket(sDateTime InTimeStamp, eNetworkPacketType InType, std::string InAddress, std::string InClassName, std::string InFunctionName, std::string InData)
		: TimeStamp(InTimeStamp)
		, Address(InAddress)
		, Type(InType)
		, ClassName(InClassName)
		, FunctionName(InFunctionName)
		, Data(InData)
	{}

	friend void operator<<(sArchive& Archive, const sPacket& data)
	{
		Archive << data.TimeStamp;
		Archive << data.Address;
		Archive << (std::uint8_t)data.Type;
		Archive << data.ClassName;
		Archive << data.FunctionName;
		Archive << data.Data;
	}

	friend void operator>>(const sArchive& Archive, sPacket& data)
	{
		Archive >> data.TimeStamp;
		Archive >> data.Address;
		std::uint8_t eType = 0;
		Archive >> eType;
		Archive >> data.ClassName;
		Archive >> data.FunctionName;
		//Archive >> data.Data;
		data.Data.assign(Archive.GetDataAsString(), Archive.GetPosition(), Archive.GetSize() - Archive.GetPosition());
		data.Type = (eNetworkPacketType)eType;
	}
};

struct sClientInfo
{
	std::string PlayerName;
	friend void operator<<(sArchive& Archive, const sClientInfo& data)
	{
		Archive << data.PlayerName;
	}

	friend void operator>>(const sArchive& Archive, sClientInfo& data)
	{
		Archive >> data.PlayerName;
	}
};

struct sServerInfo
{
	std::string ServerName;
	std::size_t ConnectedPlayerCount;
	std::size_t MaximumConnectedPlayerSize;
	std::string LevelName;

	struct sConnectedPlayerInfo
	{
		std::uint32_t ID = 0;
		bool bIsValid = false;
		std::uint64_t Ping = 0;
		eNetworkRole Role = eNetworkRole::None;
		std::string NetworkAddress;
		std::uint32_t PlayerIndex;
		std::string PlayerName;

		friend void operator<<(sArchive& Archive, const sConnectedPlayerInfo& data)
		{
			Archive << data.NetworkAddress;
			Archive << data.PlayerIndex;
			Archive << data.PlayerName;
		}

		friend void operator>>(const sArchive& Archive, sConnectedPlayerInfo& data)
		{
			Archive >> data.NetworkAddress;
			Archive >> data.PlayerIndex;
			Archive >> data.PlayerName;
		}

		friend bool operator==(sConnectedPlayerInfo v1, sConnectedPlayerInfo v2)
		{
			return v1.ID == v2.ID && v1.NetworkAddress == v2.NetworkAddress && v1.PlayerIndex == v2.PlayerIndex && v1.PlayerName == v2.PlayerName && v1.Role == v2.Role;
		}
		friend bool operator!=(sConnectedPlayerInfo v1, sConnectedPlayerInfo v2)
		{
			return v1.ID != v2.ID || v1.NetworkAddress != v2.NetworkAddress || v1.PlayerIndex != v2.PlayerIndex || v1.PlayerName != v2.PlayerName || v1.Role != v2.Role;
		}
	};

	std::vector<sConnectedPlayerInfo> ConnectedPlayerInfos;

	friend void operator<<(sArchive& Archive, const sServerInfo& data)
	{
		Archive << data.ServerName;
		Archive << data.ConnectedPlayerCount;
		Archive << data.MaximumConnectedPlayerSize;
		Archive << data.LevelName;
		Archive << data.ConnectedPlayerInfos;
	}

	friend void operator>>(const sArchive& Archive, sServerInfo& data)
	{
		Archive >> data.ServerName;
		Archive >> data.ConnectedPlayerCount;
		Archive >> data.MaximumConnectedPlayerSize;
		Archive >> data.LevelName;
		Archive >> data.ConnectedPlayerInfos;
	}
};

class IServer
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IServer)
public:
	virtual bool CreateSession(std::string Name, sGameInstance* Instance, std::string Level, std::uint16_t Port = 27020, std::size_t PlayerCount = 8) = 0;
	virtual bool DestroySession() = 0;

	virtual void Tick(const double DeltaTime) = 0;

	virtual sGameInstance* GetGameInstance() const = 0;

	virtual bool IsServerRunning() const = 0;

	virtual std::size_t GetPlayerSize() const = 0;

	virtual std::string GetLevel() const = 0;
	virtual bool ChangeLevel(std::string Level) = 0;

	virtual void SetServerName(std::string Name) = 0;

	virtual void SetMaximumMessagePerTick(std::size_t Size) = 0;
	virtual std::size_t GetMaximumMessagePerTick() const = 0;

	virtual void CallRPC(std::string Address, std::string ClassName, std::string Name, const sArchive& Params, std::optional<bool> reliable = std::nullopt) = 0;

	void OnSessionCreated();
	void OnSessionDestroyed();
	void OnPlayerConnectedToServer(std::string PlayerName, std::string NetAddress);
	void OnPlayerDisconnectedFromServer(std::string PlayerName, std::string NetAddress);
};

class IClient
{
	sBaseClassBody(sClassDefaultProtectedConstructor, IClient)
public:
	virtual void Tick(const double DeltaTime) = 0;

	virtual sGameInstance* GetGameInstance() const = 0;

	virtual bool Connect(sGameInstance* Instance, std::string ip = "127.0.0.1", std::uint16_t Port = 27020) = 0;
	virtual bool Disconnect() = 0;
	virtual bool IsConnected() const = 0;

	virtual void SetMaximumMessagePerTick(std::size_t Size) = 0;
	virtual std::size_t GetMaximumMessagePerTick() const = 0;

	virtual void CallRPC(std::string Address, std::string ClassName, std::string Name, const sArchive& Params, std::optional<bool> reliable = std::nullopt) = 0;

	virtual std::uint64_t GetLatency() const = 0;

	void OnConnectedToServer();
	void OnDisconnectedFromServer();
	void OnPlayerConnectedToServer(std::string PlayerName, std::string NetAddress);
	void OnPlayerDisconnectedFromServer(std::string PlayerName, std::string NetAddress);
};

#if Enable_GameNetworkingSockets

class GNSServer : public IServer
{
	sClassBody(sClassConstructor, GNSServer, IServer)
public:
	GNSServer();
	virtual ~GNSServer();

	virtual void Tick(const double DeltaTime) override final;

	virtual sGameInstance* GetGameInstance() const override final { return Instance; }

	virtual bool CreateSession(std::string Name, sGameInstance* Instance, std::string Level, std::uint16_t Port = 27020, std::size_t PlayerCount = 8) override;
	virtual bool DestroySession() override;

	virtual bool IsServerRunning() const override { return bIsServerRunning; }

	virtual std::size_t GetPlayerSize() const override { return ServerInfo.MaximumConnectedPlayerSize; }

	void CallRPCFromClient(HSteamNetConnection clientID, std::string Address, std::string ClassName, std::string FunctionName, std::optional<std::string> Data = std::nullopt, bool reliable = true);
	void CallRPCFromClients(std::string Address, std::string ClassName, std::string FunctionName, std::optional<std::string> Data = std::nullopt, bool reliable = true, HSteamNetConnection excludeClientID = k_HSteamNetConnection_Invalid);

	template <typename... Args>
	void CallRPCFromClientEx(HSteamNetConnection clientID, std::string Address, std::string ClassName, std::string FunctionName, bool reliable, Args&&... args)
	{
		CallRPCFromClient(clientID, Address, ClassName, FunctionName, sArchive(args...), reliable);
	}
	template <typename... Args>
	void CallRPCFromClientsEx(std::string Address, std::string ClassName, std::string FunctionName, bool reliable, HSteamNetConnection excludeClientID, Args&&... args)
	{
		CallRPCFromClients(Address, ClassName, FunctionName, sArchive(args...), reliable, excludeClientID);
	}
	void DirectCallToClient(HSteamNetConnection clientID, std::string FunctionName, bool reliable = true, std::optional<std::string> Data = std::nullopt);
	void DirectCallToClients(std::string FunctionName, HSteamNetConnection excludeClientID, bool reliable = true, std::optional<std::string> Data = std::nullopt);
	template <typename... Args>
	void DirectCallToClientEx(HSteamNetConnection clientID, std::string Name, bool reliable, Args&&... args)
	{
		DirectCallToClient(clientID, Name, reliable, sArchive(args...));
	}
	template <typename... Args>
	void DirectCallToClientsEx(std::string Name, HSteamNetConnection excludeClientID, bool reliable, Args&&... args)
	{
		DirectCallToClients(Name, excludeClientID, reliable, sArchive(args...));
	}

	void CallMessageRPCFromClient(HSteamNetConnection clientID, std::string Message, bool reliable = true);
	void CallMessageRPCFromClients(std::string Message, bool reliable = true, HSteamNetConnection excludeClientID = k_HSteamNetConnection_Invalid);

	void SendToClient(HSteamNetConnection clientID, const sArchive& Archive, bool reliable = true);
	void SendToClients(const sArchive& Archive, bool reliable = true, HSteamNetConnection excludeClientID = k_HSteamNetConnection_Invalid);

	void SendBufferToClient(HSteamNetConnection clientID, const void* buffer, std::size_t size, bool reliable = true);
	void SendBufferToAllClients(const void* buffer, std::size_t size, bool reliable = true, HSteamNetConnection excludeClientID = k_HSteamNetConnection_Invalid);

	void SendStringToClient(HSteamNetConnection clientID, const std::string& string, bool reliable = true);
	void SendStringToAllClients(const std::string& string, bool reliable = true, HSteamNetConnection excludeClientID = k_HSteamNetConnection_Invalid);

	void PushMessageForAllClients(void* buffer, std::size_t size, bool reliable = true, HSteamNetConnection excludeClientID = k_HSteamNetConnection_Invalid);
	void PushMessage(HSteamNetConnection clientID, void* buffer, std::size_t size, bool reliable = true);
	void SendMessages();

	void KickClient(HSteamNetConnection clientID);
	void SetDebugClientNick(HSteamNetConnection hConn, const char* nick);

	virtual std::string GetLevel() const override;
	virtual bool ChangeLevel(std::string Level) override;

	virtual void SetServerName(std::string Name) override;

	virtual void SetMaximumMessagePerTick(std::size_t Size) override;
	virtual std::size_t GetMaximumMessagePerTick() const override { return MaximumMessagePerTick; }

	virtual void CallRPC(std::string Address, std::string ClassName, std::string Name, const sArchive& Params, std::optional<bool> reliable = std::nullopt) override;
	template <typename... Args>
	void CallRPCEx(std::uint32_t CalledPlayerIndex, std::string Address, std::string ClassName, std::string Name, std::optional<bool> reliable = std::nullopt, Args&&... args)
	{
		return CallRPC(CalledPlayerIndex, Address, ClassName, Name, sArchive(args...), reliable);
	}

	void PingClient(HSteamNetConnection clientID);

private:
	void PollIncomingMessages();

	void PollConnectionStateChanges();

	static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo);
	void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);

	void OnServerInfoRefresh();
	void OnPlayerConnecting(HSteamNetConnection ID);
	void OnPlayerConnected(HSteamNetConnection ID);
	void OnPlayerDisconnected(HSteamNetConnection ID);
	void OnPlayerChangeName(HSteamNetConnection ID, std::string Name);

	void PrintToConsole(std::string Message);

	bool IsPlayerExist(std::string Name) const;
	bool IsPlayerExist(HSteamNetConnection ID) const;
	sServerInfo::sConnectedPlayerInfo GetPlayerInfo(HSteamNetConnection ID) const;

	HSteamNetConnection GetPlayerIDFromAddress(std::string NetworkAddress) const;
	HSteamNetConnection GetPlayerIDFromName(std::string Name) const;
	std::uint32_t GetPlayerIndexFromName(std::string Name) const;
	HSteamNetConnection GetPlayerIDFromIndex(std::uint32_t Index) const;
	std::uint32_t GetPlayerIndexFromID(HSteamNetConnection ID) const;
	std::uint32_t GetNextPlayerIndex() const;

	void OnClientSuccessfullyConnected(HSteamNetConnection ID, sClientInfo Info);

	void HandleMessages(HSteamNetConnection ID, sPacket Packet, std::optional<bool> reliable = std::nullopt);

	bool IsPlayerNameUnique(HSteamNetConnection ID, std::string Name) const;

	void ValidateClient(HSteamNetConnection ID, std::string Data);

	void PingFromClient(HSteamNetConnection clientID, std::uint64_t TimeMS);

private:
	std::mutex Mutex;
	bool bIsServerRunning;

	HSteamListenSocket m_hListenSock;
	HSteamNetPollGroup m_hPollGroup;
	ISteamNetworkingSockets* m_pInterface;
	ISteamNetworkingUtils* Utils;

	SteamNetworkingIPAddr serverLocalAddr;

	sGameInstance* Instance;

	std::size_t MaximumMessagePerTick;

	std::vector<SteamNetworkingMessage_t*> Messages;
	sServerInfo ServerInfo;

	std::vector<HSteamNetConnection> KickList;
	std::vector<std::uint32_t> BannedIPList;

private:
	void StringFromClient(std::uint32_t ClientID, std::string STR);
};

class GNSClient : public IClient
{
	sClassBody(sClassConstructor, GNSClient, IClient)
public:
	GNSClient();
	virtual ~GNSClient();

	virtual void Tick(const double DeltaTime) override final;

	virtual sGameInstance* GetGameInstance() const override final { return Instance; }

	virtual bool Connect(sGameInstance* Instance, std::string ip = "127.0.0.1", std::uint16_t Port = 27020) override;
	virtual bool Disconnect() override;

	virtual bool IsConnected() const override { return bIsConnected; }

	void CallRPCFromServer(std::string Address, std::string ClassName, std::string FunctionName, bool reliable = true, std::optional<std::string> Data = std::nullopt);
	template <typename... Args>
	void CallRPCFromServerEx(std::string Address, std::string ClassName, std::string Name, bool reliable, Args&&... args)
	{
		CallRPCFromServer(Address, ClassName, Name, reliable, sArchive(args...));
	}
	void DirectCallToServer(std::string FunctionName, bool reliable = true, std::optional<std::string> Data = std::nullopt);
	template <typename... Args>
	void DirectCallToServerEx(std::string Name, bool reliable, Args&&... args)
	{
		DirectCallToServer(Name, reliable, sArchive(args...));
	}
	void CallMessageRPCFromServer(std::string Message, bool reliable = true);
	void SendToServer(const sArchive& Archive, bool reliable = true);
	void SendBufferToServer(const void* buffer, std::size_t Size, bool reliable = true);
	void SendStringToServer(const std::string& string, bool reliable = true);
	void PushMessage(void* buffer, std::size_t size, bool reliable = true);
	void SendMessages();

	virtual void SetMaximumMessagePerTick(std::size_t Size) override;
	virtual std::size_t GetMaximumMessagePerTick() const override { return MaximumMessagePerTick; }

	virtual void CallRPC(std::string Address, std::string ClassName, std::string Name, const sArchive& Params, std::optional<bool> reliable = std::nullopt) override;
	template <typename... Args>
	void CallRPCEx(std::string Address, std::string ClassName, std::string Name, std::optional<bool> reliable = std::nullopt, Args&&... args)
	{
		CallRPC(Address, ClassName, Name, sArchive(args...), reliable);
	}

	void PingServer();

	virtual std::uint64_t GetLatency() const override final { return Latency; }

private:
	void PollIncomingMessages();
	void PollConnectionStateChanges();

	void StringFromServer(std::string STR);

	static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo);
	void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);

	void OnReciveServerInfo(sServerInfo Info);
	void OnConnecting(sServerInfo Info);
	void OnConnected(sServerInfo::sConnectedPlayerInfo ID, sServerInfo Info);
	void OnNewPlayerConnected(sServerInfo::sConnectedPlayerInfo ID, sServerInfo Info);
	void OnPlayerDisconnected(sServerInfo::sConnectedPlayerInfo ID, sServerInfo Info);
	bool OnServerLevelChanged(std::string Level);

	void OnNameChanged();
	void OnNameChangedFromServer(std::string NewName);
	void OnPlayerNameChanged(sServerInfo pInfo);

	void PrintToConsole(std::string Message);

	void HandleMessages(sPacket Packet);

	void ClientValidation();

	void PingFromServer(std::uint64_t duration);

private:
	std::mutex Mutex;
	bool bIsConnected;

	HSteamNetConnection m_hConnection;
	ISteamNetworkingSockets* m_pInterface;
	SteamNetworkingIPAddr addrServer;

	sGameInstance* Instance;

	std::size_t MaximumMessagePerTick;
	sServerInfo Info;
	sServerInfo::sConnectedPlayerInfo ClientInfo;
	std::string PlayerNetworkAddress;

	std::vector<SteamNetworkingMessage_t*> Messages;
	ISteamNetworkingUtils* Utils;

	bool bIsValidationCalled;
	std::uint64_t Latency;
	std::uint64_t Time;
};

#endif

#if Enable_Winsock
class WSServer : public IServer
{
	sClassBody(sClassConstructor, WSServer, IServer)
public:
	WSServer();
	virtual ~WSServer();

	virtual void Tick(const double DeltaTime) override final;

	virtual sGameInstance* GetGameInstance() const override final { return Instance; }

	virtual bool CreateSession(std::string Name, sGameInstance* Instance, std::string Level, std::uint16_t Port = 27020, std::size_t PlayerCount = 8) override;
	virtual bool DestroySession() override;

	virtual bool IsServerRunning() const override { return bIsServerRunning; }

	virtual std::size_t GetPlayerSize() const override { return ServerInfo.MaximumConnectedPlayerSize; }

	void CallRPCFromClient(std::uint32_t clientID, std::string Address, std::string ClassName, std::string FunctionName, std::optional<std::string> Data = std::nullopt, bool reliable = true);
	void CallRPCFromClients(std::string Address, std::string ClassName, std::string FunctionName, std::optional<std::string> Data = std::nullopt, bool reliable = true, std::uint32_t excludeClientID = 0);

	template <typename... Args>
	void CallRPCFromClientEx(std::uint32_t clientID, std::string Address, std::string ClassName, std::string FunctionName, bool reliable, Args&&... args)
	{
		CallRPCFromClient(clientID, Address, ClassName, FunctionName, sArchive(args...), reliable);
	}
	template <typename... Args>
	void CallRPCFromClientsEx(std::string Address, std::string ClassName, std::string FunctionName, bool reliable, std::uint32_t excludeClientID, Args&&... args)
	{
		CallRPCFromClients(Address, ClassName, FunctionName, sArchive(args...), reliable, excludeClientID);
	}
	void DirectCallToClient(std::uint32_t clientID, std::string FunctionName, bool reliable = true, std::optional<std::string> Data = std::nullopt);
	void DirectCallToClients(std::string FunctionName, std::uint32_t excludeClientID, bool reliable = true, std::optional<std::string> Data = std::nullopt);
	template <typename... Args>
	void DirectCallToClientEx(std::uint32_t clientID, std::string Name, bool reliable, Args&&... args)
	{
		DirectCallToClient(clientID, Name, reliable, sArchive(args...));
	}
	template <typename... Args>
	void DirectCallToClientsEx(std::string Name, std::uint32_t excludeClientID, bool reliable, Args&&... args)
	{
		DirectCallToClients(Name, excludeClientID, reliable, sArchive(args...));
	}

	void CallMessageRPCFromClient(std::uint32_t clientID, std::string Message, bool reliable = true);
	void CallMessageRPCFromClients(std::string Message, bool reliable = true, std::uint32_t excludeClientID = 0);

	void SendToClient(std::uint32_t clientID, const sArchive& Archive, bool reliable = true);
	void SendToClients(const sArchive& Archive, bool reliable = true, std::uint32_t excludeClientID = 0);

	void SendBufferToClient(std::uint32_t clientID, const void* buffer, std::size_t size, bool reliable = true);
	void SendBufferToAllClients(const void* buffer, std::size_t size, bool reliable = true, std::uint32_t excludeClientID = 0);

	void SendStringToClient(std::uint32_t clientID, const std::string& string, bool reliable = true);
	void SendStringToAllClients(const std::string& string, bool reliable = true, std::uint32_t excludeClientID = 0);

	void PushMessageForAllClients(void* buffer, std::size_t size, bool reliable = true, std::uint32_t excludeClientID = 0);
	void PushMessage(std::uint32_t clientID, void* buffer, std::size_t size, bool reliable = true);
	void SendMessages();

	void KickClient(std::uint32_t clientID);

	virtual std::string GetLevel() const override;
	virtual bool ChangeLevel(std::string Level) override;

	virtual void SetServerName(std::string Name) override;

	virtual void SetMaximumMessagePerTick(std::size_t Size) override;
	virtual std::size_t GetMaximumMessagePerTick() const override { return MaximumMessagePerTick; }

	virtual void CallRPC(std::string Address, std::string ClassName, std::string Name, const sArchive& Params, std::optional<bool> reliable = std::nullopt) override;
	template <typename... Args>
	void CallRPCEx(std::uint32_t CalledPlayerIndex, std::string Address, std::string ClassName, std::string Name, std::optional<bool> reliable = std::nullopt, Args&&... args)
	{
		return CallRPC(CalledPlayerIndex, Address, ClassName, Name, sArchive(args...), reliable);
	}

	void PingClient(std::uint32_t clientID);

private:
	void PollIncomingMessages();

	void OnServerInfoRefresh();
	void OnPlayerConnecting(std::uint32_t ID);
	void OnPlayerConnected(std::uint32_t ID);
	void OnPlayerDisconnected(std::uint32_t ID);
	void OnPlayerChangeName(std::uint32_t ID, std::string Name);

	void PrintToConsole(std::string Message);

	bool IsPlayerExist(std::string Name) const;
	bool IsPlayerExist(std::uint32_t ID) const;
	sServerInfo::sConnectedPlayerInfo GetPlayerInfo(std::uint32_t ID) const;

	std::uint32_t GetPlayerIDFromAddress(std::string NetworkAddress) const;
	std::uint32_t GetPlayerIDFromName(std::string Name) const;
	std::uint32_t GetPlayerIndexFromName(std::string Name) const;
	std::uint32_t GetPlayerIDFromIndex(std::uint32_t Index) const;
	std::uint32_t GetPlayerIndexFromID(std::uint32_t ID) const;
	std::uint32_t GetNextPlayerIndex() const;

	void OnClientSuccessfullyConnected(std::uint32_t ID, sClientInfo Info);

	void HandleMessages(std::uint32_t ID, const sPacket& Packet, std::optional<bool> reliable = std::nullopt);

	bool IsPlayerNameUnique(std::uint32_t ID, std::string Name) const;

	void ValidateClient(std::uint32_t ID, std::string Data);

	void PingFromClient(std::uint32_t clientID, std::uint64_t TimeMS);

private:
	std::mutex Mutex;
	std::atomic<bool> bIsServerRunning;

	sGameInstance* Instance;

	std::size_t MaximumMessagePerTick;

	sServerInfo ServerInfo;

	std::vector<std::uint32_t> KickList;
	std::vector<std::uint32_t> BannedIPList;

	struct sClientSocket
	{
		bool IsValid = false;
		SOCKET Socket = INVALID_SOCKET;
		std::uint32_t TimeOutTest = 0;
		sClientSocket(SOCKET Socket = INVALID_SOCKET)
			: IsValid(false)
			, Socket(Socket)
			, TimeOutTest(0)
		{}
	};
	std::map<std::uint32_t, sClientSocket> Clients;

	WSADATA wsaData;
	SOCKET ListenSocket;

	struct sMsg
	{
		std::uint32_t ID = 0;
		sPacket Packet = sPacket();
		sMsg(std::uint32_t InID, const sPacket& InPacket)
			: ID(InID)
			, Packet(InPacket)
		{}
	};
	std::queue<sMsg> Packets;

	std::uint32_t ClientCounter;

private:
	void StringFromClient(std::uint32_t ClientID, std::string STR);
};

class WSClient : public IClient
{
	sClassBody(sClassConstructor, WSClient, IClient)
public:
	WSClient();
	virtual ~WSClient();

	virtual void Tick(const double DeltaTime) override final;

	virtual sGameInstance* GetGameInstance() const override final { return Instance; }

	virtual bool Connect(sGameInstance* Instance, std::string ip = "127.0.0.1", std::uint16_t Port = 27020) override;
	virtual bool Disconnect() override;

	virtual bool IsConnected() const override { return bIsConnected; }

	void CallRPCFromServer(std::string Address, std::string ClassName, std::string FunctionName, bool reliable = true, std::optional<std::string> Data = std::nullopt);
	template <typename... Args>
	void CallRPCFromServerEx(std::string Address, std::string ClassName, std::string Name, bool reliable, Args&&... args)
	{
		CallRPCFromServer(Address, ClassName, Name, reliable, sArchive(args...));
	}
	void DirectCallToServer(std::string FunctionName, bool reliable = true, std::optional<std::string> Data = std::nullopt);
	template <typename... Args>
	void DirectCallToServerEx(std::string Name, bool reliable, Args&&... args)
	{
		DirectCallToServer(Name, reliable, sArchive(args...));
	}
	void CallMessageRPCFromServer(std::string Message, bool reliable = true);
	void SendToServer(const sArchive& Archive, bool reliable = true);
	void SendBufferToServer(const void* buffer, std::size_t Size, bool reliable = true);
	void SendStringToServer(const std::string& string, bool reliable = true);
	void PushMessage(void* buffer, std::size_t size, bool reliable = true);
	void SendMessages();

	virtual void SetMaximumMessagePerTick(std::size_t Size) override;
	virtual std::size_t GetMaximumMessagePerTick() const override { return MaximumMessagePerTick; }

	virtual void CallRPC(std::string Address, std::string ClassName, std::string Name, const sArchive& Params, std::optional<bool> reliable = std::nullopt) override;
	template <typename... Args>
	void CallRPCEx(std::string Address, std::string ClassName, std::string Name, std::optional<bool> reliable = std::nullopt, Args&&... args)
	{
		CallRPC(Address, ClassName, Name, sArchive(args...), reliable);
	}

	void PingServer();

	virtual std::uint64_t GetLatency() const override final { return Latency; }

private:
	void PollIncomingMessages();

	void StringFromServer(std::string STR);

	void OnReciveServerInfo(sServerInfo Info);
	void OnConnecting(sServerInfo Info);
	void OnConnected(sServerInfo::sConnectedPlayerInfo ID, sServerInfo Info);
	void OnNewPlayerConnected(sServerInfo::sConnectedPlayerInfo ID, sServerInfo Info);
	void OnPlayerDisconnected(sServerInfo::sConnectedPlayerInfo ID, sServerInfo Info);
	bool OnServerLevelChanged(std::string Level);

	void OnNameChanged();
	void OnNameChangedFromServer(std::string NewName);
	void OnPlayerNameChanged(sServerInfo pInfo);

	void PrintToConsole(std::string Message);

	void HandleMessages(const sPacket& Packet);

	void ClientValidation();

	void PingFromServer(std::uint64_t duration);

private:
	std::mutex Mutex;
	std::atomic<bool> bIsConnected;

	sGameInstance* Instance;

	std::size_t MaximumMessagePerTick;
	sServerInfo Info;
	sServerInfo::sConnectedPlayerInfo ClientInfo;
	std::string PlayerNetworkAddress;

	bool bIsValidationCalled;
	std::uint64_t Latency;
	std::uint64_t Time;

	WSADATA wsaData;
	SOCKET ConnectSocket;

	std::queue<sPacket> Packets;
};

#endif

#if Enable_ENET
class ENetServer : public IServer
{
	sClassBody(sClassConstructor, ENetServer, IServer)
public:
	ENetServer();
	virtual ~ENetServer();

	void StartServer(std::uint16_t Port, std::size_t PlayerCount = 8);
	void CloseServer();

	bool IsServerRunning() const { return bIsServerRunning; }

	std::size_t GetPlayerSize() const { return PlayerSize; }

private:
	void ParsePacket(int ID, char* Data);
	void SendPacket(ENetPeer* Peer, int Channel, const char* Data, bool IsReliable);
	void BroadcastPacket(const char* Data, int Channel, bool IsReliable);

private:
	std::mutex Mutex;
	std::atomic<bool> bIsServerRunning;
	std::size_t PlayerSize;
	ENetHost* Server;
};

class ENetClient : public IClient
{
	sClassBody(sClassConstructor, ENetClient, IClient)
public:
	ENetClient();
	virtual ~ENetClient();

	bool Connect(std::string ip, std::uint16_t Port = 27020);
	void Disconnect();

	bool IsConnected() const { return bJoined; }

	void* GetData() const;

private:
	void SendPacket(const char* pData, std::uint8_t Channel, bool IsReliable);

private:
	std::mutex Mutex;
	ENetHost* Client;
	ENetPeer* Peer;

	std::atomic<bool> bJoined;
};

#endif

namespace
{
	IServer::UniquePtr CreateServer()
	{
#if Enable_GameNetworkingSockets
		return GNSServer::CreateUnique();
#elif Enable_ENET
		return ENetServer::CreateUnique();
#else
		return WSServer::CreateUnique();
#endif
	}
	IClient::UniquePtr CreateClient()
	{
#if Enable_GameNetworkingSockets
		return GNSClient::CreateUnique();
#elif Enable_ENET
		return ENetClient::CreateUnique();
#else
		return WSClient::CreateUnique();
#endif
	}
}
