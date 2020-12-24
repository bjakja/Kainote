//  Copyright (c) 2012 - 2020, Marcin Drob

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
#include <wx/regex.h>
#include <wx/ffile.h>
#include "KaiMessageBox.h"
#include "SubsGridPreview.h"
#include <algorithm>


bool sortstart(Dialogue *i, Dialogue *j){
	/*if (i->treeState != j->treeState)
		return i->treeState < j->treeState;*/
	if (i->Start.mstime != j->Start.mstime){
		return (i->Start.mstime < j->Start.mstime);
	}
	return i->End.mstime < j->End.mstime;
}
bool sortend(Dialogue *i, Dialogue *j){
	/*if (i->treeState != j->treeState)
		return i->treeState < j->treeState;*/
	if (i->End.mstime != j->End.mstime){
		return (i->End.mstime < j->End.mstime);
	}
	return i->Start.mstime < j->Start.mstime;
}
bool sortstyle(Dialogue *i, Dialogue *j){
	/*if (i->treeState != j->treeState)
		return i->treeState < j->treeState;*/
	if (i->Style != j->Style){
		return (i->Style.CmpNoCase(j->Style) < 0);
	}
	return i->Start.mstime < j->Start.mstime;
}
bool sortactor(Dialogue *i, Dialogue *j){
	/*if (i->treeState != j->treeState)
		return i->treeState < j->treeState;*/
	if (i->Actor != j->Actor){
		return (i->Actor.CmpNoCase(j->Actor) < 0);
	}
	return i->Start.mstime < j->Start.mstime;
}
bool sorteffect(Dialogue *i, Dialogue *j){
	/*if (i->treeState != j->treeState)
		return i->treeState < j->treeState;*/
	if (i->Effect != j->Effect){
		return (i->Effect.CmpNoCase(j->Effect) < 0);
	}
	return i->Start.mstime < j->Start.mstime;
}
bool sortlayer(Dialogue *i, Dialogue *j){
	/*if (i->treeState != j->treeState)
		return i->treeState < j->treeState;*/
	if (i->Layer != j->Layer){
		return (i->Layer < j->Layer);
	}
	return i->Start.mstime < j->Start.mstime;
}


SubsGridBase::SubsGridBase(wxWindow *parent, const long int id, const wxPoint& pos, const wxSize& size, long style)
	: KaiScrolledWindow(parent, id, pos, size, style | wxVERTICAL)
{
	file = new SubsFile(&editionMutex);
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
		Kai->SetStatusText(L"", 0);
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
	scrollPosition = 0;
	scrollPositionId = 0;
	lastRow = 0;
	scHor = 0;
}
void SubsGridBase::AddLine(Dialogue *line)
{
	file->AppendDialogue(line);
}

void SubsGridBase::ChangeLine(unsigned char editionType, Dialogue *line1, size_t wline, long cells, bool selline/*=false*/, bool dummy/*=false*/)
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
	if (cells != TXT && cells != TXTTL && cells != TXT + TXTTL)
		AdjustWidths(cells);

	if (selline)
		NextLine();
	else{
		Refresh(false);
	}
	SetModified(editionType, false, dummy);
}

void SubsGridBase::ChangeCell(long cells, size_t wline, Dialogue *what)
{
	Dialogue *dial = CopyDialogue(wline);
	if (cells & LAYER){
		dial->Layer = what->Layer;
	}
	if (cells & START){
		dial->Start = what->Start;
	}
	if (cells & END){
		dial->End = what->End;
	}
	if (cells & STYLE){
		dial->Style = what->Style;
	}
	if (cells & ACTOR){
		dial->Actor = what->Actor;
	}
	if (cells & MARGINL){
		dial->MarginL = what->MarginL;
	}
	if (cells & MARGINR){
		dial->MarginR = what->MarginR;
	}
	if (cells & MARGINV){
		dial->MarginV = what->MarginV;
	}
	if (cells & EFFECT){
		dial->Effect = what->Effect;
	}
	if (cells & TXT){
		dial->Text = what->Text;
	}
	if (cells & COMMENT){
		dial->IsComment = what->IsComment;
	}
	if (cells & TXTTL){
		dial->TextTl = what->TextTl;
	}
}


void SubsGridBase::Convert(char type)
{
	if (Options.GetBool(CONVERT_SHOW_SETTINGS)){
		OptionsDialog od(Kai, Kai);
		od.OptionsTree->ChangeSelection(2);
		od.okok->SetFocus();
		if (od.ShowModal() == wxID_CANCEL){ return; }
	}
	if (Options.GetBool(CONVERT_FPS_FROM_VIDEO) && tab->VideoPath != L""){
		Options.SetString(CONVERT_FPS, Kai->GetStatusText(4).BeforeFirst(L' '));
	}
	if (Options.GetFloat(CONVERT_FPS) < 1){ KaiMessageBox(_("Nieprawidłowy FPS. Popraw opcje i spróbuj ponownie.")); return; }

	bool newendtimes = Options.GetBool(CONVERT_NEW_END_TIMES);
	const wxString & stname = Options.GetString(CONVERT_STYLE);
	int endt = Options.GetInt(CONVERT_TIME_PER_CHARACTER);
	const wxString & prefix = Options.GetString(CONVERT_ASS_TAGS_TO_INSERT_IN_LINE);

	size_t i = 0;
	Dialogue *lastDialc = NULL;
	while (i < file->GetCount())
	{
		if ((type > ASS) && (subsFormat < SRT) && file->GetDialogue(i)->IsComment){
			while (i < file->GetCount() && file->GetDialogue(i)->IsComment){
				file->DeleteDialogues(i, i + 1);
			}
			if (i >= file->GetCount())
				break;
		}
		Dialogue *dialc = file->CopyDialogue(i);
		dialc->Convert(type, prefix);
		if ((newendtimes && type != TMP) || subsFormat == TMP)
		{
			if (lastDialc){
				if (lastDialc->End.mstime > dialc->Start.mstime){
					lastDialc->End = dialc->Start;
				}
			}

			int newend = (endt * dialc->Text.Len());
			if (newend < 1000){ newend = 1000; }
			newend += dialc->Start.mstime;
			dialc->End.NewTime(newend);
		}

		lastDialc = dialc;
		i++;
	}

	if (type == ASS){
		LoadDefault(false, true, false);
		wxString resx = Options.GetString(CONVERT_RESOLUTION_WIDTH);
		wxString resy = Options.GetString(CONVERT_RESOLUTION_HEIGHT);
		if (resx == L""){ resx = L"1280"; }
		if (resy == L""){ resx = L"720"; }
		AddSInfo(L"PlayResX", resx, false);
		AddSInfo(L"PlayResY", resy, false);
		AddSInfo(L"YCbCr Matrix", L"TV.601", false);
		const wxString & catalog = Options.GetString(CONVERT_STYLE_CATALOG);

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
		file->SortAll([](Dialogue *i, Dialogue *j){
			if (i->Start.mstime != j->Start.mstime){
				return (i->Start.mstime < j->Start.mstime);
			}
			if (i->End.mstime != j->End.mstime){
				return (i->End.mstime < j->End.mstime);
			}
			return (i->Text.CmpNoCase(j->Text) < 0);
		});
		Dialogue *lastDialogue = file->GetDialogue(0);
		size_t i = 1;
		while (i < file->GetCount()){
			Dialogue *actualDialogue = file->GetDialogue(i);
			if (lastDialogue->Start == actualDialogue->Start &&
				lastDialogue->End == actualDialogue->End &&
				lastDialogue->Text == actualDialogue->Text){
				file->DeleteDialogues(i - 1, i);
				lastDialogue = actualDialogue;
				continue;
			}
			else if (actualDialogue->Text == L""){
				file->DeleteDialogues(i, i + 1);
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
	Edit->SetLine((currentLine < GetCount()) ? currentLine : 0);
	SpellErrors.clear();
	SetModified(GRID_CONVERT);
	RefreshColumns();
}

void SubsGridBase::SaveFile(const wxString &filename, bool normalSave, bool loadFromEditbox)
{
	wxMutexLocker lock(editionMutex);

	int saveAfterCharacterCount = Options.GetInt(GRID_SAVE_AFTER_CHARACTER_COUNT);
	bool showOriginalOnVideo = !Options.GetBool(TL_MODE_HIDE_ORIGINAL_ON_VIDEO);
	bool dummyEditboxChanges = (loadFromEditbox && !saveAfterCharacterCount);
	if (dummyEditboxChanges || saveAfterCharacterCount > 1){
		Edit->Send(EDITBOX_LINE_EDITION, false, dummyEditboxChanges, true);
	}
	wxString txt;
	const wxString &tlmode = GetSInfo(L"TLMode");
	bool translated = tlmode == L"Translated";
	bool tlmodeOn = tlmode != L"";

	OpenWrite ow(filename, true);

	if (subsFormat < SRT){
		//if (cstat){
			//AddSInfo(L"Last Style Storage", Options.actualStyleDir, false);
			AddSInfo(L"Active Line", std::to_wstring(currentLine), false);
			wxString subsPath = tab->SubsPath.BeforeLast(L'\\');
			if (Edit->ABox){
				wxString path = (Edit->ABox->audioName.StartsWith(subsPath) && normalSave) ?
					Edit->ABox->audioName.AfterLast(L'\\') : Edit->ABox->audioName;
				AddSInfo(L"Audio File", path, false);
			}
			if (!tab->VideoPath.empty()){
				wxString path = (tab->VideoPath.StartsWith(subsPath) && normalSave) ?
					tab->VideoPath.AfterLast(L'\\') : tab->VideoPath;
				AddSInfo(L"Video File", path, false);
			}
			if (!tab->KeyframesPath.empty()){
				wxString path = (tab->KeyframesPath.StartsWith(subsPath) && normalSave) ?
					tab->KeyframesPath.AfterLast(L'\\') : tab->KeyframesPath;
				AddSInfo(L"Keyframes File", path, false);
			}
		//}

		txt << L"[Script Info]\r\n;Script generated by " << Options.progname << L"\r\n";
		GetSInfos(txt, translated);
		txt << L"\r\n[V4+ Styles]\r\nFormat: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding \r\n";
		GetStyles(txt, translated);
		txt << L" \r\n[Events]\r\nFormat: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\r\n";
	}
	ow.PartFileWrite(txt);

	txt = GetSInfo(L"TLMode Style");
	wxString raw;
	if (loadFromEditbox){
		for (size_t i = 0; i < file->GetCount(); i++)
		{
			Dialogue *dial = file->GetDialogue(i);
			if (!ignoreFiltered && !dial->isVisible || dial->NonDialogue){ continue; }
			//a tu trzeba w przypadku ebrow pobrać editbox line
			if (i == currentLine){ dial = Edit->line; };

			if (tlmodeOn){
				bool hasTextTl = dial->TextTl != L"";
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
					raw << i + 1 << L"\r\n";
				}
				dial->GetRaw(&raw);
			}

			ow.PartFileWrite(raw);
			raw.Empty();

		}
	}
	else{
		for (size_t i = 0; i < file->GetCount(); i++)
		{
			Dialogue *dial = file->GetDialogue(i);

			if (tlmodeOn){
				bool hasTextTl = dial->TextTl != L"";
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
					raw << i + 1 << L"\r\n";
				}
				dial->GetRaw(&raw);
			}

			ow.PartFileWrite(raw);
			raw.Empty();

			if (normalSave && dial->GetState() & 1){ dial->ChangeDialogueState(2); }

		}
	}

	ow.CloseFile();
	if (normalSave){
		file->SetLastSave();
		Refresh(false);
	}
}

void SubsGridBase::AddStyle(Styles *nstyl)
{
	file->AddStyle(nstyl);
}

void SubsGridBase::ChangeStyle(Styles *nstyl, size_t i)
{
	file->ChangeStyle(nstyl, i);
}

size_t SubsGridBase::StylesSize()
{
	return file->StylesSize();
}

Styles * SubsGridBase::GetStyle(size_t i, const wxString &name/*=""*/)
{
	return file->GetStyle(i, name);
}

std::vector<Styles*> *SubsGridBase::GetStyleTable()
{
	return file->GetStyleTable();
}
//multiplication have to be set to zero, then gets number of multiplication
int SubsGridBase::FindStyle(const wxString &name, int *multip)
{
	return file->FindStyle(name, multip);
}

void SubsGridBase::GetStyles(wxString &stylesText, bool tld/*=false*/)
{
	file->GetStyles(stylesText, tld);
}

void SubsGridBase::DelStyle(int i)
{
	file->DeleleStyle(i);
}

size_t SubsGridBase::GetCount()
{
	return file->GetCount();
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
	VideoFfmpeg *FFMS2 = tab->Video->GetFFMS2();
	if (byFrame && !FFMS2){ wxLogMessage(_("Wideo nie zostało wczytane przez FFMS2")); return; }
	//1 forward / backward, 2 Start Time For V/A Timing, 4 Move to video time, 8 Move to audio time;
	int moveTimeOptions = Options.GetInt(SHIFT_TIMES_OPTIONS);

	//Time to move
	int time = (!byFrame) ? Options.GetInt(SHIFT_TIMES_TIME) : 0;
	int frame = (byFrame) ? Options.GetInt(SHIFT_TIMES_DISPLAY_FRAMES) : 0;
	int whichLines = MAX(0, Options.GetInt(SHIFT_TIMES_WHICH_LINES));
	int whichTimes = MAX(0, Options.GetInt(SHIFT_TIMES_WHICH_TIMES));
	int correctEndTimes = Options.GetInt(SHIFT_TIMES_CORRECT_END_TIMES);
	//1 Lead In, 2 Lead Out, 4 Make times continous, 8 Snap to keyframe;
	int PostprocessorOptions = Options.GetInt(POSTPROCESSOR_ON);
	int LeadIn = 0, LeadOut = 0, ThresholdStart = 0, ThresholdEnd = 0,
		KeyframeBeforeStart = 0, KeyframeAfterStart = 0, KeyframeBeforeEnd = 0, KeyframeAfterEnd = 0;

	if (PostprocessorOptions){
		if (subsFormat == TMP || PostprocessorOptions < 16){ PostprocessorOptions = 0; }
		else if (PostprocessorOptions & 8 && !FFMS2){ PostprocessorOptions ^= 8; }
		LeadIn = Options.GetInt(POSTPROCESSOR_LEAD_IN);
		LeadOut = Options.GetInt(POSTPROCESSOR_LEAD_OUT);
		ThresholdStart = Options.GetInt(POSTPROCESSOR_THRESHOLD_START);
		ThresholdEnd = Options.GetInt(POSTPROCESSOR_THRESHOLD_END);
		KeyframeBeforeStart = Options.GetInt(POSTPROCESSOR_KEYFRAME_BEFORE_START);
		KeyframeAfterStart = Options.GetInt(POSTPROCESSOR_KEYFRAME_AFTER_START);
		KeyframeBeforeEnd = Options.GetInt(POSTPROCESSOR_KEYFRAME_BEFORE_END);
		KeyframeAfterEnd = Options.GetInt(POSTPROCESSOR_KEYFRAME_AFTER_END);
	}
	wxString styles = Options.GetString(SHIFT_TIMES_STYLES);
	if (styles.empty() && whichLines == 5){
		KaiMessageBox(_("Nie wybrano stylów do przesunięcia"), _("Uwaga"));
		return;
	}
	else{
		styles = L"," + styles + L",";
	}
	bool changeTagTimes = (moveTimeOptions & 32) > 0;

	if (!(moveTimeOptions & 1)){
		time = (-time);
		frame = (-frame);
	}
	// video / audio move start or end
	int VAS = moveTimeOptions & 2;

	std::multimap<Dialogue *, int, compare> tmpmap;

	if (whichTimes != 0 && !PostprocessorOptions){
		int answer = KaiMessageBox(wxString::Format(_("Czy naprawdę chcesz przesuwać tylko czasy %s?"),
			(whichTimes == 1) ? _("początkowe") : _("końcowe")), _("Potwierdzenie"), wxYES_NO);
		if (answer == wxNO){ return; }
	}

	if (subsFormat == TMP){ whichTimes = 1; }

	//bool fromStyle = false;

	size_t firstSelection = FirstSelection();
	if (firstSelection == -1 && whichLines != 0 && whichLines != 4){
		KaiMessageBox(_("Nie zaznaczono linii do przesunięcia"), _("Uwaga")); return;
	}

	int difftime = (VAS) ? file->GetDialogue(markedLine)->Start.mstime : file->GetDialogue(markedLine)->End.mstime;

	if ((moveTimeOptions & 4) && tab->Video->GetState() != None){
		if (byFrame){
			frame += tab->Video->GetCurrentFrame() - FFMS2->GetFramefromMS(difftime);
		}
		else{
			int addedTimes = tab->Video->GetFrameTime(VAS != 0) - difftime;
			if (addedTimes < 0){ addedTimes -= 10; }
			time += ZEROIT(addedTimes);
		}

	}
	else if ((moveTimeOptions & 8) && Edit->ABox && Edit->ABox->audioDisplay->hasMark){
		if (byFrame){
			frame += FFMS2->GetFramefromMS(Edit->ABox->audioDisplay->curMarkMS - difftime);
		}
		else{
			int addedTimes = Edit->ABox->audioDisplay->curMarkMS - difftime;
			if (addedTimes < 0){ addedTimes -= 10; }
			time += ZEROIT(addedTimes);
		}
	}

	int firsttime = GetDialogue(firstSelection)->Start.mstime;
	Dialogue *dialc;
	Dialogue *Dial;
	bool skipFiltered = !ignoreFiltered;
	if (PostprocessorOptions){ time = 0; frame = 0; VAS = 0; whichTimes = 0; correctEndTimes = 0; moveTimeOptions = 0; }

	for (size_t i = 0; i < file->GetCount(); i++)
	{
		Dial = file->GetDialogue(i);
		if (skipFiltered && !Dial->isVisible || Dial->NonDialogue){ continue; }

		if (whichLines == 0
			|| (whichLines == 1 && file->IsSelected(i))//selected lines
			|| (whichLines == 3 && firsttime <= Dial->Start.mstime)//times higher or equal
			|| (whichLines == 2 && i >= firstSelection)//from selection
			|| (whichLines == 4 && firsttime >= Dial->Start.mstime)//times lower or equal
			|| (whichLines == 5 && styles.Find(L"," + Dial->Style + L",") != -1))//by choosen styles
		{

			dialc = file->CopyDialogue(i, true, true);
			int startTrimed = 0, endTrimed = 0, duration = 0;
			if (changeTagTimes){
				tab->Video->GetStartEndDelay(dialc->Start.mstime, dialc->End.mstime, &startTrimed, &endTrimed);
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
					int startFrame = FFMS2->GetFramefromMS(dialc->Start.mstime) + frame;
					dialc->Start.NewTime(ZEROIT(tab->Video->GetFrameTimeFromFrame(startFrame)));
				}
				if (whichTimes != 1){
					int endFrame = FFMS2->GetFramefromMS(dialc->End.mstime) + frame;
					dialc->End.NewTime(ZEROIT(tab->Video->GetFrameTimeFromFrame(endFrame)));
				}
				dialc->ChangeDialogueState(1);
			}
			if (changeTagTimes){
				int newStartTrimed = 0, newEndTrimed = 0;
				tab->Video->GetStartEndDelay(dialc->Start.mstime, dialc->End.mstime, &newStartTrimed, &newEndTrimed);
				if (byFrame){ newEndTrimed += ((dialc->End.mstime - dialc->Start.mstime) - duration); }
				dialc->ChangeTimes(newStartTrimed - startTrimed, (newEndTrimed - endTrimed));
			}

			if (correctEndTimes > 0 || PostprocessorOptions > 16){
				tmpmap.insert(std::pair<Dialogue *, int>(dialc, i));
			}
			else{
				dialc->ClearParse();
			}

		}// if shifted line

	}//loop for

	// tu jeszcze należy poprawić uwzględniając linijkę z czasem przed tablicą i czasem po niej
	// a może to w ogóle nie jest potrzebne?
	if (correctEndTimes > 0 || PostprocessorOptions > 16){
		bool hasend = false;
		int newstarttime = -1;
		int endt = Options.GetInt(CONVERT_TIME_PER_CHARACTER);
		bool isPreviousEndGreater = false;
		bool isEndGreater = false;
		bool previousIsKeyFrame = true;
		for (auto cur = tmpmap.begin(); cur != tmpmap.end(); cur++){
			auto it = cur;
			dialc = cur->first;
			it++;
			if (!(it != tmpmap.end())){ it = cur; hasend = true; }
			else{
				isEndGreater = dialc->End > it->first->Start || dialc->End == it->first->End || cur == tmpmap.begin();
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
			bool foundStartKeyframe = false;
			bool foundEndKeyframe = false;
			int oldStart = dialc->Start.mstime;
			int oldEnd = dialc->End.mstime;
			int numOfStartModifications = 0;
			int numOfEndModifications = 0;
			int previousEnd = 0;
			auto itplus = it;

			if (cur != tmpmap.begin()) {
				it--;
				if (!hasend) { it--; }
				previousEnd = it->first->End.mstime;
			}
			

			if (PostprocessorOptions & 1) {
				dialc->Start.Change(-LeadIn);
				if (dialc->Start < it->first->End && cur != tmpmap.begin()) {
					dialc->Start = it->first->End;
				}
				if (oldStart != dialc->Start.mstime)
					numOfStartModifications++;
			}
			if (PostprocessorOptions & 2) {
				dialc->End.Change(LeadOut);
				if (dialc->End > itplus->first->Start) {
					dialc->End = itplus->first->Start;
				}
				if (oldEnd != dialc->End.mstime)
					numOfEndModifications++;
			}

			//Keyframes goes first and rest cannot apply when keyframe was set
			if (PostprocessorOptions & 8){
				int startRange = dialc->Start.mstime - KeyframeBeforeStart;
				int startRange1 = oldStart + KeyframeAfterStart;
				int endRange = oldEnd - KeyframeBeforeEnd;
				int endRange1 = dialc->End.mstime + KeyframeAfterEnd;
				

				int keyMS = 0;
				int startResult = INT_MAX;
				int endResult = -1;
				for (size_t g = 0; g < FFMS2->KeyFrames.Count(); g++) {
					keyMS = FFMS2->KeyFrames[g];
					if (keyMS > startRange && keyMS < startRange1) {
						keyMS = ZEROIT(tab->Video->GetFrameTimeFromTime(keyMS));
						if (oldStart == keyMS) {
							startRange = -1; startRange1 = -1; startResult = INT_MAX;
							dialc->Start.NewTime(oldStart);
							numOfStartModifications--;
						}
						if (startResult > keyMS/* && keyMS != dialc->Start.mstime*/){ startResult = keyMS; }
					}
					if (keyMS > endRange && keyMS < endRange1) {
						keyMS = ZEROIT(tab->Video->GetFrameTimeFromTime(keyMS));
						if (oldEnd == keyMS) {
							endRange = -1; endRange1 = -1; endResult = -1;
							dialc->End.NewTime(oldEnd);
							numOfEndModifications--;
						}
						if (endResult < keyMS && keyMS > dialc->Start.mstime){ endResult = keyMS; }
					}
				}
				//here is main problem we do not know if next start will be changed
				//and it makes mess here but changing only start should be enough
				//startResult >= compareStart and endResult <= compareEnd should be changed or even remove
				if (startResult != INT_MAX){
					if (dialc->Start != startResult){
						dialc->Start.NewTime(startResult);
						numOfStartModifications++;
					}
					foundStartKeyframe = true;
				}
				if (endResult != -1){
					if (dialc->End != endResult){
						dialc->End.NewTime(endResult);
						numOfEndModifications++;
					}
					foundEndKeyframe = true;
				}
				//make sure that start is greater or even then end time of previous line
				//and correct times but only when previous line end time was not greater than start time on start
				//then discard correction.
				if (dialc->Start.mstime < previousEnd && !isPreviousEndGreater && previousEnd < dialc->End.mstime && foundStartKeyframe){
					dialc->Start.NewTime(previousEnd);
					numOfStartModifications--;
					foundStartKeyframe = false;
				}
			}
			

			if (PostprocessorOptions & 4){
				int cdiff = (ThresholdEnd + ThresholdStart);
				int tdiff = dialc->Start.mstime - it->first->End.mstime;
				newstarttime = -1;
				if (tdiff <= cdiff && tdiff > 0 && !previousIsKeyFrame) {
					int coeff = ((float)tdiff / (float)cdiff) * ThresholdEnd;
					int newtime = ZEROIT(coeff);
					it->first->End.Change(newtime);
					newstarttime = it->first->End.mstime;
					it->first->ChangeDialogueState(1);

				}
				if (!foundStartKeyframe && newstarttime != -1){
					dialc->Start.NewTime(newstarttime);
					newstarttime = -1;
					numOfStartModifications++;
				}
				

			}
			if (numOfStartModifications > 0 || numOfEndModifications > 0)
				dialc->ChangeDialogueState(1);

			previousIsKeyFrame = foundEndKeyframe;
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
	//wxBell();
#endif
}


void SubsGridBase::SortIt(short what, bool all)
{
	SaveSelections();
	if (all){
		for (size_t i = 0; i < GetCount(); i++){ file->GetDialogue(i)->ChangeDialogueState(1); }
		file->SortAll((what == 0) ? sortstart : (what == 1) ? sortend : (what == 2) ? sortstyle :
			(what == 3) ? sortactor : (what == 4) ? sorteffect : sortlayer);
	}
	else{
		file->SortSelected((what == 0) ? sortstart : (what == 1) ? sortend : (what == 2) ? sortstyle :
			(what == 3) ? sortactor : (what == 4) ? sorteffect : sortlayer);
	}

	file->edited = true;
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
	file->DeleteSelectedDialogues();
	int sel = FirstSelection();
	SpellErrors.clear();
	SaveSelections(true);
	if (GetCount() < 1){ AddLine(new Dialogue()); }
	SetModified(GRID_DELETE_LINES, true, false, sel);
	Thaw();
	RefreshColumns();
}

bool SubsGridBase::MoveRows(int step, bool keyStep /*= false*/)
{
	//this would be less complicated if it use a id for calculation
	//but it would take more time for calculate ids for all selected lines
	wxArrayInt sels;
	file->GetSelections(sels);

	if (sels.GetCount() < 1 && step == 0){ return false; }
	int blocker = 0;
	int backblocker = 0;
	// get skipped lines from start
	if (step < 0){
		for (auto cur = sels.begin(); cur != sels.end(); cur++){
			if (blocker == file->GetElementByKey(*cur)){
				blocker++;
			}
			else
				break;
		}
	}
	int size = GetKeyFromPosition(GetCount(), -1);
	int blockersize = size;
	std::vector<std::pair<Dialogue*, int>> selectedDialogs;
	for (auto cur = sels.rbegin(); cur != (sels.rend() - blocker); cur++)
	{
		int sel = *cur;
		//skip lines from end to
		if (step > 0 && blockersize == sel){
			blockersize = GetKeyFromPosition(blockersize, -1);
			backblocker++;
			continue;
		}
		selectedDialogs.push_back(std::make_pair(GetDialogue(sel), sel));
		DeleteRow(sel);
		file->EraseSelection(sel);
	}
	//if something is not skipped go to moving
	if (!selectedDialogs.size())
		return false;

	
	if (step < 0){
		int lastSel = -1;
		for (auto cur = selectedDialogs.rbegin(); cur != selectedDialogs.rend(); cur++)
		{
			const std::pair<Dialogue*, int> &dialPair = *cur;
			int sel = GetKeyFromPosition(dialPair.second, step);
			//blocks lines from start tnat it not step onto skipped lines that are unchanged
			if (sel < blocker){
				sel = blocker;
				blocker++;
			}
			// when moving a block of lines and it steps into hidden lines
			// the sel gives the same value for every element of hidden block
			// that why this if checks if last value is not equal or more
			if (sel <= lastSel){
				sel = GetKeyFromPosition(lastSel, 1);
			}
			//copy lines that it can change state
			//when undo or redo used
			Dialogue *Dialc = dialPair.first->Copy();
			Dialc->ChangeDialogueState(1);
			InsertRows(sel, 1, Dialc);
			file->InsertSelection(sel);
			lastSel = sel;
		}
	}
	else
	{
		int lastSel = -1;
		for (auto cur = selectedDialogs.rbegin(); cur != selectedDialogs.rend(); cur++)
		{
			const std::pair<Dialogue*, int> &dialPair = *cur;
			int sel = GetKeyFromPosition(dialPair.second, step);
			//adds one when hidden block is after sel
			//cause it only change place and line doesn't change it
			if (sel - step != dialPair.second){ 
				sel++; 
			}
			//blocks lines from end tnat it not step onto skipped lines that are unchanged
			if (sel > size - backblocker){
				sel = size - backblocker;
				backblocker++;
			}
			// when moving a block of lines and it steps into hidden lines
			// the sel gives the same value for every element of hidden block
			// that why this if checks if last value is not equal or less
			if (sel <= lastSel){
				sel = GetKeyFromPosition(lastSel, 1);
			}
			//copy lines that it can change state
			//when undo or redo used
			Dialogue *Dialc = dialPair.first->Copy();
			Dialc->ChangeDialogueState(1);
			InsertRows(sel, 1, Dialc);
			file->InsertSelection(sel);
			lastSel = sel;
		}
	}
	size_t firstSelection = FirstSelection();
	Edit->SetLine(firstSelection);
	ScrollTo(firstSelection, true);
	Refresh(false);
	return true;
}

void SubsGridBase::DeleteText()
{
	wxArrayInt sels;
	file->GetSelections(sels);
	for (auto i : sels){
		CopyDialogue(i)->Text = L"";
	}
	SetModified(GRID_DELETE_TEXT);
	Refresh(false);
}
void SubsGridBase::UpdateUR(bool toolbar)
{
	bool undo = false, _redo = false;
	file->GetURStatus(&undo, &_redo);
	Kai->Menubar->Enable(GLOBAL_UNDO, undo);
	Kai->Menubar->Enable(GLOBAL_REDO, _redo);
	Kai->Menubar->Enable(GLOBAL_UNDO_TO_LAST_SAVE, file->GetActualHistoryIter() != 0 && file->GetLastSaveIter() != -1);
	if (toolbar){
		Kai->Toolbar->UpdateId(GLOBAL_UNDO, undo);
		Kai->Toolbar->UpdateId(GLOBAL_REDO, _redo);
		Kai->Toolbar->UpdateId(GLOBAL_SAVE_SUBS, true);
		int iter = file->Iter();
		Kai->Toolbar->UpdateId(GLOBAL_HISTORY, iter > 0);
	}
}

bool SubsGridBase::IsModified()
{
	return file->CanSave();
}

void SubsGridBase::GetUndo(bool redo, int iter)
{
	//wxMutexLocker lock(editionMutex);
	Freeze();
	const wxString &resolution = GetSInfo(L"PlayResX") + L" x " + GetSInfo(L"PlayResY");
	const wxString &matrix = GetSInfo(L"YCbCr Matrix");
	const wxString &tlmode = GetSInfo(L"TLMode");
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
			tab->Video->DisableVisuals(subsFormat != ASS);
		}
	}

	Thaw();

	if (StyleStore::HasStore()){
		StyleStore *SS = StyleStore::Get();
		SS->ASSList->SetArray(file->GetStyleTable());
		SS->ASSList->Refresh(false);
	}
	SpellErrors.clear();

	const wxString &newtlmode = GetSInfo(L"TLMode");
	if (newtlmode != tlmode){
		hasTLMode = (newtlmode == L"Yes");
		showOriginal = (GetSInfo(L"TLMode Showtl") == L"Yes" || (hasTLMode && Options.GetBool(TL_MODE_SHOW_ORIGINAL) != 0));
		Edit->SetTlMode(hasTLMode);
	}
	if (Comparison){
		SubsComparison();
	}

	int corrected = -1;
	Edit->SetLine(file->FindVisibleKey(file->GetActiveLine(), &corrected));
	markedLine = file->FindVisibleKey(file->GetMarkerLine());
	scrollPosition = file->FindVisibleKey(file->GetScrollPosition());
	scrollPositionId = file->GetElementByKey(scrollPosition);
	if (corrected >= 0){
		file->EraseSelection(file->GetActiveLine());
		file->InsertSelection(corrected);
	}

	RefreshColumns();
	Edit->RefreshStyle();
	VideoCtrl *vb = tab->Video;

	const wxString &newResolution = GetSInfo(L"PlayResX") + L" x " + GetSInfo(L"PlayResY");
	if (resolution != newResolution){
		Kai->SetSubsResolution();
		vb->ChangeOnScreenResolution(tab);
	}
	const wxString &newmatrix = GetSInfo(L"YCbCr Matrix");
	if (matrix != newmatrix){
		vb->SetColorSpace(newmatrix);
	}

	if (Edit->Visual < CHANGEPOS){

		if (vb->IsShown() || vb->IsFullScreen()){ vb->OpenSubs(OPEN_DUMMY); }
		int seekAfter = 0;
		vb->GetVideoListsOptions(NULL, &seekAfter);
		if (seekAfter > 1){
			if (vb->GetState() == Paused || (vb->GetState() == Playing && (seekAfter == 3 || seekAfter == 5))){
				vb->Seek(Edit->line->Start.mstime);
			}
		}
		else{
			if (vb->GetState() == Paused){ vb->Render(); }
		}
	}
	else if (Edit->Visual == CHANGEPOS){
		vb->SetVisual(true);
	}
	else {
		vb->SetVisual(true, true);
		if (vb->GetState() == Paused){ vb->Render(); }
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
	//wxMutexLocker lock(editionMutex);
	file->DummyUndo(newIter);
	if (SpellErrors.size() > currentLine)
		SpellErrors[currentLine].clear();

	Edit->SetLine(currentLine, false, false);
	RefreshColumns();
	UpdateUR();
	Kai->Label(file->GetActualHistoryIter());
	VideoCtrl *vb = tab->Video;
	if (vb->GetState() != None){
		vb->OpenSubs(OPEN_DUMMY);
		vb->Render();
	}
}

size_t SubsGridBase::FirstSelection(size_t *firstSelectionId /*= NULL*/)
{
	return file->FirstSelection();
}


//Uważaj na dodawanie do niszczarki, 
//bo brak dodania gdy trzeba to wycieki pamięci,
//a podwójne dodanie to krasz przy niszczeniu obiektu.
void SubsGridBase::InsertRows(int Row,
	const std::vector<Dialogue *> &RowsTable, bool AddToDestroy)
{
	file->InsertRows(Row, RowsTable, AddToDestroy);
	//spellErrors Array take all dialogues for compatybility
	//but can be simply cleared
	if (SpellErrors.size() > Row){
		TextData emptyarray;
		SpellErrors.insert(SpellErrors.begin() + Row, RowsTable.size(), emptyarray);
	}
}

//Uważaj na dodawanie do niszczarki, 
//bo brak dodania gdy trzeba to wycieki pamięci,
//a podwójne dodanie to krasz przy niszczeniu obiektu.
void SubsGridBase::InsertRows(int Row, int NumRows, Dialogue *Dialog, bool AddToDestroy, bool Save)
{
	file->InsertRows(Row, NumRows, Dialog, AddToDestroy, Save);
	//spellErrors Array take all dialogues for compatybility
	//but can be simply cleared
	if (SpellErrors.size() > Row){
		TextData emptyarray;
		SpellErrors.insert(SpellErrors.begin() + Row, NumRows, emptyarray);
	}
	if (Save){
		file->InsertSelection(Row);
		SetModified(GRID_INSERT_ROW);
		Refresh(false);
	}
}

void SubsGridBase::SetSubsFormat(wxString ext)
{
	subsFormat = ASS;
	int rw = 0;
	char subsext = (ext == L"ass" || ext == L"ssa") ? ASS : (ext == L"srt") ? SRT : TMP;
	while (rw < GetCount())
	{
		Dialogue *dial = GetDialogue(rw);
		if (dial->NonDialogue || dial->Format == 0){ rw++; }
		else if (!ext.empty() && (subsext != dial->Format || (subsext == TMP && dial->Format>SRT))){ rw++; }//form=dial->Form; 
		else{ subsFormat = dial->Format; break; }
	}
}

//need to guard
void SubsGridBase::AddSInfo(const wxString &SI, wxString val, bool save)
{
	file->AddSInfo(SI, val, save);
}

void SubsGridBase::GetSInfos(wxString &textSinfo, bool tld/*=false*/)
{
	file->GetSInfos(textSinfo, tld);
}

//dont guard cause most of functions that it uses have own gauard
//Every SetModified have to find on list and add etitionType
void SubsGridBase::SetModified(unsigned char editionType, bool redit, bool dummy, int SetEditBoxLine, bool Scroll)
{
	if (file->IsNotSaved()){
		//wxMutexLocker lock(editionMutex);
		if (!IsModified()){
			Kai->Toolbar->UpdateId(GLOBAL_SAVE_SUBS, true);
			Kai->Menubar->Enable(GLOBAL_SAVE_SUBS, true);
		}
		if (Comparison){
			SubsComparison();
		}
		if (!savedSelections){
			SaveSelections();
		}
		savedSelections = false;
		if (redit){
			int newCurrentLine = (SetEditBoxLine >= 0) ? SetEditBoxLine : currentLine;
			if (newCurrentLine >= GetCount()){ newCurrentLine = GetCount() - 1; }
			lastRow = newCurrentLine;
			int w, h;
			GetClientSize(&w, &h);
			if (Scroll){
				if (Options.GetBool(GRID_DONT_CENTER_ACTIVE_LINE))
					MakeVisible(newCurrentLine);
				else
					ScrollTo(newCurrentLine, true);
			}
			Edit->SetLine(newCurrentLine);
			file->InsertSelection(newCurrentLine);
		}
		file->SaveUndo(editionType, currentLine, markedLine);
		Kai->Label(file->GetActualHistoryIter(), false, Kai->Tabs->FindPanel(tab));
		if (!dummy){
			VideoCtrl *vb = tab->Video;
			if (Edit->Visual >= CHANGEPOS){
				vb->SetVisual(true);
			}
			else{
				if (vb->IsShown() || vb->IsFullScreen()){ vb->OpenSubs(OPEN_DUMMY); }

				int seekAfter;
				vb->GetVideoListsOptions(NULL, &seekAfter);
				if (seekAfter > 1){
					if (vb->GetState() == Paused || (vb->GetState() == Playing && (seekAfter == 3 || seekAfter == 5))){
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
	file->SwapRows(frst, scnd);
	if (SpellErrors.size() > frst && SpellErrors.size() > scnd){
		TextData tmpspell = SpellErrors[frst];
		SpellErrors[frst] = SpellErrors[scnd];
		SpellErrors[scnd] = tmpspell;
	}
	Refresh(false);
	if (sav){ SetModified(GRID_SWAP); }
}

void SubsGridBase::LoadSubtitles(const wxString &str, wxString &ext)
{
	bool oldHasTlMode = hasTLMode;
	int active = 0;

	SubsLoader SL((SubsGrid*)this, str, ext);

	if (oldHasTlMode != hasTLMode){
		Edit->SetTlMode(hasTLMode);
		Kai->Menubar->Enable(GLOBAL_SAVE_TRANSLATION, hasTLMode);
	}
	if (hasTLMode && (GetSInfo(L"TLMode Showtl") == L"Yes" || Options.GetBool(TL_MODE_SHOW_ORIGINAL))){ showOriginal = true; }


	if (subsFormat == MDVD || subsFormat == MPL2){
		int endt = Options.GetInt(CONVERT_TIME_PER_CHARACTER);
		for (size_t i = 0; i < GetCount(); i++){
			Dialogue *dial = GetDialogue(i);

			if (dial->End.mstime == 0){
				int newend = (endt*dial->Text.Len());
				if (newend < 1000){ newend = 1000; }
				newend += dial->Start.mstime;
				dial->End.NewTime(newend);
				if (i<GetCount() - 1){
					if (dial->End > file->GetDialogue(i + 1)->Start){
						dial->End = file->GetDialogue(i + 1)->Start;
					}
				}
			}
		}
	}
	else if (subsFormat == ASS){
		if (ext != L"ass"){ originalFormat = 0; if (StylesSize() < 1){ AddStyle(new Styles()); } }
		Edit->TlMode->Enable(true); Edit->RefreshStyle();
		if (Options.GetBool(GRID_LOAD_SORTED_SUBS)){
			file->SortAll(sortstart);
		}
		active = wxAtoi(GetSInfo(L"Active Line"));
		if (active >= GetCount()){ active = 0; }
	}
	else{ Edit->TlMode->Enable(false); }


	file->InsertSelection(active);
	lastRow = active;
	markedLine = active;

	file->EndLoad(OPEN_SUBTITLES, active);

	RefreshColumns();
	//it's faster to change load audio to true than setting audio from kainoteMain 
	//to avoid not changed line when loaded only subtitles.
	Edit->SetLine(active, true, false);

	Edit->HideControls();

	if (StyleStore::HasStore() && subsFormat == ASS){ StyleStore::Get()->LoadAssStyles(); }
	if (subsFormat == ASS){
		int filterBy = Options.GetInt(GRID_FILTER_BY);
		if (filterBy && Options.GetBool(GRID_FILTER_AFTER_LOAD) && filterBy != FILTER_BY_SELECTIONS){
			isFiltered = true;
			SubsGridFiltering filter((SubsGrid*)this, currentLine);
			filter.Filter(true);
		}
		Edit->RebuildActorEffectLists();
	}
	((SubsGridWindow*)this)->ScrollTo(active, false, -4);
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
		if (GetSInfo(L"TLMode") == L""){
			
			int ssize = file->StylesSize();
			if (ssize > 0){
				Styles *tlstyl = GetStyle(0, L"Default")->Copy();
				for (int i = 0; i < ssize; i++){
					wxString ns = L"TLmode";
					wxString nss = (i == 0) ? ns : ns << i;
					if (FindStyle(nss) == -1){ tlstyl->Name = nss; AddSInfo(L"TLMode Style", nss); break; }
				}
				tlstyl->Alignment = L"8";
				AddStyle(tlstyl);
			}

		}
		AddSInfo(L"TLMode", L"Yes");
		hasTLMode = true;
		if (Options.GetBool(TL_MODE_SHOW_ORIGINAL)){ showOriginal = true; }
		Kai->Menubar->Enable(GLOBAL_SAVE_TRANSLATION, true);

	}
	else{
		if (KaiMessageBox(_("Czy na pewno chcesz wyłączyć tryb tłumacza?\nObcojęzyczny tekst przetłumaczonych linijek zostanie usunięty."), 
			_("Potwierdzenie"), wxYES_NO, NULL, wxDefaultPosition, wxNO) == wxNO){
			return true;
		}

		int iinf = -1;
		GetSInfo(L"TLMode", &iinf);
		if (iinf >= 0){
			file->DeleteSInfo(iinf);
		}
		iinf = -1;
		const wxString &vall = GetSInfo(L"TLMode Style", &iinf);
		if (iinf >= 0){
			int g = FindStyle(vall);
			if (g >= 0){ DelStyle(g); }
			file->DeleteSInfo(iinf);
		}

		for (size_t i = 0; i < file->GetCount(); i++)
		{
			Dialogue *dial = file->GetDialogue(i);
			Dialogue *dialc = NULL;
			if (dial->TextTl != L"")
			{
				dialc = file->CopyDialogue(i);
				dialc->Text = dialc->TextTl;
				dialc->TextTl = L"";
			}
			if (dial->IsDoubtful()){
				if (!dialc){ dialc = CopyDialogue(i); }
				dialc->ChangeState(4);
			}
		}

		hasTLMode = false;
		showOriginal = false;
		Kai->Menubar->Enable(GLOBAL_SAVE_TRANSLATION, false);
	}
	Edit->RefreshStyle();
	SpellErrors.clear();
	Refresh(false);
	SetModified((mode) ? GRID_TURN_ON_TLMODE : GRID_TURN_OFF_TLMODE);
	return false;
}

//this method should have different name
//it also can change line to previous
void SubsGridBase::NextLine(int direction)
{
	if (Edit->ABox && Edit->ABox->audioDisplay->hold != 0){ return; }
	int size = GetCount();
	size_t newCurrentLine = GetKeyFromPosition(currentLine, direction, false);
	if (newCurrentLine == -1){
		if (direction < 0)
			return;

		size_t lastvisible = file->GetElementByKey(size - 1);
		Dialogue *tmp = GetDialogue(lastvisible)->Copy();
		int eend = tmp->End.mstime;
		tmp->Start.NewTime(eend);
		tmp->End.NewTime(eend + 5000);
		tmp->Text = L"";
		tmp->TextTl = L"";
		AddLine(tmp);
		SetModified(GRID_APPEND_LINE, false);
		AdjustWidths(subsFormat > TMP ? (START | END) : 0);
		newCurrentLine = GetCount() - 1;
	}
	int h, w;
	GetClientSize(&w, &h);
	if (Options.GetBool(GRID_DONT_CENTER_ACTIVE_LINE))
		MakeVisible(newCurrentLine);
	else
		ScrollTo(newCurrentLine, true);

	file->ClearSelections();
	file->InsertSelection(newCurrentLine);
	lastRow = newCurrentLine;
	//AdjustWidths(0);
	Refresh(false);
	Edit->SetLine(newCurrentLine, true, true, false, true);
	SubsGrid *grid = (SubsGrid*)this;
	if (Comparison){ grid->ShowSecondComparedLine(newCurrentLine); }
	else if (grid->preview){ grid->preview->NewSeeking(); }
}



void SubsGridBase::LoadDefault(bool line, bool sav, bool endload)
{
	if (line)
	{
		AddLine(new Dialogue());
		AddStyle(new Styles());
		file->InsertSelection(0);
		originalFormat = subsFormat = ASS;
	}
	AddSInfo(L"Title", L"Kainote Ass File", sav);
	AddSInfo(L"PlayResX", L"1280", sav);
	AddSInfo(L"PlayResY", L"720", sav);
	AddSInfo(L"ScaledBorderAndShadow", L"yes", sav);
	AddSInfo(L"WrapStyle", L"0", sav);
	AddSInfo(L"ScriptType", L"v4.00+", sav);
	AddSInfo(L"Last Style Storage", L"Default", sav);
	AddSInfo(L"YCbCr Matrix", L"TV.601", sav);
	if (endload){
		file->EndLoad(NEW_SUBTITLES, 0);
	}
}

Dialogue *SubsGridBase::CopyDialogue(size_t i, bool push)
{
	if (push && (int)SpellErrors.size() > i){ SpellErrors[i].clear(); }
	return file->CopyDialogue(i, push);
}

Dialogue * SubsGridBase::CopyDialogueWithOffset(size_t i, int offset, bool push /*= true*/)
{
	size_t newPos = GetKeyFromPosition(i, offset, false);
	if (newPos != -1){
		return CopyDialogue(newPos);
	}
	return NULL;
}

Dialogue *SubsGridBase::GetDialogue(size_t i)
{
	return file->GetDialogue(i);
}

Dialogue * SubsGridBase::GetDialogueWithOffset(size_t i, int offset)
{
	size_t newPos = GetKeyFromPosition(i, offset, false);
	if (newPos != -1){
		return GetDialogue(newPos);
	}
	return NULL;
}

const wxString & SubsGridBase::GetSInfo(const wxString &key, int *ii/*=0*/)
{
	return file->GetSInfo(key, ii);
}

SInfo *SubsGridBase::GetSInfoP(const wxString &key, int *ii)
{
	return file->GetSInfoP(key, ii);
}

size_t SubsGridBase::SInfoSize()
{
	return file->SInfoSize();
}

wxString *SubsGridBase::SaveText()
{
	wxString *path = new wxString();

	wxString ext = (subsFormat < SRT) ? L"ass" : (subsFormat == SRT) ? L"srt" : L"txt";

	(*path) << Options.pathfull << L"\\Subs\\DummySubs_" << Notebook::GetTabs()->FindPanel(tab) << L"." << ext;

	SaveFile(*path, false, true);

	return path;
}

//this function is called from another thread
//need to guard every change in dialogues, styles, sinfos, and editbox->line
wxString *SubsGridBase::GetVisible(bool *visible, wxPoint *point, wxArrayInt *selected, bool allSubs)
{
	wxMutexLocker lock(editionMutex);
	bool showOriginalOnVideo = !Options.GetBool(TL_MODE_HIDE_ORIGINAL_ON_VIDEO);
	int _time = tab->Video->Tell();
	bool toEnd = tab->Video->GetState() == Playing;
	wxString *txt = new wxString();
	wchar_t bom = 0xFEFF;
	*txt << wxString(bom);
	if (subsFormat == ASS){
		(*txt) << L"[Script Info]\r\n";
		GetSInfos(*txt, false);
		(*txt) << L"\r\n[V4+ Styles]\r\nFormat: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding \r\n";
		GetStyles(*txt, false);
		(*txt) << L" \r\n[Events]\r\nFormat: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\r\n";
	}
	Edit->Send(EDITBOX_LINE_EDITION, false, true);
	if ((_time >= Edit->line->Start.mstime || toEnd) && _time < Edit->line->End.mstime){
		if (visible){ *visible = true; }
	}
	else if (visible){
		*visible = false;
	}
	bool noLine = true;
	bool isTlmode = GetSInfo(L"TLMode") == L"Yes";
	const wxString &tlStyle = GetSInfo(L"TLMode Style");
	int j = 1;

	for (size_t i = 0; i < file->GetCount(); i++)
	{
		Dialogue *dial = file->GetDialogue(i);
		if (!ignoreFiltered && !dial->isVisible || dial->NonDialogue){ continue; }
		if (i == currentLine){
			dial = Edit->line;
		}
		if (selected && file->IsSelected(i)){
			selected->Add(txt->length());
			continue;
		}
		if ((toEnd && _time <= dial->Start.mstime) || (_time >= dial->Start.mstime && _time < dial->End.mstime) || allSubs){
			if (isTlmode && dial->TextTl != L""){
				if (showOriginalOnVideo)
					dial->GetRaw(txt, false, tlStyle);

				dial->GetRaw(txt, true);
			}
			else if(dial->Text != L""){
				if (subsFormat == SRT){
					(*txt) << j << L"\r\n";
					j++;
				}
				dial->GetRaw(txt);
			}
			if (point && i == currentLine){
				int all = txt->length(); 
				point->x = all - 2;
				int len = (isTlmode && dial->TextTl != L"") ?
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

bool SubsGridBase::IsLineVisible()
{
	int _time = tab->Video->Tell();
	bool toEnd = tab->Video->GetState() == Playing;
	return ((_time >= Edit->line->Start.mstime || toEnd) && _time < Edit->line->End.mstime);
}


void SubsGridBase::OnBackupTimer(wxTimerEvent &event)
{
	Kai->SetStatusText(_("Autozapis"), 0);
	wxString path;
	wxString ext = (subsFormat < SRT) ? L"ass" : (subsFormat == SRT) ? L"srt" : L"txt";

	path << Options.pathfull << L"\\Subs\\" << tab->SubsName.BeforeLast(L'.')
		<< L"_" << Notebook::GetTabs()->FindPanel(tab) << L"_" << numsave << L"." << ext;

	SaveFile(path, false);
	int maxFiles = Options.GetInt(AUTOSAVE_MAX_FILES);
	if (maxFiles > 1 && numsave >= maxFiles){ numsave = 0; }
	numsave++;
	makebackup = true;
	nullifyTimer.Start(5000, true);
}

void SubsGridBase::GetASSRes(int *x, int *y)
{
	const wxString &oldx = GetSInfo(L"PlayResX");
	const wxString &oldy = GetSInfo(L"PlayResY");
	int nx = wxAtoi(oldx);
	int ny = wxAtoi(oldy);
	bool changed = false;
	if (nx < 1 && ny < 1){
		nx = 384; ny = 288;
		AddSInfo(L"PlayResX", L"384");
		AddSInfo(L"PlayResY", L"288");
		changed = true;
	}
	else if (nx < 1){
		nx = (float)ny*(4.0 / 3.0); if (ny == 1024){ nx = 1280; }
		AddSInfo(L"PlayResX", std::to_wstring(nx));
		changed = true;
	}
	else if (ny < 1){
		ny = (float)nx*(3.0 / 4.0); if (nx == 1280){ ny = 1024; }
		AddSInfo(L"PlayResY", std::to_wstring(ny));
		changed = true;
	}
	*x = nx;
	*y = ny;
	//if(changed){SetModified(ASS_PROPERTIES, false, true, -1, false);}
}


void SubsGridBase::SaveSelections(bool clear)
{
	file->SaveSelections(clear, currentLine, markedLine, scrollPosition);
	savedSelections = true;
}

void SubsGridBase::GetCommonStyles(SubsGridBase *_grid, wxArrayString &styleTable)
{
	std::vector<Styles *> *styles1 = file->GetStyleTable();
	std::vector<Styles *> *styles2 = _grid->file->GetStyleTable();
	for (auto style1 : *styles1){
		for (auto style2 : *styles2){
			if (style1->Name == style2->Name){
				styleTable.Add(style1->Name); break;
			}
		}
	}
}

void SubsGridBase::SubsComparison()
{
	int comparisonType = Options.GetInt(SUBS_COMPARISON_TYPE);
	if (!comparisonType && compareStyles.size() < 1){ return; }
	bool compareByVisible = (comparisonType & COMPARE_BY_VISIBLE) != 0;
	bool compareByTimes = (comparisonType & COMPARE_BY_TIMES) != 0;
	bool compareByStyles = (comparisonType & COMPARE_BY_STYLES) != 0;
	bool compareByChosenStyles = compareStyles.size() > 0;
	bool compareBySelections = (comparisonType & COMPARE_BY_SELECTIONS) != 0;
	int firstSize = CG1->file->GetCount(), secondSize = CG2->file->GetCount();
	if (CG1->Comparison){ CG1->Comparison->clear(); }
	else{ CG1->Comparison = new std::vector<compareData>; }
	if (CG2->Comparison){ CG2->Comparison->clear(); }
	else{ CG2->Comparison = new std::vector<compareData>; }
	CG1->Comparison->resize(firstSize, compareData());
	CG2->Comparison->resize(secondSize, compareData());

	int lastJ = 0;

	for (int i = 0; i < firstSize; i++){

		int j = lastJ;
		Dialogue *dial1 = CG1->file->GetDialogue(i);
		if (compareByVisible && !dial1->isVisible){ continue; }
		while (j < secondSize){

			Dialogue *dial2 = CG2->file->GetDialogue(j);
			if (compareByVisible && !dial2->isVisible){ j++; continue; }

			if (compareByTimes && (dial1->Start != dial2->Start || dial1->End != dial2->End)){ j++; continue; }

			if (compareByStyles && dial1->Style != dial2->Style){ j++; continue; }

			if (compareByChosenStyles && (compareStyles.Index(dial1->Style) == -1 || dial1->Style != dial2->Style)){ j++; continue; }

			if (compareBySelections && (!CG1->file->IsSelected(i) || !CG2->file->IsSelected(j))){ j++; continue; }

			compareData & firstCompare = CG1->Comparison->at(i);
			compareData & secondCompare = CG2->Comparison->at(j);
			CompareTexts(firstCompare, secondCompare, (CG1->hasTLMode && dial1->TextTl != L"") ? dial1->TextTl : dial1->Text,
				(CG2->hasTLMode && dial2->TextTl != L"") ? dial2->TextTl : dial2->Text);
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


	size_t l1 = first.length(), l2 = second.length();
	size_t sz = (l1 + 1) * (l2 + 1) * sizeof(size_t);
	size_t w = l2 + 1;
	size_t* dpt;
	size_t i1, i2;
	dpt = new size_t[sz];

	if (dpt == NULL)
	{
		KaiLog(L"memory allocation failed");
		return;
	}

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

size_t SubsGridBase::GetKeyFromScrollPos(int numOfLines)
{
	if (numOfLines < 0){
		int visibleLines = 0;
		for (size_t i = scrollPosition; i + 1 > 0; i--){
			if (numOfLines == visibleLines)
				return i;

			if (*GetDialogue(i)->isVisible)
				visibleLines--;
		}
		return 0;
	}

	size_t visibleLines = 0;
	for (size_t i = scrollPosition; i < GetCount(); i++){
		if (numOfLines == visibleLines)
			return i;

		if (*GetDialogue(i)->isVisible)
			visibleLines++;
	}

	return GetCount();
}


size_t SubsGridBase::GetKeyFromPosition(size_t position, int delta, bool safe /*= true*/)
{
	if (position > GetCount())
		return (safe) ? 0 : -1;

	int visibleLines = 0;
	if (delta > 0){
		size_t i = position + 1;
		while (i < GetCount()){

			if (*GetDialogue(i)->isVisible)
				visibleLines++;
			
			if (delta == visibleLines)
				return i;

			i++;

		}
		return (safe) ? GetKeyFromPosition(GetCount(), -1) : -1;
	}
	else if (delta < 0 && position > 0){
		size_t i = position - 1;
		while (i + 1 > 0){
			if (*GetDialogue(i)->isVisible)
				visibleLines--;

			if (delta == visibleLines)
				return i;

			i--;
		}
		return (safe) ? file->GetElementById(0) : -1;
	}
	return position;
}


SubsGrid* SubsGridBase::CG1 = NULL;
SubsGrid* SubsGridBase::CG2 = NULL;
bool SubsGridBase::hasCompare = false;
wxArrayString SubsGridBase::compareStyles = wxArrayString();