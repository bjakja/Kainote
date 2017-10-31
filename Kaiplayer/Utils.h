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

#ifndef UTILS_H
#define UTILS_H

#include <wx/wx.h>

class MyTokenizer{
public:
	MyTokenizer(const wxString &TextToTokenize, const wxString &delimiters, int flag);
	wxString GetNextToken();
	wxString GetPrevToken();
	void SetPosition(int position);
	int GetPosition();
	bool HasMoreTokens();
	size_t CountTokens();

private:
	wxString text;
	wxString delims;
	int pos;
	int lastDelim;
	int flag;
	bool hasMoreTokens;
};

enum{
	FLAG_NOEMPTY=1,
	FLAG_EMPTYALL=2,
	FLAG_EMPTYEND=4,
	FLAG_RET_DELIMS=8

};

#endif // !UTILS_H

