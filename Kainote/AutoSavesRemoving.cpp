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
//#include "config.h"
#include "MappedButton.h"
#include "KaiStaticBoxSizer.h"
//#include "Notebook.h"
//#include "LogHandler.h"
#include <ctime>
#include <wx/filedlg.h>
#include <wx/dir.h>


AutoSavesRemoving::AutoSavesRemoving(wxWindow* parent)
	: KaiDialog(parent, -1, _("Usuń pliki tymczasowe"))
{
	std::time_t t = std::time(0);
	std::tm* now = std::localtime(&t);

	DialogSizer* main = new DialogSizer(wxVERTICAL);
	//date
	KaiStaticBoxSizer* date = new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Usuń pliki starsze niż"));
	wxBoxSizer* dateAndRemoveAll = new wxBoxSizer(wxHORIZONTAL);
	wxArrayString days;
	for (int i = 1; i < 32; i++) {
		days.Add(std::to_wstring(i));
	}
	day = new KaiChoice(this, ID_DATE_DAY_LIST, wxDefaultPosition, wxDefaultSize, days);
	day->SetSelection(now->tm_mday - 1);
	wxString months[] = { _("Styczeń"), _("Luty"), _("Marzec"), _("Kwiecień"),
		_("Maj"), _("Czerwiec"), _("Lipiec"), _("Sierpień"), _("Wrzesień"),
		_("Październik"), _("Listopad"), _("Grudzień") };
	month = new KaiChoice(this, ID_DATE_MONTH_LIST, wxDefaultPosition, wxDefaultSize, 12, months);
	
	wxArrayString years;
	for (int i = 2012; i <= now->tm_year + 1900; i++) {
		years.Add(std::to_wstring(i));
	}
	year = new KaiChoice(this, -1, wxDefaultPosition, wxDefaultSize, years);

	Bind(wxEVT_COMMAND_CHOICE_SELECTED, [=](wxCommandEvent& evt) {
		int sely = year->GetSelection();
		wxString selyear = year->GetString(sely);
		int numyear = wxAtoi(selyear);
		if (numyear < now->tm_year + 1900)
			return;

		int selm = month->GetSelection();
		if (selm < now->tm_mon)
			return;

		int seld = day->GetSelection() + 1;
		if (seld < now->tm_mday)
			return;

		year->SetSelection(sely - 1);
		}, ID_DATE_DAY_LIST, ID_DATE_MONTH_LIST);
	//month before today 0-11
	int monthBeforeMonth = now->tm_mon - 1;
	if (monthBeforeMonth < 0) {
		year->SetSelection(year->GetCount() - 2);
		monthBeforeMonth = 11;
	}
	else
		year->SetSelection(year->GetCount() - 1);

	month->SetSelection(monthBeforeMonth);

	date->Add(day, 0, wxALL, 3);
	date->Add(month, 0, wxALL, 3);
	date->Add(year, 0, wxALL, 3);

	//remove all
	KaiStaticBoxSizer* removeAll = new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Usuń z wszystkich folderów"));
	MappedButton* removeAllTemporary =
		new MappedButton(this, ID_REMOVE_ALL, _("Usuń wszystkie pliki"));
	MappedButton* removeAllTemporaryByDate =
		new MappedButton(this, ID_REMOVE_ALL_BY_DATE, _("Usuń wszystkie starsze pliki"));

	removeAll->Add(removeAllTemporary, 1, wxALL | wxEXPAND, 3);
	removeAll->Add(removeAllTemporaryByDate, 1, wxALL | wxEXPAND, 3);

	dateAndRemoveAll->Add(date, 0, wxRIGHT, 3);
	dateAndRemoveAll->Add(removeAll, 1, wxEXPAND, 0);

	//auto saves
	KaiStaticBoxSizer* autoSaves = new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Auto zapis"));
	MappedButton* removeSelectedAutoSaves =
		new MappedButton(this, ID_REMOVE_SELECTED_AUTO_SAVES, _("Usuń wybrane pliki auto zapisu"));
	MappedButton* removeAllAutoSaves =
		new MappedButton(this, ID_REMOVE_ALL_AUTO_SAVES, _("Usuń wszystkie pliki auto zapisu"));
	MappedButton* removeAutoSavesByDate =
		new MappedButton(this, ID_REMOVE_AUTO_SAVES_BY_DATE, _("Usuń starsze pliki auto zapisu"));

	autoSaves->Add(removeSelectedAutoSaves, 1, wxALL | wxEXPAND, 3);
	autoSaves->Add(removeAllAutoSaves, 1, wxALL | wxEXPAND, 3);
	autoSaves->Add(removeAutoSavesByDate, 1, wxALL | wxEXPAND, 3);

	//indices
	KaiStaticBoxSizer* indices = new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Indeks"));
	MappedButton* removeSelectedIndices =
		new MappedButton(this, ID_REMOVE_SELECTED_INDICES, _("Usuń wybrane indeksy"));
	MappedButton* removeAllIndices =
		new MappedButton(this, ID_REMOVE_ALL_INDICES, _("Usuń wszystkie indeksy"));
	MappedButton* removeIndicesByDate =
		new MappedButton(this, ID_REMOVE_INDICES_BY_DATE, _("Usuń starsze indeksy"));

	indices->Add(removeSelectedIndices, 1, wxALL | wxEXPAND, 3);
	indices->Add(removeAllIndices, 1, wxALL | wxEXPAND, 3);
	indices->Add(removeIndicesByDate, 1, wxALL | wxEXPAND, 3);

	//audio cache
	KaiStaticBoxSizer* audioCache = new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Audio cache"));
	MappedButton* removeSelectedAudioCache =
		new MappedButton(this, ID_REMOVE_SELECTED_AUDIO_CACHES, _("Usuń wybrane pliki audio cache"));
	MappedButton* removeAllAudioCache =
		new MappedButton(this, ID_REMOVE_ALL_AUDIO_CACHES, _("Usuń wszystkie pliki audio cache"));
	MappedButton* removeAudioCacheByDate =
		new MappedButton(this, ID_REMOVE_AUDIO_CACHE_BY_DATE, _("Usuń starsze pliki audio cache"));

	audioCache->Add(removeSelectedAudioCache, 1, wxALL | wxEXPAND, 3);
	audioCache->Add(removeAllAudioCache, 1, wxALL | wxEXPAND, 3);
	audioCache->Add(removeAudioCacheByDate, 1, wxALL | wxEXPAND, 3);

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent& evt) {
		ClearSelected(evt.GetId());
		}, ID_REMOVE_SELECTED_AUTO_SAVES, ID_REMOVE_SELECTED_AUDIO_CACHES);

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent& evt) {
		ClearAll(evt.GetId());
		}, ID_REMOVE_ALL_AUTO_SAVES, ID_REMOVE_ALL_AUDIO_CACHES);

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent& evt) {
		ClearByDate(evt.GetId());
		}, ID_REMOVE_AUTO_SAVES_BY_DATE, ID_REMOVE_AUDIO_CACHE_BY_DATE);

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent& evt) {
		ClearAll(ID_REMOVE_ALL_AUTO_SAVES);
		ClearAll(ID_REMOVE_ALL_INDICES);
		ClearAll(ID_REMOVE_ALL_AUDIO_CACHES);
		}, ID_REMOVE_ALL);

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent& evt) {
		ClearByDate(ID_REMOVE_AUTO_SAVES_BY_DATE);
		ClearByDate(ID_REMOVE_INDICES_BY_DATE);
		ClearByDate(ID_REMOVE_AUDIO_CACHE_BY_DATE);
		}, ID_REMOVE_ALL_BY_DATE);

	main->Add(dateAndRemoveAll, 1, wxALL | wxEXPAND, 2);
	main->Add(autoSaves, 1, wxALL | wxEXPAND, 2);
	main->Add(indices, 1, wxALL | wxEXPAND, 2);
	main->Add(audioCache, 1, wxALL | wxEXPAND, 2);
	SetSizerAndFit(main);
	CenterOnParent();
}

void AutoSavesRemoving::ClearSelected(int id)
{
	wxString folder = (id == ID_REMOVE_SELECTED_AUTO_SAVES) ? L"\\Subs\\" :
		(id == ID_REMOVE_SELECTED_INDICES) ? L"\\Indices\\" : L"\\AudioCache\\";

	wxString path = Options.pathfull + folder;
	wxString description = (id == ID_REMOVE_SELECTED_AUTO_SAVES) ?
		_("Pliki napisów (*.ass),(*.ssa),(*.srt),(*.sub),(*.txt)|*.ass;*.ssa;*.srt;*.sub;*.txt") :
		(id == ID_REMOVE_SELECTED_INDICES) ? _("Pliki indeksów (*.ffindex)|*.ffindex") :
		_("Pliki audio cache (*.w64)|*.w64");

	wxFileDialog* FileDialog = new wxFileDialog(this, _("Wybierz plik i do usunięcia"), path,
		emptyString, description,
		wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);

	if (FileDialog->ShowModal() == wxID_OK) {
		wxArrayString paths;
		FileDialog->GetPaths(paths);
		for (auto& path : paths) {
			_wremove(path.wc_str());
		}
	}
	FileDialog->Destroy();
}

void AutoSavesRemoving::ClearAll(int id)
{
	wxString folder = (id == ID_REMOVE_ALL_AUTO_SAVES) ? L"\\Subs\\" :
		(id == ID_REMOVE_ALL_INDICES) ? L"\\Indices\\" : L"\\AudioCache\\";

	wxString path = Options.pathfull + folder;
	wxDir dir(path);
	if (dir.IsOpened()) {
		wxArrayString paths;
		dir.GetAllFiles(path, &paths);
		for (auto& path : paths) {
			_wremove(path.wc_str());
		}
		return;
	}
	KaiLog(_("Nie można otworzyć folderu plików tymczasowych"));
}

void AutoSavesRemoving::ClearByDate(int id)
{
	wxString folder = (id == ID_REMOVE_AUTO_SAVES_BY_DATE) ? L"\\Subs\\" :
		(id == ID_REMOVE_INDICES_BY_DATE) ? L"\\Indices\\" : L"\\AudioCache\\";

	wxString path = Options.pathfull + folder;
	wxString findPath = path + L"*";

	TIME_ZONE_INFORMATION timeZoneInfo;
	GetTimeZoneInformation(&timeZoneInfo);

	SYSTEMTIME chosenTime;

	chosenTime.wMonth = month->GetSelection() + 1;
	chosenTime.wDay = day->GetSelection() + 1;
	chosenTime.wYear = wxAtoi(year->GetString(year->GetSelection()));
	chosenTime.wHour = 14;
	chosenTime.wMinute = 11;
	chosenTime.wSecond = 11;
	wxArrayString paths;

	WIN32_FIND_DATAW data;
	HANDLE h = FindFirstFileW(findPath.wc_str(), &data);
	if (h == INVALID_HANDLE_VALUE)
	{
		KaiLog(_("Nie można otworzyć folderu plików tymczasowych"));
		return;
	}

	while (1) {
		int result = FindNextFile(h, &data);
		if (result == ERROR_NO_MORE_FILES || result == 0) { break; }
		else if (data.nFileSizeLow == 0) { continue; }
		SYSTEMTIME accessSystemTime;
		SYSTEMTIME accessSystemUniversalTime;
		FileTimeToSystemTime(&data.ftLastWriteTime, &accessSystemUniversalTime);
		BOOL succeeded = SystemTimeToTzSpecificLocalTime(&timeZoneInfo, &accessSystemUniversalTime, &accessSystemTime);
		if (accessSystemTime.wYear > chosenTime.wYear)
			continue;

		if (accessSystemTime.wMonth > chosenTime.wMonth && accessSystemTime.wYear == chosenTime.wYear)
			continue;

		if (accessSystemTime.wDay >= chosenTime.wDay && accessSystemTime.wYear == chosenTime.wYear && 
			accessSystemTime.wMonth == chosenTime.wMonth)
			continue;
		
		paths.Add(path + data.cFileName);
	}

	FindClose(h);

	for (auto& path : paths) {
		_wremove(path.wc_str());
	}
}
