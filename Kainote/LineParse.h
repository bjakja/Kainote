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

#pragma once

#include <wx/string.h>
#include <wx/dynarray.h>
#include <wx/window.h>
#include <vector>

class Dialogue;
class GraphicsContext;

class MisspellData {
public:
	wxString misspell;
	int posStart;
	int posEnd;
	MisspellData() {
		posStart = -1;
		posEnd = -1;
	};
	MisspellData(const wxString &_misspell, int posstart, int posend) {
		misspell = _misspell;
		posStart = posstart;
		posEnd = posend;
	}
};

class TextData {
public:
	short chars = 0;
	wxString wraps;
	wxArrayInt errors;
	bool isInit = false;
	bool badWraps = false;
	void clear() {
		errors.Clear(); 
		isInit = false;
		chars = 0;
		wraps.clear();
		badWraps = false;
	};
	size_t size() { return errors.GetCount(); }
	//void insert(_wxArraywxArrayInt *it, size_t n, const _wxArraywxArrayInt &val) { errors.insert(it, n, val); }
	void Add(int val) { errors.Add(val); };
	_wxArraywxArrayInt & operator[](size_t i) const { return errors[i]; }
	void Init(const wxString &text, bool spellchecker, int subsFormat, int tagReplaceLen);
	void Init2(const wxString &text, bool spellchecker, int subsFormat, std::vector<MisspellData> *misspels);
	void SetEmpty() {
		errors.Clear();
		isInit = true;
		chars = 0;
		wraps = L"0/";
		badWraps = false;
	}
	int GetCPS(Dialogue *line) const;
	wxString GetStrippedWraps();
	void DrawMisspells(wxString &text, const wxPoint &pos, GraphicsContext *gc, const wxColour &col, int gridHeight);
	void DrawMisspells(wxString &text, const wxPoint &pos, wxWindow *grid, 
		wxDC *dc, const wxColour &col, int gridHeight, const wxFont &font);
};

