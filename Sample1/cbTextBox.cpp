/* ---------------------------------------------------------------------------------------
* MIT License
*
* Copyright (c) 2022 Davut Coþkun.
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

#include "pch.h"
#include "cbTextBox.h"

using namespace cbgui;

void cbTextBox::cbTextSlot::ReplaceContent(const cbWidget::SharedPtr& pContent)
{
	if (auto Text = std::dynamic_pointer_cast<cbString>(pContent))
	{
		if (Content)
		{
			Content = nullptr;
		}
		Content = Text;
		Content->AttachToSlot(this);
		GetOwner<cbTextBox>()->SlotContentReplaced(this);
	}
}

std::vector<cbVector4> cbTextBox::TextHighlight::GenerateVertices() const
{
	std::vector<cbVector4> Vertices;

	for (const auto& Rect : Highlights)
	{
		const auto Verts = cbGeometryFactory::Create4DPlaneVerticesFromRect(Rect);
		Vertices.insert(Vertices.end(), Verts.begin(), Verts.end());
	}

	return Vertices;
}

std::vector<cbVector> cbTextBox::TextHighlight::GenerateTextureCoordinate() const
{
	std::vector<cbVector> TCs;
	for (std::size_t i = 0; i < Highlights.size(); i++)
	{
		const auto TC = cbGeometryFactory::GeneratePlaneTextureCoordinate();
		TCs.insert(TCs.end(), TC.begin(), TC.end());
	}
	return TCs;
}

std::vector<std::uint32_t> cbTextBox::TextHighlight::GenerateIndices() const
{
	return cbGeometryFactory::GeneratePlaneIndices((std::uint32_t)Highlights.size());
}

cbTextBox::cbTextBox()
	: Super()
	, Transform(cbTransform(cbDimension(100.0f, 30.0f)))
	, TextSlot(cbTextSlot::Create(this))
	, SelectedIndex(std::nullopt)
	, Pressed(false)
	, Cursor(cbCursor::Create(this))
	, Highlight(TextHighlight::Create(this))
	, bEditMode(false)
	, TextBoxType(eTextBoxType::All)
	, Limit(std::nullopt)
{
	TextSlot->Inserted();
}

cbTextBox::cbTextBox(const std::u32string& Text, const cbTextDesc& TextDesc)
	: Super()
	, Transform(cbTransform(cbDimension(100.0f, 30.0f)))
	, TextSlot(cbTextSlot::Create(this, Text, TextDesc))
	, SelectedIndex(std::nullopt)
	, Pressed(false)
	, Cursor(cbCursor::Create(this))
	, Highlight(TextHighlight::Create(this))
	, bEditMode(false)
	, TextBoxType(eTextBoxType::All)
	, Limit(std::nullopt)
{
	TextSlot->Inserted();
}

cbTextBox::~cbTextBox()
{
	TextSlot = nullptr;
	Cursor = nullptr;
	Highlight = nullptr;
}

void cbTextBox::SetXY(std::optional<float> X, std::optional<float> Y, bool Force)
{
	if (X.has_value() && Y.has_value())
	{
		if ((HasOwner() || IsAlignedToCanvas()))
		{
			if (Force)
			{
				if (GetVerticalAlignment() != eVerticalAlignment::Align_NONE && GetHorizontalAlignment() != eHorizontalAlignment::Align_NONE)
					return;
			}
			else
			{
				return;
			}
		}
		if (Transform.SetLocation(cbVector(*X, *Y)))
		{
			UpdateSlotAlignments();
			Cursor->UpdateAlignments();
			Highlight->UpdateAlignments();
			NotifyCanvas_WidgetUpdated();
		}
	}
	else if (X.has_value())
	{
		if ((HasOwner() || IsAlignedToCanvas()))
		{
			if (Force)
			{
				if (GetHorizontalAlignment() != eHorizontalAlignment::Align_NONE)
					return;
			}
			else
			{
				return;
			}
		}
		if (Transform.SetPositionX(*X))
		{
			UpdateSlotHorizontalAlignment();
			Cursor->UpdateHorizontalAlignment();
			Highlight->UpdateHorizontalAlignment();
			NotifyCanvas_WidgetUpdated();
		}
	}
	else if (Y.has_value())
	{
		if ((HasOwner() || IsAlignedToCanvas()))
		{
			if (Force)
			{
				if (GetVerticalAlignment() != eVerticalAlignment::Align_NONE)
					return;
			}
			else
			{
				return;
			}
		}
		if (Transform.SetPositionY(*Y))
		{
			UpdateSlotVerticalAlignment();
			Cursor->UpdateVerticalAlignment();
			Highlight->UpdateVerticalAlignment();
			NotifyCanvas_WidgetUpdated();
		}
	}
}

void cbTextBox::SetWidthHeight(std::optional<float> Width, std::optional<float> Height)
{
	if (Width.has_value() && Height.has_value())
	{
		if (Transform.SetDimension(cbDimension(*Width, *Height)))
		{
			if (HasOwner())
				DimensionUpdated();
			else if (IsAlignedToCanvas())
				AlignToCanvas();
			else
			{
				UpdateSlotAlignments();
				Cursor->UpdateAlignments();
				Highlight->UpdateAlignments();
				NotifyCanvas_WidgetUpdated();
			}
		}
	}
	else if (Width.has_value())
	{
		if (Transform.SetWidth(*Width))
		{
			if (HasOwner())
				DimensionUpdated();
			else if (IsAlignedToCanvas())
				AlignToCanvas();
			else
			{
				UpdateSlotHorizontalAlignment();
				Cursor->UpdateHorizontalAlignment();
				Highlight->UpdateHorizontalAlignment();
				NotifyCanvas_WidgetUpdated();
			}
		}
	}
	else if (Height.has_value())
	{
		if (Transform.SetHeight(*Height))
		{
			if (HasOwner())
				DimensionUpdated();
			else if (IsAlignedToCanvas())
				AlignToCanvas();
			else
			{
				UpdateSlotVerticalAlignment();
				Cursor->UpdateVerticalAlignment();
				Highlight->UpdateVerticalAlignment();
				NotifyCanvas_WidgetUpdated();
			}
		}
	}
}

void cbTextBox::SetRotation(const float Rotation)
{
	if (Transform.Rotate2D(Rotation))
	{
		if (TextSlot)
			TextSlot->UpdateRotation();
		if (Cursor)
			Cursor->UpdateRotation();
		if (Highlight)
			Highlight->ClearHighlights();
		NotifyCanvas_WidgetUpdated();
	}
}

void cbTextBox::SetPadding(const cbMargin& Padding)
{
	if (Transform.SetPadding(Padding))
	{
		if (HasOwner())
			DimensionUpdated();
		else if (IsAlignedToCanvas())
			AlignToCanvas();
	}
}

void cbTextBox::UpdateStatus()
{
	if (!TextSlot)
		return;

	TextSlot->UpdateStatus();
}

std::vector<cbWidgetObj*> cbTextBox::GetAllChildren() const
{
	if (TextSlot)
		return std::vector<cbWidgetObj*>{ Highlight.get(), TextSlot.get(), Cursor.get() };
	return std::vector<cbWidgetObj*>{ };
}

void cbTextBox::OnSlotVisibilityChanged(cbSlot* Slot)
{
}

void cbTextBox::OnSlotDimensionUpdated(cbSlot* Slot)
{
}

void cbTextBox::SlotContentReplaced(cbTextSlot* Slot)
{
	Slot->UpdateStatus();
	Slot->UpdateRotation();

	WrapOrUpdateAlignments();
}

bool cbTextBox::OnRemoveSlot(cbSlot* pSlot)
{
	if (!TextSlot)
		return false;

	if (cbICanvas* Canvas = GetCanvas())
		Canvas->SlotRemoved(this, TextSlot.get());
	TextSlot = nullptr;

	return true;
}

cbSlot* cbTextBox::GetOverlappingSlot(const cbBounds& Bounds) const
{
	if (TextSlot)
		if (TextSlot->Intersect(Bounds))
			return TextSlot.get();
	return nullptr;
}

const std::u32string cbTextBox::GetText() const
{
	const auto& pText = TextSlot->GetText();
	return pText->GetText();
}

void cbTextBox::SetText(const std::u32string& inText, const std::optional<cbTextDesc> Desc)
{
	const auto& pText = TextSlot->GetText();
	pText->SetText(inText, Desc);

	if (Cursor->IsEnabled())
	{
		SelectedIndex = 0;
		Cursor->UpdateAlignments();
		Highlight->ClearHighlights();
	}
	WrapOrUpdateAlignments();
}

void cbTextBox::SetText(const std::string& Text, const std::optional<cbTextDesc> Desc)
{
	const auto& pText = TextSlot->GetText();
	if (Desc.has_value())
		pText->SetString((Text), Desc.value());
	else
		pText->SetString((Text));

	if (Cursor->IsEnabled())
	{
		SelectedIndex = 0;
		Cursor->UpdateAlignments();
		Highlight->ClearHighlights();
	}
	WrapOrUpdateAlignments();
}

void cbTextBox::SetText(const cbString::SharedPtr& Text)
{
	TextSlot->ReplaceContent(Text);

	if (Cursor->IsEnabled())
	{
		SelectedIndex = 0;
		Cursor->UpdateAlignments();
		Highlight->ClearHighlights();
	}
	WrapOrUpdateAlignments();
}

void cbTextBox::SetVertexColorStyle(const cbVertexColorStyle& style)
{
	const auto& pText = TextSlot->GetText();
	pText->SetVertexColorStyle(style);
}

void cbTextBox::AppendText(const std::u32string& Text)
{
	const auto& pText = TextSlot->GetText();
	pText->AppendText(Text);

	if (Cursor->IsEnabled())
	{
		SelectedIndex = 0;
		Cursor->UpdateAlignments();
		Highlight->ClearHighlights();
	}
	WrapOrUpdateAlignments();
}

void cbTextBox::AddChar(const char32_t pChar, const std::optional<std::size_t> Index, bool Filtered)
{
	const auto& pText = TextSlot->GetText();
	pText->AddChar(pChar, Index, Filtered);

	if (Cursor->IsEnabled())
	{
		SelectedIndex = 0;
		Cursor->UpdateAlignments();
		Highlight->ClearHighlights();
	}
	WrapOrUpdateAlignments();
}

void cbTextBox::AddChar(const char32_t* pChar, const std::optional<std::size_t> Index, bool Filtered)
{
	AddChar(*pChar, Index, Filtered);
}

void cbTextBox::SetFontType(const eFontType& FontType)
{
	const auto& pText = TextSlot->GetText();
	pText->SetFontType(FontType);

	if (Cursor->IsEnabled())
	{
		SelectedIndex = 0;
		Cursor->UpdateAlignments();
		Highlight->ClearHighlights();
	}
	WrapOrUpdateAlignments();
}
void cbTextBox::SetFontSize(const std::size_t& Size)
{
	const auto& pText = TextSlot->GetText();
	pText->SetFontSize(Size);

	if (Cursor->IsEnabled())
	{
		SelectedIndex = 0;
		Cursor->UpdateAlignments();
		Highlight->ClearHighlights();
	}
	WrapOrUpdateAlignments();
}
void cbTextBox::SetTextJustify(eTextJustify Justify)
{
	const auto& pText = TextSlot->GetText();
	pText->SetTextJustify(Justify);

	if (Cursor->IsEnabled())
	{
		SelectedIndex = 0;
		Cursor->UpdateAlignments();
		Highlight->ClearHighlights();
	}
	WrapOrUpdateAlignments();
}
std::size_t cbTextBox::GetTextSize(bool Filtered) const
{
	const auto& pText = TextSlot->GetText();
	return pText->GetTextSize(Filtered);
}
bool cbTextBox::RemoveChar(std::size_t InIndex)
{
	const auto& pText = TextSlot->GetText();
	auto Res = pText->RemoveChar(InIndex);

	if (Cursor->IsEnabled())
	{
		SelectedIndex = 0;
		Cursor->UpdateAlignments();
		Highlight->ClearHighlights();
	}
	WrapOrUpdateAlignments();

	return Res;
}
bool cbTextBox::RemoveChars(const std::vector<std::size_t>& Indices)
{
	const auto& pText = TextSlot->GetText();
	auto Res = pText->RemoveChars(Indices);

	if (Cursor->IsEnabled())
	{
		SelectedIndex = 0;
		Cursor->UpdateAlignments();
		Highlight->ClearHighlights();
	}
	WrapOrUpdateAlignments();

	return Res;
}
void cbTextBox::ClearText()
{
	const auto& pText = TextSlot->GetText();
	pText->Clear();

	if (Cursor->IsEnabled())
	{
		SelectedIndex = 0;
		Cursor->UpdateAlignments();
		Highlight->ClearHighlights();
	}
	WrapOrUpdateAlignments();
}

std::vector<cbTextStyle> cbTextBox::GetTextStyles() const
{
	const auto& pText = TextSlot->GetText();
	return pText->GetTextStyles();
}
void cbTextBox::AddTextStyle(const cbTextStyle& Style)
{
	const auto& pText = TextSlot->GetText();
	pText->AddTextStyle(Style);

	if (Cursor->IsEnabled())
	{
		SelectedIndex = 0;
		Cursor->UpdateAlignments();
		Highlight->ClearHighlights();
	}
	WrapOrUpdateAlignments();
}
void cbTextBox::RemoveTextStyle(const std::size_t& Index)
{
	const auto& pText = TextSlot->GetText();
	pText->RemoveTextStyle(Index);

	if (Cursor->IsEnabled())
	{
		SelectedIndex = 0;
		Cursor->UpdateAlignments();
		Highlight->ClearHighlights();
	}
	WrapOrUpdateAlignments();
}
void cbTextBox::SetTextStylingEnabled(const bool& Value)
{
	const auto& pText = TextSlot->GetText();
	pText->SetTextStylingEnabled(Value);

	if (Cursor->IsEnabled())
	{
		SelectedIndex = 0;
		Cursor->UpdateAlignments();
		Highlight->ClearHighlights();
	}
	WrapOrUpdateAlignments();
}
void cbTextBox::SetAscenderDescenderPowerOfTwo(const bool& Value)
{
	const auto& pText = TextSlot->GetText();
	pText->SetAscenderDescenderPowerOfTwo(Value);

	if (Cursor->IsEnabled())
	{
		SelectedIndex = 0;
		Cursor->UpdateAlignments();
		Highlight->ClearHighlights();
	}
	WrapOrUpdateAlignments();
}

void cbTextBox::SetAutoWrap(const bool& Value)
{
	const auto& pText = TextSlot->GetText();
	pText->SetAutoWrap(Value);

	if (Cursor->IsEnabled())
	{
		SelectedIndex = 0;
		Cursor->UpdateAlignments();
		Highlight->ClearHighlights();
	}
	WrapOrUpdateAlignments();
}
bool cbTextBox::IsAutoWrapEnabled() const
{
	const auto& pText = TextSlot->GetText();
	return pText->IsAutoWrapEnabled();
}
void cbTextBox::SetCustomWrapWidth(const std::optional<float> value)
{
	const auto& pText = TextSlot->GetText();
	pText->SetCustomWrapWidth(value);

	if (Cursor->IsEnabled())
	{
		SelectedIndex = 0;
		Cursor->UpdateAlignments();
		Highlight->ClearHighlights();
	}
	WrapOrUpdateAlignments();
}
float cbTextBox::GetAutoWrapWidth() const
{
	const auto& pText = TextSlot->GetText();
	return pText->GetAutoWrapWidth();
}

void cbTextBox::SetTextBoxType(eTextBoxType Type)
{
	TextBoxType = Type;
	const auto& pText = TextSlot->GetText();
	pText->Clear();

	if (Cursor->IsEnabled())
	{
		SelectedIndex = 0;
		Cursor->UpdateAlignments();
		Highlight->ClearHighlights();
	}
}
void cbTextBox::CharacterLimit(std::size_t inLimit)
{
	Limit = inLimit;

	const auto& pText = TextSlot->GetText();
GoBack:
	if (Limit < pText->GetTextSize())
	{
		pText->RemoveLastChar();
		goto GoBack;
	}

	if (Cursor->IsEnabled())
	{
		SelectedIndex = 0;
		Cursor->UpdateAlignments();
		Highlight->ClearHighlights();
	}
}

void cbTextBox::EnableEditMode()
{
	bEditMode = true;
	Cursor->Enable();
	Cursor->Hidden(false);
	Highlight->Enable();
	Highlight->Hidden(false);

	if (Cursor->IsEnabled())
	{
		SelectedIndex = 0;
		Cursor->UpdateAlignments();
		Highlight->ClearHighlights();
	}
}

void cbTextBox::DisableEditMode()
{
	bEditMode = true;
	Cursor->Disable();
	Cursor->Hidden(true);
	Highlight->Disable();
	Highlight->Hidden(true);

	if (Cursor->IsEnabled())
	{
		SelectedIndex = 0;
		Cursor->UpdateAlignments();
		Highlight->ClearHighlights();
	}
}

void cbTextBox::BlockChar(char32_t Char)
{
	if (std::find(BlockChars.begin(), BlockChars.end(), Char) != BlockChars.end())
		return;

	BlockChars.push_back(Char);
}

void cbTextBox::UnblockChar(char32_t Char)
{
	auto it = std::find(BlockChars.begin(), BlockChars.end(), Char);
	if (it != BlockChars.end())
	{
		auto index = std::distance(BlockChars.begin(), it);
		BlockChars.erase(BlockChars.begin() + index);
	}
}

bool cbTextBox::IsCharacterBlocked(char32_t Char) const
{
	return std::find(BlockChars.begin(), BlockChars.end(), Char) != BlockChars.end();
}

double u16stod(std::u32string const& u16s)
{
	char buf[std::numeric_limits<double>::max_digits10 + 1];

	std::transform(std::begin(u16s), std::end(u16s), buf,
		[](char32_t c) { return char(c); });

	buf[u16s.size()] = '\0'; // terminator

	// some error checking here?
	return std::strtod(buf, NULL);
}

void cbTextBox::InputCallback()
{
	if (fStringInputCallback)
		fStringInputCallback(GetText());

	if (fInputCallback)
	{
		auto STR = GetText();
		fInputCallback(u16stod(STR));
	}
}

std::int32_t cbTextBox::GetInteger() const
{
	auto STR = GetText();
	return static_cast<std::int32_t>(u16stod(STR));
}

float cbTextBox::GetFloat() const
{
	auto STR = GetText();
	return (float)u16stod(STR);
}

bool cbTextBox::OnKeyDown(std::uint64_t KeyCode)
{
	if (!IsFocused() || !IsEnabled())
		return false;

	const char32_t C16((char32_t)KeyCode);

	if (C16 == 8)
	{
		if (SelectedIndex.has_value())
		{
			if (SelectedIndex.value() > 0)
			{
				if (Highlight->IsHighlighted())
				{
					std::vector<std::size_t> Indices;
					for (std::size_t i = Highlight->HighlightStartIndex.value(); i <= Highlight->HighlightEndIndex.value(); i++)
					{
						Indices.push_back(i);
						SelectedIndex = Highlight->HighlightStartIndex == SelectedIndex.value() ? SelectedIndex.value() : SelectedIndex.value() - 1;
					}
					TextSlot->GetText()->RemoveChars(Indices);
					Highlight->ClearHighlights();
				}

				auto GetIndex = [&](bool subtraction = true) -> std::size_t
				{
					std::size_t Index = subtraction ? SelectedIndex.value() - 1 : SelectedIndex.value() + 1;
					while (Index >= 0)
					{
						std::optional<cbCharacterData> CharacterData = TextSlot->GetText()->GetCharacterData(Index);
						if (!CharacterData.has_value())
							break;
						if (CharacterData.value().Ignored && !CharacterData.value().IsNewLine())
							Index = subtraction ? Index - 1 : Index + 1;
						else
							break;
					}
					return Index;
				};

				SelectedIndex = (std::uint32_t)GetIndex();
				TextSlot->GetText()->RemoveChar(SelectedIndex.value());

				std::optional<cbCharacterData> Data = TextSlot->GetText()->GetCharacterData(SelectedIndex.value());
				if (Data.has_value())
				{
					if (Data.value().Ignored && !Data.value().IsNewLine())
					{
						SelectedIndex = (std::uint32_t)GetIndex(false);
					}
				}
				WrapOrUpdateAlignments();

				if (Cursor->IsEnabled())
					Cursor->UpdateAlignments();
			}
		}
		else
		{
			TextSlot->GetText()->RemoveLastChar();
			WrapOrUpdateAlignments();
		}
		return true;
	}
	else if (C16 == 13)
	{
		if (!IsCharacterBlocked(C16))
		{
			const char32_t NewLine('\n');
			TextSlot->GetText()->AddChar(NewLine, SelectedIndex);
			if (Limit.has_value())
			{
				if (Limit.value() <= GetTextSize())
					return false;
			}
			if (SelectedIndex.has_value())
				SelectedIndex = SelectedIndex.value() + 1;
		}

		if (Highlight->IsHighlighted())
		{
			std::vector<std::size_t> Indices;
			for (std::size_t i = Highlight->HighlightStartIndex.value(); i <= Highlight->HighlightEndIndex.value(); i++)
			{
				Indices.push_back(i);
				SelectedIndex = Highlight->HighlightStartIndex == SelectedIndex.value() ? SelectedIndex.value() : SelectedIndex.value() - 1;
			}
			TextSlot->GetText()->RemoveChars(Indices);
			Highlight->ClearHighlights();
		}

		if (TextBoxType == eTextBoxType::Integer)
		{
			if (GetTextSize() == 0)
			{
				AddChar(U"0");
				SelectedIndex = 1;
			}

		}
		else if (TextBoxType == eTextBoxType::Float)
		{
			if (GetTextSize() == 0)
			{
				AddChar(U"0");
				AddChar(U".");
				AddChar(U"0");
			}
			else if (TextSlot->GetText()->GetCharacterCount(46) == 0)
			{
				AddChar(U".");
				AddChar(U"0");
			}
			else
			{
				auto STR = GetText();
				std::size_t Loc = STR.find(U".");
				if (Loc == (GetTextSize() - 1))
					AddChar(U"0");
			}
		}

		InputCallback();
		WrapOrUpdateAlignments();

		if (Cursor->IsEnabled())
			Cursor->UpdateAlignments();

		return true;
	}

	if (IsCharacterBlocked(C16))
		return false;

	if (TextBoxType == eTextBoxType::Integer && !isdigit((int)KeyCode))
		return false;
	else if (TextBoxType == eTextBoxType::Float && (KeyCode != 46 && !isdigit((int)KeyCode)) || (KeyCode == 46 && TextSlot->GetText()->GetCharacterCount(46) >= 1))
		return false;

	if (Limit.has_value())
	{
		if (TextBoxType != eTextBoxType::Float)
		{
			if (Limit.value() <= GetTextSize())
				return false;
		}
		else
		{
			if (Limit.value() > GetTextSize())
			{
				if (C16 == 46)
				{
					if (TextSlot->GetText()->GetCharacterCount(46) == 1)
						return false;
				}
				else if (TextSlot->GetText()->GetCharacterCount(46) == 0)
				{
					if (GetTextSize() >= Limit.value() - 2)
					{
						return false;
					}
				}
				else if (TextSlot->GetText()->GetCharacterCount(46) == 1)
				{
					if (GetTextSize() >= Limit.value())
					{
						return false;
					}
				}
			}
			else
			{
				return false;
			}
		}
	}

	//if ((TextBoxType == eTextBoxType::Float && (KeyCode != 46 && TextSlot->GetText()->GetCharacterCount(46) == 0)))
	//	return false;

	if (Highlight->IsHighlighted())
	{
		std::vector<std::size_t> Indices;
		for (std::size_t i = Highlight->HighlightStartIndex.value(); i <= Highlight->HighlightEndIndex.value(); i++)
		{
			Indices.push_back(i);
			SelectedIndex = Highlight->HighlightStartIndex == SelectedIndex.value() ? SelectedIndex.value() : *SelectedIndex > 0 ? SelectedIndex.value() - 1 : SelectedIndex.value();
		}
		TextSlot->GetText()->RemoveChars(Indices);
		Highlight->ClearHighlights();
	}

	TextSlot->GetText()->AddChar(C16, SelectedIndex, true);
	if (SelectedIndex.has_value())
		SelectedIndex = SelectedIndex.value() + 1;

	WrapOrUpdateAlignments();

	if (Cursor->IsEnabled())
		Cursor->UpdateAlignments();

	return true;
}

bool cbTextBox::OnMouseEnter(const cbMouseInput& Mouse)
{
	if (!IsFocusable() || !IsEnabled())
		return false;

	if (!IsInside(Mouse.MouseLocation))
		return false;

	if (IsItCulled())
		return false;

	if (!IsFocused())
		SetFocus(true);

	return true;
}

bool cbTextBox::OnMouseMove(const cbMouseInput& Mouse)
{
	return false;
}

bool cbTextBox::OnMouseButtonUp(const cbMouseInput& Mouse)
{
	if (!IsFocused() || !IsEnabled())
		return false;

	Pressed = false;

	return true;
}

bool cbTextBox::OnMouseButtonDown(const cbMouseInput& Mouse)
{
	if (!IsFocused() || !IsEnabled() || Pressed)
		return false;

	PastSelectedIndex = std::nullopt;

	if (!IsInside(Mouse.MouseLocation))
	{
		if (Cursor->IsEnabled())
		{
			SelectedIndex = std::nullopt;
			Cursor->UpdateAlignments();
			Cursor->Disable();
			Cursor->Hidden(true);

			if (IsFocused())
				SetFocus(false);
		}

		if (TextBoxType == eTextBoxType::Integer)
		{
			if (GetTextSize() == 0)
			{
				AddChar(U"0");
				SelectedIndex = 1;
			}

		}
		else if (TextBoxType == eTextBoxType::Float)
		{
			if (GetTextSize() == 0)
			{
				AddChar(U"0");
				AddChar(U".");
				AddChar(U"0");
			}
			else if (TextSlot->GetText()->GetCharacterCount(46) == 0)
			{
				AddChar(U".");
				AddChar(U"0");
			}
			else
			{
				auto STR = GetText();
				const std::size_t Loc = STR.find(U".");
				if (Loc == (GetTextSize() - 1))
					AddChar(U"0");
			}
		}

		InputCallback();

		return false;
	}

	if (SelectedIndex.has_value())
	{
		if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0)
		{
			PastSelectedIndex = SelectedIndex;
		}
		else
		{
			Highlight->ClearHighlights();
		}
	}

	Pressed = true;

	SelectedIndex = std::nullopt;

	std::size_t TotalChars = 0;
	eCursorPosition CursorPosition = eCursorPosition::Left;

	auto Text = TextSlot->GetText();
	for (std::size_t i = 0; i < Text->GetLineSize(); i++)
	{
		cbBounds LineBounds;
		std::vector<cbCharacterData> CharData;
		Text->GetLineData(i, LineBounds, CharData);

		if (LineBounds.IsInside(Mouse.MouseLocation))
		{
			//std::cout << "Line Index : " << i << std::endl;

			if (CharData.size() > 0)
			{
				std::size_t CharIndex = 0;
				for (const auto& Char : CharData)
				{
					if (Char.Bounds.GetCenter().X > Mouse.MouseLocation.X)
						break;

					//std::cout << "Char Index : " << CharIndex << std::endl;
					if (!Char.IsNewLine())
						SelectedIndex = std::uint32_t(TotalChars + CharIndex + 1);
					else
						SelectedIndex = std::uint32_t(TotalChars + CharIndex);
					//std::cout << "SelectedIndex : " << SelectedIndex.value() << std::endl;

					CursorPosition = eCursorPosition::Left;

					CharIndex++;
				}

				if (SelectedIndex.has_value())
				{
					if (Text->GetTextSize() == SelectedIndex.value())
						CursorPosition = eCursorPosition::Right;
				}
				else
				{
					CursorPosition = eCursorPosition::Left;
					SelectedIndex = (std::uint32_t)TotalChars;
				}
				break;
			}
			else
			{
				CursorPosition = eCursorPosition::Left;
				SelectedIndex = (std::uint32_t)TotalChars;
			}
		}
		TotalChars += CharData.size();
	}

	if (SelectedIndex.has_value())
	{
		if (!Cursor->IsEnabled())
			Cursor->Enable();
		Cursor->SetCursorPosition(CursorPosition);
		Cursor->UpdateVerticalAlignment();
	}

	UpdateHighlight();

	return true;
}

void cbTextBox::UpdateHighlight()
{
	if (PastSelectedIndex.has_value() && SelectedIndex.has_value())
	{
		Highlight->ClearHighlights();
		std::optional<cbCharacterData> PastChar_ = TextSlot->GetText()->GetCharacterData(PastSelectedIndex.value());
		std::optional<cbCharacterData> CurrentChar_ = TextSlot->GetText()->GetCharacterData(SelectedIndex.value() == 0 ? 0 : SelectedIndex.value() - 1);
		if (PastChar_.has_value() && CurrentChar_.has_value())
		{
			cbCharacterData PastChar = PastChar_.value();
			cbCharacterData CurrentChar = CurrentChar_.value();

			if (PastChar.LineOrder < CurrentChar.LineOrder)
			{
				for (std::size_t i = PastChar.LineOrder; i <= CurrentChar.LineOrder; i++)
				{
					std::optional<cbBounds> Line = TextSlot->GetText()->GetLineBounds(i, false);

					if (Line.has_value())
					{
						if (i == PastChar.LineOrder && i != CurrentChar.LineOrder)
						{
							Highlight->SetHighlight(PastChar.Bounds.Min.X, Line.value().Max.X, Line.value().Min.Y, Line.value().Max.Y);
							Highlight->HighlightStartIndex = PastChar.CharacterOrder;
						}
						else if (i != PastChar.LineOrder && i != CurrentChar.LineOrder)
						{
							Highlight->SetHighlight(Line.value().Min.X, Line.value().Max.X, Line.value().Min.Y, Line.value().Max.Y);
						}
						else if (i != PastChar.LineOrder && i == CurrentChar.LineOrder)
						{
							Highlight->SetHighlight(Line.value().Min.X, CurrentChar.Bounds.Max.X, Line.value().Min.Y, Line.value().Max.Y);
							Highlight->HighlightEndIndex = CurrentChar.CharacterOrder;
						}
					}
				}
			}
			else if (PastChar.LineOrder > CurrentChar.LineOrder)
			{
				for (std::size_t i = CurrentChar.LineOrder; i <= PastChar.LineOrder; i++)
				{
					std::optional<cbBounds> Line = TextSlot->GetText()->GetLineBounds(i, false);

					if (Line.has_value())
					{
						if (i == PastChar.LineOrder && i != CurrentChar.LineOrder)
						{
							std::optional<cbCharacterData> PastChar__ = TextSlot->GetText()->GetCharacterData(PastSelectedIndex.value() - 1);
							if (!PastChar__.has_value())
								break;
							cbCharacterData NewPastChar = PastChar__.value();
							Highlight->SetHighlight(Line.value().Min.X, NewPastChar.Bounds.Max.X, Line.value().Min.Y, Line.value().Max.Y);
							Highlight->HighlightEndIndex = NewPastChar.CharacterOrder;
						}
						else if (i != PastChar.LineOrder && i != CurrentChar.LineOrder)
						{
							Highlight->SetHighlight(Line.value().Min.X, Line.value().Max.X, Line.value().Min.Y, Line.value().Max.Y);
						}
						else if (i != PastChar.LineOrder && i == CurrentChar.LineOrder)
						{
							std::optional<cbCharacterData> CurrentChar__ = TextSlot->GetText()->GetCharacterData(SelectedIndex.value());
							if (!CurrentChar__.has_value())
								break;
							cbCharacterData NewCurrentChar = CurrentChar__.value();
							Highlight->SetHighlight(NewCurrentChar.Bounds.Min.X, Line.value().Max.X, Line.value().Min.Y, Line.value().Max.Y);
							Highlight->HighlightStartIndex = NewCurrentChar.CharacterOrder;
						}
					}
				}
			}
			else if (PastChar.LineOrder == CurrentChar.LineOrder)
			{
				std::optional<cbBounds> Line = TextSlot->GetText()->GetLineBounds(CurrentChar.LineOrder, false);

				if (Line.has_value())
				{
					if (PastChar.CharacterOrder <= CurrentChar.CharacterOrder)
					{
						Highlight->SetHighlight(PastChar.Bounds.Min.X, CurrentChar.Bounds.Max.X, Line.value().Min.Y, Line.value().Max.Y);
						Highlight->HighlightStartIndex = PastChar.CharacterOrder;
						Highlight->HighlightEndIndex = CurrentChar.CharacterOrder;
					}
					else if (PastChar.CharacterOrder > CurrentChar.CharacterOrder)
					{
						std::optional<cbCharacterData> PastChar__ = TextSlot->GetText()->GetCharacterData(PastSelectedIndex.value() - 1);
						if (!PastChar__.has_value())
							return;
						PastChar = PastChar__.value();
						std::optional<cbCharacterData> CurrentChar__ = TextSlot->GetText()->GetCharacterData(SelectedIndex.value());
						if (!CurrentChar__.has_value())
							return;
						CurrentChar = CurrentChar__.value();
						Highlight->SetHighlight(CurrentChar.Bounds.Min.X, PastChar.Bounds.Max.X, Line.value().Min.Y, Line.value().Max.Y);
						Highlight->HighlightStartIndex = CurrentChar.CharacterOrder;
						Highlight->HighlightEndIndex = PastChar.CharacterOrder;
					}
				}
			}
		}
	}
}

bool cbTextBox::OnMouseLeave(const cbMouseInput& Mouse)
{
	if (!IsFocusable() || !IsEnabled() || !IsFocused())
		return false;

	if (Cursor->IsEnabled())
		return false;

	if (IsFocused())
		SetFocus(false);

	return true;
}

void cbTextBox::WrapOrUpdateAlignments()
{
	if (IsItWrapped())
	{
		Wrap();
	}
	else
	{
		if (IsAlignedToCanvas())
			AlignToCanvas(true);
		else
			UpdateSlotAlignments();
	}
}

void cbTextBox::UpdateSlotVerticalAlignment()
{
	if (TextSlot)
		TextSlot->UpdateVerticalAlignment();

	Cursor->UpdateVerticalAlignment();
	Highlight->ClearHighlights();
}

void cbTextBox::UpdateSlotHorizontalAlignment()
{
	if (TextSlot)
		TextSlot->UpdateHorizontalAlignment();

	Cursor->UpdateHorizontalAlignment();
	Highlight->ClearHighlights();
}

std::optional<cbBounds> cbTextBox::GetCursorSelectedCharBounds() const
{
	if (SelectedIndex.has_value())
	{
		std::optional<cbCharacterData> Data = TextSlot->GetText()->GetCharacterData(SelectedIndex.value());
		if (Data.has_value())
			return Data.value().Bounds;
	}
	else
	{
		std::optional<cbCharacterData> Data = TextSlot->GetText()->GetCharacterData(TextSlot->GetText()->GetTextSize() - 1);
		if (Data.has_value())
			return Data.value().Bounds;
		else
			return GetBounds();
	}
	return std::nullopt;
}

std::optional<cbBounds> cbTextBox::GetCurrentSelectedLineBounds() const
{
	if (SelectedIndex.has_value())
	{
		std::optional<cbCharacterData> Data = TextSlot->GetText()->GetCharacterData(SelectedIndex.value());
		if (Data.has_value())
		{
			std::optional<cbBounds> Line = TextSlot->GetText()->GetLineBounds(Data.value().LineOrder);
			return Line;
		}
	}
	else
	{
		std::optional<cbCharacterData> Data = TextSlot->GetText()->GetCharacterData(TextSlot->GetText()->GetTextSize() - 1);
		if (Data.has_value())
		{
			std::optional<cbBounds> Line = TextSlot->GetText()->GetLineBounds(Data.value().LineOrder);
			return Line;
		}
		else
		{
			Cursor->Disable();
			Cursor->Hidden(true);
		}
	}
	return std::nullopt;
}

void cbTextBox::UpdateVerticalAlignment(const bool ForceAlign)
{
	if (HasOwner())
	{
		const float OwnerHeight = GetOwner()->GetHeight();
		const std::optional<const eVerticalAlignment> CustomAlignment = GetNonAlignedHeight() > OwnerHeight ? std::optional<eVerticalAlignment>(eVerticalAlignment::Align_Fill) : std::nullopt;

		if (ForceAlign && Transform.IsHeightAligned())
			Transform.ResetHeightAlignment();

		if (Transform.Align(CustomAlignment.has_value() ? CustomAlignment.value() : GetVerticalAlignment(), GetOwner()->GetBounds(), GetVerticalAnchor()) || ForceAlign)
		{
			UpdateSlotVerticalAlignment();
			NotifyCanvas_WidgetUpdated();
		}
	}
	else if (IsAlignedToCanvas())
	{
		const auto Canvas = GetCanvas();
		const float OwnerHeight = Canvas->GetScreenDimension().GetHeight();
		const std::optional<const eVerticalAlignment> CustomAlignment = GetNonAlignedHeight() > OwnerHeight ? std::optional<eVerticalAlignment>(eVerticalAlignment::Align_Fill) : std::nullopt;

		if (ForceAlign && Transform.IsHeightAligned())
			Transform.ResetHeightAlignment();

		if (Transform.Align(CustomAlignment.has_value() ? CustomAlignment.value() : GetVerticalAlignment(), Canvas->GetScreenBounds(), GetVerticalAnchor()) || ForceAlign)
		{
			UpdateSlotVerticalAlignment();
			NotifyCanvas_WidgetUpdated();
		}
		else if (GetVerticalAlignment() == eVerticalAlignment::Align_NONE || ForceAlign)
		{
			const auto CanvasOffset = cbgui::GetAnchorPointsFromRect(GetCanvasAnchor(), Canvas->GetScreenBounds());
			Transform.SetPositionOffsetY(CanvasOffset.Y);
			UpdateSlotVerticalAlignment();
			NotifyCanvas_WidgetUpdated();
		}
	}
	else
	{
		if (Transform.IsHeightAligned())
		{
			Transform.ResetHeightAlignment();
			UpdateSlotVerticalAlignment();
			NotifyCanvas_WidgetUpdated();
		}
		else
		{
			UpdateSlotVerticalAlignment();
		}
	}
}

void cbTextBox::UpdateHorizontalAlignment(const bool ForceAlign)
{
	if (HasOwner())
	{
		const float OwnerWidth = GetOwner()->GetWidth();
		const std::optional<const eHorizontalAlignment> CustomAlignment = GetNonAlignedWidth() > OwnerWidth ? std::optional<eHorizontalAlignment>(eHorizontalAlignment::Align_Fill) : std::nullopt;

		if (ForceAlign && Transform.IsWidthAligned())
			Transform.ResetWidthAlignment();

		if (Transform.Align(CustomAlignment.has_value() ? CustomAlignment.value() : GetHorizontalAlignment(), GetOwner()->GetBounds(), GetHorizontalAnchor()) || ForceAlign)
		{
			UpdateSlotHorizontalAlignment();
			NotifyCanvas_WidgetUpdated();
		}
	}
	else if (IsAlignedToCanvas())
	{
		const auto Canvas = GetCanvas();
		const float OwnerWidth = Canvas->GetScreenDimension().GetWidth();
		const std::optional<const eHorizontalAlignment> CustomAlignment = GetNonAlignedWidth() > OwnerWidth ? std::optional<eHorizontalAlignment>(eHorizontalAlignment::Align_Fill) : std::nullopt;

		if (ForceAlign && Transform.IsWidthAligned())
			Transform.ResetWidthAlignment();

		if (Transform.Align(CustomAlignment.has_value() ? CustomAlignment.value() : GetHorizontalAlignment(), Canvas->GetScreenBounds(), GetHorizontalAnchor()) || ForceAlign)
		{
			UpdateSlotHorizontalAlignment();
			NotifyCanvas_WidgetUpdated();
		}		
		else if (GetHorizontalAlignment() == eHorizontalAlignment::Align_NONE || ForceAlign)
		{
			const auto CanvasOffset = cbgui::GetAnchorPointsFromRect(GetCanvasAnchor(), Canvas->GetScreenBounds());
			Transform.SetPositionOffsetX(CanvasOffset.X);
			UpdateSlotHorizontalAlignment();
			NotifyCanvas_WidgetUpdated();
		}
	}
	else
	{
		if (Transform.IsWidthAligned())
		{
			Transform.ResetWidthAlignment();
			UpdateSlotHorizontalAlignment();
			NotifyCanvas_WidgetUpdated();
		}
		else
		{
			UpdateSlotHorizontalAlignment();
		}
	}
}

void cbTextBox::UpdateRotation()
{
	if (HasOwner())
		Transform.SetRollOffset(GetOwner()->GetRotation());
	else if (IsAlignedToCanvas())
		Transform.SetRollOffset(GetCanvas()->GetScreenRotation());
	else
		Transform.SetRollOffset(0.0f);

	if (TextSlot)
		TextSlot->UpdateRotation();
	if (Cursor)
		Cursor->UpdateRotation();
	if (Highlight)
		Highlight->ClearHighlights();
	NotifyCanvas_WidgetUpdated();
}

bool cbTextBox::WrapVertical()
{
	if (!IsItWrapped())
		return false;

	float Height = 0.0;
	{
		const auto& Content = TextSlot->GetContent();
		const float SlotHeight = Content->GetNonAlignedHeight();
		const cbMargin& Padding = Content->GetPadding();
		if ((SlotHeight + (Padding.Top + Padding.Bottom)) > Height)
			Height += SlotHeight + (Padding.Top + Padding.Bottom);
	}

	if (Transform.CompressHeight(Height))
	{
		return true;
	}
	return false;
}

bool cbTextBox::WrapHorizontal()
{
	if (!IsItWrapped())
		return false;

	float Width = 0.0f;
	{
		const auto& Content = TextSlot->GetContent();
		const float SlotWidth = Content->GetNonAlignedWidth();
		const cbMargin& Padding = Content->GetPadding();
		if ((SlotWidth + (Padding.Left + Padding.Right)) > Width)
			Width += SlotWidth + (Padding.Left + Padding.Right);
	}

	if (Transform.CompressWidth(Width))
	{
		return true;
	}
	return false;
}

bool cbTextBox::UnWrapVertical()
{
	if (Transform.IsHeightCompressed())
	{
		Transform.ResetHeightCompressed();

		return true;
	}
	return false;
}

bool cbTextBox::UnWrapHorizontal()
{
	if (Transform.IsWidthCompressed())
	{
		Transform.ResetWidthCompressed();

		return true;
	}
	return false;
}

void cbTextBox::OnAttach()
{
	WrapVertical();
	WrapHorizontal();
	NotifyCanvas_WidgetUpdated();
	UpdateSlotAlignments();
}

void cbTextBox::OnRemoveFromParent()
{
	if (Transform.IsWidthAligned() || Transform.IsHeightAligned())
	{
		Transform.ResetWidthAlignment();
		Transform.ResetHeightAlignment();
		NotifyCanvas_WidgetUpdated();
	}

	UnWrap();
}
