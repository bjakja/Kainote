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

#include "KaiTextCtrl.h"
#include "Menu.h"
#include <wx/clipbrd.h>
#include <wx/msw/private.h>


KaiTextCtrl::KaiTextCtrl(wxWindow *parent, int id, const wxString &text, const wxPoint& pos,const wxSize& size, long _style, const wxValidator & validator, const wxString & name)
	:KaiScrolledWindow(parent,id,pos,size,_style|wxWANTS_CHARS,name)
	, background(WindowBackground)
	, foreground(WindowText)
{
	KText=text;
	
	bmp=NULL;
	fsize=10;
	posY=0;
	scPos=0;
	style = _style/*|wxWANTS_CHARS*//*|wxTE_MULTILINE|wxALIGN_CENTER*/;
	if(style & wxTE_MULTILINE){maxSize=MAXINT;}else{
		style|=wxALIGN_CENTER_VERTICAL; maxSize=1000;
		KText.Replace("\r","");
		KText.Replace("\n","");
	}
	SetCursor(wxCURSOR_IBEAM);
	wxAcceleratorEntry entries[38];
	entries[0].Set(wxACCEL_NORMAL, WXK_DELETE,ID_TDEL);
	entries[1].Set(wxACCEL_NORMAL, WXK_BACK,ID_TBACK);
	entries[2].Set(wxACCEL_CTRL, WXK_BACK,ID_TCBACK);
	entries[3].Set(wxACCEL_NORMAL, WXK_LEFT,ID_TLEFT);
	entries[4].Set(wxACCEL_NORMAL, WXK_RIGHT,ID_TRIGHT);
	entries[5].Set(wxACCEL_NORMAL, WXK_UP,ID_TUP);
	entries[6].Set(wxACCEL_NORMAL, WXK_DOWN,ID_TDOWN);
	entries[7].Set(wxACCEL_NORMAL, WXK_HOME,ID_THOME);
	entries[8].Set(wxACCEL_NORMAL, WXK_END,ID_TEND);
	entries[9].Set(wxACCEL_CTRL, WXK_LEFT,ID_TCLEFT);
	entries[10].Set(wxACCEL_CTRL, WXK_RIGHT,ID_TCRIGHT);
	entries[11].Set(wxACCEL_SHIFT, WXK_LEFT,ID_TSLEFT);
	entries[12].Set(wxACCEL_SHIFT, WXK_RIGHT,ID_TSRIGHT);
	entries[13].Set(wxACCEL_SHIFT, WXK_UP,ID_TSUP);
	entries[14].Set(wxACCEL_SHIFT, WXK_DOWN,ID_TSDOWN);
	entries[15].Set(wxACCEL_SHIFT|wxACCEL_CTRL, WXK_LEFT,ID_TCSLEFT);
	entries[16].Set(wxACCEL_SHIFT|wxACCEL_CTRL, WXK_RIGHT,ID_TCSRIGHT);
	entries[17].Set(wxACCEL_SHIFT, WXK_HOME,ID_TSHOME);
	entries[18].Set(wxACCEL_SHIFT, WXK_END,ID_TSEND);
	entries[19].Set(wxACCEL_CTRL, 'A',ID_TCTLA);
	entries[20].Set(wxACCEL_CTRL, 'V',ID_TCTLV);
	entries[21].Set(wxACCEL_CTRL, 'C',ID_TCTLC);
	entries[22].Set(wxACCEL_CTRL, 'X',ID_TCTLX);
	entries[23].Set(wxACCEL_NORMAL, 395,ID_TWMENU);
	entries[24].Set(wxACCEL_NORMAL, WXK_PAGEDOWN,ID_TPDOWN);
	entries[25].Set(wxACCEL_NORMAL, WXK_PAGEUP,ID_TPUP);
	entries[26].Set(wxACCEL_SHIFT|wxACCEL_CTRL, WXK_END,ID_TCSEND);
	int numEntries = 27;
	bool setNumpadAccels = !Options.GetBool(TextFieldAllowNumpadHotkeys);
	if(setNumpadAccels){
		entries[27].Set(wxACCEL_NORMAL, WXK_NUMPAD0,WXK_NUMPAD0+10000);
		entries[28].Set(wxACCEL_NORMAL, WXK_NUMPAD1,WXK_NUMPAD1+10000);
		entries[29].Set(wxACCEL_NORMAL, WXK_NUMPAD2,WXK_NUMPAD2+10000);
		entries[30].Set(wxACCEL_NORMAL, WXK_NUMPAD3,WXK_NUMPAD3+10000);
		entries[31].Set(wxACCEL_NORMAL, WXK_NUMPAD4,WXK_NUMPAD4+10000);
		entries[32].Set(wxACCEL_NORMAL, WXK_NUMPAD5,WXK_NUMPAD5+10000);
		entries[33].Set(wxACCEL_NORMAL, WXK_NUMPAD6,WXK_NUMPAD6+10000);
		entries[34].Set(wxACCEL_NORMAL, WXK_NUMPAD7,WXK_NUMPAD7+10000);
		entries[35].Set(wxACCEL_NORMAL, WXK_NUMPAD8,WXK_NUMPAD8+10000);
		entries[36].Set(wxACCEL_NORMAL, WXK_NUMPAD9,WXK_NUMPAD9+10000);
		numEntries = 37;
	}
	bool processEnter = !(style & wxTE_PROCESS_ENTER) && (style & wxTE_MULTILINE);
	if(processEnter){entries[37].Set(wxACCEL_NORMAL, WXK_RETURN,ID_TRETURN); numEntries++;}
	wxAcceleratorTable accel(numEntries, entries);
	SetAcceleratorTable(accel);
	Connect(ID_TDEL,ID_TRETURN,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&KaiTextCtrl::OnAccelerator);
	if(setNumpadAccels){
		Bind(wxEVT_COMMAND_MENU_SELECTED,[=](wxCommandEvent &evt){
			int key = evt.GetId() - 10276;
			wxKeyEvent kevt;
			kevt.m_uniChar = key;
			OnCharPress(kevt);
		}, WXK_NUMPAD0+10000, WXK_NUMPAD9+10000);
	}


	Cursor.x=Cursor.y=Selend.x=Selend.y = oldstart = oldend=0;

	holding = dholding = firstdhold = modified = false;

	font= parent->GetFont();//wxFont(9,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,"Tahoma",wxFONTENCODING_DEFAULT);
	int fw,fh;
	GetTextExtent("#TWFfGH", &fw, &fh);
	Fheight=fh;
	wxSize newSize((size.x<1)? 100 : size.x, (size.y<1)? fh+10 : size.y);
	caret= new wxCaret(this, 1, Fheight);
	SetCaret(caret);
	caret->Move(3,(newSize.y-fh)/2);
	caret->Show();
	SetMinSize(newSize);

	CalcWrap(false);
	SetValidator(validator);

	Bind(wxEVT_LEFT_DOWN, &KaiTextCtrl::OnMouseEvent, this);
	Bind(wxEVT_LEFT_UP, &KaiTextCtrl::OnMouseEvent, this);
	Bind(wxEVT_LEFT_DCLICK, &KaiTextCtrl::OnMouseEvent, this);
	Bind(wxEVT_RIGHT_UP, &KaiTextCtrl::OnMouseEvent, this);
	Bind(wxEVT_MOTION, &KaiTextCtrl::OnMouseEvent, this);
	Bind(wxEVT_ENTER_WINDOW, &KaiTextCtrl::OnMouseEvent, this);
	Bind(wxEVT_LEAVE_WINDOW, &KaiTextCtrl::OnMouseEvent, this);
	Bind(wxEVT_MOUSEWHEEL, &KaiTextCtrl::OnMouseEvent, this);
	Bind(wxEVT_KEY_DOWN, &KaiTextCtrl::OnKeyPress, this);
	
	timer.SetOwner(this, 29067);
	Bind(wxEVT_TIMER, [=](wxTimerEvent &evt){
		CalcWrap(false); 
		Cursor.x = Selend.x = KText.Len() - 1;
		Cursor.y = Selend.y = FindY(Cursor.x);
		MakeCursorVisible(); 
	}, 29067);
}

KaiTextCtrl::~KaiTextCtrl()
{
	if(bmp)delete bmp;
}

void KaiTextCtrl::SetValue(const wxString &text, bool modif, bool newSel)
{
	if(modif){modified=modif;}
	KText=text;
	if(!(style & wxTE_MULTILINE)){
		KText.Replace("\r","");
		KText.Replace("\n","");
	}
	textStyles.clear();
	CalcWrap(false);
	
	if(newSel){SetSelection(0,0);}
	else{
		if((size_t)Cursor.x>KText.Len()){Cursor.x = KText.Len();Cursor.y = FindY(Cursor.x);}
		Refresh(false);
	}
}
void KaiTextCtrl::AppendText(const wxString &text)
{
	size_t len = KText.Len();
	if(len >= maxSize){
		return;
	}
	if(len + text.Len()> maxSize){
		KText << text.SubString(0, maxSize - len);
	}else{
		KText<<text;
	}
	if(!(style & wxTE_MULTILINE)){
		KText.Replace("\r","");
		KText.Replace("\n","");
	}
	//CalcWrap(false, len); Refresh(false);
	timer.Start(10, true);
}

void KaiTextCtrl::AppendTextWithStyle(const wxString &text, const wxColour &color)
{
	size_t textLen = KText.Len();
	size_t size = textStyles.size();
	if (size > 0 && textStyles[size - 1].to == textLen - 1 && textStyles[size - 1].color == color){
		textStyles[size - 1].to = textLen + text.Len() - 1;
	}
	else{
		textStyles.push_back(TextStyle(textLen, textLen + text.Len() - 1, color));
	}
	AppendText(text);
}

void KaiTextCtrl::SetStyle(size_t from, size_t to, const wxColour &color)
{
	size_t ret;
	FindStyle(from, &ret, true);
	textStyles.insert(textStyles.begin()+ret, TextStyle(from, to, color));
	Refresh(false);
}

bool KaiTextCtrl::FindStyle(size_t pos, size_t *ret, bool returnSize /*= false*/)
{
	for (size_t i = 0; i < textStyles.size(); i++){
		if (textStyles[i].to >= pos){
			bool ishit = (textStyles[i].from <= pos);
			*ret = i;
			if (ishit){ *ret--; }
			return ishit;
		}
	}
	*ret = (returnSize) ? textStyles.size() : textStyles.size() - 1;
	return false;
}

void KaiTextCtrl::MoveStyles(size_t textPos, int moveIndex)
{
	size_t tablePos=0;
	bool res = FindStyle(textPos, &tablePos, true);
	for (size_t i = tablePos; i < textStyles.size(); i++){
		if (i == tablePos && textStyles[i].to < textPos){
			continue;
		}
		else if (i == tablePos && textStyles[i].from < textPos){
			textStyles[i].to += moveIndex;
			continue;
		}
		textStyles[i].from += moveIndex;
		textStyles[i].to += moveIndex;
	}
}

void KaiTextCtrl::DeleteStyles(size_t textStart, size_t textEnd)
{
	assert(textEnd > textStart);
	size_t i = 0;
	size_t newIndex = textEnd - textStart;
	while ( i < textStyles.size()){
		TextStyle & ts = textStyles[i];
		if (ts.to > textEnd){
			break;
		}
		if (ts.to >= textStart){
			
			if (ts.from < textStart && textStart > 0){
				ts.to = textStart-1;
			}
			else if (ts.to < textEnd){
				textStyles.erase(textStyles.begin() + i);
				continue;
			}
			else{
				ts.from = textStart;
			}
		}
		
		i++;
	}
}

void KaiTextCtrl::CalcWrap(bool sendevent)
{
	//Wrapped=KText;
	long multiline = (style & wxTE_MULTILINE);
	wraps.clear();
	wraps.Add(0);
	positioning.clear();
	positioning.Add(0);
	int w,h,fw=0,fh=0;
	GetSize(&w,&h);
	wxMemoryDC dc((bmp) ? *bmp : wxBitmap(10, 10));
	dc.SetFont(font);
	if(KText!="" && multiline){
		int podz = 0;
		//wxString wrapchars=" \\,;}{()\n\r";
		size_t i = 0;
		int nwrap=-1;
		int allwrap=-1;
		size_t len = KText.Len();
		bool seekSpace = true;
		int mesureSize = w - 10;
		if (mesureSize <= 10){ for (int i = 1; i < len; i++){ wraps.Add(i);  positioning.Add(5); return; } }
		int stylewrap = (style & wxALIGN_CENTER_HORIZONTAL) ? 1 : (style & wxALIGN_RIGHT) ? 2 : 0;
		int pos = 5;
		while (i < len)
		{
			size_t nfound = KText.find(wxUniChar('\n'), i);
			i = (nfound != -1) ? nfound : len - 1;
			size_t j = i;
			while (podz < i)
			{
				GetTextExtent(KText.Mid(podz, j-podz+1), &fw, &fh);

				allwrap = j;
				if (fw >= mesureSize){
					if (seekSpace){
						size_t res = KText.rfind(wxUniChar(' '), j);
						if (res != -1 && res > podz){
							j = res-1;
							nwrap = j+2;
							continue;
						}
					}
					seekSpace = false;
					j--;
					continue;
				}
				else if (0 >= j - podz + 1){ 
					j = allwrap = podz + 1; 
				}
				//else if (i != j){
				//	size_t res = KText.find(' ', j);
				//	if (res != -1 && res>podz && res<i){
				//		nwrap = j;
				//		j = res;
				//		//if (KText[j] == ' ' || KText[j] == '\n'){ nwrap++; }
				//	}
				//	else{
				//		j++; 
				//	}
				//	continue;
				//}
				//if (nwrap < 0 && allwrap < 0){
					//j++; continue;
				//}
				size_t wwrap = (nwrap != -1 && i != j) ? nwrap : allwrap + 1;
				if (stylewrap){
					GetTextExtent(KText.Mid(podz, wwrap - podz), &fw, &fh);
					pos = (style == 1) ? ((w - fw) / 2) : (w - fw) - 5;
				}
				positioning.Add(pos);
				wraps.Add((wwrap > len) ? len : wwrap);
				podz = wwrap;
				nwrap = -1;
				allwrap = -1;
				seekSpace = true;
				j = i;
			}
			i++;
			
		}
	}else{
		wraps.Add(KText.Len());
		GetTextExtent(KText, &fw, &fh);
		int rightPos = (w - fw);
		int pos = (style & wxALIGN_CENTER_HORIZONTAL)? (rightPos/2) : 
			(style & wxALIGN_RIGHT)? rightPos-5 : 5;

		if(pos<5){pos=5;}
		positioning.Add(pos);
	}
	int rightPos = h - Fheight;
	posY = (style & wxALIGN_CENTER_VERTICAL)? (rightPos/2) : (style & wxALIGN_BOTTOM)? rightPos-2 : 2;
	if(posY<2){posY=2;}
	//if(multiline){posY -=scPos;}
	if(sendevent){wxCommandEvent evt2(wxEVT_COMMAND_TEXT_UPDATED, GetId()); AddPendingEvent(evt2);}
}

void KaiTextCtrl::OnCharPress(wxKeyEvent& event)
{
	int key=event.GetKeyCode();
	if(!(!(event.ControlDown() && !event.AltDown()) && (key>30 || key == 0))){
		event.Skip();return;
	}
	wxUniChar wkey=event.GetUnicodeKey();
	if((style & wxTE_READONLY)){return;}
	if(wkey){
		if(Cursor!=Selend){
			int curx=Cursor.x;
			int selx=Selend.x;
			if(curx>selx){int tmp=curx;curx=selx;selx=tmp;}
			KText.Remove(curx,selx-curx);
			if(Cursor.x<Selend.x){Selend=Cursor;}else{Cursor=Selend;}
		}
		size_t len=KText.Len();
		if(len >= maxSize){wxBell();return;}
		if((size_t)Cursor.x>=len){KText<<wkey;}
		else{KText.insert(Cursor.x,1,wkey);}
		MoveStyles(Cursor.x, 1);
		CalcWrap();
		//SendEvent();
		if(Cursor.x+1>wraps[Cursor.y+1]){Cursor.y++;}
		Cursor.x++;
		Selend=Cursor;
		//Refresh(false);
		MakeCursorVisible();
		modified=true;
	}

}

void KaiTextCtrl::OnKeyPress(wxKeyEvent& event)
{
	event.Skip();
}

void KaiTextCtrl::OnAccelerator(wxCommandEvent& event)
{
	//bool shouldpropagate = event.ShouldPropagate();
	int step=0;
	int len;
	int ID=event.GetId();
	long readOnly = (style & wxTE_READONLY);
	wxPoint pixelPos;
	switch(ID){
	case ID_TCBACK:
		if(Cursor.x==0 || readOnly){return;}
		Selend.x=Cursor.x;
		FindWord((Cursor.x<2)? 0 : Cursor.x-1 ,&Cursor.x,&len);
		if(Cursor.x==1 && KText[0]==' '){Cursor.x--;}
	case ID_TDEL:
	case ID_TBACK:
		if(readOnly){return;}
		if(Cursor!=Selend){
			int curx=Cursor.x;
			int selx=Selend.x;
			if(curx>selx){int tmp=curx;curx=selx;selx=tmp;}
			KText.Remove(curx,selx-curx);
			DeleteStyles(curx, selx);
			MoveStyles(curx, -(selx - curx));
			Selend=Cursor;
			CalcWrap();
			//SendEvent();
			SetSelection(curx,curx);
		}else
		{
			if(ID==ID_TBACK){
				if(Cursor.x<1){return;} 
				Cursor.x--;
			}
			if(ID==ID_TDEL && Cursor.x >= (int)KText.Len()){return;}
			KText.Remove(Cursor.x,1);
			DeleteStyles(Cursor.x, Cursor.x+1);
			MoveStyles(Cursor.x, -1);
		}
		len=wraps.size();
		CalcWrap();
		//SendEvent();
		//wxLogStatus("cursorx %i wraps %i",Cursor.x, wraps[Cursor.y]);
		if(Cursor.x<wraps[Cursor.y] || (Cursor.x==wraps[Cursor.y] && len != wraps.size())){Cursor.y--;}
		else if(Cursor.x>wraps[Cursor.y+1]){Cursor.y++;}
		Selend=Cursor;
		MakeCursorVisible();
		//Refresh(false);
		modified=true;
		break;

	case ID_TLEFT:
	case ID_TCLEFT:
	case ID_TSLEFT:
	case ID_TCSLEFT:
		if(ID==ID_TLEFT && Selend.x<Cursor.x){Cursor=Selend; MakeCursorVisible();return;}
		if(Cursor.x<1){return;}
		if(ID==ID_TCLEFT||ID==ID_TCSLEFT){
			FindWord(Cursor.x-1,&Cursor.x,0);
		}
		if(Cursor.x-1<wraps[Cursor.y] && Cursor.x!=0){Cursor.y--;}
		else if(ID!=ID_TCLEFT&&ID!=ID_TCSLEFT){Cursor.x--;}


		if(ID<ID_TSLEFT){Selend=Cursor;}
		//Refresh(false);
		MakeCursorVisible(true);
		break;

	case ID_TRIGHT:
	case ID_TCRIGHT:
	case ID_TSRIGHT:
	case ID_TCSRIGHT:
		if(ID==ID_TRIGHT && Selend.x>Cursor.x){Cursor=Selend;MakeCursorVisible();return;}
		if(Cursor.x>=(int)KText.Len()){return;}
		if(ID==ID_TCRIGHT||ID==ID_TCSRIGHT){
			if(Cursor.x==KText.Len()-1){
				Cursor.x++;
			}else{
				FindWord(Cursor.x+1,0,&Cursor.x);}
		}
		if(Cursor.x+1>wraps[Cursor.y+1] && Cursor.y<(int)wraps.size()-2){Cursor.y++;}
		else if(ID!=ID_TCRIGHT && ID!=ID_TCSRIGHT){Cursor.x++;}

		if(ID<ID_TSRIGHT){Selend=Cursor;}
		MakeCursorVisible();
		break;

	case ID_TDOWN:
	case ID_TSDOWN:
		len=KText.Len();
		if(Cursor.y>=(int)wraps.size()-2){Cursor.y=wraps.size()-2; Cursor.x=len;}
		else{
			pixelPos = PosFromCursor(Cursor);
			HitTest(pixelPos, &Cursor);
			/*Cursor.x-=wraps[Cursor.y];
			Cursor.y++;
			Cursor.x+=wraps[Cursor.y];
			if(Cursor.x>len){Cursor.x=len;}*/
		}
		if(ID<ID_TSDOWN){Selend=Cursor;}
		MakeCursorVisible(true);
		break;

	case ID_TUP:
	case ID_TSUP:

		pixelPos = PosFromCursor(Cursor);
		pixelPos.y -= (Fheight*2);
		HitTest(pixelPos, &Cursor);
		//if(Cursor.y<1){return;}
		/*Cursor.x-=wraps[Cursor.y];
		Cursor.y--;
		if(Cursor.y<1){Cursor.y=0;Cursor.x=0;}
		else{Cursor.x+=wraps[Cursor.y];}*/
		if(ID<ID_TSUP){Selend=Cursor;}
		MakeCursorVisible(true);
		break;
	case ID_THOME:
	case ID_TEND:
	case ID_TSHOME:
	case ID_TSEND:
	case ID_TCSEND:
		Cursor.x=wraps[(ID==ID_TEND||ID==ID_TSEND) ? Cursor.y+1 : Cursor.y];
		if(ID==ID_TCSEND){Cursor.x=KText.Len(); Cursor.y=wraps.size()-2;}
		if(ID<ID_TSHOME){Selend=Cursor;}
		MakeCursorVisible(true);
		break;
	case ID_TCTLA:

		Cursor.x=Cursor.y=0;
		Selend.x=KText.Len(); Selend.y=wraps.size()-2;
		//Refresh(false);
		MakeCursorVisible(true);
		break;
	case ID_TCTLV:
		if(readOnly){return;}
		Paste();
		break;
	case ID_TCTLC:
	case ID_TCTLX:
		if(ID>ID_TCTLC && readOnly){return;}
		Copy(ID>ID_TCTLC);

		break;
	case ID_TWMENU:
		//Selend=Cursor;
		ContextMenu(PosFromCursor(Cursor));
		break;
	case ID_TRETURN:
		if(KText.Len()>=maxSize){wxBell();return;}
		KText.insert(Cursor.x,"\n");
		MoveStyles(Cursor.x, 1);
		Cursor.x++;Cursor.y++;
		Selend=Cursor;
		CalcWrap();
		//SendEvent();
		//Refresh(false);
		MakeCursorVisible();
		break;
	case ID_TPDOWN:
	case ID_TPUP:

		break;

	default:

		break;
	}
}

void KaiTextCtrl::OnMouseEvent(wxMouseEvent& event)
{
	bool click=event.LeftDown();
	bool leftup=event.LeftUp();
	if(event.ButtonDown()){SetFocus();if(!click){Refresh(false);}}

	if(leftup && (holding||dholding)){
		holding=dholding=false;
		if(HasCapture()){ReleaseMouse();}
		//event.Skip();
		return;
	}
	if(event.LeftDClick()){
		wxPoint mpos=event.GetPosition();
		int start,end;
		wxPoint ht;
		if(Cursor.x<Selend.x){Selend=Cursor;}else{Cursor=Selend;}
		HitTest(mpos,&ht);
		FindWord(ht.x,&start,&end);
		oldend=tmpend=end;
		oldstart=tmpstart=start;
		SetSelection(start,end);
		firstdhold = dholding = true;
		if(!HasCapture()){CaptureMouse();}
		//event.Skip();
		return;
	}
	if(click){
		wxPoint cur;
		HitTest(event.GetPosition(),&cur);
		Cursor=cur;
		if(!event.ShiftDown()){Selend=Cursor;}
		Refresh(false);
		holding=true;
		if(!HasCapture()){CaptureMouse();}
	}

	if(holding){
		wxPoint cur;
		HitTest(event.GetPosition(),&cur);
		Cursor=cur;
		MakeCursorVisible(true);
		//Refresh(false);
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
		ContextMenu(pos);
	}

	if(event.GetWheelRotation() != 0 && (style & wxTE_MULTILINE)){
		if(style & SCROLL_ON_FOCUS && !HasFocus()){event.Skip(); return;}
		int step = 10 * event.GetWheelRotation() / event.GetWheelDelta();
		if(step>0 && scPos==0){return;}
		scPos = MAX(scPos - step, 0);
		Refresh(false);
	}
	event.Skip();
}


void KaiTextCtrl::OnSize(wxSizeEvent& event)
{
	wxSize size = GetClientSize();
	if (lastSize != size){
		CalcWrap(false);
		Cursor.y = FindY(Cursor.x);
		Selend.y = FindY(Selend.x);
		MakeCursorVisible();
		lastSize = size;
	}
}

int KaiTextCtrl::FindY(int x)
{
	for(size_t p=1; p<wraps.size(); p++){ 
		if(x<wraps[p]){return (p-1);} 
	}
	return wraps.size()-2;
}

void KaiTextCtrl::OnPaint(wxPaintEvent& event)
{
	int w = 0, h = 0;
	GetClientSize(&w,&h);
	if(w==0||h==0){return;}
	wxPaintDC dc(this);
	int bitmaph;
	int bitmapw;
	long multiline = (style & wxTE_MULTILINE);
	if(multiline){
		bitmaph= (wraps.size()*Fheight)+4;
		bitmapw = w;
		if(bitmaph>h){
			if(!HasScrollbar(wxVERTICAL)){
				//CalcWrap(false);
				bitmaph= (wraps.size()*Fheight)+4;
			}
			int sw=0,sh=0;
			int diff= h;
			int diff2= bitmaph;
			if(scPos>diff2-diff){scPos=diff2-diff;}
			if(SetScrollBar(wxVERTICAL, scPos,diff,diff2 ,diff-2)){
				GetClientSize(&w,&h);
			}
		}else{
			
			bitmaph=h;
			scPos=0;
			if(SetScrollBar(wxVERTICAL, scPos,h,0,h)){
				GetClientSize(&w,&h);
			}
		}
		
	}
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

	DrawFld(bmpDC,w,h,w,h);


	dc.Blit(0,0,w,h,&bmpDC,0,0);

}

void KaiTextCtrl::DrawFld(wxDC &dc,int w, int h, int windoww, int windowh)
{
	int fw=0,fh=0;
	bool enabled = IsThisEnabled();
	const wxColour & bg = (enabled)? Options.GetColour(background) : Options.GetColour(WindowBackgroundInactive);
	const wxColour & fg = Options.GetColour(foreground);
	wxColour border = (style & wxBORDER_NONE)? bg : 
		(HasFocus())? Options.GetColour(TextFieldBorderOnFocus) : 
		(enabled)? Options.GetColour(TextFieldBorder) : 
		Options.GetColour((style & wxBORDER_NONE)? WindowBackgroundInactive : ButtonBorderInactive);
	dc.SetFont(font);
	dc.SetBrush(wxBrush(bg));
	dc.SetPen(wxPen(border));
	dc.DrawRectangle(0,0,w,h);
	if(wraps.size()<2 || positioning.size()<2){return;}
	const wxColour &cselection = (HasFocus())? Options.GetColour(TextFieldSelection) : 
		Options.GetColour(TextFieldSelectionNoFocus);
	const wxColour &textInactive = Options.GetColour(WindowTextInactive);

	//Contsel=false;
	//posY=2;
	long multiline = (style & wxTE_MULTILINE);
	int tmpPosY = posY;
	int tmpPosX = (multiline) ? 0 : -scPos;
	if (multiline){ tmpPosY -= scPos; }
	bool isfirst=true;
	int wline=0;
	int wchar=0;

	dc.SetFont(font);
	wxString alltext=KText+" ";
	int len=alltext.Len();
	wxUniChar bchar=alltext[Cursor.x];


	int fww;
	dc.SetPen(*wxTRANSPARENT_PEN);

	if(Cursor.x!=Selend.x || Cursor.y != Selend.y){
		wxPoint fst, scd;
		if((Cursor.x + Cursor.y) > (Selend.x + Selend.y)){fst=Selend;scd=Cursor;}
		else{fst=Cursor, scd=Selend;}

		dc.SetBrush(wxBrush(wxColour(cselection)));
		fww=0;
		//rysowanie zaznaczenia
		for(int j=fst.y; j<=scd.y; j++){
			if (multiline && ((j * Fheight) + Fheight - scPos < 0 || (j * Fheight) - scPos > h)){
				continue;
			}
			if(j==fst.y){
				wxString ftext=KText.SubString(wraps[j],fst.x-1);
				if(wraps[j]>fst.x-1){fw=0;}
				else{
					GetTextExtent(ftext, &fw, &fh);
				}
				wxString stext=KText.SubString(fst.x,(fst.y==scd.y)? scd.x-1 : wraps[j+1]-1);
				GetTextExtent(stext, &fww, &fh);

			}else{
				fw=0;
				wxString selText = KText.SubString(wraps[j], (j==scd.y)? scd.x-1 : wraps[j+1]-1);
				GetTextExtent(selText, &fww, &fh);
			}
			dc.DrawRectangle(positioning[j+1] + fw + tmpPosX,(j*Fheight)+tmpPosY,fww,Fheight);
		}
	}
	int cursorPos = Cursor.x;
	int cursorI = Cursor.y;
	//if(cursorI<0){Cursor.y=0; cursorI=0;}
	if(HasFocus() && (cursorPos>=wraps[cursorI] && cursorPos<=wraps[cursorI+1])){
		int fww=-1;
		if(cursorPos!=wraps[cursorI]){
			size_t start = wraps[cursorI];
			size_t end = (cursorPos<wraps[cursorI+1])? cursorPos-1 : wraps[cursorI+1]-1;
			wxString beforeCursor = KText.SubString(start, end);
			GetTextExtent(beforeCursor, &fww, &fh);
		}
		caret->Move(positioning[cursorI+1] + fww + tmpPosX, tmpPosY + (Fheight * cursorI));

	}
	size_t k = 0;
	bool blockSkip = false;
	//rysowanie tesktu
	for(size_t i = 1; i< wraps.size(); i++){
		if (multiline && (tmpPosY + Fheight < 0 || tmpPosY > h)){
			tmpPosY += Fheight;
			continue;
		}
		size_t linefrom = wraps[i - 1];
		size_t lineto = wraps[i] - 1;
		wxString line = KText.SubString(linefrom, lineto);
		
		bool drawed = false;
		int drawX = 0;
		size_t lastto = linefrom;
		while (k < textStyles.size() && enabled){
			size_t from = textStyles[k].from;
			size_t to = textStyles[k].to;
			if (to < linefrom && !blockSkip){
				k++;
			}
			else if (/*from >= linefrom && */from <= lineto || blockSkip){
				//draw
				if (from - 1 > linefrom && from > 0){
					size_t newto = (from - linefrom) - 1;
					if (lastto < from-1){
						if (lastto > linefrom + 1){
							wxString normalstyle = line.SubString(0, lastto - linefrom - 1);
							GetTextExtent(normalstyle, &fww,0);
						}
						else{ fww = 0; }
						dc.SetTextForeground(fg);
						wxString normalText = line.SubString((lastto < linefrom)? 0 : lastto - linefrom, newto);
						normalText.Replace("\r", "");
						normalText.Replace("\n", "");
						normalText.Replace("\t", "        ");
						dc.DrawText(normalText, positioning[i] + tmpPosX + fww, tmpPosY);
					}
					wxString preline = line.SubString(0, newto);
					GetTextExtent(preline, &drawX,0);
				}
				else{ drawX = 0; }
				dc.SetTextForeground(textStyles[k].color);
				wxString colorizedText = line.SubString((from < linefrom) ? 0 : from - linefrom, (to < lineto) ? to - linefrom : line.Len() - 1);
				colorizedText.Replace("\r", "");
				colorizedText.Replace("\n", "");
				colorizedText.Replace("\t", "        ");
				dc.DrawText(colorizedText, positioning[i] + tmpPosX + drawX, tmpPosY);
				drawed = true;
				if (to <= lineto){
					blockSkip = false;
					k++;
				}
				else{
					blockSkip = true;
					lastto = to + 1;
					break;
				}
			}
			else{
				//lastto = to + 1;
				break;
			}
			lastto = to+1;
		}
		
		if (drawed){
			if (lastto < lineto){
				if (lastto - 1 >= linefrom){
					wxString preline = line.SubString(0, (lastto - linefrom) - 1);
					GetTextExtent(preline, &drawX, NULL);
				}
				else{ drawX = 0; }
				dc.SetTextForeground(fg);
				wxString normalText = line.SubString(lastto >= linefrom ? lastto - linefrom : 0, line.Len() - 1);
				normalText.Replace("\r", "");
				normalText.Replace("\n", "");
				normalText.Replace("\t", "        ");
				dc.DrawText(normalText, positioning[i] + tmpPosX + drawX, tmpPosY);
			}
		}else{
			line.Replace("\r", "");
			line.Replace("\n", "");
			line.Replace("\t", "        ");
			dc.SetTextForeground((enabled) ? fg : textInactive);
			dc.DrawText(line, positioning[i] + tmpPosX, tmpPosY);
		}

		tmpPosY+=Fheight;

	}
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dc.SetPen(wxPen(border));
	dc.DrawRectangle(0,0,w,h);

}

bool KaiTextCtrl::HitTest(wxPoint pos, wxPoint *cur)
{
	int w,h,fw=0,fh=0;
	GetClientSize(&w,&h);
	if(style & wxTE_MULTILINE){pos.y+=(scPos);}
	if(!(style & wxTE_MULTILINE)){pos.x+=(scPos);}

	cur->y= (!(style & wxTE_MULTILINE))? 0 : (pos.y-posY)/Fheight;
	if(cur->y<0||wraps.size()<2){cur->y=0;cur->x=0;return false;}
	if(cur->y>=(int)wraps.size()-1){
		cur->y=wraps.size()-2;cur->x=wraps[wraps.size()-2];
	}else{cur->x=wraps[cur->y];}

	bool find=false;
	wxString txt=KText+" ";

	int wlen=KText.Len();
	int fww;
	for(int i = cur->x;i<wraps[cur->y+1]+1;i++)
	{
		GetTextExtent(txt.SubString(cur->x,i), &fw, &fh);
		GetTextExtent(txt[i], &fww, &fh);//}
		fw += positioning[cur->y+1];
		if(fw-(fww/2)-1>pos.x){cur->x=i; find=true;break;}
	}
	if(!find){
		cur->x = wraps[cur->y+1];
	}
	return find;
}

bool KaiTextCtrl::Modified()
{
	return modified;
}
void KaiTextCtrl::GetSelection(long *start, long *end)
{
	bool iscur= ((Cursor.x + Cursor.y) > (Selend.x + Selend.y));
	*start=(!iscur)? Cursor.x : Selend.x;
	*end=(iscur)? Cursor.x : Selend.x;
}

void KaiTextCtrl::SetSelection(int start, int end, bool noEvent)
{
	//if((Cursor.x!=end || Selend.x!=start) && !noEvent){wxCommandEvent evt(CURSOR_MOVED,GetId());AddPendingEvent(evt);}
	int len = KText.Len();
	Cursor.x=MID(0, end, len);
	Selend.x=MID(0, start, len);
	Selend.y=FindY(Selend.x);
	Cursor.y=FindY(Cursor.x);

	//Refresh(false);
	MakeCursorVisible();
}

wxString KaiTextCtrl::GetValue() const
{
	return KText;
}

void KaiTextCtrl::Replace(int start, int end, const wxString & rep, bool sendEvent /*= true*/)
{
	modified=true;
	KText.replace(start, end-start,rep);
	if(start!=end)
		DeleteStyles(start, end);
	MoveStyles(start, rep.Len() - (end - start));
	CalcWrap(sendEvent);
	Cursor.x=0;Cursor.y=0;
	Selend=Cursor;
	MakeCursorVisible();
}

void KaiTextCtrl::OnKillFocus(wxFocusEvent& event)
{
	Refresh(false);
}

void KaiTextCtrl::FindWord(int pos, int *start, int *end)
{
	wxString wfind=" }])-—'`\"\\;:,.({[><?!*~@#$%^&/+=\t";
	int len=KText.Len();
	if(len<1){Cursor.x = Cursor.y = 0; *start=0; *end=0; return;}
	bool fromend=(start!=NULL);

	if(!fromend){pos--;}
	pos=MID(0,pos,len-1);
	bool hasres=false;
	int lastres=-1;
	if(fromend){
		*start=(fromend)? 0 : len;
		for(int i=pos; i>=0; i--){
			int res=wfind.Find(KText[i]);
			if(res!=-1){lastres=res;}
			if(res!=-1&&!hasres){
				if(i==pos){hasres=true;continue;}
				*start = i+1;
				break;
			}else if(hasres&&res==-1){
				if(i+1==pos){continue;}
				else if(lastres<1 && i+2==pos){hasres=false; continue;}
				*start=(lastres>0 && (KText[pos]==' '||i+2<pos))?i+1 : i+2;
				break;
			}
		}
	}
	if(!end){return;}
	*end=(fromend && end==NULL)? 0 : len;
	for(int i=pos; i<len; i++){
		int res=wfind.Find(KText[i]);
		if(res!=-1&&!hasres){
			//wxLogStatus("ipos2 %i %i",i, pos);
			if(i==pos){hasres=true;continue;}
			*end=(res<1)? i+1 : i;
			break;
		}else if(hasres&&res==-1){
			//wxLogStatus("ipos3 %i %i",i, pos);
			*end= i;
			break;
		}
	}


}

void KaiTextCtrl::ContextMenu(wxPoint mpos)
{
	Menu menut;
	menut.Append(TEXT_COPY,_("&Kopiuj"))->Enable(Selend.x!=Cursor.x);
	menut.Append(TEXT_CUT,_("Wy&tnij"))->Enable(Selend.x!=Cursor.x && !(style & wxTE_READONLY));
	menut.Append(TEXT_PASTE,_("&Wklej"))->Enable(!(style & wxTE_READONLY));

	menut.AppendSeparator();
	menut.Append(TEXT_SEEKWORDL,_("Szukaj tłumaczenia słowa na ling.pl"))->Enable(Selend.x!=Cursor.x);
	menut.Append(TEXT_SEEKWORDB,_("Szukaj tłumaczenia słowa na pl.ba.bla"))->Enable(Selend.x!=Cursor.x);
	menut.Append(TEXT_SEEKWORDG,_("Szukaj tłumaczenia słowa w google"))->Enable(Selend.x!=Cursor.x);

	menut.Append(TEXT_DEL,_("&Usuń"))->Enable(Selend.x!=Cursor.x);

	int id=-1;
	id=menut.GetPopupMenuSelection(mpos, this);

	if(id==TEXT_COPY){
		Copy();}
	else if(id==TEXT_CUT){
		Copy(true);}
	else if(id==TEXT_PASTE){
		Paste();}
	else if(id==TEXT_DEL){
		long from, to;
		GetSelection(&from,&to);
		KText.Remove(from,to-from);
		DeleteStyles(from, to);
		MoveStyles(from, -(to - from));
		CalcWrap();
		//SendEvent();
		SetSelection(from,from);modified=true;
	}else if(id>=TEXT_SEEKWORDL && id<=TEXT_SEEKWORDG){
		wxString page=(id==TEXT_SEEKWORDL)? L"http://ling.pl/" : 
			(id==TEXT_SEEKWORDB)? L"http://pl.bab.la/slownik/angielski-polski/" : L"https://www.google.com/search?q=";
		long from, to;
		GetSelection(&from, &to);
		wxString word= KText.SubString(from, to-1).Trim();
		//if(word.IsWord()){
		word.Replace(" ", "+");
		wxString url=page+word;
		WinStruct<SHELLEXECUTEINFO> sei;
		sei.lpFile = url.c_str();
		sei.lpVerb = wxT("open");
		sei.nShow = SW_RESTORE;
		sei.fMask = SEE_MASK_FLAG_NO_UI; // we give error message ourselves

		ShellExecuteEx(&sei);
		//}

	}


}

void KaiTextCtrl::Copy(bool cut)
{
	if(Selend.x==Cursor.x){return;}
	int curx=Cursor.x;int selx=Selend.x;if(curx>selx){int tmp=curx;curx=selx;selx=tmp;}

	wxString whatcopy=KText.SubString(curx,selx-1);
	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData( new wxTextDataObject(whatcopy) );
		wxTheClipboard->Close();
		wxTheClipboard->Flush();
	}
	if(cut){
		KText.Remove(curx,selx-curx);
		DeleteStyles(curx, selx);
		MoveStyles(curx, -(selx - curx));
		CalcWrap();
		//SendEvent();
		SetSelection(curx,curx);
		modified=true;
	}
}


void KaiTextCtrl::Paste()
{
	if (wxTheClipboard->Open())
	{
		if (wxTheClipboard->IsSupported( wxDF_TEXT ))
		{
			wxTextDataObject data;
			wxTheClipboard->GetData( data );
			wxString whatpaste = data.GetText();
			//whatpaste.Replace("\t"," ");
			if(!(style & wxTE_MULTILINE)){
				whatpaste.Replace("\n"," ");
				whatpaste.Replace("\r","");
			}

			if(KText.Len() >= maxSize){
				wxTheClipboard->Close();
				wxBell();
				return;
			}
			if(KText.Len() + whatpaste.Len()> maxSize){
				whatpaste = whatpaste.SubString(0, maxSize - KText.Len());
			}
			
			int curx=Cursor.x;
			int selx=Selend.x;if(curx>selx){int tmp=curx;curx=selx;selx=tmp;}
			if(Selend.x!=Cursor.x){
				KText.Remove(curx,selx-curx);
				DeleteStyles(curx, selx);
			}
			KText.insert(curx, whatpaste);
			
			MoveStyles(curx, whatpaste.Len() - (selx - curx));
			modified=true;
			CalcWrap();
			int whre = curx + whatpaste.Len();
			SetSelection(whre, whre);

		}
		wxTheClipboard->Close();
	}
}

wxPoint KaiTextCtrl::PosFromCursor(wxPoint cur, bool correctToScroll)
{
	int fw, fh;
	if(cur.x<=0||cur.y<0){return wxPoint(-scPos+2, (Fheight-scPos));}
	if(wraps.size()<2 || wraps[cur.y]==cur.x){fw=0;}
	else{
		wxString beforeCursor = KText.SubString(wraps[cur.y],cur.x-1);
		GetTextExtent(beforeCursor, &fw, &fh);
	}
	wxPoint result;
	//TODO: test if it need cur.y+1
	result.x=fw + positioning[cur.y+1];
	result.y=(cur.y+1)*Fheight;
	result.y += posY;
	
	if(style & wxTE_MULTILINE){result.y -= scPos;}
	else{result.x -= scPos;}
	
	return result;
}

void KaiTextCtrl::OnScroll(wxScrollWinEvent& event)
{
	int newPos=0;
	int tsize= wraps.size()-1;
	if(event.GetEventType()==wxEVT_SCROLLWIN_LINEUP)
	{
		newPos=scPos-10;
		if(newPos<0){newPos=0;return;}
	}
	else if(event.GetEventType()==wxEVT_SCROLLWIN_LINEDOWN)
	{
		newPos=scPos+10;
	}
	else if(event.GetEventType()==wxEVT_SCROLLWIN_PAGEUP)
	{
		wxSize size=GetClientSize();
		newPos=scPos;
		newPos-=(size.y/Fheight - 1);
		newPos=MAX(0,newPos);
	}
	else if(event.GetEventType()==wxEVT_SCROLLWIN_PAGEDOWN)
	{
		wxSize size=GetClientSize();
		newPos=scPos;
		newPos+=(size.y/Fheight - 1);
		newPos=MIN(newPos,tsize-1);
	}
	else{newPos = event.GetPosition();}
	if (scPos != newPos) {
		scPos = newPos;
		Refresh(false);
	}
}

void KaiTextCtrl::GetTextExtent(const wxString &textToMesure, int *textWidth, int *textHeight, bool correct/*=false*/)
{
	wxString txt = textToMesure;
	txt.Replace("\r", "");
	txt.Replace("\n", "");
	txt.Replace("\t", "        ");
	wxWindow::GetTextExtent(txt, textWidth, textHeight, 0, 0, &font);

	//if(correct && (textToMesure[0]=='\\' || textToMesure[0]=='j' || textToMesure[0]=='T' 
	//	|| textToMesure[0]=='Ł' || textToMesure[0]=='Y')){
	//		(*textWidth)--;
	//		//if(textToMesure[0]=='j' && textFont &&textFont->GetFaceName()=="Tahoma"){(*textWidth)--;}
	//}
}

void KaiTextCtrl::SetWindowStyle(long _style){
	//if(!(style & wxTE_MULTILINE)){style|=wxALIGN_CENTER_VERTICAL;}
	style |= _style;
	wxWindow::SetWindowStyle(_style);
	scPos=0;
	CalcWrap();
	//SendEvent();
	Refresh(false);
}

void KaiTextCtrl::MakeCursorVisible(bool refreshit)
{
	wxSize size = GetClientSize();
	wxPoint pixelPos = PosFromCursor(Cursor);
	long multiline = style & wxTE_MULTILINE; 
	if(!multiline){
		int moveFactor = size.x/5;
		if(pixelPos.x < 5){
			scPos -= (pixelPos.x > -moveFactor)? moveFactor : abs(pixelPos.x)+moveFactor;
			if(scPos<0){scPos=0;}
		}else if (pixelPos.x > size.x-6) {
			
			int fh, fw;
			GetTextExtent(KText, &fw, &fh);
			fw -= (size.x - 10);
			if(fw>=0){
				int moving = pixelPos.x - (size.x - 10);
				scPos += (moving < moveFactor)? moveFactor : moving+moveFactor;
				if(scPos>fw){
					scPos=fw;
				}
			}
		}
	}else{
		if(pixelPos.y < 3){
			scPos -= (pixelPos.y > -Fheight)? Fheight : (abs(pixelPos.y)+10);
			scPos = ((scPos / Fheight)*Fheight)- Fheight;
			if(scPos<0){scPos=0;}
		}else if(pixelPos.y > size.y-4){
			int bitmaph= (wraps.size()*Fheight)+4;
			int moving = pixelPos.y - (size.y - 10);
			scPos += (moving < Fheight)? Fheight : moving+Fheight;
			scPos = ((scPos / Fheight)*Fheight)/* + Fheight*/;
			if(scPos>bitmaph){scPos=bitmaph;}
		}
	}
	Refresh(false);
}

bool KaiTextCtrl::Enable(bool enable)
{
	wxWindow::Enable(enable);
	Refresh(false);
	//Update();
	return true;
}

//void KaiTextCtrl::SendEvent()
//{
//	
//}

wxIMPLEMENT_ABSTRACT_CLASS(KaiTextCtrl, wxWindow);

BEGIN_EVENT_TABLE(KaiTextCtrl,wxWindow)
	EVT_PAINT(KaiTextCtrl::OnPaint)
	EVT_SIZE(KaiTextCtrl::OnSize)
	EVT_ERASE_BACKGROUND(KaiTextCtrl::OnEraseBackground)
	//EVT_MOUSE_EVENTS(KaiTextCtrl::OnMouseEvent)
	EVT_CHAR(KaiTextCtrl::OnCharPress)
	//EVT_KEY_DOWN(KaiTextCtrl::OnKeyPress)
	EVT_KILL_FOCUS(KaiTextCtrl::OnKillFocus)
	EVT_SET_FOCUS(KaiTextCtrl::OnKillFocus)
	EVT_SCROLLWIN(KaiTextCtrl::OnScroll)
	EVT_MOUSE_CAPTURE_LOST(KaiTextCtrl::OnLostCapture)
END_EVENT_TABLE()

