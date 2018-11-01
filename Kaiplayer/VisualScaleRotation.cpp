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
	if (!tagXFound && !tagYFound)
		return;

	if (time != oldtime && moveValues[6] > 3){
		from = CalcMovePos();
		from.x = ((from.x / coeffW) - zoomMove.x)*zoomScale.x;
		from.y = ((from.y / coeffH) - zoomMove.y)*zoomScale.y;
		if (selectedTool == 0){
			int addy = (AN > 3) ? 60 : -60, addx = (AN % 3 == 0) ? -60 : 60;
			to.x = from.x + (scale.x*addx);
			to.y = from.y + (scale.y*addy);
		}
		else{
			if (!hasOrg)
				org = from;
			else
				to = org;
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
	if (hasOrg){
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

	D3DXMatrixRotationYawPitchRoll(&matRotate, D3DXToRadian(-angle.y), D3DXToRadian(angle.x), 0);
	if (from != org){
		float txx = ((from.x / s.x) * 60) - 30;
		float tyy = ((from.y / s.y) * 60) - 30;
		D3DXMatrixTranslation(&matTramsate, (txx - (xxx)), -(tyy - (yyy * 30)), 0.0f);
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
	if (!hasOrg)
		return;

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
	bool click = evt.LeftDown() || evt.RightDown() || evt.MiddleDown();
	bool holding = evt.LeftIsDown() || evt.RightIsDown() || evt.MiddleIsDown();
	bool leftc = evt.LeftDown();
	bool rightc = evt.RightDown();
	bool middlec = evt.MiddleDown();

	int x, y;
	evt.GetPosition(&x, &y);

	if (evt.ButtonUp() && type != 255){
		if (tab->Video->HasCapture()){ tab->Video->ReleaseMouse(); }
		ChangeInLines(false);
		if (selectedTool == 1){
			to = org;
			if (isOrg){
				lastmove.x = atan2((org.y - y), (org.x - x)) * (180.f / 3.1415926536f);
				lastmove.x += lastmove.y;
			}
		}
		else if (selectedTool == 2)
			oldAngle = angle;

		if (!hasArrow){ tab->Video->SetCursor(wxCURSOR_ARROW); hasArrow = true; }
		if (isOrg)
			isOrg = false;
		else
			beforeMove = afterMove;
	}
	if (selectedTool == 0 && !holding){
		if (abs(lastmove.x - x) < 8 && abs(lastmove.y - y) < 8 && (tagXFound || tagYFound)){
			if (hasArrow){ 
				tab->Video->SetCursor((tagXFound && tagYFound) ? wxCURSOR_SIZING : tagYFound ? wxCURSOR_SIZENS : wxCURSOR_SIZEWE);
				hasArrow = false; 
			}
		}
		else if (abs(lastmove.x - x) < 8 && abs(from.y - y) < 8 && tagXFound){
			if (hasArrow){ tab->Video->SetCursor(wxCURSOR_SIZEWE); hasArrow = false; } }
		else if (abs(lastmove.y - y) < 8 && abs(from.x - x) < 8 && tagYFound){
			if (hasArrow){ tab->Video->SetCursor(wxCURSOR_SIZENS); hasArrow = false; } }
		else if (!hasArrow){ tab->Video->SetCursor(wxCURSOR_ARROW); hasArrow = true; }
	}
	if (click){
		tab->Video->CaptureMouse();
		afterMove = beforeMove;
		type = 255;
		if (selectedTool == 0)
			OnClickScaling(x, y, leftc, rightc, middlec, evt.ShiftDown());
		else if (selectedTool == 1)
			OnClickRotationZ(x, y);
		else if (selectedTool == 2)
			OnClickRotationXY(x, y, leftc, rightc, middlec);
	}
	else if (holding && type != 255){
		if (selectedTool == 0)
			OnHoldingScaling(x, y, evt.ShiftDown());
		else
			OnHoldingRotation(x, y);
	}
}

void ScaleRotation::OnClickRotationZ(int x, int y)
{
	if (tagXFound){
		tab->Video->SetCursor(wxCURSOR_SIZING);
		hasArrow = false;
	}
	if (abs(org.x - x) < 8 && abs(org.y - y) < 8 && hasOrg){
		isOrg = true;
		lastOrg = org;
		diffs.x = org.x - x;
		diffs.y = org.y - y;
		return;
	}
	else if(tagXFound){
		type = 0;
		lastmove.x = atan2((org.y - y), (org.x - x)) * (180.f / 3.1415926536f);
		lastmove.x += lastmove.y;
	}
}

void ScaleRotation::OnClickRotationXY(int x, int y, bool leftClick, bool rightClick, bool middleClick)
{
	//by simplify code I have to set frx as type 0 and fry as type 1 even on left click is fry
	if (leftClick && tagYFound){ type = 1; }//fry
	if (rightClick && tagXFound){ type = 0; }//frx
	if (middleClick && (tagXFound || tagYFound)){ 
		type = (tagXFound && tagYFound) ? 2 : tagYFound? 1 : 0;
	}//frx + fry
	if (abs(org.x - x) < 8 && abs(org.y - y) < 8 && hasOrg){
		isOrg = true;
		lastOrg = org;
		diffs.x = org.x - x;
		diffs.y = org.y - y;
	}
	firstmove = D3DXVECTOR2(x, y);
	hasArrow = false;
	if (type == 0){ tab->Video->SetCursor(wxCURSOR_SIZENS); }
	if (type == 1){ tab->Video->SetCursor(wxCURSOR_SIZEWE); }
	if (type == 2){ tab->Video->SetCursor(wxCURSOR_SIZING); }
}

void ScaleRotation::OnClickScaling(int x, int y, bool leftClick, bool rightClick, bool middleClick, bool shiftDown)
{
	int grabbed = -1;
	if (leftClick && tagXFound){ type = 0; }//fscx
	if (rightClick && tagYFound){ type = 1; }//fscy
	if ((middleClick || (leftClick && shiftDown)) && (tagXFound || tagYFound)){ 
		type = (tagXFound && tagYFound) ? 2 : tagYFound ? 1 : 0; 
	}//fscx + fscy
	if (abs(lastmove.x - x) < 8 && abs(from.y - y) < 8 && tagXFound){ grabbed = type = 0; }
	else if (abs(lastmove.y - y) < 8 && abs(from.x - x) < 8 && tagYFound){ grabbed = type = 1; }
	else if (abs(lastmove.x - x) < 8 && abs(lastmove.y - y) < 8 && (tagXFound || tagYFound)){ 
		grabbed = type = (tagXFound && tagYFound) ? 2 : tagYFound ? 1 : 0; 
	}
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
	if (selectedTool == 1){
		float angle = lastmove.x - atan2((org.y - to.y), (org.x - to.x)) * (180.f / 3.1415926536f);
		angle = fmodf(angle + 360.f, 360.f);
		lastmove.y = angle;
		afterMove.x = angle;
	}
	else{
		if (type != 1){
			//frx use y axis moving mouse top bottom
			float angx = (to.y - firstmove.y) - oldAngle.x;
			angle.x = fmodf((-angx) + 360.f, 360.f);
		}
		if (type > 0){
			// fry use x axis moving mouse left right
			float angy = (to.x - firstmove.x) + oldAngle.y;// zmieniony plus na minus by nie trzeba by³o 
			angle.y = fmodf(angy + 360.f, 360.f);//przetrzymywaæ oldAngle i angle w minusach.
		}
		afterMove = angle;
	}
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
		if ((!an3 && to.x - from.x < 1) || (an3 && to.x - from.x > -1)){
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
	if (to.x == from.x){ 
		to.x = from.x + 60.f; 
	}
	if (to.y == from.y){ 
		to.y = from.y + 60.f; 
	}

	if (type != 1){
		float resx = (abs(to.x - from.x)) / 60.f;
		afterMove.x = (resx * 100);
		scale.x = resx;
	}if (type > 0){
		float resy = (abs(to.y - from.y)) / 60.f;
		afterMove.y = (resy * 100);
		scale.y = resy;
	}

	ChangeInLines();
}

void ScaleRotation::SetCurVisual()
{
	D3DXVECTOR2 linepos = GetPosnScale(&scale, &AN, moveValues);
	if (moveValues[6] > 3){ linepos = CalcMovePos(); }
	from = D3DXVECTOR2(((linepos.x / coeffW) - zoomMove.x)*zoomScale.x,
		((linepos.y / coeffH) - zoomMove.y)*zoomScale.y);

	wxString lineText = (tab->Grid->hasTLMode && tab->Edit->line->TextTl != L"") ? tab->Edit->line->TextTl : tab->Edit->line->Text;
	wxString foundTag;
	tagXFound = tagYFound = false;

	if (selectedTool == 0){//scale
		int addy = (AN > 3) ? 60 : -60, addx = (AN % 3 == 0) ? -60 : 60;
		if (SeekTags(lineText, L"fscx([0-9.-]+)", &foundTag))
			tagXFound = true;
		if (SeekTags(lineText, L"fscy([0-9.-]+)", &foundTag))
			tagYFound = true;

		to.x = from.x + (scale.x*addx);
		to.y = from.y + (scale.y*addy);
		beforeMove.x = scale.x * 100;
		beforeMove.y = scale.y * 100;
	}
	else if (selectedTool == 1){//rotationz
		lastmove = beforeMove = D3DXVECTOR2(0, 0);
		if (SeekTags(lineText, L"frz?([0-9.-]+)", &foundTag)){
			double result = 0; 
			if (foundTag.ToDouble(&result)){
				lastmove.y = result;
				lastmove.x += lastmove.y;
				beforeMove.x = result;
				beforeMove.y = result;
				tagXFound = true;
			}
		}
		if (SeekTags(lineText, L"org\\(([^\\)]+)", &foundTag)){
			wxString rest;
			double orx, ory;
			if (foundTag.BeforeFirst(L',', &rest).ToDouble(&orx)){ org.x = ((orx / coeffW) - zoomMove.x)*zoomScale.x; hasOrg = true; }
			if (rest.ToDouble(&ory)){ org.y = ((ory / coeffH) - zoomMove.y)*zoomScale.y; hasOrg = true; }
		}
		else{ org = from; }
		to = org;
	}
	else if (selectedTool == 2){//rotationxy
		oldAngle = beforeMove = D3DXVECTOR2(0, 0);
		if (SeekTags(lineText, L"frx([^\\\\}]+)", &foundTag)){
			double result = 0; 
			if (foundTag.ToDouble(&result)){
				//beforeMove and afterMove have to be x for frx and y for fry
				//it needs normalization to avoid bugs
				oldAngle.x = beforeMove.x = result;
				tagXFound = true;
			}
		}
		if (SeekTags(lineText, L"fry([^\\\\}]+)", &foundTag)){
			double result = 0; 
			if (foundTag.ToDouble(&result)){
				//beforeMove and afterMove have to be x for frx and y for fry
				oldAngle.y = beforeMove.y = result;
				tagYFound = true;
			}
		}
		if (SeekTags(lineText, L"org\\(([^\\)]+)", &foundTag)){
			wxString rest;
			double orx, ory;
			if (foundTag.BeforeFirst(L',', &rest).ToDouble(&orx)){ org.x = ((orx / coeffW) - zoomMove.x)*zoomScale.x; hasOrg = true; }
			if (rest.ToDouble(&ory)){ org.y = ((ory / coeffH) - zoomMove.y)*zoomScale.y; hasOrg = true; }
		}
		else{ org = from; }
		firstmove = to;
		angle = oldAngle;
		lastmove = org;
	}

}

bool ScaleRotation::SeekTags(const wxString &text, const wxString &pattern, wxString *result)
{
	wxRegEx re(pattern, wxRE_ADVANCED);
	if (re.Matches(text)){
		*result = re.GetMatch(text, 1);
		return true;
	}
	return false;
}


void ScaleRotation::ChangeInLines(bool dummy)
{
	bool showOriginalOnVideo = !Options.GetBool(TL_MODE_HIDE_ORIGINAL_ON_VIDEO);
	if (isOrg && selectedTool < 1)
		isOrg = false;

	D3DXVECTOR2 moving = afterMove - beforeMove;
	if (isOrg){
		moving.x = (((org.x - lastOrg.x) / zoomScale.x) + zoomMove.x) * coeffW;
		moving.y = (((org.y - lastOrg.y) / zoomScale.y) + zoomMove.y) * coeffH;
	}
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
				KaiLog(L"Sizes mismatch");
				return;
			}
		}

		dtxt = new wxString(*dummytext);
	}
	bool skipInvisible = dummy && tab->Video->GetState() != Playing;
	wxString tmp;

	const wxString &tlModeStyle = tab->Grid->GetSInfo(L"TLMode Style");
	int moveLength = 0;

	wxString typeString = (type == 0) ? L"x" : (type == 1) ? L"y" : L"([xy])";

	wxString tagpattern = (selectedTool == 0) ? wxString::Format(L"fsc%s([0-9.-]+)", typeString) :
		(isOrg) ? L"org\\(([0-9.-]+)\\,([0-9.-]+)\\)" : (selectedTool == 1) ? L"frz?([0-9.-]+)" : 
		wxString::Format(L"fr%s([0-9.-]+)", typeString);

	wxRegEx re(L"\\\\" + tagpattern, wxRE_ADVANCED);
	if (!re.IsValid()){
		KaiLog(L"Bad pattern " + tagpattern);
	}
	bool hasTlMode = tab->Grid->hasTLMode;

	for (size_t i = 0; i < sels.size(); i++){
		int lineI = sels[i];
		wxString txt;
		Dialogue *Dial = tab->Grid->GetDialogue(lineI);

		if (skipInvisible && !(_time >= Dial->Start.mstime && _time <= Dial->End.mstime)){ continue; }
		bool istexttl = (hasTlMode && Dial->TextTl != L"");
		txt = (istexttl) ? Dial->TextTl : Dial->Text;
		
		size_t startMatch = 0, lenMatch = 0;
		size_t textPosition = 0;
		while (re.Matches(txt.Mid(textPosition))){
			wxString changedValue;
			re.GetMatch(&startMatch, &lenMatch, (type == 2 && !isOrg) ? 2 : 1);
			int position = textPosition + startMatch;
			tmp = txt.Mid(position, lenMatch);
			
			double value=0.0;
			if (tmp.ToDouble(&value)){
				if (isOrg){
					changedValue << (value + moving.x) << L",";
					size_t startSecondOrg = 0, lenSecondOrg = 0;
					re.GetMatch(&startSecondOrg, &lenSecondOrg, 2);
					wxString tmp2 = txt.Mid(textPosition + startSecondOrg, lenSecondOrg);
					if (tmp2.ToDouble(&value)){
						changedValue << (value + moving.y);
						lenMatch = (startSecondOrg - startMatch) + tmp2.Len();
					}
					else
						changedValue = L"";
				}
				else if (type == 0){ changedValue << (value + moving.x); }
				else if (type == 1){ changedValue << (value + moving.y); }
				else if (type == 2){
					wxString XorY = re.GetMatch(txt.Mid(textPosition), 1);
					if (XorY == L"x")
						changedValue << (value + moving.x);
					else if (XorY == L"y")
						changedValue << (value + moving.y);
				}
			}
			
			if (!changedValue.empty()){
				if (lenMatch){ txt.erase(txt.begin() + position, txt.begin() + position + lenMatch); }
				txt.insert(position, changedValue);
				lenMatch = changedValue.Len();
			}
			textPosition += startMatch + lenMatch;
		}
		if (dummy){
			Dialogue Cpy = Dialogue(*Dial);
			if (istexttl) {
				Cpy.TextTl = txt;
				wxString tlLines;
				if (showOriginalOnVideo)
					Cpy.GetRaw(&tlLines, false, tlModeStyle);

				Cpy.GetRaw(&tlLines, true);
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
			if (lineI == tab->Grid->currentLine){
				if (hasTlMode && !istexttl){
					tab->Edit->TextEditOrig->SetTextS(txt, false, false, true);
				}
				else{
					tab->Edit->TextEdit->SetTextS(txt, false, false, true);
				}
			}

		}
		else{
			if (istexttl)
				tab->Grid->CopyDialogue(sels[i])->TextTl = txt;
			else
				tab->Grid->CopyDialogue(sels[i])->Text = txt;
		}
	}
	if (dummy){
		RenderSubs(dtxt);
	}
	else{
		tab->Video->hasVisualEdition = true;
		if (tab->Edit->splittedTags){ tab->Edit->TextEditOrig->SetModified(); }
		tab->Grid->SetModified(VISUAL_SCALE_ROTATION_SHIFTER, true);
		tab->Grid->Refresh();
	}
}


void ScaleRotation::ChangeTool(int _tool)
{
	if (selectedTool == _tool)
		return;

	selectedTool = _tool;
	SetCurVisual();
	tab->Video->Render(false);
}

