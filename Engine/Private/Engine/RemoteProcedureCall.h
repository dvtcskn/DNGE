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

#include <string>
#include <map>
#include <vector>
#include <unordered_map>
#include <functional>
#include <type_traits>
#include <mutex>
#include <ostream>
#include <iostream>
#include <queue>
#include <thread>
#include <any>

#include "Engine/AbstractEngine.h"

class RemoteProcedureCallManager
{
	sBaseClassBody(sClassNoDefaults, RemoteProcedureCallManager);
private:
	RemoteProcedureCallManager() = default;
	RemoteProcedureCallManager(const RemoteProcedureCallManager& Other) = delete;
	RemoteProcedureCallManager& operator=(const RemoteProcedureCallManager&) = delete;

public:
	static RemoteProcedureCallManager& Get()
	{
		static RemoteProcedureCallManager instance;
		return instance;
	}

private:
	std::mutex mutex;
	std::map<std::string, std::map<std::string, std::vector<RemoteProcedureCallBase*>>> Functions;

public:
	~RemoteProcedureCallManager()
	{
		Destroy();
	}

	inline void Destroy()
	{
		for (auto& Address : Functions)
		{
            for (auto& Class : Address.second)
            {
                for (auto& pfn : Class.second)
                {
                    delete pfn;
                    pfn = nullptr;
                }
            }
		}
		Functions.clear();
	}

    inline bool IsExist(std::string InAddress, std::string ClassName, std::string Name) const
    {
        return GetRPC(InAddress, ClassName, Name) != nullptr;
    }

    inline void ChangeBase(std::string InOldName, std::string NewName)
    {
        auto nodeHandler = Functions.extract(InOldName);
        nodeHandler.key() = NewName;
        Functions.insert(std::move(nodeHandler));
    }

    inline RemoteProcedureCallBase* GetRPC(std::string InAddress, std::string ClassName, std::string Name) const
    {
        for (const auto& Address : Functions)
        {
            if (Address.first == InAddress)
            {
                for (const auto& Class : Address.second)
                {
                    if (Class.first == ClassName)
                    {
                        for (const auto& pfn : Class.second)
                        {
                            if (pfn->GetName() == Name)
                            {
                                return pfn;
                            }
                        }
                        break;
                    }
                }
                break;
            }
        }
        return nullptr;
    }

    inline void Unregister(std::string Address)
    {
        if (Functions.find(Address) != Functions.end()) 
        {
            for (auto& Class : Functions[Address])
            {
                for (auto& pfn : Class.second)
                {
                    delete pfn;
                    pfn = nullptr;
                }
            }
            Functions.erase(Address);
        }
    }

    inline void Unregister(std::string Address, const std::string& InName)
    {
        if (Functions.find(Address) != Functions.end())
        {
            if (Functions[Address].find(InName) != Functions[Address].end())
            {
                for (auto& pfn : Functions[Address][InName])
                {
                    delete pfn;
                    pfn = nullptr;
                }

                Functions[Address].erase(InName);
            }
        }
    }

    inline void Unregister(std::string Address, const std::string& InClassName, const std::string& InName)
    {
        if (Functions.find(Address) != Functions.end())
        {
            if (Functions[Address].find(InClassName) != Functions[Address].end())
            {
                RemoteProcedureCallBase* RPC = nullptr;
                for (auto& pfn : Functions[Address][InClassName])
                {
                    if (pfn->GetName() == InName)
                    {
                        RPC = pfn;
                        break;
                    }
                }

                auto& RPCs = Functions[Address][InClassName];
                RPCs.erase(std::remove_if(RPCs.begin(), RPCs.end(), [&](RemoteProcedureCallBase* RPC) {
                    return RPC->GetName() == InName;
                    }), RPCs.end());

                if (RPC)
                {
                    delete RPC;
                    RPC = nullptr;
                }
            }
        }
    }

    inline void Register(std::string Address, const std::string& InClassName, RemoteProcedureCallBase* RPC)
    {
        if (IsExist(Address, InClassName, RPC->GetName()))
            Unregister(Address, InClassName, RPC->GetName());
        Functions[Address][InClassName].push_back(RPC);
    }

    // Register a function to be called remotely.
    template<typename... Args>
    inline  void Register(std::string Address, const std::string& InClassName, eRPCType InType, const std::string& InName, bool bReliable, /*int InNumArgs,*/ const std::function<void(Args...)>& pfn)
    {
        Functions[Address][InClassName] = new RemoteProcedureCall(InType, InName, bReliable, /*InNumArgs,*/ pfn);
    }

    // Call a function remotely.
    inline bool SetParams(std::string Address, const std::string& InClassName, const std::string& rpcName, const sArchive& Params)
    {
        if (RemoteProcedureCallBase* RPC = GetRPC(Address, InClassName, rpcName))
        {
            RPC->SetParams(Params);
        }
        else
        {
            std::cerr << "RPC '" << rpcName << "' not found." << std::endl;
        }
        return false;
    }

    // Call a function remotely.
    inline bool SetParamsAndCall(std::string Address, const std::string& InClassName, const std::string& rpcName, const sArchive& Params)
    {
        if (RemoteProcedureCallBase* RPC = GetRPC(Address, InClassName, rpcName))
        {
            RPC->SetParams(Params);
            RPC->Call();
            return true;
        }
        else
        {
            std::cerr << "RPC '" << rpcName << "' not found." << std::endl;
        }
        return false;
    }

    // Call a function remotely.
    template <typename... Args>
    inline bool Call(std::string Address, const std::string& InClassName, const std::string& rpcName, std::tuple<Args...> Params)
    {
        if (RemoteProcedureCallBase* RPC = GetRPC(Address, InClassName, rpcName))
        {
            static_cast<RemoteProcedureCall<Args...>*>(RPC)->SetParamsAsTuple(Params);
            static_cast<RemoteProcedureCall<Args...>*>(RPC)->Call();
        }
        else
        {
            std::cerr << "RPC '" << rpcName << "' not found." << std::endl;
        }
        return false;
    }

    // Call a function remotely.
    //template <typename... Args>
    inline bool Call(std::string Address, const std::string& InClassName, const std::string& rpcName/*, Args... args*/)
    {
        if (RemoteProcedureCallBase* RPC = GetRPC(Address, InClassName, rpcName))
        {
            RPC->Call();
        }
        else
        {
            std::cerr << "RPC '" << rpcName << "' not found." << std::endl;
        }
        return false;
    }
};
