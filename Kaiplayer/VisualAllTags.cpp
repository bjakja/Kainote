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
		}
	}
	line->End();

	float thumbposdiff = -actualTag.rangeMin;
	float thumbpos = (thumbValue * coeff) + left + thumbposdiff;
	float thumbleft = thumbpos - 4;
	float thumbright = thumbpos + 4;
	float thumbtop = top - 10;
	float thumbbottom = bottom + 10;
	D3DCOLOR fill = (thumbState == 1) ? 0xAACC8748 : (thumbState == 2) ? 0xAAFCE6B1 : 0xAA121150;
	VERTEX v9[9];
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
		if (onThumb) {
			thumbValue = (x - left - thumbposdiff) / coeff;
			thumbValue = MID(actualTag.rangeMin, thumbValue, actualTag.rangeMax);
			ChangeInLines(true);
		}
	}

	if (event.LeftDown() || event.LeftDClick()) {
		if (!tab->Video->HasFocus()) {
			tab->Video->CaptureMouse();
		}
		holding = true;
		lastThumbValue = thumbValue;
	}

	if (event.LeftUp()) {
		holding = false;
		if (tab->Video->HasFocus()) {
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
	tab->Video->Render(false);
}

void AllTags::ChangeTool(int _tool)
{
	currentTag = _tool;
	SetCurVisual();
}

void AllTags::GetVisual(wxString* visual, const wxString& curValue)
{
	float value = thumbValue;
	if (changeMoveDiff) {
		double val = 0;
		if (curValue.ToCDouble(&val)) {
			value = val + (thumbValue - lastThumbValue);
		}
	}
	
	*visual = L"\\" + actualTag.tag + getfloat(value);
}

void AllTags::ChangeVisual(wxString* txt, Dialogue* _dial)
{
	wxString tagpattern = actualTag.tag + L"([0-9.-]+)";
	wxString tmp;
	tab->Edit->FindValue(tagpattern, &tmp, *txt, 0, actualTag.mode);

	wxString visualText;
	GetVisual(&visualText, tmp);
	ChangeText(txt, visualText, tab->Edit->InBracket, tab->Edit->Placed);
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

			Dialogue* Dial = grid->GetDialogue(sels[i]);
			if (skipInvisible && !(_time >= Dial->Start.mstime && _time <= Dial->End.mstime)) { continue; }

			wxString txt = Dial->GetTextNoCopy();
			ChangeVisual(&txt, Dial);
			if (!dummy) {
				grid->CopyDialogue(sels[i])->SetText(txt);
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
		wxString tmp;

		wxString tagpattern = actualTag.tag + L"([0-9.-]+)";
		edit->FindValue(tagpattern, &tmp, txt, 0, actualTag.mode);

		wxString visualText;
		GetVisual(&visualText, tmp);
		ChangeText(&txt, visualText, edit->InBracket, edit->Placed);
		if (!dummytext) {
			bool vis = false;
			dummytext = grid->GetVisible(&vis, &dumplaced);
			if (!vis) { SAFE_DELETE(dummytext); return; }
		}
		editor->SetTextS(txt, false, false);
		editor->SetSelection(edit->Placed.x, edit->Placed.x, true);
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
