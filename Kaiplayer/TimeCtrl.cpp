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


#include "TimeCtrl.h"
#include <wx/regex.h>
#include <wx/clipbrd.h>
#include "Config.h"
#include "NumCtrl.h"
#include "kainoteMain.h"

TimeCtrl::TimeCtrl(wxWindow* parent, const long int id, const wxString& val, const wxPoint& pos,const wxSize& size, long style,const wxValidator& validator, const wxString& name)
	: KaiTextCtrl(parent, id, val, pos, size, style)
	,timeUnchanged(true)
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

	form=ASS;
	showFrames=false;
	pastes=false;
	holding=false;
	changedBackGround=false;
	oldpos=0;
	oldposx=0;
	curpos=0;
	grad=10;

	Connect(wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&TimeCtrl::OnTimeWrite);
	Connect(wxEVT_KEY_DOWN,(wxObjectEventFunction)&TimeCtrl::OnKeyEvent);

	SetMaxLength(20);

	Bind(wxEVT_RIGHT_DOWN, &TimeCtrl::OnMouseEvent, this);
	Bind(wxEVT_RIGHT_UP, &TimeCtrl::OnMouseEvent, this);
	Bind(wxEVT_MOTION, &TimeCtrl::OnMouseEvent, this);
	Bind(wxEVT_MOUSEWHEEL, &TimeCtrl::OnMouseEvent, this);
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		timeUnchanged=false;
		if(form>=MDVD || showFrames){evt.Skip(); return;}
		pastes=true;
		
		if (wxTheClipboard->Open())
		{
			if (wxTheClipboard->IsSupported( wxDF_TEXT ))
			{
				wxTextDataObject data;
				wxTheClipboard->GetData( data );
				wxString whatpaste = data.GetText();
				SetValue(whatpaste, true, false);
				SetSelection(0,whatpaste.Length());

			}
			wxTheClipboard->Close();
			//wxTheClipboard->Flush();
		}
	}, ID_TCTLV);
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		if(form>=MDVD || showFrames){evt.Skip(); return;}
		SetSelection(0,GetValue().Length());
		Copy();
	}, ID_TCTLC);
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		//napisaæ tutaj zerowanie przy zaznaczeniu i ogólnie cofanie kursora i zerowanie jednej cyfry
		timeUnchanged=false;
		if(form>=MDVD || showFrames){evt.Skip(); return;}
		long from, to;
		GetSelection(&from, &to);
		wxString timetxt = GetValue();
		if(from==to){
			if(from>0){from--;}
			else{return;}
		}

		for(long i = from; i < to; i++)
		{
			wxUniChar nChar = timetxt[i];
			if(nChar != ':' && nChar != '.' && nChar != ','){
				timetxt[i] = '0';
			}
		}
		if(from>0 && (timetxt[from-1] == ':' || timetxt[from-1] == '.' || timetxt[from-1] == ',')){
			from--;
		}
		SetValue(timetxt);
		SetSelection(from,from);
	}, ID_TBACK);
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		timeUnchanged=false;
		if(form>=MDVD || showFrames){evt.Skip();}
		//napisaæ tutaj zerowanie przy zaznaczeniu i ogólnie nieruchomy kursor i zerowanie jednej cyfry
		//w Aegi shit happens wiêc olejê.
	}, ID_TDEL);

}

TimeCtrl::~TimeCtrl()
{

}

void TimeCtrl::OnTimeWrite(wxCommandEvent& event)
{
	event.Skip();
	wxString txt = GetValue();
	long selst=0, seled=0;
	GetSelection(&selst,&seled);

	if(!pastes && selst==seled && (selst>0) && selst<(long)txt.Len() && form<MDVD && !showFrames){


		wxString nChar = txt.Mid(selst,1);
		//wxString aChar = txt.Mid(selst-1,1);




		if (nChar == ":"||nChar == "."||nChar == ",") {
			wxString tmp = txt;
			txt = tmp.Left(selst-1);
			txt += nChar;
			txt += tmp.Mid(selst-1,1);
			txt += tmp.Mid(selst+2);

			selst++;
			seled++;
		}
		else if(nChar.IsEmpty()) {txt.Remove(selst-1,1);}
		else{txt.Remove(selst,1);}
		if(selst>1 && txt[selst-2]==':' &&  wxAtoi(wxString(txt[selst-1])) > 5){txt=txt.replace(selst-1,1,"5");}


		SetValue(txt, true,false);
		SetSelection(selst,seled);

	}
	pastes=false;
	if(IsModified()){wxCommandEvent evt2(NUMBER_CHANGED, GetId()); AddPendingEvent(evt2);}
	timeUnchanged=false;
}

void TimeCtrl::OnKeyEvent(wxKeyEvent& event)
{
	int key = event.GetKeyCode();
	bool astmp=(form<MDVD && !showFrames);
	//wxLogStatus("key %i", key);
	if (astmp){
		long from=0,to=0;
		GetSelection(&from,&to);
		wxString txt=GetValue();
		
		if (to != from && (key > 47 && key < 59 || key > 323 && key < 334)) {
			
			wxString seltxt=txt.SubString(from,to-1);
			wxRegEx reg("[0-9]",wxRE_ADVANCED);
			reg.ReplaceAll(&seltxt,_T("0"));
			//txt.erase(from,to);
			wxString all=txt.Left(from);
			all+=seltxt;
			all+=txt.Mid(to);
			SetValue(all, true, false);
			SetSelection(from,from);
		}
		if(from >= txt.Len()){
			wxBell();return;
		}

	}
	event.Skip();
}


void TimeCtrl::SetTime(const STime &newtime, bool stillModified, int opt)
{
	if(mTime==newtime && stillModified){return;}
	timeUnchanged=true;
	mTime=newtime;
	form = mTime.GetFormat();
	if(showFrames && opt){
		VideoCtrl *vb = ((TabPanel *)Notebook::GetTab())->Video;
		if(vb->VFF){
			mTime.orgframe = vb->VFF->GetFramefromMS(mTime.mstime);
			//opt 2 = end frame
			if(opt==2){mTime.orgframe--;}
		}else{
			//wxLogMessage(_("Wideo nie jest wczytane przez FFMS2"));
		}
	}
	SetValue(mTime.raw(showFrames? FRAME : form),stillModified);
	if(stillModified){
		SetForegroundColour("#FF0000");
		changedBackGround=true;
	}
}
//0 nothing, 1 -halframe (start), 2 +halfframe (end)
STime TimeCtrl::GetTime(char opt)
{
	mTime.SetRaw(GetValue(),showFrames? FRAME : form);
	if(showFrames && !timeUnchanged){
		STime cpy = STime(mTime);
		cpy.ChangeFormat(form);
		VideoCtrl *vb = ((TabPanel *)Notebook::GetTab())->Video;
		if(vb->VFF){
			
			int time = (!opt)? vb->VFF->GetMSfromFrame(cpy.orgframe) : 
				vb->GetFrameTimeFromFrame(cpy.orgframe, opt == 1);
			cpy.mstime = ZEROIT(time);
		}else{
			//wxLogMessage(_("Wideo nie jest wczytane przez FFMS2"));
		}
		return cpy;
	}else{
		mTime.ChangeFormat(form);
	}
	return mTime;
}

void TimeCtrl::ChangeFormat(char frm, float fps)
{
	mTime.ChangeFormat(frm,fps);
	form=frm;
	SetValue(mTime.raw(showFrames? FRAME : form),false, false);
}

char TimeCtrl::GetFormat()
{
	return form;
}

void TimeCtrl::OnMouseEvent(wxMouseEvent &event) {
	bool rclick = event.RightDown();
	bool right_up = event.RightUp();
	int posy=event.GetY();
	int posx=event.GetX();

	if(holding&&right_up)
	{
		holding=false;
		SetFocus();
		ReleaseMouse();
		return;
	}
	if(holding)
	{
		bool changed = false;
		if((oldpos+5)<posy){
			mstime-=grad;
			if(mstime==(-grad)){mstime=0;return;}
			if(mstime<0){mstime=0;}
			changed=true;
			oldpos=posy;
		}
		else if((oldpos-5)>posy){
			mstime+=grad;
			if(mstime==(35999999+grad)){mstime=35999999;return;}
			if(mstime>35999999){mstime=35999999;}
			changed=true;
			oldpos=posy;
		}
		if((oldposx+10)<posx){
			mstime-=(grad*10);
			if(mstime==(-(grad*10))){mstime=0;return;}
			if(mstime<0){mstime=0;}
			changed=true;
			oldposx=posx;
		}else if((oldposx-10)>posx){
			mstime+=(grad*10);
			if(mstime==35999999+(grad*10)){mstime=35999999;return;}
			if(mstime>35999999){mstime=35999999;}
			changed=true;
			oldposx=posx;
		}
		if(changed){
			if(showFrames){mTime.orgframe = mstime;}
			else{mTime.mstime = mstime;}
			SetValue(mTime.raw(showFrames? FRAME : form),true, false);
			wxCommandEvent evt2(NUMBER_CHANGED, GetId()); AddPendingEvent(evt2);
			timeUnchanged=false;
		}
	}

	if(rclick)
	{
		//wxLogStatus("tc right");
		holding=true;
		oldpos=posy;
		oldposx=posx;
		wxPoint pos;
		HitTest (wxPoint(posx, posy), &pos);
		int startPos=(form==ASS)? 2 : 3;
		if(form == FRAME || form == MDVD || form == MPL2){grad=1;}
		else if(pos.x<startPos){grad=3600000;}
		else if(pos.x<startPos+3){grad=60000;}
		else if(pos.x<startPos+6){grad=1000;}
		else{grad=10;}
		mTime.SetRaw(GetValue(),showFrames? FRAME : form);
		mstime = (form==FRAME)? mTime.orgframe : mTime.mstime;
		curpos=pos.x;
		SetSelection(pos.x,pos.x);
		SetFocus();
		CaptureMouse();
		//return;
	}

	if (event.GetWheelRotation() != 0) {
		wxPoint pos;
		HitTest (wxPoint(posx, posy), &pos);
		int startPos=(form==ASS)? 2 : 3;
		if(form == FRAME || form == MDVD || form == MPL2){ grad=1; }
		else if(pos.x < startPos){grad=3600000;}
		else if(pos.x < startPos+3){grad=60000;}
		else if(pos.x < startPos+6){grad=1000;}
		else{grad=10;}
		mTime.SetRaw(GetValue(),showFrames? FRAME : form);
		int step = (event.GetWheelRotation() / event.GetWheelDelta())*grad;
		if(form==FRAME){mTime.orgframe += step;}
		else{mTime.mstime += step;}
		if(form == ASS && (mstime<0||mstime>35999999)){return;}
		SetValue(mTime.raw(showFrames? FRAME : form),true, false);

		wxCommandEvent evt2(NUMBER_CHANGED, GetId()); AddPendingEvent(evt2);
		timeUnchanged=false;
		return;
	}

	event.Skip();
}

void TimeCtrl::OnCopy(wxCommandEvent &event)
{
	SetFocus();
	SetSelection(0,GetValue().Length());
	Copy();
}

void TimeCtrl::OnPaste(wxCommandEvent &event)
{
	pastes=true;
	SetFocus();
	SetSelection(0,GetValue().Length());
	Paste();
	SetSelection(0,GetValue().Length());
	timeUnchanged=false;
	pastes=false;
}

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
	EVT_MENU(Time_Copy,TimeCtrl::OnCopy)
	EVT_MENU(Time_Paste,TimeCtrl::OnPaste)
	EVT_MOUSE_CAPTURE_LOST(TimeCtrl::OnMouseLost)
END_EVENT_TABLE()
