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

#ifndef EDITBOX_H_INCLUDED
#define EDITBOX_H_INCLUDED

#pragma once

#include <wx/window.h>
#include <wx/stattext.h>
#include "TimeCtrl.h"
#include "SubsDialogue.h"
#include "MyTextEditor.h"
#include "NumCtrl.h"
#include "ListControls.h"
#include "AudioBox.h"
#include "MappedButton.h"
#include "KaiRadioButton.h"
#include "KaiDialog.h"

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

class DescTxtCtrl : public KaiChoice
{
public:
	DescTxtCtrl(wxWindow *parent, int id, const wxSize &size, const wxString &desc, const wxValidator &validator = wxDefaultValidator);
	virtual ~DescTxtCtrl(){};
	void ChangeValue(wxString &val);
private:
	void OnFocus(wxFocusEvent &evt);
	void OnKillFocus(wxFocusEvent &evt);
	wxString description;

};

class txtdialog :public KaiDialog
{
public:
	KaiTextCtrl *txt;
	KaiChoice *type;
	txtdialog(wxWindow *parent, int id, const wxString &txtt, int type, const wxPoint &position);
	virtual ~txtdialog(){};
};

class TagButton :public MappedButton
{
public:
	TagButton(wxWindow *parent, int id, const wxString &name, wxString tooltip, const wxSize &size);
	virtual ~TagButton(){};
	
private:
	void OnMouseEvent(wxMouseEvent& event);
	txtdialog *tagtxt;
	wxString tag;
	int type;
	//DECLARE_EVENT_TABLE()
};


class EditBox : public wxWindow
{
public:
	EditBox(wxWindow *parent, Grid *grid1, kainoteFrame *kaif, int idd);
	virtual ~EditBox();
	void SetLine(int Row, bool setaudio=true, bool save=true, bool nochangeline=false, bool autoPlay = false);
	void SetTl(bool tl);
	void Send(bool selline=true, bool dummy=false, bool visualdummy=false);
	void RefreshStyle(bool resetline=false);
	bool FindVal(const wxString &wval, wxString *returnval, const wxString &text="", bool *endsel=0, bool fromstart=false);
	void HideControls();
	void UpdateChars(const wxString &text);

	Grid *grid;
	int ebrow;

	AudioBox* ABox;
	MTextEditor* TextEdit;
	MTextEditor* TextEditTl;
	KaiCheckBox* TlMode;
	KaiRadioButton* Times;
	KaiRadioButton* Frames;
	KaiCheckBox* Comment;
	NumCtrl* LayerEdit;
	TimeCtrl* StartEdit;
	TimeCtrl* EndEdit;
	TimeCtrl* DurEdit;
	KaiChoice* StyleChoice;
	DescTxtCtrl* ActorEdit;
	NumCtrl* MarginLEdit;
	NumCtrl* MarginREdit;
	NumCtrl* MarginVEdit;
	DescTxtCtrl* EffectEdit;
	EBStaticText *Chars;
	EBStaticText *Chtime;
	MappedButton* StyleEdit;
	MappedButton* Bfont;
	MappedButton* Bcol1;
	MappedButton* Bcol2;
	MappedButton* Bcol3;
	MappedButton* Bcol4;
	MappedButton* Bbold;
	MappedButton* Bital;
	MappedButton* Bund;
	MappedButton* Bstrike;
	MappedButton* Bcpall;
	MappedButton* Bcpsel;
	MappedButton* Bhide;
	ToggleButton* DoubtfulTL;
	ToggleButton* AutoMoveTags;
	KaiChoice* Ban;


	void PutinText(const wxString &text, bool focus=true, bool onlysel=false, wxString *texttoPutin=0);
	void PutinNonass(const wxString &text, const wxString &tag);
	//ustawia tekst i gdy trzeba wstawia tagi z orygina³u ustawiaj¹c w³aœciw¹ pozycjê kursora.
	void SetTextWithTags();
	void ClearErrs();
	void OnEdit(wxCommandEvent& event);
	bool SetBackgroundColour(const wxColour &col);
	bool IsCursorOnStart();
	void FindNextDoubtfulTl(wxCommandEvent& event);
	void FindNextUnTranslated(wxCommandEvent& event);

	wxBoxSizer* BoxSizer1;

	Dialogue *line;
	wxPoint Placed;
	bool InBracket;
	bool splittedTags;
	bool OnVideo;
	bool lastVisible;
	int Visual;
	int EditCounter;

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
	void AllColorClick(int kol);
	void OnBoldClick(wxCommandEvent& event);
	void OnItalicClick(wxCommandEvent& event);
	void OnUnderlineClick(wxCommandEvent& event);
	void OnStrikeClick(wxCommandEvent& event);
	void OnAnChoice(wxCommandEvent& event);
	void OnTlMode(wxCommandEvent& event);
	void OnCopyAll(wxCommandEvent& event);
	void OnCopySelection(wxCommandEvent& event);
	void OnDoubtfulTl(wxCommandEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnSplit(wxCommandEvent& event);
	void OnHideOriginal(wxCommandEvent& event);
	void OnPasteDifferents(wxCommandEvent& event);
	void OnColorChange(wxCommandEvent& event);
	void OnButtonTag(wxCommandEvent& event);
	void OnCursorMoved(wxCommandEvent& event);
	void OnAutoMoveTags(wxCommandEvent& event);
	void OnChangeTimeDisplay(wxCommandEvent& event);
	void OnStyleEdit(wxCommandEvent& event);
	void DoTooltips();

	bool isdetached;
	wxMutex mutex;
	wxString num;
	int CurrentDoubtful;
	int CurrentUntranslated;
};


//uwaga nie zmieniaæ kolejnoœci bo szlag trafi¹ skróty zapisane w hotkeys.txt
enum{
	ID_COMMENT=3989,
	ID_STYLE,
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
	MENU_COMMIT,
	MENU_NEWLINE,
	ID_DOUBTFULTL,
	ID_AUTOMOVETAGS,
	ID_TIMES_FRAMES,
	
};

#endif // EDITBOX_H_INCLUDED
