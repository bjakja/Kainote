//  Copyright (c) 2018, Marcin Drob

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

#include "FindReplaceResultsDialog.h"
#include "FindReplace.h"

wxDEFINE_EVENT(CHOOSE_RESULT, wxCommandEvent);

FindReplaceResultsDialog::FindReplaceResultsDialog(wxWindow *parent, FindReplace *FR, bool _findInFiles)
	: KaiDialog(parent, -1, _("Wyniki szukania"), wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER)
{
	DialogSizer * main = new DialogSizer(wxVERTICAL);
	resultsList = new KaiListCtrl(this, 23323, wxDefaultPosition, wxSize(700,300));
	resultsList->InsertColumn(0, "", TYPE_TEXT, -1);
	resultsList->SetHeaderHeight(0);
	main->Add(resultsList, 1, wxEXPAND | wxALL, 2);
	Bind(CHOOSE_RESULT, [=](wxCommandEvent &evt){
		SeekResults *results = (SeekResults*)evt.GetClientData();
		if (!results){
			KaiLogDebug("chujnia, ktoś ukradł rezultat wyszukiwania");
			return;
		}

		FR->ShowResult(results->tab, results->path, results->keyLine, results->findPosition);
	}, 23323);
	wxBoxSizer *buttonsSizer = new wxBoxSizer(wxHORIZONTAL);

	MappedButton *checkAll = new MappedButton(this, ID_CHECK_ALL, _("Zahacz wszystko"));
	MappedButton *unCheckAll = new MappedButton(this, ID_UNCHECK_ALL, _("Odhacz wszystko"));
	replaceChecked = new MappedButton(this, ID_REPLACE_CHECKED, _("Zamień"));
	ReplaceText = new KaiChoice(this, -1, FR->actualReplace, wxDefaultPosition, wxDefaultSize, FR->replaceRecent);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		CheckUncheckAll(true);
	}, ID_CHECK_ALL);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		CheckUncheckAll(false);
	}, ID_UNCHECK_ALL);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		replaceChecked->Enable(false);
		FR->ReplaceChecked();
	}, ID_REPLACE_CHECKED);
	buttonsSizer->Add(checkAll, 1, wxALL, 2);
	buttonsSizer->Add(unCheckAll, 1, wxALL, 2);
	buttonsSizer->Add(replaceChecked, 1, wxALL, 2);
	buttonsSizer->Add(ReplaceText, 0, wxALL | wxEXPAND, 2);
	main->Add(buttonsSizer, 0, wxALL, 2);
	SetSizerAndFit(main);

	findInFiles = _findInFiles;
}

FindReplaceResultsDialog::~FindReplaceResultsDialog()
{

}

void FindReplaceResultsDialog::SetHeader(const wxString &text)
{
	resultsCounter++;
	ResultsHeader *header = new ResultsHeader(text, resultsCounter);
	resultsList->AppendItemWithExtent(header);
}

void FindReplaceResultsDialog::SetResults(const wxString &text, const wxPoint &pos, TabPanel *_tab, int _idLine, int _keyLine, const wxString &_path)
{
	resultsCounter++;
	SeekResults *results = new SeekResults(text, pos, _tab, _idLine, _keyLine, _path);
	resultsList->AppendItemWithExtent(results);
}

void FindReplaceResultsDialog::ClearList()
{
	resultsCounter = 0;
	resultsList->ClearList();
	replaceChecked->Enable();
}

void FindReplaceResultsDialog::FilterList()
{
	//mode here is 1 visible blocks 0 hidden blocks
	resultsList->FilterList(0, 1);
	resultsList->Refresh(false);
}

void FindReplaceResultsDialog::CheckUncheckAll(bool check /*= true*/)
{
	for (size_t i = 0; i < resultsList->GetCount(); i++){
		Item *item = resultsList->GetItem(i, 0);
		if (item){
			item->modified = check;
		}
	}
	resultsList->Refresh(false);
}


void FindReplaceResultsDialog::GetReplaceString(wxString *replaceString)
{
	*replaceString = ReplaceText->GetValue();
}

void ResultsHeader::OnMouseEvent(wxMouseEvent &event, bool _enter, bool leave, KaiListCtrl *theList, Item **changed /* = NULL */)
{
	bool isOnCheckbox = event.GetX() < 19;
	if ((_enter && isOnCheckbox) || (!enter && isOnCheckbox)){
		enter = true;
		theList->Refresh(false);
	}
	else if (leave || (enter && !isOnCheckbox)){
		enter = false;
		theList->Refresh(false);
	}

	if (isOnCheckbox && (event.LeftDown() || event.LeftDClick())){
		modified = !modified;
		int i = positionInTable;
		while (theList->GetType(i, 0) == TYPE_TEXT){
			Item *item = theList->GetItem(i, 0);
			if (item){
				item->modified = modified;
			}
			i++;
		}
		theList->Refresh(false);
	}
	else if (event.LeftDown() || event.LeftDClick()){
		int i = positionInTable;
		while (theList->GetType(i, 0) == TYPE_TEXT){
			theList->FilterRow(i, (isVisible) ? NOT_VISIBLE : VISIBLE_BLOCK);
			i++;
		}
		isVisible = !isVisible;
		theList->FinalizeFiltering();
	}
}

void ResultsHeader::OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, KaiListCtrl *theList)
{
	wxSize ex = theList->GetTextExtent(name);
	wxString bitmapName = (modified) ? "checkbox_selected" : "checkbox";
	wxBitmap checkboxBmp = wxBITMAP_PNG(bitmapName);
	if (enter){ BlueUp(&checkboxBmp); }
	dc->DrawBitmap(checkboxBmp, x + 1, y + (height - 13) / 2);
	dc->SetTextForeground(Options.GetColour(FIND_RESULT_FILENAME_FOREGROUND));
	dc->SetTextBackground(Options.GetColour(FIND_RESULT_FILENAME_BACKGROUND));
	dc->SetBackgroundMode(wxSOLID);
	needTooltip = ex.x + 18 > width - 8;
	wxRect cur(x + 18, y, width - 8, height);
	dc->SetClippingRegion(cur);
	dc->DrawLabel(name, cur, wxALIGN_CENTER_VERTICAL);
	dc->DestroyClippingRegion();
	dc->SetTextForeground(Options.GetColour(theList->IsThisEnabled() ? WindowText : WindowTextInactive));
	dc->SetBackgroundMode(wxTRANSPARENT);
}

void SeekResults::OnMouseEvent(wxMouseEvent &event, bool _enter, bool leave, KaiListCtrl *theList, Item **changed /* = NULL */)
{
	bool isOnCheckbox = event.GetX() < 19;
	if ((_enter && isOnCheckbox) || (!enter && isOnCheckbox && !leave)){
		enter = true;
		theList->Refresh(false);
	}
	else if (leave || (enter && !isOnCheckbox)){
		enter = false;
		theList->Refresh(false);
	}

	if (isOnCheckbox && (event.LeftDown() || event.LeftDClick())){
		modified = !modified;
		int i = theList->FindItem(0, this);
		int j = i - 1;
		if (i < 0){
			theList->Refresh(false);
			return;
		}
		bool somethingChecked = false;
		while (theList->GetType(j, 0) == TYPE_TEXT){
			Item *item = theList->GetItem(j, 0);
			if (item && item->modified){
				somethingChecked = true;
			}
			j--;
		}
		while (theList->GetType(i, 0) == TYPE_TEXT){
			Item *item = theList->GetItem(i, 0);
			if (item && item->modified){
				somethingChecked = true;
				goto done;
			}
			i++;
		}
	done:
		Item *header = theList->GetItem(j, 0);
		if (header/* && header->type == TYPE_HEADER*/){
			header->modified = somethingChecked;
		}
		theList->Refresh(false);
	}
	else if (event.LeftDClick()){
		wxCommandEvent *evt = new wxCommandEvent(CHOOSE_RESULT, theList->GetId());
		evt->SetClientData(this);
		wxQueueEvent(theList->GetParent(), evt);
	}
}

void SeekResults::OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, KaiListCtrl *theList)
{
	wxString lineNum = wxString::Format(_("Linia %i: "), idLine);
	wxString lineAndNum = lineNum + name;
	wxSize ex = theList->GetTextExtent(lineAndNum);
	wxSize exOfFound = theList->GetTextExtent(lineAndNum.Mid(0, findPosition.x + lineNum.length()));
	wxString bitmapName = (modified) ? "checkbox_selected" : "checkbox";
	wxBitmap checkboxBmp = wxBITMAP_PNG(bitmapName);
	if (enter){ BlueUp(&checkboxBmp); }
	dc->DrawBitmap(checkboxBmp, x + 1, y + (height - 13) / 2);

	needTooltip = ex.x + 18 > width - 8;
	wxRect cur(x + 18, y, width - 8, height);
	dc->SetClippingRegion(cur);
	dc->DrawLabel(lineAndNum, cur, wxALIGN_CENTER_VERTICAL);
	dc->DestroyClippingRegion();
	dc->SetTextForeground(Options.GetColour(FIND_RESULT_FOUND_PHRASE_FOREGROUND));
	const wxColour &background = Options.GetColour(FIND_RESULT_FOUND_PHRASE_BACKGROUND);
	dc->SetBrush(wxBrush(background));
	dc->SetPen(wxPen(background));
	wxString foundText = lineAndNum.Mid(findPosition.x + lineNum.length(), findPosition.y);
	wxSize exFoundText = theList->GetTextExtent(foundText);
	dc->DrawRectangle(x + exOfFound.x + 18, y + ((height - exOfFound.y) / 2), exFoundText.x, height);
	dc->DrawText(foundText, x + exOfFound.x + 18, y + ((height - exOfFound.y) / 2));

	dc->SetTextForeground(Options.GetColour(theList->IsThisEnabled() ? WindowText : WindowTextInactive));
}

wxSize SeekResults::GetTextExtents(KaiListCtrl *theList){
	wxString lineNum = wxString::Format(_("Linia %i: "), idLine);
	wxString lineAndNum = lineNum + name;
	wxSize size = theList->GetTextExtent(lineAndNum);
	size.x += 28;
	size.y += 4;
	return size;
}