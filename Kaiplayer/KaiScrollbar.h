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
public:
	KaiScrollbar(wxWindow *parent, int id, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, int style = wxSB_VERTICAL);
	virtual ~KaiScrollbar(){if(bmp){delete bmp;}};
	void SetScrollbar(int pos, int visible, int range);
private:
	void OnSize(wxSizeEvent& evt);
	void OnPaint(wxPaintEvent& evt);
	void OnMouseEvent(wxMouseEvent &evt);
	void OnMouseLost(wxMouseCaptureLostEvent &evt){if(HasCapture()){ReleaseMouse();} holding=false; }
	void OnErase(wxEraseEvent &evt){};
	int thumbPos;
	int unitPos;
	int visibleSize;
	int allSize;
	int thumbSize;
	int diff;
	int thumbRange;
	bool isVertical;
	bool holding;
	byte element;
	wxSize oldSize;
	wxTimer sendEvent;
	wxBitmap *bmp;
	DECLARE_EVENT_TABLE()
};

enum{
	ELEMENT_BUTTON_TOP=1,
	ELEMENT_BUTTON_BOTTOM=2,
	ELEMENT_THUMB=4,
	ELEMENT_BETWEEN_THUMB=8
};

#endif