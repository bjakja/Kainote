#ifndef EDITBOX_H_INCLUDED
#define EDITBOX_H_INCLUDED

#pragma once

#include <wx/wx.h>
#include "TimeCtrl.h"
#include "SubsDialogue.h"
#include "MyTextEditor.h"
#include "NumCtrl.h"
#include "AudioBox.h"
#include <d3dx9math.h>

class kainoteFrame;
class Grid;

class EBStaticText : public wxStaticText
{
public:
	EBStaticText(wxWindow *parent, const wxString &txt, const wxSize &size=wxDefaultSize);
	virtual ~EBStaticText(){};
	void OnEraseBackground(wxEraseEvent &event);
	DECLARE_EVENT_TABLE()
};

class DescTxtCtrl : public wxTextCtrl
{
public:
	DescTxtCtrl(wxWindow *parent, const wxSize &size, const wxString &desc);
	virtual ~DescTxtCtrl(){};
	void ChangeValue(wxString &val);
private:
	void OnFocus(wxFocusEvent &evt);
	void OnKillFocus(wxFocusEvent &evt);
	wxString description;
	DECLARE_EVENT_TABLE()
};

class txtdialog :public wxDialog
{
public:
	wxTextCtrl *txt;
	wxChoice *type;
	txtdialog(wxWindow *parent, int id, const wxString &txtt, int type, const wxPoint &position);
	virtual ~txtdialog(){};
};

class TagButton :public wxButton
{
public:
	TagButton(wxWindow *parent, int id, const wxString &name, const wxSize &size);
	virtual ~TagButton(){};
	
private:
	void OnMouseEvent(wxMouseEvent& event);
	txtdialog *tagtxt;
	wxString tag;
	int type;
	DECLARE_EVENT_TABLE()
};


class EditBox : public wxWindow
{
public:
	EditBox(wxWindow *parent, Grid *grid1, kainoteFrame *kaif, int idd);
	virtual ~EditBox();
	void SetIt(int Row, bool setaudio=true, bool save=true, bool nochangeline=false);
	void SetTl(bool tl);
	void Send(bool selline=true, bool dummy=false, bool visualdummy=false);
	void RefreshStyle(bool resetline=false);
	bool FindVal(wxString wval, wxString *returnval, wxString text="", bool *endsel=0);
	void HideControls();
	void UpdateChars(wxString text);

	Grid *grid;
	int ebrow;

	AudioBox* ABox;
	MTextEditor* TextEdit;
	MTextEditor* TextEditTl;
	wxCheckBox* TlMode;
	wxCheckBox* Comment;
	NumCtrl* LayerEdit;
	TimeCtrl* StartEdit;
	TimeCtrl* EndEdit;
	TimeCtrl* DurEdit;
	wxChoice* StyleChoice;
	DescTxtCtrl* ActorEdit;
	NumCtrl* MarginLEdit;
	NumCtrl* MarginREdit;
	NumCtrl* MarginVEdit;
	DescTxtCtrl* EffectEdit;
	EBStaticText *Chars;
	EBStaticText *Chtime;
	wxButton* Bfont;
	wxButton* Bcol1;
	wxButton* Bcol2;
	wxButton* Bcol3;
	wxButton* Bcol4;
	wxButton* Bbold;
	wxButton* Bital;
	wxButton* Bund;
	wxButton* Bstrike;
	wxButton* Bcpall;
	wxButton* Bcpsel;
	wxButton* Bhide;
	wxToggleButton* AutoMoveTags;
	wxChoice* Ban;


	void PutinText(wxString text, bool focus=true, bool onlysel=false, wxString *texttoPutin=0);
	void PutinNonass(wxString text, wxString tag);
	//ustawia tekst i gdy trzeba wstawia tagi z orygina³u ustawiaj¹c w³aœciw¹ pozycjê kursora.
	void SetTextWithTags();
	void ClearErrs();
	void OnEdit(wxCommandEvent& event);

	wxBoxSizer* BoxSizer1;

	Dialogue *line;
	wxPoint Placed;
	bool InBracket;
	bool splittedTags;
	bool OnVideo;
	int Visual;

private:
	
	
	wxString lasttag;
	int cursorpos;

	wxBoxSizer* BoxSizer2;
	wxBoxSizer* BoxSizer3;
	wxBoxSizer* BoxSizer4;
	wxBoxSizer* BoxSizer5;
	wxBoxSizer* BoxSizer6;

	void OnNewline(wxCommandEvent& event);
	void OnCommit(wxCommandEvent& event);
	void OnFontClick(wxCommandEvent& event);
	void OnColorClick(wxCommandEvent& event);
	void AllColClick(int kol);
	void OnBoldClick(wxCommandEvent& event);
	void OnItalClick(wxCommandEvent& event);
	void OnUndClick(wxCommandEvent& event);
	void OnStrikeClick(wxCommandEvent& event);
	void OnAnChoice(wxCommandEvent& event);
	void OnTlMode(wxCommandEvent& event);
	void OnCpAll(wxCommandEvent& event);
	void OnCpSel(wxCommandEvent& event);
	void OnReplClick(wxCommandEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnSplit(wxCommandEvent& event);
	void OnHideOrig(wxCommandEvent& event);
	void OnPasteDiff(wxCommandEvent& event);
	void OnColorChange(wxCommandEvent& event);
	void OnButtonTag(wxCommandEvent& event);
	void OnCursorMoved(wxCommandEvent& event);
	void OnAutoMoveTags(wxCommandEvent& event);
	void DoTooltips();

	bool isdetached;
	wxMutex mutex;
	wxString num;
};



enum{
	ID_CHECKBOX1=3989,
	IDSTYLE,
	ID_FONT,
	ID_UND,
	ID_STRIKE,
	ID_AN,
	ID_TLMODE,
	ID_CPALL,
	ID_CPSEL,
	ID_HIDE,
	ID_COL1,
	ID_COL2,
	ID_COL3,
	ID_COL4,
	MENU_ZATW,
	MENU_NLINE,
	ID_AUTOMOVETAGS
	
};

#endif // EDITBOX_H_INCLUDED
