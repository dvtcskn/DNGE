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
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <algorithm>
#include <string>
#include <random>
#include <chrono>
#include <thread>
#include <mutex>
#include <queue>
#include <map>
#include <cctype>
#include <Windows.h>
#include <winsock2.h>
#include "Network.h"
#include "Engine/AbstractEngine.h"
#include "RemoteProcedureCall.h"
#include <chrono>

#if Enable_Winsock
// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#endif

#if Enable_GameNetworkingSockets
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
#include <steam/steam_api.h>
#endif

#pragma comment (lib, "GameNetworkingSockets.lib")
#if _DEBUG
#pragma comment (lib, "libprotobufd.lib")
#else
#pragma comment (lib, "libprotobuf.lib")
#endif
#pragma comment (lib, "libcrypto.lib")

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#endif

void IServer::OnSessionCreated()
{
	GetGameInstance()->SessionCreated();
}

void IServer::OnSessionDestroyed()
{
	GetGameInstance()->SessionDestroyed();
}

void IServer::OnPlayerConnectedToServer(std::string PlayerName, std::string NetAddress)
{
	GetGameInstance()->OnPlayerConnectedToServer(PlayerName, NetAddress);
}

void IServer::OnPlayerDisconnectedFromServer(std::string PlayerName, std::string NetAddress)
{
	GetGameInstance()->OnPlayerDisconnectedFromServer(PlayerName, NetAddress);
}

void IClient::OnConnectedToServer()
{
	GetGameInstance()->Connected();
}

void IClient::OnDisconnectedFromServer()
{
	GetGameInstance()->Disconnected();
}

void IClient::OnPlayerConnectedToServer(std::string PlayerName, std::string NetAddress)
{
	if (!Network::IsHost())
		GetGameInstance()->PlayerConnected(PlayerName, NetAddress);
}

void IClient::OnPlayerDisconnectedFromServer(std::string PlayerName, std::string NetAddress)
{
	if (!Network::IsHost())
		GetGameInstance()->PlayerDisconnected(PlayerName, NetAddress);
}

#if Enable_GameNetworkingSockets

bool bIsInitialized = false;

SteamNetworkingMicroseconds g_logTimeZero;

GNSServer* s_pServerCallbackInstance = nullptr;
GNSClient* s_pClientCallbackInstance = nullptr;

static void DebugOutput(ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg)
{
	SteamNetworkingMicroseconds time = SteamNetworkingUtils()->GetLocalTimestamp() - g_logTimeZero;
	printf("%10.6f %s\n", time * 1e-6, pszMsg);
	fflush(stdout);
	if (eType == k_ESteamNetworkingSocketsDebugOutputType_Bug)
	{
		fflush(stdout);
		fflush(stderr);
	}
}

static void FatalError(const char* fmt, ...)
{
	char text[2048];
	va_list ap;
	va_start(ap, fmt);
	vsprintf_s(text, fmt, ap);
	va_end(ap);
	char* nl = strchr(text, '\0') - 1;
	if (nl >= text && *nl == '\n')
		*nl = '\0';
	DebugOutput(k_ESteamNetworkingSocketsDebugOutputType_Bug, text);
}

static void Printf(const char* fmt, ...)
{
	char text[2048];
	va_list ap;
	va_start(ap, fmt);
	vsprintf_s(text, fmt, ap);
	va_end(ap);
	char* nl = strchr(text, '\0') - 1;
	if (nl >= text && *nl == '\n')
		*nl = '\0';
	DebugOutput(k_ESteamNetworkingSocketsDebugOutputType_Msg, text);
}

static void InitSteamDatagramConnectionSockets()
{
#ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
	SteamDatagramErrMsg errMsg;
	if (!GameNetworkingSockets_Init(nullptr, errMsg))
		FatalError("GameNetworkingSockets_Init failed.  %s", errMsg);
#else
	SteamDatagram_SetAppID(570); // Just set something, doesn't matter what
	SteamDatagram_SetUniverse(false, k_EUniverseDev);

	SteamDatagramErrMsg errMsg;
	if (!SteamDatagramClient_Init(errMsg))
		FatalError("SteamDatagramClient_Init failed.  %s", errMsg);

	// Disable authentication when running with Steam, for this
	// example, since we're not a real app.
	//
	// Authentication is disabled automatically in the open-source
	// version since we don't have a trusted third party to issue
	// certs.
	SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_IP_AllowWithoutAuth, 1);
#endif

	g_logTimeZero = SteamNetworkingUtils()->GetLocalTimestamp();

	SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput);

	bIsInitialized = true;
}

static void ShutdownSteamDatagramConnectionSockets()
{
	// Give connections time to finish up.  This is an application layer protocol
	// here, it's not TCP.  Note that if you have an application and you need to be
	// more sure about cleanup, you won't be able to do this.  You will need to send
	// a message and then either wait for the peer to close the connection, or
	// you can pool the connection to see if any reliable data is pending.
	std::this_thread::sleep_for(std::chrono::milliseconds(500));

#ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
	GameNetworkingSockets_Kill();
#else
	SteamDatagramClient_Kill();
#endif

	bIsInitialized = false;
}

GNSServer::GNSServer()
	: Super()
	, m_hListenSock(0)
	, m_hPollGroup(0)
	, m_pInterface(nullptr)
	, Instance(nullptr)
	, serverLocalAddr(SteamNetworkingIPAddr())
	, MaximumMessagePerTick(32)
	, bIsServerRunning(false)
{
	s_pServerCallbackInstance = this;
	if (!bIsInitialized)
		InitSteamDatagramConnectionSockets();
	Utils = SteamNetworkingUtils_Lib();

	ServerInfo.MaximumConnectedPlayerSize = 8;

	RegisterRPCfn("Global", "GNSServer", "StringFromClient", eRPCType::Server, true, false, std::bind(&GNSServer::StringFromClient, this, std::placeholders::_1, std::placeholders::_2), std::uint32_t, std::string);
	RegisterRPCfn("Global", "GNSServer", "OnClientSuccessfullyConnected", eRPCType::Server, true, false, std::bind(&GNSServer::OnClientSuccessfullyConnected, this, std::placeholders::_1, std::placeholders::_2), std::uint32_t, sClientInfo);
	RegisterRPCfn("Global", "GNSServer", "ClientValidation", eRPCType::Server, true, false, std::bind(&GNSServer::ValidateClient, this, std::placeholders::_1, std::placeholders::_2), std::uint32_t, std::string);
	RegisterRPCfn("Global", "GNSServer", "PingFromClient", eRPCType::Server, true, false, std::bind(&GNSServer::PingFromClient, this, std::placeholders::_1, std::placeholders::_2), std::uint32_t, std::uint64_t);
	RegisterRPCfn("Global", "GNSServer", "PingClient", eRPCType::Server, true, false, std::bind(&GNSServer::PingClient, this, std::placeholders::_1), std::uint32_t);
}

GNSServer::~GNSServer()
{
	DestroySession();

	for (auto& Message : Messages)
		Message->Release();
	Messages.clear();

	Network::UnregisterRPC("Global", "GNSServer");

	Utils = nullptr;
	Instance = nullptr;

	m_pInterface = nullptr;

	ServerInfo.ConnectedPlayerInfos.clear();

	//if (bIsInitialized)
		ShutdownSteamDatagramConnectionSockets();

	s_pServerCallbackInstance = nullptr;
}

void GNSServer::Tick(const double DeltaTime)
{
	if (bIsServerRunning)
	{
		//auto MS = Engine::GetUTCTimeNow().GetTotalMillisecond();

		//if ((MS - gTime) > 70)
		{
			PollIncomingMessages();
			PollConnectionStateChanges();
			SendMessages();
			//gTime = Engine::GetUTCTimeNow().GetTotalMillisecond();
		}
	}
}

void GNSServer::PollIncomingMessages()
{
	while (bIsServerRunning)
	{
		std::vector<ISteamNetworkingMessage*> pIncomingMsgs(MaximumMessagePerTick);
		int numMsgs = m_pInterface->ReceiveMessagesOnPollGroup(m_hPollGroup, pIncomingMsgs.data(), MaximumMessagePerTick);
		if (numMsgs == 0)
			break;
		if (numMsgs < 0)
		{
			FatalError("Error checking for messages");
			return;
		}

		for (int i = 0; i < numMsgs; i++)
		{
			ISteamNetworkingMessage* pIncomingMsg = pIncomingMsgs[i];
			if (std::find(KickList.begin(), KickList.end(), pIncomingMsg->m_conn) != KickList.end())
				continue;

			assert(IsPlayerExist(pIncomingMsg->m_conn));

			/*SteamNetConnectionRealTimeStatus_t* pStatus = nullptr;
			int nLanes = 0;
			SteamNetConnectionRealTimeLaneStatus_t* pLanes = nullptr;
			m_pInterface->GetConnectionRealTimeStatus(pIncomingMsg->m_conn, pStatus, nLanes, pLanes);

			if (pLanes)
				PrintToConsole("Ping : " + std::to_string(pLanes->m_usecQueueTime));*/

			sArchive pArchive;
			pArchive.SetData((std::uint8_t*)pIncomingMsg->m_pData, pIncomingMsg->m_cbSize);
			sPacket Packet;
			pArchive >> Packet;

			auto Info = GetPlayerInfo(pIncomingMsg->m_conn);
			if (!Info.bIsValid)
			{
				if (Packet.Type == eNetworkPacketType::Validation)
				{
					ValidateClient(pIncomingMsg->m_conn, Packet.Data);
				}
				else
				{
					PrintToConsole("Validation Skipped! msg : " + Packet.FunctionName);
					KickClient(pIncomingMsg->m_conn);
				}
			}
			else
			{
				HandleMessages(pIncomingMsg->m_conn, Packet);
			}

			pIncomingMsg->Release();
		}
	}
}

void GNSServer::HandleMessages(HSteamNetConnection ID, sPacket Packet, std::optional<bool> reliable)
{
	if (Packet.Type == eNetworkPacketType::RPC)
	{
		auto RPC = RemoteProcedureCallManager::Get().GetRPC(Packet.Address, Packet.ClassName, Packet.FunctionName);
		if (!RPC)
		{
			PrintToConsole("Failed to Call RPC | Address : " + Packet.Address + " | ClassName : " + Packet.ClassName + " | FunctionName : " + Packet.FunctionName);
			return;
		}

		//PrintToConsole("RPC Called | Address : " + Packet.Address + " | ClassName : " + Packet.ClassName + " | FunctionName : " + Packet.FunctionName);

		switch (RPC->GetType())
		{
		case eRPCType::Client:
			CallRPCFromClient(ID, Packet.Address, Packet.ClassName, Packet.FunctionName, Packet.Data, reliable.has_value() ? *reliable : RPC->IsReliable());
			CallRPCFromClients(Packet.Address, Packet.ClassName, Packet.FunctionName, Packet.Data, reliable.has_value() ? *reliable : RPC->IsReliable(), ID);
			break;
		case eRPCType::Server:
			if (RPC->IsReqTimeStamp())
				RPC->Call(sArchive(Packet.TimeStamp, Packet.Data));
			else
				RPC->Call(sArchive(Packet.Data));
			break;
		case eRPCType::ServerAndClient:
			if (RPC->IsReqTimeStamp())
				RPC->Call(sArchive(Packet.TimeStamp, Packet.Data));
			else
				RPC->Call(sArchive(Packet.Data));
			CallRPCFromClient(ID, Packet.Address, Packet.ClassName, Packet.FunctionName, Packet.Data, reliable.has_value() ? *reliable : RPC->IsReliable());
			CallRPCFromClients(Packet.Address, Packet.ClassName, Packet.FunctionName, Packet.Data, reliable.has_value() ? *reliable : RPC->IsReliable(), ID);
			break;
		}
	}
	else if (Packet.Type == eNetworkPacketType::DirectCall)
	{
		auto RPC = RemoteProcedureCallManager::Get().GetRPC("Global", "GNSServer", Packet.FunctionName);
		if (!RPC)
		{
			//PrintToConsole("RPC Not Called!");
			return;
		}

		sArchive ParamWithID;
		ParamWithID << ID;
		ParamWithID << Packet.Data;

		switch (RPC->GetType())
		{
		case eRPCType::ServerAndClient:
		case eRPCType::Client:
			//PrintToConsole("RPC Not Called!");
			break;
		case eRPCType::Server:
			RPC->Call(ParamWithID);
			break;
		}
	}
	else if (Packet.Type == eNetworkPacketType::String)
	{
		StringFromClient(ID, Packet.Data);
	}
}

bool GNSServer::CreateSession(std::string Name, sGameInstance* pInstance, std::string Level, std::uint16_t Port, std::size_t PlayerCount)
{
	std::lock_guard<std::mutex> locker(Mutex);

	if (bIsServerRunning)
		return false;

	if (!bIsInitialized)
		InitSteamDatagramConnectionSockets();

	ServerInfo.LevelName = Level;
	ServerInfo.MaximumConnectedPlayerSize = PlayerCount;

	Instance = pInstance;

	Instance->DisableSplitScreen();
	while (Instance->GetPlayerCount() != 0)
	{
		Instance->RemoveLastPlayer();
	}

	ServerInfo.ServerName = Name;

	Instance->OpenLevel(ServerInfo.LevelName);

	serverLocalAddr.Clear();
	serverLocalAddr.m_port = Port;

	// Select instance to use.  For now we'll always use the default.
	// But we could use SteamGameServerNetworkingSockets() on Steam.
	m_pInterface = SteamNetworkingSockets();

	// Start listening
	SteamNetworkingConfigValue_t opt;
	opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)GNSServer::SteamNetConnectionStatusChangedCallback);

	m_hListenSock = m_pInterface->CreateListenSocketIP(serverLocalAddr, 1, &opt);

	if (m_hListenSock == k_HSteamListenSocket_Invalid)
	{
		bIsServerRunning = false;
		FatalError("Failed to listen on port %d", serverLocalAddr.m_port);
		return false;
	}
	m_hPollGroup = m_pInterface->CreatePollGroup();
	if (m_hPollGroup == k_HSteamNetPollGroup_Invalid)
	{
		bIsServerRunning = false;
		FatalError("Failed to listen on port %d", serverLocalAddr.m_port);
		return false;
	}
	PrintToConsole("Server listening on port " + std::to_string(serverLocalAddr.m_port));

	bIsServerRunning = true;

	OnSessionCreated();

	return true;
}

bool GNSServer::DestroySession()
{
	std::lock_guard<std::mutex> locker(Mutex);

	if (!bIsServerRunning)
		return false;

	PrintToConsole("Closing connections...");
	for (auto it : ServerInfo.ConnectedPlayerInfos)
	{
		// Send them one more goodbye message.  Note that we also have the
		// connection close reason as a place to send final data.  However,
		// that's usually best left for more diagnostic/debug text not actual
		// protocol strings.

		/*sPacket Packet;
		Packet.Type = eNetworkPacketType::String;
		Packet.Name = "Server is shutting down. Goodbye.";

		auto Serialized = NetworkSerialize(Packet);
		SendStringToClient(it.first, Serialized, true);*/

		m_pInterface->FlushMessagesOnConnection(it.ID);
	}
	for (auto it : ServerInfo.ConnectedPlayerInfos)
	{
		bool Result = m_pInterface->CloseConnection(it.ID, ESteamNetConnectionEnd::k_ESteamNetConnectionEnd_App_Generic, "Server Shutdown", true);
		if (!Result)
		{
			PrintToConsole("Error::CloseConnection");
		}
	}
	//m_mapClients.clear();
	ServerInfo.ConnectedPlayerInfos.clear();

	m_pInterface->DestroyPollGroup(m_hPollGroup);
	m_hPollGroup = k_HSteamNetPollGroup_Invalid;

	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	const bool Result = m_pInterface->CloseListenSocket(m_hListenSock);
	m_hListenSock = k_HSteamListenSocket_Invalid;

	bIsServerRunning = false;

	while (Instance->GetPlayerCount() != 0)
	{
		Instance->RemoveLastPlayer();
	}

	OnSessionDestroyed();

	return Result;
}

void GNSServer::SendToClient(HSteamNetConnection clientID, const sArchive& Archive, bool reliable)
{
	SendBufferToClient(clientID, Archive.GetData().data(), Archive.GetSize(), reliable);
}

void GNSServer::SendToClients(const sArchive& Archive, bool reliable, HSteamNetConnection excludeClientID)
{
	for (const auto& clientInfo : ServerInfo.ConnectedPlayerInfos)
	{
		if (clientInfo.ID != excludeClientID)
			SendToClient(clientInfo.ID, Archive, reliable);
	}
}

void GNSServer::SendStringToAllClients(const std::string& string, bool reliable, HSteamNetConnection excludeClientID)
{
	SendBufferToAllClients(string.data(), string.size(), reliable, excludeClientID);
}

void GNSServer::SendBufferToClient(HSteamNetConnection clientID, const void* buffer, std::size_t size, bool reliable)
{
	int64 pOutMessageNumber = 0;
	EResult Result = m_pInterface->SendMessageToConnection(clientID, buffer, size, reliable ? k_nSteamNetworkingSend_Reliable : k_nSteamNetworkingSend_Unreliable, &pOutMessageNumber);
}

void GNSServer::SendBufferToAllClients(const void* buffer, std::size_t size, bool reliable, HSteamNetConnection excludeClientID)
{
	for (const auto& clientInfo : ServerInfo.ConnectedPlayerInfos)
	{
		if (clientInfo.ID != excludeClientID)
			SendBufferToClient(clientInfo.ID, buffer, size, reliable);
	}
}

void GNSServer::SendStringToClient(HSteamNetConnection clientID, const std::string& string, bool reliable)
{
	SendBufferToClient(clientID, string.data(), string.size(), reliable);
}

void GNSServer::CallRPC(std::string Address, std::string ClassName, std::string Name, const sArchive& Params, std::optional<bool> reliable)
{
	HandleMessages(GetPlayerIDFromAddress(Address), sPacket(Engine::GetUTCTimeNow(), eNetworkPacketType::RPC, Address, ClassName, Name, Params), reliable);
}

void GNSServer::CallRPCFromClient(HSteamNetConnection clientID, std::string Address, std::string ClassName, std::string FunctionName, std::optional<std::string> Data, bool reliable)
{
	sPacket Packet;
	Packet.TimeStamp = Engine::GetUTCTimeNow();
	Packet.Address = Address;
	Packet.ClassName = ClassName;
	Packet.FunctionName = FunctionName;
	Packet.Data = Data.has_value() ? *Data : "";
	Packet.Type = eNetworkPacketType::RPC;
	sArchive Archive;
	Archive << Packet;
	//SendStringToClient(clientID, Archive.GetDataAsString(), reliable);
	SendBufferToClient(clientID, Archive.GetData().data(), Archive.GetSize(), reliable);
}

void GNSServer::CallRPCFromClients(std::string Address, std::string ClassName, std::string FunctionName, std::optional<std::string> Data, bool reliable, HSteamNetConnection excludeClientID)
{
	for (const auto& clientInfo : ServerInfo.ConnectedPlayerInfos)
	{
		if (clientInfo.ID != excludeClientID)
			CallRPCFromClient(clientInfo.ID, Address, ClassName, FunctionName, Data, reliable);
	}
}

void GNSServer::DirectCallToClient(HSteamNetConnection clientID, std::string FunctionName, bool reliable, std::optional<std::string> Data)
{
	sPacket Packet;
	Packet.TimeStamp = Engine::GetUTCTimeNow();
	Packet.Address = "Global";
	Packet.ClassName = "GNSClient";
	Packet.FunctionName = FunctionName;
	Packet.Data = Data.has_value() ? *Data : "";
	Packet.Type = eNetworkPacketType::DirectCall;
	sArchive Archive;
	Archive << Packet;
	//SendStringToClient(clientID, Archive.GetDataAsString(), reliable);
	SendBufferToClient(clientID, Archive.GetData().data(), Archive.GetSize(), reliable);
}

void GNSServer::DirectCallToClients(std::string FunctionName, HSteamNetConnection excludeClientID, bool reliable, std::optional<std::string> Data)
{
	for (const auto& clientInfo : ServerInfo.ConnectedPlayerInfos)
	{
		if (clientInfo.ID != excludeClientID)
			DirectCallToClient(clientInfo.ID, FunctionName, reliable, Data);
	}
}

void GNSServer::CallMessageRPCFromClient(HSteamNetConnection clientID, std::string Message, bool reliable)
{
	sPacket Packet;
	Packet.TimeStamp = Engine::GetUTCTimeNow();
	Packet.Address = "Global";
	Packet.ClassName = "GNSClient";
	Packet.FunctionName = "StringFromServer\n";
	Packet.Data = Message;
	Packet.Type = eNetworkPacketType::String;
	sArchive Archive;
	Archive << Packet;
	//SendStringToClient(clientID, Archive.GetDataAsString(), reliable);
	SendBufferToClient(clientID, Archive.GetData().data(), Archive.GetSize(), reliable);
}

void GNSServer::CallMessageRPCFromClients(std::string Message, bool reliable, HSteamNetConnection excludeClientID)
{
	for (const auto& clientInfo : ServerInfo.ConnectedPlayerInfos)
	{
		if (clientInfo.ID != excludeClientID)
			CallMessageRPCFromClient(clientInfo.ID, Message, reliable);
	}
}

void GNSServer::PushMessageForAllClients(void* buffer, std::size_t size, bool reliable, HSteamNetConnection excludeClientID)
{
	for (const auto& clientInfo : ServerInfo.ConnectedPlayerInfos)
	{
		if (clientInfo.ID != excludeClientID)
			PushMessage(clientInfo.ID, buffer, size, reliable);
	}
}

void GNSServer::PushMessage(HSteamNetConnection clientID, void* buffer, std::size_t size, bool reliable)
{
	/*SteamNetworkingMessage_t::Release Bugged!!!*/

	/*SteamNetworkingMessage_t* Message = Utils->AllocateMessage(1);
	Message->m_conn = clientID;
	Message->m_cbSize = size;
	Message->m_pData = buffer;
	Message->m_nConnUserData = 0;
	Message->m_nMessageNumber = 1;
	Message->m_nFlags = reliable ? k_nSteamNetworkingSend_Reliable : k_nSteamNetworkingSend_Unreliable;
	Messages.push_back(Message);*/
}

void GNSServer::SendMessages()
{
	//int64 pOutMessageNumberOrResult = 0;
	//m_pInterface->SendMessages(Messages.size(), Messages.data(), &pOutMessageNumberOrResult);

	/*Release() Bugged!!!*/
	//for (auto& Message : Messages)
	//	Message->Release();
	Messages.clear();
}

void GNSServer::StringFromClient(std::uint32_t ClientID, std::string STR)
{
	PrintToConsole("MessageFromClient : ID : " + std::to_string(ClientID) + " Message : " + STR);
}

void GNSServer::KickClient(HSteamNetConnection clientID)
{
	KickList.push_back(clientID);
	std::size_t TryCounter = 0;
	bool IsClosed = false;
	OnPlayerDisconnected(clientID);
	while (!IsClosed && TryCounter < 50)
	{
		IsClosed = m_pInterface->CloseConnection(clientID, 0, nullptr, false);
		TryCounter++;
	}
	if (!IsClosed)
		PrintToConsole("Failed To Kick. Try Count : " + std::to_string(TryCounter));
	else
		PrintToConsole("Player Kicked. ID : " + std::to_string(clientID));
}

void GNSServer::SetDebugClientNick(HSteamNetConnection hConn, const char* nick)
{
	// Remember their nick
	//m_mapClients[hConn].m_sNick = nick;

	// Set the connection name, too, which is useful for debugging
	m_pInterface->SetConnectionName(hConn, nick);
	PrintToConsole("SetDebugClientNick : " + std::string(nick));
}

std::string GNSServer::GetLevel() const
{
	return ServerInfo.LevelName;
}

bool GNSServer::ChangeLevel(std::string Level)
{
	ServerInfo.LevelName = Level;
	Instance->OpenLevel(ServerInfo.LevelName);
	CallRPCFromClients("Global", "GNSClient", "OnServerLevelChanged", ServerInfo.LevelName);
	return true;
}

void GNSServer::SetServerName(std::string Name)
{
	ServerInfo.LevelName = Name;
}

void GNSServer::SetMaximumMessagePerTick(std::size_t Size)
{
	MaximumMessagePerTick = Size;
}

void GNSServer::OnServerInfoRefresh()
{
	CallRPCFromClientsEx("Global", "GNSClient", "OnReciveServerInfo", true, 0, ServerInfo);
}

void GNSServer::OnPlayerConnecting(HSteamNetConnection ID)
{
	PrintToConsole("OnPlayerConnecting");
	CallRPCFromClientEx(ID, "Global", "GNSClient", "ClientValidation", true);
	CallRPCFromClientEx(ID, "Global", "GNSClient", "OnConnecting", true, ServerInfo);
}

void GNSServer::OnPlayerConnected(HSteamNetConnection ID)
{
	ServerInfo.ConnectedPlayerCount++;

	auto Player = Instance->CreatePlayer(Instance->GetPlayerCount() == 0 ? eNetworkRole::Host : eNetworkRole::SimulatedProxy);
	if (!Player)
	{
		throw std::runtime_error("Failed to create player!");
	}

	sServerInfo::sConnectedPlayerInfo Info = { ID, false, 0, Player->GetNetworkRole(), Player->GetClassNetworkAddress(), GetNextPlayerIndex(), Player->GetPlayerName() };
	ServerInfo.ConnectedPlayerInfos.push_back(Info);
	SetDebugClientNick(ID, Player->GetPlayerName().c_str());

	std::sort(ServerInfo.ConnectedPlayerInfos.begin(), ServerInfo.ConnectedPlayerInfos.end(), [](const sServerInfo::sConnectedPlayerInfo& a, const sServerInfo::sConnectedPlayerInfo& b) {
		return a.PlayerIndex < b.PlayerIndex;
		});

	OnPlayerConnectedToServer(Player->GetPlayerName(), Player->GetClassNetworkAddress());

	CallRPCFromClientEx(ID, "Global", "GNSClient", "OnConnected", true, Info, ServerInfo);
	CallRPCFromClientsEx("Global", "GNSClient", "OnNewPlayerConnected", true, ID, Info, ServerInfo);
}

void GNSServer::OnPlayerDisconnected(HSteamNetConnection ID)
{
	if (ServerInfo.ConnectedPlayerCount == 0)
		return;

	if (!IsPlayerExist(ID))
		return;

	auto PlayerIndex = GetPlayerIndexFromID(ID);
	if (PlayerIndex == 0)
		return;

	bool Exist = Instance->IsPlayerIndexExist(GetPlayerIndexFromID(ID));

	OnPlayerConnectedToServer(Instance->GetPlayer(GetPlayerIndexFromID(ID))->GetPlayerName(), Instance->GetPlayer(GetPlayerIndexFromID(ID))->GetClassNetworkAddress());

	Instance->RemovePlayer(GetPlayerIndexFromID(ID));

	auto PlayerID = GetPlayerIndexFromID(ID);
	PrintToConsole("Disconnected Player ID :" + std::to_string(PlayerID) + (Exist ? " : Player Exist : TRUE" : " : Player Exist : FALSE"));

	CallRPCFromClientsEx("Global", "GNSClient", "OnPlayerDisconnected", true, 0, GetPlayerInfo(ID), ServerInfo);

	ServerInfo.ConnectedPlayerCount--;
	ServerInfo.ConnectedPlayerInfos.erase(std::remove_if(ServerInfo.ConnectedPlayerInfos.begin(), ServerInfo.ConnectedPlayerInfos.end(), [&](sServerInfo::sConnectedPlayerInfo& Info)
		{
			return Info.ID == ID;
		}), ServerInfo.ConnectedPlayerInfos.end());

	std::sort(ServerInfo.ConnectedPlayerInfos.begin(), ServerInfo.ConnectedPlayerInfos.end(), [](const sServerInfo::sConnectedPlayerInfo& a, const sServerInfo::sConnectedPlayerInfo& b) {
		return a.PlayerIndex < b.PlayerIndex;
		});

	for (auto& Info : ServerInfo.ConnectedPlayerInfos)
	{
		auto Player = Instance->GetPlayer(Info.PlayerIndex);
		if (!Player)
		{
			throw std::runtime_error("OnPlayerDisconnected::PlayerNotFound");
		}
		if (Info.NetworkAddress != Player->GetClassNetworkAddress())
		{
			RemoteProcedureCallManager::Get().ChangeBase(Info.NetworkAddress, Player->GetClassNetworkAddress());
			Info.NetworkAddress = Player->GetClassNetworkAddress();
		}
	}
}

void GNSServer::OnPlayerChangeName(HSteamNetConnection ID, std::string Name)
{
	for (auto& Info : ServerInfo.ConnectedPlayerInfos)
	{
		if (Info.ID == ID)
		{
			Info.PlayerName = Name;
			CallRPCFromClientsEx("Global", "GNSClient", "OnPlayerNameChanged", true, 0, ServerInfo);
			break;
		}
	}
}

void GNSServer::ValidateClient(HSteamNetConnection ID, std::string Data)
{
	std::string SomeEncryptedCode = "Some Encrypted Code";
	sArchive A(Data);
	std::string pData;
	A >> pData;

	for (auto& Info : ServerInfo.ConnectedPlayerInfos)
	{
		if (ID == Info.ID)
		{
			if (pData == SomeEncryptedCode)
			{
				Info.bIsValid = true;
				CallRPCFromClientsEx("Global", "GNSClient", "OnNewPlayerConnected", true, ID, ServerInfo);
				PrintToConsole("Client Validated");
			}
			else
			{
				PrintToConsole("Failed to Verify Client!");
				KickClient(ID);
			}
			break;
		}
	}
}

void GNSServer::OnClientSuccessfullyConnected(HSteamNetConnection ID, sClientInfo ClientInfo)
{
	PrintToConsole("OnClientSuccessfullyConnected");

	if (!IsPlayerNameUnique(ID, ClientInfo.PlayerName))
	{
		DirectCallToClientEx(ID, "OnNameChangedFromServer", true, sArchive(Instance->GetPlayer(GetPlayerIndexFromID(ID))->GetPlayerName()));
	}
	else
	{
		for (auto& Info : ServerInfo.ConnectedPlayerInfos)
		{
			if (ID == Info.ID)
			{
				Info.PlayerName = ClientInfo.PlayerName;
				Instance->GetPlayer(GetPlayerIndexFromID(ID))->SetPlayerName(Info.PlayerName);
				break;
			}
		}
	}

	auto Level = Instance->GetActiveLevel();
	auto Players = Instance->GetPlayers();

	Instance->GetPlayer(GetPlayerIndexFromID(ID))->SpawnPlayerFocusedActor();

	for (const auto& Player : Players)
	{
		//Player->SpawnPlayerFocusedActor();
		if (Instance->GetPlayer(GetPlayerIndexFromID(ID)) == Player)
			continue;
		auto Spawn = Level->GetSpawnNode("PlayerSpawn", Player->GetPlayerIndex());
		CallRPC(Player->GetClassNetworkAddress(), "Player", "SpawnPlayerFocusedActor_Client", sArchive(Spawn.Location, Spawn.LayerIndex), true);
		Player->GetPlayerFocusedActor()->SetLocation(Player->GetPlayerFocusedActor()->GetLocation());
	}
}

void GNSServer::PingClient(HSteamNetConnection clientID)
{
	DirectCallToClientEx(clientID, "PingFromServer", false, Engine::GetUTCTimeNow().GetCurrentDayTimeAsDouble());
}

void GNSServer::PingFromClient(HSteamNetConnection clientID, std::uint64_t TimeMS)
{
	DirectCallToClientEx(clientID, "PingFromServer", false, TimeMS);

	std::uint64_t TotalMS = Engine::GetUTCTimeNow().GetTotalMillisecond();
	std::uint64_t Ping = TotalMS - TimeMS;

	for (auto& Info : ServerInfo.ConnectedPlayerInfos)
	{
		if (Info.ID == clientID)
		{
			Info.Ping = Ping;
			break;
		}
	}

	//PrintToConsole("Ping req from Client(" + std::to_string(clientID) + ") : " + std::to_string(Ping));
}

bool GNSServer::IsPlayerExist(std::string Name) const
{
	for (const auto& Info : ServerInfo.ConnectedPlayerInfos)
	{
		if (Info.PlayerName == Name)
			return true;
	}
	return false;
}

bool GNSServer::IsPlayerExist(HSteamNetConnection ID) const
{
	for (const auto& Info : ServerInfo.ConnectedPlayerInfos)
	{
		if (Info.ID == ID)
			return true;
	}
	return false;
}

sServerInfo::sConnectedPlayerInfo GNSServer::GetPlayerInfo(HSteamNetConnection ID) const
{
	for (const auto& Info : ServerInfo.ConnectedPlayerInfos)
	{
		if (Info.ID == ID)
			return Info;
	}
	return sServerInfo::sConnectedPlayerInfo();
}

HSteamNetConnection GNSServer::GetPlayerIDFromAddress(std::string NetworkAddress) const
{
	for (const auto& Info : ServerInfo.ConnectedPlayerInfos)
	{
		if (Info.NetworkAddress == NetworkAddress)
			return Info.ID;
	}
	return HSteamNetConnection();
}

HSteamNetConnection GNSServer::GetPlayerIDFromName(std::string Name) const
{
	for (const auto& Info : ServerInfo.ConnectedPlayerInfos)
	{
		if (Info.PlayerName == Name)
			return Info.ID;
	}
	return HSteamNetConnection();
}

std::uint32_t GNSServer::GetPlayerIndexFromName(std::string Name) const
{
	for (const auto& Info : ServerInfo.ConnectedPlayerInfos)
	{
		if (Info.PlayerName == Name)
			return Info.PlayerIndex;
	}
	return HSteamNetConnection();
}

HSteamNetConnection GNSServer::GetPlayerIDFromIndex(std::uint32_t Index) const
{
	for (const auto& Info : ServerInfo.ConnectedPlayerInfos)
	{
		if (Info.PlayerIndex == Index)
			return Info.ID;
	}
	return HSteamNetConnection();
}

std::uint32_t GNSServer::GetPlayerIndexFromID(HSteamNetConnection ID) const
{
	for (const auto& Info : ServerInfo.ConnectedPlayerInfos)
	{
		if (Info.ID == ID)
			return Info.PlayerIndex;
	}
	return std::uint32_t();
}

std::uint32_t GNSServer::GetNextPlayerIndex() const
{
	if (ServerInfo.ConnectedPlayerInfos.size() == 0)
		return 0;
	std::uint32_t PlayerIndex = 0;
	for (const auto& Info : ServerInfo.ConnectedPlayerInfos)
	{
		if (PlayerIndex < Info.PlayerIndex)
		{
			return PlayerIndex;
		}
		else
		{
			PlayerIndex++;
		}
	}
	return PlayerIndex;
}

bool GNSServer::IsPlayerNameUnique(HSteamNetConnection ID, std::string PlayerName) const
{
	for (auto& Info : ServerInfo.ConnectedPlayerInfos)
	{
		if (ID == Info.ID)
		{
			if (PlayerName == Info.PlayerName)
			{
				continue;
			}
		}
		else if (PlayerName == Info.PlayerName)
		{
			return false;
		}
	}
	return true;
}

void GNSServer::PrintToConsole(std::string Message)
{
	std::cout << "Server : " << Message << std::endl;
}

void GNSServer::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo)
{
	// What's the state of the connection?
	switch (pInfo->m_info.m_eState)
	{
	case k_ESteamNetworkingConnectionState_None:
		break;

	case k_ESteamNetworkingConnectionState_ClosedByPeer:
	case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
	{
		PrintToConsole(std::to_string(pInfo->m_hConn));

		char temp[1024];

		if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ClosedByPeer)
			PrintToConsole("k_ESteamNetworkingConnectionState_ClosedByPeer");
		if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
			PrintToConsole("k_ESteamNetworkingConnectionState_ProblemDetectedLocally");

		// Ignore if they were not previously connected.  (If they disconnected
		// before we accepted the connection.)
		if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connected)
		{
			// Locate the client.  Note that it should have been found, because this
			// is the only codepath where we remove clients (except on shutdown),
			// and connection change callbacks are dispatched in queue order.
			//auto itClient = m_mapClients.find(pInfo->m_hConn);
			//assert(itClient != m_mapClients.end());

			assert(IsPlayerExist(pInfo->m_hConn));

			sServerInfo::sConnectedPlayerInfo PlayerInfo;
			for (const auto& Info : ServerInfo.ConnectedPlayerInfos)
			{
				if (Info.ID == pInfo->m_hConn)
				{
					PlayerInfo = Info;
					break;
				}
			}

			// Select appropriate log messages
			const char* pszDebugLogAction;
			if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
			{
				pszDebugLogAction = "problem detected locally";
				sprintf_s(temp, "Alas, %s hath fallen into shadow.  (%s)", PlayerInfo.PlayerName.c_str(), pInfo->m_info.m_szEndDebug);
			}
			else
			{
				// Note that here we could check the reason code to see if
				// it was a "usual" connection or an "unusual" one.
				pszDebugLogAction = "closed by peer";
				sprintf_s(temp, "%s hath departed", PlayerInfo.PlayerName.c_str());
			}

			// Spew something to our own log.  Note that because we put their nick
			// as the connection description, it will show up, along with their
			// transport-specific data (e.g. their IP address)
			PrintToConsole("Connection "
				+ std::string(pInfo->m_info.m_szConnectionDescription) + " "
				+ std::string(pszDebugLogAction)
				+ " reason : "
				+ std::to_string(pInfo->m_info.m_eEndReason) + " "
				+ std::string(pInfo->m_info.m_szEndDebug)
			);

			//m_mapClients.erase(itClient);

			// Send a message so everybody else knows what happened
			//SendStringToAllClients(temp);
		}
		else
		{
			assert(pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting);
		}

		OnPlayerDisconnected(pInfo->m_hConn);

		// Clean up the connection.  This is important!
		// The connection is "closed" in the network sense, but
		// it has not been destroyed.  We must close it on our end, too
		// to finish up.  The reason information do not matter in this case,
		// and we cannot linger because it's already closed on the other end,
		// so we just pass 0's.
		m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
		break;
	}

	case k_ESteamNetworkingConnectionState_Connecting:
	{
		if (ServerInfo.ConnectedPlayerCount >= ServerInfo.MaximumConnectedPlayerSize)
		{
			PrintToConsole("Can't accept connection. Server is full.");
			m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
			return;
		}

		if (std::find(BannedIPList.begin(), BannedIPList.end(), pInfo->m_info.m_addrRemote.GetIPv4()) != BannedIPList.end())
		{
			m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
			return;
		}

		// This must be a new connection
		//assert(m_mapClients.find(pInfo->m_hConn) == m_mapClients.end());
		assert(!IsPlayerExist(pInfo->m_hConn));

		PrintToConsole("Connection request from " + std::string(pInfo->m_info.m_szConnectionDescription));

		// A client is attempting to connect
		// Try to accept the connection.
		if (m_pInterface->AcceptConnection(pInfo->m_hConn) != k_EResultOK)
		{
			// This could fail.  If the remote host tried to connect, but then
			// disconnected, the connection may already be half closed.  Just
			// destroy whatever we have on our side.
			m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
			PrintToConsole("Can't accept connection.  (It was already closed?)");
			break;
		}

		// Assign the poll group
		if (!m_pInterface->SetConnectionPollGroup(pInfo->m_hConn, m_hPollGroup))
		{
			m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
			PrintToConsole("Failed to set poll group?");
			break;
		}
		OnPlayerConnecting(pInfo->m_hConn);
		break;
	}

	case k_ESteamNetworkingConnectionState_Connected:
	{
		// We will get a callback immediately after accepting the connection.
		// Since we are the server, we can ignore this, it's not news to us.
		PrintToConsole("Player Connected::" + std::to_string(pInfo->m_hConn));
		OnPlayerConnected(pInfo->m_hConn);

		break;
	}
	default:
		// Silences -Wswitch
		break;
	}
}

void GNSServer::PollConnectionStateChanges()
{
	m_pInterface->RunCallbacks();
}

void GNSServer::SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo)
{
	s_pServerCallbackInstance->OnSteamNetConnectionStatusChanged(pInfo);
}

GNSClient::GNSClient()
	: Super()
	, m_pInterface(nullptr)
	, m_hConnection(0)
	, Instance(nullptr)
	, addrServer(SteamNetworkingIPAddr())
	, MaximumMessagePerTick(64)
	, bIsConnected(false)
	, Latency(0)
	, bIsValidationCalled(false)
{
	Time = Engine::GetUTCTimeNow().GetTotalMillisecond();

	s_pClientCallbackInstance = this;
	if (!bIsInitialized)
		InitSteamDatagramConnectionSockets();

	Utils = SteamNetworkingUtils_Lib();

	RegisterRPCfn("Global", "GNSClient", "StringFromServer", eRPCType::Client, true, false, std::bind(&GNSClient::StringFromServer, this, std::placeholders::_1), std::string);
	RegisterRPCfn("Global", "GNSClient", "OnServerLevelChanged", eRPCType::Client, true, false, std::bind(&GNSClient::OnServerLevelChanged, this, std::placeholders::_1), std::string);
	RegisterRPCfn("Global", "GNSClient", "OnReciveServerInfo", eRPCType::Client, true, false, std::bind(&GNSClient::OnReciveServerInfo, this, std::placeholders::_1), sServerInfo);
	RegisterRPCfn("Global", "GNSClient", "OnConnected", eRPCType::Client, true, false, std::bind(&GNSClient::OnConnected, this, std::placeholders::_1, std::placeholders::_2), sServerInfo::sConnectedPlayerInfo, sServerInfo);
	RegisterRPCfn("Global", "GNSClient", "OnConnecting", eRPCType::Client, true, false, std::bind(&GNSClient::OnConnecting, this, std::placeholders::_1), sServerInfo);
	RegisterRPCfn("Global", "GNSClient", "OnNewPlayerConnected", eRPCType::Client, true, false, std::bind(&GNSClient::OnNewPlayerConnected, this, std::placeholders::_1, std::placeholders::_2), sServerInfo::sConnectedPlayerInfo, sServerInfo);
	RegisterRPCfn("Global", "GNSClient", "OnPlayerDisconnected", eRPCType::Client, true, false, std::bind(&GNSClient::OnPlayerDisconnected, this, std::placeholders::_1, std::placeholders::_2), sServerInfo::sConnectedPlayerInfo, sServerInfo);
	RegisterRPCfn("Global", "GNSClient", "OnNameChanged", eRPCType::Client, true, false, std::bind(&GNSClient::OnNameChanged, this));
	RegisterRPCfn("Global", "GNSClient", "OnNameChangedFromServer", eRPCType::Client, true, false, std::bind(&GNSClient::OnNameChangedFromServer, this, std::placeholders::_1), std::string);
	RegisterRPCfn("Global", "GNSClient", "OnPlayerNameChanged", eRPCType::Client, true, false, std::bind(&GNSClient::OnPlayerNameChanged, this, std::placeholders::_1), sServerInfo);
	RegisterRPCfn("Global", "GNSClient", "ClientValidation", eRPCType::Client, true, false, std::bind(&GNSClient::ClientValidation, this));
	RegisterRPCfn("Global", "GNSClient", "PingServer", eRPCType::Client, true, false, std::bind(&GNSClient::PingServer, this));
	RegisterRPCfn("Global", "GNSClient", "PingFromServer", eRPCType::Client, true, false, std::bind(&GNSClient::PingFromServer, this, std::placeholders::_1), std::uint64_t);
}

GNSClient::~GNSClient()
{
	Disconnect();

	Network::UnregisterRPC("Global", "GNSClient");

	Utils = nullptr;
	Instance = nullptr;

	m_pInterface = nullptr;

	//if (bIsInitialized)
		ShutdownSteamDatagramConnectionSockets();

	s_pClientCallbackInstance = nullptr;
}

void GNSClient::Tick(const double DeltaTime)
{
	if (bIsConnected)
	{
		auto MS = Engine::GetUTCTimeNow().GetTotalMillisecond();
		if (bIsValidationCalled && ((MS - Time) >= 1000))
		{
			PingServer();
			Time = Engine::GetUTCTimeNow().GetTotalMillisecond();
		}

		PollIncomingMessages();
		PollConnectionStateChanges();
		SendMessages();

		//CallMessageRPCFromServer("sadasd");
	}
}

void GNSClient::PollIncomingMessages()
{
	while (bIsConnected)
	{
		std::vector<ISteamNetworkingMessage*> pIncomingMsgs(MaximumMessagePerTick);
		int numMsgs = m_pInterface->ReceiveMessagesOnConnection(m_hConnection, pIncomingMsgs.data(), MaximumMessagePerTick);
		if (numMsgs == 0)
			break;

		if (numMsgs < 0)
		{
			FatalError("Error checking for messages");
		}

		for (int i = 0; i < numMsgs; i++)
		{
			ISteamNetworkingMessage* pIncomingMsg = pIncomingMsgs[i];

			sArchive pArchive;
			pArchive.SetData((std::uint8_t*)pIncomingMsg->m_pData, pIncomingMsg->m_cbSize);
			sPacket Packet;
			pArchive >> Packet;

			HandleMessages(Packet);
			
			pIncomingMsg->Release();
		}
	}
}

void GNSClient::HandleMessages(sPacket Packet)
{
	if (Packet.Type == eNetworkPacketType::RPC)
	{
		auto RPC = RemoteProcedureCallManager::Get().GetRPC(Packet.Address == PlayerNetworkAddress ? ClientInfo.NetworkAddress : Packet.Address == ClientInfo.NetworkAddress ? PlayerNetworkAddress : Packet.Address, Packet.ClassName, Packet.FunctionName);
		if (!RPC)
		{
			PrintToConsole("Failed to Call RPC | Address : " + (Packet.Address == PlayerNetworkAddress ? ClientInfo.NetworkAddress : Packet.Address == ClientInfo.NetworkAddress ? PlayerNetworkAddress : Packet.Address) + " | ClassName : " + Packet.ClassName + " | FunctionName : " + Packet.FunctionName);
			return;
		}

		//if (!Network::IsHost() && (Packet.Address == "0" || Packet.Address == "1"))
		//	PrintToConsole("RPC Called | Address : " + (Packet.Address == PlayerNetworkAddress ? ClientInfo.NetworkAddress : Packet.Address == ClientInfo.NetworkAddress ? PlayerNetworkAddress : Packet.Address) + " | ClassName : " + Packet.ClassName + " | FunctionName : " + Packet.FunctionName);

		switch (RPC->GetType())
		{
		case eRPCType::Client:
			if (RPC->IsReqTimeStamp())
				RPC->Call(sArchive(Packet.TimeStamp, Packet.Data));
			else
				RPC->Call(sArchive(Packet.Data));
			break;
		case eRPCType::Server:
			CallRPCFromServer(ClientInfo.NetworkAddress/*Packet.Address*/, Packet.FunctionName, Packet.Data, RPC->IsReliable());
			break;
		case eRPCType::ServerAndClient:
			// Called on Server first
			if (RPC->IsReqTimeStamp())
				RPC->Call(sArchive(Packet.TimeStamp, Packet.Data));
			else
				RPC->Call(sArchive(Packet.Data));
			break;
		}
	}
	else if (Packet.Type == eNetworkPacketType::DirectCall)
	{
		auto RPC = RemoteProcedureCallManager::Get().GetRPC("Global", "GNSClient", Packet.FunctionName);
		if (!RPC)
		{
			//PrintToConsole("RPC Not Called!");
			return;
		}

		//PrintToConsole("DirectCall " + Packet.FunctionName);

		switch (RPC->GetType())
		{
		case eRPCType::Server:
		case eRPCType::ServerAndClient:
			//PrintToConsole("RPC Not Called!");
			break;
		case eRPCType::Client:
			RPC->Call(sArchive(Packet.Data));
			break;
		}
	}
	else if (Packet.Type == eNetworkPacketType::String)
	{
		StringFromServer(Packet.Data);
	}
}

bool GNSClient::Connect(sGameInstance* pInstance, std::string ip, std::uint16_t Port)
{
	std::lock_guard<std::mutex> locker(Mutex);

	if (bIsConnected)
		return false;

	if (!bIsInitialized)
		InitSteamDatagramConnectionSockets();

	Info = sServerInfo();

	Instance = pInstance;
	while (Instance->GetPlayerCount() != 0)
	{
		Instance->RemoveLastPlayer();
	}

	addrServer.Clear();

	if (ip.size() > 0)
		addrServer.ParseString(std::string(ip).c_str());
	addrServer.m_port = Port;

	// Select instance to use.  For now we'll always use the default.
	m_pInterface = SteamNetworkingSockets();

	// Start connecting
	char szAddr[SteamNetworkingIPAddr::k_cchMaxString];
	addrServer.ToString(szAddr, sizeof(szAddr), true);
	PrintToConsole("Connecting to server at " + std::string(szAddr));
	SteamNetworkingConfigValue_t opt;
	opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)GNSClient::SteamNetConnectionStatusChangedCallback);
	m_hConnection = m_pInterface->ConnectByIPAddress(addrServer, 1, &opt);
	if (m_hConnection == k_HSteamNetConnection_Invalid)
	{
		bIsConnected = false;
		FatalError("Failed to create connection");
	}

	bIsConnected = true;

	return true;
}

bool GNSClient::Disconnect()
{
	std::lock_guard<std::mutex> locker(Mutex);

	if (!bIsConnected)
		return false;

	Info = sServerInfo();

	bIsConnected = false;

	bool bIsClosed = m_pInterface->CloseConnection(m_hConnection, 0, nullptr, false);

	Instance->OpenLevel("DefaultLevel");

	OnDisconnectedFromServer();

	return bIsClosed;
}

void GNSClient::StringFromServer(std::string STR)
{
	PrintToConsole(STR);
}

void GNSClient::SetMaximumMessagePerTick(std::size_t Size)
{
	MaximumMessagePerTick = Size;
}

void GNSClient::SendBufferToServer(const void* buffer, std::size_t Size, bool reliable)
{
	if (!bIsValidationCalled)
	{
		PrintToConsole("SendBufferToServer called before validation and ignored.");
		return;
	}
	int64 pOutMessageNumber = 0;
	EResult result = m_pInterface->SendMessageToConnection(m_hConnection, buffer, Size, reliable ? k_nSteamNetworkingSend_Reliable : k_nSteamNetworkingSend_Unreliable, &pOutMessageNumber);
}

void GNSClient::SendToServer(const sArchive& Archive, bool reliable)
{
	SendBufferToServer(Archive.GetData().data(), Archive.GetSize(), reliable);
}

void GNSClient::CallRPCFromServer(std::string Address, std::string ClassName, std::string FunctionName, bool reliable, std::optional<std::string> Data)
{
	if (!bIsValidationCalled)
	{
		PrintToConsole("RPC called before validation and ignored.| Address : " + Address + " | ClassName : " + ClassName + " | FunctionName : " + FunctionName);
		return;
	}
	sPacket Packet;
	Packet.TimeStamp = Engine::GetUTCTimeNow();
	Packet.Address = Address == PlayerNetworkAddress ? ClientInfo.NetworkAddress : Address;
	Packet.ClassName = ClassName;
	Packet.FunctionName = FunctionName;
	Packet.Data = Data.has_value() ? *Data : "";
	Packet.Type = eNetworkPacketType::RPC;
	sArchive Archive;
	Archive << Packet;
	//SendStringToServer(Archive.GetDataAsString(), reliable);
	SendBufferToServer(Archive.GetData().data(), Archive.GetSize(), reliable);
}

void GNSClient::CallRPC(std::string Address, std::string ClassName, std::string Name, const sArchive& Params, std::optional<bool> reliable)
{
	CallRPCFromServer(Address, ClassName, Name, reliable.has_value() ? *reliable : RemoteProcedureCallManager::Get().GetRPC(Address, ClassName, Name)->IsReliable(), Params);
}

void GNSClient::CallMessageRPCFromServer(std::string Message, bool reliable)
{
	if (!bIsValidationCalled)
	{
		PrintToConsole("CallMessageRPCFromServer called before validation and ignored. | Message : " + Message);
		return;
	}
	sArchive DataArchive;
	//DataArchive << m_hConnection;
	DataArchive << Message;
	sPacket Packet;
	Packet.TimeStamp = Engine::GetUTCTimeNow();
	Packet.Address = "Global";
	Packet.ClassName = "GNSServer";
	Packet.FunctionName = "StringFromClient";
	Packet.Data = DataArchive.GetDataAsString(); // m_hConnection (ClientID)
	Packet.Type = eNetworkPacketType::String;
	sArchive Archive;
	Archive << Packet;
	//SendStringToServer(Archive.GetDataAsString(), reliable);
	SendBufferToServer(Archive.GetData().data(), Archive.GetSize(), reliable);
}

void GNSClient::DirectCallToServer(std::string FunctionName, bool reliable, std::optional<std::string> Data)
{
	if (!bIsValidationCalled)
	{
		PrintToConsole("DirectCallToServer called before validation and ignored. | FunctionName : " + FunctionName);
		return;
	}
	sPacket Packet;
	Packet.TimeStamp = Engine::GetUTCTimeNow();
	Packet.Address = "Global";
	Packet.ClassName = "GNSServer";
	Packet.FunctionName = FunctionName;
	Packet.Data = Data.has_value() ? *Data : "";
	Packet.Type = eNetworkPacketType::DirectCall;
	sArchive Archive;
	Archive << Packet;
	//SendStringToServer(Archive.GetDataAsString(), reliable);
	SendBufferToServer(Archive.GetData().data(), Archive.GetSize(), reliable);
}

void GNSClient::SendStringToServer(const std::string& string, bool reliable)
{
	SendBufferToServer((void*)string.data(), string.size(), reliable);
}

void GNSClient::PushMessage(void* buffer, std::size_t size, bool reliable)
{
	/*SteamNetworkingMessage_t::Release Bugged!!!*/

	/*SteamNetworkingMessage_t* Message = Utils->AllocateMessage(1);
	Message->m_conn = m_hConnection;
	Message->m_cbSize = size;
	Message->m_pData = buffer;
	Message->m_nConnUserData = 0;
	Message->m_nMessageNumber = 1;
	Message->m_nFlags = reliable ? k_nSteamNetworkingSend_Reliable : k_nSteamNetworkingSend_Unreliable;
	Messages.push_back(Message);*/
}

void GNSClient::SendMessages()
{
	//int64 pOutMessageNumberOrResult = 0;
	//m_pInterface->SendMessages(Messages.size(), Messages.data(), &pOutMessageNumberOrResult);

	/*Release Bugged!!!*/
	//for (auto& Message : Messages)
	//	Message->Release();
	Messages.clear();
}

void GNSClient::ClientValidation()
{
	PrintToConsole("ClientValidation");
	bIsValidationCalled = true;
	sPacket Packet;
	Packet.TimeStamp = Engine::GetUTCTimeNow();
	Packet.Address = "Global";
	Packet.ClassName = "GNSServer";
	Packet.FunctionName = "ValidateClient";
	Packet.Data = "Some Encrypted Code";
	Packet.Type = eNetworkPacketType::Validation;
	SendToServer(sArchive(Packet), true);
}

void GNSClient::PingServer()
{
	DirectCallToServerEx("PingFromClient", false, Engine::GetUTCTimeNow().GetTotalMillisecond());
}

void GNSClient::PingFromServer(std::uint64_t duration)
{
	Latency = Engine::GetUTCTimeNow().GetTotalMillisecond() - duration;
	//PrintToConsole("Ping : " + std::to_string(Ping));
}

void GNSClient::OnReciveServerInfo(sServerInfo pInfo)
{
	Info = pInfo;
}

void GNSClient::OnConnecting(sServerInfo pInfo)
{
	PrintToConsole("OnConnecting");
	Info = pInfo;
}

void GNSClient::OnConnected(sServerInfo::sConnectedPlayerInfo ID, sServerInfo inInfo)
{
	PrintToConsole("OnConnected");
	ClientInfo = ID;
	PlayerNetworkAddress = "";
	if (!Network::IsHost() && Instance->GetPlayerCount() == 0)
	{
		auto Player = Instance->CreatePlayer(eNetworkRole::Client);
		Player->DespawnPlayerFocusedActor();
		PlayerNetworkAddress = Player->GetClassNetworkAddress();
	}
	else
	{
		PlayerNetworkAddress = Instance->GetPlayer(0)->GetClassNetworkAddress();
	}
	Info = inInfo;
	if (!Network::IsHost())
	{
		Instance->OpenLevel(Info.ServerName);
		//Instance->GetPlayer()->SpawnPlayerFocusedActor();
	}
	OnConnectedToServer();

	if (!Network::IsHost())
	{
		for (const auto& pInfo : Info.ConnectedPlayerInfos)
		{
			if (pInfo == ClientInfo)
			{
				continue;
			}

			OnPlayerConnectedToServer(pInfo.PlayerName, pInfo.NetworkAddress == PlayerNetworkAddress ? ClientInfo.NetworkAddress : pInfo.NetworkAddress);
		}
	}

	sClientInfo pClientInfo;
	pClientInfo.PlayerName = Instance->GetPlayer()->GetPlayerName();
	DirectCallToServerEx("OnClientSuccessfullyConnected", true, pClientInfo);
}

void GNSClient::OnNewPlayerConnected(sServerInfo::sConnectedPlayerInfo ID, sServerInfo pInfo)
{
	if (Network::IsHost())
		return;

	Info = pInfo;
	if (ID != ClientInfo)
		OnPlayerConnectedToServer(ID.PlayerName, ID.NetworkAddress == PlayerNetworkAddress ? ClientInfo.NetworkAddress : ID.NetworkAddress);
	//OnPlayerConnectedToServer();
}

void GNSClient::OnPlayerDisconnected(sServerInfo::sConnectedPlayerInfo ID, sServerInfo pInfo)
{
	if (Network::IsHost())
		return;

	Info = pInfo;
	if (ID != ClientInfo)
		OnPlayerDisconnectedFromServer(ID.PlayerName, ID.NetworkAddress == PlayerNetworkAddress ? ClientInfo.NetworkAddress : ID.NetworkAddress);
	//OnPlayerDisconnectedFromServer();
}

bool GNSClient::OnServerLevelChanged(std::string Level)
{
	Instance->OpenLevel(Level);
	return true;
}

void GNSClient::OnPlayerNameChanged(sServerInfo pInfo)
{
	Info = pInfo;
}

void GNSClient::OnNameChanged()
{
	DirectCallToServerEx("OnPlayerChangeName", true, Instance->GetPlayer()->GetPlayerName());
}

void GNSClient::OnNameChangedFromServer(std::string NewName)
{
	PrintToConsole("OnNameChangedFromServer : " + NewName);
	Instance->GetPlayer()->SetPlayerName(NewName);
}

void GNSClient::PrintToConsole(std::string Message)
{
	std::cout << "Client : " << Message << std::endl;
}

void GNSClient::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo)
{
	assert(pInfo->m_hConn == m_hConnection || m_hConnection == k_HSteamNetConnection_Invalid);

	// What's the state of the connection?
	switch (pInfo->m_info.m_eState)
	{
	case k_ESteamNetworkingConnectionState_Linger:
		PrintToConsole("k_ESteamNetworkingConnectionState_Linger");
		break;
	case k_ESteamNetworkingConnectionState_Dead:
		PrintToConsole("k_ESteamNetworkingConnectionState_Dead");
		break;
	case k_ESteamNetworkingConnectionState_None:
		// NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
		break;

	case k_ESteamNetworkingConnectionState_ClosedByPeer:
	case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
	{
		bIsConnected = false;

		// Print an appropriate message
		if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting)
		{
			// Note: we could distinguish between a timeout, a rejected connection,
			// or some other transport problem.
			PrintToConsole("Failed to connect to disconnect from host. " + std::string(pInfo->m_info.m_szEndDebug));
		}
		else if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
		{
			PrintToConsole("Lost connection. " + std::string(pInfo->m_info.m_szEndDebug));
		}
		else
		{
			// NOTE: We could check the reason code for a normal disconnection
			PrintToConsole("The connection was lost for an unknown reason. " + std::string(pInfo->m_info.m_szEndDebug));
		}

		// Clean up the connection.  This is important!
		// The connection is "closed" in the network sense, but
		// it has not been destroyed.  We must close it on our end, too
		// to finish up.  The reason information do not matter in this case,
		// and we cannot linger because it's already closed on the other end,
		// so we just pass 0's.
		bool IsClosed = m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
		//if (IsClosed)
		//	PrintToConsole("Failed To Close");
		m_hConnection = k_HSteamNetConnection_Invalid;

		if (Instance->GetPlayerCount() == 0)
		{
			auto Player = Instance->CreatePlayer();
			Player->DespawnPlayerFocusedActor();
		}
		Instance->OpenLevel("DefaultLevel");
		Instance->GetPlayer()->SpawnPlayerFocusedActor();

		break;
	}

	case k_ESteamNetworkingConnectionState_Connecting:
	{
		// We will get this callback when we start connecting.
		// We can ignore this.

		PrintToConsole("k_ESteamNetworkingConnectionState_Connecting");
		ClientValidation();
		Instance->DisableSplitScreen();
		while (Instance->GetPlayerCount() != 0)
		{
			Instance->RemoveLastPlayer();
		}
	}
	break;

	case k_ESteamNetworkingConnectionState_Connected:
	{
		PrintToConsole("Connected to server OK");
	}
	break;

	default:
		// Silences -Wswitch
		break;
	}
}

void GNSClient::PollConnectionStateChanges()
{
	m_pInterface->RunCallbacks();
}

void GNSClient::SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo)
{
	s_pClientCallbackInstance->OnSteamNetConnectionStatusChanged(pInfo);
}

#endif

#if Enable_Winsock

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

WSServer::WSServer()
	: MaximumMessagePerTick(64)
	, ClientCounter(0)
	, bIsServerRunning(false)
	, Instance(nullptr)
{
	ListenSocket = INVALID_SOCKET;

	// Initialize Winsock
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	ServerInfo.MaximumConnectedPlayerSize = 8;

	RegisterRPCfn("Global", "WSServer", "StringFromClient", eRPCType::Server, true, false, std::bind(&WSServer::StringFromClient, this, std::placeholders::_1, std::placeholders::_2), std::uint32_t, std::string);
	RegisterRPCfn("Global", "WSServer", "OnClientSuccessfullyConnected", eRPCType::Server, true, false, std::bind(&WSServer::OnClientSuccessfullyConnected, this, std::placeholders::_1, std::placeholders::_2), std::uint32_t, sClientInfo);
	RegisterRPCfn("Global", "WSServer", "ClientValidation", eRPCType::Server, true, false, std::bind(&WSServer::ValidateClient, this, std::placeholders::_1, std::placeholders::_2), std::uint32_t, std::string);
	RegisterRPCfn("Global", "WSServer", "PingFromClient", eRPCType::Server, true, false, std::bind(&WSServer::PingFromClient, this, std::placeholders::_1, std::placeholders::_2), std::uint32_t, std::uint64_t);
	RegisterRPCfn("Global", "WSServer", "PingClient", eRPCType::Server, true, false, std::bind(&WSServer::PingClient, this, std::placeholders::_1), std::uint32_t);
}

WSServer::~WSServer()
{
	DestroySession();

	Network::UnregisterRPC("Global", "WSServer");

	Instance = nullptr;

	ServerInfo.ConnectedPlayerInfos.clear();

	KickList.clear();
	BannedIPList.clear();

	while (!Packets.empty())
	{
		Packets.pop();
	}

	// cleanup
	WSACleanup();
}

void WSServer::Tick(const double DeltaTime)
{
	if (bIsServerRunning)
	{
		//auto MS = Engine::GetUTCTimeNow().GetTotalMillisecond();

		//if ((MS - gTime) > 70)
		{
			PollIncomingMessages();
			SendMessages();
			//gTime = Engine::GetUTCTimeNow().GetTotalMillisecond();
		}
	}
}

void WSServer::PollIncomingMessages()
{
	std::size_t Counter = 0;
	while (bIsServerRunning)
	{
		std::lock_guard<std::mutex> locker(Mutex);
		for (auto& Client : Clients)
		{
			if (!Client.second.IsValid)
			{
				OnPlayerConnecting(Client.first);
				OnPlayerConnected(Client.first);
				Client.second.IsValid = true;
			}
		}

		if (!Packets.empty()/* && Counter < MaximumMessagePerTick*/)
		{
			sMsg Msg = Packets.front();

			auto Info = GetPlayerInfo(Msg.ID);
			if (!Info.bIsValid)
			{
				if (Msg.Packet.Type == eNetworkPacketType::Validation)
				{
					ValidateClient(Msg.ID, Msg.Packet.Data);
				}
				else
				{
					PrintToConsole("Validation Skipped! msg : " + Msg.Packet.FunctionName);
					KickClient(Msg.ID);
				}
			}
			else
			{
				HandleMessages(Msg.ID, Msg.Packet);
			}

			Packets.pop();
			Counter++;
		}
		else
		{
			break;
		}
	}
}

void WSServer::HandleMessages(std::uint32_t ID, const sPacket& Packet, std::optional<bool> reliable)
{
	if (Packet.Type == eNetworkPacketType::RPC)
	{
		auto RPC = RemoteProcedureCallManager::Get().GetRPC(Packet.Address, Packet.ClassName, Packet.FunctionName);
		if (!RPC)
		{
			PrintToConsole("Failed to Call RPC | Address : " + Packet.Address + " | ClassName : " + Packet.ClassName + " | FunctionName : " + Packet.FunctionName);
			return;
		}

		//PrintToConsole("RPC Called | Address : " + Packet.Address + " | ClassName : " + Packet.ClassName + " | FunctionName : " + Packet.FunctionName);

		switch (RPC->GetType())
		{
		case eRPCType::Client:
			CallRPCFromClient(ID, Packet.Address, Packet.ClassName, Packet.FunctionName, Packet.Data, reliable.has_value() ? *reliable : RPC->IsReliable());
			CallRPCFromClients(Packet.Address, Packet.ClassName, Packet.FunctionName, Packet.Data, reliable.has_value() ? *reliable : RPC->IsReliable(), ID);
			break;
		case eRPCType::Server:
			if (RPC->IsReqTimeStamp())
				RPC->Call(sArchive(Packet.TimeStamp, Packet.Data));
			else
				RPC->Call(sArchive(Packet.Data));
			break;
		case eRPCType::ServerAndClient:
			if (RPC->IsReqTimeStamp())
				RPC->Call(sArchive(Packet.TimeStamp, Packet.Data));
			else
				RPC->Call(sArchive(Packet.Data));
			CallRPCFromClient(ID, Packet.Address, Packet.ClassName, Packet.FunctionName, Packet.Data, reliable.has_value() ? *reliable : RPC->IsReliable());
			CallRPCFromClients(Packet.Address, Packet.ClassName, Packet.FunctionName, Packet.Data, reliable.has_value() ? *reliable : RPC->IsReliable(), ID);
			break;
		}
	}
	else if (Packet.Type == eNetworkPacketType::DirectCall)
	{
		auto RPC = RemoteProcedureCallManager::Get().GetRPC("Global", "WSServer", Packet.FunctionName);
		if (!RPC)
		{
			PrintToConsole("DirectCall::RPC Not Called!");
			return;
		}

		sArchive ParamWithID;
		ParamWithID << ID;
		ParamWithID << Packet.Data;

		switch (RPC->GetType())
		{
		case eRPCType::ServerAndClient:
		case eRPCType::Client:
			//PrintToConsole("RPC Not Called!");
			break;
		case eRPCType::Server:
			RPC->Call(ParamWithID);
			break;
		}
	}
	else if (Packet.Type == eNetworkPacketType::String)
	{
		StringFromClient(ID, Packet.Data);
	}
}

bool WSServer::CreateSession(std::string Name, sGameInstance* pInstance, std::string Level, std::uint16_t Port, std::size_t PlayerCount)
{
	std::lock_guard<std::mutex> locker(Mutex);

	if (bIsServerRunning)
		return false;

	ServerInfo.LevelName = Level;
	ServerInfo.MaximumConnectedPlayerSize = PlayerCount;

	Instance = pInstance;

	Instance->DisableSplitScreen();
	while (Instance->GetPlayerCount() != 0)
	{
		Instance->RemoveLastPlayer();
	}

	ServerInfo.ServerName = Name;

	Instance->OpenLevel(ServerInfo.LevelName);

	int iResult;
	struct addrinfo* result = NULL;
	struct addrinfo hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, std::to_string(Port).c_str(), &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for the server to listen for client connections.
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

	bIsServerRunning.store(true, std::memory_order_release);

	Engine::QueueJob([&]()
		{
			while (bIsServerRunning.load(std::memory_order_acquire))
			{
				SOCKET ClientSocket = accept(ListenSocket, NULL, NULL);
				if (ClientSocket != INVALID_SOCKET)
				{
					if (ServerInfo.ConnectedPlayerCount >= ServerInfo.MaximumConnectedPlayerSize)
					{
						closesocket(ClientSocket);
						continue;
					}
					std::lock_guard<std::mutex> locker(Mutex);

					Clients.insert({ ClientCounter, ClientSocket });
					ClientCounter++;
				}
			}
		}
	);

	Engine::QueueJob([&]()
		{
			std::vector<std::uint8_t> buffer(256 * MaximumMessagePerTick);

			int bytesReceived = 0;

			while (bIsServerRunning.load(std::memory_order_acquire))
			{
				for (const auto& Client : Clients)
				{
					// Attempt to receive data (non-blocking)
					bytesReceived = recv(Client.second.Socket, (char*)buffer.data(), (int)buffer.size(), 0);
					//bytesReceived = recvfrom(Client.second, buffer, sizeof(buffer), 0, NULL, NULL);

					if (bytesReceived == SOCKET_ERROR) {
						int error = WSAGetLastError();

						if (error == WSAEWOULDBLOCK) {
							// No data available, continue or sleep and retry
							std::this_thread::sleep_for(std::chrono::milliseconds(100));
							continue;
						}
						else {
							std::cerr << "Error in recv: " << error << std::endl;
							break;
						}
					}

					// Process received data (use bytesReceived)
					if (bytesReceived > 0)
					{
						std::lock_guard<std::mutex> locker(Mutex);
						sArchive pArchive;
						pArchive.AppendData(buffer, bytesReceived);

						int i = 1;
						while (pArchive.GetPosition() < pArchive.GetSize())
						{
							sPacket Packet;
							//pArchive.ResizeData(256 * 32);
							pArchive >> Packet;
							Packets.push(sMsg(Client.first, Packet));
							pArchive.ResetPos(i * 256);
							i++;

							if (Packet.Address == "" || Packet.FunctionName == "" || Packet.ClassName == "")
								PrintToConsole("Empty");
						}
					}
				}
			}
		}
	);

	return true;
}

bool WSServer::DestroySession()
{
	std::lock_guard<std::mutex> locker(Mutex);

	if (!bIsServerRunning)
		return false;

	// No longer need server socket
	closesocket(ListenSocket);

	PrintToConsole("Closing connections...");
	
	//m_mapClients.clear();
	ServerInfo.ConnectedPlayerInfos.clear();

	bIsServerRunning.store(false, std::memory_order_release);

	while (Instance->GetPlayerCount() != 0)
	{
		Instance->RemoveLastPlayer();
	}

	for (auto& Client : Clients)
	{
		closesocket(Client.second.Socket);
	}
	Clients.clear();

	OnSessionDestroyed();

	return true;
}

void WSServer::SendToClient(std::uint32_t clientID, const sArchive& Archive, bool reliable)
{
	SendBufferToClient(clientID, Archive.GetData().data(), Archive.GetSize(), reliable);
}

void WSServer::SendToClients(const sArchive& Archive, bool reliable, std::uint32_t excludeClientID)
{
	for (const auto& clientInfo : ServerInfo.ConnectedPlayerInfos)
	{
		if (clientInfo.ID != excludeClientID)
			SendToClient(clientInfo.ID, Archive, reliable);
	}
}

void WSServer::SendStringToAllClients(const std::string& string, bool reliable, std::uint32_t excludeClientID)
{
	SendBufferToAllClients(string.data(), string.size(), reliable, excludeClientID);
}

void WSServer::SendBufferToClient(std::uint32_t clientID, const void* buffer, std::size_t Size, bool reliable)
{
	if (!Clients.contains(clientID))
		return;

	int result = send(Clients[clientID].Socket, (char*)buffer, (int)Size, 0);

	/*sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(27020);
	inet_pton(AF_INET, "127.0.0.1", &(serverAddr.sin_addr));*/

	//int result = sendto(ListenSocket, (char*)buffer, Size, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

	if (result == SOCKET_ERROR) 
	{
		int error = WSAGetLastError();
		if (error == 10053)
		{
			if (Clients[clientID].TimeOutTest == 24)
			{
				closesocket(Clients[clientID].Socket);
				OnPlayerDisconnected(clientID);
				return;
			}
			Clients[clientID].TimeOutTest++;
			PrintToConsole("Time out counter : " + std::to_string(Clients[clientID].TimeOutTest));
			SendBufferToClient(clientID, buffer, Size, reliable);
			return;
		}
		else if (error == 10038)
		{
			PrintToConsole("The client has already been disconnected.");
		}
		else
		{
			// Handle the error based on the value of 'error'
			PrintToConsole("Error in sendto: " + std::to_string(error));
		}
	}
	else 
	{
		// 'result' contains the number of bytes sent
		Clients[clientID].TimeOutTest = 0;
	}
}

void WSServer::SendBufferToAllClients(const void* buffer, std::size_t size, bool reliable, std::uint32_t excludeClientID)
{
	for (const auto& clientInfo : ServerInfo.ConnectedPlayerInfos)
	{
		if (clientInfo.ID != excludeClientID)
			SendBufferToClient(clientInfo.ID, buffer, size, reliable);
	}
}

void WSServer::SendStringToClient(std::uint32_t clientID, const std::string& string, bool reliable)
{
	SendBufferToClient(clientID, string.data(), string.size(), reliable);
}

void WSServer::CallRPC(std::string Address, std::string ClassName, std::string Name, const sArchive& Params, std::optional<bool> reliable)
{
	HandleMessages(GetPlayerIDFromAddress(Address), sPacket(Engine::GetUTCTimeNow(), eNetworkPacketType::RPC, Address, ClassName, Name, Params), reliable);
}

void WSServer::CallRPCFromClient(std::uint32_t clientID, std::string Address, std::string ClassName, std::string FunctionName, std::optional<std::string> Data, bool reliable)
{
	sPacket Packet;
	Packet.TimeStamp = Engine::GetUTCTimeNow();
	Packet.Address = Address;
	Packet.ClassName = ClassName;
	Packet.FunctionName = FunctionName;
	Packet.Data = Data.has_value() ? *Data : "";
	Packet.Type = eNetworkPacketType::RPC;
	sArchive Archive;
	Archive.ResizeData(256);
	Archive << Packet;
	//SendStringToClient(clientID, Archive.GetDataAsString(), reliable);
	SendBufferToClient(clientID, Archive.GetData().data(), Archive.GetSize(), reliable);
}

void WSServer::CallRPCFromClients(std::string Address, std::string ClassName, std::string FunctionName, std::optional<std::string> Data, bool reliable, std::uint32_t excludeClientID)
{
	for (const auto& clientInfo : ServerInfo.ConnectedPlayerInfos)
	{
		if (clientInfo.ID != excludeClientID)
			CallRPCFromClient(clientInfo.ID, Address, ClassName, FunctionName, Data, reliable);
	}
}

void WSServer::DirectCallToClient(std::uint32_t clientID, std::string FunctionName, bool reliable, std::optional<std::string> Data)
{
	sPacket Packet;
	Packet.TimeStamp = Engine::GetUTCTimeNow();
	Packet.Address = "Global";
	Packet.ClassName = "WSClient";
	Packet.FunctionName = FunctionName;
	Packet.Data = Data.has_value() ? *Data : "";
	Packet.Type = eNetworkPacketType::DirectCall;
	sArchive Archive;
	Archive.ResizeData(256);
	Archive << Packet;
	//SendStringToClient(clientID, Archive.GetDataAsString(), reliable);
	SendBufferToClient(clientID, Archive.GetData().data(), Archive.GetSize(), reliable);
}

void WSServer::DirectCallToClients(std::string FunctionName, std::uint32_t excludeClientID, bool reliable, std::optional<std::string> Data)
{
	for (const auto& clientInfo : ServerInfo.ConnectedPlayerInfos)
	{
		if (clientInfo.ID != excludeClientID)
			DirectCallToClient(clientInfo.ID, FunctionName, reliable, Data);
	}
}

void WSServer::CallMessageRPCFromClient(std::uint32_t clientID, std::string Message, bool reliable)
{
	sPacket Packet;
	Packet.TimeStamp = Engine::GetUTCTimeNow();
	Packet.Address = "Global";
	Packet.ClassName = "WSClient";
	Packet.FunctionName = "StringFromServer\n";
	Packet.Data = Message;
	Packet.Type = eNetworkPacketType::String;
	sArchive Archive;
	Archive.ResizeData(256);
	Archive << Packet;
	//SendStringToClient(clientID, Archive.GetDataAsString(), reliable);
	SendBufferToClient(clientID, Archive.GetData().data(), Archive.GetSize(), reliable);
}

void WSServer::CallMessageRPCFromClients(std::string Message, bool reliable, std::uint32_t excludeClientID)
{
	for (const auto& clientInfo : ServerInfo.ConnectedPlayerInfos)
	{
		if (clientInfo.ID != excludeClientID)
			CallMessageRPCFromClient(clientInfo.ID, Message, reliable);
	}
}

void WSServer::PushMessageForAllClients(void* buffer, std::size_t size, bool reliable, std::uint32_t excludeClientID)
{
	for (const auto& clientInfo : ServerInfo.ConnectedPlayerInfos)
	{
		if (clientInfo.ID != excludeClientID)
			PushMessage(clientInfo.ID, buffer, size, reliable);
	}
}

void WSServer::PushMessage(std::uint32_t clientID, void* buffer, std::size_t size, bool reliable)
{

}

void WSServer::SendMessages()
{

}

void WSServer::StringFromClient(std::uint32_t ClientID, std::string STR)
{
	PrintToConsole("MessageFromClient : ID : " + std::to_string(ClientID) + " Message : " + STR);
}

void WSServer::KickClient(std::uint32_t clientID)
{
	KickList.push_back(clientID);
	std::size_t TryCounter = 0;
	bool IsClosed = false;
	OnPlayerDisconnected(clientID);
	while (!IsClosed && TryCounter < 50)
	{
		//IsClosed = m_pInterface->CloseConnection(clientID, 0, nullptr, false);
		closesocket(Clients[clientID].Socket);
		TryCounter++;
	}
	if (!IsClosed)
		PrintToConsole("Failed To Kick. Try Count : " + std::to_string(TryCounter));
	else
		PrintToConsole("Player Kicked. ID : " + std::to_string(clientID));
}

std::string WSServer::GetLevel() const
{
	return ServerInfo.LevelName;
}

bool WSServer::ChangeLevel(std::string Level)
{
	ServerInfo.LevelName = Level;
	Instance->OpenLevel(ServerInfo.LevelName);
	CallRPCFromClients("Global", "WSClient", "OnServerLevelChanged", ServerInfo.LevelName);
	return true;
}

void WSServer::SetServerName(std::string Name)
{
	ServerInfo.LevelName = Name;
}

void WSServer::SetMaximumMessagePerTick(std::size_t Size)
{
	MaximumMessagePerTick = Size;
}

void WSServer::OnServerInfoRefresh()
{
	CallRPCFromClientsEx("Global", "WSClient", "OnReciveServerInfo", true, 0, ServerInfo);
}

void WSServer::OnPlayerConnecting(std::uint32_t ID)
{
	PrintToConsole("OnPlayerConnecting");
	CallRPCFromClientEx(ID, "Global", "WSClient", "ClientValidation", true);
	CallRPCFromClientEx(ID, "Global", "WSClient", "OnConnecting", true, ServerInfo);
}

void WSServer::OnPlayerConnected(std::uint32_t ID)
{
	PrintToConsole("OnPlayerConnected");
	ServerInfo.ConnectedPlayerCount++;

	auto Player = Instance->CreatePlayer(Instance->GetPlayerCount() == 0 ? eNetworkRole::Host : eNetworkRole::SimulatedProxy);
	if (!Player)
	{
		throw std::runtime_error("Failed to create player!");
	}

	sServerInfo::sConnectedPlayerInfo Info = { ID, false, 0, Player->GetNetworkRole(), Player->GetClassNetworkAddress(), GetNextPlayerIndex(), Player->GetPlayerName() };
	ServerInfo.ConnectedPlayerInfos.push_back(Info);
	//SetDebugClientNick(ID, Player->GetPlayerName().c_str());

	std::sort(ServerInfo.ConnectedPlayerInfos.begin(), ServerInfo.ConnectedPlayerInfos.end(), [](const sServerInfo::sConnectedPlayerInfo& a, const sServerInfo::sConnectedPlayerInfo& b) {
		return a.PlayerIndex < b.PlayerIndex;
		});

	OnPlayerConnectedToServer(Player->GetPlayerName(), Player->GetClassNetworkAddress());

	CallRPCFromClientEx(ID, "Global", "WSClient", "OnConnected", true, Info, ServerInfo);
	CallRPCFromClientsEx("Global", "WSClient", "OnNewPlayerConnected", true, ID, Info, ServerInfo);
}

void WSServer::OnPlayerDisconnected(std::uint32_t ID)
{
	if (ServerInfo.ConnectedPlayerCount == 0)
		return;

	if (!IsPlayerExist(ID))
		return;

	auto PlayerIndex = GetPlayerIndexFromID(ID);
	if (PlayerIndex == 0)
		return;

	bool Exist = Instance->IsPlayerIndexExist(GetPlayerIndexFromID(ID));

	OnPlayerConnectedToServer(Instance->GetPlayer(GetPlayerIndexFromID(ID))->GetPlayerName(), Instance->GetPlayer(GetPlayerIndexFromID(ID))->GetClassNetworkAddress());

	Instance->RemovePlayer(GetPlayerIndexFromID(ID));

	auto PlayerID = GetPlayerIndexFromID(ID);
	PrintToConsole("Disconnected Player ID :" + std::to_string(PlayerID) + (Exist ? " : Player Exist : TRUE" : " : Player Exist : FALSE"));

	CallRPCFromClientsEx("Global", "WSClient", "OnPlayerDisconnected", true, 0, GetPlayerInfo(ID), ServerInfo);

	ServerInfo.ConnectedPlayerCount--;
	ServerInfo.ConnectedPlayerInfos.erase(std::remove_if(ServerInfo.ConnectedPlayerInfos.begin(), ServerInfo.ConnectedPlayerInfos.end(), [&](sServerInfo::sConnectedPlayerInfo& Info)
		{
			return Info.ID == ID;
		}), ServerInfo.ConnectedPlayerInfos.end());

	std::sort(ServerInfo.ConnectedPlayerInfos.begin(), ServerInfo.ConnectedPlayerInfos.end(), [](const sServerInfo::sConnectedPlayerInfo& a, const sServerInfo::sConnectedPlayerInfo& b) {
		return a.PlayerIndex < b.PlayerIndex;
		});

	Clients.erase(ID);

	for (auto& Info : ServerInfo.ConnectedPlayerInfos)
	{
		auto Player = Instance->GetPlayer(Info.PlayerIndex);
		if (!Player)
		{
			throw std::runtime_error("OnPlayerDisconnected::PlayerNotFound");
		}
		if (Info.NetworkAddress != Player->GetClassNetworkAddress())
		{
			RemoteProcedureCallManager::Get().ChangeBase(Info.NetworkAddress, Player->GetClassNetworkAddress());
			Info.NetworkAddress = Player->GetClassNetworkAddress();
		}
	}
}

void WSServer::OnPlayerChangeName(std::uint32_t ID, std::string Name)
{
	for (auto& Info : ServerInfo.ConnectedPlayerInfos)
	{
		if (Info.ID == ID)
		{
			Info.PlayerName = Name;
			CallRPCFromClientsEx("Global", "WSClient", "OnPlayerNameChanged", true, 0, ServerInfo);
			break;
		}
	}
}

void WSServer::ValidateClient(std::uint32_t ID, std::string Data)
{
	std::string SomeEncryptedCode = "Some Encrypted Code";
	sArchive A(Data);
	std::string pData;
	A >> pData;

	for (auto& Info : ServerInfo.ConnectedPlayerInfos)
	{
		if (ID == Info.ID)
		{
			if (pData == SomeEncryptedCode)
			{
				Info.bIsValid = true;
				CallRPCFromClientsEx("Global", "WSClient", "OnNewPlayerConnected", true, ID, ServerInfo);
				PrintToConsole("Client Validated");
			}
			else
			{
				PrintToConsole("Failed to Verify Client!");
				KickClient(ID);
			}
			break;
		}
	}
}

void WSServer::OnClientSuccessfullyConnected(std::uint32_t ID, sClientInfo ClientInfo)
{
	PrintToConsole("OnClientSuccessfullyConnected");

	if (!IsPlayerNameUnique(ID, ClientInfo.PlayerName))
	{
		DirectCallToClientEx(ID, "OnNameChangedFromServer", true, sArchive(Instance->GetPlayer(GetPlayerIndexFromID(ID))->GetPlayerName()));
	}
	else
	{
		for (auto& Info : ServerInfo.ConnectedPlayerInfos)
		{
			if (ID == Info.ID)
			{
				Info.PlayerName = ClientInfo.PlayerName;
				Instance->GetPlayer(GetPlayerIndexFromID(ID))->SetPlayerName(Info.PlayerName);
				break;
			}
		}
	}

	auto Level = Instance->GetActiveLevel();
	auto Players = Instance->GetPlayers();

	Instance->GetPlayer(GetPlayerIndexFromID(ID))->SpawnPlayerFocusedActor();

	for (const auto& Player : Players)
	{
		//Player->SpawnPlayerFocusedActor();
		if (Instance->GetPlayer(GetPlayerIndexFromID(ID)) == Player)
			continue;
		auto Spawn = Level->GetSpawnNode("PlayerSpawn", Player->GetPlayerIndex());
		CallRPC(Player->GetClassNetworkAddress(), "Player", "SpawnPlayerFocusedActor_Client", sArchive(Spawn.Location, Spawn.LayerIndex), true);
		Player->GetPlayerFocusedActor()->SetLocation(Player->GetPlayerFocusedActor()->GetLocation());
	}
}

void WSServer::PingClient(std::uint32_t clientID)
{
	DirectCallToClientEx(clientID, "PingFromServer", false, Engine::GetUTCTimeNow().GetCurrentDayTimeAsDouble());
}

void WSServer::PingFromClient(std::uint32_t clientID, std::uint64_t TimeMS)
{
	DirectCallToClientEx(clientID, "PingFromServer", false, TimeMS);

	std::uint64_t TotalMS = Engine::GetUTCTimeNow().GetTotalMillisecond();
	std::uint64_t Ping = TotalMS - TimeMS;

	for (auto& Info : ServerInfo.ConnectedPlayerInfos)
	{
		if (Info.ID == clientID)
		{
			Info.Ping = Ping;
			break;
		}
	}

	//PrintToConsole("Ping req from Client(" + std::to_string(clientID) + ") : " + std::to_string(Ping));
}

bool WSServer::IsPlayerExist(std::string Name) const
{
	for (const auto& Info : ServerInfo.ConnectedPlayerInfos)
	{
		if (Info.PlayerName == Name)
			return true;
	}
	return false;
}

bool WSServer::IsPlayerExist(std::uint32_t ID) const
{
	for (const auto& Info : ServerInfo.ConnectedPlayerInfos)
	{
		if (Info.ID == ID)
			return true;
	}
	return false;
}

sServerInfo::sConnectedPlayerInfo WSServer::GetPlayerInfo(std::uint32_t ID) const
{
	for (const auto& Info : ServerInfo.ConnectedPlayerInfos)
	{
		if (Info.ID == ID)
			return Info;
	}
	return sServerInfo::sConnectedPlayerInfo();
}

std::uint32_t WSServer::GetPlayerIDFromAddress(std::string NetworkAddress) const
{
	for (const auto& Info : ServerInfo.ConnectedPlayerInfos)
	{
		if (Info.NetworkAddress == NetworkAddress)
			return Info.ID;
	}
	return std::uint32_t();
}

std::uint32_t WSServer::GetPlayerIDFromName(std::string Name) const
{
	for (const auto& Info : ServerInfo.ConnectedPlayerInfos)
	{
		if (Info.PlayerName == Name)
			return Info.ID;
	}
	return std::uint32_t();
}

std::uint32_t WSServer::GetPlayerIndexFromName(std::string Name) const
{
	for (const auto& Info : ServerInfo.ConnectedPlayerInfos)
	{
		if (Info.PlayerName == Name)
			return Info.PlayerIndex;
	}
	return std::uint32_t();
}

std::uint32_t WSServer::GetPlayerIDFromIndex(std::uint32_t Index) const
{
	for (const auto& Info : ServerInfo.ConnectedPlayerInfos)
	{
		if (Info.PlayerIndex == Index)
			return Info.ID;
	}
	return std::uint32_t();
}

std::uint32_t WSServer::GetPlayerIndexFromID(std::uint32_t ID) const
{
	for (const auto& Info : ServerInfo.ConnectedPlayerInfos)
	{
		if (Info.ID == ID)
			return Info.PlayerIndex;
	}
	return std::uint32_t();
}

std::uint32_t WSServer::GetNextPlayerIndex() const
{
	if (ServerInfo.ConnectedPlayerInfos.size() == 0)
		return 0;
	std::uint32_t PlayerIndex = 0;
	for (const auto& Info : ServerInfo.ConnectedPlayerInfos)
	{
		if (PlayerIndex < Info.PlayerIndex)
		{
			return PlayerIndex;
		}
		else
		{
			PlayerIndex++;
		}
	}
	return PlayerIndex;
}

bool WSServer::IsPlayerNameUnique(std::uint32_t ID, std::string PlayerName) const
{
	for (auto& Info : ServerInfo.ConnectedPlayerInfos)
	{
		if (ID == Info.ID)
		{
			if (PlayerName == Info.PlayerName)
			{
				continue;
			}
		}
		else if (PlayerName == Info.PlayerName)
		{
			return false;
		}
	}
	return true;
}

void WSServer::PrintToConsole(std::string Message)
{
	std::cout << "Server : " << Message << std::endl;
}

WSClient::WSClient()
	: MaximumMessagePerTick(64)
	, Instance(nullptr)
	, bIsConnected(false)
	, Latency(0)
	, Time(0)
	, bIsValidationCalled(false)
{
	ConnectSocket = INVALID_SOCKET;

	// Initialize Winsock
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	RegisterRPCfn("Global", "WSClient", "StringFromServer", eRPCType::Client, true, false, std::bind(&WSClient::StringFromServer, this, std::placeholders::_1), std::string);
	RegisterRPCfn("Global", "WSClient", "OnServerLevelChanged", eRPCType::Client, true, false, std::bind(&WSClient::OnServerLevelChanged, this, std::placeholders::_1), std::string);
	RegisterRPCfn("Global", "WSClient", "OnReciveServerInfo", eRPCType::Client, true, false, std::bind(&WSClient::OnReciveServerInfo, this, std::placeholders::_1), sServerInfo);
	RegisterRPCfn("Global", "WSClient", "OnConnected", eRPCType::Client, true, false, std::bind(&WSClient::OnConnected, this, std::placeholders::_1, std::placeholders::_2), sServerInfo::sConnectedPlayerInfo, sServerInfo);
	RegisterRPCfn("Global", "WSClient", "OnConnecting", eRPCType::Client, true, false, std::bind(&WSClient::OnConnecting, this, std::placeholders::_1), sServerInfo);
	RegisterRPCfn("Global", "WSClient", "OnNewPlayerConnected", eRPCType::Client, true, false, std::bind(&WSClient::OnNewPlayerConnected, this, std::placeholders::_1, std::placeholders::_2), sServerInfo::sConnectedPlayerInfo, sServerInfo);
	RegisterRPCfn("Global", "WSClient", "OnPlayerDisconnected", eRPCType::Client, true, false, std::bind(&WSClient::OnPlayerDisconnected, this, std::placeholders::_1, std::placeholders::_2), sServerInfo::sConnectedPlayerInfo, sServerInfo);
	RegisterRPCfn("Global", "WSClient", "OnNameChanged", eRPCType::Client, true, false, std::bind(&WSClient::OnNameChanged, this));
	RegisterRPCfn("Global", "WSClient", "OnNameChangedFromServer", eRPCType::Client, true, false, std::bind(&WSClient::OnNameChangedFromServer, this, std::placeholders::_1), std::string);
	RegisterRPCfn("Global", "WSClient", "OnPlayerNameChanged", eRPCType::Client, true, false, std::bind(&WSClient::OnPlayerNameChanged, this, std::placeholders::_1), sServerInfo);
	RegisterRPCfn("Global", "WSClient", "ClientValidation", eRPCType::Client, true, false, std::bind(&WSClient::ClientValidation, this));
	RegisterRPCfn("Global", "WSClient", "PingServer", eRPCType::Client, true, false, std::bind(&WSClient::PingServer, this));
	RegisterRPCfn("Global", "WSClient", "PingFromServer", eRPCType::Client, true, false, std::bind(&WSClient::PingFromServer, this, std::placeholders::_1), std::uint64_t);
}

WSClient::~WSClient()
{
	Network::UnregisterRPC("Global", "WSClient");

	Instance = nullptr;

	while (!Packets.empty())
	{
		Packets.pop();
	}

	Disconnect();
	WSACleanup();
}

void WSClient::Tick(const double DeltaTime)
{
	if (bIsConnected)
	{
		auto MS = Engine::GetUTCTimeNow().GetTotalMillisecond();
		if (bIsValidationCalled && ((MS - Time) >= 60))
		{
			PingServer();
			Time = Engine::GetUTCTimeNow().GetTotalMillisecond();
		}

		PollIncomingMessages();
		SendMessages();

		//CallMessageRPCFromServer("sadasd");
	}
}

void WSClient::PollIncomingMessages()
{
	std::size_t Counter = 0;
	while (bIsConnected)
	{
		std::lock_guard<std::mutex> locker(Mutex);
		if (!Packets.empty() /*&& Counter < MaximumMessagePerTick*/)
		{
			HandleMessages(Packets.front());
			Packets.pop();
			Counter++;
		}
		else
		{
			break;
		}
	}
}

void WSClient::HandleMessages(const sPacket& Packet)
{
	if (Packet.Type == eNetworkPacketType::RPC)
	{
		auto RPC = RemoteProcedureCallManager::Get().GetRPC(Packet.Address == PlayerNetworkAddress ? ClientInfo.NetworkAddress : Packet.Address == ClientInfo.NetworkAddress ? PlayerNetworkAddress : Packet.Address, Packet.ClassName, Packet.FunctionName);
		if (!RPC)
		{
			PrintToConsole("Failed to Call RPC | Address : " + (Packet.Address == PlayerNetworkAddress ? ClientInfo.NetworkAddress : Packet.Address == ClientInfo.NetworkAddress ? PlayerNetworkAddress : Packet.Address) + " | ClassName : " + Packet.ClassName + " | FunctionName : " + Packet.FunctionName);
			return;
		}

		//if (!Network::IsHost() && (Packet.Address == "0" || Packet.Address == "1"))
		//	PrintToConsole("RPC Called | Address : " + (Packet.Address == PlayerNetworkAddress ? ClientInfo.NetworkAddress : Packet.Address == ClientInfo.NetworkAddress ? PlayerNetworkAddress : Packet.Address) + " | ClassName : " + Packet.ClassName + " | FunctionName : " + Packet.FunctionName);

		switch (RPC->GetType())
		{
		case eRPCType::Client:
			if (RPC->IsReqTimeStamp())
				RPC->Call(sArchive(Packet.TimeStamp, Packet.Data));
			else
				RPC->Call(sArchive(Packet.Data));
			break;
		case eRPCType::Server:
			CallRPCFromServer(ClientInfo.NetworkAddress/*Packet.Address*/, Packet.FunctionName, Packet.Data, RPC->IsReliable());
			break;
		case eRPCType::ServerAndClient:
			// Called on Server first
			if (RPC->IsReqTimeStamp())
				RPC->Call(sArchive(Packet.TimeStamp, Packet.Data));
			else
				RPC->Call(sArchive(Packet.Data));
			break;
		}
	}
	else if (Packet.Type == eNetworkPacketType::DirectCall)
	{
		auto RPC = RemoteProcedureCallManager::Get().GetRPC("Global", "WSClient", Packet.FunctionName);
		if (!RPC)
		{
			//PrintToConsole("RPC Not Called!");
			return;
		}

		//PrintToConsole("DirectCall " + Packet.FunctionName);

		switch (RPC->GetType())
		{
		case eRPCType::Server:
		case eRPCType::ServerAndClient:
			//PrintToConsole("RPC Not Called!");
			break;
		case eRPCType::Client:
			RPC->Call(sArchive(Packet.Data));
			break;
		}
	}
	else if (Packet.Type == eNetworkPacketType::String)
	{
		StringFromServer(Packet.Data);
	}
}

bool WSClient::Connect(sGameInstance* pInstance, std::string ip, std::uint16_t Port)
{
	std::lock_guard<std::mutex> locker(Mutex);

	if (bIsConnected)
		return false;

	Info = sServerInfo();

	Instance = pInstance;
	while (Instance->GetPlayerCount() != 0)
	{
		Instance->RemoveLastPlayer();
	}

	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;

	int iResult;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(ip.c_str(), std::to_string(Port).c_str(), &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		return false;
	}

	int T = sizeof(sPacket);

	ClientValidation();

	bIsConnected.store(true, std::memory_order_release);

	Engine::QueueJob([&]()
		{
			std::vector<std::uint8_t> buffer(256 * MaximumMessagePerTick);
			sArchive LoD;

			int bytesReceived = 0;

			while (bIsConnected.load(std::memory_order_acquire))
			{
				bytesReceived = recv(ConnectSocket, (char*)buffer.data(), (int)buffer.size(), 0);
				//bytesReceived = recvfrom(ConnectSocket, buffer, sizeof(buffer), 0, NULL, NULL);

				if (bytesReceived == SOCKET_ERROR)
				{
					int error = WSAGetLastError();

					if (error == WSAEWOULDBLOCK)
					{
						// No data available, continue or sleep and retry
						std::this_thread::sleep_for(std::chrono::milliseconds(100));
						continue;
					}
					else {
						std::cerr << "Error in recv: " << error << std::endl;
						break;
					}
				}

				if (bytesReceived > 0)
				{
					std::lock_guard<std::mutex> locker(Mutex);
					sArchive pArchive;
					//pArchive.ResizeData(256 * 32);
					pArchive.AppendData(buffer, bytesReceived);

					int i = 1;
					while (pArchive.GetPosition() < pArchive.GetSize())
					{
						sPacket Packet;
						pArchive >> Packet;
						Packets.push(Packet);
						pArchive.ResetPos(i * 256);
						i++;

						if (Packet.Address == "" || Packet.FunctionName == "" || Packet.ClassName == "")
							PrintToConsole("Empty");
					}
				}
			}
		}
	);

	return true;
}

bool WSClient::Disconnect()
{
	std::lock_guard<std::mutex> locker(Mutex);

	if (!bIsConnected)
		return false;

	Info = sServerInfo();

	bIsConnected.store(false, std::memory_order_release);

	WSAEVENT NewEvent;
	NewEvent = WSACreateEvent();

	int iResult = 0;
	// shutdown the connection since no more data will be sent
	iResult = WSAEventSelect(ConnectSocket, NewEvent, FD_CLOSE);
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		printf("Client : shutdown failed with error: %d\n", WSAGetLastError());
	}

	// cleanup
	closesocket(ConnectSocket);

	Instance->OpenLevel("DefaultLevel");

	OnDisconnectedFromServer();

	return true;
}

void WSClient::StringFromServer(std::string STR)
{
	PrintToConsole(STR);
}

void WSClient::SetMaximumMessagePerTick(std::size_t Size)
{
	MaximumMessagePerTick = Size;
}

void WSClient::SendBufferToServer(const void* buffer, std::size_t Size, bool reliable)
{
	if (!bIsValidationCalled)
	{
		PrintToConsole("SendBufferToServer called before validation and ignored.");
		return;
	}

	int result = send(ConnectSocket, (char*)buffer, (int)Size, 0);

	if (result == SOCKET_ERROR) 
	{
		int error = WSAGetLastError();
		// Handle the error based on the value of 'error'
		PrintToConsole("Error in sendto: " + std::to_string(error));
	}
	else
	{
		// 'result' contains the number of bytes sent
	}
}

void WSClient::SendToServer(const sArchive& Archive, bool reliable)
{
	SendBufferToServer(Archive.GetData().data(), Archive.GetSize(), reliable);
}

void WSClient::CallRPCFromServer(std::string Address, std::string ClassName, std::string FunctionName, bool reliable, std::optional<std::string> Data)
{
	if (!bIsValidationCalled)
	{
		PrintToConsole("RPC called before validation and ignored.| Address : " + Address + " | ClassName : " + ClassName + " | FunctionName : " + FunctionName);
		return;
	}
	sPacket Packet;
	Packet.TimeStamp = Engine::GetUTCTimeNow();
	Packet.Address = Address == PlayerNetworkAddress ? ClientInfo.NetworkAddress : Address;
	Packet.ClassName = ClassName;
	Packet.FunctionName = FunctionName;
	Packet.Data = Data.has_value() ? *Data : "";
	Packet.Type = eNetworkPacketType::RPC;
	sArchive Archive;
	Archive.ResizeData(256);
	Archive << Packet;
	//SendStringToServer(Archive.GetDataAsString(), reliable);
	SendBufferToServer(Archive.GetData().data(), Archive.GetSize(), reliable);
}

void WSClient::CallRPC(std::string Address, std::string ClassName, std::string Name, const sArchive& Params, std::optional<bool> reliable)
{
	CallRPCFromServer(Address, ClassName, Name, reliable.has_value() ? *reliable : RemoteProcedureCallManager::Get().GetRPC(Address, ClassName, Name)->IsReliable(), Params);
}

void WSClient::CallMessageRPCFromServer(std::string Message, bool reliable)
{
	if (!bIsValidationCalled)
	{
		PrintToConsole("CallMessageRPCFromServer called before validation and ignored. | Message : " + Message);
		return;
	}
	sArchive DataArchive;
	DataArchive << Message;
	sPacket Packet;
	Packet.TimeStamp = Engine::GetUTCTimeNow();
	Packet.Address = "Global";
	Packet.ClassName = "WSServer";
	Packet.FunctionName = "StringFromClient";
	Packet.Data = DataArchive.GetDataAsString();
	Packet.Type = eNetworkPacketType::String;
	sArchive Archive;
	Archive.ResizeData(256);
	Archive << Packet;
	//SendStringToServer(Archive.GetDataAsString(), reliable);
	SendBufferToServer(Archive.GetData().data(), Archive.GetSize(), reliable);
}

void WSClient::DirectCallToServer(std::string FunctionName, bool reliable, std::optional<std::string> Data)
{
	if (!bIsValidationCalled)
	{
		PrintToConsole("DirectCallToServer called before validation and ignored. | FunctionName : " + FunctionName);
		return;
	}
	sPacket Packet;
	Packet.TimeStamp = Engine::GetUTCTimeNow();
	Packet.Address = "Global";
	Packet.ClassName = "WSServer";
	Packet.FunctionName = FunctionName;
	Packet.Data = Data.has_value() ? *Data : "";
	Packet.Type = eNetworkPacketType::DirectCall;
	sArchive Archive;
	Archive.ResizeData(256);
	Archive << Packet;
	//SendStringToServer(Archive.GetDataAsString(), reliable);
	SendBufferToServer(Archive.GetData().data(), Archive.GetSize(), reliable);
}

void WSClient::SendStringToServer(const std::string& string, bool reliable)
{
	SendBufferToServer((void*)string.data(), string.size(), reliable);
}

void WSClient::PushMessage(void* buffer, std::size_t size, bool reliable)
{

}

void WSClient::SendMessages()
{

}

void WSClient::ClientValidation()
{
	PrintToConsole("ClientValidation");
	bIsValidationCalled = true;
	sPacket Packet;
	Packet.TimeStamp = Engine::GetUTCTimeNow();
	Packet.Address = "Global";
	Packet.ClassName = "WSServer";
	Packet.FunctionName = "ValidateClient";
	Packet.Data = "Some Encrypted Code";
	Packet.Type = eNetworkPacketType::Validation;
	sArchive Archive;
	Archive.ResizeData(256);
	Archive << Packet;
	SendToServer(Archive, true);
}

void WSClient::PingServer()
{
	DirectCallToServerEx("PingFromClient", false, Engine::GetUTCTimeNow().GetTotalMillisecond());
}

void WSClient::PingFromServer(std::uint64_t duration)
{
	Latency = Engine::GetUTCTimeNow().GetTotalMillisecond() - duration;
	//PrintToConsole("Ping : " + std::to_string(Ping));
}

void WSClient::OnReciveServerInfo(sServerInfo pInfo)
{
	Info = pInfo;
}

void WSClient::OnConnecting(sServerInfo pInfo)
{
	PrintToConsole("OnConnecting");
	Info = pInfo;
}

void WSClient::OnConnected(sServerInfo::sConnectedPlayerInfo ID, sServerInfo inInfo)
{
	PrintToConsole("OnConnected");
	ClientInfo = ID;
	PlayerNetworkAddress = "";
	if (!Network::IsHost() && Instance->GetPlayerCount() == 0)
	{
		auto Player = Instance->CreatePlayer(eNetworkRole::Client);
		Player->DespawnPlayerFocusedActor();
		PlayerNetworkAddress = Player->GetClassNetworkAddress();
	}
	else
	{
		PlayerNetworkAddress = Instance->GetPlayer(0)->GetClassNetworkAddress();
	}
	Info = inInfo;
	if (!Network::IsHost())
	{
		Instance->OpenLevel(Info.ServerName);
		//Instance->GetPlayer()->SpawnPlayerFocusedActor();
	}
	OnConnectedToServer();

	if (!Network::IsHost())
	{
		for (const auto& pInfo : Info.ConnectedPlayerInfos)
		{
			if (pInfo == ClientInfo)
			{
				continue;
			}

			OnPlayerConnectedToServer(pInfo.PlayerName, pInfo.NetworkAddress == PlayerNetworkAddress ? ClientInfo.NetworkAddress : pInfo.NetworkAddress);
		}
	}

	sClientInfo pClientInfo;
	pClientInfo.PlayerName = Instance->GetPlayer()->GetPlayerName();
	DirectCallToServerEx("OnClientSuccessfullyConnected", true, pClientInfo);
}

void WSClient::OnNewPlayerConnected(sServerInfo::sConnectedPlayerInfo ID, sServerInfo pInfo)
{
	if (Network::IsHost())
		return;

	Info = pInfo;
	if (ID != ClientInfo)
		OnPlayerConnectedToServer(ID.PlayerName, ID.NetworkAddress == PlayerNetworkAddress ? ClientInfo.NetworkAddress : ID.NetworkAddress);
	//OnPlayerConnectedToServer();
}

void WSClient::OnPlayerDisconnected(sServerInfo::sConnectedPlayerInfo ID, sServerInfo pInfo)
{
	if (Network::IsHost())
		return;

	Info = pInfo;
	if (ID != ClientInfo)
		OnPlayerDisconnectedFromServer(ID.PlayerName, ID.NetworkAddress == PlayerNetworkAddress ? ClientInfo.NetworkAddress : ID.NetworkAddress);
	//OnPlayerDisconnectedFromServer();
}

bool WSClient::OnServerLevelChanged(std::string Level)
{
	Instance->OpenLevel(Level);
	return true;
}

void WSClient::OnPlayerNameChanged(sServerInfo pInfo)
{
	Info = pInfo;
}

void WSClient::OnNameChanged()
{
	DirectCallToServerEx("OnPlayerChangeName", true, Instance->GetPlayer()->GetPlayerName());
}

void WSClient::OnNameChangedFromServer(std::string NewName)
{
	PrintToConsole("OnNameChangedFromServer : " + NewName);
	Instance->GetPlayer()->SetPlayerName(NewName);
}

void WSClient::PrintToConsole(std::string Message)
{
	std::cout << "Client : " << Message << std::endl;
}

#endif

#if Enable_ENET

#pragma comment(lib, "enet64.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")

ENetServer::ENetServer()
	: bIsServerRunning(false)
	, PlayerSize(8)
	, Server(nullptr)
{
	enet_initialize();
}

ENetServer::~ENetServer()
{
	CloseServer();
	enet_deinitialize();
	Server = nullptr;
}

void ENetServer::StartServer(std::uint16_t Port, std::size_t PlayerCount)
{
	std::lock_guard<std::mutex> locker(Mutex);

	if (bIsServerRunning)
		return;

	PlayerSize = PlayerCount;

	bIsServerRunning.store(true, std::memory_order_release);

	Engine::QueueJob([&]()
		{
			/* Bind the server to the default localhost.     */
			/* A specific host address can be specified by   */
			/* enet_address_set_host (& address, "x.x.x.x"); */
			ENetAddress adress;
			adress.host = ENET_HOST_ANY;
			/* Bind the server to port. */
			adress.port = 7777;

			Server = enet_host_create(&adress	/* the address to bind the server host to */,
				PlayerSize						/* allow up to 'PlayerSize' clients and/or outgoing connections */,
				2								/* allow up to 2 channels to be used, 0 and 1 */,
				0								/* assume any amount of incoming bandwidth */,
				0								/* assume any amount of outgoing bandwidth */);

			if (!Server)
			{
				std::cout << "An error occurred while trying to create an ENet server host." << std::endl;
				return;
			}

			ENetEvent event;

			while (enet_host_service(Server, &event, 1000) > 0 && bIsServerRunning.load(std::memory_order_acquire))
			{
				switch (event.type)
				{
				case ENET_EVENT_TYPE_CONNECT:
				{
					printf("A new client connected from %x:%u.\n",
						event.peer->address.host,
						event.peer->address.port);
					/* Store any relevant client information here. */
					event.peer->data = (void*)"Client information";

					break;
				}
				case ENET_EVENT_TYPE_RECEIVE:
				{
					printf("A packet of length %u containing %s was received from %s (Host Address %x:%u) on channel %u.\n",
						event.packet->dataLength,
						event.packet->data,
						event.peer->data,
						event.peer->address.host,
						event.peer->address.port,
						event.channelID);

					/* Clean up the packet now that we're done using it. */
					enet_packet_destroy(event.packet);

					break;
				}
				case ENET_EVENT_TYPE_DISCONNECT:
				{
					printf("A client disconnected from %x:%u.\n",
						event.peer->address.host,
						event.peer->address.port);
					/* Reset the peer's client information. */
					event.peer->data = NULL;

					break;
				}
				}
			}

			bIsServerRunning.store(false, std::memory_order_release);
			enet_host_destroy(Server);
		});
}

void ENetServer::CloseServer()
{
	bIsServerRunning.store(false, std::memory_order_release);
}

void ENetServer::ParsePacket(int ID, char* Data)
{
}

void ENetServer::SendPacket(ENetPeer* Peer, int Channel, const char* Data, bool IsReliable)
{
	ENetPacket* Packet = enet_packet_create(Data, strlen(Data) + 1, IsReliable ? ENetPacketFlag::ENET_PACKET_FLAG_RELIABLE : ENetPacketFlag::ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
	enet_peer_send(Peer, Channel, Packet);
}

void ENetServer::BroadcastPacket(const char* Data, int Channel, bool IsReliable)
{
	ENetPacket* Packet = enet_packet_create(Data, strlen(Data) + 1, IsReliable ? ENetPacketFlag::ENET_PACKET_FLAG_RELIABLE : ENetPacketFlag::ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
	enet_host_broadcast(Server, Channel, Packet);
}


ENetClient::ENetClient()
	: bJoined(false)
	, Peer(nullptr)
{
	Client = enet_host_create(nullptr, 1, 2, 0, 0);
}

ENetClient::~ENetClient()
{
	Disconnect();
	enet_host_destroy(Client);
}

void ENetClient::SendPacket(const char* pData, std::uint8_t Channel, bool IsReliable)
{
	ENetPacket* Packet = enet_packet_create(pData, strlen(pData) + 1, IsReliable ? ENetPacketFlag::ENET_PACKET_FLAG_RELIABLE : ENetPacketFlag::ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
	enet_peer_send(Peer, Channel, Packet);
}

bool ENetClient::Join(std::string ip, std::uint16_t Port)
{
	std::lock_guard<std::mutex> locker(Mutex);

	if (bJoined)
		return false;

	ENetAddress adress;
	ENetEvent event;

	if (ip.empty())
		enet_address_set_host(&adress, "127.0.0.1");
	else
		enet_address_set_host(&adress, ip.c_str());

	adress.port = Port;

	/* Initiate the connection, allocating the two channels 0 and 1. */
	Peer = enet_host_connect(Client, &adress, 2, 0);

	if (Peer == nullptr)
	{
		std::cout << "No available peers for initiating an ENet connection." << std::endl;
		return false;
	}

	if (enet_host_service(Client, &event, 5000) > 0
		&& event.type == ENET_EVENT_TYPE_CONNECT)
	{
		std::cout << "Connection succeeded." << std::endl;
	}
	else
	{
		std::cout << "Connection failed." << std::endl;
		enet_peer_reset(Peer);
		return false;
	}

	bJoined.store(true, std::memory_order_release);

	Engine::QueueJob([&]()
		{
			while (bJoined.load(std::memory_order_acquire))
			{
				ENetEvent event;
				while (enet_host_service(Client, &event, 0) > 0/* && bJoined.load(std::memory_order_acquire)*/)
				{
					switch (event.type)
					{
					case ENET_EVENT_TYPE_RECEIVE:
					{
						printf("A packet of length %u containing %s was received from %s (Host Address %x:%u) on channel %u.\n",
							event.packet->dataLength,
							event.packet->data,
							event.peer->data,
							event.peer->address.host,
							event.peer->address.port,
							event.channelID);
						/* Clean up the packet now that we're done using it. */
						enet_packet_destroy(event.packet);

						break;
					}
					case ENET_EVENT_TYPE_DISCONNECT:
					{

						break;
					}
					}
				}
			}
		}
	);

	return true;
}

void* ENetClient::GetData() const
{
	return nullptr;
}

void ENetClient::Disconnect()
{
	if (!Peer)
		return;

	bJoined.store(false, std::memory_order_release);

	ENetEvent event;

	enet_peer_disconnect(Peer, 0);
	/*
	* Allow up to 3 seconds for the disconnect to succeed
	* and drop any packets received packets.
	*/
	while (enet_host_service(Client, &event, 3000) > 0)
	{
		switch (event.type)
		{
		case ENET_EVENT_TYPE_RECEIVE:
			enet_packet_destroy(event.packet);
			break;
		case ENET_EVENT_TYPE_DISCONNECT:
			std::cout << "Disconnection succeeded." << std::endl;
			return;
		}
	}

	/* We've arrived here, so the disconnect attempt didn't */
	/* succeed yet.  Force the connection down.             */
	enet_peer_reset(Peer);
}

#endif
