#ifndef EDITBOX_H_INCLUDED
#define EDITBOX_H_INCLUDED

#pragma once

#include <wx/wx.h>
#include "TimeCtrl.h"
#include "SubsDialogue.h"
#include "MyTextEditor.h"
#include "NumCtrl.h"
#include "audio_box.h"
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
	D3DXVECTOR2 GetPosnScale(D3DXVECTOR2 *scale, byte *an, bool beforeCursor=false, bool draw=true);

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
	wxChoice* Ban;


	void PutinText(wxString text, bool focus=true, bool onlysel=false);
	void PutinNonass(wxString text, wxString tag);
	wxString GetTags(const wxString &text) const;
	void ClearErrs();
	wxString GetVisual(int type=0);
	void SetClip(wxString clip,bool dummy);
	void SetVisual(wxString visual,bool dummy,int type);
	wxString *dummytext;

	wxBoxSizer* BoxSizer1;

	Dialogue *line;


	bool OnVideo;
	int Visual;

private:

	wxString lasttag;
	bool InBracket;
	wxPoint Placed;
	wxPoint dumplaced;
	int cursorpos;

	wxBoxSizer* BoxSizer2;
	wxBoxSizer* BoxSizer3;
	wxBoxSizer* BoxSizer4;
	wxBoxSizer* BoxSizer5;
	wxBoxSizer* BoxSizer6;

	void OnNline(wxCommandEvent& event);
	void OnZatw(wxCommandEvent& event);
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
	void OnEdit(wxCommandEvent& event);
	void OnColorChange(wxCommandEvent& event);
	void OnButtonTag(wxCommandEvent& event);
	void OnCursorMoved(wxCommandEvent& event);
	void DoTooltips();

	bool isdetached;
	wxMutex mutex;
	wxString num;
};



enum{
	ID_CHECKBOX1=3989,
	IDSTYLE,
	ID_FONT,
	ID_BOLD,
	ID_ITAL,
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
	ID_SPLIT,
	ID_STARTDIFF,
	ID_ENDDIFF
};

#endif // EDITBOX_H_INCLUDED
