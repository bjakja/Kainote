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


//#include <wx/wx.h>
#include <Windows.h>
#include <vector>
#include <map>
#include "KaiRadioButton.h"
#include "MappedButton.h"
#include "KaiTextCtrl.h"
#include "KaiDialog.h"
#include <wx/textctrl.h>
#include <wx/zipstrm.h>

class SubsFont{
public:
	SubsFont(const wxString &_name, const LOGFONTW &_logFont, int bold, bool italic);
	SubsFont(){};
	LOGFONTW &GetLogFont(HDC dc);
	wxString name;
	LOGFONTW logFont;
	int bold; 
	int italic;
	bool fakeBoldItalic;
	bool fakeBold;
	bool fakeItalic;
	bool fakeNormal;
};

class FontCollectorDialog : public KaiDialog
{
public:
	FontCollectorDialog(wxWindow *parent);
	virtual ~FontCollectorDialog(){};
	typedef std::map<wxUniChar,wxString> CharMap;
	std::map<wxString,CharMap> FontMap;
	KaiTextCtrl *path;
	MappedButton *choosepath;
	KaiRadioBox *opts;
	wxTextCtrl *console;
	MappedButton *bok;
	MappedButton *bcancel;
	KaiCheckBox *fromMKV;
	KaiCheckBox *subsdir;
	void PutChars(wxString txt, wxString fn, int move);
	void GetAssFonts(std::vector<bool> &found, bool check=false);
	bool CheckPathAndGlyphs(bool onlyGlyphs, std::vector<bool> &found);
	void CopyFonts(bool check=false);
	void CopyMKVFonts();
	void MuxVideoWithSubs();
	wxString destdir;
	wxArrayString facenames;
	wxArrayString fontnames;
	std::vector<LOGFONTW> logFonts;
	std::map<wxString, wxString> notFindFontsLog;
	std::map<wxString, wxString> findFontsLog;
	std::map<wxString, SubsFont> foundFonts;
	std::multimap<long, wxString> fontSizes;
	wxString copypath;
	wxString fontfolder;
	wxColour warning;
	wxColour normal;
private:
	bool SaveFont(const wxString &fontname);
	void EnumerateFonts();
	void OnButtonStart(wxCommandEvent &event);
	void OnChangeOpt(wxCommandEvent &event);
	void OnButtonPath(wxCommandEvent &event);
	wxZipOutputStream *zip;
	wxStopWatch sw;
};

