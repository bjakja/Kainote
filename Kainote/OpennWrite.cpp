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
#include <limits>
#include <vector>


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
		if (!filetest.Open(filename, wxFile::read, wxS_DEFAULT)) { return false; }
		wxFileOffset fileLength = filetest.Length();
		if (fileLength < 0 || static_cast<unsigned long long>(fileLength) >
			static_cast<unsigned long long>(std::numeric_limits<size_t>::max())) {
			return false;
		}
		std::vector<char> buff(static_cast<size_t>(fileLength));
		if (!buff.empty() && filetest.Read(buff.data(), buff.size()) != buff.size()) {
			return false;
		}

		utf8 = buff.size() >= 3 && static_cast<unsigned char>(buff[0]) == 0xEF &&
			static_cast<unsigned char>(buff[1]) == 0xBB &&
			static_cast<unsigned char>(buff[2]) == 0xBF;
		if (!utf8){
			utf8 = IsUTF8withoutBOM(buff.data(), buff.size());
			if (!utf8) {
				wxString result;
				if (CheckCharSet(buff.data(), buff.size(), &result)) {
					result.MakeUpper();
					conv = new wxCSConv(result);
				}
			}
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
	if (!buf && size != 0) { return false; }
	bool onlySawAscii = true;
	size_t pos = 0;
	auto isContinuation = [](unsigned char ch) { return ch >= 0x80 && ch <= 0xBF; };
	while (pos < size) {
		unsigned char first = static_cast<unsigned char>(buf[pos++]);
		if (first <= 0x7F) { continue; }
		onlySawAscii = false;

		if (first >= 0xC2 && first <= 0xDF) {
			if (pos >= size || !isContinuation(static_cast<unsigned char>(buf[pos++]))) { return false; }
		}
		else if (first >= 0xE0 && first <= 0xEF) {
			if (pos + 1 >= size) { return false; }
			unsigned char second = static_cast<unsigned char>(buf[pos++]);
			unsigned char third = static_cast<unsigned char>(buf[pos++]);
			if (!isContinuation(third) ||
				(first == 0xE0 ? second < 0xA0 || second > 0xBF :
				 first == 0xED ? second < 0x80 || second > 0x9F : !isContinuation(second))) {
				return false;
			}
		}
		else if (first >= 0xF0 && first <= 0xF4) {
			if (pos + 2 >= size) { return false; }
			unsigned char second = static_cast<unsigned char>(buf[pos++]);
			unsigned char third = static_cast<unsigned char>(buf[pos++]);
			unsigned char fourth = static_cast<unsigned char>(buf[pos++]);
			if (!isContinuation(third) || !isContinuation(fourth) ||
				(first == 0xF0 ? second < 0x90 || second > 0xBF :
				 first == 0xF4 ? second < 0x80 || second > 0x8F : !isContinuation(second))) {
				return false;
			}
		}
		else {
			return false;
		}
	}

	return !onlySawAscii;

}

bool OpenWrite::CheckCharSet(char* buf, size_t size, wxString* result)
{
	uchardet_t detector = uchardet_new();
	if (!detector)
		return false;

	if (uchardet_handle_data(detector, buf, size) != 0) {
		uchardet_delete(detector);
		return false;
	}
	uchardet_data_end(detector);
	const char* encoding = uchardet_get_charset(detector);
	bool detected = encoding && *encoding;
	if (detected)
		*result = wxString(encoding);

	uchardet_delete(detector);

	return detected;
}
