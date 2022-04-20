//  Copyright (c) 2016-2020, Marcin Drob

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
#include <vector>
#include <map>
#include <set>
#include "KaiRadioButton.h"
#include "MappedButton.h"
#include "KaiTextCtrl.h"
#include "KaiDialog.h"
#include <wx/zipstrm.h>
#include <wx/thread.h>

class SubsFile;

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

//for now I disabled multithreading
//it needs mor changes, every new or table editing must 
//be secured by critical section
//It can replace one elements by another, fonts will randomly disappear
class FontLogContent{
public:
	FontLogContent();
	FontLogContent(const wxString &_info, bool _notFound = false){
		//cs.Enter();
		info = _info;
		notFound = _notFound;
		//cs.Leave();
	}
	void SetStyle(int tab, const wxString &style){
		//cs.Enter();
		styles[style].Add(tab);
		//cs.Leave();
	}
	void SetLine(int tab, int line){
		//cs.Enter();
		lines[line].Add(tab);
		//cs.Leave();
	}
	void AppendInfo(const wxString &_info){
		//cs.Enter();
		info << _info << L"\n";
		//cs.Leave();
	}
	void AppendWarnings(const wxString &warning){
		//cs.Enter();
		warnings << warning << L"\n";
		//cs.Leave();
	}
	void DoLog(FontCollector *fc);
	void SetNotFound(bool _notFound = true){
		notFound = _notFound;
	}
	// no pointer checking
	bool CheckPosition(int position, bool *isStyle){
		if (position >= stylesArea.x && position <= stylesArea.y){
			*isStyle = true;
			return true;
		}
		if (position >= linesArea.x && position <= linesArea.y){
			*isStyle = false;
			return true;
		}
		return false;
	}

	std::map<wxString, wxArrayInt> styles;
	std::map<int, wxArrayInt> lines;
	wxString info;
	wxString warnings;
	bool notFound = false;
	wxPoint stylesArea = wxPoint(-1, -1);
	wxPoint linesArea = wxPoint(-1, -1);
	//wxCriticalSection cs;
};


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
	MappedButton *bStartOnAllTabs;
	MappedButton *bOpenFontFolder;
	MappedButton *bClose;
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
	void EnableControls(bool enable = true);
	void OnConsoleDoubleClick(wxMouseEvent &evt);
	void ParseDoubleClickResults(FontLogContent *flc, int cursorPos, bool isStyle);
	void OpenStyle(int tab, const wxString &style);
	void SetLine(int tab, int line);

	FontCollector *fc;
	wxWindowDisabler *disabler = nullptr;
};

class FontCollector
{
	friend class FontCollectorDialog;
	friend class FontCollectorThread;
	friend class TabFontSeekThread;
public:
	FontCollector(wxWindow *parent);
	~FontCollector();
	void CheckOrCopyFonts();
	void CopyMKVFonts();
	void MuxVideoWithSubs();
	void StartCollect(int operation);
	void ShowDialog(wxWindow *parent);
	void SendMessageD(const wxString &string, const wxColour &col);
	void GetAssFonts(SubsFile *subs, int tab);

	FontCollectorDialog *fcd;
	enum{
		CHECK_FONTS = 1,
		COPY_FONTS,
		COPY_MKV_FONTS = 4,
		MUX_VIDEO_WITH_SUBS = 8,
		AS_ZIP = 16,
		ON_ALL_TABS = 32
	};
	int currentTextPosition = 0;
private:
	typedef std::set<wxUniChar> CharMap;
	std::map<wxString, CharMap> FontMap;
	void PutChars(const wxString &txt, const wxString &fn);
	
	bool CheckPathAndGlyphs(int *found, int *notfound, int *notcopied);
	bool SaveFont(const wxString &fontname, FontLogContent *flc);
	void EnumerateFonts();
	bool AddFont(const wxString &string);
	void CopyMKVFontsFromTab(const wxString &path);
	void ClearTables();
	bool MakeDirectory(bool isZip);
	bool CreateZip();
	void CloseZip();

	wxArrayString facenames;
	wxArrayString fontnames;
	std::vector<LOGFONTW> logFonts;
	std::map<wxString, FontLogContent*> notFindFontsLog;
	std::map<wxString, FontLogContent*> findFontsLog;
	std::map<wxString, SubsFont*> foundFonts;
	std::multimap<long, wxString> fontSizes;
	wxString fontfolder;
	wxString fontFolderLocal;
	wxString muxerpath;
	wxZipOutputStream *zip;
	wxStopWatch sw;
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

//class TabFontSeekThread : public wxThread
//{
//	friend class FontCollector;
//public:
//	TabFontSeekThread(FontCollector *fc, SubsFile *file, int num);
//	virtual ~TabFontSeekThread(){};
//	wxThread::ExitCode Entry();
//private:
//	FontCollector *fc;
//	SubsFile *file;
//	int numTab;
//};