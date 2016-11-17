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

#ifndef TEXTEDITOR
#define TEXTEDITOR


#include <wx/wx.h>
#include <wx/listbox.h>


class EditBox;
class kainoteFrame;

class listwindow : public wxDialog
	{
	public:
		listwindow(wxWindow *parent, wxArrayString suggest, const wxPoint& pos, wxString name="Sugestie");
		virtual ~listwindow();

		wxListBox *disperrs;
		void OnDoubleClick(wxCommandEvent& event);
	};

class TextEditor : public wxTextCtrl
	{
	public:
	TextEditor(wxWindow *parent, const long int id, bool spellchecker=true, const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize, long style=wxWANTS_CHARS, const wxString& name=wxPanelNameStr); 
	virtual ~TextEditor();

	void SetTextS(wxString text, bool Modif=false);
	void CheckText(bool rpnt=true);
    EditBox* EB;
	kainoteFrame* Kai;
	bool spell;
	bool modified;
	wxArrayString errs;
	wxString addw;
	int fsize;
	bool Modified();
	bool block;
	
	private:
	
	void OnKeyPress(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);
	void OnWrite(wxCommandEvent& event);
	
	void DoPaste1();
	void OnStartSpell(wxTimerEvent& event);
	void OnMouseEvent(wxMouseEvent& event);
	void OnSpellErrs(int elem, wxPoint mpos);
	void OnSugestion(wxCommandEvent& event);
	void OnAddWord(wxCommandEvent& event);
	void ContextMenu(wxPoint pos, int Error= -1);
	
	
	
	wxTimer spblock;
	
	DECLARE_EVENT_TABLE()
	};

enum{
	TEXTM_COPY=16545,
	TEXTM_PASTE,
	TEXTM_CUT,
	TEXTM_DEL,
	TEXTM_ADD
};

#endif