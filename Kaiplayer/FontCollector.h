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
#include <set>
#include "KaiRadioButton.h"
#include "MappedButton.h"
#include "KaiTextCtrl.h"
#include "KaiDialog.h"
#include <wx/zipstrm.h>
#include <wx/thread.h>
//enum{
//	COLOR_WARNING = 
//}

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

class FontCollector;

class FontCollectorDialog : public KaiDialog
{
	friend class FontCollector;
public:
	FontCollectorDialog(wxWindow *parent, FontCollector *_fc);
	virtual ~FontCollectorDialog();
	
	KaiTextCtrl *path;
	MappedButton *choosepath;
	KaiRadioBox *opts;
	KaiTextCtrl *console;
	MappedButton *bok;
	MappedButton *bcancel;
	KaiCheckBox *fromMKV;
	KaiCheckBox *subsdir;
	wxString destdir;
	wxString copypath;
	wxColour warning;
	wxColour normal;
private:
	void OnButtonStart(wxCommandEvent &event);
	void OnChangeOpt(wxCommandEvent &event);
	void OnButtonPath(wxCommandEvent &event);
	void EnableControls();
	FontCollector *fc;
};

class FontCollector
{
	friend class FontCollectorDialog;
	friend class FontCollectorThread;
public:
	FontCollector(wxWindow *parent);
	~FontCollector();
	void CopyFonts();
	void CopyMKVFonts();
	void MuxVideoWithSubs();
	void StartCollect(int operation);
	void ShowDialog(wxWindow *parent);
	FontCollectorDialog *fcd;
	enum{
		CHECK_FONTS=1,
		COPY_FONTS,
		COPY_MKV_FONTS=4,
		MUX_VIDEO_WITH_SUBS=8,
		AS_ZIP=16
	};
private:
	typedef std::set<wxUniChar> CharMap;
	std::map<wxString,CharMap> FontMap;
	void PutChars(const wxString &txt, const wxString &fn);
	void GetAssFonts(std::vector<bool> &found, bool check=false);
	bool CheckPathAndGlyphs(std::vector<bool> &found);
	bool SaveFont(const wxString &fontname);
	void EnumerateFonts();
	void SendMessageD(wxString string, wxColour col);
	bool AddFont(const wxString &string);

	wxArrayString facenames;
	wxArrayString fontnames;
	std::vector<LOGFONTW> logFonts;
	std::map<wxString, wxString> notFindFontsLog;
	std::map<wxString, wxString> findFontsLog;
	std::map<wxString, SubsFont> foundFonts;
	std::multimap<long, wxString> fontSizes;
	wxString fontfolder;
	wxString muxerpath;
	wxZipOutputStream *zip;
	wxStopWatch sw;
	//wxString copypath;
	int operation;
	bool reloadFonts;
};

class FontCollectorThread : public wxThread
{
	friend class FontCollector;
public:
	FontCollectorThread(FontCollector *fc);
	virtual ~FontCollectorThread(){};
	wxThread::ExitCode Entry();
private:
	FontCollector *fc;
};