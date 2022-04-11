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

#include <wx/string.h>

class SubsGrid;

class SubsLoader{
public:
	SubsLoader(SubsGrid *grid, const wxString &text, wxString &ext);
private:
	bool LoadASS(const wxString &text);
	bool LoadSRT(const wxString &text);
	bool LoadTXT(const wxString &text);
	SubsGrid *grid;
};