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

#include "AutoSaveOpen.h"
#include "KainoteMain.h"
#include <wx/dir.h>
#include <wx/tokenzr.h>

AutoSaveOpen::AutoSaveOpen(KainoteFrame* _Kai)
	: KaiDialog(_Kai, -1, _("Otwórz plik autozapisu"), 
		wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
	, Kai(_Kai)
{

	DialogSizer* mainSizer = new DialogSizer(wxVERTICAL);
	KaiStaticBoxSizer* seekingSizer = new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Wyszukaj pliki"));
	seekingText = new KaiTextCtrl(this, ID_AUTO_SAVE_SEEKING_TEXT, L"", 
		wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	filterList = new MappedButton(this, ID_AUTO_SAVE_FILTER, _("Filtruj listę"));
	seekAllWords = new KaiCheckBox(this, -1, _("Wszystkie słowa"));
	seekAllWords->SetValue(true);

	seekingSizer->Add(seekingText, 4, wxALL | wxEXPAND, 4);
	seekingSizer->Add(filterList, 1, wxTOP | wxBOTTOM | wxRIGHT | wxEXPAND, 4);
	seekingSizer->Add(seekAllWords, 1, wxTOP | wxBOTTOM | wxRIGHT | wxEXPAND, 4);

	GenerateList();
	wxBoxSizer* listsSizer = new wxBoxSizer(wxHORIZONTAL);
	KaiStaticBoxSizer* filesSizer = new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Pliki"));
	filesList = new KaiListCtrl(this, ID_AUTO_SAVE_LIST, paths, wxDefaultPosition, wxSize(300,400));

	filesSizer->Add(filesList, 1, wxALL | wxEXPAND, 2);

	KaiStaticBoxSizer* versionSizer = new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Wersje"));
	versionList = new KaiListCtrl(this, -1, wxArrayString(), wxDefaultPosition, wxSize(100, 400));

	versionSizer->Add(versionList, 1, wxALL | wxEXPAND, 2);

	listsSizer->Add(filesSizer, 3, wxRIGHT | wxEXPAND, 2);
	listsSizer->Add(versionSizer, 1, wxEXPAND, 2);
	

	wxBoxSizer* buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
	open = new MappedButton(this, ID_AUTO_SAVE_OK, _("Otwórz"), -1, wxDefaultPosition, wxSize(100, -1));
	MappedButton* cancel = new MappedButton(this, wxID_CANCEL, _("Anuluj"));

	buttonsSizer->Add(open, 1, wxALL, 4);
	buttonsSizer->Add(cancel, 1, wxTOP | wxBOTTOM | wxRIGHT, 4);

	mainSizer->Add(seekingSizer, 0, wxALL | wxEXPAND, 2);
	mainSizer->Add(listsSizer, 1, wxALL | wxEXPAND, 2);
	mainSizer->Add(buttonsSizer, 0, wxALL | wxALIGN_RIGHT, 2);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AutoSaveOpen::OnOkClick, this, ID_AUTO_SAVE_OK);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AutoSaveOpen::OnFindFiles, this, ID_AUTO_SAVE_FILTER);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AutoSaveOpen::OnEnter, this, ID_AUTO_SAVE_ENTER);
	Bind(LIST_ITEM_LEFT_CLICK, &AutoSaveOpen::OnListClick, this, ID_AUTO_SAVE_LIST);

	SetEnterId(ID_AUTO_SAVE_ENTER, false);

	SetSizerAndFit(mainSizer);
	CenterOnParent();
	SetSelection(0);

}

AutoSaveOpen::~AutoSaveOpen()
{
	for (auto cur = versions.begin(); cur != versions.end(); cur++) {
		delete (*cur);
	}
}

void AutoSaveOpen::FindFiles()
{
	wxString findString = seekingText->GetValue();
	if (findString.empty()) {
		for (size_t k = 0; k < paths.GetCount(); k++) {
			filesList->FilterItem(k, VISIBLE, false);
		}
	}
	else {
		wxArrayString tokens = wxStringTokenize(findString, L" ", wxTOKEN_STRTOK);
		bool allWords = seekAllWords->GetValue();
		for (size_t k = 0; k < paths.GetCount(); k++) {
			bool found = allWords;
			wxString path = paths[k].Lower();
			for (auto& token : tokens) {
				size_t findPos = path.find(token.Lower());
				if (allWords) {
					if (findPos == -1) {
						found = false;
						break;
					}
				}
				else {
					if (findPos != -1) {
						found = true;
						break;
					}
				}
			}
			filesList->FilterItem(k, found ? VISIBLE : NOT_VISIBLE, false);
		}
	}
	filesList->FilterFinalize();
	SetSelection(0, true);
}

void AutoSaveOpen::GenerateList()
{
	wxString path = Options.pathfull + L"\\Subs\\*";

	TIME_ZONE_INFORMATION timeZoneInfo;
	GetTimeZoneInformation(&timeZoneInfo);

	WIN32_FIND_DATAW data;
	HANDLE h = FindFirstFileW(path.wc_str(), &data);
	if (h == INVALID_HANDLE_VALUE)
	{
		KaiLog(_("Nie można otworzyć folderu autozapisu"));
		return;
	}

	while (1) {
		int result = FindNextFile(h, &data);
		if (result == ERROR_NO_MORE_FILES || result == 0) { break; }
		else if (data.nFileSizeLow == 0) { continue; }

		wxString fileName = wxString(data.cFileName);
		wxString ext;
		wxString rest = fileName.BeforeLast(L'.', &ext);
		wxString fileNum;
		wxString rest1 = rest.BeforeLast(L'_', &fileNum);
		wxString tabNum;
		wxString strippedFileName = rest1.BeforeLast(L'_', &tabNum);
		if (strippedFileName.empty()) {
			if (fileName.StartsWith(L"DummySubs")) {
				continue;
			}
			strippedFileName = _("Bez Nazwy");
		}
			

		strippedFileName << L"." << ext;

		FILETIME accessTime = data.ftLastWriteTime;
		SYSTEMTIME accessSystemTime;
		SYSTEMTIME accessSystemUniversalTime;
		FileTimeToSystemTime(&accessTime, &accessSystemUniversalTime);
		BOOL succeeded = SystemTimeToTzSpecificLocalTime(&timeZoneInfo, &accessSystemUniversalTime, &accessSystemTime);
		
		wxString accessStringTime;
		accessStringTime = wxString::Format(L"%02i/%02i/%02i   %02i:%02i:%02i",
			accessSystemTime.wMonth, accessSystemTime.wDay, accessSystemTime.wYear,
			accessSystemTime.wHour, accessSystemTime.wMinute, accessSystemTime.wSecond);

		int findResult = paths.Index(strippedFileName);
		if (findResult == -1) {
			paths.Add(strippedFileName);
			std::map<wxString, wxString, DateCompare>* ver = new std::map<wxString, wxString, DateCompare>();
			ver->insert(std::pair<wxString, wxString>(accessStringTime, fileName));
			versions.push_back(ver);
		}
		else {
			versions[findResult]->insert(std::pair<wxString, wxString>(accessStringTime, fileName));
		}
	}
	FindClose(h);
	if (!paths.GetCount()) {
		KaiLog(_("Folder autozapisu jest pusty"));
		return;
	}
}

void AutoSaveOpen::SetSelection(int num, bool isId)
{
	versionList->ClearList();
	if (num == -1 || !filesList->GetCount()) {
		filesList->SetSelection(0);
		return;
	}
	else if (num >= filesList->GetCount()) {
		num = filesList->GetCount() - 1;
	}

	filesList->SetSelection(num, false, isId);
	if (isId)
		num = filesList->GetSelection();
	
	if (num >= versions.size() || num < 0) {
		versionList->SetSelection(-1);
	}
	else {
		auto& verList = versions[num];
		for (auto cur = verList->begin(); cur != verList->end(); cur++) {
			versionList->AppendItemWithExtent(new ItemText(cur->first));
		}
		versionList->SetSelection(0);
	}
}

void AutoSaveOpen::OnFindFiles(wxCommandEvent& evt)
{
	FindFiles();
}

void AutoSaveOpen::OnOkClick(wxCommandEvent& evt)
{
	int selFile = filesList->GetSelection();
	int selVersion = versionList->GetSelection();
	if (selFile < 0 || selFile >= filesList->GetCount())
		return;

	if (!versionList->GetCount() || selVersion < 0 || selVersion >= versionList->GetCount())
		return;

	if (selFile >= versions.size())
		return;

	auto& verList = versions[selFile];

	Item *item = versionList->GetItem(selVersion, 0);
	if (!item)
		return;

	auto it = verList->find(item->name);
	if (it != verList->end()) {
		wxString filePath = Options.pathfull + L"\\Subs\\" + it->second;
		Kai->OpenFile(filePath);
		EndModal(wxOK);
		return;
	}

	KaiLog(_("Wczytywanie autozapisu nie powiodło się"));
}

void AutoSaveOpen::OnListClick(wxCommandEvent& evt)
{
	int sel = filesList->GetSelection();
	SetSelection(sel);
}

void AutoSaveOpen::OnEnter(wxCommandEvent& evt)
{
	wxWindow* win = FindFocus();
	if (seekingText->HasFocus() || filterList->HasFocus()) {
		FindFiles();
	}
	else {
		OnOkClick(evt);
	}
}
