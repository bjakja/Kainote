//  Copyright (c) 2021, Marcin Drob

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
#include <wx/msw/winundef.h>
#include <wx/string.h>
#include <wx/msw/winundef.h>
#include <wx/arrstr.h>
#include <wx/msw/winundef.h>
#include <wx/zipstrm.h>
#include "include/ffms.h"
#include <vector>

class SubsGrid;
class ProgressSink;

class Demux {
public:
	Demux() {};
	~Demux();
	bool Open(const wxString& filename);
	void Close();
	bool GetSubtitles(SubsGrid* target);
	void GetFontList(wxArrayString *list);
	bool SaveFont(int i, const wxString& path, wxZipOutputStream* zip = nullptr);

private:
	static int __stdcall GetSubtitles(long long Start, long long Duration, long long Total, const char* Line, void* ICPrivate);
	FFMS_Indexer* indexer = nullptr;
	std::vector<FFMS_Attachment*> attachments;
	ProgressSink* progress = nullptr;
	int codecType = 0;
	std::vector<wxString> subtitleList;
};