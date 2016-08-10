#include "MyTextEditor.h"
#include "EditBox.h"
#include "kainoteMain.h"
#include "config.h"
#include <wx/regex.h>
#include <wx/clipbrd.h>
//#include <wx/graphics.h>
wxDEFINE_EVENT(CURSOR_MOVED, wxCommandEvent);

class listwindow : public wxDialog
{
	public:
		listwindow(wxWindow *parent, wxArrayString suggest, const wxPoint& pos, wxString name="Sugestie");
		virtual ~listwindow();

		wxListBox *disperrs;
		void OnDoubleClick(wxCommandEvent& event);
};

listwindow::listwindow(wxWindow *parent, wxArrayString suggest, const wxPoint& pos, wxString name)
	: wxDialog(parent,-1,name,pos)
{
	wxBoxSizer *sizer=new wxBoxSizer(wxHORIZONTAL);
	disperrs=new wxListBox(this,19887,wxDefaultPosition,wxDefaultSize,suggest);
	Connect(19887,wxEVT_COMMAND_LISTBOX_DOUBLECLICKED,(wxObjectEventFunction)&listwindow::OnDoubleClick);
	sizer->Add(disperrs,1,wxEXPAND|wxALL,2);
	sizer->SetMinSize(100,100);
	SetSizerAndFit(sizer);
}
listwindow::~listwindow()
{
}

void listwindow::OnDoubleClick(wxCommandEvent& event)
{
	EndModal(wxID_OK);
}


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
	wxAcceleratorEntry entries[29];
	entries[0].Set(wxACCEL_NORMAL, WXK_DELETE,ID_DEL);
	entries[1].Set(wxACCEL_NORMAL, WXK_BACK,ID_BACK);
	entries[2].Set(wxACCEL_CTRL, WXK_BACK,ID_CBACK);
	entries[3].Set(wxACCEL_NORMAL, WXK_LEFT,ID_LEFT);
	entries[4].Set(wxACCEL_NORMAL, WXK_RIGHT,ID_RIGHT);
	entries[5].Set(wxACCEL_NORMAL, WXK_UP,ID_UP);
	entries[6].Set(wxACCEL_NORMAL, WXK_DOWN,ID_DOWN);
	entries[7].Set(wxACCEL_NORMAL, WXK_HOME,ID_HOME);
	entries[8].Set(wxACCEL_NORMAL, WXK_END,ID_END);
	entries[9].Set(wxACCEL_CTRL, WXK_LEFT,ID_CLEFT);
	entries[10].Set(wxACCEL_CTRL, WXK_RIGHT,ID_CRIGHT);
	entries[11].Set(wxACCEL_SHIFT, WXK_LEFT,ID_SLEFT);
	entries[12].Set(wxACCEL_SHIFT, WXK_RIGHT,ID_SRIGHT);
	entries[13].Set(wxACCEL_SHIFT, WXK_UP,ID_SUP);
	entries[14].Set(wxACCEL_SHIFT, WXK_DOWN,ID_SDOWN);
	entries[15].Set(wxACCEL_SHIFT|wxACCEL_CTRL, WXK_LEFT,ID_CSLEFT);
	entries[16].Set(wxACCEL_SHIFT|wxACCEL_CTRL, WXK_RIGHT,ID_CSRIGHT);
	entries[17].Set(wxACCEL_SHIFT, WXK_HOME,ID_SHOME);
	entries[18].Set(wxACCEL_SHIFT, WXK_END,ID_SEND);
	entries[19].Set(wxACCEL_CTRL, 'A',ID_CTLA);
	entries[20].Set(wxACCEL_CTRL, 'V',ID_CTLV);
	entries[21].Set(wxACCEL_CTRL, 'C',ID_CTLC);
	entries[22].Set(wxACCEL_CTRL, 'X',ID_CTLX);
	entries[23].Set(wxACCEL_NORMAL, 393,ID_WMENU);
	entries[24].Set(wxACCEL_NORMAL, 394,ID_WMENU);
	entries[25].Set(wxACCEL_NORMAL, 395,ID_WMENU);
	entries[26].Set(wxACCEL_NORMAL, WXK_PAGEDOWN,ID_PDOWN);
	entries[27].Set(wxACCEL_NORMAL, WXK_PAGEUP,ID_PUP);
	entries[28].Set(wxACCEL_SHIFT|wxACCEL_CTRL, WXK_END,ID_CSEND);
    wxAcceleratorTable accel(29, entries);
    SetAcceleratorTable(accel);
	Connect(ID_DEL,ID_PUP,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&MTextEditor::OnAccelerator);


	Cursor.x=Cursor.y=Selend.x=Selend.y = oldstart = oldend=0;
	
	holding = dholding = firstdhold = modified = false;

	font= wxFont(10,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,"Tahoma",wxFONTENCODING_DEFAULT);
	wxClientDC dc(this);
	dc.SetFont(font);
	int fw,fh;
	dc.GetTextExtent("#TWFfGH", &fw, &fh, NULL, NULL, &font);
	Fheight=fh;
	scroll=new wxScrollBar(this,3333,wxDefaultPosition,wxDefaultSize, wxSB_VERTICAL);
	scroll->SetCursor(wxCURSOR_DEFAULT);
	caret= new wxCaret(this, 1, Fheight);
	SetCaret(caret);
	caret->Move(3,1);
	caret->Show();
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
	if(resetsel){SetSelection(0,0);}//else{Refresh(false);}
}

void MTextEditor::CalcWrap(bool updatechars, bool sendevent)
{
	//Wrapped=MText;
	wraps.clear();
	wraps.Add(0);
	if(MText!=""){
		int w,h,fw,fh;
		GetClientSize(&w,&h);
		if(scroll->IsShown()){
			int sw, sh;
			scroll->GetSize(&sw,&sh);
			w-=sw;
		}
		if(!bmp){bmp = new wxBitmap(w,h);}
		wxMemoryDC dc;
		dc.SelectObject(*bmp);
		dc.SetFont(font);
		int podz=0;
		wxString wrapchars=" \\,;:}{()";
		size_t i = 0;
		int nwrap=-1;
		int allwrap=-1;
		while(i<MText.Len())
		{
			wxString wrap=MText.SubString(podz,i);
			dc.GetTextExtent(wrap, &fw, &fh, NULL, NULL, &font);
			//wxLogStatus("wrap %s fw %i w %i\r\n", wrap.data(), fw, w);
			if(fw<w-7){
				allwrap=i;
				if(wrapchars.Find(MText[i])!=-1){
					nwrap=i;
					if(MText[i]==' '){nwrap++;}
				}
			}
			else{
				int wwrap=(nwrap!=-1)? nwrap : allwrap+1;
				wraps.Add(wwrap);podz=wwrap;nwrap=-1;allwrap=-1;
			}
			i++;
		}
	}
	wraps.Add(MText.Len());
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
	if(!(event.ControlDown() && !event.AltDown()) && (key>30 || key == 0)){event.Skip();}
}

void MTextEditor::OnAccelerator(wxCommandEvent& event)
{
	int step=0;
	int len;
	int ID=event.GetId();
	switch(ID){
		case ID_CBACK:
			if(Cursor.x==0){return;}
			Selend.x=Cursor.x;
			FindWord((Cursor.x<2)? 0 : Cursor.x-1 ,&Cursor.x,&len);
			if(Cursor.x==1 && MText[0]==' '){Cursor.x--;}
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
			//wxLogStatus("cursorx %i wraps %i",Cursor.x, wraps[Cursor.y]);
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
		
		if(leftup && (holding||dholding)){
			holding=dholding=false;
			ReleaseMouse();
			return;
		}

		

		if(event.LeftDClick()){
			wxPoint mpos=event.GetPosition();
			int errn=FindError(mpos);
			if(Options.GetBool("Editbox Sugestions On Dclick") && errn>=0){
				wxString err=errs[errn];
	
				wxArrayString suggs= Kai->SC->Suggestions(err);
				wxPoint mst=wxGetMousePosition();
				mst.x-=50;mst.y+=15;
	
				listwindow lw(NULL,suggs,mst);
				if(lw.ShowModal()==wxID_OK)
				{
					int from=errors[errn*2];
					int to=errors[(errn*2)+1];
					wxString txt=lw.disperrs->GetString(lw.disperrs->GetSelection());
					MText.replace(from,to-from,txt);
					modified=true;
					CalcWrap();
					if(spell){CheckText();}
					int newto=from+txt.Len();
					SetSelection(newto,newto);
					EB->Send(false);
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
			Refresh(false);
			holding=true;
			CaptureMouse();
		}

		if(holding){
			wxPoint cur;
			HitTest(event.GetPosition(),&cur);
			Cursor=cur;
			Refresh(false);
		}
		if(dholding){
			wxPoint cur;
			int start, end;
			HitTest(event.GetPosition(),&cur);
			FindWord(cur.x,&start,&end);
			if((start==tmpstart && end==tmpend)){return;}
			tmpstart=start;tmpend=end;
			//wxLogStatus("pos %i %i %i %i", start, end, oldstart, oldend);
			
			if(start<oldstart){
				if(end==oldstart){Selend.x=oldend;Selend.y=FindY(oldend);}
				//wxLogStatus("mn%i %i", Selend.x, start);
				Cursor.x=start;
				Cursor.y=FindY(start);
			}else{
				if(oldstart==start){Selend.x=oldstart;Selend.y=FindY(oldstart);}
				//wxLogStatus("wi%i %i", Selend.x, end);
				Cursor.x=end;
				Cursor.y=FindY(end);
			}
			Refresh(false);
			
		}

		if(event.RightDown())
		{
			wxPoint pos=event.GetPosition();
			ContextMenu(pos,FindError(pos));
		}

		if (event.GetWheelRotation() != 0 && event.ControlDown()) {
			fsize += event.GetWheelRotation() / event.GetWheelDelta();
			if(fsize<7||fsize>70){fsize = MID(7,fsize,70); return;}
			font.SetPointSize(fsize);
			wxClientDC dc(this);
			dc.SetFont(font);
			int fw,fh;
			dc.GetTextExtent("#TWFfGH", &fw, &fh, NULL, NULL, &font);
			Fheight=fh;
			caret->SetSize(1,fh);
			CalcWrap(false, false);
			Refresh(false);
		}
	}

void MTextEditor::OnSize(wxSizeEvent& event)
{
	CalcWrap(true,false);
	Cursor.x=0;
	Cursor.y=0;
	Selend=Cursor;
		
	Refresh(false);
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
	if(w==0||h==0){return;}
	//block=true;
	wxPaintDC dc(this);
	int bitmaph= (wraps.size()*Fheight)+2;
	if(bitmaph>h){
		if(!scroll->IsShown()){
			scroll->Show();
			CalcWrap(false, false);
			bitmaph= (wraps.size()*Fheight)+2;
		}
		int sw=0,sh=0;
		scroll->GetSize(&sw,&sh);
        scroll->SetSize(w-sw,0,sw,h);
		int diff= h/2;
		int diff2= bitmaph/2;
		if(scPos>diff2-diff){scPos=diff2-diff;}
		scroll->SetScrollbar(scPos,diff,diff2,diff-1);
        w -= sw;
	}else
	{
		if(scroll->IsShown()){
			scroll->Hide();
			CalcWrap(false,false);
		}
		bitmaph=h;
		scPos=0;
	}
	//wxLogStatus("%i %i %i",scPos,h,bitmaph);
	
	
		
	bool direct = false;

	if (direct) {
		DrawFld(dc,w,h);
	}

	else {
		// Get size and pos
		//scrollBar->GetSize().GetWidth();
		
		// Prepare bitmap
		if (bmp) {
			if (bmp->GetWidth() < w || bmp->GetHeight() < bitmaph) {
				delete bmp;
				bmp = NULL;
			}
		}
		
		if (!bmp) bmp = new wxBitmap(w,bitmaph);

		// Draw bitmap
		wxMemoryDC bmpDC;
		
		bmpDC.SelectObject(*bmp);
		
		DrawFld(bmpDC,w,bitmaph);
		
		
		
		dc.Blit(0,-scPos,w,h+scPos,&bmpDC,0,scPos);
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.SetPen(wxPen((HasFocus())? wxColour("#3D7BAD") : wxColour("#ABADB3")/*wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT)*/));
		dc.DrawRectangle(0,0,w,h);
		dc.SetPen(wxPen("#FFFFFF"));
		dc.DrawRectangle(1,1,w-2,h-2);
		
	}
	
	//block=false;
}

void MTextEditor::DrawFld(wxDC &dc,int w, int h)
	{
	int fw=0,fh=0;
	
	

	//dc.SetPen(wxPen((HasFocus())? "#FFFFFF" : "#A4C9E3"));
	//dc.SetPen(wxPen((HasFocus())? wxColour("#3D7BAD") : wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT)));
	dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW)));
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.DrawRectangle(0,0,w,h);
	dc.SetPen(wxPen("#000000"));
	
	//if(MText==""){return;}

	bool tagi=false;
	bool slash=false;
	bool val=false;
	wxString znaki="(0123456789-&+";
	wxString cyfry="-0123456789ABCDEFabcdef.";
	wxString tagtest="";
	wxString parttext="";
	wxString mestext="";
	
	//Contsel=false;
	posY=1;
	bool isfirst=true;
	int wline=0;
	int wchar=0;

	dc.SetFont(font);
	wxString alltext=MText+" ";
	int len=alltext.Len();
	wxUniChar bchar=alltext[Cursor.x];
	if(bchar=='{')
	{
		Brackets.x=Cursor.x;
		Brackets.y=FindBracket('{','}',Cursor.x+1);//MText.find('}',Cursor.x);
	}else if(bchar=='}')
	{
		Brackets.y=Cursor.x;
		Brackets.x=FindBracket('}','{',Cursor.x-1,true);//MText.SubString(0,Cursor.x).Find('{',true);
	}else if(bchar=='(')
	{
		Brackets.x=Cursor.x;
		Brackets.y=FindBracket('(',')',Cursor.x+1);//MText.find(')',Cursor.x);
	}else if(bchar==')')
	{
		Brackets.y=Cursor.x;
		Brackets.x=FindBracket(')','(',Cursor.x-1,true);//MText.SubString(0,Cursor.x).Find('(',true);
	}else{Brackets.x=-1;Brackets.y=-1;}
	
	int fww, fwww;
	dc.SetPen(*wxTRANSPARENT_PEN);
	//rysowanie spellcheckera
	if(spell){
		int spelllen=errors.size();
	
		dc.SetBrush(Options.GetColour("Grid Spellchecker"));
		for(int g=0;g<spelllen; g+=2)
		{
			int fsty=FindY(errors[g]);
			wxString ftext=MText.SubString(wraps[fsty],errors[g]-1);
			if(wraps[fsty]>=errors[g]){fw=0;}
			else{dc.GetTextExtent(ftext, &fw, &fh, NULL, NULL, &font);}
			int scndy=FindY(errors[g+1]-1);
			wxString etext=MText.SubString(errors[g], (fsty==scndy)?errors[g+1]-1 : wraps[fsty+1]-1);
			dc.GetTextExtent(etext, &fww, &fh, NULL, NULL, &font);
			for(int q=fsty+1; q<=scndy; q++){
				int rest= (q==scndy)? errors[g+1]-1 : wraps[q+1]-1;
				//wxLogStatus("wraps[q-1] %i, rest %i %i %i %i", wraps[q], rest, q, scndy, errors[g+1]-1);
				//if(wraps[q]>rest){continue;}
				wxString btext=MText.SubString(wraps[q], rest);
				dc.GetTextExtent(btext, &fwww, &fh, NULL, NULL, &font);
				dc.DrawRectangle(3,(q*Fheight)+1,fwww,Fheight);
			}
				dc.DrawRectangle(fw+3,(fsty*Fheight)+1,fww,Fheight);
		}
	}

	if(Cursor.x!=Selend.x || Cursor.y != Selend.y){
		Brackets.x=-1;Brackets.y=-1;
		wxPoint fst, scd;
		if((Cursor.x + Cursor.y) > (Selend.x + Selend.y)){fst=Selend;scd=Cursor;}
		else{fst=Cursor, scd=Selend;}
	
		dc.SetBrush(wxBrush(wxColour((HasFocus())? "#00CCFF": "#CCCCFF")));
		fww=0;
		//rysowanie zaznaczenia
		for(int j=fst.y; j<=scd.y; j++){
		
			if(j==fst.y){
				wxString ftext=MText.SubString(wraps[j],fst.x-1);
				if(wraps[j]>fst.x-1){fw=0;}
				else{dc.GetTextExtent(ftext, &fw, &fh, NULL, NULL, &font);}
				wxString stext=MText.SubString(fst.x,(fst.y==scd.y)? scd.x-1 : wraps[j+1]-1);
			dc.GetTextExtent(stext, &fww, &fh, NULL, NULL, &font);
			//wxLogMessage("fww %i, %i", fww, fw);
			//wxLogStatus("%i %i"+stext+" "+ftext, scd.x+wraps[j]-1, wraps[j+1]);
			}
			else if(j==scd.y){
				fw=0;
				dc.GetTextExtent(MText.SubString(wraps[j],scd.x-1), &fww, &fh, NULL, NULL, &font);
			}
			else{
				fw=0;
				dc.GetTextExtent(MText.SubString(wraps[j],wraps[j+1]-1), &fww, &fh, NULL, NULL, &font);
			}
			dc.DrawRectangle(fw+3,(j*Fheight)+1,fww,Fheight);
			//if(j==scd.y)break;
		}
	}
	//rysowanie liter
	for(int i=0;i<len;i++){
		wxUniChar ch=alltext.GetChar(i);

		
		
		if(i==wraps[wline+1]){
			//if(i==MText.Len()-1){parttext<<ch;}
			if(Cursor.x+Cursor.y==wchar){
				int fww=0;
				dc.GetTextExtent(mestext+parttext, &fww, &fh, NULL, NULL, &font);
				caret->Move(fww+3,posY-(scPos*2));

			}
			
			if(parttext!=""){
				dc.GetTextExtent(mestext, &fw, &fh, NULL, NULL, &font);
				wxColour kol=(val)? "#6600FF" : (slash)? "#850085" : "#000000";
				dc.SetTextForeground(kol);
				if(isfirst && (parttext.StartsWith(L"T") || parttext.StartsWith(L"Y") || parttext.StartsWith(L"Ł"))){fw++;isfirst=false;}
				mestext<<parttext;
				dc.DrawText(parttext,fw+3,posY);
			}
			
			//posX=4;
			posY+=Fheight;
			wline++;
			wchar++;
			parttext="";
			//wchar=0;
			mestext="";
		}

		if(HasFocus()&&(Cursor.x+Cursor.y==wchar)){
			if(mestext+parttext==""){fw=0;}
			else{dc.GetTextExtent(mestext+parttext, &fw, &fh, NULL, NULL, &font);}
			//wxLogStatus("cursor %i %i %i",posY-(scPos*2), scPos, Fheight);
			caret->Move(fw+3,posY-(scPos*2));
		
		}
		parttext<<ch;
		//mestext<<ch;
		
		

		if(ch=='{'||ch=='}'){
			if(ch=='{'){
				tagi=true;
				wxString bef=parttext.BeforeLast('{');
				dc.GetTextExtent(mestext, &fw, &fh, NULL, NULL, &font);
				dc.SetTextForeground(*wxBLACK);
				if(isfirst && (bef.StartsWith("T") || bef.StartsWith("Y") || bef.StartsWith(L"Ł"))){fw++;isfirst=false;}
				dc.DrawText(bef,fw+3,posY);
				//posX+=fw;
				mestext<<bef;
				parttext="{";
			}
			else{
				//if(val){
				wxString tmp=parttext.RemoveLast(1);
				dc.GetTextExtent(mestext, &fw, &fh, NULL, NULL, &font);
				dc.SetTextForeground(wxColour((val)? "#6600FF" : "#000000" ));
				if(tmp.StartsWith("T") || tmp.StartsWith("Y") || tmp.StartsWith(L"Ł")){fw--;}
				dc.DrawText(tmp,fw+3,posY);
				//posX+=fw;
				mestext<<tmp;
				parttext="}";
				//}
				tagi=slash=val=false;
			}
			dc.GetTextExtent(mestext, &fw, &fh, NULL, NULL, &font);
			dc.SetTextForeground(*wxBLUE);
			dc.DrawText(parttext,fw+3,posY);
			mestext<<parttext;
			//posX+=fw;
			parttext="";
			val=false;
			isfirst=false;
		}
		
		if(slash){
			tagtest +=ch;
			if((znaki.Find(ch)!=-1&&tagtest!="1"&&tagtest!="2"&&tagtest!="3"&&tagtest!="4")||tagtest=="fn"||ch=='('){
				slash=false;
				wxString tmp=(tagtest=="fn")? parttext : parttext.RemoveLast(1);
				dc.GetTextExtent(mestext, &fw, &fh, NULL, NULL, &font);
				dc.SetTextForeground(wxColour("#850085"));
				dc.DrawText(tmp,fw+2,posY);
				//posX+=fw;
				mestext<<tmp;
				if(tagtest=="fn"){parttext="";}
				else{parttext=ch;}
				val=true;
				tagtest="";
			}
		}

		if((ch=='\\'||ch=='('||ch==')'||ch==',')&&tagi){
			if(ch=='\\'){slash=true;}
			if(val&&(ch=='\\'||ch==')'||ch==',')){
				wxString tmp=parttext.RemoveLast(1);
				dc.GetTextExtent(mestext, &fw, &fh, NULL, NULL, &font);
				dc.SetTextForeground(wxColour("#6600FF"));
				dc.DrawText(tmp,fw+3,posY);
				//posX+=fw;
				mestext<<tmp;
				parttext=ch;
				if(ch!=','){val=false;}
		
			}
			if(ch=='('){val=true;slash=false;}
			dc.GetTextExtent(mestext, &fw, &fh, NULL, NULL, &font);
			dc.SetTextForeground(wxColour("#FF0000"));
			dc.DrawText(parttext,fw+3,posY);
			//posX+=fw;
			mestext<<parttext;
			parttext="";
			//continue;
		}
		if(HasFocus()&&(i==Brackets.x||i==Brackets.y)){
			int bry=FindY(i);
			wxColour col("#FFFF00");
			if(Brackets.x==-1||Brackets.y==-1){col=Options.GetColour("Grid Spellchecker");}
			dc.SetBrush(wxBrush(col));
			//dc.SetPen(wxPen(col));
			if(i>0){dc.GetTextExtent(MText.SubString(wraps[bry],i-1), &fw, &fh, NULL, NULL, &font);}else{fw=0;}
			dc.GetTextExtent(MText[i], &fww, &fh, NULL, NULL, &font);
			dc.DrawRectangle(fw+3,(bry*Fheight)+1,fww,Fheight);
			wxFont fnt=dc.GetFont();
			fnt=fnt.Bold();
			dc.SetFont(fnt);
			dc.DrawText(MText[i],fw+3,(bry*Fheight)+1);
			dc.SetFont(font);
			//wxLogStatus("br x i y %i, %i", Brackets.x,Brackets.y);
		}
		
		wchar++;
	}
	
		
}

bool MTextEditor::HitTest(wxPoint pos, wxPoint *cur)
	{//~~~~
		int w,h,fw=0,fh=0;
	GetClientSize(&w,&h);
	wxMemoryDC dc;
	dc.SelectObject(*bmp);
	dc.SetFont(font);
	pos.y+=(scPos*2);
	pos.x-=2;
	
	cur->y=(pos.y/Fheight);
	if(cur->y<0||wraps.size()<2){cur->y=0;cur->x=0;return false;}
	if(cur->y>=(int)wraps.size()-1)
	{cur->y=wraps.size()-2;cur->x=wraps[wraps.size()-2];}//wxLogMessage("line %i %i",cur->y, cur->x);
	else{cur->x=wraps[cur->y];}
	
	bool find=false;
	wxString txt=MText+" ";

	int wlen=MText.Len();
	int fww;
		for(int i = cur->x;i<wraps[cur->y+1]+1;i++)
		{
			
			dc.GetTextExtent(txt.SubString(cur->x,i), &fw, &fh, NULL, NULL, &font);
			dc.GetTextExtent(txt[i], &fww, &fh, NULL, NULL, &font);
			if(fw+1-(fww/2)>pos.x){cur->x=i; find=true;break;}
			//apos+=fw;wxLogStatus("%i, %i", cur->x, cur->y);
		}
		if(!find){cur->x = wraps[cur->y+1];}
		//wxLogStatus("%i %i", cur->x, cur->y);
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
	if(!Kai->SC){spell=Kai->SpellcheckerOn();}
	if(Kai->SC){
		wxString notchar="/?<>|\\!@#$%^&*()_+=[]\t~ :;.,\"{}";
		wxString text=MText;
		errors.clear();
		errs.Clear();
		text+=" ";
		bool block=false;
		wxString word="";
		bool slash=false;
		int lasti=0;
		int firsti=0;
		for(size_t i = 0; i<text.Len();i++)
		{
			wxUniChar ch=text.GetChar(i);
			if(notchar.Find(ch)!=-1&&!block){
				if(word.Len()>1){
					if(word.StartsWith("'")){word=word.Remove(0,1);}
					if(word.EndsWith("'")){word=word.RemoveLast(1);}
					word.Trim(false);word.Trim(true);bool isgood=Kai->SC->CheckWord(word);
					if (!isgood){errs.Add(word);errors.push_back(firsti);errors.push_back(lasti+1);}
				}
				word="";firsti=i+1;
			}
			if(ch=='{'){block=true;}
			else if(ch=='}'){block=false;firsti=i+1;word="";}
			
			if(notchar.Find(ch)==-1&&text.GetChar((i==0)? 0 : i-1)!='\\'&&!block){word<<ch;lasti=i;}
			else if(!block&&text.GetChar((i==0)? 0 : i-1)=='\\'&& (ch=='N'||ch=='h')){firsti=i+1;word="";}
		}
		//Refresh(false);
	}
}

void MTextEditor::OnKillFocus(wxFocusEvent& event)
{
	Refresh(false);
}

void MTextEditor::FindWord(int pos, int *start, int *end)
{
	wxString wfind=" }])-—'`\"\\;:,.({[><?!*~@#$%^&/+=";
	int len=MText.Len();
	bool fromend=(start!=NULL);
	
	if(!fromend){pos--;}
	pos=MID(0,pos,len-1);
	//wxString ttt="tekst:";
	//wxLogStatus(ttt<<MText[pos]);
	bool hasres=false;
	int lastres=-1;
	if(fromend){
		*start=(fromend)? 0 : len;
		for(int i=pos; i>=0; i--){
			int res=wfind.Find(MText[i]);
			if(res!=-1){lastres=res;}
			if(res!=-1&&!hasres){
				if(i==pos){hasres=true;continue;}
				bool isen=(MText[i]=='\\' && MText[i+1]=='N');
				*start=(isen && pos==i+1)? i : (isen)? i+2 : i+1;
				break;
			}else if(hasres&&res==-1){
				if(i+1==pos){continue;}
				else if(lastres<1 && i+2==pos){hasres=false; continue;}
				*start=(lastres>0 && (MText[pos]==' '||i+2<pos))?i+1 : i+2;
				break;
			}
		}
	}
	if(!end){return;}
	*end=(fromend && end==NULL)? 0 : len;
	for(int i=pos; i<len; i++){
		int res=wfind.Find(MText[i]);
		if(res!=-1&&!hasres){
			//wxLogStatus("ipos2 %i %i",i, pos);
			if(i==pos){hasres=true;continue;}
			*end=(res<1)? i+1 : i;
			break;
		}else if(hasres&&res==-1){
			//wxLogStatus("ipos3 %i %i",i, pos);
			*end= (i>0 && MText[i-1]=='\\' && MText[i]=='N')? i+1 : i;
			break;
		}
	}
	
	
}

void MTextEditor::ContextMenu(wxPoint mpos, int error)
{
	wxMenu menut;
	wxString err;
	wxArrayString suggs;
	if(error>=0){err=errs[error];}
	if(!err.IsEmpty()){
		suggs= Kai->SC->Suggestions(err);
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

	if(!err.IsEmpty()){
		menut.Append(TEXTM_ADD,_("&Dodaj słowo \"")+err+_("\" do słownika"));
	}
						
	menut.Append(TEXTM_DEL,_("&Usuń"))->Enable(Selend.x!=Cursor.x);
			
	int id=-1;
	id=GetPopupMenuSelectionFromUser(menut,mpos);
			
	if(id>=30200){
		int from=errors[error*2];
		int to=errors[(error*2)+1];
		MText.replace(from,to-from,suggs[id-30200]);
		modified=true;
		CalcWrap();
		if(spell){CheckText();}
		int newto=from+suggs[id-30200].Len();
		SetSelection(newto,newto);
		EB->Send(false);
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
	else if(id==TEXTM_ADD){
		bool succ=Kai->SC->AddWord(err);
		if(!succ){wxMessageBox(_("Błąd, słowo \"")+err+_("\" nie zostało dodane."));}
		else{CheckText();EB->ClearErrs();Refresh(false);}
	}else if(id>=TEXTM_SEEKWORDL && id<=TEXTM_SEEKWORDG){
		wxString page=(id==TEXTM_SEEKWORDL)? L"http://ling.pl/" : 
			(id==TEXTM_SEEKWORDB)? L"http://pl.bab.la/slownik/angielski-polski/" : L"https://www.google.com/search?q=";
		long from, to;
		GetSelection(&from, &to);
		wxString word= MText.SubString(from, to-1).Trim();
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

void MTextEditor::Copy(bool cut)
{
	if(Selend.x==Cursor.x){return;}
	int curx=Cursor.x;int selx=Selend.x;if(curx>selx){int tmp=curx;curx=selx;selx=tmp;}
				
	wxString whatcopy=MText.SubString(curx,selx-1);
	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData( new wxTextDataObject(whatcopy) );
		wxTheClipboard->Close();
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

		//wxLogStatus("ht");
	for(size_t i=0; i<errors.size(); i+=2){
		//wxLogStatus("not found yet%i %i %i",EB->line.spells[i],EB->line.spells[i+1],cpos.x);
		if(cpos.x >= errors[i] && cpos.x <= errors[i+1]){
			//wxLogStatus("found %i",i);
			return i/2;
		}
	}

	
	return -1;
}

wxPoint MTextEditor::PosFromCursor(wxPoint cur)
{
	wxMemoryDC dc;
	dc.SelectObject(*bmp);
	dc.SetFont(font);
	int fw, fh;
	if(wraps[cur.y]==cur.x){fw=0;}
	else{dc.GetTextExtent(MText.SubString(wraps[cur.y],cur.x), &fw, &fh, NULL, NULL, &font);}
	wxPoint result;
	result.x=fw+3;
	result.y=(cur.y+1)*Fheight;
	return result;
}

void MTextEditor::OnScroll(wxScrollEvent& event)
{
	if(scroll->IsShown()){
		int newPos = event.GetPosition();
		//wxLogStatus("%i",newPos);
		if (scPos != newPos) {
			scPos = newPos;
			//wxLogStatus("%i",newPos);
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

//void MTextEditor::OnEraseBackground(wxEraseEvent& event){}
BEGIN_EVENT_TABLE(MTextEditor,wxWindow)
	EVT_PAINT(MTextEditor::OnPaint)
	EVT_SIZE(MTextEditor::OnSize)
	EVT_ERASE_BACKGROUND(MTextEditor::OnEraseBackground)
	EVT_MOUSE_EVENTS(MTextEditor::OnMouseEvent)
	EVT_CHAR(MTextEditor::OnCharPress)
	EVT_KEY_DOWN(MTextEditor::OnKeyPress)
	EVT_KILL_FOCUS(MTextEditor::OnKillFocus)
	EVT_COMMAND_SCROLL(3333,MTextEditor::OnScroll)
END_EVENT_TABLE()