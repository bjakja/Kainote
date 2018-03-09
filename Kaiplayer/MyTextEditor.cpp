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

#include "MyTextEditor.h"
#include "EditBox.h"
#include "Spellchecker.h"
#include "config.h"
#include "Menu.h"
#include <wx/regex.h>
#include <wx/clipbrd.h>
#include "KaiMessageBox.h"
#include "Stylelistbox.h"
#include "SubsFile.h"
//#include <wx/graphics.h>
#undef DrawText

wxDEFINE_EVENT(CURSOR_MOVED, wxCommandEvent);



MTextEditor::MTextEditor(wxWindow *parent, int id, bool _spell, const wxPoint& pos,const wxSize& size, long style)
	:wxWindow(parent,id,pos,size,style)
{
	spell=_spell;
	MText="";
	bmp=NULL;
	fsize=10;
	posY=0;
	scPos=0;
	//SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
	SetCursor(wxCURSOR_IBEAM);
	wxAcceleratorEntry entries[40];
	entries[0].Set(wxACCEL_NORMAL, WXK_DELETE,ID_DEL);
	entries[1].Set(wxACCEL_NORMAL, WXK_BACK,ID_BACK);
	entries[2].Set(wxACCEL_CTRL, WXK_BACK,ID_CBACK);
	entries[3].Set(wxACCEL_CTRL, WXK_DELETE,ID_CDELETE);
	entries[4].Set(wxACCEL_NORMAL, WXK_LEFT,ID_LEFT);
	entries[5].Set(wxACCEL_NORMAL, WXK_RIGHT,ID_RIGHT);
	entries[6].Set(wxACCEL_NORMAL, WXK_UP,ID_UP);
	entries[7].Set(wxACCEL_NORMAL, WXK_DOWN,ID_DOWN);
	entries[8].Set(wxACCEL_NORMAL, WXK_HOME,ID_HOME);
	entries[9].Set(wxACCEL_NORMAL, WXK_END,ID_END);
	entries[10].Set(wxACCEL_CTRL, WXK_LEFT,ID_CLEFT);
	entries[11].Set(wxACCEL_CTRL, WXK_RIGHT,ID_CRIGHT);
	entries[12].Set(wxACCEL_SHIFT, WXK_LEFT,ID_SLEFT);
	entries[13].Set(wxACCEL_SHIFT, WXK_RIGHT,ID_SRIGHT);
	entries[14].Set(wxACCEL_SHIFT, WXK_UP,ID_SUP);
	entries[15].Set(wxACCEL_SHIFT, WXK_DOWN,ID_SDOWN);
	entries[16].Set(wxACCEL_SHIFT|wxACCEL_CTRL, WXK_LEFT,ID_CSLEFT);
	entries[17].Set(wxACCEL_SHIFT|wxACCEL_CTRL, WXK_RIGHT,ID_CSRIGHT);
	entries[18].Set(wxACCEL_SHIFT, WXK_HOME,ID_SHOME);
	entries[19].Set(wxACCEL_SHIFT, WXK_END,ID_SEND);
	entries[20].Set(wxACCEL_CTRL, 'A',ID_CTLA);
	entries[21].Set(wxACCEL_CTRL, 'V',ID_CTLV);
	entries[22].Set(wxACCEL_CTRL, 'C',ID_CTLC);
	entries[23].Set(wxACCEL_CTRL, 'X',ID_CTLX);
	entries[24].Set(wxACCEL_NORMAL, WXK_WINDOWS_MENU, ID_WMENU);
	entries[25].Set(wxACCEL_NORMAL, WXK_PAGEDOWN,ID_PDOWN);
	entries[26].Set(wxACCEL_NORMAL, WXK_PAGEUP,ID_PUP);
	entries[27].Set(wxACCEL_SHIFT|wxACCEL_CTRL, WXK_END,ID_CSEND);
	int numEntries = 28;
	bool setNumpadAccels = !Options.GetBool(TextFieldAllowNumpadHotkeys);
	if(setNumpadAccels){
		entries[28].Set(wxACCEL_NORMAL, WXK_NUMPAD0,WXK_NUMPAD0+10000);
		entries[29].Set(wxACCEL_NORMAL, WXK_NUMPAD1,WXK_NUMPAD1+10000);
		entries[30].Set(wxACCEL_NORMAL, WXK_NUMPAD2,WXK_NUMPAD2+10000);
		entries[31].Set(wxACCEL_NORMAL, WXK_NUMPAD3,WXK_NUMPAD3+10000);
		entries[32].Set(wxACCEL_NORMAL, WXK_NUMPAD4,WXK_NUMPAD4+10000);
		entries[33].Set(wxACCEL_NORMAL, WXK_NUMPAD5,WXK_NUMPAD5+10000);
		entries[34].Set(wxACCEL_NORMAL, WXK_NUMPAD6,WXK_NUMPAD6+10000);
		entries[35].Set(wxACCEL_NORMAL, WXK_NUMPAD7,WXK_NUMPAD7+10000);
		entries[36].Set(wxACCEL_NORMAL, WXK_NUMPAD8,WXK_NUMPAD8+10000);
		entries[37].Set(wxACCEL_NORMAL, WXK_NUMPAD9,WXK_NUMPAD9+10000);
		numEntries = 38;
	}
	wxAcceleratorTable accel(numEntries, entries);
	SetAcceleratorTable(accel);
	Connect(ID_DEL,ID_PUP,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&MTextEditor::OnAccelerator);
	if(setNumpadAccels){
		Bind(wxEVT_COMMAND_MENU_SELECTED,[=](wxCommandEvent &evt){
			int key = evt.GetId() - 10276;
			wxKeyEvent kevt;
			kevt.m_uniChar = key;
			OnCharPress(kevt);
		}, WXK_NUMPAD0+10000, WXK_NUMPAD9+10000);
	}
	Cursor.x=Cursor.y=Selend.x=Selend.y = oldstart = oldend=0;

	holding = dholding = firstdhold = modified = wasDoubleClick = false;

	font= wxFont(10,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,"Tahoma",wxFONTENCODING_DEFAULT);
	int fw,fh;
	GetTextExtent("#TWFfGH", &fw, &fh, NULL, NULL, &font);
	Fheight=fh;
	scroll=new KaiScrollbar(this,3333,wxDefaultPosition,wxDefaultSize, wxSB_VERTICAL);
	scroll->SetCursor(wxCURSOR_DEFAULT);
	caret= new wxCaret(this, 1, Fheight);
	SetCaret(caret);
	caret->Move(3,2);
	caret->Show();
	//SetMinSize(wxSize(100,100));
	//Refresh(false);
}

MTextEditor::~MTextEditor()
{
	if(bmp)delete bmp;
}

void MTextEditor::SetTextS(const wxString &text, bool modif, bool resetsel,bool noevent)
{
	modified=modif;
	MText=text;
	CalcWrap(modif,(noevent)? false : modif);
	if(spell){CheckText();}
	if(resetsel){SetSelection(0,0);}
	else{
		if((size_t)Cursor.x>MText.Len()){Cursor.x = MText.Len();Cursor.y = FindY(Cursor.x);}
		Refresh(false);
	}
	//else{Refresh(false);}
}

void MTextEditor::CalcWrap(bool updatechars, bool sendevent)
{
	//Wrapped=MText;
	wraps.clear();
	wraps.Add(0);
	if(MText!=""){
		int w,h,fw,fh;
		GetClientSize(&w,&h);
		//if (w < 30){ return; }
		if(scroll->IsShown()){
			int sw, sh;
			scroll->GetSize(&sw,&sh);
			w-=sw;
		}
		size_t i = 0;
		size_t textLen = MText.Len();
		if (w < 20){
			while (i < textLen){ i++; wraps.Add(i); }
		}
		else{
			int podz = 0;
			wxString wrapchars = " \\,;:}{()";

			int nwrap = -1;
			int allwrap = 0;
			int approxSize = w / fsize;
			while (i < textLen)
			{
				i = podz + approxSize;
				if (i >= textLen){ i = textLen - 1; }
				bool forward = false;
				bool forwardNotFound = true;
				bool backward = false;
				bool backwardNotFound = true;
				while (1)
				{
					wxString wrap = MText.SubString(podz, i);
					GetTextExtent(wrap, &fw, &fh, NULL, NULL, &font);
					allwrap = i;
					if (fw < w - 7 && !backward && backwardNotFound && i < textLen - 1){
						int j = i + 1;
						while (j < textLen && forwardNotFound){
							if (wrapchars.Find(MText[j]) != -1){
								nwrap = j;
								if (MText[j] == ' '){ nwrap++; }
								forward = true;
								break;
							}
							j++;
						}
						if (!forward){ i++; forwardNotFound = false; }
						else{ i = j; }
					}
					else if (fw > w - 7 && i > podz){
						size_t k = i - 1;
						while (k > podz && backwardNotFound){
							if (wrapchars.Find(MText[k]) != -1){
								nwrap = k;
								if (MText[k] == ' '){ nwrap++; }
								backward = true;
								break;
							}
							k--;
						}
						if (!backward){ if (i > podz) i--; backwardNotFound = false; }
						else{ i = k; }
					}
					else{
						int wwrap = (nwrap > 0 && nwrap > wraps[wraps.size() - 1] && nwrap <= allwrap && i < textLen-1) ? nwrap : (i >= textLen-1) ? textLen : allwrap + 1;
						wraps.Add(wwrap); podz = wwrap; nwrap = -1; allwrap = i;
						i++;
						break;
					}
				}
			}
		}
	}
	else{
		wraps.Add(MText.Len());
	}
	//
	if(updatechars){EB->UpdateChars(MText);}
	if(sendevent){wxCommandEvent evt2(wxEVT_COMMAND_TEXT_UPDATED, GetId()); AddPendingEvent(evt2);}
}

void MTextEditor::OnCharPress(wxKeyEvent& event)
{
	wxUniChar wkey=event.GetUnicodeKey();
	if(wkey=='\t'){return;}
	if(wkey){
		if(Cursor!=Selend){
			int curx=Cursor.x;
			int selx=Selend.x;
			if(curx>selx){int tmp=curx;curx=selx;selx=tmp;}
			MText.Remove(curx,selx-curx);
			if(Cursor.x<Selend.x){Selend=Cursor;}else{Cursor=Selend;}
		}
		int len=MText.Len();
		if(Cursor.x>=len){MText<<wkey;}
		else{MText.insert(Cursor.x,1,wkey);}
		CalcWrap();
		if(Cursor.x+1>wraps[Cursor.y+1]){Cursor.y++;}
		Cursor.x++;
		Selend=Cursor;
		if(spell){CheckText();/*if(wkey==' '){CheckText();return;} else {spblock.Start(1000);}*/}
		Refresh(false);
		modified=true;
	}

}

void MTextEditor::OnKeyPress(wxKeyEvent& event)
{
	int key=event.GetKeyCode();
	if(!(event.ControlDown() && !event.AltDown()) && (key>30 || key == 0)){event.Skip();return;}
	if(event.ControlDown() && key == '0'){
		font.SetPointSize(10);
		int fw,fh;
		GetTextExtent("#TWFfGH", &fw, &fh, NULL, NULL, &font);
		Fheight=fh;
		caret->SetSize(1,fh);
		CalcWrap(false, false);
		Refresh(false);
	}
}

void MTextEditor::OnAccelerator(wxCommandEvent& event)
{
	int step=0;
	int len;
	int ID=event.GetId();
	switch(ID){
	case ID_CDELETE:
	case ID_CBACK:
		//len = MText.Len();
		if((ID==ID_CBACK && Cursor.x==0) || (ID==ID_CDELETE && Cursor.x>=(int)MText.Len())){return;}
		Selend.x=Cursor.x;
		if(ID==ID_CBACK){
			FindWord((Cursor.x<2)? 0 : Cursor.x-1, &Cursor.x, &len);
			if(Cursor.x==1 && MText[0]==' '){Cursor.x--;}
		}else{
			FindWord(Cursor.x, &len, &Selend.x);
		}
	case ID_DEL:
	case ID_BACK:
		if(Cursor!=Selend){
			int curx=Cursor.x;
			int selx=Selend.x;
			if(curx>selx){int tmp=curx;curx=selx;selx=tmp;}
			MText.Remove(curx,selx-curx);
			Selend=Cursor;
			CalcWrap();
			if(spell){CheckText();}
			SetSelection(curx,curx);
		}else
		{
			if(ID==ID_BACK){if(Cursor.x<1){return;} 
			Cursor.x--;
			}
			if(ID==ID_DEL && Cursor.x >= (int)MText.Len()){return;}
			MText.Remove(Cursor.x,1);
		}
		len=wraps.size();
		CalcWrap();
		
		if(Cursor.x<wraps[Cursor.y] || (Cursor.x==wraps[Cursor.y] && len != wraps.size())){Cursor.y--;}
		else if(Cursor.x>wraps[Cursor.y+1]){Cursor.y++;}
		Selend=Cursor;
		if(spell){CheckText();}
		Refresh(false);
		modified=true;
		break;

	case ID_LEFT:
	case ID_CLEFT:
	case ID_SLEFT:
	case ID_CSLEFT:
		if(ID==ID_LEFT && Selend.x<Cursor.x){Cursor=Selend;Refresh(false);return;}
		if(Cursor.x<1){return;}
		if(ID==ID_CLEFT||ID==ID_CSLEFT){
			FindWord(Cursor.x-1,&Cursor.x,0);
		}
		if(Cursor.x-1<wraps[Cursor.y] && Cursor.x!=0){Cursor.y--;}
		else if(ID!=ID_CLEFT&&ID!=ID_CSLEFT){Cursor.x--;}


		if(ID<ID_SLEFT){Selend=Cursor;}
		Refresh(false);
		break;

	case ID_RIGHT:
	case ID_CRIGHT:
	case ID_SRIGHT:
	case ID_CSRIGHT:
		if(ID==ID_RIGHT && Selend.x>Cursor.x){Cursor=Selend;Refresh(false);return;}
		if(Cursor.x>=(int)MText.Len()){return;}
		if(ID==ID_CRIGHT||ID==ID_CSRIGHT){
			if(Cursor.x==MText.Len()-1){
				Cursor.x++;
			}else{
				FindWord(Cursor.x+1,0,&Cursor.x);}
		}
		if(Cursor.x+1>wraps[Cursor.y+1] && Cursor.y<(int)wraps.size()-2){Cursor.y++;}
		else if(ID!=ID_CRIGHT&&ID!=ID_CSRIGHT){Cursor.x++;}

		if(ID<ID_SRIGHT){Selend=Cursor;}
		Refresh(false);
		break;

	case ID_DOWN:
	case ID_SDOWN:
		len=MText.Len();
		if(Cursor.y>=(int)wraps.size()-2){Cursor.y=wraps.size()-2; Cursor.x=len;}
		else{
			Cursor.x-=wraps[Cursor.y];
			Cursor.y++;
			Cursor.x+=wraps[Cursor.y];
			if(Cursor.x>len){Cursor.x=len;}
		}
		//wxLogStatus("%i %i",Cursor.x, Cursor.y);


		if(ID<ID_SDOWN){Selend=Cursor;}
		Refresh(false);
		break;

	case ID_UP:
	case ID_SUP:

		//if(Cursor.y<1){return;}
		Cursor.x-=wraps[Cursor.y];
		Cursor.y--;
		if(Cursor.y<1){Cursor.y=0;Cursor.x=0;}
		else{Cursor.x+=wraps[Cursor.y];}

		//wxLogStatus("%i %i",Cursor.x, Cursor.y);
		if(ID<ID_SUP){Selend=Cursor;}
		Refresh(false);

		break;
	case ID_HOME:
	case ID_END:
	case ID_SHOME:
	case ID_SEND:
	case ID_CSEND:
		Cursor.x=wraps[(ID==ID_END||ID==ID_SEND) ? Cursor.y+1 : Cursor.y];
		if(ID==ID_CSEND){Cursor.x=MText.Len(); Cursor.y=wraps.size()-2;}
		if(ID<ID_SHOME){Selend=Cursor;}
		Refresh(false);
		break;
	case ID_CTLA:

		Cursor.x=Cursor.y=0;
		Selend.x=MText.Len(); Selend.y=wraps.size()-2;
		Refresh(false);
		break;
	case ID_CTLV:
		Paste();
		break;
	case ID_CTLC:
	case ID_CTLX:

		Copy(ID>ID_CTLC);

		break;
	case ID_WMENU:
		//Selend=Cursor;
		ContextMenu(PosFromCursor(Cursor),FindError(Cursor,false));
		break;

	case ID_PDOWN:
	case ID_PUP:

		break;

	default:

		break;
	}
	if(ID!=ID_CTLV||ID!=ID_WMENU){wxCommandEvent evt(CURSOR_MOVED,GetId());AddPendingEvent(evt);}
}

void MTextEditor::OnMouseEvent(wxMouseEvent& event)
{
	bool click=event.LeftDown();
	bool leftup=event.LeftUp();
	if(event.ButtonDown()){SetFocus();if(!click){Refresh(false);}}

	//if(wasDoubleClick && event.Moving()){wasDoubleClick=false;}

	if(leftup && (holding||dholding)){
		holding=dholding=false;
		if(HasCapture()){ReleaseMouse();}
		return;
	}



	if(event.LeftDClick() && MText!=""){
		wasDoubleClick=true;
		time = timeGetTime();
		wxPoint mpos=event.GetPosition();
		int errn=FindError(mpos);
		if(Options.GetBool(EditboxSugestionsOnDoubleClick) && errn>=0){
			wxString err=errs[errn];

			wxArrayString suggs= SpellChecker::Get()->Suggestions(err);

			KaiListBox lw(this,suggs,_("Sugestie poprawy"));
			if(lw.ShowModal()==wxID_OK)
			{
				int from=errors[errn*2];
				int to=errors[(errn*2)+1];
				wxString txt=lw.GetSelection();
				MText.replace(from,to-from,txt);
				modified=true;
				CalcWrap();
				if(spell){CheckText();}
				int newto=from+txt.Len();
				SetSelection(newto,newto);
				EB->Send(EDITBOX_SPELL_CHECKER, false);
				modified=false;
			}
			return;
		}
		int start,end;
		wxPoint ht;
		if(Cursor.x<Selend.x){Selend=Cursor;}else{Cursor=Selend;}
		HitTest(mpos,&ht);
		FindWord(ht.x,&start,&end);
		oldend=tmpend=end;
		oldstart=tmpstart=start;
		SetSelection(start,end);
		firstdhold = dholding = true;
		CaptureMouse();
		return;
	}

	if(click){
		wxPoint cur;
		HitTest(event.GetPosition(),&cur);
		if(cur!=Cursor){wxCommandEvent evt(CURSOR_MOVED,GetId());AddPendingEvent(evt);}
		Cursor=cur;
		if(!event.ShiftDown()){Selend=Cursor;}
		if(wasDoubleClick){
			wasDoubleClick=false;
			
			if(timeGetTime()-time < 800 && dclickCurPos == event.GetPosition()){
				Cursor.x=0;
				Cursor.y=0;
				Selend.x=MText.Len();
				Selend.y=FindY(Selend.x);
				MakeCursorVisible();
				//Refresh(false);
				return;
			}
		}
		Refresh(false);
		holding=true;
		CaptureMouse();
	}

	if(holding){
		wxPoint cur;
		dclickCurPos = event.GetPosition();
		HitTest(dclickCurPos, &cur);
		Cursor=cur;
		//Refresh(false);
		MakeCursorVisible();
	}
	if(dholding){
		wxPoint cur;
		int start, end;
		HitTest(event.GetPosition(),&cur);
		FindWord(cur.x,&start,&end);
		if((start==tmpstart && end==tmpend)){return;}
		tmpstart=start;tmpend=end;

		if(start<oldstart){
			if(end==oldstart){Selend.x=oldend;Selend.y=FindY(oldend);}
			Cursor.x=start;
			Cursor.y=FindY(start);
		}else{
			if(oldstart==start){Selend.x=oldstart;Selend.y=FindY(oldstart);}
			Cursor.x=end;
			Cursor.y=FindY(end);
		}
		//Refresh(false);
		MakeCursorVisible();
	}

	if(event.RightUp())
	{
		wxPoint pos=event.GetPosition();
		ContextMenu(pos,FindError(pos));
	}

	if (event.GetWheelRotation() != 0 && event.ControlDown()) {
		fsize += event.GetWheelRotation() / event.GetWheelDelta();
		if(fsize<7||fsize>70){fsize = MID(7,fsize,70); return;}
		font.SetPointSize(fsize);
		int fw,fh;
		GetTextExtent("#TWFfGH", &fw, &fh, NULL, NULL, &font);
		Fheight=fh;
		caret->SetSize(1,fh);
		CalcWrap(false, false);
		Refresh(false);
	}else if(event.GetWheelRotation() != 0){
		int step = 30 * event.GetWheelRotation() / event.GetWheelDelta();
		if(step>0 && scPos==0){return;}
		scPos = MAX(scPos - step, 0);
		Refresh(false);
	}
}

void MTextEditor::OnSize(wxSizeEvent& event)
{
	CalcWrap(false,false);
	Cursor.y = FindY(Cursor.x);
	Selend.y = FindY(Selend.x);
	MakeCursorVisible();
}

int MTextEditor::FindY(int x)
{
	for(size_t p=1; p<wraps.size(); p++){ if(x<wraps[p]){return (p-1);} }
	return wraps.size()-2;
}

void MTextEditor::OnPaint(wxPaintEvent& event)
{
	//if(block){return;} 
	int w = 0, h = 0;
	GetClientSize(&w,&h);
	if(w < 1||h==0){return;}
	//block=true;
	wxPaintDC dc(this);
	int bitmaph= (wraps.size()*Fheight)+4;
	if(bitmaph>h){
		if(!scroll->IsShown()){
			scroll->Show();
			CalcWrap(false, false);
			Cursor.y = FindY(Cursor.x);
			Selend.y = FindY(Selend.x);
			bitmaph= (wraps.size()*Fheight)+4;
		}
		int sw=0,sh=0;
		scroll->GetSize(&sw,&sh);
		scroll->SetSize(w-sw,0,sw,h);
		int diff= h;
		int diff2= bitmaph;
		if(scPos>diff2-diff){scPos=diff2-diff;}
		scroll->SetScrollbar(scPos,diff,diff2,diff-2);
		w -= sw;
		if (w < 0){ return; }
	}else{
		if(scroll->IsShown()){
			scroll->Hide();
			CalcWrap(false,false);
			Cursor.y = FindY(Cursor.x);
			Selend.y = FindY(Selend.x);
		}
		bitmaph=h;
		scPos=0;
	}
	
	/*bool direct = false;

	if (direct) {
		DrawFld(dc,w,h,h);
	}*/

	/*else {*/
		// Prepare bitmap
		if (bmp) {
			if (bmp->GetWidth() < w || bmp->GetHeight() < h) {
				delete bmp;
				bmp = NULL;
			}
		}

		if (!bmp) bmp = new wxBitmap(w,h);

		// Draw bitmap
		wxMemoryDC bmpDC;

		bmpDC.SelectObject(*bmp);

		DrawFld(bmpDC,w,h,h);

		dc.Blit(0,0,w,h,&bmpDC,0,0);

	//}

	//block=false;
}

void MTextEditor::DrawFld(wxDC &dc,int w, int h, int windowh)
{
	int fw=0,fh=0;

	const wxColour & ctext = Options.GetColour(EditorText);
	const wxColour & ccurlybraces = Options.GetColour(EditorCurlyBraces);
	const wxColour & coperators = Options.GetColour(EditorTagOperators);
	const wxColour & cnames = Options.GetColour(EditorTagNames);
	const wxColour & cvalues = Options.GetColour(EditorTagValues);
	const wxColour & bgbraces = Options.GetColour(EditorBracesBackground);
	const wxColour & cbackground = Options.GetColour(EditorBackground);
	const wxColour & cselection = Options.GetColour(EditorSelection);
	const wxColour & cselnofocus = Options.GetColour(EditorSelectionNoFocus);
	const wxColour & cspellerrors = Options.GetColour(EditorSpellchecker);

	dc.SetBrush(cbackground);
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.DrawRectangle(0,0,w,h);

	bool tagi=false;
	bool slash=false;
	bool val=false;
	wxString znaki="(0123456789-&+";
	wxString cyfry="-0123456789ABCDEFabcdef.";
	wxString tagtest="";
	wxString parttext="";
	wxString mestext="";

	//Contsel=false;
	posY=2;
	posY -= scPos;
	bool isfirst=true;
	int wline=0;
	int wchar=0;

	dc.SetFont(font);
	wxString alltext=MText+" ";
	int len=alltext.Len();
	const wxUniChar &bchar=alltext[Cursor.x];
	if(bchar=='{')
	{
		Brackets.x=Cursor.x;
		Brackets.y=FindBracket('{','}',Cursor.x+1);
	}else if(bchar=='}')
	{
		Brackets.y=Cursor.x;
		Brackets.x=FindBracket('}','{',Cursor.x-1,true);
	}else if(bchar=='(')
	{
		Brackets.x=Cursor.x;
		Brackets.y=FindBracket('(',')',Cursor.x+1);
	}else if(bchar==')')
	{
		Brackets.y=Cursor.x;
		Brackets.x=FindBracket(')','(',Cursor.x-1,true);
	}else{Brackets.x=-1;Brackets.y=-1;}

	int fww, fwww;
	dc.SetPen(*wxTRANSPARENT_PEN);
	//rysowanie spellcheckera
	if(spell){
		int spelllen=errors.size();

		dc.SetBrush(cspellerrors);
		for(int g=0;g<spelllen; g+=2)
		{
			int fsty=FindY(errors[g]);
			if(wraps[fsty]>=errors[g]){fw=0;}
			else{
				wxString ftext = MText.SubString(wraps[fsty], errors[g] - 1);
				GetTextExtent(ftext, &fw, &fh, NULL, NULL, &font);
			}
			int scndy=FindY(errors[g+1]);
			wxString etext=MText.SubString(errors[g], (fsty==scndy)?errors[g+1] : wraps[fsty+1]);
			GetTextExtent(etext, &fww, &fh, NULL, NULL, &font);
			for(int q=fsty+1; q<=scndy; q++){
				int rest= (q==scndy)? errors[g+1] : wraps[q+1];
				wxString btext=MText.SubString(wraps[q], rest);
				GetTextExtent(btext, &fwww, &fh, NULL, NULL, &font);
				dc.DrawRectangle(3, ((q*Fheight) + 1) - scPos, fwww, Fheight);
			}
			dc.DrawRectangle(fw + 3, ((fsty*Fheight) + 1) - scPos, fww, Fheight);
		}
	}

	if(Cursor.x!=Selend.x || Cursor.y != Selend.y){
		Brackets.x=-1;Brackets.y=-1;
		wxPoint fst, scd;
		if((Cursor.x + Cursor.y) > (Selend.x + Selend.y)){fst=Selend;scd=Cursor;}
		else{fst=Cursor, scd=Selend;}

		dc.SetBrush(wxBrush(wxColour((HasFocus())? cselection: cselnofocus)));
		fww=0;
		//rysowanie zaznaczenia
		for(int j=fst.y; j<=scd.y; j++){

			if(j==fst.y){
				wxString ftext=MText.SubString(wraps[j],fst.x-1);
				if(wraps[j]>fst.x-1){fw=0;}
				else{GetTextExtent(ftext, &fw, &fh, NULL, NULL, &font);}
				wxString stext=MText.SubString(fst.x,(fst.y==scd.y)? scd.x-1 : wraps[j+1]-1);
				GetTextExtent(stext, &fww, &fh, NULL, NULL, &font);
			}
			else if(j==scd.y){
				fw=0;
				GetTextExtent(MText.SubString(wraps[j],scd.x-1), &fww, &fh, NULL, NULL, &font);
			}
			else{
				fw=0;
				GetTextExtent(MText.SubString(wraps[j],wraps[j+1]-1), &fww, &fh, NULL, NULL, &font);
			}
			dc.DrawRectangle(fw+3,((j*Fheight)+1)-scPos,fww,Fheight);
			//if(j==scd.y)break;
		}
	}
	bool hasFocus = HasFocus();
	bool cursorWasSet = false;
	//rysowanie liter
	for (int i = 0; i < len; i++){
		if (posY > h)
			break;
			
		const wxUniChar &ch=alltext[i];

		if(i==wraps[wline+1]){
			if(Cursor.x+Cursor.y==wchar){
				int fww=0;
				GetTextExtent(mestext+parttext, &fww, &fh, NULL, NULL, &font);
				caret->Move(fww+3,posY/*-(scPos)*/);
				cursorWasSet = true;
			}

			if(parttext!=""){
				GetTextExtent(mestext, &fw, &fh, NULL, NULL, &font);
				wxColour kol=(val)? cvalues : (slash)? cnames : ctext;
				dc.SetTextForeground(kol);
				mestext<<parttext;
				dc.DrawText(parttext,fw+3,posY);
			}

			//posX=4;
			posY+=Fheight;
			wline++;
			wchar++;
			parttext="";
			mestext="";
		}
		/*if (posY + Fheight<0){
			if (ch == '{')
				tagi = true;
			else if (ch == '}')
				tagi = false;

			wchar++;
			continue;
		}*/

		if (hasFocus && (Cursor.x + Cursor.y == wchar)){
			if(mestext+parttext==""){fw=0;}
			else{GetTextExtent(mestext+parttext, &fw, &fh, NULL, NULL, &font);}
			caret->Move(fw+3,posY/*-(scPos)*/);
			cursorWasSet = true;
		}
		parttext<<ch;

		if(ch=='{'||ch=='}'){
			if(ch=='{'){
				tagi=true;
				wxString bef=parttext.BeforeLast('{');
				GetTextExtent(mestext, &fw, &fh, NULL, NULL, &font);
				dc.SetTextForeground(ctext);
				dc.DrawText(bef,fw+3,posY);
				mestext<<bef;
				parttext="{";
			}else{
				wxString &tmp=parttext.RemoveLast(1);
				GetTextExtent(mestext, &fw, &fh, NULL, NULL, &font);
				dc.SetTextForeground((val)? cvalues : ctext);
				dc.DrawText(tmp,fw+3,posY);
				mestext<<tmp;
				parttext="}";
				tagi=slash=val=false;
			}
			GetTextExtent(mestext, &fw, &fh, NULL, NULL, &font);
			dc.SetTextForeground(ccurlybraces);
			dc.DrawText(parttext,fw+3,posY);
			mestext<<parttext;
			parttext="";
			val=false;
			isfirst=false;
		}

		if(slash){
			tagtest +=ch;
			if((znaki.Find(ch)!=-1&&tagtest!="1"&&tagtest!="2"&&tagtest!="3"&&tagtest!="4")||tagtest=="fn"||ch=='('){
				slash=false;
				wxString tmp=(tagtest=="fn")? parttext : parttext.RemoveLast(1);
				GetTextExtent(mestext, &fw, &fh, NULL, NULL, &font);
				dc.SetTextForeground(cnames);
				dc.DrawText(tmp,fw+3,posY);
				//posX+=fw;
				mestext<<tmp;
				if(tagtest=="fn"){parttext="";}
				else{parttext=ch;}
				val=true;
				tagtest="";
			}
		}

		if((ch=='\\' || ((ch=='('||ch==')'||ch==',') && val)) &&tagi){
			if(ch=='\\'){slash=true;}
			if(val&&(ch=='\\'||ch==')'||ch==',')){
				wxString tmp=parttext.RemoveLast(1);
				GetTextExtent(mestext, &fw, &fh, NULL, NULL, &font);
				dc.SetTextForeground(cvalues);
				dc.DrawText(tmp,fw+3,posY);
				mestext<<tmp;
				parttext=ch;
				if(ch!=','){val=false;}

			}
			if(ch=='('){val=true;slash=false;}
			GetTextExtent(mestext, &fw, &fh, NULL, NULL, &font);
			dc.SetTextForeground(coperators);
			dc.DrawText(parttext,fw+3,posY);
			mestext<<parttext;
			parttext="";
			//continue;
		}
		if(hasFocus&&(i==Brackets.x||i==Brackets.y)){
			int bry=FindY(i);
			wxColour col=bgbraces;
			if(Brackets.x==-1||Brackets.y==-1){col=cspellerrors;}
			dc.SetBrush(wxBrush(col));
			//dc.SetPen(wxPen(col));
			if(i>0){GetTextExtent(MText.SubString(wraps[bry],i-1), &fw, &fh, NULL, NULL, &font);}else{fw=0;}
			GetTextExtent(MText[i], &fww, &fh, NULL, NULL, &font);
			dc.DrawRectangle(fw+3,((bry*Fheight)+2)-scPos,fww,Fheight);
			wxFont fnt=dc.GetFont();
			fnt=fnt.Bold();
			dc.SetFont(fnt);
			dc.DrawText(MText[i], fw + 3, ((bry*Fheight) + 2) - scPos);
			dc.SetFont(font);
			
		}

		wchar++;
	}
	if (!cursorWasSet){
		caret->Move(0, -50);
	}
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dc.SetPen(wxPen((HasFocus())? Options.GetColour(EditorBorderOnFocus) : Options.GetColour(EditorBorder)));
	dc.DrawRectangle(0,0,w,windowh);
}

bool MTextEditor::HitTest(wxPoint pos, wxPoint *cur)
{
	int w,h,fw=0,fh=0;
	GetClientSize(&w,&h);
	pos.y+=(scPos);
	pos.x-=2;

	cur->y=(pos.y/Fheight);
	if(cur->y<0||wraps.size()<2){cur->y=0;cur->x=0;return false;}
	if(cur->y>=(int)wraps.size()-1)
	{cur->y=wraps.size()-2;cur->x=wraps[wraps.size()-2];}
	else{cur->x=wraps[cur->y];}

	bool find=false;
	wxString txt=MText+" ";

	int wlen=MText.Len();
	int fww;
	for(int i = cur->x;i<wraps[cur->y+1]+1;i++)
	{

		GetTextExtent(txt.SubString(cur->x,i), &fw, &fh, NULL, NULL, &font);
		GetTextExtent(txt[i], &fww, &fh, NULL, NULL, &font);
		if(fw+1-(fww/2)>pos.x){cur->x=i; find=true;break;}
	}
	if(!find){cur->x = wraps[cur->y+1];}
	
	return find;
}

bool MTextEditor::Modified()
{
	return modified;
}
void MTextEditor::GetSelection(long *start, long *end)
{
	bool iscur= ((Cursor.x + Cursor.y) > (Selend.x + Selend.y));
	*start=(!iscur)? Cursor.x : Selend.x;
	*end=(iscur)? Cursor.x : Selend.x;
}

void MTextEditor::SetSelection(int start, int end, bool noEvent)
{
	int len = MText.Len();
	end=MID(0, end, len);
	start=MID(0, start, len);
	if((Cursor.x!=end || Selend.x!=start) && !noEvent){wxCommandEvent evt(CURSOR_MOVED,GetId());AddPendingEvent(evt);}	
	Cursor.x=end;
	Selend.x=start;
	Selend.y=FindY(Selend.x);
	Cursor.y=FindY(Cursor.x);

	Refresh(false);
}

wxString MTextEditor::GetValue() const
{
	return MText;
}

void MTextEditor::Replace(int start, int end, wxString rep)
{
	modified=true;
	MText.replace(start, end-start,rep);
	CalcWrap();
	Cursor.x=0;Cursor.y=0;
	Selend=Cursor;
	Refresh(false);
}

void MTextEditor::CheckText()
{
	if(MText==""){return;}
	//wxString notchar="/?<>|\\!@#$%^&*()_+=[]\t~ :;.,\"{} ";
	wxString text=MText;
	errors.clear();
	errs.Clear();
	text+=" ";
	bool block=false;
	wxString word="";
	//bool slash=false;
	int lasti=0;
	int firsti=0;
	int lastStartBracket = -1;
	int lastEndBracket = -1;
	int lastStartCBracket = -1;
	int lastStartTBracket = -1;
	int lastEndCBracket = -1;
	for(size_t i = 0; i<text.Len();i++)
	{
		wxUniChar ch=text.GetChar(i);
		if (iswctype(WXWCHAR_T_CAST(ch), _SPACE | _DIGIT | _PUNCT)/*notchar.Find(ch)!=-1*/ && !block){
			if(word.Len()>1){
				if(word.StartsWith("'")){word=word.Remove(0,1);}
				if(word.EndsWith("'")){word=word.RemoveLast(1);}
				word.Trim(false);word.Trim(true);
				bool isgood=SpellChecker::Get()->CheckWord(word);
				if (!isgood){errs.Add(word);errors.push_back(firsti);errors.push_back(lasti);}
			}
			word="";firsti=i+1;
		}
		if (block){
			if (ch == '{'){ errors.push_back(lastStartCBracket); errors.push_back(lastStartCBracket); errs.Add(""); }
			if (ch == '\\' && text[(i == 0) ? 0 : i - 1] == '\\'){ errors.push_back(i); errors.push_back(i); errs.Add(""); }
			if (ch == '('){
				if (i > 1 && text[i - 2] == '\\' && text[i - 1]){ lastStartTBracket = i; continue; }
				if (lastStartBracket > lastEndBracket){
					errors.push_back(lastStartBracket); errors.push_back(lastStartBracket); errs.Add("");
				}
				lastStartBracket = i;
			}
			if (ch == ')'){
				if (lastStartBracket < lastEndBracket || lastStartBracket < 0){
					if (lastStartTBracket > 0 && (lastStartTBracket < lastEndBracket || lastStartBracket < lastStartTBracket)){
						lastStartTBracket = -1; continue;
					}
					errors.push_back(i); errors.push_back(i); errs.Add("");
				}
				lastEndBracket = i;
			}
		}
		if (!block && ch == '}'){
			errors.push_back(i); errors.push_back(i); errs.Add("");
		}
		if (lastStartTBracket >= 0 && ch == '{' || ch == '}'){
			errors.push_back(lastStartTBracket); errors.push_back(lastStartTBracket); errs.Add("");
			lastStartTBracket = -1;
		}
		if (ch == '{'){ block = true; lastStartCBracket = i; continue; }
		else if (ch == '}'){ block = false; lastEndCBracket = i; firsti = i + 1; word = ""; continue; }

		if (!block && /*notchar.Find(ch)==-1*/ !iswctype(WXWCHAR_T_CAST(ch), _SPACE | _DIGIT | _PUNCT) &&
			text.GetChar((i == 0) ? 0 : i - 1) != '\\'){ word << ch; lasti = i; }
		else if(!block && text.GetChar((i==0)? 0 : i-1)=='\\'){
			word="";
			if(ch == 'N' || ch == 'n' || ch =='h'){
				firsti=i + 1;
			}else{
				firsti=i;
				word<<ch;
			}
		}
	}
	
	if (lastStartCBracket > lastEndCBracket){ errors.push_back(lastStartCBracket); errors.push_back(lastStartCBracket); errs.Add(""); }
	if (lastStartBracket > lastEndBracket){ errors.push_back(lastStartBracket); errors.push_back(lastStartBracket); errs.Add(""); }
	if (lastStartTBracket >= 0){ errors.push_back(lastStartTBracket); errors.push_back(lastStartTBracket); errs.Add(""); }
}

void MTextEditor::OnKillFocus(wxFocusEvent& event)
{
	Refresh(false);
}

void MTextEditor::FindWord(int pos, int *start, int *end)
{
	wxString wfind=" ]})({[-—'`\"\\;:,.><?!*~@#$%^&/+=";
	int len=MText.Len();
	if(len<1){Cursor.x = Cursor.y = 0; *start=0; *end=0; return;}
	bool fromend=(start!=NULL);

	if(!fromend){pos--;}
	pos=MID(0,pos,len-1);
	bool hasres=false;
	int lastres=-1;
	if(fromend){
		*start=(fromend)? 0 : len;
		for(int i=pos; i>=0; i--){
			int res=wfind.Find(MText[i]);
			if (lastres == 0 && res != 0 && i + 2 <= pos && MText[i + 1] == ' '){
				*start = i + 2;
				break;
			}
			if (res != -1){ lastres = res; }
			if(res!=-1&&!hasres){
				if(i==pos){hasres=true;continue;}
				bool isen=(MText[i]=='\\' && MText[i+1]=='N');
				*start=(isen && pos==i+1)? i : (isen)? i+2 : i+1;
				break;
			}else if(hasres && res==-1){
				if (lastres < 1 && (i + 2 == pos || i + 1 == pos)){ hasres = false; continue; }
				*start = ((lastres>3 && lastres<6 && i + 2 <= pos) || i + 1 == pos || MText[i + 2] == ' ') ? i + 1 : i + 2;
				break;
			}
		}
	}
	if(!end){return;}
	*end=(fromend && end==NULL)? 0 : len;
	for(int i=pos; i<len; i++){
		int res=wfind.Find(MText[i]);
		if (res == 0 /*&& i > pos*/){
			*end = i + 1;
			break;
		}
		if(res!=-1&&!hasres){
			if(i==pos){hasres=true;continue;}
			*end=(res<1)? i+1 : i;
			break;
		}else if(hasres && res==-1){
			*end= (i>0 && MText[i-1]=='\\' && MText[i]=='N')? i+1 : i;
			break;
		}
	}


}

void MTextEditor::ContextMenu(wxPoint mpos, int error)
{
	Menu menut;
	wxString err;
	wxArrayString suggs;
	if(error>=0){err=errs[error];}
	if(!err.IsEmpty()){
		suggs= SpellChecker::Get()->Suggestions(err);
		for(size_t i=0; i<suggs.size(); i++){
			menut.Append(i+30200,suggs[i]);
		}

		if(suggs.size()>0){menut.AppendSeparator();}
	}


	menut.Append(TEXTM_COPY,_("&Kopiuj"))->Enable(Selend.x!=Cursor.x);
	menut.Append(TEXTM_CUT,_("Wy&tnij"))->Enable(Selend.x!=Cursor.x);
	menut.Append(TEXTM_PASTE,_("&Wklej"));

	menut.AppendSeparator();
	menut.Append(TEXTM_SEEKWORDL,_("Szukaj tłumaczenia słowa na ling.pl"))->Enable(Selend.x!=Cursor.x);
	menut.Append(TEXTM_SEEKWORDB,_("Szukaj tłumaczenia słowa na pl.ba.bla"))->Enable(Selend.x!=Cursor.x);
	menut.Append(TEXTM_SEEKWORDG,_("Szukaj tłumaczenia słowa w google"))->Enable(Selend.x!=Cursor.x);
	menut.Append(TEXTM_SEEKWORDS,_("Szukaj synonimu na synonimy.net"))->Enable(Selend.x!=Cursor.x);

	if(!err.IsEmpty()){
		menut.Append(TEXTM_ADD,wxString::Format(_("&Dodaj słowo \"%s\" do słownika"),err));
	}

	menut.Append(TEXTM_DEL,_("&Usuń"))->Enable(Selend.x!=Cursor.x);

	int id=-1;
	id=menut.GetPopupMenuSelection(mpos, this);
	if(id<0)return;
	if(id>=30200){
		int from=errors[error*2];
		int to=errors[(error*2)+1];
		MText.replace(from,to-from,suggs[id-30200]);
		modified=true;
		CalcWrap();
		if(spell){CheckText();}
		int newto=from+suggs[id-30200].Len();
		SetSelection(newto,newto);
		EB->Send(EDITBOX_SPELL_CHECKER,false);
		modified=false;
	}
	else if(id==TEXTM_COPY){
		Copy();}
	else if(id==TEXTM_CUT){
		Copy(true);}
	else if(id==TEXTM_PASTE){
		Paste();}
	else if(id==TEXTM_DEL){
		long from, to;
		GetSelection(&from,&to);
		MText.Remove(from,to-from);
		CalcWrap();
		SetSelection(from,from);modified=true;}
	else if(id==TEXTM_ADD && !err.IsEmpty()){
		bool succ = SpellChecker::Get()->AddWord(err);
		if(!succ){KaiMessageBox(wxString::Format(_("Błąd, słowo \"%s\" nie zostało dodane."),err));}
		else{CheckText();EB->ClearErrs();Refresh(false);}
	}else if(id>=TEXTM_SEEKWORDL && id<=TEXTM_SEEKWORDS){
		wxString page=(id==TEXTM_SEEKWORDL)? L"http://ling.pl/" : 
			(id==TEXTM_SEEKWORDB)? L"http://pl.bab.la/slownik/angielski-polski/" : 
			(id==TEXTM_SEEKWORDG)? L"https://www.google.com/search?q=" :
			L"http://synonim.net/synonim/";
		long from, to;
		GetSelection(&from, &to);
		wxString word= MText.SubString(from, to-1).Trim();
		
		word.Replace(" ", "+");
		wxString url=page+word;
		WinStruct<SHELLEXECUTEINFO> sei;
		sei.lpFile = url.c_str();
		sei.lpVerb = wxT("open");
		sei.nShow = SW_RESTORE;
		sei.fMask = SEE_MASK_FLAG_NO_UI; // we give error message ourselves

		ShellExecuteEx(&sei);

	}


}

void MTextEditor::Copy(bool cut)
{
	if(Selend.x==Cursor.x){return;}
	int curx=Cursor.x;int selx=Selend.x;if(curx>selx){int tmp=curx;curx=selx;selx=tmp;}

	wxString whatcopy=MText.SubString(curx,selx-1);
	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData( new wxTextDataObject(whatcopy) );
		wxTheClipboard->Close();
		wxTheClipboard->Flush();
	}
	if(cut){
		MText.Remove(curx,selx-curx);
		CalcWrap();
		if(spell){CheckText();}
		SetSelection(curx,curx);
		modified=true;
	}
}


void MTextEditor::Paste()
{
	if (wxTheClipboard->Open())
	{
		if (wxTheClipboard->IsSupported( wxDF_TEXT ))
		{
			wxTextDataObject data;
			wxTheClipboard->GetData( data );
			wxString whatpaste = data.GetText();
			whatpaste.Replace("\n"," ");
			whatpaste.Replace("\r","");
			whatpaste.Replace("\f","");
			whatpaste.Replace("\t"," ");
			int curx=Cursor.x;
			int selx=Selend.x;if(curx>selx){int tmp=curx;curx=selx;selx=tmp;}
			if(Selend.x!=Cursor.x){
				MText.Remove(curx,selx-curx);}
			MText.insert(curx,whatpaste);
			modified=true;
			CalcWrap();
			if(spell){CheckText();}
			int whre=curx+whatpaste.Len();
			SetSelection(whre, whre);

		}
		wxTheClipboard->Close();
	}
}

int MTextEditor::FindError(wxPoint mpos,bool mouse)
{
	wxPoint cpos;

	if(!mouse){
		cpos=mpos;
	}else if(mouse && !HitTest(mpos, &cpos)){return-1;}

	for(size_t i=0; i<errors.size(); i+=2){
		if(cpos.x >= errors[i] && cpos.x <= errors[i+1]){
			return i/2;
		}
	}


	return -1;
}

wxPoint MTextEditor::PosFromCursor(wxPoint cur)
{
	int fw, fh;
	//if(wraps[cur.y]==cur.x){fw=0;}
	//if(cur.x<=0||cur.y<0){return wxPoint(-scPos+2, (Fheight-scPos));}
	if(wraps.size()<2 || wraps[cur.y]==cur.x){fw=0;}
	else{GetTextExtent(MText.SubString(wraps[cur.y],cur.x), &fw, &fh, NULL, NULL, &font);}
	wxPoint result;
	result.x=fw+3;
	result.y=(cur.y+1)*Fheight;
	return result;
}

void MTextEditor::OnScroll(wxScrollEvent& event)
{
	if(scroll->IsShown()){
		int newPos = event.GetPosition();
		if (scPos != newPos) {
			scPos = newPos;
			Refresh(false);
		}
	}
}

int MTextEditor::FindBracket(wxUniChar sbrkt, wxUniChar ebrkt, int pos, bool fromback)
{
	int i = pos;
	int brkts=0;
	while( (fromback)? i >= 0 : i< (int)MText.Len())
	{
		if(MText[i]==sbrkt){brkts++;}
		else if(MText[i]==ebrkt){if(brkts==0){return i;}brkts--;}

		if(fromback){i--;}else{i++;}
	}
	return -1;
}

void MTextEditor::SpellcheckerOnOff()
{
	spell = !spell;
	Refresh(false);
}

void MTextEditor::MakeCursorVisible()
{
	wxSize size = GetClientSize();
	wxPoint pixelPos = PosFromCursor(Cursor);
	pixelPos.y-=scPos;
	
	if(pixelPos.y < 3){
		scPos -= (pixelPos.y > -Fheight)? Fheight : (abs(pixelPos.y)+10);
		scPos = ((scPos / Fheight)*Fheight) - Fheight;
		if(scPos<0){scPos=0;}
	}else if(pixelPos.y > size.y-4){
		int bitmaph= (wraps.size()*Fheight)+4;
		int moving = pixelPos.y - (size.y - 10);
		scPos += (moving < Fheight)? Fheight : moving+Fheight;
		scPos = ((scPos / Fheight)*Fheight)+Fheight;
		if(scPos>bitmaph){scPos=bitmaph;}
	}
	Refresh(false);
}


BEGIN_EVENT_TABLE(MTextEditor,wxWindow)
	EVT_PAINT(MTextEditor::OnPaint)
	EVT_SIZE(MTextEditor::OnSize)
	EVT_ERASE_BACKGROUND(MTextEditor::OnEraseBackground)
	EVT_MOUSE_EVENTS(MTextEditor::OnMouseEvent)
	EVT_CHAR(MTextEditor::OnCharPress)
	EVT_KEY_DOWN(MTextEditor::OnKeyPress)
	EVT_KILL_FOCUS(MTextEditor::OnKillFocus)
	EVT_SET_FOCUS(MTextEditor::OnKillFocus)
	EVT_COMMAND_SCROLL(3333,MTextEditor::OnScroll)
	EVT_MOUSE_CAPTURE_LOST(MTextEditor::OnLostCapture)
END_EVENT_TABLE()