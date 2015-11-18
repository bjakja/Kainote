#ifndef FINDREPLACE_H
#define FINDREPLACE_H

#pragma once


#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/radiobut.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/wx.h>
class kainoteFrame;


class findreplace: public wxDialog
{
	public:

		findreplace(kainoteFrame* kfparent, findreplace* last, bool replace, bool sellines=false);
		virtual ~findreplace();

        kainoteFrame* Kai;
		
		
		wxButton* Button4;
		wxRadioButton* RadioButton1;
		wxRadioButton* RadioButton2;
		wxRadioButton* RadioButton3;
		wxRadioButton* RadioButton4;
		wxRadioButton* RadioButton5;
		wxRadioButton* RadioButton6;
		wxRadioButton* RadioButton7;
		wxButton* Button1;
		wxButton* Bplus;
		
		wxCheckBox* MatchCase;
		wxCheckBox* RegEx;
		wxCheckBox* StartLine;
		wxCheckBox* EndLine;
		wxButton* Button2;
		wxButton* Button3;
		wxComboBox* FindText;
		wxComboBox* RepText;
		wxTextCtrl* tcstyle;
		wxCheckBox *Fdial;
		wxCheckBox *Fcomm;
		wxRadioBox *Actions;
		wxRadioBox *Selections;
	
		void ReloadStyle();
		void SelectLine();
		void AddRecent();
		void OnStylesWin(wxCommandEvent& event);
		void OnSelections(wxCommandEvent& event);
		void OnStylesWin1(wxCommandEvent& event);
		void Reset();
		bool repl;
	private:
        int posrow;
		int reprow;
        int postxt;
		int findstart;
		int findend;
        bool fnext;
		
		bool fromstart;
        void Find();
		
		void OnReplaceAll(wxCommandEvent& event);
		void OnButtonFind(wxCommandEvent& event);
		void OnButtonRep(wxCommandEvent& event);
		void OnTextUpdate(wxCommandEvent& event);
		void OnClose(wxCommandEvent& event);
		void OnRecheck(wxCommandEvent& event);
};

enum
	{
	ID_BREP=13737,
	ID_BREPALL,
	ID_BFIND,
	ID_BCLOSE,
	ID_BPLUS,
	ID_TCSTYLE,
	ID_FINDTEXT,
	ID_REPTEXT,
	ID_SLINE,
	ID_ELINE
	};

#endif