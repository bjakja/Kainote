//  Copyright (c) 2016 - 2026, Marcin Drob

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


#include "OpennWrite.h"
#include "config.h"
#include <wx/filename.h>
#include <wx/ffile.h>
#include <wx/filefn.h>
#include <wx/log.h>
#include "LogHandler.h"
#include <uchardet.h>


OpenWrite::OpenWrite()
{

}

OpenWrite::OpenWrite(const wxString &fileName, bool clear)
{
	wxFileName fname;
	fname.Assign(fileName);
	if (!fname.DirExists()){ wxFileName::Mkdir(KaiPathDir(fileName), 511, wxPATH_MKDIR_FULL); }
	if (fname.FileExists() && !fname.IsFileReadable()){ return; }
	if (!file.Exists(fileName)){
		if (!file.Create(fileName, false, wxS_DEFAULT)){ KaiLog(_("Nie można utworzyć pliku.")); }
	}
	else{
		if (!file.Open(fileName, (clear) ? wxFile::write : wxFile::write_append, wxS_DEFAULT)){ KaiLog(_("Nie można otworzyć pliku.")); };
	}
	isfirst = clear;
}

OpenWrite::~OpenWrite()
{
	CloseFile();
}

bool OpenWrite::FileOpen(const wxString &filename, wxString *riddenText, bool test)
{

	bool utf8 = true;
	wxMBConv *conv = NULL;
	wxFileName fname;
	fname.Assign(filename);

	if (!fname.IsFileReadable()){ return false; }
	if (test){
		wxFile filetest;
		wchar_t b[4];
		filetest.Open(filename, wxFile::read, wxS_DEFAULT);
		filetest.Read(b, 4);

		utf8 = ((static_cast <int>(b[0]) > 48000)) ? true : false;
		if (!utf8){
			size_t size = filetest.Length();
			char *buff = new char[size];
			filetest.Read(buff, size);
			utf8 = IsUTF8withoutBOM(buff, size);
			if (!utf8) {
				wxString result;
				if (CheckCharSet(buff, size, &result)) {
					result.MakeUpper();
					conv = new wxCSConv(result);
				}
			}
			delete[] buff;
		}
		filetest.Close();
	}
	wxFFile fileo;
	fileo.Open(filename, L"r");
	if (fileo.IsOpened()){
		wxCSConv convl = wxConvLocal;
		if (utf8){
			fileo.ReadAll(riddenText);
		}
		else{ fileo.ReadAll(riddenText, conv? *conv : convl); }
		fileo.Close();
		if (conv)
			delete conv;
		if (riddenText->empty()) return false;
		return true;
	}
	if (conv)
		delete conv;

	return false;
}

void OpenWrite::FileWrite(const wxString &fileName, const wxString &textfile, bool utf)
{

	wxFileName fname;
	fname.Assign(fileName);
	if (!fname.DirExists()){ wxFileName::Mkdir(KaiPathDir(fileName), 511, wxPATH_MKDIR_FULL); }
	if (fname.FileExists() && !fname.IsFileReadable()){
		KaiLog(_("Nie można odczytać pliku."));
		return;
	}

	wxFile file;
	if (!file.Exists(fileName)){
		file.Create(fileName, false, wxS_DEFAULT);
	}
	else{
		file.Open(fileName, wxFile::write, wxS_DEFAULT);
	}
	if (file.IsOpened()){
		if (utf){
			wchar_t bom = 0xFEFF;
			file.Write(wxString(bom) + textfile, wxConvUTF8);
		}
		else{ file.Write(textfile, wxConvLocal); }
		file.Close();
	}

}
void OpenWrite::PartFileWrite(const wxString &parttext)
{
	if (!file.IsOpened()){ KaiLog(_("Plik nie został otwarty.")); return; }
	if (isfirst){
		wchar_t bom = 0xFEFF;
		if (!file.Write(wxString(bom) + parttext/*,wxConvUTF8*/)){ KaiLog(_("Nie można zapisać do pliku.")); };
		isfirst = false;
		return;
	}
	if (!file.Write(parttext/*,wxConvUTF8*/)){ KaiLog(_("Nie można zapisać do pliku.")); };
}

void OpenWrite::CloseFile()
{
	if (file.IsOpened()){ file.Close(); }
}

bool OpenWrite::IsUTF8withoutBOM(char* buf, size_t size)
{
	bool only_saw_ascii_range = true;
	size_t pos = 0;
	int	more_chars = 0;
	while (pos < size)
	{
		unsigned char ch = buf[pos++];
		if (ch <= 127)
		{
			// 1 byte
			more_chars = 0;

		}
		else if (ch >= 194 && ch <= 223)
		{
			// 2 Byte
			more_chars = 1;

		}
		else if (ch >= 224 && ch <= 239)
		{
			// 3 Byte
			more_chars = 2;

		}
		else if (ch >= 240 && ch <= 244)
		{
			// 4 Byte
			more_chars = 3;

		}
		else
		{
			//if this will not work, maybe set continue;
			continue;
			//return false; // Not utf8
		}
		// Check secondary chars are in range if we are expecting any
		while (more_chars && pos < size)
		{
			only_saw_ascii_range = false; // Seen non-ascii chars now
			ch = buf[pos++];
			if (ch < 128 || ch > 205) { return false; } // Not utf8
			--more_chars;
		}
	}

	if (only_saw_ascii_range){ return false; }

	return true;

}

bool OpenWrite::CheckCharSet(char* buf, size_t size, wxString* result)
{
	uchardet_t detector = uchardet_new();
	if (!detector)
		return false;

	int intresult = uchardet_handle_data(detector, buf, size);
	uchardet_data_end(detector);
	const char* encoding = uchardet_get_charset(detector);
	if (encoding && *encoding)
		*result = wxString(encoding);
	else
		return false;

	uchardet_delete(detector);

	return true;
}
