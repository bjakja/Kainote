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

#include "AutoSavesRemoving.h"
#include "MappedButton.h"
#include "ListControls.h"
#include "config.h"
#include <ctime>

AutoSavesRemoving::AutoSavesRemoving(wxWindow* parent)
	:KaiDialog(parent, -1, _("Usuñ pliki autozapisów oraz cache"))
{
	std::time_t t = std::time(0);
	std::tm* now = std::localtime(&t);

	DialogSizer* main = new DialogSizer(wxVERTICAL);
	//date
	wxStaticBoxSizer* date = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Data dla usuwania starszych plików"));
	wxArrayString days;
	for (int i = 1; i < 32; i++) {
		days.Add(std::to_wstring(i));
	}
	KaiChoice* day = new KaiChoice(this, -1, wxDefaultPosition, wxDefaultSize, days);
	day->SetSelection(now->tm_mday - 1);
	wxString months[] = { _("Styczeñ"), _("Luty"), _("Marzec"), _("Kwiecieñ"), 
		_("Maj"), _("Czerwiec"), _("Lipiec"), _("Sierpieñ"), _("Wrzesieñ"), 
		_("PaŸdziernik"), _("Listopad"), _("Grudzieñ") };
	KaiChoice* month = new KaiChoice(this, -1, wxDefaultPosition, wxDefaultSize, 12, months);
	wxArrayString years;
	for (int i = 2012; i <= now->tm_year + 1900; i++) {
		years.Add(std::to_wstring(i));
	}
	KaiChoice* year = new KaiChoice(this, -1, wxDefaultPosition, wxDefaultSize, years);
	
	//month before today 0-11
	int monthBeforeMonth = now->tm_mon - 1;
	if (monthBeforeMonth < 0) {
		year->SetSelection(year->GetCount() - 2);
		monthBeforeMonth = 11;
	}else
		year->SetSelection(year->GetCount() - 1);

	month->SetSelection(monthBeforeMonth);

	date->Add(day, 0, wxALL | wxEXPAND, 3);
	date->Add(month, 0, wxALL | wxEXPAND, 3);
	date->Add(year, 0, wxALL | wxEXPAND, 3);

	//auto saves
	wxStaticBoxSizer* autoSaves = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Auto zapis"));
	MappedButton* removeSelectedAutoSaves = 
		new MappedButton(this, ID_REMOVE_SELECTED_AUTO_SAVES, _("Usuñ wybrane pliki auto zapisu"));
	MappedButton* removeAllAutoSaves =
		new MappedButton(this, ID_REMOVE_ALL_AUTO_SAVES, _("Usuñ wszystkie pliki auto zapisu"));
	MappedButton* removeAutoSavesByDate =
		new MappedButton(this, ID_REMOVE_AUTO_SAVES_BY_DATE, _("Usuñ starsze pliki auto zapisu"));

	autoSaves->Add(removeSelectedAutoSaves, 1, wxALL, 3);
	autoSaves->Add(removeAllAutoSaves, 1, wxALL, 3);
	autoSaves->Add(removeAutoSavesByDate, 1, wxALL, 3);

	//indices
	wxStaticBoxSizer* indices = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Indeks"));
	MappedButton* removeSelectedIndices =
		new MappedButton(this, ID_REMOVE_SELECTED_AUTO_SAVES, _("Usuñ wybrane indeksy"));
	MappedButton* removeAllIndices =
		new MappedButton(this, ID_REMOVE_ALL_AUTO_SAVES, _("Usuñ wszystkie indeksy"));
	MappedButton* removeIndicesByDate =
		new MappedButton(this, ID_REMOVE_AUTO_SAVES_BY_DATE, _("Usuñ starsze indeksy"));

	indices->Add(removeSelectedIndices, 1, wxALL, 3);
	indices->Add(removeAllIndices, 1, wxALL, 3);
	indices->Add(removeIndicesByDate, 1, wxALL, 3);

	//audio cache
	wxStaticBoxSizer* audioCache = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Audio cache"));
	MappedButton* removeSelectedAutoSaves =
		new MappedButton(this, ID_REMOVE_SELECTED_AUTO_SAVES, _("Usuñ wybrane pliki autozapisu"));
	MappedButton* removeAllAutoSaves =
		new MappedButton(this, ID_REMOVE_ALL_AUTO_SAVES, _("Usuñ wszystkie pliki autozapisu"));
	MappedButton* removeAutoSavesByDate =
		new MappedButton(this, ID_REMOVE_AUTO_SAVES_BY_DATE, _("Usuñ starsze pliki autozapisu"));

	audioCache->Add(removeSelectedAutoSaves, 1, wxALL, 3);
	audioCache->Add(removeAllAutoSaves, 1, wxALL, 3);
	audioCache->Add(removeAutoSavesByDate, 1, wxALL, 3);

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent& evt) {
		ClearSelected(evt.GetId());
		}, ID_REMOVE_SELECTED_AUTO_SAVES, ID_REMOVE_SELECTED_AUDIO_CACHES);

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent& evt) {
		ClearAll(evt.GetId());
		}, ID_REMOVE_ALL_AUTO_SAVES, ID_REMOVE_ALL_AUDIO_CACHES);

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent& evt) {
		ClearByDate(evt.GetId());
		}, ID_REMOVE_AUTO_SAVES_BY_DATE, ID_REMOVE_AUDIO_CACHE_BY_DATE);

	main->Add(date, 1, wxALL, 2);
	main->Add(autoSaves, 1, wxALL, 2);
	main->Add(indices, 1, wxALL, 2);
	main->Add(audioCache, 1, wxALL, 2);
	SetSizerAndFit(main);
	CenterOnParent();
}

void AutoSavesRemoving::ClearSelected(int id)
{
	wxString folder = (id == ID_REMOVE_SELECTED_AUTO_SAVES) ? L"\\Subs\\" :
		(id == ID_REMOVE_SELECTED_INDICES) ? L"\\Indices\\" : L"\\Audio Cache\\";

	wxString path = Options.pathfull + folder;
}

void AutoSavesRemoving::ClearAll(int id)
{
	wxString folder = (id == ID_REMOVE_ALL_AUTO_SAVES) ? L"\\Subs\\" :
		(id == ID_REMOVE_ALL_INDICES) ? L"\\Indices\\" : L"\\Audio Cache\\";

	wxString path = Options.pathfull + folder;
}

void AutoSavesRemoving::ClearByDate(int id)
{
	wxString folder = (id == ID_REMOVE_AUTO_SAVES_BY_DATE) ? L"\\Subs\\" :
		(id == ID_REMOVE_INDICES_BY_DATE) ? L"\\Indices\\" : L"\\Audio Cache\\";

	wxString path = Options.pathfull + folder;
}
