
#include "SubsGrid.h"
#include "config.h"
#include "timeconv.h"
#include "EditBox.h"
#include "kainoteMain.h"
#include "OpennWrite.h"
#include "OptionsDialog.h"
#include "TabDialog.h"
#include <wx/tokenzr.h>
#include <wx/event.h>
#include <algorithm>
#include <wx/regex.h>
#include <wx/ffile.h>
//#include <thread>

bool sortstart(Dialogue *i,Dialogue *j){ 
	if(i->Start.mstime!=j->Start.mstime){
		return (i->Start.mstime<j->Start.mstime);
	}
	return i->End.mstime<j->End.mstime;
}
bool sortend(Dialogue *i,Dialogue *j){
	if(i->End.mstime!=j->End.mstime){
		return (i->End.mstime<j->End.mstime);
	}
	return i->Start.mstime<j->Start.mstime;
}
bool sortstyle(Dialogue *i,Dialogue *j){
	if(i->Style!=j->Style){
		return (i->Style.CmpNoCase(j->Style)<0); 
	}
	return i->Start.mstime<j->Start.mstime;
}
bool sortactor(Dialogue *i,Dialogue *j){
	if(i->Actor!=j->Actor){
		return (i->Actor.CmpNoCase(j->Actor)<0); 
	}
	return i->Start.mstime<j->Start.mstime;
}
bool sorteffect(Dialogue *i,Dialogue *j){ 
	if(i->Effect!=j->Effect){
		return (i->Effect.CmpNoCase(j->Effect)<0); 
	}
	return i->Start.mstime<j->Start.mstime;
}
bool sortlayer(Dialogue *i,Dialogue *j){
	if(i->Layer!=j->Layer){
		return (i->Layer<j->Layer); 
	}
	return i->Start.mstime<j->Start.mstime;
}

bool SubsGrid::IsNum(wxString test) {
	bool isnumber=true;
	wxString testchars="0123456789 ";
	for(size_t i=0;i<test.Len();i++){
		wxUniChar ch=test.GetChar(i);
		if(testchars.Find(ch)==-1){isnumber=false;break;}
	}
	return isnumber;
}



SubsGrid::SubsGrid(wxWindow *parent, const long int id,const wxPoint& pos,const wxSize& size, long style, const wxString& name)
	: wxWindow(parent, id, pos, size, style, name)
{

	posY=0;
	posX=0;
	row=mtimerow=0;
	scPos=0;
	scHor=0;
	oldX=-1;
	visible=Options.GetInt("Grid Hide Collums");
	holding = false;
	hideover=Options.GetBool("Grid Hide Tags");
	Modified=false;
	first=true;
	makebkp=true;
	extendRow=-1;
	lastRow=0;
	lastsel=-1;
	transl=false;
	showtl=false;
	ismenushown=false;
	bmp=NULL;
	numsave=0;
	file=new SubsFile();


	LoadDefault();
	timer.SetOwner(this,ID_AUTIMER);
	SetStyle();
	AdjustWidths();
	SetFocus();
}


SubsGrid::~SubsGrid()
{
	Clearing();
	if(bmp){delete bmp;bmp=NULL;}
}

void SubsGrid::SetStyle() 
{
	wxString fontname = Options.GetString("Grid Font Name");
	font.SetFaceName(fontname);
	if (!font.IsOk())
		font.SetFamily(wxFONTFAMILY_SWISS );
	font.SetWeight(wxFONTWEIGHT_NORMAL);
	font.SetPointSize(Options.GetInt("Grid Font Size"));

	{
		wxClientDC dc(this);
		dc.SetFont(font);
		int fw,fh;
		dc.GetTextExtent("#TWFfGH", &fw, &fh, NULL, NULL, &font);
		GridHeight=fh+2;
	}


	Refresh(false);
}


void SubsGrid::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);
	int w = 0;
	int h = 0;
	int sw=0;
	int sh=0;
	GetClientSize(&w,&h);
	//scrollBar->GetSize(&sw,&sh);
	//scrollBar->SetSize(w-sw,0,sw,h);
	// w -= 18;
	bool direct = false;

	if (direct) {
		DrawGrid(dc,w,h);
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
		DrawGrid(bmpDC,w,h);
		dc.Blit(0,0,w,h,&bmpDC,0,0);
	}

}

void SubsGrid::DrawGrid(wxDC &mdc,int w, int h)
{

	mdc.SetFont(font);
	wxMemoryDC tdc;
	wxBitmap tbmp(w+scHor,h);
	tdc.SelectObject(tbmp);
	tdc.SetFont(font);

	wxColour labelBkCol=Options.GetColour("Grid Label Saved");
	wxColour labelBkColN=Options.GetColour("Grid Label Normal");
	wxColour labelBkColM=Options.GetColour("Grid Label Modified");
	wxColour linesCol=Options.GetColour("Grid Lines");
	wxColour subsBkCol=Options.GetColour("Grid Dialogue");
	wxColour comm=Options.GetColour("Grid Comment");
	wxColour seldial=Options.GetColour("Grid Selected Dialogue");
	wxColour selcom=Options.GetColour("Grid Selected Comment");
	wxColour textcol=Options.GetColour("Grid Text");
	wxColour collcol=Options.GetColour("Grid Collisions");
	wxString chtag=Options.GetString("Grid tag changing char");

	//mdc.SetBackground(wxBrush(linesCol));
	//tdc.SetBackground(wxBrush(linesCol));
	mdc.SetPen(*wxTRANSPARENT_PEN);
	mdc.SetBrush(wxBrush(linesCol));
	mdc.DrawRectangle(0,0,GridWidth[0]+1,h);
	tdc.SetPen(*wxTRANSPARENT_PEN);
	tdc.SetBrush(wxBrush(linesCol));
	tdc.DrawRectangle(0,0,w+scHor-(GridWidth[0]+1),h);

	int ilcol;
	posY=0;

	bool isd=false;
	bool bg=false;
	bool unkstyle=false;
	bool shorttime=false;
	int states=2;
	int size=GetCount();
	if(SpellErrors.size()<(size_t)size){
		SpellErrors.resize(size);
	}

	Dialogue *acdial=GetDial(MID(0,Edit->ebrow,size));
	Dialogue *Dial;

	int panelrows=(h/(GridHeight+1))+1;
	int scrows=scPos+panelrows;
	//gdy widzimy koniec napisów
	if(scrows >= size + 3){
		bg=true;
		scrows = size + 1;
		scPos=(scrows-panelrows)+2;// dojechanie do końca napisów
		if(panelrows > size + 1){scPos=0;}// w przypadku gdy całe napisy są widoczne, wtedy nie skrollujemy i pozycja =0
	}
	else if(scrows >= size + 2){
		scrows--;//w przypadku gdy mamy linię przed końcem napisów musimy zaniżyć wynik bo przekroczy tablicę.
	}
	SetScrollbar(wxVERTICAL,scPos,panelrows, size + 3);
	int fw,fh,bfw,bfh;

	for(int i=scPos;i<scrows;i++){

		wxArrayString strings;

		if (i==scPos){
			strings.Add("#");
			if (form<SRT){
				strings.Add(_("W."));
			}
			strings.Add(_("Start"));
			if (form!=TMP){
				strings.Add(_("Koniec"));
			}
			if (form<SRT){
				strings.Add(_("Styl"));
				strings.Add(_("Aktor"));
				strings.Add(_("M.L."));
				strings.Add(_("M.P."));
				strings.Add(_("M.Pi."));
				strings.Add(_("Efekt"));
			}
			if(form!=TMP){strings.Add(_("ZNS"));}
			strings.Add(_("Tekst"));
			if(showtl){strings.Add(_("Tekst tłumaczenia"));}
		}else{
			Dial=GetDial(i-1);

			strings.Add(wxString::Format("%i",i));

			isd=Dial->IsComment;
			states=Dial->State;
			if (form<SRT){
				strings.Add(wxString::Format("%i",Dial->Layer));
			}
			strings.Add(Dial->Start.raw());
			if (form!=TMP){
				strings.Add(Dial->End.raw());
			}
			if (form<SRT){
				if(FindStyle(Dial->Style)==-1){unkstyle=true;}else{unkstyle=false;}
				strings.Add(Dial->Style);//(transl&&Dial.TextTl=="")?"TlStyl":
				strings.Add(Dial->Actor);
				strings.Add(wxString::Format("%i",Dial->MarginL));
				strings.Add(wxString::Format("%i",Dial->MarginR));
				strings.Add(wxString::Format("%i",Dial->MarginV));
				strings.Add(Dial->Effect);
			}
			if(form!=TMP && !(CNZ & visible)){
				int chtime;
				if( SpellErrors[i-1].size()<1 ){
					chtime=CalcChars((!showtl&&transl&&Dial->TextTl!="")? 
						Dial->TextTl : Dial->Text) / ((Dial->End.mstime-Dial->Start.mstime)/1000.0f);
					if(chtime<0 || chtime>999){chtime=999;}
					SpellErrors[i-1].push_back(chtime);

				}else{ chtime = SpellErrors[i-1][0];}
				strings.Add(wxString::Format("%i",chtime));
				shorttime=chtime>15;
			}else{
				if(form!=TMP){strings.Add("");}
				if(SpellErrors[i-1].size()==0){SpellErrors[i-1].push_back(0);}
			}

			wxString txt=Dial->Text;
			wxString txttl=Dial->TextTl;
			if (hideover){
				wxRegEx reg("\\{[^\\{]*\\}",wxRE_ADVANCED);
				reg.ReplaceAll(&txt,chtag);
				if(showtl){reg.ReplaceAll(&txttl,chtag);}
			}
			if(txt.Len()>1000){txt=txt.SubString(0,1000)+"...";}
			strings.Add((!showtl&&transl&&txttl!="")?txttl : txt);
			if(showtl){strings.Add(txttl);}

			if(Options.GetBool("Editbox Spellchecker") &&
				(!transl && Dial->Text!="" || transl && Dial->TextTl!="")){
					if(SpellErrors[i-1].size()<2){
						CheckText(strings[strings.size()-1],SpellErrors[i-1]);
					}
			} 
		}

		posX=0;


		ilcol=strings.GetCount();

		wxRect cur;
		bool isCenter;
		for (int j=0; j<ilcol; j++){
			wxDC &dc=(j==0)? mdc : tdc;
			if(showtl&&j==ilcol-2){
				int podz=(w+scHor-posX-1)/2;
				GridWidth[j]=podz;
				GridWidth[j+1]=podz;}

			if(!showtl&&j==ilcol-1){GridWidth[j]=w+scHor-posX-(GridWidth[0]+1);}

			if(GridWidth[j]>0){
				dc.SetPen(*wxTRANSPARENT_PEN);
				wxColour kol=subsBkCol;
				if(i==scPos||j==0&&states==0){kol=labelBkColN;}
				else if(j==0&&states==2){kol=labelBkCol;}
				else if(j==0&&states==1){kol=labelBkColM;}
				else if(isd&&j!=0){kol=comm;}

				if(i>scPos){
					if(sel.find(i-1)!=sel.end()&&j!=0){
						kol=seldial; 
						if(isd&&j!=0){kol=selcom;}
					}
				}
				dc.SetBrush(wxBrush(kol));
				if(unkstyle && j==4 || shorttime && (j==10||(j==3 && form>ASS))){
					dc.SetBrush(wxBrush(Options.GetColour("Grid Spellchecker")));
				}

				dc.DrawRectangle(posX,posY,GridWidth[j],GridHeight);

				if(i!=scPos && j==ilcol-1 && SpellErrors[i-1].size()>2){
					for(size_t k = 1; k < SpellErrors[i-1].size(); k+=2){

						wxString err=strings[j].SubString(SpellErrors[i-1][k], SpellErrors[i-1][k+1]);
						err.Trim();
						if(SpellErrors[i-1][k]>0){
							wxString berr=strings[j].Mid(0, SpellErrors[i-1][k]);
							dc.GetTextExtent(berr, &bfw, &bfh, NULL, NULL, &font);
						}else{bfw=0;}

						dc.GetTextExtent(err, &fw, &fh, NULL, NULL, &font);
						dc.SetBrush(wxBrush(Options.GetColour("Grid Spellchecker")));
						dc.DrawRectangle(posX+bfw+4,posY,fw,GridHeight);
					}

				}

				bool collis=(i!=scPos && i!=Edit->ebrow+1 && (Dial->Start >= acdial->Start && Dial->Start < acdial->End 
					|| Dial->End > acdial->Start && Dial->Start <= acdial->End) ); 
				dc.SetTextForeground( (collis)? collcol : textcol);



				if(form<SRT){isCenter=!(j == 4 || j == 5 || j == 9 || j == 11 || j == 12);}
				else if(form==TMP){isCenter=!(j == 2);}
				else{isCenter=!(j == 4);}
				if(j==ilcol-1 && (strings[j].StartsWith("T") || strings[j].StartsWith("Y") || strings[j].StartsWith(L"Ł"))){posX++;}
				cur = wxRect(posX+4,posY,GridWidth[j]-7,GridHeight);
				dc.SetClippingRegion(cur);
				dc.DrawLabel(strings[j],cur,isCenter ? wxALIGN_CENTER : (wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT));
				dc.DestroyClippingRegion();

				if(j!=0){posX+=GridWidth[j]+1;}
			}
		}

		posY+=GridHeight+1;

	}


	if(bg){
		mdc.SetPen(*wxTRANSPARENT_PEN);
		mdc.SetBrush(wxBrush(labelBkColN));
		mdc.DrawRectangle(0,posY,GridWidth[0],h);
		tdc.SetPen(*wxTRANSPARENT_PEN);
		tdc.SetBrush(wxBrush(Options.GetColour("Grid Background")));
		tdc.DrawRectangle(0,posY,w+scHor-GridWidth[0]+1,h);
	}
	if(mtimerow>=scPos&&mtimerow<=scrows){
		mdc.SetBrush(*wxTRANSPARENT_BRUSH);
		mdc.SetPen(wxPen(Options.GetColour("Grid Active Line"),3));
		mdc.DrawRectangle(1,((mtimerow-scPos+1)*(GridHeight+1))-1,(GridWidth[0]-1),GridHeight+2);
	}

	if(Edit->ebrow>=scPos&&Edit->ebrow<=scrows){
		tdc.SetBrush(*wxTRANSPARENT_BRUSH);
		tdc.SetPen(wxPen(Options.GetColour("Grid Active Line")));
		tdc.DrawRectangle(scHor,((Edit->ebrow-scPos+1)*(GridHeight+1))-1,w+scHor-(GridWidth[0]+1),GridHeight+2);
	}

	mdc.Blit(GridWidth[0]+1,0,w+scHor-(GridWidth[0]+1),h,&tdc,scHor,0);
}

void SubsGrid::AdjustWidths(int cell)
{

	wxClientDC dc(this);
	dc.SetFont(font);

	int law=0,stw=0,edw=0,syw=0,acw=0,efw=0,fw,fh;
	bool shml=false,shmr=false,shmv=false;


	int maxx=GetCount();

	dc.GetTextExtent(wxString::Format("%i",GetCount()), &fw, &fh, NULL, NULL, &font);
	GridWidth[0]=fw+10;
	Dialogue *ndial;
	for(int i=0;i<maxx;i++)
	{
		ndial=GetDialCor(i);
		if(START & cell){
			dc.GetTextExtent(ndial->Start.raw(), &fw, &fh, NULL, NULL, &font);
			if(fw+10>stw){stw=fw+10;}}
		if((END & cell)&&form!=TMP){
			dc.GetTextExtent(ndial->End.raw(), &fw, &fh, NULL, NULL, &font);
			if(fw+10>edw){edw=fw+10;}
		}


		if(form<SRT){
			if((LAYER & cell) && ndial->Layer!=0){
				dc.GetTextExtent(wxString::Format("%i",ndial->Layer), &fw, &fh, NULL, NULL, &font);
				if(fw+10>law){law=fw+10;}}
			if(STYLE & cell){	
				dc.GetTextExtent(ndial->Style, &fw, &fh, NULL, NULL, &font);
				if(fw+10>syw){syw=fw+10;}}
			if((ACTOR & cell) && ndial->Actor!=""){
				dc.GetTextExtent(ndial->Actor, &fw, &fh, NULL, NULL, &font);
				if(fw+10>acw){acw=fw+10;}}
			if((EFFECT & cell) && ndial->Effect!=""){
				dc.GetTextExtent(ndial->Effect, &fw, &fh, NULL, NULL, &font);
				if(fw+10>efw){efw=fw+10;}}
			if((MARGINL & cell) && ndial->MarginL!=0){shml=true;}
			if((MARGINR & cell) && ndial->MarginR!=0){shmr=true;}
			if((MARGINV & cell) && ndial->MarginV!=0){shmv=true;}
		}
	}

	if((form<SRT)? (LAYER & cell) : (START & cell)){
		wxString frst=(form<SRT)?_("W."):_("Start");
		dc.GetTextExtent(frst, &fw, &fh, NULL, NULL, &font);
		GridWidth[1] = (form<SRT)?law : stw;
		if(fw+10>GridWidth[1]&&GridWidth[1]!=0){GridWidth[1]=fw+10;}
	}

	if((form<SRT)? (START & cell) : (END & cell)){
		wxString scnd=(form<SRT)?_("Start"):_("Koniec");
		dc.GetTextExtent(scnd, &fw, &fh, NULL, NULL, &font);
		GridWidth[2] = (form<SRT)?stw : edw;
		if(fw+10>GridWidth[2]){GridWidth[2]=fw+10;};
	}
	if(form<SRT){
		if(END & cell){
			dc.GetTextExtent(_("Koniec"), &fw, &fh, NULL, NULL, &font);
			GridWidth[3] = edw;
			if(fw+10>GridWidth[3]){GridWidth[3]=fw+10;};
		}

		if(STYLE & cell){
			dc.GetTextExtent(_("Styl"), &fw, &fh, NULL, NULL, &font);
			GridWidth[4] = syw;
			if(fw+10>GridWidth[4]){GridWidth[4]=fw+10;}
		}

		if(ACTOR & cell){
			dc.GetTextExtent(_("Actor"), &fw, &fh, NULL, NULL, &font);
			if(fw+10>acw&&acw!=0){acw=fw+10;};
			GridWidth[5] = (acw==0)?0:acw;
		}

		if(224 & cell){
			dc.GetTextExtent(_("M.Pi."), &fw, &fh, NULL, NULL, &font);
			if(MARGINL & cell){GridWidth[6]=(!shml)?0:fw+4;}
			if(MARGINR & cell){GridWidth[7]=(!shmr)?0:fw+4;}
			if(MARGINV & cell){GridWidth[8]=(!shmv)?0:fw+10;}
		}

		if(EFFECT & cell){
			dc.GetTextExtent(_("Efekt"), &fw, &fh, NULL, NULL, &font);
			if(fw+10>efw&&efw!=0){efw=fw+10;};
			GridWidth[9] = (efw==0)?0:efw;
		}
	}

	if(CNZ & cell){
		dc.GetTextExtent(_("ZNS"), &fw, &fh, NULL, NULL, &font);
		GridWidth[(form<SRT)?10 : 3]=fw+5;
	}

	if(form==TMP){GridWidth[2]=0;GridWidth[3]=0;GridWidth[10]=0;}
	if(form>ASS){GridWidth[4]=0;GridWidth[5]=0;GridWidth[6]=0;GridWidth[7]=0;GridWidth[8]=0;GridWidth[9]=0;}
	if((form<SRT)? (LAYER & visible) : (START & visible)){GridWidth[1]=0;}
	if((form<SRT)? (START & visible) : (END & visible)){GridWidth[2]=0;}
	if((form<SRT)? (END & visible) : (CNZ & visible)){GridWidth[3]=0;}
	if(STYLE & visible){GridWidth[4]=0;}
	if(ACTOR & visible){GridWidth[5]=0;}
	if(MARGINL & visible){GridWidth[6]=0;}
	if(MARGINR & visible){GridWidth[7]=0;}
	if(MARGINV & visible){GridWidth[8]=0;}
	if(EFFECT & visible){GridWidth[9]=0;}
	if(CNZ & visible){GridWidth[10]=0;}
	first=false;

}


void SubsGrid::Clearing()
{
	sel.clear();

	wxDELETE(file);
	SpellErrors.clear();
	Modified=false;
	first=true;
	scPos=0;
	lastRow=0;
	scHor=0;
}
void SubsGrid::AddLine(Dialogue *line)
{
	if(line->NonDial){delete line; return;}
	file->subs->ddials.push_back(line);
	file->subs->dials.push_back(line);
}
void SubsGrid::RepaintWindow(int cell)
{
	AdjustWidths(cell);
	Refresh(false);
}
void SubsGrid::ChangeLine(Dialogue *line1, int wline, long cells, bool selline, bool dummy)
{
	lastRow=wline;
	wxArrayInt sels=GetSels();
	if(sels.size()<2 || cells & TXT || cells & TXTTL){
		ChangeCell(cells,wline,line1);
	}else{
		for(size_t i=0;i<sels.size();i++){
			ChangeCell(cells,sels[i],line1);
		}
		if(selline){wline=sels[sels.size()-1];}
	}
	if(wline>=GetCount()-1 && selline){
		Dialogue *tmp=new Dialogue(); 
		tmp->State=1;
		int eend=line1->End.mstime; 
		tmp->Start.NewTime(eend); 
		tmp->End.NewTime(eend+5000);
		tmp->Style=line1->Style;
		if(form!=ASS){tmp->Conv(form);}
		AddLine(tmp);
	}
	AdjustWidths(cells);

	if(selline){

		lastRow=wline+1;
		sel.clear();
		sel[lastRow]=true;
		int h,w;
		GetClientSize(&w,&h);
		scPos = MID(0, lastRow-((h/(GridHeight+1))/2), GetCount()-1);

	}
	Refresh(false);
	if(selline){Edit->SetIt(lastRow);}
	SetModified(false,dummy);

}

void SubsGrid::ChangeCell(long wcell, int wline, Dialogue *what)
{
	Dialogue *dial=CopyDial(wline);
	if(wcell & LAYER){
		dial->Layer=what->Layer;}
	if(wcell & START){
		dial->Start=what->Start;}
	if(wcell & END){
		dial->End=what->End;}
	if(wcell & STYLE){
		dial->Style=what->Style;}
	if(wcell & ACTOR){
		dial->Actor=what->Actor;}
	if(wcell & MARGINL){
		dial->MarginL=what->MarginL;}
	if(wcell & MARGINR){
		dial->MarginR=what->MarginR;}
	if(wcell & MARGINV){
		dial->MarginV=what->MarginV;}
	if(wcell & EFFECT){
		dial->Effect=what->Effect;}
	if(wcell & TXT){
		dial->Text=what->Text;
		//if(!transl){SpellErrors[wline].clear();CharsPerSec[wline]=-1;}
	}
	if(wcell & COMMENT){
		dial->IsComment=what->IsComment;}
	if(wcell & TXTTL){
		dial->TextTl=what->TextTl;
		//if(!transl){SpellErrors[wline].clear();CharsPerSec[wline]=-1;}
	}
}

Dialogue *SubsGrid::GetDialCor(int rw)
{
	Dialogue *dial=file->subs->dials[rw];
	if(first){
		if(dial->Form!=form){dial->Conv(form);} 
		if(dial->Start.mstime > dial->End.mstime){
			dial->End.mstime=dial->Start.mstime;
		}
	}
	return dial;
}

void SubsGrid::OnScroll(wxScrollWinEvent& event)
{
	int newPos=0;
	if(event.GetEventType()==wxEVT_SCROLLWIN_LINEUP)
	{
		newPos=scPos-1;
		if(newPos<0){newPos=0;return;}
	}
	else if(event.GetEventType()==wxEVT_SCROLLWIN_LINEDOWN)
	{
		newPos=scPos+1;
		if(newPos>=GetCount()){newPos=GetCount()-1;return;}
	}
	else if(event.GetEventType()==wxEVT_SCROLLWIN_PAGEUP)
	{
		wxSize size=GetClientSize();
		newPos=scPos;
		newPos-=(size.y/GridHeight - 1);
		newPos=MAX(0,newPos);
	}
	else if(event.GetEventType()==wxEVT_SCROLLWIN_PAGEDOWN)
	{
		wxSize size=GetClientSize();
		newPos=scPos;
		newPos+=(size.y/GridHeight - 1);
		newPos=MIN(newPos,GetCount()-1);
	}
	else{newPos = event.GetPosition();}
	if (scPos != newPos) {
		scPos = newPos;
		Refresh(false);
	}
}

void SubsGrid::OnSize(wxSizeEvent& event)
{
	//wxSize size= GetClientSize();
	Refresh(false);
}

void SubsGrid::SelectRow(int row, bool addToSelected, bool select, bool norefresh)
{
	row = MID(0,row,GetCount()-1);
	if(addToSelected){
		if (!select){sel.erase( sel.find(row));}
		else{sel[row]=select;}
		if(norefresh){return;}
		int w = 0;
		int h = 0;
		GetClientSize(&w,&h);
		RefreshRect(wxRect(0,(row+1-scPos)*(GridHeight+1),w,GridHeight+1),false);
		//Refresh(false);

	}
	else{
		sel.clear();
		sel[row]=select;
		if(norefresh){return;}
		Refresh(false);
	}
}

void SubsGrid::ScrollTo(int y, bool center){
	int w,h;
	GetClientSize(&w,&h);
	if(center){y-=(h/(GridHeight+1))/2;}
	int nextY = MID(0,y,GetCount()+2 - h/(GridHeight+1));

	if (scPos != nextY) {
		scPos = nextY;
		Refresh(false);
	}
}


void SubsGrid::OnMouseEvent(wxMouseEvent &event) {

	int w,h;
	GetClientSize (&w, &h);

	//wxLogStatus("lines %i", event.GetLinesPerAction());

	bool shift = event.ShiftDown();
	bool alt = event.AltDown();
	bool ctrl = event.CmdDown();

	bool click = event.LeftDown();
	bool left_up = event.LeftUp();
	bool dclick = event.LeftDClick();
	bool middle = event.MiddleDown();
	bool right = event.RightDown();
	int curY=(event.GetY());
	int curX=(event.GetX());

	if(ismenushown){ScreenToClient(&curX,&curY);}
	int row = curY / (GridHeight+1)+scPos-1;


	if (left_up && !holding) {
		return;
	}

	// Get focus
	if (event.ButtonDown()) {
		SetFocus();}

	// Popup
	if (right && !ctrl) {
		if(curX<=GridWidth[0]){
			mtimerow=row;
			Refresh(false);
			//wxLogStatus("mark");
		}else{
			ContextMenu(event.GetPosition());
		}

		return;
	}

	// Mouse wheel
	if (event.GetWheelRotation() != 0) {
		int step = 3 * event.GetWheelRotation() / event.GetWheelDelta();
		ScrollTo(scPos - step);
		return;}



	if (left_up && holding) {
		holding = false;
		if(file->IsNotSaved()&&lastsel!=-1){SetModified();}
		ReleaseMouse();
		if(oldX!=-1){return;}
	}


	// Click type
	if (click) {
		holding = true;
		if (!shift) lastRow = row;
		lastsel=row;
		oldX=(curY<GridHeight)?curX : -1;
		CaptureMouse();
	}
	if(holding && oldX!=-1){
		int diff=(oldX-curX);
		if((scHor==0&&diff<0)||diff==0||(scHor>1500&&diff>0)){return;}
		scHor=scHor+diff;
		oldX=curX;
		if (scHor<0){scHor=0;}
		Refresh(false);
		return;
	}

	if (!(row < scPos || row >= GetCount())) {

		if(holding && alt && lastsel!=row)
		{
			if (lastsel != -1) {
				file->edited=true;
				//if(!sel[lastsel]){
				//SelectRow(row,false,true);
				//SwapRows(lastsel,row);
				// }else{
				/*if (row < scPos+1 ){
				MoveRows(-1);
				}else if(row > scPos+h/(GridHeight+1)-3){
				MoveRows(1);
				}else{*/
				MoveRows(row-lastsel);//}
				// }
			}
			lastsel=row;
			//return;
		}


		// Toggle selected
		if (left_up && ctrl && !shift && !alt) {
			SelectRow(row,true,!(sel.find(row)!=sel.end()));
			return;
		}

		// Normal click
		if (!shift && !alt) {


			//jakbym chciał znów dać zmianę edytowanej linii z ctrl to muszę dorobić mu refresh
			if ((click && !ctrl) || (dclick && ctrl)) {// || dclick
				Edit->SetIt(row,true,true,true);
				SelectRow(row);
				extendRow = -1;
			}


			int mvtal= Options.GetInt("Move Video To Active Line");
			//1-kliknięcie lewym
			//2-kliknięcie lewym i edycja na pauzie
			//3-kliknięcie lewym i edycja na pauzie i odtwarzaniu

			if (dclick||(left_up && mvtal < 4 && mvtal > 0 )){
				TabPanel *pan=(TabPanel*)GetParent();
				if(pan->Video->GetState()!=None){
					if(pan->Video->GetState()==Stopped){pan->Video->Play();pan->Video->Pause();}
					short wh=(form<SRT)?2:1;
					int whh=2;
					for(int i = 0;i<=wh;i++){whh+=GridWidth[i];}
					whh-=scHor;
					bool isstart;
					int vczas;
					if(event.GetX()>=whh&&event.GetX()<whh+GridWidth[wh+1]&&form!=TMP){ 
						vczas=GetDial(row)->End.mstime; isstart=false;}
					else{vczas=GetDial(row)->Start.mstime; isstart=true;}
					if(ctrl){vczas-=1000;SelectRow(row);}
					pan->Video->Seek(MAX(0,vczas),isstart);
					if(Edit->ABox){Edit->ABox->audioDisplay->UpdateImage(true);}
				}
			}



			if (click || dclick || left_up)
				return;
		}

		if (middle){
			VideoCtrl *video=Kai->GetTab()->Video;
			if(video->GetState()!=None){//
				video->PlayLine(GetDial(row)->Start.mstime, GetDial(row)->End.mstime- video->avtpf-10);
			}

		}
	}

	// Scroll to keep visible
	if (holding) {
		// Find direction
		int scdelta=(alt)? 1 : 3;
		int minVis = scPos + 1;
		int maxVis = scPos+h/(GridHeight+1) - 2;
		int delta = 0;
		if (row < minVis && row!=0) delta = -scdelta;
		if (row > maxVis) delta = scdelta;

		if (delta) {
			ScrollTo(scPos + delta);//row - (h / (GridHeight+1)) row

			// End the hold if this was a mousedown to avoid accidental
			// selection of extra lines
			if (click) {// && row!=GetCount()-1
				holding = false;
				left_up = true;
				ReleaseMouse();
			}
		}
	}
	// Block select
	if ((left_up && shift && !alt)|| (holding && !ctrl && !alt && !shift && lastsel!=row)) {
		if (lastRow != -1) {
			// Keyboard selection continues from where the mouse was last used
			extendRow = lastRow;

			// Set boundaries
			int i1 = MID(0,row, GetCount()-1);
			int i2 = lastRow;
			if (i1 > i2) {
				int aux = i1;
				i1 = i2;
				i2 = aux;
			}

			// Toggle each
			bool notFirst = false;
			for (int i=i1;i<=i2;i++) {
				SelectRow(i, notFirst || ctrl,true,true);
				notFirst = true;
			}
			lastsel=row;
			Refresh(false);
		}
	}


}

void SubsGrid::Convert(char type)
{
	if(Options.GetBool("Show settings window")){
		OptionsDialog od(Kai,Kai);
		od.OptionsTree->ChangeSelection(2);
		od.okok->SetFocus();
		if(od.ShowModal()==wxID_CANCEL){return;}
	}
	if(Options.GetBool("FPS from video")&&Kai->GetTab()->VideoPath!=""){
		Options.SetString("Default FPS",Kai->StatusBar1->GetStatusText(2).BeforeFirst(' '));}
	if(Options.GetFloat("Default FPS")<1){wxMessageBox(_("Nieprawidłowy FPS. Popraw opcje i spróbuj ponownie."));return;}

	bool newendtimes=Options.GetBool("New end times");
	wxString stname=Options.GetString("Default Style");
	int endt=Options.GetInt("Time show of letter");
	wxString prefix=Options.GetString("Ass Conversion Prefix");
	//wxMessageBox("pętla");
	int i=0;
	while(i<GetCount())
	{
		//Dialogue *dial=GetDial(i);
		if((type>ASS) && (form<SRT) && GetDial(i)->IsComment){
			while(GetDial(i)->IsComment){
				DeleteRow(i);
			}
		}
		Dialogue *dialc=CopyDial(i);
		dialc->Conv(type,prefix);
		if((newendtimes && type!=TMP)||form==TMP)
		{
			if(i>0){
				if(GetDial(i-1)->End.mstime > dialc->Start.mstime){
					GetDial(i-1)->End = dialc->Start;
				}
			}

			int newend=(endt * dialc->Text.Len());
			if(newend<1000){newend=1000;}
			newend += dialc->Start.mstime;
			dialc->End.NewTime(newend);
		}

		i++;	
	}

	if(type==ASS){
		LoadDefault(false,true,false);
		wxString resx=Options.GetString("Convert Resolution W");
		wxString resy=Options.GetString("Convert Resolution H");
		if(resx==""){resx="1280";}
		if(resy==""){resx="720";}
		AddSInfo("PlayResX",resx, false);
		AddSInfo("PlayResY",resy, false);
		wxString catalog=Options.GetString("Default Style Catalog");

		if(Options.dirs.Index(catalog)!=-1){Options.LoadStyles(catalog);}
		int stind=Options.FindStyle(stname);

		if(stind<0){Styles *newstyl=new Styles(); newstyl->Name=stname;AddStyle(newstyl);}
		else{AddStyle(Options.GetStyle(stind)->Copy());}
		Edit->RefreshStyle();
	}
	if(form==ASS){std::sort(file->subs->dials.begin(),file->subs->dials.end(),sortstart);}
	form=type;
	Edit->SetIt(Edit->ebrow);
	SetModified();
	RepaintWindow();
}
/*
DWORD SubsGrid::saveproc(void* param)
{
SubsGrid * grid = (SubsGrid*)param;
wxString txt=grid->GetSInfo("TLMode Style");
wxString kkk=grid->GetSInfo("TLMode");
//wxString mode = (wxFile::Exists(grid->fn))? 
//OpenWrite ow(grid->fn,false);
//FILE *file = _wfopen(grid->fn.fn_str(),L"a+b");
//wxFFile file;
//file.Open(grid->fn, "a+");
//wxString tmppath="\\?\\"+grid->fn;
//wxLogStatus("przed stworzeniem pliku");
HANDLE ffile=CreateFile(grid->fn.fn_str(), GENERIC_WRITE, FILE_SHARE_WRITE,NULL, OPEN_EXISTING, 0, NULL);
//wxLogStatus("stworzyło plik");
//wxMutex mutex1;
DWORD savesize=0;
wxMutex mutex2;
while(1)
{
if(grid->acline>=grid->GetCount()){break;}
Dialogue *dial=grid->GetDial(grid->acline++);
//grid->acline++;
//wxLogStatus("acline %i", grid->acline);
wxString wynik;
if(kkk!=""){
if(kkk!="Translated" && dial->TextTl!=""){
wynik<<dial->GetRaw(false,txt)<<dial->GetRaw(true);
}
else{
wynik<<dial->GetRaw(dial->TextTl!="");
}
}else{
if(grid->form==SRT){wynik<<grid->acline+1<<"\r\n";}
wynik<<dial->GetRaw();

}
if(dial->State==1&&grid->cstate){dial->State=2;}
wxScopedCharBuffer buffer= wynik.mb_str(wxConvUTF8);//mb_str(wxConvUTF8)//wxScopedCharBuffer 
int size = strlen(buffer);
grid->mutex.Lock();
SetFilePointer(ffile,0,0,FILE_END);
WriteFile(ffile, buffer, size, &savesize, 0);
grid->mutex.Unlock();
}
//ow.CloseFile();
//file.Close();
CloseHandle(ffile);
return 0;
}
*/
void SubsGrid::SaveFile(wxString filename, bool cstat)
{
	if(Options.GetBool("Grid save without enter")){
		Edit->Send(false);
	}
	wxString txt;
	wxString kkk;

	OpenWrite ow(filename,true);

	if (form<SRT){
		AddSInfo("Last Style Storage",Options.acdir, false);

		AddSInfo("Active Line", kkk<<Edit->ebrow, false);

		txt<<"[Script Info]\r\n;Plik utworzony przez "<<Options.progname<<"\r\n"<<GetSInfos(GetSInfo("TLMode")=="Translated");
		txt<<"\r\n[V4+ Styles]\r\nFormat: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding \r\n";
		txt<<GetStyles(GetSInfo("TLMode")=="Translated");
		txt<<" \r\n[Events]\r\nFormat: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\r\n";
	}
	ow.PartFileWrite(txt);

	txt=GetSInfo("TLMode Style");
	kkk=GetSInfo("TLMode");
	//HANDLE worker[4];
	for(int i=0;i<GetCount();i++)
	{
		//if(it->first==i&&it!=file->scomm.end()){ow.PartFileWrite(it->second);if(it!=file->scomm.end()){it++;}}
		Dialogue *dial=GetDial(i);


		if(kkk!=""){
			if(kkk!="Translated" && dial->TextTl!=""){
				ow.PartFileWrite(dial->GetRaw(false,txt));
				ow.PartFileWrite(dial->GetRaw(true));}
			else{
				ow.PartFileWrite(dial->GetRaw((dial->TextTl!="")));
			}
		}else{
			wxString wynik;
			if(form==SRT){wynik<<i+1<<"\r\n";}
			ow.PartFileWrite(wynik+dial->GetRaw());
		}
		if(dial->State==1&&cstat){dial->State=2;}

		//worker[i] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)saveproc,this,0, NULL);

	}
	//WaitForMultipleObjects(4, worker, TRUE, INFINITE);
	ow.CloseFile();
	if(cstat){Refresh(false);}
}

void SubsGrid::HideOver()
{
	hideover=!hideover;
	Options.SetBool("Grid Hide Tags",hideover);
	SpellErrors.clear();
	Refresh(false);
}

void SubsGrid::AddStyle(Styles *nstyl)
{
	file->subs->dstyles.push_back(nstyl);
	file->subs->styles.push_back(nstyl);
}

void SubsGrid::ChangeStyle(Styles *nstyl, int i)
{
	file->subs->dstyles.push_back(nstyl);
	file->subs->styles[i]=nstyl;
}

int SubsGrid::StylesSize()
{
	return file->subs->styles.size();
}

Styles *SubsGrid::GetStyle(int i,wxString name)
{
	if(name!=""){
		for(unsigned int j=0; j < file->subs->styles.size();j++)
		{
			if(name==file->subs->styles[j]->Name){return file->subs->styles[j];}
		}
	}
	return file->subs->styles[i];
}

std::vector<Styles*> *SubsGrid::GetStyleTable()
{
	return &file->subs->styles;
}

//multiplication musi być ustawione na zero, wtedy zwróci ilość multiplikacji
int SubsGrid::FindStyle(wxString name,int *multip)
{
	int isfound=-1;
	for(unsigned int j=0;j < file->subs->styles.size();j++)
	{
		if(name==file->subs->styles[j]->Name){
			isfound=j;
			if(multip){*multip++;
			}else{break;}
		}
	}
	return isfound;
}

wxString SubsGrid::GetStyles(bool tld)
{   
	wxString allst;
	wxString tmpst;
	if(tld){tmpst=GetSInfo("TLMode Style");}
	for(size_t i=0;i<file->subs->styles.size();i++)
	{
		if(!(tld&&file->subs->styles[i]->Name==tmpst)){
			allst<<file->subs->styles[i]->styletext();}
	}
	return allst;
}

void SubsGrid::DelStyle(int i)
{
	file->edited=true;
	file->subs->styles.erase(file->subs->styles.begin()+i);
}

int SubsGrid::GetCount()
{
	return file->subs->dials.size();
}

int SubsGrid::FirstSel()
{
	if(!sel.empty()){
		return sel.begin()->first;
	}
	return -1;
}

class compare { // simple comparison function
public:
	bool operator()(Dialogue *i,Dialogue *j) {

		if(i->Start.mstime!=j->Start.mstime){
			return (i->Start.mstime<j->Start.mstime);
		}
		return i->End.mstime<j->End.mstime;
	} // returns x>y
};

void SubsGrid::ChangeTime()
{
	VideoCtrl *vb=Kai->GetTab()->Video;
	int added=0;
	//1 forward / backward, 2 Start Time For V/A Timing, 4 Move to video time, 8 Move to audio time;
	int mto = Options.GetInt("Moving time options");
	int tim = Options.GetInt("Change Time");
	int lmd = MAX(0,Options.GetInt("Change mode"));
	int seb = MAX(0,Options.GetInt("Start end times"));
	int CT  = Options.GetInt("Corect times");
	//1 Lead In, 2 Lead Out, 4 Make times continous, 8 Snap to keyframe;
	int pe  = Options.GetInt("Postprocessor enabling");
	int li, lo, ts, te, kbs, kas, kbe, kae;
	if(pe){
		if(form==TMP || pe<16){pe=0;}
		else if(pe & 8 && !vb->VFF){pe^=8;}
		li  = Options.GetInt("Lead in");
		lo  = Options.GetInt("Lead out");
		ts  = Options.GetInt("Threshold start");
		te  = Options.GetInt("Threshold end");
		kbs = Options.GetInt("Keyframe before start");
		kas = Options.GetInt("Keyframe after start");
		kbe = Options.GetInt("Keyframe before end");
		kae = Options.GetInt("Keyframe after end");
	}
	wxString style=Options.GetString("Styles of time change");
	if(!(mto & 1)){tim=(-tim);}
	int VAS = mto & 2;


	std::map<Dialogue *,int,compare> tmpmap;
	/*if(CT>0||pe>19){
		for (int i=0; i<GetCount(); i++)
		{
			tmpmap[GetDial(i)]=i;
		}
	}*/

	if(seb!=0){
		int answer=wxMessageBox(wxString::Format(_("Czy naprawdę chcesz przesuwać tylko czasy %s?"), 
			(seb==1)? _("początkowe") : _("końcowe")),_("Potwierdzenie"),wxYES_NO);
		if(answer==wxNO){return;}
	}

	if(form==TMP){seb=1;}

	bool fromstyl=false;

	int fs=FirstSel();
	if (fs==-1&&lmd!=0&&lmd!=4){
		wxMessageBox(_("Nie zaznaczono linii do przesunięcia"),_("Uwaga"));return;}

	int difftime=(VAS)? file->subs->dials[mtimerow]->Start.mstime : file->subs->dials[mtimerow]->End.mstime;
	int halfframe= (VAS)? -(vb->avtpf/2) : (vb->avtpf/2);
	if((mto & 4) && vb->GetState()!=None){
		added= vb->Tell() - difftime + halfframe - 10;
		added=ZEROIT(added);
	}
	else if((mto & 8) && Edit->ABox->audioDisplay->hasMark){
		added= Edit->ABox->audioDisplay->curMarkMS - difftime;
		added=ZEROIT(added);
	}

	
	wxArrayString stcomp;
	if(lmd==4){
		int g=0;
		wxStringTokenizer styles(style,";");
		while(styles.HasMoreTokens()){
			wxString styl=styles.GetNextToken();
			styl.Trim(false);
			styl.Trim(true);
			stcomp.Add(styl);
			//wxLogStatus(styl);
			g++;
		}
	}

	int firsttime=GetDial(fs)->Start.mstime;
	Dialogue *dialc;
	tim+=added;
	int newstarttime=-1;

	for (int i=0;i<GetCount();i++)
	{
		if(lmd==4 && stcomp.GetCount()>0 ){
			fromstyl=false;
			wxString styl=file->subs->dials[i]->Style;
			for(size_t i=0;i<stcomp.GetCount();i++){
				if(styl==stcomp[i]){fromstyl=true;}
			}
		}
		
		if( lmd==0
			|| ( lmd==1 && sel.find(i) != sel.end() ) 
			|| ( lmd==3 && firsttime <= file->subs->dials[i]->Start.mstime ) 
			|| ( lmd==2 && i>=fs )
			|| fromstyl)
		{

			dialc=file->CopyDial(i,true,true);
			if(tim!=0){
				if(seb!=2){dialc->Start.Change(tim);}
				if(seb!=1){dialc->End.Change(tim);}
				dialc->State=1;
			}
			if(CT>0 || pe>16){
				if(CT>1){
					int endt=Options.GetInt("Time show of letter");
					int newend=(endt*dialc->Text.Len());
					if(newend<1000){newend=1000;}
					newend+=dialc->Start.mstime;
					dialc->End.NewTime(newend);
				}
				if(pe & 1){dialc->Start.Change(-li);dialc->State=1;}
				if(pe & 2){dialc->End.Change(lo);dialc->State=1;}
				if(CT>0 || pe>19){
					tmpmap[dialc]=i;
					
				}
			}

			//file->subs->dials[i]=dialc;
		}// if przesuwana linia

	}//pętla for

	// tu jeszcze należy poprawić uwzględniając linijkę z czasem przed tablicą i czasem po niej
	// a może to w ogóle nie jest potrzebne?
	if(CT>0 || pe>19){
		bool hasend=false;
		int Halfframe=vb->avtpf/2.f;
		for(auto cur=tmpmap.begin(); cur != tmpmap.end(); cur++){
			auto it = cur;
			dialc = cur->first;//file->subs->dials[cur->second];
			it++;
			if(!(it!=tmpmap.end())){it=cur; hasend=true;}
			if(CT>0 && dialc->End > it->first->Start && !hasend){
				dialc->End = it->first->Start;
				dialc->State=1;
			}
			if(pe & 4){
				int cdiff= (te+ts);
				int tdiff = it->first->Start.mstime - dialc->End.mstime;
				if(newstarttime != -1){
					dialc->Start.NewTime(newstarttime);
					newstarttime = -1;
					dialc->State=1;
				}
				if(tdiff <= cdiff && tdiff > 0){
					int wsp = ((float)tdiff / (float)cdiff)*te;
					int newtime=ZEROIT(wsp);
					dialc->End.Change(newtime);
					newstarttime = dialc->End.mstime;
					dialc->State=1; //dialcopy->State=1;

				}

			}
			if(pe & 8){
				int strtrng = dialc->Start.mstime - kbs;
				int strtrng1 = dialc->Start.mstime + kas;
				int endrng = dialc->End.mstime - kbe;
				int endrng1 = dialc->End.mstime + kae;
				int pors = 0;
				int pore = (hasend)? INT_MAX : it->first->Start.mstime + kas;
					
				if(cur!=tmpmap.begin()){
					it--;
					if(!hasend){it--;}
					pors = it->first->End.mstime - kbe;
				}

				int keyMS=0;
				int strtres=INT_MAX;
				int endres=-1;
				for (unsigned int g=0;g<vb->VFF->KeyFrames.Count();g++) {
					keyMS = vb->VFF->KeyFrames[g];
					if (keyMS > strtrng && keyMS < strtrng1) {
						keyMS = ZEROIT((keyMS-Halfframe));
						if(strtres>keyMS && keyMS != dialc->Start.mstime){strtres = keyMS;}
					}
					if (keyMS > endrng && keyMS < endrng1) {
						keyMS = ZEROIT((keyMS-Halfframe));
						if(endres<keyMS && keyMS != dialc->Start.mstime){endres = keyMS;}
					}
				}
				if(strtres!=INT_MAX && strtres >= pors){
					//strtres -= vb->avtpf/2.f;
					dialc->Start.NewTime(strtres);
					dialc->State=1;
				}
				if(endres!=-1 && endres <= pore){
					//endres -= vb->avtpf/2.f;
					dialc->End.NewTime(endres);
					dialc->State=1;
				}
			}

			//file->subs->dials[cur->second]=dialc;
		}

	}
	SpellErrors.clear();
	SetModified();
	if(form>TMP){RepaintWindow(START|END);}else{Refresh(false);}
}


void SubsGrid::SortIt(short what, bool all)
{

	std::vector<Dialogue*> selected;
	if(all){
		for(int i=0;i<GetCount();i++){file->subs->dials[i]->State=1;}
	}else{
		for(auto cur=sel.begin(); cur!=sel.end(); cur++){
			Dialogue *dial=file->subs->dials[cur->first];
			dial->State=1;
			selected.push_back(dial);
		}
	}
	std::sort((all)? file->subs->dials.begin() : selected.begin(), (all)? file->subs->dials.end() : selected.end(),
		(what==0)? sortstart : (what==1)? sortend : (what==2)? sortstyle : 
		(what==3)? sortactor : (what==4)? sorteffect : sortlayer);

	if(!all){
		int ii=0;
		for(auto cur=sel.begin(); cur!=sel.end(); cur++){
			file->subs->dials[cur->first]=selected[ii++];
		}
		selected.clear();
	}
	file->edited=true;
	SpellErrors.clear();
	SetModified();
	Refresh(false);
} 

void SubsGrid::OnKeyPress(wxKeyEvent &event) {
	// Get size
	int w,h;
	GetClientSize(&w,&h);

	// Get scan code
	int key = event.GetKeyCode();
	bool ctrl = event.m_controlDown;
	bool alt = event.m_altDown;
	bool shift = event.m_shiftDown;

	// The "menu" key, simulate a right-click
	if (key == WXK_MENU || key == WXK_WINDOWS_MENU) {
		wxPoint pos;
		pos.x = w/2;
		//if (shift) {
		//pos.y = lineHeight/2;
		//OnPopupMenu(true, pos);
		//}
		//else {
		pos.y = (Edit->ebrow+1-scPos) * GridHeight + GridHeight/2;
		ContextMenu(pos);
		//}
		return;
	}

	// Select all
	if (key == 'A' && ctrl && !alt && !shift) {
		//SelectRow(0,false,true,true);
		for(int i=0;i<GetCount();i++){
			sel[i]=true;
		}
		Refresh(false);
	}

	// Up/down
	int dir = 0;
	if (key == WXK_UP) dir = -1;
	if (key == WXK_DOWN) dir = 1;
	if (key == WXK_PAGEUP) {
		dir = -(h/GridHeight - 1);
	}
	if (key == WXK_PAGEDOWN) {
		dir = h/GridHeight - 1;
	}
	if (key == WXK_HOME) {
		dir = -GetCount();
	}
	if (key == WXK_END) {
		dir = GetCount();
	}
	if (key == WXK_RETURN){
		Edit->TextEdit->SetFocus();}

	// Moving
	if (dir) {
		// Move selection
		if (!ctrl && !shift && !alt) {
			// Move to extent first
			int curLine = Edit->ebrow;
			if (extendRow != -1) {
				curLine = extendRow;
				extendRow = -1;
			}

			int next = MID(0,curLine+dir,GetCount()-1);
			Edit->SetIt(next);
			SelectRow(next);
			int gridh=((h/(GridHeight+1))-1);
			if(dir==1||dir==-1){
				bool above= (next<=scPos);
				bool below= (next>=scPos+gridh);
				if(above||below){ScrollTo(above? next-1 : next-gridh+1);}
			}else{
				ScrollTo(next);}

			return;
		}


		// Move selected
		if (alt&&!shift) {
			if((dir==1||dir==-1)&&FirstSel()!=-1){MoveRows(dir,true);ScrollTo(scPos+dir);}
			return;
		}

		// Shift-selection
		if (shift && !ctrl && !alt) {
			// Find end
			if (extendRow == -1) extendRow = Edit->ebrow;
			extendRow = MID(0,extendRow+dir,GetCount()-1);

			// Set range
			int i1 = Edit->ebrow;
			int i2 = extendRow;
			if (i2 < i1) {
				int aux = i1;
				i1 = i2;
				i2 = aux;
			}

			// Select range
			bool notfirst=false;
			for (int i=i1;i<=i2;i++) {
				SelectRow(i,notfirst);
				notfirst=true;
			}

			int gridh=((h/(GridHeight+1))-1);
			if(extendRow==scPos&&(dir==1||dir==-1)){
				ScrollTo(extendRow-1);}
			else if(extendRow==scPos+gridh&&(dir==1||dir==-1)){
				ScrollTo(extendRow-gridh+1);}
			else if(dir!=1&&dir!=-1){
				ScrollTo(extendRow);}
			return;
		}
	}




}

void SubsGrid::DeleteRow(int rw, int len)
{
	int rwlen=rw+len;
	file->edited=true;
	file->subs->dials.erase(file->subs->dials.begin()+rw, file->subs->dials.begin()+rwlen);
	if((int)SpellErrors.size()>rwlen){ SpellErrors.erase(SpellErrors.begin()+rw, SpellErrors.begin()+rwlen);}
	else{SpellErrors.clear();}
}

void SubsGrid::DeleteRows()
{
	Freeze();
	wxArrayInt sels=GetSels(true);
	for(int i= sels.size()-1; i>=0; i--)
	{
		file->subs->dials.erase(file->subs->dials.begin()+sels[i]);
		SpellErrors.erase(SpellErrors.begin()+sels[i]);
	}
	if(GetCount()<1){AddLine(new Dialogue());}
	if(sels.size()>0){file->edited=true;}
	SetModified();
	Thaw();
	RepaintWindow();
}

void SubsGrid::MoveRows(int step, bool sav)
{
	wxArrayInt sels=GetSels();

	if (sels.GetCount()<1){return;}
	if(step<0){

		for (size_t i=0;i<sels.GetCount();i++)
		{
			int istep=sels[i]+step;
			//if(istep<tmpstep){istep=tmpstep;tmpstep++;}
			if(istep<0){break;}
			sel[istep]=true;
			sel.erase(sel.find(sels[i]));
			if(step!=-1){
				InsertRows(istep,1,GetDial(sels[i]),false);
				DeleteRow(sels[i],1);
			}else{
				SwapRows(sels[i],istep);
			}

		}
	}else
	{
		for (int i=sels.GetCount()-1;i>=0;i--)
		{
			int istep=sels[i]+step;
			//if(istep<0){istep=tmpstep;tmpstep++;}
			if(istep>GetCount()-1) break;
			sel[istep]=true;
			sel.erase(sel.find(sels[i]));
			if(step!=1){
				//break;
				Dialogue *dial=GetDial(sels[i]);
				DeleteRow(sels[i],1);
				InsertRows(istep,1,dial,false);
			}
			else{
				SwapRows(sels[i],istep);
			}
		}
	}
	Edit->SetIt(FirstSel());
	Refresh(false);
}

void SubsGrid::DeleteText()
{
	for (int i=0;i<GetCount();i++)
	{
		if(sel.find(i)!=sel.end()){
			CopyDial(i)->Text="";}
	}
	SetModified();
	Refresh(false);
}
void SubsGrid::UpdateUR(bool toolbar)
{
	bool undo=false, _redo=false;
	file->GetURStatus(&undo, &_redo);
	Kai->MenuBar->Enable(Undo,undo);
	Kai->MenuBar->Enable(Redo,_redo);
	if(toolbar){
		Kai->Toolbar->UpdateId(Undo,undo);
		Kai->Toolbar->UpdateId(Redo,_redo);
	}
}

void SubsGrid::GetUndo(bool redo)
{
	TabPanel *pan =Kai->GetTab();
	Freeze();
	if(redo){file->Redo();}else{file->Undo();}


	UpdateUR();

	/*if(!redo){Kai->MenuBar->Enable(ID_REDO1,true);Kai->Toolbar->UpdateId(ID_REDO1,true);}
	if(file->Iter()==file->maxx()){Kai->MenuBar->Enable(ID_REDO1,false);Kai->Toolbar->UpdateId(ID_REDO1,false);}


	if(file->Iter()){
	if(redo){Modified=true;Kai->MenuBar->Enable(ID_UNDO1,true);Kai->Toolbar->UpdateId(ID_UNDO1,true);}
	}else{Modified=false;Kai->MenuBar->Enable(ID_UNDO1,false);Kai->Toolbar->UpdateId(ID_UNDO1,false);}*/
	Kai->Label(file->Iter());


	char oldform=form;SetSubsForm();
	if(oldform!=form){
		pan->CTime->Contents();
		pan->Edit->HideControls();
		Kai->UpdateToolbar();
	}
	sel.clear();
	int erow=Edit->ebrow;if(erow>=GetCount()){erow=GetCount()-1;}
	sel[erow]=true;
	lastRow=erow;
	Thaw();

	if(Kai->ss){Kai->ss->ASS->SetArray(&file->subs->styles);Kai->ss->ASS->Refresh(false);}
	SpellErrors.clear();
	RepaintWindow();
	Edit->SetIt(erow);
	Edit->RefreshStyle();
	VideoCtrl *vb=pan->Video;
	if(vb->IsShown()){vb->OpenSubs(SaveText());}
	int opt=Options.GetInt("Move Video To Active Line");
	if(opt>1){
		if(vb->GetState()==Paused || (vb->GetState()==Playing && (opt==3 || opt==5))){
			vb->Seek(Edit->line->Start.mstime);}
	}else{
		if(vb->GetState()==Paused){vb->Render();}
	}


	if(makebkp){
		timer.Start(20000,true);
		//CreateTimerQueueTimer(&qtimer,0,WAITORTIMERCALLBACK(OnBcktimer),this,20000,0,WT_EXECUTEONLYONCE);
		makebkp=false;
	}
}

wxArrayInt SubsGrid::GetSels(bool deselect)
{
	wxArrayInt sels;
	for(std::map<int,bool>::iterator i=sel.begin();i!=sel.end();i++)
	{
		sels.Add(i->first);
	}
	if(deselect){sel.clear();}

	return sels;
}

//Uważaj na dodawanie do niszczarki, 
//bo brak dodania gdy trzeba to wycieki pamięci,
//a podwójne dodanie to krasz przy niszczeniu obiektu.
void SubsGrid::InsertRows(int Row, 
						  std::vector<Dialogue *> RowsTable,
						  bool AddToDestroy)
{
	file->subs->dials.insert(file->subs->dials.begin()+Row, RowsTable.begin(), RowsTable.end());
	wxArrayInt emptyarray;
	SpellErrors.insert(SpellErrors.begin()+Row,RowsTable.size(),emptyarray);
	if(AddToDestroy){file->subs->ddials.insert(file->subs->ddials.end(), RowsTable.begin(), RowsTable.end());}
}

//Uważaj na dodawanie do niszczarki, 
//bo brak dodania gdy trzeba to wycieki pamięci,
//a podwójne dodanie to krasz przy niszczeniu obiektu.
void SubsGrid::InsertRows(int Row, int NumRows, Dialogue *Dialog, bool AddToDestroy, bool Save)
{
	file->subs->dials.insert(file->subs->dials.begin()+Row,NumRows,Dialog);
	wxArrayInt emptyarray;
	SpellErrors.insert(SpellErrors.begin()+Row,NumRows,emptyarray);
	if(AddToDestroy){file->subs->ddials.push_back(Dialog);}
	if(Save){
		sel[Row]=true;
		SetModified();
		Refresh(false);
	}
}

void SubsGrid::SetSubsForm(wxString ext)
{
	form=ASS;
	int rw=0;
	char subsext=(ext=="ass"||ext=="ssa")? ASS: (ext=="srt")? SRT : TMP;
	while(rw<GetCount())
	{
		Dialogue *dial=GetDial(rw);
		if (dial->NonDial || dial->Form==0){rw++;}
		else if(!ext.empty() && (subsext!=dial->Form || (subsext==TMP && dial->Form>SRT))){rw++;}//form=dial->Form; 
		else{form=dial->Form; break;}
	}
}


void SubsGrid::AddSInfo(wxString SI, wxString val, bool save)
{
	wxString key;
	if(val==""){
		key=SI.BeforeFirst(':');
		key.Trim(false);
		key.Trim(true);
		val=SI.AfterFirst(':');
		val.Trim(false);
		val.Trim(true);}
	else{key=SI;}
	SInfo *oldinfo=NULL;
	int ii= -1;
	oldinfo = GetSInfoP(key,&ii);
	//wxLogStatus("id oi %i" + key ,oldinfo); 
	if(!oldinfo || save){
		oldinfo=new SInfo(key,val);
		if(ii<0){
			file->subs->sinfo.push_back(oldinfo);}
		else{
			file->subs->sinfo[ii]=oldinfo;}
		file->subs->dsinfo.push_back(oldinfo);
	}else{
		oldinfo->Val=val;
	}
	//if(save){

	//}
}

wxString SubsGrid::GetSInfos(bool tld)
{
	wxString TextSI="";
	for (std::vector<SInfo*>::iterator cur= file->subs->sinfo.begin();cur!=file->subs->sinfo.end();cur++) {
		if(!(tld&& (*cur)->Name.StartsWith("TLMode"))){
			TextSI<<(*cur)->Name << ": " << (*cur)->Val << "\r\n";
		}
	}
	return TextSI;
}

void SubsGrid::SetModified(bool redit, bool dummy, bool refvid)
{
	if(file->IsNotSaved()){
		if(file->Iter()<1||!Modified){
			Modified=true;
		}

		Kai->Label(file->Iter()+1);
		if(redit)
		{
			int erow=Edit->ebrow;
			if(erow>=GetCount()){erow=GetCount()-1;}
			lastRow=erow;
			if(scPos>erow){scPos=MAX(0,(erow-4));}
			Edit->SetIt(erow);
			sel[erow]=true;
		}
		file->SaveUndo();
		if(!dummy){
			VideoCtrl *vb=Kai->GetTab()->Video;
			if(vb->IsShown()){vb->OpenSubs(SaveText());}

			int opt=Options.GetInt("Move Video To Active Line");
			if(opt>1){
				if(vb->GetState()==Paused || (vb->GetState()==Playing && (opt==3 || opt==5))){
					vb->Seek(Edit->line->Start.mstime);}
			}else{
				if(vb->GetState()==Paused){vb->Render();}
			}
		}

		if(makebkp){
			timer.Start(20000, true);
			//CreateTimerQueueTimer(&qtimer,0,WAITORTIMERCALLBACK(OnBcktimer),this,20000,0,WT_EXECUTEONLYONCE);
			makebkp=false;
		}
		UpdateUR();
	}
}

void SubsGrid::SwapRows(int frst, int scnd, bool sav)
{

	Dialogue *tmp=file->subs->dials[frst];
	file->subs->dials[frst]=file->subs->dials[scnd];
	file->subs->dials[scnd]=tmp;
	wxArrayInt tmpspell=SpellErrors[frst];
	SpellErrors[frst]=SpellErrors[scnd];
	SpellErrors[scnd] = tmpspell;
	Refresh(false);
	if(sav){SetModified();}
}

void SubsGrid::Loadfile(wxString str,wxString ext){

	Clearing();
	int active=0;
	file=new SubsFile();

	if(ext=="srt"){

		wxStringTokenizer tokenizer(str,"\n",wxTOKEN_STRTOK);
		tokenizer.GetNextToken();

		wxString text1;
		while ( tokenizer.HasMoreTokens() ){
			wxString text=tokenizer.GetNextToken().Trim();
			if(IsNum(text)){if(text1!=""){
				AddLine(new Dialogue(text1.Trim())); text1="";}}
			else{text1<<text<<"\r\n";}
		}

		if(text1!=""){AddLine( new Dialogue(text1.Trim())); text1="";}
	}
	else{

		short sinfoo=0;
		char format=ASS;

		wxStringTokenizer tokenizer(str,"\n",wxTOKEN_STRTOK);


		while ( tokenizer.HasMoreTokens() && (ext=="ass"||ext=="ssa")){
			wxString token = tokenizer.GetNextToken().Trim(false);
			if(token.StartsWith("Style: "))
			{
				//1 = ASS, 2 = SSA, potrzebne tylko przy odczycie napisów.
				AddStyle(new Styles(token,format));
				sinfoo=2;
			}
			else if(token.StartsWith("Format:") && sinfoo==2){
				break;
			}
			else if(token.StartsWith("[V4")){
				if(!token.StartsWith("[V4+")){format=2;}//ze względu na to, że wywaliłem ssa z formatów
				//muszę posłać do konstruktora styli 2 jako format SSA, nie używam tu srt, 
				//żeby później źle tego nie zinterpretować
				sinfoo=1;
			}
			else if(!token.StartsWith(";")&&sinfoo==0&&token.Find(':')!=wxNOT_FOUND){
				AddSInfo(token);
			}
		}


		bool tlmode=(GetSInfo("TLMode")=="Yes");
		if(GetSInfo("Active Line")!="" &&(ext=="ass"||ext=="ssa")){active=wxAtoi(GetSInfo("Active Line"));}
		if(transl&&!tlmode){
			transl=false;
			showtl=false;
			Kai->MenuBar->Enable(SaveTranslation,false);
			Edit->SetTl(false);
		}
		wxString tlstyle;
		if(tlmode){tlstyle=GetSInfo("TLMode Style");if(tlstyle==""){tlmode=false;}}

		while ( tokenizer.HasMoreTokens() )
		{
			wxString token = tokenizer.GetNextToken();
			if(token.StartsWith("[Font")||token.StartsWith("[Attach")){break;}
			Dialogue *dl= new Dialogue(token);
			if(!tlmode){
				AddLine(dl);
			}
			else if(tlmode && dl->Style==tlstyle){
				wxString ntoken = tokenizer.GetNextToken();
				Dialogue tl(ntoken);
				dl->Style=tl.Style; 
				dl->TextTl=tl.Text;
				AddLine(dl);
			}else if(tlmode && dl->Text!=""){
				AddLine(dl);
			}else{delete dl;}
		}


	}

	if(GetCount()<1){LoadDefault();wxMessageBox(_("Niepoprawny format (plik uszkodzony lub zawiera błędy)"));form=ASS;}
	else{SetSubsForm();}
	origform=form;

	if(GetSInfo("TLMode")=="Yes"){
		Edit->SetTl(true);
		transl=true;
		if(GetSInfo("TLMode Showtl")=="Yes"){showtl=true;}
		Kai->MenuBar->Enable(SaveTranslation,true);
	}


	if(form==MDVD||form==MPL2){
		int endt=Options.GetInt("Time show of letter");
		for(int i=0;i<GetCount();i++){
			Dialogue *dial=GetDial(i);

			if(dial->End.mstime==0){
				int newend=(endt*dial->Text.Len());
				if(newend<1000){newend=1000;}
				newend+=dial->Start.mstime;
				dial->End.NewTime(newend);
				if(i<GetCount()-1){
					if(dial->End>file->subs->dials[i+1]->Start){
						dial->End=file->subs->dials[i+1]->Start;
					}
				}
			}
		}
	}
	else if(form==ASS){
		if(ext!="ass"){origform=0;AddStyle(new Styles());}
		Edit->TlMode->Enable(true);Edit->RefreshStyle();
		if(Options.GetBool("Grid Load Sorted")){std::sort(file->subs->dials.begin(), file->subs->dials.end(), sortstart);}}
	else{Edit->TlMode->Enable(false);}
	if(active>=GetCount()){active=0;}


	sel[active]=true;
	lastRow=active;
	mtimerow=active;

	scPos=MAX(0,active-3);
	RepaintWindow();

	Edit->SetIt(active,false,false);
	//wxLogMessage("setit");
	Edit->HideControls();
	//wxLogMessage("hide controls");
	//wxLogMessage("repaint");
	//AdjustWidths();
	//wxLogMessage("all");
	file->EndLoad();
	if(Kai->ss && form==ASS){Kai->ss->LoadAssStyles();}

}

void SubsGrid::SetStartTime(int stime)
{

	wxArrayInt sels=GetSels();
	for(size_t i=0;i<sels.size();i++){
		Dialogue *dialc=file->CopyDial(sels[i]);
		dialc->Start.NewTime(stime);
	}
	if(sels.size()){
		SetModified();
		Refresh(false);
	}
}

void SubsGrid::SetEndTime(int etime)
{
	wxArrayInt sels=GetSels();
	for(size_t i=0;i<sels.size();i++){
		Dialogue *dialc=file->CopyDial(sels[i]);
		dialc->End.NewTime(etime);
	}
	if(sels.size()){
		SetModified();
		Refresh(false);
	}
}

bool SubsGrid::SetTlMode(bool mode)
{
	if(mode){
		if(GetSInfo("TLMode")==""){
			//for(int i=0;i<GetCount();i++){file->subs->dials[i]->spells.clear();}
			
			int ssize=file->subs->styles.size();
			if(ssize>0){
				Styles *tlstyl=GetStyle(0,"Default")->Copy();
				for(int i =0; i < ssize; i++){
					wxString ns="TLmode";
					wxString nss=(i==0)?ns:ns<<i;
					if(FindStyle(nss)==-1){tlstyl->Name=nss; AddSInfo("TLMode Style", nss);break;}
				}
				tlstyl->Alignment="8";
				AddStyle(tlstyl);}

		}
		AddSInfo("TLMode", "Yes");
		transl=true;
		Kai->MenuBar->Enable(SaveTranslation,true);
		SpellErrors.clear();
		Refresh(false);

	}else{
		if(wxMessageBox(_("Czy na pewno chcesz wyłączyć tryb tłumaczenia?\nObcojęzyczny tekst przetłumaczonych linijek zostanie usunięty."),_("Potwierdzenie"),wxYES_NO)==wxNO)
		{
			return true;
		}

		int iinf=-1;
		GetSInfo("TLMode",&iinf);
		if(iinf>=0){
			file->subs->sinfo.erase(file->subs->sinfo.begin()+iinf);
		}
		iinf=-1;
		wxString vall=GetSInfo("TLMode Style",&iinf);
		if(iinf>=0){
			int g=FindStyle(vall);
			if(g>=0){DelStyle(g);}
			file->subs->sinfo.erase(file->subs->sinfo.begin()+iinf);
		}

		for(int i=0; i<GetCount(); i++)
		{
			if(file->subs->dials[i]->TextTl!="")
			{
				Dialogue *dial= GetDial(i);
				dial->Text = dial->TextTl;
				dial->TextTl="";
			}
			//file->subs->dials[i]->spells.clear();
		}

		transl=false;
		showtl=false;
		Kai->MenuBar->Enable(SaveTranslation,false);
	}
	Refresh(false);
	if(Notebook::GetTab()->Video->GetState()!=None){Notebook::GetTab()->Video->OpenSubs(SaveText());
	if(Notebook::GetTab()->Video->GetState()==Paused){Notebook::GetTab()->Video->Render();}}
	return false;
}

void SubsGrid::SelVideoLine(int curtime)
{
	if(Kai->GetTab()->Video->GetState()==None && curtime<0){return;}

	int time=(curtime<0)? Kai->GetTab()->Video->Tell() : curtime;
	int prevtime=0;
	int durtime=(curtime<0)? Kai->GetTab()->Video->GetDuration() : 36000000;
	int idr=0,ip=0;
	//wxLogMessage("time %i, durtime %i",time,durtime);
	for(int i =0;i<GetCount();i++)
	{
		Dialogue *dial = GetDial(i);
		if(!dial->IsComment && (dial->Text!="" || dial->TextTl!="")){
			if(time>= dial->Start.mstime&&time<=dial->End.mstime)
			{Edit->SetIt(i);SelectRow(i);ScrollTo(i-4);
			break;}
			if(dial->Start.mstime > prevtime && dial->Start.mstime < time){prevtime = dial->Start.mstime;ip=i;}
			if(dial->Start.mstime < durtime && dial->Start.mstime > time){durtime = dial->Start.mstime;idr=i;}

		}
		if(i==GetCount()-1){if((time-prevtime)>(durtime-time)){Edit->SetIt(idr);SelectRow(idr);ScrollTo(idr-4);}
		else{Edit->SetIt(ip);SelectRow(ip);ScrollTo(ip-4);}}
	}

}

void SubsGrid::NextLine(int dir)
{
	if(Edit->ABox&&Edit->ABox->audioDisplay->hold!=0){return;}
	int nebrow=Edit->ebrow+dir;
	if(nebrow<0){return;}
	if(nebrow>=GetCount()){
		Dialogue *tmp=new Dialogue(); 
		Dialogue *last=GetDial(GetCount()-1);
		int eend= last->End.mstime; 
		tmp->Start.NewTime(eend+1); 
		tmp->End.NewTime(eend+5000);
		tmp->Style=last->Style;
		tmp->State=1;
		AddLine(tmp);
		SetModified(false);
	}
	int h,w;
	GetClientSize(&w,&h);
	scPos = MID(0, nebrow-((h/(GridHeight+1))/2), GetCount()-1);
	sel.clear();
	sel[nebrow]=true;
	AdjustWidths(0);
	Refresh(false);Edit->SetIt(nebrow);
	if(Edit->ABox){Edit->ABox->audioDisplay->SetDialogue(Edit->line,nebrow);}
	if(Kai->GetTab()->Video->GetState()!=None&&Options.GetBool("Editbox Video Time")){
		Kai->GetTab()->Video->Seek(GetDial(nebrow)->Start.mstime-5);}
}

void SubsGrid::CheckText(wxString text, wxArrayInt &errs)
{
	if(!Kai->SC){Options.SetBool("Editbox Spellchecker",Kai->SpellcheckerOn());}
	if(Kai->SC){
		//Dialogue *dial=GetDial(rw);
		wxString notchar="/?<>|\\!@#$%^&*()_+=[]\t~ :;.,\"{}";
		//wxString text=(transl)? dial->TextTl : dial->Text;

		/*if (hideover){
		wxRegEx reg("\\{[^\\{]*\\}",wxRE_ADVANCED);
		reg.ReplaceAll(&text,"*");
		}*/

		text+=" ";
		bool block=false;
		wxString word="";
		//wxString deb;
		bool slash=false;
		int lasti=0;
		int firsti=0;
		for(size_t i = 0; i<text.Len();i++)
		{
			wxUniChar ch=text.GetChar(i);
			if(notchar.Find(ch)!=-1&&!block){if(word.Len()>1){
				if(word.StartsWith("'")){word=word.Remove(0,1);}
				if(word.EndsWith("'")){word=word.RemoveLast(1);}
				word.Trim(false);
				word.Trim(true);
				bool isgood=Kai->SC->CheckWord(word);
				if (!isgood){errs.push_back(firsti);errs.push_back(lasti);}
				//wxMessageBox(word);
			}word="";firsti=i+1;}
			if(ch=='{'){block=true;}
			else if(ch=='}'){block=false;firsti=i+1;word="";}


			if(notchar.Find(ch)==-1&&text.GetChar((i==0)? 0 : i-1)!='\\'&&!block){word<<ch;lasti=i;}
			else if(!block&&text.GetChar((i==0)? 0 : i-1)=='\\'){firsti=i+1;word="";}
		}
		if(errs.size()<2){errs.push_back(0);}
	}
}




void SubsGrid::LoadDefault(bool line,bool sav,bool endload)
{
	if(line)
	{
		AddLine(new Dialogue());
		AddStyle(new Styles());
		sel[0]=true;
		origform=form=ASS;
	}
	AddSInfo("Title","Kainote Ass File",sav);
	AddSInfo("PlayResX","1280",sav);
	AddSInfo("PlayResY","720",sav);
	AddSInfo("ScaledBorderAndShadow","yes",sav);
	AddSInfo("WrapStyle","0",sav);
	AddSInfo("ScriptType","v4.00+",sav);
	AddSInfo("Last Style Storage","Podstawowy",sav);
	if(endload){file->EndLoad();}
}

Dialogue *SubsGrid::CopyDial(int i, bool push)
{
	if(push && (int)SpellErrors.size() > i){SpellErrors[i].Clear();}
	return file->CopyDial(i, push);
}

Dialogue *SubsGrid::GetDial(int i)
{
	return file->subs->dials[i];
}

wxString SubsGrid::GetSInfo(wxString key, int *ii)
{
	int i=0;
	for(std::vector<SInfo*>::iterator it=file->subs->sinfo.begin(); it!=file->subs->sinfo.end(); it++)
	{
		if(key==(*it)->Name) {if(ii){*ii=i;} return (*it)->Val;}
		i++;
	}
	return "";
}

SInfo *SubsGrid::GetSInfoP(wxString key,int *ii)
{
	int i=0;
	for(std::vector<SInfo*>::iterator it=file->subs->sinfo.begin(); it!=file->subs->sinfo.end(); it++)
	{
		if(key==(*it)->Name) {if(ii){*ii=i;};return (*it);}
		i++;
	}
	*ii=-1;
	return NULL;
}

int SubsGrid::SInfoSize()
{
	return file->subs->sinfo.size();
}

wxString *SubsGrid::SaveText()
{
	wxString *txt=new wxString();

	if (form<SRT){

		(*txt)<<"[Script Info]\r\n"<<GetSInfos(GetSInfo("TLMode")=="Translated");
		(*txt)<<"\r\n[V4+ Styles]\r\nFormat: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding \r\n";
		(*txt)<<GetStyles(GetSInfo("TLMode")=="Translated");
		(*txt)<<" \r\n[Events]\r\nFormat: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\r\n";
	}

	bool tlmode=(GetSInfo("TLMode")=="Yes");
	wxString tlstyle=GetSInfo("TLMode Style");

	for(int i=0;i<GetCount();i++)
	{
		Dialogue *dial=GetDial(i);
		if(i==Edit->ebrow){Edit->Send(false,true);dial=Edit->line;}
		if(tlmode && dial->TextTl!=""){
			(*txt)<<dial->GetRaw(false,tlstyle);
			(*txt)<<dial->GetRaw(true);

		}else{
			wxString wynik;
			if(form==SRT){wynik<<i+1<<"\r\n";}
			(*txt)<<wynik+dial->GetRaw();
		}

	}

	return txt;
}

wxString *SubsGrid::GetVisible(wxPoint *EBText, bool *visible)
{
	TabPanel *pan=(TabPanel*)GetParent();
	int _time=pan->Video->Tell();
	wxString *txt=new wxString();

	wxArrayInt Lines;

	if(_time>=GetDial(Edit->ebrow)->Start.mstime&&_time <= GetDial(Edit->ebrow)->End.mstime)
	{
		Edit->Send(false,true);
		*visible=true;
	}else{
		*visible=false;
	}

	//wxLogStatus("lines %i %i %i", _time, file->dial[Edit->ebrow].Start.mstime, file->dial[Edit->ebrow].End.mstime);
	for(int i=0; i<GetCount(); i++){
		Dialogue *dial=GetDial(i);
		if(_time >= dial->Start.mstime && _time <= dial->End.mstime){
			Lines.Add(i);
		}

	}



	(*txt)<<"[Script Info]\r\n"<<GetSInfos(false);
	(*txt)<<"\r\n[V4+ Styles]\r\nFormat: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding \r\n";
	(*txt)<<GetStyles(false);
	(*txt)<<" \r\n[Events]\r\nFormat: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\r\n";

	for(size_t i=0;i<Lines.size();i++)
	{
		Dialogue *dial= (Lines[i]==Edit->ebrow)? Edit->line : GetDial(Lines[i]);
		if(GetSInfo("TLMode")=="Yes" && dial->TextTl!=""){
			(*txt)<<dial->GetRaw(false,GetSInfo("TLMode Style"));
			(*txt)<<dial->GetRaw(true);

		}else{
			(*txt)<<dial->GetRaw();}
		if(Lines[i]==Edit->ebrow){
			int all= txt->Len();EBText->y=all-2;
			all-= (GetSInfo("TLMode")=="Yes" && dial->TextTl!="")? 
				dial->TextTl.Len() : dial->Text.Len();
			EBText->x=all-2;
		}

	}
	if(Lines.empty()){
		Dialogue *dial=Edit->line;
		if(GetSInfo("TLMode")=="Yes" && dial->TextTl!=""){
			(*txt)<<dial->GetRaw(false,GetSInfo("TLMode Style"));
			(*txt)<<dial->GetRaw(true);

		}else{
			(*txt)<<dial->GetRaw();
		}
		int all= txt->Len();EBText->y=all-2;
		all-= (GetSInfo("TLMode")=="Yes" && dial->TextTl!="")? 
			dial->TextTl.Len() : dial->Text.Len();
		EBText->x=all-2;
	}
	//wxLogMessage(txt);
	return txt;
}


//wxRect SubsGrid::GetMetrics(int line)
//{
//	Dialogue *dial = GetDial(Edit->ebrow);
//	Styles *styl=GetStyle(0,dial->Style);
//	wxString tags[]={"fscx","fscy","fsp","fs"};
//	int vals[]={wxAtoi(styl->ScaleX),wxAtoi(styl->ScaleY),wxAtoi(styl->Spacing),wxAtoi(styl->Fontsize)};
//	wxString tag, rest;
//	
//	wxStringTokenizer ktok(dial->Text,"\\}",wxTOKEN_STRTOK);
//	while(ktok.HasMoreTokens()){
//		wxString text=ktok.NextToken();
//		for(int i=0;i<4;i++){
//			if(text.StartsWith(tags[i],&rest)){
//				vals[i]=wxAtoi(rest);
//				break;
//			}
//		}
//	}
//	wxString txt=(!showtl && transl && dial->TextTl!="")? dial->TextTl : dial->Text;
//	double posx=0, posy=0, posx1=-1, posy1=-1, t1=-1, t2=-1;
//	int an=2;
//	bool haspos=false;
//	wxRegEx regan("\\\\an([0-9]"),wxRE_ADVANCED|wxRE_ICASE);
//	wxRegEx reg("\\\\pos\\(([^,]*\\,([^\\)]*)"),wxRE_ADVANCED|wxRE_ICASE);
//	wxRegEx regm("\\\\move\\(([^,]*\\,([^,]*)\\,([^,]*)\\,([^\\)]*)"),wxRE_ADVANCED|wxRE_ICASE);
//	if(regan.Matches(txt)){
//		an = wxAtoi(regan.GetMatch(txt,1));
//	}else{an = wxAtoi(styl->Alignment);}
//
//	if(reg.Matches(txt)){
//		if(reg.GetMatch(txt,1).ToDouble(&posx) && reg.GetMatch(txt,2).ToDouble(&posy)){haspos=true;}
//	}else if(regm.Matches(txt)){
//		if(reg.GetMatch(txt,1).ToDouble(&posx) && reg.GetMatch(txt,2).ToDouble(&posy) 
//			&& reg.GetMatch(txt,3).ToDouble(&posx1) ){haspos=true;}
//		wxString lastmatch=reg.GetMatch(txt,4);
//		if(lastmatch.Find(',')==-1){
//			if(!lastmatch.ToDouble(&posy1)){haspos=false;}
//		}else{
//			wxString rest1, rest2;
//			wxString posy1s = lastmatch.BeforeFirst(',',&rest1);
//			wxString t1s = rest1.BeforeFirst(',',&rest2);
//			wxString t2s = rest2;
//			if(!posy1s.ToDouble(&posy1) || !t1s.ToDouble(&t1) || !t2s.ToDouble(&t2)){haspos=false;}
//		}
//	}
//
//	wxRegEx regst("\\{[^\\{]*\\}",wxRE_ADVANCED);
//	regst.ReplaceAll(&txt,"");
//	wxRect rc(-1,-1,-1,-1);
//	double width = 0, height =0, descent =0, extlead=0;
//	double fontsize = vals[3]*32;
//	double spacing = vals[2]*32;
//		
//	//wxLogStatus(txt);
//
//	HDC thedc = CreateCompatibleDC(0);
//	if (!thedc) return rc;
//	SetMapMode(thedc, MM_TEXT);
//
//	LOGFONTW lf;
//	ZeroMemory(&lf, sizeof(lf));
//	lf.lfHeight = (LONG)fontsize;
//	lf.lfWeight = styl->Bold ? FW_BOLD : FW_NORMAL;
//	lf.lfItalic = styl->Italic;
//	lf.lfUnderline = styl->Underline;
//	lf.lfStrikeOut = styl->StrikeOut;
//	lf.lfCharSet = wxAtoi(styl->Encoding);
//	lf.lfOutPrecision = OUT_TT_PRECIS;
//	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
//	lf.lfQuality = ANTIALIASED_QUALITY;
//	lf.lfPitchAndFamily = DEFAULT_PITCH|FF_DONTCARE;
//	_tcsncpy(lf.lfFaceName, styl->Fontname.c_str(), 32);
//
//	HFONT thefont = CreateFontIndirect(&lf);
//	if (!thefont) return rc;
//	SelectObject(thedc, thefont);
//		
//	SIZE sz;
//	wxString text;
//	while(1){
//		int find = txt.find("\\N");
//		if(find!=-1){text=txt.substr(0,find).Trim();txt=txt.Mid(find+2).Trim(false);}
//		else{text=txt;}
//		size_t thetextlen = text.length();
//		const TCHAR *thetext = text.wc_str();
//	
//		GetTextExtentPoint32(thedc, thetext, (int)thetextlen, &sz);
//		
//		if(spacing){
//			sz.cx+=(spacing*thetextlen);
//		}
//		if((double)sz.cx>width){
//			width = sz.cx;
//		}
//		height += sz.cy;
//		if(find==-1){break;}
//	}
//
//
//	TEXTMETRIC tm;
//	GetTextMetrics(thedc, &tm);
//	descent = tm.tmDescent;
//	extlead= tm.tmExternalLeading;
//
//	DeleteObject(thedc);
//	DeleteObject(thefont);
//	
//	width = (vals[0] / 100.0) * (width/32);
//	height = (vals[1] / 100.0) * (height/32);
//	descent = (vals[1] / 100.0) * (descent/32);
//	extlead = (vals[1] / 100.0) * (extlead/32);
//	//wxLogStatus("vals %i %i %i %i", vals[0],vals[1], (int)width, an);
//	
//	int resx=0, resy=0;
//	GetASSRes(&resx, &resy);
//	
//	
//	if(!haspos){
//		
//		posy= (dial->MarginV>0)? dial->MarginV : wxAtoi(styl->MarginV);
//		if(an<4){
//			posy= resy-posy-height;
//		}else if(an<7){
//			posy= (resy/2)-(height/2);
//		}
//		if(an % 3 == 1){
//			posx= (dial->MarginL>0)? dial->MarginL : wxAtoi(styl->MarginL);
//		}else if(an % 3 == 2){
//			posx= (resx/2)-(width/2);
//		}else{
//			posx= (dial->MarginR>0)? dial->MarginR : wxAtoi(styl->MarginR);
//			posx= resx-posx-width;
//		}
//	}else{
//		if(an<4){
//			posy -= height;
//		}else if(an<7){
//			posy -= (height/2);
//		}
//		if(an % 3 == 0){
//			posx -= width;
//		}else if(an % 3 == 2){
//			posx -= (width/2);
//		}
//	}
//	int w,h;
//	Notebook::GetTab()->Video->GetClientSize(&w,&h);
//	float wspx=(float)(w-1)/(float)resx;
//	float wspy=(float)(h-45)/(float)resy;
//	//wxLogStatus("vidsize %f %f", wspx, wspy);
//	height = height + (descent +extlead);
//	rc.width= width;// *wspx;
//	rc.height= height;// *wspy;
//	rc.x=posx;//*wspx;
//	rc.y=posy;//*wspy;
//	return rc;
//}


void SubsGrid::OnBcktimer(wxTimerEvent &event)
{
	TabPanel *pan=(TabPanel*)GetParent();
	wxLogStatus(_("Autozapis"));
	wxString path;
	wxString ext=(form<SRT)? "ass" : (form=SRT)? "srt" : "txt";

	path<<Options.pathfull<<"\\Subs\\"<<pan->SubsName.BeforeLast('.')
		<<"_"<<Notebook::GetTabs()->FindPanel(pan)<<"_"<< numsave <<"."<<ext;

	SaveFile(path, false);


	numsave = !numsave;
	makebkp=true;
	wxLogStatus("");
}

void SubsGrid::GetASSRes(int *x,int *y)
{
	wxString oldx=GetSInfo("PlayResX");
	wxString oldy=GetSInfo("PlayResY");
	int nx=wxAtoi(oldx);
	int ny=wxAtoi(oldy);
	if(nx<1 && ny<1){nx=384;ny=288;}
	else if(nx<1){nx=(float)ny*(4.0/3.0);if(ny==1024){nx=1280;}}
	else if(ny<1){ny=(float)nx*(3.0/4.0);if(nx==1280){ny=1024;}}
	*x=nx;
	*y=ny;
}

int SubsGrid::CalcChars(wxString txt, wxString *lines, bool *bad)
{
	int len=txt.Len();
	bool block=false;bool slash=false;
	int chars=0, lastchars=0, ns=0;
	wxUniChar brs=(form==SRT)? '<' : '{';
	wxUniChar bre=(form==SRT)? '>' : '}';
	for(int i=0; i<len; i++)
	{
		if(txt[i]==brs){block=true;}
		else if(txt[i]==bre){block=false;}
		else if(txt[i]=='\\' && !block){slash=true; continue;}
		else if(slash){
			if(txt[i]=='N'){
				if(lines){
					ns++;
					int linechars=(chars-lastchars);
					if(!(*bad)){*bad=(linechars>43 || ns>1);}
					(*lines)<<linechars<<"/";
					lastchars=chars;
				}
			}else if(txt[i]!='h'){chars+=2;}
			slash=false;
		}
		else if(!block && txt[i]!=' '){chars++;}

	}
	if(lines){
		int linechars=(chars-lastchars);
		if(!(*bad)){*bad=(linechars>43 || ns>1);}
		(*lines)<<linechars<<"/";
	}

	//wxLogStatus("%i", chars);
	return chars;
}

BEGIN_EVENT_TABLE(SubsGrid,wxWindow)
	EVT_PAINT(SubsGrid::OnPaint)
	EVT_SIZE(SubsGrid::OnSize)
	EVT_SCROLLWIN(SubsGrid::OnScroll)
	EVT_MOUSE_EVENTS(SubsGrid::OnMouseEvent)
	EVT_KEY_DOWN(SubsGrid::OnKeyPress)
	EVT_TIMER(ID_AUTIMER,SubsGrid::OnBcktimer)
	END_EVENT_TABLE()

