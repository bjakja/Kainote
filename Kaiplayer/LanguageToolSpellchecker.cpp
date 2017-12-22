//  Copyright (c) 2017, Marcin Drob

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

#include "SubsGrid.h"
#include "LanguageToolSpellchecker.h"

LanguageToolSpellchecker::LanguageToolSpellchecker(SubsGrid *_grid)
	:grid(_grid)
{
	
}

void LanguageToolSpellchecker::CheckLines(size_t from, size_t to)
{
	size_t size = grid->GetCount();
	size_t ltfrom = (from > 0) ? from-1 : 0;
	size_t ltto = (to < size-1) ? to + 1 : size-1;
}

void LanguageToolSpellchecker::StripTags()
{

}
