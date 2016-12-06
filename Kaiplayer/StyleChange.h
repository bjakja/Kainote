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

#ifndef COLORCHANGE_H
#define COLORCHANGE_H

	#include <wx/stattext.h>
	#include <wx/textctrl.h>
	#include <wx/checkbox.h>
	#include <wx/radiobut.h>
	#include <wx/statbox.h>
	#include <wx/dialog.h>
	#include "ListControls.h"
	#include "MappedButton.h"
    #include "StylePreview.h"
    #include "stylestore.h"
    #include "NumCtrl.h"

bool sortf(wxString name1,wxString name2);

class ColorChange: public wxWindow
{
	public:

		ColorChange(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ColorChange();

		wxCheckBox* sob;
		wxRadioButton* rb1;
		wxRadioButton* rb2;
		wxRadioButton* rb3;
		wxRadioButton* rb4;
        wxRadioButton* rb5;
		wxRadioButton* rb6;
		wxRadioButton* rb7;
		wxRadioButton* rb8;
		wxRadioButton* rb9;
		wxCheckBox* si;
		MappedButton* btnOk;
		KaiTextCtrl* sname;
		MappedButton* s2;
		MappedButton* s3;
		NumCtrl* ssize;
		MappedButton* btnCommit;
		MappedButton* btnCancel;
		MappedButton* btnFullscreen;
		wxComboBox* sfont;
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
		wxCheckBox* ss;
		wxCheckBox* su;
		wxCheckBox* sb;
		KaiChoice* senc;
		NumCtrl* smr;
		NumCtrl* smv;
		NumCtrl* sml;
		StylePreview* Preview;

		void UpdateValues(Styles *styl);
		void UpdatePreview();

		stylestore* SS;
		void OnOKClick(wxCommandEvent& event);
		void OnCancelClick(wxCommandEvent& event);
		bool block;
	
		

	private:

		void OnAllCols(int kol);
		void Ons1Click(wxCommandEvent& event);
		void Ons2Click(wxCommandEvent& event);
		void Ons3Click(wxCommandEvent& event);
		void Ons4Click(wxCommandEvent& event);
		void OnStyleVideo(wxCommandEvent& event);
		void OnStyleFull(wxCommandEvent& event);
		void UpdateStyle();
		void OnUpdatePreview(wxCommandEvent& event);
		
		void OnSetFocus(wxFocusEvent& event);

		void DoTooltips();
        //DialogColorPicker* ColourDialog1;

		Styles *tab;
		wxArrayString encs;
		
		
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
	ID_BONVID,
	ID_BONFULL
	};

#endif
