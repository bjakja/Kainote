//  Copyright (c) 2021 - 2026, Marcin Drob

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
	: KaiDialog(parent, -1, _("Remove temporary files"))
{
	std::time_t t = std::time(0);
	std::tm* now = std::localtime(&t);

	DialogSizer* main = new DialogSizer(wxVERTICAL);
	//date
	KaiStaticBoxSizer* date = new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Remove files older than"));
	wxBoxSizer* dateAndRemoveAll = new wxBoxSizer(wxHORIZONTAL);
	wxArrayString days;
	for (int i = 1; i < 32; i++) {
		days.Add(std::to_wstring(i));
	}
	day = new KaiChoice(this, ID_DATE_DAY_LIST, wxDefaultPosition, wxDefaultSize, days);
	day->SetSelection(now->tm_mday - 1);
	wxString months[] = { _("January"), _("February"), _("March"), _("April"),
		_("May"), _("June"), _("July"), _("August"), _("September"),
		_("October"), _("November"), _("December") };
	month = new KaiChoice(this, ID_DATE_MONTH_LIST, wxDefaultPosition, wxDefaultSize, 12, months);
	
	wxArrayString years;
	for (int i = 2012; i <= now->tm_year + 1900; i++) {
		years.Add(std::to_wstring(i));
	}
	year = new KaiChoice(this, -1, wxDefaultPosition, wxDefaultSize, years);

	Bind(wxEVT_COMMAND_CHOICE_SELECTED, [=, this](wxCommandEvent& evt) {
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
	KaiStaticBoxSizer* removeAll = new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Remove from all folders"));
	MappedButton* removeAllTemporary =
		new MappedButton(this, ID_REMOVE_ALL, _("Remove all files"));
	MappedButton* removeAllTemporaryByDate =
		new MappedButton(this, ID_REMOVE_ALL_BY_DATE, _("Remove all older files"));

	removeAll->Add(removeAllTemporary, 1, wxALL | wxEXPAND, 3);
	removeAll->Add(removeAllTemporaryByDate, 1, wxALL | wxEXPAND, 3);

	dateAndRemoveAll->Add(date, 0, wxRIGHT, 3);
	dateAndRemoveAll->Add(removeAll, 1, wxEXPAND, 0);

	//auto saves
	KaiStaticBoxSizer* autoSaves = new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Auto save"));
	MappedButton* removeSelectedAutoSaves =
		new MappedButton(this, ID_REMOVE_SELECTED_AUTO_SAVES, _("Remove selected autosave files"));
	MappedButton* removeAllAutoSaves =
		new MappedButton(this, ID_REMOVE_ALL_AUTO_SAVES, _("Remove all auto save files"));
	MappedButton* removeAutoSavesByDate =
		new MappedButton(this, ID_REMOVE_AUTO_SAVES_BY_DATE, _("Remove older auto save files"));

	autoSaves->Add(removeSelectedAutoSaves, 1, wxALL | wxEXPAND, 3);
	autoSaves->Add(removeAllAutoSaves, 1, wxALL | wxEXPAND, 3);
	autoSaves->Add(removeAutoSavesByDate, 1, wxALL | wxEXPAND, 3);

	//indices
	KaiStaticBoxSizer* indices = new KaiStaticBoxSizer(wxHORIZONTAL, this, _("FFMS2 index"));
	MappedButton* removeSelectedIndices =
		new MappedButton(this, ID_REMOVE_SELECTED_INDICES, _("Remove selected index files"));
	MappedButton* removeAllIndices =
		new MappedButton(this, ID_REMOVE_ALL_INDICES, _("Remove all index files"));
	MappedButton* removeIndicesByDate =
		new MappedButton(this, ID_REMOVE_INDICES_BY_DATE, _("Remove older index files"));

	indices->Add(removeSelectedIndices, 1, wxALL | wxEXPAND, 3);
	indices->Add(removeAllIndices, 1, wxALL | wxEXPAND, 3);
	indices->Add(removeIndicesByDate, 1, wxALL | wxEXPAND, 3);

	//audio cache
	KaiStaticBoxSizer* audioCache = new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Audio cache"));
	MappedButton* removeSelectedAudioCache =
		new MappedButton(this, ID_REMOVE_SELECTED_AUDIO_CACHES, _("Remove selected audio cache files"));
	MappedButton* removeAllAudioCache =
		new MappedButton(this, ID_REMOVE_ALL_AUDIO_CACHES, _("Remove all audio cache files"));
	MappedButton* removeAudioCacheByDate =
		new MappedButton(this, ID_REMOVE_AUDIO_CACHE_BY_DATE, _("Remove older audio cache files"));

	audioCache->Add(removeSelectedAudioCache, 1, wxALL | wxEXPAND, 3);
	audioCache->Add(removeAllAudioCache, 1, wxALL | wxEXPAND, 3);
	audioCache->Add(removeAudioCacheByDate, 1, wxALL | wxEXPAND, 3);

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=, this](wxCommandEvent& evt) {
		ClearSelected(evt.GetId());
		}, ID_REMOVE_SELECTED_AUTO_SAVES, ID_REMOVE_SELECTED_AUDIO_CACHES);

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=, this](wxCommandEvent& evt) {
		ClearAll(evt.GetId());
		}, ID_REMOVE_ALL_AUTO_SAVES, ID_REMOVE_ALL_AUDIO_CACHES);

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=, this](wxCommandEvent& evt) {
		ClearByDate(evt.GetId());
		}, ID_REMOVE_AUTO_SAVES_BY_DATE, ID_REMOVE_AUDIO_CACHE_BY_DATE);

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=, this](wxCommandEvent& evt) {
		ClearAll(ID_REMOVE_ALL_AUTO_SAVES);
		ClearAll(ID_REMOVE_ALL_INDICES);
		ClearAll(ID_REMOVE_ALL_AUDIO_CACHES);
		}, ID_REMOVE_ALL);

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=, this](wxCommandEvent& evt) {
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
	wxString folder = (id == ID_REMOVE_SELECTED_AUTO_SAVES) ? L"/Subs/" :
		(id == ID_REMOVE_SELECTED_INDICES) ? L"/Indices/" : L"/AudioCache/";

	wxString path = Options.pathfull + folder;
	wxString description = (id == ID_REMOVE_SELECTED_AUTO_SAVES) ?
		_("Subtitle files (*.ass),(*.ssa),(*.srt),(*.sub),(*.txt)|*.ass;*.ssa;*.srt;*.sub;*.txt") :
		(id == ID_REMOVE_SELECTED_INDICES) ? _("Index files (*.ffindex)|*.ffindex") :
		_("Audio cache files (*.w64)|*.w64");

	wxFileDialog* FileDialog = new wxFileDialog(this, _("Choose file to remove"), path,
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
	wxString folder = (id == ID_REMOVE_ALL_AUTO_SAVES) ? L"/Subs/" :
		(id == ID_REMOVE_ALL_INDICES) ? L"/Indices/" : L"/AudioCache/";

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
	KaiLog(_("Cannot open temporary files folder"));
}

void AutoSavesRemoving::ClearByDate(int id)
{
	wxString folder = (id == ID_REMOVE_AUTO_SAVES_BY_DATE) ? L"/Subs/" :
		(id == ID_REMOVE_INDICES_BY_DATE) ? L"/Indices/" : L"/AudioCache/";

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
		KaiLog(_("Cannot open temporary files folder"));
		return;
	}

	do {
		wxString fileName = wxString(data.cFileName);
		if (fileName == L"." || fileName == L".." || data.nFileSizeLow == 0) { continue; }
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
		
		paths.Add(path + fileName);
	} while (FindNextFile(h, &data));

	FindClose(h);

	for (auto& path : paths) {
		_wremove(path.wc_str());
	}
}
