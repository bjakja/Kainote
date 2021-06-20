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


#include "EditBox.h"
#include "SubsGrid.h"
#include "KainoteApp.h"
#include <wx/regex.h>
//#include <wx/tglbtn.h>
#include "FontDialog.h"
#include "Visuals.h"
#include "KaiMessageBox.h"


ComboBoxCtrl::ComboBoxCtrl(wxWindow *parent, int id, const wxSize &size, const wxString &desc, const wxValidator &validator)
	:KaiChoice(parent, id, L"", wxDefaultPosition, size, wxArrayString(), 0, validator)
{
	description = desc;
	choiceText->Bind(wxEVT_SET_FOCUS, &ComboBoxCtrl::OnFocus, this);
	choiceText->Bind(wxEVT_KILL_FOCUS, &ComboBoxCtrl::OnKillFocus, this);
}

void ComboBoxCtrl::ChangeValue(const wxString &val)
{
	if (val == L"" && !choiceText->HasFocus()){
		SetForegroundColour(WINDOW_TEXT_INACTIVE);
		SetValue(description);
		Refresh(false);
	}
	else{
		SetForegroundColour(WINDOW_TEXT);
		SetValue(val);
		Refresh(false);
	}
}

void ComboBoxCtrl::OnFocus(wxFocusEvent &evt)
{
	if (choiceText->GetForegroundColour() == WINDOW_TEXT_INACTIVE){
		SetValue(L"");
		SetForegroundColour(WINDOW_TEXT);
	}
	evt.Skip();
}
void ComboBoxCtrl::OnKillFocus(wxFocusEvent &evt)
{
	if (GetValue() == L""){
		SetForegroundColour(WINDOW_TEXT_INACTIVE);
		SetValue(description);
	}
	evt.Skip();
}




TagButtonDialog::TagButtonDialog(wxWindow *parent, int id, const wxString &txtt, const wxString &_name, int _type)
	:KaiDialog(parent, id, _("Wpisz tag ASS"))
{
	DialogSizer *siz = new DialogSizer(wxVERTICAL);
	wxBoxSizer *siz1 = new wxBoxSizer(wxHORIZONTAL);
	wxString types[3] = { _("Tag wstawiany w miejsce kursora"), _("Tag wstawiany na początku tekstu"), _("Zwykły tekst") };
	type = new KaiChoice(this, -1, wxDefaultPosition, wxDefaultSize, 3, types);
	type->SetSelection(_type);
	name = new KaiTextCtrl(this, -1, _name, wxDefaultPosition, wxSize(150, 25), wxTE_PROCESS_ENTER);
	txt = new KaiTextCtrl(this, -1, txtt, wxDefaultPosition, wxSize(150, 25), wxTE_PROCESS_ENTER);
	txt->SetSelection(0, txtt.length());
	txt->SetFocus();
	siz->Add(type, 0, wxEXPAND | wxALL, 4);
	siz->Add(new KaiStaticText(this, -1, _("Nazwa przycisku")), 0, wxEXPAND | wxALL, 4);
	siz->Add(name, 0, wxEXPAND | wxLEFT | wxRIGHT, 4);
	siz->Add(new KaiStaticText(this, -1, _("Tag przycisku")), 0, wxEXPAND | wxALL, 4);
	siz->Add(txt, 0, wxEXPAND | wxLEFT | wxRIGHT, 4);
	siz1->Add(new MappedButton(this, wxID_OK, _("Zapisz tag")), 0, wxEXPAND | wxALL, 4);
	siz1->Add(new MappedButton(this, wxID_CANCEL, _("Anuluj")), 0, wxEXPAND | wxALL, 4);
	siz->Add(siz1, 0, wxEXPAND, 0);
	SetSizerAndFit(siz);
	MoveToMousePosition(this);

}
wxDEFINE_EVENT(TAG_BUTTON_EDITION, wxCommandEvent);

TagButton::TagButton(wxWindow *parent, int id, const wxString &_name, const wxString &_tag, int _type, const wxSize &size)
	: MappedButton(parent, id, _name, L"", wxDefaultPosition, size, EDITBOX_HOTKEY)
{
	wxString rest;
	type = _type;
	tag = _tag;
	if (tag != L""){
		SetToolTip(tag);
	}
	name = _name;
	Bind(wxEVT_LEFT_UP, &TagButton::OnMouseEvent, this);
	Bind(wxEVT_RIGHT_UP, &TagButton::OnMouseEvent, this);
}

void TagButton::OnMouseEvent(wxMouseEvent& event)
{
	if (event.RightUp() || (tag == L"" && event.LeftUp())){
		wxCommandEvent evt(TAG_BUTTON_EDITION, GetId());
		ProcessEvent(evt);
		SetFocus();
		return;
	}
	else if (event.LeftUp()){
		bool oldclicked = clicked;
		clicked = false;
		Refresh(false);
		if (oldclicked){
			wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, GetId());
			ProcessEvent(evt);
		}
		return;
	}

	event.Skip();
}



EditBox::EditBox(wxWindow *parent, int idd)
	: KaiPanel(parent, idd)
	, EditCounter(1)
	, ABox(NULL)
	, line(NULL)
	, lastVisible(true)
	, CurrentDoubtful(0)
	, CurrentUntranslated(0)
	, TagButtonManager(NULL)
{
	//Warning don't use any grid in constructor cause editbox is constructed
	//before Subsgrid cause of tab shortcut order
	tab = (TabPanel*)GetParent();
	SetTabPanel(tab);
	//SetForegroundColour(Options.GetColour(WINDOW_TEXT));
	SetBackgroundColour(Options.GetColour(WINDOW_BACKGROUND));
	wxWindow::SetFont(*Options.GetFont(-1));
	currentLine = 0;
	isdetached = splittedTags = false;
	Visual = 0;

	wxArrayString ans;
	ans.Add(L"an1");
	ans.Add(L"an2");
	ans.Add(L"an3");
	ans.Add(L"an4");
	ans.Add(L"an5");
	ans.Add(L"an6");
	ans.Add(L"an7");
	ans.Add(L"an8");
	ans.Add(L"an9");


	Bfont = new MappedButton(this, EDITBOX_CHANGE_FONT, L"", _("Wybór czcionki"), 
		wxDefaultPosition, wxDefaultSize, EDITBOX_HOTKEY, MAKE_SQUARE_BUTTON);
	Bfont->SetBitmap(wxBITMAP_PNG(L"FONTS"));
	Bbold = new MappedButton(this, EDITBOX_INSERT_BOLD, L"", _("Pogrubienie"),
		wxDefaultPosition, wxDefaultSize, EDITBOX_HOTKEY, MAKE_SQUARE_BUTTON);
	Bbold->SetBitmap(wxBITMAP_PNG(L"BOLD"));
	Bital = new MappedButton(this, EDITBOX_INSERT_ITALIC, L"", _("Pochylenie"),
		wxDefaultPosition, wxDefaultSize, EDITBOX_HOTKEY, MAKE_SQUARE_BUTTON);
	Bital->SetBitmap(wxBITMAP_PNG(L"ITALIC"));
	Bund = new MappedButton(this, EDITBOX_CHANGE_UNDERLINE, L"", _("Podkreślenie"),
		wxDefaultPosition, wxDefaultSize, EDITBOX_HOTKEY, MAKE_SQUARE_BUTTON);
	Bund->SetBitmap(wxBITMAP_PNG(L"UNDER"));
	Bstrike = new MappedButton(this, EDITBOX_CHANGE_STRIKEOUT, L"", _("Przekreślenie"),
		wxDefaultPosition, wxDefaultSize, EDITBOX_HOTKEY, MAKE_SQUARE_BUTTON);
	Bstrike->SetBitmap(wxBITMAP_PNG(L"STRIKE"));
	Bcol1 = new MappedButton(this, EDITBOX_CHANGE_COLOR_PRIMARY, L"", _("Kolor podstawowy"), 
		wxDefaultPosition, wxDefaultSize, EDITBOX_HOTKEY, MAKE_SQUARE_BUTTON);
	Bcol1->SetBitmap(wxBITMAP_PNG(L"Kolor1"));
	Bcol1->Bind(wxEVT_RIGHT_UP, &EditBox::OnColorRightClick, this);
	Bcol2 = new MappedButton(this, EDITBOX_CHANGE_COLOR_SECONDARY, L"", _("Kolor zastępczy do karaoke"), 
		wxDefaultPosition, wxDefaultSize, EDITBOX_HOTKEY, MAKE_SQUARE_BUTTON);
	Bcol2->SetBitmap(wxBITMAP_PNG(L"Kolor2"));
	Bcol2->Bind(wxEVT_RIGHT_UP, &EditBox::OnColorRightClick, this);
	Bcol3 = new MappedButton(this, EDITBOX_CHANGE_COLOR_OUTLINE, L"", _("Kolor obwódki"), 
		wxDefaultPosition, wxDefaultSize, EDITBOX_HOTKEY, MAKE_SQUARE_BUTTON);
	Bcol3->SetBitmap(wxBITMAP_PNG(L"Kolor3"));
	Bcol3->Bind(wxEVT_RIGHT_UP, &EditBox::OnColorRightClick, this);
	Bcol4 = new MappedButton(this, EDITBOX_CHANGE_COLOR_SHADOW, L"", _("Kolor cienia"), 
		wxDefaultPosition, wxDefaultSize, EDITBOX_HOTKEY, MAKE_SQUARE_BUTTON);
	Bcol4->SetBitmap(wxBITMAP_PNG(L"Kolor4"));
	Bcol4->Bind(wxEVT_RIGHT_UP, &EditBox::OnColorRightClick, this);
	
	Ban = new KaiChoice(this, ID_AN, wxDefaultPosition, wxDefaultSize, ans);
	Ban->Select(1);

	BoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
	BoxSizer4->Add(Bfont, 0, wxALL, 2);
	BoxSizer4->Add(Bbold, 0, wxALL, 2);
	BoxSizer4->Add(Bital, 0, wxALL, 2);
	BoxSizer4->Add(Bund, 0, wxALL, 2);
	BoxSizer4->Add(Bstrike, 0, wxALL, 2);
	BoxSizer4->Add(Bcol1, 0, wxALL, 2);
	BoxSizer4->Add(Bcol2, 0, wxALL, 2);
	BoxSizer4->Add(Bcol3, 0, wxALL, 2);
	BoxSizer4->Add(Bcol4, 0, wxALL, 2);
	BoxSizer4->Add(Ban, 0, wxALL, 2);

	SetTagButtons();

	TlMode = new KaiCheckBox(this, ID_TLMODE, _("Tryb tłumacza"));
	TlMode->SetValue(false);
	TlMode->Enable(false);
	LineNumber = new KaiStaticText(this, -1, _("Linia: 0"));
	Chars = new KaiStaticText(this, -1, _("Łamania: 0/86"));
	Chtime = new KaiStaticText(this, -1, _("Znaki na sekundę: 0<=15"));
	bool asFrames = Options.GetBool(EDITBOX_TIMES_TO_FRAMES_SWITCH);
	Times = new KaiRadioButton(this, ID_TIMES_FRAMES, _("Czas"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	Times->SetValue(!asFrames);
	Times->Enable(false);
	Frames = new KaiRadioButton(this, ID_TIMES_FRAMES, _("Klatki"));
	Frames->Enable(false);
	if (asFrames){
		Frames->SetValue(asFrames);
	}

	Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, &EditBox::OnChangeTimeDisplay, this, ID_TIMES_FRAMES);

	BoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
	BoxSizer5->Add(LineNumber, 0, /*wxALIGN_CENTER | */wxLEFT | wxEXPAND, 2);
	BoxSizer5->Add(Chars, 0, /*wxALIGN_CENTER |*/ wxLEFT | wxEXPAND, 6);
	BoxSizer5->Add(Chtime, 0, /*wxALIGN_CENTER | */wxLEFT | wxEXPAND, 6);
	BoxSizer5->Add(TlMode, 0, wxALIGN_CENTER | wxLEFT, 6);
	BoxSizer5->Add(Times, 0, wxALIGN_CENTER | wxLEFT, 2);
	BoxSizer5->Add(Frames, 0, wxALIGN_CENTER | wxLEFT, 2);


	Bcpall = new MappedButton(this, EDITBOX_PASTE_ALL_TO_TRANSLATION, _("Wklej wszystko"), EDITBOX_HOTKEY);
	Bcpall->Hide();
	Bcpsel = new MappedButton(this, EDITBOX_PASTE_SELECTION_TO_TRANSLATION, _("Wklej zaznaczone"), EDITBOX_HOTKEY);
	Bcpsel->Hide();
	Bhide = new MappedButton(this, EDITBOX_HIDE_ORIGINAL, _("Ukryj oryginał"), EDITBOX_HOTKEY);
	Bhide->Hide();
	DoubtfulTL = new ToggleButton(this, ID_DOUBTFULTL, _("Niepewne"));
	DoubtfulTL->Hide();
	AutoMoveTags = new ToggleButton(this, ID_AUTOMOVETAGS, _("Przenoszenie tagów"));
	AutoMoveTags->Hide();

	BoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
	BoxSizer6->Add(Bcpsel, 0, wxALIGN_CENTER | wxLEFT | wxTOP | wxBOTTOM, 2);
	BoxSizer6->Add(Bcpall, 0, wxALIGN_CENTER | wxLEFT | wxTOP | wxBOTTOM, 2);
	BoxSizer6->Add(Bhide, 0, wxALIGN_CENTER | wxLEFT | wxTOP | wxBOTTOM, 2);
	BoxSizer6->Add(DoubtfulTL, 0, wxALIGN_CENTER | wxLEFT | wxTOP | wxBOTTOM, 2);
	BoxSizer6->Add(AutoMoveTags, 0, wxALIGN_CENTER | wxLEFT | wxTOP | wxBOTTOM, 2);


	TextEdit = new TextEditor(this, ID_TEXT_EDITOR, true, wxDefaultPosition, wxSize(-1, 30));
	TextEdit->EB = this;
	
	TextEditOrig = new TextEditor(this, ID_TEXT_EDITOR, false, wxDefaultPosition, wxSize(-1, 30));
	TextEditOrig->EB = this;
	
	TextEditOrig->Hide();
	Comment = new KaiCheckBox(this, ID_COMMENT, _("Komentarz")/*, wxDefaultPosition, wxSize(82, -1)*/);
	Comment->SetValue(false);
	LayerEdit = new NumCtrl(this, ID_NUM_CONTROL, L"", -10000000, 10000000, true, wxDefaultPosition, wxSize(50, -1));
	int fw, fh;
	GetTextExtent(L"00:00:00,000", &fw, &fh);
	fw += 6;
	StartEdit = new TimeCtrl(this, ID_NUM_CONTROL, L"", wxDefaultPosition, wxSize(fw, -1), wxTE_CENTER);
	EndEdit = new TimeCtrl(this, ID_NUM_CONTROL, L"", wxDefaultPosition, wxSize(fw, -1), wxTE_CENTRE);
	DurEdit = new TimeCtrl(this, ID_NUM_CONTROL, L"", wxDefaultPosition, wxSize(fw, -1), wxTE_CENTRE);
	wxArrayString styles;
	styles.Add(L"Default");
	StyleChoice = new KaiChoice(this, ID_STYLE, wxDefaultPosition, wxSize(100, -1), styles);//wxSize(145,-1)
	StyleEdit = new MappedButton(this, ID_EDIT_STYLE, _("Edytuj"), EDITBOX_HOTKEY/*, wxDefaultPosition, wxSize(45, -1)*/);
	//second line
	wxTextValidator valid(wxFILTER_EXCLUDE_CHAR_LIST);
	valid.SetCharExcludes(L",");
	ActorEdit = new ComboBoxCtrl(this, ID_COMBO_BOX_CTRL, wxSize(90, -1), _("Aktor"), valid);
	GetTextExtent(L"0000", &fw, &fh);
	fw += 10;
	MarginLEdit = new NumCtrl(this, ID_NUM_CONTROL, L"", 0, 9999, true, wxDefaultPosition, wxSize(fw, -1), wxTE_CENTRE);
	MarginREdit = new NumCtrl(this, ID_NUM_CONTROL, L"", 0, 9999, true, wxDefaultPosition, wxSize(fw, -1), wxTE_CENTRE);
	MarginVEdit = new NumCtrl(this, ID_NUM_CONTROL, L"", 0, 9999, true, wxDefaultPosition, wxSize(fw, -1), wxTE_CENTRE);
	EffectEdit = new ComboBoxCtrl(this, ID_COMBO_BOX_CTRL, wxSize(90, -1), _("Efekt"), valid);

	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	BoxSizer2->Add(Comment, 0, wxLEFT | wxALIGN_CENTER, 2);
	BoxSizer2->Add(LayerEdit, 0, wxLEFT, 2);
	BoxSizer2->Add(StartEdit, 0, wxLEFT, 2);
	BoxSizer2->Add(EndEdit, 0, wxLEFT, 2);
	BoxSizer2->Add(DurEdit, 0, wxLEFT, 2);
	BoxSizer2->Add(StyleChoice, 4, wxLEFT | wxEXPAND, 2);
	BoxSizer2->Add(StyleEdit, 0, wxLEFT | wxRIGHT, 2);
	BoxSizer2->Add(ActorEdit, 3, wxEXPAND);
	BoxSizer2->Add(MarginLEdit, 0, wxLEFT, 2);
	BoxSizer2->Add(MarginREdit, 0, wxLEFT, 2);
	BoxSizer2->Add(MarginVEdit, 0, wxLEFT, 2);
	BoxSizer2->Add(EffectEdit, 3, wxLEFT | wxRIGHT | wxEXPAND, 2);

	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	BoxSizer1->Add(BoxSizer4, 0, wxLEFT | wxRIGHT | wxTOP, 2);
	BoxSizer1->Add(BoxSizer5, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 2);
	BoxSizer1->Add(TextEditOrig, 3, wxEXPAND | wxLEFT | wxRIGHT, 4);
	BoxSizer1->Add(BoxSizer6, 0, wxLEFT | wxRIGHT, 2);
	BoxSizer1->Add(TextEdit, 3, wxEXPAND | wxLEFT | wxRIGHT, 4);
	BoxSizer1->Add(BoxSizer2, 0, wxEXPAND | wxALL, 2);
	
	BoxSizer3 = NULL;

	SetSizer(BoxSizer1);

	Connect(ID_COMMENT, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&EditBox::OnCommit);
	Connect(ID_TLMODE, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&EditBox::OnTlMode);
	Connect(ID_COMBO_BOX_CTRL, wxEVT_COMMAND_COMBOBOX_SELECTED, (wxObjectEventFunction)&EditBox::OnCommit);
	Connect(ID_STYLE, wxEVT_COMMAND_CHOICE_SELECTED, (wxObjectEventFunction)&EditBox::OnCommit);
	Connect(EDITBOX_INSERT_BOLD, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EditBox::OnBoldClick);
	Connect(EDITBOX_INSERT_ITALIC, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EditBox::OnItalicClick);
	Connect(EDITBOX_CHANGE_UNDERLINE, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EditBox::OnUnderlineClick);
	Connect(EDITBOX_CHANGE_STRIKEOUT, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EditBox::OnStrikeClick);
	Connect(ID_AN, wxEVT_COMMAND_CHOICE_SELECTED, (wxObjectEventFunction)&EditBox::OnAnChoice);
	Connect(EDITBOX_CHANGE_FONT, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EditBox::OnFontClick);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &EditBox::OnColorClick, this, EDITBOX_CHANGE_COLOR_PRIMARY, EDITBOX_CHANGE_COLOR_SHADOW);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &EditBox::OnStyleEdit, this, ID_EDIT_STYLE);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &EditBox::OnCopyAll, this, EDITBOX_PASTE_ALL_TO_TRANSLATION);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &EditBox::OnCopySelection, this, EDITBOX_PASTE_SELECTION_TO_TRANSLATION);
	Connect(EDITBOX_HIDE_ORIGINAL, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EditBox::OnHideOriginal);
	Connect(ID_DOUBTFULTL, wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, (wxObjectEventFunction)&EditBox::OnDoubtfulTl);
	Connect(ID_AUTOMOVETAGS, wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, (wxObjectEventFunction)&EditBox::OnAutoMoveTags);
	Connect(EDITBOX_COMMIT, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EditBox::OnCommit);
	Connect(EDITBOX_COMMIT_GO_NEXT_LINE, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EditBox::OnNewline);
	Connect(EDITBOX_FIND_NEXT_DOUBTFUL, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EditBox::FindNextDoubtfulTl);
	Connect(EDITBOX_FIND_NEXT_UNTRANSLATED, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EditBox::FindNextUnTranslated);
	Connect(EDITBOX_SET_DOUBTFUL, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EditBox::OnDoubtfulTl);
	Connect(EDITBOX_SPLIT_LINE, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EditBox::OnSplit);
	Connect(EDITBOX_START_DIFFERENCE, EDITBOX_END_DIFFERENCE, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EditBox::OnPasteDifferents);
	Connect(wxEVT_SIZE, (wxObjectEventFunction)&EditBox::OnSize);
	if (!Options.GetBool(DISABLE_LIVE_VIDEO_EDITING)){
		Connect(ID_NUM_CONTROL, NUMBER_CHANGED, (wxObjectEventFunction)&EditBox::OnEdit);
		Connect(ID_TEXT_EDITOR, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&EditBox::OnEdit);
	}
	Connect(ID_TEXT_EDITOR, CURSOR_MOVED, (wxObjectEventFunction)&EditBox::OnCursorMoved);
	DoTooltips();
	if (asFrames){
		StartEdit->ShowFrames(asFrames);
		EndEdit->ShowFrames(asFrames);
		DurEdit->ShowFrames(asFrames);
	}
}

EditBox::~EditBox()
{
	wxDELETE(line);
}

void EditBox::SetLine(int Row, bool setaudio, bool save, bool nochangeline, bool autoPlay)
{
	bool rowChanged = currentLine != Row;
	//when preview is shown do not block setline 
	//cause after click on preview and click back on original shit happens
	if (nochangeline && !rowChanged && !tab->Grid->preview){ goto done; }
	// space for guard by mutex
	// don't use editionMutex with OpenSubs opened later
	{
		wxMutexLocker lock(grid->GetMutex());
		//showing / hiding tlmode controls when subs preview is on
		//it must hide it after return to original grid
		if (tab->Grid->preview && TextEditOrig->IsShown() != grid->hasTLMode) {
			SetTlMode(grid->hasTLMode, true);
		}

		if (Options.GetInt(GRID_SAVE_AFTER_CHARACTER_COUNT) > 1 && rowChanged && save) {
			Send(EDITBOX_LINE_EDITION, false);
		}
		size_t gridCount = grid->GetCount();
		if (currentLine < gridCount) {
			Dialogue *prevDial = grid->GetDialogue(currentLine);
			if (prevDial->Start.mstime > prevDial->End.mstime) {
				prevDial->End = prevDial->Start;
				grid->Refresh(false);
			}
		}
		if (StartEdit->changedBackGround) {
			StartEdit->SetForegroundColour(WINDOW_TEXT);
		}
		if (EndEdit->changedBackGround) {
			EndEdit->SetForegroundColour(WINDOW_TEXT);
		}
		if (DurEdit->changedBackGround) {
			DurEdit->SetForegroundColour(WINDOW_TEXT);
		}
		if (Row >= gridCount)
			Row = gridCount - 1;

		currentLine = Row;
		grid->markedLine = Row;
		grid->currentLine = Row;
		wxDELETE(line);
		line = grid->GetDialogue(currentLine)->Copy();
		LineNumber->SetLabelText(wxString::Format(_("Linia: %i"), (int)grid->file->GetElementByKey(currentLine) + 1));
		Comment->SetValue(line->IsComment);
		LayerEdit->SetInt(line->Layer);
		StartEdit->SetTime(line->Start, false, 1);
		EndEdit->SetTime(line->End, false, 2);
		if (grid->showFrames) {
			STime durationTime = EndEdit->GetTime() - StartEdit->GetTime();
			durationTime.orgframe++;
			DurEdit->SetTime(durationTime);
		}
		else {
			DurEdit->SetTime(line->End - line->Start);
		}
		StyleChoice->SetSelection(StyleChoice->FindString(line->Style, true));
		ActorEdit->ChangeValue(line->Actor);
		MarginLEdit->SetInt(line->MarginL);
		MarginREdit->SetInt(line->MarginR);
		MarginVEdit->SetInt(line->MarginV);
		EffectEdit->ChangeValue(line->Effect);
		TextEdit->SetState((!line->IsComment) ? 0 : (line->Effect->StartsWith(L"template")) ? 2 : (line->Effect->StartsWith(L"code")) ? 3 : 1);
		SetTextWithTags();

		if (DoubtfulTL->IsShown()) {
			DoubtfulTL->SetValue(line->IsDoubtful());
		}


		//set characters per seconds and wraps
		UpdateChars();
	}

	//block show audio and video when preview enabled
	//and editbox shows line from preview grid != tab->Grid
	if (tab->Grid->preview && tab->Grid != grid)
		return;

	if (setaudio && ABox && ABox->IsShown()){ ABox->audioDisplay->SetDialogue(line, currentLine); }

done:
	VideoCtrl *vb = tab->Video;
	int playAfter = 0, seekAfter = 0;
	tab->Video->GetVideoListsOptions(&playAfter, &seekAfter);

	if (seekAfter == 1 && playAfter < 2 && !nochangeline && rowChanged){
		if (vb->GetState() != None){
			if (vb->GetState() == Playing){ vb->Pause(); }
			vb->Seek(line->Start.mstime);
		}
		//return;
	}

	if (playAfter > 0 && autoPlay){
		if (playAfter == 1){
			if (ABox){
				wxWindow *focused = wxWindow::FindFocus();
				wxCommandEvent evt; ABox->OnPlaySelection(evt);
				focused->SetFocus();
			}
		}
		else{
			if (tab->Video->IsShown() || tab->Video->IsFullScreen()){
				Dialogue *next = grid->GetDialogue(grid->GetKeyFromPosition(currentLine, 1));
				int ed = line->End.mstime, nst = next->Start.mstime;
				int playend = (nst > ed && playAfter > 2) ? nst : ed;
				tab->Video->PlayLine(line->Start.mstime, tab->Video->GetPlayEndTime(playend));
			}
		}
	}

	if (Visual > CHANGEPOS && rowChanged/* && !nochangeline*/){
		bool visualClip = Visual == VECTORCLIP;
		tab->Video->SetVisual(false, !visualClip);
	}

	//Set time and differents in video text field
	if (tab->Video->IsShown() && tab->Video->GetState() != None && seekAfter == 0){
		tab->Video->RefreshTime();
	}

}

void EditBox::UpdateChars()
{

	if (line->IsComment){
		Chars->SetLabelText(L"");
		Chtime->SetLabelText(L"");
	}
	else{
		TextEditor * editor = GetEditor();
		const TextData &td = editor->GetTextData();
		wxString wraps;
		if (td.wraps.length() < 50) { wraps = td.wraps; }
		else {
			wraps = td.wraps.Mid(0, 50).BeforeLast(L'/') + L"/.../";
		}
		Chars->SetLabelText(_("Łamania: ") + wraps + L"43");
		Chars->SetForegroundColour((td.badWraps) ? WINDOW_WARNING_ELEMENTS : WINDOW_TEXT);
		int chtime = td.GetCPS(line);
		Chtime->SetLabelText(wxString::Format(_("Znaki na sekundę: %i<=15"), chtime));
		Chtime->SetForegroundColour((chtime > 15) ? WINDOW_WARNING_ELEMENTS : WINDOW_TEXT);
	}
	BoxSizer5->Layout();
	Frames->Refresh(false);
	Frames->Update();
	Times->Refresh(false);
	Times->Update();
}

//Getting data from editbox controls
//gotoNextLine go to next line or stays in actual active
//dummy do not save dialogue to grid
//visualdummy do not refresh video, using for visual clips
void EditBox::Send(unsigned char editionType, bool gotoNextLine, bool dummy, bool visualdummy)
{
	long cellm = 0;
	if (!dummy && !visualdummy && StartEdit->changedBackGround){
		StartEdit->SetForegroundColour(WINDOW_TEXT);
	}
	if (!dummy && !visualdummy && EndEdit->changedBackGround){
		EndEdit->SetForegroundColour(WINDOW_TEXT);
	}
	if (!dummy && !visualdummy && DurEdit->changedBackGround){
		DurEdit->SetForegroundColour(WINDOW_TEXT);
	}
	if (line->IsComment != Comment->GetValue()){
		line->IsComment = !line->IsComment;
		cellm |= COMMENT;
	}

	if (LayerEdit->IsModified()){
		line->Layer = LayerEdit->GetInt();
		cellm |= LAYER;
		LayerEdit->SetModified(dummy);
	}

	if (StartEdit->IsModified() || StartEdit->HasFocus()){
		line->Start = StartEdit->GetTime(1);
		cellm |= START;
		StartEdit->SetModified(dummy);
	}
	if (EndEdit->IsModified() || EndEdit->HasFocus()){
		line->End = EndEdit->GetTime(2);
		cellm |= END;
		EndEdit->SetModified(dummy);
	}
	if (DurEdit->IsModified()){
		line->End = EndEdit->GetTime();
		cellm |= END;
		DurEdit->SetModified(dummy);
	}

	wxString checkstyle = StyleChoice->GetString(StyleChoice->GetSelection());
	if (line->Style != checkstyle && checkstyle != L"" || StyleChoice->HasFocus()){
		line->Style = checkstyle;
		cellm |= STYLE;
	}
	if (ActorEdit->IsModified()){
		line->Actor = ActorEdit->GetValue();
		cellm |= ACTOR;
		ActorEdit->SetModified(dummy);
	}
	if (MarginLEdit->IsModified()){
		line->MarginL = MarginLEdit->GetInt();
		cellm |= MARGINL;
		MarginLEdit->SetModified(dummy);
	}
	if (MarginREdit->IsModified()){
		line->MarginR = MarginREdit->GetInt();
		cellm |= MARGINR;
		MarginREdit->SetModified(dummy);
	}
	if (MarginVEdit->IsModified()){
		line->MarginV = MarginVEdit->GetInt();
		cellm |= MARGINV;
		MarginVEdit->SetModified(dummy);
	}
	if (EffectEdit->IsModified()){
		line->Effect = EffectEdit->GetValue();
		cellm |= EFFECT;
		EffectEdit->SetModified(dummy);
	}
	if (TextEditOrig->IsShown() && (TextEditOrig->IsModified() || (splittedTags && TextEdit->IsModified()))) {
		line->Text = TextEditOrig->GetValue(false);
		cellm |= TXT;
		if(!splittedTags)
		TextEditOrig->SetModified(dummy);
	}
	if (TextEdit->IsModified() || (splittedTags && TextEditOrig->IsModified())){
		if (TextEditOrig->IsShown()){
			line->TextTl = TextEdit->GetValue(false);
			cellm |= TXTTL;
		}
		else{
			line->Text = TextEdit->GetValue(false);
			cellm |= TXT;
		}
		TextEdit->SetModified(dummy);
		if (splittedTags)
			TextEditOrig->SetModified(dummy);
	}
	

	if (cellm){
		if (currentLine < grid->GetCount() && !dummy){
			grid->ChangeLine(editionType, line, currentLine, cellm, gotoNextLine, visualdummy);
			if (cellm & ACTOR || cellm & EFFECT){
				RebuildActorEffectLists();
			}
		}
	}
	else if (gotoNextLine){ grid->NextLine(); }
}


void EditBox::PutinNonass(const wxString &text, const wxString &tag)
{
	if (grid->subsFormat == TMP) return;
	long from, to, whre;
	size_t start = 0, len = 0;
	bool match = false;
	TextEdit->GetSelection(&from, &to);
	wxString txt = TextEdit->GetValue();
	bool oneline = (grid->file->SelectionsSize() < 2);
	if (oneline){//Changing only in editbox
		if (grid->subsFormat == SRT){

			wxRegEx srttag(L"\\</?" + text + L"\\>", wxRE_ADVANCED | wxRE_ICASE);
			if (srttag.Matches(txt.SubString(from - 4, from + 4))){
				srttag.GetMatch(&start, &len, 0);
				if (len + start >= 4 && start <= 4)
				{
					whre = from - 4 + start;
					txt.Remove(whre, len);
					txt.insert(whre, L"<" + tag + L">");
					whre += 3;
					match = true;
				}
			}
			if (!match){ txt.insert(from, L"<" + tag + L">"); from += 3; to += 3; whre = from; }
			if (from != to){
				match = false;
				if (srttag.Matches(txt.SubString(to - 4, to + 4))){
					srttag.GetMatch(&start, &len, 0);
					if (len + start >= 4 && start <= 4)
					{
						txt.Remove(to - 4 + start, len);
						txt.insert(to - 4 + start, L"</" + tag + L">");
						whre = to + start;
						match = true;
					}
				}
				if (!match){ txt.insert(to, L"</" + tag + L">"); whre = to + 4; }
			}

		}
		else if (grid->subsFormat == MDVD){


			wxRegEx srttag(L"\\{" + text + L"}", wxRE_ADVANCED | wxRE_ICASE);
			int wheres = txt.SubString(0, from).Find(L'|', true);
			if (wheres == -1){ wheres = 0; }
			if (srttag.Matches(txt.Mid(wheres))){
				if (srttag.GetMatch(&start, &len, 0))
				{
					whre = wheres + start;
					txt.Remove(whre, len);
					txt.insert(whre, L"{" + tag + L"}");
					match = true;
				}
			}
			if (!match){ txt.insert(wheres, L"{" + tag + L"}"); }

		}

		TextEdit->SetTextS(txt, true);
		TextEdit->SetFocus();
		TextEdit->SetSelection(whre, whre);
	}
	else
	{//Changes in all selected lines
		wxString chars = (grid->subsFormat == SRT) ? L"<" : L"{";
		wxString chare = (grid->subsFormat == SRT) ? L">" : L"}";
		wxArrayInt sels;
		grid->file->GetSelections(sels);
		for (size_t i = 0; i < sels.size(); i++)
		{
			Dialogue *dialc = grid->file->CopyDialogue(sels[i]);
			wxString txt = dialc->Text;
			if (txt.StartsWith(chars))
			{
				wxRegEx rex(chars + tag + chare, wxRE_ADVANCED | wxRE_ICASE);
				rex.ReplaceAll(&txt, L"");
				dialc->Text = chars + text + chare + txt;
			}
			else
			{
				dialc->Text->Prepend(chars + text + chare);
			}
		}
		grid->SetModified(EDITBOX_MULTILINE_EDITION);
		grid->Refresh(false);
	}

}

void EditBox::OnFontClick(wxCommandEvent& event)
{
	char form = grid->subsFormat;
	Styles *mstyle = (form < SRT) ? grid->GetStyle(0, line->Style)->Copy() : new Styles();
	actualStyle = Styles(*mstyle);
	int tmpIter = grid->file->Iter();
	if (form < SRT){
		wxString tmp;
		if (FindTag(L"b(0|1)", L"", 0, true)){
			GetTextResult(&tmp);
			mstyle->Bold = tmp == L"1";
		}
		if (FindTag(L"i(0|1)", L"", 0, true)){
			GetTextResult(&tmp);
			mstyle->Italic = tmp == L"1";
		}
		if (FindTag(L"u(0|1)", L"", 0, true)){
			GetTextResult(&tmp);
			mstyle->Underline = tmp == L"1";
		}
		if (FindTag(L"s(0|1)", L"", 0, true)){
			GetTextResult(&tmp);
			mstyle->StrikeOut = tmp == L"1";
		}
		if (FindTag(L"fs([0-9.]+)", L"", 0, true)){
			GetTextResult(&tmp);
			mstyle->SetFontSizeString(tmp); 
		}
		if (FindTag(L"fn(.*)", L"", 0, true)){
			GetTextResult(&tmp);
			mstyle->Fontname = tmp; 
		}
	}
	FontDialog *FD = FontDialog::Get(this, mstyle);
	FD->Bind(FONT_CHANGED, &EditBox::OnFontChange, this, FD->GetId());
	if (FD->ShowModal() == wxID_OK) {
		wxString txt = TextEdit->GetValue();
		TextEditor * editor = TextEdit;
		if (grid->hasTLMode && txt == L""){
			editor = TextEditOrig;
			txt = TextEditOrig->GetValue();
		}
		wxPoint pos = GetPositionInText();
		if (txt[pos.x] != L'}'){
			int bracketPos = txt.find(L"}", pos.x);
			if (bracketPos != -1){ pos.x = pos.y = bracketPos + 1; }
		}
		editor->SetSelection(pos.x, pos.x);
		editor->SetFocus();
	}
	else{
		grid->DummyUndo(tmpIter);
	}

}

void EditBox::OnFontChange(wxCommandEvent& event){
	FontDialog *FD = (FontDialog *)event.GetClientData();
	if (!FD)
		return;
	Styles * input = NULL;
	Styles * output = NULL;
	FD->GetStyles(&input, &output);
	if (input && output)
		ChangeFont(output, input);
}

void EditBox::ChangeFont(Styles *retStyle, Styles *editedStyle)
{
	char form = grid->subsFormat;
	wxString tmp;
	if (retStyle->Fontname != editedStyle->Fontname)
	{
		if (form < SRT){
			FindTag(L"fn(.*)", L"", 0, true);
			PutTagInText(L"\\fn" + retStyle->Fontname, L"\\fn" + actualStyle.Fontname, false);
		}
		else{ PutinNonass(L"F:" + retStyle->Fontname, L"f:([^}]*)"); }
	}
	if (retStyle->Fontsize != editedStyle->Fontsize)
	{
		if (form < SRT){
			FindTag(L"fs([0-9]+)", L"", 0, true);
			PutTagInText(L"\\fs" + retStyle->Fontsize, L"\\fs" + actualStyle.Fontsize, false);
		}
		else{ PutinNonass(L"S:" + retStyle->Fontname, L"s:([^}]*)"); }
	}
	if (retStyle->Bold != editedStyle->Bold)
	{
		if (form < SRT){
			wxString bld = (retStyle->Bold) ? L"1" : L"0";
			wxString bld1 = (actualStyle.Bold) ? L"1" : L"0";
			FindTag(L"b(0|1)", L"", 0, true);
			PutTagInText(L"\\b" + bld, L"\\b" + bld1, false);
		}
		else{ PutinNonass(L"y:b", (retStyle->Bold) ? L"Y:b" : L""); }
	}
	if (retStyle->Italic != editedStyle->Italic)
	{
		if (form < SRT){
			wxString ital = (retStyle->Italic) ? L"1" : L"0";
			wxString ital1 = (actualStyle.Italic) ? L"1" : L"0";
			FindTag(L"i(0|1)", L"", 0, true);
			PutTagInText(L"\\i" + ital, L"\\i" + ital1, false);
		}
		else{ PutinNonass(L"y:i", (retStyle->Italic) ? L"Y:i" : L""); }
	}
	if (retStyle->Underline != editedStyle->Underline)
	{
		FindTag(L"u(0|1)", L"", 0, true);
		wxString under = (retStyle->Underline) ? L"1" : L"0";
		wxString under1 = (actualStyle.Underline) ? L"1" : L"0";
		PutTagInText(L"\\u" + under, L"\\u" + under1, false);
	}
	if (retStyle->StrikeOut != editedStyle->StrikeOut)
	{
		FindTag(L"s(0|1)", L"", 0, true);
		wxString strike = (retStyle->StrikeOut) ? L"1" : L"0";
		wxString strike1 = (actualStyle.StrikeOut) ? L"1" : L"0";
		PutTagInText(L"\\s" + strike, L"\\s" + strike1, false);
	}
}

void EditBox::AllColorClick(int numColor, bool leftClick /*= true*/)
{
	//check if need to switch hotkeys
	if (Options.GetBool(COLORPICKER_SWITCH_CLICKS))
		leftClick = !leftClick;

	wxString tmptext = TextEdit->GetValue();
	TextEditor *editor = TextEdit;
	int tmpIter = grid->file->Iter();
	if (grid->hasTLMode && tmptext == L""){
		tmptext = TextEditOrig->GetValue();
		editor = TextEditOrig;
	}

	GetColor(&actualColor, numColor);
	actualColorstr = actualColor.GetAss(false, true);

	if (leftClick){
		DialogColorPicker *ColourDialog = DialogColorPicker::Get(this, actualColor.GetWX(), numColor);
		MoveToMousePosition(ColourDialog);
		ColourDialog->Connect(ID_COLOR_PICKER_DIALOG, COLOR_CHANGED, (wxObjectEventFunction)&EditBox::OnColorChange, 0, this);
		ColourDialog->Bind(COLOR_TYPE_CHANGED, [=](wxCommandEvent &evt){
			GetColor(&actualColor, evt.GetInt());
			actualColorstr = actualColor.GetAss(false, true);
			ColourDialog->SetColor(actualColor, 0, false);
		}, ID_COLOR_PICKER_DIALOG);
		if (ColourDialog->ShowModal() == wxID_OK) {
			//Called only to add color to recent
			ColourDialog->GetColor();
			wxString txt = editor->GetValue();
			wxPoint pos = GetPositionInText();
			if (txt[pos.x] != L'}'){
				int bracketPos = txt.find(L"}", pos.x);
				if (bracketPos != -1){ pos.x = bracketPos + 1; }
			}
			editor->SetSelection(pos.x, pos.x);
		}
		else{
			grid->DummyUndo(tmpIter);
		}
	}
	else{
		SimpleColorPicker scp(this, actualColor, numColor);
		SimpleColorPickerDialog *scpd = scp.GetDialog();
		int spcdId = scpd->GetId();
		scpd->Bind(COLOR_CHANGED, &EditBox::OnColorChange, this, spcdId);
		scpd->Bind(COLOR_TYPE_CHANGED, [=](wxCommandEvent &evt){
			AssColor col;
			GetColor(&col, evt.GetInt());
			scpd->SetColor(col);
		}, spcdId);

		AssColor ret;
		if (scp.PickColor(&ret)){
			scpd->AddRecent();
			wxString txt = editor->GetValue();
			wxPoint pos = GetPositionInText();
			if (txt[pos.x] != L'}'){
				int bracketPos = txt.find(L"}", pos.x);
				if (bracketPos != -1){ pos.x = bracketPos + 1; }
			}
			editor->SetSelection(pos.x, pos.x);
		}
		else{
			grid->DummyUndo(tmpIter);
		}
	}
	editor->SetFocus();
}

void EditBox::GetColor(AssColor *actualColor, int numColor)
{
	if ((grid->subsFormat < SRT)){
		wxString colorNumber;
		colorNumber << numColor;
		wxString retTag;
		wxString tag = (numColor == 1) ? L"?c&(.*)" : L"c&(.*)";
		Styles *style = grid->GetStyle(0, line->Style);
		*actualColor = (numColor == 1) ? style->PrimaryColour :
			(numColor == 2) ? style->SecondaryColour :
			(numColor == 3) ? style->OutlineColour :
			style->BackColour;
		if (FindTag(colorNumber + tag, L"", 0, true)){
			GetTextResult(&retTag);
			actualColor->Copy(AssColor(L"&" + retTag));
		}
		//when knowing about alpha tag will be needed You must change it like in method OnColorChange
		if (FindTag(colorNumber + L"a&|alpha(.*)", L"", 0, true)){
			GetTextResult(&retTag);
			actualColor->SetAlphaString(retTag); 
		}
	}
}

void EditBox::OnColorClick(wxCommandEvent& event)
{
	AllColorClick(event.GetId() - EDITBOX_CHANGE_COLOR_PRIMARY + 1);
}

void EditBox::OnColorRightClick(wxMouseEvent& event)
{
	AllColorClick(event.GetId() - EDITBOX_CHANGE_COLOR_PRIMARY + 1, false);
}

void EditBox::OnCommit(wxCommandEvent& event)
{
	///test it too if it's needed
	//tab->Video->m_blockRender = true;
	if (splittedTags && (TextEdit->IsModified() || TextEditOrig->IsModified())){
		TextEdit->SetModified(); TextEditOrig->SetModified(); splittedTags = false;
	}
	Send(EDITBOX_LINE_EDITION, false, false, Visual != 0);
	if (event.GetId() == ID_COMMENT){
		TextEdit->SetState((!line->IsComment) ? 0 : 
			(line->Effect->StartsWith(L"template")) ? 2 : 
			(line->Effect->StartsWith(L"code")) ? 3 : 1, true);
	}
	if (Visual){
		tab->Video->SetVisual(true);
	}
	if (StyleChoice->HasFocus() || Comment->HasFocus()){ grid->SetFocus(); }
	if (ABox){ ABox->audioDisplay->SetDialogue(line, currentLine); }
	//tab->Video->m_blockRender = false;
}

void EditBox::OnNewline(wxCommandEvent& event)
{
	if (splittedTags && (TextEdit->IsModified() || TextEditOrig->IsModified())){ 
		TextEdit->SetModified(); 
		TextEditOrig->SetModified(); 
	}
	bool noNewLine = !(StartEdit->HasFocus() || EndEdit->HasFocus() || 
		DurEdit->HasFocus()) || !Options.GetBool(EDITBOX_DONT_GO_TO_NEXT_LINE_ON_TIMES_EDIT);
	if (!noNewLine && ABox){ ABox->audioDisplay->SetDialogue(line, currentLine); }
	Send(EDITBOX_LINE_EDITION, noNewLine);
	if (Visual == CHANGEPOS){
		tab->Video->SetVisual(true, true);
	}
	splittedTags = false;
}

void EditBox::OnBoldClick(wxCommandEvent& event)
{
	if (grid->subsFormat < SRT){
		Styles *mstyle = grid->GetStyle(0, line->Style);
		wxString value = (mstyle->Bold) ? L"1" : L"0";
		wxString nvalue = (mstyle->Bold) ? L"0" : L"1";
		if (FindTag(L"b(0|1)", L"", 0, true)){ 
			GetTextResult(&value);
			nvalue = (value == L"1") ? L"0" : L"1";
		}
		PutTagInText(L"\\b" + nvalue, L"\\b" + value);
	}
	else if (grid->subsFormat == SRT){ PutinNonass(L"b", L"b"); }
	else { PutinNonass(L"y:b", L"Y:b"); }
}

void EditBox::OnItalicClick(wxCommandEvent& event)
{
	if (grid->subsFormat < SRT){
		Styles *mstyle = grid->GetStyle(0, line->Style);
		wxString value = (mstyle->Italic) ? L"1" : L"0";
		wxString nvalue = (mstyle->Italic) ? L"0" : L"1";
		if (FindTag(L"i(0|1)", L"", 0, true)){ 
			GetTextResult(&value);
			nvalue = (value == L"1") ? L"0" : L"1";
		}
		PutTagInText(L"\\i" + nvalue, L"\\i" + value);
	}
	else if (grid->subsFormat == SRT){ PutinNonass(L"i", L"i"); }
	else if (grid->subsFormat == MDVD){ PutinNonass(L"y:i", L"Y:i"); }
	else{ PutinNonass(L"/", L"/"); }
}

void EditBox::OnUnderlineClick(wxCommandEvent& event)
{
	if (grid->subsFormat < SRT){
		Styles *mstyle = grid->GetStyle(0, line->Style);
		wxString value = (mstyle->Underline) ? L"1" : L"0";
		wxString nvalue = (mstyle->Underline) ? L"0" : L"1";
		if (FindTag(L"u(0|1)", L"", 0, true)){ 
			GetTextResult(&value);
			nvalue = (value == L"1") ? L"0" : L"1";
		}
		PutTagInText(L"\\u" + nvalue, L"\\u" + value);
	}
	else if (grid->subsFormat == SRT){ PutinNonass(L"u", L"u"); }
}

void EditBox::OnStrikeClick(wxCommandEvent& event)
{
	if (grid->subsFormat < SRT){
		Styles *mstyle = grid->GetStyle(0, line->Style);
		wxString value = (mstyle->StrikeOut) ? L"1" : L"0";
		wxString nvalue = (mstyle->StrikeOut) ? L"0" : L"1";
		if (FindTag(L"s(0|1)", L"", 0, true)){ 
			GetTextResult(&value);
			nvalue = (value == L"1") ? L"0" : L"1";
		}
		PutTagInText(L"\\s" + nvalue, L"\\s" + value);
	}
	else if (grid->subsFormat == SRT){ PutinNonass(L"s", L"s"); }
}

void EditBox::OnAnChoice(wxCommandEvent& event)
{
	FindTag(L"an([0-9])", L"", 1);
	PutTagInText(L"\\" + Ban->GetString(Ban->GetSelection()), L"", true);
}

void EditBox::OnTlMode(wxCommandEvent& event)
{
	bool show = !TextEditOrig->IsShown();
	if (grid->SetTlMode(show)){ TlMode->SetValue(true); return; }
	SetTlMode(show);
	SetLine(currentLine);
}

void EditBox::OnAccelerator(wxCommandEvent& event)
{
	switch (event.GetId())
	{
	
		case EDITBOX_CHANGE_FONT: OnFontClick(event); break;
		case EDITBOX_CHANGE_UNDERLINE: OnUnderlineClick(event); break;
		case EDITBOX_CHANGE_STRIKEOUT: OnStrikeClick(event); break;
		case EDITBOX_PASTE_ALL_TO_TRANSLATION: OnCopyAll(event); break;
		case EDITBOX_PASTE_SELECTION_TO_TRANSLATION: OnCopySelection(event); break;
		case EDITBOX_HIDE_ORIGINAL: OnHideOriginal(event); break;
		case EDITBOX_CHANGE_COLOR_PRIMARY: 
		case EDITBOX_CHANGE_COLOR_SECONDARY: 
		case EDITBOX_CHANGE_COLOR_OUTLINE: 
		case EDITBOX_CHANGE_COLOR_SHADOW: OnColorClick(event); break;
		case EDITBOX_COMMIT: OnCommit(event); break;
		case EDITBOX_COMMIT_GO_NEXT_LINE: OnNewline(event); break;
		case EDITBOX_INSERT_BOLD: OnBoldClick(event); break;
		case EDITBOX_INSERT_ITALIC: OnItalicClick(event); break;
		case EDITBOX_SPLIT_LINE: OnSplit(event); break;
		case EDITBOX_START_DIFFERENCE: OnPasteDifferents(event); break;
		case EDITBOX_END_DIFFERENCE: OnPasteDifferents(event); break;
		case EDITBOX_FIND_NEXT_DOUBTFUL: FindNextDoubtfulTl(event); break;
		case EDITBOX_FIND_NEXT_UNTRANSLATED: FindNextUnTranslated(event); break;
		case EDITBOX_SET_DOUBTFUL: OnDoubtfulTl(event); break;
		case EDITBOX_TAG_BUTTON1: 
		case EDITBOX_TAG_BUTTON2: 
		case EDITBOX_TAG_BUTTON3: 
		case EDITBOX_TAG_BUTTON4: 
		case EDITBOX_TAG_BUTTON5: 
		case EDITBOX_TAG_BUTTON6: 
		case EDITBOX_TAG_BUTTON7: 
		case EDITBOX_TAG_BUTTON8: 
		case EDITBOX_TAG_BUTTON9: 
		case EDITBOX_TAG_BUTTON10: 
		case EDITBOX_TAG_BUTTON11: 
		case EDITBOX_TAG_BUTTON12: 
		case EDITBOX_TAG_BUTTON13: 
		case EDITBOX_TAG_BUTTON14: 
		case EDITBOX_TAG_BUTTON15: 
		case EDITBOX_TAG_BUTTON16: 
		case EDITBOX_TAG_BUTTON17: 
		case EDITBOX_TAG_BUTTON18: 
		case EDITBOX_TAG_BUTTON19: 
		case EDITBOX_TAG_BUTTON20: OnButtonTag(event); break;
		default:
			break;
	}
}

int EditBox::GetFormat()
{
	return grid->subsFormat;
}

//dummy tlmode is used on preview when 
void EditBox::SetTlMode(bool tl, bool dummyTlMode /*= false*/)
{
	TextEditOrig->Show(tl);
	Bcpall->Show(tl);
	Bcpsel->Show(tl);
	Bhide->Show(tl);
	DoubtfulTL->Show(tl);
	AutoMoveTags->Show(tl);
	AutoMoveTags->SetValue(Options.GetBool(AUTO_MOVE_TAGS_FROM_ORIGINAL));
	BoxSizer1->Layout();
	if (!dummyTlMode){
		if (TlMode->GetValue() != tl){ TlMode->SetValue(tl); }
		KainoteFrame *Kai = (KainoteFrame*)Notebook::GetTabs()->GetParent();
		Kai->Toolbar->UpdateId(GLOBAL_SAVE_TRANSLATION, tl);
	}
}

void EditBox::OnCopyAll(wxCommandEvent& event)
{
	TextEdit->SetTextS(TextEditOrig->GetValue(false), true, true, false, false);
	TextEdit->SetFocus();
}

void EditBox::OnCopySelection(wxCommandEvent& event)
{
	long from, to, fromtl, totl;
	TextEditOrig->GetSelection(&from, &to);
	if (from != to){
		wxString txt = TextEditOrig->GetValue();
		wxString txt1 = TextEdit->GetValue();
		TextEdit->GetSelection(&fromtl, &totl);
		wxString txtt = txt.SubString(from, to - 1);
		txt1.insert(fromtl, txtt);
		TextEdit->SetTextS(txt1, true);
		TextEdit->SetFocus();
		long whre = txtt.length();
		TextEdit->SetSelection(fromtl + whre, fromtl + whre);
	}
}



void EditBox::RefreshStyle(bool resetline)
{
	StyleChoice->Clear();
	for (size_t i = 0; i < grid->StylesSize(); i++){
		StyleChoice->Append(grid->GetStyle(i)->Name);
	}
	int selection = grid->FindStyle(line->Style);
	StyleChoice->SetSelection(selection);

	if (resetline){
		if (grid->GetCount() > 0){
			SetLine(0);
		}
		else{ currentLine = 0; }
	}
}


void EditBox::DoTooltips()
{
	Ban->SetToolTip(_("Położenie tekstu"));
	TlMode->SetToolTip(_("Tryb tłumacza wyświetla i zapisuje zarówno tekst obcojęzyczny, jak i tekst tłumaczenia"));
	Bcpall->SetToolTip(_("Kopiuje cały tekst obcojęzyczny do pola z tłumaczeniem"));
	Bcpsel->SetToolTip(_("Kopiuje zaznaczony tekst obcojęzyczny do pola z tłumaczeniem"));
	//TextEdit->SetToolTip(_("Tekst linijki / tekst tłumaczenia, gdy tryb tłumacza jest włączony."));
	//TextEditOrig->SetToolTip(_("tekst obcojęzyczny."));
	Comment->SetToolTip(_("Ustawia linijkę jako komentarz. Komentarze nie są wyświetlane"));
	LayerEdit->SetToolTip(_("Warstwa linijki, wyższe warstwy są na wierzchu"));
	StartEdit->SetToolTip(_("Czas początkowy linijki"));
	EndEdit->SetToolTip(_("Czas końcowy linijki"));
	DurEdit->SetToolTip(_("Czas trwania linijki"));
	StyleChoice->SetToolTip(_("Styl linijki"));
	StyleEdit->SetToolTip(_("Umożliwia szybką edycję stylu linijki"));
	ActorEdit->SetToolTip(_("Oznaczenie aktora linijki. Nie wpływa na wygląd napisów"));
	MarginLEdit->SetToolTip(_("Margines lewy linijki"));
	MarginREdit->SetToolTip(_("Margines prawy linijki"));
	MarginVEdit->SetToolTip(_("Margines górny i dolny linijki"));
	EffectEdit->SetToolTip(_("Efekt linijki. Służy do oznaczania linijek, na których zastosowane ma być karaoke bądź efekty VSFiltra"));
	Chars->SetToolTip(_("Ilość znaków w każdej linijce.\nNie więcej niż 43 znaki na linijkę (maksymalnie 2 linijki)"));
	Chtime->SetToolTip(_("Znaki na sekundę.\nNie powinny przekraczać 15 znaków na sekundę"));
}

wxPoint EditBox::FindBrackets(const wxString & text, long from)
{
	bool haveStartBracket = false;
	bool haveEndBracket = false;
	int endBrakcetPos = -1;
	int startBrakcetPos = -1;
	size_t len = text.length();
	size_t i = (from - 1 < 1) ? 1 : (from - 1);
	for (; i < len; i++){
		const wxUniChar & ch = text[i];
		if (ch == L'}'){
			haveEndBracket = true;
			endBrakcetPos = i;
		}
		else if (ch == L'{' && i + 1 > from && i != from){
			if (!haveEndBracket)
				break;
			else
				haveEndBracket = false;
		}
		else if (haveEndBracket){
			break;
		}
	}
	size_t k = from < len ? from : len - 1;
	
	for (; k + 1 > 0; k--){
		const wxUniChar & ch = text[k];
		if (ch == L'{'){
			haveStartBracket = true;
			startBrakcetPos = k;
		}
		else if (ch == L'}' && k + 1 < from){
			if (!haveStartBracket)
				break;
			else
				haveStartBracket = false;
		}
		else if (haveStartBracket){
			break;
		}
		
	}
	// no end bracket after block ...}{...cursor}
	// no need to correct it, end bracket without start is displayed as text
	
	// no first bracket after block {cursor...}{...
	if (haveEndBracket && i >= len && endBrakcetPos + 1 < len)
		endBrakcetPos = len - 1;
	// no end bracket
	if (startBrakcetPos != -1 && endBrakcetPos == -1)
		endBrakcetPos = len - 1;
	// no start bracket
	if (startBrakcetPos == -1 && endBrakcetPos != -1)
		endBrakcetPos = -1;

	return wxPoint(startBrakcetPos, endBrakcetPos);
}

void EditBox::SwapRTLTextLines(wxString* text, int posOfSplit, const wxString& splitTag)
{
	size_t textStart = 0;
	size_t textEnd = text->length() - 1;
	size_t splitTagLen = splitTag.length();
	if (posOfSplit < 1 || posOfSplit + splitTagLen >= textEnd)
		return;

	bool gotEnd = false;
	if (posOfSplit > 0) {
		for (size_t i = posOfSplit - 1; i + 1 > 0; i--) {
			if ((*text)[i] == splitTag[0]) {
				if (splitTagLen == 1) {
					textStart = i + 1;
					break;
				}
				else if (splitTagLen > 1 && (*text)[i + 1] == splitTag[1]) {
					textStart = i + 2;
					break;
				}
			}
		}
	}
	if (posOfSplit + 2 <= textEnd) {
		for (size_t i = posOfSplit + splitTagLen; i < text->length(); i++) {
			if ((*text)[i] == splitTag[0]) {
				if (splitTagLen == 1) {
					textEnd = i - 1;
					gotEnd = true;
					break;
				}
				else if (splitTagLen > 1 && (*text)[i + 1] == splitTag[1]) {
					textEnd = i - 1;
					gotEnd = true;
					break;
				}
			}
		}
	}
	wxString result;
	if (textStart != 0) {
		result = text->Mid(0, textStart);
	}
	if (posOfSplit + splitTagLen - 1 < text->length() - 1) {
		size_t startpos = posOfSplit + splitTagLen;
		result << text->Mid(startpos, textEnd - startpos + 1);
	}
	result << splitTag;
	if (posOfSplit - 1 > 0) {
		result << text->Mid(textStart, posOfSplit - textStart);
	}
	if (text->length() > textEnd + 1) {
		result << text->Mid(textEnd + 1);
	}

	*text = result;
}

void EditBox::OnSize(wxSizeEvent& event)
{
	int w, h;
	GetClientSize(&w, &h);
	if (isdetached && w > 850){
		BoxSizer1->Detach(BoxSizer3);

		for (int i = 5; i >= 0; i--){ BoxSizer3->Detach(i); }
		BoxSizer2->Insert(0, Comment, 0, wxLEFT | wxALIGN_CENTER, 4);
		BoxSizer2->Add(ActorEdit, 5, wxEXPAND);
		BoxSizer2->Add(MarginLEdit, 0, wxLEFT, 2);
		BoxSizer2->Add(MarginREdit, 0, wxLEFT, 2);
		BoxSizer2->Add(MarginVEdit, 0, wxLEFT, 2);
		BoxSizer2->Add(EffectEdit, 5, wxLEFT | wxRIGHT | wxEXPAND, 2);
		delete BoxSizer3; BoxSizer3 = NULL;
		SetSizer(BoxSizer1);

		isdetached = false;
	}
	else if (!isdetached && w <= 850)
	{
		for (int i = 11; i >= 7; i--){ BoxSizer2->Detach(i); }
		BoxSizer2->Detach(0);
		BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
		BoxSizer3->Add(Comment, 0, wxLEFT | wxALIGN_CENTER, 4);
		BoxSizer3->Add(ActorEdit, 5, wxEXPAND | wxLEFT, 2);
		BoxSizer3->Add(MarginLEdit, 0, wxLEFT, 2);
		BoxSizer3->Add(MarginREdit, 0, wxLEFT, 2);
		BoxSizer3->Add(MarginVEdit, 0, wxLEFT, 2);
		BoxSizer3->Add(EffectEdit, 5, wxLEFT | wxRIGHT | wxEXPAND, 2);
		BoxSizer1->Add(BoxSizer3, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 2);
		SetSizer(BoxSizer1);

		isdetached = true;
	}

	if (ABox){
		wxSize aboxSize = ABox->GetClientSize();
		int minEBSize = (TextEditOrig->IsShown()) ? 200 : 150;
		if ((h - aboxSize.y) < minEBSize){
			ABox->SetMinSize(wxSize(-1, h - minEBSize));
			Options.SetInt(AUDIO_BOX_HEIGHT, h - minEBSize);
		}
	}

	event.Skip();

}

void EditBox::HideControls()
{
	bool state = grid->subsFormat < SRT;

	Ban->Enable(state);
	Bcol2->Enable(state);
	Bcol3->Enable(state);
	Bcol4->Enable(state);
	Comment->Enable(state);
	LayerEdit->Enable(state);
	StyleChoice->Enable(state);
	StyleEdit->Enable(state);
	ActorEdit->Enable(state);
	MarginLEdit->Enable(state);
	MarginREdit->Enable(state);
	MarginVEdit->Enable(state);
	EffectEdit->Enable(state);

	state = grid->subsFormat < SRT || grid->subsFormat == MDVD;
	Bcol1->Enable(state);
	Bfont->Enable(state);

	state = grid->subsFormat <= SRT || grid->subsFormat == MDVD;
	Bbold->Enable(state);

	state = grid->subsFormat <= SRT;
	Bund->Enable(state);
	Bstrike->Enable(state);

	state = grid->subsFormat != TMP;
	EndEdit->Enable(state);
	DurEdit->Enable(state);
	Bital->Enable(state);
	if (state){ BoxSizer4->Layout(); }
}

void EditBox::ClearErrs(bool spellcheckerOnOff/*=false*/, bool enableSpellchecker /*= false*/)
{
	Notebook *nb = Notebook::GetTabs();
	for (size_t i = 0; i < nb->Size(); i++)
	{
		TabPanel* tab = nb->Page(i);
		tab->Grid->SpellErrors.clear();
		if (spellcheckerOnOff)
			tab->Edit->TextEdit->SpellcheckerOnOff(enableSpellchecker);
		else
			tab->Edit->TextEdit->ClearSpellcheckerTable();
	}
	grid->Refresh(false);
	TextEdit->Refresh(false);
}

void EditBox::OnSplit(wxCommandEvent& event)
{
	wxString Splitchar = (grid->subsFormat <= SRT) ? L"\\N" : L"|";
	bool isOriginal = (grid->hasTLMode && TextEdit->GetValue() == L"" && !TextEdit->HasFocus());
	//text editor
	TextEditor *tedit = (isOriginal) ? TextEditOrig : TextEdit;
	wxString txt = tedit->GetValue();
	long strt, ennd;
	tedit->GetSelection(&strt, &ennd);
	if (strt > 0 && txt[strt - 1] == L' '){ strt--; }
	if (ennd < (int)txt.length() && txt[ennd] == L' '){ ennd++; }

	if (strt != ennd){ txt.Remove(strt, ennd - strt); }
	
	txt.insert(strt, Splitchar);
	if (tedit->HasRTLText()) {
		SwapRTLTextLines(&txt, strt, Splitchar);
	}
	
	tedit->SetTextS(txt, true);
	long whre = strt + Splitchar.length();
	tedit->SetSelection(whre, whre);
}

void EditBox::OnHideOriginal(wxCommandEvent& event)
{
	wxString texttl = TextEditOrig->GetValue();
	texttl = L"{" + texttl + L"}";
	TextEdit->SetFocus();
	TextEditOrig->SetTextS(texttl, true);
}

void EditBox::OnPasteDifferents(wxCommandEvent& event)
{
	if (tab->Video->GetState() == None){ wxBell(); return; }
	int idd = event.GetId();
	int vidtime = tab->Video->Tell();
	if (vidtime < line->Start.mstime || vidtime > line->End.mstime){ wxBell(); return; }
	int diff = (idd == EDITBOX_START_DIFFERENCE) ? vidtime - ZEROIT(line->Start.mstime) : abs(vidtime - ZEROIT(line->End.mstime));
	long startPosition, endPosition;
	TextEdit->GetSelection(&startPosition, &endPosition);
	wxString diffAsString;
	diffAsString << diff;
	TextEdit->Replace(startPosition, endPosition, diffAsString);
	int npos = startPosition + diffAsString.length();
	TextEdit->SetSelection(npos, npos);
}

void EditBox::OnEdit(wxCommandEvent& event)
{
	//subs preview will switch grid to preview grid that's why we need to change its video, we dont want to change subtitles
	bool startEndFocus = StartEdit->HasFocus() || EndEdit->HasFocus();
	bool durFocus = DurEdit->HasFocus();
	
	bool visible = true;
	if (startEndFocus){
		line->End = EndEdit->GetTime(2);
		line->Start = StartEdit->GetTime(1);
		if (line->Start > line->End){
			if (StartEdit->HasFocus()){
				StartEdit->SetForegroundColour(WINDOW_WARNING_ELEMENTS);
				StartEdit->changedBackGround = true;
			}
			else{
				EndEdit->SetForegroundColour(WINDOW_WARNING_ELEMENTS);
				EndEdit->changedBackGround = true;
			}
		}
		else if (StartEdit->changedBackGround){
			StartEdit->SetForegroundColour(WINDOW_TEXT);
		}
		else if (EndEdit->changedBackGround){
			EndEdit->SetForegroundColour(WINDOW_TEXT);
		}

		STime durTime = line->End - line->Start;
		if (durTime.mstime < 0){ durTime.mstime = 0; }
		DurEdit->SetTime(durTime, false, 1);
	}
	else if (durFocus){
		line->End = line->Start + DurEdit->GetTime();
		EndEdit->SetTime(line->End, false, 2);
		EndEdit->MarkDirty();
	}
	if (durFocus || startEndFocus){
		if (ABox && ABox->IsShown()){ ABox->audioDisplay->SetDialogue(line, currentLine); }
		UpdateChars();
	}

	int saveAfter = Options.GetInt(GRID_SAVE_AFTER_CHARACTER_COUNT);
	if (saveAfter && EditCounter >= saveAfter){
		Send(EDITBOX_LINE_EDITION, false, false, true);
		if (hasPreviewGrid){
			tab->Grid->RefreshPreview();
		}
		EditCounter = 1;
		if (ABox && ABox->audioDisplay->hasKara && event.GetId() > 0)
			ABox->audioDisplay->SetDialogue(line, currentLine);
	}
	else{ EditCounter++; }

	if (hasPreviewGrid)
		return;

	if (Visual > 0){
		tab->Video->SetVisual(true);
		return;
	}

	int openFlag = CLOSE_SUBTITLES;
	if (tab->Video->GetState() != None){
		//visible=true;
		visible = grid->IsLineVisible();
		if (!visible && (lastVisible != visible || grid->file->IsSelected(currentLine))){ 
			visible = true; 
			lastVisible = false; 
		}
		else{ lastVisible = visible; }
		//make sure that dummy edition is true when line is not visible
		if (!visible){
			tab->Video->GetRenderer()->m_HasDummySubs = true;
		}
		else {
			openFlag = OPEN_DUMMY;
		}
	}

	if (visible && (tab->Video->IsShown() || tab->Video->IsFullScreen())){
		tab->Video->OpenSubs(openFlag);
		if (Visual > 0){ tab->Video->ResetVisual(); }
		else if (tab->Video->GetState() == Paused){ tab->Video->Render(); }
	}

}

void EditBox::OnColorChange(ColorEvent& event)
{
	AssColor choosenColor = event.GetColor();
	wxString choosenColorAsString = choosenColor.GetAss(false, true);
	if (grid->subsFormat < SRT){
		int intColorNumber = event.GetColorType();
		wxString colorNumber;
		colorNumber << intColorNumber;
		wxString colorString;
		wxString tag = (intColorNumber == 1) ? L"?c&(.*)&" : L"c&(.*)&";
		Styles *style = grid->GetStyle(0, line->Style);

		bool found = FindTag(colorNumber + tag, L"", 0, true);
		//check only colors, not aplha
		if (actualColor.r != choosenColor.r || actualColor.g != choosenColor.g || 
			actualColor.b != choosenColor.b){

			PutTagInText(L"\\" + colorNumber + L"c" + choosenColorAsString + L"&", 
				L"\\" + colorNumber + L"c" + actualColorstr + L"&", false);
		}
		//else if (found)
			//PutTagInText(L"", L"", false);

		if (found = FindTag(L"(" + colorNumber + L"a&|alpha.*)", L"", 0, true)){
			GetTextResult(&colorString);
			if (!colorString.StartsWith(colorNumber + L"a&")){
				//colorString = colorString.Mid(5);
				wxPoint pos = GetPositionInText();
				pos.y++;
				pos.x = pos.y;
				SetPositionInText(pos);
			}
			//col.SetAlphaString(colorString);
		}

		if (actualColor.a != choosenColor.a){
			PutTagInText(L"\\" + colorNumber + wxString::Format(L"a&H%02X&", choosenColor.a), 
				L"\\" + colorNumber + wxString::Format(L"a&H%02X&", actualColor.a), false);
		}
		//else if (found)
			//PutTagInText(L"", L"", false);

	}
	else{ PutinNonass(L"C:" + choosenColorAsString.Mid(2), L"C:([^}]*)"); }
}

void EditBox::OnButtonTag(wxCommandEvent& event)
{
	wxArrayString tagOptions;
	Options.GetTable((CONFIG)(event.GetId() - EDITBOX_TAG_BUTTON1 + EDITBOX_TAG_BUTTON_VALUE1), tagOptions);
	if (tagOptions.size() < 2){ wxBell(); return; }
	wxString type = tagOptions[1];
	wxString tag = tagOptions[0];

	if (type != L"2"){
		if (!tag.StartsWith(L"\\")){ tag.Prepend(L"\\"); }
		wxString delims = L"1234567890-&()[]";
		bool found = false;
		wxString findtag;
		for (int i = 2; i < (int)tag.length(); i++)
		{
			if (delims.Find(tag[i]) != -1)
			{
				found = true;
				findtag = tag.SubString(1, i - 1);
				break;
			}
		}
		if (!found){ findtag = tag.AfterFirst(L'\\'); }
		wxString result;
		//fn and r do not have number value we have to treat it special \r works good only when return itself as a value
		//we did not need this value at all.
		bool isFN = findtag.StartsWith(L"fn");
		bool isR = findtag.StartsWith(L"r");
		wxString pattern = (isFN) ? L"fn(.*)" :
			(isR) ? L"(r.*)" : findtag + L"([0-9\\(&-].*)";
		bool onceInText = type == L"1";
		if (FindTag(pattern, L"", onceInText, !onceInText)) {
			GetTextResult(&result);
		}
		else if(!isR){
			wxString tagTofind = (isFN) ? L"fn" : findtag;
			if (!TagValueFromStyle(NULL, tagTofind, &result))
				result = L"0";
		}
		wxString resetTag = result.empty() ? L"" : (isFN) ? L"\\fn" + result : (isR) ? L"\\" + result : L"\\" + findtag + result;
		PutTagInText(tag, resetTag);
	}
	else{
		bool oneline = (grid->file->SelectionsSize() < 2);

		long from, to;
		wxString txt = TextEdit->GetValue();
		TextEditor *editor = TextEdit;
		if (grid->hasTLMode && txt == L""){ 
			txt = TextEditOrig->GetValue(); 
			editor = TextEditOrig; 
		}
		editor->GetSelection(&from, &to);
		if (oneline){
			if (from != to){
				txt.erase(txt.begin() + from, txt.begin() + to);
			}
			int klamras = txt.Mid(from).Find(L'{');
			int klamrae = txt.Mid(from).Find(L'}');

			if (klamrae != -1 && (klamras == -1 || klamras > klamrae) && klamras < from && klamrae > from){
				from += klamrae + 1;
			}
			txt.insert(from, tag);
			from += tag.length();
			editor->SetTextS(txt, true);
			editor->SetSelection(from, from);
		}
		else{
			wxArrayInt sels;
			grid->file->GetSelections(sels);
			for (size_t i = 0; i < sels.size(); i++){
				long cpyfrom = from;
				Dialogue *dialc = grid->CopyDialogue(sels[i]);
				wxString &txt = dialc->GetText();
				int klamras = txt.Mid(from).Find(L'{');
				int klamrae = txt.Mid(from).Find(L'}');

				if (klamrae != -1 && (klamras == -1 || klamras > klamrae) && klamras<from && klamrae>from){
					cpyfrom += klamrae + 1;
				}
				if (cpyfrom >= txt.size())
					cpyfrom = txt.size();
				txt.insert(cpyfrom, tag);
			}
			grid->SetModified(EDITBOX_MULTILINE_EDITION);
			grid->Refresh(false);

		}

	}

}

class NumTagButtons : public KaiDialog
{
public:
	NumTagButtons(wxWindow *parent)
		:KaiDialog(parent, -1, _("Zmień ilość przycisków"))
	{
		DialogSizer *sizer = new DialogSizer(wxVERTICAL);
		wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
		numTagButtons = new NumCtrl(this, -1, Options.GetString(EDITBOX_TAG_BUTTONS), 0, 20, true);
		MappedButton *ok = new MappedButton(this, wxID_OK, L"OK");
		MappedButton *cancel = new MappedButton(this, wxID_CANCEL, _("Anuluj"));
		buttonSizer->Add(ok, 1, wxEXPAND | wxALL, 4);
		buttonSizer->Add(cancel, 1, wxEXPAND | wxALL, 4);
		sizer->Add(numTagButtons, 1, wxEXPAND | wxALL, 4);
		sizer->Add(buttonSizer, 1, wxEXPAND, 0);
		sizer->SetMinSize(wxSize(200, -1));
		SetSizerAndFit(sizer);
		MoveToMousePosition(this);
	}

	NumCtrl *numTagButtons;
};

void EditBox::OnEditTag(wxCommandEvent &event)
{
	int id = event.GetId();
	if (id == ID_NUM_TAG_BUTTONS){
		NumTagButtons ntb(this);
		if (ntb.ShowModal() == wxID_CANCEL){ return; }
		Options.SetInt(EDITBOX_TAG_BUTTONS, ntb.numTagButtons->GetInt());
		SetTagButtons();
		return;
	}
	wxWindow *win = FindWindow(id);
	if (!win){ return; }
	TagButton *tb = (TagButton*)win;
	if (!tb){ return; }

	TagButtonDialog tagtxt(tb, -1, tb->tag, tb->name, tb->type);

	if (tagtxt.ShowModal() == wxID_OK){
		tb->tag = tagtxt.txt->GetValue();
		tb->type = tagtxt.type->GetSelection();
		wxString newname = tagtxt.name->GetValue();
		if (newname != tb->name){
			tb->SetLabelText(newname); tb->name = newname;
			Menu *menu = TagButtonManager->GetMenu();
			MenuItem *item = menu->FindItem(id);
			if (item){
				item->label = newname;
			}
		}
		wxString svtag = L"{\n\t" + tb->tag;
		Options.SetString((CONFIG)(id - EDITBOX_TAG_BUTTON1 + EDITBOX_TAG_BUTTON_VALUE1),
			svtag << L"\n\t" << tb->type << L"\n\t" << tb->name << L"\n}");
		Options.SaveOptions(true, false);
		if (tb->tag != L""){ tb->SetToolTip(tb->tag); }
	}

}

void EditBox::OnAutoMoveTags(wxCommandEvent& event)
{
	SetTextWithTags(true);
	Options.SetBool(AUTO_MOVE_TAGS_FROM_ORIGINAL, AutoMoveTags->GetValue());
	Options.SaveOptions();
}

void EditBox::SetTextWithTags(bool RefreshVideo)
{
	if (grid->hasTLMode && line->TextTl == L"" && AutoMoveTags->GetValue() && Visual <= CROSS){
		wxString Text = line->Text;
		Text.Replace(L"}{", L"");
		int getr = Text.Find(L'}');
		if (getr > -1){
			int brackets = Text.find(L"{");
			wxString restText;
			if (Text.length() > (size_t)getr + 1){ restText = Text.Mid(getr + 1); }
			int pos = 0;
			wxString txtOrg;
			wxString txtTl;
			if (Text.StartsWith(L"{")){
				txtTl = Text.substr(0, getr + 1);
				pos = txtTl.length();
			}
			else if (brackets > 0){
				txtOrg = Text.substr(0, brackets);
				txtTl = Text.SubString(brackets, getr);
			}
			else{
				txtOrg = Text.substr(0, getr + 1);
			}

			while (1){
				brackets = restText.find(L"{");
				getr = restText.Find(L'}');
				if (brackets != -1 && getr != -1){
					txtOrg += restText.substr(0, brackets);
					txtTl += restText.SubString(brackets, getr);
					if (restText.length() > (size_t)getr + 1){ 
						restText = restText.Mid(getr + 1); 
					}
					else{ break; }
				}
				else{
					txtOrg += restText;
					break;
				}
			}



			TextEdit->SetTextS(txtTl, false, true, false, false);
			TextEditOrig->SetTextS(txtOrg, false, true, false, false);
			splittedTags = true;

			TextEdit->SetSelection(pos, pos);
			TextEdit->SetFocus();
			goto done;
		}
	}
	if (splittedTags){ delete line; line = grid->GetDialogue(currentLine)->Copy(); }
	splittedTags = false;
	TextEdit->SetTextS((TextEditOrig->IsShown()) ? line->TextTl : line->Text, TextEdit->IsModified(), true, false, false);
	if (TextEditOrig->IsShown()){ TextEditOrig->SetTextS(line->Text, TextEditOrig->IsModified(), true, false, false); }
done:
	if (RefreshVideo){
		VideoCtrl *vb = tab->Video;
		if (vb->GetState() != None){
			vb->OpenSubs(OPEN_DUMMY);
			vb->Render();
		}
	}
}

void EditBox::OnCursorMoved(wxCommandEvent& event)
{
	if (Visual == SCALE || Visual == ROTATEZ || Visual == ROTATEXY || Visual == CLIPRECT || Visual == ALL_TAGS){
		//here grid has nothing to do if someone would want to make effect by visual on this video, 
		//he can without problems
		tab->Video->ResetVisual();
	}
}

void EditBox::OnChangeTimeDisplay(wxCommandEvent& event)
{
	bool frame = Frames->GetValue();
	grid->ChangeTimeDisplay(frame);
	wxDELETE(line);
	line = grid->GetDialogue(currentLine)->Copy();
	StartEdit->ShowFrames(frame);
	EndEdit->ShowFrames(frame);
	DurEdit->ShowFrames(frame);
	StartEdit->SetTime(line->Start, false, 1);
	EndEdit->SetTime(line->End, false, 2);
	if (frame){
		STime durationTime = EndEdit->GetTime() - StartEdit->GetTime();
		durationTime.orgframe++;
		DurEdit->SetTime(durationTime);
	}
	else{
		DurEdit->SetTime(line->End - line->Start);
	}

	grid->RefreshColumns(START | END);
	Options.SetBool(EDITBOX_TIMES_TO_FRAMES_SWITCH, frame);
}

bool EditBox::SetBackgroundColour(const wxColour &col)
{
	if (ABox){ ABox->SetBackgroundColour(col); }
	wxWindow::SetBackgroundColour(col);
	return true;
}

//shows style editing window without style manager, window is non modal.
void EditBox::OnStyleEdit(wxCommandEvent& event)
{
	StyleStore::ShowStyleEdit();
}

bool EditBox::IsCursorOnStart()
{
	// is it possible that some selections can be hidden?
	if (grid->file->SelectionsSize() > 1){ return true; }
	/*if(Visual == CLIPRECT || Visual == MOVE){return true;}
	wxString txt=TextEdit->GetValue();
	MTextEditor *GLOBAL_EDITOR = TextEdit;
	if(grid->transl && txt=="" ){
	txt = TextEditOrig->GetValue();
	GLOBAL_EDITOR = TextEditOrig;
	}
	long from=0, to=0;
	GLOBAL_EDITOR->GetSelection(&from, &to);
	if(from == 0 || txt.StartsWith("{")){
	txt.Replace("}{","");
	int endBracket = txt.Find(L'}');
	if(endBracket == -1 || endBracket <= from+1){
	return true;
	}
	}*/
	return false;
}


void EditBox::OnDoubtfulTl(wxCommandEvent& event)
{
	if (!grid->hasTLMode){ wxBell(); return; }
	line->ChangeState(4);
	wxArrayInt sels;
	grid->file->GetSelections(sels);
	for (size_t i = 0; i < sels.size(); i++){
		Dialogue *dial = grid->file->CopyDialogue(sels[i]);
		dial->ChangeState(4);
	}
	if (event.GetId() == EDITBOX_SET_DOUBTFUL){
		grid->NextLine();
	}
	else{
		grid->Refresh(false);
	}
}

void EditBox::FindNextDoubtfulTl(wxCommandEvent& event)
{
	if (!grid->hasTLMode){ wxBell(); return; }
SeekDoubtful:
	for (size_t i = CurrentDoubtful; i < grid->GetCount(); i++){
		Dialogue *dial = grid->GetDialogue(i);
		if (!dial->isVisible)
			continue;

		if (dial->IsDoubtful()){
			SetLine(i);
			grid->SelectRow(i);
			grid->ScrollTo(i, true);
			CurrentDoubtful = i + 1;
			return;
		}
	}
	if (CurrentDoubtful == 0){ KaiMessageBox(_("Nie znaleziono niepewnych")); return; }
	CurrentDoubtful = 0;
	goto SeekDoubtful;
}

void EditBox::FindNextUnTranslated(wxCommandEvent& event)
{
	if (!grid->hasTLMode){ wxBell(); return; }
SeekUntranslated:
	for (size_t i = CurrentUntranslated; i < grid->GetCount(); i++){
		Dialogue *dial = grid->GetDialogue(i);
		if (!dial->isVisible)
			continue;

		if (dial->TextTl == L""/* && !dial->IsComment*/){
			SetLine(i);
			grid->SelectRow(i);
			grid->ScrollTo(i, true);
			CurrentUntranslated = i + 1;
			return;
		}
	}
	if (CurrentUntranslated == 0){ KaiMessageBox(_("Nie znaleziono nieprzetłumaczonych")); return; }
	CurrentUntranslated = 0;
	goto SeekUntranslated;
}

void EditBox::SetTagButtons()
{
	//Twenty buttons + last with arrow
	//11 is num of other elements then buttons
	//there should be used const int for it
	int numofButtons = BoxSizer4->GetItemCount() - 11;
	int numTagButtons = Options.GetInt(EDITBOX_TAG_BUTTONS);
	if (numTagButtons > numofButtons){
		Menu *menu = new Menu();
		for (int i = 0; i < numTagButtons; i++)
		{
			wxArrayString tagOption;
			Options.GetTable((CONFIG)(i + EDITBOX_TAG_BUTTON_VALUE1), tagOption, wxTOKEN_RET_EMPTY_ALL);
			wxString name;
			wxString tag;
			int type = 0;
			if (tagOption.size() > 2){ name = tagOption[2]; }
			else{ name = wxString::Format(L"T%i", i + 1); }
			if (tagOption.size() > 1){
				type = wxAtoi(tagOption[1]);
			}
			if (tagOption.size() > 0){
				tag = tagOption[0];
			}
			if (i >= numofButtons){
				if (!TagButtonManager){
					BoxSizer4->Add(new TagButton(this, EDITBOX_TAG_BUTTON1 + i, name, tag, type, wxDefaultSize), 0, wxALL, 2);
				}
				else if (i >= numofButtons){
					BoxSizer4->Insert(10 + i, new TagButton(this, EDITBOX_TAG_BUTTON1 + i, name, tag, type, wxDefaultSize), 0, wxALL, 2);
				}
				Connect(EDITBOX_TAG_BUTTON1 + i, TAG_BUTTON_EDITION, (wxObjectEventFunction)&EditBox::OnEditTag);
				Connect(EDITBOX_TAG_BUTTON1 + i, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EditBox::OnButtonTag);
			}
			menu->Append(EDITBOX_TAG_BUTTON1 + i, name);
		}
		menu->Append(ID_NUM_TAG_BUTTONS, _("Zmień ilość przycisków"));
		if (!TagButtonManager){
			TagButtonManager = new MenuButton(this, -1, _("Zarządzaj przyciskami tagów"), wxDefaultPosition, wxDefaultSize);
			BoxSizer4->Add(TagButtonManager, 0, wxALIGN_CENTER | wxALL, 2);
			Connect(ID_NUM_TAG_BUTTONS, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EditBox::OnEditTag);
		}
		TagButtonManager->PutMenu(menu);
		Layout();
	}
	else{
		Menu *menu = TagButtonManager->GetMenu();
		for (int i = numTagButtons; i < numofButtons; i++)
		{
			menu->Delete(numTagButtons);
			wxSizerItem *item = BoxSizer4->GetItem(numTagButtons + 10);
			wxWindow *win = item->GetWindow();
			BoxSizer4->Remove(numTagButtons + 10);
			win->Destroy();
		}
		Layout();
	}




}

void EditBox::SetActiveLineToDoubtful()
{
	CurrentDoubtful = currentLine;
	CurrentUntranslated = currentLine;
};

void EditBox::RebuildActorEffectLists()
{
	ActorEdit->Clear();
	EffectEdit->Clear();
	for (size_t i = 0; i < grid->GetCount(); i++){
		Dialogue *dial = grid->GetDialogue(i);
		if (!dial->Actor.empty() && ActorEdit->FindString(dial->Actor, true) < 0){
			ActorEdit->Append(dial->Actor);
		}
		if (!dial->Effect.empty() && EffectEdit->FindString(dial->Effect, true) < 0){
			EffectEdit->Append(dial->Effect);
		}
	}
	ActorEdit->Sort();
	EffectEdit->Sort();
}

void EditBox::SetGrid(SubsGrid *_grid, bool isPreview){
	if (grid != _grid){
		grid = _grid;
		hasPreviewGrid = isPreview;
		RebuildActorEffectLists();
		RefreshStyle();
		if (isPreview){
			tab->Video->RemoveVisual();
		}
		tab->Video->DisableVisuals(isPreview);
	}
}

bool EditBox::LoadAudio(const wxString &audioFileName, bool fromVideo)
{
	if (ABox){
		ABox->SetFile(audioFileName, fromVideo);
	}
	else{
		ABox = new AudioBox(this, grid);
		ABox->SetFile(audioFileName, fromVideo);

		if (ABox->audioDisplay->loaded){
			windowResizer = new KaiWindowResizer(this,
				[=](int newpos){//canSize
				wxSize size = GetClientSize();
				int minEBSize = (TextEditOrig->IsShown()) ? 200 : 150;
				return (newpos > 150 && size.y - newpos > minEBSize);
			}, [=](int newpos, bool shiftDown){//doSize
				ABox->SetMinSize(wxSize(-1, newpos));
				BoxSizer1->Layout();
				TextEdit->Refresh(false);
				TlMode->Refresh(false);
				Options.SetInt(AUDIO_BOX_HEIGHT, newpos);
				Options.SaveAudioOpts();
			});
			BoxSizer1->Prepend(windowResizer, 0, wxEXPAND);
			BoxSizer1->Prepend(ABox, 0, wxLEFT | wxRIGHT | wxEXPAND, 4);

			if (!tab->Video->IsShown()){
				int audioHeight = Options.GetInt(AUDIO_BOX_HEIGHT);
				int minEBSize = (TextEditOrig->IsShown()) ? 200 : 150;
				SetMinSize(wxSize(500, audioHeight + minEBSize));
				GetParent()->Layout();
			}else
				Layout();
			
			ABox->audioDisplay->SetFocus();
		}
		else{ ABox->Destroy(); ABox = NULL; }
	}
	if (!ABox)
		return false;
	else if (!ABox->audioDisplay->loaded){
		ABox->Destroy();
		ABox = NULL;
		return false;
	}

	return true;
}

void EditBox::CloseAudio()
{
	if (!windowResizer || !ABox)
		return;

	BoxSizer1->Remove(1);
	BoxSizer1->Remove(0);
	windowResizer->Destroy();
	windowResizer = NULL;
	ABox->Destroy();
	ABox = NULL;
	Layout();
}

TextEditor * EditBox::GetEditor(const wxString &text)
{
	if (!text.empty()){
		if (TextEditOrig->GetValue() == text)
			return TextEditOrig;

		return TextEdit;
	}
	else if (grid->hasTLMode && TextEdit->GetValue() == L"")
		return TextEditOrig;

	return TextEdit;
}

bool EditBox::SetFont(const wxFont &font)
{
	wxFont ebFont = font;
	ebFont.SetPointSize(font.GetPointSize() - 1);
	const wxWindowList& siblings = GetChildren();

	for (wxWindowList::compatibility_iterator nodeAfter = siblings.GetFirst();
		nodeAfter;
		nodeAfter = nodeAfter->GetNext()){

		wxWindow *win = nodeAfter->GetData();
		win->SetFont(ebFont);
	}

	wxWindow::SetFont(ebFont);
	int fw, fh;
	GetTextExtent(L"000000", &fw, &fh);
	fw += 6;
	fh += 10;
	LayerEdit->SetMinSize(wxSize(fw, fh));
	GetTextExtent(L"00:00:00,000", &fw, &fh);
	fw += 6;
	fh += 10;
	StartEdit->SetMinSize(wxSize(fw, fh));
	EndEdit->SetMinSize(wxSize(fw, fh));
	DurEdit->SetMinSize(wxSize(fw, fh));
	GetTextExtent(L"0000", &fw, &fh);
	fw += 10;
	fh += 10;
	MarginLEdit->SetMinSize(wxSize(fw, fh));
	MarginREdit->SetMinSize(wxSize(fw, fh));
	MarginVEdit->SetMinSize(wxSize(fw, fh));

	/*if (ABox){
		ABox->Layout();
	}*/
	Layout();
	return true;
}
