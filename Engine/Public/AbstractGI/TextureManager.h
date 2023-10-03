#pragma once

#include "Engine/AbstractEngine.h"
#include <map>
#include <vector>

class sTextureManager
{
	sBaseClassBody(sClassNoDefaults, sTextureManager);
private:
	sTextureManager() = default;
	sTextureManager(const sTextureManager& Other) = delete;
	sTextureManager& operator=(const sTextureManager&) = delete;

public:
	static sTextureManager& Get()
	{
		static sTextureManager instance;
		return instance;
	}

private:
	std::vector<ITexture2D::SharedPtr> Textures;

	inline bool IsTextureExist(ITexture2D* Texture)
	{
		for (const auto& pTexture : Textures)
		{
			if (Texture == pTexture.get())
				return true;
		}
		return false;
	}

public:
	~sTextureManager()
	{
		Destroy();
	}

	inline void Destroy()
	{
		for (auto& Texture : Textures)
			Texture = nullptr;
		Textures.clear();
	}

	inline void DestroyTexture(std::wstring Path, std::string Name)
	{
		std::vector<ITexture2D::SharedPtr>::iterator it = Textures.begin();
		while (it != Textures.end())
		{
			if ((*it))
			{
				if ((*it)->GetPath() == Path)
				{
					if ((*it)->GetName() == Name)
					{
						it = Textures.erase(it);
					}
					else
					{
						it++;
					}
				}
				else
				{
					it++;
				}
			}
			else
			{
				it = Textures.erase(it);
			}
		}
	}
	inline void DestroyTexture(std::string Name)
	{
		std::vector<ITexture2D::SharedPtr>::iterator it = Textures.begin();
		while (it != Textures.end())
		{
			if ((*it))
			{
				if ((*it)->GetName() == Name)
				{
					it = Textures.erase(it);
				}
				else
				{
					it++;
				}
			}
			else
			{
				it = Textures.erase(it);
			}
		}
	}

	inline bool IsTextureExist(std::wstring Path, std::string Name)
	{
		for (const auto& Texture : Textures)
		{
			if (Texture->GetPath() == Path)
			{
				if (Texture->GetName() == Name)
				{
					return true;
				}
			}
		}
		return false;
	}

	inline bool IsTextureExist(std::string Name)
	{
		for (const auto& Texture : Textures)
		{
			if (Texture->GetName() == Name)
			{
				return true;
			}
		}
		return false;
	}

	inline bool StoreTexture(const ITexture2D::SharedPtr& Texture)
	{
		if (!IsTextureExist(Texture.get()))
		{
			Textures.push_back(Texture);
			return true;
		}
		return false;
	}
	inline ITexture2D* GetTexture(std::wstring Path, std::string Name) const
	{
		for (const auto& Texture : Textures)
		{
			if (Texture->GetPath() == Path)
			{
				if (Texture->GetName() == Name)
				{
					return Texture.get();
				}
			}
		}
		return nullptr;
	}
	inline ITexture2D* GetTexture(std::string Name) const
	{
		for (const auto& Texture : Textures)
		{
			if (Texture->GetName() == Name)
			{
				return Texture.get();
			}
		}
		return nullptr;
	}
	inline ITexture2D::SharedPtr GetTextureAsShared(std::wstring Path, std::string Name) const
	{
		for (const auto& Texture : Textures)
		{
			if (Texture->GetPath() == Path)
			{
				if (Texture->GetName() == Name)
				{
					return Texture;
				}
			}
		}
		return nullptr;
	}
	inline ITexture2D::SharedPtr GetTextureAsShared(std::string Name) const
	{
		for (const auto& Texture : Textures)
		{
			if (Texture->GetName() == Name)
			{
				return Texture;
			}
		}
		return nullptr;
	}
};
