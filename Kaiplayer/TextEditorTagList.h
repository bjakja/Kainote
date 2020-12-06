//  Copyright (c) 2018 - 2020, Marcin Drob

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
	SHOW_DESCRIPTION = 4,
	ID_SHOW_ALL_TAGS = 5667,
	ID_SHOW_VSFILTER_MOD_TAGS,
	ID_SHOW_DESCRIPTION

};

class TagListItem
{
public:
	void ShowItem(int option){
		showDescription = (option & SHOW_DESCRIPTION) != 0;

		isVisible = (type == 0 || ((option & TYPE_TAG_USED_IN_VISUAL) && (type & TYPE_TAG_USED_IN_VISUAL)) ||
			((option & TYPE_TAG_VSFILTER_MOD) && (type & TYPE_TAG_VSFILTER_MOD)));
	};
	void ShowItem(const wxString &keyWord){
		if (isVisible)
			isVisible = tag.StartsWith(keyWord);
	};
	void GetTagText(wxString *text){
		*text = tag;
		if (showDescription)
			*text << L" - " << description;
	}
	void GetTag(wxString *text){
		//maybe for now only add brackets when tag need it;
		if (needBrackets)
			*text = tag + L"()";
		else
			*text = tag;
	}
	TagListItem(const wxString & _tag, const wxString &_description, unsigned char _type, int option, bool _needBrackets = false){
		tag = _tag;
		description = _description;
		type = _type;
		needBrackets = _needBrackets;
		ShowItem(option);
	}
	wxString tag;
	wxString description;
	unsigned char type;
	bool isVisible = true;
	bool showDescription = false;
	bool needBrackets = false;
};

class PopupTagList;

class PopupWindow : public wxPopupWindow{
	friend class PopupTagList;
public:

	PopupWindow(wxWindow *DialogParent, PopupTagList *Parent, int Height);
	~PopupWindow();
	void Popup(const wxPoint &pos, const wxSize &controlSize, int selectedItem);
	void SetSelection(int pos);
	int GetSelection() { return sel; }
private:
	void OnMouseEvent(wxMouseEvent &evt);
	void OnPaint(wxPaintEvent &event);
	void OnScroll(wxScrollEvent& event);
	void OnLostCapture(wxMouseCaptureLostEvent &evt){ if (HasCapture()) ReleaseMouse(); };
	int sel;
	int scrollPositionV;

protected:
	wxBitmap *bmp;
	int height;
	KaiScrollbar *scroll;
	bool blockMouseEvent = true;
	PopupTagList *parent;
};

class PopupTagList {
	friend class PopupWindow;
public:
	PopupTagList(wxWindow *DialogParent);
	~PopupTagList();
	void Popup(const wxPoint &pos, const wxSize &controlSize, int selectedItem);
	void SetSelection(int pos) { if (popup) { popup->SetSelection(pos); } };
	int GetSelection() { if (popup) { return popup->GetSelection(); } return -1; }
	size_t GetCount();
	void AppendToKeyword(wxUniChar ch);
	int GetId() {
		if (popup) {
			return popup->GetId();
		}
		return -1;
	}
	void CalcPosAndSize(wxPoint *pos, wxSize *size, const wxSize &controlSize);
	void FilterListViaOptions(int otptions);
	void FilterListViaKeyword(const wxString &keyWord, bool setKeyword = true);
	int FindItemById(int id);
	TagListItem *GetItem(int pos);
	void InitList(int option);
private:
	PopupWindow *popup = NULL;
	wxWindow *Parent = NULL;
	wxSize controlSize;
	wxPoint position;
	wxString keyWord;
	size_t lastItems = 0;
	std::vector<TagListItem*> itemsList;
	int height = 0;
};