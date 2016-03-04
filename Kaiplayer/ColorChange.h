#ifndef COLORCHANGE_H
#define COLORCHANGE_H

	#include <wx/stattext.h>
	#include <wx/textctrl.h>
	#include <wx/checkbox.h>
	#include <wx/radiobut.h>
	#include <wx/statbox.h>
	#include <wx/choice.h>
	#include <wx/button.h>
	#include <wx/dialog.h>
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
		wxButton* Button1;
		wxTextCtrl* sname;
		wxButton* s2;
		wxButton* s3;
		NumCtrl* ssize;
		wxButton* Button2;
		wxButton* Button3;
		wxButton* Button4;
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
		wxButton* s4;
		wxButton* s1;
		wxCheckBox* ss;
		wxCheckBox* su;
		wxCheckBox* sb;
		wxChoice* senc;
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
