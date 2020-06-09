//  Copyright (c) 2016 - 2020, Marcin Drob

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

#include <wx/stattext.h>
#include "ListControls.h"
#include "MappedButton.h"
#include "StylePreview.h"
#include "StyleStore.h"
#include "NumCtrl.h"
#include "KaiRadioButton.h"
#include "KaiDialog.h"

//bool sortf(wxString name1,wxString name2);


class StyleChange : public wxWindow
{
public:

	StyleChange(wxWindow* parent, bool window = true, const wxPoint& pos = wxDefaultPosition);
	virtual ~StyleChange();

	KaiCheckBox* borderStyle;
	KaiRadioButton* alignment1;
	KaiRadioButton* alignment2;
	KaiRadioButton* alignment3;
	KaiRadioButton* alignment4;
	KaiRadioButton* alignment5;
	KaiRadioButton* alignment6;
	KaiRadioButton* alignment7;
	KaiRadioButton* alignment8;
	KaiRadioButton* alignment9;
	KaiCheckBox* textItalic;
	MappedButton* btnOk;
	KaiTextCtrl* styleName;
	MappedButton* color2;
	MappedButton* color3;
	NumCtrl* fontSize;
	MappedButton* btnCommit;
	MappedButton* btnCancel;
	KaiChoice* styleFont;
	KaiTextCtrl *fontFilter;
	ToggleButton *Filter;
	NumCtrl* alpha1;
	NumCtrl* alpha2;
	NumCtrl* alpha3;
	NumCtrl* alpha4;
	NumCtrl* scaleX;
	NumCtrl* scaleY;
	NumCtrl* outline;
	NumCtrl* shadow;
	NumCtrl* angle;
	NumCtrl* spacing;
	MappedButton* color4;
	MappedButton* color1;
	KaiCheckBox* textStrikeout;
	KaiCheckBox* textUnderline;
	KaiCheckBox* textBold;
	KaiChoice* textEncoding;
	NumCtrl* rightMargin;
	NumCtrl* verticalMargin;
	NumCtrl* leftMargin;
	StylePreview* Preview;
	bool allowMultiEdition = true;


	void UpdateValues(Styles *styl, bool allowMultiEdition, bool enableNow);
	void UpdatePreview();

	StyleStore* SS;
	void OnOKClick(wxCommandEvent& event);
	void OnCancelClick(wxCommandEvent& event);
	bool block;
	bool Show(bool show = true);
	bool Destroy();
	bool IsShown();
	bool SetFont(const wxFont &font);

private:

	void OnAllCols(int kol, bool leftClick = true);
	void OnColor1Click(wxCommandEvent& event);
	void OnColor2Click(wxCommandEvent& event);
	void OnColor3Click(wxCommandEvent& event);
	void OnColor4Click(wxCommandEvent& event);
	void OnColor1RightClick(wxMouseEvent& event);
	void OnColor2RightClick(wxMouseEvent& event);
	void OnColor3RightClick(wxMouseEvent& event);
	void OnColor4RightClick(wxMouseEvent& event);
	void OnCommit(wxCommandEvent& event);
	void UpdateStyle();
	void OnUpdatePreview(wxCommandEvent& event);

	void OnSetFocus(wxFocusEvent& event);

	void DoTooltips();
	void GetColorControls(MappedButton** color, NumCtrl** alpha, int numColor);
	void UpdateColor(const AssColor &color, int numColor);
	void CommitChange(bool close);
	void CloseWindow();

	Styles *updateStyle;
	Styles *CompareStyle = NULL;
	AssColor lastColor;
	wxArrayString encs;
	KaiDialog *SCD;

};


enum{
	ID_FONTNAME = 23435,
	ID_FONTSIZE,
	ID_BCOLOR1,
	ID_BCOLOR2,
	ID_BCOLOR3,
	ID_BCOLOR4,
	ID_SALPHA1,
	ID_SALPHA2,
	ID_SALPHA3,
	ID_SALPHA4,
	ID_CBOLD,
	ID_CITAL,
	ID_CUNDER,
	ID_CSTRIKE,
	ID_TOUTLINE,
	ID_TSHADOW,
	ID_TSCALEX,
	ID_TSCALEY,
	ID_TANGLE,
	ID_TSPACING,
	ID_COBOX,
	ID_RAN1,
	ID_RAN2,
	ID_RAN3,
	ID_RAN4,
	ID_RAN5,
	ID_RAN6,
	ID_RAN7,
	ID_RAN8,
	ID_RAN9,
	ID_TMARGINL,
	ID_TMARGINR,
	ID_TMARGINV,
	ID_CENCODING,
	ID_BOK,
	ID_BCANCEL,
	ID_B_CHANGE_ALL_SELECTED_STYLES,
	ID_B_COMMIT
};


