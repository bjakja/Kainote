
#include "TimeCtrl.h"
#include <wx/regex.h>
#include <wx/clipbrd.h>
#include "Config.h"
#include "NumCtrl.h"

TimeCtrl::TimeCtrl(wxWindow* parent, const long int id, const wxString& val, const wxPoint& pos,const wxSize& size, long style,const wxValidator& validator, const wxString& name)
	: wxTextCtrl(parent, id, val, pos, size, style, validator, name)
{
	wxTextValidator valid(wxFILTER_INCLUDE_CHAR_LIST);
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




		if (nChar == _(":")||nChar == _(".")||nChar == _(",")) {
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


		ChangeValue(txt);
		SetModified(true);
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
			wxRegEx reg(_("[0-9]"),wxRE_ADVANCED);
			reg.ReplaceAll(&seltxt,_T("0"));
			//txt.erase(from,to);
			wxString all=txt.Left(from);
			all+=seltxt;
			all+=txt.Mid(to);
			ChangeValue(all);
			SetSelection(from,from);
			SetModified(true);
		}


	}
	if(!astmp||(key != WXK_BACK && key != WXK_DELETE)){event.Skip();}
	if (event.ControlDown()) {

		if (key == 'C' || key == 'X') {

			//SetSelection(0,GetValue().Length());
			Copy();//CopyTime();
		}
		if (key == 'V') {
			pastes=true;
			SetSelection(0,GetValue().Length());
			//Paste();
			if (wxTheClipboard->Open())
			{
				if (wxTheClipboard->IsSupported( wxDF_TEXT ))
				{
					wxTextDataObject data;
					wxTheClipboard->GetData( data );
					wxString whatpaste = data.GetText();
					ChangeValue(whatpaste);
					SetSelection(0,whatpaste.Length());
					SetModified(true);

				}
				wxTheClipboard->Close();
				wxTheClipboard->Flush();
			}

			//pastes=false;
		}
	}
}


void TimeCtrl::SetTime(STime newtime, bool stillModified)
{
	if(mTime==newtime){return;}
	mTime=newtime;
	form=mTime.GetFormat();
	ChangeValue(newtime.raw());
	if(!stillModified){
		SetModified(false);
	}else{
		SetForegroundColour("#FF0000");
		changedBackGround=true;
		SetModified(true);
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
	ChangeValue(mTime.raw());
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
			SetValue(value.GetFormatted(form));MarkDirty();oldpos=posy;
			SetSelection(curpos,curpos);
		}
		else if((oldpos-5)>posy){
			value.mstime+=grad;
			if(value.mstime==(35999999+grad)){value.mstime=35999999;return;}
			if(value.mstime>35999999){value.mstime=35999999;}
			SetValue(value.GetFormatted(form));MarkDirty();oldpos=posy;
			SetSelection(curpos,curpos);
		}
		if((oldposx+10)<posx){
			value.mstime-=(grad*10);
			if(value.mstime==(-(grad*10))){value.mstime=0;return;}
			if(value.mstime<0){value.mstime=0;}
			SetValue(value.GetFormatted(form));MarkDirty();oldposx=posx;
			SetSelection(curpos,curpos);

		}else if((oldposx-10)>posx){
			value.mstime+=(grad*10);
			if(value.mstime==35999999+(grad*10)){value.mstime=35999999;return;}
			if(value.mstime>35999999){value.mstime=35999999;}
			SetValue(value.GetFormatted(form));MarkDirty();oldposx=posx;
			SetSelection(curpos,curpos);
		}
		if(IsModified()){
			wxCommandEvent evt2(NUMBER_CHANGED, GetId()); AddPendingEvent(evt2);
		}
	}

	if(rclick)
	{
		holding=true;
		oldpos=posy;
		oldposx=posx;
		long pos;
		HitTest (wxPoint(posx, posy), &pos);
		//wxString kkk;
		//wxMessageBox(kkk<<pos);
		if(pos<2){grad=3600000;}
		else if(pos<5){grad=60000;}
		else if(pos<8){grad=1000;}
		else{grad=10;}
		value.SetRaw(GetValue(),form);
		long cpos;
		HitTest(wxPoint(posx,posy),&cpos);
		curpos=cpos;
		SetSelection(cpos,cpos);
		SetFocus();
		CaptureMouse();
	}

	if (event.GetWheelRotation() != 0) {
		long pos;
		HitTest (wxPoint(posx, posy), &pos);
		//wxString kkk;
		//wxMessageBox(kkk<<pos);
		if(pos<2){grad=3600000;}
		else if(pos<5){grad=60000;}
		else if(pos<8){grad=1000;}
		else{grad=10;}
		value.SetRaw(GetValue(),form);
		int step = event.GetWheelRotation() / event.GetWheelDelta();
		value.mstime+=(step*grad);
		if(value.mstime<0||value.mstime>35999999){return;}
		SetValue(value.GetFormatted(form));MarkDirty();

		wxCommandEvent evt2(NUMBER_CHANGED, GetId()); AddPendingEvent(evt2);
		return;}

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

BEGIN_EVENT_TABLE(TimeCtrl, wxTextCtrl)
	EVT_MOUSE_EVENTS(TimeCtrl::OnMouseEvent)
	EVT_MENU(Time_Copy,TimeCtrl::OnCopy)
	EVT_MENU(Time_Paste,TimeCtrl::OnPaste)
	END_EVENT_TABLE()
