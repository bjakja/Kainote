//  Copyright (c) 2016 - 2020, Marcin Drob

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

#include <wx/string.h>
#include <wx/thread.h>


class STime
{
private:

	char form;

public:
	int orgframe;
	//wxString raw;
	int mstime;

	STime();
	STime(int ms, int orgFrame = 0);
	~STime();
	void SetRaw(wxString rawtime, char format);
	void NewTime(int ms);
	void NewFrame(int frame);
	void ParseMS(wxString time);
	wxString raw(char format = 0);//, float fps=0
	char GetFormat();
	void ChangeFormat(char format, float fps = 0);
	wxString GetFormatted(char format);
	void Change(int ms);
	void ChangeFrame(int ms);
	bool operator> (const STime &comp);
	bool operator< (const STime &comp);
	bool operator>= (const STime &comp);
	bool operator<= (const STime &comp);
	bool operator== (const STime &comp);
	bool operator!= (const STime &comp);
	STime operator- (const STime &comp);
	STime operator+ (const STime &comp);
};



