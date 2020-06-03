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

#pragma once
#include <wx/wx.h>

class Dialogue;
class SubsGrid;

class SubsGridFiltering
{
public:
	SubsGridFiltering(SubsGrid *_grid, int _activeLine);
	~SubsGridFiltering();

	void Filter(bool autoFiltering = false);
	void FilterPartial(int from);
	void HideSelections();
	void MakeTree();
	void RemoveFiltering();

private:
	inline bool CheckHiding(Dialogue *dial, int i);
	void TurnOffFiltering();
	void FilteringFinalize();
	SubsGrid *grid;
	bool Invert;
	//int activeLineDiff = 0;
	int activeLine;
	int filterBy = 0;
	int selectionsJ = 0;
	wxArrayInt keySelections;
	wxArrayString styles;
};

enum{
	FILTER_BY_STYLES = 1,
	FILTER_BY_SELECTIONS,
	FILTER_BY_DIALOGUES = 4,
	FILTER_BY_DOUBTFUL = 8,
	FILTER_BY_UNTRANSLATED = 16,
};
