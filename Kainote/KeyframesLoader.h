//  Copyright (c) 2018 - 2026, Marcin Drob

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
//#include <wx/dynarray.h>
#include <wx/tokenzr.h>
#include <vector>
class Timebase;


class KeyframeLoader
{
public:
	//keyframes are frame numbers in the file, turned into ms with timebase
	KeyframeLoader(const wxString &filename, std::vector<int> *keyframes, const Timebase &timebase);
private:
	void LoadFile(const wxString &filename);
	void OpenAegisubKeyframes(wxStringTokenizer *kftokenizer);
	void OpenOtherKeyframes(int type, wxStringTokenizer *kftokenizer);

	std::vector<int> *keyframes;
	const Timebase &timebase;
};

enum{
	TYPE_XVID,
	TYPE_DIVX,
	TYPE_X264,
};