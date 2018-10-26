//  Copyright (c) 2018, Marcin Drob

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

#include <wx/popupwin.h>
#include <wx/msw/popupwin.h>
#include <vector>

class KaiScrollbar;

enum{
	TYPE_NORMAL = 0,
	TYPE_TAG_USED_IN_VISUAL,
	TYPE_TAG_VSFILTER_MOD,
	SHOW_DESCRIPTION = 4
};

class TagListItem
{
public:
	TagListItem(const wxString & _tag, const wxString &_description, unsigned char _type){
		tag = _tag;
		description = _description;
		type = _type;
	}
	void ShowItem(int option){
		showDescription = (option & SHOW_DESCRIPTION) != 0;

		isVisible = ((option & TYPE_TAG_USED_IN_VISUAL) == (type & TYPE_TAG_USED_IN_VISUAL) &&
			(option & TYPE_TAG_VSFILTER_MOD) == (type & TYPE_TAG_VSFILTER_MOD));
	};
	void GetTagText(wxString *text){
		*text = tag;
		if (showDescription)
			*text << L" - " << description;
	}
	wxString tag;
	wxString description;
	unsigned char type;
	bool isVisible = true;
	bool showDescription = false;
};

class PopupTagList : public wxPopupWindow{
public:

	PopupTagList(wxWindow *DialogParent);
	~PopupTagList();
	void Popup(const wxPoint &pos, const wxSize &controlSize, int selectedItem);
	void CalcPosAndSize(wxPoint *pos, wxSize *size, const wxSize &controlSize);
	void SetSelection(int pos);
private:
	void OnMouseEvent(wxMouseEvent &evt);
	void OnKeyPress(wxKeyEvent &event);
	void OnPaint(wxPaintEvent &event);
	void OnScroll(wxScrollEvent& event);
	void OnLostCapture(wxMouseCaptureLostEvent &evt){ if (HasCapture()) ReleaseMouse(); };
	void InitList();
	int sel;
	int scPos;

protected:
	wxBitmap *bmp;
	std::vector<TagListItem*> itemsList;
	wxWindow *Parent;
	int orgY;
	int height;
	KaiScrollbar *scroll;
};