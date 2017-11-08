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

#include "SubsGridFiltering.h"
#include "SubsGrid.h"

SubsGridFiltering::SubsGridFiltering(SubsGrid *_grid, int _activeLine)
	:grid(_grid)
	, Invert(false)
	, activeLine(_activeLine)
{
}

SubsGridFiltering::~SubsGridFiltering()
{
}

void SubsGridFiltering::Filter()
{
	grid->hasHiddenLinesAtStart = false;
	Invert = Options.GetBool(GridFilterInverted);
	int filterBy = Options.GetInt(GridFilterBy);
	switch (filterBy)
	{
	case 0:
		TurnOffFiltering(); break;
	case 1:
		FilterByDoubtful(); break;
	case 2:
		FilterByUntranslated(); break;
	case 3:
		FilterBySelections(); break;
	case 4:
	{
		wxArrayString styles;
		Options.GetTable(GridFilterStyles, styles, ";");
		FilterByStyles(styles);
		break;
	}
	default:
		break;
	}
	FilteringFinalize();
}

void SubsGridFiltering::FilterPartial(int from, bool hide)
{
	File *Subs = grid->file->GetSubs();
	Dialogue *lastDial = NULL;
	bool hasHiddenLinesAtStart = (from == 0 && grid->hasHiddenLinesAtStart);
	int keyFrom = (hasHiddenLinesAtStart)? 0 : grid->file->GetElementById(from);
	int keyTo = keyFrom;
	
	for (int i = keyFrom; i < Subs->dials.size(); i++){
		Dialogue *dial = Subs->dials[i];
		if (!dial->NonDialogue){ 
			if (lastDial && (dial->isVisible == VISIBLE || dial->isVisible == VISIBLE_HIDDEN_BLOCK)){
				keyTo = i - 1; 
				if (!hide){ lastDial->isVisible = VISIBLE_END_BLOCK; }
				break;
			}
			dial->isVisible = (hide && (i == keyFrom) && !hasHiddenLinesAtStart) ? VISIBLE_HIDDEN_BLOCK :
				(!hide && i == keyFrom && !hasHiddenLinesAtStart) ? VISIBLE_START_BLOCK :
				!hide ? VISIBLE_BLOCK :
				NOT_VISIBLE;
			if (i != keyFrom || hasHiddenLinesAtStart){
				if (hide && i <= activeLine){
					activeLineDiff--;
				}
				else if (!hide && i <= activeLine){
					activeLineDiff++;
				}
			}
		}
		lastDial = dial;
	}
	if (keyFrom == keyTo){ 
		keyTo = Subs->dials.size() - 1; 
		if (!hide)
			Subs->dials[keyTo]->isVisible = VISIBLE_END_BLOCK;
		wxLogStatus("Something went wrong with partially hiding it is better to check it for potencial bugs."); 
	}
	grid->file->ReloadVisibleDialogues(keyFrom, keyTo);
	grid->RefreshSubsOnVideo(activeLine + activeLineDiff);
	grid->RefreshColumns();
}

void SubsGridFiltering::FilterByDoubtful()
{
	File *Subs = grid->file->GetSubs();
	Dialogue *lastDial = NULL;
	bool BeginBlock = false;
	for (int i = 0; i < Subs->dials.size(); i++){
		Dialogue *dial = Subs->dials[i];
		if (dial->NonDialogue) continue;
		int hasDoubtful = (dial->State & 4);
		if (hasDoubtful && !Invert || !hasDoubtful && Invert){
			if (!BeginBlock){
				BeginBlock = true;
				if (lastDial){
					lastDial->isVisible = VISIBLE_HIDDEN_BLOCK;
				}
				else{ grid->hasHiddenLinesAtStart = true; }
			}
			if (dial->isVisible && i <= activeLine){ activeLineDiff--; }
			dial->isVisible = NOT_VISIBLE;
		}
		else{
			if (BeginBlock){
				BeginBlock = false;
			}
			if (!dial->isVisible && i <= activeLine){ activeLineDiff++; }
			dial->isVisible = VISIBLE;
		}
		lastDial = dial;
	}
}

void SubsGridFiltering::FilterByUntranslated()
{
	File *Subs = grid->file->GetSubs();
	Dialogue *lastDial = NULL;
	bool BeginBlock = false;
	for (int i = 0; i < Subs->dials.size(); i++){
		Dialogue *dial = Subs->dials[i];
		if (dial->NonDialogue) continue;
		bool hasUntranslated = dial->TextTl.empty();
		if (hasUntranslated && !Invert || !hasUntranslated && Invert){
			if (!BeginBlock){
				BeginBlock = true;
				if (lastDial){
					lastDial->isVisible = VISIBLE_HIDDEN_BLOCK;
				}
				else{ grid->hasHiddenLinesAtStart = true; }
			}
			if (dial->isVisible && i <= activeLine){ activeLineDiff--; }
			dial->isVisible = NOT_VISIBLE;
		}
		else{
			if (BeginBlock){
				BeginBlock = false;
			}
			if (!dial->isVisible && i <= activeLine){ activeLineDiff++; }
			dial->isVisible = VISIBLE;
		}
		lastDial = dial;
	}
}
inline bool FindStyle(const wxArrayString &styles, const wxString &dialStyle){
	for (auto style : styles){ 
		if (style == dialStyle) {
			return true; 
		}
	}
	return false;
}

void SubsGridFiltering::FilterByStyles(wxArrayString &styles)
{
	File *Subs = grid->file->GetSubs();
	Dialogue *lastDial = NULL;
	bool BeginBlock = false;
	for (int i = 0; i < Subs->dials.size(); i++){
		Dialogue *dial = Subs->dials[i];
		if (dial->NonDialogue) continue;
		bool hasStyle = FindStyle(styles, dial->Style);
		if (hasStyle && !Invert || !hasStyle && Invert){
			if (!BeginBlock){
				BeginBlock = true;
				if (lastDial){
					lastDial->isVisible = VISIBLE_HIDDEN_BLOCK;
				}
				else{ grid->hasHiddenLinesAtStart = true; }
			}
			if (dial->isVisible && i <= activeLine){ activeLineDiff--; }
			dial->isVisible = NOT_VISIBLE;
		}
		else{
			if (BeginBlock){
				BeginBlock = false;
			}
			if (!dial->isVisible && i <= activeLine){ activeLineDiff++; }
			dial->isVisible = VISIBLE;
		}
		lastDial = dial;
	}
}

void SubsGridFiltering::FilterBySelections(bool addToFiltering)
{
	File *Subs = grid->file->GetSubs();
	wxArrayInt sels = grid->GetSelectionsKeys();
	int selssize = sels.size();
	Dialogue *lastDial = NULL;
	bool BeginBlock = false;
	int j = 0;
	for (int i = 0; i < Subs->dials.size(); i++){
		Dialogue *dial = Subs->dials[i];
		if (dial->NonDialogue) continue;
		bool isSelected = false;
		if (j < selssize){ isSelected = sels[j] == i; if (isSelected){ j++; } }
		if (isSelected && !Invert || !isSelected && Invert){
			if (!BeginBlock){
				BeginBlock = true;
				if (lastDial){
					lastDial->isVisible = VISIBLE_HIDDEN_BLOCK;
				}
				else{ grid->hasHiddenLinesAtStart = true; }
			}
			if (dial->isVisible && i <= activeLine){ activeLineDiff--; }
			dial->isVisible = NOT_VISIBLE; 
		}
		else{
			if (BeginBlock){ 
				BeginBlock = false; 
			}
			if (!addToFiltering){
				if (!dial->isVisible && i <= activeLine){ activeLineDiff++; }
				dial->isVisible = VISIBLE;
			}
		}
		lastDial = dial;
	}
	if (addToFiltering){ FilteringFinalize(); }
}

void SubsGridFiltering::TurnOffFiltering()
{
	int keyActiveLine = grid->file->GetElementById(activeLine);
	File *Subs = grid->file->GetSubs();
	int i = 0;
	for (auto dial : Subs->dials){
		if (dial->isVisible != 1 && !dial->NonDialogue){ 
			if (i <= keyActiveLine && dial->isVisible < 1){ activeLineDiff++; }
			dial->isVisible = VISIBLE; 
		}
		i++;
	}
}

void SubsGridFiltering::FilteringFinalize()
{
	grid->file->ReloadVisibleDialogues();
	grid->RefreshSubsOnVideo(activeLine + activeLineDiff);
	grid->RefreshColumns();
}