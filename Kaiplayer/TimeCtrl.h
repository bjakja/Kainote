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

#pragma once

#include <wx/wx.h>
#include "SubsTime.h"
#include "KaiTextCtrl.h"

class TimeCtrl : public KaiTextCtrl
{
   public:
	TimeCtrl(wxWindow* parent, const long int id, const wxString& val="0:00:00.00", const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize, long style=0,const wxValidator& validator=wxDefaultValidator, const wxString& name=wxTextCtrlNameStr);
	virtual ~TimeCtrl();
	//0 dont setup frame, 1 start frame, 2 end frame,
	void SetTime(const STime &newtime, bool stillModified=false, int opt = 0);
	//0 nothing, 1 -halframe (start), 2 +halfframe (end),
	STime GetTime(char opt = 0);
	void ChangeFormat(char frm, float fps=0);
	char GetFormat();
	bool HasShownFrames(){return showFrames;}
	void ShowFrames(bool show=true){showFrames = show;}

	//void SetModified(bool modified);
	bool changedBackGround;
private:
	void OnMouseLost(wxMouseCaptureLostEvent& event){if(HasCapture()){ReleaseMouse();}; holding = false;};
	char form;
	STime mTime;
	bool pastes;
	bool holding;
	int oldpos;
	int oldposx;
	int curpos;
	int mstime;
	int grad;
	bool showFrames;
	bool timeUnchanged;
	void OnTimeWrite(wxCommandEvent& event);
	void OnMouseEvent(wxMouseEvent &event);
	void OnKeyEvent(wxKeyEvent& event);
	//void OnPaste(wxCommandEvent &event);
	//void OnCopy(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};
enum{
	Time_Copy=4404,
	Time_Paste=4405,
	//START_EDIT=(1<<22),
	//END_EDIT=(1<<23),
};


