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

#ifndef CTwindow_H
#define CTwindow_H
#include "TimeCtrl.h"
#include "SubsTime.h"
#include "NumCtrl.h"
#include "MappedButton.h"
#include "ListControls.h"
#include "KaiRadioButton.h"
#include <wx/wx.h>
#include <wx/collpane.h>
#include "KaiStaticBoxSizer.h"

class kainoteFrame;



class CTwindow: public wxWindow//wxScrolled<wxWindow>
{
public:
	
	CTwindow(wxWindow* parent,kainoteFrame* kfparent,wxWindowID id=-1,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize,long style=0);
	virtual ~CTwindow();
	KaiRadioButton* StartVAtime;
	KaiRadioButton* EndVAtime;
	KaiChoice *WhichLines;
	KaiChoice *WhichTimes;
	KaiRadioButton* Forward;
	KaiRadioButton* Backward;

	MappedButton* AddStyles;
	MappedButton* MoveTime;
	TimeCtrl* TimeText;
	KaiTextCtrl* Stylestext;
	KaiCheckBox* videotime;
	KaiCheckBox* audiotime;
	KaiChoice* CorTime;
	KaiCheckBox* LeadIn;
	KaiCheckBox* LeadOut;
	KaiCheckBox* Continous;
	KaiCheckBox* SnapKF;
	NumCtrl* LITime;
	NumCtrl* LOTime;
	NumCtrl* ThresStart;
	NumCtrl* ThresEnd;
	NumCtrl* BeforeStart;
	NumCtrl* AfterStart;
	NumCtrl* BeforeEnd;
	NumCtrl* AfterEnd;
	KaiStaticBoxSizer *liosizer;
	KaiStaticBoxSizer *consizer;
	KaiStaticBoxSizer *snapsizer;
	KaiScrollbar *scroll;
	wxWindow *panel; 

	void Contents(bool addopts=true);
	void RefVals(CTwindow *from=NULL);
	void OnOKClick(wxCommandEvent& event);
	wxBoxSizer *Main;


private:

	char form;
	kainoteFrame* Kai;
	bool isscrollbar;
	int scPos;
	MappedButton *coll;

	void OnAddStyles(wxCommandEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnScroll(wxScrollEvent& event);
	void OnMouseScroll(wxMouseEvent& event);
	void AudioVideoTime(wxCommandEvent &event);
	void CollapsePane(wxCommandEvent &event);
	void DoTooltips();
	bool SetBackgroundColour(const wxColour &col);
	bool SetForegroundColour(const wxColour &col);
	DECLARE_EVENT_TABLE()
};
/*
class mypanel: public wxPanel
{
public:
mypanel(wxWindow* parent);
virtual ~mypanel();
private:
void OnMouseEvent(wxMouseEvent& event);

CTwindow *CTwin;
wxWindow *focused;
DECLARE_EVENT_TABLE()
};
*/

enum{
	ID_RADIOBUTTON1=11134,
	ID_RADIOBUTTON2,
	ID_BSTYLE,
	ID_CLOSE,
	ID_SCROLL,
	ID_MOVE,
	ID_VIDEO,
	ID_AUDIO
};

#endif