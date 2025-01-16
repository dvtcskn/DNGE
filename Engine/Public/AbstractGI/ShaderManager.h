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
	std::vector<IShader::SharedPtr> Shaders;

	inline bool IsShaderExist(IShader* Shader)
	{
		for (const auto& pShader : Shaders)
		{
			if (Shader == pShader.get())
				return true;
		}
		return false;
	}

public:
	~sShaderManager()
	{
		Destroy();
	}

	inline void Destroy()
	{
		for (auto& Shader : Shaders)
			Shader = nullptr;
		Shaders.clear();
	}

	inline void DestroyShader(std::wstring Path, std::string Name)
	{
		std::vector<IShader::SharedPtr>::iterator it = Shaders.begin();
		while (it != Shaders.end())
		{
			if ((*it))
			{
				if ((*it)->GetPath() == Path)
				{
					if ((*it)->GetName() == Name)
					{
						it = Shaders.erase(it);
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
				it = Shaders.erase(it);
			}
		}
	}
	inline void DestroyShader(std::string Name)
	{
		std::vector<IShader::SharedPtr>::iterator it = Shaders.begin();
		while (it != Shaders.end())
		{
			if ((*it))
			{
				if ((*it)->GetName() == Name)
				{
					it = Shaders.erase(it);
				}
				else
				{
					it++;
				}
			}
			else
			{
				it = Shaders.erase(it);
			}
		}
	}

	inline bool IsShaderExist(std::wstring Path, std::string Name)
	{
		for (const auto& Shader : Shaders)
		{
			if (Shader->GetPath() == Path)
			{
				if (Shader->GetName() == Name)
				{
					return true;
				}
			}
		}
		return false;
	}

	inline bool IsShaderExist(std::string Name)
	{
		for (const auto& Shader : Shaders)
		{
			if (Shader->GetName() == Name)
			{
				return true;
			}
		}
		return false;
	}

	inline bool StoreShader(const IShader::SharedPtr& Texture)
	{
		if (!IsShaderExist(Texture.get()))
		{
			Shaders.push_back(Texture);
			return true;
		}
		return false;
	}
	inline IShader* GetShader(std::wstring Path, std::string Name) const
	{
		for (const auto& Shader : Shaders)
		{
			if (Shader->GetPath() == Path)
			{
				if (Shader->GetName() == Name)
				{
					return Shader.get();
				}
			}
		}
		return nullptr;
	}
	inline IShader* GetShader(std::string Name) const
	{
		for (const auto& Shader : Shaders)
		{
			if (Shader->GetName() == Name)
			{
				return Shader.get();
			}
		}
		return nullptr;
	}
	inline IShader::SharedPtr GetShaderAsShared(std::wstring Path, std::string Name) const
	{
		for (const auto& Shader : Shaders)
		{
			if (Shader->GetPath() == Path)
			{
				if (Shader->GetName() == Name)
				{
					return Shader;
				}
			}
		}
		return nullptr;
	}
	inline IShader::SharedPtr GetShaderAsShared(std::string Name) const
	{
		for (const auto& Shader : Shaders)
		{
			if (Shader->GetName() == Name)
			{
				return Shader;
			}
		}
		return nullptr;
	}
};
