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

#ifndef __KAISCROLLBAR__
#define __KAISCROLLBAR__

#include <wx/wx.h>

class KaiScrollbar : public wxWindow
{
	friend class KaiScrolledWindow;
public:
	KaiScrollbar(wxWindow *parent, int id, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, int style = wxSB_HORIZONTAL);
	virtual ~KaiScrollbar(){if(bmp){delete bmp;}};
	void SetScrollbar(int pos, int visible, int range, bool refresh = true);
	int unitPos;
	int visibleSize;
	int allSize;
private:
	void OnSize(wxSizeEvent& evt);
	void OnPaint(wxPaintEvent& evt);
	void OnMouseEvent(wxMouseEvent &evt);
	void OnMouseLost(wxMouseCaptureLostEvent &evt){if(HasCapture()){ReleaseMouse();} holding=false; }
	void OnErase(wxEraseEvent &evt){};
	void SendEvent();
	int thumbPos;
	int thumbSize;
	int diff;
	int thumbRange;
	int allVisibleSize;
	bool isVertical;
	bool holding;
	bool enter;
	//bool clicked;
	bool integrated;
	byte element;
	//wxSize oldSize;
	wxTimer sendEvent;
	wxTimer arrowLoop;
	wxBitmap *bmp;
	DECLARE_EVENT_TABLE()
};

class KaiScrolledWindow : public wxWindow
{
	friend class KaiScrollbar;
public:
	KaiScrolledWindow(wxWindow *parent, int id, const wxPoint& pos = wxDefaultPosition, 
		const wxSize& size = wxDefaultSize, long style = 0, const wxString& name = wxEmptyString);
	virtual ~KaiScrolledWindow(){};
	bool SetScrollBar(int orientation, int pos, int maxVisible, int allItems, bool refresh = true);
	void SetScrollbar(int orientation, int pos, int maxVisible, int allItems, bool refresh = true){
		SetScrollBar(orientation, pos, maxVisible, allItems, refresh);
	}
	void SetScrollPos (int orientation, int pos, bool refresh=true);
	void GetSize(int *x, int *y);
	void GetClientSize(int *x, int *y);
	wxSize GetSize();
	wxSize GetClientSize();
	void AlwaysShowScrollbars (bool hflag=true, bool vflag=true){};
	int GetScrollPos (int orientation) const {return (orientation == wxHORIZONTAL)? horizontal->unitPos : vertical->unitPos;}
 	int GetScrollRange (int orientation) const{return (orientation == wxHORIZONTAL)? horizontal->allSize : vertical->allSize;}
 	int GetScrollThumb (int orientation) const{return (orientation == wxHORIZONTAL)? horizontal->visibleSize : vertical->visibleSize;}
	bool HasScrollbar (int orient) const {return (orient == wxHORIZONTAL)? horizontal->IsShown() : vertical->IsShown();};
	bool IsScrollbarAlwaysShown (int orient) const{return false;};
 	bool ScrollLines (int lines);
 	bool ScrollPages (int pages);
private:
	KaiScrollbar *horizontal;
	KaiScrollbar *vertical;
};

enum{
	ELEMENT_BUTTON_TOP=1,
	ELEMENT_BUTTON_BOTTOM=2,
	ELEMENT_THUMB=4,
	ELEMENT_BETWEEN_THUMB=8
};

#endif