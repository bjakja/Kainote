//  Copyright (c) 2021, Marcin Drob

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

#include "VisualAllTagsEdition.h"
#include "KaiMessageBox.h"
#include "OpennWrite.h"
#include "config.h"

AllTagsEdition::AllTagsEdition(wxWindow* parent, const wxPoint& pos,
	std::vector<AllTagsSetting>* _tags, int curTag)
	:KaiDialog(parent, -1, _("Edycja tagów"), pos)
{
	if (curTag < 0 || curTag >= _tags->size())
		curTag = 0;

	selection = curTag;
	tags = std::vector<AllTagsSetting>(*_tags);
	currentTag = tags[selection];
	wxArrayString list;
	GetNames(&tags, &list);
	DialogSizer *main = new DialogSizer(wxVERTICAL);
	KaiStaticBoxSizer* tagSizer = new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Edytowany tag"));
	tagList = new KaiChoice(this, ID_TAG_LIST, wxDefaultPosition, wxDefaultSize, list);
	tagList->SetSelection(selection);
	Bind(wxEVT_COMMAND_CHOICE_SELECTED, &AllTagsEdition::OnListChanged, this, ID_TAG_LIST);
	newTagName = new KaiTextCtrl(this, -1);
	MappedButton* addTag = new MappedButton(this, ID_BUTTON_ADD_TAG, _("Dodaj tag"));
	MappedButton* removeTag = new MappedButton(this, ID_BUTTON_REMOVE_TAG, _("Usuń tag"));
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AllTagsEdition::OnAddTag, this, ID_BUTTON_ADD_TAG);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AllTagsEdition::OnRemoveTag, this, ID_BUTTON_REMOVE_TAG);
	tagSizer->Add(tagList, 1, wxALL | wxEXPAND, 4);
	tagSizer->Add(newTagName, 1, wxALL | wxEXPAND, 4);
	tagSizer->Add(addTag, 1, wxALL | wxEXPAND, 4);
	tagSizer->Add(removeTag, 1, wxALL | wxEXPAND, 4);
	KaiStaticBoxSizer* editionSizer = new KaiStaticBoxSizer(wxVERTICAL, this, _("Edycja"));
	wxBoxSizer* nameTagSizer = new wxBoxSizer(wxHORIZONTAL);
	tagName = new KaiTextCtrl(this, -1, currentTag.name);
	tagWithoutSlash = new KaiTextCtrl(this, -1, currentTag.tag);
	nameTagSizer->Add(new KaiStaticText(this, -1, _("Nazwa:")), 1, wxALL | wxEXPAND, 4);
	nameTagSizer->Add(tagName, 1, wxALL | wxEXPAND, 4);
	nameTagSizer->Add(new KaiStaticText(this, -1, _("Tag:")), 1, wxALL | wxEXPAND, 4);
	nameTagSizer->Add(tagWithoutSlash, 1, wxALL | wxEXPAND, 4);
	wxBoxSizer* minMaxSizer = new wxBoxSizer(wxHORIZONTAL);
	minValue = new NumCtrl(this, -1, currentTag.rangeMin, -10000, 10000, false);
	maxValue = new NumCtrl(this, -1, currentTag.rangeMax, -10000, 10000, false);
	minMaxSizer->Add(new KaiStaticText(this, -1, _("Minimalna wartość:")), 1, wxALL | wxEXPAND, 4);
	minMaxSizer->Add(minValue, 1, wxALL | wxEXPAND, 4);
	minMaxSizer->Add(new KaiStaticText(this, -1, _("Maksymalna wartość:")), 1, wxALL | wxEXPAND, 4);
	minMaxSizer->Add(maxValue, 1, wxALL | wxEXPAND, 4);
	wxBoxSizer* valStepSizer = new wxBoxSizer(wxHORIZONTAL);
	values[0] = new NumCtrl(this, -1, currentTag.values[0], -10000, 10000, false);
	step = new NumCtrl(this, -1, currentTag.step, -10000, 10000, false);
	valStepSizer->Add(new KaiStaticText(this, -1, _("Wartość:")), 1, wxALL | wxEXPAND, 4);
	valStepSizer->Add(values[0], 1, wxALL | wxEXPAND, 4);
	valStepSizer->Add(new KaiStaticText(this, -1, _("Przeskok:")), 1, wxALL | wxEXPAND, 4);
	valStepSizer->Add(step, 1, wxALL | wxEXPAND, 4);
	wxBoxSizer* modesSizer = new wxBoxSizer(wxHORIZONTAL);
	wxString modes[] = { _("Wstawiany w miejscu kursora"), _("Wstawiany tylko na początku"), 
		_("Wycinek prostokątny z obsługą animacji")};
	mode = new KaiChoice(this, -1, wxDefaultPosition, wxDefaultSize, 3, modes);
	mode->SetSelection(currentTag.mode);
	digitAfterDot = new NumCtrl(this, -1, L"1", 0, 6, true);
	digitAfterDot->SetInt(currentTag.digitsAfterDot);
	modesSizer->Add(mode, 2, wxALL | wxEXPAND, 4);
	modesSizer->Add(new KaiStaticText(this, -1, _("Liczby po przecinku:")), 1, wxALL | wxEXPAND, 4);
	modesSizer->Add(digitAfterDot, 1, wxALL | wxEXPAND, 4);

	wxBoxSizer* valuesSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* valuesAndInsertModeSizer = new wxBoxSizer(wxHORIZONTAL);
	wxString insertModes[6] = { _("Dodaj"), _("Wstaw"), _("Pomnóż"), _("Pomnóż+"), _("Gradient tekst"), _("Gradient linia") };
	wxString valuesStr[4] = { _("Brak dodatkowych wartości"), 
		_("jedna dodatkowa wartość"), 
		_("dwie dodatkowe wartości"), 
		_("trzy dodatkowe wartości") };
	numOfValues = new KaiChoice(this, ID_ADDITIONAL_VALUES_LIST, wxDefaultPosition, wxDefaultSize, 4, valuesStr);
	numOfValues->SetToolTip(_("Używane tylko w przypadku gdy tag ma 2 wartości bądź więcej"));
	numOfValues->SetSelection(currentTag.numOfValues);
	tagInsertMode = new KaiChoice(this, ID_INSERT_MODES_LIST, wxDefaultPosition, wxDefaultSize, 6, insertModes);
	tagInsertMode->SetToolTip(_("Opcje zmiany tagów"));
	tagInsertMode->SetSelection(currentTag.tagMode);
	for (int i = 0; i < 4; i++) {
		values[i] = new NumCtrl(this, -1, currentTag.values[i], -10000, 10000, false);
		values[i]->SetToolTip(wxString::Format(_("Wartość %i"), i + 2));
		values[i]->Enable(currentTag.numOfValues > i);
	}

	Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, [=](wxCommandEvent& evt) {
		int numAdditionalValues = numOfValues->GetSelection();
		for (int i = 1; i < 4; i++) {
			values[i]->Enable(numAdditionalValues >= i);
		}
		}, ID_ADDITIONAL_VALUES_LIST);
	valuesSizer->Add(new KaiStaticText(this, -1, _("Dodatkowe wartości:")), 1, wxALL | wxEXPAND, 4);
	for (int i = 0; i < 4; i++) {
		valuesSizer->Add(values[i], 1, wxALL | wxEXPAND, 4);
	}

	editionSizer->Add(nameTagSizer, 0, wxEXPAND, 0);
	editionSizer->Add(minMaxSizer, 0, wxEXPAND, 0);
	editionSizer->Add(valStepSizer, 0, wxEXPAND, 0);
	editionSizer->Add(modesSizer, 0, wxEXPAND, 0);
	editionSizer->Add(valuesSizer, 0, wxEXPAND, 0);

	wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	MappedButton* commit = new MappedButton(this, ID_BUTTON_COMMIT, _("Zastosuj"));
	MappedButton* OK = new MappedButton(this, ID_BUTTON_OK, L"OK");
	MappedButton* cancel = new MappedButton(this, wxID_CANCEL, _("Anuluj"));
	MappedButton* resetDefault = new MappedButton(this, ID_BUTTON_RESET_DEFAULT, _("Przywróć domyślne"));
	buttonSizer->Add(commit, 1, wxALL, 4);
	buttonSizer->Add(OK, 1, wxALL, 4);
	buttonSizer->Add(cancel, 1, wxALL, 4);
	buttonSizer->Add(resetDefault, 1, wxALL, 4);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AllTagsEdition::OnSave, this, ID_BUTTON_OK, ID_BUTTON_COMMIT);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AllTagsEdition::OnResetDefault, this, ID_BUTTON_RESET_DEFAULT);
	main->Add(tagSizer, 0, wxALL | wxEXPAND, 2);
	main->Add(editionSizer, 0, wxALL | wxEXPAND, 2);
	main->Add(buttonSizer, 0, wxALL | wxEXPAND, 2);
	SetSizerAndFit(main);
	CenterOnParent();
	SetEnterId(ID_BUTTON_OK);
}

void AllTagsEdition::OnSave(wxCommandEvent& evt)
{
	Save(evt.GetId());
}

void AllTagsEdition::OnResetDefault(wxCommandEvent& evt)
{
	if (KaiMessageBox(_("Czy na pewno chcesz przywrócić ustawienia domyślne?"), 
		_("Potwierdzenie"), wxYES_NO, this) == wxYES){
		wxString path = Options.pathfull + L"\\Config\\AllTagsSettings.txt";
		_wremove(path.wc_str());
		tags.clear();
		LoadSettings(&tags);
		wxArrayString names;
		GetNames(&tags, &names);
		tagList->PutArray(&names);
		SetTag(tagList->GetSelection());
	}
}

void AllTagsEdition::OnAddTag(wxCommandEvent& evt)
{
	wxString newTagNameStr = newTagName->GetValue();
	if (newTagNameStr.empty()) {
		KaiMessageBox(_("Wpisz nazwę nowego tagu."), _("Błąd"), wxOK, this);
		return;
	}
	if (tagList->FindString(newTagNameStr) != -1) {
		KaiMessageBox(_("Nowy tag już istnieje na liście, wpisz inną nazwę."), _("Błąd"), wxOK, this);
		return;
	}
	currentTag = AllTagsSetting(newTagNameStr);
	tags.push_back(currentTag);
	tagList->Append(currentTag.name);
	selection = tags.size() - 1;
	tagList->SetSelection(selection);
	SetTagFromSettings();
}

void AllTagsEdition::OnRemoveTag(wxCommandEvent& evt)
{
	if (selection < 0 || selection >= tags.size()) {
		KaiMessageBox(L"Selected tag is out of range of tagList.", L"Error", wxOK, this);
		return;
	}
	if (tags.size() <= 1) {
		KaiMessageBox(_("Nie można usunąć wszystkich tagów z listy"), _("Błąd"), wxOK, this);
		return;
	}
	tags.erase(tags.begin() + selection);
	tagList->Delete(selection);
	if (selection >= tagList->GetCount()) {
		tagList->SetSelection(tagList->GetCount() - 1);
	}
	else
		tagList->SetSelection(selection);

	selection = tagList->GetSelection();
	//list is empty
	if (selection < 0) {
		currentTag = AllTagsSetting();
	}
	else {
		currentTag = tags[selection];
	}
	SetTagFromSettings();
}

void AllTagsEdition::OnListChanged(wxCommandEvent& evt)
{
	if (CheckModified()) {
		if (KaiMessageBox(wxString::Format(_("Zapisać zmiany tagu \"%s\"?"),
			currentTag.tag), _("Potwierdzenie"), wxYES_NO, this) == wxYES) {
			Save(ID_BUTTON_COMMIT);
		}
	}
	SetTag(tagList->GetSelection());
}

void AllTagsEdition::UpdateTag()
{
	currentTag.name = tagName->GetValue();
	currentTag.tag = tagWithoutSlash->GetValue();
	currentTag.rangeMin = (float)minValue->GetDouble();
	currentTag.rangeMax = (float)maxValue->GetDouble();
	currentTag.step = (float)step->GetDouble();
	currentTag.mode = mode->GetSelection();
	currentTag.digitsAfterDot = digitAfterDot->GetInt();
	currentTag.numOfValues = numOfValues->GetSelection();
	numOfValues->SetSelection(currentTag.numOfValues - 1);
	for (int i = 0; i <= currentTag.numOfValues && values[i]; i++) {
		currentTag.values[i] = (float)values[i]->GetDouble();
	}

}

void AllTagsEdition::SetTagFromSettings()
{
	tagName->SetValue(currentTag.name);
	tagWithoutSlash->SetValue(currentTag.tag);
	minValue->SetDouble(currentTag.rangeMin);
	maxValue->SetDouble(currentTag.rangeMax);
	step->SetDouble(currentTag.step);
	mode->SetSelection(currentTag.mode);
	numOfValues->SetSelection(currentTag.numOfValues - 1);
	for (int i = 0; i <= currentTag.numOfValues && values[i]; i++) {
		values[i]->SetDouble(currentTag.values[i]);
	}
}

void AllTagsEdition::SetTag(int num)
{
	if (num < 0 || num >= tags.size())
		num = 0;

	currentTag = tags[num];
	selection = num;
	SetTagFromSettings();

}

bool AllTagsEdition::CheckModified()
{
	if (currentTag.name != tagName->GetValue() ||
		currentTag.tag != tagWithoutSlash->GetValue() ||
		currentTag.rangeMin != (float)minValue->GetDouble() ||
		currentTag.rangeMax != (float)maxValue->GetDouble() ||
		currentTag.step != (float)step->GetDouble() ||
		currentTag.mode != mode->GetSelection() ||
		currentTag.digitsAfterDot != digitAfterDot->GetInt() ||
		currentTag.numOfValues != numOfValues->GetSelection())
	{
		return true;
	}
	for (int i = 0; i <= currentTag.numOfValues; i++) {
		if (values[i] && 
			currentTag.values[i] != (float)values[i]->GetDouble()) {
			return true;
		}
	}
	return false;
}

void AllTagsEdition::Save(int id)
{
	UpdateTag();
	if (currentTag.tag.empty()) {
		KaiMessageBox(_("Pole \"Tag\" nie może być puste."), _("Błąd"), wxOK, this);
		return;
	}
	if (currentTag.name.empty()) {
		currentTag.name = currentTag.tag;
		tagName->SetValue(currentTag.name);
	}
	if (currentTag.rangeMax <= currentTag.rangeMin) {
		KaiMessageBox(_("Pole \"Maksymalna wartość\" musi zawierać wartość\nwiększą od pola \"Minimalna wartość\"."), _("Błąd"), wxOK, this);
		return;
	}
	if (((currentTag.rangeMax - currentTag.rangeMin) / currentTag.step) < 2) {
		KaiMessageBox(_("Pole \"Przeskok\" zawiera liczbę zbyt wysoką dla danego przedziału."), _("Błąd"), wxOK, this);
		return;
	}
	if (selection < 0 || selection >= tags.size()) {
		KaiMessageBox(L"Selected tag is out of range of tagList.", L"Error", wxOK, this);
		return;
	}
	tags[selection] = currentTag;

	if (id == ID_BUTTON_OK) {
		SaveSettings(&tags);
		EndModal(wxID_OK);
	}
}

void LoadSettings(std::vector<AllTagsSetting>* tags)
{
	wxString path = Options.pathfull + L"\\Config\\AllTagsSettings.txt";
	OpenWrite ow;
	wxString txtSettings;
	//go inside when used pravious version of hydra
	if (!ow.FileOpen(path, &txtSettings, false) || !txtSettings.StartsWith(L"HYDRA2.0")) {
		//write entire setings in plain text
		//Tag: name, tag, min, max, value, step, num digits after dot, paste mode, tag mode, [valuey], [valuex1],[valuex1]
		txtSettings = L"HYDRA2.0"
					L"Tag: blur, blur,       0, 100,  0,  0.5,  1, 0, 1\n"\
					L"Tag: border, bord,     0, 50,   0,  1,    1, 0, 0\n"\
					L"Tag: blur edge, be,    0, 100,  0,  1,    1, 0, 1\n"\
					L"Tag: fading, fad,      0, 2000, 0,  5,    0, 1, 1, 0\n"\
					L"Tag: fax, fax,       -10, 10,   0,  0.01, 3, 0, 0\n"\
					L"Tag: fay, fay,       -10, 10,   0,  0.01, 3, 0, 0\n"\
					L"Tag: font size, fs,   20, 300,  70, 1,    0, 0, 0\n"\
					L"Tag: spacing, fsp,  -100, 100,  0,  1,    1, 0, 1\n"\
					L"Tag: shadow, shad,     0, 80,   0,  1,    1, 0, 0\n"\
					L"Tag: xborder, xbord,   0, 80,   0,  1,    1, 0, 0\n"\
					L"Tag: yborder, ybord,   0, 80,   0,  1,    1, 0, 0\n"\
					L"Tag: xshadow, xshad, -80, 80,   0,  1,    1, 0, 0\n"\
					L"Tag: yshadow, yshad, -80, 80,   0,  1,    1, 0, 0\n"\
					L"Tag: position, pos,    0, 100,  0,  1,    1, 1, 2, 0\n";
	}
	wxStringTokenizer tokenzer(txtSettings, "\n", wxTOKEN_STRTOK);
	tokenzer.GetNextToken();
	// no need to check version of hydra again just skip first token
	while (tokenzer.HasMoreTokens()) {
		wxString token = tokenzer.GetNextToken();
		wxString tagtxt;
		if (token.StartsWith(L"Tag: ", &tagtxt)) {
			wxStringTokenizer tkzer(tagtxt, ",", wxTOKEN_STRTOK);
			AllTagsSetting tmp;
			tmp.name = tkzer.GetNextToken();
			if (!tkzer.HasMoreTokens())
				continue;
			tmp.tag = tkzer.GetNextToken().Trim(false);
			double tmpval = 0;
			if (!tkzer.HasMoreTokens())
				continue;
			if (!tkzer.GetNextToken().Trim(false).ToCDouble(&tmpval))
				continue;
			tmp.rangeMin = tmpval;
			if (!tkzer.HasMoreTokens())
				continue;
			if (!tkzer.GetNextToken().Trim(false).ToCDouble(&tmpval))
				continue;
			tmp.rangeMax = tmpval;
			if (!tkzer.HasMoreTokens())
				continue;
			if (!tkzer.GetNextToken().Trim(false).ToCDouble(&tmpval))
				continue;
			tmp.values[0] = tmpval;
			if (!tkzer.HasMoreTokens())
				continue;
			if (!tkzer.GetNextToken().Trim(false).ToCDouble(&tmpval))
				continue;
			tmp.step = tmpval;
			if (!tkzer.HasMoreTokens())
				continue;
			tmp.digitsAfterDot = wxAtoi(tkzer.GetNextToken().Trim(false));
			if (!tkzer.HasMoreTokens())
				continue;
			tmp.mode = wxAtoi(tkzer.GetNextToken().Trim(false));
			if (!tkzer.HasMoreTokens())
				continue;
			tmp.tagMode = wxAtoi(tkzer.GetNextToken().Trim(false));
			for (int i = 1; tkzer.HasMoreTokens(); i++) {
				if (tkzer.GetNextToken().Trim(false).ToCDouble(&tmpval)) {
					tmp.values[i] = tmpval;
					tmp.numOfValues = i;
				}
			}
			tags->push_back(tmp);
		}
	}
}

void GetNames(std::vector<AllTagsSetting>* tags, wxArrayString* nameList)
{
	for (size_t i = 0; i < tags->size(); i++) {
		AllTagsSetting tag = (*tags)[i];
		nameList->Add(tag.name);
	}
}

void SaveSettings(std::vector<AllTagsSetting>* tags)
{
	wxString path = Options.pathfull + L"\\Config\\AllTagsSettings.txt";
	OpenWrite ow(path);
	for (size_t i = 0; i < tags->size(); i++) {
		AllTagsSetting tag = (*tags)[i];
		wxString tagText = L"Tag: ";
		tagText << tag.name << ", " << tag.tag << ", " << tag.rangeMin << ", " <<
			tag.rangeMax << ", " << tag.values[0] << ", " << tag.step << ", " <<
			(int)tag.digitsAfterDot << ", " << (int)tag.mode;
		if (tag.numOfValues) {
			for (int i = 1; i <= tag.numOfValues; i++) {
				tagText << ", " << tag.values[i];
			}
		}
		tagText << "\n";
		ow.PartFileWrite(tagText);
	}
}


