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

#include "NumCtrl.h"
#include "KaiCheckBox.h"
#include "MappedButton.h"
#include "KaiScrollBar.h"
#include <wx/wx.h>
#include "StylePreview.h"
#include "KaiDialog.h"

wxDECLARE_EVENT(FONT_CHANGED, wxCommandEvent);

class FontList : public wxWindow
{
public:
	FontList(wxWindow *parent, long id, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);
	virtual ~FontList();

	//void Insert(wxString facename,int pos);
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
	wxArrayString *fonts;

	KaiScrollbar *scrollBar;
	wxBitmap *bmp;
	wxFont font;

	DECLARE_EVENT_TABLE()
};

//for realtime change connect with event FONT_CHANGED
class FontDialog : public KaiDialog
{
public:
	virtual ~FontDialog();

	void GetStyles(Styles **inputStyle, Styles **outputStyle);
	Styles *GetFont();
	// class gets style and release it later
	static FontDialog *Get(wxWindow *parent, Styles *acstyl);
private:
	FontDialog(wxWindow *parent, Styles *acstyl);
	FontDialog(const FontDialog & copy) = delete;
	void SetStyle();
	FontList *Fonts;
	StylePreview *Preview;
	NumCtrl *FontSize;
	KaiCheckBox *Bold;
	KaiCheckBox *Italic;
	KaiCheckBox *Underl;
	KaiCheckBox *Strike;
	MappedButton *Buttok;
	MappedButton *Buttcancel;
	KaiTextCtrl *FontName;
	Styles *editedStyle = NULL;
	Styles *resultStyle = NULL;
	wxTimer fontChangedTimer;

	void OnUpdatePreview(wxCommandEvent& event);
	void OnFontChanged(wxCommandEvent& event);
	void OnUpdateText(wxCommandEvent& event);
	void OnScrollList(wxCommandEvent& event);
	void UpdatePreview();
	static FontDialog *FDialog;

};

class FontPickerButton : public MappedButton{
public:
	FontPickerButton(wxWindow *parent, int id, const wxFont& font,
			 const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);
	virtual ~FontPickerButton(){};
	wxFont GetSelectedFont();
	void OnClick(wxCommandEvent &evt);
private:
	void ChangeFont(const wxFont &font);
	wxDECLARE_ABSTRACT_CLASS(FontPickerButton);
};


enum{
	ID_FONTLIST=14567,
	ID_FONTSIZE1,
	ID_FONTATTR,
	ID_FONT_NAME,
	ID_SCROLL1,
	ID_SCROLLUP=30060,
	ID_SCROLLDOWN=30061
};
