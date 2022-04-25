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





#include <wx/msw/winundef.h>
#include "VisualDrawingShapes.h"
#include "KaiStaticBoxSizer.h"
#include "MappedButton.h"
#include "KaiStaticText.h"
#include "config.h"
#include "OpennWrite.h"
#include "KaiMessageBox.h"
#include "TabPanel.h"
#include "EditBox.h"
#include "VideoBox.h"
#include "Visuals.h"
#include "RendererVideo.h"
//

enum {
	LEFT = 1,
	RIGHT,
	TOP = 4,
	BOTTOM = 8,
	INSIDE = 16,
	OUTSIDE = 32
};

ShapesEdition::ShapesEdition(TabPanel* _tab, const wxPoint& pos, std::vector<ShapesSetting>* _shapes, int curShape)
	: KaiDialog(_tab, -1, _("Edycja kształtów wektorowych"), pos)
	, tab(_tab)
{
	if (curShape < 0 || curShape >= _shapes->size())
		curShape = 0;

	selection = curShape;
	shapes = std::vector<ShapesSetting>(*_shapes);
	currentShape = shapes[selection];
	wxArrayString list;
	GetNames(&shapes, &list);
	DialogSizer* main = new DialogSizer(wxVERTICAL);
	KaiStaticBoxSizer* shapeSizer = new KaiStaticBoxSizer(wxHORIZONTAL, this, _("Edytowany kształt"));
	shapeList = new KaiChoice(this, ID_SHAPE_LIST, wxDefaultPosition, wxDefaultSize, list);
	shapeList->SetSelection(selection);
	Bind(wxEVT_COMMAND_CHOICE_SELECTED, &ShapesEdition::OnListChanged, this, ID_SHAPE_LIST);
	newShapeName = new KaiTextCtrl(this, -1);
	MappedButton* addShape = new MappedButton(this, ID_BUTTON_ADD_SHAPE, _("Dodaj kształt"));
	MappedButton* removeShape = new MappedButton(this, ID_BUTTON_REMOVE_SHAPE, _("Usuń kształt"));
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
	wxBoxSizer* modeSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* scalingModeSizer = new wxBoxSizer(wxHORIZONTAL);
	wxString modes[] = { _("Zmienna szerokość i wysokość"), _("Zachowaj proporcje boków"), _("Zmienna tylko szerokość") };
	mode = new KaiChoice(this, -1, wxDefaultPosition, wxDefaultSize, 3, modes);
	mode->SetSelection(currentShape.mode);
	wxString scalingModes[] = { _("Zmiana koordynatów rysunku"), _("Zmiana skali") };
	scalingMode = new KaiChoice(this, -1, wxDefaultPosition, wxDefaultSize, 2, scalingModes);
	scalingMode->SetSelection(currentShape.scalingMode);
	scalingMode->Enable(false);
	nameSizer->Add(new KaiStaticText(this, -1, _("Nazwa:")), 1, wxALL | wxEXPAND, 4);
	nameSizer->Add(shapeName, 1, wxALL | wxEXPAND, 4);
	modeSizer->Add(new KaiStaticText(this, -1, _("Skalowanie względem kursora:")), 1, wxALL | wxEXPAND, 4);
	modeSizer->Add(mode, 1, wxALL | wxEXPAND, 4);
	scalingModeSizer->Add(new KaiStaticText(this, -1, _("Rodzaj skalowania:")), 1, wxALL | wxEXPAND, 4);
	scalingModeSizer->Add(scalingMode, 1, wxALL | wxEXPAND, 4);

	wxBoxSizer* shapeButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	MappedButton* getShapeFromLine = new MappedButton(this, ID_BUTTON_GET_SHAPE_FROM_LINE, _("Pobierz kształt z aktywnej linii"));
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ShapesEdition::OnGetShapeFromLine, this, ID_BUTTON_GET_SHAPE_FROM_LINE);
	editionSizer->Add(nameSizer, 1, wxALL | wxEXPAND, 2);
	editionSizer->Add(modeSizer, 1, wxALL | wxEXPAND, 2);
	editionSizer->Add(scalingModeSizer, 1, wxALL | wxEXPAND, 2);
	shapeButtonSizer->Add(new KaiStaticText(this, -1, _("Kształt:")), 1, wxALL | wxEXPAND, 4);
	shapeButtonSizer->Add(getShapeFromLine, 1, wxALL | wxEXPAND, 4);
	editionSizer->Add(shapeButtonSizer, 1, wxALL | wxEXPAND, 2);
	editionSizer->Add(shapeAsASS, 0, wxALL | wxEXPAND, 6);

	wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	MappedButton* commit = new MappedButton(this, ID_BUTTON_COMMIT, _("Zastosuj"));
	MappedButton* OK = new MappedButton(this, ID_BUTTON_OK, L"OK");
	MappedButton* cancel = new MappedButton(this, wxID_CANCEL, _("Anuluj"));
	MappedButton* resetDefault = new MappedButton(this, ID_BUTTON_RESET_DEFAULT, _("Przywróć domyślne"));
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
	if (KaiMessageBox(_("Czy na pewno chcesz przywrócić ustawienia domyślne?"),
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

void ShapesEdition::OnAddShape(wxCommandEvent& evt)
{
	wxString newShapeNameStr = newShapeName->GetValue();
	if (newShapeNameStr.empty()) {
		KaiMessageBox(_("Wpisz nazwę nowego ksztatłu."), _("Błąd"), wxOK, this);
		return;
	}
	if (shapeList->FindString(newShapeNameStr) != -1) {
		KaiMessageBox(_("Nowy ksztatł już istnieje na liście, wpisz inną nazwę."), _("Błąd"), wxOK, this);
		return;
	}
	currentShape = ShapesSetting(newShapeNameStr);
	shapes.push_back(currentShape);
	shapeList->Append(currentShape.name);
	selection = shapes.size() - 1;
	shapeList->SetSelection(selection);
	SetShapeFromSettings();
}

void ShapesEdition::OnRemoveShape(wxCommandEvent& evt)
{
	if (selection < 0 || selection >= shapes.size()) {
		KaiMessageBox(L"Selected shape is out of range of shapeList.", L"Error", wxOK, this);
		return;
	}
	if (shapes.size() <= 1) {
		KaiMessageBox(_("Nie można usunąć wszystkich kształtów z listy"), _("Błąd"), wxOK, this);
		return;
	}
	shapes.erase(shapes.begin() + selection);
	shapeList->Delete(selection);
	if (selection >= shapeList->GetCount()) {
		shapeList->SetSelection(shapeList->GetCount() - 1);
	}
	else
		shapeList->SetSelection(selection);

	selection = shapeList->GetSelection();
	//list is empty
	if (selection < 0) {
		currentShape = ShapesSetting();
	}
	else {
		currentShape = shapes[selection];
	}
	SetShapeFromSettings();
}

void ShapesEdition::OnListChanged(wxCommandEvent& evt)
{
	if (CheckModified()) {
		if (KaiMessageBox(wxString::Format(_("Zapisać zmiany kształtu \"%s\"?"),
			currentShape.shape), _("Potwierdzenie"), wxYES_NO, this) == wxYES) {
			Save(ID_BUTTON_COMMIT);
		}
	}
	SetShape(shapeList->GetSelection());
}

void ShapesEdition::OnGetShapeFromLine(wxCommandEvent& evt)
{
	wxString tags[] = { L"p" };
	ParseData* pdata = tab->edit->line->ParseTags(tags, 1);
	if (pdata->tags.size()) {
		for (size_t i = 0; i < pdata->tags.size(); i++) {
			TagData* tag = pdata->tags[i];
			if (tag->tagName == L"pvector") {
				//if shape exists then create a new
				if (!currentShape.shape.empty()) {
					currentShape = ShapesSetting();
					currentShape.name = _("Bez nazwy");
					currentShape.shape = tag->value;
					shapeName->SetValue(currentShape.name, true);
					shapes.push_back(currentShape);
					shapeList->Append(currentShape.name);
					selection = shapes.size() - 1;
					shapeList->SetSelection(selection);
				}
				else {
					currentShape.shape = tag->value;
				}
				shapeAsASS->SetValue(tag->value);
			}
		}
	}
}

void ShapesEdition::UpdateShape()
{
	currentShape.name = shapeName->GetValue();
	currentShape.shape = shapeAsASS->GetValue();
	currentShape.mode = mode->GetSelection();
	currentShape.scalingMode = scalingMode->GetSelection();
}

void ShapesEdition::SetShapeFromSettings()
{
	shapeName->SetValue(currentShape.name);
	shapeAsASS->SetValue(currentShape.shape);
	mode->SetSelection(currentShape.mode);
	scalingMode->SetSelection(currentShape.scalingMode);
}

void ShapesEdition::SetShape(int num)
{
	if (num < 0 || num >= shapes.size())
		num = 0;

	currentShape = shapes[num];
	selection = num;
	SetShapeFromSettings();
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
		KaiMessageBox(_("Pole \"kształt\" nie może być puste."), _("Błąd"), wxOK, this);
		return;
	}
	if (currentShape.name.empty()) {
		currentShape.name = _("Bez nazwy");
		shapeName->SetValue(currentShape.name);
	}
	if (selection < 0 || selection >= shapes.size()) {
		KaiMessageBox(L"Selected shape is out of range of shapeList.", L"Error", wxOK, this);
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
	wxString path = Options.pathfull + L"\\Config\\ShapesSettings.txt";
	OpenWrite ow;
	wxString txtSettings;
	if (!ow.FileOpen(path, &txtSettings, false)) {
		//write entire setings in plain text
		//Shape: name, shape, shape mode, scale mode
		txtSettings = L"Shape: rectangle; m 0 0 l 100 0 100 100 0 100; 0; 0\n"\
			L"Shape: circle; m -100 -100 b -45 -155 45 -155 100 -100 b 155 -45 155 45 100 100 b 46 155 -45 155 -100 100 b -155 45 -155 -45 -100 -100; 1; 0\n"\
			L"Shape: rounded square 1; m -100 -25 b -100 -92 -92 -100 -25 -100 l 25 -100 b 92 -100 100 -92 100 -25 l 100 25 b 100 92 92 100 25 100 l -25 100 b -92 100 -100 92 -100 25 l -100 -25; 1; 0\n"\
			L"Shape: rounded square 2; m -100 -60 b -100 -92 -92 -100 -60 -100 l 60 -100 b 92 -100 100 -92 100 -60 l 100 60 b 100 92 92 100 60 100 l -60 100 b -92 100 -100 92 -100 60 l -100 -60; 1; 0\n"\
			L"Shape: rounded square 3; m -100 -85 b -100 -96 -96 -100 -85 -100 l 85 -100 b 96 -100 100 -96 100 -85 l 100 85 b 100 96 96 100 85 100 l -85 100 b -96 100 -100 96 -100 85 l -100 -85; 1; 0\n";
	}
	wxStringTokenizer tokenzer(txtSettings, "\n", wxTOKEN_STRTOK);
	while (tokenzer.HasMoreTokens()) {
		wxString token = tokenzer.GetNextToken();
		wxString shapetxt;
		if (token.StartsWith(L"Shape: ", &shapetxt)) {
			wxStringTokenizer tkzer(shapetxt, ";", wxTOKEN_STRTOK);
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
		ShapesSetting shapeSetting = (*shapes)[i];
		nameList->Add(shapeSetting.name);
	}
}

void SaveSettings(std::vector<ShapesSetting>* shapes)
{
	wxString path = Options.pathfull + L"\\Config\\ShapesSettings.txt";
	OpenWrite ow(path);
	for (size_t i = 0; i < shapes->size(); i++) {
		ShapesSetting shapeSetting = (*shapes)[i];
		wxString shapeText = L"Shape: ";
		shapeText << shapeSetting.name << "; " << shapeSetting.shape << "; " <<
			(int)shapeSetting.mode << "; " <<
			(int)shapeSetting.scalingMode << "\n";
		ow.PartFileWrite(shapeText);
	}
}

Shapes::Shapes() {
	//shapes = VideoToolbar::GetShapesSettings();
};

void Shapes::OnMouseEvent(wxMouseEvent& evt)
{
	if (shape < 0) {
		DrawingAndClip::OnMouseEvent(evt);
		return;
	}
	bool click = evt.LeftDown();
	bool holding = evt.LeftIsDown();

	int x, y;
	evt.GetPosition(&x, &y);
	if (evt.ButtonUp()) {
		if (tab->video->HasCapture()) { tab->video->ReleaseMouse(); }
		if (rectangleVisible) {
			if (drawingRectangle[1].y == drawingRectangle[0].y ||
				drawingRectangle[1].x == drawingRectangle[0].x)
				rectangleVisible = false;

			SortPoints();
		}
		if (rectangleVisible) {
			SetDrawingScale();
			SetClip(false);
		}

		if (!tab->video->HasArrow()) { tab->video->SetCursor(wxCURSOR_ARROW); }
	}

	if (!holding && rectangleVisible) {

		bool setarrow = false;
		int test = HitTest(D3DXVECTOR2(x, y));
		if (test < INSIDE) {
			setarrow = true;
			tab->video->SetCursor((test < 4) ? wxCURSOR_SIZEWE :
				(test >= 4 && test % 4 == 0) ? wxCURSOR_SIZENS :
				(test == (TOP + LEFT) || test == (BOTTOM + RIGHT)) ? wxCURSOR_SIZENWSE : wxCURSOR_SIZENESW);
		}
		if (!setarrow) { tab->video->SetCursor(wxCURSOR_ARROW); }
	}
	if (click || evt.LeftDClick()) {
		if (!tab->video->HasCapture()) { tab->video->CaptureMouse(); }
		grabbed = OUTSIDE;
		D3DXVECTOR2 point = PointToSubtitles(x, y);
		if (rectangleVisible) {
			grabbed = HitTest(D3DXVECTOR2(x, y), true);
			if (grabbed == INSIDE) {
				if (drawingRectangle[0].x <= point.x &&
					drawingRectangle[1].x >= point.x &&
					drawingRectangle[0].y <= point.y &&
					drawingRectangle[1].y >= point.y) {
					diffs.x = x;
					diffs.y = y;
				}
			}
		}
		if (!rectangleVisible || grabbed == OUTSIDE) {
			drawingRectangle[0] = drawingRectangle[1] = point;
			grabbed = OUTSIDE;
			rectangleVisible = true;
		}

	}
	else if (holding && grabbed != -1) {
		bool axisX = true;
		if (grabbed < INSIDE) {
			if (grabbed & LEFT || grabbed & RIGHT) {
				x = MID(VideoSize.x, x, VideoSize.width);
				int posInTable = (grabbed & RIGHT) ? 1 : 0;
				drawingRectangle[posInTable].x =
					((((x + diffs.x) / zoomScale.x) + zoomMove.x) * coeffW);
				if (grabbed & LEFT && drawingRectangle[0].x > drawingRectangle[1].x) {
					drawingRectangle[0].x = drawingRectangle[1].x;
				}
				if (grabbed & RIGHT && drawingRectangle[1].x < drawingRectangle[0].x) {
					drawingRectangle[1].x = drawingRectangle[0].x;
				}
			}
			if (grabbed & TOP || grabbed & BOTTOM) {
				axisX = false;
				y = MID(VideoSize.y, y, VideoSize.height);
				int posInTable = (grabbed & BOTTOM) ? 1 : 0;
				drawingRectangle[posInTable].y =
					((((y + diffs.y) / zoomScale.y) + zoomMove.y) * coeffH);
				if (grabbed & TOP && drawingRectangle[0].y > drawingRectangle[1].y) {
					drawingRectangle[0].y = drawingRectangle[1].y;
				}
				if (grabbed & BOTTOM && drawingRectangle[1].y < drawingRectangle[0].y) {
					drawingRectangle[1].y = drawingRectangle[0].y;
				}
			}
		}
		else if (grabbed == INSIDE) {
			float movex = (((x - diffs.x) / zoomScale.x) * coeffW),
				movey = (((y - diffs.y) / zoomScale.y) * coeffH);
			drawingRectangle[0].x += movex;
			drawingRectangle[0].y += movey;
			drawingRectangle[1].x += movex;
			drawingRectangle[1].y += movey;
			diffs.x = x;
			diffs.y = y;
		}
		else if (grabbed == OUTSIDE) {
			drawingRectangle[1] = PointToSubtitles(x, y);
		}
		bool bothScaleX = currentShape.mode == ShapesSetting::BOTH_SCALE_X;
		if ((bothScaleX && !evt.ShiftDown() || !bothScaleX && evt.ShiftDown()) && grabbed != INSIDE) {
			SetSquareShape(axisX);
		}
		SetDrawingScale();
		if (rectangleVisible)
			SetClip(true);
		else
			tab->video->Render(false);
	}
}

void Shapes::DrawVisual(int time)
{
	if (shape < 0) {
		DrawingAndClip::DrawVisual(time);
		return;
	}
	if (rectangleVisible) {
		D3DXVECTOR2 point1 = PointToVideo(drawingRectangle[0]);
		D3DXVECTOR2 point2 = PointToVideo(drawingRectangle[1]);
		D3DXVECTOR2 v4[5];
		v4[0] = point1;
		v4[1].x = point2.x;
		v4[1].y = point1.y;
		v4[2] = point2;
		v4[3].x = point1.x;
		v4[3].y = point2.y;
		v4[4] = point1;
		line->Begin();
		line->Draw(v4, 5, 0xFFBB0000);
		line->End();
	}
}

//void Shapes::SetShape(int curshape)
//{
//	shapes = VideoToolbar::GetShapesSettings();
//	if (!shapes->size() || curshape - 1 == shape)
//		return;
//
//	if (curshape <= 0) {
//		shape = -1;
//		return;
//	}
//	else if (curshape > shapes->size()) {
//		shape = shapes->size() - 1;
//	}
//	else {
//		//first iteration is "choose" shapes starts from 1
//		shape = curshape - 1;
//	}
//	currentShape = (*shapes)[shape];
//	points.clear();
//	GetVectorPoints(currentShape.shape, &points);
//	shapeSize = CalcDrawingSize(alignment, &points, true);
//}
//
//void Shapes::GetVisual(wxString* drawing)
//{
//	if (shape < 0) {
//		DrawingAndClip::GetVisual(drawing);
//		return;
//	}
//	if (shapeScale.x > 0.f && rectangleVisible) {
//		wxString format = L"6.2f";
//		wxString lasttype;
//		int countB = 0;
//		bool spline = false;
//		size_t psize = points.size();
//		//nothing to do with frz cause drawing already rotated, cannot rotate 
//		//drawing only can make red rectangle to be more on drawing
//		//get -minx and -miny with scale
//		D3DXVECTOR2 offsetxy = CalcDrawingAnchor(alignment, &points);
//		//get rectangle x < x1
//		float rectx = drawingRectangle[0].x < drawingRectangle[1].x ?
//			drawingRectangle[0].x : drawingRectangle[1].x;
//		float recty = drawingRectangle[0].y < drawingRectangle[1].y ?
//			drawingRectangle[0].y : drawingRectangle[1].y;
//		float rectx1 = drawingRectangle[0].x > drawingRectangle[1].x ?
//			drawingRectangle[0].x : drawingRectangle[1].x;
//		float recty1 = drawingRectangle[0].y > drawingRectangle[1].y ?
//			drawingRectangle[0].y : drawingRectangle[1].y;
//		//subtract diff of position and rect
//		//position is already / by line scale
//		offsetxy.x -= ((_x - rectx) - 1);
//		offsetxy.y -= ((_y - recty) - 1);
//		//add diffs for alignments
//		if (alignment % 3 == 2) {
//			offsetxy.x += (rectx1 - rectx) / 2;
//		}
//		else if (alignment % 3 == 0) {
//			offsetxy.x += (rectx1 - rectx);
//		}
//		if (alignment < 4) {
//			offsetxy.y += (recty1 - recty);
//		}
//		else if (alignment < 7) {
//			offsetxy.y += (recty1 - recty) / 2;
//		}
//		
//		
//		//get drawing as string
//		for (size_t i = 0; i < psize; i++)
//		{
//			ClipPoint pos = points[i];
//			pos.x = (pos.x * shapeScale.x) + offsetxy.x;
//			pos.y = (pos.y * shapeScale.y) + offsetxy.y;
//
//			if (countB && !pos.start) {
//				*drawing << getfloat(pos.x, format) << L" " << getfloat(pos.y, format) << L" ";
//				countB++;
//			}
//			else {
//				if (spline) {
//					*drawing << L"c ";
//					spline = false;
//				}
//				if (lasttype != pos.type || pos.type == L"m") {
//					*drawing << pos.type << L" ";
//					lasttype = pos.type;
//				}
//				*drawing << getfloat(pos.x, format) << L" " << getfloat(pos.y, format) << L" ";
//				if (pos.type == L"b" || pos.type == L"s") {
//					countB = 1;
//					if (pos.type == L"s")
//						spline = true;
//				}
//			}
//			//fix for m one after another
//			if (pos.type == L"m" && psize > 1 && ((i >= psize - 1) ||
//				(i < psize - 1 && points[i + 1].type == L"m"))) {
//				*drawing << L"l " << getfloat(pos.x, format) << L" " << getfloat(pos.y, format) << L" ";
//			}
//		}
//		if (spline) { *drawing << L"c "; }
//		drawing->Trim();
//	}
//}

void Shapes::SetScale(wxString* txt, size_t position, int* diff)
{
	/*if (currentShape.scalingMode == ShapesSetting::CHANGE_SCALE && shapeScale.x > 0.f) {
		SetFromTo(0, position);
		FindTag(L"fscx([0-9.-]+)", *txt, 3);
		wxPoint textPos = GetPositionInText();
		wxString newTag = L"\\fscx" + getfloat(shapeScale.x * 100);
		Replace(newTag, txt);
		if (diff) {
			*diff = (textPos.y - textPos.x + 1) - newTag.length();
		}
		if (currentShape.mode != ShapesSetting::ONLY_SCALE_X) {
			FindTag(L"fscy([0-9.-]+)", *txt, 3);
			wxPoint textPos = GetPositionInText();
			wxString newTag = L"\\fscy" + getfloat(shapeScale.y * 100);
			Replace(newTag, txt);
			if (diff) {
				*diff += (textPos.y - textPos.x + 1) - newTag.length();
			}
		}
	}*/
}

int Shapes::HitTest(const D3DXVECTOR2& pos, bool diff)
{
	int resultX = 0, resultY = 0, resultInside = 0, resultFinal = 0, oldpointx = 0, oldpointy = 0;
	for (int i = 0; i < 2; i++) {
		D3DXVECTOR2 point = PointToVideo(drawingRectangle[i]);
		if (abs(pos.x - point.x) < 5) {
			if (diff) {
				diffs.x = point.x - pos.x;
			}
			resultX |= (i + 1);
		}
		if (abs(pos.y - point.y) < 5) {
			if (diff) {
				diffs.y = point.y - pos.y;
			}
			resultY |= ((i + 1) * 4);
		}
		if (i) {
			resultInside |= (resultX ||
				(oldpointx <= point.x && oldpointx <= pos.x && point.x >= pos.x) ||
				(oldpointx >= point.x && oldpointx >= pos.x && point.x <= pos.x)) ? INSIDE : OUTSIDE;
			resultInside |= (resultY ||
				(oldpointx <= point.x && oldpointy <= pos.y && point.y >= pos.y) ||
				(oldpointx >= point.x && oldpointy >= pos.y && point.y <= pos.y)) ? INSIDE : OUTSIDE;
		}
		else {
			oldpointx = point.x;
			oldpointy = point.y;
		}
	}

	resultFinal = (resultInside & OUTSIDE) ? OUTSIDE : INSIDE;
	if (resultFinal == INSIDE) {
		resultFinal |= resultX;
		resultFinal |= resultY;
		if (resultFinal > INSIDE) { resultFinal ^= INSIDE; }
	}
	return resultFinal;
}

void Shapes::SortPoints()
{
	if (drawingRectangle[1].y < drawingRectangle[0].y) {
		float tmpy = drawingRectangle[0].y;
		drawingRectangle[0].y = drawingRectangle[1].y;
		drawingRectangle[1].y = tmpy;
	}
	if (drawingRectangle[1].x < drawingRectangle[0].x) {
		float tmpx = drawingRectangle[0].x;
		drawingRectangle[0].x = drawingRectangle[1].x;
		drawingRectangle[1].x = tmpx;
	}
}

void Shapes::SetDrawingScale()
{
	if (rectangleVisible) {
		int an = alignment;
		//float bordery = border.y / 2;
		//float borderx = border.x / 2;
		//need to use rectangle[0] > rectangle[1]
		float rectx = drawingRectangle[0].x < drawingRectangle[1].x ?
			drawingRectangle[0].x : drawingRectangle[1].x;
		float recty = drawingRectangle[0].y < drawingRectangle[1].y ?
			drawingRectangle[0].y : drawingRectangle[1].y;
		float rectx1 = drawingRectangle[0].x > drawingRectangle[1].x ?
			drawingRectangle[0].x : drawingRectangle[1].x;
		float recty1 = drawingRectangle[0].y > drawingRectangle[1].y ?
			drawingRectangle[0].y : drawingRectangle[1].y;
		
		if (rectx >= rectx1 - 10) {
			shapeScale.x = 0.f;
		}
		else {
			float rectSizeX = rectx1 - rectx;
			shapeScale.x = rectSizeX / shapeSize.x;
		}
		if (recty >= recty1 - 10) {
			shapeScale.y = 0.f;
		}
		else {
			float rectSizeY = recty1 - recty;
			shapeScale.y = rectSizeY / shapeSize.y;
		}
	}

}

void Shapes::SetSquareShape(bool axisX)
{
	float rectx = drawingRectangle[0].x < drawingRectangle[1].x ?
		drawingRectangle[0].x : drawingRectangle[1].x;
	float rectx1 = drawingRectangle[0].x > drawingRectangle[1].x ?
		drawingRectangle[0].x : drawingRectangle[1].x;
	float recty = drawingRectangle[0].y < drawingRectangle[1].y ?
		drawingRectangle[0].y : drawingRectangle[1].y;
	float recty1 = drawingRectangle[0].y > drawingRectangle[1].y ?
		drawingRectangle[0].y : drawingRectangle[1].y;
	float aspectRatio = shapeSize.y == 0 ? 1 : shapeSize.x / shapeSize.y;
	if (axisX) {
		float height = recty + ((rectx1 - rectx) / aspectRatio);
		if (drawingRectangle[0].y > drawingRectangle[1].y) {
			drawingRectangle[0].y = height;
		}
		else {
			drawingRectangle[1].y = height;
		}
	}
	else {
		float width = rectx + ((recty1 - recty) * aspectRatio);
		if (drawingRectangle[0].x > drawingRectangle[1].x) {
			drawingRectangle[0].x = width;
		}
		else {
			drawingRectangle[1].x = width;
		}
	}
}

D3DXVECTOR2 Shapes::PointToVideo(const D3DXVECTOR2& point)
{
	float pointx = ((point.x / coeffW) - zoomMove.x) * zoomScale.x,
		pointy = ((point.y / coeffH) - zoomMove.y) * zoomScale.y;
	return D3DXVECTOR2(pointx, pointy);
}

D3DXVECTOR2 Shapes::PointToSubtitles(float x, float y)
{
	float pointx = ((x / zoomScale.x) + zoomMove.x) * coeffW,
		pointy = ((y / zoomScale.y) + zoomMove.y) * coeffH;
	return D3DXVECTOR2(pointx, pointy);
}

//D3DXVECTOR2 Shapes::CalcDrawingAnchor(int alignment, const std::vector<ClipPoint>* points)
//{
//	if (points->size() < 1) { return D3DXVECTOR2(0, 0); }
//
//	float minx = FLT_MAX;
//	float miny = FLT_MAX;
//	for (size_t i = 0; i < points->size(); i++)
//	{
//		ClipPoint p = points->at(i);
//		p.x *= shapeScale.x;
//		p.y *= shapeScale.y;
//		if (p.x < minx) { minx = p.x; }
//		if (p.y < miny) { miny = p.y; }
//	}
//	D3DXVECTOR2 result = D3DXVECTOR2(0, 0);
//	result.x = -(minx);
//	result.y = -(miny);
//	return result;
//}
