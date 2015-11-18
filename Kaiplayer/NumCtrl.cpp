
#include "NumCtrl.h"


wxString getdouble(double num)
	{
	wxString strnum=wxString::Format(_T("%f"),num);
	strnum.Replace(",",".");
	int rmv=0;
	for(int i=strnum.Len()-1;i>0;i--)
		{
		if(strnum[i]=='0'){rmv++;}
		else if(strnum[i]=='.'){rmv++;break;}
		else{break;}
		}
	if(rmv){strnum.RemoveLast(rmv);}
	return strnum;
	}

NumCtrl::NumCtrl(wxWindow *parent,long id,wxString text, int rangefrom, int rangeto, bool intonly, const wxPoint &pos, const wxSize &size, long style)
	:wxTextCtrl(parent, id, text, pos, size, style)
	{

	rfrom=rangefrom;
	rto=rangeto;
	if (rto<rfrom){rto=rangefrom;rfrom=rangeto;}
	oint=intonly;

	value=rfrom;
	oldpos=rfrom;
	holding=false;
	SetString(text);

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
	if (rfrom<0){
		includes.Add(_T("-"));
		}
	if (!oint){
		includes.Add(_T("."));
		includes.Add(_T(","));
		}
	valid.SetIncludes(includes);
	SetValidator(valid);

	Connect(wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&NumCtrl::OnNumWrite);
    //Connect(wxEVT_KEY_DOWN,(wxObjectEventFunction)&NumCtrl::OnKeyEvent);
	
	}

NumCtrl::~NumCtrl()
	{
}


void NumCtrl::SetString(wxString val)
	{
	if(!val.ToDouble(&value))
		{val.ToCDouble(&value);}

	if(oint){
		int finds=val.Find('.',true);
		if(finds!=-1){val=val.BeforeFirst('.');}
		finds=val.Find(',',true);
		if(finds!=-1){val=val.BeforeFirst(',');}
		}else{
			val.Replace(",",".");}
	if(value>(double)rto)
		{
		value=rto;
		val = getdouble(value);
		}
	if(value<(double)rfrom)
		{
		value=rfrom;
		val = getdouble(value);
		}
	oldval=val;
	SetValue(val);
	}

void NumCtrl::SetInt(int val)
{
	if(val>rto){val=rto;}
	else if(val<rfrom){val=rfrom;}
	value=(double)val;
	wxString kkk;
	oldval=kkk<<val;
	SetValue(kkk);
}

void NumCtrl::SetDouble(double val)
{
	if(val>(double)rto){val=rto;}
	else if(val<(double)rfrom){val=rfrom;}
	value=val;
	oldval=getdouble(val);
	SetValue(oldval);
}

wxString NumCtrl::GetString()
{
	wxString val=GetValue();
	val.Replace(",",".");
	if(val=="-"){val=oldval;}
	if(val.StartsWith('.')){val.Prepend("0");}
	if(val.EndsWith('.')){val.Append("0");}
	if(!val.ToCDouble(&value)){
		val=oldval;
	}
	return val;
}

int NumCtrl::GetInt()
{
	wxString val=GetString();
	val.ToCDouble(&value);
	//if(value>(double)rto)
	//	value=rto;
	//if(value<(double)rfrom)
	//	value=rfrom;
    return (int)value;
}

double NumCtrl::GetDouble()
{
	wxString val=GetString();
	val.ToCDouble(&value);
	//if(value>(double)rto)
	//	value=(double)rto;
	//if(value<(double)rfrom)
	//	value=(double)rfrom;
    return value;
}



void NumCtrl::OnNumWrite(wxCommandEvent& event)
{
	long from,to;
	GetSelection(&from,&to);
	wxString val=GetValue();
	val.Replace(",",".");
	if(val=="-"||val==""){}
	else if(val.EndsWith(".")){if(val.Replace(".","")>1){SetValue(oldval);wxBell();}}
	else if(val.StartsWith(".")){if(val.Replace(".","")>1){SetValue(oldval);wxBell();}}
	else if(!val.ToCDouble(&value)||value>(double)rto||value<(double)rfrom){
		SetValue(oldval);wxBell();
	}else{oldval=val;}
	if(IsModified()){wxCommandEvent evt2(wxEVT_COMMAND_BUTTON_CLICKED, GetId()); AddPendingEvent(evt2);}
	event.Skip();
}

void NumCtrl::OnMouseEvent(wxMouseEvent &event)
{
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
			double nval=value-1;
			if(value<(double)rfrom){return;}
			SetValue(getdouble(nval));MarkDirty();
			oldpos=posy;value=nval;
			SetSelection(curpos,curpos);
		}else if((oldpos-5)>posy){
			double nval=value+1;
			if(value>(double)rto){return;}
			SetValue(getdouble(nval));MarkDirty();
			oldpos=posy;value=nval;
			SetSelection(curpos,curpos);
		}else if((oldposx+10)<posx){
			double nval=value-10;
			if(value==(double)rfrom){return;}
			if(nval<(double)rfrom){nval=rfrom;}
			SetValue(getdouble(nval));MarkDirty();oldposx=posx;value=nval;
			SetSelection(curpos,curpos);
		}
		else if((oldposx-10)>posx){
			double nval=value+10;
			if(value==(double)rto){return;}
			if(nval>(double)rto){nval=rto;}
			SetValue(getdouble(nval));MarkDirty();oldposx=posx;value=nval;
			SetSelection(curpos,curpos);
		}
		if(IsModified()){wxCommandEvent evt2(wxEVT_COMMAND_BUTTON_CLICKED, GetId()); AddPendingEvent(evt2);}
	}

	if(rclick)
	{
		holding=true;
		oldpos=posy;
		oldposx=posx;
		long cpos;
		HitTest(wxPoint(posx,posy),&cpos);
		curpos=cpos;
		SetSelection(cpos,cpos);
		SetFocus();
		CaptureMouse();
	}

	
	if (event.GetWheelRotation() != 0) {
		int step = event.GetWheelRotation() / event.GetWheelDelta();
		value+=step;
		if(value<(double)rfrom||value>(double)rto){return;}
		SetValue(getdouble(value));MarkDirty();
		return;}

	event.Skip();
}




BEGIN_EVENT_TABLE(NumCtrl, wxTextCtrl)
	EVT_MOUSE_EVENTS(NumCtrl::OnMouseEvent)
	//EVT_MENU(Time_Copy,NumCtrl::OnCopy)
	//EVT_MENU(Time_Paste,NumCtrl::OnPaste)
END_EVENT_TABLE()

