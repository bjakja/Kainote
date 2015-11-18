#ifndef CTwindow_H
#define CTwindow_H
#include "TimeCtrl.h"
#include "timeconv.h"




	#include <wx/textctrl.h>
	#include <wx/checkbox.h>
	#include <wx/statbox.h>
	#include <wx/radiobut.h>
	#include <wx/button.h>
	#include <wx/window.h>
class kainoteFrame;

class CTwindow: public wxWindow
{
	public:

		CTwindow(wxWindow* parent,kainoteFrame* kfparent,wxWindowID id=-1,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize,long style=0);
		virtual ~CTwindow();

		wxRadioButton* RadioButton9;
		wxRadioButton* RadioButton1;
		wxRadioButton* RadioButton12;
		wxRadioButton* RadioButton2;
		wxRadioButton* RadioButton6;
		wxRadioButton* RadioButton5;
		wxButton* Button1;
		wxRadioButton* RadioButton4;
		wxRadioButton* RadioButton7;
		wxStaticBox* StaticBox1;
		wxRadioButton* RadioButton3;
		wxButton* Button2;
		wxStaticBox* StaticBox2;
		wxButton* Button3;
		wxRadioButton* RadioButton8;
		TimeCtrl* timetext;
		wxRadioButton* RadioButton10;
		wxTextCtrl* TextCtrl2;
		wxStaticBox* StaticBox3;
		wxCheckBox* videotime;
		wxRadioButton* RadioButton11;
		wxPanel* panel;
        wxScrollBar* scrollBar;
		void Contents();

       
	
		
		

	private:

        STime ct;
        wxString form;
		kainoteFrame* Kai;
		int scpos;
		
		void OnOKClick(wxCommandEvent& event);
		void OnMouseEvent(wxMouseEvent& event);
		void OnRadioButton1Select(wxCommandEvent& event);
		void OnRadioButton2Select(wxCommandEvent& event);
		void OnButton1Click(wxCommandEvent& event);
		void OnSize(wxSizeEvent& event);
        void OnScroll(wxScrollEvent& event);
		void DoTooltips();
	
DECLARE_EVENT_TABLE()
};



enum{
		ID_RADIOBUTTON1=11134,
		ID_RADIOBUTTON2,
		style,
		ID_CLOSE,
		ID_SCROLL,
		ID_MOVE
};

#endif