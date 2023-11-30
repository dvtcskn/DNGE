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
#include "GGameInstance.h"
#include "GPlayerController.h"
#include "GPlayerCharacter.h"
#include "GAIController.h"
#include <Utilities/ConfigManager.h>

GGameInstance::GGameInstance(IMetaWorld* World)
	: Super(World)
{
	AddAIController(GAIController::Create(this));
}

GGameInstance::~GGameInstance()
{
	Network::UnregisterRPC("Instance", "GGameInstance");
}

void GGameInstance::OnBeginPlay()
{
	RegisterRPCfn("Instance", "GGameInstance", "ResetLevel_Client", eRPCType::Client, true, false, std::bind(&GGameInstance::ResetLevel_Client, this));

	std::string Role = ConfigManager::Get().GetGameConfig().NetworkRole;
	if (Role == "Host")
	{
		Network::CreateSession("My Session", this, "DefaultLevel", GetLimitPlayer());
	}
	else if (Role == "Client")
	{
		Network::Connect(this, ConfigManager::Get().GetGameConfig().IP, ConfigManager::Get().GetGameConfig().Port);
	}
}

void GGameInstance::OnTick(const double DeltaTime)
{
}

sPlayerProxyBase::SharedPtr GGameInstance::ConstructPlayerProxy(std::string PlayerName, std::string NetAddress)
{
	return sPlayerProxyBase::Create(this, PlayerName, NetAddress, GProxyController::Create(), GPlayerCharacter::Create("GPlayerCharacter"));
}

void GGameInstance::InputProcess(const GMouseInput& MouseInput, const GKeyboardChar& KeyboardChar)
{
	Super::InputProcess(MouseInput, KeyboardChar);
}

void GGameInstance::ResetLevel()
{
	if (Network::IsClient())
		return;

	Network::CallRPC("Instance", "GGameInstance", "ResetLevel_Client", sArchive());
	Super::ResetLevel();
}

void GGameInstance::ResetLevel_Client()
{
	if (Network::IsHost())
		return;

	//auto Players = GetPlayers();
	//for (const auto& Player : Players)
	//	Player->SpawnPlayerFocusedActor();
}
