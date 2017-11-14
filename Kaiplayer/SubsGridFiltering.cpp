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
	Invert = Options.GetBool(GridFilterInverted);
	filterBy = Options.GetInt(GridFilterBy);
	if (filterBy & FILTER_BY_STYLES){
		Options.GetTable(GridFilterStyles, styles, ";");
	}
	if (filterBy & FILTER_BY_SELECTIONS){
		grid->GetSelectionsKeys(keySelections);
	}

	File *Subs = grid->file->GetSubs();
	for (int i = 0; i < Subs->dials.size(); i++){
		Dialogue *dial = Subs->dials[i];
		if (dial->NonDialogue) continue;
		bool hideDialogue = CheckHiding(dial, i);
		if (hideDialogue && !Invert || !hideDialogue && Invert){
			if (*dial->isVisible && i <= activeLine){ activeLineDiff--; }
			dial->isVisible = NOT_VISIBLE;
		}
		else{
			if (!dial->isVisible && i <= activeLine){ activeLineDiff++; }
			dial->isVisible = VISIBLE;
		}
	}
	
	FilteringFinalize();
}

void SubsGridFiltering::FilterPartial(int from)
{
	File *Subs = grid->file->GetSubs();
	Dialogue *lastDial = NULL;
	int keyFrom = (from < 0) ? 0 : grid->file->GetElementById(from)+1;
	int keyTo = keyFrom;
	bool hide = true;
	bool changed = false;
	
	for (int i = keyFrom; i < Subs->dials.size(); i++){
		Dialogue *dial = Subs->dials[i];
		if (!dial->NonDialogue){
			if (lastDial && dial->isVisible == VISIBLE){
				keyTo = i - 1;
				changed = true;
				break;
			}
			if (!dial->isVisible){ hide = false; }
			dial->isVisible = !hide ? VISIBLE_BLOCK : NOT_VISIBLE;
			if (hide && i <= activeLine){
				activeLineDiff--;
			}
			else if (!hide && i <= activeLine){
				activeLineDiff++;
			}
			lastDial = dial;
		}
	}
	if (!changed){
		keyTo = Subs->dials.size() - 1; 
		wxLogStatus("Something went wrong with partially hiding it is better to check it for potencial bugs."); 
	}
	grid->file->ReloadVisibleDialogues(keyFrom, keyTo);
	grid->RefreshSubsOnVideo(activeLine + activeLineDiff, false);
	grid->RefreshColumns();
}

void SubsGridFiltering::HideSelections()
{
	File *Subs = grid->file->GetSubs();
	grid->GetSelectionsKeys(keySelections);
	int selssize = keySelections.size();
	int j = 0;
	for (int i = 0; i < Subs->dials.size(); i++){
		Dialogue *dial = Subs->dials[i];
		if (dial->NonDialogue) continue;
		bool isSelected = false;
		if (j < selssize){ isSelected = keySelections[j] == i; if (isSelected){ j++; } }
		if (isSelected && !Invert || !isSelected && Invert){
			if (*dial->isVisible && i <= activeLine){ activeLineDiff--; }
			dial->isVisible = NOT_VISIBLE; 
		}
	}
	FilteringFinalize(); 
}
void SubsGridFiltering::RemoveFiltering()
{
	TurnOffFiltering();
	FilteringFinalize();
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

inline bool SubsGridFiltering::CheckHiding(Dialogue *dial, int i)
{
	int result = filterBy;
	if (filterBy & FILTER_BY_SELECTIONS && selectionsJ < keySelections.size() && keySelections[selectionsJ] == i){
		selectionsJ++;
		result ^= FILTER_BY_SELECTIONS;
	}
	if (filterBy & FILTER_BY_STYLES){
		for (auto style : styles){
			if (style == dial->Style) {
				result ^= FILTER_BY_STYLES; break;
			}
		}
	}
	if (filterBy & FILTER_BY_DIALOGUES && !dial->IsComment){
		result ^= FILTER_BY_DIALOGUES;
	}
	if (filterBy & FILTER_BY_DOUBTFUL && (dial->State & 4)){
		result ^= FILTER_BY_DOUBTFUL;
		if (filterBy &FILTER_BY_UNTRANSLATED) 
			result ^= FILTER_BY_UNTRANSLATED;
	}
	if (filterBy & FILTER_BY_UNTRANSLATED && dial->TextTl.empty()){
		result ^= FILTER_BY_UNTRANSLATED;
		if (filterBy &FILTER_BY_DOUBTFUL)
			result ^= FILTER_BY_DOUBTFUL;
	}
	return result != 0;
}