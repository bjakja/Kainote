//  Copyright (c) 2016, Marcin Drob

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

#ifndef __KAI_TOOLBAR__
#define __KAI_TOOLBAR__

#include <wx/wx.h>
#include "Menu.h"
#include <vector>

static int iconsize=24;

class toolitem
{
public:
	toolitem(wxBitmap *_icon, const wxString& _label, int _id, bool _enable, byte _type)
	{
		icon=_icon; label=_label; id=_id; enabled=_enable;type=_type;size=iconsize;
	}
	//types 0 - normal icon, 1 -icon with submenu 2 - clickable element, 3 - spacer 
	toolitem(byte _type, int _size, int _id=-1, bool enable=false)
	{
		type = _type; size=_size; enabled=enable; id=_id;
	}
	bool Enable(bool enable)
	{
		if(enabled!=enable){enabled=enable;return true;}
		return false;
	}
	wxBitmap GetBitmap()
	{
		if(!icon){return wxBitmap();}
		if(!enabled){
			return wxBitmap(icon->ConvertToImage().ConvertToGreyscale());
		}
		return *icon;
	}
	int GetType(){
		return type;
	}
	wxBitmap *icon;
	wxString label;
	int id;
	int size;
	byte type;
	bool enabled;
};

class KaiToolbar :public wxWindow
{
	friend class ToolbarMenu;
public:
	KaiToolbar(wxWindow *Parent, MenuBar *mainm, int id, bool vertical);
	virtual ~KaiToolbar();

	void AddItem(int id, const wxString &label, wxBitmap *normal,bool enable, byte type=0);
	void InsertItem(int id, int index, const wxString &label, wxBitmap *normal,bool enable, byte type=0);
	void AddSpacer();
	void InsertSpacer(int index);
	void UpdateId(int id, bool enable);
	bool Updatetoolbar();
	void InitToolbar();
	void AddID(int id);
	wxArrayInt ids;

private:
	void OnMouseEvent(wxMouseEvent &event);
	void OnToolbarOpts(wxCommandEvent &event);
	void OnPaint(wxPaintEvent &event);
	void OnSize(wxSizeEvent &evt);
	wxPoint FindElem(wxPoint pos);
	std::vector<toolitem*> tools;
	
	bool vertical;
	bool Clicked;
	bool wasmoved;
	int wh;
	int oldelem;
	int sel;
	wxBitmap *bmp;
	MenuBar *mb;
	DECLARE_EVENT_TABLE()
};

class ToolbarMenu :public wxDialog
{
	friend class KaiToolbar;
public:
	ToolbarMenu(KaiToolbar*parent, const wxPoint &pos);
	virtual ~ToolbarMenu(){wxDELETE(bmp);};

private:
	void OnMouseEvent(wxMouseEvent &evt);
	void OnPaint(wxPaintEvent &event);
	void OnScroll(wxScrollEvent& event);
	void OnIdle(wxIdleEvent& event);
	void OnLostCapture(wxMouseCaptureLostEvent &evt){if(HasCapture()){ReleaseMouse();}};
	KaiToolbar*parent;
	wxBitmap *bmp;
	KaiScrollbar *scroll;
	int sel;
	int fh;
	int scPos;
	DECLARE_EVENT_TABLE()
};

#endif