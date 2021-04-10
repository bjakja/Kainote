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

#pragma once

#include "KaiDialog.h"
#include "KaiListCtrl.h"
#include "KaiTextCtrl.h"
#include "KaiCheckBox.h"
#include "MappedButton.h"
#include <map>

class KainoteFrame;

class DateCompare {
public:
	bool operator()(wxString i, wxString j) const {
		int y1, m1, d1, h1, mi1, s1;
		int y2, m2, d2, h2, mi2, s2;
		int c = swscanf_s(i.wc_str(), L"%d/%d/%d   %d:%d:%d", &m1, &d1, &y1, &h1, &mi1, &s1);
		int c1 = swscanf_s(j.wc_str(), L"%d/%d/%d   %d:%d:%d", &m2, &d2, &y2, &h2, &mi2, &s2);
		if (c == 6 && c1 == 6) {
			if (y1 != y2) {
				return y1 > y2;
			}
			else if (m1 != m2) {
				return m1 > m2;
			}
			else if (d1 != d2) {
				return d1 > d2;
			}
			else if (h1 != h2) {
				return h1 > h2;
			}
			else if (mi1 != mi2) {
				return mi1 > mi2;
			}
			return s1 > s2;
		}
		return i.CmpNoCase(j) > 0;
	} // returns x>y
};

class AutoSaveOpen : public KaiDialog 
{
public:
	AutoSaveOpen(KainoteFrame *_Kai);
	~AutoSaveOpen();
private:
	void FindFiles();
	void GenerateList();
	void SetSelection(int num, bool isId = false);
	void OnFindFiles(wxCommandEvent& evt);
	void OnOkClick(wxCommandEvent& evt);
	void OnListClick(wxCommandEvent& evt);
	void OnEnter(wxCommandEvent& evt);
	KaiListCtrl *filesList;
	KaiListCtrl *versionList;
	KaiTextCtrl* seekingText;
	KaiCheckBox* seekAllWords;
	MappedButton* filterList;
	MappedButton* open;
	KainoteFrame* Kai;
	typedef std::map<wxString, wxString, DateCompare> versionMap;
	std::vector<versionMap*> versions;
	wxArrayString paths;
};

enum {
	ID_AUTO_SAVE_SEEKING_TEXT = 22345,
	ID_AUTO_SAVE_FILTER,
	ID_AUTO_SAVE_OK,
	ID_AUTO_SAVE_LIST,
	ID_AUTO_SAVE_ENTER
};