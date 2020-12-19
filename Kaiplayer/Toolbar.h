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

#include <wx/wx.h>
#include "Menu.h"
#include <vector>


class toolitem
{
public:
	toolitem::toolitem(wxBitmap *_icon, const wxString& _label, short _id, bool _enable, byte _type, bool _toggled)
	{
		icon = _icon; label = _label; id = _id; enabled = _enable; type = _type; size = 24;
		if (type == 2){ toggled = _toggled; }
	}
	//types 0 - normal icon, 1 -icon with submenu, 2 - togglebutton, 3 - clickable element, 4 - spacer
	toolitem(byte _type, byte _size, short _id = -1, bool enable = false)
	{
		type = _type; size = _size; enabled = enable; id = _id;
	}
	bool Enable(bool enable)
	{
		if (enabled != enable){ enabled = enable; return true; }
		return false;
	}
	wxBitmap GetBitmap()
	{
		if (!icon){ return wxBitmap(); }
		if (!enabled){
			return wxBitmap(icon->ConvertToImage().ConvertToGreyscale());
		}
		return *icon;
	}
	int GetType(){
		return type;
	}
	wxBitmap *icon = NULL;
	wxString label;
	short id;
	byte size;
	byte type;
	bool toggled = false;
	bool enabled;
};

class KaiToolbar :public wxWindow
{
	friend class ToolbarMenu;
public:
	KaiToolbar(wxWindow *Parent, MenuBar *mainm, int id);
	virtual ~KaiToolbar();

	void AddItem(int id, const wxString &label, wxBitmap *normal, bool enable, byte type = 0, bool toggled = false);
	void InsertItem(int id, int index, const wxString &label, wxBitmap *normal, bool enable, byte type = 0, bool toggled = false);
	void AddSpacer();
	void InsertSpacer(int index);
	toolitem *FindItem(int id);
	void UpdateId(int id, bool enable);
	bool Updatetoolbar();
	void InitToolbar();
	void AddID(int id);
	int GetThickness(){ return toolbarSize; }
	//void ChangeOrientation(byte _alignment){ alignment = _alignment; }
	bool SetFont(const wxFont &font);
	wxArrayInt ids;
private:
	void OnMouseEvent(wxMouseEvent &event);
	void OnToolbarOpts(wxCommandEvent &event);
	void OnPaint(wxPaintEvent &event);
	void OnSize(wxSizeEvent &evt);
	//void OnEraseBackground(wxEraseEvent &evt){};
	wxPoint FindElem(wxPoint pos);
	std::vector<toolitem*> tools;

	byte alignment;
	bool Clicked;
	bool wasmoved;
	int wh;
	int oldelem;
	int sel;
	int thickness = 24;
	int toolbarSize = 24;
	wxBitmap *bmp;
	MenuBar *mb;

	DECLARE_EVENT_TABLE()
};

class ToolbarMenu :public wxDialog
{
	friend class KaiToolbar;
public:
	ToolbarMenu(KaiToolbar *parent, const wxPoint &pos, const wxSize &size, int _height);
	virtual ~ToolbarMenu(){ wxDELETE(bmp); };
private:
	void OnMouseEvent(wxMouseEvent &evt);
	void OnPaint(wxPaintEvent &event);
	void OnScroll(wxScrollEvent& event);
	void OnIdle(wxIdleEvent& event);
	void OnLostCapture(wxMouseCaptureLostEvent &evt){ if (HasCapture()){ ReleaseMouse(); } };
	KaiToolbar *parent;
	wxBitmap *bmp;
	KaiScrollbar *scroll;
	KaiChoice *alignments;
	int sel;
	int fh;
	int scPos;
	DECLARE_EVENT_TABLE()
};

#define TOOLBAR_EVENT 1000
