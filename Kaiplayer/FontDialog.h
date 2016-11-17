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

#ifndef FONTDIALOG
#define FONTDIALOG


#include "NumCtrl.h"
#include <wx/wx.h>
#include "StylePreview.h"


class FontList : public wxWindow
{
public:
	FontList(wxWindow *parent, long id, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);
	virtual ~FontList();

	void Insert(wxString facename,int pos);
	void SetSelection(int pos);
	void SetSelectionByName(wxString name);
	void SetSelectionByPartialName(wxString PartialName);
	void Scroll(int step);
	wxString GetString(int line);
	int GetSelection();

private:

	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);
	void DrawFld(wxDC &dc,int w, int h);
	void OnScroll(wxScrollEvent& event);
	void OnMouseEvent(wxMouseEvent& event);


	int sel;
	int scPos;
	int Height;
	bool holding;
	wxArrayString fonts;

	wxScrollBar *scrollBar;
	wxBitmap *bmp;
	wxFont font;

	DECLARE_EVENT_TABLE()
};

class FontDialog : public wxDialog
{
public:
	FontDialog(wxWindow *parent, Styles *acstyl);
	virtual ~FontDialog();

	Styles *GetFont();

private:
	FontList *Fonts;
	StylePreview *Preview;
	NumCtrl *FontSize;
	wxCheckBox *Bold;
	wxCheckBox *Italic;
	wxCheckBox *Underl;
	wxCheckBox *Strike;
	wxButton *Buttok;
	wxButton *Buttcancel;
	wxTextCtrl *FontName;

	void OnUpdatePreview(wxCommandEvent& event);
	void OnFontChanged(wxCommandEvent& event);
	void OnUpdateText(wxCommandEvent& event);
	void OnScrollList(wxCommandEvent& event);
	void UpdatePreview();


};

enum{
	ID_FONTLIST=14567,
	ID_FONTSIZE1,
	ID_FONTATTR,
	ID_FONTNAME,
	ID_SCROLL1,
	ID_SCROLLUP=30060,
	ID_SCROLLDOWN=30061
};
#endif