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


#include "SubsGrid.h"
#include "config.h"
#include "EditBox.h"

#include "kainoteMain.h"
#include "OpennWrite.h"
#include "OptionsDialog.h"
#include "Tabs.h"
#include <wx/tokenzr.h>
#include <wx/event.h>
#include <algorithm>
#include <wx/regex.h>
#include <wx/ffile.h>
#include "KaiMessageBox.h"
#include <thread>

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

bool SubsGrid::IsNum(const wxString &test) {
	bool isnumber=true;
	wxString testchars="0123456789";
	for(size_t i=0;i<test.Len();i++){
		wxUniChar ch=test.GetChar(i);
		if(testchars.Find(ch)==-1){isnumber=false;break;}
	}
	return isnumber;
}



SubsGrid::SubsGrid(wxWindow *parent, const long int id,const wxPoint& pos,const wxSize& size, long style, const wxString& name)
	: KaiScrolledWindow(parent, id, pos, size, style|wxVERTICAL, name)
{
	posY=0;
	posX=0;
	row=markedLine=lastActiveLine=0;
	scPos=0;
	scHor=0;
	oldX=-1;
	visible=Options.GetInt(GridHideCollums);
	holding = false;
	hideover=Options.GetBool(GridHideTags);
	Modified=false;
	first=true;
	makebkp=true;
	extendRow=-1;
	lastRow=0;
	lastsel=-1;
	transl=false;
	showtl=false;
	ismenushown=false;
	showFrames=false;
	Comparsion=NULL;
	bmp=NULL;
	numsave=0;
	file=new SubsFile();


	LoadDefault();
	timer.SetOwner(this,ID_AUTIMER);
	nullifyTimer.SetOwner(this,27890);
	Bind(wxEVT_TIMER,[=](wxTimerEvent &evt){
		Kai->SetStatusText("",0);
	},27890);
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
	wxString fontname = Options.GetString(GridFontName);
	font.SetFaceName(fontname);
	if (!font.IsOk())
		font.SetFamily(wxFONTFAMILY_SWISS );
	font.SetWeight(wxFONTWEIGHT_NORMAL);
	font.SetPointSize(Options.GetInt(GridFontSize));

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

	int w = 0;
	int h = 0;
	GetClientSize(&w,&h);
	bool bg=false;
	int size=GetCount();
	panelrows=(h/(GridHeight+1))+1;
	int scrows=scPos+panelrows;
	//gdy widzimy koniec napisów
	if(scrows >= size + 3){
		bg=true;
		scrows = size + 1;
		scPos=(scrows-panelrows)+2;// dojechanie do końca napisów
		if(panelrows > size + 3){scPos=0;}// w przypadku gdy całe napisy są widoczne, wtedy nie skrollujemy i pozycja =0
	}
	else if(scrows >= size + 2){
		bg=true;
		scrows--;//w przypadku gdy mamy linię przed końcem napisów musimy zaniżyć wynik bo przekroczy tablicę.
	}
	if(SetScrollBar(wxVERTICAL,scPos,panelrows, size + 3, panelrows-3)){
		GetClientSize(&w,&h);
	}

	// Prepare bitmap
	if (bmp) {
		if (bmp->GetWidth() < w+scHor || bmp->GetHeight() < h) {
			delete bmp;
			bmp = NULL;
		}
	}
	if (!bmp) bmp = new wxBitmap(w+scHor,h);

	// Draw bitmap
	wxMemoryDC tdc;
	tdc.SelectObject(*bmp);
	int firstCol = GridWidth[0]+1;



	tdc.SetFont(font);
	//wxMemoryDC tdc;
	//wxBitmap tbmp(w+scHor,h);
	//tdc.SelectObject(tbmp);
	//tdc.SetFont(font);
	wxColour header = Options.GetColour(GridHeader);
	wxColour headerText = Options.GetColour(GridHeaderText);
	wxColour labelBkCol=Options.GetColour(GridLabelSaved);
	wxColour labelBkColN=Options.GetColour(GridLabelNormal);
	wxColour labelBkColM=Options.GetColour(GridLabelModified);
	wxColour linesCol=Options.GetColour(GridLines);
	wxColour subsBkCol=Options.GetColour(GridDialogue);
	wxColour comm=Options.GetColour(GridComment);
	wxColour seldial=Options.GetColour(GridSelectedDialogue);
	wxColour selcom=Options.GetColour(GridSelectedComment);
	wxColour textcol=Options.GetColour(GridText);
	wxColour collcol=Options.GetColour(GridCollisions);
	wxColour SpelcheckerCol=Options.GetColour(GridSpellchecker);
	wxColour ComparsionCol=Options.GetColour(GridComparison);
	wxColour ComparsionBGCol=Options.GetColour(GridComparisonBackground);
	wxColour ComparsionBGSelCol=Options.GetColour(GridComparisonBackgroundSelected);
	wxColour ComparsionBGCmntCol=Options.GetColour(GridComparisonCommentBackground);
	wxColour ComparsionBGCmntSelCol=Options.GetColour(GridComparisonCommentBackgroundSelected);
	wxString chtag=Options.GetString(GridTagsSwapChar);
	wxColour visibleOnVideo = Options.GetColour(GridVisibleOnVideo);
	bool SpellCheckerOn = Options.GetBool(SpellcheckerOn);

	tdc.SetPen(*wxTRANSPARENT_PEN);
	tdc.SetBrush(wxBrush(linesCol));
	tdc.DrawRectangle(0,0,w+scHor/*-(GridWidth[0]+1)*/,h);

	int ilcol;
	posY=0;

	bool isComment=false;
	bool unkstyle=false;
	bool shorttime=false;
	int states=0;

	if(SpellErrors.size()<(size_t)size){
		SpellErrors.resize(size);
	}

	Dialogue *acdial=GetDial(MID(0,Edit->ebrow,size-1));
	Dialogue *Dial;
	TabPanel *tab = (TabPanel*)GetParent();
	int VideoPos = tab->Video->Tell();

	int fw,fh,bfw,bfh;
	wxColour kol;
	visibleLines.clear();
	

	for(int i=scPos;i<scrows;i++){

		wxArrayString strings;
		bool comparison = false;
		bool isSelected = false;

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
			kol = header;
		}else{
			Dial=GetDial(i-1);

			strings.Add(wxString::Format("%i",i));

			isComment=Dial->IsComment;
			states=Dial->State;
			if (form<SRT){
				strings.Add(wxString::Format("%i",Dial->Layer));
			}
			
			if(showFrames && tab->Video->VFF){
				VideoFfmpeg *VFF = tab->Video->VFF;
				wxString frame;
				frame << VFF->GetFramefromMS(Dial->Start.mstime);
				strings.Add(frame);
				if (form!=TMP){
					frame="";
					frame << VFF->GetFramefromMS(Dial->End.mstime)-1;
					strings.Add(frame);
				}
			}else{
				strings.Add(Dial->Start.raw(form));
				if (form!=TMP){strings.Add(Dial->End.raw(form));}
			}
			
			if (form<SRT){
				if(FindStyle(Dial->Style)==-1){unkstyle=true;}else{unkstyle=false;}
				strings.Add(Dial->Style);
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

			if(SpellCheckerOn && (!transl && Dial->Text!="" || transl && Dial->TextTl!="")){
				if(SpellErrors[i-1].size()<2){
					CheckText(strings[strings.size()-1],SpellErrors[i-1]);
				}
			} 
			if(sel.find(i-1) != sel.end()){
				isSelected = true;
			}
			bool comparison = (Comparsion && Comparsion->at(i-1).size()>0);//visibleLines
			bool visibleLine = (Dial->Start.mstime <= VideoPos && Dial->End.mstime > VideoPos);
			kol = (comparison)? ComparsionBGCol : 
				(visibleLine)? visibleOnVideo : 
				subsBkCol;
			if(isComment){kol= (comparison)? ComparsionBGCmntCol : comm;}
			if(isSelected){
				if(isComment){kol = (comparison)? ComparsionBGCmntSelCol : selcom;}
				else{kol= (comparison)? ComparsionBGSelCol : seldial; }
			}
			if(visibleLine){visibleLines.push_back(true);}
			else{visibleLines.push_back(false);}
		}

		posX=0;


		ilcol=strings.GetCount();
		

		wxRect cur;
		bool isCenter;
		wxColour label= (states == 0)? labelBkColN : (states == 2)? labelBkCol : labelBkColM;
		for (int j=0; j<ilcol; j++){
			if(showtl&&j==ilcol-2){
				int podz=(w + scHor - posX) / 2;
				GridWidth[j]=podz;
				GridWidth[j+1]=podz;
			}

			if(!showtl&&j==ilcol-1){GridWidth[j] = w + scHor - posX/* - (GridWidth[0] + 1)*/;}
			

			if(GridWidth[j]>0){
				tdc.SetPen(*wxTRANSPARENT_PEN);
				
				tdc.SetBrush(wxBrush((j==0 && i!=scPos)? label : kol));
				if(unkstyle && j==4 || shorttime && (j==10||(j==3 && form>ASS))){
					tdc.SetBrush(wxBrush(SpelcheckerCol));
				}

				tdc.DrawRectangle(posX,posY,GridWidth[j],GridHeight);

				if(i!=scPos && j==ilcol-1){

					if(SpellErrors[i-1].size()>2){
						tdc.SetBrush(wxBrush(SpelcheckerCol));
						for(size_t k = 1; k < SpellErrors[i-1].size(); k+=2){

							wxString err=strings[j].SubString(SpellErrors[i-1][k], SpellErrors[i-1][k+1]);
							err.Trim();
							if(SpellErrors[i-1][k]>0){
								wxString berr=strings[j].Mid(0, SpellErrors[i-1][k]);
								tdc.GetTextExtent(berr, &bfw, &bfh, NULL, NULL, &font);
							}else{bfw=0;}

							tdc.GetTextExtent(err, &fw, &fh, NULL, NULL, &font);
							tdc.DrawRectangle(posX+bfw+3,posY,fw,GridHeight);
						}
					}


					if(comparison){
						tdc.SetTextForeground(ComparsionCol);

						for(size_t k = 1; k < Comparsion->at(i-1).size(); k+=2){
							//if(Comparsion->at(i-1)[k]==Comparsion->at(i-1)[k+1]){continue;}
							wxString cmp=strings[j].SubString(Comparsion->at(i-1)[k], Comparsion->at(i-1)[k+1]);

							if(cmp==""){continue;}
							if(cmp==" "){cmp="_";}
							wxString bcmp;
							if(Comparsion->at(i-1)[k]>0){
								bcmp=strings[j].Mid(0, Comparsion->at(i-1)[k]);
								tdc.GetTextExtent(bcmp, &bfw, &bfh, NULL, NULL, &font);
							}else{bfw=0;}

							tdc.GetTextExtent(cmp, &fw, &fh, NULL, NULL, &font);
							if((cmp.StartsWith("T") || cmp.StartsWith("Y") || cmp.StartsWith(L"Ł"))){bfw++;}

							tdc.DrawText(cmp,posX+bfw+2,posY);
							tdc.DrawText(cmp,posX+bfw+4,posY);
							tdc.DrawText(cmp,posX+bfw+2,posY+2);
							tdc.DrawText(cmp,posX+bfw+4,posY+2);
						}

					}

				}


				bool collis=(i!=scPos && i!=Edit->ebrow+1 && 
					(Dial->Start >= acdial->Start && Dial->Start < acdial->End || 
					Dial->End > acdial->Start && Dial->Start <= acdial->End)); 

				if(form<SRT){isCenter=!(j == 4 || j == 5 || j == 9 || j == 11 || j == 12);}
				else if(form==TMP){isCenter=!(j == 2);}
				else{isCenter=!(j == 4);}

				tdc.SetTextForeground((i == scPos)? headerText : (collis)? collcol : textcol);
				if(j==ilcol-1 && (strings[j].StartsWith("T") || strings[j].StartsWith("Y") || strings[j].StartsWith(L"Ł"))){posX++;}
				cur = wxRect(posX+3,posY,GridWidth[j]-6,GridHeight);
				tdc.SetClippingRegion(cur);
				tdc.DrawLabel(strings[j], cur, isCenter ? wxALIGN_CENTER : (wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT));
				tdc.DestroyClippingRegion();

				posX+=GridWidth[j]+1;

			}
		}

		posY+=GridHeight+1;

	}


	if(bg){
		tdc.SetPen(*wxTRANSPARENT_PEN);
		tdc.SetBrush(wxBrush(Options.GetColour(GridBackground)));
		tdc.DrawRectangle(0,posY,w+scHor,h);
	}
	if(markedLine>=scPos&&markedLine<=scrows){
		tdc.SetBrush(*wxTRANSPARENT_BRUSH);
		tdc.SetPen(wxPen(Options.GetColour(GridActiveLine),3));
		tdc.DrawRectangle(1,((markedLine-scPos+1)*(GridHeight+1))-1,(GridWidth[0]-1),GridHeight+2);
	}

	if(Edit->ebrow>=scPos&&Edit->ebrow<=scrows){
		tdc.SetBrush(*wxTRANSPARENT_BRUSH);
		tdc.SetPen(wxPen(Options.GetColour(GridActiveLine)));
		tdc.DrawRectangle(scHor,((Edit->ebrow-scPos+1)*(GridHeight+1))-1,w+scHor,GridHeight+2);
	}

	wxPaintDC dc(this);
	dc.Blit(0,0,firstCol,h,&tdc,0,0);
	dc.Blit(firstCol,0,w+scHor,h,&tdc,scHor+firstCol,0);
}

void SubsGrid::AdjustWidths(int cell)
{

	wxClientDC dc(this);
	dc.SetFont(font);

	int law=0,startMax=0,endMax=0,stw=0,edw=0,syw=0,acw=0,efw=0,fw=0,fh=0;
	bool shml=false,shmr=false,shmv=false;


	int maxx=GetCount();

	dc.GetTextExtent(wxString::Format("%i",GetCount()), &fw, &fh, NULL, NULL, &font);
	GridWidth[0]=fw+10;
	Dialogue *ndial;
	for(int i=0;i<maxx;i++){
		ndial=GetDialCor(i);
		if(START & cell){
			if(ndial->Start.mstime > startMax){startMax = ndial->Start.mstime;}
		}
		if((END & cell) && form!=TMP){
			if(ndial->End.mstime > endMax){endMax = ndial->End.mstime;}
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

	if(START & cell){
		STime start(startMax);
		if(showFrames){
			VideoFfmpeg *VFF = ((TabPanel*)GetParent())->Video->VFF;
			start.orgframe = VFF->GetFramefromMS(start.mstime);
		}
		dc.GetTextExtent(start.raw(showFrames? FRAME : form), &fw, &fh, NULL, NULL, &font);
		stw=fw+10;
	}
	if(END & cell){
		STime end(endMax);
		if(showFrames){
			VideoFfmpeg *VFF = ((TabPanel*)GetParent())->Video->VFF;
			end.orgframe = VFF->GetFramefromMS(end.mstime);
		}
		dc.GetTextExtent(end.raw(showFrames? FRAME : form), &fw, &fh, NULL, NULL, &font);
		edw=fw+10;
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
			dc.GetTextExtent(_("Aktor"), &fw, &fh, NULL, NULL, &font);
			if(fw+10>acw&&acw!=0){acw=fw+10;};
			GridWidth[5] = (acw==0)?0:acw;
		}

		if(224 & cell){
			dc.GetTextExtent(_("M.Pi."), &fw, &fh, NULL, NULL, &font);
			if(MARGINL & cell){GridWidth[6]=(!shml)?0:fw+10;}
			if(MARGINR & cell){GridWidth[7]=(!shmr)?0:fw+10;}
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
	SAFE_DELETE(Comparsion);
	SAFE_DELETE(file);
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
	if(sels.size()<2){
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
	if(selline){
		Edit->SetLine(lastRow,true,true,false,true);
		/*VideoCtrl *vb = ((TabPanel*)GetParent())->Video;
		if(vb->vToolbar->videoSeekAfter->GetSelection()==1 && vb->vToolbar->videoPlayAfter->GetSelection()<2){
			if(vb->GetState()!=None){
				if(vb->GetState()==Playing){vb->Pause();}
				vb->Seek(Edit->line->Start.mstime);
			}
		}*/
	}
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
		dial->Text=what->Text;}
	if(wcell & COMMENT){
		dial->IsComment=what->IsComment;}
	if(wcell & TXTTL){
		dial->TextTl=what->TextTl;
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
	else{
		newPos = event.GetPosition();
	}
	//wxLogStatus("scroll %i %i", newPos, scPos);
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
	//done:
	if(Edit->Visual==CHANGEPOS){
		Kai->GetTab()->Video->SetVisual();
		Kai->GetTab()->Video->Render();
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

void SubsGrid::SetVideoLineTime(wxMouseEvent &evt)
{
	TabPanel *tab=(TabPanel*)GetParent();
	if(tab->Video->GetState()!=None){
		if(tab->Video->GetState()!=Paused){
			if(tab->Video->GetState()==Stopped){tab->Video->Play();}
			tab->Video->Pause();
		}
		short wh=(form<SRT)?2:1;
		int whh=2;
		for(int i = 0;i<=wh;i++){whh+=GridWidth[i];}
		whh-=scHor;
		bool isstart;
		int vczas;
		bool getEndTime = evt.GetX()>=whh && evt.GetX()<whh+GridWidth[wh+1] && form!=TMP;
		if(getEndTime){ 
			vczas=Edit->line->End.mstime; isstart=false;}
		else{
			vczas=Edit->line->Start.mstime; isstart=true;
		}
		if(evt.LeftDClick() && evt.ControlDown()){vczas-=1000;}
		tab->Video->Seek(MAX(0,vczas),isstart,true,false);
		if(Edit->ABox){Edit->ABox->audioDisplay->Update(getEndTime);}
	}
}

void SubsGrid::OnMouseEvent(wxMouseEvent &event) {

	int w,h;
	GetClientSize (&w, &h);



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
	if(click && curX<=GridWidth[0] ){
		TabPanel *tab=(TabPanel*)GetParent();
		if(tab->Video->GetState()!=None && !(row < scPos || row >= GetCount())){
			if(tab->Video->GetState()!=Paused){
				if(tab->Video->GetState()==Stopped){tab->Video->Play();}
				tab->Video->Pause();
			}
			int vtime=0;
			bool isstart=true;
			if(shift && form!=TMP){ 
				vtime=GetDial(row)->End.mstime; isstart=false;}
			else{
				vtime=GetDial(row)->Start.mstime; isstart=true;
			}
			if(ctrl){vtime-=1000;}
			tab->Video->Seek(MAX(0,vtime),isstart,true,false);
			if(Edit->ABox){Edit->ABox->audioDisplay->Update(shift && form!=TMP);}
		}
		return;
	}
	if (right && !ctrl) {
		if(curX<=GridWidth[0]){
			markedLine=row;
			Refresh(false);

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
	TabPanel *pan=(TabPanel*)GetParent();
	bool changeActive = Options.GetBool(GridChangeActiveOnSelection);
	int mvtal= pan->Video->vToolbar->videoSeekAfter->GetSelection();//
	int pas=pan->Video->vToolbar->videoPlayAfter->GetSelection();
	if (!(row < scPos || row >= GetCount())) {

		if(holding && alt && lastsel!=row)
		{
			if (lastsel != -1) {
				file->edited=true;
				MoveRows(row-lastsel);
			}
			lastsel=row;
			//return;
		}


		// Toggle selected
		if (left_up && ctrl && !shift && !alt ) {
			if(lastActiveLine != row){
				SelectRow(row,true,!(sel.find(row)!=sel.end()));
				return;
			}
			
		}


		// Normal click
		if (!shift && !alt) {


			//jakbym chciał znów dać zmianę edytowanej linii z ctrl to muszę dorobić mu refresh
			if (click && (changeActive || !ctrl) || (dclick && ctrl)) {/*(click && !ctrl)*/ 
				lastActiveLine = Edit->ebrow;
				Edit->SetLine(row,true,true,true,!ctrl);
				if(changeActive){Refresh(false);}
				if(!ctrl || dclick){
					SelectRow(row);
					extendRow = -1;
				}
			}

			//1-kliknięcie lewym
			//2-kliknięcie lewym i edycja na pauzie
			//3-kliknięcie lewym i edycja na pauzie i odtwarzaniu

			if (dclick||(click && lastActiveLine != row && mvtal < 4 && mvtal > 0 ) && pas < 2){
				SetVideoLineTime(event);
			}

			if (click || dclick || left_up)
				return;
		}

		if (middle){
			VideoCtrl *video=Kai->GetTab()->Video;
			if(video->GetState()!=None){//
				video->PlayLine(GetDial(row)->Start.mstime, video->GetPlayEndTime(GetDial(row)->End.mstime) /*- video->avtpf*/);
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
			row = MID(0,row, GetCount()-1);
			int i1 = row;
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
			if(changeActive){
				lastActiveLine = Edit->ebrow;
				Edit->SetLine(row,true,true,false);
				//if(mvtal < 4 && mvtal > 0){
					//SetVideoLineTime(event);
				//}
			}
			lastsel=row;
			Refresh(false);
			if(Edit->Visual==CHANGEPOS/* || Edit->Visual==MOVEALL*/){
				Kai->GetTab()->Video->SetVisual();
				Kai->GetTab()->Video->Render();
			}
		}
	}


}

void SubsGrid::Convert(char type)
{
	if(Options.GetBool(ConvertShowSettings)){
		OptionsDialog od(Kai,Kai);
		od.OptionsTree->ChangeSelection(2);
		od.okok->SetFocus();
		if(od.ShowModal()==wxID_CANCEL){return;}
	}
	if(Options.GetBool(ConvertFPSFromVideo)&&Kai->GetTab()->VideoPath!=""){
		Options.SetString(ConvertFPS, Kai->GetStatusText(4).BeforeFirst(' '));
	}
	if(Options.GetFloat(ConvertFPS)<1){KaiMessageBox(_("Nieprawidłowy FPS. Popraw opcje i spróbuj ponownie."));return;}

	bool newendtimes=Options.GetBool(ConvertNewEndTimes);
	wxString stname=Options.GetString(ConvertStyle);
	int endt=Options.GetInt(ConvertTimePerLetter);
	wxString prefix=Options.GetString(ConvertASSTagsOnLineStart);
	//KaiMessageBox("pętla");
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
		wxString resx=Options.GetString(ConvertResolutionWidth);
		wxString resy=Options.GetString(ConvertResolutionHeight);
		if(resx==""){resx="1280";}
		if(resy==""){resx="720";}
		AddSInfo("PlayResX",resx, false);
		AddSInfo("PlayResY",resy, false);
		AddSInfo("YCbCr Matrix","TV.601",false);
		wxString catalog=Options.GetString(ConvertStyleCatalog);

		if(Options.dirs.Index(catalog)!=-1){Options.LoadStyles(catalog);}
		int stind=Options.FindStyle(stname);

		if(stind<0){Styles *newstyl=new Styles(); newstyl->Name=stname;AddStyle(newstyl);}
		else{AddStyle(Options.GetStyle(stind)->Copy());}
		Edit->RefreshStyle();
	}
	if(form==ASS){
		std::sort(file->subs->dials.begin(),file->subs->dials.end(),[](Dialogue *i, Dialogue *j){
			if(i->Start.mstime!=j->Start.mstime){
				return (i->Start.mstime<j->Start.mstime);
			}
			if(i->End.mstime!=j->End.mstime){
				return (i->End.mstime<j->End.mstime);
			}
			//if(i->Style!=j->Style){
			//return (i->Style.CmpNoCase(j->Style)<0);
			//}
			return (i->Text.CmpNoCase(j->Text)<0);
		});
		Dialogue *lastDialogue = GetDial(0);
		int i = 1; 
		while(i < GetCount()){
			Dialogue *actualDialogue = GetDial(i);
			if(lastDialogue->Start == actualDialogue->Start && 
				lastDialogue->End == actualDialogue->End && 
				lastDialogue->Text == actualDialogue->Text){
					DeleteRow(i-1);
					lastDialogue = actualDialogue;
					continue;
			}else if(actualDialogue->Text==""){
				DeleteRow(i);
				continue;
			}
			lastDialogue = actualDialogue;
			i++;
		}
		Kai->SetStatusText("",5);
	}else{
		Kai->SetSubsResolution();
	}

	form=type;
	Edit->SetLine((Edit->ebrow < GetCount())? Edit->ebrow : 0);
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

HANDLE ffile=CreateFile(grid->fn.fn_str(), GENERIC_WRITE, FILE_SHARE_WRITE,NULL, OPEN_EXISTING, 0, NULL);

//wxMutex mutex1;
DWORD savesize=0;
wxMutex mutex2;
while(1)
{
if(grid->acline>=grid->GetCount()){break;}
Dialogue *dial=grid->GetDial(grid->acline++);
//grid->acline++;

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
void SubsGrid::SaveFile(const wxString &filename, bool cstat)
{
	if(Options.GetInt(GridSaveAfterCharacterCount)>1){
		bool oldOnVideo = Edit->OnVideo;
		Edit->Send(false,false,true);
		Edit->OnVideo = oldOnVideo;
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
		if(dial->State==1 && cstat){dial->State=2;}

		//worker[i] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)saveproc,this,0, NULL);

	}
	//WaitForMultipleObjects(4, worker, TRUE, INFINITE);
	ow.CloseFile();
	if(cstat){Refresh(false);}
}

void SubsGrid::HideOver()
{
	hideover=!hideover;
	Options.SetBool(GridHideTags,hideover);
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

void SubsGrid::ChangeTimes(bool byFrame)
{
	VideoCtrl *vb=((TabPanel*)GetParent())->Video;
	if(byFrame && !vb->VFF){wxLogMessage(_("Wideo nie zostało wczytane przez FFMS2"));return;}
	//1 forward / backward, 2 Start Time For V/A Timing, 4 Move to video time, 8 Move to audio time;
	int moveTimeOptions = Options.GetInt(MoveTimesOptions);
	
	//Time to move
	int time = (!byFrame)? Options.GetInt(MoveTimesTime) : 0;
	int frame = (byFrame)? Options.GetInt(MoveTimesFrames) : 0;
	int whichLines = MAX(0,Options.GetInt(MoveTimesWhichLines));
	int whichTimes = MAX(0,Options.GetInt(MoveTimesWhichTimes));
	int correctEndTimes  = Options.GetInt(MoveTimesCorrectEndTimes);
	//1 Lead In, 2 Lead Out, 4 Make times continous, 8 Snap to keyframe;
	int PostprocessorOptions  = Options.GetInt(PostprocessorEnabling);
	int li=0, lo=0, ts=0, te=0, kbs=0, kas=0, kbe=0, kae=0;
	if(PostprocessorOptions){
		if(form==TMP || PostprocessorOptions<16){PostprocessorOptions=0;}
		else if(PostprocessorOptions & 8 && !vb->VFF){PostprocessorOptions^=8;}
		li  = Options.GetInt(PostprocessorLeadIn);
		lo  = Options.GetInt(PostprocessorLeadOut);
		ts  = Options.GetInt(PostprocessorThresholdStart);
		te  = Options.GetInt(PostprocessorThresholdEnd);
		kbs = Options.GetInt(PostprocessorKeyframeBeforeStart);
		kas = Options.GetInt(PostprocessorKeyframeAfterStart);
		kbe = Options.GetInt(PostprocessorKeyframeBeforeEnd);
		kae = Options.GetInt(PostprocessorKeyframeAfterEnd);
	}
	wxString style=Options.GetString(MoveTimesStyles);

	if(!(moveTimeOptions & 1)){
		time = (-time);
		frame = (-frame);
	}
	// video / audio move start or end
	int VAS = moveTimeOptions & 2;


	std::map<Dialogue *,int,compare> tmpmap;


	if(whichTimes!=0){
		int answer=KaiMessageBox(wxString::Format(_("Czy naprawdę chcesz przesuwać tylko czasy %s?"), 
			(whichTimes==1)? _("początkowe") : _("końcowe")),_("Potwierdzenie"),wxYES_NO);
		if(answer==wxNO){return;}
	}

	if(form==TMP){whichTimes=1;}

	bool fromstyl=false;

	int fs=FirstSel();
	if (fs==-1 && whichLines!=0 && whichLines!=4){
		KaiMessageBox(_("Nie zaznaczono linii do przesunięcia"),_("Uwaga"));return;
	}

	int difftime=(VAS)? file->subs->dials[markedLine]->Start.mstime : file->subs->dials[markedLine]->End.mstime;
	//Start time - halfframe / end time + halfframe
	if((moveTimeOptions & 4) && vb->GetState()!=None){
		if(byFrame){
			frame += vb->GetCurrentFrame() - vb->VFF->GetFramefromMS(difftime);
		}else{
			int addedTimes= vb->GetFrameTime(VAS != 0) - difftime;
			if(addedTimes<0){addedTimes-=10;}
			time += ZEROIT(addedTimes);
		}
		
	}
	else if((moveTimeOptions & 8) && Edit->ABox->audioDisplay->hasMark){
		if(byFrame){
			frame += vb->VFF->GetFramefromMS(Edit->ABox->audioDisplay->curMarkMS - difftime);
		}else{
			int addedTimes= Edit->ABox->audioDisplay->curMarkMS - difftime;
			if(addedTimes<0){addedTimes-=10;}
			time += ZEROIT(addedTimes);
		}
	}


	wxArrayString stcomp;
	if(whichLines == 4){
		int g=0;
		wxStringTokenizer styles(style,";");
		while(styles.HasMoreTokens()){
			wxString styl=styles.GetNextToken();
			styl.Trim(false);
			styl.Trim(true);
			stcomp.Add(styl);

			g++;
		}
	}

	int firsttime=GetDial(fs)->Start.mstime;
	Dialogue *dialc;

	for (int i=0;i<GetCount();i++)
	{
		if(whichLines==4 && stcomp.GetCount()>0 ){
			fromstyl=false;
			wxString styl=file->subs->dials[i]->Style;
			for(size_t i=0;i<stcomp.GetCount();i++){
				if(styl==stcomp[i]){fromstyl=true;}
			}
		}

		if( whichLines==0
			|| ( whichLines==1 && sel.find(i) != sel.end() ) 
			|| ( whichLines==3 && firsttime <= file->subs->dials[i]->Start.mstime ) 
			|| ( whichLines==2 && i>=fs )
			|| fromstyl)
		{

			dialc=file->CopyDial(i,true,true);
			if(time!=0){
				int startTrimed = vb->TrimTimeToFrame(dialc->Start.mstime);
				int endTrimed = vb->TrimTimeToFrame(dialc->End.mstime);
				if(whichTimes!=2){dialc->Start.Change(time);}
				if(whichTimes!=1){dialc->End.Change(time);}
				dialc->State=1;
				int newStartTrimed = vb->TrimTimeToFrame(dialc->Start.mstime);
				int newEndTrimed = vb->TrimTimeToFrame(dialc->End.mstime);
				dialc->ChangeTimes(newStartTrimed - startTrimed, newEndTrimed - endTrimed);
			}else if(frame!=0){
				int startTrimed = vb->TrimTimeToFrame(dialc->Start.mstime);
				int endTrimed = vb->TrimTimeToFrame(dialc->End.mstime);
				if(whichTimes!=2){
					int startFrame = vb->VFF->GetFramefromMS(dialc->Start.mstime)+frame;
					dialc->Start.NewTime(vb->GetFrameTimeFromFrame(startFrame));
				}
				if(whichTimes!=1){
					int endFrame = vb->VFF->GetFramefromMS(dialc->End.mstime)+frame;
					dialc->End.NewTime(vb->GetFrameTimeFromFrame(endFrame));
				}
				dialc->State=1;
				int newStartTrimed = vb->TrimTimeToFrame(dialc->Start.mstime);
				int newEndTrimed = vb->TrimTimeToFrame(dialc->End.mstime);
				dialc->ChangeTimes(newStartTrimed - startTrimed, newEndTrimed - endTrimed);
			}
			if(correctEndTimes>0 || PostprocessorOptions>16){
				if(correctEndTimes>1){
					int endt=Options.GetInt(ConvertTimePerLetter);
					int newend=(endt*dialc->Text.Len());
					if(newend<1000){newend=1000;}
					newend+=dialc->Start.mstime;
					dialc->End.NewTime(newend);
				}
				if(PostprocessorOptions & 1){dialc->Start.Change(-li);dialc->State=1;}
				if(PostprocessorOptions & 2){dialc->End.Change(lo);dialc->State=1;}
				if(correctEndTimes>0 || PostprocessorOptions>19){
					tmpmap[dialc]=i;

				}
			}else{
				dialc->ClearParse();
			}

		}// if przesuwana linia

	}//pętla for

	// tu jeszcze należy poprawić uwzględniając linijkę z czasem przed tablicą i czasem po niej
	// a może to w ogóle nie jest potrzebne?
	if(correctEndTimes>0 || PostprocessorOptions>19){
		bool hasend=false;
		int newstarttime=-1;

		for(auto cur=tmpmap.begin(); cur != tmpmap.end(); cur++){
			auto it = cur;
			dialc = cur->first;//file->subs->dials[cur->second];
			it++;
			if(!(it!=tmpmap.end())){it=cur; hasend=true;}
			if(correctEndTimes>0 && dialc->End > it->first->Start && !hasend){
				dialc->End = it->first->Start;
				dialc->State=1;
			}
			if(PostprocessorOptions & 4){
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
			if(PostprocessorOptions & 8){
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
						keyMS = ZEROIT(vb->GetFrameTimeFromTime(keyMS));
						if(strtres>keyMS && keyMS != dialc->Start.mstime){strtres = keyMS;}
					}
					if (keyMS > endrng && keyMS < endrng1) {
						keyMS = ZEROIT(vb->GetFrameTimeFromTime(keyMS));
						if(endres<keyMS && keyMS > dialc->Start.mstime){endres = keyMS;}
					}
				}
				if(strtres!=INT_MAX && strtres >= pors){
					dialc->Start.NewTime(strtres);
					dialc->State=1;
				}
				if(endres!=-1 && endres <= pore){
					dialc->End.NewTime(endres);
					dialc->State=1;
				}
			}
			dialc->ClearParse();
		}

	}

	SpellErrors.clear();
	SetModified();
	if(form>TMP){RepaintWindow(START|END);}else{Refresh(false);}
#if _DEBUG
	wxBell();
#endif
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
	if (key == WXK_WINDOWS_MENU) {
		wxPoint pos;
		pos.x = w/2;
		pos.y = (Edit->ebrow+1-scPos) * GridHeight + GridHeight/2;
		ContextMenu(pos);
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
			Edit->SetLine(next);
			TabPanel *pan = (TabPanel*)GetParent();
			int mvtal= pan->Video->vToolbar->videoSeekAfter->GetSelection();//Options.GetInt(MoveVideoToActiveLine);
			int pasel= pan->Video->vToolbar->videoPlayAfter->GetSelection();//Options.GetInt(PlayAfterSelection);
			//1-kliknięcie lewym
			//2-kliknięcie lewym i edycja na pauzie
			//3-kliknięcie lewym i edycja na pauzie i odtwarzaniu
			if ( mvtal < 4 && mvtal > 0 && pasel>1){
				
				if(pan->Video->GetState()==Stopped){pan->Video->Play();pan->Video->Pause();}
				int vczas=GetDial(next)->Start.mstime;
				pan->Video->Seek(MAX(0,vczas),true,true,false);
			}
			SelectRow(next);
			int gridh=((h/(GridHeight+1))-1);
			if(dir==1||dir==-1){
				bool above= (next<=scPos);
				bool below= (next>=scPos+gridh);
				if(above||below){ScrollTo(above? next-1 : next-gridh+1);}
			}else{
				ScrollTo(next);}
			lastRow=next;
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
			extendRow = lastRow = MID(0,extendRow+dir,GetCount()-1);
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
	Edit->SetLine(FirstSel());
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
	Kai->Menubar->Enable(Undo,undo);
	Kai->Menubar->Enable(Redo,_redo);
	if(toolbar){
		Kai->Toolbar->UpdateId(Undo,undo);
		Kai->Toolbar->UpdateId(Redo,_redo);
	}
}

void SubsGrid::GetUndo(bool redo)
{
	TabPanel *pan =Kai->GetTab();
	Freeze();
	wxString resolution = GetSInfo("PlayResX") +" x "+ GetSInfo("PlayResY");
	wxString matrix = GetSInfo("YCbCr Matrix");
	if(redo){file->Redo();}else{file->Undo();}


	UpdateUR();

	Kai->Label(file->Iter());


	char oldform=form;SetSubsForm();
	if(oldform!=form){
		pan->CTime->Contents();
		pan->Edit->HideControls();
		Kai->UpdateToolbar();
	}

	int erow=Edit->ebrow;
	if(erow>=GetCount()){
		erow=GetCount()-1;
		sel.clear();
		sel[erow]=true;
		lastRow=erow;
	}

	Thaw();

	if(StyleStore::HasStore()){
		StyleStore *SS = StyleStore::Get();
		SS->ASS->SetArray(&file->subs->styles);
		SS->ASS->Refresh(false);
	}
	SpellErrors.clear();
	RepaintWindow();
	Edit->SetLine(erow);
	Edit->RefreshStyle();
	for(auto cur = sel.begin(); cur != sel.end(); cur++){
		if(cur->first >= GetCount()){
			sel.erase(cur,sel.end());
			break;
		}
	}
	VideoCtrl *vb=pan->Video;
	if(Edit->Visual < CHANGEPOS){
		
		if(vb->IsShown() || vb->isFullscreen){vb->OpenSubs(GetVisible()/*SaveText()*/);Edit->OnVideo=true;}
		int opt=vb->vToolbar->videoSeekAfter->GetSelection();//Options.GetInt(MoveVideoToActiveLine);
		if(opt>1){
			if(vb->GetState()==Paused || (vb->GetState()==Playing && (opt==3 || opt==5))){
				vb->Seek(Edit->line->Start.mstime);}
		}else{
			if(vb->GetState()==Paused){vb->Render();}
		}
	}else if(Edit->Visual==CHANGEPOS){
		vb->SetVisual(false, true);
	}
	wxString newResolution = GetSInfo("PlayResX") +" x "+ GetSInfo("PlayResY");
	if(resolution != newResolution){Kai->SetSubsResolution();}
	wxString newmatrix = GetSInfo("YCbCr Matrix");
	if(matrix != newmatrix){
		vb->SetColorSpace(newmatrix);
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


void SubsGrid::AddSInfo(const wxString &SI, wxString val, bool save)
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

void SubsGrid::SetModified(bool redit, bool dummy, int SetEditBoxLine)
{
	if(file->IsNotSaved()){
		if(file->Iter()<1||!Modified){
			Modified=true;
		}
		if(Comparsion){
			Kai->Tabs->SubsComparsion();
		}

		Kai->Label(file->Iter()+1);
		if(redit)
		{
			int erow= (SetEditBoxLine >= 0)? SetEditBoxLine : Edit->ebrow;
			if(erow>=GetCount()){erow=GetCount()-1;}
			lastRow=erow;
			if(scPos>erow){scPos=MAX(0,(erow-4));}
			Edit->SetLine(erow);
			sel[erow]=true;
		}
		file->SaveUndo();
		if(!dummy){
			VideoCtrl *vb=Kai->GetTab()->Video;
			if(Edit->Visual >= CHANGEPOS){
				vb->SetVisual(false, true);
			}else{
				if(vb->IsShown() || vb->isFullscreen){vb->OpenSubs(GetVisible()/*SaveText()*/);Edit->OnVideo=true;}

				int opt=vb->vToolbar->videoSeekAfter->GetSelection();//Options.GetInt(MoveVideoToActiveLine);
				if(opt>1){
					if(vb->GetState()==Paused || (vb->GetState()==Playing && (opt==3 || opt==5))){
						vb->Seek(Edit->line->Start.mstime);}
				}else{
					if(vb->GetState()==Paused){vb->Render();}
				}
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

void SubsGrid::Loadfile(const wxString &str,const wxString &ext){

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
		bool isASS= (ext=="ass"||ext=="ssa");
		wxStringTokenizer tokenizer(str,"\n",wxTOKEN_STRTOK);


		while ( tokenizer.HasMoreTokens() && isASS){
			wxString token = tokenizer.GetNextToken().Trim(false);
			if(token.StartsWith("Style: "))
			{
				//1 = ASS, 2 = SSA, potrzebne tylko przy odczycie napisów.
				AddStyle(new Styles(token,format));
				sinfoo=2;
			}
			else if(sinfoo==2 && !token.StartsWith("Style:")){
				break;
			}
			else if(token.StartsWith("[V4")){
				if(!token.StartsWith("[V4+")){format=2;}//ze względu na to, że wywaliłem ssa z formatów
				//muszę posłać do konstruktora styli 2 jako format SSA, nie używam tu srt, 
				//żeby później źle tego nie zinterpretować
				sinfoo=1;
			}
			else if(!token.StartsWith(";") && !token.StartsWith("[") && sinfoo==0 && token.Find(':')!=wxNOT_FOUND){
				AddSInfo(token);
			}
		}
		if(ext == "ssa"){AddSInfo("ScriptType", "4.00+");}

		bool tlmode=(GetSInfo("TLMode")=="Yes");
		if(GetSInfo("Active Line")!="" &&(ext=="ass"||ext=="ssa")){active=wxAtoi(GetSInfo("Active Line"));}
		if(transl&&!tlmode){
			transl=false;
			showtl=false;
			Kai->Menubar->Enable(SaveTranslation,false);
			Edit->SetTl(false);
		}
		wxString tlstyle;
		if(tlmode){tlstyle=GetSInfo("TLMode Style");if(tlstyle==""){tlmode=false;}}
		wxString matrix = GetSInfo("YCbCr Matrix");
		if(matrix == "" || matrix == "None" ){AddSInfo("YCbCr Matrix","TV.601");}

		while ( tokenizer.HasMoreTokens() )
		{
			wxString token = tokenizer.GetNextToken();
			if(isASS && !(token.StartsWith("Dial") || token.StartsWith("Comm"))){continue;}
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

	if(GetCount()<1){LoadDefault();KaiMessageBox(_("Niepoprawny format (plik uszkodzony lub zawiera błędy)"));form=ASS;}
	else{SetSubsForm();}
	origform=form;

	if(GetSInfo("TLMode")=="Yes"){
		Edit->SetTl(true);
		transl=true;
		if(GetSInfo("TLMode Showtl")=="Yes"){showtl=true;}
		Kai->Menubar->Enable(SaveTranslation,true);
	}


	if(form==MDVD||form==MPL2){
		int endt=Options.GetInt(ConvertTimePerLetter);
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
		if(Options.GetBool(GridLoadSortedSubs)){std::sort(file->subs->dials.begin(), file->subs->dials.end(), sortstart);}}
	else{Edit->TlMode->Enable(false);}
	if(active>=GetCount()){active=0;}


	sel[active]=true;
	lastRow=active;
	markedLine=active;

	scPos=MAX(0,active-3);
	RepaintWindow();
	Edit->OnVideo = true;
	Edit->SetLine(active,false,false);

	Edit->HideControls();

	file->EndLoad();
	if(StyleStore::HasStore() && form==ASS){StyleStore::Get()->LoadAssStyles();}
	if(form == ASS){RebuildActorEffectLists();}
}

void SubsGrid::SetStartTime(int stime)
{
	Edit->Send(false,false,true);
	wxArrayInt sels=GetSels();
	for(size_t i=0;i<sels.size();i++){
		Dialogue *dialc=CopyDial(sels[i]);
		if(!dialc){continue;}
		dialc->Start.NewTime(stime);
		if(dialc->End<stime){dialc->End.NewTime(stime);}
	}
	if(sels.size()){
		SetModified();
		Refresh(false);
	}
}

void SubsGrid::SetEndTime(int etime)
{
	Edit->Send(false,false,true);
	wxArrayInt sels=GetSels();
	for(size_t i=0;i<sels.size();i++){
		Dialogue *dialc=CopyDial(sels[i]);
		if(!dialc){continue;}
		dialc->End.NewTime(etime);
		if(dialc->Start>etime){dialc->Start.NewTime(etime);}
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
		Kai->Menubar->Enable(SaveTranslation,true);

		Refresh(false);

	}else{
		if(KaiMessageBox(_("Czy na pewno chcesz wyłączyć tryb tłumaczenia?\nObcojęzyczny tekst przetłumaczonych linijek zostanie usunięty."),_("Potwierdzenie"),wxYES_NO)==wxNO)
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
		Kai->Menubar->Enable(SaveTranslation,false);
	}
	SpellErrors.clear();
	Refresh(false);
	VideoCtrl *vb = ((TabPanel*)GetParent())->Video;
	if(vb->GetState()!=None){
		vb->OpenSubs(GetVisible()/*SaveText()*/);
		if(vb->GetState()==Paused){vb->Render();}
		Edit->OnVideo=true;
	}
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
			{Edit->SetLine(i);SelectRow(i);ScrollTo(i-4);
			break;}
			if(dial->Start.mstime > prevtime && dial->Start.mstime < time){prevtime = dial->Start.mstime;ip=i;}
			if(dial->Start.mstime < durtime && dial->Start.mstime > time){durtime = dial->Start.mstime;idr=i;}

		}
		if(i==GetCount()-1){if((time-prevtime)>(durtime-time)){Edit->SetLine(idr);SelectRow(idr);ScrollTo(idr-4);}
		else{Edit->SetLine(ip);SelectRow(ip);ScrollTo(ip-4);}}
	}

}

void SubsGrid::NextLine(int dir)
{
	if(Edit->ABox&&Edit->ABox->audioDisplay->hold!=0){return;}
	int nebrow=Edit->ebrow+dir;
	if(nebrow<0){return;}
	if(nebrow>=GetCount()){
		Dialogue *tmp=GetDial(GetCount()-1)->Copy();
		int eend= tmp->End.mstime; 
		tmp->Start.NewTime(eend); 
		tmp->End.NewTime(eend+5000);
		AddLine(tmp);
		SetModified(false);
	}
	int h,w;
	GetClientSize(&w,&h);
	scPos = MID(0, nebrow-((h/(GridHeight+1))/2), GetCount()-1);
	sel.clear();
	sel[nebrow]=true;
	AdjustWidths(0);
	Refresh(false);
	Edit->SetLine(nebrow,true,true,false,true);
	//if(Edit->ABox){Edit->ABox->audioDisplay->SetDialogue(Edit->line,nebrow);}
	
}

void SubsGrid::CheckText(wxString text, wxArrayInt &errs)
{

	wxString notchar="/?<>|\\!@#$%^&*()_+=[]\t~ :;.,\"{} ";
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
			bool isgood=SpellChecker::Get()->CheckWord(word);
			if (!isgood){errs.push_back(firsti);errs.push_back(lasti);}
			//KaiMessageBox(word);
		}word="";firsti=i+1;}
		if(ch=='{'){block=true;}
		else if(ch=='}'){block=false;firsti=i+1;word="";}


		if(notchar.Find(ch) == -1&& text.GetChar((i==0)? 0 : i-1) != '\\' && !block){word<<ch;lasti=i;}
		else if( !block && text.GetChar((i==0)? 0 : i-1)=='\\'){
			word="";
			if(ch == 'N' || ch == 'n' || ch =='h'){
				firsti=i + 1;
			}else{
				firsti=i;
				word<<ch;
			}
		}
	}
	if(errs.size()<2){errs.push_back(0);}

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
	AddSInfo("YCbCr Matrix","TV.601",sav);
	if(endload){file->EndLoad();}
}

Dialogue *SubsGrid::CopyDial(int i, bool push)
{
	if(push && (int)SpellErrors.size() > i){SpellErrors[i].Clear();}
	return file->CopyDial(i, push);
}

Dialogue *SubsGrid::GetDial(int i)
{
	if(i >= (int)file->subs->dials.size()){return NULL;}
	return file->subs->dials[i];
}

wxString SubsGrid::GetSInfo(const wxString &key, int *ii)
{
	int i=0;
	for(std::vector<SInfo*>::iterator it=file->subs->sinfo.begin(); it!=file->subs->sinfo.end(); it++)
	{
		if(key==(*it)->Name) {if(ii){*ii=i;} return (*it)->Val;}
		i++;
	}
	return "";
}

SInfo *SubsGrid::GetSInfoP(const wxString &key,int *ii)
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

wxString *SubsGrid::GetVisible(bool *visible, wxPoint *point, wxArrayInt *selected)
{
	TabPanel *pan=(TabPanel*)GetParent();
	int _time=pan->Video->Tell();
	bool toEnd = pan->Video->GetState() == Playing;
	wxString *txt=new wxString();

	(*txt)<<"[Script Info]\r\n"<<GetSInfos(false);
	(*txt)<<"\r\n[V4+ Styles]\r\nFormat: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding \r\n";
	(*txt)<<GetStyles(false);
	(*txt)<<" \r\n[Events]\r\nFormat: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\r\n";

	Edit->Send(false,true);
	if(_time >= Edit->line->Start.mstime && _time <= Edit->line->End.mstime)
	{
		if(visible){*visible=true;}
	}else if(visible){
		*visible=false;
	}
	bool noLine = true;
	bool isTlmode = GetSInfo("TLMode")=="Yes";
	wxString tlStyle = GetSInfo("TLMode Style");
	for(int i=0; i<GetCount(); i++){
		Dialogue *dial=GetDial(i);
		if(i==Edit->ebrow){ 
			dial = Edit->line;
		}
		if(selected && sel.find(i)!=sel.end()){
			selected->Add(txt->Len());
			continue;
		}
		if((toEnd && _time <= dial->Start.mstime) || (_time >= dial->Start.mstime && _time <= dial->End.mstime)){
			//if(trimSels && sel.find(i)!=sel.end()){continue;}
			if( isTlmode && dial->TextTl!=""){
				(*txt)<<dial->GetRaw(false,tlStyle);
				(*txt)<<dial->GetRaw(true);
			}else{
				(*txt)<<dial->GetRaw();
			}
			if( point && i==Edit->ebrow ){
				int all= txt->Len();point->x=all-2;
				int len = (isTlmode && dial->TextTl!="")? 
					dial->TextTl.Len() : dial->Text.Len();
				point->y = len;
				point->x -= len;
			}
			noLine = false;
		}

	}
	if(noLine){
		(*txt)<<Dialogue().GetRaw();
	}


	return txt;
}

//wxString *SubsGrid::GetSubsToEnd(bool *visible, wxPoint *point)
//{
//	TabPanel *pan=(TabPanel*)GetParent();
//	int _time=pan->Video->Tell();
//	wxString *txt=new wxString();
//
//	(*txt)<<"[Script Info]\r\n"<<GetSInfos(false);
//	(*txt)<<"\r\n[V4+ Styles]\r\nFormat: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding \r\n";
//	(*txt)<<GetStyles(false);
//	(*txt)<<" \r\n[Events]\r\nFormat: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\r\n";
//
//	Edit->Send(false,true);
//	if(_time >= Edit->line->Start.mstime && _time <= Edit->line->End.mstime)
//	{
//		if(visible){*visible=true;}
//	}else if(visible){
//		*visible=false;
//	}
//	bool noLine = true;
//	bool isTlmode = GetSInfo("TLMode")=="Yes";
//	for(int i=0; i<GetCount(); i++){
//		Dialogue *dial=GetDial(i);
//		if(i==Edit->ebrow){ 
//			dial = Edit->line;
//		}
//		if(_time >= dial->Start.mstime){
//			if( isTlmode && dial->TextTl!=""){
//				(*txt)<<dial->GetRaw(false,GetSInfo("TLMode Style"));
//				(*txt)<<dial->GetRaw(true);
//			}else{
//				(*txt)<<dial->GetRaw();
//			}
//			noLine = false;
//		}
//		if( point && i==Edit->ebrow ){
//			int all= txt->Len();point->x=all-2;
//			int len = (GetSInfo("TLMode")=="Yes" && dial->TextTl!="")? 
//				dial->TextTl.Len() : dial->Text.Len();
//			point->y = len;
//			point->x -= len;
//		}
//	}
//	if(noLine){
//		(*txt)<<Dialogue().GetRaw();
//	}
//
//
//	return txt;
//}

//VOID CALLBACK callbackfunc ( PVOID   lpParameter, BOOLEAN TimerOrWaitFired) {
//	Automation *auto_ = (Automation*)lpParameter;
//	auto_->ReloadScripts(true);
//	DeleteTimerQueueTimer(auto_->handle,0,0);
//}

void SubsGrid::OnBcktimer(wxTimerEvent &event)
{
	TabPanel *pan=(TabPanel*)GetParent();
	Kai->SetStatusText(_("Autozapis"),0);
	wxString path;
	wxString ext=(form<SRT)? "ass" : (form==SRT)? "srt" : "txt";

	path<<Options.pathfull<<"\\Subs\\"<<pan->SubsName.BeforeLast('.')
		<<"_"<<Notebook::GetTabs()->FindPanel(pan)<<"_"<< numsave <<"."<<ext;

	SaveFile(path, false);
	int maxFiles = Options.GetInt(AutoSaveMaxFiles);
	if(maxFiles>1 && numsave >= maxFiles){numsave=0;}
	numsave++;
	makebkp=true;
	nullifyTimer.Start(5000,true);
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

int SubsGrid::CalcChars(const wxString &txt, wxString *lines, bool *bad)
{
	int len=txt.Len();
	bool block=false;bool slash=false;
	bool drawing = false;
	int chars=0, lastchars=0, ns=0;
	wxUniChar brs=(form==SRT)? '<' : '{';
	wxUniChar bre=(form==SRT)? '>' : '}';
	for(int i=0; i<len; i++)
	{
		if(txt[i]==brs){block=true;}
		else if(txt[i]==bre){block=false;}
		else if(block && txt[i]=='p' && txt[i-1]=='\\' && (i+1 < len && wxString(txt[i+1]).IsNumber())){
			if(txt[i+1]=='0'){drawing=false;}
			else{drawing=true;}
		}
		else if(txt[i]=='\\' && !block && !drawing){slash=true; continue;}
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
		else if(!block && !drawing && txt[i]!=' '){chars++;}

	}
	if(lines){
		int linechars=(chars-lastchars);
		if(!(*bad)){*bad=(linechars>43 || ns>1);}
		(*lines)<<linechars<<"/";
	}


	return chars;
}

void SubsGrid::RebuildActorEffectLists()
{
	Edit->ActorEdit->Clear();
	Edit->EffectEdit->Clear();
	for(int i = 0; i<GetCount(); i++){
		Dialogue *dial = GetDial(i);
		if(!dial->Actor.IsEmpty() && Edit->ActorEdit->FindString(dial->Actor, true) <0 ){
			Edit->ActorEdit->Append(dial->Actor);
		}
		if(!dial->Effect.IsEmpty() && Edit->EffectEdit->FindString(dial->Effect, true) <0 ){
			Edit->EffectEdit->Append(dial->Effect);
		}
	}
	Edit->ActorEdit->Sort();
	Edit->EffectEdit->Sort();
}

void SubsGrid::RefreshIfVisible(int time)
{
	int scrows = scPos + panelrows - 1;
	int count = GetCount();
	if(scrows>=count){scrows = count;}
	if(scPos<0){scPos=0;}
	if((int)visibleLines.size() < scrows-scPos){return;}
	int counter = 0;
	for(int i = scPos; i < scrows; i++){
		Dialogue *dial = GetDial(i);
		bool isVisible = dial->Start.mstime <= time && dial->End.mstime > time;
		if(isVisible != visibleLines[counter++]){
			Refresh(false);
			break;
		}
		
	}
	
}

void SubsGrid::ChangeTimeDisplay(bool frame)
{
	TabPanel *tab = (TabPanel*)GetParent();
	VideoFfmpeg *VFF = tab->Video->VFF;
	if(frame && VFF){
		/*for(int i=0; i < GetCount(); i++){
			Dialogue *Dial = GetDial(i);
			if(!Dial){continue;}
			Dial->Start.orgframe = VFF->GetFramefromMS(Dial->Start.mstime);
			Dial->End.orgframe = VFF->GetFramefromMS(Dial->End.mstime)-1;
		}*/
		//ShowFrames shows it properly when orgframe is setup
		showFrames = true;
		/*STime ct = tab->CTime->TimeText->GetTime();
		tab->CTime->TimeText->ShowFrames(showFrames);
		ct.orgframe = VFF->GetFramefromMS(ct.mstime);
		tab->CTime->TimeText->SetTime(ct);*/
		
	}else{
		showFrames = false;
		/*STime ct = tab->CTime->TimeText->GetTime();
		ct.ChangeFormat(ASS);
		tab->CTime->TimeText->ShowFrames(showFrames);
		tab->CTime->TimeText->SetTime(ct);*/
	}
	
}

BEGIN_EVENT_TABLE(SubsGrid,KaiScrolledWindow)
	EVT_PAINT(SubsGrid::OnPaint)
	EVT_SIZE(SubsGrid::OnSize)
	EVT_SCROLLWIN(SubsGrid::OnScroll)
	EVT_MOUSE_EVENTS(SubsGrid::OnMouseEvent)
	EVT_KEY_DOWN(SubsGrid::OnKeyPress)
	EVT_TIMER(ID_AUTIMER,SubsGrid::OnBcktimer)
	EVT_ERASE_BACKGROUND(SubsGrid::OnEraseBackground)
	EVT_MOUSE_CAPTURE_LOST(SubsGrid::OnLostCapture)
END_EVENT_TABLE()

