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

	#include <wx/stattext.h>
	#include "ListControls.h"
	#include "MappedButton.h"
    #include "StylePreview.h"
    #include "StyleStore.h"
    #include "NumCtrl.h"
	#include "KaiRadioButton.h"
	#include "KaiDialog.h"

//bool sortf(wxString name1,wxString name2);


class StyleChange: public wxWindow
{
	public:

		StyleChange(wxWindow* parent, bool window=true, const wxPoint& pos=wxDefaultPosition);
		virtual ~StyleChange();

		KaiCheckBox* sob;
		KaiRadioButton* rb1;
		KaiRadioButton* rb2;
		KaiRadioButton* rb3;
		KaiRadioButton* rb4;
        KaiRadioButton* rb5;
		KaiRadioButton* rb6;
		KaiRadioButton* rb7;
		KaiRadioButton* rb8;
		KaiRadioButton* rb9;
		KaiCheckBox* si;
		MappedButton* btnOk;
		KaiTextCtrl* sname;
		MappedButton* s2;
		MappedButton* s3;
		NumCtrl* ssize;
		MappedButton* btnCommit;
		MappedButton* btnCancel;
		MappedButton* btnCommitOnStyles;
		KaiChoice* sfont;
		KaiTextCtrl *fontFilter;
		ToggleButton *Filter;
		NumCtrl* alpha1;
		NumCtrl* alpha2;
		NumCtrl* alpha3;
		NumCtrl* alpha4;
		NumCtrl* ssx;
		NumCtrl* ssy;
		NumCtrl* sou;
		NumCtrl* ssh;
		NumCtrl* san;
		NumCtrl* ssp;
		MappedButton* s4;
		MappedButton* s1;
		KaiCheckBox* ss;
		KaiCheckBox* su;
		KaiCheckBox* sb;
		KaiChoice* senc;
		NumCtrl* smr;
		NumCtrl* smv;
		NumCtrl* sml;
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

	private:

		void OnAllCols(int kol);
		void Ons1Click(wxCommandEvent& event);
		void Ons2Click(wxCommandEvent& event);
		void Ons3Click(wxCommandEvent& event);
		void Ons4Click(wxCommandEvent& event);
		void OnChangeAllSelectedStyles(wxCommandEvent& event);
		void OnCommit(wxCommandEvent& event);
		void UpdateStyle();
		void OnUpdatePreview(wxCommandEvent& event);
		
		void OnSetFocus(wxFocusEvent& event);

		void DoTooltips();
        
		Styles *tab;
		Styles *CompareStyle = NULL;
		wxArrayString encs;
		KaiDialog *SCD;
		
};


enum{
	ID_FONTNAME=23435,
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


