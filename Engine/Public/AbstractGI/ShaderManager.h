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
