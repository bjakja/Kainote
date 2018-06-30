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

#include "Visuals.h"
#include "TabPanel.h"
#include <wx/regex.h>

ScaleRotation::ScaleRotation()
	: Visuals()
	, angle(0, 0)
	, oldAngle(0, 0)
	, org(0, 0)
	, scale(0, 0)
	, lastOrg(0, 0)
	, beforeMove(0, 0)
	, afterMove(0, 0)
{

}

void ScaleRotation::DrawVisual(int time)
{
	if (time != oldtime && tbl[6] > 3){
		from = CalcMovePos();
		from.x = ((from.x / coeffW) - zoomMove.x)*zoomScale.x;
		from.y = ((from.y / coeffH) - zoomMove.y)*zoomScale.y;
		to = from;
		if (selectedTool != 0){
			if (org == from){
				org = from;
			}
			else{
				to = org;
			}
		}
	}
	if (selectedTool == 0)
		DrawScale(time);
	else if (selectedTool == 1)
		DrawRotationZ(time);
	else if (selectedTool == 2)
		DrawRotationXY(time);

}

void ScaleRotation::DrawScale(int time)
{

	D3DXVECTOR2 v4[15];
	int addy = (AN > 3) ? 60 : -60, addx = (AN % 3 == 0) ? -60 : 60;

	float movex = from.x + addx, movey = from.y + addy;

	if (type != 1){ movex = to.x; }//strza³ka w poziomie i czêœæ strza³ki po skosie
	else{ movex = from.x + (scale.x*addx); }
	if (type > 0){ movey = to.y; }//strza³ka w pionie i czêœæ strza³ki po skosie
	else{ movey = from.y + (scale.y*addy); }
	if (movex == from.x){ movex = from.x + addx; }
	else if (movey == from.y){ movey = from.y + addy; }

	lastmove.x = movex;
	lastmove.y = movey;
	v4[0] = from;//strza³ka pozioma
	v4[1].x = movex;
	v4[1].y = from.y;//strza³ka pozioma
	v4[2] = from;//strza³ka skoœna
	v4[3].x = movex;
	v4[3].y = movey;//strza³ka skoœna
	v4[4] = from;//strza³ka pionowa
	v4[5].x = from.x;
	v4[5].y = movey;//strza³ka pionowa

	for (int i = 1; i < 6; i += 2){
		DrawArrow(v4[0], &v4[i]);
	}

	line->Begin();
	line->Draw(v4, 2, 0xFFBB0000);
	line->End();
	line->Begin();
	line->Draw(&v4[2], 2, 0xFFBB0000);
	line->End();
	line->Begin();
	line->Draw(&v4[4], 2, 0xFFBB0000);
	line->End();
}

void ScaleRotation::DrawRotationZ(int time)
{
	float rad = 0.01745329251994329576923690768489f;
	float radius = sqrt(pow(abs(org.x - from.x), 2) + pow(abs(org.y - from.y), 2)) + 40;
	D3DXVECTOR2 v2[6];
	VERTEX v5[726];
	CreateVERTEX(&v5[0], org.x, org.y + (radius + 10.f), 0xAA121150);
	CreateVERTEX(&v5[1], org.x, org.y + radius, 0xAA121150);
	for (int j = 0; j < 181; j++){
		float xx = org.x + ((radius + 10.f) * sin((j * 2) * rad));
		float yy = org.y + ((radius + 10.f) * cos((j * 2) * rad));
		float xx1 = org.x + (radius * sin((j * 2) * rad));
		float yy1 = org.y + (radius * cos((j * 2) * rad));
		CreateVERTEX(&v5[j + 364], xx, yy, 0xAAFF0000);
		CreateVERTEX(&v5[j + 545], xx1, yy1, 0xAAFF0000);
		if (j < 1){ continue; }
		CreateVERTEX(&v5[(j * 2)], xx, yy, 0xAA121150);
		CreateVERTEX(&v5[(j * 2) + 1], xx1, yy1, 0xAA121150);

	}
	if (radius){
		float xx1 = org.x + ((radius - 40) * sin(lastmove.y * rad));
		float yy1 = org.y + ((radius - 40) * cos(lastmove.y * rad));
		v2[0].x = xx1 - 5.0f;
		v2[0].y = yy1;
		v2[1].x = xx1 + 5.0f;
		v2[1].y = yy1;
		v2[2] = org;
		v2[3].x = xx1;
		v2[3].y = yy1;
		float xx2 = xx1 + (radius * sin((lastmove.y + 90) * rad));
		float yy2 = yy1 + (radius * cos((lastmove.y + 90) * rad));
		float xx3 = xx1 + (radius * sin((lastmove.y - 90) * rad));
		float yy3 = yy1 + (radius * cos((lastmove.y - 90) * rad));
		v2[4].x = xx2;
		v2[4].y = yy2;
		v2[5].x = xx3;
		v2[5].y = yy3;
		line->SetWidth(10.f);
		line->Begin();
		line->Draw(v2, 2, 0xAAFF0000);
		line->End();
		line->SetWidth(2.f);
		line->Begin();
		line->Draw(&v2[2], 2, 0xFFBB0000);
		line->Draw(&v2[4], 2, 0xFFBB0000);
		line->End();
	}
	v2[0] = org;
	v2[1] = to;
	v2[2].x = org.x - 10.0f;
	v2[2].y = org.y;
	v2[3].x = org.x + 10.0f;
	v2[3].y = org.y;
	v2[4].x = org.x;
	v2[4].y = org.y - 10.0f;
	v2[5].x = org.x;
	v2[5].y = org.y + 10.0f;
	line->SetWidth(5.f);

	HRN(device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 360, v5, sizeof(VERTEX)), "primitive failed");
	HRN(device->DrawPrimitiveUP(D3DPT_LINESTRIP, 180, &v5[364], sizeof(VERTEX)), "primitive failed");
	HRN(device->DrawPrimitiveUP(D3DPT_LINESTRIP, 180, &v5[545], sizeof(VERTEX)), "primitive failed");
	line->SetWidth(2.f);
	line->Begin();
	line->Draw(&v2[2], 2, 0xFFBB0000);
	line->End();
	line->Begin();
	line->Draw(&v2[4], 2, 0xFFBB0000);
	line->End();
	line->Begin();
	line->Draw(&v2[0], 2, 0xFFBB0000);
	line->End();
}

void ScaleRotation::DrawRotationXY(int time)
{
	wxSize s = VideoSize.GetSize();
	float ratio = (float)s.x / (float)s.y;
	float xxx = ((org.x / s.x) * 2) - 1;
	float yyy = ((org.y / s.y) * 2) - 1;
	D3DXMATRIX mat;
	D3DXMATRIX matRotate;    // a matrix to store the rotation information
	D3DXMATRIX matTramsate;

	D3DXMatrixRotationYawPitchRoll(&matRotate, D3DXToRadian(-angle.x), D3DXToRadian(angle.y), 0);
	if (from != org){
		float txx = ((from.x / s.x) * 60) - 30;
		float tyy = ((from.y / s.y) * 60) - 30;
		D3DXMatrixTranslation(&matTramsate, txx - (xxx), -(tyy - (yyy * 30)), 0.0f);
		matRotate = matTramsate*matRotate;
	}

	D3DXMATRIX matView;    // the view transform matrix

	D3DXMatrixLookAtLH(&matView,
		&D3DXVECTOR3(0.0f, 0.0f, -17.2f),    // the camera position default -17.2
		&D3DXVECTOR3(0.0f, 0.0f, 0.0f),    // the look-at position
		&D3DXVECTOR3(0.0f, 1.0f, 0.0f));    // the up direction

	device->SetTransform(D3DTS_VIEW, &matView);    // set the view transform to matView

	D3DXMATRIX matProjection;     // the projection transform matrix

	D3DXMatrixPerspectiveFovLH(&matProjection,
		D3DXToRadian(120),    // the horizontal field of view default 120
		ratio, // aspect ratio
		1.0f,    // the near view-plane
		10000.0f);    // the far view-plane

	D3DXMatrixTranslation(&matTramsate, xxx, -yyy, 0.0f);
	device->SetTransform(D3DTS_PROJECTION, &(matProjection*matTramsate));    // set the projection

	device->SetTransform(D3DTS_WORLD, &matRotate);

	device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	VERTEX vertices[199];
	bool ster = true;

	float mm = 60.0f / 12.0f;
	float j = 30 - mm;
	float gg = 1.0f / 12.f;
	float g = gg;
	int re = 122, gr = 57, bl = 36;

	for (int i = 0; i < 44; i += 4)
	{
		if (i == 20){ re = 255; gr = 155; }
		else{ re = 122; gr = 57; }
		CreateVERTEX(&vertices[i], j, -30.f, D3DCOLOR_ARGB((int)(g * 155), re, gr, bl));
		CreateVERTEX(&vertices[i + 1], j, 0.f, D3DCOLOR_ARGB((int)(255), re, gr, bl));
		CreateVERTEX(&vertices[i + 2], j, 0.f, D3DCOLOR_ARGB((int)(255), re, gr, bl));
		CreateVERTEX(&vertices[i + 3], j, 30.f, D3DCOLOR_ARGB((int)(g * 155), re, gr, bl));
		CreateVERTEX(&vertices[i + 44], -30.f, j, D3DCOLOR_ARGB((int)(g * 155), re, gr, bl));
		CreateVERTEX(&vertices[i + 45], 0.f, j, D3DCOLOR_ARGB((int)(255), re, gr, bl));
		CreateVERTEX(&vertices[i + 46], 0.f, j, D3DCOLOR_ARGB((int)(255), re, gr, bl));
		CreateVERTEX(&vertices[i + 47], 30.f, j, D3DCOLOR_ARGB((int)(g * 155), re, gr, bl));
		j -= mm;
		if (g == 1.f){ ster = false; }
		if (ster){
			g += gg;
		}
		else{
			g -= gg;
		}
	}
	device->DrawPrimitiveUP(D3DPT_LINELIST, 44, vertices, sizeof(VERTEX));
	float addy = (AN < 4) ? 9.f : -9.f, addx = (AN % 3 == 0) ? -9.f : 9.f;
	float add1y = (AN < 4) ? 10.f : -10.f, add1x = (AN % 3 == 0) ? -10.f : 10.f;
	CreateVERTEX(&vertices[176], 0.f, addy, 0xFFBB0000);//line y
	CreateVERTEX(&vertices[177], 0.f, 0.f, 0xFFBB0000);
	CreateVERTEX(&vertices[178], addx, 0.f, 0xFFBB0000); //line x
	CreateVERTEX(&vertices[179], 0.f, 0.f, 0xFFBB0000); //line z
	CreateVERTEX(&vertices[180], 0.f, 0.f, 0xFFBB0000, 9.f);
	CreateVERTEX(&vertices[181], 0.f, add1y, 0xFFBB0000); //arrow y
	CreateVERTEX(&vertices[182], 0.f, addy, 0xFFBB0000, -0.6f);
	CreateVERTEX(&vertices[183], -0.6f, addy, 0xFFBB0000);
	CreateVERTEX(&vertices[184], 0.f, addy, 0xFFBB0000, 0.6f);
	CreateVERTEX(&vertices[185], 0.6f, addy, 0xFFBB0000);
	CreateVERTEX(&vertices[186], 0.f, addy, 0xFFBB0000, -0.6f);
	CreateVERTEX(&vertices[187], add1x, 0.f, 0xFFBB0000);//arrow x
	CreateVERTEX(&vertices[188], addx, 0.f, 0xFFBB0000, -0.6f);
	CreateVERTEX(&vertices[189], addx, -0.6f, 0xFFBB0000);
	CreateVERTEX(&vertices[190], addx, 0.f, 0xFFBB0000, 0.6f);
	CreateVERTEX(&vertices[191], addx, 0.6f, 0xFFBB0000, 0.f);
	CreateVERTEX(&vertices[192], addx, 0.f, 0xFFBB0000, -0.6f);
	CreateVERTEX(&vertices[193], 0.f, 0.f, 0xFFBB0000, 10.f); //arrow z
	CreateVERTEX(&vertices[194], -0.6f, 0.f, 0xFFBB0000, 9.f);
	CreateVERTEX(&vertices[195], 0.f, 0.6f, 0xFFBB0000, 9.f);
	CreateVERTEX(&vertices[196], 0.6f, 0.f, 0xFFBB0000, 9.f);
	CreateVERTEX(&vertices[197], 0.f, -0.6f, 0xFFBB0000, 9.f);
	CreateVERTEX(&vertices[198], -0.6f, 0.f, 0xFFBB0000, 9.f);
	device->DrawPrimitiveUP(D3DPT_LINESTRIP, 2, &vertices[176], sizeof(VERTEX));
	device->DrawPrimitiveUP(D3DPT_LINELIST, 1, &vertices[179], sizeof(VERTEX));
	device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 4, &vertices[181], sizeof(VERTEX));
	device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 4, &vertices[187], sizeof(VERTEX));
	device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 4, &vertices[193], sizeof(VERTEX));
	D3DXMATRIX matOrtho;
	D3DXMATRIX matIdentity;

	D3DXMatrixOrthoOffCenterLH(&matOrtho, 0, s.x, s.y, 0, 0.0f, 1.0f);
	D3DXMatrixIdentity(&matIdentity);

	HRN(device->SetTransform(D3DTS_PROJECTION, &matOrtho), _("Nie mo¿na ustawiæ macierzy projekcji"));
	HRN(device->SetTransform(D3DTS_WORLD, &matIdentity), _("Nie mo¿na ustawiæ macierzy œwiata"));
	HRN(device->SetTransform(D3DTS_VIEW, &matIdentity), _("Nie mo¿na ustawiæ macierzy widoku"));
	D3DXVECTOR2 v2[4];
	v2[0].x = org.x - 10.0f;
	v2[0].y = org.y;
	v2[1].x = org.x + 10.0f;
	v2[1].y = org.y;
	v2[2].x = org.x;
	v2[2].y = org.y - 10.0f;
	v2[3].x = org.x;
	v2[3].y = org.y + 10.0f;
	line->Begin();
	line->Draw(&v2[0], 2, 0xFFBB0000);
	line->End();
	line->Begin();
	line->Draw(&v2[2], 2, 0xFFBB0000);
	line->End();
}


void ScaleRotation::OnMouseEvent(wxMouseEvent &evt)
{
	if (blockevents){ return; }
	bool click = evt.LeftDown();
	bool holding = evt.LeftIsDown();
	bool leftc = evt.LeftDown();
	bool rightc = evt.RightDown();
	bool middlec = evt.MiddleDown();

	int x, y;
	evt.GetPosition(&x, &y);

	if (evt.ButtonUp()){
		if (tab->Video->HasCapture()){ tab->Video->ReleaseMouse(); }
		ChangeInLines(false);
		if (!hasArrow){ tab->Video->SetCursor(wxCURSOR_ARROW); hasArrow = true; }
	}

	if (click){
		tab->Video->CaptureMouse();
		if (selectedTool == 0)
			OnClickScaling(x, y, leftc, rightc, middlec, evt.ShiftDown());
		else if (selectedTool == 1)
			OnClickRotationZ(x, y);
		else if (selectedTool == 2)
			OnClickRotationXY(x, y, leftc, rightc, middlec);
	}
	else if (holding){
		if (selectedTool == 0)
			OnHoldingScaling(x, y, evt.ShiftDown());
		else
			OnHoldingRotation(x, y);
	}
}

void ScaleRotation::OnClickRotationZ(int x, int y)
{
	tab->Video->SetCursor(wxCURSOR_SIZING);
	hasArrow = false;
	if (abs(org.x - x) < 8 && abs(org.y - y) < 8){
		isOrg = true;
		lastOrg = org;
		diffs.x = org.x - x;
		diffs.y = org.y - y;
		return;
	}
	else{
		lastmove.x = atan2((org.y - y), (org.x - x)) * (180.f / 3.1415926536f);
		lastmove.x += lastmove.y;
	}
}

void ScaleRotation::OnClickRotationXY(int x, int y, bool leftClick, bool rightClick, bool middleClick)
{
	if (leftClick){ type = 0; tab->Video->SetCursor(wxCURSOR_SIZEWE); }//fry
	if (rightClick){ type = 1; tab->Video->SetCursor(wxCURSOR_SIZENS); }//frx
	if (middleClick){ type = 2; tab->Video->SetCursor(wxCURSOR_SIZING); }//frx + fry
	if (abs(org.x - x) < 8 && abs(org.y - y) < 8){
		isOrg = true;
		lastOrg = org;
		diffs.x = org.x - x;
		diffs.y = org.y - y;
	}
	firstmove = D3DXVECTOR2(x, y);
	/*if (type == 0){ tab->Video->SetCursor(wxCURSOR_SIZEWE); }
	if (type == 1){ tab->Video->SetCursor(wxCURSOR_SIZENS); }
	if (type == 2){ tab->Video->SetCursor(wxCURSOR_SIZING); }*/
	hasArrow = false;
}

void ScaleRotation::OnClickScaling(int x, int y, bool leftClick, bool rightClick, bool middleClick, bool shiftDown)
{
	int grabbed = -1;
	if (leftClick){ type = 0; }//fscx
	if (rightClick){ type = 1; }//fscy
	if (middleClick || leftClick && shiftDown){ type = 2; }//fscx + fscy
	if (abs(lastmove.x - x) < 8 && abs(from.y - y) < 8){ grabbed = 0; type = 0; }
	else if (abs(lastmove.y - y) < 8 && abs(from.x - x) < 8){ grabbed = 1; type = 1; }
	else if (abs(lastmove.x - x) < 8 && abs(lastmove.y - y) < 8){ grabbed = 2; type = 2; }
	diffs.x = lastmove.x - x;
	diffs.y = lastmove.y - y;
	if (type == 0){ tab->Video->SetCursor(wxCURSOR_SIZEWE); }
	if (type == 1){ tab->Video->SetCursor(wxCURSOR_SIZENS); }
	if (type == 2){ tab->Video->SetCursor(wxCURSOR_SIZING); }
	hasArrow = false;
	int addy = (AN > 3) ? 60 : -60, addx = (AN % 3 == 0) ? -60 : 60;
	if (leftClick && shiftDown){
		type = 2;
		diffs.x = x;
		diffs.y = y;
		to.x = from.x;
		to.y = from.y;
		to.x += (addx*scale.x);
		to.y += (addy*scale.y);
		return;
	}
	if (grabbed == -1){

		diffs.x = (from.x - x) + (addx*scale.x);
		diffs.y = (from.y - y) + (addy*scale.y);
	}
	to.x = x; to.y = y;

}

void ScaleRotation::OnHoldingRotation(int x, int y)
{
	if (isOrg){
		org.x = x + diffs.x;
		org.y = y + diffs.y;

		ChangeInLines();
		return;
	}
	to.x = x; to.y = y;
	ChangeInLines();
}

void ScaleRotation::OnHoldingScaling(int x, int y, bool hasShift)
{
	if (hasShift){
		//zamieniamy te wartoœci by przesuwanie w osi x dzia³a³o poprawnie, a nie na odwrót.
		//drgawki nawet w photoshopie wystêpuj¹ bo wtedy jedna oœ ma + druga - i w zale¿noœci od tego która jest u¿yta
		//zwiêksza b¹dŸ zmniejsza nam tekst/rysunek.
		int diffx = abs(x - diffs.x);
		int diffy = abs(diffs.y - y);
		int move = (diffx > diffy) ? x - diffs.x : diffs.y - y;

		D3DXVECTOR2 copyto = to;
		wxPoint copydiffs = diffs;
		bool an3 = AN % 3 == 0;
		if (AN > 3 && !an3)
			to.x = to.x - move;
		else
			to.x = to.x + move;

		to.y = to.y - move;
		diffs.x = x;
		diffs.y = y;
		if ((!an3 && to.x - from.x<1) || (an3 && to.x - from.x>-1)){
			diffs = copydiffs;
			to = copyto;
		}
	}
	else{
		if (type != 1){
			to.x = x + diffs.x;
		}
		if (type != 0){
			to.y = y + diffs.y;
		}
	}

	ChangeInLines();
}

void ScaleRotation::SetCurVisual()
{
	D3DXVECTOR2 linepos = GetPosnScale(&scale, &AN, tbl);
	if (tbl[6] > 3){ linepos = CalcMovePos(); }
	from = D3DXVECTOR2(((linepos.x / coeffW) - zoomMove.x)*zoomScale.x,
		((linepos.y / coeffH) - zoomMove.y)*zoomScale.y);

	if (selectedTool == 0){//scale
		int addy = (AN > 3) ? 60 : -60, addx = (AN % 3 == 0) ? -60 : 60;
		to.x = from.x + (scale.x*addx);
		to.y = from.y + (scale.y*addy);
	}
	else if (selectedTool == 1){//rotationz
		lastmove = D3DXVECTOR2(0, 0);
		wxString res;
		if (tab->Edit->FindVal("frz?([0-9.-]+)", &res)){
			double result = 0; res.ToDouble(&result);
			lastmove.y = result;
			lastmove.x += lastmove.y;
		}
		if (tab->Edit->FindVal("org\\(([^\\)]+)", &res)){
			wxString rest;
			double orx, ory;
			if (res.BeforeFirst(',', &rest).ToDouble(&orx)){ org.x = ((orx / coeffW) - zoomMove.x)*zoomScale.x; }
			if (rest.ToDouble(&ory)){ org.y = ((ory / coeffH) - zoomMove.y)*zoomScale.y; }
		}
		else{ org = from; }
		to = org;
	}
	else if (selectedTool == 2){//rotationxy
		wxString res;
		oldAngle = D3DXVECTOR2(0, 0);
		if (tab->Edit->FindVal("frx([^\\\\}]+)", &res)){
			double result = 0; res.ToDouble(&result);
			oldAngle.y = result;
		}
		if (tab->Edit->FindVal("fry([^\\\\}]+)", &res)){
			double result = 0; res.ToDouble(&result);
			oldAngle.x = result;
		}
		if (tab->Edit->FindVal("org\\(([^\\)]+)", &res)){
			wxString rest;
			double orx, ory;
			if (res.BeforeFirst(',', &rest).ToDouble(&orx)){ org.x = ((orx / coeffW) - zoomMove.x)*zoomScale.x; }
			if (rest.ToDouble(&ory)){ org.y = ((ory / coeffH) - zoomMove.y)*zoomScale.y; }
		}
		else{ org = from; }
		firstmove = to;
		angle = oldAngle;
		lastmove = org;
	}

}

void ScaleRotation::ChangeInLines(bool dummy)
{
	D3DXVECTOR2 moving = afterMove - beforeMove;
	int _time = tab->Video->Tell();
	wxArrayInt sels;
	tab->Grid->file->GetSelections(sels);
	wxString *dtxt;
	if (dummy){
		if (!dummytext){
			selPositions.clear();
			bool visible = false;
			dummytext = tab->Grid->GetVisible(&visible, 0, &selPositions);
			if (selPositions.size() != sels.size()){
				KaiLog("Sizes mismatch");
				return;
			}
		}

		dtxt = new wxString(*dummytext);
	}
	bool skipInvisible = dummy && tab->Video->GetState() != Playing;
	wxString tmp;

	const wxString &tlModeStyle = tab->Grid->GetSInfo("TLMode Style");
	int moveLength = 0;
	if (isOrg && selectedTool < 1)
		isOrg = false;

	wxString typeString = (type == 0) ? L"x" : (type == 1) ? L"y" : L"([xy])";

	wxString tagpattern = (selectedTool == 0) ? wxString::Format(L"fsc%s([0-9.-]+)", typeString) :
		(isOrg) ? L"org\\(([0-9.-]+)\\,([0-9.-]+)\\)" : (selectedTool == 1) ? L"frz?([0-9.-]+)" : 
		wxString::Format(L"fr%s([0-9.-]+)", typeString);

	wxRegEx re(L"\\\\" + tagpattern, wxRE_ADVANCED);
	if (!re.IsValid()){
		KaiLog("Bad pattern " + tagpattern);
	}

	for (size_t i = 0; i < sels.size(); i++){
		wxString txt;
		Dialogue *Dial = tab->Grid->GetDialogue(sels[i]);

		if (skipInvisible && !(_time >= Dial->Start.mstime && _time <= Dial->End.mstime)){ continue; }
		bool istexttl = (tab->Grid->hasTLMode && Dial->TextTl != "");
		txt = (istexttl) ? Dial->TextTl : Dial->Text;
		
		size_t startMatch = 0, lenMatch = 0;
		size_t textPosition = 0;
		while (re.Matches(txt.Mid(textPosition))){
			wxString changedValue;
			tmp = re.GetMatch(txt, (type == 2 && !isOrg)? 2 : 1);
			
			double value=0.0;
			if (tmp.ToDouble(&value)){
				if (isOrg){
					changedValue << (value + moving.x) << L",";
					wxString tmp2 = re.GetMatch(txt, 2);
					if (tmp2.ToDouble(&value)){
						changedValue << (value + moving.y);
					}
					else
						changedValue = "";
				}
				else if (type == 0){ changedValue << (value + moving.x); }
				else if (type == 1){ changedValue << (value + moving.y); }
				else if (type == 2){
					wxString XorY = re.GetMatch(txt, 1);
					if (XorY == "x")
						changedValue << (value + moving.x);
					else if (XorY == "y")
						changedValue << (value + moving.y);
				}
			}
			
			if (re.GetMatch(&startMatch, &lenMatch, 1) && !changedValue.empty()){
				if (lenMatch){ txt.erase(txt.begin() + startMatch, txt.begin() + startMatch + lenMatch); }
				txt.insert(startMatch, changedValue);
			}
			textPosition = startMatch + lenMatch;
		}
		if (dummy){
			Dialogue Cpy = Dialogue(*Dial);
			if (istexttl) {
				Cpy.TextTl = txt;
				wxString tlLines;
				Cpy.GetRaw(&tlLines, true);
				Cpy.GetRaw(&tlLines, false, tlModeStyle);
				dtxt->insert(selPositions[i] + moveLength, tlLines);
				moveLength += tlLines.Len();
			}
			else{
				Cpy.Text = txt;
				wxString thisLine;
				Cpy.GetRaw(&thisLine);
				dtxt->insert(selPositions[i] + moveLength, thisLine);
				moveLength += thisLine.Len();
			}


		}
		else{
			tab->Grid->CopyDialogue(sels[i])->Text = txt;
		}
	}
	if (dummy){
		if (!tab->Video->OpenSubs(dtxt)){ KaiLog(_("Nie mo¿na otworzyæ napisów")); }
		tab->Video->VisEdit = true;
		tab->Video->Render();
	}
	else{
		tab->Video->VisEdit = true;
		if (tab->Edit->splittedTags){ tab->Edit->TextEditOrig->modified = true; }
		tab->Grid->SetModified(VISUAL_POSITION_SHIFTER, true);
		tab->Grid->Refresh();
	}
}


void ScaleRotation::ChangeTool(int _tool)
{
	selectedTool = _tool;
	SetCurVisual();
	tab->Video->Render(false);
}

