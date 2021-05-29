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

	tags = std::vector<AllTagsSetting>(*_tags);
	currentTag = tags[curTag];
	wxArrayString list;
	GetNames(&tags, &list);
	DialogSizer *main = new DialogSizer(wxVERTICAL);
	KaiStaticBoxSizer* tagSizer = new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Edytowany tag"));
	tagList = new KaiChoice(this, ID_TAG_LIST, wxDefaultPosition, wxDefaultSize, list);
	tagList->SetSelection(curTag);
	Bind(wxEVT_COMMAND_CHOICE_SELECTED, &AllTagsEdition::OnListChanged, this, ID_BUTTON_ADD_TAG);
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
	value = new NumCtrl(this, -1, currentTag.value, -10000, 10000, false);
	step = new NumCtrl(this, -1, currentTag.step, -10000, 10000, false);
	valStepSizer->Add(new KaiStaticText(this, -1, _("Wartość:")), 1, wxALL | wxEXPAND, 4);
	valStepSizer->Add(value, 1, wxALL | wxEXPAND, 4);
	valStepSizer->Add(new KaiStaticText(this, -1, _("Przeskok:")), 1, wxALL | wxEXPAND, 4);
	valStepSizer->Add(step, 1, wxALL | wxEXPAND, 4);
	wxBoxSizer* modeVal2Sizer = new wxBoxSizer(wxHORIZONTAL);
	wxString modes[] = { _("Wstawiany w każdym miejscu"), _("Wstawiany tylko na początku"), 
		_("Wycinek prostokątny z obsługą animacji")};
	mode = new KaiChoice(this, -1, wxDefaultPosition, wxDefaultSize, 3, modes);
	mode->SetSelection(currentTag.mode);
	value2 = new NumCtrl(this, -1, currentTag.value2, -10000, 10000, false);
	value2->SetToolTip(_("Używane tylko w przypadku gdy tag ma 2 wartości bądź więcej"));
	modeVal2Sizer->Add(mode, 2, wxALL | wxEXPAND, 4);
	modeVal2Sizer->Add(new KaiStaticText(this, -1, _("Wartość 2:")), 1, wxALL | wxEXPAND, 4);
	modeVal2Sizer->Add(value2, 1, wxALL | wxEXPAND, 4);
	editionSizer->Add(nameTagSizer, 0, wxEXPAND, 0);
	editionSizer->Add(minMaxSizer, 0, wxEXPAND, 0);
	editionSizer->Add(valStepSizer, 0, wxEXPAND, 0);
	editionSizer->Add(modeVal2Sizer, 0, wxEXPAND, 0);
	wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	MappedButton* commit = new MappedButton(this, ID_BUTTON_COMMIT, _("Zastosuj"));
	MappedButton* OK = new MappedButton(this, ID_BUTTON_OK, L"OK");
	MappedButton* cancel = new MappedButton(this, wxID_CANCEL, _("Anuluj"));
	buttonSizer->Add(commit, 1, wxALL, 4);
	buttonSizer->Add(OK, 1, wxALL, 4);
	buttonSizer->Add(cancel, 1, wxALL, 4);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AllTagsEdition::OnSave, this, ID_BUTTON_OK, ID_BUTTON_COMMIT);
	main->Add(tagSizer, 0, wxALL | wxEXPAND, 2);
	main->Add(editionSizer, 0, wxALL | wxEXPAND, 2);
	main->Add(buttonSizer, 0, wxALL | wxEXPAND, 2);
	SetSizerAndFit(main);
	CenterOnParent();
}

void AllTagsEdition::OnSave(wxCommandEvent& evt)
{
	int id = GetId();
	UpdateTag();
	if (currentTag.tag.empty()) {
		KaiMessageBox(_("Pole \"Tag\" nie może być puste."), _("Błąd"), wxOK, this);
		return;
	}
	if (currentTag.name.empty()) {
		currentTag.name = currentTag.tag;
		tagName->SetValue(currentTag.name);
	}
	if (currentTag.rangeMax - currentTag.rangeMin <= 0) {
		KaiMessageBox(_("Pole \"Maksymalna wartość\" musi zawierać wartość\nwiększą od pola \"Minimalna wartość\"."), _("Błąd"), wxOK, this);
		return;
	}
	if ((currentTag.rangeMax - currentTag.rangeMin) / currentTag.step > 2) {
		KaiMessageBox(_("Pole \"Przeskok\" zawiera liczbę zbyt wysoką dla danego przedziału."), _("Błąd"), wxOK, this);
		return;
	}
	int sel = tagList->GetSelection();
	if (sel < 0 || sel >= tags.size()) {
		KaiMessageBox(L"Selected tag is out of range of tagList.", L"Error", wxOK, this);
		return;
	}
	tags[sel] = currentTag;

	if (id == ID_BUTTON_OK) {
		SaveSettings(&tags);
		EndModal(wxID_OK);
	}
}

void AllTagsEdition::OnAddTag(wxCommandEvent& evt)
{
	wxString newTagNameStr = newTagName->GetValue();
	if (newTagNameStr.empty()) {
		KaiMessageBox(_("Wpisz nazwę nowego tagu."), _("Błąd"), wxOK, this);
		return;
	}
	currentTag = AllTagsSetting(newTagNameStr);
	tags.push_back(currentTag);
	tagList->Append(currentTag.name);
	SetTagFromSettings();
}

void AllTagsEdition::OnRemoveTag(wxCommandEvent& evt)
{
	int sel = tagList->GetSelection();
	if (sel < 0 || sel >= tags.size()) {
		KaiMessageBox(L"Selected tag is out of range of tagList.", L"Error", wxOK, this);
		return;
	}
	if (tags.size() <= 1) {
		KaiMessageBox(_("Nie można usunąć wszystkich tagów z listy"), _("Błąd"), wxOK, this);
		return;
	}
	tags.erase(tags.begin() + sel);
	tagList->Delete(sel);
	if (sel >= tagList->GetCount()) {
		tagList->SetSelection(tagList->GetCount() - 1);
	}
	else
		tagList->Refresh(false);

	sel = tagList->GetSelection();
	//list is empty
	if (sel < 0) {
		currentTag = AllTagsSetting();
	}
	else {
		currentTag = tags[sel];
	}
	SetTagFromSettings();
}

void AllTagsEdition::OnListChanged(wxCommandEvent& evt)
{
	int sel = tagList->GetSelection();
	SetTag(sel);
}

void AllTagsEdition::UpdateTag()
{
	currentTag.name = tagName->GetValue();
	currentTag.tag = tagWithoutSlash->GetValue();
	currentTag.rangeMin = (float)minValue->GetDouble();
	currentTag.rangeMax = (float)maxValue->GetDouble();
	currentTag.value = (float)value->GetDouble();
	currentTag.step = (float)step->GetDouble();
	currentTag.mode = mode->GetSelection();
	currentTag.value2 = (float)value2->GetDouble();
	if (currentTag.value2 || currentTag.name == L"fad" || currentTag.name == L"pos" ||
		currentTag.name == L"move" || currentTag.name == L"clip" || currentTag.name == L"iclip") {
		currentTag.has2value = true;
	}
}

void AllTagsEdition::SetTagFromSettings()
{
	tagName->SetValue(currentTag.name);
	tagWithoutSlash->SetValue(currentTag.tag);
	minValue->SetDouble(currentTag.rangeMin);
	maxValue->SetDouble(currentTag.rangeMax);
	value->SetDouble(currentTag.value);
	step->SetDouble(currentTag.step);
	mode->SetSelection(currentTag.mode);
	if (currentTag.has2value)
		value2->SetDouble(currentTag.value2);
	else
		value2->SetDouble(0);
}

void AllTagsEdition::SetTag(int num)
{
	if (num < 0 || num >= tags.size())
		num = 0;

	currentTag = tags[num];
	
	SetTagFromSettings();

}

void LoadSettings(std::vector<AllTagsSetting>* tags)
{
	wxString path = Options.pathfull + L"\\Config\\AllTagsSettings.txt";
	OpenWrite ow;
	wxString txtSettings;
	if (!ow.FileOpen(path, &txtSettings, false)) {
		//write entire setings in plain text
		//Tag: name, tag, min, max, value, step, mode, [valuey]
		txtSettings = L"Tag: blur, blur, 0, 200, 0, 0.5, 0\n"\
			L"Tag: border, bord, 0, 50, 0, 1, 0\n"\
			L"Tag: blur edge, be, 0, 100, 0, 1, 0\n"\
			L"Tag: fading, fad, 0, 2000, 0, 5, 1, 0\n"\
			L"Tag: fax, fax, -10, 10, 0, 0.05, 0\n"\
			L"Tag: fay, fay, -10, 10, 0, 0.05, 0\n"\
			L"Tag: font size, fs, 0, 300, 70, 1, 0\n"\
			L"Tag: spacing, fsp, -100, 100, 0, 1, 0\n"\
			L"Tag: shadow, shad, 0, 80, 0, 1, 0\n"\
			L"Tag: xborder, xbord, 0, 80, 0, 1, 0\n"\
			L"Tag: yborder, ybord, 0, 80, 0, 1, 0\n"\
			L"Tag: xshadow, xshad, -80, 80, 0, 1, 0\n"\
			L"Tag: yshadow, yshad, -80, 80, 0, 1, 0\n";
	}
	wxStringTokenizer tokenzer(txtSettings, "\n", wxTOKEN_STRTOK);
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
			tmp.value = tmpval;
			if (!tkzer.HasMoreTokens())
				continue;
			if (!tkzer.GetNextToken().Trim(false).ToCDouble(&tmpval))
				continue;
			tmp.step = tmpval;
			if (!tkzer.HasMoreTokens())
				continue;
			tmp.mode = wxAtoi(tkzer.GetNextToken().Trim(false));
			if (tkzer.HasMoreTokens()) {
				if (tkzer.GetNextToken().Trim(false).ToCDouble(&tmpval)) {
					tmp.value2 = tmpval;
					tmp.has2value = true;
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
			tag.rangeMax << ", " << tag.value << ", " << tag.step << ", " <<
			tag.mode;
		if (tag.has2value) {
			tagText << ", " << tag.value2;
		}
		tagText << "\n";
		ow.PartFileWrite(tagText);
	}
}


