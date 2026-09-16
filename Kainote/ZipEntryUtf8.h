//  Copyright (c) 2016-2026, Marcin Drob

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
#include <wx/zipstrm.h>

//wxWidgets writes entry names with the conversion given to wxZipOutputStream,
//but it never sets the language encoding flag (general purpose bit 11), so
//extractors fall back to CP437 and mangle every non ASCII name (issue #440).
//Setting that flag is all it takes to make the UTF-8 names the stream already
//writes readable everywhere. wxZipEntry keeps SetFlags protected, hence the
//subclass.
class Utf8ZipEntry : public wxZipEntry
{
public:
	explicit Utf8ZipEntry(const wxString &name)
		: wxZipEntry(name)
	{
		SetFlags(GetFlags() | UTF8_NAME_FLAG);
	}

private:
	enum { UTF8_NAME_FLAG = 0x800 };
};
