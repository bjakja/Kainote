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
#include "KaiStaticText.h"
#include "MenuButton.h"

class SubsGrid;


class DescTxtCtrl : public KaiChoice
{
public:
	DescTxtCtrl(wxWindow *parent, int id, const wxSize &size, const wxString &desc, const wxValidator &validator = wxDefaultValidator);
	virtual ~DescTxtCtrl(){};
	void ChangeValue(const wxString &val);
private:
	void OnFocus(wxFocusEvent &evt);
	void OnKillFocus(wxFocusEvent &evt);
	wxString description;

};

class txtdialog :public KaiDialog
{
public:
	KaiTextCtrl *txt;
	KaiTextCtrl *name;
	KaiChoice *type;
	txtdialog(wxWindow *parent, int id, const wxString &txtt, const wxString &_name, int type);
	virtual ~txtdialog(){};
};

class TagButton :public MappedButton
{
public:
	TagButton(wxWindow *parent, int id, const wxString &name, const wxString &tag, int type, const wxSize &size);
	virtual ~TagButton(){};
	wxString tag;
	wxString name;
	int type;
private:
	void OnMouseEvent(wxMouseEvent& event);
	txtdialog *tagtxt;
	
};


class EditBox : public wxWindow
{
public:
	EditBox(wxWindow *parent, SubsGrid *grid, int idd);
	virtual ~EditBox();
	void SetLine(int Row, bool setaudio=true, bool save=true, bool nochangeline=false, bool autoPlay = false);
	void SetTl(bool tl);
	void Send(unsigned char editionType, bool selline=true, bool dummy=false, bool visualdummy=false);
	void RefreshStyle(bool resetline=false);
	bool FindVal(const wxString &wval, wxString *returnval, const wxString &text="", bool *endsel=0, bool fromstart=false);
	void HideControls();
	void UpdateChars(const wxString &text);

	AudioBox* ABox;
	MTextEditor* TextEdit;
	MTextEditor* TextEditOrig;
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
	KaiStaticText *Chars;
	KaiStaticText *Chtime;
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
	MenuButton* TagButtonManager;
	ToggleButton* DoubtfulTL;
	ToggleButton* AutoMoveTags;
	KaiChoice* Ban;


	void PutinText(const wxString &text, bool focus=true, bool onlysel=false, wxString *texttoPutin=0);
	void PutinNonass(const wxString &text, const wxString &tag);
	//ustawia tekst i gdy trzeba wstawia tagi z orygina³u ustawiaj¹c w³aœciw¹ pozycjê kursora.
	void SetTextWithTags(bool RefreshVideo = false);
	void ClearErrs();
	void OnEdit(wxCommandEvent& event);
	bool SetBackgroundColour(const wxColour &col);
	bool IsCursorOnStart();
	void FindNextDoubtfulTl(wxCommandEvent& event);
	void FindNextUnTranslated(wxCommandEvent& event);
	void SetActiveLineToDoubtful();
	void SetGrid(SubsGrid *_grid, bool isPreview = false){
		grid = _grid;
		hasPreviewGrid = isPreview;
	}

	wxBoxSizer* BoxSizer1;

	Dialogue *line;
	wxPoint Placed;
	bool InBracket;
	bool splittedTags;
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

	void ChangeFont(Styles *retStyle, Styles *editedStyle);
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
	void OnEditTag(wxCommandEvent& event);
	void OnCursorMoved(wxCommandEvent& event);
	void OnAutoMoveTags(wxCommandEvent& event);
	void OnChangeTimeDisplay(wxCommandEvent& event);
	void OnStyleEdit(wxCommandEvent& event);
	void OnFontChange(wxCommandEvent& event);
	void SetTagButtons();
	void DoTooltips();

	bool isdetached;
	bool hasPreviewGrid = false;
	wxMutex mutex;
	wxString colorNumber;
	int CurrentDoubtful;
	int CurrentUntranslated;
	int currentLine;
	SubsGrid *grid;
};


//uwaga nie zmieniaæ kolejnoœci bo szlag trafi¹ skróty zapisane w hotkeys.txt
enum{
	ID_COMMENT=3979,
	ID_STYLE,
	ID_AN,
	ID_TLMODE,
	ID_DOUBTFULTL,
	ID_AUTOMOVETAGS,
	ID_TIMES_FRAMES,
	
};

