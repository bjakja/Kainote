//  Copyright (c) 2020, Marcin Drob

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

#include "LineParse.h"
#include "SpellChecker.h"
#include "SubsDialogue.h"
#include "GraphicsD2D.h"
#include <wx/dc.h>
#include <wx/dcmemory.h>

void TextData::Init(const wxString &text, bool spellchecker, int subsFormat, int tagReplaceLen) {
	if (isInit)
		return;
	if (text.empty()) {
		SetEmpty();
		return;
	}

	SpellChecker::Get()->CheckTextAndBrackets(text, this, spellchecker, subsFormat, NULL, tagReplaceLen);
	isInit = true;
}

void TextData::Init2(const wxString & text, bool spellchecker, int subsFormat, std::vector<MisspellData> * misspels)
{
	SpellChecker::Get()->CheckTextAndBrackets(text, this, spellchecker, subsFormat, misspels, -1);
}

int TextData::GetCPS(Dialogue *line) const
{
	int characterTime = chars / ((line->End.mstime - line->Start.mstime) / 1000.0f);
	if (characterTime < 0 || characterTime > 999) { characterTime = 999; }
	return characterTime;
}

wxString TextData::GetStrippedWraps()
{
	if (wraps.EndsWith(L"/"))
		wraps.RemoveLast(1);
	return wraps;
}

void TextData::DrawMisspells(wxString &text, const wxPoint & pos, GraphicsContext *gc, const wxColour & col, int gridHeight)
{
	if (errors.size() > 1) {
		text.Replace(L"\t", L" ");
		gc->SetBrush(wxBrush(col));
		double bfw = 0, bfh = 0, fw = 0, fh = 0;
		for (size_t s = 0; s < errors.size(); s += 2) {
			wxString err = text.SubString(errors[s], errors[s + 1]);
			err.Trim();
			if (errors[s] > 0) {

				wxString berr = text.Mid(0, errors[s]);
				gc->GetTextExtent(berr, &bfw, &bfh);
			}
			else { bfw = 0; }

			gc->GetTextExtent(err, &fw, &fh);
			gc->DrawRectangle(pos.x + bfw + 3, pos.y, fw, gridHeight);
		}
	}
}

void TextData::DrawMisspells(wxString &text, const wxPoint & pos, wxWindow * grid, 
	wxDC * dc, const wxColour & col, int gridHeight, const wxFont &font)
{
	if (errors.size() > 1) {
		text.Replace(L"\t", L" ");
		dc->SetBrush(wxBrush(col));
		int bfw, bfh, fw, fh;
		for (size_t s = 0; s < errors.size(); s += 2) {
			wxString err = text.SubString(errors[s], errors[s + 1]);
			err.Trim();
			if (errors[s] > 0) {

				wxString berr = text.Mid(0, errors[s]);
				grid->GetTextExtent(berr, &bfw, &bfh, NULL, NULL, &font);
			}
			else { bfw = 0; }

			grid->GetTextExtent(err, &fw, &fh, NULL, NULL, &font);
			dc->DrawRectangle(pos.x + bfw + 3, pos.y, fw, gridHeight);
		}
	}
}
