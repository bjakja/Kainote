//  Copyright (c) 2012-2017, Marcin Drob

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


#include "SubsGridBase.h"
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

bool SubsGridBase::IsNumber(const wxString &test) {
	bool isnumber=true;
	wxString testchars="0123456789";
	for(size_t i=0;i<test.Len();i++){
		wxUniChar ch=test.GetChar(i);
		if(testchars.Find(ch)==-1){isnumber=false;break;}
	}
	return isnumber;
}



SubsGridBase::SubsGridBase(wxWindow *parent, const long int id,const wxPoint& pos,const wxSize& size, long style)
	: KaiScrolledWindow(parent, id, pos, size, style | wxVERTICAL)
{
	file = new SubsFile();
	Modified=false;
	makebackup=true;
	ismenushown=false;
	showFrames=false;
	Comparison=NULL;
	numsave=0;
	
	LoadDefault();
	timer.SetOwner(this, ID_AUTIMER);
	//reset autosave on statusbar
	nullifyTimer.SetOwner(this, 27890);
	Bind(wxEVT_TIMER, [=](wxTimerEvent &evt){
		Kai->SetStatusText("", 0);
	}, 27890);
}


SubsGridBase::~SubsGridBase()
{
	Clearing();
}



void SubsGridBase::Clearing()
{
	Selections.clear();
	SAFE_DELETE(Comparison);
	SAFE_DELETE(file);
	SpellErrors.clear();
	Modified=false;
	first=true;
	scPos=0;
	lastRow=0;
	scHor=0;
}
void SubsGridBase::AddLine(Dialogue *line)
{
	file->subs->ddials.push_back(line);
	file->subs->dials.push_back(line);
	if (line->isVisible){
		file->IdConverter->insert(file->subs->dials.size() - 1);
	}
}

void SubsGridBase::ChangeLine(unsigned char editionType, Dialogue *line1, int wline, long cells, bool selline, bool dummy)
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
		if(subsFormat!=ASS){tmp->Conv(subsFormat);}
		AddLine(tmp);
	}
	AdjustWidths(cells);

	if(selline){
		SaveSelections(true);
		lastRow=wline+1;
		Selections.insert(lastRow);
		int h,w;
		GetClientSize(&w,&h);
		scPos = MID(0, lastRow-((h/(GridHeight+1))/2), GetCount()-1);

	}
	Refresh(false);
	if(selline){
		Edit->SetLine(lastRow,true,true,false,true);
	}
	SetModified(editionType,false,dummy);

}

void SubsGridBase::ChangeCell(long wcell, int wline, Dialogue *what)
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


void SubsGridBase::Convert(char type)
{
	if(GetSInfo("TLMode")=="Yes"){return;}
	if(Options.GetBool(ConvertShowSettings)){
		OptionsDialog od(Kai,Kai);
		od.OptionsTree->ChangeSelection(2);
		od.okok->SetFocus();
		if(od.ShowModal()==wxID_CANCEL){return;}
	}
	if(Options.GetBool(ConvertFPSFromVideo) && Kai->GetTab()->VideoPath!=""){
		Options.SetString(ConvertFPS, Kai->GetStatusText(4).BeforeFirst(' '));
	}
	if(Options.GetFloat(ConvertFPS)<1){KaiMessageBox(_("Nieprawidłowy FPS. Popraw opcje i spróbuj ponownie."));return;}

	bool newendtimes=Options.GetBool(ConvertNewEndTimes);
	wxString stname=Options.GetString(ConvertStyle);
	int endt=Options.GetInt(ConvertTimePerLetter);
	wxString prefix=Options.GetString(ConvertASSTagsOnLineStart);
	
	int i=0;
	while(i<GetCount())
	{
		if((type>ASS) && (subsFormat<SRT) && GetDialogue(i)->IsComment){
			while(GetDialogue(i)->IsComment){
				DeleteRow(i);
			}
		}
		Dialogue *dialc=CopyDial(i);
		dialc->Conv(type,prefix);
		if((newendtimes && type!=TMP)||subsFormat==TMP)
		{
			if(i>0){
				if(GetDialogue(i-1)->End.mstime > dialc->Start.mstime){
					GetDialogue(i-1)->End = dialc->Start;
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
		Kai->SetSubsResolution();
	}
	if(subsFormat==ASS){
		std::sort(file->subs->dials.begin(),file->subs->dials.end(),[](Dialogue *i, Dialogue *j){
			if(i->Start.mstime!=j->Start.mstime){
				return (i->Start.mstime<j->Start.mstime);
			}
			if(i->End.mstime!=j->End.mstime){
				return (i->End.mstime<j->End.mstime);
			}
			return (i->Text.CmpNoCase(j->Text)<0);
		});
		Dialogue *lastDialogue = GetDialogue(0);
		int i = 1; 
		while(i < GetCount()){
			Dialogue *actualDialogue = GetDialogue(i);
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
		
	}else if(type == ASS){
		Kai->SetSubsResolution();
	}

	subsFormat=type;
	file->ReloadVisibleDialogues();
	Edit->SetLine((Edit->ebrow < GetCount())? Edit->ebrow : 0);
	SetModified(GRID_CONVERT);
	RefreshColumns();
	
	if(Edit->Visual > 0){
		TabPanel *tab = Kai->GetTab();
		tab->Video->SetVisual(true);
	}
}

void SubsGridBase::SaveFile(const wxString &filename, bool cstat, bool loadFromEditbox)
{
	int saveAfterCharacterCount = Options.GetInt(GridSaveAfterCharacterCount);
	bool dummyEditboxChanges = (loadFromEditbox && !saveAfterCharacterCount);
	if (dummyEditboxChanges || saveAfterCharacterCount > 1){
		bool oldOnVideo = Edit->OnVideo;
		// no i tu mamy do poprawki dummy subs;
		Edit->Send(EDITBOX_LINE_EDITION, false, dummyEditboxChanges, true);
		Edit->OnVideo = oldOnVideo;
	}
	wxString txt;
	wxString tlmode = GetSInfo("TLMode");
	bool translated = tlmode == "Translated";
	bool tlmodeOn = tlmode != "";

	OpenWrite ow(filename,true);

	if (subsFormat<SRT){
		if(cstat){
			AddSInfo("Last Style Storage",Options.acdir, false);
			AddSInfo("Active Line", std::to_string(Edit->ebrow), false);
		}

		txt << "[Script Info]\r\n;Plik utworzony przez " << Options.progname << "\r\n" << GetSInfos(translated);
		txt<<"\r\n[V4+ Styles]\r\nFormat: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding \r\n";
		txt << GetStyles(translated);
		txt<<" \r\n[Events]\r\nFormat: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\r\n";
	}
	ow.PartFileWrite(txt);

	txt=GetSInfo("TLMode Style");
	wxString raw;
	if (dummyEditboxChanges){
		for (int i = 0; i < GetCount(); i++)
		{
			//a tu trzeba w przypadku ebrow pobrać editbox line
			Dialogue *dial = (i == Edit->ebrow) ? Edit->line : GetDialogue(i);

			if (tlmodeOn){
				bool hasTextTl = dial->TextTl != "";
				if (!translated && (hasTextTl || dial->State & 4)){
					dial->GetRaw(&raw, false, txt);
					dial->GetRaw(&raw, true);
				}
				else{
					dial->GetRaw(&raw, hasTextTl);
				}
			}
			else{

				if (subsFormat == SRT){
					raw << i + 1 << "\r\n";
				}
				dial->GetRaw(&raw);
			}

			ow.PartFileWrite(raw);
			raw.Empty();

		}
	}
	else{
		for (int i = 0; i < file->subs->dials.size(); i++)
		{
			//a tu trzeba w przypadku ebrow pobrać editbox line
			Dialogue *dial = file->subs->dials[i];

			if (tlmodeOn){
				bool hasTextTl = dial->TextTl != "";
				if (!translated && (hasTextTl || dial->State & 4)){
					dial->GetRaw(&raw, false, txt);
					dial->GetRaw(&raw, true);
				}
				else{
					dial->GetRaw(&raw, hasTextTl);
				}
			}
			else{

				if (subsFormat == SRT){
					raw << i + 1 << "\r\n";
				}
				dial->GetRaw(&raw);
			}

			ow.PartFileWrite(raw);
			raw.Empty();

			if (dial->State & 1 && cstat){ dial->State++; }

		}
	}
	
	ow.CloseFile();
	if(cstat){Refresh(false);}
}

void SubsGridBase::AddStyle(Styles *nstyl)
{
	file->subs->dstyles.push_back(nstyl);
	file->subs->styles.push_back(nstyl);
}

void SubsGridBase::ChangeStyle(Styles *nstyl, int i)
{
	file->subs->dstyles.push_back(nstyl);
	file->subs->styles[i]=nstyl;
}

int SubsGridBase::StylesSize()
{
	return file->subs->styles.size();
}

Styles *SubsGridBase::GetStyle(int i,wxString name)
{
	if(name!=""){
		for(unsigned int j=0; j < file->subs->styles.size();j++)
		{
			if(name==file->subs->styles[j]->Name){return file->subs->styles[j];}
		}
	}
	return file->subs->styles[i];
}

std::vector<Styles*> *SubsGridBase::GetStyleTable()
{
	return &file->subs->styles;
}

//multiplication musi być ustawione na zero, wtedy zwróci ilość multiplikacji
int SubsGridBase::FindStyle(wxString name,int *multip)
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

wxString SubsGridBase::GetStyles(bool tld)
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

void SubsGridBase::DelStyle(int i)
{
	file->edited=true;
	file->subs->styles.erase(file->subs->styles.begin()+i);
}

int SubsGridBase::GetCount()
{
	return file->IdConverter->size();
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

void SubsGridBase::ChangeTimes(bool byFrame)
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
		if(subsFormat==TMP || PostprocessorOptions<16){PostprocessorOptions=0;}
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
	bool changeTagTimes = (moveTimeOptions & 32) > 0;

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

	if(subsFormat==TMP){whichTimes=1;}

	bool fromstyl=false;

	int fs=FirstSel();
	if (fs==-1 && whichLines!=0 && whichLines!=4){
		KaiMessageBox(_("Nie zaznaczono linii do przesunięcia"),_("Uwaga"));return;
	}

	int difftime = (VAS) ? file->GetDialogue(markedLine)->Start.mstime : file->GetDialogue(markedLine)->End.mstime;
	
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

	int firsttime=GetDialogue(fs)->Start.mstime;
	//int endDiff = 0;
	Dialogue *dialc;

	for (int i=0;i<GetCount();i++)
	{
		if(whichLines==4 && stcomp.GetCount()>0 ){
			fromstyl=false;
			wxString styl=file->GetDialogue(i)->Style;
			for(size_t i=0;i<stcomp.GetCount();i++){
				if(styl==stcomp[i]){fromstyl=true;}
			}
		}

		if( whichLines==0
			|| ( whichLines==1 && Selections.find(i) != Selections.end() ) 
			|| ( whichLines==3 && firsttime <= file->GetDialogue(i)->Start.mstime ) 
			|| ( whichLines==2 && i>=fs )
			|| fromstyl)
		{

			dialc=file->CopyDial(i,true,true);
			int startTrimed = 0, endTrimed = 0, duration = 0;
			if(changeTagTimes){
				vb->GetStartEndDelay(dialc->Start.mstime, dialc->End.mstime, &startTrimed, &endTrimed);
			}
			if(time!=0){
				if(whichTimes!=2){dialc->Start.Change(time);}
				if(whichTimes!=1){dialc->End.Change(time);}
				dialc->State = 1 + (dialc->State & 4);
			}else if(frame!=0){
				if(whichTimes==0){
					duration = dialc->End.mstime - dialc->Start.mstime;
				}
				if(whichTimes!=2){
					int startFrame = vb->VFF->GetFramefromMS(dialc->Start.mstime)+frame;
					dialc->Start.NewTime(ZEROIT(vb->GetFrameTimeFromFrame(startFrame)));
				}
				if(whichTimes!=1){
					//endDiff = dialc->End.mstime;
					int endFrame = vb->VFF->GetFramefromMS(dialc->End.mstime)+frame;
					dialc->End.NewTime(ZEROIT(vb->GetFrameTimeFromFrame(endFrame)));
					//endDiff = dialc->End.mstime - endDiff;
				}
				dialc->State=1 + (dialc->State & 4);
			}
			if(changeTagTimes){
				int newStartTrimed=0, newEndTrimed=0;
				vb->GetStartEndDelay(dialc->Start.mstime, dialc->End.mstime, &newStartTrimed, &newEndTrimed);
				if(byFrame){newEndTrimed += ((dialc->End.mstime - dialc->Start.mstime) - duration);}
				dialc->ChangeTimes(newStartTrimed - startTrimed, (newEndTrimed - endTrimed));
			}

			if(correctEndTimes>0 || PostprocessorOptions>16){
				if(correctEndTimes>1){
					int endt=Options.GetInt(ConvertTimePerLetter);
					int newend=(endt*dialc->Text.Len());
					if(newend<1000){newend=1000;}
					newend+=dialc->Start.mstime;
					dialc->End.NewTime(newend);
				}
				if(PostprocessorOptions & 1){dialc->Start.Change(-li);dialc->State=1 + (dialc->State & 4);}
				if(PostprocessorOptions & 2){dialc->End.Change(lo);dialc->State=1 + (dialc->State & 4);}
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
			dialc = cur->first;//file->GetDialogue(cur->second);
			it++;
			if(!(it!=tmpmap.end())){it=cur; hasend=true;}
			if(correctEndTimes>0 && dialc->End > it->first->Start && !hasend){
				dialc->End = it->first->Start;
				dialc->State=1 + (dialc->State & 4);
			}
			if(PostprocessorOptions & 4){
				int cdiff= (te+ts);
				int tdiff = it->first->Start.mstime - dialc->End.mstime;
				if(newstarttime != -1){
					dialc->Start.NewTime(newstarttime);
					newstarttime = -1;
					dialc->State=1 + (dialc->State & 4);
				}
				if(tdiff <= cdiff && tdiff > 0){
					int wsp = ((float)tdiff / (float)cdiff)*te;
					int newtime=ZEROIT(wsp);
					dialc->End.Change(newtime);
					newstarttime = dialc->End.mstime;
					dialc->State=1 + (dialc->State & 4); 

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
					dialc->State=1 + (dialc->State & 4);
				}
				if(endres!=-1 && endres <= pore){
					dialc->End.NewTime(endres);
					dialc->State=1 + (dialc->State & 4);
				}
			}
			dialc->ClearParse();
		}

	}

	SpellErrors.clear();
	int tmpMarked = markedLine;
	SetModified(SHIFT_TIMES, true, false, -1, false);
	markedLine = tmpMarked;
	if(subsFormat>TMP){RefreshColumns(START|END);}else{Refresh(false);}
#if _DEBUG
	wxBell();
#endif
}


void SubsGridBase::SortIt(short what, bool all)
{
	SaveSelections();
	std::vector<Dialogue*> selected;
	if(all){
		for(int i=0;i<GetCount();i++){file->GetDialogue(i)->State=1 + (file->GetDialogue(i)->State & 4);}
	}else{
		for(auto cur=Selections.begin(); cur!=Selections.end(); cur++){
			Dialogue *dial=file->GetDialogue(*cur);
			dial->State=1 + (dial->State & 4);
			selected.push_back(dial);
		}
	}
	std::sort((all)? file->subs->dials.begin() : selected.begin(), (all)? file->subs->dials.end() : selected.end(),
		(what==0)? sortstart : (what==1)? sortend : (what==2)? sortstyle : 
		(what==3)? sortactor : (what==4)? sorteffect : sortlayer);

	if(!all){
		int ii=0;
		for(auto cur=Selections.begin(); cur!=Selections.end(); cur++){
			(*file)[*cur] = selected[ii++];
		}
		selected.clear();
	}
	file->edited=true;
	file->ReloadVisibleDialogues();
	SpellErrors.clear();
	SetModified(GRID_SORT_LINES);
	Refresh(false);
} 


void SubsGridBase::DeleteRow(int rw, int len)
{
	int rwlen=rw+len;
	file->DeleteDialogues(rw, rwlen);
	if((int)SpellErrors.size()>rwlen){ SpellErrors.erase(SpellErrors.begin()+rw, SpellErrors.begin()+rwlen);}
	else{SpellErrors.clear();}
}

void SubsGridBase::DeleteRows()
{
	Freeze();
	for (auto i = Selections.rbegin(); i != Selections.rend(); i++)
	{
		int sel = *i;
		file->subs->dials.erase(file->subs->dials.begin() + file->IdConverter->getElementById(sel));
		SpellErrors.erase(SpellErrors.begin() + sel);
		file->IdConverter->deleteItemById(sel);
	}
	if (Selections.size()>0){ file->edited = true; }
	SaveSelections(true);
	if(GetCount()<1){AddLine(new Dialogue());}
	SetModified(GRID_DELETE_LINES);
	Thaw();
	RefreshColumns();
}

void SubsGridBase::MoveRows(int step, bool sav)
{
	SaveSelections();
	wxArrayInt sels=GetSels();

	if (sels.GetCount()<1){return;}
	if(step<0){

		for (size_t i=0;i<sels.GetCount();i++)
		{
			int istep=sels[i]+step;
			//if(istep<tmpstep){istep=tmpstep;tmpstep++;}
			if(istep<0){break;}
			Selections.insert(istep);
			Selections.erase(Selections.find(sels[i]));
			if(step!=-1){
				InsertRows(istep,1,GetDialogue(sels[i]),false);
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
			Selections.insert(istep);
			Selections.erase(Selections.find(sels[i]));
			if(step!=1){
				//break;
				Dialogue *dial=GetDialogue(sels[i]);
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

void SubsGridBase::DeleteText()
{
	for (int i=0;i<GetCount();i++)
	{
		if(Selections.find(i)!=Selections.end()){
			CopyDial(i)->Text="";}
	}
	SetModified(GRID_DELETE_TEXT);
	Refresh(false);
}
void SubsGridBase::UpdateUR(bool toolbar)
{
	bool undo=false, _redo=false;
	file->GetURStatus(&undo, &_redo);
	Kai->Menubar->Enable(Undo,undo);
	Kai->Menubar->Enable(Redo,_redo);
	Kai->Menubar->Enable(SaveSubs, true);
	if(toolbar){
		Kai->Toolbar->UpdateId(Undo,undo);
		Kai->Toolbar->UpdateId(Redo,_redo);
		Kai->Toolbar->UpdateId(SaveSubs, true);
	}
}

void SubsGridBase::GetUndo(bool redo, int iter)
{
	TabPanel *pan =Kai->GetTab();
	Freeze();
	wxString resolution = GetSInfo("PlayResX") +" x "+ GetSInfo("PlayResY");
	wxString matrix = GetSInfo("YCbCr Matrix");
	wxString tlmode = GetSInfo("TLMode");
	SaveSelections();
	if(iter != -2){if(file->SetHistory(iter)){Thaw();return;}}
	else if(redo){file->Redo();}else{file->Undo();}


	UpdateUR();

	Kai->Label(file->Iter());


	char oldform=subsFormat;SetSubsForm();
	if(oldform!=subsFormat){
		pan->CTime->Contents();
		pan->Edit->HideControls();
		Kai->UpdateToolbar();
	}

	Selections = file->subs->sel;
	Thaw();

	if(StyleStore::HasStore()){
		StyleStore *SS = StyleStore::Get();
		SS->ASS->SetArray(&file->subs->styles);
		SS->ASS->Refresh(false);
	}
	SpellErrors.clear();

	wxString newtlmode = GetSInfo("TLMode");
	if(newtlmode != tlmode){
		hasTLMode = (newtlmode == "Yes");
		showOriginal = (GetSInfo("TLMode Showtl") == "Yes" || (hasTLMode && Options.GetBool(TlModeShowOriginal) != 0));
		Edit->SetTl(hasTLMode);
	}

	int tmpMarked = markedLine;
	Edit->SetLine(MAX(0,file->subs->activeLine-1));
	if (tmpMarked < GetCount())
		markedLine = tmpMarked;
	RefreshColumns();
	Edit->RefreshStyle();
	
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
	

	if(makebackup){
		timer.Start(20000,true);
		//CreateTimerQueueTimer(&qtimer,0,WAITORTIMERCALLBACK(OnBcktimer),this,20000,0,WT_EXECUTEONLYONCE);
		makebackup=false;
	}
}

void SubsGridBase::DummyUndo(int newIter)
{
	file->DummyUndo(newIter);
	SpellErrors[Edit->ebrow].clear();
	Edit->SetLine(Edit->ebrow,false, false);
	RefreshColumns();
	UpdateUR();
	Kai->Label(newIter);
	VideoCtrl *vb = Kai->GetTab()->Video;
	if(vb->GetState() != None){
		vb->OpenSubs(GetVisible());
		vb->Render();
	}
}

int SubsGridBase::FirstSel()
{
	if (!Selections.empty()){
		return *Selections.begin();
	}
	return -1;
}

wxArrayInt SubsGridBase::GetSels(bool deselect)
{
	wxArrayInt sels;
	for(std::set<int>::iterator i=Selections.begin();i!=Selections.end();i++)
	{
		sels.Add(*i);
	}
	if(deselect){Selections.clear();}

	return sels;
}

//Uważaj na dodawanie do niszczarki, 
//bo brak dodania gdy trzeba to wycieki pamięci,
//a podwójne dodanie to krasz przy niszczeniu obiektu.
void SubsGridBase::InsertRows(int Row, 
						  const std::vector<Dialogue *> &RowsTable,
						  bool AddToDestroy)
{
	int convertedRow = file->IdConverter->getElementById(Row);
	file->subs->dials.insert(file->subs->dials.begin() + convertedRow, RowsTable.begin(), RowsTable.end());
	for (int i = 0; i < RowsTable.size(); i++){
		if (RowsTable[i]->isVisible){ 
			file->IdConverter->insert(i + convertedRow);
		}
	}
	wxArrayInt emptyarray;
	SpellErrors.insert(SpellErrors.begin()+Row,RowsTable.size(),emptyarray);
	if(AddToDestroy){file->subs->ddials.insert(file->subs->ddials.end(), RowsTable.begin(), RowsTable.end());}
}

//Uważaj na dodawanie do niszczarki, 
//bo brak dodania gdy trzeba to wycieki pamięci,
//a podwójne dodanie to krasz przy niszczeniu obiektu.
void SubsGridBase::InsertRows(int Row, int NumRows, Dialogue *Dialog, bool AddToDestroy, bool Save)
{
	SaveSelections();
	int convertedRow = file->IdConverter->getElementById(Row);
	file->subs->dials.insert(file->subs->dials.begin() + convertedRow, NumRows, Dialog);
	for (int i = convertedRow; i < convertedRow + NumRows; i++){
		if (file->subs->dials[i]->isVisible){ 
			file->IdConverter->insert(i); 
		}
	}
	wxArrayInt emptyarray;
	SpellErrors.insert(SpellErrors.begin()+Row,NumRows,emptyarray);
	if(AddToDestroy){file->subs->ddials.push_back(Dialog);}
	if(Save){
		Selections.insert(Row);
		SetModified(GRID_INSERT_ROW);
		Refresh(false);
	}
}

void SubsGridBase::SetSubsForm(wxString ext)
{
	subsFormat=ASS;
	int rw=0;
	char subsext=(ext=="ass"||ext=="ssa")? ASS: (ext=="srt")? SRT : TMP;
	while(rw<GetCount())
	{
		Dialogue *dial=GetDialogue(rw);
		if (dial->NonDialogue || dial->Form==0){rw++;}
		else if(!ext.empty() && (subsext!=dial->Form || (subsext==TMP && dial->Form>SRT))){rw++;}//form=dial->Form; 
		else{subsFormat=dial->Form; break;}
	}
}


void SubsGridBase::AddSInfo(const wxString &SI, wxString val, bool save)
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

wxString SubsGridBase::GetSInfos(bool tld)
{
	wxString TextSI="";
	for (std::vector<SInfo*>::iterator cur= file->subs->sinfo.begin();cur!=file->subs->sinfo.end();cur++) {
		if(!(tld&& (*cur)->Name.StartsWith("TLMode"))){
			TextSI<<(*cur)->Name << ": " << (*cur)->Val << "\r\n";
		}
	}
	return TextSI;
}
//wszystkie set modified trzeba znaleźć i dodać editiontype.
void SubsGridBase::SetModified(unsigned char editionType, bool redit, bool dummy, int SetEditBoxLine, bool Scroll)
{
	if(file->IsNotSaved()){
		if(file->Iter()<1||!Modified){
			Kai->Toolbar->UpdateId(SaveSubs, true);
			Kai->Menubar->Enable(SaveSubs, true);
			Modified=true;
		}
		if(Comparison){
			Kai->Tabs->SubsComparison();
		}
		if (!savedSelections){
			SaveSelections();
		}
		savedSelections = false;
		Kai->Label(file->Iter()+1);
		int ebrow = Edit->ebrow;	
		if(redit){
			int erow= (SetEditBoxLine >= 0)? SetEditBoxLine : ebrow;
			if(erow>=GetCount()){erow=GetCount()-1;}
			lastRow=erow;
			if(scPos>erow && Scroll){scPos=MAX(0,(erow-4));}
			Edit->SetLine(erow);
			Selections.insert(erow);
		}
		file->SaveUndo(editionType, ebrow+1);
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

		if(makebackup){
			timer.Start(20000, true);
			//CreateTimerQueueTimer(&qtimer,0,WAITORTIMERCALLBACK(OnBcktimer),this,20000,0,WT_EXECUTEONLYONCE);
			makebackup=false;
		}
		UpdateUR();
	}
}

void SubsGridBase::SwapRows(int frst, int scnd, bool sav)
{

	Dialogue *tmp=file->GetDialogue(frst);
	(*file)[frst]=file->GetDialogue(scnd);
	(*file)[scnd] = tmp;
	wxArrayInt tmpspell=SpellErrors[frst];
	SpellErrors[frst]=SpellErrors[scnd];
	SpellErrors[scnd] = tmpspell;
	Refresh(false);
	if(sav){SetModified(GRID_SWAP_LINES);}
}

void SubsGridBase::Loadfile(const wxString &str,const wxString &ext){

	Clearing();
	int active=0;
	file=new SubsFile();

	if(ext=="srt"){

		wxStringTokenizer tokenizer(str,"\n",wxTOKEN_STRTOK);
		tokenizer.GetNextToken();

		wxString text1;
		while ( tokenizer.HasMoreTokens() ){
			wxString text=tokenizer.GetNextToken().Trim();
			if(IsNumber(text)){if(text1!=""){
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
		if(hasTLMode&&!tlmode){
			hasTLMode=false;
			showOriginal=false;
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
			if (isASS && !(token.StartsWith("Dial") || token.StartsWith("Comm") || token.StartsWith(";"))){ continue; }
			Dialogue *dl= new Dialogue(token);
			if(!tlmode){
				AddLine(dl);
			}
			else if(tlmode && dl->Style==tlstyle){
				wxString ntoken = tokenizer.GetNextToken();
				Dialogue tl(ntoken);
				dl->Style=tl.Style; 
				dl->TextTl=tl.Text;
				if(dl->Effect == "\fD"){
					dl->State |=4;
				}
				dl->Effect = tl.Effect;
				AddLine(dl);
			}else{
				//stary tryb tłumaczenia nie istnieje od 2 lat, nie ma sensu usuwać pustych linii, zważywszy na to, że to może usunąć coś ważnego.
				AddLine(dl);
			}
		}


	}

	if(GetCount()<1){LoadDefault();KaiMessageBox(_("Niepoprawny format (plik uszkodzony lub zawiera błędy)"));subsFormat=ASS;}
	else{SetSubsForm();}
	originalFormat=subsFormat;

	if(GetSInfo("TLMode")=="Yes"){
		Edit->SetTl(true);
		hasTLMode=true;
		if(GetSInfo("TLMode Showtl")=="Yes" || Options.GetBool(TlModeShowOriginal)){showOriginal=true;}
		Kai->Menubar->Enable(SaveTranslation,true);
	}


	if(subsFormat==MDVD||subsFormat==MPL2){
		int endt=Options.GetInt(ConvertTimePerLetter);
		for(int i=0;i<GetCount();i++){
			Dialogue *dial=GetDialogue(i);

			if(dial->End.mstime==0){
				int newend=(endt*dial->Text.Len());
				if(newend<1000){newend=1000;}
				newend+=dial->Start.mstime;
				dial->End.NewTime(newend);
				if(i<GetCount()-1){
					if(dial->End>file->GetDialogue(i+1)->Start){
						dial->End=file->GetDialogue(i+1)->Start;
					}
				}
			}
		}
	}
	else if(subsFormat==ASS){
		if(ext!="ass"){originalFormat=0;AddStyle(new Styles());}
		Edit->TlMode->Enable(true);Edit->RefreshStyle();
		if(Options.GetBool(GridLoadSortedSubs)){
			std::sort(file->subs->dials.begin(), file->subs->dials.end(), sortstart);
		}
	}
	else{Edit->TlMode->Enable(false);}
	if(active>=GetCount()){active=0;}


	Selections.insert(active);
	lastRow=active;
	markedLine=active;

	scPos=MAX(0,active-3);
	file->subs->sel = Selections;
	file->EndLoad(OPEN_SUBTITLES, active + 1);

	RefreshColumns();
	Edit->OnVideo = true;
	Edit->SetLine(active,false,false);

	Edit->HideControls();

	if(StyleStore::HasStore() && subsFormat==ASS){StyleStore::Get()->LoadAssStyles();}
	if(subsFormat == ASS){RebuildActorEffectLists();}
	
}

void SubsGridBase::SetStartTime(int stime)
{
	Edit->Send(EDITBOX_LINE_EDITION,false,false,true);
	wxArrayInt sels=GetSels();
	for(size_t i=0;i<sels.size();i++){
		Dialogue *dialc=CopyDial(sels[i]);
		if(!dialc){continue;}
		dialc->Start.NewTime(stime);
		if(dialc->End<stime){dialc->End.NewTime(stime);}
	}
	if(sels.size()){
		SetModified(GRID_SET_START_TIME);
		Refresh(false);
	}
}

void SubsGridBase::SetEndTime(int etime)
{
	Edit->Send(EDITBOX_LINE_EDITION,false,false,true);
	wxArrayInt sels=GetSels();
	for(size_t i=0;i<sels.size();i++){
		Dialogue *dialc=CopyDial(sels[i]);
		if(!dialc){continue;}
		dialc->End.NewTime(etime);
		if(dialc->Start>etime){dialc->Start.NewTime(etime);}
	}
	if(sels.size()){
		SetModified(GRID_SET_END_TIME);
		Refresh(false);
	}
}

bool SubsGridBase::SetTlMode(bool mode)
{
	if(mode){
		if(GetSInfo("TLMode")==""){
			//for(int i=0;i<GetCount();i++){file->GetDialogue(i)->spells.clear();}

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
		hasTLMode=true;
		if(Options.GetBool(TlModeShowOriginal)){showOriginal = true;}
		Kai->Menubar->Enable(SaveTranslation,true);

		//Refresh(false);

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
			Dialogue *dial= GetDialogue(i);
			Dialogue *dialc= NULL;
			if(dial->TextTl!="")
			{
				dialc = CopyDial(i);
				dialc->Text = dialc->TextTl;
				dialc->TextTl="";
			}
			if(dial->State >= 4){
				if(!dialc){dialc = CopyDial(i);}
				dialc->State -= 4;
			}
		}

		hasTLMode=false;
		showOriginal=false;
		Kai->Menubar->Enable(SaveTranslation,false);
	}
	SpellErrors.clear();
	Refresh(false);
	SetModified((mode)? GRID_TURN_ON_TLMODE : GRID_TURN_OFF_TLMODE);
	//VideoCtrl *vb = ((TabPanel*)GetParent())->Video;
	//if(vb->GetState()!=None){
	//	vb->OpenSubs(GetVisible()/*SaveText()*/);
	//	if(vb->GetState()==Paused){vb->Render();}
	//	Edit->OnVideo=true;
	//}
	return false;
}

void SubsGridBase::NextLine(int dir)
{
	if(Edit->ABox && Edit->ABox->audioDisplay->hold!=0){return;}
	int nebrow=Edit->ebrow+dir;
	if(nebrow<0){return;}
	if(nebrow>=GetCount()){
		Dialogue *tmp=GetDialogue(GetCount()-1)->Copy();
		int eend= tmp->End.mstime; 
		tmp->Start.NewTime(eend); 
		tmp->End.NewTime(eend+5000);
		tmp->Text="";
		tmp->TextTl="";
		AddLine(tmp);
		SetModified(GRID_APPEND_LINE, false);
	}
	int h,w;
	GetClientSize(&w,&h);
	scPos = MID(0, nebrow-((h/(GridHeight+1))/2), GetCount()-1);
	Selections.clear();
	Selections.insert(nebrow);
	lastRow = nebrow;
	AdjustWidths(0);
	Refresh(false);
	Edit->SetLine(nebrow,true,true,false,true);
	//if(Edit->ABox){Edit->ABox->audioDisplay->SetDialogue(Edit->line,nebrow);}
	
}



void SubsGridBase::LoadDefault(bool line,bool sav,bool endload)
{
	if(line)
	{
		AddLine(new Dialogue());
		AddStyle(new Styles());
		Selections.insert(0);
		originalFormat=subsFormat=ASS;
	}
	AddSInfo("Title","Kainote Ass File",sav);
	AddSInfo("PlayResX","1280",sav);
	AddSInfo("PlayResY","720",sav);
	AddSInfo("ScaledBorderAndShadow","yes",sav);
	AddSInfo("WrapStyle","0",sav);
	AddSInfo("ScriptType","v4.00+",sav);
	AddSInfo("Last Style Storage","Default",sav);
	AddSInfo("YCbCr Matrix","TV.601",sav);
	//Kai->Toolbar->UpdateId(SaveSubs, false);
	//Kai->Menubar->Enable(SaveSubs, false);
	if(endload){
		file->subs->sel = Selections;
		file->EndLoad(NEW_SUBTITLES, 1);
	}
}

Dialogue *SubsGridBase::CopyDial(int i, bool push)
{
	if(push && (int)SpellErrors.size() > i){SpellErrors[i].Clear();}
	return file->CopyDial(i, push);
}

Dialogue *SubsGridBase::GetDialogue(int i)
{
	if(i >= (int)file->subs->dials.size()){
		return NULL;
	}
	return file->GetDialogue(i);
}

wxString SubsGridBase::GetSInfo(const wxString &key, int *ii)
{
	int i=0;
	for(std::vector<SInfo*>::iterator it=file->subs->sinfo.begin(); it!=file->subs->sinfo.end(); it++)
	{
		if(key==(*it)->Name) {if(ii){*ii=i;} return (*it)->Val;}
		i++;
	}
	return "";
}

SInfo *SubsGridBase::GetSInfoP(const wxString &key,int *ii)
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

int SubsGridBase::SInfoSize()
{
	return file->subs->sinfo.size();
}

wxString *SubsGridBase::SaveText()
{
	wxString *path=new wxString();

	TabPanel *tab = (TabPanel*)GetParent();
	wxString ext=(subsFormat<SRT)? "ass" : (subsFormat==SRT)? "srt" : "txt";

	(*path)<<Options.pathfull<<"\\Subs\\DummySubs_"<<Notebook::GetTabs()->FindPanel(tab)<<"."<<ext;

	SaveFile(*path, false, true);

	return path;
}


wxString *SubsGridBase::GetVisible(bool *visible, wxPoint *point, wxArrayInt *selected)
{
	TabPanel *pan=(TabPanel*)GetParent();
	int _time=pan->Video->Tell();
	bool toEnd = pan->Video->GetState() == Playing;
	wxString *txt=new wxString();
	if(subsFormat== ASS){
		(*txt)<<"[Script Info]\r\n"<<GetSInfos(false);
		(*txt)<<"\r\n[V4+ Styles]\r\nFormat: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding \r\n";
		(*txt)<<GetStyles(false);
		(*txt)<<" \r\n[Events]\r\nFormat: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\r\n";
	}
	Edit->Send(EDITBOX_LINE_EDITION, false, true);
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
		Dialogue *dial=GetDialogue(i);
		if(i==Edit->ebrow){ 
			dial = Edit->line;
		}
		if(selected && Selections.find(i)!=Selections.end()){
			selected->Add(txt->Len());
			continue;
		}
		if((toEnd && _time <= dial->Start.mstime) || (_time >= dial->Start.mstime && _time <= dial->End.mstime)){
			//if(trimSels && sel.find(i)!=sel.end()){continue;}
			if( isTlmode && dial->TextTl!=""){
				dial->GetRaw(txt, false, tlStyle);
				dial->GetRaw(txt, true);
			}else{
				dial->GetRaw(txt);
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
		if (subsFormat == ASS){
			Dialogue().GetRaw(txt);
		}
		else{
			delete txt;
			return NULL;
		}
	}

	return txt;
}


void SubsGridBase::OnBackupTimer(wxTimerEvent &event)
{
	TabPanel *pan=(TabPanel*)GetParent();
	Kai->SetStatusText(_("Autozapis"),0);
	wxString path;
	wxString ext=(subsFormat<SRT)? "ass" : (subsFormat==SRT)? "srt" : "txt";

	path<<Options.pathfull<<"\\Subs\\"<<pan->SubsName.BeforeLast('.')
		<<"_"<<Notebook::GetTabs()->FindPanel(pan)<<"_"<< numsave <<"."<<ext;

	SaveFile(path, false);
	int maxFiles = Options.GetInt(AutoSaveMaxFiles);
	if(maxFiles>1 && numsave >= maxFiles){numsave=0;}
	numsave++;
	makebackup=true;
	nullifyTimer.Start(5000,true);
}

void SubsGridBase::GetASSRes(int *x,int *y)
{
	wxString oldx=GetSInfo("PlayResX");
	wxString oldy=GetSInfo("PlayResY");
	int nx=wxAtoi(oldx);
	int ny=wxAtoi(oldy);
	bool changed = false;
	if(nx<1 && ny<1){
		nx=384;ny=288;
		AddSInfo("PlayResX", "384");
		AddSInfo("PlayResY", "288");
		changed = true;
	}
	else if(nx<1){
		nx=(float)ny*(4.0/3.0);if(ny==1024){nx=1280;}
		AddSInfo("PlayResX", std::to_string(nx));
		changed = true;
	}
	else if(ny<1){
		ny=(float)nx*(3.0/4.0);if(nx==1280){ny=1024;}
		AddSInfo("PlayResY", std::to_string(ny));
		changed = true;
	}
	*x=nx;
	*y=ny;
	//if(changed){SetModified(ASS_PROPERTIES, false, true, -1, false);}
}


void SubsGridBase::RebuildActorEffectLists()
{
	Edit->ActorEdit->Clear();
	Edit->EffectEdit->Clear();
	for(int i = 0; i<GetCount(); i++){
		Dialogue *dial = GetDialogue(i);
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

void SubsGridBase::SaveSelections(bool clear)
{
	file->undo[file->iter]->sel = Selections;
	file->undo[file->iter]->activeLine = Edit->ebrow+1;
	savedSelections = true;
	if (clear){ Selections.clear(); }
}





