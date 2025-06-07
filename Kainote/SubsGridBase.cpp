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



#include "SubsFile.h"
#include "SubsGridBase.h"
#include "SubsGridPreview.h"
#include "SubsLoader.h"
#include "SubsGridFiltering.h"
#include "config.h"
#include "OpennWrite.h"
#include "OptionsDialog.h"
#include "AudioBox.h"
#include "KaiMessageBox.h"
#include "stylestore.h"
#include "Toolbar.h"
#include "TabPanel.h"
#include "ShiftTimes.h"
#include "VisualDrawingShapes.h"
#include "VisualClips.h"
#include "Visuals.h"
#include "SubtitlesProviderManager.h"
#include <algorithm>
#include <wx/tokenzr.h>
#include <wx/event.h>
#include <wx/regex.h>
#include <wx/ffile.h>
#include <wx/window.h>
#include <locale>

SubsGridBase* SubsGridBase::CG1 = NULL;
SubsGridBase* SubsGridBase::CG2 = NULL;
bool SubsGridBase::hasCompare = false;
wxArrayString SubsGridBase::compareStyles = wxArrayString();


bool sortstart(Dialogue *i, Dialogue *j)
{
	if (i->Start.mstime != j->Start.mstime){
		return (i->Start.mstime < j->Start.mstime);
	}
	return i->End.mstime < j->End.mstime;
}

bool sortend(Dialogue *i, Dialogue *j)
{
	if (i->End.mstime != j->End.mstime){
		return (i->End.mstime < j->End.mstime);
	}
	return i->Start.mstime < j->Start.mstime;
}

bool sortstyle(Dialogue *i, Dialogue *j)
{
	if (i->Style != j->Style){
		const std::collate<wchar_t>& f = std::use_facet<std::collate<wchar_t>>(KainoteFrame::GetLocale());
		const wchar_t* s1 = (&i->Style).wc_str();
		const wchar_t* s2 = (&j->Style).wc_str();
		return (f.compare(&s1[0], &s1[0] + wcslen(s1),
			&s2[0], &s2[0] + wcslen(s2)) < 0);

	}
	return i->Start.mstime < j->Start.mstime;
}

bool sortactor(Dialogue *i, Dialogue *j)
{
	if (i->Actor != j->Actor){
		const std::collate<wchar_t>& f = std::use_facet<std::collate<wchar_t>>(KainoteFrame::GetLocale());
		const wchar_t* s1 = (&i->Actor).wc_str();
		const wchar_t* s2 = (&j->Actor).wc_str();
		return (f.compare(&s1[0], &s1[0] + wcslen(s1),
			&s2[0], &s2[0] + wcslen(s2)) < 0);

	}
	return i->Start.mstime < j->Start.mstime;
}

bool sorteffect(Dialogue *i, Dialogue *j)
{
	if (i->Effect != j->Effect){
		const std::collate<wchar_t>& f = std::use_facet<std::collate<wchar_t>>(KainoteFrame::GetLocale());
		const wchar_t* s1 = (&i->Actor).wc_str();
		const wchar_t* s2 = (&j->Actor).wc_str();
		return (f.compare(&s1[0], &s1[0] + wcslen(s1),
			&s2[0], &s2[0] + wcslen(s2)) < 0);

	}
	return i->Start.mstime < j->Start.mstime;
}

bool sortlayer(Dialogue *i, Dialogue *j)
{
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
	Comparison = nullptr;
	numsave = 0;

	LoadDefault();
	timer.SetOwner(this, ID_AUTIMER);
	//reset autosave on statusbar
	nullifyTimer.SetOwner(this, 27890);
	Bind(wxEVT_TIMER, [=](wxTimerEvent &evt){
		Kai->SetStatusText(emptyString, 0);
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
		OptionsDialog od(Kai);
		od.OptionsTree->ChangeSelection(1);
		od.okok->SetFocus();
		if (od.ShowModal() == wxID_CANCEL){
			return; 
		}
	}
	if (Options.GetBool(CONVERT_FPS_FROM_VIDEO) && tab->VideoPath != emptyString){
		Options.SetString(CONVERT_FPS, Kai->GetStatusText(4).BeforeFirst(L' '));
	}
	if (Options.GetFloat(CONVERT_FPS) < 1){ KaiMessageBox(_("Nieprawidłowy FPS. Popraw opcje i spróbuj ponownie.")); return; }

	bool newendtimes = Options.GetBool(CONVERT_NEW_END_TIMES);
	const wxString & stname = Options.GetString(CONVERT_STYLE);
	int endt = Options.GetInt(CONVERT_TIME_PER_CHARACTER);
	const wxString & prefix = Options.GetString(CONVERT_ASS_TAGS_TO_INSERT_IN_LINE);

	size_t i = 0;
	Dialogue *lastDialc = nullptr;
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
		if (resx == emptyString){ resx = L"1280"; }
		if (resy == emptyString){ resx = L"720"; }
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
		edit->RefreshStyle();
	}
	if (subsFormat == ASS){
		file->SortAll([](Dialogue *i, Dialogue *j){
			if (i->Start.mstime != j->Start.mstime){
				return (i->Start.mstime < j->Start.mstime);
			}
			if (i->End.mstime != j->End.mstime){
				return (i->End.mstime < j->End.mstime);
			}
			const std::collate<wchar_t>& f = std::use_facet<std::collate<wchar_t>>(KainoteFrame::GetLocale());
			const wchar_t* s1 = (&i->Text).wc_str();
			const wchar_t* s2 = (&j->Text).wc_str();
			return (f.compare(&s1[0], &s1[0] + wcslen(s1),
				&s2[0], &s2[0] + wcslen(s2)) < 0);
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
			else if (actualDialogue->Text == emptyString){
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
	edit->SetLine((currentLine < GetCount()) ? currentLine : 0);
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
		edit->Send(EDITBOX_LINE_EDITION, false, dummyEditboxChanges, true);
	}
	wxString txt;
	const wxString &tlmode = GetSInfo(L"TLMode");
	bool translated = tlmode == L"Translated";
	bool tlmodeOn = tlmode != emptyString;

	OpenWrite ow(filename, true);

	if (subsFormat < SRT){
		//if (cstat){
			//AddSInfo(L"Last Style Storage", Options.actualStyleDir, false);
			AddSInfo(L"Active Line", std::to_wstring(currentLine), false);
			wxString subsPath = tab->SubsPath.BeforeLast(L'\\');
			if (edit->ABox){
				wxString path = (edit->ABox->audioName.StartsWith(subsPath) && normalSave) ?
					edit->ABox->audioName.AfterLast(L'\\') : edit->ABox->audioName;
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
			//when i == editbox line get the last changes
			if (i == currentLine){ dial = edit->line; };

			if (tlmodeOn){
				bool hasTextTl = dial->TextTl != emptyString;
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
					raw << int(i + 1) << L"\r\n";
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
				bool hasTextTl = dial->TextTl != emptyString;
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
					raw << int(i + 1) << L"\r\n";
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
	bool operator()(Dialogue *i, Dialogue *j) const{

		if (i->Start.mstime != j->Start.mstime){
			return (i->Start.mstime < j->Start.mstime);
		}
		return i->End.mstime < j->End.mstime;
	} // returns x>y
};

void SubsGridBase::ChangeTimes(bool byFrame)
{
	Provider *FFMS2 = tab->video->GetFFMS2();
	if (byFrame && !FFMS2){ 
		KaiLog(_("Wideo nie zostało wczytane przez FFMS2")); return; }
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

	if ((moveTimeOptions & 4) && tab->video->GetState() != None){
		if (byFrame){
			frame += tab->video->GetCurrentFrame() - FFMS2->GetFramefromMS(difftime);
		}
		else{
			int addedTimes = tab->video->GetFrameTime(VAS != 0) - difftime;
			if (addedTimes < 0){ addedTimes -= 10; }
			time += ZEROIT(addedTimes);
		}

	}
	else if ((moveTimeOptions & 8) && edit->ABox && edit->ABox->audioDisplay->hasMark){
		if (byFrame){
			frame += FFMS2->GetFramefromMS(edit->ABox->audioDisplay->curMarkMS - difftime);
		}
		else{
			int addedTimes = edit->ABox->audioDisplay->curMarkMS - difftime;
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
				tab->video->GetStartEndDelay(dialc->Start.mstime, dialc->End.mstime, &startTrimed, &endTrimed);
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
					dialc->Start.NewTime(ZEROIT(tab->video->GetFrameTimeFromFrame(startFrame)));
				}
				if (whichTimes != 1){
					int endFrame = FFMS2->GetFramefromMS(dialc->End.mstime) + frame;
					dialc->End.NewTime(ZEROIT(tab->video->GetFrameTimeFromFrame(endFrame)));
				}
				dialc->ChangeDialogueState(1);
			}
			if (changeTagTimes){
				int newStartTrimed = 0, newEndTrimed = 0;
				tab->video->GetStartEndDelay(dialc->Start.mstime, dialc->End.mstime, &newStartTrimed, &newEndTrimed);
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

	// add to table also lines with times before and after lines in table
	// maybe it's not needed?
	if (correctEndTimes > 0 || PostprocessorOptions > 16){
		bool hasend = false;
		int newstarttime = -1;
		int endt = Options.GetInt(CONVERT_TIME_PER_CHARACTER);
		bool isPreviousEndGreater = false;
		bool isEndGreater = false;
		bool previousIsKeyFrame = true;
		bool isPreviousEndEdited = false;
		if (!FFMS2) {
			KaiLog(_("Wideo nie zostało wczytane przez FFMS2")); 
			return;
		}

		const wxArrayInt& keyFrames = FFMS2->GetKeyframes();
		wxArrayInt keyFramesStart;
		for (size_t g = 0; g < keyFrames.Count(); g++) {
			int keyMS = keyFrames[g];
			keyFramesStart.Add(ZEROIT(tab->video->GetFrameTimeFromTime(keyMS)));
		}
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
				//no need to correct
				numOfStartModifications++;
			}
			if (PostprocessorOptions & 2) {
				dialc->End.Change(LeadOut);
				//no need to correct
				numOfEndModifications++;
			}

			//Keyframes goes first and rest cannot apply when keyframe was set
			if (PostprocessorOptions & 8){
				int startRange = dialc->Start.mstime - KeyframeBeforeStart;
				int startRange1 = oldStart + KeyframeAfterStart;
				int endRange = oldEnd - KeyframeBeforeEnd;
				int endRange1 = dialc->End.mstime + KeyframeAfterEnd;
				
				if (dialc->GetTextNoCopy() == "I dorzucić na kupę złomu!")
				{
					bool costam = false;
				}
				
				int startResult = INT_MAX;
				int endResult = -1;
				//it uses only keyframes move to start time (- fpstime / 2)
				for (size_t g = 0; g < keyFramesStart.Count(); g++) {
					int keyMSS = keyFramesStart[g];
					if (keyMSS >= startRange && keyMSS <= startRange1) {
						if (oldStart == keyMSS) {
							startRange = -1; startRange1 = -1; startResult = INT_MAX;
							dialc->Start.NewTime(oldStart);
							numOfStartModifications--;
						}
						if (startResult > keyMSS/* && keyMS != dialc->Start.mstime*/){
							if (startResult == INT_MAX || abs(startResult - dialc->Start.mstime) > abs(keyMSS - dialc->Start.mstime))
								startResult = keyMSS;
						}
					}
					if (keyMSS >= endRange && keyMSS <= endRange1) {
						if (oldEnd == keyMSS) {
							endRange = -1; endRange1 = -1; endResult = -1;
							dialc->End.NewTime(oldEnd);
							numOfEndModifications--;
						}
						if (endResult < keyMSS && keyMSS > dialc->Start.mstime) {
							if (endResult == -1 || abs(endResult - dialc->End.mstime) > abs(keyMSS - dialc->End.mstime))
								endResult = keyMSS;
						}
					}
				}
				//here is main problem we do not know if next start will be changed
				//and it makes mess here but changing only start should be enough
				//startResult >= compareStart and endResult <= compareEnd should be changed or even remove
				
				if (startResult != INT_MAX){
					int checkEnd = endResult != -1 ? endResult : dialc->End.mstime;

					if (dialc->Start != startResult && (checkEnd - startResult > 600)){
						dialc->Start.NewTime(startResult);
						numOfStartModifications++;
					}
					foundStartKeyframe = true;
				}
				if (endResult != -1){
					if (dialc->End != endResult && (endResult - dialc->Start.mstime > 600)){
						dialc->End.NewTime(endResult);
						numOfEndModifications++;
					}
					foundEndKeyframe = true;
				}
				//make sure that start is greater or even then end time of previous line
				//and correct times but only when previous line end time was not greater than start time on start
				//then discard correction.
				if (dialc->Start.mstime < previousEnd && !isPreviousEndGreater && previousEnd < oldEnd && isPreviousEndEdited) {
					it->first->End.NewTime(dialc->Start.mstime);
					//numOfStartModifications--;
					//foundStartKeyframe = false;
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
			isPreviousEndEdited = numOfEndModifications > 0;
			dialc->ClearParse();
		}

	}

	SpellErrors.clear();
	int tmpMarked = markedLine;
	SetModified(SHIFT_TIMES, true, false, -1, false);
	markedLine = tmpMarked;
	if (subsFormat > TMP){ RefreshColumns(START | END); }
	else{ Refresh(false); }
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
	edit->SetLine(firstSelection);
	ScrollTo(firstSelection, true);
	Refresh(false);
	return true;
}

void SubsGridBase::DeleteText()
{
	wxArrayInt sels;
	file->GetSelections(sels);
	for (auto i : sels){
		CopyDialogue(i)->Text = emptyString;
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
	Kai->Menubar->Enable(GLOBAL_SAVE_SUBS, true);
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

void SubsGridBase::DoUndo(bool redo, int iter)
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
		tab->shiftTimes->Contents();
		tab->edit->HideControls();
		Kai->UpdateToolbar();
		if (oldformat == ASS || subsFormat == ASS){
			tab->video->DisableVisuals(subsFormat != ASS);
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
		edit->SetTlMode(hasTLMode);
	}
	if (Comparison){
		SubsComparison();
	}

	int corrected = -1;
	edit->SetLine(file->FindVisibleKey(file->GetActiveLine(), &corrected));
	markedLine = file->FindVisibleKey(file->GetMarkerLine());
	scrollPosition = file->FindVisibleKey(file->GetScrollPosition());
	scrollPositionId = file->GetElementByKey(scrollPosition);
	if (corrected >= 0){
		file->EraseSelection(file->GetActiveLine());
		file->InsertSelection(corrected);
	}

	RefreshColumns();
	edit->RefreshStyle();
	VideoBox *vb = tab->video;

	const wxString &newResolution = GetSInfo(L"PlayResX") + L" x " + GetSInfo(L"PlayResY");
	if (resolution != newResolution){
		Kai->SetSubsResolution();
		vb->ChangeOnScreenResolution(tab);
	}
	const wxString &newmatrix = GetSInfo(L"YCbCr Matrix");
	if (matrix != newmatrix){
		vb->SetColorSpace(newmatrix);
	}

	if (edit->Visual < CHANGEPOS){

		if (vb->IsShown() || vb->IsFullScreen()){ vb->OpenSubs(OPEN_DUMMY); }
		int seekAfter = 0;
		vb->GetVideoListsOptions(nullptr, &seekAfter);
		if (seekAfter > 1){
			if (vb->GetState() == Paused || (vb->GetState() == Playing && (seekAfter == 3 || seekAfter == 5))){
				vb->Seek(edit->line->Start.mstime);
			}
		}
		else{
			if (vb->GetState() == Paused){ vb->Render(); }
		}
	}
	else if (edit->Visual == CHANGEPOS){
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

	edit->SetLine(currentLine, false, false);
	RefreshColumns();
	UpdateUR();
	Kai->Label(file->GetActualHistoryIter());
	VideoBox *vb = tab->video;
	if (vb->GetState() != None){
		vb->OpenSubs(OPEN_DUMMY);
		vb->Render();
	}
}

size_t SubsGridBase::FirstSelection(size_t *firstSelectionId /*= nullptr*/)
{
	return file->FirstSelection();
}

// Warning for adding to destroy
// no adding makes memory leaks
// two addings makes crash when object is destroyed.
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

// Warning for adding to destroy
// no adding makes memory leaks
// two addings makes crash when object is destroyed.
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
			edit->SetLine(newCurrentLine);
			file->InsertSelection(newCurrentLine);
		}
		file->SaveUndo(editionType, currentLine, markedLine);
		Kai->Label(file->GetActualHistoryIter(), false, Kai->Tabs->FindPanel(tab));
		if (!dummy){
			VideoBox *vb = tab->video;
			if (edit->Visual >= CHANGEPOS){
				vb->SetVisual(true);
			}
			else{
				if (vb->IsShown() || vb->IsFullScreen()){ vb->OpenSubs(OPEN_DUMMY); }

				int seekAfter;
				vb->GetVideoListsOptions(nullptr, &seekAfter);
				if (seekAfter > 1){
					if (vb->GetState() == Paused || (vb->GetState() == Playing && (seekAfter == 3 || seekAfter == 5))){
						vb->Seek(edit->line->Start.mstime);
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
		edit->SetTlMode(hasTLMode);
		Kai->Menubar->Enable(GLOBAL_SAVE_TRANSLATION, hasTLMode);
	}
	if (hasTLMode && (GetSInfo(L"TLMode Showtl") == L"Yes" || Options.GetBool(TL_MODE_SHOW_ORIGINAL))){ 
		showOriginal = true; 
	}


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
		edit->TlMode->Enable(true); edit->RefreshStyle();
		if (Options.GetBool(GRID_LOAD_SORTED_SUBS)){
			file->SortAll(sortstart);
		}
		active = wxAtoi(GetSInfo(L"Active Line"));
		if (active >= GetCount()){ active = 0; }
	}
	else{ edit->TlMode->Enable(false); }


	file->InsertSelection(active);
	lastRow = active;
	markedLine = active;

	file->EndLoad(OPEN_SUBTITLES, active);

	RefreshColumns();
	//it's faster to change load audio to true than setting audio from kainoteMain 
	//to avoid not changed line when loaded only subtitles.
	edit->SetLine(active, true, false);

	edit->HideControls();

	if (StyleStore::HasStore() && subsFormat == ASS){ StyleStore::Get()->LoadAssStyles(); }
	if (subsFormat == ASS){
		int filterBy = Options.GetInt(GRID_FILTER_BY);
		if (filterBy && Options.GetBool(GRID_FILTER_AFTER_LOAD) && 
			filterBy != FILTER_BY_SELECTIONS){
			isFiltered = true;
			SubsGridFiltering filter((SubsGrid*)this, currentLine);
			filter.Filter(true);
		}
		edit->RebuildActorEffectLists();
	}
	((SubsGridWindow*)this)->ScrollTo(active, false, -4);
}

void SubsGridBase::SetStartTime(int stime)
{
	edit->Send(EDITBOX_LINE_EDITION, false, false, true);
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
	edit->Send(EDITBOX_LINE_EDITION, false, false, true);
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
		if (GetSInfo(L"TLMode") == emptyString){
			
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
			_("Potwierdzenie"), wxYES_NO, nullptr, wxDefaultPosition, wxNO) == wxNO){
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
			Dialogue *dialc = nullptr;
			if (dial->TextTl != emptyString)
			{
				dialc = file->CopyDialogue(i);
				dialc->Text = dialc->TextTl;
				dialc->TextTl = emptyString;
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
	edit->RefreshStyle();
	SpellErrors.clear();
	Refresh(false);
	SetModified((mode) ? GRID_TURN_ON_TLMODE : GRID_TURN_OFF_TLMODE);
	return false;
}

//this method should have different name
//it also can change line to previous
void SubsGridBase::NextLine(int direction)
{
	if (edit->ABox && edit->ABox->audioDisplay->hold != 0){ return; }
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
		tmp->Text = emptyString;
		tmp->TextTl = emptyString;
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
	edit->SetLine(newCurrentLine, true, true, false, true);
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
	return nullptr;
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
	return nullptr;
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


//this function is called from another thread
//need to guard every change in dialogues, styles, sinfos, and editbox->line
wxString *SubsGridBase::GetVisible(bool *visible, wxPoint *point, wxArrayInt *selected, bool allSubs)
{
	wxMutexLocker lock(editionMutex);
	bool showOriginalOnVideo = !Options.GetBool(TL_MODE_HIDE_ORIGINAL_ON_VIDEO);
	int _time = tab->video->Tell();
	bool toEnd = tab->video->GetState() == Playing;
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
	edit->Send(EDITBOX_LINE_EDITION, false, true);
	if ((_time >= edit->line->Start.mstime || toEnd) && _time < edit->line->End.mstime){
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
			dial = edit->line;
		}
		if (selected && file->IsSelected(i)){
			selected->Add(txt->length());
			continue;
		}
		if ((toEnd && _time <= dial->Start.mstime) || (_time >= dial->Start.mstime && _time < dial->End.mstime) || allSubs){
			if (isTlmode && dial->TextTl != emptyString){
				if (showOriginalOnVideo)
					dial->GetRaw(txt, false, tlStyle);

				dial->GetRaw(txt, true);
			}
			else if(dial->Text != emptyString){
				if (subsFormat == SRT){
					(*txt) << j << L"\r\n";
					j++;
				}
				dial->GetRaw(txt);
			}
			if (point && i == currentLine){
				
				int len = (isTlmode && dial->TextTl != emptyString) ?
					dial->TextTl.Len() : dial->Text.Len();
				if (!len) {
					dial->GetRaw(txt);
				}
				int all = txt->length();
				point->x = all - 2;
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
			return nullptr;
		}
	}

	return txt;
}

bool SubsGridBase::IsLineVisible()
{
	int _time = tab->video->Tell();
	bool toEnd = tab->video->GetState() == Playing;
	return ((_time >= edit->line->Start.mstime || toEnd) && _time < edit->line->End.mstime);
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
	SubsGridBase* CCG1 = CG1;
	SubsGridBase* CCG2 = CG2;

	int firstSize = CG1->file->GetCount(), 
		secondSize = CG2->file->GetCount();
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

			compareData & firstCompare = CCG1->Comparison->at(i);
			compareData & secondCompare = CCG2->Comparison->at(j);
			CompareTexts(firstCompare, secondCompare, 
				(CCG1->hasTLMode && dial1->TextTl != emptyString) ? dial1->TextTl : dial1->Text,
				(CCG2->hasTLMode && dial2->TextTl != emptyString) ? dial2->TextTl : dial2->Text);
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

	if (dpt == nullptr)
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
			CG1->Comparison = nullptr;
			CG1->Refresh(false);
		}
		if (CG2){
			delete CG2->Comparison;
			CG2->Comparison = nullptr;
			CG2->Refresh(false);
		}
		CG1 = nullptr;
		CG2 = nullptr;
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




