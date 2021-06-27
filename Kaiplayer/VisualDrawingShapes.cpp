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
#include "TabPanel.h"

enum {
	LEFT = 1,
	RIGHT,
	TOP = 4,
	BOTTOM = 8,
	INSIDE = 16,
	OUTSIDE = 32
};

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
	wxBoxSizer* modeSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* scalingModeSizer = new wxBoxSizer(wxHORIZONTAL);
	wxString modes[] = { _("Zmienna szerokoœæ i wysokoœæ"), _("Wysokoœæ jest równa szerokoœci"), _("Zmienna tylko szerokoœæ") };
	mode = new KaiChoice(this, -1, wxDefaultPosition, wxDefaultSize, 3, modes);
	wxString scalingModes[] = { _("Zmiana koordynatów rysunku"), _("zmiana skali") };
	scalingMode = new KaiChoice(this, -1, wxDefaultPosition, wxDefaultSize, 2, scalingModes);
	nameSizer->Add(new KaiStaticText(this, -1, _("Nazwa:")), 1, wxALL | wxEXPAND, 4);
	nameSizer->Add(shapeName, 1, wxALL | wxEXPAND, 4);
	modeSizer->Add(new KaiStaticText(this, -1, _("Skalowanie wzglêdem kursora:")), 1, wxALL | wxEXPAND, 4);
	modeSizer->Add(mode, 1, wxALL | wxEXPAND, 4);
	scalingModeSizer->Add(new KaiStaticText(this, -1, _("Rodzaj skalowania:")), 1, wxALL | wxEXPAND, 4);
	scalingModeSizer->Add(scalingMode, 1, wxALL | wxEXPAND, 4);

	editionSizer->Add(nameSizer, 1, wxALL | wxEXPAND, 2);
	editionSizer->Add(modeSizer, 1, wxALL | wxEXPAND, 2);
	editionSizer->Add(scalingModeSizer, 1, wxALL | wxEXPAND, 2);
	editionSizer->Add(new KaiStaticText(this, -1, _("Kszta³t:")), 0, wxALL | wxEXPAND, 6);
	editionSizer->Add(shapeAsASS, 0, wxALL | wxEXPAND, 6);

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

void ShapesEdition::OnAddShape(wxCommandEvent& evt)
{
	wxString newShapeNameStr = newShapeName->GetValue();
	if (newShapeNameStr.empty()) {
		KaiMessageBox(_("Wpisz nazwê nowego tagu."), _("B³¹d"), wxOK, this);
		return;
	}
	if (shapeList->FindString(newShapeNameStr) != -1) {
		KaiMessageBox(_("Nowy tag ju¿ istnieje na liœcie, wpisz inn¹ nazwê."), _("B³¹d"), wxOK, this);
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
		KaiMessageBox(L"Selected tag is out of range of shapeList.", L"Error", wxOK, this);
		return;
	}
	if (shapes.size() <= 1) {
		KaiMessageBox(_("Nie mo¿na usun¹æ wszystkich kszta³tów z listy"), _("B³¹d"), wxOK, this);
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
		if (KaiMessageBox(wxString::Format(_("Zapisaæ zmiany tagu \"%s\"?"),
			currentShape.shape), _("Potwierdzenie"), wxYES_NO, this) == wxYES) {
			Save(ID_BUTTON_COMMIT);
		}
	}
	SetShape(shapeList->GetSelection());
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

Shapes* Shapes::shapethis = NULL;

void Shapes::OnMouseEvent(wxMouseEvent& evt)
{
	if (!shape) {
		DrawingAndClip::OnMouseEvent(evt);
		return;
	}
	bool click = evt.LeftDown();
	bool holding = evt.LeftIsDown();

	int x, y;
	evt.GetPosition(&x, &y);
	if (evt.ButtonUp()) {
		if (tab->Video->HasCapture()) { tab->Video->ReleaseMouse(); }
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

		if (!tab->Video->HasArrow()) { tab->Video->SetCursor(wxCURSOR_ARROW); }
	}

	if (!holding && rectangleVisible) {

		bool setarrow = false;
		int test = HitTest(D3DXVECTOR2(x, y));
		if (test < INSIDE) {
			setarrow = true;
			tab->Video->SetCursor((test < 4) ? wxCURSOR_SIZEWE :
				(test >= 4 && test % 4 == 0) ? wxCURSOR_SIZENS :
				(test == (TOP + LEFT) || test == (BOTTOM + RIGHT)) ? wxCURSOR_SIZENWSE : wxCURSOR_SIZENESW);
		}
		if (!setarrow) { tab->Video->SetCursor(wxCURSOR_ARROW); }
	}
	if (click || evt.LeftDClick()) {
		if (!tab->Video->HasCapture()) { tab->Video->CaptureMouse(); }
		grabbed = OUTSIDE;
		float pointx = ((x / zoomScale.x) + zoomMove.x) * coeffW,
			pointy = ((y / zoomScale.y) + zoomMove.y) * coeffH;
		if (rectangleVisible) {
			grabbed = HitTest(D3DXVECTOR2(x, y), true);
			if (grabbed == INSIDE) {
				if (drawingRectangle[0].x <= pointx &&
					drawingRectangle[1].x >= pointx &&
					drawingRectangle[0].y <= pointy &&
					drawingRectangle[1].y >= pointy) {
					diffs.x = x;
					diffs.y = y;
				}
			}
		}
		if (!rectangleVisible || grabbed == OUTSIDE) {
			drawingRectangle[0].x = drawingRectangle[1].x = pointx;
			drawingRectangle[0].y = drawingRectangle[1].y = pointy;
			grabbed = OUTSIDE;
			rectangleVisible = true;
		}

	}
	else if (holding && grabbed != -1) {
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
			float pointx = ((x / zoomScale.x) + zoomMove.x) * coeffW,
				pointy = ((y / zoomScale.y) + zoomMove.y) * coeffH;
			drawingRectangle[1].x = pointx;
			drawingRectangle[1].y = pointy;
		}
		SetDrawingScale();
		if (rectangleVisible)
			SetClip(true);
		else
			tab->Video->Render(false);
	}
}

void Shapes::DrawVisual(int time)
{
	if (!shape) {
		DrawingAndClip::DrawVisual(time);
		return;
	}
}

void Shapes::SetShape(int curshape)
{
	if (!shapes->size() || curshape == shape)
		return;

	if (curshape <= 0)
		shape = -1;
	if (curshape > shapes->size()) {
		shape = shapes->size() - 1;
	}
	currentShape = (*shapes)[shape];
	GetVectorPoints(currentShape.shape, &points);
	shapeSize = CalcDrawingSize(alignment, &points, true);

	if (!points.empty()) {
		D3DXVECTOR2 offsetxy = CalcDrawingSize(alignment, &points);
		float rad = 0.01745329251994329576923690768489f;
		D3DXVECTOR2 orgpivot = { abs(org.x - _x), abs(org.y - _y) };
		float s = sin(-frz * rad);
		float c = cos(-frz * rad);
		for (size_t i = 0; i < points.size(); i++) {
			points[i].x -= offsetxy.x;
			points[i].y -= offsetxy.y;
			if (frz)
				RotateDrawing(&points[i], s, c, orgpivot);
		}
	}
}

void Shapes::GetVisual(wxString* drawing)
{
	if (!shape) {
		DrawingAndClip::GetVisual(drawing);
		return;
	}
	if (scale.x > 0.f && scale.y > 0.f) {
		wxString format = L"6.2f";
		wxString lasttype;
		int countB = 0;
		bool spline = false;
		size_t psize = points.size();
		std::vector<ClipPoint> originalPoints;

		if (frz) {
			float rad = 0.01745329251994329576923690768489f;
			D3DXVECTOR2 orgpivot = { abs(org.x - _x), abs(org.y - _y) };
			float s = sin(frz * rad);
			float c = cos(frz * rad);
			originalPoints = points;
			for (size_t i = 0; i < psize; i++) {
				RotateDrawing(&points[i], s, c, orgpivot);
			}
		}
		D3DXVECTOR2 offsetxy = CalcDrawingSize(alignment, &points);


		for (size_t i = 0; i < psize; i++)
		{
			ClipPoint pos = points[i];
			float x = currentShape.mode == ShapesSetting::CHANGE_POINTS ?
				(pos.x / scale.x) + offsetxy.x : pos.x + offsetxy.x;
			float y = currentShape.mode == ShapesSetting::CHANGE_POINTS ?
				(pos.y / scale.y) + offsetxy.y : pos.y + offsetxy.y;

			if (countB && !pos.start) {
				*drawing << getfloat(x, format) << L" " << getfloat(y, format) << L" ";
				countB++;
			}
			else {
				if (spline) {
					*drawing << L"c ";
					spline = false;
				}
				if (lasttype != pos.type || pos.type == L"m") {
					*drawing << pos.type << L" ";
					lasttype = pos.type;
				}
				*drawing << getfloat(x, format) << L" " << getfloat(y, format) << L" ";
				if (pos.type == L"b" || pos.type == L"s") {
					countB = 1;
					if (pos.type == L"s")
						spline = true;
				}
			}
			//fix for m one after another
			if (pos.type == L"m" && psize > 1 && ((i >= psize - 1) ||
				(i < psize - 1 && points[i + 1].type == L"m"))) {
				*drawing << L"l " << getfloat(x, format) << L" " << getfloat(y, format) << L" ";
			}
		}
		if (spline) { *drawing << L"c "; }
		drawing->Trim();
		if (originalPoints.size()) {
			points = originalPoints;
		}
	}
}

void Shapes::SetScale(wxString* txt, size_t position)
{
	SetFromTo(0, position);
	FindTag(L"fscx([0-9.-]+)", *txt, 3);
	Replace(L"\\fscx" + getfloat(scale.x), txt);
	FindTag(L"fscy([0-9.-]+)", *txt, 3);
	Replace(L"\\fscy" + getfloat(scale.y), txt);
}

int Shapes::HitTest(const D3DXVECTOR2& pos, bool diff)
{
	int resultX = 0, resultY = 0, resultInside = 0, resultFinal = 0, oldpointx = 0, oldpointy = 0;
	for (int i = 0; i < 2; i++) {
		float pointx = ((drawingRectangle[i].x / coeffW) - zoomMove.x) * zoomScale.x,
			pointy = ((drawingRectangle[i].y / coeffH) - zoomMove.y) * zoomScale.y;
		//bool hasResult = false;
		if (abs(pos.x - pointx) < 5) {
			if (diff) {
				diffs.x = pointx - pos.x;
			}
			resultX |= (i + 1);
		}
		if (abs(pos.y - pointy) < 5) {
			if (diff) {
				diffs.y = pointy - pos.y;
			}
			resultY |= ((i + 1) * 4);
		}
		if (i) {
			resultInside |= (resultX ||
				(oldpointx <= pointx && oldpointx <= pos.x && pointx >= pos.x) ||
				(oldpointx >= pointx && oldpointx >= pos.x && pointx <= pos.x)) ? INSIDE : OUTSIDE;
			resultInside |= (resultY ||
				(oldpointx <= pointx && oldpointy <= pos.y && pointy >= pos.y) ||
				(oldpointx >= pointx && oldpointy >= pos.y && pointy <= pos.y)) ? INSIDE : OUTSIDE;
		}
		else {
			oldpointx = pointx;
			oldpointy = pointy;
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
		
		if (rectx >= rectx1 + 10) {
			scale.x = 0.f;
		}
		else {
			float rectSizeX = rectx1 - rectx;
			scale.x = shapeSize.x / rectSizeX;
		}
		if (recty >= recty1 + 10) {
			scale.y = 0.f;
		}
		else {
			float rectSizeY = recty1 - recty;
			scale.y = shapeSize.y / rectSizeY;
		}

	}

}