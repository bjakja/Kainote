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
#include "KainoteMain.h"


// header element of the results list
void ReplacerResultsHeader::OnMouseEvent(wxMouseEvent &event, bool enter, bool leave, KaiListCtrl *theList, Item **changed /* = NULL */)
{
	if (event.GetX() < 19 && event.LeftDown() || event.LeftDClick()){
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
		while(theList->GetType(i, 0) == TYPE_TEXT){
			theList->FilterRow(i, (isVisible) ? NOT_VISIBLE : VISIBLE_BLOCK);
			i++;
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

wxSize ReplacerResultsHeader::GetTextExtents(KaiListCtrl *theList){
	wxSize size = theList->GetTextExtent(name);
	size.x += 28;
	size.y += 4;
	return size;
}


// Seek results element
void ReplacerSeekResults::OnMouseEvent(wxMouseEvent &event, bool enter, bool leave, KaiListCtrl *theList, Item **changed /* = NULL */)
{
	if (event.GetX() < 19 && event.LeftDown() || event.LeftDClick()){
		modified = !modified;
		theList->Refresh(false);
		//here should be a function that will change state of header
	}
	else if (event.LeftDClick()){
		wxCommandEvent *evt = new wxCommandEvent(CHOOSE_RESULT, theList->GetId());
		evt->SetClientData(this);
		wxQueueEvent(theList->GetParent(), evt);
	}
}

void ReplacerSeekResults::OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, KaiListCtrl *theList)
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

wxSize ReplacerSeekResults::GetTextExtents(KaiListCtrl *theList){
	wxString lineNum = wxString::Format(_("Linia %i: "), idLine);
	wxString lineAndNum = lineNum + name;
	wxSize size = theList->GetTextExtent(lineAndNum);
	size.x += 28;
	size.y += 4;
	return size;
}


FindResultDialog::FindResultDialog(wxWindow *parent, MisspellReplacer *_MR)
	:KaiDialog(parent, -1, _("Wyniki szukania"), wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER)
	,MR(_MR)
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
	main->Add(buttonsSizer, 0, wxALL, 2);
	SetSizerAndFit(main);
}

void FindResultDialog::SetHeader(const wxString &text)
{
	resultsCounter++;
	ReplacerResultsHeader *header = new ReplacerResultsHeader(text, resultsCounter);
	ResultsList->AppendItemWithExtent(header);
}

void FindResultDialog::SetResults(const wxString &text, const wxPoint &pos, TabPanel *_tab, int _keyLine, int idLine, int numOfRule)
{
	resultsCounter++;
	ReplacerSeekResults *results = new ReplacerSeekResults(text, pos, _tab, _keyLine, idLine, numOfRule);
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


