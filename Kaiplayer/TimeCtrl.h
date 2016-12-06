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

#ifndef TIMECTRL_H_INCLUDED
#define TIMECTRL_H_INCLUDED

#include <wx/wx.h>
#include "SubsTime.h"
#include "KaiTextCtrl.h"

class TimeCtrl : public KaiTextCtrl
{
    public:
	TimeCtrl(wxWindow* parent, const long int id, const wxString& val=_("0:00:00.00"), const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize, long style=0,const wxValidator& validator=wxDefaultValidator, const wxString& name=wxTextCtrlNameStr);
	virtual ~TimeCtrl();

	void SetTime(STime newtime, bool stillModified=false);
	STime GetTime();
	void ChangeFormat(char frm, float fps=0);
	//void SetModified(bool modified);
	bool changedBackGround;
	private:
	char form;
	STime mTime;
	bool pastes;
	bool holding;
	int oldpos;
	int oldposx;
	int curpos;
	STime value;
	int grad;
	void OnTimeWrite(wxCommandEvent& event);
	void OnMouseEvent(wxMouseEvent &event);
	void OnKeyEvent(wxKeyEvent& event);
	void OnPaste(wxCommandEvent &event);
	void OnCopy(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};
enum{
	Time_Copy=4404,
	Time_Paste=4405
};


#endif // TIMECTRL_H_INCLUDED
