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

TimeCtrl::TimeCtrl(wxWindow* parent, const long int id, const wxString& val, const wxPoint& pos,const wxSize& size, long style,const wxValidator& validator, const wxString& name)
	: KaiTextCtrl(parent, id, val, pos, size, style)
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
		pastes=true;
		//SetSelection(0,GetValue().Length());
		//Paste();
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
		SetSelection(0,GetValue().Length());
		Copy();
	}, ID_TCTLC);
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		//napisaæ tutaj zerowanie przy zaznaczeniu i ogólnie cofanie kursora i zerowanie jednej cyfry
		if(form>=MDVD){evt.Skip(); return;}
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
		if(form>=MDVD){evt.Skip();}
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

	if(!pastes&&selst==seled&&(selst>0)&&(selst<(long)txt.Len()||form<MDVD)){


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
}

void TimeCtrl::OnKeyEvent(wxKeyEvent& event)
{
	int key = event.GetKeyCode();
	bool astmp=(form<MDVD);
	//wxLogStatus("key %i", key);
	if (astmp){

		long from=0,to=0;
		GetSelection(&from,&to);
		if (to != from && (key > 47 && key < 59 || key > 323 && key < 334)) {
			wxString txt=GetValue();
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


	}
	event.Skip();
}


void TimeCtrl::SetTime(STime newtime, bool stillModified)
{
	if(mTime==newtime && stillModified){return;}
	mTime=newtime;
	form=mTime.GetFormat();
	SetValue(newtime.raw(),stillModified);
	if(stillModified){
		SetForegroundColour("#FF0000");
		changedBackGround=true;
	}
}

STime TimeCtrl::GetTime()
{
	mTime.ParseMS(GetValue());
	return mTime;
}

void TimeCtrl::ChangeFormat(char frm, float fps)
{
	mTime.ChangeFormat(frm,fps);
	form=frm;
	SetValue(mTime.raw(),false, false);
}

void TimeCtrl::OnMouseEvent(wxMouseEvent &event) {
	/*if (event.RightUp()) {

	wxMenu menu;
	menu.Append(Time_Copy,_("&Kopiuj"));
	menu.Append(Time_Paste,_("&Wklej"));
	PopupMenu(&menu);
	return;

	}
	*/
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
		if((oldpos+5)<posy){
			value.mstime-=grad;
			if(value.mstime==(-grad)){value.mstime=0;return;}
			if(value.mstime<0){value.mstime=0;}
			SetValue(value.GetFormatted(form),true, false);oldpos=posy;
			SetSelection(curpos,curpos);
		}
		else if((oldpos-5)>posy){
			value.mstime+=grad;
			if(value.mstime==(35999999+grad)){value.mstime=35999999;return;}
			if(value.mstime>35999999){value.mstime=35999999;}
			SetValue(value.GetFormatted(form),true, false);oldpos=posy;
			SetSelection(curpos,curpos);
		}
		if((oldposx+10)<posx){
			value.mstime-=(grad*10);
			if(value.mstime==(-(grad*10))){value.mstime=0;return;}
			if(value.mstime<0){value.mstime=0;}
			SetValue(value.GetFormatted(form),true, false);oldposx=posx;
			SetSelection(curpos,curpos);

		}else if((oldposx-10)>posx){
			value.mstime+=(grad*10);
			if(value.mstime==35999999+(grad*10)){value.mstime=35999999;return;}
			if(value.mstime>35999999){value.mstime=35999999;}
			SetValue(value.GetFormatted(form),true, false);oldposx=posx;
			SetSelection(curpos,curpos);
		}
		if(IsModified()){
			wxCommandEvent evt2(NUMBER_CHANGED, GetId()); AddPendingEvent(evt2);
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
		//wxString kkk;
		//wxMessageBox(kkk<<pos);
		if(pos.x<2){grad=3600000;}
		else if(pos.x<5){grad=60000;}
		else if(pos.x<8){grad=1000;}
		else{grad=10;}
		value.SetRaw(GetValue(),form);
		//wxPoint cpos;
		//HitTest(wxPoint(posx,posy),&cpos);
		curpos=pos.x;
		SetSelection(pos.x,pos.x);
		SetFocus();
		CaptureMouse();
		//return;
	}

	if (event.GetWheelRotation() != 0) {
		wxPoint pos;
		HitTest (wxPoint(posx, posy), &pos);
		//wxString kkk;
		//wxMessageBox(kkk<<pos);
		if(pos.x<2){grad=3600000;}
		else if(pos.x<5){grad=60000;}
		else if(pos.x<8){grad=1000;}
		else{grad=10;}
		value.SetRaw(GetValue(),form);
		int step = event.GetWheelRotation() / event.GetWheelDelta();
		value.mstime+=(step*grad);
		if(value.mstime<0||value.mstime>35999999){return;}
		SetValue(value.GetFormatted(form),true, false);

		wxCommandEvent evt2(NUMBER_CHANGED, GetId()); AddPendingEvent(evt2);
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
