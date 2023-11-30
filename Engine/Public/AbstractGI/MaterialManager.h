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

#include "Material.h"

class sMaterialManager
{
	sBaseClassBody(sClassNoDefaults, sMaterialManager);
private:
	sMaterialManager() = default;
	sMaterialManager(const sMaterialManager& Other) = delete;
	sMaterialManager& operator=(const sMaterialManager&) = delete;

public:
	static sMaterialManager& Get()
	{
		static sMaterialManager instance;
		return instance;
	}

private:
	std::vector<sMaterial::SharedPtr> Materials;

public:
	~sMaterialManager()
	{
		Destroy();
	}

	inline void Destroy()
	{
		for (auto& Material : Materials)
			Material = nullptr;
		Materials.clear();
	}

	inline void DestroyMaterial(std::string MaterialName)
	{
		sMaterial::SharedPtr Mat = nullptr;
		for (auto& Material : Materials)
		{
			if (Material->GetName() == MaterialName)
			{
				Mat = Material;
			}
		}
		Materials.erase(std::find(Materials.begin(), Materials.end(), Mat));
		Mat = nullptr;
	}

	inline void DestroyMaterialInstance(std::string MaterialName, std::string InstanceName)
	{
		auto Material = GetMaterial(MaterialName);
		if (Material)
			Material->DestroyInstance(InstanceName);
	}

	inline void StoreMaterial(const sMaterial::SharedPtr& Material)
	{
		Materials.push_back(Material);
	}
	inline sMaterial* GetMaterial(std::string MaterialName) const
	{
		for (auto& Material : Materials)
		{
			if (Material->GetName() == MaterialName)
			{
				return Material.get();
			}
		}
		return nullptr;
	}
	inline sMaterial::SharedPtr GetMaterialAsShared(std::string MaterialName) const
	{
		for (auto& Material : Materials)
		{
			if (Material->GetName() == MaterialName)
			{
				return Material;
			}
		}
		return nullptr;
	}

	inline sMaterial::sMaterialInstance* CreateMaterialInstance(std::string MaterialName, std::string InstanceName) const
	{
		auto Material = GetMaterial(MaterialName);
		if (Material)
			return Material->CreateInstance(InstanceName).get();
		return nullptr;
	}
	inline sMaterial::sMaterialInstance::SharedPtr CreateSharedMaterialInstance(std::string MaterialName, std::string InstanceName) const
	{
		auto Material = GetMaterial(MaterialName);
		return Material->CreateInstance(InstanceName);
	}
	
	inline sMaterial::sMaterialInstance* GetMaterialInstance(std::string MaterialName, std::string InstanceName) const
	{
		auto Material = GetMaterial(MaterialName);
		if (Material)
			return Material->GetInstance(InstanceName).get();
		return nullptr;
	}
	inline sMaterial::sMaterialInstance* GetMaterialInstance(std::string MaterialName, std::size_t Index) const
	{
		auto Material = GetMaterial(MaterialName);
		if (Material)
			return Material->GetInstance(Index).get();
		return nullptr;
	}
	inline sMaterial::sMaterialInstance::SharedPtr GetSharedMaterialInstance(std::string MaterialName, std::string InstanceName) const
	{
		auto Material = GetMaterial(MaterialName);
		return Material->GetInstance(InstanceName);
	}
};
