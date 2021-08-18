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

#include <wx/panel.h>
#include <wx/frame.h>
#include "VideoSlider.h"
#include "BitmapButton.h"
#include "KaiTextCtrl.h"
#include "KaiStaticText.h"
#include "KaiCheckBox.h"

#undef GetClassInfo
class VideoToolbar;

class Fullscreen : public wxFrame
{
	friend class VideoCtrl;
public:
	Fullscreen(wxWindow* parent, const wxPoint& pos, const wxSize &size);
	virtual ~Fullscreen();
	BitmapButton* bprev;
	BitmapButton* bpause;
	BitmapButton* bstop;
	BitmapButton* bnext;
	BitmapButton* bpline;
	KaiTextCtrl* mstimes;
	KaiStaticText *Videolabel;
	VideoSlider* vslider;
	VolSlider* volslider;
	wxPanel* panel;
	VideoToolbar *vToolbar;
	KaiCheckBox *showToolbar;
	void OnSize();	
	void HideToolbar(bool hide);
	int panelsize;
	int buttonSection;
private:
	void OnMouseEvent(wxMouseEvent& evt);
	void OnKeyPress(wxKeyEvent& evt);
	void SetAccels();
	void OnUseWindowHotkey(wxCommandEvent& event);
	void OnPaint(wxPaintEvent& evt);
	wxWindow *vb;
	int toolBarHeight;
	DECLARE_EVENT_TABLE()
};

