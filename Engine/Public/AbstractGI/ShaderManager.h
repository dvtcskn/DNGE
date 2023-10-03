#pragma once

#include "Engine/AbstractEngine.h"
#include <map>
#include <vector>

class sShaderManager
{
	sBaseClassBody(sClassNoDefaults, sShaderManager);
private:
	sShaderManager() = default;
	sShaderManager(const sShaderManager& Other) = delete;
	sShaderManager& operator=(const sShaderManager&) = delete;

public:
	static sShaderManager& Get()
	{
		static sShaderManager instance;
		return instance;
	}

private:
	//std::vector<ITexture2D::SharedPtr> Textures;

	inline bool IsShaderExist(ITexture2D* Texture)
	{
		/*for (const auto& pTexture : Textures)
		{
			if (Texture == pTexture.get())
				return true;
		}*/
		return false;
	}

public:
	~sShaderManager()
	{
		Destroy();
	}

	inline void Destroy()
	{
		//for (auto& Texture : Textures)
		//	Texture = nullptr;
		//Textures.clear();
	}

	inline void DestroyShader(std::wstring Path, std::string Name)
	{

	}
	inline void DestroyShader(std::string Name)
	{

	}

	inline bool IsShaderExist(std::wstring Path, std::string Name)
	{

		return false;
	}

	inline bool IsShaderExist(std::string Name)
	{

		return false;
	}

	inline bool StoreShader(const ITexture2D::SharedPtr& Texture)
	{

		return false;
	}
	inline ITexture2D* GetShader(std::wstring Path, std::string Name) const
	{

		return nullptr;
	}
	inline ITexture2D* GetShader(std::string Name) const
	{

		return nullptr;
	}
	inline ITexture2D::SharedPtr GetShaderAsShared(std::wstring Path, std::string Name) const
	{

		return nullptr;
	}
	inline ITexture2D::SharedPtr GetShaderAsShared(std::string Name) const
	{

		return nullptr;
	}
};
