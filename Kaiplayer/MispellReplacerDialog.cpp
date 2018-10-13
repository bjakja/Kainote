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

#include "MispellReplacerDialog.h"
#include "MisspellReplacer.h"



void ReplacerResultsHeader::OnMouseEvent(wxMouseEvent &event, bool enter, bool leave, KaiListCtrl *theList, Item **changed /* = NULL */)
{
	if (event.LeftDown()){
		int i = positionInTable + 1;
		while(theList->GetType(i, 0) == TYPE_TEXT){
			theList->FilterRow(i, (isVisible) ? NOT_VISIBLE : VISIBLE_BLOCK);
		}
		isVisible = !isVisible;
	}
}

void ReplacerResultsHeader::OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, KaiListCtrl *theList)
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

void ReplacerSeekResults::OnMouseEvent(wxMouseEvent &event, bool enter, bool leave, KaiListCtrl *theList, Item **changed /* = NULL */)
{
	if (event.LeftDClick()){
		wxCommandEvent *evt = new wxCommandEvent(CHOOSE_RESULT, theList->GetId());
		evt->SetClientData(this);
		wxQueueEvent(theList->GetParent(), evt);
	}
}

void ReplacerSeekResults::OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, KaiListCtrl *theList)
{
	wxSize ex = theList->GetTextExtent(name);
	wxSize exOfFound = theList->GetTextExtent(name.Mid(0, findPosition.x));
	wxString bitmapName = (modified) ? "checkbox_selected" : "checkbox";
	wxBitmap checkboxBmp = wxBITMAP_PNG(bitmapName);
	if (enter){ BlueUp(&checkboxBmp); }
	dc->DrawBitmap(checkboxBmp, x + 1, y + (height - 13) / 2);

	needTooltip = ex.x + 18 > width - 8;
	wxRect cur(x + 18, y, width - 8, height);
	dc->SetClippingRegion(cur);
	dc->DrawLabel(name, cur, wxALIGN_CENTER_VERTICAL);
	dc->DestroyClippingRegion();
	dc->SetTextForeground(Options.GetColour(FIND_RESULT_FOUND_PHRASE_FOREGROUND));
	const wxColour &background = Options.GetColour(FIND_RESULT_FOUND_PHRASE_BACKGROUND);
	dc->SetBrush(wxBrush(background));
	dc->SetPen(wxPen(background));
	wxString foundText = name.Mid(findPosition.x, findPosition.y);
	wxSize exFoundText = theList->GetTextExtent(foundText);
	dc->DrawRectangle(x + exOfFound.x + 18, y + ((height - exOfFound.y) / 2), exFoundText.x, height);
	dc->DrawText(foundText, x + exOfFound.x + 18, y + ((height - exOfFound.y) / 2));

	dc->SetTextForeground(Options.GetColour(theList->IsThisEnabled() ? WindowText : WindowTextInactive));
}

FindResultDialog::FindResultDialog(wxWindow *parent, MisspellReplacer *MR)
	:KaiDialog(parent, -1, _("Wyniki szukania"))
{
	DialogSizer * main = new DialogSizer(wxVERTICAL);
	ResultsList = new KaiListCtrl(this, 23323, wxDefaultPosition, wxSize(700, 300));
	ResultsList->InsertColumn(0, "", TYPE_TEXT, -1);
	ResultsList->SetHeaderHeight(0);
	main->Add(ResultsList, 1, wxEXPAND | wxALL, 2);
	
	Bind(CHOOSE_RESULT, [=](wxCommandEvent &evt){
		ReplacerSeekResults *results = (ReplacerSeekResults*)evt.GetClientData();
		if (!results){
			KaiLogDebug("chujnia, ktoœ ukrad³ rezultat wyszukiwania");
			return;
		}
		//maybe some day I will add changing in folder
		MR->ShowResult(results->tab/*, results->path*/, results->keyLine);
	}, 23323);

	wxBoxSizer *buttonsSizer = new wxBoxSizer(wxHORIZONTAL);

	MappedButton *checkAll = new MappedButton(this, ID_CHECK_ALL, _("Zahacz wszystko"));
	MappedButton *unCheckAll = new MappedButton(this, ID_UNCHECK_ALL, _("Odhacz wszystko"));
	MappedButton *replaceChecked = new MappedButton(this, ID_REPLACE_CHECKED, _("Zamieñ"));
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		CheckUncheckAll(true);
	}, ID_CHECK_ALL);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		CheckUncheckAll(false);
	}, ID_UNCHECK_ALL);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
		MR->ReplaceChecked();
	}, ID_REPLACE_CHECKED);
	buttonsSizer->Add(checkAll, 1, wxALL, 2);
	buttonsSizer->Add(unCheckAll, 1, wxALL, 2);
	buttonsSizer->Add(replaceChecked, 1, wxALL, 2);
	main->Add(ResultsList, 0, wxALL, 2);
	SetSizerAndFit(main);
}

void FindResultDialog::SetHeader(const wxString &text)
{
	resultsCounter++;
	ReplacerResultsHeader *header = new ReplacerResultsHeader(text, resultsCounter);
	ResultsList->AppendItemWithExtent(header);
}

void FindResultDialog::SetResults(const wxString &text, const wxPoint &pos, TabPanel *_tab, int _keyLine, const wxString &_path)
{
	resultsCounter++;
	ReplacerSeekResults *results = new ReplacerSeekResults(text, pos, _tab, _keyLine, _path);
	ResultsList->AppendItemWithExtent(results);
}

void FindResultDialog::ClearList()
{
	resultsCounter = 0;
	ResultsList->ClearList();
}

void FindResultDialog::CheckUncheckAll(bool check /*= true*/)
{
	for (size_t i = 0; i < ResultsList->GetCount(); i++){
		Item *item = ResultsList->GetItem(i, 0);
		if (item){
			item->modified = check;
		}
	}
	ResultsList->Refresh(false);
}


