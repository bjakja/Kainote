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
{
}

SubsGridFiltering::~SubsGridFiltering()
{
}

void SubsGridFiltering::Filter()
{
	Invert = Options.GetBool(GridFilterInverted);
	bool filterBy = Options.GetInt(GridFilterBy);
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

void SubsGridFiltering::FilterPartial(int from, int to, bool hide)
{
	File *Subs = grid->file->GetSubs();
	int keyFrom = grid->file->GetElementById(from);
	int keyTo = grid->file->GetElementById(to);
	if (keyFrom > 0){ keyFrom--; }
	else if (keyFrom < Subs->dials.size()){ keyTo++; }
	for (int i = keyFrom; i <= keyTo; i++){
		Dialogue *dial = Subs->dials[i];
		if (!dial->NonDialogue){ 
			dial->isVisible = (hide && ((i != 0 && i == keyFrom) || (i == 0 && i == keyTo))) ? VISIBLE_HIDDEN_BLOCK :
				(!hide && i == keyFrom) ? VISIBLE_START_BLOCK : 
				(!hide && i == keyTo) ? VISIBLE_END_BLOCK : 
				!hide ? VISIBLE :
				NOT_VISIBLE;
		}
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

void SubsGridFiltering::FilterBySelections()
{
	File *Subs = grid->file->GetSubs();
	Dialogue *lastDial = NULL;
	int isFirstInvisible = 0;
	for (int i = 0; i < Subs->dials.size(); i++){
		Dialogue *dial = Subs->dials[i];
		if (dial->NonDialogue) continue;
		if (grid->Selections.find(grid->file->GetElementByKey(i)) != grid->Selections.end()){ dial->isVisible = !Invert; }
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

void SubsGridFiltering::TurnOffFiltering()
{
	File *Subs = grid->file->GetSubs();
	for (auto dial : Subs->dials){
		if (!dial->isVisible && !dial->NonDialogue){ dial->isVisible = VISIBLE; }
	}
}

void SubsGridFiltering::FilteringFinalize()
{
	grid->file->ReloadVisibleDialogues();
	grid->RefreshColumns();
}