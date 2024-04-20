//  Copyright (c) 2016 - 2022, Marcin Drob

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
#include "kainoteFrame.h"
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
#include <wx/dir.h>
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
					wxString::Format(_("Ten skrót już istnieje jako skrót do \"%s\".\nCo zrobić?"),
					doubledHkName), _("Uwaga"), wxYES | wxOK | wxCANCEL);
				msg.SetOkLabel(_("Zamień skróty"));
				msg.SetYesLabel(_("Usuń skrót"));
				result = msg.ShowModal();
			}
			else{
				int buttonFlag = (idtypes.size() < 2) ? wxOK : 0;
				KaiMessageDialog msg(theList,
					wxString::Format(_("Ten skrót już istnieje w %s jako skrót do \"%s\".\nCo zrobić?"),
					(idtypes.size() > 1) ? _("innych oknach") : _("innym oknie"), doubledHkName), _("Uwaga"), wxYES_NO | buttonFlag | wxCANCEL);
				if (idtypes.size() < 2)
					msg.SetOkLabel(_("Zamień skróty"));
				msg.SetYesLabel(_("Usuń skrót"));
				msg.SetNoLabel(_("Ustaw mimo to"));
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

OptionsDialog::OptionsDialog(wxWindow *parent)
	: KaiDialog(parent, -1, _("Opcje"))
{
	windowNames = new wxString[5]{ _("Globalny"), _("Napisy"), _("Edytor"), _("Wideo"), _("Audio") };
	OptionsTree = new KaiTreebook(this, -1);

	Stylelist = nullptr;
	Katlist = nullptr;

	wxIcon icn;
	icn.CopyFromBitmap(CreateBitmapFromPngResource(L"SETTINGS"));
	SetIcon(icn);

	wxWindow *GLOBAL_EDITOR = new wxWindow(OptionsTree, -1);
	wxWindow *EditorAdvanced = new wxWindow(OptionsTree, -1);
	wxWindow *ConvOpt = new wxWindow(OptionsTree, -1);
	wxWindow *Hotkeyss = new wxWindow(OptionsTree, -1);
	wxWindow *AudioMain = new wxWindow(OptionsTree, -1);
	wxWindow *AudioSecond = new wxWindow(OptionsTree, -1);
	wxWindow *video = new wxWindow(OptionsTree, -1);
	wxWindow *Themes = new wxWindow(OptionsTree, -1);
	wxWindow *Assocs = new wxWindow(OptionsTree, -1);
	wxWindow *SubsProps = new SubtitlesProperties(OptionsTree, this);

	hkeymodif = 0;
	if (!Options.AudioOpts && !Options.LoadAudioOpts()){ KaiMessageBox(_("Nie można wczytać opcji audio"), _("Błąd")); }

	//Main
	{
		const int optsSize = 18;
		wxBoxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
		wxString labels[optsSize] = { _("Wczytywanie posortowanych napisów"), _("Włącz sprawdzanie pisowni"),
			_("Zaznaczaj linijkę z czasem aktywnej\nlinijki poprzedniej zakładki"),
			_("Pokaż sugestie po dwukrotnym kliknięciu błędu"), _("Otwieraj napisy zawsze w nowej karcie"),
			_("Nie przechodź do następnej linii przy edycji czasów"),
			_("Wyłącz pokazywanie edycji na wideo\n(wymaga ponownego otwarcia zakładek)"),
			_("Włącz szukanie widocznej linii\npo wyjściu z pełnego ekranu"),
			_("Włącz przenoszenie wartości pola przesuwania czasów"), _("Zmieniaj aktywną linię przy zaznaczaniu"),
			_("Pokazuj oryginał w trybie tłumacza"), _("Ukryj oryginał na wideo w trybie tłumacza"),
			_("Nie zmieniaj zaznaczeń przy duplikacji linii dialogowych"), 
			_("Nie wypośrodkowuj aktywnej linii w polu napisów"), _("Używaj skróty klawiszowe numpada w polach tekstowych"),
			_("Wyłącz ostrzeżenia w narzędziach edycji wizualnej"), _("Nie ostrzegaj o niezgodności rozdzielczości"),
			_("Kompatybilność ze starymi skryptami Kainote") };
		CONFIG opts[optsSize] = { GRID_LOAD_SORTED_SUBS, SPELLCHECKER_ON, AUTO_SELECT_LINES_FROM_LAST_TAB,
			EDITBOX_SUGGESTIONS_ON_DOUBLE_CLICK, OPEN_SUBS_IN_NEW_TAB, EDITBOX_DONT_GO_TO_NEXT_LINE_ON_TIMES_EDIT,
			DISABLE_LIVE_VIDEO_EDITING, GRID_SET_VISIBLE_LINE_AFTER_FULL_SCREEN, SHIFT_TIMES_CHANGE_VALUES_WITH_TAB,
			GRID_CHANGE_ACTIVE_ON_SELECTION, TL_MODE_SHOW_ORIGINAL, TL_MODE_HIDE_ORIGINAL_ON_VIDEO, 
			GRID_DUPLICATION_DONT_CHANGE_SELECTION, GRID_DONT_CENTER_ACTIVE_LINE,
			TEXT_FIELD_ALLOW_NUMPAD_HOTKEYS, VIDEO_VISUAL_WARNINGS_OFF,
			DONT_ASK_FOR_BAD_RESOLUTION, AUTOMATION_OLD_SCRIPTS_COMPATIBILITY };
		wxString localePath = Options.pathfull + L"\\Locale";
		wxDir kat(localePath);
		wxArrayString langs;
		if (kat.IsOpened()){
			kat.GetAllFiles(localePath, &langs, L"*.mo", wxDIR_FILES);
		}
		programLanguages.push_back(L"pl");
		for (size_t i = 0; i < langs.GetCount(); i++){
			wxString fulllang = langs[i].AfterLast(L'\\').BeforeLast(L'.');
			programLanguages.push_back(fulllang);
			const wxString &fullName = Options.FindLanguage(fulllang);
			langs[i] = fullName;
		}
		langs.Insert(L"Polski", 0);
		KaiStaticBoxSizer *langSizer = new KaiStaticBoxSizer(wxVERTICAL, GLOBAL_EDITOR, _("Język (wymaga restartu programu)"));
		KaiChoice *programLanguage = new KaiChoice(GLOBAL_EDITOR, ID_PROGRAM_LANGUAGE, wxDefaultPosition, wxDefaultSize, langs);
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
		if (dictionaries.size() == 0){ dictionaries.Add(_("Umieść pliki .dic i .aff do folderu \"Dictionary\"")); }
		KaiStaticBoxSizer *dicSizer = new KaiStaticBoxSizer(wxVERTICAL, GLOBAL_EDITOR, _("Język sprawdzania pisowni (folder \"Dictionary\")"));

		KaiChoice *dic = new KaiChoice(GLOBAL_EDITOR, ID_DICTIONARY_LANGUAGE, wxDefaultPosition, wxDefaultSize, dictionaries);

		dic->SetSelection(dic->FindString(Options.FindLanguage(Options.GetString(DICTIONARY_LANGUAGE))));
		ConOpt(dic, DICTIONARY_LANGUAGE);
		dicSizer->Add(dic, 0, wxALL | wxEXPAND, 2);
		MainSizer->Add(dicSizer, 0, wxRIGHT | wxEXPAND, 5);

		for (int i = 0; i < optsSize; i++)
		{
			KaiCheckBox *opt = new KaiCheckBox(GLOBAL_EDITOR, -1, labels[i]);
			opt->SetValue(Options.GetBool(opts[i]));
			ConOpt(opt, opts[i]);
			MainSizer->Add(opt, 0, wxALL, 2);
		}
		GLOBAL_EDITOR->SetSizerAndFit(MainSizer);

		
		wxBoxSizer *Main1Sizer = new wxBoxSizer(wxVERTICAL);
		//Warning id ID_NUMBER_CONTROL is only for NumCtrl, normal text fields have to have another id
		NumCtrl *gridSaveAfter = new NumCtrl(EditorAdvanced, ID_NUMBER_CONTROL, Options.GetString(GRID_SAVE_AFTER_CHARACTER_COUNT), 0, 10000, true, wxDefaultPosition, wxSize(120, -1), wxTE_PROCESS_ENTER);
		gridSaveAfter->SetToolTip(_("Zero całkowicie wyłącza zapis przy edycji"));
		NumCtrl *autoSaveMax = new NumCtrl(EditorAdvanced, ID_NUMBER_CONTROL, Options.GetString(AUTOSAVE_MAX_FILES), 2, 1000000, true, wxDefaultPosition, wxSize(120, -1), wxTE_PROCESS_ENTER);
		autoSaveMax->SetToolTip(_("Liczbę plików autozapisu można ustawić od 2 do 1000000"));
		int numMaxChars = Options.GetInt(TAB_TEXT_MAX_CHARS);
		if (!numMaxChars)
			numMaxChars = 40;
		NumCtrl *maxTabChars = new NumCtrl(EditorAdvanced, ID_NUMBER_CONTROL, std::to_wstring(numMaxChars), 20, 150, true, wxDefaultPosition, wxSize(120, -1), wxTE_PROCESS_ENTER);
		maxTabChars->SetToolTip(_("Liczbę znaków widocznych na zakładce można ustawić od 20 do 150"));
		NumCtrl *ltl = new NumCtrl(EditorAdvanced, ID_NUMBER_CONTROL, Options.GetString(AUTOMATION_TRACE_LEVEL), 0, 5, true, wxDefaultPosition, wxSize(120, -1), wxTE_PROCESS_ENTER);
		NumCtrl *sc = new NumCtrl(EditorAdvanced, ID_NUMBER_CONTROL, Options.GetString(GRID_INSERT_START_OFFSET), -100000, 100000, true, wxDefaultPosition, wxSize(120, -1), wxTE_PROCESS_ENTER);
		NumCtrl *sc1 = new NumCtrl(EditorAdvanced, ID_NUMBER_CONTROL, Options.GetString(GRID_INSERT_END_OFFSET), -100000, 100000, true, wxDefaultPosition, wxSize(120, -1), wxTE_PROCESS_ENTER);
		KaiTextCtrl *sc2 = new KaiTextCtrl(EditorAdvanced, ID_TAGS_SWAP_CHARACTER, Options.GetString(GRID_TAGS_SWAP_CHARACTER), wxDefaultPosition, wxSize(120, -1), wxTE_PROCESS_ENTER);
		
		ConOpt(gridSaveAfter, GRID_SAVE_AFTER_CHARACTER_COUNT);
		ConOpt(autoSaveMax, AUTOSAVE_MAX_FILES);
		ConOpt(ltl, AUTOMATION_TRACE_LEVEL);
		ConOpt(maxTabChars, TAB_TEXT_MAX_CHARS);
		ConOpt(sc, GRID_INSERT_START_OFFSET);
		ConOpt(sc1, GRID_INSERT_END_OFFSET);
		ConOpt(sc2, GRID_TAGS_SWAP_CHARACTER);
		wxBoxSizer *MainSizer2 = new wxBoxSizer(wxHORIZONTAL);
		MainSizer2->Add(new KaiStaticText(EditorAdvanced, -1, _("Ilość edycji do zapisu")/*, wxDefaultPosition, wxSize(256, -1)*/), 5, wxALIGN_CENTRE_VERTICAL | wxEXPAND);
		MainSizer2->Add(gridSaveAfter, 0, wxEXPAND);
		wxBoxSizer *MainSizer3 = new wxBoxSizer(wxHORIZONTAL);
		MainSizer3->Add(new KaiStaticText(EditorAdvanced, -1, _("Maksymalna ilość plików autozapisu")/*, wxDefaultPosition, wxSize(256, -1)*/), 5, /*wxALIGN_CENTRE_VERTICAL | */wxEXPAND);
		MainSizer3->Add(autoSaveMax, 0, wxEXPAND);
		wxBoxSizer *MainSizer4 = new wxBoxSizer(wxHORIZONTAL);
		MainSizer4->Add(new KaiStaticText(EditorAdvanced, -1, _("Przyspieszenie klatki początkowej w ms:")/*, wxDefaultPosition, wxSize(256, -1)*/), 5, /*wxALIGN_CENTRE_VERTICAL | */wxEXPAND);
		MainSizer4->Add(sc, 0, wxEXPAND);
		wxBoxSizer *MainSizer5 = new wxBoxSizer(wxHORIZONTAL);
		MainSizer5->Add(new KaiStaticText(EditorAdvanced, -1, _("Przyspieszenie klatki końcowej w ms:")), 5, /*wxALIGN_CENTRE_VERTICAL | */wxEXPAND);
		MainSizer5->Add(sc1, 0, wxEXPAND);
		wxBoxSizer *MainSizer6 = new wxBoxSizer(wxHORIZONTAL);
		MainSizer6->Add(new KaiStaticText(EditorAdvanced, -1, _("Znak podmiany tagów ASS:")), 5, /*wxALIGN_CENTRE_VERTICAL | */wxEXPAND);
		MainSizer6->Add(sc2, 0, wxEXPAND);
		wxBoxSizer *MainSizer7 = new wxBoxSizer(wxHORIZONTAL);
		MainSizer7->Add(new KaiStaticText(EditorAdvanced, -1, _("Ilość znaków widocznych na zakładce")), 5, /*wxALIGN_CENTRE_VERTICAL | */wxEXPAND);
		MainSizer7->Add(maxTabChars, 0, wxEXPAND);
		wxBoxSizer *MainSizer8 = new wxBoxSizer(wxHORIZONTAL);
		MainSizer8->Add(new KaiStaticText(EditorAdvanced, -1, _("Poziom śledzenia logów skryptów LUA")), 5, /*wxALIGN_CENTRE_VERTICAL | */wxEXPAND);
		MainSizer8->Add(ltl, 0, wxEXPAND);

		//MainSizer->Add(MainSizer2,0,wxLEFT|wxTOP,2);

		FontPickerButton *optf = new FontPickerButton(EditorAdvanced, -1, wxFont(Options.GetInt(GRID_FONT_SIZE), wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, Options.GetString(GRID_FONT)));
		ConOpt(optf, GRID_FONT);
		optf->SetMinSize(ltl->GetMinSize());
		wxBoxSizer *MainSizer9 = new wxBoxSizer(wxHORIZONTAL);
		MainSizer9->Add(new KaiStaticText(EditorAdvanced, -1, _("Czcionka pola napisów:")), 5, wxRIGHT | /*wxALIGN_CENTRE_VERTICAL | */wxEXPAND, 10);
		MainSizer9->Add(optf, 0, wxEXPAND);

		FontPickerButton *programFont = new FontPickerButton(EditorAdvanced, -1, wxFont(Options.GetInt(PROGRAM_FONT_SIZE), wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, Options.GetString(PROGRAM_FONT)));
		ConOpt(programFont, PROGRAM_FONT);
		programFont->SetMinSize(ltl->GetMinSize());
		wxBoxSizer *MainSizer10 = new wxBoxSizer(wxHORIZONTAL);
		MainSizer10->Add(new KaiStaticText(EditorAdvanced, -1, _("Czcionka programu:")), 5, wxRIGHT | /*wxALIGN_CENTRE_VERTICAL | */wxEXPAND, 10);
		MainSizer10->Add(programFont, 0, wxEXPAND);

		KaiStaticBoxSizer *alm = new KaiStaticBoxSizer(wxHORIZONTAL, EditorAdvanced, _("Sposób wczytywania skryptów autoload"));
		wxString methods[] = { _("Przy starcie programu asynchronicznie"), _("Przy starcie programu"), _("Przy otwarciu menu asynchronicznie"), _("Przy otwarciu menu") };
		KaiChoice *cmb = new KaiChoice(EditorAdvanced, ID_KAI_CHOICE, wxDefaultPosition, wxSize(200, -1), 4, methods, wxTE_PROCESS_ENTER);
		cmb->SetSelection(Options.GetInt(AUTOMATION_LOADING_METHOD));
		ConOpt(cmb, AUTOMATION_LOADING_METHOD);
		alm->Add(cmb, 1, wxCENTER | wxEXPAND | wxALL, 2);

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

		EditorAdvanced->SetSizerAndFit(Main1Sizer);
	}

	//conversion settings
	{
		wxBoxSizer *ConvOptSizer1 = new wxBoxSizer(wxVERTICAL);

		KaiStaticBoxSizer *obr = new KaiStaticBoxSizer(wxHORIZONTAL, ConvOpt, _("Wybierz katalog"));
		KaiStaticBoxSizer *obr0 = new KaiStaticBoxSizer(wxHORIZONTAL, ConvOpt, _("Wybierz styl"));
		KaiStaticBoxSizer *obr1 = new KaiStaticBoxSizer(wxHORIZONTAL, ConvOpt, _("Wybierz FPS"));
		KaiStaticBoxSizer *obr2 = new KaiStaticBoxSizer(wxHORIZONTAL, ConvOpt, _("Czas w ms na jedną literę"));
		KaiStaticBoxSizer *obr3 = new KaiStaticBoxSizer(wxHORIZONTAL, ConvOpt, _("Tagi wstawiane na początku każdej linijki ass"));
		KaiStaticBoxSizer *obr4 = new KaiStaticBoxSizer(wxHORIZONTAL, ConvOpt, _("Rozdzielczość przy konwersji na ASS"));
		wxArrayString styles;
		wxArrayString FPSes;



		FPSes.Add(L"23.976"); FPSes.Add(L"24"); FPSes.Add(L"25"); FPSes.Add(L"29.97"); FPSes.Add(L"30"); FPSes.Add(L"60");

		for (int i = 0; i < 2; i++){
			wxString optname = (i == 0) ? Options.GetString(CONVERT_STYLE_CATALOG) : Options.GetString(CONVERT_STYLE);
			if (i != 0){
				for (int i = 0; i < Options.StoreSize(); i++){
					styles.Add(Options.GetStyle(i)->Name);
				}
			}
			KaiChoice *cmb = new KaiChoice(ConvOpt, (i == 0) ? 28888 : 28889, wxDefaultPosition, wxSize(200, -1), (i == 0) ? Options.dirs : styles, wxTE_PROCESS_ENTER);

			int sel = cmb->FindString(optname);

			if (sel >= 0){ cmb->SetSelection(sel); if (i == 0 && Options.actualStyleDir != optname){ Options.LoadStyles(optname); } }
			else{
				if (i == 0){ sel = cmb->FindString(Options.actualStyleDir); }
				cmb->SetSelection(MAX(0, sel));
				wxString what = (i == 0) ? _("katalog dla stylu") : _("styl");
				KaiMessageBox(wxString::Format(_("Wybrany %s konwersji nie istnieje,\nzostanie zmieniony na domyślny"), what), _("Uwaga"));
			}

			ConOpt(cmb, (i == 0) ? CONVERT_STYLE_CATALOG : CONVERT_STYLE);
			if (i == 0){
				obr->Add(cmb, 1, wxCENTER | wxALL, 2);
				ConvOptSizer1->Add(obr, 0, wxRIGHT | wxEXPAND, 5);
				Katlist = cmb;
				Connect(28888, wxEVT_COMMAND_CHOICE_SELECTED, (wxObjectEventFunction)&OptionsDialog::OnChangeCatalog);
			}
			else{
				obr0->Add(cmb, 1, wxCENTER | wxALL, 2);
				ConvOptSizer1->Add(obr0, 0, wxRIGHT | wxEXPAND, 5);
				Stylelist = cmb;
			}
		}
		const wxString & convFPS = Options.GetString(CONVERT_FPS);
		KaiChoice *cmb = new KaiChoice(ConvOpt, -1, convFPS, wxDefaultPosition, wxSize(200, -1), FPSes, wxTE_PROCESS_ENTER);
		int sel = cmb->FindString(convFPS);
		if (sel >= 0){ cmb->SetSelection(sel); }
		else{ cmb->SetValue(convFPS); }

		ConOpt(cmb, CONVERT_FPS);
		obr1->Add(cmb, 1, wxCENTER | wxALL, 2);
		ConvOptSizer1->Add(obr1, 0, wxRIGHT | wxEXPAND, 5);


		for (int i = 0; i < 3; i++)
		{
			KaiCheckBox *opt = new KaiCheckBox(ConvOpt, -1, (i == 0) ? _("FPS z wideo") : 
				(i == 1) ? _("Nowe czasy końcowe") : _("Pokaż okno przed konwersją"));
			CONFIG optname = (i == 0) ? CONVERT_FPS_FROM_VIDEO : (i == 1) ? CONVERT_NEW_END_TIMES : CONVERT_SHOW_SETTINGS;
			opt->SetValue(Options.GetBool(optname));
			ConOpt(opt, optname);
			ConvOptSizer1->Add(opt, 0, wxRIGHT | wxEXPAND, 5);
		}

		NumCtrl *sc = new NumCtrl(ConvOpt, ID_NUMBER_CONTROL, Options.GetString(CONVERT_TIME_PER_CHARACTER), 30, 1000, true, 
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

		KaiTextCtrl *tc = new KaiTextCtrl(ConvOpt, -1, Options.GetString(CONVERT_ASS_TAGS_TO_INSERT_IN_LINE), wxDefaultPosition, wxSize(250, -1), wxTE_PROCESS_ENTER);
		ConOpt(tc, CONVERT_ASS_TAGS_TO_INSERT_IN_LINE);
		obr3->Add(tc, 1, wxALL /*| wxALIGN_CENTER*/ | wxEXPAND, 2);
		ConvOptSizer1->Add(obr3, 0, wxRIGHT | wxEXPAND, 5);

		ConvOpt->SetSizerAndFit(ConvOptSizer1);
	}
	//video
	{
		wxString voptspl[] = { _("Otwórz wideo z menu kontekstowego na pełnym ekranie"), _("Lewy przycisk myszy pauzuje wideo"),
			_("Otwieraj wideo z czasem aktywnej linii"), _("Preferowane ścieżki audio (oddzielone średnikiem)"),
			_("Sposób szukania wideo w FFMS2 (wymaga ponownego wczytania)"), _("Filtr wyświetlania napisów") };
		CONFIG vopts[] = { VIDEO_FULL_SCREEN_ON_START, VIDEO_PAUSE_ON_CLICK, OPEN_VIDEO_AT_ACTIVE_LINE,
			ACCEPTED_AUDIO_STREAM, FFMS2_VIDEO_SEEKING, VSFILTER_INSTANCE };
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
		prefaudio->Add(tc, 1, wxALL /*| wxALIGN_CENTER*/ | wxEXPAND, 2);
		MainSizer->Add(prefaudio, 0, wxRIGHT | wxEXPAND, 5);
		KaiStaticBoxSizer *seekingsizer = new KaiStaticBoxSizer(wxHORIZONTAL, video, voptspl[4]);

		wxString seekingOpts[] = { _("Liniowe"), _("Normalne"), 
			_("Niebezpieczne (szybkie w każdym przypadku)"), _("Agresywne (szybkie przy cofaniu)") };
		KaiChoice *sopts = new KaiChoice(video, ID_KAI_CHOICE, 
			wxDefaultPosition, wxSize(200, -1), 4, seekingOpts, wxTE_PROCESS_ENTER);
		int selection = Options.GetInt(vopts[4]);
		if (selection < 0 || selection > 3) {
			selection = 2;
			Options.SetInt(vopts[4], selection);
			Options.SaveOptions(true, false);
		}
		sopts->SetSelection(selection);
		seekingsizer->Add(sopts, 1, wxALL /*| wxALIGN_CENTER*/ | wxEXPAND, 2);
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
		filtersizer->Add(vsfiltersList, 1, wxALL /*| wxALIGN_CENTER*/ | wxEXPAND, 2);
		MainSizer->Add(filtersizer, 0, wxRIGHT | wxEXPAND, 5);
		ConOpt(vsfiltersList, vopts[5]);
		video->SetSizerAndFit(MainSizer);
	}
	//Hotkeys
	{
		wxBoxSizer *HkeysSizer = new wxBoxSizer(wxVERTICAL);
		KaiStaticBoxSizer *filterMode = new KaiStaticBoxSizer(wxHORIZONTAL, Hotkeyss, _("Wybierz rodzaj filtrowania"));
		wxString filteringModes[] = { _("Wszystko"), _("Ustawione skróty"), _("Skróty Globalne"),
			_("Skróty napisów"), _("Skróty edytora"), _("Skróty wideo"), _("Skróty audio"), };
		KaiChoice *filterList = new KaiChoice(Hotkeyss, 14568, wxDefaultPosition, wxDefaultSize, 7, filteringModes);
		filterList->SetSelection(0);
		filterList->SetToolTip(_("Rodzaj filtrowania:"));

		filterMode->Add(filterList, 1, wxALL | wxEXPAND, 2);
		HkeysSizer->Add(filterMode, 0, wxEXPAND);
		wxString mesureText = _("Globalny") + L" " + _("Lista osób pomocnych przy tworzeniu programu");
		wxString mesureText2 = L"Ctrl-Shift-Delete";
		int fw, fww, fh;
		GetTextExtent(mesureText, &fw, &fh);
		GetTextExtent(mesureText2, &fww, &fh);

		Shortcuts = new KaiListCtrl(Hotkeyss, 26667, wxDefaultPosition);
		Shortcuts->InsertColumn(0, _("Funkcja"), TYPE_TEXT, (fw < 275)? 275 : fw);
		Shortcuts->InsertColumn(1, _("Skrót"), TYPE_TEXT, (fww < 80)? 80 : fww);
		Connect(26667, LIST_ITEM_DOUBLECLICKED, (wxObjectEventFunction)&OptionsDialog::OnMapHkey);
		//Connect(26667,LIST_ITEM_RIGHT_CLICK,(wxObjectEventFunction)&OptionsDialog::OnResetHkey);

		if (!Hkeys.AudioKeys && !Hkeys.LoadHkeys(true)){ 
			KaiMessageBox(_("Nie można wczytać skrótów klawiszowych audio"), _("Błąd")); 
		}

		AddHotkeysOnList();

		HkeysSizer->Add(Shortcuts, 1, wxALL | wxEXPAND, 4);
		wxBoxSizer *buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
		MappedButton *setHotkey = new MappedButton(Hotkeyss, 23232, _("Mapuj skrót"));
		Connect(23232, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&OptionsDialog::OnMapHkey);
		MappedButton *resetHotkey = new MappedButton(Hotkeyss, 23231, _("Przywróć skrót domyślny"));
		Connect(23231, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&OptionsDialog::OnResetHkey);
		MappedButton *deleteHotkey = new MappedButton(Hotkeyss, 23230, _("Usuń skrót"));
		Connect(23230, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&OptionsDialog::OnDeleteHkey);
		buttonsSizer->Add(setHotkey, 0, wxALL, 2);
		buttonsSizer->Add(resetHotkey, 0, wxALL, 2);
		buttonsSizer->Add(deleteHotkey, 0, wxALL, 2);
		HkeysSizer->Add(buttonsSizer, 0, wxALL | wxALIGN_CENTER, 2);
		// filter list it need created list
		Bind(wxEVT_COMMAND_CHOICE_SELECTED, [=](wxCommandEvent &evt){
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
		wxString names[numOfElements] = { _("Wyświetlaj czas przy kursorze"), _("Wyświetlaj znaczniki sekund"), _("Wyświetlaj tło zaznaczenia"),
			_("Wyświetlaj pozycję wideo"), _("Wyświetlaj klatki kluczowe"), _("Przewijaj wykres audio przy odtwarzaniu"),
			_("Aktywuj okno audio po najechaniu"), _("Przyklejaj do klatek kluczowych"), _("Przyklejaj do pozostałych linii"),
			_("Nie odtwarzaj audio po zmianie linijki"), _("Scalaj wszystkie \"n\" z poprzednią sylabą"), 
			_("Przenoś linie sylab po kliknięciu"),_("Wczytuj audio do pamięci RAM") };

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
		audioCacheFilesLimit->SetToolTip(_("Przedział od 0 do 10000, gdzie 0 wyłącza\ncałkowicie usuwanie plików audio cache."));
		wxString inact[3] = { _("Brak"), _("Przed i po aktywnej"), _("Wszystkie widoczne") };
		KaiChoice *displayNonActiveLines = new KaiChoice(AudioSecond, ID_KAI_CHOICE, wxDefaultPosition, wxSize(300, -1), 3, inact);
		displayNonActiveLines->SetSelection(Options.GetInt(opts1[2]));
		ConOpt(Delay, opts1[0]);
		ConOpt(markPlayTime, opts1[1]);
		ConOpt(lineThickness, opts1[3]);
		ConOpt(displayNonActiveLines, opts1[2]);
		ConOpt(audioCacheFilesLimit, opts1[4]);
		ConOpt(leadInTime, opts1[5]);
		ConOpt(leadOutTime, opts1[6]);
		KaiStaticBoxSizer *DelaySizer = new KaiStaticBoxSizer(wxVERTICAL, AudioSecond, _("Opóźnienie audio w ms"));
		KaiStaticBoxSizer *markPlayTimeSizer = new KaiStaticBoxSizer(wxVERTICAL, AudioSecond, _("Czas odtwarzania audio przed i po znaczniku w ms"));
		wxString elems[] = { _("Wstęp"), _("Zakończenie") };
		KaiStaticBoxSizer *leadInAndOut = new KaiStaticBoxSizer(wxHORIZONTAL, AudioSecond, 2, elems);
		KaiStaticBoxSizer *lineThicknessSizer = new KaiStaticBoxSizer(wxVERTICAL, AudioSecond, _("Grubość linii znaczników"));
		KaiStaticBoxSizer *audioCacheFilesLimitSizer = new KaiStaticBoxSizer(wxVERTICAL, AudioSecond, _("Limit plików audio cache"));
		KaiStaticBoxSizer *displayNonActiveLinesSizer = new KaiStaticBoxSizer(wxVERTICAL, AudioSecond, _("Sposób wyświetlania nieaktywnych linijek"));
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
			_("Okno tło"), _("Okno nieaktywne tło"), _("Okno tekst"), _("Okno nieaktywny tekst"),
			_("Okno obramowanie"), _("Okno obramowanie nieaktywne"), _("Okno tło obramowania"),
			_("Okno tło obramowania nieaktywne"), _("Okno tekst nagłówka"), _("Okno tekst nagłówka nieaktywny"),
			_("Okno najechany element nagłówka"), _("Okno wciśnięty element nagłówka"),
			_("Okno najechane zamykanie"), _("Okno wciśnięte zamykanie"), _("Okno elementy ostrzegające"),
			//subtitles
			_("Napisy tekst"), _("Napisy tło"), _("Napisy tło dialogów"), _("Napisy tło komentarzy"),
			_("Napisy zaznaczenia (przezroczystość)"), _("Napisy linijki widoczne na wideo"),
			_("Napisy kolidujące linie"), _("Napisy obramowanie linijki"), _("Napisy obramowanie aktywnej linijki"),
			_("Napisy nagłówek"), _("Napisy tekst nagłówka"),
			_("Napisy etykieta"), _("Napisy etykieta zmodyfikowanej linii"), _("Napisy etykieta zapisanej linii"),
			_("Napisy etykieta niepewnej linii"), _("Napisy tło błędów pisowni"), _("Napisy obramowanie porównania"),
			_("Napisy tło porównania brak zgodności"), _("Napisy tło porównania zgodność"), 
			_("Napisy tło komentarza por. brak zgodności"), _("Napisy tło komentarza por. zgodność"),
			//editor
			_("Edytor tekst"), _("Edytor nazwy tagów"), _("Edytor wartości tagów"),
			_("Edytor nawiasy klamrowe"), _("Edytor operatory tagów"), _("Edytor dzielenie linii i rysunki ASS"), _("Edytor zmienne template"),
			_("Edytor znaczniki kodu template"), _("Edytor funkcje template"), _("Edytor słowa kluczowe template"), 
			_("Edytor ciągi znaków template"), _("Edytor zaznaczenie znalezionej frazy"), _("Edytor tło nawiasów"), 
			_("Edytor tło"), _("Edytor zaznaczenie"), _("Edytor zaznaczenie w nieaktywnym oknie"),
			_("Edytor obramowanie"), _("Edytor obramowanie aktywnego okna"), _("Edytor błędy pisowni"),
			//audio
			_("Audio tło"), _("Audio znacznik start"), _("Audio znacznik koniec"), _("Audio znacznik przesuwania czasów"),
			_("Audio znaczniki nieaktywnej linijki"), _("Audio kursor"), _("Audio znaczniki sekund"), _("Audio klatki kluczowe"),
			_("Audio znaczniki sylab"), _("Audio tekst sylab"), _("Audio zaznaczenie"),
			_("Audio zaznaczenie po modyfikacji"), _("Audio tło nieaktywnych linijek"), _("Audio wykres falowy"),
			_("Audio nieaktywny wykres falowy"), _("Audio zmodyfikowany wykres falowy"), _("Audio zaznaczony wykres falowy"),
			_("Audio tło spektrum"), _("Audio echo spektrum"), _("Audio spektrum"),
			//controls
			_("Pole tekstowe tło"), _("Pole tekstowe obramowanie"),
			_("Pole tekstowe obramowanie aktywnego okna"), _("Pole tekstowe zaznaczenie"),
			_("Pole tekstowe zaznaczenie w nieaktywnym oknie"),
			_("Przycisk i lista tło"), _("Przycisk i lista tło po najechaniu"),
			_("Przycisk i lista tło po wciśnięciu"), _("Przycisk i lista tło aktywnego"),
			_("Przycisk i lista obramowanie"), _("Przycisk i lista obramowanie po najechaniu"),
			_("Przycisk i lista obramowanie po wciśnięciu"), _("Przycisk i lista obramowanie aktywnego"),
			_("Przycisk i lista obramowanie nieaktywne"), _("Przełącznik tło włączonego"),
			_("Przełącznik obramowanie włączonego"), _("Pasek przewijania tło"), _("Pasek przewijania suwak"),
			_("Pasek przewijania suwak po najechaniu"), _("Pasek przewijania suwak po wciśnięciu"),
			_("Ramka z opisem obramowanie"), _("Lista statyczna obramowanie"), _("Lista statyczna tło"),
			_("Lista statyczna zaznaczenie"), _("Lista statyczna tło nagłówka"), _("Lista statyczna tekst nagłówka"),
			_("Pasek statusu obramowanie"),
			//menu bar
			_("Pasek menu tło 1"), _("Pasek menu tło 2"), _("Pasek menu obramowanie zaznaczenia"), 
			_("Pasek menu najechane tło zaznaczenia"), _("Pasek menu kliknięte tło zaznaczenia"), 
			_("Menu tło"), _("Menu obramowanie zaznaczenia"), _("Menu tło zaznaczenia"),
			//tab bar
			_("Pasek zakładek tło 1"), _("Pasek zakładek tło 2"), _("Zakładki obramowanie aktywnej"),
			_("Zakładki obramowanie nieaktywnej"), _("Zakładki tło aktywnej"), _("Zakładki tło nieaktywnej"),
			_("Zakładki tło nieaktywnej po najechaniu"), _("Zakładki tło drugiej widocznej zakładki"),
			_("Zakładki tekst aktywnej"), _("Zakładki tekst nieaktywnej"), _("Zakładki zamknięcie po najechaniu"),
			_("Pasek zakładek strzałka"), _("Pasek zakładek strzałka tło"),
			_("Pasek zakładek strzałka tło po najechaniu"),
			//slider
			_("Suwak ścieżka tło"), _("Suwak ścieżka obramowanie"), _("Suwak obramowanie"),
			_("Suwak obramowanie po najechaniu"), _("Suwak obramowanie po wciśnięciu"), _("Suwak tło"),
			_("Suwak tło po najechaniu"), _("Suwak tło po wciśnięciu"),
			//miscellanous
			_("Linia zmiany rozdzielczości kropki"), _("Wynik szukania czcionka nazwy pliku"), _("Wynik szukania tło nazwy pliku"),
			_("Wynik szukania czcionka znalezionej frazy"), _("Wynik szukania tło znalezionej frazy"),
			//styles preview
			_("Pierwszy kolor podglądu stylów"), _("Drugi kolor podglądu stylów")
		};


		wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
		wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
		wxArrayString choices;
		wxArrayString files;
		wxString pathwn = Options.pathfull + L"\\Themes\\";
		const wxString & programTheme = Options.GetString(PROGRAM_THEME);
		wxDir kat(pathwn);
		if (kat.IsOpened()){
			kat.GetAllFiles(pathwn, &files, L"*.txt", wxDIR_FILES);
		}
		for (size_t i = 0; i < files.size(); i++){
			choices.Add(files[i].AfterLast(L'\\').BeforeLast(L'.'));
		}
		if (choices.Index(L"DarkSentro", false) == -1){
			choices.Insert(L"DarkSentro", 0);
		}
		if (choices.Index(L"LightSentro", false) == -1){
			choices.Insert(L"LightSentro", 1);
		}
		KaiChoice *themeList = new KaiChoice(Themes, 14567, wxDefaultPosition, wxDefaultSize, choices);
		themeList->SetSelection(themeList->FindString(programTheme));
		themeList->SetToolTip(_("Nazwa motywu:"));
		KaiTextCtrl *newTheme = new KaiTextCtrl(Themes, -1, emptyString);
		newTheme->SetToolTip(_("Nazwa kopiowanego motywu.\nMotywów domyślnych: DarkSentro i LightSentro\nnie można edytować, należy je skopiować."));
		MappedButton *copyTheme = new MappedButton(Themes, 14566, _("Kopiuj"));



		sizer->Add(themeList, 0, wxALL | wxEXPAND, 2);
		sizer1->Add(newTheme, 1, wxRIGHT | wxTOP | wxBOTTOM | wxEXPAND, 2);
		sizer1->Add(copyTheme, 0, wxLEFT | wxTOP | wxBOTTOM, 2);
		sizer->Add(sizer1, 0, wxALL | wxEXPAND, 2);

		KaiStaticText *warning = new KaiStaticText(Themes, -1, _("UWAGA! Przezroczystość działa tylko na wykresie audio,\npolu tekstowym i polu napisów."));
		sizer->Add(warning, 0, wxALL | wxEXPAND, 2);

		wxString mesureText = _("Pole tekstowe zaznaczenie w nieaktywnym oknie");
		wxString mesureText2 = L"######FFFFFFFF";
		int fw, fww, fh;
		GetTextExtent(mesureText, &fw, &fh);
		GetTextExtent(mesureText2, &fww, &fh);

		KaiListCtrl *List = new KaiListCtrl(Themes, -1, wxDefaultPosition, wxSize(300, -1));
		List->InsertColumn(0, _("Nazwa"), TYPE_TEXT, fw < 240 ? 240 : fw);
		List->InsertColumn(1, _("Kolor"), TYPE_COLOR, fww < 150 ? 150 : fww);
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
			wxString dir = Options.pathfull + L"\\Themes\\";
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
		Bind(wxEVT_COMMAND_CHOICE_SELECTED, [=](wxCommandEvent &evt){
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
			siz.x;
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
		wxString extensionsDesc[] = { _("Napisy ASS"), _("Napisy SSA"), _("Napisy SRT"), _("Napisy SUB"), 
			_("Napisy TXT"), _("Wideo MKV"), _("Wideo MP4"), _("Wideo AVI"), _("Wideo OGM"),
			_("Wideo WMV"), _("Wideo ASF"), _("Wideo RMVB"), _("Wideo RM"), _("Wideo 3GP"), 
			_("Wideo MPG"), _("Wideo MPEG"), _("Wideo TS"), _("Wideo M2TS") };
		int numExtensions = 18;

		Registry::CheckFileAssociation(extensions, numExtensions, registeredExts);

		wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
		KaiStaticText *warning = new KaiStaticText(Assocs, -1, 
			_("UWAGA! Działanie na każdym Windowsie jest inne.\nPrzykładowo na Windows7 WMP zablokował\nmożliwość zmiany skojarzeń wideo."));
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
		wxString buttonTexts[] = { _("Zaznacz wszystko"), _("Zaznacz napisy"), _("Zaznacz wideo"), _("Odznacz wszystko") };
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
	OptionsTree->AddPage(GLOBAL_EDITOR, _("Edytor"));
	//After adding page before convert change it in function Convert in SubsGridBase.cpp!!!
	OptionsTree->AddSubPage(ConvOpt, _("Konwersja"));
	OptionsTree->AddSubPage(EditorAdvanced, _("Zaawansowane"));
	OptionsTree->AddPage(video, _("Wideo"));
	OptionsTree->AddPage(AudioMain, _("Audio"));
	OptionsTree->AddSubPage(AudioSecond, _("Zaawansowane"));
	OptionsTree->AddPage(Themes, _("Motywy"));
	OptionsTree->AddPage(Hotkeyss, _("Skróty klawiszowe"));
	OptionsTree->AddPage(Assocs, _("Skojarzenia"));
	OptionsTree->AddPage(SubsProps, _("Właściwości napisów"));
	OptionsTree->Fit();

	//adding buttons
	wxBoxSizer *ButtonsSizer = new wxBoxSizer(wxHORIZONTAL);

	okok = new MappedButton(this, wxID_OK, L"OK");
	MappedButton *oknow = new MappedButton(this, ID_BCOMMIT, _("Zastosuj"));
	MappedButton *cancel = new MappedButton(this, wxID_CANCEL, _("Anuluj"));
	MappedButton *resetDefaults = new MappedButton(this, ID_RESET_DEFAULTS, _("Ustaw domyślne"));

	ButtonsSizer->Add(okok, 1, wxRIGHT, 2);
	ButtonsSizer->Add(oknow, 1, wxRIGHT, 2);
	ButtonsSizer->Add(cancel, 1, wxRIGHT, 2);
	ButtonsSizer->Add(resetDefaults, 1, wxRIGHT, 2);

	DialogSizer *TreeSizer = new DialogSizer(wxVERTICAL);
	TreeSizer->Add(OptionsTree, 0, wxALL, 2);
	TreeSizer->Add(ButtonsSizer, 0, wxBOTTOM | wxALIGN_CENTER, 4);
	SetSizerAndFit(TreeSizer);

	CenterOnParent();

	Connect(wxID_OK, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&OptionsDialog::OnSaveClick);
	Connect(ID_BCOMMIT, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&OptionsDialog::OnSaveClick);
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
				if (OB.option <= AUDIO_WHEEL_DEFAULT_TO_ZOOM){ audio = true; }
				if (OB.option == SPELLCHECKER_ON){
					Notebook::GetTab()->edit->ClearErrs(true, value);
				}
			}
		}
		else if (OB.ctrl->IsKindOf(CLASSINFO(FontPickerButton))){
			FontPickerButton *fpc = (FontPickerButton*)OB.ctrl;
			wxFont font = fpc->GetSelectedFont();
			wxString fontname = font.GetFaceName();
			int fontsize = font.GetPointSize();
			if (Options.GetString(OB.option) != fontname){
				Options.SetString(OB.option, fontname); 
				fontmod = true;
			}
			CONFIG fontSizeOption = (OB.option == GRID_FONT) ? GRID_FONT_SIZE : PROGRAM_FONT_SIZE;
			if (Options.GetInt(fontSizeOption) != fontsize){
				Options.SetInt(fontSizeOption, fontsize); 
				fontmod = true;
			}
			if (OB.option == PROGRAM_FONT && fontmod){
				Options.FontsClear();
				KainoteFrame* Kai =
					/*wxDynamicCast<*/(KainoteFrame*)Notebook::GetTabs()->GetParent();//>
				Kai->SetAccels();
				Kai->DestroyDialogs();
				Kai->SetFont(*Options.GetFont());
				Kai->Layout();
			}
		}
		else if (OB.ctrl->IsKindOf(CLASSINFO(KaiChoice))){
			KaiChoice *cbx = (KaiChoice*)OB.ctrl;
			if (cbx->GetWindowStyle() & KAI_COMBO_BOX){
				wxString color = cbx->GetValue();
				if (Options.GetString(OB.option) != color){
					Options.SetString(OB.option, color);
				}
			}
			else if (cbx->GetId() != ID_KAI_CHOICE){
				//dictionary language
				if (cbx->GetId() == ID_DICTIONARY_LANGUAGE){
					int sel = cbx->GetSelection();
					if (sel >= 0 && sel < dictionaryLanguagesSymbols.GetCount()){
						wxString language = dictionaryLanguagesSymbols[sel];
						if (Options.GetString(OB.option) != language){
							Options.SetString(OB.option, language);
							SpellChecker::Destroy();
							Notebook::GetTab()->edit->ClearErrs();
						}
					}
				}//program language
				else if (cbx->GetId() == ID_PROGRAM_LANGUAGE){
					int sel = cbx->GetSelection();
					if (sel >= 0 && sel < programLanguages.size()){
						wxString language = programLanguages[sel];
						if (Options.GetString(OB.option) != language)
							Options.SetString(OB.option, language);
					}
				}
				else{
					wxString option = cbx->GetString(cbx->GetSelection());
					if (Options.GetString(OB.option) != option){
						Options.SetString(OB.option, option);
						//vsfilter change
						if (cbx->GetId() == ID_VSFILTER_PROVIDER){
							SubtitlesProviderManager::DestroyProviders();
							Notebook::RefreshVideo();
						}
					}
				}
			}
			else{
				if (Options.GetInt(OB.option) != cbx->GetSelection()){
					Options.SetInt(OB.option, cbx->GetSelection());
				}
			}
			if (OB.option <= AUDIO_WHEEL_DEFAULT_TO_ZOOM){ audio = true; }
		}
		else if (OB.ctrl->IsKindOf(CLASSINFO(KaiTextCtrl))){

			if (OB.ctrl->GetId() != ID_NUMBER_CONTROL){
				KaiTextCtrl *sc = (KaiTextCtrl*)OB.ctrl;
				wxString str = sc->GetValue();
				if (Options.GetString(OB.option) != str){
					Options.SetString(OB.option, str);
					if (OB.option == GRID_TAGS_SWAP_CHARACTER){
						Notebook* tabs = Notebook::GetTabs();
						for (size_t i = 0; i < tabs->Size(); i++) {
							TabPanel *page = tabs->Page(i);
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

				if (OB.option == ID_COLOR_CONFIG){
					list->SaveAll(1);
					Options.SaveColors();
					ChangeColors();
				}
				else if (OB.option == ID_HOTKEYS_CONFIG){
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
					wxString extensions[] = { L".ass", L".ssa", L".srt", L".sub", L".txt", L".mkv", L".mp4", L".avi", 
						L".ogm", L".wmv", L".asf", L".rmvb", L".rm", L".3gp", L".mpg", L".mpeg", L".ts", L".m2ts" };
					wxString extensionsDesc[] = { _("Napisy ASS"), _("Napisy SSA"), _("Napisy SRT"), _("Napisy SUB"),
						_("Napisy TXT"), _("Wideo MKV"), _("Wideo MP4"), _("Wideo AVI"), _("Wideo OGM"),
						_("Wideo WMV"), _("Wideo ASF"), _("Wideo RMVB"), _("Wideo RM"), _("Wideo 3GP"),
						_("Wideo MPG"), _("Wideo MPEG"), _("Wideo TS"), _("Wideo M2TS") };

					for (size_t i = 0; i < registeredExts.size(); i++){
						if (list->GetItem(i, 0)->modified != registeredExts[i]){
							if (registeredExts[i])
								Registry::RemoveFileAssociation(extensions[i]);
							else
								Registry::AddFileAssociation(extensions[i], extensionsDesc[i], i);
						}
					}
					//Registry::RefreshRegistry();
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
