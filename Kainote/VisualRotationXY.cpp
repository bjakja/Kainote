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
#include "config.h"
#include "Visuals.h"
#include "TabPanel.h"
#include "VideoCtrl.h"
#include "RendererVideo.h"

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
	D3DXVECTOR3 matrixVector(0.0f, 0.0f, -17.2f);
	D3DXVECTOR3 matrixVector1(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 matrixVector2(0.0f, 1.0f, 0.0f);

	D3DXMatrixLookAtLH(&matView,
		&matrixVector,    // the camera position default -17.2
		&matrixVector1,    // the look-at position
		&matrixVector2);    // the up direction

	device->SetTransform(D3DTS_VIEW, &matView);    // set the view transform to matView

	D3DXMATRIX matProjection;     // the projection transform matrix

	D3DXMatrixPerspectiveFovLH(&matProjection,
		D3DXToRadian(120),    // the horizontal field of view default 120
		ratio, // aspect ratio
		1.0f,    // the near view-plane
		10000.0f);    // the far view-plane

	D3DXMatrixTranslation(&matTramsate, xxx, -yyy, 0.0f);
	D3DXMATRIX matrixTranslate(matProjection * matTramsate);
	device->SetTransform(D3DTS_PROJECTION, &matrixTranslate);    // set the projection

	device->SetTransform(D3DTS_WORLD, &matRotate);

	device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	vertex vertices[199];
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
	device->DrawPrimitiveUP(D3DPT_LINELIST, 44, vertices, sizeof(vertex));
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
	device->DrawPrimitiveUP(D3DPT_LINESTRIP, 2, &vertices[176], sizeof(vertex));
	device->DrawPrimitiveUP(D3DPT_LINELIST, 1, &vertices[179], sizeof(vertex));
	device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 4, &vertices[181], sizeof(vertex));
	device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 4, &vertices[187], sizeof(vertex));
	device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 4, &vertices[193], sizeof(vertex));
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
		if (tab->video->HasCapture()){ tab->video->ReleaseMouse(); }
		SetVisual(false);
		oldAngle = angle;
		if (!tab->video->HasArrow()) { tab->video->SetCursor(wxCURSOR_ARROW); }
		isOrg = false;
	}

	if (click){
		tab->video->CaptureMouse();
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
		if (type == 0){ tab->video->SetCursor(wxCURSOR_SIZEWE); }
		if (type == 1){ tab->video->SetCursor(wxCURSOR_SIZENS); }
		if (type == 2){ tab->video->SetCursor(wxCURSOR_SIZING); }
	}
	else if (holding){
		if (isOrg){
			org.x = x + diffs.x;
			org.y = y + diffs.y;
			SetVisual(true);//type also have number 100 for org.
			return;
		}
		to.x = x; to.y = y;
		SetVisual(true);
	}

}

void RotationXY::SetCurVisual()
{
	D3DXVECTOR2 linepos = GetPosnScale(nullptr, &AN, moveValues);
	if (moveValues[6] > 3){ linepos = CalcMovePos(); }
	from = to = D3DXVECTOR2(((linepos.x / coeffW) - zoomMove.x) * zoomScale.x,
		((linepos.y / coeffH) - zoomMove.y) * zoomScale.y);

	oldAngle = D3DXVECTOR2(0, 0);
	if (FindTag(L"frx([0-9.-]+)", currentLineText, changeAllTags)){
		double result = 0; 
		GetDouble(&result);
		oldAngle.y = result;
	}
	if (FindTag(L"fry([0-9.-]+)", currentLineText, changeAllTags)){
		double result = 0; 
		GetDouble(&result);
		oldAngle.x = result;
	}
	if (FindTag(L"org\\(([^\\)]+)", currentLineText)){
		double orx, ory;
		if (GetTwoValueDouble(&orx, &ory)) {
			org.x = ((orx / coeffW) - zoomMove.x) * zoomScale.x;
			org.y = ((ory / coeffH) - zoomMove.y) * zoomScale.y;
		}
		else { org = from; }
	}
	else{ org = from; }
	firstmove = to;
	angle = oldAngle;
	lastmove = org;

}

void RotationXY::ChangeVisual(wxString *txt, Dialogue *dial, size_t numOfSelections)
{
	if (isOrg){
		ChangeOrg(txt, dial, (((org.x - lastOrg.x) / zoomScale.x) + zoomMove.x)*coeffW,
			(((org.y - lastOrg.y) / zoomScale.y) + zoomMove.y)*coeffH);
		return;
	}

	wxString tag;
	if (type != 1){
		if (changeAllTags) {
			auto replfunc = [=](const FindData& data, wxString* result) {
				float oldangle = data.finding.empty()? oldAngle.x : std::stof(data.finding.ToStdString());
				angle.x = (to.x - firstmove.x) + oldangle;
				angle.x = fmodf(angle.x + 360.f, 360.f);
				*result = getfloat(angle.x);
			};
			ReplaceAll(L"fry([0-9.-]+)", L"fry", txt, replfunc, true);
		}
		else {
			angle.x = (to.x - firstmove.x) + oldAngle.x;
			angle.x = fmodf(angle.x + 360.f, 360.f);
			tag = L"\\fry" + getfloat(angle.x);
			FindTag(L"fry([0-9.-]+)", *txt, 1);
			Replace(tag, txt);
		}
	}
	if (type != 0){
		if (changeAllTags) {
			auto replfunc = [=](const FindData& data, wxString* result) {
				float oldangle = data.finding.empty() ? oldAngle.y : std::stof(data.finding.ToStdString());
				//swap plus to minus to not keep oldAngle and angle in minuses
				float angy = (to.y - firstmove.y) - oldangle;
				angle.y = fmodf((-angy) + 360.f, 360.f);
				*result = getfloat(angle.y);
			};
			ReplaceAll(L"frx([0-9.-]+)", L"frx", txt, replfunc, true);
		}
		else {
			//swap plus to minus to not keep oldAngle and angle in minuses
			float angy = (to.y - firstmove.y) - oldAngle.y;
			angle.y = fmodf((-angy) + 360.f, 360.f);
			tag = L"\\frx" + getfloat(angle.y);
			FindTag(L"frx([0-9.-]+)", *txt, 1);
			Replace(tag, txt);
		}
	}

}

wxPoint RotationXY::ChangeVisual(wxString* txt)
{
	if (isOrg) {
		//org is placed only on time in line
		FindTag(L"org\\(([^\\)]+)", *txt, 1);
		wxString visual = L"\\org(" + getfloat(((org.x / zoomScale.x) + zoomMove.x) * coeffW) + L"," +
			getfloat(((org.y / zoomScale.y) + zoomMove.y) * coeffH) + L")";

		Replace(visual, txt);
		return GetPositionInText();
	}

	wxString tag;
	if (type != 1) {
		if (changeAllTags) {
			auto replfunc = [=](const FindData& data, wxString* result) {
				float oldangle = data.finding.empty() ? oldAngle.x : std::stof(data.finding.ToStdString());
				angle.x = (to.x - firstmove.x) + oldangle;
				angle.x = fmodf(angle.x + 360.f, 360.f);
				*result = getfloat(angle.x);
			};
			ReplaceAll(L"fry([0-9.-]+)", L"fry", txt, replfunc, true);
			//this function don't return any position need to return any for cursor placing
			FindTag(L"frz?([0-9.-]+)", *txt);
		}
		else {
			angle.x = (to.x - firstmove.x) + oldAngle.x;
			angle.x = fmodf(angle.x + 360.f, 360.f);
			tag = L"\\fry" + getfloat(angle.x);
			FindTag(L"fry([0-9.-]+)", *txt);
			Replace(tag, txt);
		}
	}
	if (type != 0) {
		if (changeAllTags) {
			auto replfunc = [=](const FindData& data, wxString* result) {
				float oldangle = data.finding.empty() ? oldAngle.y : std::stof(data.finding.ToStdString());
				//swap plus to minus to not keep oldAngle and angle in minuses
				float angy = (to.y - firstmove.y) - oldangle;
				angle.y = fmodf((-angy) + 360.f, 360.f);
				*result = getfloat(angle.y);
			};
			ReplaceAll(L"frx([0-9.-]+)", L"frx", txt, replfunc, true);
			//this function don't return any position need to return any for cursor placing
			FindTag(L"frz?([0-9.-]+)", *txt);
		}
		else {
			//swap plus to minus to not keep oldAngle and angle in minuses
			float angy = (to.y - firstmove.y) - oldAngle.y;
			angle.y = fmodf((-angy) + 360.f, 360.f);
			tag = L"\\frx" + getfloat(angle.y);
			FindTag(L"frx([0-9.-]+)", *txt);
			Replace(tag, txt);
		}
	}
	return GetPositionInText();
}


void RotationXY::OnKeyPress(wxKeyEvent &evt)
{

}

void RotationXY::ChangeTool(int _tool, bool blockSetCurVisual)
{
	bool oldChangeAllTags = changeAllTags;
	changeAllTags = _tool == 0;
	replaceTagsInCursorPosition = !changeAllTags;
	if (oldChangeAllTags != changeAllTags) {
		if(!blockSetCurVisual)
			SetCurVisual();

		tab->video->Render(false);
	}
}