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

	float ypos = VideoSize.height - 20;
	float left = 20;
	float right = VideoSize.width - 40;
	float bottom = ypos;
	float top = ypos - 8;

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
	float lastPos = 0;
	line->Begin();
	for (float i = left; i <= right; i += step) {
		if (i - lastPos > 10) {
			D3DXVECTOR2 linepoints[] = { D3DXVECTOR2(i, top - 6), D3DXVECTOR2(i, top + 8 + 6) };
			line->Draw(linepoints, 2, 0xFFBB0000);
			lastPos = i;
		}
	}
	line->End();

	float thumbposdiff = -actualTag.rangeMin;
	float thumbpos = (thumbValue * coeff) + left + thumbposdiff;
	float thumbleft = thumbpos - 4;
	float thumbright = thumbpos + 4;
	float thumbtop = top - 10;
	float thumbbottom = bottom + 10;
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
}

void AllTags::OnMouseEvent(wxMouseEvent& event)
{
	float range = actualTag.rangeMax - actualTag.rangeMin;
	if (range <= 0) {
		KaiLog(L"Bad range");
		return;
	}
	float thumbposdiff = -actualTag.rangeMin;

	float ypos = VideoSize.height - 20;
	float left = 20;
	float right = VideoSize.width - 40;
	float bottom = ypos;
	float top = ypos - 8;
	float sliderRange = right - left;
	float coeff = sliderRange / range;
	float step = actualTag.step * coeff;
	float thumbpos = (thumbValue * coeff) + left + thumbposdiff;
	float thumbleft = thumbpos - 4;
	float thumbright = thumbpos + 4;
	float thumbtop = top - 10;
	float thumbbottom = bottom + 10;
	float x = event.GetX();
	float y = event.GetY();
	bool onThumb = false;
	bool onSlider = false;
	//outside slider, nothing to do
	if ((x < left || y < thumbtop || x > right || y > thumbbottom) && !holding)
		return;

	//on thumb position
	if (x >= thumbleft && x <= thumbright && y >= thumbtop && y <= thumbbottom) {
		onThumb = true;
	}//on slider
	else if (y >= top - 5 && y <= bottom + 5)
		onSlider = true;
	//if not holding just return, nothing to do
	else if (!holding)
		return;

	if (holding) {
		//calculate new thumb value from mouse position
		thumbValue = (x - left - thumbposdiff) / coeff;
		thumbValue = MID(actualTag.rangeMin, thumbValue, actualTag.rangeMax);
		if(lastThumbValue != thumbValue)
			ChangeInLines(true);

		lastThumbValue = thumbValue;
	}

	if (event.LeftDown() || event.LeftDClick()) {
		lastThumbValue = firstThumbValue = thumbValue;
		if (onThumb) {
			if (!tab->Video->HasCapture()) {
				tab->Video->CaptureMouse();
			}
			holding = true;
		}
		else if (onSlider) {
			thumbValue = (x - left - thumbposdiff) / coeff;
			thumbValue = MID(actualTag.rangeMin, thumbValue, actualTag.rangeMax);
			ChangeInLines(true);
		}
	}

	if (event.LeftUp()) {
		holding = false;
		if (tab->Video->HasCapture()) {
			tab->Video->ReleaseMouse();
		}
		ChangeInLines(false);
	}
}

void AllTags::SetCurVisual()
{
	
	if (currentTag < 0 || currentTag >= tags->size())
		currentTag = 0;
	actualTag = (*tags)[currentTag];
	FindTagValues();
	thumbValue = actualTag.value;
	tab->Video->Render(false);
}

void AllTags::FindTagValues()
{
	if (FindTag(actualTag.tag + L"([0-9.,\\(\\) ]*)", L"", actualTag.mode)) {
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

void AllTags::GetVisualValue(wxString* visual, const wxString& curValue, bool dummy)
{
	float value = thumbValue;
	float valuediff = dummy? thumbValue - lastThumbValue : thumbValue - firstThumbValue;
	wxString strval;
	if (curValue.empty()) {
		//value = thumbValue;
		
		strval = getfloat(changeMoveDiff? MID(actualTag.rangeMin, valuediff, actualTag.rangeMax) : value);
	}
	else if (curValue.StartsWith(L"(")) {
		//remove brackets;
		wxStringTokenizer toknzr(curValue.Mid(1, curValue.length() - 2), L",", wxTOKEN_STRTOK);
		strval = L"(";
		while (toknzr.HasMoreTokens())
		{
			wxString token = toknzr.GetNextToken().Trim(false).Trim();
			double val = 0;
			if (token.ToCDouble(&val)) {
				val += valuediff;
				strval << getfloat(val);
			}
		}
		strval << L")";
	}
	else {
		double val = 0;
		wxString trimed = curValue;
		trimed.Trim(false).Trim();
		if (trimed.ToCDouble(&val)) {
			val += valuediff;
			strval = getfloat(val);
		}
	}
	
	*visual = strval;
}

void AllTags::ChangeVisual(wxString* txt, bool dummy)
{
	auto replfunc = [=](const FindData& data, wxString* result) {
		GetVisualValue(result, data.finding, dummy);
	};
	ReplaceAll(actualTag.tag + L"([0-9.,\\(\\) ]*)", actualTag.tag, txt, replfunc, true);
}

void AllTags::ChangeInLines(bool dummy)
{
	EditBox* edit = tab->Edit;
	SubsGrid* grid = tab->Grid;

	bool isOriginal = (grid->hasTLMode && edit->TextEdit->GetValue() == L"");
	//Get editor
	TextEditor* editor = (isOriginal) ? edit->TextEditOrig : edit->TextEdit;
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
			ChangeVisual(&txt, false);
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
		wxString txt = editor->GetValue();
		ChangeVisual(&txt, dummy);
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

