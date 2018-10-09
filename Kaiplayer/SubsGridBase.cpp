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
#include "SubsLoader.h"
#include "SubsGridFiltering.h"
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
#include "SubsGridPreview.h"


bool sortstart(Dialogue *i, Dialogue *j){
	if (i->treeState != j->treeState)
		return i->treeState < j->treeState;
	if (i->Start.mstime != j->Start.mstime){
		return (i->Start.mstime < j->Start.mstime);
	}
	return i->End.mstime < j->End.mstime;
}
bool sortend(Dialogue *i, Dialogue *j){
	if (i->treeState != j->treeState)
		return i->treeState < j->treeState;
	if (i->End.mstime != j->End.mstime){
		return (i->End.mstime < j->End.mstime);
	}
	return i->Start.mstime < j->Start.mstime;
}
bool sortstyle(Dialogue *i, Dialogue *j){
	if (i->treeState != j->treeState)
		return i->treeState < j->treeState;
	if (i->Style != j->Style){
		return (i->Style.CmpNoCase(j->Style) < 0);
	}
	return i->Start.mstime < j->Start.mstime;
}
bool sortactor(Dialogue *i, Dialogue *j){
	if (i->treeState != j->treeState)
		return i->treeState < j->treeState;
	if (i->Actor != j->Actor){
		return (i->Actor.CmpNoCase(j->Actor) < 0);
	}
	return i->Start.mstime < j->Start.mstime;
}
bool sorteffect(Dialogue *i, Dialogue *j){
	if (i->treeState != j->treeState)
		return i->treeState < j->treeState;
	if (i->Effect != j->Effect){
		return (i->Effect.CmpNoCase(j->Effect) < 0);
	}
	return i->Start.mstime < j->Start.mstime;
}
bool sortlayer(Dialogue *i, Dialogue *j){
	if (i->treeState != j->treeState)
		return i->treeState < j->treeState;
	if (i->Layer != j->Layer){
		return (i->Layer < j->Layer);
	}
	return i->Start.mstime < j->Start.mstime;
}


SubsGridBase::SubsGridBase(wxWindow *parent, const long int id, const wxPoint& pos, const wxSize& size, long style)
	: KaiScrolledWindow(parent, id, pos, size, style | wxVERTICAL)
{
	file = new SubsFile();
	makebackup = true;
	ismenushown = false;
	showFrames = false;
	Comparison = NULL;
	numsave = 0;

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
	SAFE_DELETE(Comparison);
	SAFE_DELETE(file);
	SpellErrors.clear();
	isFiltered = false;
	showOriginal = false;
	first = true;
	scPos = 0;
	lastRow = 0;
	scHor = 0;
}
void SubsGridBase::AddLine(Dialogue *line)
{
	file->subs->ddials.push_back(line);
	file->subs->dials.push_back(line);
	if (*line->isVisible){
		file->IdConverter->insert(file->subs->dials.size() - 1);
	}
}

void SubsGridBase::ChangeLine(unsigned char editionType, Dialogue *line1, int wline, long cells, bool selline, bool dummy)
{
	lastRow = wline;
	wxArrayInt sels;
	file->GetSelections(sels);
	if (sels.size() < 2){
		ChangeCell(cells, wline, line1);
	}
	else{
		for (size_t i = 0; i < sels.size(); i++){
			ChangeCell(cells, sels[i], line1);
		}
	}
	
	AdjustWidths(cells);
	if (selline)
		NextLine();
	else{
		Refresh(false);
	}
	SetModified(editionType, false, dummy);
}

void SubsGridBase::ChangeCell(long wcell, int wline, Dialogue *what)
{
	Dialogue *dial = CopyDialogue(wline);
	if (wcell & LAYER){
		dial->Layer = what->Layer;
	}
	if (wcell & START){
		dial->Start = what->Start;
	}
	if (wcell & END){
		dial->End = what->End;
	}
	if (wcell & STYLE){
		dial->Style = what->Style;
	}
	if (wcell & ACTOR){
		dial->Actor = what->Actor;
	}
	if (wcell & MARGINL){
		dial->MarginL = what->MarginL;
	}
	if (wcell & MARGINR){
		dial->MarginR = what->MarginR;
	}
	if (wcell & MARGINV){
		dial->MarginV = what->MarginV;
	}
	if (wcell & EFFECT){
		dial->Effect = what->Effect;
	}
	if (wcell & TXT){
		dial->Text = what->Text;
	}
	if (wcell & COMMENT){
		dial->IsComment = what->IsComment;
	}
	if (wcell & TXTTL){
		dial->TextTl = what->TextTl;
	}
}


void SubsGridBase::Convert(char type)
{
	if (Options.GetBool(ConvertShowSettings)){
		OptionsDialog od(Kai, Kai);
		od.OptionsTree->ChangeSelection(2);
		od.okok->SetFocus();
		if (od.ShowModal() == wxID_CANCEL){ return; }
	}
	if (Options.GetBool(ConvertFPSFromVideo) && Kai->GetTab()->VideoPath != ""){
		Options.SetString(ConvertFPS, Kai->GetStatusText(4).BeforeFirst(' '));
	}
	if (Options.GetFloat(ConvertFPS) < 1){ KaiMessageBox(_("Nieprawidłowy FPS. Popraw opcje i spróbuj ponownie.")); return; }

	bool newendtimes = Options.GetBool(ConvertNewEndTimes);
	const wxString & stname = Options.GetString(ConvertStyle);
	int endt = Options.GetInt(ConvertTimePerLetter);
	const wxString & prefix = Options.GetString(ConvertASSTagsOnLineStart);

	int i = 0;
	File *subs = file->GetSubs();
	auto &dials = subs->dials;
	while (i<dials.size())
	{
		if ((type>ASS) && (subsFormat < SRT) && dials[i]->IsComment){
			while (dials[i]->IsComment){
				file->DeleteDialoguesByKeys(i, i + 1);
			}
		}
		Dialogue *dialc = file->CopyDialogueByKey(i);
		dialc->Convert(type, prefix);
		if ((newendtimes && type != TMP) || subsFormat == TMP)
		{
			if (i > 0){
				if (dials[i - 1]->End.mstime > dialc->Start.mstime){
					dials[i - 1]->End = dialc->Start;
				}
			}

			int newend = (endt * dialc->Text.Len());
			if (newend < 1000){ newend = 1000; }
			newend += dialc->Start.mstime;
			dialc->End.NewTime(newend);
		}

		i++;
	}

	if (type == ASS){
		LoadDefault(false, true, false);
		wxString resx = Options.GetString(ConvertResolutionWidth);
		wxString resy = Options.GetString(ConvertResolutionHeight);
		if (resx == ""){ resx = "1280"; }
		if (resy == ""){ resx = "720"; }
		AddSInfo("PlayResX", resx, false);
		AddSInfo("PlayResY", resy, false);
		AddSInfo("YCbCr Matrix", "TV.601", false);
		const wxString & catalog = Options.GetString(ConvertStyleCatalog);

		if (Options.dirs.Index(catalog) != -1){ Options.LoadStyles(catalog); }
		int stind = Options.FindStyle(stname);

		if (stind < 0){
			Styles *newstyl = new Styles();
			newstyl->Name = stname;
			AddStyle(newstyl);
		}
		else{ AddStyle(Options.GetStyle(stind)->Copy()); }
		Edit->RefreshStyle();
	}
	if (subsFormat == ASS){
		std::sort(file->subs->dials.begin(), file->subs->dials.end(), [](Dialogue *i, Dialogue *j){
			if (i->Start.mstime != j->Start.mstime){
				return (i->Start.mstime < j->Start.mstime);
			}
			if (i->End.mstime != j->End.mstime){
				return (i->End.mstime < j->End.mstime);
			}
			return (i->Text.CmpNoCase(j->Text) < 0);
		});
		Dialogue *lastDialogue = dials[0];
		int i = 1;
		while (i < dials.size()){
			Dialogue *actualDialogue = dials[i];
			if (lastDialogue->Start == actualDialogue->Start &&
				lastDialogue->End == actualDialogue->End &&
				lastDialogue->Text == actualDialogue->Text){
				file->DeleteDialoguesByKeys(i - 1, i);
				lastDialogue = actualDialogue;
				continue;
			}
			else if (actualDialogue->Text == ""){
				file->DeleteDialoguesByKeys(i, i + 1);
				continue;
			}
			lastDialogue = actualDialogue;
			i++;
		}
		
	}
	char oldSubsFormat = subsFormat;
	subsFormat = type;
	if (type == ASS || oldSubsFormat == ASS){
		Kai->SetSubsResolution();
	}
	file->ReloadVisibleDialogues();
	Edit->SetLine((currentLine < GetCount()) ? currentLine : 0);
	SpellErrors.clear();
	SetModified(GRID_CONVERT);
	RefreshColumns();
}

void SubsGridBase::SaveFile(const wxString &filename, bool cstat, bool loadFromEditbox)
{
	int saveAfterCharacterCount = Options.GetInt(GridSaveAfterCharacterCount);
	bool showOriginalOnVideo = !Options.GetBool(TL_MODE_HIDE_ORIGINAL_ON_VIDEO);
	bool dummyEditboxChanges = (loadFromEditbox && !saveAfterCharacterCount);
	if (dummyEditboxChanges || saveAfterCharacterCount > 1){
		Edit->Send(EDITBOX_LINE_EDITION, false, dummyEditboxChanges, true);
	}
	wxString txt;
	const wxString &tlmode = GetSInfo("TLMode");
	bool translated = tlmode == "Translated";
	bool tlmodeOn = tlmode != "";

	OpenWrite ow(filename, true);

	if (subsFormat < SRT){
		if (cstat){
			AddSInfo("Last Style Storage", Options.actualStyleDir, false);
			AddSInfo("Active Line", std::to_string(currentLine), false);
			TabPanel *tab = (TabPanel*)GetParent();
			wxString subsPath = tab->SubsPath.BeforeLast('\\');
			if (Edit->ABox){
				wxString path = (Edit->ABox->audioName.StartsWith(subsPath)) ? 
					Edit->ABox->audioName.AfterLast('\\') : Edit->ABox->audioName;
				AddSInfo("Audio File", path, false);
			}
			if (!tab->VideoPath.empty()){
				wxString path = (tab->VideoPath.StartsWith(subsPath)) ?
					tab->VideoPath.AfterLast('\\') : tab->VideoPath;
				AddSInfo("Video File", path, false);
			}
			if (!tab->KeyframesPath.empty()){
				wxString path = (tab->KeyframesPath.StartsWith(subsPath)) ?
					tab->KeyframesPath.AfterLast('\\') : tab->KeyframesPath;
				AddSInfo("Keyframes File", path, false);
			}
		}
		
		txt << "[Script Info]\r\n;Script generated by " << Options.progname << "\r\n";
		GetSInfos(txt, translated);
		txt << "\r\n[V4+ Styles]\r\nFormat: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding \r\n";
		GetStyles(txt, translated);
		txt << " \r\n[Events]\r\nFormat: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\r\n";
	}
	ow.PartFileWrite(txt);

	txt = GetSInfo("TLMode Style");
	wxString raw;
	if (loadFromEditbox){
		int activeLineKey = file->GetElementById(currentLine);
		for (int i = 0; i < file->subs->dials.size(); i++)
		{
			Dialogue *dial = file->subs->dials[i];
			if (!ignoreFiltered && !dial->isVisible || dial->NonDialogue){ continue; }
			//a tu trzeba w przypadku ebrow pobrać editbox line
			if (i == activeLineKey){ dial = Edit->line; };

			if (tlmodeOn){
				bool hasTextTl = dial->TextTl != "";
				if (!translated && (hasTextTl || dial->IsDoubtful())){
					if (showOriginalOnVideo)
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
			Dialogue *dial = file->subs->dials[i];

			if (tlmodeOn){
				bool hasTextTl = dial->TextTl != "";
				if (!translated && (hasTextTl || dial->IsDoubtful())){
					dial->GetRaw(&raw, false, txt, !showOriginalOnVideo);
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

			if (cstat && dial->GetState() & 1){ dial->ChangeDialogueState(2); }

		}
	}

	ow.CloseFile();
	if (cstat){ 
		file->SetLastSave();
		Refresh(false); 
	}
}

void SubsGridBase::AddStyle(Styles *nstyl)
{
	file->subs->dstyles.push_back(nstyl);
	file->subs->styles.push_back(nstyl);
}

void SubsGridBase::ChangeStyle(Styles *nstyl, int i)
{
	file->subs->dstyles.push_back(nstyl);
	file->subs->styles[i] = nstyl;
}

int SubsGridBase::StylesSize()
{
	return file->subs->styles.size();
}

Styles *SubsGridBase::GetStyle(int i, const wxString &name)
{
	if (name != ""){
		for (unsigned int j = 0; j < file->subs->styles.size(); j++)
		{
			if (name == file->subs->styles[j]->Name){ return file->subs->styles[j]; }
		}
	}
	return file->subs->styles[i];
}

std::vector<Styles*> *SubsGridBase::GetStyleTable()
{
	return &file->subs->styles;
}

//multiplication musi być ustawione na zero, wtedy zwróci ilość multiplikacji
int SubsGridBase::FindStyle(const wxString &name, int *multip)
{
	int isfound = -1;
	for (unsigned int j = 0; j < file->subs->styles.size(); j++)
	{
		if (name == file->subs->styles[j]->Name){
			isfound = j;
			if (multip){
				*multip++;
			}
			else{ break; }
		}
	}
	return isfound;
}

void SubsGridBase::GetStyles(wxString &stylesText, bool tld/*=false*/)
{
	wxString tmpst;
	if (tld){ tmpst = GetSInfo("TLMode Style"); }
	for (size_t i = 0; i < file->subs->styles.size(); i++)
	{
		if (!(tld&&file->subs->styles[i]->Name == tmpst)){
			stylesText << file->subs->styles[i]->GetRaw();
		}
	}
}

void SubsGridBase::DelStyle(int i)
{
	file->edited = true;
	file->subs->styles.erase(file->subs->styles.begin() + i);
}

int SubsGridBase::GetCount()
{
	return file->IdConverter->size();
}

class compare { // simple comparison function
public:
	bool operator()(Dialogue *i, Dialogue *j) {

		if (i->Start.mstime != j->Start.mstime){
			return (i->Start.mstime < j->Start.mstime);
		}
		return i->End.mstime < j->End.mstime;
	} // returns x>y
};

void SubsGridBase::ChangeTimes(bool byFrame)
{
	VideoCtrl *vb = ((TabPanel*)GetParent())->Video;
	if (byFrame && !vb->VFF){ wxLogMessage(_("Wideo nie zostało wczytane przez FFMS2")); return; }
	//1 forward / backward, 2 Start Time For V/A Timing, 4 Move to video time, 8 Move to audio time;
	int moveTimeOptions = Options.GetInt(MoveTimesOptions);

	//Time to move
	int time = (!byFrame) ? Options.GetInt(MoveTimesTime) : 0;
	int frame = (byFrame) ? Options.GetInt(MoveTimesFrames) : 0;
	int whichLines = MAX(0, Options.GetInt(MoveTimesWhichLines));
	int whichTimes = MAX(0, Options.GetInt(MoveTimesWhichTimes));
	int correctEndTimes = Options.GetInt(MoveTimesCorrectEndTimes);
	//1 Lead In, 2 Lead Out, 4 Make times continous, 8 Snap to keyframe;
	int PostprocessorOptions = Options.GetInt(PostprocessorEnabling);
	int LeadIn = 0, LeadOut = 0, ThresholdStart = 0, ThresholdEnd = 0, 
		KeyframeBeforeStart = 0, KeyframeAfterStart = 0, KeyframeBeforeEnd = 0, KeyframeAfterEnd = 0;

	if (PostprocessorOptions){
		if (subsFormat == TMP || PostprocessorOptions < 16){ PostprocessorOptions = 0; }
		else if (PostprocessorOptions & 8 && !vb->VFF){ PostprocessorOptions ^= 8; }
		LeadIn = Options.GetInt(PostprocessorLeadIn);
		LeadOut = Options.GetInt(PostprocessorLeadOut);
		ThresholdStart = Options.GetInt(PostprocessorThresholdStart);
		ThresholdEnd = Options.GetInt(PostprocessorThresholdEnd);
		KeyframeBeforeStart = Options.GetInt(PostprocessorKeyframeBeforeStart);
		KeyframeAfterStart = Options.GetInt(PostprocessorKeyframeAfterStart);
		KeyframeBeforeEnd = Options.GetInt(PostprocessorKeyframeBeforeEnd);
		KeyframeAfterEnd = Options.GetInt(PostprocessorKeyframeAfterEnd);
	}
	wxString styles = Options.GetString(MoveTimesStyles);
	if (styles.empty() && whichLines == 5){
		KaiMessageBox(_("Nie wybrano stylów do przesunięcia"), _("Uwaga"));
		return;
	}
	else{
		styles = "," + styles + ",";
	}
	bool changeTagTimes = (moveTimeOptions & 32) > 0;

	if (!(moveTimeOptions & 1)){
		time = (-time);
		frame = (-frame);
	}
	// video / audio move start or end
	int VAS = moveTimeOptions & 2;

	std::map<Dialogue *, int, compare> tmpmap;

	if (whichTimes != 0){
		int answer = KaiMessageBox(wxString::Format(_("Czy naprawdę chcesz przesuwać tylko czasy %s?"),
			(whichTimes == 1) ? _("początkowe") : _("końcowe")), _("Potwierdzenie"), wxYES_NO);
		if (answer == wxNO){ return; }
	}

	if (subsFormat == TMP){ whichTimes = 1; }

	//bool fromStyle = false;

	int firstSelection = FirstSelection();
	if (firstSelection == -1 && whichLines != 0 && whichLines != 4){
		KaiMessageBox(_("Nie zaznaczono linii do przesunięcia"), _("Uwaga")); return;
	}

	int difftime = (VAS) ? file->GetDialogue(markedLine)->Start.mstime : file->GetDialogue(markedLine)->End.mstime;

	if ((moveTimeOptions & 4) && vb->GetState() != None){
		if (byFrame){
			frame += vb->GetCurrentFrame() - vb->VFF->GetFramefromMS(difftime);
		}
		else{
			int addedTimes = vb->GetFrameTime(VAS != 0) - difftime;
			if (addedTimes < 0){ addedTimes -= 10; }
			time += ZEROIT(addedTimes);
		}

	}
	else if ((moveTimeOptions & 8) && Edit->ABox && Edit->ABox->audioDisplay->hasMark){
		if (byFrame){
			frame += vb->VFF->GetFramefromMS(Edit->ABox->audioDisplay->curMarkMS - difftime);
		}
		else{
			int addedTimes = Edit->ABox->audioDisplay->curMarkMS - difftime;
			if (addedTimes < 0){ addedTimes -= 10; }
			time += ZEROIT(addedTimes);
		}
	}

	File *Subs = file->GetSubs();
	int firsttime = GetDialogue(firstSelection)->Start.mstime;
	Dialogue *dialc;
	Dialogue *Dial;
	bool skipFiltered = !ignoreFiltered;
	if (PostprocessorOptions){ time = 0; frame = 0; VAS = 0; whichTimes = 0; correctEndTimes = 0; moveTimeOptions = 0; }

	for (int i = 0; i < Subs->dials.size(); i++)
	{
		Dial = Subs->dials[i];
		if (skipFiltered && !Dial->isVisible || Dial->NonDialogue){ continue; }

		if (whichLines == 0
			|| (whichLines == 1 && file->IsSelectedByKey(i))//selected lines
			|| (whichLines == 3 && firsttime <= Dial->Start.mstime)//times higher or equal
			|| (whichLines == 2 && i >= firstSelection)//from selection
			|| (whichLines == 4 && firsttime >= Dial->Start.mstime)//times lower or equal
			|| (whichLines == 5 && styles.Find("," + Dial->Style + ",") != -1))//by choosen styles
		{

			dialc = file->CopyDialogueByKey(i, true, true);
			int startTrimed = 0, endTrimed = 0, duration = 0;
			if (changeTagTimes){
				vb->GetStartEndDelay(dialc->Start.mstime, dialc->End.mstime, &startTrimed, &endTrimed);
			}
			if (time != 0){
				if (whichTimes != 2){ dialc->Start.Change(time); }
				if (whichTimes != 1){ dialc->End.Change(time); }
				dialc->ChangeDialogueState(1);
			}
			else if (frame != 0){
				if (whichTimes == 0){
					duration = dialc->End.mstime - dialc->Start.mstime;
				}
				if (whichTimes != 2){
					int startFrame = vb->VFF->GetFramefromMS(dialc->Start.mstime) + frame;
					dialc->Start.NewTime(ZEROIT(vb->GetFrameTimeFromFrame(startFrame)));
				}
				if (whichTimes != 1){
					int endFrame = vb->VFF->GetFramefromMS(dialc->End.mstime) + frame;
					dialc->End.NewTime(ZEROIT(vb->GetFrameTimeFromFrame(endFrame)));
				}
				dialc->ChangeDialogueState(1);
			}
			if (changeTagTimes){
				int newStartTrimed = 0, newEndTrimed = 0;
				vb->GetStartEndDelay(dialc->Start.mstime, dialc->End.mstime, &newStartTrimed, &newEndTrimed);
				if (byFrame){ newEndTrimed += ((dialc->End.mstime - dialc->Start.mstime) - duration); }
				dialc->ChangeTimes(newStartTrimed - startTrimed, (newEndTrimed - endTrimed));
			}

			if (correctEndTimes > 0 || PostprocessorOptions > 16){
				/*if (correctEndTimes > 1){
					int endt = Options.GetInt(ConvertTimePerLetter);
					int newend = (endt*dialc->Text.Len());
					if (newend < 1000){ newend = 1000; }
					newend += dialc->Start.mstime;
					dialc->End.NewTime(newend);
				}*/
				if (PostprocessorOptions & 1){ dialc->Start.Change(-LeadIn); dialc->ChangeDialogueState(1); }
				if (PostprocessorOptions & 2){ dialc->End.Change(LeadOut); dialc->ChangeDialogueState(1); }
				if (correctEndTimes > 0 || PostprocessorOptions > 19){
					tmpmap[dialc] = i;

				}
			}
			else{
				dialc->ClearParse();
			}

		}// if shifted line

	}//loop for

	// tu jeszcze należy poprawić uwzględniając linijkę z czasem przed tablicą i czasem po niej
	// a może to w ogóle nie jest potrzebne?
	if (correctEndTimes > 0 || PostprocessorOptions > 19){
		bool hasend = false;
		int newstarttime = -1;
		int endt = Options.GetInt(ConvertTimePerLetter);
		bool isPreviousEndGreater = false;
		bool isEndGreater = false;
		for (auto cur = tmpmap.begin(); cur != tmpmap.end(); cur++){
			auto it = cur;
			dialc = cur->first;
			it++;
			if (!(it != tmpmap.end())){ it = cur; hasend = true; }
			else{
				isEndGreater = dialc->End > it->first->Start;
				if (correctEndTimes > 1){
					if (isEndGreater)
						goto postprocessor;

					int newend = (endt * dialc->Text.Len());
					if (newend < 1000){ newend = 1000; }
					newend += dialc->Start.mstime;
					dialc->End.NewTime(newend);
					dialc->ChangeDialogueState(1);
				}
				if (correctEndTimes > 0 && dialc->End > it->first->Start/* && dialc->Start < it->first->Start*/){
					dialc->End = it->first->Start;
					dialc->ChangeDialogueState(1);
				}
			}
			postprocessor:
			if (PostprocessorOptions & 4){
				int cdiff = (ThresholdEnd + ThresholdStart);
				int tdiff = it->first->Start.mstime - dialc->End.mstime;
				if (newstarttime != -1){
					dialc->Start.NewTime(newstarttime);
					newstarttime = -1;
					dialc->ChangeDialogueState(1);
				}
				if (tdiff <= cdiff && tdiff > 0){
					int coeff = ((float)tdiff / (float)cdiff) * ThresholdEnd;
					int newtime = ZEROIT(coeff);
					dialc->End.Change(newtime);
					newstarttime = dialc->End.mstime;
					dialc->ChangeDialogueState(1);

				}

			}
			if (PostprocessorOptions & 8){
				int startRange = dialc->Start.mstime - KeyframeBeforeStart;
				int startRange1 = dialc->Start.mstime + KeyframeAfterStart;
				int endRange = dialc->End.mstime - KeyframeBeforeEnd;
				int endRange1 = dialc->End.mstime + KeyframeAfterEnd;
				int previousEnd = 0;

				if (cur != tmpmap.begin()){
					it--;
					if (!hasend){ it--; }
					previousEnd = it->first->End.mstime;
				}

				int keyMS = 0;
				int startResult = INT_MAX;
				int endResult = -1;
				for (size_t g = 0; g < vb->VFF->KeyFrames.Count(); g++) {
					keyMS = vb->VFF->KeyFrames[g];
					if (keyMS > startRange && keyMS < startRange1) {
						keyMS = ZEROIT(vb->GetFrameTimeFromTime(keyMS));
						if (startResult > keyMS && keyMS != dialc->Start.mstime){ startResult = keyMS; }
					}
					if (keyMS > endRange && keyMS < endRange1) {
						keyMS = ZEROIT(vb->GetFrameTimeFromTime(keyMS));
						if (endResult < keyMS && keyMS > dialc->Start.mstime){ endResult = keyMS; }
					}
				}
				//here is main problem we do not know if next start will be changed
				//and it makes mess here but changing only start should be enough
				//startResult >= compareStart and endResult <= compareEnd should be changed or even remove
				if (startResult != INT_MAX){
					dialc->Start.NewTime(startResult);
					dialc->ChangeDialogueState(1);
				}
				if (endResult != -1){
					dialc->End.NewTime(endResult);
					dialc->ChangeDialogueState(1);
				}
				//make sure that start is greater or even then end time of previous line
				//and correct times but only when previous line end time was not greater than start time on start
				//then discard correction.
				if (dialc->Start.mstime < previousEnd && !isPreviousEndGreater && previousEnd < dialc->End.mstime){
					dialc->Start.NewTime(previousEnd);
					dialc->ChangeDialogueState(1);
				}
			}
			isPreviousEndGreater = isEndGreater;
			dialc->ClearParse();
		}

	}

	SpellErrors.clear();
	int tmpMarked = markedLine;
	SetModified(SHIFT_TIMES, true, false, -1, false);
	markedLine = tmpMarked;
	if (subsFormat > TMP){ RefreshColumns(START | END); }
	else{ Refresh(false); }
#if _DEBUG
	wxBell();
#endif
}


void SubsGridBase::SortIt(short what, bool all)
{
	SaveSelections();
	std::vector<Dialogue*> selected;
	if (all){
		for (int i = 0; i < GetCount(); i++){ file->GetDialogue(i)->ChangeDialogueState(1); }
	}
	else{
		for (auto cur = file->subs->Selections.begin(); cur != file->subs->Selections.end(); cur++){
			Dialogue *dial = file->subs->dials[*cur];
			dial->ChangeDialogueState(1);
			selected.push_back(dial);
		}
	}
	std::sort((all) ? file->subs->dials.begin() : selected.begin(), (all) ? file->subs->dials.end() : selected.end(),
		(what == 0) ? sortstart : (what == 1) ? sortend : (what == 2) ? sortstyle :
		(what == 3) ? sortactor : (what == 4) ? sorteffect : sortlayer);

	if (!all){
		int ii = 0;
		for (auto cur = file->subs->Selections.begin(); cur != file->subs->Selections.end(); cur++){
			file->subs->dials[*cur] = selected[ii++];
		}
		selected.clear();
	}
	file->edited = true;
	file->ReloadVisibleDialogues();
	SpellErrors.clear();
	SetModified(GRID_SORT_LINES);
	Refresh(false);
}


void SubsGridBase::DeleteRow(int rw, int len)
{
	int rwlen = rw + len;
	file->DeleteDialogues(rw, rwlen);
	if ((int)SpellErrors.size() > rwlen){ SpellErrors.erase(SpellErrors.begin() + rw, SpellErrors.begin() + rwlen); }
	else{ SpellErrors.clear(); }
}

void SubsGridBase::DeleteRows()
{
	Freeze();
	int sel = 0;
	for (auto i = file->subs->Selections.rbegin(); i != file->subs->Selections.rend(); i++)
	{
		sel = *i;
		file->subs->dials.erase(file->subs->dials.begin() + sel);
		file->IdConverter->deleteItemByKey(sel);
	}
	SpellErrors.clear();
	if (file->subs->Selections.size() > 0){ file->edited = true; }
	SaveSelections(true);
	if (GetCount() < 1){ AddLine(new Dialogue()); }
	SetModified(GRID_DELETE_LINES, true, false, file->GetElementByKey(sel));
	Thaw();
	RefreshColumns();
}

void SubsGridBase::MoveRows(int step, bool sav)
{
	SaveSelections();
	wxArrayInt sels;
	file->GetSelections(sels);

	if (sels.GetCount() < 1){ return; }
	if (step < 0){

		for (size_t i = 0; i < sels.GetCount(); i++)
		{
			int sel = sels[i];
			int istep = sel + step;
			if (istep < 0){ break; }
			file->InsertSelection(istep);
			file->EraseSelection(sel);
			if (step != -1){
				InsertRows(istep, 1, GetDialogue(sel), false);
				DeleteRow(sel, 1);
			}
			else{
				SwapRows(sel, istep);
			}

		}
	}
	else
	{
		for (int i = sels.GetCount() - 1; i >= 0; i--)
		{
			int sel = sels[i];
			int istep = sel + step;
			if (istep > GetCount() - 1) break;
			file->InsertSelection(istep);
			file->EraseSelection(sel);
			if (step != 1){
				Dialogue *dial = GetDialogue(sel);
				DeleteRow(sel, 1);
				InsertRows(istep, 1, dial, false);
			}
			else{
				SwapRows(sel, istep);
			}
		}
	}
	Edit->SetLine(FirstSelection());
	Refresh(false);
}

void SubsGridBase::DeleteText()
{
	wxArrayInt sels;
	file->GetSelections(sels);
	for (auto i : sels){
		CopyDialogue(i)->Text = "";
	}
	SetModified(GRID_DELETE_TEXT);
	Refresh(false);
}
void SubsGridBase::UpdateUR(bool toolbar)
{
	bool undo = false, _redo = false;
	file->GetURStatus(&undo, &_redo);
	Kai->Menubar->Enable(Undo, undo);
	Kai->Menubar->Enable(Redo, _redo);
	Kai->Menubar->Enable(UndoToLastSave, file->GetActualHistoryIter() != 0 && file->lastSave != -1);
	Kai->Menubar->Enable(SaveSubs, true);
	if (toolbar){
		Kai->Toolbar->UpdateId(Undo, undo);
		Kai->Toolbar->UpdateId(Redo, _redo);
		Kai->Toolbar->UpdateId(SaveSubs, true);
	}
}

bool SubsGridBase::IsModified()
{
	return file->CanSave();
}

void SubsGridBase::GetUndo(bool redo, int iter)
{
	TabPanel *tab = (TabPanel*)GetParent();
	Freeze();
	const wxString &resolution = GetSInfo("PlayResX") + " x " + GetSInfo("PlayResY");
	const wxString &matrix = GetSInfo("YCbCr Matrix");
	const wxString &tlmode = GetSInfo("TLMode");
	SaveSelections();
	savedSelections = false;
	bool failed = false;
	if (iter != -2){ failed = file->SetHistory(iter); }
	else if (redo){ failed = file->Redo(); }
	else{ failed = file->Undo(); }
	if (failed){ Thaw(); return; }

	UpdateUR();

	Kai->Label(file->GetActualHistoryIter());


	char oldformat = subsFormat;
	SetSubsFormat();
	if (oldformat != subsFormat){
		tab->ShiftTimes->Contents();
		tab->Edit->HideControls();
		Kai->UpdateToolbar();
		if (oldformat == ASS || subsFormat == ASS){
			tab->Video->vToolbar->DisableVisuals(subsFormat != ASS);
		}
	}

	Thaw();

	if (StyleStore::HasStore()){
		StyleStore *SS = StyleStore::Get();
		SS->ASSList->SetArray(&file->subs->styles);
		SS->ASSList->Refresh(false);
	}
	SpellErrors.clear();

	const wxString &newtlmode = GetSInfo("TLMode");
	if (newtlmode != tlmode){
		hasTLMode = (newtlmode == "Yes");
		showOriginal = (GetSInfo("TLMode Showtl") == "Yes" || (hasTLMode && Options.GetBool(TlModeShowOriginal) != 0));
		Edit->SetTl(hasTLMode);
	}
	//Odtąd nie będzie trzeba tego zabezpieczać, FindIdFromKey nie zwróci -1;
	int corrected = -1;
	Edit->SetLine(file->FindIdFromKey(file->subs->activeLine, &corrected));
	markedLine = file->FindIdFromKey(file->subs->markerLine);
	scPos = file->FindIdFromKey(file->subs->scrollPosition);
	if (corrected >= 0){
		file->EraseSelectionKey(file->subs->activeLine);
		file->InsertSelectionKey(corrected);
	}

	RefreshColumns();
	Edit->RefreshStyle();
	VideoCtrl *vb = tab->Video;

	const wxString &newResolution = GetSInfo("PlayResX") + " x " + GetSInfo("PlayResY");
	if (resolution != newResolution){ 
		Kai->SetSubsResolution(); 
		vb->ChangeOnScreenResolution(tab);
	}
	const wxString &newmatrix = GetSInfo("YCbCr Matrix");
	if (matrix != newmatrix){
		vb->SetColorSpace(newmatrix);
	}

	if (Edit->Visual < CHANGEPOS){

		if (vb->IsShown() || vb->isFullscreen){ vb->OpenSubs(GetVisible());}
		int opt = vb->vToolbar->videoSeekAfter->GetSelection();
		if (opt > 1){
			if (vb->GetState() == Paused || (vb->GetState() == Playing && (opt == 3 || opt == 5))){
				vb->Seek(Edit->line->Start.mstime);
			}
		}
		else{
			if (vb->GetState() == Paused){ vb->Render(); }
		}
	}
	else if (Edit->Visual == CHANGEPOS){
		vb->SetVisual(false, true);
	}
	else {
		vb->SetVisual(false, true, true);
	}
	


	if (makebackup){
		timer.Start(20000, true);
		//CreateTimerQueueTimer(&qtimer,0,WAITORTIMERCALLBACK(OnBcktimer),this,20000,0,WT_EXECUTEONLYONCE);
		makebackup = false;
	}
}

void SubsGridBase::DummyUndo(int newIter)
{
	if (newIter >= file->Iter())
		return;
	file->DummyUndo(newIter);
	SpellErrors[currentLine].clear();
	Edit->SetLine(currentLine, false, false);
	RefreshColumns();
	UpdateUR();
	Kai->Label(file->GetActualHistoryIter());
	VideoCtrl *vb = ((TabPanel*)GetParent())->Video;
	if (vb->GetState() != None){
		vb->OpenSubs(GetVisible());
		vb->Render();
	}
}

int SubsGridBase::FirstSelection()
{
	if (!file->subs->Selections.empty()){
		// return only visible element when nothing is visible, return -1;
		for (auto it = file->subs->Selections.begin(); it != file->subs->Selections.end(); it++){
			int sel = file->IdConverter->getElementByKey(*it);
			if (sel >= 0)
				return sel;
		}
	}
	return -1;
}


//Uważaj na dodawanie do niszczarki, 
//bo brak dodania gdy trzeba to wycieki pamięci,
//a podwójne dodanie to krasz przy niszczeniu obiektu.
void SubsGridBase::InsertRows(int Row,
	const std::vector<Dialogue *> &RowsTable,
	bool AddToDestroy, bool asKey)
{
	int convertedRow = (asKey) ? Row : file->IdConverter->getElementById(Row);
	if (convertedRow < 0){ convertedRow = file->subs->dials.size(); }
	file->subs->dials.insert(file->subs->dials.begin() + convertedRow, RowsTable.begin(), RowsTable.end());
	//file->ReloadVisibleDialogues(convertedRow, convertedRow + RowsTable.size());
	for (int i = 0; i < RowsTable.size(); i++){
		file->IdConverter->insert(i + convertedRow);
		if (!RowsTable[i]->isVisible){
			file->IdConverter->deleteItemByKey(i + convertedRow, false);
		}
	}
	if (asKey){ Row = file->IdConverter->getElementByKey(Row); }
	if (Row >= 0){
		wxArrayInt emptyarray;
		SpellErrors.insert(SpellErrors.begin() + Row, RowsTable.size(), emptyarray);
	}
	if (AddToDestroy){ file->subs->ddials.insert(file->subs->ddials.end(), RowsTable.begin(), RowsTable.end()); }
}

//Uważaj na dodawanie do niszczarki, 
//bo brak dodania gdy trzeba to wycieki pamięci,
//a podwójne dodanie to krasz przy niszczeniu obiektu.
void SubsGridBase::InsertRows(int Row, int NumRows, Dialogue *Dialog, bool AddToDestroy, bool Save, bool asKey)
{
	//SaveSelections();
	int convertedRow = (asKey) ? Row : file->IdConverter->getElementById(Row);
	if (convertedRow < 0){ convertedRow = file->subs->dials.size(); }
	file->subs->dials.insert(file->subs->dials.begin() + convertedRow, NumRows, Dialog);
	for (int i = convertedRow; i < convertedRow + NumRows; i++){
		file->IdConverter->insert(i);
		if (!file->subs->dials[i]->isVisible){
			file->IdConverter->deleteItemByKey(i, false);
		}
	}
	//file->ReloadVisibleDialogues(convertedRow, convertedRow + NumRows);
	if (asKey){ Row = file->IdConverter->getElementByKey(Row); }
	if (Row >= 0){
		wxArrayInt emptyarray;
		SpellErrors.insert(SpellErrors.begin() + Row, NumRows, emptyarray);
	}
	if (AddToDestroy){ file->subs->ddials.push_back(Dialog); }
	if (Save){
		file->InsertSelectionKey(convertedRow);
		SetModified(GRID_INSERT_ROW);
		Refresh(false);
	}
}

void SubsGridBase::SetSubsFormat(wxString ext)
{
	subsFormat = ASS;
	int rw = 0;
	char subsext = (ext == "ass" || ext == "ssa") ? ASS : (ext == "srt") ? SRT : TMP;
	while (rw<GetCount())
	{
		Dialogue *dial = GetDialogue(rw);
		if (dial->NonDialogue || dial->Format == 0){ rw++; }
		else if (!ext.empty() && (subsext != dial->Format || (subsext == TMP && dial->Format>SRT))){ rw++; }//form=dial->Form; 
		else{ subsFormat = dial->Format; break; }
	}
}


void SubsGridBase::AddSInfo(const wxString &SI, wxString val, bool save)
{
	wxString key;
	if (val == ""){
		key = SI.BeforeFirst(':');
		key.Trim(false);
		key.Trim(true);
		val = SI.AfterFirst(':');
		val.Trim(false);
		val.Trim(true);
	}
	else{ key = SI; }
	SInfo *oldinfo = NULL;
	int ii = -1;
	oldinfo = GetSInfoP(key, &ii);

	if (!oldinfo || save){
		oldinfo = new SInfo(key, val);
		if (ii < 0){
			file->subs->sinfo.push_back(oldinfo);
		}
		else{
			file->subs->sinfo[ii] = oldinfo;
		}
		file->subs->dsinfo.push_back(oldinfo);
	}
	else{
		oldinfo->Val = val;
	}
	//if(save){

	//}
}

void SubsGridBase::GetSInfos(wxString &textSinfo, bool tld/*=false*/)
{
	for (std::vector<SInfo*>::iterator cur = file->subs->sinfo.begin(); cur != file->subs->sinfo.end(); cur++) {
		if (!(tld && (*cur)->Name.StartsWith("TLMode"))){
			textSinfo << (*cur)->Name << ": " << (*cur)->Val << "\r\n";
		}
	}
}
//wszystkie set modified trzeba znaleźć i dodać editiontype.
void SubsGridBase::SetModified(unsigned char editionType, bool redit, bool dummy, int SetEditBoxLine, bool Scroll)
{
	if (file->IsNotSaved()){
		if (!IsModified()){
			Kai->Toolbar->UpdateId(SaveSubs, true);
			Kai->Menubar->Enable(SaveSubs, true);
		}
		if (Comparison){
			SubsComparison();
		}
		if (!savedSelections){
			SaveSelections();
		}
		savedSelections = false;
		int ebrow = currentLine;
		if (redit){
			int erow = (SetEditBoxLine >= 0) ? SetEditBoxLine : ebrow;
			if (erow >= GetCount()){ erow = GetCount() - 1; }
			lastRow = erow;
			int w, h;
			GetClientSize(&w, &h);
			if ((scPos > erow || scPos + (h / (GridHeight + 1)) < erow + 2) && Scroll){
				scPos = MAX(0, erow - ((h / (GridHeight + 1)) / 2));
			}
			Edit->SetLine(erow);
			file->InsertSelection(erow);
		}
		file->SaveUndo(editionType, ebrow, markedLine);
		Kai->Label(file->GetActualHistoryIter());
		if (!dummy){
			VideoCtrl *vb = ((TabPanel*)GetParent())->Video;
			if (Edit->Visual >= CHANGEPOS){
				vb->SetVisual(false, true);
			}
			else{
				if (vb->IsShown() || vb->isFullscreen){ vb->OpenSubs(GetVisible());}

				int opt = vb->vToolbar->videoSeekAfter->GetSelection();
				if (opt > 1){
					if (vb->GetState() == Paused || (vb->GetState() == Playing && (opt == 3 || opt == 5))){
						vb->Seek(Edit->line->Start.mstime);
					}
				}
				else{
					if (vb->GetState() == Paused){ vb->Render(); }
				}
			}
		}

		if (makebackup){
			timer.Start(20000, true);
			//CreateTimerQueueTimer(&qtimer,0,WAITORTIMERCALLBACK(OnBcktimer),this,20000,0,WT_EXECUTEONLYONCE);
			makebackup = false;
		}
		UpdateUR();
	}
}

void SubsGridBase::SwapRows(int frst, int scnd, bool sav)
{

	Dialogue *tmp = file->GetDialogue(frst);
	(*file)[frst] = file->GetDialogue(scnd);
	(*file)[scnd] = tmp;
	wxArrayInt tmpspell = SpellErrors[frst];
	SpellErrors[frst] = SpellErrors[scnd];
	SpellErrors[scnd] = tmpspell;
	Refresh(false);
	if (sav){ SetModified(GRID_SWAP_LINES); }
}

void SubsGridBase::LoadSubtitles(const wxString &str, wxString &ext)
{
	bool oldHasTlMode = hasTLMode;
	int active = 0;

	SubsLoader SL((SubsGrid*)this, str, ext);

	if (oldHasTlMode != hasTLMode){
		Edit->SetTl(hasTLMode);
		Kai->Menubar->Enable(SaveTranslation, hasTLMode);
	}
	if (hasTLMode && (GetSInfo("TLMode Showtl") == "Yes" || Options.GetBool(TlModeShowOriginal))){ showOriginal = true; }


	if (subsFormat == MDVD || subsFormat == MPL2){
		int endt = Options.GetInt(ConvertTimePerLetter);
		for (int i = 0; i < GetCount(); i++){
			Dialogue *dial = GetDialogue(i);

			if (dial->End.mstime == 0){
				int newend = (endt*dial->Text.Len());
				if (newend < 1000){ newend = 1000; }
				newend += dial->Start.mstime;
				dial->End.NewTime(newend);
				if (i<GetCount() - 1){
					if (dial->End>file->GetDialogue(i + 1)->Start){
						dial->End = file->GetDialogue(i + 1)->Start;
					}
				}
			}
		}
	}
	else if (subsFormat == ASS){
		if (ext != "ass"){ originalFormat = 0; if (StylesSize() < 1){ AddStyle(new Styles()); } }
		Edit->TlMode->Enable(true); Edit->RefreshStyle();
		if (Options.GetBool(GridLoadSortedSubs)){
			std::sort(file->subs->dials.begin(), file->subs->dials.end(), sortstart);
		}
		active = wxAtoi(GetSInfo("Active Line"));
		if (active >= GetCount()){ active = 0; }
	}
	else{ Edit->TlMode->Enable(false); }



	file->InsertSelection(active);
	lastRow = active;
	markedLine = active;

	scPos = MAX(0, active - 3);
	file->EndLoad(OPEN_SUBTITLES, active);

	RefreshColumns();
	Edit->SetLine(active, false, false);

	Edit->HideControls();

	if (StyleStore::HasStore() && subsFormat == ASS){ StyleStore::Get()->LoadAssStyles(); }
	if (subsFormat == ASS){
		if (Options.GetBool(GridFilterAfterLoad)){
			isFiltered = true;
			SubsGridFiltering filter((SubsGrid*)this, currentLine);
			filter.Filter(true);
		}
		Edit->RebuildActorEffectLists();
	}

}

void SubsGridBase::SetStartTime(int stime)
{
	Edit->Send(EDITBOX_LINE_EDITION, false, false, true);
	wxArrayInt sels;
	file->GetSelections(sels);
	for (size_t i = 0; i < sels.size(); i++){
		Dialogue *dialc = CopyDialogue(sels[i]);
		if (!dialc){ continue; }
		dialc->Start.NewTime(stime);
		if (dialc->End < stime){ dialc->End.NewTime(stime); }
	}
	if (sels.size()){
		SetModified(GRID_SET_START_TIME);
		Refresh(false);
	}
}

void SubsGridBase::SetEndTime(int etime)
{
	Edit->Send(EDITBOX_LINE_EDITION, false, false, true);
	wxArrayInt sels;
	file->GetSelections(sels);
	for (size_t i = 0; i < sels.size(); i++){
		Dialogue *dialc = CopyDialogue(sels[i]);
		if (!dialc){ continue; }
		dialc->End.NewTime(etime);
		if (dialc->Start > etime){ dialc->Start.NewTime(etime); }
	}
	if (sels.size()){
		SetModified(GRID_SET_END_TIME);
		Refresh(false);
	}
}

bool SubsGridBase::SetTlMode(bool mode)
{
	if (mode){
		if (GetSInfo("TLMode") == ""){
			//for(int i=0;i<GetCount();i++){file->GetDialogue(i)->spells.clear();}

			int ssize = file->subs->styles.size();
			if (ssize > 0){
				Styles *tlstyl = GetStyle(0, "Default")->Copy();
				for (int i = 0; i < ssize; i++){
					wxString ns = "TLmode";
					wxString nss = (i == 0) ? ns : ns << i;
					if (FindStyle(nss) == -1){ tlstyl->Name = nss; AddSInfo("TLMode Style", nss); break; }
				}
				tlstyl->Alignment = "8";
				AddStyle(tlstyl);
			}

		}
		AddSInfo("TLMode", "Yes");
		hasTLMode = true;
		if (Options.GetBool(TlModeShowOriginal)){ showOriginal = true; }
		Kai->Menubar->Enable(SaveTranslation, true);

		//Refresh(false);

	}
	else{
		if (KaiMessageBox(_("Czy na pewno chcesz wyłączyć tryb tłumaczenia?\nObcojęzyczny tekst przetłumaczonych linijek zostanie usunięty."), _("Potwierdzenie"), wxYES_NO) == wxNO)
		{
			return true;
		}

		int iinf = -1;
		GetSInfo("TLMode", &iinf);
		if (iinf >= 0){
			file->subs->sinfo.erase(file->subs->sinfo.begin() + iinf);
		}
		iinf = -1;
		const wxString &vall = GetSInfo("TLMode Style", &iinf);
		if (iinf >= 0){
			int g = FindStyle(vall);
			if (g >= 0){ DelStyle(g); }
			file->subs->sinfo.erase(file->subs->sinfo.begin() + iinf);
		}

		for (int i = 0; i < file->GetAllCount(); i++)
		{
			Dialogue *dial = file->GetDialogueByKey(i);
			Dialogue *dialc = NULL;
			if (dial->TextTl != "")
			{
				dialc = file->CopyDialogueByKey(i);
				dialc->Text = dialc->TextTl;
				dialc->TextTl = "";
			}
			if (dial->IsDoubtful()){
				if (!dialc){ dialc = CopyDialogueByKey(i); }
				dialc->ChangeState(4);
			}
		}

		hasTLMode = false;
		showOriginal = false;
		Kai->Menubar->Enable(SaveTranslation, false);
	}
	SpellErrors.clear();
	Refresh(false);
	SetModified((mode) ? GRID_TURN_ON_TLMODE : GRID_TURN_OFF_TLMODE);
	return false;
}

void SubsGridBase::NextLine(int dir)
{
	if (Edit->ABox && Edit->ABox->audioDisplay->hold != 0){ return; }
	int size = GetCount();
	if (size < 1){ dir = 0; }
	int nebrow = currentLine + dir;
	if (nebrow < 0){ return; }
	if (nebrow >= size){
		Dialogue *tmp = GetDialogue(size - 1)->Copy();
		int eend = tmp->End.mstime;
		tmp->Start.NewTime(eend);
		tmp->End.NewTime(eend + 5000);
		tmp->Text = "";
		tmp->TextTl = "";
		AddLine(tmp);
		SetModified(GRID_APPEND_LINE, false);
		AdjustWidths(subsFormat>TMP? (START | END) : 0);
	}
	int h, w;
	GetClientSize(&w, &h);
	scPos = MID(0, nebrow - ((h / (GridHeight + 1)) / 2), GetCount() - 1);
	file->ClearSelections();
	file->InsertSelection(nebrow);
	lastRow = nebrow;
	//AdjustWidths(0);
	Refresh(false);
	Edit->SetLine(nebrow, true, true, false, true);
	SubsGrid *grid = (SubsGrid*)this;
	if (Comparison){ grid->ShowSecondComparedLine(nebrow); }
	else if (grid->preview){ grid->preview->NewSeeking(); }
}



void SubsGridBase::LoadDefault(bool line, bool sav, bool endload)
{
	if (line)
	{
		AddLine(new Dialogue());
		AddStyle(new Styles());
		file->InsertSelectionKey(0);
		originalFormat = subsFormat = ASS;
	}
	AddSInfo("Title", "Kainote Ass File", sav);
	AddSInfo("PlayResX", "1280", sav);
	AddSInfo("PlayResY", "720", sav);
	AddSInfo("ScaledBorderAndShadow", "yes", sav);
	AddSInfo("WrapStyle", "0", sav);
	AddSInfo("ScriptType", "v4.00+", sav);
	AddSInfo("Last Style Storage", "Default", sav);
	AddSInfo("YCbCr Matrix", "TV.601", sav);
	if (endload){
		file->EndLoad(NEW_SUBTITLES, 0);
	}
}

Dialogue *SubsGridBase::CopyDialogue(int i, bool push)
{
	if (push && (int)SpellErrors.size() > i){ SpellErrors[i].Clear(); }
	return file->CopyDialogue(i, push);
}
Dialogue *SubsGridBase::CopyDialogueByKey(int i, bool push)
{
	if (push && (int)SpellErrors.size() > i){ SpellErrors[i].Clear(); }
	return file->CopyDialogueByKey(i, push);
}

Dialogue *SubsGridBase::GetDialogue(int i)
{
	if (i >= (int)file->subs->dials.size()){
		return NULL;
	}
	return file->GetDialogue(i);
}

const wxString & SubsGridBase::GetSInfo(const wxString &key, int *ii/*=0*/)
{
	int i = 0;
	for (std::vector<SInfo*>::iterator it = file->subs->sinfo.begin(); it != file->subs->sinfo.end(); it++)
	{
		if (key == (*it)->Name) { if (ii){ *ii = i; } return (*it)->Val; }
		i++;
	}
	return emptyString;
}

SInfo *SubsGridBase::GetSInfoP(const wxString &key, int *ii)
{
	int i = 0;
	for (std::vector<SInfo*>::iterator it = file->subs->sinfo.begin(); it != file->subs->sinfo.end(); it++)
	{
		if (key == (*it)->Name) { if (ii){ *ii = i; }; return (*it); }
		i++;
	}
	*ii = -1;
	return NULL;
}

int SubsGridBase::SInfoSize()
{
	return file->subs->sinfo.size();
}

wxString *SubsGridBase::SaveText()
{
	wxString *path = new wxString();

	TabPanel *tab = (TabPanel*)GetParent();
	wxString ext = (subsFormat < SRT) ? "ass" : (subsFormat == SRT) ? "srt" : "txt";

	(*path) << Options.pathfull << "\\Subs\\DummySubs_" << Notebook::GetTabs()->FindPanel(tab) << "." << ext;

	SaveFile(*path, false, true);

	return path;
}


wxString *SubsGridBase::GetVisible(bool *visible, wxPoint *point, wxArrayInt *selected)
{
	bool showOriginalOnVideo = !Options.GetBool(TL_MODE_HIDE_ORIGINAL_ON_VIDEO);
	TabPanel *pan = (TabPanel*)GetParent();
	int _time = pan->Video->Tell();
	bool toEnd = pan->Video->GetState() == Playing;
	wxString *txt = new wxString();
	if (subsFormat == ASS){
		(*txt) << "[Script Info]\r\n";
		GetSInfos(*txt, false);
		(*txt) << "\r\n[V4+ Styles]\r\nFormat: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding \r\n";
		GetStyles(*txt, false);
		(*txt) << " \r\n[Events]\r\nFormat: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\r\n";
	}
	Edit->Send(EDITBOX_LINE_EDITION, false, true);
	if ((_time >= Edit->line->Start.mstime || toEnd) && _time < Edit->line->End.mstime){
		if (visible){ *visible = true; }
	}
	else if (visible){
		*visible = false;
	}
	bool noLine = true;
	bool isTlmode = GetSInfo("TLMode") == "Yes";
	const wxString &tlStyle = GetSInfo("TLMode Style");
	int j = 1;
	int activeLineKey = file->GetElementById(currentLine);
	
	for (int i = 0; i < file->subs->dials.size(); i++)
	{
		Dialogue *dial = file->subs->dials[i];
		if (!ignoreFiltered && !dial->isVisible || dial->NonDialogue){ continue; }
		if (i == activeLineKey){
			dial = Edit->line;
		}
		if (selected && file->IsSelectedByKey(i)){
			selected->Add(txt->Len());
			continue;
		}
		if ((toEnd && _time <= dial->Start.mstime) || (_time >= dial->Start.mstime && _time < dial->End.mstime)){
			if (isTlmode && dial->TextTl != ""){
				if (showOriginalOnVideo)
					dial->GetRaw(txt, false, tlStyle);

				dial->GetRaw(txt, true);
			}
			else{
				if (subsFormat == SRT){
					(*txt) << j << "\r\n";
					j++;
				}
				dial->GetRaw(txt);
			}
			if (point && i == activeLineKey){
				int all = txt->Len(); point->x = all - 2;
				int len = (isTlmode && dial->TextTl != "") ?
					dial->TextTl.Len() : dial->Text.Len();
				point->y = len;
				point->x -= len;
			}
			noLine = false;
		}

	}
	if (noLine){
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
	TabPanel *pan = (TabPanel*)GetParent();
	Kai->SetStatusText(_("Autozapis"), 0);
	wxString path;
	wxString ext = (subsFormat < SRT) ? "ass" : (subsFormat == SRT) ? "srt" : "txt";

	path << Options.pathfull << "\\Subs\\" << pan->SubsName.BeforeLast('.')
		<< "_" << Notebook::GetTabs()->FindPanel(pan) << "_" << numsave << "." << ext;

	SaveFile(path, false);
	int maxFiles = Options.GetInt(AutoSaveMaxFiles);
	if (maxFiles > 1 && numsave >= maxFiles){ numsave = 0; }
	numsave++;
	makebackup = true;
	nullifyTimer.Start(5000, true);
}

void SubsGridBase::GetASSRes(int *x, int *y)
{
	const wxString &oldx = GetSInfo("PlayResX");
	const wxString &oldy = GetSInfo("PlayResY");
	int nx = wxAtoi(oldx);
	int ny = wxAtoi(oldy);
	bool changed = false;
	if (nx < 1 && ny < 1){
		nx = 384; ny = 288;
		AddSInfo("PlayResX", "384");
		AddSInfo("PlayResY", "288");
		changed = true;
	}
	else if (nx < 1){
		nx = (float)ny*(4.0 / 3.0); if (ny == 1024){ nx = 1280; }
		AddSInfo("PlayResX", std::to_string(nx));
		changed = true;
	}
	else if (ny < 1){
		ny = (float)nx*(3.0 / 4.0); if (nx == 1280){ ny = 1024; }
		AddSInfo("PlayResY", std::to_string(ny));
		changed = true;
	}
	*x = nx;
	*y = ny;
	//if(changed){SetModified(ASS_PROPERTIES, false, true, -1, false);}
}


void SubsGridBase::SaveSelections(bool clear)
{
	file->undo[file->iter]->Selections = file->subs->Selections;
	//tutaj muszą być przeróbki na klucze
	file->undo[file->iter]->activeLine = file->GetElementById(currentLine);
	file->undo[file->iter]->markerLine = file->GetElementById(markedLine);
	file->undo[file->iter]->scrollPosition = file->GetElementById(scPos);
	if (clear){ file->ClearSelections(); }
	savedSelections = true;
}

void SubsGridBase::GetCommonStyles(SubsGridBase *_grid, wxArrayString &styleTable)
{
	std::vector<Styles *> &styles1 = file->subs->styles;
	std::vector<Styles *> &styles2 = _grid->file->subs->styles;
	for (auto style1 : styles1){
		for (auto style2 : styles2){
			if (style1->Name == style2->Name){
				styleTable.Add(style1->Name); break;
			}
		}
	}
}

void SubsGridBase::SubsComparison()
{
	int comparisonType = Options.GetInt(SubsComparisonType);
	if (!comparisonType && compareStyles.size() < 1){ return; }
	bool compareByVisible = (comparisonType & COMPARE_BY_VISIBLE) != 0;
	bool compareByTimes = (comparisonType & COMPARE_BY_TIMES) != 0;
	bool compareByStyles = (comparisonType & COMPARE_BY_STYLES) != 0;
	bool compareByChosenStyles = compareStyles.size() > 0;
	bool compareBySelections = (comparisonType & COMPARE_BY_SELECTIONS) != 0;
	int firstSize = CG1->file->GetAllCount(), secondSize = CG2->file->GetAllCount();
	if (CG1->Comparison){ CG1->Comparison->clear(); }
	else{ CG1->Comparison = new std::vector<compareData>; }
	if (CG2->Comparison){ CG2->Comparison->clear(); }
	else{ CG2->Comparison = new std::vector<compareData>; }
	CG1->Comparison->resize(firstSize, compareData());
	CG2->Comparison->resize(secondSize, compareData());

	int lastJ = 0;

	for (int i = 0; i<firstSize; i++){

		int j = lastJ;
		Dialogue *dial1 = CG1->file->GetDialogueByKey(i);
		if (compareByVisible && !dial1->isVisible){ continue; }
		while (j<secondSize){

			Dialogue *dial2 = CG2->file->GetDialogueByKey(j);
			if (compareByVisible && !dial2->isVisible){ j++; continue; }

			if (compareByTimes && (dial1->Start != dial2->Start || dial1->End != dial2->End)){ j++; continue; }

			if (compareByStyles && dial1->Style != dial2->Style){ j++; continue; }

			if (compareByChosenStyles && (compareStyles.Index(dial1->Style) == -1 || dial1->Style != dial2->Style)){ j++; continue; }

			if (compareBySelections && (!CG1->file->IsSelectedByKey(i) || !CG2->file->IsSelectedByKey(j))){ j++; continue; }

			compareData & firstCompare = CG1->Comparison->at(i);
			compareData & secondCompare = CG2->Comparison->at(j);
			CompareTexts(firstCompare, secondCompare, (CG1->hasTLMode && dial1->TextTl != "") ? dial1->TextTl : dial1->Text,
				(CG2->hasTLMode && dial2->TextTl != "") ? dial2->TextTl : dial2->Text);
			firstCompare.secondComparedLine = j;
			secondCompare.secondComparedLine = i;
			lastJ = j + 1;
			break;
			j++;
		}

	}

	CG1->Refresh(false);
	CG2->Refresh(false);
}


void SubsGridBase::CompareTexts(compareData &firstCompare, compareData &secondCompare, const wxString &first, const wxString &second)
{
	if (first == second){
		firstCompare.differences = false;
		secondCompare.differences = false;
		return;
	}
	firstCompare.push_back(1);
	secondCompare.push_back(1);


	size_t l1 = first.Len(), l2 = second.Len();
	size_t sz = (l1 + 1) * (l2 + 1) * sizeof(size_t);
	size_t w = l2 + 1;
	size_t* dpt;
	size_t i1, i2;
	dpt = new size_t[sz];

	if (//sz / (l1 + 1) / (l2 + 1) != sizeof(size_t) ||
		//(
		dpt == NULL)
	{
		KaiLog("memory allocation failed");
		return;
	}

	/*for (i1 = 0; i1 <= l1; i1++)
	dpt[w * i1 + 0] = 0;
	for (i2 = 0; i2 <= l2; i2++)
	dpt[w * 0 + i2] = 0;*/
	memset(dpt, 0, sz);

	for (i1 = 1; i1 <= l1; i1++){
		for (i2 = 1; i2 <= l2; i2++)
		{
			if (first[l1 - i1] == second[l2 - i2])
			{
				dpt[w * i1 + i2] = dpt[w * (i1 - 1) + (i2 - 1)] + 1;
			}
			else if (dpt[w * (i1 - 1) + i2] > dpt[w * i1 + (i2 - 1)])
			{
				dpt[w * i1 + i2] = dpt[w * (i1 - 1) + i2];
			}
			else
			{
				dpt[w * i1 + i2] = dpt[w * i1 + (i2 - 1)];
			}
		}
	}

	int sfirst = -1, ssecond = -1;
	i1 = l1; i2 = l2;
	for (;;){
		if ((i1 > 0) && (i2 > 0) && (first[l1 - i1] == second[l2 - i2])){
			if (sfirst >= 0){
				firstCompare.push_back(sfirst);
				firstCompare.push_back((l1 - i1) - 1);
				sfirst = -1;
			}
			if (ssecond >= 0){
				secondCompare.push_back(ssecond);
				secondCompare.push_back((l2 - i2) - 1);
				ssecond = -1;
			}
			i1--; i2--; continue;
		}
		else{
			if (i1 > 0 && (i2 == 0 || dpt[w * (i1 - 1) + i2] >= dpt[w * i1 + (i2 - 1)])){
				if (sfirst == -1){ sfirst = l1 - i1; }
				i1--; continue;
			}
			else if (i2 > 0 && (i1 == 0 || dpt[w * (i1 - 1) + i2] < dpt[w * i1 + (i2 - 1)])){
				if (ssecond == -1){ ssecond = l2 - i2; }
				i2--; continue;
			}
		}

		break;
	}
	if (sfirst >= 0){
		firstCompare.push_back(sfirst);
		firstCompare.push_back((l1 - i1) - 1);
	}
	if (ssecond >= 0){
		secondCompare.push_back(ssecond);
		secondCompare.push_back((l2 - i2) - 1);
	}

	delete dpt;
}

void SubsGridBase::RemoveComparison()
{
	if (hasCompare){
		if (CG1){
			delete CG1->Comparison;
			CG1->Comparison = NULL;
			CG1->Refresh(false);
		}
		if (CG2){
			delete CG2->Comparison;
			CG2->Comparison = NULL;
			CG2->Refresh(false);
		}
		CG1 = NULL;
		CG2 = NULL;
		hasCompare = false;
	}
}

SubsGrid* SubsGridBase::CG1 = NULL;
SubsGrid* SubsGridBase::CG2 = NULL;
bool SubsGridBase::hasCompare = false;
wxArrayString SubsGridBase::compareStyles = wxArrayString();