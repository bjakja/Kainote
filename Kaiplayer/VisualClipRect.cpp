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
#include <wx/regex.h>

enum{
	LEFT = 1,
	RIGHT,
	TOP = 4,
	BOTTOM = 8,
	INSIDE = 16,
	OUTSIDE = 32
};

ClipRect::ClipRect()
	: Visuals()
	, invClip(false)
	, showClip(false)
	, grabbed(-1)
{
}

void ClipRect::DrawVisual(int time)
{
	if (!showClip){ return; }
	int x1, x2, y1, y2;
	if (Corner[0].x < Corner[1].x){
		x1 = Corner[0].x;
		x2 = Corner[1].x;
	}
	else{
		x1 = Corner[1].x;
		x2 = Corner[0].x;
	}
	if (Corner[0].y < Corner[1].y){
		y1 = Corner[0].y;
		y2 = Corner[1].y;
	}
	else{
		y1 = Corner[1].y;
		y2 = Corner[0].y;
	}


	D3DXVECTOR2 v2[5];
	wxSize s = VideoSize.GetSize();
	v2[0].x = ((x1 / coeffW) - zoomMove.x) * zoomScale.x;
	v2[0].y = ((y1 / coeffH) - zoomMove.y) * zoomScale.y;
	v2[1].x = v2[0].x;
	v2[1].y = (((y2 / coeffH) - zoomMove.y) * zoomScale.y) - 1;
	v2[2].x = (((x2 / coeffW) - zoomMove.x) * zoomScale.x) - 1;
	v2[2].y = v2[1].y;
	v2[3].x = v2[2].x;
	v2[3].y = v2[0].y;
	v2[4].x = v2[0].x;
	v2[4].y = v2[0].y;
	if (v2[0].x > v2[2].x){
		v2[2].x = v2[3].x = v2[0].x;
	}
	if (v2[0].y > v2[1].y){
		v2[1].y = v2[2].y = v2[0].y;
	}

	if (!invClip){


		VERTEX v24[12];
		CreateVERTEX(&v24[0], 0, 0, 0x88000000);
		CreateVERTEX(&v24[1], s.x, 0, 0x88000000);
		CreateVERTEX(&v24[2], v2[2].x, v2[0].y, 0x88000000);
		CreateVERTEX(&v24[3], v2[0].x, v2[0].y, 0x88000000);
		CreateVERTEX(&v24[4], v2[0].x, v2[2].y, 0x88000000);
		CreateVERTEX(&v24[5], 0, s.y, 0x88000000);
		CreateVERTEX(&v24[6], s.x, s.y, 0x88000000);
		CreateVERTEX(&v24[7], 0, s.y, 0x88000000);
		CreateVERTEX(&v24[8], v2[0].x, v2[2].y, 0x88000000);
		CreateVERTEX(&v24[9], v2[2].x, v2[2].y, 0x88000000);
		CreateVERTEX(&v24[10], v2[2].x, v2[0].y, 0x88000000);
		CreateVERTEX(&v24[11], s.x, 0, 0x88000000);

		HRN(device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 4, v24, sizeof(VERTEX)), L"primitive failed");
		HRN(device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 4, &v24[6], sizeof(VERTEX)), L"primitive failed");
	}
	else{
		VERTEX v24[4];
		CreateVERTEX(&v24[0], v2[0].x, v2[0].y, 0x88000000);
		CreateVERTEX(&v24[1], v2[2].x, v2[0].y, 0x88000000);
		CreateVERTEX(&v24[2], v2[0].x, v2[2].y, 0x88000000);
		CreateVERTEX(&v24[3], v2[2].x, v2[2].y, 0x88000000);
		HRN(device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v24, sizeof(VERTEX)), L"primitive failed");
	}
	line->SetWidth(1);
	line->Begin();
	line->Draw(v2, 5, 0xFFBB0000);
	line->End();
}

void ClipRect::OnMouseEvent(wxMouseEvent &evt)
{
	if (blockevents){ return; }
	bool click = evt.LeftDown();
	bool holding = (evt.LeftIsDown());

	int x, y;
	evt.GetPosition(&x, &y);

	if (evt.ButtonUp()){
		if (tab->Video->HasCapture()){ tab->Video->ReleaseMouse(); }
		if (showClip ){
			if (Corner[1].y == Corner[0].y || Corner[1].x == Corner[0].x)
				showClip = false;

			if (Corner[1].y < Corner[0].y){
				float tmpy = Corner[0].y;
				Corner[0].y = Corner[1].y;
				Corner[1].y = tmpy;
			}
			if (Corner[1].x < Corner[0].x){
				float tmpy = Corner[0].x;
				Corner[0].x = Corner[1].x;
				Corner[1].x = tmpy;
			}
		}
		if (showClip)
			SetVisual(false);

		if (!tab->Video->HasArrow()){ tab->Video->SetCursor(wxCURSOR_ARROW); }
	}

	if (!holding && showClip){

		bool setarrow = false;
		int test = HitTest(D3DXVECTOR2(x, y), false);
		if (test < INSIDE){
			setarrow = true;
			tab->Video->SetCursor((test < 4) ? wxCURSOR_SIZEWE :
				(test >= 4 && test % 4 == 0) ? wxCURSOR_SIZENS :
				(test == (TOP + LEFT) || test == (BOTTOM + RIGHT)) ? wxCURSOR_SIZENWSE : wxCURSOR_SIZENESW);
		}
		if (!setarrow ){ tab->Video->SetCursor(wxCURSOR_ARROW); }
	}
	if (click){
		if (!tab->Video->HasCapture()){ tab->Video->CaptureMouse(); }
		grabbed = OUTSIDE;
		int pointx = ((x / zoomScale.x) + zoomMove.x) *coeffW,
			pointy = ((y / zoomScale.y) + zoomMove.y) *coeffH;
		if (showClip){
			grabbed = HitTest(D3DXVECTOR2(x, y));
			if (grabbed == INSIDE){
				if (Corner[0].x <= pointx && Corner[1].x >= pointx && Corner[0].y <= pointy && Corner[1].y >= pointy){
					diffs.x = x;
					diffs.y = y;
					grabbed = 100;
				}
			}
		}
		if (!showClip || grabbed == OUTSIDE){
			Corner[0].x = Corner[1].x = pointx;
			Corner[0].y = Corner[1].y = pointy;
			grabbed = 1000;
			showClip = true;
		}

	}
	else if (holding && grabbed != -1){

		if (grabbed<16){
			if (grabbed & LEFT || grabbed & RIGHT){
				x = MID(VideoSize.x, x, VideoSize.width);
				Corner[(grabbed & RIGHT) ? 1 : 0].x = ((((x + diffs.x) / zoomScale.x) + zoomMove.x) *coeffW);
				if (grabbed & LEFT && Corner[0].x > Corner[1].x){ Corner[0].x = Corner[1].x; }
				if (grabbed & RIGHT && Corner[1].x < Corner[0].x){ Corner[1].x = Corner[0].x; }
			}
			if (grabbed & TOP || grabbed & BOTTOM){
				y = MID(VideoSize.y, y, VideoSize.height);
				Corner[(grabbed & BOTTOM) ? 1 : 0].y = ((((y + diffs.y) / zoomScale.y) + zoomMove.y) *coeffH);
				if (grabbed & TOP && Corner[0].y > Corner[1].y){ Corner[0].y = Corner[1].y; }
				if (grabbed & BOTTOM && Corner[1].y < Corner[0].y){ Corner[1].y = Corner[0].y; }
			}
		}
		else if (grabbed == 100){
			float movex = (((x - diffs.x) / zoomScale.x) *coeffW),
				movey = (((y - diffs.y) / zoomScale.y) *coeffH);
			Corner[0].x += movex;
			Corner[0].y += movey;
			Corner[1].x += movex;
			Corner[1].y += movey;
			diffs.x = x;
			diffs.y = y;
		}
		else if (grabbed == 1000){
			int pointx = ((x / zoomScale.x) + zoomMove.x) *coeffW,
				pointy = ((y / zoomScale.y) + zoomMove.y) *coeffH;
			//if(Corner[0].x == pointx || Corner[0].y == pointy){return;}
			Corner[1].x = pointx;
			Corner[1].y = pointy;
		}
		SetVisual(true);
	}


}

void ClipRect::SetCurVisual()
{
	int x1 = 0, x2 = SubsSize.x, y1 = 0, y2 = SubsSize.y;
	bool found = FindTag(L"(i?clip[^\\)]+)");
	const FindData& data = GetResult();
	if (found && data.finding.Freq(L',') == 3){
		int match = 1;
		wxRegEx re(L"\\(([0-9-]+)[, ]*([0-9-]+)[, ]*([0-9-]+)[, ]*([0-9-]+)", wxRE_ADVANCED);

		if (re.Matches(data.finding)){
			x1 = wxAtoi(re.GetMatch(data.finding, match));
			y1 = wxAtoi(re.GetMatch(data.finding, match + 1));
			x2 = wxAtoi(re.GetMatch(data.finding, match + 2));
			y2 = wxAtoi(re.GetMatch(data.finding, match + 3));
			showClip = true;
			invClip = data.finding.StartsWith("i");
		}

	}
	else{
		showClip = false;
	}

	Corner[0] = D3DXVECTOR2(x1, y1);
	Corner[1] = D3DXVECTOR2(x2, y2);

}

int ClipRect::HitTest(D3DXVECTOR2 pos, bool diff)
{
	int resultX = 0, resultY = 0, resultInside = 0, resultFinal = 0, oldpointx = 0, oldpointy = 0;
	for (int i = 0; i < 2; i++){
		int pointx = ((Corner[i].x / coeffW) - zoomMove.x)*zoomScale.x,
			pointy = ((Corner[i].y / coeffH) - zoomMove.y)*zoomScale.y;
		bool hasResult = false;
		if (abs(pos.x - pointx) < 5){
			if (diff){
				diffs.x = (pointx)-pos.x;
			}
			resultX |= (i + 1);
		}
		if (abs(pos.y - pointy) < 5){
			if (diff){
				diffs.y = (pointy)-pos.y;
			}
			resultY |= ((i + 1) * 4);
		}
		if (i){
			resultInside |= (resultX ||
				(oldpointx <= pointx && oldpointx <= pos.x && pointx >= pos.x) ||
				(oldpointx >= pointx && oldpointx >= pos.x && pointx <= pos.x)) ? INSIDE : OUTSIDE;
			resultInside |= (resultY ||
				(oldpointx <= pointx && oldpointy <= pos.y && pointy >= pos.y) ||
				(oldpointx >= pointx && oldpointy >= pos.y && pointy <= pos.y)) ? INSIDE : OUTSIDE;
		}
		else{
			oldpointx = pointx;
			oldpointy = pointy;
		}
	}

	resultFinal = (resultInside & OUTSIDE) ? OUTSIDE : INSIDE;
	if (resultFinal == INSIDE){
		resultFinal |= resultX;
		resultFinal |= resultY;
		if (resultFinal > INSIDE){ resultFinal ^= INSIDE; }
	}
	return resultFinal;
}

wxPoint ClipRect::ChangeVisual(wxString* txt)
{
	int x1, x2, y1, y2;
	if (Corner[0].x < Corner[1].x) {
		x1 = Corner[0].x;
		x2 = Corner[1].x;
	}
	else {
		x1 = Corner[1].x;
		x2 = Corner[0].x;
	}
	if (Corner[0].y < Corner[1].y) {
		y1 = Corner[0].y;
		y2 = Corner[1].y;
	}
	else {
		y1 = Corner[1].y;
		y2 = Corner[0].y;
	}
	wxString val;
	wxString tag = wxString::Format(L"\\%sclip(%i,%i,%i,%i)", (invClip) ? L"i" : L"", x1, y1, x2, y2);
	FindTag(L"i?clip(.+)", *txt, 2);
	Replace(tag, txt);
	return GetPositionInText();
}

void ClipRect::ChangeVisual(wxString *txt, Dialogue *dial)
{
	int x1, x2, y1, y2;
	if (Corner[0].x < Corner[1].x){
		x1 = Corner[0].x;
		x2 = Corner[1].x;
	}
	else{
		x1 = Corner[1].x;
		x2 = Corner[0].x;
	}
	if (Corner[0].y < Corner[1].y){
		y1 = Corner[0].y;
		y2 = Corner[1].y;
	}
	else{
		y1 = Corner[1].y;
		y2 = Corner[0].y;
	}
	wxString val;
	wxString tag = wxString::Format(L"\\%sclip(%i,%i,%i,%i)", (invClip) ? L"i" : L"", x1, y1, x2, y2);
	FindTag(L"i?clip(.+)", *txt, 1);
	Replace(tag, txt);
}

void ClipRect::OnKeyPress(wxKeyEvent &evt)
{
	int key = evt.GetKeyCode();
	bool left = key == 'A';
	bool right = key == 'D';
	bool up = key == 'W';
	bool down = key == 'S';

	if ((left || right || up || down) && evt.GetModifiers() != wxMOD_ALT && showClip){
		float directionX = (left) ? -1 : (right) ? 1 : 0;
		float directionY = (up) ? -1 : (down) ? 1 : 0;
		if (evt.ShiftDown()){
			/*if (directionX)
			directionY = directionX;
			else if (directionY)
			directionX = directionY;*/
			directionX /= 10.f;
			directionY /= 10.f;
		}
		/*if (evt.ControlDown()){
		directionX /= 10;
		directionY /= 10;
		}*/

		Corner[1].x += directionX;
		Corner[1].y += directionY;

		SetVisual(true);
		SetVisual(false);
		return;
	}
	evt.Skip();
}

