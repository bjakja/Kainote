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

Scale::Scale()
	: Visuals()
	, type(0)
	, grabbed(-1)
{
}

void Scale::DrawVisual(int time)
{
	
	if (time != oldtime && moveValues[6] > 3){
		from = CalcMovePos();
		from.x = ((from.x / coeffW) - zoomMove.x) * zoomScale.x;
		from.y = ((from.y / coeffH) - zoomMove.y) * zoomScale.y;
		to.x = from.x + (scale.x * arrowLengths.x);
		to.y = from.y + (scale.y * arrowLengths.y);
	}

	D3DXVECTOR2 v4[15];
	
	v4[0] = from;//horizontal arrow
	v4[1].x = to.x;
	v4[1].y = from.y;//horizontal arrow
	v4[2] = from;//skew arrow
	v4[3].x = to.x;
	v4[3].y = to.y;//skew arrow
	v4[4] = from;//vertical arrow
	v4[5].x = from.x;
	v4[5].y = to.y;//vertical arrow

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

wxString Scale::GetVisual()
{
	wxString result;
	
	if (type != 1){
		result += L"\\fscx" + getfloat(scale.x * 100);
	}if (type != 0){
		result += L"\\fscy" + getfloat(scale.y * 100);
	}

	return result;
}

void Scale::OnMouseEvent(wxMouseEvent &evt)
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
		if (!hasArrow){ tab->Video->SetCursor(wxCURSOR_ARROW); hasArrow = true; }
	}

	if (!holding){
		if (abs(to.x - x) < 11 && abs(to.y - y) < 11){ 
			if (hasArrow){ tab->Video->SetCursor(wxCURSOR_SIZING); hasArrow = false; } 
		}
		else if (abs(to.x - x) < 11 && abs(from.y - y) < 11){ 
			if (hasArrow){ tab->Video->SetCursor(wxCURSOR_SIZEWE); hasArrow = false; } 
		}
		else if (abs(to.y - y) < 11 && abs(from.x - x) < 11){ 
			if (hasArrow){ tab->Video->SetCursor(wxCURSOR_SIZENS); hasArrow = false; } 
		}
		else if (!hasArrow){ tab->Video->SetCursor(wxCURSOR_ARROW); hasArrow = true; }
	}
	if (click){
		if (leftc){ type = 0; }
		if (rightc){ type = 1; }
		if (middlec || leftc && evt.ShiftDown()){ type = 2; }
		if (abs(to.x - x) < 11 && abs(from.y - y) < 11){ grabbed = 0; type = 0; }
		else if (abs(to.y - y) < 11 && abs(from.x - x) < 11){ grabbed = 1; type = 1; }
		else if (abs(to.x - x) < 11 && abs(to.y - y) < 11){ grabbed = 2; type = 2; }
		diffs.x = to.x - x;
		diffs.y = to.y - y;
		if (type == 0){ tab->Video->SetCursor(wxCURSOR_SIZEWE); }
		if (type == 1){ tab->Video->SetCursor(wxCURSOR_SIZENS); }
		if (type == 2){ tab->Video->SetCursor(wxCURSOR_SIZING); }
		hasArrow = false;
		if (leftc && evt.ShiftDown()){
			type = 2;
			diffs.x = x;
			diffs.y = y;
			return;
		}
		if (grabbed == -1){

			diffs.x = (from.x - x) + (arrowLengths.x * scale.x);
			diffs.y = (from.y - y) + (arrowLengths.y * scale.y);
		}
	}
	else if (holding){
		if (evt.ShiftDown()){
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
			if ((!an3 && (to.x - from.x) < 1 ) || (an3 && (to.x - from.x) > -1)){
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
		scale.x = abs((to.x - from.x) / arrowLengths.x);
		scale.y = abs((to.y - from.y) / arrowLengths.y);

		SetVisual(true, type);
	}
}

void Scale::SetCurVisual()
{
	D3DXVECTOR2 linepos = GetPosnScale(&scale, &AN, moveValues);
	if (moveValues[6] > 3){ linepos = CalcMovePos(); }
	from = D3DXVECTOR2(((linepos.x / coeffW) - zoomMove.x) * zoomScale.x,
		((linepos.y / coeffH) - zoomMove.y) * zoomScale.y);

	arrowLengths.y = (AN > 3) ? 100.f : -100.f, arrowLengths.x = (AN % 3 == 0) ? -100.f : 100.f;
	to.x = from.x + (scale.x * arrowLengths.x);
	to.y = from.y + (scale.y * arrowLengths.y);

}

void Scale::ChangeVisual(wxString *txt, Dialogue *dial)
{
	wxString tag;
	wxString val;

	if (type != 1){
		tag = L"\\fscx" + getfloat(scale.x * 100);

		tab->Edit->FindValue(L"fscx([0-9.-]+)", &val, *txt, 0, 1);
		ChangeText(txt, tag, tab->Edit->InBracket, tab->Edit->Placed);
	}
	if (type != 0){
		tag = L"\\fscy" + getfloat(scale.y * 100);

		tab->Edit->FindValue(L"fscy([0-9.-]+)", &val, *txt, 0, 1);
		ChangeText(txt, tag, tab->Edit->InBracket, tab->Edit->Placed);
	}


}

void Scale::OnKeyPress(wxKeyEvent &evt)
{
	int key = evt.GetKeyCode();
	bool left = key == L'A';
	bool right = key == L'D';
	bool up = key == L'W';
	bool down = key == L'S';
	
	if ((left || right || up || down) && evt.GetModifiers() != wxMOD_ALT){

		float unitx = abs(arrowLengths.x / 100);
		float unity = abs(arrowLengths.y / 100);
		float directionX = (left) ? -unitx : (right) ? unitx : 0;
		float directionY = (up) ? -unity : (down) ? unity : 0;
		type = (directionX) ? 0 : 1;
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
		to.x += directionX;
		to.y += directionY;
		scale.x = abs((to.x - from.x) / arrowLengths.x);
		scale.y = abs((to.y - from.y) / arrowLengths.y);
		
		SetVisual(true, type);
		SetVisual(false, type);
		return;
	}
	evt.Skip();
}