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
