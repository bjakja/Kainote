//  Copyright (c) 2016, Marcin Drob

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

RotationXY::RotationXY()
	: Visuals()
	, isOrg(false)
	, type(0)
	, angle(0, 0)
	, oldAngle(0, 0)
	, org(0, 0)
{
}

void RotationXY::DrawVisual(int time)
{
	if (time != oldtime && moveValues[6] > 3){
		BOOL noOrg = (org == from);
		from = CalcMovePos();
		from.x = ((from.x / coeffW) - zoomMove.x) * zoomScale.x;
		from.y = ((from.y / coeffH) - zoomMove.y) * zoomScale.y;
		to = from;
		if (noOrg)
			org = from;
		else
			to = org;
	}
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
	device->SetTransform(D3DTS_PROJECTION, &(matProjection * matTramsate));    // set the projection

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

	HRN(device->SetTransform(D3DTS_PROJECTION, &matOrtho), _("Nie można ustawić macierzy projekcji"));
	HRN(device->SetTransform(D3DTS_WORLD, &matIdentity), _("Nie można ustawić macierzy świata"));
	HRN(device->SetTransform(D3DTS_VIEW, &matIdentity), _("Nie można ustawić macierzy widoku"));
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

wxString RotationXY::GetVisual()
{
	if (isOrg){
		return L"\\org(" + getfloat(((org.x / zoomScale.x) + zoomMove.x) * coeffW) + L"," +
			getfloat(((org.y / zoomScale.y) + zoomMove.y) * coeffH) + L")";
	}

	wxString result;
	if (type != 1){
		angle.x = (to.x - firstmove.x) + oldAngle.x;
		angle.x = fmodf(angle.x + 360.f, 360.f);
		result += L"\\fry" + getfloat(angle.x);

	}
	if (type != 0){
		//swap plus to minus to not keep oldAngle and angle in minuses
		float angy = (to.y - firstmove.y) - oldAngle.y;
		angle.y = fmodf((-angy) + 360.f, 360.f);
		result += L"\\frx" + getfloat(angle.y);

	}
	return result;
}

void RotationXY::OnMouseEvent(wxMouseEvent &evt)
{
	if (blockevents){ return; }
	bool click = evt.LeftDown() || evt.RightDown() || evt.MiddleDown();
	bool holding = (evt.LeftIsDown() || evt.RightIsDown() || evt.MiddleIsDown());
	bool leftc = evt.LeftDown();
	bool rightc = evt.RightDown();
	bool middlec = evt.MiddleDown();

	int x, y;
	evt.GetPosition(&x, &y);

	if (evt.ButtonUp()){
		if (tab->Video->HasCapture()){ tab->Video->ReleaseMouse(); }
		SetVisual(false, type);
		oldAngle = angle;
		if (!hasArrow){ tab->Video->SetCursor(wxCURSOR_ARROW); hasArrow = true; }
		isOrg = false;
	}

	if (click){
		tab->Video->CaptureMouse();
		if (leftc){ type = 0; }//fry
		if (rightc){ type = 1; }//frx
		if (middlec){ type = 2; }//frx + fry
		if (abs(org.x - x) < 8 && abs(org.y - y) < 8){
			isOrg = true;
			lastOrg = org;
			diffs.x = org.x - x;
			diffs.y = org.y - y;
		}
		firstmove = D3DXVECTOR2(x, y);
		if (type == 0){ tab->Video->SetCursor(wxCURSOR_SIZEWE); }
		if (type == 1){ tab->Video->SetCursor(wxCURSOR_SIZENS); }
		if (type == 2){ tab->Video->SetCursor(wxCURSOR_SIZING); }
		hasArrow = false;
	}
	else if (holding){
		if (isOrg){
			org.x = x + diffs.x;
			org.y = y + diffs.y;
			SetVisual(true, 100);//type also have number 100 for org.
			return;
		}
		to.x = x; to.y = y;
		SetVisual(true, type);
	}

}

void RotationXY::SetCurVisual()
{
	D3DXVECTOR2 linepos = GetPosnScale(NULL, &AN, moveValues);
	if (moveValues[6] > 3){ linepos = CalcMovePos(); }
	from = to = D3DXVECTOR2(((linepos.x / coeffW) - zoomMove.x) * zoomScale.x,
		((linepos.y / coeffH) - zoomMove.y) * zoomScale.y);

	wxString res;
	oldAngle = D3DXVECTOR2(0, 0);
	if (tab->Edit->FindValue(L"frx([0-9.-]+)", &res)){
		double result = 0; res.ToDouble(&result);
		oldAngle.y = result;
	}
	if (tab->Edit->FindValue(L"fry([0-9.-]+)", &res)){
		double result = 0; res.ToDouble(&result);
		oldAngle.x = result;
	}
	if (tab->Edit->FindValue(L"org\\(([^\\)]+)", &res)){
		wxString rest;
		double orx, ory;
		if (res.BeforeFirst(L',', &rest).ToDouble(&orx)){ org.x = ((orx / coeffW) - zoomMove.x) * zoomScale.x; }
		if (rest.ToDouble(&ory)){ org.y = ((ory / coeffH) - zoomMove.y) * zoomScale.y; }
	}
	else{ org = from; }
	firstmove = to;
	angle = oldAngle;
	lastmove = org;

}

void RotationXY::ChangeVisual(wxString *txt, Dialogue *dial)
{
	if (isOrg){
		ChangeOrg(txt, dial, (((org.x - lastOrg.x) / zoomScale.x) + zoomMove.x)*coeffW,
			(((org.y - lastOrg.y) / zoomScale.y) + zoomMove.y)*coeffH);
		return;
	}

	wxString tag;
	wxString val;
	if (type != 1){
		angle.x = (to.x - firstmove.x) + oldAngle.x;
		angle.x = fmodf(angle.x + 360.f, 360.f);
		tag = L"\\fry" + getfloat(angle.x);
		tab->Edit->FindValue(L"fry([0-9.-]+)", &val, *txt, 0, 1);
		ChangeText(txt, tag, tab->Edit->InBracket, tab->Edit->Placed);
	}
	if (type != 0){
		//swap plus to minus to not keep oldAngle and angle in minuses
		float angy = (to.y - firstmove.y) - oldAngle.y;
		angle.y = fmodf((-angy) + 360.f, 360.f);
		tag = L"\\frx" + getfloat(angle.y);
		tab->Edit->FindValue(L"frx([0-9.-]+)", &val, *txt, 0, 1);
		ChangeText(txt, tag, tab->Edit->InBracket, tab->Edit->Placed);
	}

}

void RotationXY::OnKeyPress(wxKeyEvent &evt)
{

}