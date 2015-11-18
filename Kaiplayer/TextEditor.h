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