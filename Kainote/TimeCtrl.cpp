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

#undef DRAWTEXT
#include "TimeCtrl.h"
#include "NumCtrl.h"
#include "SubsDialogue.h"
#include "Provider.h"

#include <wx/regex.h>
#include <wx/clipbrd.h>

#include "kainoteFrame.h"
#include "Config.h"


TimeCtrl::TimeCtrl(wxWindow* parent, const long int id, const wxString& val, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name)
	: KaiTextCtrl(parent, id, val, pos, size, style)
	, timeUnchanged(true)
{
	KaiTextValidator valid(wxFILTER_INCLUDE_CHAR_LIST);
	wxArrayString includes;
	includes.Add(_T("0"));
	includes.Add(_T("1"));
	includes.Add(_T("2"));
	includes.Add(_T("3"));
	includes.Add(_T("4"));
	includes.Add(_T("5"));
	includes.Add(_T("6"));
	includes.Add(_T("7"));
	includes.Add(_T("8"));
	includes.Add(_T("9"));
	valid.SetIncludes(includes);
	SetValidator(valid);

	form = ASS;
	showFrames = false;
	//pastes=false;
	holding = false;
	changedBackGround = false;
	oldposy = 0;
	oldposx = 0;
	curpos = 0;
	grad = 10;

	Connect(wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&TimeCtrl::OnTimeWrite);
	Connect(wxEVT_KEY_DOWN, (wxObjectEventFunction)&TimeCtrl::OnKeyEvent);

	SetMaxLength(20);

	Bind(wxEVT_RIGHT_DOWN, &TimeCtrl::OnMouseEvent, this);
	Bind(wxEVT_RIGHT_UP, &TimeCtrl::OnMouseEvent, this);
	Bind(wxEVT_MOTION, &TimeCtrl::OnMouseEvent, this);
	Bind(wxEVT_MOUSEWHEEL, &TimeCtrl::OnMouseEvent, this);
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){


		if (wxTheClipboard->Open())
		{
			if (wxTheClipboard->IsSupported(wxDF_TEXT))
			{
				wxTextDataObject data;
				wxTheClipboard->GetData(data);
				wxString whatpaste = data.GetText();
				if (form >= MDVD || showFrames){
					if (whatpaste.IsNumber()){
						timeUnchanged = false;
						evt.Skip();
					}
					else{
						wxBell();
					}
					wxTheClipboard->Close();
					return;
				}
				wxString pattern = (form == ASS) ? L"^[0-9]\\:[0-5][0-9]\\:[0-5][0-9]\\.[0-9][0-9]$" :
					(form == SRT) ? L"^[0-9][0-9]\\:[0-5][0-9]\\:[0-5][0-9]\\,[0-9][0-9][0-9]$" :
					L"^[0-9][0-9]\\:[0-5][0-9]\\:[0-5][0-9]$";
				wxRegEx timeCheck(pattern, wxRE_ADVANCED);
				if (timeCheck.Matches(whatpaste)){
					SetValue(whatpaste, true, false);
					SetSelection(0, whatpaste.Length());
					wxCommandEvent evt2(NUMBER_CHANGED, GetId()); AddPendingEvent(evt2);
					timeUnchanged = false;
					//pastes=true;
				}
				else{
					wxBell();
				}
			}
			wxTheClipboard->Close();
			//wxTheClipboard->Flush();
		}
	}, ID_TCTLV);
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		if (form >= MDVD || showFrames){ evt.Skip(); return; }
		SetSelection(0, GetValue().Length());
		Copy();
	}, ID_TCTLC);
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		//napisaæ tutaj zerowanie przy zaznaczeniu i ogólnie cofanie kursora i zerowanie jednej cyfry
		timeUnchanged = false;
		if (form >= MDVD || showFrames){ evt.Skip(); return; }
		long from, to;
		GetSelection(&from, &to);
		wxString timetxt = GetValue();
		if (from == to){
			if (from > 0){ from--; }
			else{ return; }
		}

		for (long i = from; i < to; i++)
		{
			wxUniChar nChar = timetxt[i];
			if (nChar != L':' && nChar != L'.' && nChar != L','){
				timetxt[i] = L'0';
			}
		}
		if (from > 0 && (timetxt[from - 1] == L':' || timetxt[from - 1] == L'.' || timetxt[from - 1] == L',')){
			from--;
		}
		SetValue(timetxt);
		SetSelection(from, from);
	}, ID_TBACK);
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		timeUnchanged = false;
		if (form >= MDVD || showFrames){ evt.Skip(); }
		//napisaæ tutaj zerowanie przy zaznaczeniu i ogólnie nieruchomy kursor i zerowanie jednej cyfry
		//w Aegi shit happens wiêc olejê.
	}, ID_TDEL);

	bool setNumpadAccels = !Options.GetBool(TEXT_FIELD_ALLOW_NUMPAD_HOTKEYS);
	if (setNumpadAccels){
		Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
			int key = evt.GetId() - 10276;
			wxKeyEvent kevt;
			kevt.m_uniChar = key;
			kevt.m_keyCode = key;
			OnKeyEvent(kevt);
			if (kevt.GetSkipped()){
				evt.Skip();
			}
		}, WXK_NUMPAD0 + 10000, WXK_NUMPAD9 + 10000);
	}

}

TimeCtrl::~TimeCtrl()
{

}

void TimeCtrl::OnTimeWrite(wxCommandEvent& event)
{
	event.Skip();
	wxString txt = GetValue();
	long selst = 0, seled = 0;
	GetSelection(&selst, &seled);

	if (selst == seled && (selst > 0) && selst < (long)txt.Len() && form < MDVD && !showFrames){

		wxString nChar = txt.Mid(selst, 1);


		if (nChar == L":" || nChar == L"." || nChar == L",") {
			wxString tmp = txt;
			txt = tmp.Left(selst - 1);
			txt += nChar;
			txt += tmp.Mid(selst - 1, 1);
			txt += tmp.Mid(selst + 2);

			selst++;
			seled++;
		}
		else if (nChar.IsEmpty()) { txt.Remove(selst - 1, 1); }
		else{ txt.Remove(selst, 1); }
		if (selst > 1 && txt[selst - 2] == L':' &&  wxAtoi(wxString(txt[selst - 1])) > 5){ 
			txt = txt.replace(selst - 1, 1, L"5"); }


		SetValue(txt, true, false);
		SetSelection(selst, seled);

	}
	//pastes=false;
	if (IsModified()){ wxCommandEvent evt2(NUMBER_CHANGED, GetId()); AddPendingEvent(evt2); }
	timeUnchanged = false;
}

void TimeCtrl::OnKeyEvent(wxKeyEvent& event)
{
	int key = event.GetKeyCode();
	bool astmp = (form < MDVD && !showFrames);
	if (astmp){
		long from = 0, to = 0;
		GetSelection(&from, &to);
		wxString txt = GetValue();

		if (to != from && (key > 47 && key < 59 || key > 323 && key < 334)) {

			wxString seltxt = txt.SubString(from, to - 1);
			wxRegEx reg(L"[0-9]", wxRE_ADVANCED);
			reg.ReplaceAll(&seltxt, _T("0"));
			//txt.erase(from,to);
			wxString all = txt.Left(from);
			all += seltxt;
			all += txt.Mid(to);
			SetValue(all, true, false);
			SetSelection(from, from);
		}
		if (from >= (long)txt.Len()){
			wxBell(); return;
		}

	}
	event.Skip();
}


void TimeCtrl::SetTime(const SubsTime &newtime, bool stillModified, int opt)
{
	if (mTime == newtime && stillModified){ return; }
	timeUnchanged = true;
	mTime = newtime;
	form = mTime.GetFormat();
	bool canShowFrames = showFrames && vb && vb->HasFFMS2();

	if (canShowFrames && opt){
		mTime.orgframe = vb->GetFFMS2()->GetFramefromMS(mTime.mstime);
		//opt 2 = end frame
		if (opt == 2) 
			mTime.orgframe--; 
	}

	SetValue(mTime.raw(canShowFrames || (showFrames && !vb) ? FRAME : form), stillModified);
	if (stillModified){
		SetForegroundColour(WINDOW_WARNING_ELEMENTS);
		changedBackGround = true;
	}
}
//0 nothing, 1 -halframe (start), 2 +halfframe (end)
SubsTime TimeCtrl::GetTime(char opt)
{
	bool canShowFrames = showFrames && vb && vb->HasFFMS2();
	mTime.SetRaw(GetValue(), canShowFrames || (showFrames && !vb)? FRAME : form);
	if (canShowFrames && !timeUnchanged){
		SubsTime cpy = SubsTime(mTime);
		cpy.ChangeFormat(form);
		int time = (!opt) ? vb->GetFFMS2()->GetMSfromFrame(cpy.orgframe) :
			vb->GetRenderer()->GetFrameTimeFromFrame(cpy.orgframe, opt == 1);

		cpy.mstime = ZEROIT(time);
		return cpy;
	}
	else{
		mTime.ChangeFormat(form, 25.f);
	}
	return mTime;
}

void TimeCtrl::ChangeFormat(char frm, float fps)
{
	mTime.ChangeFormat(frm, fps);
	form = frm;
	SetValue(mTime.raw(showFrames ? FRAME : form), false, false);
}

char TimeCtrl::GetFormat()
{
	return form;
}

void TimeCtrl::OnMouseEvent(wxMouseEvent &event) {
	bool rclick = event.RightDown();
	bool right_up = event.RightUp();
	int posy = event.GetY();
	int posx = event.GetX();

	if (holding && right_up)
	{
		holding = false;
		SetFocus();
		ReleaseMouse();
		return;
	}
	if (holding)
	{
		bool changed = false;
		int absy = abs(posy - oldposy);
		int absx = abs(posx - oldposx);
		int tmpgrad = grad;
		if (absy >= absx){
			if (absy >= 8){
				mstime = (oldposy < posy) ? mstime - grad : mstime + grad;
				//KaiLog(wxString::Format("nval+-1: %f", (float)nval));
				oldposy = posy;
				//reset oldposx cause next time it will change it by 10 
				//even if it's still moved in one direction
				oldposx = posx;
				changed = true;
			}
		}
		else{
			if (absx >= 10){
				tmpgrad = grad * 10;
				mstime = (oldposx > posx) ? mstime - tmpgrad : mstime + tmpgrad;
				oldposx = posx;
				//reset oldposx cause next time it will change it by 1
				//even if it's still moved in one direction
				oldposy = posy;
				changed = true;
			}
		}
		if (mstime < 0){ mstime = 0; }
		else if (mstime > 35999999){ mstime = 35999999; }
		if (mstime == (-tmpgrad)){ mstime = 0; return; }
		else if (mstime == (35999999 + tmpgrad)){ mstime = 35999999; return; }
		
		if (changed){
			if (showFrames){ mTime.orgframe = mstime; }
			else{ mTime.mstime = mstime; }
			SetValue(mTime.raw(showFrames ? FRAME : form), true, false);
			wxCommandEvent evt2(NUMBER_CHANGED, GetId()); AddPendingEvent(evt2);
			timeUnchanged = false;
		}
	}

	if (rclick)
	{
		holding = true;
		oldposy = posy;
		oldposx = posx;
		wxPoint pos;
		HitTest(wxPoint(posx, posy), &pos);
		int startPos = (form == ASS) ? 2 : 3;
		if (form == FRAME || form == MDVD || form == MPL2){ grad = 1; }
		else if (pos.x < startPos){ grad = 3600000; }
		else if (pos.x < startPos + 3){ grad = 60000; }
		else if (pos.x < startPos + 6){ grad = 1000; }
		else{ grad = 10; }
		mTime.SetRaw(GetValue(), showFrames ? FRAME : form);
		mstime = (form == FRAME) ? mTime.orgframe : mTime.mstime;
		curpos = pos.x;
		SetSelection(pos.x, pos.x);
		SetFocus();
		CaptureMouse();
		//return;
	}

	if (event.GetWheelRotation() != 0) {
		wxPoint pos;
		HitTest(wxPoint(posx, posy), &pos);
		int startPos = (form == ASS) ? 2 : 3;
		if (form == FRAME || form == MDVD || form == MPL2){ grad = 1; }
		else if (pos.x < startPos){ grad = 3600000; }
		else if (pos.x < startPos + 3){ grad = 60000; }
		else if (pos.x < startPos + 6){ grad = 1000; }
		else{ grad = 10; }
		mTime.SetRaw(GetValue(), showFrames ? FRAME : form);
		int step = (event.GetWheelRotation() / event.GetWheelDelta())*grad;
		if (form == FRAME){ mTime.orgframe += step; }
		else{ mTime.mstime += step; }
		if (mTime.mstime < 0 || mTime.mstime > 35999999 || mTime.orgframe < 0){ return; }
		SetValue(mTime.raw(showFrames ? FRAME : form), true, false);
		wxCommandEvent evt2(NUMBER_CHANGED, GetId()); AddPendingEvent(evt2);
		timeUnchanged = false;
		return;
	}

	event.Skip();
}

//void TimeCtrl::OnCopy(wxCommandEvent &event)
//{
//	SetFocus();
//	SetSelection(0,GetValue().Length());
//	Copy();
//}
//
//void TimeCtrl::OnPaste(wxCommandEvent &event)
//{
//	pastes=true;
//	SetFocus();
//	SetSelection(0,GetValue().Length());
//	Paste();
//	SetSelection(0,GetValue().Length());
//	timeUnchanged=false;
//	pastes=false;
//}

//void TimeCtrl::SetModified(bool modified)
//{
//	if(modified)
//	{
//		SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK));
//	}else{
//		SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
//
//	}
//	wxTextCtrl::SetModified(modified);
//}

BEGIN_EVENT_TABLE(TimeCtrl, KaiTextCtrl)
//EVT_MOUSE_EVENTS(TimeCtrl::OnMouseEvent)
//EVT_MENU(Time_Copy,TimeCtrl::OnCopy)
//EVT_MENU(Time_Paste,TimeCtrl::OnPaste)
EVT_MOUSE_CAPTURE_LOST(TimeCtrl::OnMouseLost)
END_EVENT_TABLE()
