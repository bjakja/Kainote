//  Copyright (c) 2016, Marcin Drob

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

#include <wx/file.h>
#include <wx/thread.h>

class OpenWrite
{
    public:
    OpenWrite();
    OpenWrite(const wxString &filename, bool clear=true);
    ~OpenWrite();
    void CloseFile();
    bool FileOpen(const wxString &filename, wxString *riddenText,bool test=true);
    void FileWrite(const wxString &filename, const wxString &alltext, bool utf=true);
    void PartFileWrite(const wxString &parttext);
	bool IsUTF8withoutBOM(const char* buf, size_t size);
	wxFile file;
    private:

    bool isfirst;
};


