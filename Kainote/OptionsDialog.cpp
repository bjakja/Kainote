//  Copyright (c) 2016 - 2026, Marcin Drob

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


#include "Registry.h"
#include "OptionsDialog.h"
#include "config.h"
#include "KainoteFrame.h"
#include "Hotkeys.h"
#include "NumCtrl.h"
#include "ColorPicker.h"
#include "KaiTextCtrl.h"
#include "FontDialog.h"
#include "OpennWrite.h"
#include "KaiMessageBox.h"
#include "KaiStaticText.h"
#include "OptionsPanels.h"
#include "StyleChange.h"
#include "SubtitlesProviderManager.h"
#include "SpellChecker.h"
#include "TabPanel.h"
#include "EditBox.h"
#include "Notebook.h"
#include "SubsGrid.h"
#include "AudioBox.h"
#include "KaiStaticBoxSizer.h"
#include "FontEnumerator.h"
#include <wx/dir.h>
#include <wx/dirdlg.h>
#include <wx/filename.h>
//config have Windows trash
#include "config.h"

void ItemHotkey::OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, KaiListCtrl *theList)
{
	wxSize ex = dc->GetTextExtent(accel);

	if (modified){ 
		dc->SetTextForeground(Options.GetColour(WINDOW_WARNING_ELEMENTS)); 
	}
	else{ dc->SetTextForeground(Options.GetColour(theList->IsThisEnabled() ? WINDOW_TEXT : WINDOW_TEXT_INACTIVE)); }
	needTooltip = ex.x > width - 8;
	wxRect cur(x, y, width - 8, height);
	dc->SetClippingRegion(cur);
	dc->DrawLabel(accel, cur, wxALIGN_CENTER_VERTICAL);
	dc->DestroyClippingRegion();
}

void ItemHotkey::OnMouseEvent(wxMouseEvent &event, bool enter, bool leave, KaiListCtrl *theList, Item **changed)
{
	if (enter){
		if (needTooltip)
			theList->SetToolTip(accel);
		else if (theList->HasToolTips())
			theList->UnsetToolTip();
	}
}

void ItemHotkey::OnMapHotkey(KaiListCtrl *theList, int y)
{
	HkeysDialog hkd(theList, name, hotkeyId.Type, !name.StartsWith("Script"));

	if (hkd.ShowModal() == wxID_OK){
		if (OptionsDialog::hotkeysCopy.size() == 0)
			OptionsDialog::hotkeysCopy = std::map<idAndType, hdata>(Hkeys.GetHotkeysMap());

		wxString hotkey = accel;
		
		std::vector< std::map<idAndType, hdata>::iterator> idtypes;
		for (auto cur = OptionsDialog::hotkeysCopy.begin(); cur != OptionsDialog::hotkeysCopy.end(); cur++){
			if (cur->second.Accel == hkd.hotkey && cur->first.id != hotkeyId.id){
				idtypes.push_back(cur);
			}
		}
		if (idtypes.size()){
			bool doubledHotkey = false;
			wxString doubledHkName;
			for (auto &idtype : idtypes){
				if (idtype->first.Type == hkd.type ||
					(idtype->first.Type >= VIDEO_HOTKEY && hkd.type >= VIDEO_HOTKEY) ||
					((hotkeyId.id >= VIDEO_PLAY_PAUSE && hotkeyId.id <= VIDEO_5_SECONDS_BACKWARD ||
					idtype->first.id >= VIDEO_PLAY_PAUSE && idtype->first.id <= VIDEO_5_SECONDS_BACKWARD) &&
					(idtype->first.Type >= GRID_HOTKEY || hkd.type >= GRID_HOTKEY))){
					doubledHotkey = true;
					doubledHkName = Hkeys.GetName(idtype->first.id);
					if (doubledHkName.empty())
						doubledHkName = idtype->second.Name;
					break;
				}
				else{
					if (!doubledHkName.empty())
						doubledHkName += L", ";

					wxString hotkeyName = Hkeys.GetName(idtype->first.id);
					if (hotkeyName.empty())
						hotkeyName = idtype->second.Name;
					doubledHkName += OptionsDialog::windowNames[idtype->first.Type] + L" " + hotkeyName;
				}
			}

			int result = wxCANCEL;
			if (doubledHotkey){
				KaiMessageDialog msg(theList,
					wxString::Format(_("This hotkey already exists for \"%s\".\nWhat to do?"),
					doubledHkName), _("Warning"), wxYES | wxOK | wxCANCEL);
				msg.SetOkLabel(_("Switch hotkeys"));
				msg.SetYesLabel(_("Delete hotkey"));
				result = msg.ShowModal();
			}
			else{
				int buttonFlag = (idtypes.size() < 2) ? wxOK : 0;
				KaiMessageDialog msg(theList,
					wxString::Format(_("This shortcut already exists in %s as a shortcut for \"%s\".\nWhat would you like to do?"),
					(idtypes.size() > 1) ? _("other windows") : _("another window"), doubledHkName), _("Warning"), wxYES_NO | buttonFlag | wxCANCEL);
				if (idtypes.size() < 2)
					msg.SetOkLabel(_("Switch hotkeys"));
				msg.SetYesLabel(_("Delete hotkey"));
				msg.SetNoLabel(_("Set anyway"));
				result = msg.ShowModal();
			}
			if (result == wxYES || result == wxOK){
				if (result == wxYES){ hotkey = emptyString; }
				for (auto &idtype : idtypes){
					if (doubledHotkey && idtype->first.Type != hkd.type)
						continue;

					int nitem = theList->FindItem(0, OptionsDialog::windowNames[idtype->first.Type] + L" " + Hkeys.GetName(idtype->first.id));
					if (nitem >= 0){
						ItemHotkey* item = (ItemHotkey*)theList->CopyRow(nitem, 1);
						item->accel = hotkey;
						item->modified = true;
						ItemText* textitem = (ItemText*)theList->GetItem(nitem, 0);
						textitem->modified = true;
						theList->Refresh(false);
						idtype->second.Accel = hotkey;
					}
				}
			}
			else if (result == wxCANCEL){ return; }
		}

		if (hotkeyId.Type != hkd.type){
			int nitem = theList->FindItem(0, OptionsDialog::windowNames[hkd.type] + " " + name);
			if (nitem < 0){
				ItemHotkey* itemcopied = (ItemHotkey*)theList->CopyRow(y, 1, true);
				ItemText *itemtext = (ItemText*)theList->GetItem(theList->GetCount() - 1, 0);
				itemtext->name = OptionsDialog::windowNames[hkd.type] + " " + name;
				itemtext->modified = true;
				itemcopied->accel = hkd.hotkey;
				itemcopied->modified = true;
				itemcopied->hotkeyId = idAndType(hotkeyId.id, hkd.type);
				int pos = theList->GetCount();
				theList->ScrollTo(pos);
				theList->SetSelection(pos);
				OptionsDialog::hotkeysCopy[idAndType(hotkeyId.id, hkd.type)] = hdata(name, hkd.hotkey);
				theList->SetModified(true);
				theList->PushHistory();
			}
			else {
				ItemHotkey* item = (ItemHotkey*)theList->CopyRow(nitem, 1);
				item->accel = hkd.hotkey;
				item->modified = true;
				ItemText* textitem = (ItemText*)theList->GetItem(nitem, 0);
				textitem->modified = true;

				theList->ScrollTo(nitem + 1);
				theList->SetSelection(nitem);
				OptionsDialog::hotkeysCopy[idAndType(hotkeyId.id, hkd.type)] = hdata(name, hkd.hotkey);
				theList->SetModified(true);
				theList->PushHistory();
			}
			return;
		}
		ItemHotkey* item = (ItemHotkey*)theList->CopyRow(y, 1);
		item->accel = hkd.hotkey;
		item->modified = true;
		ItemText* textitem = (ItemText*)theList->GetItem(y, 0);
		textitem->modified = true;
		theList->Refresh(false);
		OptionsDialog::hotkeysCopy[hotkeyId] = hdata(name, hkd.hotkey);
		theList->SetModified(true);
		theList->PushHistory();
	}
}

void ItemHotkey::OnResetHotkey(KaiListCtrl *theList, int y)
{
	if (OptionsDialog::hotkeysCopy.size() == 0)
		OptionsDialog::hotkeysCopy = std::map<idAndType, hdata>(Hkeys.GetHotkeysMap());
	const wxString &defKet = Hkeys.GetDefaultKey(hotkeyId);
	ItemHotkey *itemKey = (ItemHotkey*)theList->CopyRow(y, 1);
	itemKey->accel = defKet;
	itemKey->modified = true;
	ItemText* textitem = (ItemText*)theList->GetItem(y, 0);
	textitem->modified = true;
	theList->SetModified(true);
	theList->Refresh(false);
	theList->PushHistory();
	OptionsDialog::hotkeysCopy[hotkeyId] = hdata(name, defKet);
}

void ItemHotkey::OnDeleteHotkey(KaiListCtrl *theList, int y)
{
	if (OptionsDialog::hotkeysCopy.size() == 0)
		OptionsDialog::hotkeysCopy = std::map<idAndType, hdata>(Hkeys.GetHotkeysMap());
	ItemHotkey *itemKey = (ItemHotkey*)theList->CopyRow(y, 1);
	itemKey->accel = emptyString;
	itemKey->modified = true;
	theList->SetModified(true);
	theList->Refresh(false);
	theList->PushHistory();
	OptionsDialog::hotkeysCopy[hotkeyId] = hdata(name, emptyString);
}

void ItemHotkey::Save()
{
	if (modified){
		modified = false;
	}
}

void ItemHotkey::OnChangeHistory(){
	OptionsDialog::hotkeysCopy[hotkeyId] = hdata(name, accel);
}
//modes 0 All
//1 setted
//2 global
//3 subs
//4 editor
//5 video
//6 audio
int ItemHotkey::OnVisibilityChange(int mode){
	switch (mode){
	case 1:
		return (accel != emptyString) ? VISIBLE : NOT_VISIBLE;
	case 2:
		return (hotkeyId.Type == GLOBAL_HOTKEY) ? VISIBLE : NOT_VISIBLE;
	case 3:
		return (hotkeyId.Type == GRID_HOTKEY) ? VISIBLE : NOT_VISIBLE;
	case 4:
		return (hotkeyId.Type == EDITBOX_HOTKEY) ? VISIBLE : NOT_VISIBLE;
	case 5:
		return (hotkeyId.Type == VIDEO_HOTKEY) ? VISIBLE : NOT_VISIBLE;
	case 6:
		return (hotkeyId.Type == AUDIO_HOTKEY) ? VISIBLE : NOT_VISIBLE;
	default:
		return VISIBLE;
	}
}

wxString *OptionsDialog::windowNames = nullptr;
std::map<idAndType, hdata> OptionsDialog::hotkeysCopy;

OptionsDialog::OptionsDialog(wxWindow* parent)
	: KaiDialog(parent, -1, _("Options"), wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER | wxFULL_REPAINT_ON_RESIZE)
{
	windowNames = new wxString[5]{ _("Global"), _("Subtitles"), _("Editor"), _("Video"), _("Audio") };
	OptionsTree = new KaiTreebook(this, -1);

	Stylelist = nullptr;
	Katlist = nullptr;

	wxIcon icn;
	icn.CopyFromBitmap(CreateBitmapFromPngResource(L"SETTINGS"));
	SetIcon(icn);

	wxWindow* GLOBAL_EDITOR = new wxWindow(OptionsTree, -1);
	wxWindow* EditorAdvanced = new wxWindow(OptionsTree, -1);
	wxWindow* ConvOpt = new wxWindow(OptionsTree, -1);
	wxWindow* Hotkeyss = new wxWindow(OptionsTree, -1);
	wxWindow* AudioMain = new wxWindow(OptionsTree, -1);
	wxWindow* AudioSecond = new wxWindow(OptionsTree, -1);
	wxWindow* video = new wxWindow(OptionsTree, -1);
	wxWindow* Themes = new wxWindow(OptionsTree, -1);
	wxWindow* Assocs = new wxWindow(OptionsTree, -1);
	wxWindow* SubsProps = new SubtitlesProperties(OptionsTree, this);

	hkeymodif = 0;
	if (!Options.AudioOpts && !Options.LoadAudioOpts()) { KaiMessageBox(_("Cannot load audio configuration"), _("Error")); }

	//Main
	{
		const int optsSize = 18;
		wxBoxSizer* MainSizer = new wxBoxSizer(wxVERTICAL);
		wxString labels[optsSize] = { _("Open sorted subtitles"), _("Turn spell checking"),
			_("Select the line with the time line\nof the previous active tab"),
			_("Show suggestions by double-clicking on misspell"), _("Always open subtitles in a new tab"),
			_("Stay on selected line when editing times"),
			_("Turn off edits preview on video\n(re-opening tab is required)"),
			_("Turn searching of visible line\nafter switching from full screen"),
			_("Synchronize time shifting window in all tabs"), _("Change active line after add to selection"),
			_("Show original in translator mode"), _("Hide original on video in translator mode"),
			_("Do not change selections when duplicating dialogue lines"),
			_("Do not vertically center the active line in the subtitle grid"), _("Use numpad shortcuts in text fields"),
			_("Turn off visual tools warning"), _("Do not warn about resolution mismatch"),
			_("Compatibility with older Kainote scripts") };
		CONFIG opts[optsSize] = { GRID_LOAD_SORTED_SUBS, SPELLCHECKER_ON, AUTO_SELECT_LINES_FROM_LAST_TAB,
			EDITBOX_SUGGESTIONS_ON_DOUBLE_CLICK, OPEN_SUBS_IN_NEW_TAB, EDITBOX_DONT_GO_TO_NEXT_LINE_ON_TIMES_EDIT,
			DISABLE_LIVE_VIDEO_EDITING, GRID_SET_VISIBLE_LINE_AFTER_FULL_SCREEN, SHIFT_TIMES_CHANGE_VALUES_WITH_TAB,
			GRID_CHANGE_ACTIVE_ON_SELECTION, TL_MODE_SHOW_ORIGINAL, TL_MODE_HIDE_ORIGINAL_ON_VIDEO,
			GRID_DUPLICATION_DONT_CHANGE_SELECTION, GRID_DONT_CENTER_ACTIVE_LINE,
			TEXT_FIELD_ALLOW_NUMPAD_HOTKEYS, VIDEO_VISUAL_WARNINGS_OFF,
			DONT_ASK_FOR_BAD_RESOLUTION, AUTOMATION_OLD_SCRIPTS_COMPATIBILITY };
		wxArrayString tags;
		if (wxTranslations* translations = wxTranslations::Get())
			tags = translations->GetAvailableTranslations(KAINOTE_CATALOG_DOMAIN);
		tags.Sort();

		// Built in one pass so the tags and the labels cannot drift apart; the
		// selection is mapped back to a tag by index.
		wxArrayString langs;
		programLanguages.push_back(L"en");
		langs.Add(L"English");
		for (size_t i = 0; i < tags.GetCount(); i++) {
			if (tags[i] == L"en")
				continue;
			programLanguages.push_back(tags[i]);
			langs.Add(Options.FindLanguage(tags[i]));
		}
		KaiStaticBoxSizer* langSizer = new KaiStaticBoxSizer(wxVERTICAL, GLOBAL_EDITOR, _("Language (program restart required)"));
		KaiChoice* programLanguage = new KaiChoice(GLOBAL_EDITOR, ID_PROGRAM_LANGUAGE, wxDefaultPosition, wxDefaultSize, langs);
		int sel = programLanguage->FindString(Options.FindLanguage(Options.GetString(PROGRAM_LANGUAGE)));
		if (sel < 0)
			sel = 0;
		programLanguage->SetSelection(sel);
		programLanguage->SetFocus();
		ConOpt(programLanguage, PROGRAM_LANGUAGE);
		langSizer->Add(programLanguage, 0, wxALL | wxEXPAND, 2);
		MainSizer->Add(langSizer, 0, wxRIGHT | wxEXPAND, 5);
		wxArrayString dictionaries;
		SpellChecker::AvailableDics(dictionaries, dictionaryLanguagesSymbols);
		if (dictionaries.size() == 0) { dictionaries.Add(_("Put files .dic and .aff to \"Dictionary\" folder")); }
		KaiStaticBoxSizer* dicSizer = new KaiStaticBoxSizer(wxVERTICAL, GLOBAL_EDITOR, _("Spell checker language (\"Dictionary\" folder)"));

		KaiChoice* dic = new KaiChoice(GLOBAL_EDITOR, ID_DICTIONARY_LANGUAGE, wxDefaultPosition, wxDefaultSize, dictionaries);

		dic->SetSelection(dic->FindString(Options.FindLanguage(Options.GetString(DICTIONARY_LANGUAGE))));
		ConOpt(dic, DICTIONARY_LANGUAGE);
		dicSizer->Add(dic, 0, wxALL | wxEXPAND, 2);
		MainSizer->Add(dicSizer, 0, wxRIGHT | wxEXPAND, 5);

		for (int i = 0; i < optsSize; i++)
		{
			KaiCheckBox* opt = new KaiCheckBox(GLOBAL_EDITOR, -1, labels[i]);
			opt->SetValue(Options.GetBool(opts[i]));
			ConOpt(opt, opts[i]);
			MainSizer->Add(opt, 0, wxALL, 2);
		}
		GLOBAL_EDITOR->SetSizerAndFit(MainSizer);
	}
	//editor advenced
	{
		wxBoxSizer* Main1Sizer = new wxBoxSizer(wxVERTICAL);
		//Warning id ID_NUMBER_CONTROL is only for NumCtrl, normal text fields have to have another id
		NumCtrl* gridSaveAfter = new NumCtrl(EditorAdvanced, ID_NUMBER_CONTROL, Options.GetString(GRID_SAVE_AFTER_CHARACTER_COUNT), 0, 10000, true, wxDefaultPosition, wxSize(120, -1), wxTE_PROCESS_ENTER);
		gridSaveAfter->SetToolTip(_("0 turns off saving while editing"));
		NumCtrl* autoSaveMax = new NumCtrl(EditorAdvanced, ID_NUMBER_CONTROL, Options.GetString(AUTOSAVE_MAX_FILES), 2, 1000000, true, wxDefaultPosition, wxSize(120, -1), wxTE_PROCESS_ENTER);
		autoSaveMax->SetToolTip(_("Number of autosaves can be set from 2 to 1000000"));
		int numMaxChars = Options.GetInt(TAB_TEXT_MAX_CHARS);
		if (!numMaxChars)
			numMaxChars = 40;
		NumCtrl* maxTabChars = new NumCtrl(EditorAdvanced, ID_NUMBER_CONTROL, std::to_wstring(numMaxChars), 20, 150, true, wxDefaultPosition, wxSize(120, -1), wxTE_PROCESS_ENTER);
		maxTabChars->SetToolTip(_("Number of tab name characters. Range from 20 to 150"));
		NumCtrl* ltl = new NumCtrl(EditorAdvanced, ID_NUMBER_CONTROL, Options.GetString(AUTOMATION_TRACE_LEVEL), 0, 5, true, wxDefaultPosition, wxSize(120, -1), wxTE_PROCESS_ENTER);
		NumCtrl* sc = new NumCtrl(EditorAdvanced, ID_NUMBER_CONTROL, Options.GetString(GRID_INSERT_START_OFFSET), -100000, 100000, true, wxDefaultPosition, wxSize(120, -1), wxTE_PROCESS_ENTER);
		NumCtrl* sc1 = new NumCtrl(EditorAdvanced, ID_NUMBER_CONTROL, Options.GetString(GRID_INSERT_END_OFFSET), -100000, 100000, true, wxDefaultPosition, wxSize(120, -1), wxTE_PROCESS_ENTER);
		KaiTextCtrl* sc2 = new KaiTextCtrl(EditorAdvanced, ID_TAGS_SWAP_CHARACTER, Options.GetString(GRID_TAGS_SWAP_CHARACTER), wxDefaultPosition, wxSize(120, -1), wxTE_PROCESS_ENTER);

		ConOpt(gridSaveAfter, GRID_SAVE_AFTER_CHARACTER_COUNT);
		ConOpt(autoSaveMax, AUTOSAVE_MAX_FILES);
		ConOpt(ltl, AUTOMATION_TRACE_LEVEL);
		ConOpt(maxTabChars, TAB_TEXT_MAX_CHARS);
		ConOpt(sc, GRID_INSERT_START_OFFSET);
		ConOpt(sc1, GRID_INSERT_END_OFFSET);
		ConOpt(sc2, GRID_TAGS_SWAP_CHARACTER);
		wxBoxSizer* MainSizer2 = new wxBoxSizer(wxHORIZONTAL);
		MainSizer2->Add(new KaiStaticText(EditorAdvanced, -1, _("Number of edits to save")/*, wxDefaultPosition, wxSize(256, -1)*/), 5, wxEXPAND);
		MainSizer2->Add(gridSaveAfter, 0, wxEXPAND);
		wxBoxSizer* MainSizer3 = new wxBoxSizer(wxHORIZONTAL);
		MainSizer3->Add(new KaiStaticText(EditorAdvanced, -1, _("Maximum number of autosave files")/*, wxDefaultPosition, wxSize(256, -1)*/), 5, /*wxALIGN_CENTRE_VERTICAL | */wxEXPAND);
		MainSizer3->Add(autoSaveMax, 0, wxEXPAND);
		wxBoxSizer* MainSizer4 = new wxBoxSizer(wxHORIZONTAL);
		MainSizer4->Add(new KaiStaticText(EditorAdvanced, -1, _("Start frame offset in ms:")/*, wxDefaultPosition, wxSize(256, -1)*/), 5, /*wxALIGN_CENTRE_VERTICAL | */wxEXPAND);
		MainSizer4->Add(sc, 0, wxEXPAND);
		wxBoxSizer* MainSizer5 = new wxBoxSizer(wxHORIZONTAL);
		MainSizer5->Add(new KaiStaticText(EditorAdvanced, -1, _("End frame offset in ms:")), 5, /*wxALIGN_CENTRE_VERTICAL | */wxEXPAND);
		MainSizer5->Add(sc1, 0, wxEXPAND);
		wxBoxSizer* MainSizer6 = new wxBoxSizer(wxHORIZONTAL);
		MainSizer6->Add(new KaiStaticText(EditorAdvanced, -1, _("ASS tag replacement:")), 5, /*wxALIGN_CENTRE_VERTICAL | */wxEXPAND);
		MainSizer6->Add(sc2, 0, wxEXPAND);
		wxBoxSizer* MainSizer7 = new wxBoxSizer(wxHORIZONTAL);
		MainSizer7->Add(new KaiStaticText(EditorAdvanced, -1, _("Number of tab name characters")), 5, /*wxALIGN_CENTRE_VERTICAL | */wxEXPAND);
		MainSizer7->Add(maxTabChars, 0, wxEXPAND);
		wxBoxSizer* MainSizer8 = new wxBoxSizer(wxHORIZONTAL);
		MainSizer8->Add(new KaiStaticText(EditorAdvanced, -1, _("LUA scripts tracking level")), 5, /*wxALIGN_CENTRE_VERTICAL | */wxEXPAND);
		MainSizer8->Add(ltl, 0, wxEXPAND);

		//MainSizer->Add(MainSizer2,0,wxLEFT|wxTOP,2);

		FontPickerButton* optf = new FontPickerButton(EditorAdvanced, -1, wxFont(Options.GetInt(GRID_FONT_SIZE), wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, Options.GetString(GRID_FONT)));
		ConOpt(optf, GRID_FONT);
		optf->SetMinSize(ltl->GetMinSize());
		wxBoxSizer* MainSizer9 = new wxBoxSizer(wxHORIZONTAL);
		MainSizer9->Add(new KaiStaticText(EditorAdvanced, -1, _("Subtitle grid font:")), 5, wxRIGHT | /*wxALIGN_CENTRE_VERTICAL | */wxEXPAND, 10);
		MainSizer9->Add(optf, 0, wxEXPAND);

		FontPickerButton* programFont = new FontPickerButton(EditorAdvanced, -1, wxFont(Options.GetInt(PROGRAM_FONT_SIZE), wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, Options.GetString(PROGRAM_FONT)));
		ConOpt(programFont, PROGRAM_FONT);
		programFont->SetMinSize(ltl->GetMinSize());
		wxBoxSizer* MainSizer10 = new wxBoxSizer(wxHORIZONTAL);
		MainSizer10->Add(new KaiStaticText(EditorAdvanced, -1, _("Program font:")), 5, wxRIGHT | /*wxALIGN_CENTRE_VERTICAL | */wxEXPAND, 10);
		MainSizer10->Add(programFont, 0, wxEXPAND);

		KaiCheckBox* allCharWraps = new KaiCheckBox(EditorAdvanced, -1, _("Calculate spaces and punctation characters for wraps"));
		allCharWraps->SetValue(Options.GetBool(CALC_SPACES_AND_PUNCTATION_FOR_WRAPS));
		ConOpt(allCharWraps, CALC_SPACES_AND_PUNCTATION_FOR_WRAPS);
		KaiCheckBox* allCharCPS = new KaiCheckBox(EditorAdvanced, -1, _("Calculate spaces and punctation characters for CPS"));
		allCharCPS->SetValue(Options.GetBool(CALC_SPACES_AND_PUNCTATION_FOR_CPS));
		ConOpt(allCharCPS, CALC_SPACES_AND_PUNCTATION_FOR_CPS);
		

		KaiStaticBoxSizer* alm = new KaiStaticBoxSizer(wxHORIZONTAL, EditorAdvanced, _("Autoload loading method"));
		wxString methods[] = { _("After program start asynchronously"), _("After program start"), _("After open menu asynchronously"), _("After open menu") };
		KaiChoice* cmb = new KaiChoice(EditorAdvanced, ID_KAI_CHOICE, wxDefaultPosition, wxSize(200, -1), 4, methods, wxTE_PROCESS_ENTER);
		cmb->SetSelection(Options.GetInt(AUTOMATION_LOADING_METHOD));
		ConOpt(cmb, AUTOMATION_LOADING_METHOD);
		alm->Add(cmb, 1, wxCENTER | wxEXPAND | wxALL, 2);

		KaiStaticBoxSizer* fontsPath = new KaiStaticBoxSizer(wxHORIZONTAL, EditorAdvanced, _("Folder with external fonts"));
		KaiTextCtrl* path = new KaiTextCtrl(EditorAdvanced, ID_EXTERNAL_FONTS_FOLDER, Options.GetString(EXTERNAL_FONTS_DIRECTORY));
		ConOpt(path, EXTERNAL_FONTS_DIRECTORY);
		MappedButton* choosePath = new MappedButton(EditorAdvanced, ID_EXTERNAL_FONTS_CHOOSE_FOLDER, _("Choose"));
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=, this](wxCommandEvent& event) {
			wxDirDialog ddlg(this, _("Choose choose external font folder"), path->GetValue());
			ddlg.ShowModal();
			path->SetValue(ddlg.GetPath());
			}, ID_EXTERNAL_FONTS_CHOOSE_FOLDER);
		
		fontsPath->Add(path, 4, wxALL | wxEXPAND, 4);
		fontsPath->Add(choosePath, 1, wxALL | wxEXPAND, 4);

		Main1Sizer->Add(allCharWraps, 0, wxALL, 2);
		Main1Sizer->Add(allCharCPS, 0, wxALL, 2);
		Main1Sizer->Add(MainSizer2, 0, wxRIGHT | wxLEFT | wxTOP | wxEXPAND, 5);
		Main1Sizer->Add(MainSizer3, 0, wxRIGHT | wxLEFT | wxTOP | wxEXPAND, 5);
		Main1Sizer->Add(MainSizer4, 0, wxRIGHT | wxLEFT | wxTOP | wxEXPAND, 5);
		Main1Sizer->Add(MainSizer5, 0, wxRIGHT | wxLEFT | wxTOP | wxEXPAND, 5);
		Main1Sizer->Add(MainSizer6, 0, wxRIGHT | wxLEFT | wxTOP | wxEXPAND, 5);
		Main1Sizer->Add(MainSizer7, 0, wxRIGHT | wxLEFT | wxTOP | wxEXPAND, 5);
		Main1Sizer->Add(MainSizer8, 0, wxRIGHT | wxLEFT | wxTOP | wxEXPAND, 5);
		Main1Sizer->Add(MainSizer9, 0, wxRIGHT | wxLEFT | wxTOP | wxEXPAND, 5);
		Main1Sizer->Add(MainSizer10, 0, wxRIGHT | wxLEFT | wxTOP | wxEXPAND, 5);
		Main1Sizer->Add(alm, 0, wxRIGHT | wxTOP | wxEXPAND, 4);
		Main1Sizer->Add(fontsPath, 0, wxRIGHT | wxTOP | wxEXPAND, 4);

		EditorAdvanced->SetSizerAndFit(Main1Sizer);
	}

	//conversion settings
	{
		wxBoxSizer* ConvOptSizer1 = new wxBoxSizer(wxVERTICAL);

		KaiStaticBoxSizer* obr = new KaiStaticBoxSizer(wxHORIZONTAL, ConvOpt, _("Choose catalog"));
		KaiStaticBoxSizer* obr0 = new KaiStaticBoxSizer(wxHORIZONTAL, ConvOpt, _("Choose style"));
		KaiStaticBoxSizer* obr1 = new KaiStaticBoxSizer(wxHORIZONTAL, ConvOpt, _("Choose FPS"));
		KaiStaticBoxSizer* obr2 = new KaiStaticBoxSizer(wxHORIZONTAL, ConvOpt, _("Time for one letter in milliseconds"));
		KaiStaticBoxSizer* obr3 = new KaiStaticBoxSizer(wxHORIZONTAL, ConvOpt, _("Tags to paste at the beginning of every ASS line"));
		KaiStaticBoxSizer* obr4 = new KaiStaticBoxSizer(wxHORIZONTAL, ConvOpt, _("Resolution when converting to ASS"));
		wxArrayString styles;
		wxArrayString FPSes;



		FPSes.Add(L"23.976"); FPSes.Add(L"24"); FPSes.Add(L"25"); FPSes.Add(L"29.97"); FPSes.Add(L"30"); FPSes.Add(L"60");

		for (int i = 0; i < 2; i++) {
			wxString optname = (i == 0) ? Options.GetString(CONVERT_STYLE_CATALOG) : Options.GetString(CONVERT_STYLE);
			if (i != 0) {
				for (int i = 0; i < Options.StoreSize(); i++) {
					styles.Add(Options.GetStyle(i)->Name);
				}
			}
			KaiChoice* cmb = new KaiChoice(ConvOpt,
				(i == 0) ? ID_CONVERSION_STYLE_CATALOG : ID_CONVERSION_STYLE,
				wxDefaultPosition, wxSize(200, -1), (i == 0) ? Options.dirs : styles, wxTE_PROCESS_ENTER);

			int sel = cmb->FindString(optname);

			if (sel >= 0) {
				cmb->SetSelection(sel);
				if (i == 0 && Options.actualStyleDir != optname) {
					Options.LoadStyles(optname);
				}
			}
			else {
				if (i == 0) { sel = cmb->FindString(Options.actualStyleDir); }
				cmb->SetSelection(MAX(0, sel));
				wxString what = (i == 0) ? _("catalog for style") : _("style");
				KaiMessageBox(wxString::Format(_("The selected %s for conversion does not exist\nand will be changed to the default"), what), _("Warning"));
			}

			ConOpt(cmb, (i == 0) ? CONVERT_STYLE_CATALOG : CONVERT_STYLE);
			if (i == 0) {
				obr->Add(cmb, 1, wxCENTER | wxALL, 2);
				ConvOptSizer1->Add(obr, 0, wxRIGHT | wxEXPAND, 5);
				Katlist = cmb;
				Connect(ID_CONVERSION_STYLE_CATALOG, wxEVT_COMMAND_CHOICE_SELECTED, (wxObjectEventFunction)&OptionsDialog::OnChangeCatalog);
			}
			else {
				obr0->Add(cmb, 1, wxCENTER | wxALL, 2);
				ConvOptSizer1->Add(obr0, 0, wxRIGHT | wxEXPAND, 5);
				Stylelist = cmb;
			}
		}
		const wxString& convFPS = Options.GetString(CONVERT_FPS);
		KaiChoice* cmb = new KaiChoice(ConvOpt, -1, convFPS, wxDefaultPosition, wxSize(200, -1), FPSes, wxTE_PROCESS_ENTER);
		int sel = cmb->FindString(convFPS);
		if (sel >= 0) { cmb->SetSelection(sel); }
		else { cmb->SetValue(convFPS); }

		ConOpt(cmb, CONVERT_FPS);
		obr1->Add(cmb, 1, wxCENTER | wxALL, 2);
		ConvOptSizer1->Add(obr1, 0, wxRIGHT | wxEXPAND, 5);


		for (int i = 0; i < 3; i++)
		{
			KaiCheckBox* opt = new KaiCheckBox(ConvOpt, -1, (i == 0) ? _("FPS from video") :
				(i == 1) ? _("New end times") : _("Show window before conversion"));
			CONFIG optname = (i == 0) ? CONVERT_FPS_FROM_VIDEO : (i == 1) ? CONVERT_NEW_END_TIMES : CONVERT_SHOW_SETTINGS;
			opt->SetValue(Options.GetBool(optname));
			ConOpt(opt, optname);
			ConvOptSizer1->Add(opt, 0, wxRIGHT | wxEXPAND, 5);
		}

		NumCtrl* sc = new NumCtrl(ConvOpt, ID_NUMBER_CONTROL, Options.GetString(CONVERT_TIME_PER_CHARACTER), 30, 1000, true,
			wxDefaultPosition, wxSize(250, -1), wxTE_PROCESS_ENTER);
		ConOpt(sc, CONVERT_TIME_PER_CHARACTER);
		obr2->Add(sc, 1, wxALL /*| wxALIGN_CENTER*/ | wxEXPAND, 2);
		ConvOptSizer1->Add(obr2, 0, wxRIGHT | wxEXPAND, 5);

		sc = new NumCtrl(ConvOpt, ID_NUMBER_CONTROL, Options.GetString(CONVERT_RESOLUTION_WIDTH), 1, 3000, true,
			wxDefaultPosition, wxSize(115, -1), wxTE_PROCESS_ENTER);
		ConOpt(sc, CONVERT_RESOLUTION_WIDTH);
		obr4->Add(sc, 1, wxALL /*| wxALIGN_CENTER*/ | wxEXPAND, 2);

		KaiStaticText* txt = new KaiStaticText(ConvOpt, -1, L" X ");
		obr4->Add(txt, 0, wxTOP, 5);

		sc = new NumCtrl(ConvOpt, ID_NUMBER_CONTROL, Options.GetString(CONVERT_RESOLUTION_HEIGHT), 1, 3000, true, wxDefaultPosition, wxSize(115, -1), wxTE_PROCESS_ENTER);
		ConOpt(sc, CONVERT_RESOLUTION_HEIGHT);
		obr4->Add(sc, 1, wxALL /*| wxALIGN_CENTER*/ | wxEXPAND, 2);
		ConvOptSizer1->Add(obr4, 0, wxRIGHT | wxEXPAND, 5);

		KaiTextCtrl* tc = new KaiTextCtrl(ConvOpt, -1, Options.GetString(CONVERT_ASS_TAGS_TO_INSERT_IN_LINE), wxDefaultPosition, wxSize(250, -1), wxTE_PROCESS_ENTER);
		ConOpt(tc, CONVERT_ASS_TAGS_TO_INSERT_IN_LINE);
		obr3->Add(tc, 1, wxALL /*| wxALIGN_CENTER*/ | wxEXPAND, 2);
		ConvOptSizer1->Add(obr3, 0, wxRIGHT | wxEXPAND, 5);

		ConvOpt->SetSizerAndFit(ConvOptSizer1);
	}
	//video
	{
		wxString voptspl[] = { _("Open video from context menu on full screen"), _("Left mouse button pauses video"),
			_("Open video with time of active line"), _("Preferred audio (separated by semicolons)"),
			_("FFMS2 video seeking method (requires reloading)"), _("Subtitle display filter"),
			_("Start video zoom in percent.")};
		CONFIG vopts[] = { VIDEO_FULL_SCREEN_ON_START, VIDEO_PAUSE_ON_CLICK, OPEN_VIDEO_AT_ACTIVE_LINE,
			ACCEPTED_AUDIO_STREAM, FFMS2_VIDEO_SEEKING, VSFILTER_INSTANCE, VIDEO_ZOOM_PERCENT };
		wxBoxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
		for (int i = 0; i < 3; i++)
		{
			KaiCheckBox *opt = new KaiCheckBox(video, -1, voptspl[i]);
			opt->SetValue(Options.GetBool(vopts[i]));
			ConOpt(opt, vopts[i]);
			MainSizer->Add(opt, 0, wxALL, 2);
		}
		KaiStaticBoxSizer *prefaudio = new KaiStaticBoxSizer(wxHORIZONTAL, video, voptspl[3]);
		KaiTextCtrl *tc = new KaiTextCtrl(video, -1, Options.GetString(vopts[3]), 
			wxDefaultPosition, wxSize(250, -1), wxTE_PROCESS_ENTER);
		ConOpt(tc, vopts[3]);
		prefaudio->Add(tc, 1, wxALL | wxEXPAND, 2);
		MainSizer->Add(prefaudio, 0, wxRIGHT | wxEXPAND, 5);
		KaiStaticBoxSizer *seekingsizer = new KaiStaticBoxSizer(wxHORIZONTAL, video, voptspl[4]);

		wxString seekingOpts[] = { _("Linear"), _("Normal"), 
			_("Unsafe (always fast)"), _("Aggressive (fast in rewind)") };
		KaiChoice *sopts = new KaiChoice(video, ID_KAI_CHOICE, 
			wxDefaultPosition, wxSize(200, -1), 4, seekingOpts, wxTE_PROCESS_ENTER);
		int selection = Options.GetInt(vopts[4]);
		if (selection < 0 || selection > 3) {
			selection = 2;
			Options.SetInt(vopts[4], selection);
			Options.SaveOptions(true, false);
		}
		sopts->SetSelection(selection);
		seekingsizer->Add(sopts, 1, wxALL | wxEXPAND, 2);
		MainSizer->Add(seekingsizer, 0, wxRIGHT | wxEXPAND, 5);
		ConOpt(sopts, vopts[4]);

		KaiStaticBoxSizer *filtersizer = new KaiStaticBoxSizer(wxHORIZONTAL, video, voptspl[5]);
		wxArrayString vsfilters;
		SubtitlesProviderManager::GetProviders(&vsfilters);
		KaiChoice *vsfiltersList = new KaiChoice(video, ID_VSFILTER_PROVIDER, 
			wxDefaultPosition, wxSize(200, -1), vsfilters, wxTE_PROCESS_ENTER);
		wxString name = Options.GetString(vopts[5]);
		int result = vsfilters.Index(name);
		if (result < 0)
			result = 0;
		vsfiltersList->SetSelection(result);
		filtersizer->Add(vsfiltersList, 1, wxALL | wxEXPAND, 2);
		MainSizer->Add(filtersizer, 0, wxRIGHT | wxEXPAND, 5);

		KaiStaticBoxSizer* zoomsizer = new KaiStaticBoxSizer(wxHORIZONTAL, video, voptspl[6]);
		int percent = Options.GetInt(vopts[6]);
		if (percent < 100 || percent > 1100)
			percent = 200;

		NumCtrl* zoomPercent = new NumCtrl(video, ID_VIDEO_ZOOM_PERCENT, (double)percent, 100., 1100., true);
		ConOpt(zoomPercent, vopts[6]);
		zoomsizer->Add(zoomPercent, 1, wxALL | wxEXPAND, 2);
		MainSizer->Add(zoomsizer, 0, wxRIGHT | wxEXPAND, 5);
		ConOpt(vsfiltersList, vopts[5]);
		video->SetSizerAndFit(MainSizer);
	}
	//Hotkeys
	{
		wxBoxSizer *HkeysSizer = new wxBoxSizer(wxVERTICAL);
		KaiStaticBoxSizer *filterMode = new KaiStaticBoxSizer(wxHORIZONTAL, Hotkeyss, _("Choose filtering"));
		wxString filteringModes[] = { _("All"), _("Set shortcuts"), _("Global shortcuts"),
			_("Subtitle shortcuts"), _("Editor shortcuts"), _("Video shortcuts"), _("Audio Shortcuts"), };
		KaiChoice *filterList = new KaiChoice(Hotkeyss, 14568, wxDefaultPosition, wxDefaultSize, 7, filteringModes);
		filterList->SetSelection(0);
		filterList->SetToolTip(_("Filtering mode:"));

		filterMode->Add(filterList, 1, wxALL | wxEXPAND, 2);
		HkeysSizer->Add(filterMode, 0, wxEXPAND);
		wxString mesureText = _("Global") + L" " + _("Credits");
		wxString mesureText2 = L"Alt-Shift-Delete";
		int fw, fww, fh;
		GetTextExtent(mesureText, &fw, &fh);
		GetTextExtent(mesureText2, &fww, &fh);

		Shortcuts = new KaiListCtrl(Hotkeyss, 26667, wxDefaultPosition);
		Shortcuts->InsertColumn(0, _("Function"), TYPE_TEXT, (fw < 275)? 275 : fw);
		Shortcuts->InsertColumn(1, _("Hotkey"), TYPE_TEXT, (fww < 80)? 80 : fww);
		Connect(26667, LIST_ITEM_DOUBLECLICKED, (wxObjectEventFunction)&OptionsDialog::OnMapHkey);
		//Connect(26667,LIST_ITEM_RIGHT_CLICK,(wxObjectEventFunction)&OptionsDialog::OnResetHkey);

		if (!Hkeys.AudioKeys && !Hkeys.LoadHkeys(true)){ 
			KaiMessageBox(_("Cannot load audio hotkeys"), _("Error"));
		}

		AddHotkeysOnList();

		HkeysSizer->Add(Shortcuts, 1, wxALL | wxEXPAND, 4);
		wxBoxSizer *buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
		MappedButton *setHotkey = new MappedButton(Hotkeyss, 23232, _("Map hotkey"));
		Connect(23232, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&OptionsDialog::OnMapHkey);
		MappedButton *resetHotkey = new MappedButton(Hotkeyss, 23231, _("Restore default hotkey"));
		Connect(23231, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&OptionsDialog::OnResetHkey);
		MappedButton *deleteHotkey = new MappedButton(Hotkeyss, 23230, _("Delete hotkey"));
		Connect(23230, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&OptionsDialog::OnDeleteHkey);
		buttonsSizer->Add(setHotkey, 0, wxALL, 2);
		buttonsSizer->Add(resetHotkey, 0, wxALL, 2);
		buttonsSizer->Add(deleteHotkey, 0, wxALL, 2);
		HkeysSizer->Add(buttonsSizer, 0, wxALL | wxALIGN_CENTER, 2);
		// filter list it need created list
		Bind(wxEVT_COMMAND_CHOICE_SELECTED, [=, this](wxCommandEvent &evt){
			//we check to second collumn that contain shortcut
			Shortcuts->FilterList(1, filterList->GetSelection());
		}, 14568);

		Hotkeyss->SetSizerAndFit(HkeysSizer);
		ConOpt(Shortcuts, (CONFIG)ID_HOTKEYS_CONFIG);
	}

	//Audio main

	{
		wxBoxSizer *audio = new wxBoxSizer(wxVERTICAL);
		const int numOfElements = 13;
		wxString names[numOfElements] = { _("Show time next to cursor"), _("Show seconds markers"), _("Show background selection"),
			_("Show video position"), _("Show keyframes"), _("Follow audio during playback"),
			_("Activate the audio when hover"), _("Snap to keyframe"), _("Snap to other lines"),
			_("Do not play audio after changing the line"), _("Merge all the \"n\" with the previous syllable"),
			_("Move syllable line after click"),_("Load audio into RAM") };

		CONFIG opts[numOfElements] = { AUDIO_DRAW_TIME_CURSOR, AUDIO_DRAW_SECONDARY_LINES, AUDIO_DRAW_SELECTION_BACKGROUND, AUDIO_DRAW_VIDEO_POSITION,
			AUDIO_DRAW_KEYFRAMES, AUDIO_LOCK_SCROLL_ON_CURSOR, AUDIO_AUTO_FOCUS, AUDIO_SNAP_TO_KEYFRAMES, AUDIO_SNAP_TO_OTHER_LINES,
			AUDIO_DONT_PLAY_WHEN_LINE_CHANGES, AUDIO_MERGE_EVERY_N_WITH_SYLLABLE, AUDIO_KARAOKE_MOVE_ON_CLICK, AUDIO_RAM_CACHE };

		for (int i = 0; i < numOfElements; i++)
		{
			KaiCheckBox *opt = new KaiCheckBox(AudioMain, -1, names[i]);
			opt->SetValue(Options.GetBool(opts[i]));
			ConOpt(opt, opts[i]);
			audio->Add(opt, 0, wxALL, 2);
		}
		AudioMain->SetSizerAndFit(audio);
	}
	//audio second tab
	{
		wxBoxSizer *audio2 = new wxBoxSizer(wxVERTICAL);

		CONFIG opts1[7] = { AUDIO_DELAY, AUDIO_MARK_PLAY_TIME, AUDIO_INACTIVE_LINES_DISPLAY_MODE, 
			AUDIO_LINE_BOUNDARIES_THICKNESS, AUDIO_CACHE_FILES_LIMIT, AUDIO_LEAD_IN_VALUE, AUDIO_LEAD_OUT_VALUE };
		NumCtrl *Delay = new NumCtrl(AudioSecond, ID_NUMBER_CONTROL, Options.GetString(opts1[0]), -50000000, 50000000, true, wxDefaultPosition, wxSize(300, -1), 0);
		NumCtrl *markPlayTime = new NumCtrl(AudioSecond, ID_NUMBER_CONTROL, Options.GetString(opts1[1]), 400, 5000, true, wxDefaultPosition, wxSize(300, -1), 0);
		NumCtrl *lineThickness = new NumCtrl(AudioSecond, ID_NUMBER_CONTROL, Options.GetString(opts1[3]), 1, 5, true, wxDefaultPosition, wxSize(300, -1), 0);
		NumCtrl *audioCacheFilesLimit = new NumCtrl(AudioSecond, ID_NUMBER_CONTROL, Options.GetString(opts1[4]), 0, 10000, true, wxDefaultPosition, wxSize(300, -1), 0);
		NumCtrl *leadInTime = new NumCtrl(AudioSecond, ID_NUMBER_CONTROL, Options.GetString(opts1[5]), 0, 10000, true, wxDefaultPosition, wxSize(120, -1), 0);
		NumCtrl *leadOutTime = new NumCtrl(AudioSecond, ID_NUMBER_CONTROL, Options.GetString(opts1[6]), 0, 10000, true, wxDefaultPosition, wxSize(120, -1), 0);
		audioCacheFilesLimit->SetToolTip(_("Range from 0 to 10000, where 0 turns off\nremoving audio cache files."));
		wxString inact[3] = { _("None"), _("Before and after the active"), _("All visible") };
		KaiChoice *displayNonActiveLines = new KaiChoice(AudioSecond, ID_KAI_CHOICE, wxDefaultPosition, wxSize(300, -1), 3, inact);
		displayNonActiveLines->SetSelection(Options.GetInt(opts1[2]));
		ConOpt(Delay, opts1[0]);
		ConOpt(markPlayTime, opts1[1]);
		ConOpt(lineThickness, opts1[3]);
		ConOpt(displayNonActiveLines, opts1[2]);
		ConOpt(audioCacheFilesLimit, opts1[4]);
		ConOpt(leadInTime, opts1[5]);
		ConOpt(leadOutTime, opts1[6]);
		KaiStaticBoxSizer *DelaySizer = new KaiStaticBoxSizer(wxVERTICAL, AudioSecond, _("Audio delay in milliseconds"));
		KaiStaticBoxSizer *markPlayTimeSizer = new KaiStaticBoxSizer(wxVERTICAL, AudioSecond, _("Audio to play before and after the marker in milliseconds"));
		wxString elems[] = { _("Lead-in"), _("Lead-out") };
		KaiStaticBoxSizer *leadInAndOut = new KaiStaticBoxSizer(wxHORIZONTAL, AudioSecond, 2, elems);
		KaiStaticBoxSizer *lineThicknessSizer = new KaiStaticBoxSizer(wxVERTICAL, AudioSecond, _("Line boundaries thickness"));
		KaiStaticBoxSizer *audioCacheFilesLimitSizer = new KaiStaticBoxSizer(wxVERTICAL, AudioSecond, _("Audio cache files limit"));
		KaiStaticBoxSizer *displayNonActiveLinesSizer = new KaiStaticBoxSizer(wxVERTICAL, AudioSecond, _("The way to display inactive lines"));
		DelaySizer->Add(Delay, 1, wxALL | wxEXPAND, 2);
		markPlayTimeSizer->Add(markPlayTime, 1, wxALL | wxEXPAND, 2);
		leadInAndOut->Add(leadInTime, 1, wxALL | wxEXPAND, 2);
		leadInAndOut->Add(leadOutTime, 1, wxALL | wxEXPAND, 2);
		lineThicknessSizer->Add(lineThickness, 1, wxALL | wxEXPAND, 2);
		audioCacheFilesLimitSizer->Add(audioCacheFilesLimit, 1, wxALL | wxEXPAND, 2);
		displayNonActiveLinesSizer->Add(displayNonActiveLines, 1, wxALL | wxEXPAND, 2);
		audio2->Add(DelaySizer, 0, wxRIGHT | wxEXPAND, 5);
		audio2->Add(markPlayTimeSizer, 0, wxRIGHT | wxEXPAND, 5);
		audio2->Add(leadInAndOut, 0, wxRIGHT | wxEXPAND, 5);
		audio2->Add(lineThicknessSizer, 0, wxRIGHT | wxEXPAND, 5);
		audio2->Add(audioCacheFilesLimitSizer, 0, wxRIGHT | wxEXPAND, 5);
		audio2->Add(displayNonActiveLinesSizer, 0, wxRIGHT | wxEXPAND, 5);


		AudioSecond->SetSizerAndFit(audio2);
	}

	//Themes
	{
		const int numColors = 139;
		wxString labels[numColors] = {
			//window
			_("Window background"), _("Inactive window background"), _("Window text"), _("Inactive window text"),
			_("Window border"), _("Inactive window border"), _("Window border background"),
			_("Inactive window border background"), _("Window header text"), _("Inactive window header text"),
			_("Window invaded element header"), _("Window pressed element header"),
			_("Window closing upon hover"), _("Window pressed closing"), _("Window elements warning"),
			//subtitles
			_("Subtitle text"), _("Subtitle background"), _("Subtitle dialogue background"), _("Subtitle comment background"),
			_("Subtitle selection (transparency)"), _("Subtitle lines visible on video"),
			_("Subtitle colliding lines"), _("Subtitle line border"), _("Subtitle active line border"),
			_("Subtitle header background"), _("Subtitle header text"),
			_("Subtitle line label"), _("Subtitle modified-line label"), _("Subtitle saved-line label"),
			_("Subtitle unconfirmed-line label"), _("Subtitle spelling-error background"), _("Subtitle comparison border"),
			_("Subtitle comparison mismatch background"), _("Subtitle comparison match background"),
			_("Subtitle comparison comment mismatch background"), _("Subtitle comparison comment match background"),
			//editor
			_("Editor text"), _("Editor tag names"), _("Editor tag values"),
			_("Editor curly braces"), _("Editor tags operators"), _("Editor line splitting and ASS drawings"), _("Editor template variables"),
			_("Editor template code marks"), _("Editor template functions"), _("Editor template keywords"),
			_("Editor template strings"), _("Editor found words selection"), _("Editor brackets background"),
			_("Editor background"), _("Editor selection"), _("Editor inactive window selection"),
			_("Editor border"), _("Editor border on focus"), _("Editor spelling error background"),
			//audio
			_("Audio background"), _("Audio start marker"), _("Audio end marker"), _("Audio time shift marker"),
			_("Audio inactive line marker"), _("Audio cursor"), _("Audio seconds boundaries"), _("Audio keyframes"),
			_("Audio syllable marker"), _("Audio syllable text"), _("Audio selection"),
			_("Audio the selection after modification"), _("Audio inactive line background"), _("Audio waveform"),
			_("Audio Inactive waveform"), _("Audio modified waveform"), _("Audio selected waveform"),
			_("Audio spectrum background"), _("Audio spectrum echo"), _("Audio spectrum"),
			//controls
			_("Text field background"), _("Text field border"),
			_("Text field border on focus"), _("Text field selection"),
			_("Text field inactive window selection"),
			_("Button and list background"), _("Button and list background upon hover"),
			_("Button and list pushed background"), _("Button and list background on focus"),
			_("Button and list border"), _("Button and list border upon hover"),
			_("Button and list border of pushed"), _("Button and list border on focus"),
			_("Button and list inactive border"), _("Togglebutton toggled background"),
			_("Togglebutton toggled border"), _("Scroll bar background"), _("Scroll bar slider"),
			_("Scroll bar slider upon hover"), _("Scroll bar pushed slider"),
			_("Staticbox border"), _("Static list border"), _("Static list background"),
			_("Static list selection"), _("Static list header background"), _("Static list header text"),
			_("Status bar border"),
			//menu bar
			_("Menu bar background 1"), _("Menu bar background 2"), _("Menu bar selection border"),
			_("Menu bar selection hover bacground"), _("Menu bar selection clicked bacground"),
			_("Menu background"), _("Menu selection border"), _("Menu selection background"),
			//tab bar
			_("Tab bar background 1"), _("Tab bar background 2"), _("Tab active border"),
			_("Tab inactive border"), _("Tab active background"), _("Tab inactive background"),
			_("Tab inactive background upon hover"), _("Tab background of second visible tab"),
			_("Tab active text"), _("Tab inactive text"), _("Tab close upon hover"),
			_("Tab bar arrow"), _("Tab bar arrow background"),
			_("Tab bar arrow background upon hover"),
			//slider
			_("Slider path background"), _("Slider path border"), _("Slider border"),
			_("Slider border upon hover"), _("Slider border of pushed"), _("Slider background"),
			_("Slider background upon hover"), _("Slider background of pushed"),
			//miscellanous
			_("Resize line dots"), _("Finding result file name foreground"), _("Finding result file name background"),
			_("Finding result found phrase foreground"), _("Finding result found phrase background"),
			//styles preview
			_("First color of style preview background"), _("Second color of style preview background")
		};


		wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
		wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
		wxArrayString choices;
		wxArrayString files;
		wxString pathwn = Options.pathfull + L"/Themes/";
		const wxString & programTheme = Options.GetString(PROGRAM_THEME);
		wxDir kat(pathwn);
		if (kat.IsOpened()){
			kat.GetAllFiles(pathwn, &files, L"*.txt", wxDIR_FILES);
		}
		for (size_t i = 0; i < files.size(); i++){
			choices.Add(KaiPathName(files[i]).BeforeLast(L'.'));
		}
		if (choices.Index(L"DarkSentro", false) == -1){
			choices.Insert(L"DarkSentro", 0);
		}
		if (choices.Index(L"LightSentro", false) == -1){
			choices.Insert(L"LightSentro", 1);
		}
		KaiChoice *themeList = new KaiChoice(Themes, 14567, wxDefaultPosition, wxDefaultSize, choices);
		themeList->SetSelection(themeList->FindString(programTheme));
		themeList->SetToolTip(_("Theme name:"));
		KaiTextCtrl *newTheme = new KaiTextCtrl(Themes, -1, emptyString);
		newTheme->SetToolTip(_("Name of the copied theme.\nDefault themes, DarkSentro and LightSentro,\ncannot be edited and must be copied."));
		MappedButton *copyTheme = new MappedButton(Themes, 14566, _("Copy"));



		sizer->Add(themeList, 0, wxALL | wxEXPAND, 2);
		sizer1->Add(newTheme, 1, wxRIGHT | wxTOP | wxBOTTOM | wxEXPAND, 2);
		sizer1->Add(copyTheme, 0, wxLEFT | wxTOP | wxBOTTOM, 2);
		sizer->Add(sizer1, 0, wxALL | wxEXPAND, 2);

		KaiStaticText *warning = new KaiStaticText(Themes, -1, _("Warning! Transparency works only on the audio spectrum,\ntext field, and subtitle grid."));
		sizer->Add(warning, 0, wxALL | wxEXPAND, 2);

		wxString mesureText = _("Text field inactive window selection");
		wxString mesureText2 = L"######FFFFFFFF";
		int fw, fww, fh;
		GetTextExtent(mesureText, &fw, &fh);
		GetTextExtent(mesureText2, &fww, &fh);

		KaiListCtrl *List = new KaiListCtrl(Themes, -1, wxDefaultPosition, wxSize(300, -1));
		List->InsertColumn(0, _("Name"), TYPE_TEXT, fw < 240 ? 240 : fw);
		List->InsertColumn(1, _("Color"), TYPE_COLOR, fww < 120 ? 120 : fww);
		for (int i = 0; i < numColors; i++)
		{
			int row = List->AppendItem(new ItemText(labels[i]));
			AssColor col = Options.GetColor((COLOR)(i + 1));
			List->SetItem(row, 1, new ItemColor(col, i + 1));
		}
		sizer->Add(List, 1, wxALL | wxEXPAND, 2);
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
			wxString themeName = newTheme->GetValue();
			if (themeName.IsEmpty() || choices.Index(themeName, false) != -1){ wxBell(); return; }
			wxString originalName = themeList->GetString(themeList->GetSelection());
			wxString dir = Options.pathfull + L"/Themes/";
			wxString copyPath = dir + themeName + L".txt";
			if (originalName == L"DarkSentro" || originalName == L"LightSentro"){
				Options.SaveColors(copyPath);
				List->Enable(true);
				List->Refresh(false);
			}
			else{

				if (!wxDirExists(dir)){
					wxBell(); return;
				}
				wxString originalPath = dir + originalName + L".txt";
				wxCopyFile(originalPath, copyPath, false);
			}
			Options.SetString(PROGRAM_THEME, themeName);
			if (!List->IsEnabled()){ List->Enable(false); }
			newTheme->SetValue(emptyString);
			int size = themeList->Append(themeName);
			themeList->SetSelection(size);
		}, 14566);
		Bind(wxEVT_COMMAND_CHOICE_SELECTED, [=, this](wxCommandEvent &evt){
			wxString themeName = themeList->GetString(themeList->GetSelection());
			if (themeName.IsEmpty()){ return; }
			Options.LoadColors(themeName);
			for (int i = 0; i < numColors; i++)
			{
				ItemColor *item = (ItemColor*)List->GetItem(i, 1);
				item->col = Options.GetColor((COLOR)item->colOptNum);
			}
			ChangeColors();
			List->Enable(themeName != L"DarkSentro" && themeName != L"LightSentro");
		}, 14567);
		if (programTheme == L"DarkSentro" || programTheme == L"LightSentro"){ List->Enable(false); }
		Themes->SetSizerAndFit(sizer);
		List->StartEdition();
		List->SetSelection(0);
		ConOpt(List, (CONFIG)ID_COLOR_CONFIG);
		Bind(LIST_ITEM_DOUBLECLICKED, [=](wxCommandEvent &evt){
			int selection = List->GetSelection();
			Item *item = List->GetItem(selection, 1);
			if (!item)
				return;

			ItemColor *itemc = (ItemColor*)item;

			DialogColorPicker *dcp = DialogColorPicker::Get(List, itemc->col);
			wxPoint mst = wxGetMousePosition();
			wxSize siz = dcp->GetSize();
			wxRect rc = wxGetClientDisplayRect();
			mst.x -= (siz.x / 2);
			mst.x = MID(rc.x, mst.x, rc.width - siz.x);
			mst.y += 15;
			mst.y = MID(rc.y, mst.y, rc.height - siz.y);
			dcp->Move(mst);
			if (dcp->ShowModal() == wxID_OK) {
				ItemColor *copy = (ItemColor*)List->CopyRow(selection, 1);
				if (copy){
					copy->col = dcp->GetColor();
					copy->modified = true;
					List->SetModified(true);
					List->PushHistory();
					List->Refresh(false);
				}
			}
		}, List->GetId());
	}
	//associations
	{
	wxString extensions[] = { L".ass", L".ssa", L".srt", L".sub", L".txt", L".mkv", L".mp4", L".avi", L".ogm",
			L".wmv", L".asf", L".rmvb", L".rm", L".3gp", L".mpg", L".mpeg", L".ts", L".m2ts" };
		wxString extensionsDesc[] = { _("ASS subtitles"), _("SSA subtitles"), _("SRT subtitles"), _("SUB subtitles"), 
			_("TXT subtitles"), _("Video MKV"), _("Video MP4"), _("Video AVI"), _("Video OGM"),
			_("Video WMV"), _("Video ASF"), _("Video RMVB"), _("Video RM"), _("Video 3GP"), 
			_("Video MPG"), _("Video MPEG"), _("Video TS"), _("Video M2TS") };
		int numExtensions = 18;

		Registry::CheckFileAssociation(extensions, numExtensions, registeredExts);

		wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
		KaiStaticText *warning = new KaiStaticText(Assocs, -1, 
			_("WARNING! Behavior differs on each Windows version.\nFor example, on Windows 7 WMP blocked\nthe ability to change video associations."));
		sizer->Add(warning, 0, wxEXPAND | wxALL, 4);
		KaiListCtrl *CheckListBox = new KaiListCtrl(Assocs, -1, numExtensions, extensionsDesc);
		for (int i = 0; i < numExtensions; i++){
			CheckListBox->GetItem(i, 0)->modified = registeredExts[i];
		}
		//type 1 = select all, 2 = select subs, 3 = select video, 4 = deselect all, 
		auto changeSelections = [=](wxCommandEvent &evt){
			int type = evt.GetId() - 17776;
			for (int i = 0; i < numExtensions; i++){
				CheckListBox->GetItem(i, 0)->modified = 
					(type == 1 || (type == 2 && i < 4) || (type == 3 && i>4)) ? true : false;
			}
			CheckListBox->SetModified(true);
			CheckListBox->Refresh(false);
		};
		wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *buttonSizer1 = new wxBoxSizer(wxHORIZONTAL);
		wxString buttonTexts[] = { _("Select all"), _("Select subtitles"), _("Select video"), _("Deselect all") };
		for (int i = 0; i < 4; i++){
			MappedButton *btn = new MappedButton(Assocs, 17777 + i, buttonTexts[i]);
			if (i < 2)
				buttonSizer->Add(btn, 1, wxALL, 2);
			else
				buttonSizer1->Add(btn, 1, wxALL, 2);
		}
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, changeSelections, 17777, 17780);
		sizer->Add(CheckListBox, 1, wxEXPAND | wxALL, 4);
		sizer->Add(buttonSizer, 0, wxEXPAND /*| wxALIGN_CENTER_VERTICAL*/ | wxALL,2);
		sizer->Add(buttonSizer1, 0, wxEXPAND /*| wxALIGN_CENTER_VERTICAL*/ | wxALL, 2);
		ConOpt(CheckListBox, (CONFIG)3000);
		Assocs->SetSizerAndFit(sizer);
	}

	//Adding pages
	OptionsTree->AddPage(GLOBAL_EDITOR, _("Editor"));
	//After adding page before convert change it in function Convert in SubsGridBase.cpp!!!
	OptionsTree->AddSubPage(ConvOpt, _("Conversion"));
	OptionsTree->AddSubPage(EditorAdvanced, _("Advanced"));
	OptionsTree->AddPage(video, _("Video"));
	OptionsTree->AddPage(AudioMain, _("Audio"));
	OptionsTree->AddSubPage(AudioSecond, _("Advanced"));
	OptionsTree->AddPage(Themes, _("Themes"));
	OptionsTree->AddPage(Hotkeyss, _("Hotkeys"));
#ifdef _WIN32
	OptionsTree->AddPage(Assocs, _("Associations"));
#endif
	OptionsTree->AddPage(SubsProps, _("Subtitle properties"));
	OptionsTree->Fit();

	//adding buttons
	wxBoxSizer *ButtonsSizer = new wxBoxSizer(wxHORIZONTAL);

	okok = new MappedButton(this, wxID_OK, L"OK");
	MappedButton *oknow = new MappedButton(this, ID_BCOMMIT, _("Apply"));
	MappedButton *cancel = new MappedButton(this, wxID_CANCEL, _("Cancel"));
	MappedButton *resetDefaults = new MappedButton(this, ID_RESET_DEFAULTS, _("Set default"));

	ButtonsSizer->Add(okok, 1, wxRIGHT, 2);
	ButtonsSizer->Add(oknow, 1, wxRIGHT, 2);
	ButtonsSizer->Add(cancel, 1, wxRIGHT, 2);
	ButtonsSizer->Add(resetDefaults, 1, wxRIGHT, 2);

	DialogSizer *TreeSizer = new DialogSizer(wxVERTICAL);
	TreeSizer->Add(OptionsTree, 1, wxALL | wxEXPAND, 2);
	TreeSizer->Add(ButtonsSizer, 0, wxBOTTOM | wxALIGN_CENTER, 4);
	SetSizerAndFit(TreeSizer);

	CenterOnParent();

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &OptionsDialog::OnSaveClick, this, wxID_OK);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &OptionsDialog::OnSaveClick, this, ID_BCOMMIT);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &OptionsDialog::OnResetDefault, this, ID_RESET_DEFAULTS);
}

OptionsDialog::~OptionsDialog()
{
	/*if(GetReturnCode ()==wxID_OK){
		SetOptions();
		if(hkeymodif==1){Hkeys.SaveHkeys();Kai->SetAccels();}
		else if(hkeymodif==2){
		Hkeys.SaveHkeys(true);
		if(Kai->GetTab()->edit->ABox){Kai->GetTab()->edit->ABox->SetAccels();}
		}
		}*/
	delete[] windowNames;
	handles.clear();
}

void OptionsDialog::ConOpt(wxWindow *ctrl, CONFIG option)
{
	OptionsBind Obind;
	Obind.ctrl = ctrl;
	Obind.option = option;
	handles.push_back(Obind);
}

void OptionsDialog::OnSaveClick(wxCommandEvent& event)
{
	SetOptions(false);
	/*if(hkeymodif==1){Hkeys.SaveHkeys();Kai->SetAccels();}
	else if(hkeymodif==2){
	Hkeys.SaveHkeys(true);
	if(Kai->GetTab()->edit->ABox){Kai->GetTab()->edit->ABox->SetAccels();}
	}*/
	if (event.GetId() == wxID_OK){ EndModal(wxID_OK); }
}

void OptionsDialog::SetOptions(bool saveall)
{
	bool fontmod = false;
	bool colmod = false;
	bool audio = false;
	for (size_t i = 0; i < handles.size(); i++)
	{
		OptionsBind OB = handles[i];

		if (OB.ctrl->IsKindOf(CLASSINFO(KaiCheckBox))){
			KaiCheckBox *cb = (KaiCheckBox*)OB.ctrl;
			bool value = cb->GetValue();
			if (Options.GetBool(OB.option) != value){
				Options.SetBool(OB.option, value);
				if (OB.option <= AUDIO_WHEEL_DEFAULT_TO_ZOOM) { audio = true; }
				if (OB.option == SPELLCHECKER_ON) {
					Notebook::GetTab()->edit->ClearErrs(true, value);
				}
			}
		}
		else if (OB.ctrl->IsKindOf(CLASSINFO(FontPickerButton))) {
			FontPickerButton* fpc = (FontPickerButton*)OB.ctrl;
			wxFont font = fpc->GetSelectedFont();
			wxString fontname = font.GetFaceName();
			int fontsize = font.GetPointSize();
			if (Options.GetString(OB.option) != fontname) {
				Options.SetString(OB.option, fontname);
				fontmod = true;
			}
			CONFIG fontSizeOption = (OB.option == GRID_FONT) ? GRID_FONT_SIZE : PROGRAM_FONT_SIZE;
			if (Options.GetInt(fontSizeOption) != fontsize) {
				Options.SetInt(fontSizeOption, fontsize);
				fontmod = true;
			}
			if (OB.option == PROGRAM_FONT && fontmod) {
				Options.FontsClear();
				KainoteFrame* Kai =
					/*wxDynamicCast<*/(KainoteFrame*)Notebook::GetTabs()->GetParent();//>
				Kai->SetAccels();
				Kai->DestroyDialogs();
				Kai->SetFont(*Options.GetFont());
				Kai->Layout();
			}
		}
		else if (OB.ctrl->IsKindOf(CLASSINFO(KaiChoice))) {
			KaiChoice* cbx = (KaiChoice*)OB.ctrl;
			if (cbx->GetWindowStyle() & KAI_COMBO_BOX) {
				wxString color = cbx->GetValue();
				if (Options.GetString(OB.option) != color) {
					Options.SetString(OB.option, color);
				}
			}
			else if (cbx->GetId() != ID_KAI_CHOICE) {
				//dictionary language
				if (cbx->GetId() == ID_DICTIONARY_LANGUAGE) {
					int sel = cbx->GetSelection();
					if (sel >= 0 && sel < dictionaryLanguagesSymbols.GetCount()) {
						wxString language = dictionaryLanguagesSymbols[sel];
						if (Options.GetString(OB.option) != language) {
							Options.SetString(OB.option, language);
							SpellChecker::Destroy();
							Notebook::GetTab()->edit->ClearErrs();
						}
					}
				}//program language
				else if (cbx->GetId() == ID_PROGRAM_LANGUAGE) {
					int sel = cbx->GetSelection();
					if (sel >= 0 && sel < programLanguages.size()) {
						wxString language = programLanguages[sel];
						if (Options.GetString(OB.option) != language)
							Options.SetString(OB.option, language);
					}
				}
				else {
					wxString option = cbx->GetString(cbx->GetSelection());
					if (Options.GetString(OB.option) != option) {
						Options.SetString(OB.option, option);
						//vsfilter change
						if (cbx->GetId() == ID_VSFILTER_PROVIDER) {
							SubtitlesProviderManager::DestroyProviders();
							Notebook::RefreshVideo();
						}
						else if (cbx->GetId() == ID_CONVERSION_STYLE_CATALOG || cbx->GetId() == ID_CONVERSION_STYLE) {
							Options.DeleteConversionStyle();
							Notebook::RefreshVideo();
						}
					}
				}
			}
			else {
				if (Options.GetInt(OB.option) != cbx->GetSelection()) {
					Options.SetInt(OB.option, cbx->GetSelection());
				}
			}
			if (OB.option <= AUDIO_WHEEL_DEFAULT_TO_ZOOM) { audio = true; }
		}
		else if (OB.ctrl->IsKindOf(CLASSINFO(KaiTextCtrl))) {

			if (OB.ctrl->GetId() != ID_NUMBER_CONTROL) {
				KaiTextCtrl* sc = (KaiTextCtrl*)OB.ctrl;
				wxString str = sc->GetValue();
				if (Options.GetString(OB.option) != str) {
					//we need to call function before set a new path
					//to remove loaded fonts from last folder
					if (OB.option == EXTERNAL_FONTS_DIRECTORY) {
						wxString separator(wxFileName::GetPathSeparator());
						str = KaiNormalizePath(str);
						if (!str.empty() && !str.EndsWith(separator))
							str << separator;

						FontEnum.ReloadExternalFontsToProcess(str, this);
					}
					Options.SetString(OB.option, str);
					if (OB.option == GRID_TAGS_SWAP_CHARACTER) {
						Notebook* tabs = Notebook::GetTabs();
						for (size_t i = 0; i < tabs->Size(); i++) {
							TabPanel* page = tabs->Page(i);
							page->grid->SpellErrors.clear();
						}
					}
				}
				if (sc->GetId() == ID_TAGS_SWAP_CHARACTER){
					colmod = true;
				}
			}
			else{
				NumCtrl *sc = (NumCtrl*)OB.ctrl;
				int num = sc->GetInt();
				if (Options.GetInt(OB.option) != num){
					Options.SetInt(OB.option, num);
				}
			}
			if (OB.option <= AUDIO_WHEEL_DEFAULT_TO_ZOOM){ audio = true; }
		}
		else if (OB.ctrl->IsKindOf(CLASSINFO(KaiListCtrl))){
			KaiListCtrl *list = (KaiListCtrl*)OB.ctrl;
			if (list->GetModified()){

				if (OB.option == (CONFIG)ID_COLOR_CONFIG){
					list->SaveAll(1);
					Options.SaveColors();
					ChangeColors();
				}
				else if (OB.option == (CONFIG)ID_HOTKEYS_CONFIG){
					if (list->GetModified() && hotkeysCopy.size()){
						list->SaveAll(1);
						Hkeys.SetHotkeysMap(hotkeysCopy);
						Hkeys.SaveHkeys();
						Hkeys.SaveHkeys(true);
						KainoteFrame* Kai =
							/*wxDynamicCast<*/(KainoteFrame*)Notebook::GetTabs()->GetParent();//>
						Kai->SetAccels();
					}
				}
				else{
#ifdef _WIN32
					wxString extensions[] = { L".ass", L".ssa", L".srt", L".sub", L".txt", L".mkv", L".mp4", L".avi",
						L".ogm", L".wmv", L".asf", L".rmvb", L".rm", L".3gp", L".mpg", L".mpeg", L".ts", L".m2ts" };
					wxString extensionsDesc[] = { _("ASS subtitles"), _("SSA subtitles"), _("SRT subtitles"), _("SUB subtitles"),
						_("TXT subtitles"), _("Video MKV"), _("Video MP4"), _("Video AVI"), _("Video OGM"),
						_("Video WMV"), _("Video ASF"), _("Video RMVB"), _("Video RM"), _("Video 3GP"),
						_("Video MPG"), _("Video MPEG"), _("Video TS"), _("Video M2TS") };

					for (size_t i = 0; i < registeredExts.size(); i++){
						if (list->GetItem(i, 0)->modified != registeredExts[i]){
							if (registeredExts[i])
								Registry::RemoveFileAssociation(extensions[i]);
							else
								Registry::AddFileAssociation(extensions[i], extensionsDesc[i], i);
						}
					}
					//Registry::RefreshRegistry();
#endif
				}
			}
		}
	}
	if (fontmod){
		Notebook::GetTab()->grid->SetStyle();
		Notebook::GetTab()->grid->RefreshColumns();
		if (Notebook::GetTabs()->split){
			Notebook::GetTabs()->GetSecondPage()->grid->SetStyle();
			Notebook::GetTabs()->GetSecondPage()->grid->RefreshColumns();
		}
	}
	if (colmod){
		Notebook::GetTab()->grid->Refresh(false);
		if (Notebook::GetTabs()->split){
			Notebook::GetTabs()->GetSecondPage()->grid->Refresh(false);

		}
	}
	if (audio && Notebook::GetTab()->edit->ABox){ 
		Notebook::GetTab()->edit->ABox->audioDisplay->ChangeOptions();
	}
	Options.SaveOptions();
	Options.SaveAudioOpts();
}

void OptionsDialog::OnMapHkey(wxCommandEvent& event)
{
	int inum = Shortcuts->GetSelection();
	if (inum < 0){ return; }
	ItemHotkey * item = (ItemHotkey *)Shortcuts->GetItem(inum, 1);
	if (item){ item->OnMapHotkey(Shortcuts, inum); }
}

void OptionsDialog::OnResetHkey(wxCommandEvent& event)
{
	int inum = Shortcuts->GetSelection();
	if (inum < 0){ return; }
	ItemHotkey * item = (ItemHotkey *)Shortcuts->GetItem(inum, 1);
	if (item){ item->OnResetHotkey(Shortcuts, inum); }
}

void OptionsDialog::OnDeleteHkey(wxCommandEvent& event)
{
	int inum = Shortcuts->GetSelection();
	if (inum < 0){ return; }
	ItemHotkey * item = (ItemHotkey *)Shortcuts->GetItem(inum, 1);
	if (item){ item->OnDeleteHotkey(Shortcuts, inum); }
}

void OptionsDialog::OnChangeCatalog(wxCommandEvent& event)
{
	Options.SaveOptions(false);
	Options.LoadStyles(Katlist->GetString(Katlist->GetSelection()));
	Stylelist->Clear();
	for (int i = 0; i < Options.StoreSize(); i++){
		Stylelist->Append(Options.GetStyle(i)->Name);
	}
	Stylelist->SetSelection(0);
}

void OptionsDialog::ChangeColors(){

	const wxColour & windowColor = Options.GetColour(WINDOW_BACKGROUND);
	const wxColour & textColor = Options.GetColour(WINDOW_TEXT);
	Notebook *nb = Notebook::GetTabs();
	//tabs colors		
	for (size_t i = 0; i < nb->Size(); i++){
		TabPanel *tab = nb->Page(i);
		tab->SetBackgroundColour(windowColor);
		tab->SetForegroundColour(textColor);
		if (tab->edit->ABox){
			tab->edit->ABox->audioDisplay->ChangeOptions();
		}
		const wxWindowList& siblings = tab->GetChildren();
		for (auto it = siblings.begin(); it != siblings.end(); it++){
			(*it)->SetBackgroundColour(windowColor);
			(*it)->SetForegroundColour(textColor);
		}

	}
	//tree colours
	OptionsTree->SetColours(windowColor, textColor);
	//dialogs colours
	wxWindowList::compatibility_iterator node = wxTopLevelWindows.GetFirst();
	while (node)
	{
		wxWindow* win = node->GetData();
		win->SetBackgroundColour(windowColor);
		win->SetForegroundColour(textColor);
		win->Refresh();
		node = node->GetNext();
	}

	StyleStore::Get()->cc->UpdatePreview();
}

void OptionsDialog::ResetDefault()
{
	Options.ResetDefault();
	for (size_t i = 0; i < handles.size(); i++)
	{
		const OptionsBind &OB = handles[i];
		if (OB.ctrl->IsKindOf(CLASSINFO(KaiCheckBox))){
			KaiCheckBox *cb = (KaiCheckBox*)OB.ctrl;
			cb->SetValue(Options.GetBool(OB.option));
		}
		else if (OB.ctrl->IsKindOf(CLASSINFO(KaiChoice))){
			KaiChoice *cbx = (KaiChoice*)OB.ctrl;
			if (cbx->GetWindowStyle() & KAI_COMBO_BOX){
				cbx->SetValue(Options.GetString(OB.option));
			}
			else if (cbx->GetId() != ID_KAI_CHOICE){
				cbx->SetSelection(Options.GetInt(OB.option));
			}//dictionary language            vobsub                   program language dont change that
			else if (cbx->GetId() != ID_DICTIONARY_LANGUAGE || cbx->GetId() != ID_VSFILTER_PROVIDER || cbx->GetId() != ID_PROGRAM_LANGUAGE){
				cbx->SetSelection(cbx->FindString(Options.GetString(OB.option)));
			}
		}
		else if (OB.ctrl->IsKindOf(CLASSINFO(FontPickerButton))){
			FontPickerButton *fpc = (FontPickerButton*)OB.ctrl;
			wxFont font(Options.GetInt(OB.option == PROGRAM_FONT ? PROGRAM_FONT_SIZE : GRID_FONT_SIZE), 
				wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, Options.GetString(OB.option));
			fpc->ChangeFont(font);
		}
		else if (OB.ctrl->IsKindOf(CLASSINFO(KaiTextCtrl))){
			if (OB.ctrl->GetId() != ID_NUMBER_CONTROL){
				KaiTextCtrl *sc = (KaiTextCtrl*)OB.ctrl;
				sc->SetValue(Options.GetString(OB.option));
			}
			else{
				NumCtrl *sc = (NumCtrl*)OB.ctrl;
				sc->SetInt(Options.GetInt(OB.option));
			}
		}
	}
	
	Hkeys.ResetDefaults();
	Shortcuts->ClearList();
	AddHotkeysOnList();
}

void OptionsDialog::OnResetDefault(wxCommandEvent& event)
{
	ResetDefault();
}

void OptionsDialog::AddHotkeysOnList()
{
	std::map<idAndType, hdata> mappedhkeys = std::map<idAndType, hdata>(Hkeys.GetHotkeysMap());
	const std::map<int, wxString> &hkeysNames = Hkeys.GetNamesTable();

	int lastType = -1;

	//to make names in right order enumerate names, 
	//and remove hotkeys from mappedhkeys table,
	//when type changes set hotkeys that are not from it's window or scripts
	for (auto cur = hkeysNames.rbegin(); cur != hkeysNames.rend(); cur++) {
		int htype = Hkeys.GetType(cur->first);
		wxString name;
		wxString accel;
		//copy to not change cur
		auto copyCur = cur;
		//here add skipped hotkeys to put it on end
		if ((lastType != htype && lastType != -1) || !(++copyCur != hkeysNames.rend())){
			int numdelete = 0;
			for (auto curmhk = mappedhkeys.begin(); curmhk != mappedhkeys.end(); curmhk++) {
				if (lastType != curmhk->first.Type)
					break;
				//skik quit
				if (curmhk->first.id == GLOBAL_QUIT){
					numdelete++;
					continue;
				}

				wxString windowName = windowNames[curmhk->first.Type] + L" ";
				const auto & it = hkeysNames.find(curmhk->first.id);
				if (it != hkeysNames.end()){
					name = it->second;
				}
				else{
					name = curmhk->second.Name;
				}
				//add hotkey on list
				long pos = Shortcuts->AppendItem(new ItemText(windowName + name));
				Shortcuts->SetItem(pos, 1, new ItemHotkey(name, curmhk->second.Accel, curmhk->first));
				numdelete++;
			}
			//remove from mappedhotkeys
			for (int p = 0; p < numdelete; p++)
				mappedhkeys.erase(mappedhkeys.begin());
		}
		//I have to end on last element
		//and is added in above loop
		if (mappedhkeys.size() == 0)
			break;

		name = windowNames[htype] + L" " + cur->second;
		//seeking for mapped hotkey
		const auto & it = mappedhkeys.find(idAndType(cur->first, htype));
		if (it != mappedhkeys.end()){
			accel = it->second.Accel;
			mappedhkeys.erase(it);
		}
		//set element on list
		long pos = Shortcuts->AppendItem(new ItemText(name));
		Shortcuts->SetItem(pos, 1, new ItemHotkey(cur->second, accel, idAndType(cur->first, htype)));
		lastType = htype;
	}
	//setup shortcut list
	Shortcuts->StartEdition();
	Shortcuts->SetSelection(0);
}
