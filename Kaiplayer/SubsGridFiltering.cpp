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

void SubsGridFiltering::Filter(bool autoFiltering, bool removeFiltering)
{
	Invert = Options.GetBool(GRID_FILTER_INVERTED);
	filterBy = Options.GetInt(GRID_FILTER_BY);
	bool addToFilter = Options.GetBool(GRID_ADD_TO_FILTER);
	if (removeFiltering){
		if (grid->isFiltered){
			TurnOffFiltering();
			FilteringFinalize();
			grid->isFiltered = false;
		}
		return;
	}
	if (filterBy & FILTER_BY_STYLES){
		Options.GetTable(GRID_FILTER_STYLES, styles);

		size_t i = 0;
		while (i < styles.size()){
			if (grid->FindStyle(styles[i]) == -1){
				styles.RemoveAt(i);
				continue;
			}
			i++;
		}
		if (styles.size() < 1){
			grid->isFiltered = false;
			if (filterBy == FILTER_BY_STYLES)
				return;
			else
				filterBy ^= FILTER_BY_STYLES;
		}

	}
	if (filterBy & FILTER_BY_SELECTIONS){
		if (autoFiltering){ filterBy ^= FILTER_BY_SELECTIONS; }
		else{ grid->file->GetSelections(keySelections); }
	}
	Dialogue *lastDial = NULL;
	for (size_t i = 0; i < grid->file->GetCount(); i++){
		Dialogue *dial = grid->file->GetDialogue(i);
		if (dial->NonDialogue) continue;
		bool hideDialogue = CheckHiding(dial, i);
		if (hideDialogue && !Invert || !hideDialogue && Invert){
			//if (*dial->isVisible && i <= activeLine){ activeLineDiff--; }
			dial->isVisible = NOT_VISIBLE;
		}
		else if (addToFilter){
			if (lastDial && lastDial->isVisible == VISIBLE_BLOCK && dial->isVisible == NOT_VISIBLE){ 
				dial->isVisible = VISIBLE_BLOCK; 
			}
			else if (lastDial && lastDial->isVisible == NOT_VISIBLE && dial->isVisible == VISIBLE_BLOCK){ 
				lastDial->isVisible = VISIBLE_BLOCK; 
			}
			lastDial = dial;
		}
		else{
			//if (!dial->isVisible && i <= activeLine){ activeLineDiff++; }
			dial->isVisible = VISIBLE;
		}
	}

	FilteringFinalize();
}

void SubsGridFiltering::FilterPartial(int from)
{
	Dialogue *lastDial = NULL;
	int keyFrom = from + 1;
	int keyTo = keyFrom;
	bool hide = true;
	bool changed = false;

	for (size_t i = keyFrom; i < grid->file->GetCount(); i++){
		Dialogue *dial = grid->file->GetDialogue(i);
		if (!dial->NonDialogue){
			if (lastDial && dial->isVisible == VISIBLE){
				keyTo = i - 1;
				changed = true;
				break;
			}
			if (!dial->isVisible){ hide = false; }
			dial->isVisible = !hide ? VISIBLE_BLOCK : NOT_VISIBLE;
			lastDial = dial;
		}
	}
	if (!changed){
		keyTo = grid->file->GetCount() - 1;
		//KaiLogDebug("Something went wrong with partially hiding it is better to check it for potencial bugs.");
	}
	grid->RefreshSubsOnVideo(activeLine, false);
	grid->RefreshColumns();
}

void SubsGridFiltering::HideSelections()
{
	grid->file->GetSelections(keySelections);
	Dialogue *lastDial = NULL;
	int selssize = keySelections.size();
	int j = 0;
	for (int i = 0; i < grid->file->GetCount(); i++){
		Dialogue *dial = grid->file->GetDialogue(i);
		if (dial->NonDialogue) continue;
		bool isSelected = false;
		if (j < selssize){ isSelected = keySelections[j] == i; if (isSelected){ j++; } }
		if (isSelected && !Invert || !isSelected && Invert){
			dial->isVisible = NOT_VISIBLE;
		}
		if (lastDial && lastDial->isVisible == VISIBLE_BLOCK && dial->isVisible == NOT_VISIBLE){ 
			dial->isVisible = VISIBLE_BLOCK; 
		}
		else if (lastDial && lastDial->isVisible == NOT_VISIBLE && dial->isVisible == VISIBLE_BLOCK){ 
			lastDial->isVisible = VISIBLE_BLOCK; 
		}
		lastDial = dial;
	}
	FilteringFinalize();
}

void SubsGridFiltering::MakeTree()
{
	grid->file->GetSelections(keySelections);
	//Dialogue *lastDial = NULL;
	int selssize = keySelections.size();
	int j = 0;
	int treeDiff = 0;
	bool startSelection = true;
	for (int i = 0; i < grid->file->GetCount() - treeDiff; i++){
		Dialogue *dial = grid->file->GetDialogue(i + treeDiff);
		if (dial->NonDialogue || !dial->isVisible) continue;
		bool isSelected = false;
		if (j < selssize){ isSelected = keySelections[j] == i; if (isSelected){ j++; } }
		if (isSelected){
			if (startSelection){
				Dialogue *treeStart = dial->Copy();
				treeStart->IsComment = true;
				treeStart->isVisible = VISIBLE;
				treeStart->treeState = TREE_DESCRIPTION;
				treeStart->Text = L"";
				treeStart->TextTl = L"";
				grid->InsertRows(i + treeDiff, 1, treeStart, true, false);
				treeDiff++;
				startSelection = false;
			}
			dial->isVisible = NOT_VISIBLE;
			dial->treeState = TREE_CLOSED;
		}
		else if (!startSelection)
			startSelection = true;

		//if (lastDial && lastDial->isVisible == VISIBLE_BLOCK && dial->isVisible == NOT_VISIBLE){ dial->isVisible = VISIBLE_BLOCK; }
		//else if (lastDial && lastDial->isVisible == TREE_NOT_VISIBLE && dial->isVisible == VISIBLE_BLOCK){ lastDial->isVisible = VISIBLE_BLOCK; }
		//lastDial = dial;
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
	int keyActiveLine = activeLine;
	for (size_t i = 0; i < grid->file->GetCount(); i++){
		Dialogue *dial = grid->file->GetDialogue(i);
		if (dial->isVisible != VISIBLE && !dial->NonDialogue){
			dial->isVisible = VISIBLE;
		}
	}
}

void SubsGridFiltering::FilteringFinalize()
{
	grid->RefreshSubsOnVideo(activeLine);
	grid->RefreshColumns();
}

inline bool SubsGridFiltering::CheckHiding(Dialogue *dial, int i)
{
	int result = filterBy;
	if (filterBy & FILTER_BY_SELECTIONS){
		if (selectionsJ < keySelections.size() && keySelections[selectionsJ] == i){
			selectionsJ++;
			return true;
		}
		else{
			result ^= FILTER_BY_SELECTIONS;
		}
	}
	if (filterBy & FILTER_BY_STYLES){
		for (auto style : styles){
			if (style == dial->Style) {
				return true;
			}
		}
		result ^= FILTER_BY_STYLES;
	}
	if (filterBy & FILTER_BY_DIALOGUES && !dial->IsComment){
		result ^= FILTER_BY_DIALOGUES;
	}
	if (filterBy & FILTER_BY_DOUBTFUL && dial->IsDoubtful()){
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