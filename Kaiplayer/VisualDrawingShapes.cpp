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

#include "VisualDrawingShapes.h"
#include "KaiStaticBoxSizer.h"
#include "MappedButton.h"
#include "KaiStaticText.h"
#include "config.h"
#include "OpennWrite.h"
#include "KaiMessageBox.h"

ShapesEdition::ShapesEdition(wxWindow* parent, const wxPoint& pos, std::vector<ShapesSetting>* _shapes, int curShape)
	: KaiDialog(parent, -1, _("Edycja kszta³tów wektorowych"), pos)
{
	if (curShape < 0 || curShape >= _shapes->size())
		curShape = 0;

	selection = curShape;
	shapes = std::vector<ShapesSetting>(*_shapes);
	currentShape = shapes[selection];
	wxArrayString list;
	GetNames(&shapes, &list);
	DialogSizer* main = new DialogSizer(wxVERTICAL);
	KaiStaticBoxSizer* shapeSizer = new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Edytowany kszta³t"));
	shapeList = new KaiChoice(this, ID_SHAPE_LIST, wxDefaultPosition, wxDefaultSize, list);
	shapeList->SetSelection(selection);
	Bind(wxEVT_COMMAND_CHOICE_SELECTED, &ShapesEdition::OnListChanged, this, ID_SHAPE_LIST);
	newShapeName = new KaiTextCtrl(this, -1);
	MappedButton* addShape = new MappedButton(this, ID_BUTTON_ADD_SHAPE, _("Dodaj kszta³t"));
	MappedButton* removeShape = new MappedButton(this, ID_BUTTON_REMOVE_SHAPE, _("Usuñ kszta³t"));
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ShapesEdition::OnAddShape, this, ID_BUTTON_ADD_SHAPE);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ShapesEdition::OnRemoveShape, this, ID_BUTTON_REMOVE_SHAPE);
	shapeSizer->Add(shapeList, 1, wxALL | wxEXPAND, 4);
	shapeSizer->Add(newShapeName, 1, wxALL | wxEXPAND, 4);
	shapeSizer->Add(addShape, 1, wxALL | wxEXPAND, 4);
	shapeSizer->Add(removeShape, 1, wxALL | wxEXPAND, 4);
	KaiStaticBoxSizer* editionSizer = new KaiStaticBoxSizer(wxVERTICAL, this, _("Edycja"));
	wxBoxSizer* nameSizer = new wxBoxSizer(wxHORIZONTAL);
	shapeName = new KaiTextCtrl(this, -1, currentShape.name);
	shapeName->SetMaxLength(20);
	shapeAsASS = new KaiTextCtrl(this, -1, currentShape.shape, wxDefaultPosition, wxSize(-1, 300), wxTE_MULTILINE);
	nameSizer->Add(new KaiStaticText(this, -1, _("Nazwa:")), 1, wxALL | wxEXPAND, 4);
	nameSizer->Add(shapeName, 1, wxALL | wxEXPAND, 4);
	editionSizer->Add(nameSizer, 1, wxALL | wxEXPAND, 2);
	editionSizer->Add(new KaiStaticText(this, -1, _("Kszta³t:")), 0, wxALL | wxEXPAND, 6);
	editionSizer->Add(shapeAsASS, 0, wxALL | wxEXPAND, 2);
	//dorobiæ jeszcze mode i scale mode
	wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	MappedButton* commit = new MappedButton(this, ID_BUTTON_COMMIT, _("Zastosuj"));
	MappedButton* OK = new MappedButton(this, ID_BUTTON_OK, L"OK");
	MappedButton* cancel = new MappedButton(this, wxID_CANCEL, _("Anuluj"));
	MappedButton* resetDefault = new MappedButton(this, ID_BUTTON_RESET_DEFAULT, _("Przywróæ domyœlne"));
	buttonSizer->Add(commit, 1, wxALL, 4);
	buttonSizer->Add(OK, 1, wxALL, 4);
	buttonSizer->Add(cancel, 1, wxALL, 4);
	buttonSizer->Add(resetDefault, 1, wxALL, 4);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ShapesEdition::OnSave, this, ID_BUTTON_OK, ID_BUTTON_COMMIT);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ShapesEdition::OnResetDefault, this, ID_BUTTON_RESET_DEFAULT);
	main->Add(shapeSizer, 0, wxALL | wxEXPAND, 2);
	main->Add(editionSizer, 0, wxALL | wxEXPAND, 2);
	main->Add(buttonSizer, 0, wxALL | wxEXPAND, 2);
	SetSizerAndFit(main);
	CenterOnParent();
	SetEnterId(ID_BUTTON_OK);
	
}

void ShapesEdition::OnSave(wxCommandEvent& evt)
{
	Save(evt.GetId());
}

void ShapesEdition::OnResetDefault(wxCommandEvent& evt)
{
	if (KaiMessageBox(_("Czy na pewno chcesz przywróciæ ustawienia domyœlne?"),
		_("Potwierdzenie"), wxYES_NO, this) == wxYES) {
		wxString path = Options.pathfull + L"\\Config\\ShapesSettings.txt";
		_wremove(path.wc_str());
		shapes.clear();
		LoadSettings(&shapes);
		wxArrayString names;
		GetNames(&shapes, &names);
		shapeList->PutArray(&names);
		SetShape(shapeList->GetSelection());
	}
}

bool ShapesEdition::CheckModified()
{
	
	if (currentShape.name != shapeName->GetValue() ||
		currentShape.shape != shapeAsASS->GetValue() ||
		currentShape.mode != mode->GetSelection() ||
		currentShape.scalingMode != scalingMode->GetSelection())
	{
		return true;
	}
	return false;
}

void ShapesEdition::Save(int id)
{
	UpdateShape();
	if (currentShape.shape.empty()) {
		KaiMessageBox(_("Pole \"Tag\" nie mo¿e byæ puste."), _("B³¹d"), wxOK, this);
		return;
	}
	if (currentShape.name.empty()) {
		currentShape.name = _("Bez nazwy");
		shapeName->SetValue(currentShape.name);
	}
	if (selection < 0 || selection >= shapes.size()) {
		KaiMessageBox(L"Selected shape is out of range of tagList.", L"Error", wxOK, this);
		return;
	}
	shapes[selection] = currentShape;

	if (id == ID_BUTTON_OK) {
		SaveSettings(&shapes);
		EndModal(wxID_OK);
	}
}


void LoadSettings(std::vector<ShapesSetting>* shapes)
{
	wxString path = Options.pathfull + L"\\Config\\AllTagsSettings.txt";
	OpenWrite ow;
	wxString txtSettings;
	if (!ow.FileOpen(path, &txtSettings, false)) {
		//write entire setings in plain text
		//Shape: name, shape, shape mode, scale mode
		txtSettings = L"Shape: rectangle; m 0 0 l 100 0 100 100 0 100; 0; 0\n"\
			L"Shape: circle; m -100 -100 b -45 -155 45 -155 100 -100 b 155 -45 155 45 100 100 b 46 155 -45 155 -100 100 b -155 45 -155 -45 -100 -100; 1; 1\n"\
			L"Shape: rounded square 1; m -100 -25 b -100 -92 -92 -100 -25 -100 l 25 -100 b 92 -100 100 -92 100 -25 l 100 25 b 100 92 92 100 25 100 l -25 100 b -92 100 -100 92 -100 25 l -100 -25; 1; 1\n"\
			L"Shape: rounded square 2; m -100 -60 b -100 -92 -92 -100 -60 -100 l 60 -100 b 92 -100 100 -92 100 -60 l 100 60 b 100 92 92 100 60 100 l -60 100 b -92 100 -100 92 -100 60 l -100 -60; 1; 1\n"\
			L"Shape: rounded square 3; m -100 -85 b -100 -96 -96 -100 -85 -100 l 85 -100 b 96 -100 100 -96 100 -85 l 100 85 b 100 96 96 100 85 100 l -85 100 b -96 100 -100 96 -100 85 l -100 -85; 1; 1\n";
	}
	wxStringTokenizer tokenzer(txtSettings, "\n", wxTOKEN_STRTOK);
	while (tokenzer.HasMoreTokens()) {
		wxString token = tokenzer.GetNextToken();
		wxString shapetxt;
		if (token.StartsWith(L"Tag: ", &shapetxt)) {
			wxStringTokenizer tkzer(shapetxt, ",", wxTOKEN_STRTOK);
			ShapesSetting tmp;
			tmp.name = tkzer.GetNextToken();
			if (!tkzer.HasMoreTokens())
				continue;
			tmp.shape = tkzer.GetNextToken().Trim(false);
			long tmpval = 0;
			if (!tkzer.HasMoreTokens())
				continue;
			if (!tkzer.GetNextToken().Trim(false).ToLong(&tmpval))
				continue;
			tmp.mode = tmpval;
			if (!tkzer.HasMoreTokens())
				continue;
			if (!tkzer.GetNextToken().Trim(false).ToLong(&tmpval))
				continue;
			tmp.scalingMode = tmpval;

			shapes->push_back(tmp);
		}
	}
}

void GetNames(std::vector<ShapesSetting>* shapes, wxArrayString* nameList)
{
	for (size_t i = 0; i < shapes->size(); i++) {
		ShapesSetting shape = (*shapes)[i];
		nameList->Add(shape.name);
	}
}

void SaveSettings(std::vector<ShapesSetting>* shapes)
{
	wxString path = Options.pathfull + L"\\Config\\AllTagsSettings.txt";
	OpenWrite ow(path);
	for (size_t i = 0; i < shapes->size(); i++) {
		ShapesSetting shape = (*shapes)[i];
		wxString shapeText = L"Shape: ";
		shapeText << shape.name << "; " << shape.shape << "; " <<
			shape.mode << "; " <<
			shape.scalingMode << "\n";
		ow.PartFileWrite(shapeText);
	}
}