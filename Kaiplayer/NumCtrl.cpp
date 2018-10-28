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


#include "NumCtrl.h"
#include "wx/clipbrd.h"

wxDEFINE_EVENT(NUMBER_CHANGED, wxCommandEvent);

wxString getdouble(double num)
{
	wxString strnum = wxString::Format(_T("%f"), num);
	strnum.Replace(",", ".");
	int rmv = 0;
	for (int i = strnum.Len() - 1; i > 0; i--)
	{
		if (strnum[i] == '0'){ rmv++; }
		else if (strnum[i] == '.'){ rmv++; break; }
		else{ break; }
	}
	if (rmv){ strnum.RemoveLast(rmv); }
	return strnum;
}

NumCtrl::NumCtrl(wxWindow *parent, long id, wxString text, int rangefrom, int rangeto, bool intonly, const wxPoint &pos, const wxSize &size, long style)
	:KaiTextCtrl(parent, id, text, pos, size, style)
{

	rfrom = rangefrom;
	rto = rangeto;
	if (rto < rfrom){ rto = rangefrom; rfrom = rangeto; }
	oint = intonly;

	value = 0;
	oldposy = rfrom;
	holding = false;
	SetString(text);

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
	if (rfrom < 0){
		includes.Add(_T("-"));
	}
	if (!oint){
		includes.Add(_T("."));
		includes.Add(_T(","));
	}
	valid.SetIncludes(includes);
	SetValidator(valid);

	Connect(wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&NumCtrl::OnNumWrite);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &NumCtrl::OnPaste, this, ID_TCTLV);
	Bind(wxEVT_RIGHT_DOWN, &NumCtrl::OnMouseEvent, this);
	Bind(wxEVT_RIGHT_UP, &NumCtrl::OnMouseEvent, this);
	Bind(wxEVT_MOTION, &NumCtrl::OnMouseEvent, this);
	Bind(wxEVT_MOUSEWHEEL, &NumCtrl::OnMouseEvent, this);
	SetMaxLength(10);
}

NumCtrl::NumCtrl(wxWindow *parent, long id, double _value, double rangefrom, double rangeto, bool intonly, const wxPoint &pos, const wxSize &size, long style)
	:KaiTextCtrl(parent, id, "", pos, size, style)
	, value(0)
{

	rfrom = rangefrom;
	rto = rangeto;

	if (rto < rfrom){ rto = rangefrom; rfrom = rangeto; }
	oint = intonly;
	oldposy = rfrom;
	holding = false;
	SetDouble(_value);

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
	if (rfrom < 0){
		includes.Add(_T("-"));
	}
	if (!oint){
		includes.Add(_T("."));
		includes.Add(_T(","));
	}
	valid.SetIncludes(includes);
	SetValidator(valid);

	Connect(wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&NumCtrl::OnNumWrite);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &NumCtrl::OnPaste, this, ID_TCTLV);
	/*Bind(wxEVT_RIGHT_DOWN, &NumCtrl::OnMouseEvent, this);
	Bind(wxEVT_RIGHT_UP, &NumCtrl::OnMouseEvent, this);
	Bind(wxEVT_MOTION, &NumCtrl::OnMouseEvent, this);
	Bind(wxEVT_MOUSEWHEEL, &NumCtrl::OnMouseEvent, this);*/
	SetMaxLength(20);
}

NumCtrl::~NumCtrl()
{
}


void NumCtrl::SetString(wxString val)
{
	if (!val.ToDouble(&value)){ val.ToCDouble(&value); }
	if (oint){
		int finds = val.Find('.', true);
		if (finds != -1){ val = val.BeforeFirst('.'); }
		finds = val.Find(',', true);
		if (finds != -1){ val = val.BeforeFirst(','); }
	}
	else{
		val.Replace(",", ".");
	}
	if (value > rto)
	{
		value = rto;
		val = getdouble(value);
	}
	if (value < rfrom)
	{
		value = rfrom;
		val = getdouble(value);
	}
	if (val.IsEmpty()){ val = getdouble(value); }
	oldval = val;
	KaiTextCtrl::SetValue(val, false);
}

void NumCtrl::SetInt(int val)
{
	if (val > (int)rto){ val = (int)rto; }
	else if (val < (int)rfrom){ val = (int)rfrom; }
	value = (double)val;
	wxString kkk;
	oldval = kkk << val;
	KaiTextCtrl::SetValue(kkk, false);
}

void NumCtrl::SetDouble(double val)
{
	if (val > rto){ val = rto; }
	else if (val < rfrom){ val = rfrom; }
	value = val;
	oldval = getdouble(val);
	KaiTextCtrl::SetValue(oldval, false);
}
// function to block setvalue from kaitextctrl
void NumCtrl::SetValue(const wxString &text, bool modif/* =false */, bool newSel /* = true */)
{
	SetString(text);
}

wxString NumCtrl::GetString()
{
	wxString val = GetValue();
	val.Replace(",", ".");
	if (val == "-"){ val = oldval; }
	if (val.StartsWith('.')){ val.Prepend("0"); }
	if (val.EndsWith('.')){ val.Append("0"); }
	if (!val.ToCDouble(&value)){
		val = oldval;
	}
	if (value > rto){
		value = rto;
		val = getdouble(value);
	}
	if (value < rfrom){
		value = rfrom;
		val = getdouble(value);
	}
	return val;
}

int NumCtrl::GetInt()
{
	return (int)GetDouble();
}

double NumCtrl::GetDouble()
{
	wxString val = GetString();
	if (!val.ToCDouble(&value)){
		oldval.ToCDouble(&value);
	}
	if (value > rto)
		value = rto;
	if (value < rfrom)
		value = rfrom;
	return value;
}



void NumCtrl::OnNumWrite(wxCommandEvent& event)
{
	long from, to;
	GetSelection(&from, &to);
	wxString val = GetValue();
	val.Replace(",", ".");
	if (val == "-" || val == ""){}
	else if (val.EndsWith(".")){ if (val.Replace(".", "") > 1){ KaiTextCtrl::SetValue(oldval); wxBell(); } }
	else if (val.StartsWith(".")){ if (val.Replace(".", "") > 1){ KaiTextCtrl::SetValue(oldval); wxBell(); } }
	else if (!val.ToCDouble(&value) || value > rto || value < rfrom){
		/*SetValue(oldval);wxBell();*/
		//if(!isbad){SetForegroundColour(*wxRED);isbad=true;}
	}
	else{ oldval = val;/*if(isbad){SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));isbad=false;}*/ }
	if (IsModified()){ wxCommandEvent evt2(NUMBER_CHANGED, GetId()); AddPendingEvent(evt2); }
	event.Skip();
}

void NumCtrl::OnMouseEvent(wxMouseEvent &event)
{
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
		if (absy >= absx){
			if (absy >= 8){
				double nval = (oldposy < posy) ? value - 1 : value + 1;
				if (value < rfrom || value > rto){ return; }
				KaiTextCtrl::SetValue(getdouble(nval), true, false);
				oldposy = posy;
				//reset oldposx cause next time it will change it by 10 
				//even if it's still moved in one direction
				oldposx = posx;
				value = nval;
				changed = true;
			}
		}
		else{
			if (absx >= 10){
				double nval = (oldposx < posx) ? value - 10 : value + 10;
				if (value == rfrom || value == rto){ return; }
				else if (nval < rfrom){ nval = rfrom; }
				else if (nval > rto){ nval = rto; }
				KaiTextCtrl::SetValue(getdouble(nval), true, false);
				oldposx = posx; 
				//reset oldposx cause next time it will change it by 1
				//even if it's still moved in one direction
				oldposy = posy;
				value = nval;
				changed = true;
			}
		}
		if (changed){ wxCommandEvent evt2(NUMBER_CHANGED, GetId()); AddPendingEvent(evt2); }
	}

	if (rclick)
	{
		holding = true;
		oldposy = posy;
		oldposx = posx;
		wxPoint cpos;
		//HitTest(wxPoint(posx,posy),&cpos);
		//curpos=cpos.x;
		//SetSelection(cpos.x,cpos.x);
		SetFocus();
		CaptureMouse();
	}


	if (event.GetWheelRotation() != 0) {
		if (style & SCROLL_ON_FOCUS && !HasFocus()){ event.Skip(); return; }
		int step = event.GetWheelRotation() / event.GetWheelDelta();
		value += step;
		if (value<rfrom || value>rto){ return; }
		KaiTextCtrl::SetValue(getdouble(value), true, false);//MarkDirty();
		if (IsModified()){ wxCommandEvent evt2(NUMBER_CHANGED, GetId()); AddPendingEvent(evt2); }
		return;
	}

	event.Skip();
}

void NumCtrl::OnMouseLost(wxMouseCaptureLostEvent& event)
{
	if (HasCapture()){ ReleaseMouse(); }
	holding = false;
}

void NumCtrl::OnPaste(wxCommandEvent& event)
{
	if (wxTheClipboard->Open())
	{
		if (wxTheClipboard->IsSupported(wxDF_TEXT))
		{
			wxTextDataObject data;
			wxTheClipboard->GetData(data);
			wxString whatpaste = data.GetText();
			double test = 0;
			if ((oint && whatpaste.IsNumber()) || (!oint && whatpaste.ToCDouble(&test))){
				long from = 0, to = 0;
				GetSelection(&from, &to);
				Replace(from, to, whatpaste, false);
				MarkDirty();
				int newSel = from + whatpaste.Len();
				SetSelection(newSel, newSel);
				wxCommandEvent evt2(NUMBER_CHANGED, GetId()); AddPendingEvent(evt2);
			}
			else{
				wxBell();
			}

		}
		wxTheClipboard->Close();
		//wxTheClipboard->Flush();
	}
}


BEGIN_EVENT_TABLE(NumCtrl, KaiTextCtrl)
//EVT_MOUSE_EVENTS(NumCtrl::OnMouseEvent)
EVT_MOUSE_CAPTURE_LOST(NumCtrl::OnMouseLost)
END_EVENT_TABLE()

