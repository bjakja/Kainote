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


#include "TabPanel.h"
#include "RendererVideo.h"
#include "VideoBox.h"
#include "SubsGrid.h"
#include "EditBox.h"
#include "config.h"
#include "VisualDrawingShapes.h"
#include "VisualClips.h"
#include "Visuals.h"

RotationZ::RotationZ()
	: Visuals()
	, isOrg(false)
	, org(0, 0)
{
}

void RotationZ::DrawVisual(int time)
{
	if (hasTwoPoints) {
		if (visibility[1]) {
			line->SetWidth(2.f);
			line->Begin();
			line->Draw(twoPoints, 2, 0xFFBB0000);
			line->End();
			DrawRect(twoPoints[1], hover[1], 4.f);
		}
		if (visibility[0]) {
			DrawRect(twoPoints[0], hover[0], 4.f);
		}
		return;
	}
	
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
	//changed every function to float version
	float radius = sqrtf(powf(fabs(org.x - from.x), 2) + powf(fabs(org.y - from.y), 2)) + 40;
	D3DXVECTOR2 v2[6];
	VERTEX v5[726];
	D3DCOLOR color(0xAA121150);
	D3DCOLOR color1(0xAAFF0000);
	CreateVERTEX(&v5[0], org.x, org.y + (radius + 10.f), color);
	CreateVERTEX(&v5[1], org.x, org.y + radius, color);
	for (int j = 0; j < 181; j++){
		float xx = org.x + ((radius + 10.f) * sin((j * 2) * rad));
		float yy = org.y + ((radius + 10.f) * cos((j * 2) * rad));
		float xx1 = org.x + (radius * sin((j * 2) * rad));
		float yy1 = org.y + (radius * cos((j * 2) * rad));
		CreateVERTEX(&v5[j + 364], xx, yy, color1);
		CreateVERTEX(&v5[j + 545], xx1, yy1, color1);
		if (j < 1){ continue; }
		CreateVERTEX(&v5[(j * 2)], xx, yy, color);
		CreateVERTEX(&v5[(j * 2) + 1], xx1, yy1, color);

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

	HRN(device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 360, v5, sizeof(VERTEX)), L"primitive failed");
	HRN(device->DrawPrimitiveUP(D3DPT_LINESTRIP, 180, &v5[364], sizeof(VERTEX)), L"primitive failed");
	HRN(device->DrawPrimitiveUP(D3DPT_LINESTRIP, 180, &v5[545], sizeof(VERTEX)), L"primitive failed");
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

void RotationZ::OnMouseEvent(wxMouseEvent &evt)
{
	if (blockevents){ return; }
	bool click = evt.LeftDown() || evt.RightDown() || evt.MiddleDown();
	bool holding = (evt.LeftIsDown() || evt.RightIsDown() || evt.MiddleIsDown());

	int x, y;
	evt.GetPosition(&x, &y);
	float pointCatch = 5;

	if (evt.ButtonUp()){
		if (tab->video->HasCapture()){ tab->video->ReleaseMouse(); }
		SetVisual(false);
		to = org;
		if (isOrg){
			lastmove.x = atan2((org.y - y), (org.x - x)) * (180.f / 3.1415926536f);
			lastmove.x += lastmove.y;
		}
		tab->video->Render();
		if (!tab->video->HasArrow()){ tab->video->SetCursor(wxCURSOR_ARROW); }
		isOrg = false;
		grabbed = -1;
	}
	if (hasTwoPoints && evt.Moving()) {
		if (fabs(twoPoints[0].x - x) < pointCatch && fabs(twoPoints[0].y - y) < pointCatch) {
			hover[0] = true;
			tab->video->Render();
		}
		else if (fabs(twoPoints[1].x - x) < pointCatch && fabs(twoPoints[1].y - y) < pointCatch) {
			hover[1] = true;
			tab->video->Render();
		}
		else if (hover[0] || hover[1]) {
			hover[0] = false;
			hover[1] = false;
			tab->video->Render();
		}

	}

	if (click){
		tab->video->CaptureMouse();
		if (hasTwoPoints) {
			if (!visibility[0]) {
				visibility[0] = true;
				twoPoints[0].x = x;
				twoPoints[0].y = y;
				tab->video->Render(false);
				return;
			}
			else if (fabs(twoPoints[0].x - x) < pointCatch && fabs(twoPoints[0].y - y) < pointCatch) {
				diffs.x = twoPoints[0].x - x;
				diffs.y = twoPoints[0].y - y;
				grabbed = 0;
			}
			else if (!visibility[1]) {
				visibility[1] = true;
				twoPoints[1].x = x;
				twoPoints[1].y = y;
			}
			else if (fabs(twoPoints[1].x - x) < pointCatch && fabs(twoPoints[1].y - y) < pointCatch) {
				diffs.x = twoPoints[1].x - x;
				diffs.y = twoPoints[1].y - y;
				grabbed = 1;
			}
			else {
				return;
			}
			if (changeAllTags) {
				lastAngle = lastmove.y;
			}
			isfirst = true;
			tab->video->SetCursor(wxCURSOR_SIZING);
			SetVisual(true);
		}
		else {
			tab->video->SetCursor(wxCURSOR_SIZING);
			if (fabs(org.x - x) < pointCatch && fabs(org.y - y) < pointCatch) {
				isOrg = true;
				lastOrg = org;
				diffs.x = org.x - x;
				diffs.y = org.y - y;
				return;
			}
			else {
				lastmove.x = atan2((org.y - y), (org.x - x)) * (180.f / 3.1415926536f);
				lastmove.x += lastmove.y;
			}
			if (changeAllTags) {
				lastAngle = lastmove.y;
			}
		}
	}
	else if (holding){
		isfirst = true;
		if (hasTwoPoints) {
			if (grabbed != -1) {
				twoPoints[grabbed].x = x + diffs.x;
				twoPoints[grabbed].y = y + diffs.y;
				SetVisual(true);
			}
		}
		else {
			if (isOrg) {
				org.x = x + diffs.x;
				org.y = y + diffs.y;
				//type also have number 100 to be recognized
				SetVisual(true);
				return;
			}
			to.x = x; to.y = y;
			SetVisual(true);
		}
	}

}

void RotationZ::SetCurVisual()
{
	D3DXVECTOR2 linepos = GetPosnScale(nullptr, nullptr, moveValues);
	if (moveValues[6] > 3){ linepos = CalcMovePos(); }
	from = D3DXVECTOR2(((linepos.x / coeffW) - zoomMove.x) * zoomScale.x,
		((linepos.y / coeffH) - zoomMove.y) * zoomScale.y);
	double lastfrz = lastmove.y;
	lastmove = D3DXVECTOR2(0, 0);
	wxString res;
	if (FindTag(L"frz?([0-9.-]+)", currentLineText, changeAllTags)){
		double result = 0.; 
		GetDouble(&result);
		lastmove.y = result;
		lastmove.x += lastmove.y;
	}
	else{
		Styles *actualStyle = tab->grid->GetStyle(0, tab->edit->line->Style);
		double result = 0.; 
		actualStyle->Angle.ToDouble(&result);
		lastmove.y = result;
		lastmove.x += lastmove.y;
	}
	if (FindTag(L"org\\(([^\\)]+)", currentLineText)){
		double orx, ory;
		if (GetTwoValueDouble(&orx, &ory)) {
			org.x = ((orx / coeffW) - zoomMove.x) * zoomScale.x;
			org.y = ((ory / coeffH) - zoomMove.y) * zoomScale.y; 
		}
		else {
			org = from;
		}
	}
	else{ org = from; }
	to = org;
	if (hasTwoPoints && fabs(lastfrz - lastmove.y) > 0.01) {
		visibility[0] = false;
		visibility[1] = false;
	}
}

void RotationZ::ChangeVisual(wxString *txt, Dialogue *dial, size_t numOfSelections)
{
	if (hasTwoPoints && (!visibility[0] || !visibility[1]))
		return;

	if (isOrg){
		ChangeOrg(txt, dial, (((org.x - lastOrg.x) / zoomScale.x) + zoomMove.x) * coeffW,
			(((org.y - lastOrg.y) / zoomScale.y) + zoomMove.y) * coeffH);
		return;
	}

	float angle;
	if (hasTwoPoints) {
		angle = atan2((twoPoints[0].y - twoPoints[1].y), (twoPoints[0].x - twoPoints[1].x)) * (180.f / 3.1415926536f);
		angle = -angle + 180;
	}
	else
		angle = lastmove.x - atan2((org.y - to.y), (org.x - to.x)) * (180.f / 3.1415926536f);

	angle = fmodf(angle + 360.f, 360.f);

	if (changeAllTags) {
		//rotate all positions to not destroy image that it 
		//presents before rotation.
		if (preserveProportions) {
			float posRotationAngle = (lastAngle - angle);
			bool putInBracket = false;
			wxPoint textPos;
			D3DXVECTOR2 pos = GetPosition(dial, &putInBracket, &textPos);
			float rad = 0.01745329251994329576923690768489f;
			D3DXVECTOR2 orgpivot = { ((org.x / zoomScale.x) + zoomMove.x) * coeffW,
			((org.y / zoomScale.y) + zoomMove.y) * coeffH };
			if (FindTag(L"org\\(([^\\)]+)")) {
				double orx, ory;
				if (GetTwoValueDouble(&orx, &ory)) {
					orgpivot = { (float)orx, (float)ory };
				}
			}
			float s = sin(posRotationAngle * rad);
			float c = cos(posRotationAngle * rad);
			RotateZ(&pos, s, c, orgpivot);
			wxString posstr = L"\\pos(" + getfloat(pos.x) + L"," + getfloat(pos.y) + L")";
			if (moveValues[6] > 2) {
				D3DXVECTOR2 pos1(moveValues[2] - moveValues[0], moveValues[3] - moveValues[1]);
				int startTime = ZEROIT(tab->edit->line->Start.mstime);
				posstr = L"\\move(" + getfloat(pos.x) + L"," + getfloat(pos.y) + L"," + 
					getfloat(pos.x + pos1.x) + L"," + getfloat(pos.y + pos1.y) + L"," +
					getfloat(moveValues[4] - startTime, L"6.0f") + L"," +
					getfloat(moveValues[5] - startTime, L"6.0f") + L")";
			}
			if (putInBracket) { posstr = L"{" + posstr + L"}"; }
			txt->replace(textPos.x, textPos.y, posstr);
			ChangeClipRotationZ(txt, orgpivot, s, c);
		}
		auto replfunc = [=](const FindData& data, wxString* result) {
			float newangle = angle;
			float oldangle = data.finding.empty()? 0 : wxAtof(data.finding);
			newangle = oldangle - (lastAngle - angle);
			newangle = fmodf(newangle + 360.f, 360.f);
			if (isfirst) {
				lastmove.y = newangle;
				isfirst = false;
			}
			*result = getfloat(newangle);
		};
		ReplaceAll(L"frz([0-9.-]+)", L"frz", txt, replfunc, true);
	}
	else {
		lastmove.y = angle;
		wxString tag = L"\\frz" + getfloat(angle);
		wxString val;
		FindTag(L"frz?([0-9.-]+)", *txt, 1);
		Replace(tag, txt);
	}
}

wxPoint RotationZ::ChangeVisual(wxString* txt)
{
	if (hasTwoPoints && (!visibility[0] || !visibility[1])) {
		return wxPoint();
	}

	if (isOrg) {
		FindTag(L"org\\(([^\\)]+)", *txt, 1);
		wxString visual = L"\\org(" + getfloat(((org.x / zoomScale.x) + zoomMove.x) * coeffW) + L"," +
			getfloat(((org.y / zoomScale.y) + zoomMove.y) * coeffH) + L")";

		Replace(visual, txt);
		return GetPositionInText();
	}

	float angle;
	if (hasTwoPoints) {
		angle = atan2((twoPoints[0].y - twoPoints[1].y), (twoPoints[0].x - twoPoints[1].x)) * (180.f / 3.1415926536f);
		angle = -angle + 180;
	}
	else
		angle = lastmove.x - atan2((org.y - to.y), (org.x - to.x)) * (180.f / 3.1415926536f);
	angle = fmodf(angle + 360.f, 360.f);
	if (changeAllTags) {
		auto replfunc = [=](const FindData& data, wxString* result) {
			float newangle = angle;
			if (!data.finding.empty()) {
				float oldangle = wxAtof(data.finding);
				newangle = oldangle - (lastAngle - angle);
				newangle = fmodf(newangle + 360.f, 360.f);
			}
			if (isfirst) {
				lastmove.y = newangle;
				isfirst = false;
			}
			*result = getfloat(newangle);
		};
		ReplaceAll(L"frz([0-9.-]+)", L"frz", txt, replfunc, true);
		FindTag(L"frz?([0-9.-]+)", *txt);
	}
	else {
		lastmove.y = angle;
		wxString tag = L"\\frz" + getfloat(angle);
		wxString val;
		FindTag(L"frz?([0-9.-]+)", *txt);
		Replace(tag, txt);
	}
	return GetPositionInText();
}

void RotationZ::ChangeTool(int _tool, bool blockSetCurVisual) {
	bool twoPointsTool = (_tool & 1) != 0;
	bool oldChangeAllTags = changeAllTags;
	changeAllTags = (_tool & 2) != 0;
	replaceTagsInCursorPosition = !changeAllTags;
	preserveProportions = (_tool & 4) != 0;
	if (oldChangeAllTags != changeAllTags) {
		if (!blockSetCurVisual) {
			SetCurVisual();
			tab->video->Render(false);
		}
	}
	else if (twoPointsTool != hasTwoPoints) {
		hasTwoPoints = twoPointsTool;
		visibility[0] = false;
		visibility[1] = false;
		if (!blockSetCurVisual)
			tab->video->Render(false);
	}
};

void RotationZ::OnKeyPress(wxKeyEvent &evt)
{
	/*if (!hasTwoPoints)
		return;

	if (evt.ControlDown() && evt.GetKeyCode() == L'A') {
		selection[0] = true;
		selection[1] = true;
		tab->video->Render(false);
	}*/
}

void RotationZ::ChangeClipRotationZ(wxString* txt, const D3DXVECTOR2& orgPivot, float sinus, float cosinus)
{
	if (FindTag(L"(i?clip[^)]+\\))", *txt, 1)) {
		wxString clip1, clip;
		GetTextResult(&clip1);
		std::vector<ClipPoint> points;
		wxString newclip = L"\\" + clip1.BeforeFirst(L'(', &clip) + L"(";
		int vectorScale = 1;
		int clipFreq = clip.Freq(L',');
		if (clipFreq >= 3) {
			wxStringTokenizer tokenzr(clip, L",", wxTOKEN_STRTOK);
			double value;
			float xy[4] = { 0,0,0,0 };
			int counter = 0;
			bool isBad = false;
			while (tokenzr.HasMoreTokens()) {
				wxString token = tokenzr.GetNextToken();
				if (token.ToDouble(&value)) {
					xy[counter] = value;
					counter++;
				}
				else
					isBad = true;
			}
			if (!isBad) {
				points.push_back(ClipPoint(xy[0], xy[1], "m", true));
				points.push_back(ClipPoint(xy[2], xy[1], "l", true));
				points.push_back(ClipPoint(xy[2], xy[3], "l", true));
				points.push_back(ClipPoint(xy[0], xy[3], "l", true));
			}
			else {
				KaiLog(_("Nie można pobrać wartości clipa prostokątnego."));
			}
		}
		else {
			if (clipFreq >= 1) {
				wxString vscale = clip.BeforeFirst(L',', &clip1);
				int vscaleint = wxAtoi(vscale);
				if (vscaleint > 0)
					vectorScale = vscaleint;

				clip = clip1;
			}
			GetVectorPoints(clip, &points);
		}
		size_t psize = points.size();
		if (psize) {
			wxString format = L"5.0f";
			wxString lasttype;
			int countB = 0;
			bool spline = false;
			int vscale = pow(2, (vectorScale - 1));
			if (vectorScale > 1) {
				newclip << vectorScale << L",";
			}
			for (size_t i = 0; i < psize; i++)
			{
				ClipPoint pos = points[i];
				D3DXVECTOR2 posxy = D3DXVECTOR2(pos.x, pos.y);
				RotateZ(&posxy, sinus, cosinus, orgPivot);
				float x = posxy.x + 0.5, 
					  y = posxy.y + 0.5;

				if (countB && !pos.start) {
					newclip << getfloat(x, format) << L" " << getfloat(y, format) << L" ";
					countB++;
				}
				else {
					if (spline) { newclip << L"c "; spline = false; }
					if (lasttype != pos.type || pos.type == L"m") {
						newclip << pos.type << L" ";
						lasttype = pos.type;
					}
					newclip << getfloat(x, format) << L" " << getfloat(y, format) << L" ";
					if (pos.type == L"b" || pos.type == L"s") {
						countB = 1;
						if (pos.type == L"s")
							spline = true;
					}
				}
				//fix for m one after another
				if (pos.type == L"m" && psize > 1 && ((i >= psize - 1) ||
					(i < psize - 1 && points[i + 1].type == L"m"))) {
					newclip << L"l " << getfloat(x, format) << L" " << getfloat(y, format) << L" ";
				}
			}
			if (spline) { newclip << L"c "; }
			newclip.Trim();
			Replace(newclip, txt);
		}
	}
}

