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

SubsGridFiltering::SubsGridFiltering(SubsGrid *_grid)
	:grid(_grid)
	, Invert(false)
{
}

SubsGridFiltering::~SubsGridFiltering()
{
}

void SubsGridFiltering::Filter()
{
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
		Options.GetTable(GridFilterStyles, styles, ",");
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
	int keyFrom = grid->file->GetElementById(from);
	int keyTo = keyFrom;
	for (int i = keyFrom; i < Subs->dials.size(); i++){
		Dialogue *dial = Subs->dials[i];
		if (!dial->NonDialogue){ 
			if (i != keyFrom && dial->isVisible == (unsigned char)hide){ 
				keyTo = i - 1; 
				if (keyFrom == 0 && hide){ lastDial->isVisible = VISIBLE_HIDDEN_BLOCK; }
				else if (!hide){ lastDial->isVisible = VISIBLE_END_BLOCK; }
				break;
			}
			dial->isVisible = (hide && (i != 0 && i == keyFrom)) ? VISIBLE_HIDDEN_BLOCK :
				(!hide && i == keyFrom) ? VISIBLE_START_BLOCK : 
				!hide ? VISIBLE :
				NOT_VISIBLE;
		}
		lastDial = dial;
	}
	if (keyFrom == keyTo){ 
		keyTo = Subs->dials.size() - 1; 
		wxLogStatus("Something went wrong with partially hiding it is better to check it for potencial bugs."); 
	}
	grid->file->ReloadVisibleDialogues(keyFrom, keyTo);
	grid->RefreshColumns();
}

void SubsGridFiltering::FilterByDoubtful()
{
	File *Subs = grid->file->GetSubs();
	Dialogue *lastDial = NULL;
	int isFirstInvisible = 0;
	for (int i = 0; i < Subs->dials.size(); i++){
		Dialogue *dial = Subs->dials[i];
		if (dial->NonDialogue) continue;
		if (dial->State & 4){ dial->isVisible = !Invert; }
		else{ dial->isVisible = Invert; }
		if (!isFirstInvisible && !dial->isVisible){
			isFirstInvisible++;
		}
		else if (isFirstInvisible == 1 && dial->isVisible){
			dial->isVisible = VISIBLE_HIDDEN_BLOCK;
			isFirstInvisible++;
		}
		else if (lastDial && lastDial->isVisible && !dial->isVisible){
			lastDial->isVisible = VISIBLE_HIDDEN_BLOCK;
		}
		lastDial = dial;
	}
}

void SubsGridFiltering::FilterByUntranslated()
{
	File *Subs = grid->file->GetSubs();
	Dialogue *lastDial = NULL;
	int isFirstInvisible = 0;
	for (int i = 0; i < Subs->dials.size(); i++){
		Dialogue *dial = Subs->dials[i];
		if (dial->NonDialogue) continue;
		if (dial->TextTl.empty()){ dial->isVisible = !Invert; }
		else{ dial->isVisible = Invert; }
		if (!isFirstInvisible && !dial->isVisible){
			isFirstInvisible++;
		}
		else if (isFirstInvisible == 1 && dial->isVisible){
			dial->isVisible = VISIBLE_HIDDEN_BLOCK;
			isFirstInvisible++;
		}
		else if (lastDial && lastDial->isVisible && !dial->isVisible){
			lastDial->isVisible = VISIBLE_HIDDEN_BLOCK;
		}
		lastDial = dial;
	}
}
inline bool FindStyle(const wxArrayString &styles, const wxString &dialStyle){
	for (auto style : styles){ 
		if (style == dialStyle) { return true; }
	}
	return false;
}

void SubsGridFiltering::FilterByStyles(wxArrayString &styles)
{
	File *Subs = grid->file->GetSubs();
	Dialogue *lastDial = NULL;
	int isFirstInvisible = 0;
	for (int i = 0; i < Subs->dials.size(); i++){
		Dialogue *dial = Subs->dials[i];
		if (dial->NonDialogue) continue;
		if (FindStyle(styles, dial->Style)){ dial->isVisible = !Invert; }
		else{ dial->isVisible = Invert; }
		if (!isFirstInvisible && !dial->isVisible){
			isFirstInvisible++;
		}
		else if (isFirstInvisible == 1 && dial->isVisible){
			dial->isVisible = VISIBLE_HIDDEN_BLOCK;
			isFirstInvisible++;
		}
		else if (lastDial && lastDial->isVisible && !dial->isVisible){
			lastDial->isVisible = VISIBLE_HIDDEN_BLOCK;
		}
		lastDial = dial;
	}
}

void SubsGridFiltering::FilterBySelections(bool addToFiltering)
{
	File *Subs = grid->file->GetSubs();
	Dialogue *lastDial = NULL;
	int isFirstInvisible = 0;
	for (int i = 0; i < Subs->dials.size(); i++){
		Dialogue *dial = Subs->dials[i];
		if (dial->NonDialogue) continue;
		if (grid->Selections.find(grid->file->GetElementByKey(i)) != grid->Selections.end()){ 
			dial->isVisible = (addToFiltering)? false : !Invert; 
		}
		else if (!addToFiltering){ dial->isVisible = Invert; }
		if (!isFirstInvisible && !dial->isVisible){
			isFirstInvisible++;
		}
		else if (isFirstInvisible == 1 && dial->isVisible){
			dial->isVisible = VISIBLE_HIDDEN_BLOCK;
			isFirstInvisible++;
		}
		else if (lastDial && lastDial->isVisible && !dial->isVisible){
			lastDial->isVisible = VISIBLE_HIDDEN_BLOCK;
		}
		lastDial = dial;
	}
	if (addToFiltering){ FilteringFinalize(); }
}

void SubsGridFiltering::TurnOffFiltering()
{
	File *Subs = grid->file->GetSubs();
	for (auto dial : Subs->dials){
		if (!dial->isVisible && !dial->NonDialogue){ 
			dial->isVisible = VISIBLE; 
		}
	}
}

void SubsGridFiltering::FilteringFinalize()
{
	grid->file->ReloadVisibleDialogues();
	grid->RefreshColumns();
	grid->RefreshSubsOnVideo();
}