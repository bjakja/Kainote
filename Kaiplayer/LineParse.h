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

#include <wx/wx.h>

class Dialogue;

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
	};
	size_t size() { return errors.GetCount(); }
	//void insert(_wxArraywxArrayInt *it, size_t n, const _wxArraywxArrayInt &val) { errors.insert(it, n, val); }
	void Add(int val) { errors.Add(val); };
	_wxArraywxArrayInt & operator[](size_t i) const { return errors[i]; }
	void Init(const wxString &text, bool spellchecker, int subsFormat, int tagReplaceLen);
	void Init2(const wxString &text, bool spellchecker, int subsFormat, wxArrayString *misspels);
	int GetCPS(Dialogue *line) const;
};