//  Copyright (c) 2016 - 2020, Marcin Drob

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

#include "Stylelistbox.h"
#include "config.h"
#include "KainoteFrame.h"
#include "KaiStaticBoxSizer.h"
#include "Notebook.h"
#include "TabPanel.h"
#include "SubsGrid.h"


Stylelistbox::Stylelistbox(wxWindow* parent, bool styles, int numelem, wxString *arr, const wxPoint& pos, int style)
	: KaiDialog(parent, -1, (styles) ? _("Wybór stylów") : _("Wybór kolumn"), wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER)
{
	DialogSizer *Main = new DialogSizer(wxVERTICAL);
	KaiStaticBoxSizer *sizer1 = new KaiStaticBoxSizer(wxVERTICAL, this, (styles) ? _("Wybierz style") : _("Wybierz kolumny"));
	wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
	CheckListBox = new KaiListCtrl(this, -1, numelem, arr, wxDefaultPosition, wxSize(200, 300), style);
	OK = new MappedButton(this, wxID_OK, L"OK");
	Cancel = new MappedButton(this, wxID_CANCEL, _("Anuluj"));
	sizer->Add(OK, 1, wxALL, 2);
	sizer->Add(Cancel, 1, wxALL, 2);
	sizer1->Add(CheckListBox, 0, wxEXPAND);
	Main->Add(sizer1, 0, wxEXPAND | wxALL, 2);
	Main->Add(sizer, 0, wxEXPAND | wxALL, 4);
	SetSizerAndFit(Main);
	MoveToMousePosition(this);
}

Stylelistbox::Stylelistbox(wxWindow* parent, const wxArrayString& arr, bool styles, const wxPoint& pos, int style)
	: KaiDialog(parent, -1, (styles) ? _("Wybór stylów") : _("Wybór kolumn"), wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER)
{
	DialogSizer* Main = new DialogSizer(wxVERTICAL);
	KaiStaticBoxSizer* sizer1 = new KaiStaticBoxSizer(wxVERTICAL, this, (styles) ? _("Wybierz style") : _("Wybierz kolumny"));
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	size_t arrsize = arr.size();
	wxString* theList = new wxString[arrsize];
	for (size_t i = 0; i < arrsize; i++) {
		theList[i] = arr[i];
	}
	CheckListBox = new KaiListCtrl(this, -1, arr.size(), theList, wxDefaultPosition, wxSize(200, 300), style);
	delete[] theList;
	OK = new MappedButton(this, wxID_OK, L"OK");
	Cancel = new MappedButton(this, wxID_CANCEL, _("Anuluj"));
	sizer->Add(OK, 1, wxALL, 2);
	sizer->Add(Cancel, 1, wxALL, 2);
	sizer1->Add(CheckListBox, 0, wxEXPAND);
	Main->Add(sizer1, 0, wxEXPAND | wxALL, 2);
	Main->Add(sizer, 0, wxEXPAND | wxALL, 4);
	SetSizerAndFit(Main);
	MoveToMousePosition(this);
}

Stylelistbox::~Stylelistbox()
{
}

wxString GetCheckedElements(wxWindow *parent)
{

	wxString styletext;
	wxString *elems;
	const std::vector<Styles*> *styles = Notebook::GetTab()->grid->file->GetStyleTable();
	elems = new wxString[styles->size()];
	for (size_t j = 0; j < styles->size(); j++){
		Styles *acstyl = (*styles)[j];
		elems[j] = acstyl->Name;
	}
	Stylelistbox slx(parent, true, styles->size(), elems);
	if (slx.ShowModal() == wxID_OK){

		for (size_t v = 0; v < slx.CheckListBox->GetCount(); v++)
		{

			if (slx.CheckListBox->GetItem(v, 0)->modified){
				styletext << slx.CheckListBox->GetItem(v, 0)->name << L",";
			}
		}
	}
	delete[] elems;
	return styletext.BeforeLast(L',');
}

CustomCheckListBox::CustomCheckListBox(wxWindow* parent, const wxArrayString &listElems, const wxString &title, const wxPoint& pos, int style)
	: KaiDialog(parent, -1, title, wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER)
{
	DialogSizer *Main = new DialogSizer(wxVERTICAL);
	KaiStaticBoxSizer *sizer1 = new KaiStaticBoxSizer(wxVERTICAL, this, title);
	wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
	size_t numelem = listElems.size();
	wxString *arr = new wxString[numelem];
	for (size_t i = 0; i < numelem; i++){
		arr[i] = listElems[i];
	}
	CheckListBox = new KaiListCtrl(this, -1, numelem, arr, wxDefaultPosition, wxSize(200, 300), style);
	delete[] arr;
	OK = new MappedButton(this, wxID_OK, L"Ok");
	Cancel = new MappedButton(this, wxID_CANCEL, _("Anuluj"));
	sizer->Add(OK, 1, wxALL, 2);
	sizer->Add(Cancel, 1, wxALL, 2);
	sizer1->Add(CheckListBox, 0, wxEXPAND);
	Main->Add(sizer1, 0, wxEXPAND | wxALL, 2);
	Main->Add(sizer, 0, wxEXPAND | wxALL, 4);
	SetSizerAndFit(Main);
	MoveToMousePosition(this);
}


void CustomCheckListBox::GetCheckedElements(wxArrayString &checkedElements)
{


	for (size_t v = 0; v < CheckListBox->GetCount(); v++)
	{

		if (CheckListBox->GetItem(v, 0)->modified){
			checkedElements.Add(CheckListBox->GetItem(v, 0)->name);
		}
	}

}


KaiListBox::KaiListBox(wxWindow *parent, const wxArrayString &items, const wxString &title, bool centerOnParent)
	: KaiDialog(parent, -1, title, wxDefaultPosition)
	, selection(0)
{
	DialogSizer *sizer = new DialogSizer(wxVERTICAL);
	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	list = new KaiListCtrl(this, 29886, items, wxDefaultPosition, wxSize(220, 160));
	list->SetSelection(0);
	MappedButton *OK = new MappedButton(this, 8888, L"OK");
	MappedButton *Cancel = new MappedButton(this, wxID_CANCEL, _("Anuluj"));
	sizer->Add(list, 1, wxEXPAND | wxALL, 2);
	buttonSizer->Add(OK, 1, wxALL, 4);
	buttonSizer->Add(Cancel, 1, wxALL, 4);
	sizer->Add(buttonSizer, 0, wxCENTER);
	SetSizerAndFit(sizer);
	SetEnterId(8888);

	Connect(29886, LIST_ITEM_DOUBLECLICKED, (wxObjectEventFunction)&KaiListBox::OnDoubleClick);
	Connect(8888, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&KaiListBox::OnOKClick);
	if (centerOnParent){ CenterOnParent();/*parent->Raise();*/ }
	else{
		MoveToMousePosition(this);
	}
}

void KaiListBox::OnDoubleClick(wxCommandEvent& evt)
{
	selection = evt.GetInt();
	result = list->GetItem(selection, 0)->name;
	EndModal(wxID_OK);
}

void KaiListBox::OnOKClick(wxCommandEvent& evt)
{
	selection = list->GetSelection();
	if (selection < 0){ selection = 0; }
	result = list->GetItem(selection, 0)->name;
	EndModal(wxID_OK);
}