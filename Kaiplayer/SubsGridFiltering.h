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

#pragma once
#include <wx/wx.h>
class SubsGrid;

class SubsGridFiltering
{
public:
	SubsGridFiltering(SubsGrid *_grid);
	~SubsGridFiltering();

	void Filter();
	void FilterPartial(int from, bool hide = true);
	void FilterBySelections(bool addToFiltering = false);

private:
	void FilterByDoubtful();
	void FilterByUntranslated();
	void FilterByStyles(wxArrayString &styles);
	void TurnOffFiltering();
	void FilteringFinalize();
	SubsGrid *grid;
	bool Invert;
};

enum{
	NOT_VISIBLE=0,
	VISIBLE,
	VISIBLE_START_BLOCK,
	VISIBLE_BLOCK,
	VISIBLE_END_BLOCK,
	VISIBLE_HIDDEN_BLOCK
};
