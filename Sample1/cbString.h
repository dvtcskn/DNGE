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

#include <cbText.h>
#include <cbFont.h>
#include <optional>

class cbFontResources
{
private:
	cbFontResources() = default;
	cbFontResources(const cbFontResources&) = delete;
	cbFontResources(cbFontResources&&) = delete;
	cbFontResources& operator=(const cbFontResources&) = delete;
	cbFontResources& operator=(cbFontResources&&) = delete;

public:
	~cbFontResources()
	{
		Release();
	}

	void Release()
	{
		for (auto& pFont : Fonts)
		{
			delete pFont;
			pFont = nullptr;
		}
		Fonts.clear();
	}

	static cbFontResources& Get()
	{
		static cbFontResources instance;
		return instance;
	}

	std::vector<cbgui::cbIFontFamily*> GetFontFamilies() const { return Fonts; }
	cbgui::cbIFontFamily* GetFontFamily(std::string Name) const
	{
		for (auto& pFont : Fonts)
		{
			if (pFont->GetDesc().FontFamilyName == Name)
			{
				return pFont;
			}
		}
		return nullptr;
	}
	cbgui::cbIFontFamily* GetFontFamily(const std::size_t& Index = 0) const
	{
		if (Index >= Fonts.size())
			return nullptr;
		return Fonts[Index];
	}

	bool IsFontFamilyExist(const std::string& Name) const
	{
		for (const auto& FontFamily : Fonts)
		{
			if (FontFamily->GetDesc().FontFamilyName == Name)
				return true;
		}
		return false;
	}

	std::optional<std::vector<unsigned char>> GetFontFamilyTextureAtlas(const std::string& FontFamilyName) const
	{
		if (auto Font = GetFontFamily(FontFamilyName))
		{
			return Font->GetTexture();
		}
		return std::nullopt;
	}

	cbgui::cbIFontFamily* AddFont(cbgui::cbIFontFamily* FontFamily)
	{
		if (IsFontFamilyExist(FontFamily->GetFontFamilyName()))
			return GetFontFamily(FontFamily->GetFontFamilyName());

		Fonts.push_back(FontFamily);
		return FontFamily;
	}

	cbgui::cbIFontFamily* AddFreeTypeFont(const cbgui::cbFontDesc& FontDesc)
	{
		if (IsFontFamilyExist(FontDesc.FontFamilyName))
			return GetFontFamily(FontDesc.FontFamilyName);

		cbgui::cbIFontFamily* FontFamily = cbgui::CreateFreeTypeFont(FontDesc);
		Fonts.push_back(FontFamily);
		return FontFamily;
	}

private:
	std::vector<cbgui::cbIFontFamily*> Fonts;
};

class cbString : public cbgui::cbText
{
	cbClassBody(cbClassConstructor, cbString, cbgui::cbText);
public:
	cbString(const std::string& Text, const cbgui::cbTextDesc& TextDesc = cbgui::cbTextDesc())
		: cbgui::cbText(std::u32string(Text.begin(), Text.end()), TextDesc, cbFontResources::Get().GetFontFamily(0))
	{}
	cbString(const std::u32string& Text = U"", const cbgui::cbTextDesc& TextDesc = cbgui::cbTextDesc())
		: cbgui::cbText(Text, TextDesc, cbFontResources::Get().GetFontFamily(0))
	{}

	cbString(const cbString& String)
		: Super(String)
	{}

	cbWidget::SharedPtr CloneWidget()
	{
		return cbString::Create(*this);
	}

	void SetString(const std::string& Text, const std::optional<cbgui::cbTextDesc> TextDesc = std::nullopt) 
	{
		SetText(std::u32string(Text.begin(), Text.end()), TextDesc);
	}
};
