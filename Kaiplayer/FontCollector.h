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

