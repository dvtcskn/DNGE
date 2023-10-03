#pragma once

#include "Sprite.h"

class AssetManager
{
	sBaseClassBody(sClassNoDefaults, AssetManager);
private:
	AssetManager() = default;
	AssetManager(const AssetManager& Other) = delete;
	AssetManager& operator=(const AssetManager&) = delete;

public:
	static AssetManager& Get()
	{
		static AssetManager instance;
		return instance;
	}

private:
	std::vector<sSprite::SharedPtr> Sprites;
	std::vector<sSpriteSheet::SharedPtr> SpriteSheets;
	bool bIsInitialized = false;

	inline bool IsSpriteExist(sSprite* pSprite)
	{
		for (const auto& Sprite : Sprites)
		{
			if (pSprite == Sprite.get())
				return true;
		}
		return false;
	}
	inline bool IsSpriteSheetExist(sSpriteSheet* pSheet)
	{
		for (const auto& SpriteSheet : SpriteSheets)
		{
			if (pSheet == SpriteSheet.get())
				return true;
		}
		return false;
	}
public:
	~AssetManager()
	{
		Destroy();
	}

	inline void Destroy()
	{
		for (auto& SpriteSheet : SpriteSheets)
			SpriteSheet = nullptr;
		SpriteSheets.clear();
		for (auto& Sprite : Sprites)
			Sprite = nullptr;
		Sprites.clear();
	}

	inline void DestroySprite(std::string SpriteName)
	{
		sSprite::SharedPtr pSprite = nullptr;
		for (auto& Sprite : Sprites)
		{
			if (Sprite->GetName() == SpriteName)
			{
				pSprite = Sprite;
			}
		}
		Sprites.erase(std::find(Sprites.begin(), Sprites.end(), pSprite));
		pSprite = nullptr;
	}
	inline void DestroySpriteSheet(std::string SpriteName)
	{
		sSpriteSheet::SharedPtr pSprite = nullptr;
		for (auto& Sprite : SpriteSheets)
		{
			if (Sprite->GetName() == SpriteName)
			{
				pSprite = Sprite;
			}
		}
		SpriteSheets.erase(std::find(SpriteSheets.begin(), SpriteSheets.end(), pSprite));
		pSprite = nullptr;
	}

	void InitializeResources();

	inline bool IsSpriteExist(std::string Name)
	{
		for (const auto& Sprite : Sprites)
		{
			if (Sprite->GetName() == Name)
			{
				return true;
			}
		}
		return false;
	}

	inline bool IsSpriteSheetExist(std::string Name)
	{
		for (const auto& SpriteSheet : SpriteSheets)
		{
			if (SpriteSheet->GetName() == Name)
			{
				return true;
			}
		}
		return false;
	}

	inline bool StoreSprite(const sSprite::SharedPtr& Sprite)
	{
		if (!IsSpriteExist(Sprite.get()))
		{
			Sprites.push_back(Sprite);
			return true;
		}
		return false;
	}
	inline bool StoreSpriteSheet(const sSpriteSheet::SharedPtr& Sprite)
	{
		if (!IsSpriteSheetExist(Sprite.get()))
		{
			SpriteSheets.push_back(Sprite);
			return true;
		}
		return false;
	}
	inline sSprite* GetSprite(std::string SpriteName) const
	{
		for (auto& Sprite : Sprites)
		{
			if (Sprite->GetName() == SpriteName)
			{
				return Sprite.get();
			}
		}
		return nullptr;
	}
	inline sSpriteSheet* GetSpriteSheet(std::string SpriteName) const
	{
		for (auto& Sprite : SpriteSheets)
		{
			if (Sprite->GetName() == SpriteName)
			{
				return Sprite.get();
			}
		}
		return nullptr;
	}
	inline sSprite::SharedPtr GetSpriteAsShared(std::string SpriteName) const
	{
		for (auto& Sprite : Sprites)
		{
			if (Sprite->GetName() == SpriteName)
			{
				return Sprite;
			}
		}
		return nullptr;
	}
	inline sSpriteSheet::SharedPtr GetSpriteSheetAsShared(std::string SpriteName) const
	{
		for (auto& Sprite : SpriteSheets)
		{
			if (Sprite->GetName() == SpriteName)
			{
				return Sprite;
			}
		}
		return nullptr;
	}
};
