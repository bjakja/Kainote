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

#ifndef __KAISLIDER__
#define __KAISLIDER__

#include <wx/window.h>
#include <wx/timer.h>

class KaiSlider :public wxWindow
{
public:
	KaiSlider(wxWindow *parent, int id, int value, int minRange = 0, int maxRange = 100,
             const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxHORIZONTAL);
	virtual ~KaiSlider(){wxDELETE(bmp);}
	int GetValue();
	void SetValue(int _value);
	int GetThumbPosition();
	void SetThumbPosition(int position);
private:
	void OnSize(wxSizeEvent& evt);
	void OnPaint(wxPaintEvent& evt);
	void OnMouseEvent(wxMouseEvent &evt);
	void OnLostCapture(wxMouseCaptureLostEvent &evt){if(HasCapture()){ReleaseMouse();} holding = false;};
	void SendEvent();
	wxTimer pageLoop;
	int value;
	int minRange;
	int maxRange;
	int thumbPos;
	float valueDivide;
	int thumbSize;
	int thumbRange;
	int style;
	int diff;
	bool isUpDirection;
	bool enter;
	bool pushed;
	bool holding;
	wxBitmap *bmp;
	DECLARE_EVENT_TABLE()
};

#endif