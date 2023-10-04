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

#include <algorithm>
#include <map>
#include <cbStates.h>
#include <cbClassBody.h>
#include <cbMath.h>
#include "Material.h"

struct IUIMaterial
{
	cbBaseClassBody(cbClassDefaultProtectedConstructor, IUIMaterial)
};

struct UIMaterial : public IUIMaterial
{
	cbClassBody(cbClassConstructor, UIMaterial, IUIMaterial)
public:
	UIMaterial()
		: Material(nullptr)
	{}
	UIMaterial(sMaterial::sMaterialInstance* Mat)
		: Super()
		, Material(Mat)
	{}
	virtual ~UIMaterial()
	{
		Material = nullptr;
	}

	sMaterial::sMaterialInstance* Material;
};

class IUIMaterialStyle
{
	cbBaseClassBody(cbClassDefaultProtectedConstructor, IUIMaterialStyle)
public:
	virtual void SetName(const std::string& StyleName) = 0;
	virtual std::string GetName() const = 0;
	virtual UIMaterial* GetMaterial(unsigned short StyleState = 0) const = 0;
};

class UIMaterialStyle : public IUIMaterialStyle, public std::enable_shared_from_this<UIMaterialStyle>
{
	cbClassBody(cbClassConstructor, UIMaterialStyle, IUIMaterialStyle)
public:
	UIMaterialStyle(const std::string& StyleName, const UIMaterial::SharedPtr& DefaultMaterial = nullptr)
		: Name(StyleName)
		, Material(DefaultMaterial)
	{}

	virtual ~UIMaterialStyle()
	{
		Material = nullptr;
	}

	template <typename T>
	inline T* GetMaterialAs(unsigned short StyleState = 0) const
	{
		return static_cast<T*>(GetMaterial(StyleState));
	}

	virtual void SetMaterial(const UIMaterial::SharedPtr& pMaterial)
	{
		Material = pMaterial;
	}

	virtual UIMaterial* GetMaterial(unsigned short StyleState = 0) const
	{
		return Material.get();
	}

	virtual void RemoveMaterial()
	{
		Material = nullptr;
	}

	virtual void SetDefaultMaterial(const UIMaterial::SharedPtr& Color)
	{
		Material = Color;
	}

	virtual void SetName(const std::string& StyleName) override { Name = StyleName; }
	virtual std::string GetName() const override { return Name; }

protected:
	UIMaterial::SharedPtr Material;
	std::string Name;
};

class IUIFontMaterialStyle : public IUIMaterialStyle, public std::enable_shared_from_this<IUIFontMaterialStyle>
{
	cbClassBody(cbClassDefaultProtectedConstructor, IUIFontMaterialStyle, IUIMaterialStyle)
public:
};

class UIFontMaterialStyle : public IUIFontMaterialStyle
{
	cbClassBody(cbClassConstructor, UIFontMaterialStyle, IUIFontMaterialStyle)
public:
	UIFontMaterialStyle(const std::string& StyleName)
		: Name(StyleName)
	{}
	UIFontMaterialStyle(const std::string& StyleName, const std::string& FontName, const UIMaterial::SharedPtr& Mat)
		: Material(Mat)
		, FontFamilyName(FontName)
		, Name(StyleName)
	{}
	
	virtual ~UIFontMaterialStyle()
	{
		Material = nullptr;
	}

	template <typename T>
	inline T GetMaterialAs() const
	{
		return static_cast<T>(GetMaterial());
	}

	virtual UIMaterial* GetMaterial(unsigned short StyleState = 0) const override { return Material.get(); }
	inline void SetMaterial(const UIMaterial::SharedPtr& Mat) { Material = Mat; }

	inline std::string GetFontFamilyName() const { return FontFamilyName; }

	virtual void SetName(const std::string& StyleName) override { Name = StyleName; }
	virtual std::string GetName() const override { return Name; }

protected:
	UIMaterial::SharedPtr Material;
	std::string FontFamilyName;
	std::string Name;
};

class IUIButtonMaterialStyle : public IUIMaterialStyle, public std::enable_shared_from_this<IUIButtonMaterialStyle>
{
	cbClassBody(cbClassDefaultProtectedConstructor, IUIButtonMaterialStyle, IUIMaterialStyle)
public:
	typedef cbgui::eButtonState MaterialState;
public:
};

class UIButtonMaterialStyle : public IUIButtonMaterialStyle
{
	cbClassBody(cbClassConstructor, UIButtonMaterialStyle, IUIButtonMaterialStyle)
public:
	typedef cbgui::eButtonState MaterialState;

public:
	UIButtonMaterialStyle(const std::string& StyleName)
		: Name(StyleName)
		, Default(nullptr)
		, OnPressed(nullptr)
		, OnHover(nullptr)
		, Disabled(nullptr)
	{}
	
	virtual ~UIButtonMaterialStyle()
	{
		Default = nullptr;
		OnPressed = nullptr;
		OnHover = nullptr;
		Disabled = nullptr;
	}

	virtual std::vector<UIMaterial*> GetAllMaterials()
	{
		std::vector<UIMaterial*> Mats;
		Mats.push_back(Default.get());
		Mats.push_back(OnPressed.get());
		Mats.push_back(OnHover.get());
		Mats.push_back(Disabled.get());
		return Mats;
	}

	virtual UIMaterial* GetMaterial(unsigned short StyleState = 0) const
	{
		using namespace cbgui;

		const eButtonState& State = static_cast<eButtonState>(StyleState);
		switch (State)
		{
		case eButtonState::Default:
			return Default.get();
			break;
		case eButtonState::Disabled:
			return Disabled.get();
			break;
		case eButtonState::Hovered:
			return OnHover.get();
			break;
		case eButtonState::Pressed:
			return OnPressed.get();
			break;
		}
		return nullptr;
	}

	virtual void SetDefaultMaterial(const UIMaterial::SharedPtr& pMat)
	{
		Default = pMat;
	}
	virtual void SetOnPressedMaterial(const UIMaterial::SharedPtr& pMat)
	{
		OnPressed = pMat;
	}
	virtual void SetOnHoverMaterial(const UIMaterial::SharedPtr& pMat)
	{
		OnHover = pMat;
	}
	virtual void SetOnDisabledMaterial(const UIMaterial::SharedPtr& pMat)
	{
		Disabled = pMat;
	}

	inline UIMaterial* GetDefault() { return Default.get(); }
	inline UIMaterial* GetOnPressed() { return OnPressed.get(); }
	inline UIMaterial* GetOnHover() { return OnHover.get(); }
	inline UIMaterial* GetDisabled() { return Disabled.get(); }

	virtual void SetName(const std::string& StyleName) override { Name = StyleName; }
	virtual std::string GetName() const override { return Name; }

protected:
	UIMaterial::SharedPtr Default;
	UIMaterial::SharedPtr OnPressed;
	UIMaterial::SharedPtr OnHover;
	UIMaterial::SharedPtr Disabled;

	std::string Name;
};

class IUICheckBoxMaterialStyle : public IUIMaterialStyle, public std::enable_shared_from_this<IUICheckBoxMaterialStyle>
{
	cbClassBody(cbClassDefaultProtectedConstructor, IUICheckBoxMaterialStyle, IUIMaterialStyle)
public:
	typedef cbgui::eCheckBoxState MaterialState;
public:
};

class UICheckBoxMaterialStyle : public IUICheckBoxMaterialStyle
{
	cbClassBody(cbClassConstructor, UICheckBoxMaterialStyle, IUICheckBoxMaterialStyle)
public:
	UICheckBoxMaterialStyle(const std::string& StyleName)
		: Name(StyleName)
		, Unchecked(nullptr)
		, UncheckedHovered(nullptr)
		, UncheckedPressed(nullptr)
		, Checked(nullptr)
		, CheckedHovered(nullptr)
		, CheckedPressed(nullptr)
		, Undetermined(nullptr)
		, UndeterminedHovered(nullptr)
		, UndeterminedPressed(nullptr)
		, CheckedDisabled(nullptr)
		, UncheckedDisabled(nullptr)
		, UndeterminedDisabled(nullptr)
	{}

	virtual ~UICheckBoxMaterialStyle()
	{
		Unchecked = nullptr;
		UncheckedHovered = nullptr;
		UncheckedPressed = nullptr;
		Checked = nullptr;
		CheckedHovered = nullptr;
		CheckedPressed = nullptr;
		Undetermined = nullptr;
		UndeterminedHovered = nullptr;
		UndeterminedPressed = nullptr;
		CheckedDisabled = nullptr;
		UncheckedDisabled = nullptr;
		UndeterminedDisabled = nullptr;
	}

	virtual std::vector<UIMaterial*> GetAllMaterials()
	{
		std::vector<UIMaterial*> Mats;
		Mats.push_back(Unchecked.get());
		Mats.push_back(UncheckedHovered.get());
		Mats.push_back(UncheckedPressed.get());
		Mats.push_back(Checked.get());
		Mats.push_back(CheckedHovered.get());
		Mats.push_back(CheckedPressed.get());
		Mats.push_back(Undetermined.get());
		Mats.push_back(UndeterminedHovered.get());
		Mats.push_back(UndeterminedPressed.get());
		if (CheckedDisabled)
			Mats.push_back(CheckedDisabled.get());
		if (UncheckedDisabled)
			Mats.push_back(UncheckedDisabled.get());
		if (UndeterminedDisabled)
			Mats.push_back(UndeterminedDisabled.get());
		return Mats;
	}

	virtual UIMaterial* GetMaterial(unsigned short StyleState = 0) const
	{
		using namespace cbgui;

		const eCheckBoxState& State = static_cast<eCheckBoxState>(StyleState);
		switch (State)
		{
		case eCheckBoxState::UncheckedPressed:
			return GetUncheckedPressed();
			break;
		case eCheckBoxState::UncheckedHovered:
			return GetUncheckedHovered();
			break;
		case eCheckBoxState::Unchecked:
			return GetUnchecked();
			break;
		case eCheckBoxState::CheckedPressed:
			return GetCheckedPressed();
			break;
		case eCheckBoxState::CheckedHovered:
			return GetCheckedHovered();
			break;
		case eCheckBoxState::Checked:
			return GetChecked();
			break;
		case eCheckBoxState::UndeterminedPressed:
			return GetUndeterminedPressed();
			break;
		case eCheckBoxState::UndeterminedHovered:
			return GetUndeterminedHovered();
			break;
		case eCheckBoxState::Undetermined:
			return GetUndetermined();
			break;
		case eCheckBoxState::UncheckedDisabled:
		{
			const auto& Mat = GetUncheckedDisabled();
			if (Mat)
				return GetUncheckedDisabled();
			else
				return GetUnchecked();
		}
		break;
		case eCheckBoxState::CheckedDisabled:
		{
			const auto& Mat = GetCheckedDisabled();
			if (Mat)
				return GetCheckedDisabled();
			else
				return GetChecked();
		}
		break;
		case eCheckBoxState::UndeterminedDisabled:
		{
			const auto& Mat = GetUndeterminedDisabled();
			if (Mat)
				return GetUndeterminedDisabled();
			else
				return GetUndetermined();
		}
		break;
		}
		return nullptr;
	}

	virtual void SetUncheckedMaterial(const UIMaterial::SharedPtr& pMat)
	{
		Unchecked = pMat;
	}
	virtual void SetUncheckedMaterialHovered(const UIMaterial::SharedPtr& pMat)
	{
		UncheckedHovered = pMat;
	}
	virtual void SetUncheckedMaterialPressed(const UIMaterial::SharedPtr& pMat)
	{
		UncheckedPressed = pMat;
	}
	virtual void SetCheckedMaterial(const UIMaterial::SharedPtr& pMat)
	{
		Checked = pMat;
	}
	virtual void SetCheckedMaterialHovered(const UIMaterial::SharedPtr& pMat)
	{
		CheckedHovered = pMat;
	}
	virtual void SetCheckedMaterialPressed(const UIMaterial::SharedPtr& pMat)
	{
		CheckedPressed = pMat;
	}
	virtual void SetUndeterminedMaterial(const UIMaterial::SharedPtr& pMat)
	{
		Undetermined = pMat;
	}
	virtual void SetUndeterminedMaterialHovered(const UIMaterial::SharedPtr& pMat)
	{
		UndeterminedHovered = pMat;
	}
	virtual void SetUndeterminedMaterialPressed(const UIMaterial::SharedPtr& pMat)
	{
		UndeterminedPressed = pMat;
	}
	/*
	* Optional
	*/
	virtual void SetCheckedDisabledMaterial(const UIMaterial::SharedPtr& pMat)
	{
		CheckedDisabled = pMat;
	}
	/*
	* Optional
	*/
	virtual void SetUncheckedDisabledMaterial(const UIMaterial::SharedPtr& pMat)
	{
		UncheckedDisabled = pMat;
	}
	/*
	* Optional
	*/
	virtual void SetUndeterminedDisabledMaterial(const UIMaterial::SharedPtr& pMat)
	{
		UndeterminedDisabled = pMat;
	}

	inline UIMaterial* GetUnchecked() const { return Unchecked.get(); }
	inline UIMaterial* GetUncheckedHovered() const { return UncheckedHovered.get(); }
	inline UIMaterial* GetUncheckedPressed() const { return UncheckedPressed.get(); }
	inline UIMaterial* GetChecked() const { return Checked.get(); }
	inline UIMaterial* GetCheckedHovered() const { return CheckedHovered.get(); }
	inline UIMaterial* GetCheckedPressed() const { return CheckedPressed.get(); }
	inline UIMaterial* GetUndetermined() const { return Undetermined.get(); }
	inline UIMaterial* GetUndeterminedHovered() const { return UndeterminedHovered.get(); }
	inline UIMaterial* GetUndeterminedPressed()const { return UndeterminedPressed.get(); }
	inline UIMaterial* GetCheckedDisabled() const { return CheckedDisabled.get(); }
	inline UIMaterial* GetUncheckedDisabled() const { return UncheckedDisabled.get(); }
	inline UIMaterial* GetUndeterminedDisabled() const { return UndeterminedDisabled.get(); }

	virtual void SetName(const std::string& StyleName) override { Name = StyleName; }
	virtual std::string GetName() const override { return Name; }

protected:
	UIMaterial::SharedPtr Unchecked;
	UIMaterial::SharedPtr UncheckedHovered;
	UIMaterial::SharedPtr UncheckedPressed;
	UIMaterial::SharedPtr Checked;
	UIMaterial::SharedPtr CheckedHovered;
	UIMaterial::SharedPtr CheckedPressed;
	UIMaterial::SharedPtr Undetermined;
	UIMaterial::SharedPtr UndeterminedHovered;
	UIMaterial::SharedPtr UndeterminedPressed;
	UIMaterial::SharedPtr CheckedDisabled;
	UIMaterial::SharedPtr UncheckedDisabled;
	UIMaterial::SharedPtr UndeterminedDisabled;

	std::string Name;
};
