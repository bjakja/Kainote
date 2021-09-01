//  Copyright (c) 2021, Marcin Drob

//  Kainote is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  Kainote is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with Kainote.  If not, see <http://www.gnu.org/licenses/>.

#include "Visuals.h"
#include "TabPanel.h"
#include "VideoToolbar.h"
#include "Hotkeys.h"

AllTags::AllTags()
{
	replaceTagsInCursorPosition = false;
	for (size_t i = 0; i < 4; i++) {
		slider[i].SetAllTags(this);
	}
	tags = VideoToolbar::GetTagsSettings();
	if (!tags->size()) {
		LoadSettings(tags);
	}
}

void AllTags::DrawVisual(int time)
{
	for (size_t i = 0; i < actualTag.numOfValues; i++) {
		slider[i].OnDraw();
	}
}

void AllTags::OnMouseEvent(wxMouseEvent& event)
{
	if (mode >= MULTIPLY)
		multiplyCounter = 0;

	float x = event.GetX();
	float y = event.GetY();
	//have to write shift working on two or more sliders
	bool shift = event.ShiftDown();

	// right holding
	// move sliders
	if (rholding) {
		SetupSlidersPosition(y + sliderPositionDiff);
		tab->Video->Render(false);
		for (size_t i = 0; i < actualTag.numOfValues; i++) {
			slider[i].ResetOnThumbAndSlider();
		}
		if (!event.RightUp()) {
			return;
		}
	}

	if (event.RightDown() || event.RightDClick()) {
		if (!tab->Video->HasCapture()) {
			tab->Video->CaptureMouse();
		}
		sliderPositionDiff = sliderPositionY - y;
		rholding = true;
	}

	if (event.RightUp()) {
		if (tab->Video->HasCapture()) {
			tab->Video->ReleaseMouse();
		}
		rholding = false;
	}

	for (size_t i = 0; i < actualTag.numOfValues; i++) {
		slider[i].OnMouseEvent(event);
	}
}

void AllTags::OnKeyPress(wxKeyEvent& evt)
{
	if (!(actualTag.tag == L"fad" || tagMode & IS_T_ANIMATION))
		return;

	int key = evt.GetKeyCode();
	auto accel = Hkeys.GetHKey(idAndType(EDITBOX_START_DIFFERENCE, EDITBOX_HOTKEY));
	auto accel1 = Hkeys.GetHKey(idAndType(EDITBOX_END_DIFFERENCE, EDITBOX_HOTKEY));
	bool hkeystart = evt.GetModifiers() == accel.GetFlags() && key == accel.GetKeyCode();
	bool hkeyend = evt.GetModifiers() == accel1.GetFlags() && key == accel1.GetKeyCode();
	if (hkeystart || hkeyend) {
		if (tab->Video->GetState() == None) { 
			wxBell(); return; 
		}
		int vidtime = tab->Video->Tell();
		if (vidtime < tab->Edit->line->Start.mstime ||
			vidtime > tab->Edit->line->End.mstime) {
			wxBell(); return;
		}
		int diff = (hkeystart || tagMode & IS_T_ANIMATION) ?
			vidtime - ZEROIT(tab->Edit->line->Start.mstime) :
			abs(vidtime - ZEROIT(tab->Edit->line->End.mstime));
		if (hkeystart) {
			slider[0].SetThumbValue(diff);
		}
		else {
			slider[1].SetThumbValue(diff);
		}
		slider[hkeystart? 0 : 1].SetHolding(true);
		if (tab->Edit->IsCursorOnStart()) {
			SetVisual(false);
		}
		else {
			SetVisual(true);
			SetVisual(false);
		}
		slider[hkeystart ? 0 : 1].SetHolding(false);
	}
}

wxString AllTags::GetSelectedTag(wxString* txt, FindData* result)
{
	TextEditor *editor = tab->Edit->GetEditor();
	long start = 0, end = 0;
	editor->GetSelection(&start, &end);
	if (start != end) {
		if((*txt) != editor->GetValue())
			return L"";

		wxString tag = txt->Mid(start, (end - start));
		if (tag.StartsWith(L"\\")) {
			if (end + 1 < txt->length() &&tag.Freq(L'\\') == 1) {
				result->inBracket = true;
				if (!((*txt)[end] == L'\\' || (*txt)[end] == L'}')) {
					size_t slash = txt->find(L'\\', end);
					size_t endBracket = txt->find(L'}', end);
					if (slash != -1 || endBracket != -1) {
						size_t endPos = (slash < endBracket) ? slash : endBracket;
						wxString fulltag = txt->Mid(start, (endPos - start + 1));
						result->positionInText.x = start;
						result->positionInText.y = endPos;
						return fulltag;
					}
				}
				else {
					result->positionInText.x = start;
					result->positionInText.y = end - 1;
					return tag;
				}
			}
		}
	}
	return L"";
}

void AllTags::CheckTag()
{
	if (actualTag.tag == L"1a" || actualTag.tag == L"2a" ||
		actualTag.tag == L"3a" || actualTag.tag == L"4a" ||
		actualTag.tag == L"alpha") 
	{
		tagMode = IS_HEX_ALPHA;
	}
	else if (actualTag.tag == L"1c" || actualTag.tag == L"2c" ||
		actualTag.tag == L"3c" || actualTag.tag == L"4c" ||
		actualTag.tag == L"c")
	{
		tagMode = IS_HEX_COLOR;
	}
	else if (actualTag.tag == L"p" || actualTag.tag == L"clip" ||
		actualTag.tag == L"iclip")
	{
		tagMode = IS_VECTOR;
	}
	else if (actualTag.tag == L"t") {
		tagMode = IS_T_ANIMATION;
	}
	else {
		tagMode = 0;
	}
}

void AllTags::SetupSlidersPosition(int _sliderPositionY)
{
	if (sliderPositionY == _sliderPositionY && sliderPositionY != -1)
		return;

	sliderPositionY = sliderPositionY == -1 ? 40 : _sliderPositionY;
	float left = 30;
	float right = VideoSize.width - 30;
	float bottom = sliderPositionY;
	float top = sliderPositionY - 6;
	for (size_t i = 0; i < 4; i++) {
		slider[i].SetPosition(left, top, right, bottom);
		top += increase;
		bottom += increase;
	}
}

void AllTags::SetCurVisual()
{
	
	if (currentTag < 0 || currentTag >= tags->size())
		currentTag = 0;
	actualTag = (*tags)[currentTag];
	floatFormat = wxString::Format(L"5.%if", actualTag.digitsAfterDot);
	SetupSlidersPosition(sliderPositionY);
	if (mode >= MULTIPLY && mode <= MULTIPLY_PLUS) {
		AllTagsSetting tmp = (*tags)[currentTag];
		for (size_t i = 0; i < actualTag.numOfValues; i++) {
			slider[i].SetFirstThumbValue(tmp.values[i]);
		}
	}
	CheckTag();
	FindTagValues();
	if (mode < MULTIPLY || mode > MULTIPLY_PLUS) {
		for (size_t i = 0; i < actualTag.numOfValues; i++) {
			slider[i].SetThumbValue(actualTag.values[i]);
		}
	}
	tab->Video->Render(false);
}

void AllTags::FindTagValues()
{
	Styles* currentStyle = tab->Grid->GetStyle(0, tab->Edit->line->Style);
	wxString value;
	TagValueFromStyle(currentStyle, actualTag.tag, &value);
	double doubleValue = 0.;
	if (tagMode & IS_HEX_ALPHA) {
		AssColor col;
		col.SetAlphaString(value);
		actualTag.values[0] = col.a;
	}
	else if (tagMode & IS_HEX_COLOR) {
		AssColor col(value);
		actualTag.values[0] = col.r;
		actualTag.values[1] = col.g;
		actualTag.values[2] = col.b;
	}
	else {
		if (!value.ToDouble(&doubleValue))
			doubleValue = wxAtoi(value);
		actualTag.values[0] = doubleValue;
	}

	if (FindTag(actualTag.tag + L"([-0-9.,\\(\\) &A-FH]+)", L"", actualTag.mode)) {
		const FindData& data = GetResult();
		if (data.finding.StartsWith(L"(")) {
			//remove brackets;
			wxStringTokenizer toknzr(data.finding.Mid(1, data.finding.length() - 2), L",", wxTOKEN_STRTOK);
			int i = 0;
			while (toknzr.HasMoreTokens())
			{
				wxString token = toknzr.GetNextToken().Trim(false).Trim();
				double val = 0;
				if (token.ToCDouble(&val)) {
					actualTag.values[i] = val;
					if (mode < MULTIPLY)
						CheckRange(val);
				}
				i++;
				if (i >= (actualTag.numOfValues))
					break;
			}

		}
		else {
			double val = 0;
			if (tagMode & IS_HEX_ALPHA) {
				AssColor col;
				col.SetAlphaString(data.finding);
				actualTag.values[0] = col.a;
			}
			else if (tagMode & IS_HEX_COLOR) {
				AssColor col(data.finding);
				actualTag.values[0] = col.r;
				actualTag.values[1] = col.g;
				actualTag.values[2] = col.b;
			}
			else if (data.finding.ToCDouble(&val)) {
				actualTag.values[0] = val;
			}
			else
				return;

			if (mode < MULTIPLY)
				CheckRange(val);
		}
	}
}

void AllTags::ChangeTool(int _tool, bool blockSetCurVisual)
{
	if (lastTool == _tool)
		return;

	mode = _tool >> 20;
	replaceTagsInCursorPosition = mode == INSERT;

	int curtag = _tool << 12;
	currentTag = curtag >> 12;
	if(!blockSetCurVisual)
		SetCurVisual();
}

void AllTags::GetVisualValue(wxString* visual, const wxString& curValue)
{
	float value = slider[0].GetThumbValue();
	float value2 = slider[1].GetThumbValue();
	float value3 = slider[2].GetThumbValue();
	float value4 = slider[3].GetThumbValue();
	float valuediff = slider[0].GetDiffValue();
	float valuediff2 = slider[1].GetDiffValue();
	float valuediff3 = slider[2].GetDiffValue();
	float valuediff4 = slider[3].GetDiffValue();
	wxString strval;
	if (curValue.empty() || mode == MULTIPLY || mode == INSERT) {
		//mode 2 for multiply
		//mode 1 for paste only one value
		float val1 = (mode >= MULTIPLY) ?
			actualTag.values[0] + (multiplyCounter * valuediff) : value;
		float val2 = (mode >= MULTIPLY) ?
			actualTag.values[1] + (multiplyCounter * valuediff2) : value2;
		float val3 = (mode >= MULTIPLY) ?
			actualTag.values[2] + (multiplyCounter * valuediff3) : value3;

		if (tagMode & IS_HEX_ALPHA) {
			strval = wxString::Format(L"&H%02X&", MID(0, (int)(val1 + 0.5), 255));
		}
		else if (tagMode & IS_HEX_COLOR) {
			//bgr but sliders is rgb
			strval = wxString::Format(L"&H%02X%02X%02X&",
				MID(0, (int)(val3 + 0.5), 255),
				MID(0, (int)(val2 + 0.5), 255),
				MID(0, (int)(val1 + 0.5), 255));
		}
		else if (actualTag.numOfValues > 1) {
			float val4 = (mode >= MULTIPLY) ?
				actualTag.values[3] + (multiplyCounter * valuediff4) : value4;
			strval = L"(" + getfloat(val1, floatFormat) + L"," +
				getfloat(val2, floatFormat);
			if (actualTag.numOfValues >= 3) {
				strval << L"," << getfloat(val3, floatFormat);
			}
			if (actualTag.numOfValues == 4) {
				strval << L"," << getfloat(val4, floatFormat);
			}
			if (tagMode & IS_T_ANIMATION) {
				strval << L",";
			}

			if (curValue.empty() || curValue.EndsWith(")")) {
				strval << ")";
			}
		}
		else
			strval = getfloat(val1, floatFormat);

		
	}
	else if (curValue.StartsWith(L"(")) {
		bool hasLastBracket = curValue.EndsWith(L")");
		//remove brackets;
		wxStringTokenizer toknzr(curValue.Mid(1, hasLastBracket? curValue.length() - 2 : 
			curValue.length() - 1), L",", wxTOKEN_STRTOK);
		strval = L"(";
		int counter = 0;
		while (toknzr.HasMoreTokens())
		{
			wxString token = toknzr.GetNextToken().Trim(false).Trim();
			double val = 0;
			if (token.ToCDouble(&val)) {
				float valdiff = (counter % 2 == 0)? valuediff : 
					(counter % 2 == 1) ? valuediff2 :
					(counter % 2 == 2) ? valuediff3 : valuediff4;
				if (mode > MULTIPLY) {
					valdiff *= multiplyCounter;
				}
				strval << getfloat(val + valdiff, floatFormat) << L",";
			}
			counter++;
		}
		if (strval.EndsWith(L",") && !(tagMode & IS_T_ANIMATION))
			strval = strval.Mid(0, strval.length() - 1);
		if(hasLastBracket)
			strval << L")";
	}
	else {
		double val = 0;
		wxString trimed = curValue;
		trimed.Trim(false).Trim();
		//bug when tag is in \t
		bool hasEndBracked = false;
		if (trimed.EndsWith(")")) {
			trimed = trimed.Mid(0, trimed.length() - 1);
			hasEndBracked = true;
		}
		if (tagMode & IS_HEX_ALPHA) {
			AssColor col;
			col.SetAlphaString(trimed);
			float vala = (mode > MULTIPLY)? col.a + (valuediff * multiplyCounter) :
				col.a + valuediff;
			strval = wxString::Format(L"&H%02X&", MID(0, (int)(vala + 0.5), 255));
		}
		else if (tagMode & IS_HEX_COLOR) {
			AssColor col(trimed);
			float valr = (mode > MULTIPLY) ? col.r + (valuediff * multiplyCounter) :
				col.r + valuediff;
			float valg = (mode > MULTIPLY) ? col.g + (valuediff2 * multiplyCounter) :
				col.g + valuediff2;
			float valb = (mode > MULTIPLY) ? col.b + (valuediff3 * multiplyCounter) :
				col.b + valuediff3;
			strval = wxString::Format(L"&H%02X%02X%02X&",
				MID(0, (int)(valb + 0.5), 255),
				MID(0, (int)(valg + 0.5), 255),
				MID(0, (int)(valr + 0.5), 255));
		}
		else if (trimed.ToCDouble(&val)) {
			val += (mode > MULTIPLY) ? (valuediff * multiplyCounter) : valuediff;
			strval = getfloat(val, floatFormat);
		}
		else//dont add a bracket when value not set
			hasEndBracked = false;

		if (hasEndBracked)
			strval << L")";
	}
	
	*visual = strval;
}

wxPoint AllTags::ChangeVisual(wxString* txt)
{
	if (mode == 4) {
		auto replfunc = [=](const FindData& data, wxString* result, size_t numOfCharacters) {
			GetVisualValue(result, data.finding);
			if(numOfCharacters > 1)
				multiplyCounter += (1.f / (numOfCharacters - 1));
		};
		ReplaceAllByChar(actualTag.tag + L"([-0-9.,\\(\\) &A-FH]+)", actualTag.tag, txt, replfunc);
	}
	else if (mode) {
		FindTag(actualTag.tag + L"([-0-9.,\\(\\) &A-FH]+)", *txt, actualTag.mode);
		wxString strValue;
		FindData res = GetResult();
		GetVisualValue(&strValue, res.finding);
		if (tagMode & IS_T_ANIMATION) {
			wxString selectedTag = GetSelectedTag(txt, &res);
			if (!selectedTag.empty()) {
				if (strValue.EndsWith(L")")) {
					strValue.insert(strValue.length() - 1, selectedTag);
				}
				else {
					strValue << selectedTag;
				}
				SetResult(res);
			}
		}
		Replace(L"\\" + actualTag.tag + strValue, txt);
		//if there is one line there's no need to count it
	}
	else {
		auto replfunc = [=](const FindData& data, wxString* result) {
			GetVisualValue(result, data.finding);
		};
		ReplaceAll(actualTag.tag + L"([-0-9.,\\(\\) &A-FH]+)", actualTag.tag, txt, replfunc, true);
		FindTag(actualTag.tag + L"([-0-9.,\\(\\) &A-FH]+)", *txt);
	}
	return GetPositionInText();
}

void AllTags::ChangeVisual(wxString* txt, Dialogue *dial, size_t numOfSelections)
{
	if (mode == GRADIENT_TEXT) {
		auto replfunc = [=](const FindData& data, wxString* result, size_t numOfCharacters) {
			GetVisualValue(result, data.finding);
			if (numOfCharacters > 1)
				multiplyCounter += (1.f / (numOfCharacters - 1));
		};
		ReplaceAllByChar(actualTag.tag + L"([-0-9.,\\(\\) &A-FH]+)", actualTag.tag, txt, replfunc);
	}
	else if (mode) {
		FindTag(actualTag.tag + L"([-0-9.,\\(\\) &A-FH]+)", *txt, 1);
		wxString strValue;
		FindData res = GetResult();
		GetVisualValue(&strValue, res.finding);
		if (tagMode & IS_T_ANIMATION) {
			wxString selectedTag = GetSelectedTag(txt, &res);
			if (!selectedTag.empty()) {
				if (strValue.EndsWith(L")")) {
					strValue.insert(strValue.length() - 1, selectedTag);
				}
				else {
					strValue << selectedTag;
				}
				SetResult(res);
			}
		}
		Replace(L"\\" + actualTag.tag + strValue, txt);
		if (mode == GRADIENT_LINE)
			multiplyCounter += (1.f / (numOfSelections - 1));
		else if (mode >= MULTIPLY)
			multiplyCounter++;
	}
	else {
		auto replfunc = [=](const FindData& data, wxString* result) {
			GetVisualValue(result, data.finding);
		};
		ReplaceAll(actualTag.tag + L"([-0-9.,\\(\\) &A-FH]+)", actualTag.tag, txt, replfunc, true);
	}
}

void AllTags::CheckRange(float val)
{
	if (val < actualTag.rangeMin)
		actualTag.rangeMin = val;
	if (val > actualTag.rangeMax)
		actualTag.rangeMax = val;
}

void AllTags::OnMouseCaptureLost(wxMouseCaptureLostEvent& evt)
{
	rholding = false;
	for (size_t i = 0; i < 4; i++) {
		slider[i].SetHolding(false);
	}
}

