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
	tags = VideoToolbar::GetTagsSettings();
	if (!tags->size()) {
		LoadSettings(tags);
	}
}

void AllTags::DrawVisual(int time)
{
	float range = actualTag.rangeMax - actualTag.rangeMin;
	if (range <= 0) {
		KaiLog(L"Bad range");
		return;
	}

	float left = 20;
	float right = VideoSize.width - 40;
	float bottom = sliderPositionY;
	float top = sliderPositionY - 8;
	float sliderRange = right - left;
	float coeff = sliderRange / range;
	float step = actualTag.step * coeff;
	float thumbposdiff = -actualTag.rangeMin;
	int numOfLoops = actualTag.has2value ? 2 : 1;
	for (size_t i = 0; i < numOfLoops; i++) {
		float thumbtop = top - 10;
		float thumbbottom = bottom + 10;
		
		D3DCOLOR fill = 0xAA121150;
		VERTEX v9[9];
		CreateVERTEX(&v9[0], left, top, fill);
		CreateVERTEX(&v9[1], right, top, fill);
		CreateVERTEX(&v9[2], left, bottom, fill);
		CreateVERTEX(&v9[3], right, bottom, fill);
		CreateVERTEX(&v9[4], left, top, 0xFFBB0000);
		CreateVERTEX(&v9[5], right, top, 0xFFBB0000);
		CreateVERTEX(&v9[6], right, bottom, 0xFFBB0000);
		CreateVERTEX(&v9[7], left, bottom, 0xFFBB0000);
		CreateVERTEX(&v9[8], left, top, 0xFFBB0000);

		HRN(device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v9, sizeof(VERTEX)), L"primitive failed");
		HRN(device->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, &v9[4], sizeof(VERTEX)), L"primitive failed");

		float lastPos = 0;
		line->Begin();
		int j = 0;
		float rightend = right + (step / 2);
		float distance = (abs(actualTag.rangeMax) > 999 || abs(actualTag.rangeMin) > 999) ? 15 : 10;
		for (float i = left; i <= rightend; i += step) {
			if (i - lastPos > distance) {
				D3DXVECTOR2 linepoints[] = { D3DXVECTOR2(i, top - 6), D3DXVECTOR2(i, top + 8 + 6) };
				line->Draw(linepoints, 2, 0xFFBB0000);
				lastPos = i;
				float thumbOnSliderValue = ((i - left) / coeff) - thumbposdiff;
				bool ismod0 = j % 4 == 0;
				if (j % 4 == 2 || ismod0) {
					RECT rect = { (long)i - 50, ismod0 ? (long)thumbbottom + 2 : (long)thumbtop - 60, (long)i + 50, ismod0 ? (long)thumbbottom + 60 : (long)thumbtop - 2 };
					int align = ismod0 ? DT_CENTER : DT_CENTER | DT_BOTTOM;
					DRAWOUTTEXT(font, getfloat(thumbOnSliderValue, L"5.1f"), rect, align, 0xFFFFFFFF);
				}
				j++;
			}
		}
		line->End();


		float thumbpos = ((thumbValue[i] + thumbposdiff) * coeff) + left;
		float thumbleft = thumbpos - 4;
		float thumbright = thumbpos + 4;
		fill = (thumbState[i] == 1) ? 0xAACC8748 : (thumbState[i] == 2) ? 0xAAFCE6B1 : 0xAA121150;
		CreateVERTEX(&v9[0], thumbleft, thumbtop, fill);
		CreateVERTEX(&v9[1], thumbright, thumbtop, fill);
		CreateVERTEX(&v9[2], thumbleft, thumbbottom, fill);
		CreateVERTEX(&v9[3], thumbright, thumbbottom, fill);
		CreateVERTEX(&v9[4], thumbleft, thumbtop, 0xFFBB0000);
		CreateVERTEX(&v9[5], thumbright, thumbtop, 0xFFBB0000);
		CreateVERTEX(&v9[6], thumbright, thumbbottom, 0xFFBB0000);
		CreateVERTEX(&v9[7], thumbleft, thumbbottom, 0xFFBB0000);
		CreateVERTEX(&v9[8], thumbleft, thumbtop, 0xFFBB0000);

		HRN(device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v9, sizeof(VERTEX)), L"primitive failed");
		HRN(device->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, &v9[4], sizeof(VERTEX)), L"primitive failed");

		if (onThumb[i]) {
			RECT rect = { (long)thumbleft - 50, (long)thumbbottom + 10, (long)thumbright + 50, (long)thumbbottom + 50 };
			wxString num = getfloat(thumbValue[i], floatFormat);
			DRAWOUTTEXT(font, num, rect, DT_CENTER, 0xFFFFFFFF);
		}
		if (onSlider[i]) {
			float thumbOnSliderValue = ((x - left) / coeff) - thumbposdiff;
			thumbOnSliderValue = MID(actualTag.rangeMin, thumbOnSliderValue, actualTag.rangeMax);
			RECT rect = { (long)x - 50, (long)y + 20, (long)x + 50, (long)y + 70 };
			DRAWOUTTEXT(font, getfloat(thumbOnSliderValue, floatFormat), rect, DT_CENTER, 0xFFFFFFFF);
		}

		top += increase;
		bottom += increase;
	}
}

void AllTags::OnMouseEvent(wxMouseEvent& event)
{
	float range = actualTag.rangeMax - actualTag.rangeMin;
	if (range <= 0) {
		KaiLog(L"Bad range");
		return;
	}
	float thumbposdiff = -actualTag.rangeMin;

	float left = 20;
	float right = VideoSize.width - 40;
	float bottom = sliderPositionY;
	float top = sliderPositionY - 8;
	float sliderRange = right - left;
	float coeff = sliderRange / range;
	float step = actualTag.step * coeff;
	if (mode == 2)
		subtractCounter = 0;

	x = event.GetX();
	y = event.GetY();
	bool shift = event.ShiftDown();

	// right holding
	if (rholding) {
		sliderPositionY = y + sliderPositionDiff;
		tab->Video->Render(false);
		onSlider[0] = onThumb[0] = onSlider[1] = onThumb[1] = false;
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

	//wheel rotation
	if (event.GetWheelRotation() != 0) {
		if (event.ShiftDown()) {
			int step = event.GetWheelRotation() / event.GetWheelDelta();
			currentTag -= step;
			if (currentTag < 0)
				currentTag = tags->size() - 1;
			else if (currentTag >= tags->size())
				currentTag = 0;
			int tool = mode << 20;
			tool += currentTag;
			VideoCtrl* vc = tab->Video;
			vc->GetVideoToolbar()->SetItemToggled(&tool);
			ChangeTool(tool);
			return;
		}
		int rot = event.GetWheelRotation() / event.GetWheelDelta();
		size_t i = 0;
		if(mode != 2)
			firstThumbValue[i] = thumbValue[i];

		thumbValue[i] = rot < 0 ? thumbValue[i] - actualTag.step : thumbValue[i] + actualTag.step;
		thumbValue[i] = MID(actualTag.rangeMin, thumbValue[i], actualTag.rangeMax);

		if (firstThumbValue[i] != thumbValue[i]) {
			onThumb[i] = true;
			onSlider[i] = false;
			//set holding before us to know what value use
			holding[i] = true;
			if(shift)
				holding[1] = true;
			if (tab->Edit->IsCursorOnStart()) {
				SetVisual(false);
			}
			else {
				SetVisual(true);
				SetVisual(false);
			}
			holding[i] = false;
			if (shift)
				holding[1] = false;
		}
		return;
	}
	int numOfLoops = actualTag.has2value ? 2 : 1;
	for (size_t i = 0; i < numOfLoops; i++) {

		float thumbpos = ((thumbValue[i] + thumbposdiff) * coeff) + left;
		float thumbleft = thumbpos - 4;
		float thumbright = thumbpos + 4;
		float thumbtop = top - 10;
		float thumbbottom = bottom + 10;
		//leave the window
		if (event.Leaving()) {
			if (thumbState[i] != 0) {
				thumbState[i] = 0;
				tab->Video->Render(false);
			}
		}
		

		//skip unneeded positions
		if (!holding[i]) {

			//outside slider, nothing to do
			if ((x < left - 5 || y < thumbtop || x > right + 5 || y > thumbbottom)) {
				if (thumbState[i] != 0 || onSlider[i] || onThumb[i]) {
					thumbState[i] = 0;
					onSlider[i] = onThumb[i] = false;
					tab->Video->Render(false);
				}
				top += increase;
				bottom += increase;
				continue;
			}

			onThumb[i] = false;
			onSlider[i] = false;
			//on thumb position
			if (x >= thumbleft && x <= thumbright && y >= thumbtop && y <= thumbbottom) {
				onThumb[i] = true;
				if (!event.LeftDown() && !event.LeftDClick() && !event.LeftUp() && thumbState[i] != 1) {
					thumbState[i] = 1;
					tab->Video->Render(false);
				}
			}//on slider
			else {
				if (y >= top - 5 && y <= bottom + 5 && x >= left && x <= right) {
					onSlider[i] = true;
				}
				if (!event.LeftDown() && !event.LeftDClick() && !event.LeftUp()) {
					thumbState[i] = 0;
					tab->Video->Render(false);
				}

				if (!onSlider[i]) {
					top += increase;
					bottom += increase;
					continue;
				}
			}

		}

		if (holding[i]) {
			//calculate new thumb value from mouse position
			thumbValue[i] = ((x - left) / coeff) - thumbposdiff;
			thumbValue[i] = MID(actualTag.rangeMin, thumbValue[i], actualTag.rangeMax);
			if (lastThumbValue[i] != thumbValue[i]) {
				if((shift && i > 0) || !shift)
					SetVisual(true);
			}

			lastThumbValue[i] = thumbValue[i];
		}

		if (event.LeftDown() || event.LeftDClick()) {
			if(mode != 2)
				lastThumbValue[i] = firstThumbValue[i] = thumbValue[i];
			else
				lastThumbValue[i] = thumbValue[i];

			if (onThumb[i]) {
				thumbState[i] = 2;
				if (!tab->Video->HasCapture()) {
					tab->Video->CaptureMouse();
				}
				tab->Video->Render(false);
				holding[i] = true;
				if (shift) {
					holding[i == 0 ? 1 : 0] = true;
				}
			}
			else if (onSlider[i]) {
				thumbState[i] = 1;
				thumbValue[i] = ((x - left) / coeff) - thumbposdiff;
				thumbValue[i] = MID(actualTag.rangeMin, thumbValue[i], actualTag.rangeMax);
				//set holding before us to know what value use
				holding[i] = true;
				if (shift)
					holding[i == 0 ? 1 : 0] = true;
				if (tab->Edit->IsCursorOnStart()) {
					SetVisual(false);
				}
				else {
					SetVisual(true);
					SetVisual(false);
				}
				holding[i] = false;
				if (shift) {
					holding[i == 0 ? 1 : 0] = false;
					//no need tu use this function 2 times
					break;
				}
			}
		}

		if (event.LeftUp() && holding[i]) {
			thumbState[i] = 0;
			if (tab->Video->HasCapture()) {
				tab->Video->ReleaseMouse();
			}
			//if(holding[i])
			SetVisual(false);
			//lastThumbValue[i] = firstThumbValue[i] = thumbValue[i];
			if (shift) {
				holding[0] = holding[1] = false;
			}
			else {
				holding[i] = false;
			}
		}
		top += increase;
		bottom += increase;
	}
}

void AllTags::OnKeyPress(wxKeyEvent& evt)
{
	if (actualTag.tag != L"fad")
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
		int diff = (hkeystart) ?
			vidtime - ZEROIT(tab->Edit->line->Start.mstime) :
			abs(vidtime - ZEROIT(tab->Edit->line->End.mstime));
		if (hkeystart) {
			firstThumbValue[0] = thumbValue[0];
			thumbValue[0] = diff;
		}
		else {
			firstThumbValue[1] = thumbValue[1];
			thumbValue[1] = diff;
		}
		holding[hkeystart? 0 : 1] = true;
		if (tab->Edit->IsCursorOnStart()) {
			SetVisual(false);
		}
		else {
			SetVisual(true);
			SetVisual(false);
		}
		holding[hkeystart ? 0 : 1] = false;
		//firstThumbValue[0] = thumbValue[0];
		//firstThumbValue[1] = thumbValue[1];
	}
}

void AllTags::SetCurVisual()
{
	
	if (currentTag < 0 || currentTag >= tags->size())
		currentTag = 0;
	actualTag = (*tags)[currentTag];
	floatFormat = wxString::Format(L"5.%if", actualTag.DigitsAfterDot);
	if (mode == 2) {
		firstThumbValue[0] = thumbValue[0] = actualTag.value;
		firstThumbValue[1] = thumbValue[1] = actualTag.value2;
	}
	FindTagValues();
	if (mode != 2) {
		thumbValue[0] = actualTag.value;
		thumbValue[1] = actualTag.value2;
	}
	tab->Video->Render(false);
}

void AllTags::FindTagValues()
{
	Styles* currentStyle = tab->Grid->GetStyle(0, tab->Edit->line->Style);
	if (actualTag.tag == L"fs")
		actualTag.value = currentStyle->GetFontSizeDouble();
	else if(actualTag.tag == L"bord")
		actualTag.value = currentStyle->GetOtlineDouble();
	else if(actualTag.tag == L"shad")
		actualTag.value = currentStyle->GetShadowDouble();
	else if (actualTag.tag == L"fsp")
		actualTag.value = currentStyle->GetSpacingDouble();

	if (FindTag(actualTag.tag + L"([-0-9.,\\(\\) ]+)", L"", actualTag.mode)) {
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
					if (i == 0) {
						actualTag.value = val;
						if(mode != 2)
							CheckRange(val);
					}
					else if (i == 1) {
						actualTag.value2 = val;
						if (mode != 2)
							CheckRange(val);
					}
				}
				i++;
				if (i >= 2)
					break;
			}

		}
		else {
			double val = 0;
			if (data.finding.ToCDouble(&val)) {
				actualTag.value = val;
				if (mode != 2)
					CheckRange(val);
			}
		}
	}
}

void AllTags::ChangeTool(int _tool)
{
	if (lastTool == _tool)
		return;

	mode = _tool >> 20;
	replaceTagsInCursorPosition = mode == 1;

	int curtag = _tool << 12;
	currentTag = curtag >> 12;
	SetCurVisual();
}

void AllTags::GetVisualValue(wxString* visual, const wxString& curValue)
{
	float value = thumbValue[0];
	float value2 = thumbValue[1];
	float valuediff = holding[0] || mode == 2 ? thumbValue[0] - firstThumbValue[0] : 0;
	float valuediff2 = holding[1] || mode == 2 ? thumbValue[1] - firstThumbValue[1] : 0;
	wxString strval;
	if (curValue.empty() || mode) {
		//mode 2 for subtract 
		//mode 1 for paste only one value
		if (mode == 2) {
			float val1 = subtractCounter * valuediff;
			if (actualTag.has2value) {
				float val2 = subtractCounter * valuediff2;
				strval = L"(" + getfloat(actualTag.value + val1, floatFormat) + L"," +
					getfloat(actualTag.value2 + val2, floatFormat) + L")";
			}
			else
				strval = getfloat(actualTag.value + val1, floatFormat);
		}
		else {
			if (actualTag.has2value) {
				strval = L"(" + getfloat(value, floatFormat) + L"," +
					getfloat(value2, floatFormat) + L")";
			}
			else
				strval = getfloat(value, floatFormat);
		}
	}
	else if (curValue.StartsWith(L"(")) {
		//remove brackets;
		wxStringTokenizer toknzr(curValue.Mid(1, curValue.length() - 2), L",", wxTOKEN_STRTOK);
		strval = L"(";
		int counter = 0;
		while (toknzr.HasMoreTokens())
		{
			wxString token = toknzr.GetNextToken().Trim(false).Trim();
			double val = 0;
			if (token.ToCDouble(&val)) {
				val += (counter % 2 == 0)? valuediff : valuediff2;
				strval << getfloat(val, floatFormat) << L",";
			}
			counter++;
		}
		if (strval.EndsWith(L","))
			strval = strval.Mid(0, strval.length() - 1);

		strval << L")";
	}
	else {
		double val = 0;
		wxString trimed = curValue;
		trimed.Trim(false).Trim();
		if (trimed.ToCDouble(&val)) {
			val += valuediff;
			strval = getfloat(val, floatFormat);
		}
	}
	
	*visual = strval;
}

wxPoint AllTags::ChangeVisual(wxString* txt)
{
	if (mode) {
		FindTag(actualTag.tag + L"([-0-9.,\\(\\) ]+)", *txt, actualTag.mode);
		wxString strValue, strFinding;
		GetTextResult(&strFinding);
		GetVisualValue(&strValue, strFinding);
		Replace(L"\\" + actualTag.tag + strValue, txt);
		if (mode == 2)
			subtractCounter++;
	}
	else {
		auto replfunc = [=](const FindData& data, wxString* result) {
			GetVisualValue(result, data.finding);
		};
		ReplaceAll(actualTag.tag + L"([-0-9.,\\(\\) ]+)", actualTag.tag, txt, replfunc, true);
		FindTag(actualTag.tag + L"([-0-9.,\\(\\) ]+)", *txt);
	}
	return GetPositionInText();
}

void AllTags::ChangeVisual(wxString* txt, Dialogue *dial)
{
	if (mode) {
		FindTag(actualTag.tag + L"([-0-9.,\\(\\) ]+)", *txt, 1);
		wxString strValue, strFinding;
		GetTextResult(&strFinding);
		GetVisualValue(&strValue, strFinding);
		Replace(L"\\" + actualTag.tag + strValue, txt);
		if (mode == 2)
			subtractCounter++;
	}
	else {
		auto replfunc = [=](const FindData& data, wxString* result) {
			GetVisualValue(result, data.finding);
		};
		ReplaceAll(actualTag.tag + L"([-0-9.,\\(\\) ]+)", actualTag.tag, txt, replfunc, true);
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
	holding[0] = holding[1] = rholding = false;
}

