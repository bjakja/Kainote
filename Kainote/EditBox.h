//  Copyright (c) 2016-2021, Marcin Drob

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

#include "TimeCtrl.h"
#include "SubsDialogue.h"
#include "DialogueTextEditor.h"
#include "NumCtrl.h"
#include "ListControls.h"
#include "AudioBox.h"
#include "MappedButton.h"
#include "KaiRadioButton.h"
#include "KaiDialog.h"
#include "KaiStaticText.h"
#include "MenuButton.h"
#include "ColorPicker.h"
#include "KaiWindowResizer.h"
#include "KaiPanel.h"
#include "TagFindReplace.h"


class SubsGrid;
class TabPanel;


class ComboBoxCtrl : public KaiChoice
{
public:
	ComboBoxCtrl(wxWindow *parent, int id, const wxSize &size, const wxString &desc, const wxValidator &validator = wxDefaultValidator);
	virtual ~ComboBoxCtrl(){};
	void ChangeValue(const wxString &val);
private:
	void OnFocus(wxFocusEvent &evt);
	void OnKillFocus(wxFocusEvent &evt);
	wxString description;

};

class TagButtonDialog :public KaiDialog
{
public:
	KaiTextCtrl *txt;
	KaiTextCtrl *name;
	KaiChoice *type;
	TagButtonDialog(wxWindow *parent, int id, const wxString &txtt, const wxString &_name, int type);
	virtual ~TagButtonDialog(){};
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
	TagButtonDialog *tagtxt;
	
};


class EditBox : public KaiPanel, TagFindReplace
{
public:
	EditBox(wxWindow *parent, int idd);
	virtual ~EditBox();
	void SetLine(int Row, bool setaudio = true, bool save = true, bool nochangeline = false, bool autoPlay = false);
	void SetTlMode(bool tl, bool dummyTlMode = false);
	void Send(unsigned char editionType, bool selline = true, bool dummy = false, bool visualdummy = false);
	void RefreshStyle(bool resetline = false);
	void HideControls();
	void UpdateChars();

	AudioBox* ABox = nullptr;
	TextEditor* TextEdit = nullptr;
	TextEditor* TextEditOrig = nullptr;
	KaiCheckBox* TlMode = nullptr;
	KaiRadioButton* Times = nullptr;
	KaiRadioButton* Frames = nullptr;
	KaiCheckBox* Comment = nullptr;
	NumCtrl* LayerEdit = nullptr;
	TimeCtrl* StartEdit;
	TimeCtrl* EndEdit;
	TimeCtrl* DurEdit;
	KaiChoice* StyleChoice;
	ComboBoxCtrl* ActorEdit;
	NumCtrl* MarginLEdit;
	NumCtrl* MarginREdit;
	NumCtrl* MarginVEdit;
	ComboBoxCtrl* EffectEdit;
	KaiStaticText *LineNumber;
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

	//set text and needed tags from original and right position of cursor
	void SetTextWithTags(bool RefreshVideo = false);
	//second bool works only when spellcheckeronoff is true
	void ClearErrs(bool spellcheckerOnOff = false, bool enableSpellchecker = false);
	void OnEdit(wxCommandEvent& event);
	bool SetBackgroundColour(const wxColour &col);
	bool IsCursorOnStart();
	void FindNextDoubtfulTl(wxCommandEvent& event);
	void FindNextUnTranslated(wxCommandEvent& event);
	void RebuildActorEffectLists();
	void SetActiveLineToDoubtful();
	void SetGrid(SubsGrid *_grid, bool isPreview = false);
	void SetGrid1(SubsGrid* _grid) { grid = _grid; };
	bool LoadAudio(const wxString &audioFileName, bool fromVideo);
	void CloseAudio();
	TextEditor *GetEditor(const wxString &text = emptyString);
	bool SetFont(const wxFont &font);
	void OnAccelerator(wxCommandEvent& event);
	int GetFormat();

	wxBoxSizer* BoxSizer1 = nullptr;

	Dialogue *line = nullptr;
	bool splittedTags;
	bool lastVisible;
	int Visual;
	int EditCounter;
	KaiWindowResizer *windowResizer;
private:
	
	wxBoxSizer* BoxSizer2 = nullptr;
	wxBoxSizer* BoxSizer3 = nullptr;
	wxBoxSizer* BoxSizer4 = nullptr;
	wxBoxSizer* BoxSizer5 = nullptr;
	wxBoxSizer* BoxSizer6 = nullptr;

	void PutinNonass(const wxString& text, const wxString& tag);
	void ChangeFont(Styles *retStyle, Styles *editedStyle);
	void OnCommit(wxCommandEvent& event);
	void OnNewline(wxCommandEvent& event);
	void OnFontClick(wxCommandEvent& event);
	void OnColorClick(wxCommandEvent& event);
	void OnColorRightClick(wxMouseEvent& event);
	void AllColorClick(int kol, bool leftClick = true);
	void GetColor(AssColor *col, int numColor);
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
	void OnColorChange(ColorEvent& event);
	void OnButtonTag(wxCommandEvent& event);
	void OnEditTag(wxCommandEvent& event);
	void OnCursorMoved(wxCommandEvent& event);
	void OnAutoMoveTags(wxCommandEvent& event);
	void OnChangeTimeDisplay(wxCommandEvent& event);
	void OnStyleEdit(wxCommandEvent& event);
	void OnFontChange(wxCommandEvent& event);
	void SetTagButtons();
	void DoTooltips();
	wxPoint FindBrackets(const wxString & text, long from);
	void SwapRTLTextLines(wxString* text, int posOfSplit, const wxString & splitTag);
	AssColor actualColor;
	wxString actualColorstr;
	Styles actualStyle;
	bool isdetached;
	bool hasPreviewGrid = false;
	wxMutex mutex;
	int CurrentDoubtful;
	int CurrentUntranslated;
	int currentLine;
	SubsGrid *grid = nullptr;
	TabPanel *tab = nullptr;
};


//Warning cannot swap cause hotkeys from hotkeys.txt will stop working
enum{
	ID_COMMENT = 3979,
	ID_STYLE,
	ID_AN,
	ID_TLMODE,
	ID_DOUBTFULTL,
	ID_AUTOMOVETAGS,
	ID_TIMES_FRAMES,
	ID_NUM_TAG_BUTTONS = 16000,
	ID_NUM_CONTROL = 16668,
	ID_TEXT_EDITOR = 16667,
	ID_COMBO_BOX_CTRL = 16658,
	ID_EDIT_STYLE = 19989
};

