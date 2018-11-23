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

RotationZ::RotationZ()
	: Visuals()
	, isOrg(false)
	, org(0, 0)
{
}

void RotationZ::DrawVisual(int time)
{
	if (time != oldtime && moveValues[6] > 3){
		BOOL noOrg = (org == from);
		from = CalcMovePos();
		from.x = ((from.x / coeffW) - zoomMove.x)*zoomScale.x;
		from.y = ((from.y / coeffH) - zoomMove.y)*zoomScale.y;
		to = from;
		if (noOrg)
			org = from;
		else
			to = org;
	}
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

wxString RotationZ::GetVisual()
{
	if (isOrg){
		return "\\org(" + getfloat(((org.x / zoomScale.x) + zoomMove.x)*coeffW) + "," +
			getfloat(((org.y / zoomScale.y) + zoomMove.y)*coeffH) + ")";
	}

	float angle = lastmove.x - atan2((org.y - to.y), (org.x - to.x)) * (180.f / 3.1415926536f);
	angle = fmodf(angle + 360.f, 360.f);
	lastmove.y = angle;

	return "\\frz" + getfloat(angle);
}

void RotationZ::OnMouseEvent(wxMouseEvent &evt)
{
	if (blockevents){ return; }
	bool click = evt.LeftDown() || evt.RightDown() || evt.MiddleDown();
	bool holding = (evt.LeftIsDown() || evt.RightIsDown() || evt.MiddleIsDown());

	int x, y;
	evt.GetPosition(&x, &y);

	if (evt.ButtonUp()){
		if (tab->Video->HasCapture()){ tab->Video->ReleaseMouse(); }
		SetVisual(false, (isOrg) ? 100 : 0);
		to = org;
		if (isOrg){
			lastmove.x = atan2((org.y - y), (org.x - x)) * (180.f / 3.1415926536f);
			lastmove.x += lastmove.y;
		}
		tab->Video->Render();
		if (!hasArrow){ tab->Video->SetCursor(wxCURSOR_ARROW); hasArrow = true; }
		isOrg = false;
	}

	if (click){
		tab->Video->CaptureMouse();
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
	else if (holding){
		if (isOrg){
			org.x = x + diffs.x;
			org.y = y + diffs.y;

			SetVisual(true, 100);//type tak¿e ma liczbê 100 by by³o rozpoznawalne.
			return;
		}
		to.x = x; to.y = y;
		SetVisual(true, 0);

	}

}

void RotationZ::SetCurVisual()
{
	D3DXVECTOR2 linepos = GetPosnScale(NULL, NULL, moveValues);
	if (moveValues[6] > 3){ linepos = CalcMovePos(); }
	from = D3DXVECTOR2(((linepos.x / coeffW) - zoomMove.x) * zoomScale.x,
		((linepos.y / coeffH) - zoomMove.y) * zoomScale.y);
	lastmove = D3DXVECTOR2(0, 0);
	wxString res;
	if (tab->Edit->FindValue("frz?([0-9.-]+)", &res)){
		double result = 0.; 
		res.ToDouble(&result);
		lastmove.y = result;
		lastmove.x += lastmove.y;
	}
	else{
		Styles *actualStyle = tab->Grid->GetStyle(0, tab->Edit->line->Style);
		double result = 0.; 
		actualStyle->Angle.ToDouble(&result);
		lastmove.y = result;
		lastmove.x += lastmove.y;
	}
	if (tab->Edit->FindValue("org\\(([^\\)]+)", &res)){
		wxString rest;
		double orx, ory;
		if (res.BeforeFirst(',', &rest).ToDouble(&orx)){ org.x = ((orx / coeffW) - zoomMove.x) * zoomScale.x; }
		if (rest.ToDouble(&ory)){ org.y = ((ory / coeffH) - zoomMove.y) * zoomScale.y; }
	}
	else{ org = from; }
	to = org;

}

void RotationZ::ChangeVisual(wxString *txt, Dialogue *dial)
{
	if (isOrg){
		ChangeOrg(txt, dial, (((org.x - lastOrg.x) / zoomScale.x) + zoomMove.x) * coeffW,
			(((org.y - lastOrg.y) / zoomScale.y) + zoomMove.y) * coeffH);
		return;
	}

	float angle = lastmove.x - atan2((org.y - to.y), (org.x - to.x)) * (180.f / 3.1415926536f);
	angle = fmodf(angle + 360.f, 360.f);
	lastmove.y = angle;

	wxString tag = "\\frz" + getfloat(angle);
	wxString val;
	tab->Edit->FindValue("frz?([0-9.-]+)", &val, *txt, 0, 1);
	ChangeText(txt, tag, tab->Edit->InBracket, tab->Edit->Placed);
}

void RotationZ::OnKeyPress(wxKeyEvent &evt)
{

}