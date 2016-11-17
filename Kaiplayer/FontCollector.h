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

//#define UNICODE


#include <wx/wx.h>
#include <Windows.h>
#include <vector>
#include <map>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_SFNT_NAMES_H

class FontCollectorDialog : public wxDialog
{
public:
	FontCollectorDialog(wxWindow *parent);
	virtual ~FontCollectorDialog(){};
	typedef std::map<wxUniChar,wxString> CharMap;
	std::map<wxString,CharMap> FontMap;
	wxTextCtrl *path;
	wxButton *choosepath;
	wxRadioBox *opts;
	wxTextCtrl *console;
	wxButton *bok;
	wxButton *bcancel;
	wxCheckBox *fromMKV;
	wxCheckBox *subsdir;
	void PutChars(wxString txt, wxString fn, int move);
	wxString CheckChars(FT_Face face, std::map<wxUniChar, wxString> *chars);
	wxArrayString GetAssFonts(std::vector<bool> &founded, bool check=false);
	void CopyFonts(bool check=false);
	void CopyMKVFonts();
	void MuxVideoWithSubs();
	wxString destdir;
	wxArrayString facenames;
	wxArrayString fontnames;
	wxString copypath;

private:
	void OnButtonStart(wxCommandEvent &event);
	void OnChangeOpt(wxCommandEvent &event);
	void OnButtonPath(wxCommandEvent &event);
};

