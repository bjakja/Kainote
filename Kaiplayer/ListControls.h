//  Copyright (c) 2016 - 2020, Marcin Drob

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

#include <map>
#include <wx/popupwin.h>
#include <wx/msw/popupwin.h>
#include <wx/object.h>

class KaiTextCtrl;
class KaiScrollbar;
enum COLOR;

class PopupList : public wxPopupWindow{
	friend class KaiChoice;
public:

	PopupList(wxWindow *DialogParent, wxArrayString *list, std::map<int, bool> *disabled);
	~PopupList();
	void Popup(const wxPoint &pos, const wxSize &controlSize, int selectedItem);
	void CalcPosAndSize(wxPoint *pos, wxSize *size, const wxSize &controlSize);
	void EndPartialModal(int ReturnId);
	void SetSelection(int pos);
	void ScrollTo(int pos){
		scPos = pos; 
		Refresh(false);
	}
private:
	void OnMouseEvent(wxMouseEvent &evt);
	void OnKeyPress(wxKeyEvent &event);
	void OnPaint(wxPaintEvent &event);
	void OnScroll(wxScrollEvent& event);
	void OnIdle(wxIdleEvent& event);
	void OnLostCapture(wxMouseCaptureLostEvent &evt){ if (HasCapture()){ ReleaseMouse(); } };
	int sel;
	int scPos;
	
protected:
	wxBitmap *bmp;
	wxArrayString *itemsList;
	std::map<int, bool> *disabledItems;
	wxWindow *Parent;
	int orgY;
	KaiScrollbar *scroll;
	wxPoint originalPosition;
	DECLARE_EVENT_TABLE()
};

class KaiChoice :public wxWindow
{
	friend class PopupList;
public:
	//normal list wxString[]
	KaiChoice(wxWindow *parent, int id, const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize, int n = 0, const wxString choices[] = NULL,
		long style = 0, const wxValidator& validator = wxDefaultValidator);
	//normal list wxArrayString
	KaiChoice(wxWindow *parent, int id, const wxPoint& pos,
		const wxSize& size, const wxArrayString &choices,
		long style = 0, const wxValidator& validator = wxDefaultValidator);

	//combobox
	KaiChoice(wxWindow *parent, int id, const wxString &comboBoxText, const wxPoint& pos,
		const wxSize& size, const wxArrayString &choices,
		long style = 0, const wxValidator& validator = wxDefaultValidator);

	virtual ~KaiChoice();
	void SetSelection(int sel, bool changeText = true);
	void Clear();
	/*void Prepend(wxString what);*/
	void Insert(const wxString &what, int position);
	wxString GetString(int pos);
	int GetSelection(){ return choice; };
	void SetValue(const wxString &text);
	wxString GetValue();
	void ChangeListElementName(int position, const wxString & newElementName);
	void Select(int sel){ choice = sel; }
	int GetCount();
	int FindString(const wxString &text, bool caseSensitive = false);
	void EnableItem(int numItem, bool enable = true);
	int Append(const wxString &item);
	void Append(const wxArrayString &itemsArray);
	void PutArray(wxArrayString *arr);
	void GetArray(wxArrayString *arr){ if (arr) *arr = *list; }
	void Delete(int num, int nRemove = 1);
	void SetToolTip(const wxString &tooltip = L"");
	void Sort();
	bool SetBackgroundColour(COLOR col);
	bool SetForegroundColour(COLOR col);
	bool HasFocus();
	void SetFocus();
	void SetMaxLength(int maxLen);
	wxSize GetBestSize(){
		wxSize newSize;
		CalcMaxWidth(&newSize, true, true);
		return newSize;
	}
	bool SetFont(const wxFont &font);
	KaiTextCtrl *choiceText;
private:
	void OnSize(wxSizeEvent& event);
	void OnPaint(wxPaintEvent& event);
	void OnMouseEvent(wxMouseEvent &event);
	void OnKeyPress(wxKeyEvent &event);
	void OnKeyHook(wxKeyEvent &event);
	void OnEraseBackground(wxEraseEvent &event){}
	void ShowList();
	void SendEvent(int choice);
	void SetSelectionByPartialName(const wxString &partialName, bool setText = false, bool selectOnList = false);
	void SelectChoice(int sel, bool select = true, bool sendEvent = true);
	inline void CalcMaxWidth(wxSize *result, bool changex, bool changey);
	void OnArrow(wxCommandEvent &evt);
	void OnActivate(wxFocusEvent &evt);
	bool enter;
	bool clicked;
	wxArrayString *list;
	std::map<int, bool> *disabled;
	wxBitmap *bmp;
	int choice;
	bool listIsShown;
	bool focusSet;
	wxString toolTip;
	PopupList *itemList;
	wxString txtchoice;
	COLOR foreground;

	wxDECLARE_ABSTRACT_CLASS(KaiChoice);
	DECLARE_EVENT_TABLE()
};

enum{
	KAI_COMBO_BOX = 1,
	KAI_SCROLL_ON_FOCUS,
	KAI_FONT_LIST = 4
};

