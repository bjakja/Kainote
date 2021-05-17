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
#include <vector>
#include "Styles.h"
#include "ListControls.h"
#include "KaiScrollbar.h"

wxDECLARE_EVENT(SELECTION_CHANGED, wxCommandEvent);

class StyleList : public KaiScrolledWindow
{
public:
	StyleList(wxWindow *parent, long id, std::vector<Styles*> *stylearray,
		const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = 0);
	virtual ~StyleList();

	void SetSelection(int sel, bool reset = false);
	void SetSelections(const wxArrayInt &sels);
	void Scroll(int step);
	int GetSelections(wxArrayInt &sels);
	int GetNumSelections(){ return sels.size(); }
	void SetArray(std::vector<Styles*> *stylearray);
	void SendSelectionEvent();
	bool SetFont(const wxFont &font);
private:

	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);
	//void DrawFld(wxDC &dc, int w, int h);
	void OnScroll(wxScrollWinEvent& event);
	void OnMouseEvent(wxMouseEvent& event);
	void OnArrow(wxCommandEvent& event);
	void OnLostCapture(wxMouseCaptureLostEvent &evt){
		if (HasCapture()){ ReleaseMouse(); };
		holding = false;
	};

	int lastsel;
	int lastRow;
	bool Switchlines;
	wxArrayInt sels;
	int scPos;
	int Height;
	bool holding;
	std::vector<Styles*> *stylenames;
	//wxArrayString *fontseeker;

	//wxScrollBar *scrollBar;
	wxBitmap *bmp;
	wxFont font;

	DECLARE_EVENT_TABLE()
};


