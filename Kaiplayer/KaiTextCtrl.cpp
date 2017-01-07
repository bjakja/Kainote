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
#include "config.h"
#include "Menu.h"
#include <wx/clipbrd.h>
#include <wx/msw/private.h>


KaiTextCtrl::KaiTextCtrl(wxWindow *parent, int id, const wxString &text, const wxPoint& pos,const wxSize& size, long _style, const wxValidator & validator, const wxString & name)
	:KaiScrolledWindow(parent,id,pos,size,_style|wxWANTS_CHARS,name)
{
	KText=text;
	KText.Replace("\r","");
	KText.Replace("\n","");
	bmp=NULL;
	fsize=10;
	posY=0;
	scPos=0;
	style = _style/*|wxWANTS_CHARS*//*|wxTE_MULTILINE|wxALIGN_CENTER*/;
	if(style & wxTE_MULTILINE){maxSize=MAXINT;}else{style|=wxALIGN_CENTER_VERTICAL; maxSize=1000;}
	SetCursor(wxCURSOR_IBEAM);
	wxAcceleratorEntry entries[30];
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
	//entries[23].Set(wxACCEL_NORMAL, 393,ID_TWMENU);
	//entries[24].Set(wxACCEL_NORMAL, 394,ID_TWMENU);
	entries[25].Set(wxACCEL_NORMAL, 395,ID_TWMENU);
	entries[26].Set(wxACCEL_NORMAL, WXK_PAGEDOWN,ID_TPDOWN);
	entries[27].Set(wxACCEL_NORMAL, WXK_PAGEUP,ID_TPUP);
	entries[28].Set(wxACCEL_SHIFT|wxACCEL_CTRL, WXK_END,ID_TCSEND);
	bool processEnter = !(style & wxTE_PROCESS_ENTER) && (style & wxTE_MULTILINE);
	if(processEnter){entries[29].Set(wxACCEL_NORMAL, WXK_RETURN,ID_TRETURN);}
	wxAcceleratorTable accel((processEnter)? 30 : 29, entries);
	SetAcceleratorTable(accel);
	Connect(ID_TDEL,ID_TRETURN,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&KaiTextCtrl::OnAccelerator);


	Cursor.x=Cursor.y=Selend.x=Selend.y = oldstart = oldend=0;

	holding = dholding = firstdhold = modified = false;

	font= parent->GetFont();//wxFont(9,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,"Tahoma",wxFONTENCODING_DEFAULT);
	int fw,fh;
	GetTextExtent("#TWFfGH", &fw, &fh, &font);
	Fheight=fh;
	caret= new wxCaret(this, 1, Fheight);
	SetCaret(caret);
	caret->Move(3,2);
	caret->Show();
	//Refresh(false);
	wxSize newSize((size.x<1)? 100 : size.x, (size.y<1)? fh+10 : size.y);
	SetMinSize(newSize);

	CalcWrap(false);

	//foreground = parent->GetForegroundColour();
	//background = parent->GetBackgroundColour();
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
	/*Bind(wxEVT_SYS_COLOUR_CHANGED, [=](wxSysColourChangedEvent & evt){
		foreground = GetParent()->GetForegroundColour();
		background = GetParent()->GetBackgroundColour();

	});*/
	
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
	CalcWrap(false);
	
	if(newSel){SetSelection(0,0);}
	else{
		if((size_t)Cursor.x>KText.Len()){Cursor.x = KText.Len();Cursor.y = FindY(Cursor.x);}
		Refresh(false);
	}
}
void KaiTextCtrl::AppendText(const wxString &text)
{
	if(KText.Len() >= maxSize){
		return;
	}
	if(KText.Len() + text.Len()> maxSize){
		KText<<text.SubString(0, maxSize - KText.Len());
	}else{
		KText<<text;
	}
	if(!(style & wxTE_MULTILINE)){
		KText.Replace("\r","");
		KText.Replace("\n","");
	}
	
	CalcWrap(false);
	Refresh(false);
}

void KaiTextCtrl::CalcWrap(bool sendevent)
{
	//Wrapped=KText;
	wraps.clear();
	wraps.Add(0);
	positioning.clear();
	positioning.Add(0);
	bool foundN=false;
	int w,h,fw=0,fh=0;
	GetClientSize(&w,&h);
	long multiline = (style & wxTE_MULTILINE);
	if(KText!="" && multiline){
		int podz=0;
		wxString wrapchars=" \\,;:}{()\n\r";
		size_t i = 0;
		int nwrap=-1;
		int allwrap=-1;
		wxString tmptxt = KText+"\n";
		size_t len = tmptxt.Len();
		while(i<len)
		{
			wxString wrap=tmptxt.SubString(podz,i);
			GetTextExtent(wrap, &fw, &fh, &font);
			if(fw<w-7 && !foundN){
				allwrap=i;
				if(wrapchars.Find(tmptxt[i])!=-1){
					nwrap=i;
					if(tmptxt[i]==' ' || tmptxt[i]=='\n'){nwrap++;}
				}
				if(tmptxt[i]=='\n'){foundN=true;}else{i++; continue;}
			}
			if(nwrap < 0 && allwrap <0){i++; continue;}
			size_t wwrap=(nwrap!=-1)? nwrap : allwrap+1;
			wrap=tmptxt.SubString(podz,wwrap-1);
			wrap.Replace("\r","");
			wrap.Replace("\n","");
			GetTextExtent(wrap, &fw, &fh, &font);
			int pos = (style & wxALIGN_CENTER_HORIZONTAL)? ((w - fw)/2) : 
				(style & wxALIGN_RIGHT)? (w - fw)-5 : 5;
			positioning.Add(pos);
			wraps.Add((wwrap<len)? wwrap : len-1);
			podz=wwrap;
			nwrap=-1;
			allwrap=-1;
			foundN=false;

			i++;
		}
	}else{
		wraps.Add(KText.Len());
		GetTextExtent(KText, &fw, &fh, &font);
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
	if(wkey=='\t' || (style & wxTE_READONLY)){return;}
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
		CalcWrap();
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
			Selend=Cursor;
			CalcWrap();
			SetSelection(curx,curx);
		}else
		{
			if(ID==ID_TBACK){
				if(Cursor.x<1){return;} 
				Cursor.x--;
			}
			if(ID==ID_TDEL && Cursor.x >= (int)KText.Len()){return;}
			KText.Remove(Cursor.x,1);
		}
		len=wraps.size();
		CalcWrap();
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
		//wxLogStatus("%i %i",Cursor.x, Cursor.y);


		if(ID<ID_TSDOWN){Selend=Cursor;}
		//Refresh(false);
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

		//wxLogStatus("%i %i",Cursor.x, Cursor.y);
		if(ID<ID_TSUP){Selend=Cursor;}
		//Refresh(false);
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
		//Refresh(false);
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
		Cursor.x++;Cursor.y++;
		Selend=Cursor;
		CalcWrap();
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
	CalcWrap(false);
	//Cursor.x=0;
	//Cursor.y=0;
	//Selend=Cursor;
	Cursor.y = FindY(Cursor.x);
	Selend.y = FindY(Selend.x);
	//Refresh(false);
	MakeCursorVisible();
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
	//if(block){return;} 
	int w = 0, h = 0;
	GetClientSize(&w,&h);
	if(w==0||h==0){return;}
	//block=true;
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
	bool direct = false;

	if (direct) {
		DrawFld(dc,w,h,w,h);
	}

	else {
		// Get size and pos
		//scrollBar->GetSize().GetWidth();

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

	//block=false;
}

void KaiTextCtrl::DrawFld(wxDC &dc,int w, int h, int windoww, int windowh)
{
	int fw=0,fh=0;
	bool enabled = IsThisEnabled();
	wxColour bg = (background.IsOk())? background : Options.GetColour("Window Background");
	wxColour fg = (foreground.IsOk())? foreground : Options.GetColour("Window Text");
	dc.SetFont(font);
	dc.SetBrush(wxBrush((enabled)? bg : 
		Options.GetColour("Window Inactive Background")));
	dc.SetPen(wxPen((style & wxBORDER_NONE)? bg : 
		(HasFocus())? Options.GetColour("Editor Border Focus") : 
		(enabled)? Options.GetColour("Editor Border") : Options.GetColour("Button Inactive Border")));
	dc.DrawRectangle(0,0,w,h);
	if(wraps.size()<2 || positioning.size()<2){return;}
	wxColour cselection = (HasFocus())? Options.GetColour("Editor Selection") : 
		Options.GetColour("Editor Selection No Focus");


	//Contsel=false;
	//posY=2;
	
	int tmpPosY = posY;
	int tmpPosX = (style & wxTE_MULTILINE)? 0 : -scPos;
	if((style & wxTE_MULTILINE)){tmpPosY -=scPos;}
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

			if(j==fst.y){
				wxString ftext=KText.SubString(wraps[j],fst.x-1);
				if(wraps[j]>fst.x-1){fw=0;}
				else{GetTextExtent(ftext, &fw, &fh, &font);}
				wxString stext=KText.SubString(fst.x,(fst.y==scd.y)? scd.x-1 : wraps[j+1]-1);
				stext.Replace("\r", "");
				stext.Replace("\n", "");
				GetTextExtent(stext, &fww, &fh, &font);

			}
			else{
				fw=0;
				wxString selText = KText.SubString(wraps[j], (j==scd.y)? scd.x-1 : wraps[j+1]-1);
				selText.Replace("\r", "");
				selText.Replace("\n", "");
				GetTextExtent(selText, &fww, &fh, &font);
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
			beforeCursor.Replace("\r", "");
			beforeCursor.Replace("\n", "");
			GetTextExtent(beforeCursor, &fww, &fh, &font);
		}
		caret->Move(positioning[cursorI+1] + fww + tmpPosX, tmpPosY + (Fheight * cursorI));

	}
	
	//rysowanie tesktu
	for(size_t i = 1; i< wraps.size(); i++){
		wxString line = KText.SubString(wraps[i-1], wraps[i]-1).Trim().Trim(false);
		/*wxString subline;
		wxString subline1;
		subline1 = line.BeforeFirst('j', &subline);
		fww=0;*/
		/*dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
		dc.DrawText(line,positioning[i]-1,posY-1);
		dc.DrawText(line,positioning[i]-1,posY+1);
		dc.DrawText(line,positioning[i]+1,posY-1);
		dc.DrawText(line,positioning[i]+1,posY+1);*/
		/*if(subline!="" || subline1==""){
		subline = "j"+subline;
		GetTextExtent(subline1, &fww, &fh, &font);
		dc.SetTextForeground(wxSystemSettings::GetColour((enabled)? wxSYS_COLOUR_HIGHLIGHT : wxSYS_COLOUR_GRAYTEXT));
		dc.DrawText(subline,positioning[i]+fww,posY);

		}*/
		dc.SetTextForeground((enabled)? fg : Options.GetColour("Window Inactive Text"));
		dc.DrawText(line,positioning[i]+tmpPosX,tmpPosY);

		tmpPosY+=Fheight;

	}


}

bool KaiTextCtrl::HitTest(wxPoint pos, wxPoint *cur)
{//~~~~
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

		GetTextExtent(txt.SubString(cur->x,i), &fw, &fh, &font);
		GetTextExtent(txt[i], &fww, &fh, &font);
		fw += positioning[cur->y+1];
		if(fw-(fww/2)-1>pos.x){cur->x=i; find=true;break;}
	}
	if(!find){cur->x = wraps[cur->y+1];}
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

void KaiTextCtrl::Replace(int start, int end, wxString rep)
{
	modified=true;
	KText.replace(start, end-start,rep);
	CalcWrap();
	Cursor.x=0;Cursor.y=0;
	Selend=Cursor;
	//Refresh(false);
	MakeCursorVisible();
}

void KaiTextCtrl::OnKillFocus(wxFocusEvent& event)
{
	Refresh(false);
}

void KaiTextCtrl::FindWord(int pos, int *start, int *end)
{
	wxString wfind=" }])-—'`\"\\;:,.({[><?!*~@#$%^&/+=";
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
	menut.Append(TEXT_SEEKWORDL,_("Szukaj t³umaczenia s³owa na ling.pl"))->Enable(Selend.x!=Cursor.x);
	menut.Append(TEXT_SEEKWORDB,_("Szukaj t³umaczenia s³owa na pl.ba.bla"))->Enable(Selend.x!=Cursor.x);
	menut.Append(TEXT_SEEKWORDG,_("Szukaj t³umaczenia s³owa w google"))->Enable(Selend.x!=Cursor.x);

	menut.Append(TEXT_DEL,_("&Usuñ"))->Enable(Selend.x!=Cursor.x);

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
		CalcWrap();
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
		CalcWrap();
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
			whatpaste.Replace("\t"," ");
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
			//whatpaste.Replace("\f","");
			
			int curx=Cursor.x;
			int selx=Selend.x;if(curx>selx){int tmp=curx;curx=selx;selx=tmp;}
			if(Selend.x!=Cursor.x){
				KText.Remove(curx,selx-curx);
			}
			KText.insert(curx,whatpaste);
			modified=true;
			CalcWrap();
			int whre=curx+whatpaste.Len();
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
		beforeCursor.Replace("\n","");
		beforeCursor.Replace("\r","");
		GetTextExtent(beforeCursor, &fw, &fh, &font);
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

void KaiTextCtrl::GetTextExtent(const wxString &textToMesure, int *textWidth, int *textHeight, wxFont *textFont, bool correct)
{
	//wxFont *txtfont=NULL;
	//if(!textFont){txtfont = &font;}
	wxWindow::GetTextExtent(textToMesure, textWidth, textHeight, 0, 0, textFont);
	if(correct && (textToMesure[0]=='\\' || textToMesure[0]=='j' || textToMesure[0]=='T' 
		|| textToMesure[0]=='£' || textToMesure[0]=='Y')){
			(*textWidth)--;
			//if(textToMesure[0]=='j' && textFont &&textFont->GetFaceName()=="Tahoma"){(*textWidth)--;}
	}
}

void KaiTextCtrl::SetWindowStyle(long _style){
	//if(!(style & wxTE_MULTILINE)){style|=wxALIGN_CENTER_VERTICAL;}
	style |= _style;
	wxWindow::SetWindowStyle(_style);
	scPos=0;
	CalcWrap();
	Refresh(false);
}

void KaiTextCtrl::MakeCursorVisible(bool refreshit)
{
	wxSize size = GetClientSize();
	wxPoint pixelPos = PosFromCursor(Cursor);
	//wxLogStatus("cursor %i, %i", Cursor.x, Cursor.y);
	long multiline = style & wxTE_MULTILINE; 
	if(!multiline){
		int moveFactor = size.x/5;
		if(pixelPos.x < 5){
			scPos -= (pixelPos.x > -moveFactor)? moveFactor : abs(pixelPos.x)+moveFactor;
			if(scPos<0){scPos=0;}
		}else if (pixelPos.x > size.x-6) {
			
			int fh, fw;
			GetTextExtent(KText, &fw, &fh, &font);
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
			scPos = ((scPos/Fheight)-Fheight)*Fheight;
			if(scPos<0){scPos=0;}
		}else if(pixelPos.y > size.y-4){
			int bitmaph= (wraps.size()*Fheight)+4;
			int moving = pixelPos.y - (size.y - 10);
			scPos += (moving < Fheight)? Fheight : moving+Fheight;
			scPos = ((scPos/Fheight)+Fheight)*Fheight;
			if(scPos>bitmaph){scPos=bitmaph;}
		}
	}
	//if(!refreshit){return;}
	Refresh(false);
}

wxIMPLEMENT_ABSTRACT_CLASS(KaiTextCtrl, wxWindow);

BEGIN_EVENT_TABLE(KaiTextCtrl,wxWindow)
	EVT_PAINT(KaiTextCtrl::OnPaint)
	EVT_SIZE(KaiTextCtrl::OnSize)
	EVT_ERASE_BACKGROUND(KaiTextCtrl::OnEraseBackground)
	//EVT_MOUSE_EVENTS(KaiTextCtrl::OnMouseEvent)
	EVT_CHAR(KaiTextCtrl::OnCharPress)
	//EVT_KEY_DOWN(KaiTextCtrl::OnKeyPress)
	EVT_KILL_FOCUS(KaiTextCtrl::OnKillFocus)
	EVT_SCROLLWIN(KaiTextCtrl::OnScroll)
	EVT_MOUSE_CAPTURE_LOST(KaiTextCtrl::OnLostCapture)
END_EVENT_TABLE()

