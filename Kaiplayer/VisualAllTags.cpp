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

AllTags::AllTags()
{
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

	D3DCOLOR fill = /*(sel) ? 0xAAFCE6B1 : */0xAA121150;
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

	
	float sliderRange = right - left;
	float coeff = sliderRange / range;
	float step = actualTag.step * coeff;
	float thumbtop = top - 10;
	float thumbbottom = bottom + 10;
	float thumbposdiff = -actualTag.rangeMin;
	float lastPos = 0;
	line->Begin();
	int j = 0;
	for (float i = left; i <= right; i += step) {
		if (i - lastPos > 10) {
			D3DXVECTOR2 linepoints[] = { D3DXVECTOR2(i, top - 6), D3DXVECTOR2(i, top + 8 + 6) };
			line->Draw(linepoints, 2, 0xFFBB0000);
			lastPos = i;
			float thumbOnSliderValue = ((i - left) / coeff) - thumbposdiff;
			bool ismod0 = j % 4 == 0;
			if (j % 4 == 2 || ismod0) {
				RECT rect = { i - 50, ismod0 ? thumbbottom + 2 : thumbtop - 60, i + 50, ismod0 ? thumbbottom + 60 : thumbtop - 2 };
				int align = ismod0 ? DT_CENTER : DT_CENTER | DT_BOTTOM;
				DRAWOUTTEXT(font, getfloat(thumbOnSliderValue, L"5.1f"), rect, align, 0xFFFFFFFF);
			}
			j++;
		}
	}
	line->End();

	
	float thumbpos = ((thumbValue + thumbposdiff) * coeff) + left;
	float thumbleft = thumbpos - 4;
	float thumbright = thumbpos + 4;
	fill = (thumbState == 1) ? 0xAACC8748 : (thumbState == 2) ? 0xAAFCE6B1 : 0xAA121150;
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

	if (onThumb) {
		RECT rect = { (long)thumbleft - 50, (long)thumbbottom + 10, (long)thumbright + 50, (long)thumbbottom + 50 };
		DRAWOUTTEXT(font, getfloat(thumbValue, floatFormat), rect, DT_CENTER, 0xFFFFFFFF);
	}
	if (onSlider) {
		float thumbOnSliderValue = ((x - left) / coeff) - thumbposdiff;
		thumbOnSliderValue = MID(actualTag.rangeMin, thumbOnSliderValue, actualTag.rangeMax);
		RECT rect = { (long)x - 50, (long)y + 20, (long)x + 50, (long)y + 70 };
		DRAWOUTTEXT(font, getfloat(thumbOnSliderValue, floatFormat), rect, DT_CENTER, 0xFFFFFFFF);
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
	float thumbpos = ((thumbValue + thumbposdiff) * coeff) + left;
	float thumbleft = thumbpos - 4;
	float thumbright = thumbpos + 4;
	float thumbtop = top - 10;
	float thumbbottom = bottom + 10;
	x = event.GetX();
	y = event.GetY();
	//leave the window
	if (event.Leaving()) {
		if (thumbState != 0) {
			thumbState = 0;
			tab->Video->Render(false);
		}
	}
	//wheel rotation
	if (event.GetWheelRotation() != 0) {
		int rot = event.GetWheelRotation() / event.GetWheelDelta();
		firstThumbValue = thumbValue;
		thumbValue = rot < 0 ? thumbValue - actualTag.step : thumbValue + actualTag.step;
		thumbValue = MID(actualTag.rangeMin, thumbValue, actualTag.rangeMax);
		
		if (firstThumbValue != thumbValue) {
			onThumb = true;
			onSlider = false;
			if (tab->Edit->IsCursorOnStart()) {
				ChangeInLines(false);
			}
			else {
				ChangeInLines(false);
				ChangeInLines(true);
			}
			
		}
		return;
	}
	// right holding
	if (rholding) {
		sliderPositionY = y + sliderPositionDiff;
		tab->Video->Render(false);
		onSlider = onThumb = false;
		if (!event.RightUp()) {
			return;
		}
	}
	//skip unneeded positions
	if (!holding) {
		
		//outside slider, nothing to do
		if ((x < left - 5 || y < thumbtop || x > right + 5 || y > thumbbottom)) {
			if (thumbState != 0 || onSlider || onThumb) {
				thumbState = 0;
				onSlider = onThumb = false;
				tab->Video->Render(false);
			}
			return;
		}

		onThumb = false;
		onSlider = false;
		//on thumb position
		if (x >= thumbleft && x <= thumbright && y >= thumbtop && y <= thumbbottom) {
			onThumb = true;
			if (!event.LeftDown() && !event.LeftDClick() && !event.LeftUp() && thumbState != 1) {
				thumbState = 1;
				tab->Video->Render(false);
			}
		}//on slider
		else{
			if (y >= top - 5 && y <= bottom + 5 && x >= left && x <= right) {
				onSlider = true;
			}
			if (!event.LeftDown() && !event.LeftDClick() && !event.LeftUp()) {
				thumbState = 0;
				tab->Video->Render(false);
			}
			
			if (!onSlider)
				return;
		}
		
	}

	if (holding) {
		//calculate new thumb value from mouse position
		thumbValue = ((x - left) / coeff) - thumbposdiff;
		thumbValue = MID(actualTag.rangeMin, thumbValue, actualTag.rangeMax);
		if(lastThumbValue != thumbValue)
			ChangeInLines(true);

		lastThumbValue = thumbValue;
	}

	if (event.LeftDown() || event.LeftDClick()) {
		lastThumbValue = firstThumbValue = thumbValue;
		if (onThumb) {
			thumbState = 2;
			if (!tab->Video->HasCapture()) {
				tab->Video->CaptureMouse();
			}
			tab->Video->Render(false);
			holding = true;
		}
		else if (onSlider) {
			thumbState = 1;
			thumbValue = ((x - left) / coeff) - thumbposdiff;
			thumbValue = MID(actualTag.rangeMin, thumbValue, actualTag.rangeMax);
			ChangeInLines(true);
		}
	}

	if (event.LeftUp()) {
		thumbState = 0;
		holding = false;
		if (tab->Video->HasCapture()) {
			tab->Video->ReleaseMouse();
		}
		ChangeInLines(false);
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
}

void AllTags::SetCurVisual()
{
	
	if (currentTag < 0 || currentTag >= tags->size())
		currentTag = 0;
	actualTag = (*tags)[currentTag];
	floatFormat = wxString::Format(L"5.%if", actualTag.DigitsAfterDot);
	FindTagValues();
	thumbValue = actualTag.value;
	tab->Video->Render(false);
}

void AllTags::FindTagValues()
{
	bool isOriginal = (tab->Grid->hasTLMode && tab->Edit->TextEdit->GetValue() == L"");
	editor = (isOriginal) ? tab->Edit->TextEditOrig : tab->Edit->TextEdit;
	currentLineText = editor->GetValue();
	Styles* acstyl = tab->Grid->GetStyle(0, tab->Edit->line->Style);
	if (actualTag.tag == L"fs")
		actualTag.value = acstyl->GetFontSizeDouble();
	else if(actualTag.tag == L"bord")
		actualTag.value = acstyl->GetOtlineDouble();
	else if(actualTag.tag == L"shad")
		actualTag.value = acstyl->GetShadowDouble();
	else if (actualTag.tag == L"fsp")
		actualTag.value = acstyl->GetSpacingDouble();

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
						CheckRange(val);
					}
					else if (i == 1) {
						actualTag.value2 = val;
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
				CheckRange(val);
			}
		}
	}
}

void AllTags::ChangeTool(int _tool)
{
	currentTag = _tool;
	SetCurVisual();
}

void AllTags::GetVisualValue(wxString* visual, const wxString& curValue)
{
	float value = thumbValue;
	float valuediff = thumbValue - firstThumbValue;
	float valuediff2 = thumbValue2 - firstThumbValue2;
	wxString strval;
	if (curValue.empty()) {
		//value = thumbValue;
		
		strval = getfloat(changeMoveDiff? MID(actualTag.rangeMin, valuediff, actualTag.rangeMax) : value, floatFormat);
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
				strval << getfloat(val, floatFormat);
			}
			counter++;
		}
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

void AllTags::ChangeVisual(wxString* txt)
{
	auto replfunc = [=](const FindData& data, wxString* result) {
		GetVisualValue(result, data.finding);
	};
	ReplaceAll(actualTag.tag + L"([-0-9.,\\(\\) ]+)", actualTag.tag, txt, replfunc, true);
}

void AllTags::ChangeInLines(bool dummy)
{
	EditBox* edit = tab->Edit;
	SubsGrid* grid = tab->Grid;
	//Get editor
	//two stages, stage first selected lines
	if (edit->IsCursorOnStart()) {
		bool showOriginalOnVideo = !Options.GetBool(TL_MODE_HIDE_ORIGINAL_ON_VIDEO);
		wxString* dtxt;
		wxArrayInt sels;
		grid->file->GetSelections(sels);
		bool skipInvisible = dummy && tab->Video->GetState() != Playing;
		if (dummy && (!dummytext || selPositions.size() != sels.size())) {
			bool visible = false;
			selPositions.clear();
			//need to check if can delete when sizes are different dummytext is valid pointer
			SAFE_DELETE(dummytext);
			dummytext = grid->GetVisible(&visible, 0, &selPositions);
			if (selPositions.size() != sels.size()) {
				//KaiLog(L"Sizes mismatch");
				return;
			}
		}
		if (dummy) { dtxt = new wxString(*dummytext); }
		int _time = tab->Video->Tell();
		int moveLength = 0;
		const wxString& tlStyle = tab->Grid->GetSInfo(L"TLMode Style");
		for (size_t i = 0; i < sels.size(); i++) {
			int sel = sels[i];
			Dialogue* Dial = grid->GetDialogue(sel);
			if (skipInvisible && !(_time >= Dial->Start.mstime && _time <= Dial->End.mstime)) { continue; }

			wxString txt = Dial->GetTextNoCopy();
			ChangeVisual(&txt);
			if (!dummy) {
				grid->CopyDialogue(sel)->SetText(txt);
			}
			else {
				Dialogue Cpy = Dialogue(*Dial);
				if (Dial->TextTl != L"" && grid->hasTLMode) {
					Cpy.TextTl = txt;
					wxString tlLines;
					if (showOriginalOnVideo)
						Cpy.GetRaw(&tlLines, false, tlStyle);

					Cpy.GetRaw(&tlLines, true);
					dtxt->insert(selPositions[i] + moveLength, tlLines);
					moveLength += tlLines.length();
				}
				else {
					Cpy.Text = txt;
					wxString thisLine;
					Cpy.GetRaw(&thisLine);
					dtxt->insert(selPositions[i] + moveLength, thisLine);
					moveLength += thisLine.length();
				}
			}


		}

		if (!dummy) {
			tab->Video->SetVisualEdition(true);
			if (edit->splittedTags) { edit->TextEditOrig->SetModified(); }
			grid->SetModified(VISUAL_ALL_TAGS, true);
			grid->Refresh();
		}
		else {
			RenderSubs(dtxt);
		}
		return;
	}
	//put it on to editor
	if (dummy) {
		wxString txt = currentLineText;
		ChangeVisual(&txt);
		if (!dummytext) {
			bool vis = false;
			dummytext = grid->GetVisible(&vis, &dumplaced);
			if (!vis) { SAFE_DELETE(dummytext); return; }
		}

		editor->SetTextS(txt, false, false);
		FindTag(actualTag.tag + L"([0-9.,\\(\\) ]*)", L"", actualTag.mode);
		const FindData& data = GetResult();
		editor->SetSelection(data.positionInText.x, data.positionInText.x, true);
		dummytext->replace(dumplaced.x, dumplaced.y, txt);
		dumplaced.y = txt.length();
		wxString* dtxt = new wxString(*dummytext);
		RenderSubs(dtxt);
	}
	else {
		editor->SetModified();
		currentLineText = editor->GetValue();
		tab->Video->SetVisualEdition(true);
		if (edit->splittedTags) { edit->TextEditOrig->SetModified(); }
		edit->Send(VISUAL_ALL_TAGS, false, false, true);

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
	holding = rholding = false;
}

