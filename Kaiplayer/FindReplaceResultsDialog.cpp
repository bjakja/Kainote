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

FindReplaceResultsDialog::FindReplaceResultsDialog(wxWindow *parent, FindReplace *FR)
	: KaiDialog(parent, -1, _("Wyniki szukania"), wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER)
{
	DialogSizer * main = new DialogSizer(wxVERTICAL);
	resultsList = new KaiListCtrl(this, 23323, wxDefaultPosition, wxSize(700,300));
	resultsList->InsertColumn(0, "", TYPE_TEXT, -1);
	resultsList->SetHeaderHeight(0);
	main->Add(resultsList, 1, wxEXPAND | wxALL, 2);
	SetSizerAndFit(main);
	Bind(CHOOSE_RESULT, [=](wxCommandEvent &evt){
		SeekResults *results = (SeekResults*)evt.GetClientData();
		if (!results){
			wxLogStatus("chujnia, ktoœ rezultat wyszukiwania ukrad³");
			return;
		}

		FR->ShowResult(results->tab, results->path, results->keyLine);
	}, 23323);
}

FindReplaceResultsDialog::~FindReplaceResultsDialog()
{

}

void FindReplaceResultsDialog::SetHeader(const wxString &text)
{
	if (header)
		header->SetLastFilteredLine(resultsCounter);

	resultsCounter++;
	header = new ResultsHeader(text, resultsCounter);
	resultsList->AppendItemWithExtent(header);
}

void FindReplaceResultsDialog::SetResults(const wxString &text, const wxPoint &pos, TabPanel *_tab, int _keyLine, const wxString &_path)
{
	resultsCounter++;
	SeekResults *results = new SeekResults(text, pos, _tab, _keyLine, _path);
	resultsList->AppendItemWithExtent(results);
}

void FindReplaceResultsDialog::ClearList()
{
	resultsCounter = 0;
	resultsList->ClearList();
}

void FindReplaceResultsDialog::FilterList()
{
	//mode here is 1 visible blocks 0 hidden blocks
	if (header)
		header->SetLastFilteredLine(resultsCounter);
	resultsList->FilterList(0, 1);
}

void ResultsHeader::OnMouseEvent(wxMouseEvent &event, bool enter, bool leave, KaiListCtrl *theList, Item **changed /* = NULL */)
{
	if (event.LeftDown()){
		for (int i = firstFilteredLine; i < lastFilteredLine; i++){
			theList->FilterRow(i, (isVisible)? NOT_VISIBLE : VISIBLE_BLOCK);
		}
		isVisible = !isVisible;
	}
}

void ResultsHeader::OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, KaiListCtrl *theList)
{
	wxSize ex = theList->GetTextExtent(name);
	dc->SetTextForeground("#BB0099");
	dc->SetTextBackground("#440033");
	dc->SetBackgroundMode(wxSOLID);
	needTooltip = ex.x > width - 8;
	wxRect cur(x, y, width - 8, height);
	dc->SetClippingRegion(cur);
	dc->DrawLabel(name, cur, wxALIGN_CENTER_VERTICAL);
	dc->DestroyClippingRegion();
	dc->SetTextForeground(Options.GetColour(theList->IsThisEnabled() ? WindowText : WindowTextInactive));
	dc->SetBackgroundMode(wxTRANSPARENT);
}

void SeekResults::OnMouseEvent(wxMouseEvent &event, bool enter, bool leave, KaiListCtrl *theList, Item **changed /* = NULL */)
{
	if (event.LeftDClick()){
		//TODO If i write function for it call it here
		wxCommandEvent *evt = new wxCommandEvent(CHOOSE_RESULT, theList->GetId());
		evt->SetClientData(this);
		wxQueueEvent(theList->GetParent(), evt);
	}
}

void SeekResults::OnPaint(wxMemoryDC *dc, int x, int y, int width, int height, KaiListCtrl *theList)
{
	wxSize ex = theList->GetTextExtent(name);
	wxSize exOfFound = theList->GetTextExtent(name.Mid(0, findPosition.x));

	needTooltip = ex.x > width - 8;
	wxRect cur(x, y, width - 8, height);
	dc->SetClippingRegion(cur);
	dc->DrawLabel(name, cur, wxALIGN_CENTER_VERTICAL);
	dc->DestroyClippingRegion();
	dc->SetTextForeground("#FF0000");
	dc->SetTextBackground("#BBBB00");
	dc->SetBackgroundMode(wxSOLID);
	dc->DrawText(name.Mid(findPosition.x, findPosition.y), x + exOfFound.x, y + ((height - exOfFound.y) / 2));
	dc->SetBackgroundMode(wxTRANSPARENT);
	dc->SetTextForeground(Options.GetColour(theList->IsThisEnabled() ? WindowText : WindowTextInactive));
}
